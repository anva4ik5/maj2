#pragma once

#include "MemoryManager.h"
#include "ConfigManager.h"
#include <cstdint>
#include <vector>
#include <memory>

struct Vector3 {
    float x, y, z;
    Vector3(float x = 0, float y = 0, float z = 0) : x(x), y(y), z(z) {}
    
    float distanceTo(const Vector3& other) const {
        float dx = x - other.x;
        float dy = y - other.y;
        float dz = z - other.z;
        return sqrt(dx*dx + dy*dy + dz*dz);
    }
};

struct Entity {
    uintptr_t address;
    Vector3 position;
    Vector3 headPosition;
    int health;
    int team;
    bool isVisible;
    std::string name;
    std::string weapon;
};

class Aimbot {
public:
    Aimbot();
    ~Aimbot();
    
    void initialize(HANDLE hProcess);
    void update();
    void setTarget(Entity* entity);
    
    bool isAimKeyHeld() const;
    bool isDamagerKeyHeld() const;
    
    void setAimKey(int key) { aimKey = key; }
    void setDamagerKey(int key) { damagerKey = key; }
    
    void setEnabled(bool value) { enabled = value; }
    bool isEnabled() const { return enabled; }
    
    void setMode(int mode) { aimbotMode = mode; }
    void setAimBone(int bone) { aimBone = bone; }
    void setSmooth(float smooth) { aimSmooth = smooth; }
    void setPrediction(float prediction) { predictionDistance = prediction; }
    
    Entity* getBestTarget();
    void aimAt(Entity* entity);
    void shoot();
    
private:
    HANDLE hProcess;
    ConfigManager::Config* config;
    
    bool enabled;
    int aimKey;
    int damagerKey;
    int aimbotMode; // 0=FOV, 1=Distance, 2=Silent
    int aimBone;
    bool visibleOnly;
    float aimSmooth;
    float predictionDistance;
    bool silentAutoFire;
    bool randomAim;
    bool showFOV;
    bool enableDamager;
    int shootRate;
    
    int currentTargetIndex;
    uint64_t lastShotTime;
    
    std::vector<Entity> entities;
    
    void updateEntities();
    void sortTargetsByFOV();
    void sortTargetsByDistance();
    
    Vector3 calculateViewAngle(Vector3 from, Vector3 to);
    Vector3 predictPosition(Vector3 target, float prediction);
    void writeViewAngles(Vector3 angles);
    Vector3 readViewAngles();
    
    bool isEntityVisible(Entity* entity);
    bool isEntityInFOV(Entity* entity, float fov);
    
    float getFOV(Vector3 viewAngle, Vector3 targetAngle);
};
