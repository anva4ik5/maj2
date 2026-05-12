#include "JSExecutor.h"
#include <fstream>
#include <iostream>
#include <filesystem>

JSExecutor::JSExecutor() : enabled(false) {
}

JSExecutor::~JSExecutor() {
}

void JSExecutor::initialize() {
    scriptsPath = "./scripts/";
    
    // Create scripts directory if it doesn't exist
    if (!std::filesystem::exists(scriptsPath)) {
        std::filesystem::create_directory(scriptsPath);
    }
    
    // Load all scripts
    loadAllScripts();
    
    // Register API
    registerAPI();
}

void JSExecutor::loadAllScripts() {
    scripts.clear();
    
    if (!std::filesystem::exists(scriptsPath)) return;
    
    for (const auto& entry : std::filesystem::directory_iterator(scriptsPath)) {
        if (entry.path().extension() == ".js") {
            std::string filename = entry.path().filename().string();
            std::string content = readFile(entry.path().string());
            scripts[filename] = content;
        }
    }
}

void JSExecutor::loadScript(const std::string& filename) {
    std::string fullPath = scriptsPath + filename;
    std::string content = readFile(fullPath);
    
    if (!content.empty()) {
        scripts[filename] = content;
        std::cout << "Loaded script: " << filename << std::endl;
    } else {
        std::cerr << "Failed to load script: " << filename << std::endl;
    }
}

void JSExecutor::removeScript(const std::string& filename) {
    auto it = scripts.find(filename);
    if (it != scripts.end()) {
        scripts.erase(it);
        
        // Delete file
        std::string fullPath = scriptsPath + filename;
        std::filesystem::remove(fullPath);
        
        std::cout << "Removed script: " << filename << std::endl;
    }
}

void JSExecutor::editScript(const std::string& filename, const std::string& content) {
    scripts[filename] = content;
    writeFile(scriptsPath + filename, content);
    std::cout << "Edited script: " << filename << std::endl;
}

void JSExecutor::listScripts() {
    std::cout << "=== Loaded Scripts ===" << std::endl;
    for (const auto& pair : scripts) {
        std::cout << pair.first << std::endl;
    }
}

void JSExecutor::execute(const std::string& code) {
    // TODO: Implement JavaScript execution
    // This would require a JS engine like V8, QuickJS, or ChakraCore
    // For now, this is a placeholder
    
    std::cout << "Executing JS code:" << std::endl;
    std::cout << code << std::endl;
    
    // Simple command parsing (not real JS)
    if (code.find("log(") != std::string::npos) {
        size_t start = code.find("log(") + 4;
        size_t end = code.find(")", start);
        if (end != std::string::npos) {
            std::string message = code.substr(start, end - start);
            // Remove quotes
            if (message.front() == '"' || message.front() == '\'') {
                message = message.substr(1, message.length() - 2);
            }
            api_log(message);
        }
    }
}

void JSExecutor::executeScript(const std::string& filename) {
    auto it = scripts.find(filename);
    if (it != scripts.end()) {
        execute(it->second);
    } else {
        std::cerr << "Script not found: " << filename << std::endl;
    }
}

std::vector<std::string> JSExecutor::getScriptList() const {
    std::vector<std::string> list;
    for (const auto& pair : scripts) {
        list.push_back(pair.first);
    }
    return list;
}

std::string JSExecutor::readFile(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        return "";
    }
    
    std::string content((std::istreambuf_iterator<char>(file)),
                        std::istreambuf_iterator<char>());
    file.close();
    
    return content;
}

void JSExecutor::writeFile(const std::string& filename, const std::string& content) {
    std::ofstream file(filename);
    if (file.is_open()) {
        file << content;
        file.close();
    }
}

void JSExecutor::registerAPI() {
    // TODO: Register API functions with JS engine
    // This would expose cheat functions to JavaScript
}

void JSExecutor::api_log(const std::string& message) {
    std::cout << "[JS] " << message << std::endl;
}

void JSExecutor::api_print(const std::string& message) {
    std::cout << "[JS] " << message << std::endl;
}

void JSExecutor::api_setAimbotEnabled(bool enabled) {
    // TODO: Set aimbot enabled state
    std::cout << "[JS] Set aimbot: " << (enabled ? "true" : "false") << std::endl;
}

void JSExecutor::api_setESPEnabled(bool enabled) {
    // TODO: Set ESP enabled state
    std::cout << "[JS] Set ESP: " << (enabled ? "true" : "false") << std::endl;
}

void JSExecutor::api_setGodMode(bool enabled) {
    // TODO: Set god mode
    std::cout << "[JS] Set god mode: " << (enabled ? "true" : "false") << std::endl;
}

void JSExecutor::api_teleport(float x, float y, float z) {
    // TODO: Teleport player
    std::cout << "[JS] Teleport to: " << x << ", " << y << ", " << z << std::endl;
}

void JSExecutor::api_setHP(int hp) {
    // TODO: Set player HP
    std::cout << "[JS] Set HP: " << hp << std::endl;
}

void JSExecutor::api_getHP() {
    // TODO: Get player HP
    std::cout << "[JS] Get HP" << std::endl;
}

void JSExecutor::api_getPosition() {
    // TODO: Get player position
    std::cout << "[JS] Get position" << std::endl;
}
