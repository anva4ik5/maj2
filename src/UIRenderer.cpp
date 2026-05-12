#include "UIRenderer.h"
#include <iostream>
#include <cmath>
#include <algorithm>
#include <windows.h>

UIRenderer::UIRenderer(DirectXOverlay* overlay) 
    : overlay(overlay), menuVisible(false), activeTab(0), menuAlpha(0.0f), animationSpeed(1.0f) {
    menuPosition = {100, 100};
    menuSize = {600, 500};
    initializeStyle();
    initializeTabs();
}

UIRenderer::~UIRenderer() {
}

void UIRenderer::initializeStyle() {
    // Dark hacker style colors
    backgroundColor = {0.08f, 0.08f, 0.1f, 0.95f};  // Dark blue-gray
    primaryColor = {0.13f, 0.13f, 0.18f, 1.0f};       // Slightly lighter
    secondaryColor = {0.18f, 0.18f, 0.25f, 1.0f};    // Medium gray
    accentColor = {0.0f, 0.7f, 1.0f, 1.0f};          // Cyan neon
    textColor = {1.0f, 1.0f, 1.0f, 1.0f};            // White
    disabledColor = {0.4f, 0.4f, 0.4f, 1.0f};        // Gray
}

void UIRenderer::initializeTabs() {
    tabs.clear();
    tabs.emplace_back("Combat", 0);
    tabs.emplace_back("Visuals", 1);
    tabs.emplace_back("Misc", 2);
    tabs.emplace_back("Scripts", 3);
    tabs.emplace_back("KeyBinds", 4);
    tabs.emplace_back("Security", 5);
    
    if (!tabs.empty()) {
        tabs[0].active = true;
    }
}

void UIRenderer::render() {
    if (!overlay || !overlay->isInitialized()) {
        return;
    }
    
    overlay->beginFrame();
    
    if (menuVisible) {
        renderFullMenu();
    }
    
    renderElements();
    
    overlay->endFrame();
}

void UIRenderer::update(float deltaTime) {
    // Update menu alpha animation
    if (menuVisible && menuAlpha < 1.0f) {
        menuAlpha += deltaTime * 3.0f;
        if (menuAlpha > 1.0f) menuAlpha = 1.0f;
    } else if (!menuVisible && menuAlpha > 0.0f) {
        menuAlpha -= deltaTime * 3.0f;
        if (menuAlpha < 0.0f) menuAlpha = 0.0f;
    }
    
    // Update element animations
    for (auto& element : elements) {
        if (element.animation.active) {
            updateAnimation(element.animation, deltaTime);
        }
    }
    
    // Update hover states
    Vec2 mousePos = getMousePosition();
    for (auto& element : elements) {
        element.hovered = isMouseOver(element.position, element.size);
    }
}

void UIRenderer::addText(const std::string& text, Vec2 position, Color color, float size) {
    UIElement element;
    element.text = text;
    element.position = position;
    element.color = color;
    element.visible = true;
    elements.push_back(element);
}

void UIRenderer::addBox(Vec2 position, Vec2 size, Color color, float thickness) {
    UIElement element;
    element.position = position;
    element.size = size;
    element.color = color;
    element.visible = true;
    elements.push_back(element);
}

void UIRenderer::addFilledBox(Vec2 position, Vec2 size, Color color) {
    UIElement element;
    element.position = position;
    element.size = size;
    element.color = color;
    element.visible = true;
    elements.push_back(element);
}

void UIRenderer::addButton(const std::string& text, Vec2 position, Vec2 size, 
                           std::function<void()> onClick, Color color) {
    UIElement element;
    element.text = text;
    element.position = position;
    element.size = size;
    element.color = color;
    element.hoverColor = accentColor;
    element.onClick = onClick;
    element.visible = true;
    element.animation.type = AnimationType::SCALE;
    element.animation.duration = 0.2f;
    elements.push_back(element);
}

void UIRenderer::addToggle(const std::string& text, Vec2 position, bool* value, 
                           std::function<void()> onToggle, Color color) {
    UIElement element;
    element.text = text;
    element.position = position;
    element.color = color;
    element.toggleValue = value;
    element.onToggle = onToggle;
    element.visible = true;
    element.animation.type = AnimationType::GLOW;
    element.animation.duration = 0.3f;
    elements.push_back(element);
}

