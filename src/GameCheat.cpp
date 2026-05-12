#include "GameCheat.h"
#include <iostream>
#include <TlHelp32.h>

GameCheat::GameCheat() 
    : targetProcess(nullptr), targetWindow(nullptr), running(false) {
    networkClient = std::make_unique<NetworkClient>();
    overlay = std::make_unique<DirectXOverlay>();
    aimbot = std::make_unique<Aimbot>();
    misc = std::make_unique<Misc>();
    visuals = std::make_unique<Visuals>(overlay.get());
    jsExecutor = std::make_unique<JSExecutor>();
    serverLogger = std::make_unique<ServerLogger>();
    eventLogger = std::make_unique<EventLogger>();
    antiCheatBypass = std::make_unique<AntiCheatBypass>();
    logoRenderer = std::make_unique<LogoRenderer>(overlay.get());
    keyBindManager = std::make_unique<KeyBindManager>();
    obsBypass = std::make_unique<OBSBypass>();
    selfDestruct = std::make_unique<SelfDestruct>();
    adminDetector = std::make_unique<AdminDetector>();
    antiCheatDetector = std::make_unique<AntiCheatDetector>();
    uiRenderer = std::make_unique<UIRenderer>(overlay.get());
    hwidManager = std::make_unique<HWIDManager>();
    authManager = std::make_unique<AuthManager>();
    telegramBot = std::make_unique<TelegramBot>();
    backendAPI = std::make_unique<BackendAPI>();
}

GameCheat::~GameCheat() {
    stop();
    
    if (targetProcess) {
        CloseHandle(targetProcess);
    }
}

bool GameCheat::findTargetProcess() {
    std::string processName = config.getConfig().targetProcess;
    targetProcess = memoryManager.getProcessHandle(processName);
    
    if (!targetProcess) {
        std::cerr << "Failed to find process: " << processName << std::endl;
        return false;
    }
    
    std::cout << "Found target process: " << processName << std::endl;
    return true;
}

bool GameCheat::findTargetWindow() {
    std::string processName = config.getConfig().targetProcess;
    
    // Find window by process name
    HWND hwnd = nullptr;
    
    HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    PROCESSENTRY32 pe32;
    pe32.dwSize = sizeof(PROCESSENTRY32);
    
    DWORD pid = 0;
    if (Process32First(snapshot, &pe32)) {
        do {
            if (processName == pe32.szExeFile) {
                pid = pe32.th32ProcessID;
                break;
            }
        } while (Process32Next(snapshot, &pe32));
    }
    CloseHandle(snapshot);
    
    if (pid == 0) {
        return false;
    }
    
    // Find window by PID
    struct WindowData {
        DWORD pid;
        HWND hwnd;
    } data = {pid, nullptr};
    
    EnumWindows([](HWND hwnd, LPARAM lParam) -> BOOL {
        WindowData* data = reinterpret_cast<WindowData*>(lParam);
        DWORD windowPid;
        GetWindowThreadProcessId(hwnd, &windowPid);
        
        if (windowPid == data->pid && IsWindowVisible(hwnd)) {
            data->hwnd = hwnd;
            return FALSE;
        }
        return TRUE;
    }, (LPARAM)&data);
    
    targetWindow = data.hwnd;
    return targetWindow != nullptr;
}

