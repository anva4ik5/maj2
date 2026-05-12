#include "DLLInjector.h"
#include <iostream>
#include <TlHelp32.h>

bool DLLInjector::createRemoteThreadAndWait(HANDLE hProcess, LPVOID pFunction, LPVOID pParameter) {
    HANDLE hThread = CreateRemoteThread(hProcess, nullptr, 0, 
        (LPTHREAD_START_ROUTINE)pFunction, pParameter, 0, nullptr);
    
    if (!hThread) {
        return false;
    }
    
    WaitForSingleObject(hThread, INFINITE);
    CloseHandle(hThread);
    
    return true;
}

bool DLLInjector::injectDLL(HANDLE hProcess, const std::string& dllPath) {
    size_t pathLen = dllPath.length() + 1;
    
    // Allocate memory in target process
    LPVOID pRemoteMemory = VirtualAllocEx(hProcess, nullptr, pathLen, MEM_COMMIT, PAGE_READWRITE);
    if (!pRemoteMemory) {
        std::cerr << "Failed to allocate memory in target process" << std::endl;
        return false;
    }
    
    // Write DLL path to target process
    SIZE_T bytesWritten;
    if (!WriteProcessMemory(hProcess, pRemoteMemory, dllPath.c_str(), pathLen, &bytesWritten)) {
        std::cerr << "Failed to write DLL path to target process" << std::endl;
        VirtualFreeEx(hProcess, pRemoteMemory, 0, MEM_RELEASE);
        return false;
    }
    
    // Get LoadLibraryA address
    HMODULE hKernel32 = GetModuleHandleA("kernel32.dll");
    LPVOID pLoadLibraryA = (LPVOID)GetProcAddress(hKernel32, "LoadLibraryA");
    
    if (!pLoadLibraryA) {
        std::cerr << "Failed to get LoadLibraryA address" << std::endl;
        VirtualFreeEx(hProcess, pRemoteMemory, 0, MEM_RELEASE);
        return false;
    }
    
    // Create remote thread to load DLL
    if (!createRemoteThreadAndWait(hProcess, pLoadLibraryA, pRemoteMemory)) {
        std::cerr << "Failed to create remote thread" << std::endl;
        VirtualFreeEx(hProcess, pRemoteMemory, 0, MEM_RELEASE);
        return false;
    }
    
    // Cleanup
    VirtualFreeEx(hProcess, pRemoteMemory, 0, MEM_RELEASE);
    
    return true;
}

bool DLLInjector::injectDLLByPID(DWORD pid, const std::string& dllPath) {
    HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pid);
    if (!hProcess) {
        std::cerr << "Failed to open process with PID: " << pid << std::endl;
        return false;
    }
    
    bool result = injectDLL(hProcess, dllPath);
    CloseHandle(hProcess);
    
    return result;
}

bool DLLInjector::injectDLLByName(const std::string& processName, const std::string& dllPath) {
    HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (snapshot == INVALID_HANDLE_VALUE) {
        return false;
    }
    
    PROCESSENTRY32 pe32;
    pe32.dwSize = sizeof(PROCESSENTRY32);
    
    bool found = false;
    DWORD pid = 0;
    
    if (Process32First(snapshot, &pe32)) {
        do {
            std::string currentProcess = pe32.szExeFile;
            if (currentProcess == processName) {
                pid = pe32.th32ProcessID;
                found = true;
                break;
            }
        } while (Process32Next(snapshot, &pe32));
    }
    
    CloseHandle(snapshot);
    
    if (!found) {
        std::cerr << "Process not found: " << processName << std::endl;
        return false;
    }
    
    return injectDLLByPID(pid, dllPath);
}

bool DLLInjector::ejectDLL(HANDLE hProcess, HMODULE hModule) {
    HMODULE hKernel32 = GetModuleHandleA("kernel32.dll");
    LPVOID pFreeLibrary = (LPVOID)GetProcAddress(hKernel32, "FreeLibrary");
    
    if (!pFreeLibrary) {
        return false;
    }
    
    return createRemoteThreadAndWait(hProcess, pFreeLibrary, (LPVOID)hModule);
}
