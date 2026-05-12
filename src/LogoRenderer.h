#pragma once

#include "DirectXOverlay.h"
#include <string>

class LogoRenderer {
public:
    LogoRenderer(DirectXOverlay* overlay);
    ~LogoRenderer();
    
    void initialize();
    void render(Vec2 position, float scale = 1.0f);
    
    void setColor(float r, float g, float b, float a);
    void setGlowIntensity(float intensity);
    
private:
    DirectXOverlay* overlay;
    
    float colorR, colorG, colorB, colorA;
    float glowIntensity;
    
    void renderNeonText(const std::string& text, Vec2 position, float size);
    void renderNeonBox(Vec2 position, Vec2 size);
    void renderGlowEffect(Vec2 position, Vec2 size);
};
