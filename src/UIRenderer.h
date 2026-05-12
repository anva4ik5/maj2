#pragma once

#include "DirectXOverlay.h"
#include "ConfigManager.h"
#include <string>
#include <vector>
#include <functional>
#include <memory>

// Animation types
enum class AnimationType {
    NONE,
    FADE_IN,
    FADE_OUT,
    SLIDE_LEFT,
    SLIDE_RIGHT,
    SLIDE_UP,
    SLIDE_DOWN,
    SCALE,
    GLOW
};

struct Animation {
    AnimationType type;
    float duration;
    float elapsed;
    bool active;
    float startValue;
    float endValue;
    
    Animation() : type(AnimationType::NONE), duration(0.3f), elapsed(0.0f), active(false), startValue(0.0f), endValue(1.0f) {}
};

struct UIElement {
    Vec2 position;
    Vec2 size;
    std::string text;
    Color color;
    Color hoverColor;
    Color activeColor;
    bool visible;
    bool enabled;
    bool hovered;
    bool active;
    float alpha;
    float scale;
    Animation animation;
    std::function<void()> onClick;
    std::function<void()> onToggle;
    bool* toggleValue;
    
    UIElement() : visible(true), enabled(true), hovered(false), active(false), alpha(1.0f), scale(1.0f) {}
};

struct Tab {
    std::string name;
    int id;
    bool active;
    
    Tab(const std::string& n, int i) : name(n), id(i), active(false) {}
};

class UIRenderer {
public:
    UIRenderer(DirectXOverlay* overlay);
    ~UIRenderer();
    
    void render();
    void update(float deltaTime);
    
    void addText(const std::string& text, Vec2 position, Color color = Color(), float size = 16.0f);
    void addBox(Vec2 position, Vec2 size, Color color = Color(), float thickness = 1.0f);
    void addFilledBox(Vec2 position, Vec2 size, Color color = Color());
    void addButton(const std::string& text, Vec2 position, Vec2 size, 
                   std::function<void()> onClick, Color color = Color());
    void addToggle(const std::string& text, Vec2 position, bool* value, 
                   std::function<void()> onToggle, Color color = Color());
    void addSlider(const std::string& text, Vec2 position, Vec2 size, float* value, 
                   float min, float max, Color color = Color());
    void addComboBox(const std::string& text, Vec2 position, Vec2 size, 
                     std::vector<std::string> options, int* selectedIndex, Color color = Color());
    
    void clear();
    
    void setMenuVisible(bool visible) { menuVisible = visible; }
    bool isMenuVisible() const { return menuVisible; }
    
    void toggleMenu() { menuVisible = !menuVisible; }
    
    // Render full menu with all features
    void renderFullMenu();
    
    // Style settings
    void setPrimaryColor(Color color) { primaryColor = color; }
    void setSecondaryColor(Color color) { secondaryColor = color; }
    void setAccentColor(Color color) { accentColor = color; }
    void setBackgroundColor(Color color) { backgroundColor = color; }
    
private:
    DirectXOverlay* overlay;
    std::vector<UIElement> elements;
    std::vector<Tab> tabs;
    bool menuVisible;
    int activeTab;
    float menuAlpha;
    Vec2 menuPosition;
    Vec2 menuSize;
    
    // Style colors
    Color primaryColor;
    Color secondaryColor;
    Color accentColor;
    Color backgroundColor;
    Color textColor;
    Color disabledColor;
    
    // Animation
    float animationSpeed;
    
    void renderMenu();
    void renderElements();
    void renderAimbotSection();
    void renderMiscSection();
    void renderVisualsSection();
    void renderJSExecutorSection();
    void renderConfigSection();
    void renderKeyBindsSection();
    void renderSecuritySection();
    
    void renderTabs();
    void renderTabContent();
    
    void renderButton(UIElement& element);
    void renderToggle(UIElement& element);
    void renderSlider(UIElement& element);
    void renderComboBox(UIElement& element);
    
    void renderRoundedBox(Vec2 position, Vec2 size, Color color, float radius);
    void renderGlowingBox(Vec2 position, Vec2 size, Color color, float glowIntensity);
    void renderGradientBox(Vec2 position, Vec2 size, Color color1, Color color2, bool horizontal);
    
    void updateAnimation(Animation& anim, float deltaTime);
    float getAnimationValue(Animation& anim);
    
    bool isMouseOver(Vec2 position, Vec2 size);
    Vec2 getMousePosition();
    
    void initializeStyle();
    void initializeTabs();
};
