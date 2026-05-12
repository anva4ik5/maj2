#include "LogoRenderer.h"
#include <cmath>

LogoRenderer::LogoRenderer(DirectXOverlay* ov)
    : overlay(ov), colorR(0.0f), colorG(1.0f), colorB(1.0f), colorA(1.0f), glowIntensity(0.5f) {
}

LogoRenderer::~LogoRenderer() {
}

void LogoRenderer::initialize() {
    // Initialize logo resources
}

void LogoRenderer::render(Vec2 position, float scale) {
    if (!overlay || !overlay->isInitialized()) {
        return;
    }
    
    // Render neon logo
    Vec2 logoSize = {200 * scale, 60 * scale};
    
    // Draw glow effect
    renderGlowEffect(position, logoSize);
    
    // Draw neon box
    renderNeonBox(position, logoSize);
    
    // Draw neon text
    renderNeonText("GAMECHEAT", {position.x + logoSize.x / 2, position.y + logoSize.y / 2}, 24 * scale);
}

void LogoRenderer::setColor(float r, float g, float b, float a) {
    colorR = r;
    colorG = g;
    colorB = b;
    colorA = a;
}

void LogoRenderer::setGlowIntensity(float intensity) {
    glowIntensity = intensity;
}

void LogoRenderer::renderNeonText(const std::string& text, Vec2 position, float size) {
    // Calculate text width (approximate)
    float textWidth = text.length() * size * 0.6f;
    Vec2 textPos = {position.x - textWidth / 2, position.y - size / 2};
    
    // Draw glow layers
    for (int i = 3; i > 0; i--) {
        Color glowColor = {colorR, colorG, colorB, colorA * glowIntensity * 0.3f / i};
        overlay->drawText(text, {textPos.x + i, textPos.y + i}, glowColor, size);
    }
    
    // Draw main text
    Color mainColor = {colorR, colorG, colorB, colorA};
    overlay->drawText(text, textPos, mainColor, size);
}

void LogoRenderer::renderNeonBox(Vec2 position, Vec2 size) {
    // Draw glow layers
    for (int i = 3; i > 0; i--) {
        Color glowColor = {colorR, colorG, colorB, colorA * glowIntensity * 0.2f / i};
        Vec2 glowPos = {position.x - i, position.y - i};
        Vec2 glowSize = {size.x + i * 2, size.y + i * 2};
        overlay->drawBox(glowPos, glowSize, glowColor, 2.0f);
    }
    
    // Draw main box
    Color mainColor = {colorR, colorG, colorB, colorA};
    overlay->drawBox(position, size, mainColor, 2.0f);
}

void LogoRenderer::renderGlowEffect(Vec2 position, Vec2 size) {
    // Draw radial glow effect
    Vec2 center = {position.x + size.x / 2, position.y + size.y / 2};
    float maxRadius = (size.x > size.y ? size.x : size.y) * 0.8f;
    
    for (float radius = maxRadius; radius > 0; radius -= 10) {
        float alpha = (radius / maxRadius) * glowIntensity * 0.3f;
        Color glowColor = {colorR, colorG, colorB, alpha};
        
        Vec2 glowPos = {center.x - radius, center.y - radius};
        Vec2 glowSize = {radius * 2, radius * 2};
        
        overlay->drawFilledBox(glowPos, glowSize, glowColor);
    }
}