void UIRenderer::addSlider(const std::string& text, Vec2 position, Vec2 size, 
                           float* value, float min, float max, Color color) {
    UIElement element;
    element.text = text;
    element.position = position;
    element.size = size;
    element.color = color;
    element.visible = true;
    elements.push_back(element);
}

void UIRenderer::addComboBox(const std::string& text, Vec2 position, Vec2 size, 
                             std::vector<std::string> options, int* selectedIndex, Color color) {
    UIElement element;
    element.text = text;
    element.position = position;
    element.size = size;
    element.color = color;
    element.visible = true;
    elements.push_back(element);
}

void UIRenderer::clear() {
    elements.clear();
}

void UIRenderer::renderMenu() {
    if (menuAlpha <= 0.0f) return;
    
    // Apply menu alpha to background
    Color bgAlpha = backgroundColor;
    bgAlpha.a = menuAlpha * backgroundColor.a;
    
    // Render menu background with glow effect
    renderGlowingBox(menuPosition, menuSize, bgAlpha, 0.3f);
    
    // Render menu border
    Color borderColor = accentColor;
    borderColor.a = menuAlpha;
    overlay->drawBox(menuPosition, menuSize, borderColor, 2.0f);
    
    // Render menu header with gradient
    Vec2 headerPos = {menuPosition.x, menuPosition.y};
    Vec2 headerSize = {menuSize.x, 50};
    renderGradientBox(headerPos, headerSize, primaryColor, secondaryColor, true);
    
    // Render menu title
    Color titleColor = accentColor;
    titleColor.a = menuAlpha;
    overlay->drawText("MAJESTIC RP CHEAT", {menuPosition.x + 20, menuPosition.y + 15}, titleColor, 20.0f);
    
    // Render tabs
    renderTabs();
    
    // Render tab content
    renderTabContent();
}

void UIRenderer::renderFullMenu() {
    renderMenu();
}

void UIRenderer::renderTabs() {
    float tabWidth = menuSize.x / tabs.size();
    float tabHeight = 40;
    Vec2 tabPos = {menuPosition.x, menuPosition.y + 50};
    
    for (size_t i = 0; i < tabs.size(); i++) {
        Vec2 currentTabPos = {tabPos.x + i * tabWidth, tabPos.y};
        Vec2 currentTabSize = {tabWidth, tabHeight};
        
        Color tabColor = tabs[i].active ? accentColor : primaryColor;
        tabColor.a = menuAlpha;
        
        overlay->drawFilledBox(currentTabPos, currentTabSize, tabColor);
        
        if (tabs[i].active) {
            overlay->drawBox(currentTabPos, currentTabSize, accentColor, 2.0f);
        }
        
        Color textColor;
        if (tabs[i].active) {
            textColor = Color(1, 1, 1, 1);
        } else {
            textColor = Color(0.7f, 0.7f, 0.7f, 1);
        }
        textColor.a = menuAlpha;
        
        // Center text in tab
        float textWidth = tabs[i].name.length() * 8;
        float textX = currentTabPos.x + (tabWidth - textWidth) / 2;
        overlay->drawText(tabs[i].name, {textX, currentTabPos.y + 12}, textColor, 14.0f);
    }
}

void UIRenderer::renderTabContent() {
    Vec2 contentPos = {menuPosition.x, menuPosition.y + 90};
    Vec2 contentSize = {menuSize.x, menuSize.y - 90};
    
    // Render content background
    Color contentBg = primaryColor;
    contentBg.a = menuAlpha * 0.5f;
    overlay->drawFilledBox(contentPos, contentSize, contentBg);
    
    switch (activeTab) {
        case 0: renderAimbotSection(); break;
        case 1: renderVisualsSection(); break;
        case 2: renderMiscSection(); break;
        case 3: renderJSExecutorSection(); break;
        case 4: renderKeyBindsSection(); break;
        case 5: renderSecuritySection(); break;
    }
}
    yOffset += 30;
    overlay->drawText("[F3] Toggle ESP", {menuPos.x + 10, menuPos.y + yOffset}, {1, 1, 1, 1}, 16.0f);
    yOffset += 30;
    overlay->drawText("[F4] Toggle God Mode", {menuPos.x + 10, menuPos.y + yOffset}, {1, 1, 1, 1}, 16.0f);
    yOffset += 30;
    overlay->drawText("[F5] Save Config", {menuPos.x + 10, menuPos.y + yOffset}, {1, 1, 1, 1}, 16.0f);
    yOffset += 30;
    overlay->drawText("[F6] Load Config", {menuPos.x + 10, menuPos.y + yOffset}, {1, 1, 1, 1}, 16.0f);
    yOffset += 30;
    overlay->drawText("[INSERT] Exit", {menuPos.x + 10, menuPos.y + yOffset}, {1, 0, 0, 1}, 16.0f);
}

