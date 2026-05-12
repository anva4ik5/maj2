#pragma once

#include <windows.h>
#include <string>
#include <vector>

class AntiCheatBypass {
public:
    AntiCheatBypass();
    ~AntiCheatBypass();
    
    void initialize();
    void applyBypasses();
    void restore();
    
    // Majestic specific
    void applyMajesticBypasses();
    void bypassMajesticIntegrity();
    void bypassMajesticScreenshot();
    void bypassMajesticMemoryScan();
    
    // Easy Anti-Cheat specific
    void applyEACBypasses();
    void bypassEACIntegrity();
    void bypassEACMemoryScan();
    void bypassEACScreenshot();
    
    // BattlEye specific
    void applyBEBypasses();
    void bypassBEIntegrity();
    void bypassBEMemoryScan();
    
    // General anti-cheat bypasses
    void applyGeneralBypasses();
    
    // Process hiding
    bool hideProcess();
    bool unhookProcess();
    
    // Memory protection
    bool protectMemoryRegions();
    bool unprotectMemoryRegions();
    
    // Module hiding
    bool hideModule(const std::string& moduleName);
    
    // Thread protection
    bool protectThreads();
    
    // Handle cleanup
    bool cleanHandles();
    
    // Anti-debug
    void enableAntiDebug();
    void disableAntiDebug();
    
    // Signature scrambling
    void scrambleSignatures();
    
    // Screenshot bypass
    void enableScreenshotBypass();
    void disableScreenshotBypass();
    
    // NVIDIA protection
    void enableNVIDIAProtection();
    void disableNVIDIAProtection();
    void hideFromNVIDIA();
    
private:
    bool initialized;
    bool bypassesApplied;
    
    // Anti-debug checks
    bool isDebuggerPresent();
    bool isRemoteDebuggerPresent();
    bool checkHardwareBreakpoints();
    
    // Memory manipulation
    void patchMemory(void* address, const std::vector<uint8_t>& bytes);
    void restoreMemory(void* address, const std::vector<uint8_t>& originalBytes);
    
    // Handle management
    void closeUnnecessaryHandles();
    
    // Thread hiding
    bool hideThread(DWORD threadId);
    
    // Majestic specific
    void hookMajesticFunctions();
    void unhookMajesticFunctions();
    bool isMajesticRunning();
};
