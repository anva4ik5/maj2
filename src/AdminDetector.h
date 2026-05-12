#pragma once

#include <string>
#include <vector>
#include <functional>

struct AdminInfo {
    std::string name;
    int id;
    bool isAdmin;
    bool isModerator;
    bool isDeveloper;
    float distance;
};

class AdminDetector {
public:
    AdminDetector();
    ~AdminDetector();
    
    void initialize();
    void update();
    
    // Detection methods
    bool isAdminNearby();
    bool isAdminWatching();
    bool isSpectating();
    
    // Get admin list
    std::vector<AdminInfo> getNearbyAdmins();
    std::vector<AdminInfo> getAllAdmins();
    
    // Warning system
    void setWarningCallback(std::function<void()> callback);
    void setAutoDisableOnAdmin(bool enable);
    
    // Admin list management
    void addKnownAdmin(int id, const std::string& name);
    void removeKnownAdmin(int id);
    void clearKnownAdmins();
    
    // Detection settings
    void setDetectionRadius(float radius);
    void setSpectatorCheckInterval(int seconds);
    
private:
    bool initialized;
    float detectionRadius;
    int spectatorCheckInterval;
    bool autoDisableOnAdmin;
    
    std::vector<AdminInfo> knownAdmins;
    std::function<void()> warningCallback;
    
    // Detection methods
    bool checkForAdminSpectator();
    bool checkForAdminInRadius();
    bool checkForAdminCommands();
    
    // Entity scanning
    void scanForAdmins();
    bool isAdminEntity(int entityId);
    
    // Network monitoring
    bool monitorAdminPackets();
    bool detectAdminTeleport();
};
