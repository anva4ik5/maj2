#include "ConfigManager.h"
#include <fstream>
#include <sstream>
#include <iostream>

ConfigManager::ConfigManager() {
}

ConfigManager& ConfigManager::getInstance() {
    static ConfigManager instance;
    return instance;
}

std::string ConfigManager::trim(const std::string& str) {
    size_t first = str.find_first_not_of(" \t\n\r");
    if (first == std::string::npos) return "";
    
    size_t last = str.find_last_not_of(" \t\n\r");
    return str.substr(first, last - first + 1);
}

std::map<std::string, std::string> ConfigManager::parseConfigFile(const std::string& content) {
    std::map<std::string, std::string> result;
    std::istringstream stream(content);
    std::string line;
    std::string currentSection;
    
    while (std::getline(stream, line)) {
        line = trim(line);
        
        // Skip empty lines and comments
        if (line.empty() || line[0] == '#' || line[0] == ';') {
            continue;
        }
        
        // Section
        if (line[0] == '[' && line.back() == ']') {
            currentSection = line.substr(1, line.length() - 2);
            continue;
        }
        
        // Key=Value
        size_t equalPos = line.find('=');
        if (equalPos != std::string::npos) {
            std::string key = trim(line.substr(0, equalPos));
            std::string value = trim(line.substr(equalPos + 1));
            
            if (!currentSection.empty()) {
                key = currentSection + "." + key;
            }
            
            result[key] = value;
        }
    }
    
    return result;
}

bool ConfigManager::loadFromFile(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Failed to open config file: " << filename << std::endl;
        return false;
    }
    
    std::string content((std::istreambuf_iterator<char>(file)),
                        std::istreambuf_iterator<char>());
    file.close();
    
    auto values = parseConfigFile(content);
    
    // Parse values into config
    for (const auto& pair : values) {
        const std::string& key = pair.first;
        const std::string& value = pair.second;
        
        // Server
        if (key == "Server.host") config.serverHost = value;
        else if (key == "Server.port") config.serverPort = std::stoi(value);
        else if (key == "Server.shared_key") config.sharedKey = value;
        
        // General
        else if (key == "Overlay.enabled") config.enableOverlay = (value == "true");
        else if (key == "Overlay.color") config.overlayColor = std::stoi(value, nullptr, 16);
        else if (key == "General.target_process") config.targetProcess = value;
        else if (key == "General.log_level") config.logLevel = std::stoi(value);
        else if (key == "General.auto_start") config.autoStart = (value == "true");
        
        // Aimbot
        else if (key == "Aimbot.enabled") config.enableAimbot = (value == "true");
        else if (key == "Aimbot.key") config.aimKey = std::stoi(value, nullptr, 16);
        else if (key == "Aimbot.mode") config.aimbotMode = std::stoi(value);
        else if (key == "Aimbot.prediction") config.predictionDistance = std::stof(value);
        else if (key == "Aimbot.silent_auto_fire") config.silentAutoFire = (value == "true");
        else if (key == "Aimbot.random_aim") config.randomAim = (value == "true");
        else if (key == "Aimbot.aim_bone") config.aimBone = std::stoi(value);
        else if (key == "Aimbot.visible_only") config.visibleOnly = (value == "true");
        else if (key == "Aimbot.smooth") config.aimSmooth = std::stof(value);
        else if (key == "Aimbot.show_fov") config.showFOV = (value == "true");
        else if (key == "Aimbot.damager") config.enableDamager = (value == "true");
        else if (key == "Aimbot.damager_key") config.damagerKey = std::stoi(value, nullptr, 16);
        else if (key == "Aimbot.shoot_rate") config.shootRate = std::stoi(value);
        
        // Misc
        else if (key == "Misc.vehicle_no_collision") config.vehicleNoCollision = (value == "true");
        else if (key == "Misc.objects_no_collision") config.objectsNoCollision = (value == "true");
        else if (key == "Misc.noclip") config.noclip = (value == "true");
        else if (key == "Misc.reset_hp") config.resetHP = (value == "true");
        else if (key == "Misc.reset_armour") config.resetArmour = (value == "true");
        else if (key == "Misc.tp_waypoint") config.tpWaypoint = (value == "true");
        else if (key == "Misc.slide_run") config.slideRun = (value == "true");
        else if (key == "Misc.fast_run") config.fastRun = (value == "true");
        else if (key == "Misc.god_mode") config.godMode = (value == "true");
        else if (key == "Misc.suicide") config.suicide = (value == "true");
        else if (key == "Misc.damage") config.damage = (value == "true");
        else if (key == "Misc.damage_amount") config.damageAmount = std::stof(value);
        else if (key == "Misc.recoil_share") config.recoilShare = (value == "true");
        else if (key == "Misc.recoil_accuracy") config.recoilAccuracy = std::stof(value);
        else if (key == "Misc.recoil_recovery") config.recoilRecovery = std::stof(value);
        else if (key == "Misc.anim_reload_speed") config.animReloadSpeed = std::stof(value);
        
        // Visuals
        else if (key == "Visuals.enabled") config.enableESP = (value == "true");
        else if (key == "Visuals.skeleton") config.skeleton = (value == "true");
        else if (key == "Visuals.distance") config.distance = (value == "true");
        else if (key == "Visuals.snapline") config.snapline = (value == "true");
        else if (key == "Visuals.name") config.name = (value == "true");
        else if (key == "Visuals.weapons") config.weapons = (value == "true");
        else if (key == "Visuals.box") config.box = (value == "true");
        else if (key == "Visuals.static_id") config.staticID = (value == "true");
        else if (key == "Visuals.family_id") config.familyID = (value == "true");
        else if (key == "Visuals.fraction") config.fraction = (value == "true");
        else if (key == "Visuals.hp_bar") config.hpBar = (value == "true");
        else if (key == "Visuals.ammo_bar") config.ammoBar = (value == "true");
        else if (key == "Visuals.anim") config.anim = (value == "true");
        else if (key == "Visuals.radar") config.radar = (value == "true");
        else if (key == "Visuals.admin") config.admin = (value == "true");
        else if (key == "Visuals.localplayer") config.localplayer = (value == "true");
        else if (key == "Visuals.objects") config.objects = (value == "true");
        else if (key == "Visuals.vehicles") config.vehicles = (value == "true");
        
        // JS Executor
        else if (key == "JS.enabled") config.enableJSExecutor = (value == "true");
        else if (key == "JS.scripts_path") config.jsScriptsPath = value;
        
        // Logging
        else if (key == "Logging.server_logger") config.enableServerLogger = (value == "true");
        else if (key == "Logging.event_logger") config.enableEventLogger = (value == "true");
        else if (key == "Logging.dump_server") config.dumpServer = (value == "true");
    }
    
    return true;
}

