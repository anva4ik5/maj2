#include "Aimbot.h"
#include <windows.h>
#include <cmath>
#include <algorithm>

Aimbot::Aimbot() 
    : hProcess(nullptr), enabled(false), aimKey(0x02), damagerKey(0x05),
      aimbotMode(0), aimBone(6), visibleOnly(true), aimSmooth(0.5f),
      predictionDistance(0.5f), silentAutoFire(false), randomAim(false),
      showFOV(false), enableDamager(false), shootRate(100),
      currentTargetIndex(-1), lastShotTime(0) {
}

Aimbot::~Aimbot() {
}

void Aimbot::initialize(HANDLE process) {
    hProcess = process;
    config = &ConfigManager::getInstance().getConfig();
    
    // Load config values
    enabled = config->enableAimbot;
    aimKey = config->aimKey;
    damagerKey = config->damagerKey;
    aimbotMode = config->aimbotMode;
    aimBone = config->aimBone;
    visibleOnly = config->visibleOnly;
    aimSmooth = config->aimSmooth;
    predictionDistance = config->predictionDistance;
    silentAutoFire = config->silentAutoFire;
    randomAim = config->randomAim;
    showFOV = config->showFOV;
    enableDamager = config->enableDamager;
    shootRate = config->shootRate;
}

void Aimbot::update() {
    if (!hProcess || !enabled) {
        currentTargetIndex = -1;
        return;
    }
    
    // Update entities from game memory
    updateEntities();
    
    // Check if aim key is held
    if (isAimKeyHeld()) {
        // Get best target
        Entity* bestTarget = getBestTarget();
        
        if (bestTarget) {
            // Aim at target
            aimAt(bestTarget);
            
            // Auto fire if enabled
            if (silentAutoFire) {
                uint64_t currentTime = GetTickCount64();
                if (currentTime - lastShotTime >= shootRate) {
                    shoot();
                    lastShotTime = currentTime;
                }
            }
        }
    }
    
    // Check damager key
    if (enableDamager && isDamagerKeyHeld()) {
        if (currentTargetIndex >= 0 && currentTargetIndex < static_cast<int>(entities.size())) {
            // Apply damage to target
            // TODO: Implement damage logic using entities[currentTargetIndex]
        }
    }
}

bool Aimbot::isAimKeyHeld() const {
    return (GetAsyncKeyState(aimKey) & 0x8000) != 0;
}

bool Aimbot::isDamagerKeyHeld() const {
    return (GetAsyncKeyState(damagerKey) & 0x8000) != 0;
}

void Aimbot::updateEntities() {
    entities.clear();
    
    // TODO: Read entities from game memory
    // This requires knowledge of the game's memory structure
    // For now, this is a placeholder
    
    // Example (would need actual game offsets):
    /*
    uintptr_t entityList = MemoryManager::read<uintptr_t>(hProcess, ENTITY_LIST_OFFSET);
    int entityCount = MemoryManager::read<int>(hProcess, ENTITY_COUNT_OFFSET);
    
    for (int i = 0; i < entityCount; i++) {
        uintptr_t entityPtr = MemoryManager::read<uintptr_t>(hProcess, entityList + i * 8);
        
        Entity entity;
        entity.address = entityPtr;
        entity.position = MemoryManager::read<Vector3>(hProcess, entityPtr + POSITION_OFFSET);
        entity.headPosition = MemoryManager::read<Vector3>(hProcess, entityPtr + HEAD_OFFSET);
        entity.health = MemoryManager::read<int>(hProcess, entityPtr + HEALTH_OFFSET);
        entity.team = MemoryManager::read<int>(hProcess, entityPtr + TEAM_OFFSET);
        entity.isVisible = isEntityVisible(&entity);
        
        entities.push_back(entity);
    }
    */
}

Entity* Aimbot::getBestTarget() {
    currentTargetIndex = -1;
    if (entities.empty()) return nullptr;
    
    // Sort targets based on mode
    if (aimbotMode == 0) {
        sortTargetsByFOV();
    } else if (aimbotMode == 1) {
        sortTargetsByDistance();
    }
    
    // Find first valid target
    for (size_t i = 0; i < entities.size(); ++i) {
        if (visibleOnly && !entities[i].isVisible) continue;
        if (entities[i].health <= 0) continue;
        
        currentTargetIndex = static_cast<int>(i);
        return &entities[i]; // Valid only until next updateEntities() call
    }
    
    return nullptr;
}