void UIRenderer::renderFullMenu() {
    Vec2 menuPos = {50, 50};
    Vec2 menuSize = {400, 600};
    
    // Background
    overlay->drawFilledBox(menuPos, menuSize, {0.1f, 0.1f, 0.1f, 0.95f});
    overlay->drawBox(menuPos, menuSize, {1, 1, 1, 1}, 2.0f);
    
    // Title
    overlay->drawText("GameCheat Menu", {menuPos.x + 10, menuPos.y + 10}, {1, 1, 1, 1}, 24.0f);
    
    float yOffset = 50;
    
    // Aimbot section
    renderAimbotSection();
    
    // Misc section
    renderMiscSection();
    
    // Visuals section
    renderVisualsSection();
    
    // JS Executor section
    renderJSExecutorSection();
    
    // Config section
    renderConfigSection();
    
    // Footer
    yOffset = menuSize.y - 30;
    overlay->drawText("[F1] Toggle Menu", {menuPos.x + 10, menuPos.y + yOffset}, {0.7f, 0.7f, 0.7f, 1}, 14.0f);
}

void UIRenderer::renderAimbotSection() {
    Vec2 menuPos = {50, 50};
    float yOffset = 50;
    
    overlay->drawText("=== AIMBOT ===", {menuPos.x + 10, menuPos.y + yOffset}, {1, 0.5f, 0.5f, 1}, 18.0f);
    yOffset += 30;
    
    overlay->drawText("[F2] Toggle Aimbot", {menuPos.x + 20, menuPos.y + yOffset}, {1, 1, 1, 1}, 14.0f);
    yOffset += 20;
    overlay->drawText("Mode: FOV/Distance/Silent", {menuPos.x + 20, menuPos.y + yOffset}, {0.8f, 0.8f, 0.8f, 1}, 12.0f);
    yOffset += 20;
    overlay->drawText("Bone: Head/Neck/Chest", {menuPos.x + 20, menuPos.y + yOffset}, {0.8f, 0.8f, 0.8f, 1}, 12.0f);
    yOffset += 20;
    overlay->drawText("Smooth: 0.0 - 1.0", {menuPos.x + 20, menuPos.y + yOffset}, {0.8f, 0.8f, 0.8f, 1}, 12.0f);
    yOffset += 20;
    overlay->drawText("Prediction: On/Off", {menuPos.x + 20, menuPos.y + yOffset}, {0.8f, 0.8f, 0.8f, 1}, 12.0f);
}

void UIRenderer::renderMiscSection() {
    Vec2 menuPos = {50, 50};
    float yOffset = 180;
    
    overlay->drawText("=== MISC ===", {menuPos.x + 10, menuPos.y + yOffset}, {0.5f, 1, 0.5f, 1}, 18.0f);
    yOffset += 30;
    
    overlay->drawText("[F4] Toggle God Mode", {menuPos.x + 20, menuPos.y + yOffset}, {1, 1, 1, 1}, 14.0f);
    yOffset += 20;
    overlay->drawText("Noclip: On/Off", {menuPos.x + 20, menuPos.y + yOffset}, {0.8f, 0.8f, 0.8f, 1}, 12.0f);
    yOffset += 20;
    overlay->drawText("Fast Run: On/Off", {menuPos.x + 20, menuPos.y + yOffset}, {0.8f, 0.8f, 0.8f, 1}, 12.0f);
    yOffset += 20;
    overlay->drawText("No Collision: On/Off", {menuPos.x + 20, menuPos.y + yOffset}, {0.8f, 0.8f, 0.8f, 1}, 12.0f);
}

