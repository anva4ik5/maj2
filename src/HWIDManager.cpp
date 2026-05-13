#include "HWIDManager.h"
#include <iostream>
#include <sstream>
#include <iomanip>
#include <regex>
#include <fstream>
#include <iphlpapi.h>
#include <intrin.h>
#pragma comment(lib, "iphlpapi.lib")

HWIDManager::HWIDManager() {
    hwid = generateHWIDHash();
}

HWIDManager::~HWIDManager() {
}

void HWIDManager::initialize() {
    if (hwid.empty()) {
        hwid = generateHWIDHash();
    }
}

std::string HWIDManager::getHWID() {
    return hwid;
}

std::string HWIDManager::getCPUID() {
    int cpuInfo[4] = {0};
    __cpuid(cpuInfo, 0);
    
    std::stringstream ss;
    for (int i = 0; i < 4; ++i) {
        ss << std::hex << cpuInfo[i];
    }
    
    return ss.str();
}

std::string HWIDManager::getMotherboardSerial() {
    HKEY hKey;
    std::string serial = "";
    
    if (RegOpenKeyExA(HKEY_LOCAL_MACHINE, "HARDWARE\\DESCRIPTION\\System\\BIOS", 0, KEY_READ, &hKey) == ERROR_SUCCESS) {
        char data[256];
        DWORD dataSize = sizeof(data);
        
        if (RegQueryValueExA(hKey, "BaseBoardSerialNumber", NULL, NULL, (LPBYTE)data, &dataSize) == ERROR_SUCCESS) {
            serial = std::string(data);
        }
        
        RegCloseKey(hKey);
    }
    
    return serial;
}

std::string HWIDManager::getMACAddress() {
    IP_ADAPTER_INFO AdapterInfo[32];
    DWORD dwBufLen = sizeof(AdapterInfo);
    
    if (GetAdaptersInfo(AdapterInfo, &dwBufLen) == ERROR_SUCCESS) {
        PIP_ADAPTER_INFO pAdapterInfo = AdapterInfo;
        std::stringstream ss;
        
        for (int i = 0; i < 6; i++) {
            ss << std::hex << std::setw(2) << std::setfill('0') << (int)pAdapterInfo->Address[i];
            if (i < 5) ss << ":";
        }
        
        return ss.str();
    }
    
    return "00:00:00:00:00:00";
}

std::string HWIDManager::getDiskSerial() {
    std::string command = "wmic diskdrive get serialnumber";
    std::string output = executeCommand(command);
    
    // Parse output to get serial number
    std::regex serialRegex("\\b([A-Z0-9]{10,})\\b");
    std::smatch match;
    
    if (std::regex_search(output, match, serialRegex)) {
        return match[1].str();
    }
    
    return "UNKNOWN_DISK";
}

std::string HWIDManager::generateHWIDHash() {
    std::string combined = getCPUID() + getMotherboardSerial() + getMACAddress() + getDiskSerial();
    return hashString(combined);
}

bool HWIDManager::verifyHWID(const std::string& storedHWID) {
    std::string currentHWID = generateHWIDHash();
    return currentHWID == storedHWID;
}

void HWIDManager::saveHWID(const std::string& filename) {
    std::ofstream file(filename);
    if (file.is_open()) {
        file << hwid;
        file.close();
    }
}

std::string HWIDManager::loadHWID(const std::string& filename) {
    std::ifstream file(filename);
    if (file.is_open()) {
        std::string content;
        std::getline(file, content);
        file.close();
        return content;
    }
    return "";
}

std::string HWIDManager::executeCommand(const std::string& command) {
    char buffer[128];
    std::string result = "";
    
    FILE* pipe = _popen(command.c_str(), "r");
    if (pipe) {
        while (fgets(buffer, sizeof(buffer), pipe) != NULL) {
            result += buffer;
        }
        _pclose(pipe);
    }
    
    return result;
}

std::string HWIDManager::hashString(const std::string& input) {
    // Simple hash function (in production, use proper cryptographic hash)
    unsigned int hash = 5381;
    
    for (char c : input) {
        hash = ((hash << 5) + hash) + c;
    }
    
    std::stringstream ss;
    ss << std::hex << hash;
    return ss.str();
}

std::string HWIDManager::bytesToHex(const unsigned char* bytes, size_t len) {
    std::stringstream ss;
    for (size_t i = 0; i < len; i++) {
        ss << std::hex << std::setw(2) << std::setfill('0') << (int)bytes[i];
    }
    return ss.str();
}
