#include "DirectXOverlay.h"
#include <iostream>

DirectXOverlay::DirectXOverlay() 
    : device(nullptr), context(nullptr), swapChain(nullptr), 
      renderTargetView(nullptr), blendState(nullptr),
      windowHandle(nullptr), width(0), height(0), initialized(false) {
}

DirectXOverlay::~DirectXOverlay() {
    cleanup();
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
    windowHandle = targetWindow;
    
    // Get window dimensions
    RECT rect;
    GetWindowRect(targetWindow, &rect);
    width = rect.right - rect.left;
    height = rect.bottom - rect.top;
    
    // Create swap chain description
    DXGI_SWAP_CHAIN_DESC swapChainDesc = {};
    swapChainDesc.BufferCount = 1;
    swapChainDesc.BufferDesc.Width = width;
    swapChainDesc.BufferDesc.Height = height;
    swapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    swapChainDesc.BufferDesc.RefreshRate.Numerator = 60;
    swapChainDesc.BufferDesc.RefreshRate.Denominator = 1;
    swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    swapChainDesc.OutputWindow = targetWindow;
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
        // Continue anyway - transparency won't work
    }
    
    initialized = true;
    return true;
}

void DirectXOverlay::cleanup() {
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
    
    initialized = false;
}

void DirectXOverlay::beginFrame() {
    if (!initialized) return;
    
    context->OMSetRenderTargets(1, &renderTargetView, nullptr);
    
    if (blendState) {
        float blendFactor[4] = {1.0f, 1.0f, 1.0f, 1.0f};
        context->OMSetBlendState(blendState, blendFactor, 0xFFFFFFFF);
    }
}

void DirectXOverlay::endFrame() {
    if (!initialized) return;
    
    swapChain->Present(1, 0);
}

void DirectXOverlay::clearScreen(Color color) {
    if (!initialized) return;
    
    float clearColor[4] = {color.r, color.g, color.b, color.a};
    context->ClearRenderTargetView(renderTargetView, clearColor);
}

// Placeholder implementations - would need proper rendering system
void DirectXOverlay::drawText(const std::string& text, Vec2 position, Color color, float size) {
    // TODO: Implement text rendering with DirectWrite or custom font system
    // This requires creating a font texture and rendering quads
}

void DirectXOverlay::drawBox(Vec2 position, Vec2 size, Color color, float thickness) {
    // TODO: Implement box rendering with vertex buffer
}

void DirectXOverlay::drawFilledBox(Vec2 position, Vec2 size, Color color) {
    // TODO: Implement filled box rendering
}

void DirectXOverlay::drawLine(Vec2 start, Vec2 end, Color color, float thickness) {
    // TODO: Implement line rendering
}

void DirectXOverlay::drawCircle(Vec2 center, float radius, Color color, float thickness) {
    // TODO: Implement circle rendering
}
