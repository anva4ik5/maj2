#include "DirectXOverlay.h"
#include <iostream>
#include <vector>

// Windows headers expose DrawText as a macro that resolves to DrawTextW/A.
// D2D has its own ID2D1RenderTarget::DrawText member, so the macro must go.
#ifdef DrawText
#undef DrawText
#endif

// Convert UTF-8 string to wide for DirectWrite
static std::wstring utf8ToWide(const std::string& s) {
    if (s.empty()) return std::wstring();
    int n = MultiByteToWideChar(CP_UTF8, 0, s.c_str(), (int)s.size(), nullptr, 0);
    std::wstring w(n, 0);
    MultiByteToWideChar(CP_UTF8, 0, s.c_str(), (int)s.size(), w.data(), n);
    return w;
}

DirectXOverlay::DirectXOverlay()
    : device(nullptr), context(nullptr), swapChain(nullptr),
      renderTargetView(nullptr), blendState(nullptr),
      windowHandle(nullptr), targetWindow(nullptr), width(0), height(0), initialized(false) {
}

DirectXOverlay::~DirectXOverlay() {
    cleanup();
}

static LRESULT CALLBACK OverlayWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    if (msg == WM_NCHITTEST) return HTTRANSPARENT;
    return DefWindowProc(hwnd, msg, wParam, lParam);
}

static HWND CreateOverlayWindow(HWND target) {
    WNDCLASSEX wc = {};
    wc.cbSize = sizeof(WNDCLASSEX);
    wc.lpfnWndProc = OverlayWndProc;
    wc.hInstance = GetModuleHandle(nullptr);
    wc.lpszClassName = "DXOverlayWindow";
    RegisterClassEx(&wc);

    RECT rc;
    GetWindowRect(target, &rc);

    HWND hwnd = CreateWindowEx(
        WS_EX_LAYERED | WS_EX_TRANSPARENT | WS_EX_TOPMOST | WS_EX_NOACTIVATE,
        wc.lpszClassName,
        "Overlay",
        WS_POPUP,
        rc.left, rc.top,
        rc.right - rc.left, rc.bottom - rc.top,
        nullptr, nullptr, wc.hInstance, nullptr
    );

    if (hwnd) {
        SetLayeredWindowAttributes(hwnd, RGB(0, 0, 0), 0, LWA_COLORKEY);
        ShowWindow(hwnd, SW_SHOW);
    }
    return hwnd;
}

bool DirectXOverlay::createBlendState() {
    D3D11_BLEND_DESC blendDesc = {};
    blendDesc.AlphaToCoverageEnable = FALSE;
    blendDesc.IndependentBlendEnable = FALSE;

    blendDesc.RenderTarget[0].BlendEnable = TRUE;
    blendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
    blendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
    blendDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
    blendDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
    blendDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
    blendDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
    blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

    HRESULT hr = device->CreateBlendState(&blendDesc, &blendState);
    return SUCCEEDED(hr);
}

bool DirectXOverlay::setupRenderTarget() {
    ID3D11Texture2D* backBuffer = nullptr;
    HRESULT hr = swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&backBuffer);

    if (FAILED(hr)) {
        std::cerr << "Failed to get back buffer" << std::endl;
        return false;
    }

    hr = device->CreateRenderTargetView(backBuffer, nullptr, &renderTargetView);
    backBuffer->Release();

    if (FAILED(hr)) {
        std::cerr << "Failed to create render target view" << std::endl;
        return false;
    }

    return true;
}