void UIRenderer::renderVisualsSection() {
    Vec2 menuPos = {50, 50};
    float yOffset = 280;
    
    overlay->drawText("=== VISUALS ===", {menuPos.x + 10, menuPos.y + yOffset}, {0.5f, 0.5f, 1, 1}, 18.0f);
    yOffset += 30;
    
    overlay->drawText("[F3] Toggle ESP", {menuPos.x + 20, menuPos.y + yOffset}, {1, 1, 1, 1}, 14.0f);
    yOffset += 20;
    overlay->drawText("Skeleton: On/Off", {menuPos.x + 20, menuPos.y + yOffset}, {0.8f, 0.8f, 0.8f, 1}, 12.0f);
    yOffset += 20;
    overlay->drawText("Box: On/Off", {menuPos.x + 20, menuPos.y + yOffset}, {0.8f, 0.8f, 0.8f, 1}, 12.0f);
    yOffset += 20;
    overlay->drawText("Snapline: On/Off", {menuPos.x + 20, menuPos.y + yOffset}, {0.8f, 0.8f, 0.8f, 1}, 12.0f);
    yOffset += 20;
    overlay->drawText("Radar: On/Off", {menuPos.x + 20, menuPos.y + yOffset}, {0.8f, 0.8f, 0.8f, 1}, 12.0f);
}

void UIRenderer::renderJSExecutorSection() {
    Vec2 menuPos = {50, 50};
    float yOffset = 380;
    
    overlay->drawText("=== JS EXECUTOR ===", {menuPos.x + 10, menuPos.y + yOffset}, {1, 1, 0, 1}, 18.0f);
    yOffset += 30;
    
    overlay->drawText("Load Script", {menuPos.x + 20, menuPos.y + yOffset}, {0.8f, 0.8f, 0.8f, 1}, 12.0f);
    yOffset += 20;
    overlay->drawText("List Scripts", {menuPos.x + 20, menuPos.y + yOffset}, {0.8f, 0.8f, 0.8f, 1}, 12.0f);
    yOffset += 20;
    overlay->drawText("Edit Script", {menuPos.x + 20, menuPos.y + yOffset}, {0.8f, 0.8f, 0.8f, 1}, 12.0f);
}

void UIRenderer::renderConfigSection() {
    Vec2 menuPos = {50, 50};
    float yOffset = 460;
    
    overlay->drawText("=== CONFIG ===", {menuPos.x + 10, menuPos.y + yOffset}, {1, 0.8f, 0, 1}, 18.0f);
    yOffset += 30;
    
    overlay->drawText("[F5] Save Config", {menuPos.x + 20, menuPos.y + yOffset}, {1, 1, 1, 1}, 14.0f);
    yOffset += 20;
    overlay->drawText("[F6] Load Config", {menuPos.x + 20, menuPos.y + yOffset}, {1, 1, 1, 1}, 14.0f);
    yOffset += 20;
    overlay->drawText("Cloud Config: On/Off", {menuPos.x + 20, menuPos.y + yOffset}, {0.8f, 0.8f, 0.8f, 1}, 12.0f);
    yOffset += 20;
    overlay->drawText("Shared Key: ***", {menuPos.x + 20, menuPos.y + yOffset}, {0.8f, 0.8f, 0.8f, 1}, 12.0f);
}

void UIRenderer::renderElements() {
    for (auto& element : elements) {
        if (!element.visible) continue;
        
        Color finalColor = element.color;
        finalColor.a *= menuAlpha;
        
        if (element.hovered && element.hoverColor.a > 0) {
            finalColor = element.hoverColor;
            finalColor.a *= menuAlpha;
        }
        
        if (element.animation.active) {
            float animValue = getAnimationValue(element.animation);
            finalColor.a *= animValue;
        }
        
        if (!element.text.empty()) {
            overlay->drawText(element.text, element.position, finalColor, 14.0f);
        } else {
            overlay->drawFilledBox(element.position, element.size, finalColor);
        }
    }
}

void UIRenderer::renderButton(UIElement& element) {
    Color btnColor = element.hovered ? element.hoverColor : element.color;
    btnColor.a *= menuAlpha;
    
    renderRoundedBox(element.position, element.size, btnColor, 5.0f);
    
    if (element.hovered) {
        renderGlowingBox(element.position, element.size, btnColor, 0.2f);
    }
    
    Color textColor = {1, 1, 1, menuAlpha};
    overlay->drawText(element.text, {element.position.x + 10, element.position.y + 8}, textColor, 14.0f);
}