bool ConfigManager::saveToFile(const std::string& filename) {
    std::ofstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Failed to create config file: " << filename << std::endl;
        return false;
    }
    
    file << "[Server]\n";
    file << "host=" << config.serverHost << "\n";
    file << "port=" << config.serverPort << "\n";
    file << "shared_key=" << config.sharedKey << "\n";
    file << "\n";
    
    file << "[Overlay]\n";
    file << "enabled=" << (config.enableOverlay ? "true" : "false") << "\n";
    file << "color=" << std::hex << config.overlayColor << std::dec << "\n";
    file << "\n";
    
    file << "[General]\n";
    file << "target_process=" << config.targetProcess << "\n";
    file << "log_level=" << config.logLevel << "\n";
    file << "auto_start=" << (config.autoStart ? "true" : "false") << "\n";
    file << "\n";
    
    file << "[Aimbot]\n";
    file << "enabled=" << (config.enableAimbot ? "true" : "false") << "\n";
    file << "key=" << std::hex << config.aimKey << std::dec << "\n";
    file << "mode=" << config.aimbotMode << "\n";
    file << "prediction=" << config.predictionDistance << "\n";
    file << "silent_auto_fire=" << (config.silentAutoFire ? "true" : "false") << "\n";
    file << "random_aim=" << (config.randomAim ? "true" : "false") << "\n";
    file << "aim_bone=" << config.aimBone << "\n";
    file << "visible_only=" << (config.visibleOnly ? "true" : "false") << "\n";
    file << "smooth=" << config.aimSmooth << "\n";
    file << "show_fov=" << (config.showFOV ? "true" : "false") << "\n";
    file << "damager=" << (config.enableDamager ? "true" : "false") << "\n";
    file << "damager_key=" << std::hex << config.damagerKey << std::dec << "\n";
    file << "shoot_rate=" << config.shootRate << "\n";
    file << "\n";
    
    file << "[Misc]\n";
    file << "vehicle_no_collision=" << (config.vehicleNoCollision ? "true" : "false") << "\n";
    file << "objects_no_collision=" << (config.objectsNoCollision ? "true" : "false") << "\n";
    file << "noclip=" << (config.noclip ? "true" : "false") << "\n";
    file << "reset_hp=" << (config.resetHP ? "true" : "false") << "\n";
    file << "reset_armour=" << (config.resetArmour ? "true" : "false") << "\n";
    file << "tp_waypoint=" << (config.tpWaypoint ? "true" : "false") << "\n";
    file << "slide_run=" << (config.slideRun ? "true" : "false") << "\n";
    file << "fast_run=" << (config.fastRun ? "true" : "false") << "\n";
    file << "god_mode=" << (config.godMode ? "true" : "false") << "\n";
    file << "suicide=" << (config.suicide ? "true" : "false") << "\n";
    file << "damage=" << (config.damage ? "true" : "false") << "\n";
    file << "damage_amount=" << config.damageAmount << "\n";
    file << "recoil_share=" << (config.recoilShare ? "true" : "false") << "\n";
    file << "recoil_accuracy=" << config.recoilAccuracy << "\n";
    file << "recoil_recovery=" << config.recoilRecovery << "\n";
    file << "anim_reload_speed=" << config.animReloadSpeed << "\n";
    file << "\n";
    
    file << "[Visuals]\n";
    file << "enabled=" << (config.enableESP ? "true" : "false") << "\n";
    file << "skeleton=" << (config.skeleton ? "true" : "false") << "\n";
    file << "distance=" << (config.distance ? "true" : "false") << "\n";
    file << "snapline=" << (config.snapline ? "true" : "false") << "\n";
    file << "name=" << (config.name ? "true" : "false") << "\n";
    file << "weapons=" << (config.weapons ? "true" : "false") << "\n";
    file << "box=" << (config.box ? "true" : "false") << "\n";
    file << "static_id=" << (config.staticID ? "true" : "false") << "\n";
    file << "family_id=" << (config.familyID ? "true" : "false") << "\n";
    file << "fraction=" << (config.fraction ? "true" : "false") << "\n";
    file << "hp_bar=" << (config.hpBar ? "true" : "false") << "\n";
    file << "ammo_bar=" << (config.ammoBar ? "true" : "false") << "\n";
    file << "anim=" << (config.anim ? "true" : "false") << "\n";
    file << "radar=" << (config.radar ? "true" : "false") << "\n";
    file << "admin=" << (config.admin ? "true" : "false") << "\n";
    file << "localplayer=" << (config.localplayer ? "true" : "false") << "\n";
    file << "objects=" << (config.objects ? "true" : "false") << "\n";
    file << "vehicles=" << (config.vehicles ? "true" : "false") << "\n";
    file << "\n";
    
    file << "[JS]\n";
    file << "enabled=" << (config.enableJSExecutor ? "true" : "false") << "\n";
    file << "scripts_path=" << config.jsScriptsPath << "\n";
    file << "\n";
    
    file << "[Logging]\n";
    file << "server_logger=" << (config.enableServerLogger ? "true" : "false") << "\n";
    file << "event_logger=" << (config.enableEventLogger ? "true" : "false") << "\n";
    file << "dump_server=" << (config.dumpServer ? "true" : "false") << "\n";
    
    file.close();
    return true;
}

