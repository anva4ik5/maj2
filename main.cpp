#include <iostream>
#include <string>
#include <windows.h>
#include "src/GameCheat.h"
#include "src/ConfigManager.h"
#include "src/BackendAPI.h"
#include "src/HWIDManager.h"
#include "src/AuthFlow.h"

int main(int argc, char* argv[]) {
    // Force UTF-8 console output so Cyrillic displays correctly
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);
    
    std::cout << "GameCheat - Restored Version" << std::endl;
    std::cout << "=============================" << std::endl;
    
    // Load configuration
    ConfigManager& config = ConfigManager::getInstance();
    if (!config.loadFromFile("config.ini")) {
        std::cout << "Using default configuration" << std::endl;
    }
    
    // Parse command line arguments
    std::string targetProcess = config.getConfig().targetProcess;
    if (argc > 1) {
        targetProcess = argv[1];
    }
    
    std::cout << "Target process: " << targetProcess << std::endl;
    std::cout << "Server: " << config.getConfig().serverURL << std::endl;

    // ===== Authorization (login/register via Telegram code) =====
    HWIDManager hwidMgr;
    std::string hwid = hwidMgr.getHWID();

    BackendAPI authApi;
    authApi.initialize(config.getConfig().serverURL, config.getConfig().sharedKey);

    AuthFlow auth(&authApi, hwid);
    std::string token, login, telegramId;
    if (!auth.run(token, login, telegramId)) {
        std::cerr << "Authorization failed. Exiting." << std::endl;
        return 1;
    }

    std::cout << "Authorized as: " << login << " (Telegram: " << telegramId << ")" << std::endl;

    // Initialize cheat
    GameCheat cheat;
    if (!cheat.initialize()) {
        std::cerr << "Failed to initialize cheat" << std::endl;
        std::cerr << "Make sure the target process is running" << std::endl;
        return 1;
    }
    
    std::cout << "Cheat initialized successfully" << std::endl;
    std::cout << "Press Ctrl+C to stop..." << std::endl;
    
    // Run cheat
    cheat.run();
    
    std::cout << "Cheat stopped" << std::endl;
    return 0;
}
