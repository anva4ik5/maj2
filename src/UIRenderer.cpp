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
    // Modern dark cyberpunk theme
    backgroundColor = {0.06f, 0.06f, 0.09f, 0.96f};   // Deep dark background
    primaryColor = {0.10f, 0.10f, 0.14f, 1.0f};        // Panel background
    secondaryColor = {0.14f, 0.14f, 0.20f, 1.0f};     // Elevated panels
    accentColor = {0.0f, 0.85f, 0.7f, 1.0f};            // Teal/cyan neon accent
    textColor = {0.95f, 0.95f, 0.97f, 1.0f};           // Soft white
    disabledColor = {0.35f, 0.35f, 0.40f, 1.0f};       // Muted gray
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
    // Handle input
    handleKeyboardInput();
    if (menuVisible) {
        handleMouseInput();
    }
    
    // Update menu alpha animation with easing
    if (menuVisible && menuAlpha < 1.0f) {
        menuAlpha += deltaTime * 5.0f;
        if (menuAlpha > 1.0f) menuAlpha = 1.0f;
    } else if (!menuVisible && menuAlpha > 0.0f) {
        menuAlpha -= deltaTime * 5.0f;
        if (menuAlpha < 0.0f) menuAlpha = 0.0f;
    }
    
    // Update element animations
    for (auto& element : elements) {
        if (element.animation.active) {
            updateAnimation(element.animation, deltaTime);
        }
    }
    
    // Update hover states
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

