#pragma once

#include "MemoryManager.h"
#include "DLLInjector.h"
#include "NetworkClient.h"
#include "DirectXOverlay.h"
#include "ConfigManager.h"
#include "Aimbot.h"
#include "Misc.h"
#include "Visuals.h"
#include "JSExecutor.h"
#include "Logger.h"
#include "AntiCheatBypass.h"
#include "LogoRenderer.h"
#include "KeyBindManager.h"
#include "OBSBypass.h"
#include "SelfDestruct.h"
#include "AdminDetector.h"
#include "AntiCheatDetector.h"
#include "UIRenderer.h"
#include "HWIDManager.h"
#include "AuthManager.h"
#include "TelegramBot.h"
#include "BackendAPI.h"
#include <memory>
#include <thread>
#include <atomic>
#include <functional>

class GameCheat {
public:
    GameCheat();
    ~GameCheat();
    
    bool initialize();
    void run();
    void stop();
    
    bool isRunning() const { return running; }
    
    // Feature access
    Aimbot* getAimbot() { return aimbot.get(); }
    Misc* getMisc() { return misc.get(); }
    Visuals* getVisuals() { return visuals.get(); }
    JSExecutor* getJSExecutor() { return jsExecutor.get(); }
    Logger* getLogger() { return &Logger::getInstance(); }
    ServerLogger* getServerLogger() { return serverLogger.get(); }
    EventLogger* getEventLogger() { return eventLogger.get(); }
    LogoRenderer* getLogoRenderer() { return logoRenderer.get(); }
    KeyBindManager* getKeyBindManager() { return keyBindManager.get(); }
    OBSBypass* getOBSBypass() { return obsBypass.get(); }
    SelfDestruct* getSelfDestruct() { return selfDestruct.get(); }
    AdminDetector* getAdminDetector() { return adminDetector.get(); }
    AntiCheatDetector* getAntiCheatDetector() { return antiCheatDetector.get(); }
    UIRenderer* getUIRenderer() { return uiRenderer.get(); }
    HWIDManager* getHWIDManager() { return hwidManager.get(); }
    AuthManager* getAuthManager() { return authManager.get(); }
    TelegramBot* getTelegramBot() { return telegramBot.get(); }
    BackendAPI* getBackendAPI() { return backendAPI.get(); }
    
private:
    MemoryManager memoryManager;
    DLLInjector injector;
    std::unique_ptr<NetworkClient> networkClient;
    std::unique_ptr<DirectXOverlay> overlay;
    ConfigManager& config;
    
    // Feature modules
    std::unique_ptr<Aimbot> aimbot;
    std::unique_ptr<Misc> misc;
    std::unique_ptr<Visuals> visuals;
    std::unique_ptr<JSExecutor> jsExecutor;
    std::unique_ptr<ServerLogger> serverLogger;
    std::unique_ptr<EventLogger> eventLogger;
    std::unique_ptr<AntiCheatBypass> antiCheatBypass;
    std::unique_ptr<LogoRenderer> logoRenderer;
    std::unique_ptr<KeyBindManager> keyBindManager;
    std::unique_ptr<OBSBypass> obsBypass;
    std::unique_ptr<SelfDestruct> selfDestruct;
    std::unique_ptr<AdminDetector> adminDetector;
    std::unique_ptr<AntiCheatDetector> antiCheatDetector;
    std::unique_ptr<UIRenderer> uiRenderer;
    std::unique_ptr<HWIDManager> hwidManager;
    std::unique_ptr<AuthManager> authManager;
    std::unique_ptr<TelegramBot> telegramBot;
    std::unique_ptr<BackendAPI> backendAPI;
    
    HANDLE targetProcess;
    HWND targetWindow;
    std::atomic<bool> running;
    std::thread cheatThread;
    std::thread renderThread;
    std::thread inputThread;
    
    bool findTargetProcess();
    bool findTargetWindow();
    
    void cheatLoop();
    void renderLoop();
    void inputLoop();
    
    // Network
    void sendTelemetry();
    void receiveConfig();
};
