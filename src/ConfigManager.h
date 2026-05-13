#pragma once

#include <string>
#include <map>
#include <vector>

class ConfigManager {
public:
    struct Config {
        // Server
        std::string serverHost = "127.0.0.1";
        int serverPort = 8080;
        std::string serverURL = "https://majcheat-production.up.railway.app";
        std::string sharedKey = "";
        std::string telegramBotToken = "";
        std::string adminTelegramID = "";
        
        // General
        bool enableOverlay = true;
        int overlayColor = 0xFFFFFFFF;
        std::string targetProcess = "rust.exe";
        int logLevel = 1;
        bool autoStart = false;
        
        // Aimbot
        bool enableAimbot = false;
        int aimKey = 0x02; // Right mouse button
        int aimbotMode = 0; // 0=FOV, 1=Distance, 2=Silent
        float predictionDistance = 0.5f;
        bool silentAutoFire = false;
        bool randomAim = false;
        int aimBone = 6; // Head
        bool visibleOnly = true;
        float aimSmooth = 0.5f;
        bool showFOV = false;
        bool enableDamager = false;
        int damagerKey = 0x05; // X key
        int shootRate = 100; // ms between shots
        
        // Misc
        bool vehicleNoCollision = false;
        bool objectsNoCollision = false;
        bool noclip = false;
        bool resetHP = false;
        bool resetArmour = false;
        bool tpWaypoint = false;
        bool slideRun = false;
        bool fastRun = false;
        bool godMode = false;
        bool suicide = false;
        bool damage = false;
        float damageAmount = 100.0f;
        bool recoilShare = false;
        float recoilAccuracy = 1.0f;
        float recoilRecovery = 1.0f;
        float animReloadSpeed = 1.0f;
        bool invisibility = false;
        
        // Visuals
        bool enableESP = true;
        bool skeleton = true;
        bool distance = true;
        bool snapline = false;
        bool name = true;
        bool weapons = true;
        bool box = true;
        bool staticID = false;
        bool familyID = false;
        bool fraction = false;
        bool hpBar = true;
        bool ammoBar = true;
        bool anim = false;
        bool radar = false;
        bool admin = false;
        bool localplayer = false;
        bool objects = false;
        bool vehicles = false;
        
        // JS Executor
        bool enableJSExecutor = false;
        std::string jsScriptsPath = "./scripts/";
        
        // Logging
        bool enableServerLogger = false;
        bool enableEventLogger = false;
        bool dumpServer = false;
        
        // OBS Bypass
        bool obsBypassEnabled = true;
        bool obsAutoDetect = true;
        bool obsHideOverlay = true;
        
        // Game Mode
        std::string gameMode = "majestic_rp";
        
        // Self-Destruct
        bool enableSelfDestruct = false;
        int selfDestructDelay = 0;
        
        // Admin Detector
        bool enableAdminDetector = true;
        bool autoDisableOnAdmin = true;
        float adminDetectionRadius = 100.0f;
    };
    
    static ConfigManager& getInstance();
    
    bool loadFromFile(const std::string& filename);
    bool saveToFile(const std::string& filename);
    
    Config& getConfig();
    const Config& getConfig() const;
    
    void set(const std::string& key, const std::string& value);
    std::string get(const std::string& key) const;
    
    // Config management
    bool saveConfig(const std::string& name);
    bool loadConfig(const std::string& name);
    bool removeConfig(const std::string& name);
    std::vector<std::string> listConfigs();
    bool uploadConfig(const std::string& name);
    bool downloadConfig(const std::string& name);
    
private:
    ConfigManager();
    Config config;
    
    std::string trim(const std::string& str);
    std::map<std::string, std::string> parseConfigFile(const std::string& content);
};