void UIRenderer::renderFullMenu() {
    if (menuAlpha <= 0.0f) return;
    
    // Apply menu alpha to background
    Color bgAlpha = backgroundColor;
    bgAlpha.a = menuAlpha * backgroundColor.a;
    
    // Render shadow behind menu
    renderShadow({menuPosition.x + 4, menuPosition.y + 4}, menuSize, 8.0f);
    
    // Render menu background with glow effect
    renderGlowingBox(menuPosition, menuSize, bgAlpha, 0.3f);
    
    // Render menu border with neon effect
    Color borderColor = accentColor;
    borderColor.a = menuAlpha * 0.8f;
    renderNeonBorder(menuPosition, menuSize, borderColor, 1.5f);
    
    // Render modern header
    Vec2 headerPos = {menuPosition.x, menuPosition.y};
    Vec2 headerSize = {menuSize.x, 50};
    renderModernHeader(headerPos, headerSize);
    
    // Render menu title with glow
    Color titleColor = accentColor;
    titleColor.a = menuAlpha;
    overlay->drawText("MAJESTIC RP CHEAT", {menuPosition.x + 20, menuPosition.y + 15}, titleColor, 20.0f);
    
    // Render neon accent under title
    Vec2 accentPos = {menuPosition.x + 20, menuPosition.y + 42};
    Vec2 accentSize = {200, 2};
    Color accentGlow = accentColor;
    accentGlow.a = menuAlpha * 0.7f;
    overlay->drawFilledBox(accentPos, accentSize, accentGlow);
    
    // Render tabs
    renderTabs();
    
    // Render tab content
    renderTabContent();
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
        
        Color tabTextColor;
        if (tabs[i].active) {
            tabTextColor = Color(1, 1, 1, 1);
        } else {
            tabTextColor = Color(0.7f, 0.7f, 0.7f, 1);
        }
        tabTextColor.a = menuAlpha;
        
        // Center text in tab
        float textWidth = tabs[i].name.length() * 8;
        float textX = currentTabPos.x + (tabWidth - textWidth) / 2;
        overlay->drawText(tabs[i].name, {textX, currentTabPos.y + 12}, tabTextColor, 14.0f);
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
    yOffset += 20;
    overlay->drawText("[F6] Invisibility: On/Off", {menuPos.x + 20, menuPos.y + yOffset}, {0.8f, 0.8f, 0.8f, 1}, 12.0f);
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

float UIRenderer::easeOutQuad(float t) {
    return 1.0f - (1.0f - t) * (1.0f - t);
}

float UIRenderer::easeOutCubic(float t) {
    return 1.0f - pow(1.0f - t, 3);
}

float UIRenderer::easeInOutCubic(float t) {
    return t < 0.5f ? 4.0f * t * t * t : 1.0f - pow(-2.0f * t + 2.0f, 3) / 2.0f;
}

float UIRenderer::easeOutElastic(float t) {
    const float c4 = (2.0f * 3.14159265f) / 3.0f;
    return t == 0.0f ? 0.0f : t == 1.0f ? 1.0f : pow(2.0f, -10.0f * t) * sin((t * 10.0f - 0.75f) * c4) + 1.0f;
}

float UIRenderer::getAnimationValue(Animation& anim) {
    if (!anim.active) return anim.endValue;
    
    float progress = anim.elapsed / anim.duration;
    float t = progress;
    
    switch (anim.type) {
        case AnimationType::SCALE:
            t = easeOutElastic(t);
            break;
        case AnimationType::GLOW:
            t = easeInOutCubic(t);
            break;
        case AnimationType::SLIDE_LEFT:
        case AnimationType::SLIDE_RIGHT:
        case AnimationType::SLIDE_UP:
        case AnimationType::SLIDE_DOWN:
            t = easeOutCubic(t);
            break;
        default:
            t = easeOutQuad(t);
            break;
    }
    
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
    if (overlay && overlay->getWindowHandle()) {
        ScreenToClient(overlay->getWindowHandle(), &cursor);
    }
    return {static_cast<float>(cursor.x), static_cast<float>(cursor.y)};
}

void UIRenderer::handleMouseInput() {
    static bool wasMouseDown = false;
    bool isMouseDown = (GetAsyncKeyState(VK_LBUTTON) & 0x8000) != 0;
    
    if (isMouseDown && !wasMouseDown) {
        Vec2 mousePos = getMousePosition();
        
        // Check tab clicks
        float tabWidth = menuSize.x / tabs.size();
        float tabHeight = 40;
        Vec2 tabPos = {menuPosition.x, menuPosition.y + 50};
        
        for (size_t i = 0; i < tabs.size(); i++) {
            Vec2 currentTabPos = {tabPos.x + i * tabWidth, tabPos.y};
            Vec2 currentTabSize = {tabWidth, tabHeight};
            
            if (mousePos.x >= currentTabPos.x && mousePos.x <= currentTabPos.x + currentTabSize.x &&
                mousePos.y >= currentTabPos.y && mousePos.y <= currentTabPos.y + currentTabSize.y) {
                activeTab = static_cast<int>(i);
                for (auto& tab : tabs) tab.active = false;
                tabs[i].active = true;
                break;
            }
        }
        
        // Check element clicks
        for (auto& element : elements) {
            if (!element.visible || !element.enabled) continue;
            
            if (mousePos.x >= element.position.x && mousePos.x <= element.position.x + element.size.x &&
                mousePos.y >= element.position.y && mousePos.y <= element.position.y + element.size.y) {
                
                if (element.onClick) {
                    element.onClick();
                    element.animation.active = true;
                    element.animation.elapsed = 0.0f;
                    element.animation.type = AnimationType::SCALE;
                    element.animation.duration = 0.3f;
                }
                
                if (element.toggleValue && element.onToggle) {
                    *element.toggleValue = !(*element.toggleValue);
                    element.onToggle();
                    element.animation.active = true;
                    element.animation.elapsed = 0.0f;
                    element.animation.type = AnimationType::GLOW;
                    element.animation.duration = 0.3f;
                }
            }
        }
    }
    
    wasMouseDown = isMouseDown;
}

void UIRenderer::handleKeyboardInput() {
    if ((GetAsyncKeyState(VK_INSERT) & 1) || (GetAsyncKeyState(VK_END) & 1)) {
        toggleMenu();
    }
}

void UIRenderer::renderNeonBorder(Vec2 position, Vec2 size, Color color, float thickness) {
    // Inner bright line
    Color bright = color;
    bright.a = color.a * 0.9f;
    overlay->drawBox(position, size, bright, thickness);
    
    // Outer glow layers
    for (int i = 1; i <= 3; i++) {
        Color glow = color;
        glow.a = color.a * 0.15f / i;
        Vec2 glowPos = {position.x - i, position.y - i};
        Vec2 glowSize = {size.x + i * 2, size.y + i * 2};
        overlay->drawBox(glowPos, glowSize, glow, thickness);
    }
}

void UIRenderer::renderShadow(Vec2 position, Vec2 size, float blur) {
    for (int i = 1; i <= static_cast<int>(blur); i++) {
        Color shadowColor = {0.0f, 0.0f, 0.0f, 0.08f / i};
        Vec2 shadowPos = {position.x + i * 0.5f, position.y + i * 0.5f};
        Vec2 shadowSize = {size.x - i, size.y - i};
        overlay->drawFilledBox(shadowPos, shadowSize, shadowColor);
    }
}

void UIRenderer::renderModernHeader(Vec2 position, Vec2 size) {
    // Gradient header with accent line
    renderGradientBox(position, size, 
        Color(0.08f, 0.08f, 0.12f, 1.0f), 
        Color(0.12f, 0.12f, 0.18f, 1.0f), true);
    
    // Bottom accent line
    Vec2 linePos = {position.x, position.y + size.y - 2};
    Vec2 lineSize = {size.x, 2};
    overlay->drawFilledBox(linePos, lineSize, accentColor);
}
