#pragma once

#include <windows.h>
#include <string>
#include <TlHelp32.h>
#include <vector>

class MemoryManager {
public:
    static HANDLE getProcessHandle(const std::string& processName);
    static HANDLE getProcessHandleByPID(DWORD pid);
    static std::vector<DWORD> getProcessList();
    
    static bool readMemory(HANDLE hProcess, uintptr_t address, void* buffer, size_t size);
    static bool writeMemory(HANDLE hProcess, uintptr_t address, const void* buffer, size_t size);
    
    static uintptr_t allocateMemory(HANDLE hProcess, size_t size);
    static bool freeMemory(HANDLE hProcess, uintptr_t address);
    static bool changeProtection(HANDLE hProcess, uintptr_t address, size_t size, DWORD newProtect);
    
    static uintptr_t findPattern(HANDLE hProcess, uintptr_t start, size_t size, const std::string& pattern);
    static uintptr_t findModule(HANDLE hProcess, const std::string& moduleName);
    static uintptr_t getModuleBase(HANDLE hProcess, const std::string& moduleName);
    
    template<typename T>
    static T read(HANDLE hProcess, uintptr_t address) {
        T value;
        readMemory(hProcess, address, &value, sizeof(T));
        return value;
    }
    
    template<typename T>
    static bool write(HANDLE hProcess, uintptr_t address, T value) {
        return writeMemory(hProcess, address, &value, sizeof(T));
    }
};