void Aimbot::sortTargetsByFOV() {
    Vector3 currentViewAngle = readViewAngles();
    
    std::sort(entities.begin(), entities.end(), 
        [this, currentViewAngle](const Entity& a, const Entity& b) {
            Vector3 angleA = calculateViewAngle(currentViewAngle, a.headPosition);
            Vector3 angleB = calculateViewAngle(currentViewAngle, b.headPosition);
            return getFOV(currentViewAngle, angleA) < getFOV(currentViewAngle, angleB);
        });
}

void Aimbot::sortTargetsByDistance() {
    Vector3 localPlayerPos = readViewAngles(); // Should be local player position
    
    std::sort(entities.begin(), entities.end(),
        [localPlayerPos](const Entity& a, const Entity& b) {
            return a.position.distanceTo(localPlayerPos) < b.position.distanceTo(localPlayerPos);
        });
}

void Aimbot::aimAt(Entity* entity) {
    if (!entity) return;
    
    Vector3 targetPos = entity->headPosition;
    
    // Apply prediction
    if (predictionDistance > 0) {
        targetPos = predictPosition(targetPos, predictionDistance);
    }
    
    // Add random offset if enabled
    if (randomAim) {
        targetPos.x += (float)(rand() % 10 - 5) * 0.01f;
        targetPos.y += (float)(rand() % 10 - 5) * 0.01f;
        targetPos.z += (float)(rand() % 10 - 5) * 0.01f;
    }
    
    Vector3 localPlayerPos = readViewAngles(); // Should be local player position
    Vector3 targetAngle = calculateViewAngle(localPlayerPos, targetPos);
    
    // Apply smoothing
    Vector3 currentAngle = readViewAngles();
    Vector3 smoothedAngle;
    smoothedAngle.x = currentAngle.x + (targetAngle.x - currentAngle.x) * aimSmooth;
    smoothedAngle.y = currentAngle.y + (targetAngle.y - currentAngle.y) * aimSmooth;
    smoothedAngle.z = currentAngle.z + (targetAngle.z - currentAngle.z) * aimSmooth;
    
    // Write angles
    writeViewAngles(smoothedAngle);
    
    (void)entity; // Target tracking handled via currentTargetIndex
}

void Aimbot::shoot() {
    // TODO: Implement shooting logic
    // This would involve calling the game's fire function or simulating mouse click
}

Vector3 Aimbot::calculateViewAngle(Vector3 from, Vector3 to) {
    Vector3 delta;
    delta.x = to.x - from.x;
    delta.y = to.y - from.y;
    delta.z = to.z - from.z;
    
    Vector3 angle;
    angle.x = atan2(delta.z, sqrt(delta.x * delta.x + delta.y * delta.y)) * 180.0f / 3.14159265f;
    angle.y = atan2(delta.y, delta.x) * 180.0f / 3.14159265f;
    angle.z = 0;
    
    return angle;
}

Vector3 Aimbot::predictPosition(Vector3 target, float prediction) {
    // Simple linear prediction
    // TODO: Implement proper velocity-based prediction
    return target;
}

void Aimbot::writeViewAngles(Vector3 angles) {
    // TODO: Write angles to game memory
    // MemoryManager::write<Vector3>(hProcess, VIEW_ANGLE_OFFSET, angles);
}

Vector3 Aimbot::readViewAngles() {
    // TODO: Read angles from game memory
    // return MemoryManager::read<Vector3>(hProcess, VIEW_ANGLE_OFFSET);
    return Vector3();
}

bool Aimbot::isEntityVisible(Entity* entity) {
    // TODO: Implement raycast or visibility check
    return true;
}

bool Aimbot::isEntityInFOV(Entity* entity, float fov) {
    Vector3 currentAngle = readViewAngles();
    Vector3 targetAngle = calculateViewAngle(currentAngle, entity->headPosition);
    return getFOV(currentAngle, targetAngle) <= fov;
}

float Aimbot::getFOV(Vector3 viewAngle, Vector3 targetAngle) {
    float dx = targetAngle.x - viewAngle.x;
    float dy = targetAngle.y - viewAngle.y;
    return sqrt(dx*dx + dy*dy);
}
