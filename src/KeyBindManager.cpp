#include "KeyBindManager.h"
#include <windows.h>
#include <fstream>
#include <sstream>
#include <iostream>

static int safeStoi(const std::string& value, int defaultValue = 0) {
    try { return std::stoi(value); } catch (...) { return defaultValue; }
}

KeyBindManager::KeyBindManager() {
}

KeyBindManager::~KeyBindManager() {
}

void KeyBindManager::initialize() {
    // Создать дефолтные бинды
    // Пользователь может их изменить
}

void KeyBindManager::update() {
    for (auto& pair : binds) {
        KeyBind& bind = pair.second;
        
        bool isPressed = isKeyPressed(bind.keyCode);
        
        switch (bind.type) {
            case BindType::TOGGLE:
                // Срабатывает только при нажатии (edge trigger)
                if (isPressed && !bind.wasPressed) {
                    bind.isActive = !bind.isActive;
                    if (bind.action) bind.action();
                }
                break;
                
            case BindType::HOLD:
                // Активно пока удерживается
                if (isPressed != bind.isActive) {
                    bind.isActive = isPressed;
                    if (bind.action && isPressed) bind.action();
                }
                break;
                
            case BindType::TRIGGER:
                // Срабатывает при каждом нажатии
                if (isPressed && !bind.wasPressed) {
                    if (bind.action) bind.action();
                }
                break;
        }
        
        bind.wasPressed = isPressed;
    }
}

void KeyBindManager::addBind(const std::string& name, int keyCode, BindType type, std::function<void()> action) {
    KeyBind bind;
    bind.keyCode = keyCode;
    bind.type = type;
    bind.name = name;
    bind.action = action;
    bind.isActive = false;
    bind.wasPressed = false;
    
    binds[name] = bind;
}

void KeyBindManager::removeBind(const std::string& name) {
    auto it = binds.find(name);
    if (it != binds.end()) {
        binds.erase(it);
    }
}

void KeyBindManager::setBindKey(const std::string& name, int keyCode) {
    auto it = binds.find(name);
    if (it != binds.end()) {
        it->second.keyCode = keyCode;
    }
}

void KeyBindManager::setBindType(const std::string& name, BindType type) {
    auto it = binds.find(name);
    if (it != binds.end()) {
        it->second.type = type;
    }
}

KeyBind* KeyBindManager::getBind(const std::string& name) {
    auto it = binds.find(name);
    if (it != binds.end()) {
        return &it->second;
    }
    return nullptr;
}

std::vector<std::string> KeyBindManager::getBindNames() const {
    std::vector<std::string> names;
    for (const auto& pair : binds) {
        names.push_back(pair.first);
    }
    return names;
}

void KeyBindManager::saveBinds(const std::string& filename) {
    std::ofstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Failed to save binds to: " << filename << std::endl;
        return;
    }
    
    for (const auto& pair : binds) {
        const KeyBind& bind = pair.second;
        file << bind.name << "=" << bind.keyCode << "," << static_cast<int>(bind.type) << "\n";
    }
    
    file.close();
}

void KeyBindManager::loadBinds(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Failed to load binds from: " << filename << std::endl;
        return;
    }
    
    std::string line;
    while (std::getline(file, line)) {
        size_t equalPos = line.find('=');
        if (equalPos == std::string::npos) continue;
        
        std::string name = line.substr(0, equalPos);
        std::string value = line.substr(equalPos + 1);
        
        size_t commaPos = value.find(',');
        if (commaPos == std::string::npos) continue;
        
        int keyCode = safeStoi(value.substr(0, commaPos));
        int typeInt = safeStoi(value.substr(commaPos + 1));
        BindType type = static_cast<BindType>(typeInt);
        
        auto it = binds.find(name);
        if (it != binds.end()) {
            it->second.keyCode = keyCode;
            it->second.type = type;
        }
    }
    
    file.close();
}

void KeyBindManager::resetBinds() {
    binds.clear();
}

bool KeyBindManager::isKeyPressed(int keyCode) {
    return (GetAsyncKeyState(keyCode) & 0x8000) != 0;
}

std::string KeyBindManager::keyCodeToString(int keyCode) {
    // Конвертация кода клавиши в строку
    switch (keyCode) {
        case VK_F1: return "F1";
        case VK_F2: return "F2";
        case VK_F3: return "F3";
        case VK_F4: return "F4";
        case VK_F5: return "F5";
        case VK_F6: return "F6";
        case VK_INSERT: return "INSERT";
        case VK_DELETE: return "DELETE";
        case VK_HOME: return "HOME";
        case VK_END: return "END";
        case VK_PRIOR: return "PAGE UP";
        case VK_NEXT: return "PAGE DOWN";
        case 0x02: return "RMB";
        case 0x01: return "LMB";
        case 0x04: return "MMB";
        case 0x05: return "X1";
        case 0x06: return "X2";
        default:
            if (keyCode >= 0x30 && keyCode <= 0x39) {
                return std::string(1, '0' + (keyCode - 0x30));
            }
            if (keyCode >= 0x41 && keyCode <= 0x5A) {
                return std::string(1, 'A' + (keyCode - 0x41));
            }
            return std::to_string(keyCode);
    }
}

int KeyBindManager::stringToKeyCode(const std::string& str) {
    // Конвертация строки в код клавиши
    if (str == "F1") return VK_F1;
    if (str == "F2") return VK_F2;
    if (str == "F3") return VK_F3;
    if (str == "F4") return VK_F4;
    if (str == "F5") return VK_F5;
    if (str == "F6") return VK_F6;
    if (str == "INSERT") return VK_INSERT;
    if (str == "DELETE") return VK_DELETE;
    if (str == "HOME") return VK_HOME;
    if (str == "END") return VK_END;
    if (str == "PAGE UP") return VK_PRIOR;
    if (str == "PAGE DOWN") return VK_NEXT;
    if (str == "RMB") return 0x02;
    if (str == "LMB") return 0x01;
    if (str == "MMB") return 0x04;
    if (str == "X1") return 0x05;
    if (str == "X2") return 0x06;
    
    if (str.length() == 1) {
        char c = str[0];
        if (c >= '0' && c <= '9') return 0x30 + (c - '0');
        if (c >= 'A' && c <= 'Z') return 0x41 + (c - 'A');
        if (c >= 'a' && c <= 'z') return 0x41 + (c - 'a');
    }
    
    return std::stoi(str);
}
