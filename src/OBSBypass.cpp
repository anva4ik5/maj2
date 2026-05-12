#include "OBSBypass.h"
#include <tlhelp32.h>
#include <iostream>

OBSBypass::OBSBypass() : enabled(false), overlayHidden(false), obsProcessId(0) {
}

OBSBypass::~OBSBypass() {
    if (enabled) {
        disable();
    }
}

void OBSBypass::initialize() {
    // Проверяем наличие OBS
    if (isOBSPresent()) {
        std::cout << "OBS detected, enabling bypass" << std::endl;
        enable();
    }
}

void OBSBypass::enable() {
    enabled = true;
    hideFromOBS();
    hookDirectX();
}

void OBSBypass::disable() {
    enabled = false;
    restoreForOBS();
    unhookDirectX();
}

void OBSBypass::hideFromOBS() {
    // Скрыть оверлей от OBS
    // Это включает несколько методов:
    
    // 1. Изменить стиль окна
    HWND hwnd = GetActiveWindow();
    if (hwnd) {
        // Установить флаг WS_EX_NOREDIRECTIONBITMAP
        LONG_PTR style = GetWindowLongPtr(hwnd, GWL_EXSTYLE);
        SetWindowLongPtr(hwnd, GWL_EXSTYLE, style | WS_EX_NOREDIRECTIONBITMAP);
    }
    
    // 2. Изменить рендеринг для OBS
    setOBSCompatibleRendering(true);
    
    // 3. Скрыть окно от захвата
    if (hwnd) {
        hideWindowFromCapture(hwnd);
    }
}

void OBSBypass::restoreForOBS() {
    // Восстановить нормальный рендеринг
    HWND hwnd = GetActiveWindow();
    if (hwnd) {
        restoreWindowFromCapture(hwnd);
    }
    
    setOBSCompatibleRendering(false);
}

bool OBSBypass::isOBSPresent() {
    return findOBSProcess();
}

bool OBSBypass::isRecording() {
    // Проверить, идет ли запись
    // Это можно определить через проверку процессов или API OBS
    if (!isOBSPresent()) return false;
    
    // Проверка через HWND (OBS Studio)
    HWND obsWindow = FindWindowA("Qt5QWindowIcon", "OBS");
    if (obsWindow) {
        // Проверить заголовок окна на наличие "Recording"
        char title[256];
        GetWindowTextA(obsWindow, title, 256);
        std::string titleStr(title);
        return titleStr.find("Recording") != std::string::npos;
    }
    
    return false;
}

void OBSBypass::hideOverlay() {
    overlayHidden = true;
}

void OBSBypass::showOverlay() {
    overlayHidden = false;
}

bool OBSBypass::findOBSProcess() {
    HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hSnapshot == INVALID_HANDLE_VALUE) return false;
    
    PROCESSENTRY32 pe32;
    pe32.dwSize = sizeof(PROCESSENTRY32);
    
    if (Process32First(hSnapshot, &pe32)) {
        do {
            std::string processName = pe32.szExeFile;
            
            // Проверка на OBS Studio
            if (processName.find("obs64.exe") != std::string::npos ||
                processName.find("obs32.exe") != std::string::npos ||
                processName.find("OBS.exe") != std::string::npos) {
                obsProcessId = pe32.th32ProcessID;
                CloseHandle(hSnapshot);
                return true;
            }
            
            // Проверка на другие программы записи
            if (processName.find("Streamlabs") != std::string::npos ||
                processName.find("XSplit") != std::string::npos ||
                processName.find("Fraps") != std::string::npos ||
                processName.find("Bandicam") != std::string::npos) {
                obsProcessId = pe32.th32ProcessID;
                CloseHandle(hSnapshot);
                return true;
            }
            
        } while (Process32Next(hSnapshot, &pe32));
    }
    
    CloseHandle(hSnapshot);
    return false;
}

void OBSBypass::hookDirectX() {
    // Хук DirectX функций для скрытия оверлея
    // Это требует хукинга Present/EndScene
    // Placeholder для реализации
}

void OBSBypass::unhookDirectX() {
    // Удалить хуки DirectX
}

void OBSBypass::hideWindowFromCapture(HWND hwnd) {
    // Скрыть окно от захвата экрана
    // Используем SetWindowDisplayAffinity (Windows 8+)
    
    typedef BOOL (WINAPI *SetWindowDisplayAffinityFn)(HWND, DWORD);
    SetWindowDisplayAffinityFn pSetWindowDisplayAffinity = 
        (SetWindowDisplayAffinityFn)GetProcAddress(GetModuleHandleA("user32.dll"), "SetWindowDisplayAffinity");
    
    if (pSetWindowDisplayAffinity) {
        // WDA_EXCLUDEFROMCAPTURE = 0x11
        pSetWindowDisplayAffinity(hwnd, 0x11);
    }
}

void OBSBypass::restoreWindowFromCapture(HWND hwnd) {
    // Восстановить окно для захвата
    typedef BOOL (WINAPI *SetWindowDisplayAffinityFn)(HWND, DWORD);
    SetWindowDisplayAffinityFn pSetWindowDisplayAffinity = 
        (SetWindowDisplayAffinityFn)GetProcAddress(GetModuleHandleA("user32.dll"), "SetWindowDisplayAffinity");
    
    if (pSetWindowDisplayAffinity) {
        // WDA_NONE = 0
        pSetWindowDisplayAffinity(hwnd, 0);
    }
}

bool OBSBypass::isScreenBeingCaptured() {
    // Проверить, захватывается ли экран
    // Можно проверить через BitBlt или другие методы
    return isRecording();
}

void OBSBypass::setOBSCompatibleRendering(bool compatible) {
    // Изменить режим рендеринга для совместимости с OBS
    // При compatible=true использовать методы, которые не видны OBS
    // При compatible=false использовать нормальный рендеринг
}
