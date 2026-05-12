#pragma once

#include "MemoryManager.h"
#include "ConfigManager.h"
#include <cstdint>

class Misc {
public:
    Misc();
    ~Misc();
    
    void initialize(HANDLE hProcess);
    void update();
    
    // Vehicle functions
    void setVehicleNoCollision(bool enabled);
    
    // Object functions
    void setObjectsNoCollision(bool enabled);
    
    // Player functions
    void setNoclip(bool enabled);
    void resetHP();
    void resetArmour();
    void teleportToWaypoint();
    void setSlideRun(bool enabled);
    void setFastRun(bool enabled);
    void setGodMode(bool enabled);
    void suicide();
    void applyDamage(float amount);
    
    // Recoil functions
    void setRecoilShare(bool enabled);
    void setRecoilAccuracy(float accuracy);
    void setRecoilRecovery(float recovery);
    void setAnimReloadSpeed(float speed);
    
private:
    HANDLE hProcess;
    ConfigManager::Config* config;
    
    bool vehicleNoCollision;
    bool objectsNoCollision;
    bool noclip;
    bool slideRun;
    bool fastRun;
    bool godMode;
    bool recoilShare;
    
    float recoilAccuracy;
    float recoilRecovery;
    float animReloadSpeed;
    
    void applyVehicleNoCollision();
    void applyObjectsNoCollision();
    void applyNoclip();
    void applySlideRun();
    void applyFastRun();
    void applyGodMode();
    void applyRecoilModifications();
};