bool DirectXOverlay::initialize(HWND targetWindow) {
    this->targetWindow = targetWindow;

    // Get window dimensions
    RECT rect;
    GetWindowRect(targetWindow, &rect);
    width = rect.right - rect.left;
    height = rect.bottom - rect.top;

    // Create our own overlay window in this process
    windowHandle = CreateOverlayWindow(targetWindow);
    if (!windowHandle) {
        std::cerr << "Failed to create overlay window" << std::endl;
        return false;
    }

    // Create swap chain description
    DXGI_SWAP_CHAIN_DESC swapChainDesc = {};
    swapChainDesc.BufferCount = 1;
    swapChainDesc.BufferDesc.Width = width;
    swapChainDesc.BufferDesc.Height = height;
    swapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    swapChainDesc.BufferDesc.RefreshRate.Numerator = 60;
    swapChainDesc.BufferDesc.RefreshRate.Denominator = 1;
    swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    swapChainDesc.OutputWindow = windowHandle;
    swapChainDesc.SampleDesc.Count = 1;
    swapChainDesc.Windowed = TRUE;
    swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

    // Create device and swap chain
    D3D_FEATURE_LEVEL featureLevel;
    HRESULT hr = D3D11CreateDeviceAndSwapChain(
        nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, 0,
        nullptr, 0, D3D11_SDK_VERSION,
        &swapChainDesc, &swapChain, &device, &featureLevel, &context
    );

    if (FAILED(hr)) {
        std::cerr << "Failed to create D3D11 device: " << std::hex << hr << std::endl;
        cleanup();
        return false;
    }

    // Setup render target
    if (!setupRenderTarget()) {
        cleanup();
        return false;
    }

    // Create blend state for transparency
    if (!createBlendState()) {
        std::cerr << "Failed to create blend state" << std::endl;
    }
    
    // Setup Direct2D + DirectWrite on top of existing D3D11 swap chain
    if (!setupD2D()) {
        std::cerr << "Failed to setup D2D, text/primitives won't render" << std::endl;
        // Continue anyway - basic D3D11 still works
    }

    initialized = true;
    return true;
}

bool DirectXOverlay::setupD2D() {
    HRESULT hr = D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, &d2dFactory);
    if (FAILED(hr)) return false;
    
    hr = DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED, __uuidof(IDWriteFactory),
                             reinterpret_cast<IUnknown**>(&dwriteFactory));
    if (FAILED(hr)) return false;
    
    // Get DXGI surface from swap chain to bind D2D to it
    IDXGISurface* dxgiSurface = nullptr;
    hr = swapChain->GetBuffer(0, __uuidof(IDXGISurface), (void**)&dxgiSurface);
    if (FAILED(hr)) return false;
    
    D2D1_RENDER_TARGET_PROPERTIES rtProps = D2D1::RenderTargetProperties(
        D2D1_RENDER_TARGET_TYPE_DEFAULT,
        D2D1::PixelFormat(DXGI_FORMAT_UNKNOWN, D2D1_ALPHA_MODE_PREMULTIPLIED),
        96.0f, 96.0f);
    
    hr = d2dFactory->CreateDxgiSurfaceRenderTarget(dxgiSurface, &rtProps, &d2dRenderTarget);
    dxgiSurface->Release();
    if (FAILED(hr)) return false;
    
    // Default text format (Segoe UI 14pt)
    hr = dwriteFactory->CreateTextFormat(
        L"Segoe UI", nullptr,
        DWRITE_FONT_WEIGHT_NORMAL, DWRITE_FONT_STYLE_NORMAL, DWRITE_FONT_STRETCH_NORMAL,
        14.0f, L"en-us", &defaultTextFormat);
    if (FAILED(hr)) return false;
    
    // Reusable solid color brush
    hr = d2dRenderTarget->CreateSolidColorBrush(D2D1::ColorF(1, 1, 1, 1), &solidBrush);
    if (FAILED(hr)) return false;
    
    return true;
}

void DirectXOverlay::cleanupD2D() {
    if (solidBrush) { solidBrush->Release(); solidBrush = nullptr; }
    if (defaultTextFormat) { defaultTextFormat->Release(); defaultTextFormat = nullptr; }
    if (d2dRenderTarget) { d2dRenderTarget->Release(); d2dRenderTarget = nullptr; }
    if (dwriteFactory) { dwriteFactory->Release(); dwriteFactory = nullptr; }
    if (d2dFactory) { d2dFactory->Release(); d2dFactory = nullptr; }
}

void DirectXOverlay::syncWindowPosition() {
    if (!targetWindow || !windowHandle) return;
    RECT rc;
    if (GetWindowRect(targetWindow, &rc)) {
        SetWindowPos(windowHandle, HWND_TOPMOST, rc.left, rc.top,
                     rc.right - rc.left, rc.bottom - rc.top,
                     SWP_NOACTIVATE | SWP_SHOWWINDOW);
    }
}

void DirectXOverlay::cleanup() {
    cleanupD2D();
    
    if (blendState) blendState->Release();
    if (renderTargetView) renderTargetView->Release();
    if (swapChain) swapChain->Release();
    if (context) context->Release();
    if (device) device->Release();

    blendState = nullptr;
    renderTargetView = nullptr;
    swapChain = nullptr;
    context = nullptr;
    device = nullptr;

    if (windowHandle) {
        DestroyWindow(windowHandle);
        windowHandle = nullptr;
    }

    initialized = false;
}

