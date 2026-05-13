#include "MemoryManager.h"
#include <iostream>
#include <psapi.h>

#pragma comment(lib, "psapi.lib")

HANDLE MemoryManager::getProcessHandle(const std::string& processName) {
    HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (snapshot == INVALID_HANDLE_VALUE) {
        return nullptr;
    }
    
    PROCESSENTRY32 pe32;
    pe32.dwSize = sizeof(PROCESSENTRY32);
    
    if (Process32First(snapshot, &pe32)) {
        do {
            std::string currentProcess = pe32.szExeFile;
            if (currentProcess == processName) {
                HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pe32.th32ProcessID);
                CloseHandle(snapshot);
                return hProcess;
            }
        } while (Process32Next(snapshot, &pe32));
    }
    
    CloseHandle(snapshot);
    return nullptr;
}

HANDLE MemoryManager::getProcessHandleByPID(DWORD pid) {
    return OpenProcess(PROCESS_ALL_ACCESS, FALSE, pid);
}

std::vector<DWORD> MemoryManager::getProcessList() {
    std::vector<DWORD> pids;
    HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    
    if (snapshot == INVALID_HANDLE_VALUE) {
        return pids;
    }
    
    PROCESSENTRY32 pe32;
    pe32.dwSize = sizeof(PROCESSENTRY32);
    
    if (Process32First(snapshot, &pe32)) {
        do {
            pids.push_back(pe32.th32ProcessID);
        } while (Process32Next(snapshot, &pe32));
    }
    
    CloseHandle(snapshot);
    return pids;
}

bool MemoryManager::readMemory(HANDLE hProcess, uintptr_t address, void* buffer, size_t size) {
    SIZE_T bytesRead = 0;
    return ReadProcessMemory(hProcess, (LPCVOID)address, buffer, size, &bytesRead) == TRUE;
}

bool MemoryManager::writeMemory(HANDLE hProcess, uintptr_t address, const void* buffer, size_t size) {
    SIZE_T bytesWritten = 0;
    return WriteProcessMemory(hProcess, (LPVOID)address, buffer, size, &bytesWritten) == TRUE;
}

uintptr_t MemoryManager::allocateMemory(HANDLE hProcess, size_t size) {
    return (uintptr_t)VirtualAllocEx(hProcess, nullptr, size, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
}

bool MemoryManager::freeMemory(HANDLE hProcess, uintptr_t address) {
    return VirtualFreeEx(hProcess, (LPVOID)address, 0, MEM_RELEASE) == TRUE;
}

bool MemoryManager::changeProtection(HANDLE hProcess, uintptr_t address, size_t size, DWORD newProtect) {
    DWORD oldProtect;
    return VirtualProtectEx(hProcess, (LPVOID)address, size, newProtect, &oldProtect) == TRUE;
}

uintptr_t MemoryManager::findPattern(HANDLE hProcess, uintptr_t start, size_t size, const std::string& pattern) {
    std::vector<uint8_t> buffer(size);
    
    if (!readMemory(hProcess, start, buffer.data(), size)) {
        return 0;
    }
    
    // Convert pattern string to byte array
    std::vector<uint8_t> patternBytes;
    std::vector<bool> patternMask;
    
    for (size_t i = 0; i < pattern.length(); i += 3) {
        if (i + 1 >= pattern.length()) break;
        
        if (pattern[i] == '?' && (i + 1 < pattern.length() && pattern[i+1] == '?')) {
            patternBytes.push_back(0);
            patternMask.push_back(false);
        } else {
            std::string byteStr = pattern.substr(i, 2);
            uint8_t byte = (uint8_t)strtol(byteStr.c_str(), nullptr, 16);
            patternBytes.push_back(byte);
            patternMask.push_back(true);
        }
    }
    
    // Search for pattern
    if (patternBytes.empty() || patternBytes.size() > buffer.size()) {
        return 0;
    }
    for (size_t i = 0; i <= buffer.size() - patternBytes.size(); i++) {
        bool match = true;
        
        for (size_t j = 0; j < patternBytes.size(); j++) {
            if (patternMask[j] && buffer[i + j] != patternBytes[j]) {
                match = false;
                break;
            }
        }
        
        if (match) {
            return start + i;
        }
    }
    
    return 0;
}

uintptr_t MemoryManager::findModule(HANDLE hProcess, const std::string& moduleName) {
    return getModuleBase(hProcess, moduleName);
}

uintptr_t MemoryManager::getModuleBase(HANDLE hProcess, const std::string& moduleName) {
    HMODULE hModules[1024];
    DWORD cbNeeded;
    
    if (EnumProcessModules(hProcess, hModules, sizeof(hModules), &cbNeeded)) {
        for (size_t i = 0; i < cbNeeded / sizeof(HMODULE); i++) {
            char szModName[MAX_PATH];
            if (GetModuleFileNameExA(hProcess, hModules[i], szModName, sizeof(szModName))) {
                std::string modName = szModName;
                if (modName.find(moduleName) != std::string::npos) {
                    return (uintptr_t)hModules[i];
                }
            }
        }
    }
    
    return 0;
}