ConfigManager::Config& ConfigManager::getConfig() {
    return config;
}

const ConfigManager::Config& ConfigManager::getConfig() const {
    return config;
}

void ConfigManager::set(const std::string& key, const std::string& value) {
    // Simple key=value setter
    if (key == "server_host") config.serverHost = value;
    else if (key == "server_port") config.serverPort = std::stoi(value);
    else if (key == "enable_overlay") config.enableOverlay = (value == "true");
    else if (key == "enable_aimbot") config.enableAimbot = (value == "true");
    else if (key == "enable_esp") config.enableESP = (value == "true");
    else if (key == "enable_wallhack") config.enableWallhack = (value == "true");
}

std::string ConfigManager::get(const std::string& key) const {
    if (key == "server_host") return config.serverHost;
    else if (key == "server_port") return std::to_string(config.serverPort);
    else if (key == "enable_overlay") return config.enableOverlay ? "true" : "false";
    else if (key == "enable_aimbot") return config.enableAimbot ? "true" : "false";
    else if (key == "enable_esp") return config.enableESP ? "true" : "false";
    else if (key == "enable_wallhack") return config.enableWallhack ? "true" : "false";
    else if (key == "target_process") return config.targetProcess;
    return "";
}

bool ConfigManager::saveConfig(const std::string& name) {
    std::string filename = "./configs/" + name + ".ini";
    return saveToFile(filename);
}

bool ConfigManager::loadConfig(const std::string& name) {
    std::string filename = "./configs/" + name + ".ini";
    return loadFromFile(filename);
}

bool ConfigManager::removeConfig(const std::string& name) {
    std::string filename = "./configs/" + name + ".ini";
    return std::remove(filename.c_str()) == 0;
}

std::vector<std::string> ConfigManager::listConfigs() {
    std::vector<std::string> configs;
    
    // TODO: List files in ./configs/ directory
    // This would require filesystem iteration
    
    return configs;
}

bool ConfigManager::uploadConfig(const std::string& name) {
    // TODO: Upload config to server using shared key
    // This would require NetworkClient
    return false;
}

bool ConfigManager::downloadConfig(const std::string& name) {
    // TODO: Download config from server using shared key
    // This would require NetworkClient
    return false;
}