void DirectXOverlay::beginFrame() {
    if (!initialized) return;
    
    context->OMSetRenderTargets(1, &renderTargetView, nullptr);
    
    if (blendState) {
        float blendFactor[4] = {1.0f, 1.0f, 1.0f, 1.0f};
        context->OMSetBlendState(blendState, blendFactor, 0xFFFFFFFF);
    }
    
    // Clear to fully transparent every frame so previous frame doesn't ghost
    float clearColor[4] = {0, 0, 0, 0};
    context->ClearRenderTargetView(renderTargetView, clearColor);
    
    // Begin D2D drawing pass
    if (d2dRenderTarget && !d2dFrameOpen) {
        d2dRenderTarget->BeginDraw();
        d2dFrameOpen = true;
    }
}

void DirectXOverlay::endFrame() {
    if (!initialized) return;
    
    // Close D2D pass before presenting
    if (d2dRenderTarget && d2dFrameOpen) {
        d2dRenderTarget->EndDraw();
        d2dFrameOpen = false;
    }
    
    swapChain->Present(1, 0);
}

void DirectXOverlay::clearScreen(Color color) {
    if (!initialized) return;
    
    float clearColor[4] = {color.r, color.g, color.b, color.a};
    context->ClearRenderTargetView(renderTargetView, clearColor);
}

// === Real implementations using Direct2D / DirectWrite ===

void DirectXOverlay::drawText(const std::string& text, Vec2 position, Color color, float size) {
    if (!d2dRenderTarget || !solidBrush || !dwriteFactory) return;
    
    std::wstring wtext = utf8ToWide(text);
    if (wtext.empty()) return;
    
    // Create text format with requested size (cached default for 14, on-demand otherwise)
    IDWriteTextFormat* fmt = nullptr;
    if (size == 14.0f && defaultTextFormat) {
        fmt = defaultTextFormat;
        fmt->AddRef();
    } else {
        HRESULT hr = dwriteFactory->CreateTextFormat(
            L"Segoe UI", nullptr,
            DWRITE_FONT_WEIGHT_NORMAL, DWRITE_FONT_STYLE_NORMAL, DWRITE_FONT_STRETCH_NORMAL,
            size, L"en-us", &fmt);
        if (FAILED(hr) || !fmt) return;
    }
    
    solidBrush->SetColor(D2D1::ColorF(color.r, color.g, color.b, color.a));
    D2D1_RECT_F layout = D2D1::RectF(position.x, position.y, position.x + 2000.0f, position.y + size * 1.5f);
    d2dRenderTarget->DrawText(wtext.c_str(), (UINT32)wtext.size(), fmt, layout, solidBrush);
    
    fmt->Release();
}

void DirectXOverlay::drawBox(Vec2 position, Vec2 size, Color color, float thickness) {
    if (!d2dRenderTarget || !solidBrush) return;
    solidBrush->SetColor(D2D1::ColorF(color.r, color.g, color.b, color.a));
    D2D1_RECT_F rect = D2D1::RectF(position.x, position.y, position.x + size.x, position.y + size.y);
    d2dRenderTarget->DrawRectangle(rect, solidBrush, thickness);
}

void DirectXOverlay::drawFilledBox(Vec2 position, Vec2 size, Color color) {
    if (!d2dRenderTarget || !solidBrush) return;
    solidBrush->SetColor(D2D1::ColorF(color.r, color.g, color.b, color.a));
    D2D1_RECT_F rect = D2D1::RectF(position.x, position.y, position.x + size.x, position.y + size.y);
    d2dRenderTarget->FillRectangle(rect, solidBrush);
}

void DirectXOverlay::drawLine(Vec2 start, Vec2 end, Color color, float thickness) {
    if (!d2dRenderTarget || !solidBrush) return;
    solidBrush->SetColor(D2D1::ColorF(color.r, color.g, color.b, color.a));
    d2dRenderTarget->DrawLine(
        D2D1::Point2F(start.x, start.y),
        D2D1::Point2F(end.x, end.y),
        solidBrush, thickness);
}

void DirectXOverlay::drawCircle(Vec2 center, float radius, Color color, float thickness) {
    if (!d2dRenderTarget || !solidBrush) return;
    solidBrush->SetColor(D2D1::ColorF(color.r, color.g, color.b, color.a));
    D2D1_ELLIPSE ellipse = D2D1::Ellipse(D2D1::Point2F(center.x, center.y), radius, radius);
    d2dRenderTarget->DrawEllipse(ellipse, solidBrush, thickness);
}
