#pragma once

#include <string>
#include <vector>
#include <map>

enum class AntiCheatType {
    NONE,
    EASY_ANTICHEAT,
    BATTLEYE,
    VANGUARD,
    PUNKBUSTER,
    FAIRFIGHT,
    MAJESTIC,
    ALT_V,
    UNKNOWN
};

struct AntiCheatInfo {
    AntiCheatType type;
    std::string name;
    std::string version;
    std::string processName;
    bool detected;
    bool bypassAvailable;
    std::string status;
};

class AntiCheatDetector {
public:
    AntiCheatDetector();
    ~AntiCheatDetector();
    
    void initialize();
    AntiCheatInfo detect();
    
    // Check specific anti-cheats
    bool isEasyAntiCheatRunning();
    bool isBattlEyeRunning();
    bool isVanguardRunning();
    bool isMajesticRunning();
    bool isAltVRunning();
    
    // Get detected anti-cheat
    AntiCheatInfo getDetectedAntiCheat();
    
    // Check if bypass is available
    bool hasBypass(AntiCheatType type);
    
    // Add custom anti-cheat signature
    void addAntiCheatSignature(const std::string& name, const std::string& signature);
    
    // Get all supported anti-cheats
    std::vector<AntiCheatInfo> getSupportedAntiCheats();
    
private:
    std::vector<AntiCheatInfo> supportedAntiCheats;
    AntiCheatInfo detectedAntiCheat;
    
    // Detection methods
    bool checkProcess(const std::string& processName);
    bool checkModule(const std::string& moduleName);
    bool checkRegistry(const std::string& key);
    
    // Initialize supported anti-cheats
    void initializeSupportedAntiCheats();
    
    // Get anti-cheat info by type
    AntiCheatInfo getAntiCheatInfo(AntiCheatType type);
};
