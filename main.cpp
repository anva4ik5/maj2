#include <iostream>
#include <string>
#include <windows.h>
#include <vector>
#include <thread>
#include <chrono>
#include <limits>
#include "src/GameCheat.h"
#include "src/ConfigManager.h"
#include "src/BackendAPI.h"
#include "src/HWIDManager.h"
#include "src/AuthFlow.h"
#include "src/MemoryManager.h"

static void pauseBeforeExit(int code) {
    std::cout << "\nPress Enter to exit..." << std::endl;
    std::cin.clear();
    std::cin.ignore((std::numeric_limits<std::streamsize>::max)(), '\n');
    std::cin.get();
    (void)code;
}

static std::vector<std::string> resolveCandidates(const std::string& targetProcess) {
    if (targetProcess == "auto" || targetProcess.empty()) {
        return { "altv.exe", "altv-launcher.exe", "GTA5.exe", "rust.exe" };
    }
    std::vector<std::string> out;
    std::string s = targetProcess;
    size_t pos = 0;
    while ((pos = s.find_first_of(",;")) != std::string::npos) {
        std::string item = s.substr(0, pos);
        if (!item.empty()) out.push_back(item);
        s.erase(0, pos + 1);
    }
    if (!s.empty()) out.push_back(s);
    return out;
}

static bool waitForProcess(const std::vector<std::string>& candidates, std::string& foundName) {
    std::cout << "\nWaiting for game process..." << std::endl;
    std::cout << "Looking for:";
    for (const auto& n : candidates) std::cout << " " << n;
    std::cout << "\n(Press Ctrl+C to cancel)" << std::endl;
    
    int dots = 0;
    while (true) {
        for (const auto& name : candidates) {
            HANDLE h = MemoryManager::getProcessHandle(name);
            if (h) {
                CloseHandle(h);
                foundName = name;
                std::cout << "\nFound: " << name << std::endl;
                return true;
            }
        }
        std::cout << "." << std::flush;
        if (++dots % 30 == 0) std::cout << "\nstill waiting..." << std::flush;
        std::this_thread::sleep_for(std::chrono::seconds(2));
    }
}

int main(int argc, char* argv[]) {
    // Force UTF-8 console output so Cyrillic displays correctly
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);
    
    std::cout << "GameCheat - Restored Version" << std::endl;
    std::cout << "=============================" << std::endl;
    
    // Load configuration (config.ini is optional - embedded defaults are used otherwise)
    ConfigManager& config = ConfigManager::getInstance();
    if (config.loadFromFile("config.ini")) {
        std::cout << "Loaded config.ini" << std::endl;
    } else {
        std::cout << "Using built-in configuration (no config.ini)" << std::endl;
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
        pauseBeforeExit(1);
        return 1;
    }

    std::cout << "\n[OK] Authorized as: " << login;
    if (!telegramId.empty()) std::cout << " (Telegram: " << telegramId << ")";
    std::cout << std::endl;

    // Wait for game process to appear before initializing cheat
    auto candidates = resolveCandidates(targetProcess);
    std::string foundProcess;
    if (!waitForProcess(candidates, foundProcess)) {
        std::cerr << "Could not find target process" << std::endl;
        pauseBeforeExit(1);
        return 1;
    }
    
    // Persist the actually found process name so GameCheat picks it up
    config.getConfig().targetProcess = foundProcess;
    
    // Give the game a moment to fully load
    std::cout << "Waiting 3 seconds for game to fully load..." << std::endl;
    std::this_thread::sleep_for(std::chrono::seconds(3));
    
    // Initialize cheat
    GameCheat cheat;
    if (!cheat.initialize()) {
        std::cerr << "\n[ERROR] Failed to initialize cheat" << std::endl;
        std::cerr << "Possible reasons:" << std::endl;
        std::cerr << "  - Game is not fully loaded yet" << std::endl;
        std::cerr << "  - Run as Administrator" << std::endl;
        std::cerr << "  - Anti-cheat blocked the injection" << std::endl;
        pauseBeforeExit(1);
        return 1;
    }
    
    std::cout << "\n[OK] Cheat initialized successfully" << std::endl;
    std::cout << "Press END to toggle menu, Ctrl+C to stop" << std::endl;
    
    // Run cheat
    cheat.run();
    
    std::cout << "\nCheat stopped" << std::endl;
    pauseBeforeExit(0);
    return 0;
}