bool GameCheat::initialize() {
    std::cout << "Initializing GameCheat..." << std::endl;
    
    // Load configuration
    if (!config.loadFromFile("config.ini")) {
        std::cout << "Using default configuration" << std::endl;
    }
    
    // Find target process
    if (!findTargetProcess()) {
        std::cerr << "Target process not found. Please start the game first." << std::endl;
        return false;
    }
    
    // Initialize network client
    if (!networkClient->initialize()) {
        std::cerr << "Failed to initialize network client" << std::endl;
        // Continue anyway - might work offline
    } else {
        // Connect to server
        if (networkClient->connectToServer(config.getConfig().serverHost, 
                                         config.getConfig().serverPort)) {
            std::cout << "Connected to server: " << config.getConfig().serverHost 
                      << ":" << config.getConfig().serverPort << std::endl;
        }
    }
    
    // Initialize overlay (if enabled)
    if (config.getConfig().enableOverlay) {
        if (findTargetWindow()) {
            if (overlay->initialize(targetWindow)) {
                std::cout << "Overlay initialized successfully" << std::endl;
                
                // Initialize visuals with overlay
                visuals->initialize(targetProcess);
            } else {
                std::cerr << "Failed to initialize overlay" << std::endl;
            }
        } else {
            std::cerr << "Failed to find target window" << std::endl;
        }
    }
    
    // Initialize aimbot
    aimbot->initialize(targetProcess);
    
    // Initialize misc
    misc->initialize(targetProcess);
    
    // Initialize JS executor
    if (config.getConfig().enableJSExecutor) {
        jsExecutor->initialize();
    }
    
    // Initialize loggers
    if (config.getConfig().enableServerLogger) {
        serverLogger->initialize(config.getConfig().serverHost);
        serverLogger->setEnabled(true);
    }
    
    if (config.getConfig().enableEventLogger) {
        eventLogger->initialize();
        eventLogger->setEnabled(true);
    }
    
    // Initialize anti-cheat bypass
    antiCheatBypass->initialize();
    
    // Detect anti-cheat
    antiCheatDetector->initialize();
    AntiCheatInfo detectedAC = antiCheatDetector->detect();
    
    if (detectedAC.detected) {
        std::cout << "=== АНТИЧИТ ОБНАРУЖЕН ===" << std::endl;
        std::cout << "Название: " << detectedAC.name << std::endl;
        std::cout << "Версия: " << detectedAC.version << std::endl;
        std::cout << "Статус обхода: " << detectedAC.status << std::endl;
        
        if (!detectedAC.bypassAvailable) {
            std::cout << "ВНИМАНИЕ: Обход для этого античита недоступен!" << std::endl;
            std::cout << "Чит не будет работать для предотвращения бана." << std::endl;
            return false;
        }
    } else {
        std::cout << "Античит не обнаружен. Режим: без античита" << std::endl;
    }
    
    // Initialize logo renderer
    if (overlay->isInitialized()) {
        logoRenderer->initialize();
    }
    
    // Initialize key bind manager
    keyBindManager->initialize();
    
    // Only bind menu toggle to END key - user must setup other binds
    keyBindManager->addBind("menu_toggle", VK_END, BindType::TOGGLE, [this]() {
        eventLogger->logPlayerAction("Toggle menu");
    });
    
    antiCheatBypass->applyBypasses();
    
    // Apply specific bypasses based on detected anti-cheat
    if (detectedAC.detected) {
        switch (detectedAC.type) {
            case AntiCheatType::EASY_ANTICHEAT:
                antiCheatBypass->applyEACBypasses();
                break;
            case AntiCheatType::BATTLEYE:
                antiCheatBypass->applyBEBypasses();
                break;
            case AntiCheatType::MAJESTIC:
                antiCheatBypass->applyMajesticBypasses();
                break;
            default:
                antiCheatBypass->applyGeneralBypasses();
                break;
        }
    }
    
    // Initialize OBS bypass
    if (config.getConfig().obsBypassEnabled) {
        obsBypass->initialize();
        if (config.getConfig().obsAutoDetect) {
            // OBS bypass will auto-detect and enable
        } else {
            obsBypass->enable();
        }
    }
    
    // Initialize self-destruct
    selfDestruct->initialize();
    
    // Initialize admin detector
    adminDetector->initialize();
    adminDetector->setAutoDisableOnAdmin(true);
    adminDetector->setWarningCallback([this]() {
        std::cout << "ПРЕДУПРЕЖДЕНИЕ: Админ обнаружен поблизости!" << std::endl;
        // Можно запустить самоуничтожение здесь
    });
    
    std::cout << "GameCheat initialized successfully" << std::endl;
    return true;
}

void GameCheat::cheatLoop() {
    while (running) {
        // Update key binds
        keyBindManager->update();
        
        // Update admin detector
        adminDetector->update();
        
        // Update aimbot
        aimbot->update();
        
        // Update misc features
        misc->update();
        
        // Update UI renderer with delta time
        float deltaTime = 0.016f; // ~60 FPS
        uiRenderer->update(deltaTime);
        
        // Send telemetry
        sendTelemetry();
        
        // Receive config updates
        receiveConfig();
        
        // Dump server if enabled
        if (config.getConfig().dumpServer) {
            serverLogger->dumpServer();
            config.getConfig().dumpServer = false;
        }
        
        Sleep(16); // ~60 FPS
    }
}

void GameCheat::renderLoop() {
    while (running && overlay->isInitialized()) {
        // Render visuals (ESP, etc.)
        visuals->render();
        
        Sleep(16);
    }
}

void GameCheat::run() {
    running = true;
    
    // Start cheat thread
    cheatThread = std::thread(&GameCheat::cheatLoop, this);
    
    // Start render thread (if overlay is enabled)
    if (overlay->isInitialized()) {
        renderThread = std::thread(&GameCheat::renderLoop, this);
    }
    
    // Start input thread (for hotkeys)
    inputThread = std::thread(&GameCheat::inputLoop, this);
    
    std::cout << "Cheat is running. Press Ctrl+C to stop." << std::endl;
}

void GameCheat::stop() {
    running = false;
    
    // Restore anti-cheat bypasses
    if (antiCheatBypass) {
        antiCheatBypass->restore();
    }
    
    if (cheatThread.joinable()) {
        cheatThread.join();
    }
    
    if (renderThread.joinable()) {
        renderThread.join();
    }
    
    if (inputThread.joinable()) {
        inputThread.join();
    }
    
    // Execute self-destruct
    if (selfDestruct) {
        selfDestruct->execute();
    }
    
    std::cout << "Cheat stopped" << std::endl;
}

void GameCheat::inputLoop() {
    while (running) {
        // Key binds are now handled by KeyBindManager
        // This loop is kept for additional input handling if needed
        
        Sleep(10);
    }
}

void GameCheat::sendTelemetry() {
    if (!networkClient->isConnected()) {
        return;
    }
    
    // Send status update to server
    std::string data = "{"
        "\"status\":\"active\","
        "\"aimbot\":" + std::string(config.getConfig().enableAimbot ? "true" : "false") + ","
        "\"esp\":" + std::string(config.getConfig().enableESP ? "true" : "false") +
    "}";
    
    networkClient->sendPOSTRequest("/telemetry", data);
}

void GameCheat::receiveConfig() {
    if (!networkClient->isConnected()) {
        return;
    }
    
    // Request config updates from server
    std::string response = networkClient->sendGETRequest("/config");
    
    if (!response.empty()) {
        // Parse and apply config updates
        // TODO: Implement JSON parsing
    }
}
