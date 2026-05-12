#pragma once

#include <windows.h>
#include <string>

class DLLInjector {
public:
    static bool injectDLL(HANDLE hProcess, const std::string& dllPath);
    static bool injectDLLByPID(DWORD pid, const std::string& dllPath);
    static bool injectDLLByName(const std::string& processName, const std::string& dllPath);
    
    static bool ejectDLL(HANDLE hProcess, HMODULE hModule);
    
private:
    static bool createRemoteThreadAndWait(HANDLE hProcess, LPVOID pFunction, LPVOID pParameter);
};