void UIRenderer::renderToggle(UIElement& element) {
    Vec2 togglePos = {element.position.x + 200, element.position.y};
    Vec2 toggleSize = {20, 20};
    
    Color toggleColor = (element.toggleValue && *element.toggleValue) ? accentColor : primaryColor;
    toggleColor.a = menuAlpha;
    
    overlay->drawFilledBox(togglePos, toggleSize, toggleColor);
    overlay->drawBox(togglePos, toggleSize, accentColor, 1.0f);
    
    if (element.toggleValue && *element.toggleValue) {
        overlay->drawText("✓", {togglePos.x + 5, togglePos.y + 2}, {1, 1, 1, menuAlpha}, 14.0f);
    }
    
    Color textColor = {1, 1, 1, menuAlpha};
    overlay->drawText(element.text, element.position, textColor, 14.0f);
}

void UIRenderer::renderSlider(UIElement& element) {
    Color textColor = {1, 1, 1, menuAlpha};
    overlay->drawText(element.text, element.position, textColor, 14.0f);
    
    Vec2 sliderPos = {element.position.x + 200, element.position.y + 5};
    Vec2 sliderSize = {element.size.x - 200, 10};
    
    Color sliderBg = primaryColor;
    sliderBg.a = menuAlpha;
    overlay->drawFilledBox(sliderPos, sliderSize, sliderBg);
    
    Color sliderFill = accentColor;
    sliderFill.a = menuAlpha;
    overlay->drawFilledBox(sliderPos, {sliderSize.x * 0.5f, sliderSize.y}, sliderFill);
}

void UIRenderer::renderComboBox(UIElement& element) {
    Color textColor = {1, 1, 1, menuAlpha};
    overlay->drawText(element.text, element.position, textColor, 14.0f);
    
    Vec2 comboPos = {element.position.x + 200, element.position.y};
    Vec2 comboSize = {150, 25};
    
    Color comboBg = primaryColor;
    comboBg.a = menuAlpha;
    overlay->drawFilledBox(comboPos, comboSize, comboBg);
    overlay->drawBox(comboPos, comboSize, accentColor, 1.0f);
    
    overlay->drawText("Option 1", {comboPos.x + 10, comboPos.y + 5}, textColor, 12.0f);
}

void UIRenderer::renderRoundedBox(Vec2 position, Vec2 size, Color color, float radius) {
    overlay->drawFilledBox(position, size, color);
    overlay->drawBox(position, size, color, 1.0f);
}

void UIRenderer::renderGlowingBox(Vec2 position, Vec2 size, Color color, float glowIntensity) {
    for (int i = 1; i <= 3; i++) {
        Color glowColor = color;
        glowColor.a = color.a * glowIntensity / i;
        Vec2 glowPos = {position.x - i, position.y - i};
        Vec2 glowSize = {size.x + i * 2, size.y + i * 2};
        overlay->drawBox(glowPos, glowSize, glowColor, 1.0f);
    }
}

void UIRenderer::renderGradientBox(Vec2 position, Vec2 size, Color color1, Color color2, bool horizontal) {
    overlay->drawFilledBox(position, size, color1);
}

void UIRenderer::updateAnimation(Animation& anim, float deltaTime) {
    if (!anim.active) return;
    
    anim.elapsed += deltaTime;
    if (anim.elapsed >= anim.duration) {
        anim.active = false;
        anim.elapsed = anim.duration;
    }
}

float UIRenderer::getAnimationValue(Animation& anim) {
    if (!anim.active) return anim.endValue;
    
    float progress = anim.elapsed / anim.duration;
    float t = progress;
    
    // Ease out cubic
    t = 1 - pow(1 - t, 3);
    
    return anim.startValue + (anim.endValue - anim.startValue) * t;
}

bool UIRenderer::isMouseOver(Vec2 position, Vec2 size) {
    Vec2 mousePos = getMousePosition();
    return mousePos.x >= position.x && mousePos.x <= position.x + size.x &&
           mousePos.y >= position.y && mousePos.y <= position.y + size.y;
}

Vec2 UIRenderer::getMousePosition() {
    POINT cursor;
    GetCursorPos(&cursor);
    return {static_cast<float>(cursor.x), static_cast<float>(cursor.y)};
}
