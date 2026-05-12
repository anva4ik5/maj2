#pragma once

#include <windows.h>
#include <string>
#include <vector>

class OBSBypass {
public:
    OBSBypass();
    ~OBSBypass();
    
    void initialize();
    void enable();
    void disable();
    
    bool isEnabled() const { return enabled; }
    
    // Методы обхода OBS
    void hideFromOBS();
    void restoreForOBS();
    
    // Проверка наличия OBS
    bool isOBSPresent();
    bool isRecording();
    
    // Методы скрытия оверлея
    void hideOverlay();
    void showOverlay();
    
private:
    bool enabled;
    bool overlayHidden;
    
    // Поиск процессов OBS
    bool findOBSProcess();
    DWORD obsProcessId;
    
    // Хуки DirectX для скрытия оверлея
    void hookDirectX();
    void unhookDirectX();
    
    // Методы скрытия окна
    void hideWindowFromCapture(HWND hwnd);
    void restoreWindowFromCapture(HWND hwnd);
    
    // Проверка захвата экрана
    bool isScreenBeingCaptured();
    
    // Изменение рендеринга для OBS
    void setOBSCompatibleRendering(bool compatible);
};
