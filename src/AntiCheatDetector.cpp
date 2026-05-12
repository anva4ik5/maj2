#include "AntiCheatDetector.h"
#include <windows.h>
#include <tlhelp32.h>
#include <iostream>

AntiCheatDetector::AntiCheatDetector() {
    initializeSupportedAntiCheats();
}

AntiCheatDetector::~AntiCheatDetector() {
}

void AntiCheatDetector::initialize() {
    std::cout << "AntiCheatDetector initialized" << std::endl;
}

AntiCheatInfo AntiCheatDetector::detect() {
    detectedAntiCheat = {AntiCheatType::NONE, "None", "", "", false, true, "No anti-cheat detected"};
    
    // Check for known anti-cheats
    if (isEasyAntiCheatRunning()) {
        detectedAntiCheat = getAntiCheatInfo(AntiCheatType::EASY_ANTICHEAT);
        detectedAntiCheat.detected = true;
    } else if (isBattlEyeRunning()) {
        detectedAntiCheat = getAntiCheatInfo(AntiCheatType::BATTLEYE);
        detectedAntiCheat.detected = true;
    } else if (isVanguardRunning()) {
        detectedAntiCheat = getAntiCheatInfo(AntiCheatType::VANGUARD);
        detectedAntiCheat.detected = true;
    } else if (isMajesticRunning()) {
        detectedAntiCheat = getAntiCheatInfo(AntiCheatType::MAJESTIC);
        detectedAntiCheat.detected = true;
    }
    
    return detectedAntiCheat;
}

bool AntiCheatDetector::isEasyAntiCheatRunning() {
    // Check for EasyAntiCheat processes
    return checkProcess("EasyAntiCheat.exe") || 
           checkProcess("EasyAntiCheat_x64.exe") ||
           checkModule("EasyAntiCheat.sys");
}

bool AntiCheatDetector::isBattlEyeRunning() {
    // Check for BattlEye processes
    return checkProcess("BEService.exe") || 
           checkProcess("BEClient_x64.exe") ||
           checkModule("BEDaisy.sys");
}

bool AntiCheatDetector::isVanguardRunning() {
    // Check for Vanguard processes
    return checkProcess("vgk.sys") || 
           checkProcess("Riot Vanguard") ||
           checkModule("vgk.sys");
}

bool AntiCheatDetector::isMajesticRunning() {
    // Check for Majestic RP anti-cheat
    return checkProcess("gta5.exe") && 
           checkModule("majestic_ac.dll");
}

AntiCheatInfo AntiCheatDetector::getDetectedAntiCheat() {
    return detectedAntiCheat;
}

bool AntiCheatDetector::hasBypass(AntiCheatType type) {
    for (const auto& ac : supportedAntiCheats) {
        if (ac.type == type) {
            return ac.bypassAvailable;
        }
    }
    return false;
}

void AntiCheatDetector::addAntiCheatSignature(const std::string& name, const std::string& signature) {
    // Add custom anti-cheat signature for detection
}

std::vector<AntiCheatInfo> AntiCheatDetector::getSupportedAntiCheats() {
    return supportedAntiCheats;
}

bool AntiCheatDetector::checkProcess(const std::string& processName) {
    HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hSnapshot == INVALID_HANDLE_VALUE) return false;
    
    PROCESSENTRY32 pe32;
    pe32.dwSize = sizeof(PROCESSENTRY32);
    
    bool found = false;
    if (Process32First(hSnapshot, &pe32)) {
        do {
            std::string currentProcess = pe32.szExeFile;
            if (currentProcess.find(processName) != std::string::npos) {
                found = true;
                break;
            }
        } while (Process32Next(hSnapshot, &pe32));
    }
    
    CloseHandle(hSnapshot);
    return found;
}

bool AntiCheatDetector::checkModule(const std::string& moduleName) {
    // Check if module is loaded in current process
    HMODULE hModule = GetModuleHandleA(moduleName.c_str());
    return hModule != nullptr;
}

bool AntiCheatDetector::checkRegistry(const std::string& key) {
    // Check registry for anti-cheat installation
    HKEY hKey;
    if (RegOpenKeyExA(HKEY_LOCAL_MACHINE, key.c_str(), 0, KEY_READ, &hKey) == ERROR_SUCCESS) {
        RegCloseKey(hKey);
        return true;
    }
    return false;
}

void AntiCheatDetector::initializeSupportedAntiCheats() {
    // Easy Anti-Cheat
    AntiCheatInfo eac;
    eac.type = AntiCheatType::EASY_ANTICHEAT;
    eac.name = "Easy Anti-Cheat";
    eac.version = "Latest";
    eac.processName = "EasyAntiCheat.exe";
    eac.detected = false;
    eac.bypassAvailable = true;
    eac.status = "Bypass available";
    supportedAntiCheats.push_back(eac);
    
    // BattlEye
    AntiCheatInfo be;
    be.type = AntiCheatType::BATTLEYE;
    be.name = "BattlEye";
    be.version = "Latest";
    be.processName = "BEService.exe";
    be.detected = false;
    be.bypassAvailable = true;
    be.status = "Bypass available";
    supportedAntiCheats.push_back(be);
    
    // Vanguard
    AntiCheatInfo vg;
    vg.type = AntiCheatType::VANGUARD;
    vg.name = "Riot Vanguard";
    vg.version = "Latest";
    vg.processName = "vgk.sys";
    vg.detected = false;
    vg.bypassAvailable = false;
    vg.status = "Bypass not available - kernel level";
    supportedAntiCheats.push_back(vg);
    
    // Majestic
    AntiCheatInfo majestic;
    majestic.type = AntiCheatType::MAJESTIC;
    majestic.name = "Majestic RP Anti-Cheat";
    majestic.version = "Latest";
    majestic.processName = "majestic_ac.dll";
    majestic.detected = false;
    majestic.bypassAvailable = true;
    majestic.status = "Bypass available";
    supportedAntiCheats.push_back(majestic);
    
    // PunkBuster
    AntiCheatInfo pb;
    pb.type = AntiCheatType::PUNKBUSTER;
    pb.name = "PunkBuster";
    pb.version = "Latest";
    pb.processName = "PnkBstrA.exe";
    pb.detected = false;
    pb.bypassAvailable = true;
    pb.status = "Bypass available";
    supportedAntiCheats.push_back(pb);
    
    // FairFight
    AntiCheatInfo ff;
    ff.type = AntiCheatType::FAIRFIGHT;
    ff.name = "FairFight";
    ff.version = "Latest";
    ff.processName = "fairfight.dll";
    ff.detected = false;
    pb.bypassAvailable = true;
    ff.status = "Bypass available";
    supportedAntiCheats.push_back(ff);
}

AntiCheatInfo AntiCheatDetector::getAntiCheatInfo(AntiCheatType type) {
    for (const auto& ac : supportedAntiCheats) {
        if (ac.type == type) {
            return ac;
        }
    }
    
    AntiCheatInfo unknown;
    unknown.type = AntiCheatType::UNKNOWN;
    unknown.name = "Unknown Anti-Cheat";
    unknown.bypassAvailable = false;
    unknown.status = "Bypass not available - unknown anti-cheat";
    return unknown;
}
