#include <iostream>
#include <string>
#include "src/GameCheat.h"
#include "src/ConfigManager.h"

int main(int argc, char* argv[]) {
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
    std::cout << "Server: " << config.getConfig().serverHost << ":" << config.getConfig().serverPort << std::endl;
    
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
