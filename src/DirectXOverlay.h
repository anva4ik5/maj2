#pragma once

#include <d3d11.h>
#include <d3dcompiler.h>
#include <windows.h>
#include <string>
#include <memory>

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "d3dcompiler.lib")

struct Color {
    float r, g, b, a;
    Color(float r = 1.0f, float g = 1.0f, float b = 1.0f, float a = 1.0f) 
        : r(r), g(g), b(b), a(a) {}
};

struct Vec2 {
    float x, y;
    Vec2(float x = 0.0f, float y = 0.0f) : x(x), y(y) {}
};

struct Vec3 {
    float x, y, z;
    Vec3(float x = 0.0f, float y = 0.0f, float z = 0.0f) : x(x), y(y), z(z) {}
};

class DirectXOverlay {
public:
    DirectXOverlay();
    ~DirectXOverlay();
    
    bool initialize(HWND targetWindow);
    void cleanup();
    
    void beginFrame();
    void endFrame();
    
    void drawText(const std::string& text, Vec2 position, Color color = Color(), float size = 16.0f);
    void drawBox(Vec2 position, Vec2 size, Color color = Color(), float thickness = 1.0f);
    void drawFilledBox(Vec2 position, Vec2 size, Color color = Color());
    void drawLine(Vec2 start, Vec2 end, Color color = Color(), float thickness = 1.0f);
    void drawCircle(Vec2 center, float radius, Color color = Color(), float thickness = 1.0f);
    
    void clearScreen(Color color = Color(0, 0, 0, 0));
    
    bool isInitialized() const { return initialized; }
    
private:
    ID3D11Device* device;
    ID3D11DeviceContext* context;
    IDXGISwapChain* swapChain;
    ID3D11RenderTargetView* renderTargetView;
    ID3D11BlendState* blendState;
    
    HWND windowHandle;
    int width, height;
    bool initialized;
    
    bool createBlendState();
    bool setupRenderTarget();
};
