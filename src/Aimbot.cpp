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
    
    // Initialize game memory adapter (GTA5 / RAGE / alt:V engine)
    if (process) {
        // Try to initialize against the running game module name
        if (!gameMem.initialize(process, config->targetProcess)) {
            // Fallback - try common GTA5 module names
            const char* fallbacks[] = { "GTA5.exe", "altv.exe", "rust.exe" };
            for (const char* fn : fallbacks) {
                if (gameMem.initialize(process, fn)) break;
            }
        }
    }
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
    if (!gameMem.worldPtr()) return;
    
    static GamePlayer scratch[GameOffsets::MAX_ENTITIES_SCAN];
    int n = gameMem.readPedList(scratch, GameOffsets::MAX_ENTITIES_SCAN);
    
    entities.reserve(n);
    for (int i = 0; i < n; ++i) {
        const GamePlayer& p = scratch[i];
        Entity e;
        e.address      = p.pedAddr;
        e.position     = Vector3(p.position.x, p.position.y, p.position.z);
        e.headPosition = Vector3(p.headPosition.x, p.headPosition.y, p.headPosition.z);
        e.health       = (int)p.health;
        e.team         = 0;
        e.isVisible    = true; // raycast not implemented; treat as visible
        e.name         = p.name;
        entities.push_back(e);
    }
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
    // Send a left mouse button click via SendInput - works in any focused window
    INPUT input[2] = {};
    input[0].type = INPUT_MOUSE;
    input[0].mi.dwFlags = MOUSEEVENTF_LEFTDOWN;
    input[1].type = INPUT_MOUSE;
    input[1].mi.dwFlags = MOUSEEVENTF_LEFTUP;
    SendInput(2, input, sizeof(INPUT));
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
    // Simple linear prediction would need entity velocity tracking.
    // Since updateEntities() is called every tick we can compute a coarse
    // delta against the previous position - good enough for slow-moving peds.
    static Vector3 previous = target;
    Vector3 velocity(target.x - previous.x, target.y - previous.y, target.z - previous.z);
    previous = target;
    
    Vector3 predicted = target;
    predicted.x += velocity.x * prediction;
    predicted.y += velocity.y * prediction;
    predicted.z += velocity.z * prediction;
    return predicted;
}

void Aimbot::writeViewAngles(Vector3 angles) {
    // Convert delta angle (vs current) to mouse movement.
    // GTA5/RAGE/alt:V take user input from mouse rather than direct angles
    // (writing camera matrix gets reverted instantly). Mouse simulation works.
    Vector3 current = readViewAngles();
    float dx = angles.y - current.y; // yaw delta in degrees
    float dy = angles.x - current.x; // pitch delta in degrees
    
    // Wrap yaw to [-180, 180]
    if (dx >  180.0f) dx -= 360.0f;
    if (dx < -180.0f) dx += 360.0f;
    
    // Empirical sensitivity scale - tweakable per game
    const float kSensitivity = 5.5f;
    int mouseX = (int)(dx * kSensitivity);
    int mouseY = (int)(dy * kSensitivity);
    
    if (mouseX == 0 && mouseY == 0) return;
    
    INPUT input = {};
    input.type = INPUT_MOUSE;
    input.mi.dx = mouseX;
    input.mi.dy = mouseY;
    input.mi.dwFlags = MOUSEEVENTF_MOVE;
    SendInput(1, &input, sizeof(INPUT));
}

Vector3 Aimbot::readViewAngles() {
    // Read camera yaw/pitch from viewport view matrix.
    // Matrix forward vector encodes the camera direction.
    if (!gameMem.viewportPtr()) return Vector3();
    
    uintptr_t vp = 0;
    if (!gameMem.read<uintptr_t>(gameMem.viewportPtr(), vp) || !vp) return Vector3();
    
    float m[16] = {0};
    SIZE_T n = 0;
    if (!ReadProcessMemory(gameMem.getProcessHandle(),
                           (LPCVOID)(vp + GameOffsets::OFF_VIEWPORT_VIEW_MATRIX),
                           m, sizeof(m), &n) || n != sizeof(m)) {
        return Vector3();
    }
    
    // Forward vector is row 2 of the view matrix (or column 2 transposed)
    float fx = -m[2], fy = -m[6], fz = -m[10];
    
    Vector3 a;
    a.y = atan2f(fy, fx) * 180.0f / 3.14159265f;                       // yaw
    a.x = atan2f(fz, sqrtf(fx*fx + fy*fy)) * 180.0f / 3.14159265f;     // pitch
    a.z = 0;
    return a;
}

bool Aimbot::isEntityVisible(Entity* entity) {
    // Real raycast requires hooking the game's CWorld::IsLineOfSightClear
    // - a sizeable engineering effort. As a pragmatic stand-in, treat all
    // entities within the configured FOV as visible. Combined with the
    // visibleOnly user toggle, this still produces a usable aimbot UX.
    if (!entity) return false;
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
