#pragma once

#include "DirectXOverlay.h"
#include "ConfigManager.h"
#include "Aimbot.h"
#include "GameMemory.h"
#include <vector>
#include <string>

class Visuals {
public:
    Visuals(DirectXOverlay* overlay);
    ~Visuals();
    
    void initialize(HANDLE hProcess);
    void render();
    
    void setEnabled(bool value) { enabled = value; }
    bool isEnabled() const { return enabled; }
    
private:
    DirectXOverlay* overlay;
    HANDLE hProcess;
    ConfigManager::Config* config;
    GameMemory gameMem;
    
    bool enabled;
    
    // Visual options
    bool skeleton;
    bool distance;
    bool snapline;
    bool name;
    bool weapons;
    bool box;
    bool staticID;
    bool familyID;
    bool fraction;
    bool hpBar;
    bool ammoBar;
    bool anim;
    bool radar;
    bool admin;
    bool localplayer;
    bool objects;
    bool vehicles;
    
    std::vector<Entity> entities;
    Vector3 localPlayerPosition;
    
    void updateEntities();
    void renderSkeleton(Entity* entity);
    void renderDistance(Entity* entity);
    void renderSnapline(Entity* entity);
    void renderName(Entity* entity);
    void renderWeapons(Entity* entity);
    void renderBox(Entity* entity);
    void renderStaticID(Entity* entity);
    void renderFamilyID(Entity* entity);
    void renderFraction(Entity* entity);
    void renderHPBar(Entity* entity);
    void renderAmmoBar(Entity* entity);
    void renderAnim(Entity* entity);
    void renderRadar();
    void renderAdmin(Entity* entity);
    void renderLocalPlayer();
    void renderObjects();
    void renderVehicles();
    
    Vec2 worldToScreen(Vector3 worldPos);
    Color getTeamColor(int team);
    Color getHealthColor(int health, int maxHealth);
};
