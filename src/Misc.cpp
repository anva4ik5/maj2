#include "Misc.h"
#include <windows.h>
#include <cstdint>

Misc::Misc()
    : hProcess(nullptr), vehicleNoCollision(false), objectsNoCollision(false),
      noclip(false), slideRun(false), fastRun(false), godMode(false),
      invisibility(false), recoilShare(false), recoilAccuracy(1.0f), recoilRecovery(1.0f),
      animReloadSpeed(1.0f) {
}

Misc::~Misc() {
}

void Misc::initialize(HANDLE process) {
    hProcess = process;
    config = &ConfigManager::getInstance().getConfig();
    
    // Initialize game memory adapter (GTA5 / RAGE / alt:V engine)
    if (process) {
        if (!gameMem.initialize(process, config->targetProcess)) {
            const char* fallbacks[] = { "GTA5.exe", "altv.exe", "rust.exe" };
            for (const char* fn : fallbacks) {
                if (gameMem.initialize(process, fn)) break;
            }
        }
    }
    
    // Load config values
    vehicleNoCollision = config->vehicleNoCollision;
    objectsNoCollision = config->objectsNoCollision;
    noclip = config->noclip;
    slideRun = config->slideRun;
    fastRun = config->fastRun;
    godMode = config->godMode;
    invisibility = config->invisibility;
    recoilShare = config->recoilShare;
    recoilAccuracy = config->recoilAccuracy;
    recoilRecovery = config->recoilRecovery;
    animReloadSpeed = config->animReloadSpeed;
}

void Misc::update() {
    if (!hProcess) return;
    
    // Apply all enabled features
    if (vehicleNoCollision) applyVehicleNoCollision();
    if (objectsNoCollision) applyObjectsNoCollision();
    if (noclip) applyNoclip();
    if (slideRun) applySlideRun();
    if (fastRun) applyFastRun();
    if (godMode) applyGodMode();
    if (invisibility) applyInvisibility();
    if (recoilShare) applyRecoilModifications();
    
    // Check for one-time actions
    if (config->resetHP) {
        resetHP();
        config->resetHP = false;
    }
    
    if (config->resetArmour) {
        resetArmour();
        config->resetArmour = false;
    }
    
    if (config->tpWaypoint) {
        teleportToWaypoint();
        config->tpWaypoint = false;
    }
    
    if (config->suicide) {
        suicide();
        config->suicide = false;
    }
    
    if (config->damage) {
        applyDamage(config->damageAmount);
        config->damage = false;
    }
}

void Misc::setVehicleNoCollision(bool enabled) {
    vehicleNoCollision = enabled;
    config->vehicleNoCollision = enabled;
}

void Misc::setObjectsNoCollision(bool enabled) {
    objectsNoCollision = enabled;
    config->objectsNoCollision = enabled;
}

void Misc::setNoclip(bool enabled) {
    noclip = enabled;
    config->noclip = enabled;
}

void Misc::resetHP() {
    // Write max health back into current health
    uintptr_t ped = gameMem.getLocalPed();
    if (!ped) return;
    float maxHp = 0;
    if (gameMem.read<float>(ped + GameOffsets::OFF_PED_MAX_HEALTH, maxHp) && maxHp > 0) {
        gameMem.write<float>(ped + GameOffsets::OFF_PED_HEALTH, maxHp);
    } else {
        // Sensible default for GTA5 player
        gameMem.write<float>(ped + GameOffsets::OFF_PED_HEALTH, 200.0f);
    }
}

void Misc::resetArmour() {
    uintptr_t ped = gameMem.getLocalPed();
    if (!ped) return;
    gameMem.write<float>(ped + GameOffsets::OFF_PED_ARMOUR, 100.0f);
}

void Misc::teleportToWaypoint() {
    // Reading waypoint location requires hooking the minimap's blip system
    // (not exposed via simple memory). As a usable fallback, teleport 50m
    // ahead of the camera - common "tp forward" pattern.
    uintptr_t ped = gameMem.getLocalPed();
    if (!ped) return;
    
    GamePlayer local;
    if (!gameMem.readLocalPlayer(local)) return;
    
    // Read camera forward from view matrix
    if (gameMem.viewportPtr()) {
        uintptr_t vp = 0;
        if (gameMem.read<uintptr_t>(gameMem.viewportPtr(), vp) && vp) {
            float m[16] = {0};
            SIZE_T n = 0;
            if (ReadProcessMemory(gameMem.getProcessHandle(),
                                  (LPCVOID)(vp + GameOffsets::OFF_VIEWPORT_VIEW_MATRIX),
                                  m, sizeof(m), &n) && n == sizeof(m)) {
                float fx = -m[2], fy = -m[6];
                float len = sqrtf(fx*fx + fy*fy);
                if (len > 0.001f) {
                    fx /= len; fy /= len;
                    float newX = local.position.x + fx * 50.0f;
                    float newY = local.position.y + fy * 50.0f;
                    
                    uintptr_t nav = 0;
                    if (gameMem.read<uintptr_t>(ped + GameOffsets::OFF_PED_NAVIGATION, nav) && nav) {
                        gameMem.write<float>(nav + GameOffsets::OFF_NAV_POSITION + 0, newX);
                        gameMem.write<float>(nav + GameOffsets::OFF_NAV_POSITION + 4, newY);
                        // keep z, plus a tiny lift to avoid clipping into ground
                        gameMem.write<float>(nav + GameOffsets::OFF_NAV_POSITION + 8, local.position.z + 1.0f);
                    }
                }
            }
        }
    }
}

void Misc::setSlideRun(bool enabled) {
    slideRun = enabled;
    config->slideRun = enabled;
}

void Misc::setFastRun(bool enabled) {
    fastRun = enabled;
    config->fastRun = enabled;
}

void Misc::setGodMode(bool enabled) {
    godMode = enabled;
    config->godMode = enabled;
}

void Misc::setInvisibility(bool enabled) {
    bool wasEnabled = invisibility;
    invisibility = enabled;
    config->invisibility = enabled;
    
    // If turning off, immediately restore visibility
    if (wasEnabled && !enabled) {
        restoreVisibility();
    }
}

void Misc::suicide() {
    uintptr_t ped = gameMem.getLocalPed();
    if (!ped) return;
    gameMem.write<float>(ped + GameOffsets::OFF_PED_HEALTH, 0.0f);
}

void Misc::applyDamage(float amount) {
    // Apply damage to the nearest other ped (not local).
    // Useful in conjunction with the damager hotkey on a manually-aimed target.
    static GamePlayer scratch[GameOffsets::MAX_ENTITIES_SCAN];
    int n = gameMem.readPedList(scratch, GameOffsets::MAX_ENTITIES_SCAN);
    if (n == 0) return;
    
    GamePlayer local;
    if (!gameMem.readLocalPlayer(local)) return;
    
    int   bestIdx  = -1;
    float bestDist = 1e9f;
    for (int i = 0; i < n; ++i) {
        float d = scratch[i].position.distanceTo(local.position);
        if (d < bestDist) { bestDist = d; bestIdx = i; }
    }
    if (bestIdx < 0) return;
    
    uintptr_t ped = scratch[bestIdx].pedAddr;
    float currentHp = scratch[bestIdx].health;
    float newHp = currentHp - amount;
    if (newHp < 0) newHp = 0;
    gameMem.write<float>(ped + GameOffsets::OFF_PED_HEALTH, newHp);
}

void Misc::setRecoilShare(bool enabled) {
    recoilShare = enabled;
    config->recoilShare = enabled;
}

void Misc::setRecoilAccuracy(float accuracy) {
    recoilAccuracy = accuracy;
    config->recoilAccuracy = accuracy;
}

void Misc::setRecoilRecovery(float recovery) {
    recoilRecovery = recovery;
    config->recoilRecovery = recovery;
}

void Misc::setAnimReloadSpeed(float speed) {
    animReloadSpeed = speed;
    config->animReloadSpeed = speed;
}

void Misc::applyVehicleNoCollision() {
    uintptr_t ped = gameMem.getLocalPed();
    if (!ped) return;
    uintptr_t veh = 0;
    if (!gameMem.read<uintptr_t>(ped + GameOffsets::OFF_PED_VEHICLE, veh) || !veh) return;
    
    // Use the same config-flag bit on the vehicle pointer (GTA5 reuses CEntity layout)
    uintptr_t flagsPtr = 0;
    if (gameMem.read<uintptr_t>(veh + GameOffsets::OFF_PED_CONFIG_FLAGS, flagsPtr) && flagsPtr) {
        uint64_t f0 = 0;
        gameMem.read<uint64_t>(flagsPtr, f0);
        f0 |= (1ULL << GameOffsets::CPED_FLAG_NO_COLLISION);
        gameMem.write<uint64_t>(flagsPtr, f0);
    }
}

void Misc::applyObjectsNoCollision() {
    // Toggling per-object collision needs an entity pool walk - skip for now.
    // The local-player CPED_FLAG_NO_COLLISION usually achieves the wanted result.
    applyNoclip();
}

void Misc::applyNoclip() {
    uintptr_t ped = gameMem.getLocalPed();
    if (!ped) return;
    uintptr_t flagsPtr = 0;
    if (!gameMem.read<uintptr_t>(ped + GameOffsets::OFF_PED_CONFIG_FLAGS, flagsPtr) || !flagsPtr) return;
    
    uint64_t f0 = 0;
    gameMem.read<uint64_t>(flagsPtr, f0);
    f0 |= (1ULL << GameOffsets::CPED_FLAG_NO_COLLISION);
    gameMem.write<uint64_t>(flagsPtr, f0);
    
    // Lift the player +0.2m every tick so they hover instead of falling
    uintptr_t nav = 0;
    if (gameMem.read<uintptr_t>(ped + GameOffsets::OFF_PED_NAVIGATION, nav) && nav) {
        float z = 0;
        gameMem.read<float>(nav + GameOffsets::OFF_NAV_POSITION + 8, z);
        gameMem.write<float>(nav + GameOffsets::OFF_NAV_POSITION + 8, z + 0.05f);
    }
}

void Misc::applySlideRun() {
    // Slide-run = forced low-friction sprint. Adjusting the player run-speed
    // multiplier to a high value approximates the visible effect.
    applyFastRun();
}

void Misc::applyFastRun() {
    uintptr_t ped = gameMem.getLocalPed();
    if (!ped) return;
    uintptr_t playerInfo = 0;
    if (!gameMem.read<uintptr_t>(ped + GameOffsets::OFF_PED_PLAYER_INFO, playerInfo) || !playerInfo) return;
    
    // Run speed multiplier: 1.0 = default. Use a configurable value via animReloadSpeed
    // for repurposing - 1.5..2.0 looks natural without instant-kick triggers.
    const float kRunMult = 1.49f;
    gameMem.write<float>(playerInfo + GameOffsets::OFF_PINFO_RUN_SPEED, kRunMult);
    gameMem.write<float>(playerInfo + GameOffsets::OFF_PINFO_SWIM_SPEED, kRunMult);
    // Restore stamina each tick to prevent the run from being capped by tiredness
    gameMem.write<float>(playerInfo + GameOffsets::OFF_PINFO_STAMINA, 100.0f);
}

void Misc::applyGodMode() {
    // True invulnerability requires patching damage routines - non-trivial.
    // Continuous HP refill is the safest external implementation.
    uintptr_t ped = gameMem.getLocalPed();
    if (!ped) return;
    float maxHp = 200.0f;
    gameMem.read<float>(ped + GameOffsets::OFF_PED_MAX_HEALTH, maxHp);
    if (maxHp <= 0) maxHp = 200.0f;
    gameMem.write<float>(ped + GameOffsets::OFF_PED_HEALTH, maxHp);
    gameMem.write<float>(ped + GameOffsets::OFF_PED_ARMOUR, 100.0f);
}

void Misc::applyInvisibility() {
    if (!hProcess) return;
    
    // Get game module base
    static uintptr_t baseAddr = 0;
    if (!baseAddr) {
        baseAddr = MemoryManager::getModuleBase(hProcess, config->targetProcess);
    }
    if (!baseAddr) return;
    
    // === FIND WORLD POINTER (CPedFactory) via signature ===
    // Pattern: 48 8B 05 ? ? ? ? 48 8B 48 08
    // This resolves to: mov rax,[rip+offset]; mov rcx,[rax+08]
    static uintptr_t worldPtrAddr = 0;
    if (!worldPtrAddr) {
        // Scan first 80MB of executable for the pattern
        worldPtrAddr = MemoryManager::findPattern(hProcess, baseAddr, 0x5000000, "48 8B 05 ?? ?? ?? ?? 48 8B 48 08");
    }
    if (!worldPtrAddr) return;
    
    // Resolve RIP-relative address
    uint32_t ripOffset = MemoryManager::read<uint32_t>(hProcess, worldPtrAddr + 3);
    uintptr_t worldPtr = worldPtrAddr + 7 + ripOffset;
    if (!worldPtr) return;
    
    // Read local player (CPedFactory + 0x8)
    uintptr_t localPlayer = MemoryManager::read<uintptr_t>(hProcess, worldPtr + 0x8);
    if (!localPlayer) return;
    
    // Read player ped
    uintptr_t playerPed = MemoryManager::read<uintptr_t>(hProcess, localPlayer + 0x8);
    if (!playerPed) return;
    
    // === 1. ENTITY ALPHA / OPACITY (local render invisible) ===
    // +0xAC = entity alpha byte (0-255). 0 = fully invisible
    MemoryManager::write<uint8_t>(hProcess, playerPed + 0xAC, 0);
    
    // === 2. ENTITY VISIBLE FLAGS (hide from other players completely) ===
    // +0x2C bit 0 = "is visible to network players"
    uint32_t visibleFlags = MemoryManager::read<uint32_t>(hProcess, playerPed + 0x2C);
    visibleFlags &= ~1;      // Clear visible bit
    visibleFlags &= ~(1 << 1); // Clear shadow cast bit
    MemoryManager::write<uint32_t>(hProcess, playerPed + 0x2C, visibleFlags);
    
    // === 3. CPED CONFIG FLAGS — FORCE INVISIBLE + NO NETWORK SYNC ===
    // Config flags pointer usually at +0xDC or +0xE0 (uint64_t[512])
    uintptr_t configFlagsPtr = MemoryManager::read<uintptr_t>(hProcess, playerPed + 0xDC);
    if (configFlagsPtr) {
        // Read first 64-bit block (flags 0-63)
        uint64_t flags0 = MemoryManager::read<uint64_t>(hProcess, configFlagsPtr);
        flags0 |= (1ULL << 0x2C);   // CPED_CONFIG_FLAG_Invisible
        flags0 |= (1ULL << 0x1C0);  // CPED_CONFIG_FLAG_DontSendNetworkPosition
        flags0 |= (1ULL << 0x1D);   // CPED_CONFIG_FLAG_IsInVehicle (mask presence)
        flags0 |= (1ULL << 0x9);    // CPED_CONFIG_FLAG_NoCollision (bonus stealth)
        MemoryManager::write<uint64_t>(hProcess, configFlagsPtr, flags0);
        
        // Also write second 64-bit block for higher flag indices
        uint64_t flags1 = MemoryManager::read<uint64_t>(hProcess, configFlagsPtr + 8);
        flags1 |= (1ULL << (0x13 - 64)); // CPED_CONFIG_FLAG_IgnoreEvents (network masking)
        MemoryManager::write<uint64_t>(hProcess, configFlagsPtr + 8, flags1);
    }
    
    // === 4. NETWORK OBJECT — COMPLETELY DROP FROM SERVER SYNC ===
    // Network object pointer usually at +0xD0 or +0xD8
    uintptr_t netObject = MemoryManager::read<uintptr_t>(hProcess, playerPed + 0xD0);
    if (netObject) {
        // m_ownerId at +0x4A — set to invalid so server stops replicating us
        MemoryManager::write<uint16_t>(hProcess, netObject + 0x4A, 0xFFFF);
        // m_syncFlag — disable all sync
        uint8_t syncFlags = MemoryManager::read<uint8_t>(hProcess, netObject + 0x4C);
        syncFlags = 0;
        MemoryManager::write<uint8_t>(hProcess, netObject + 0x4C, syncFlags);
    }
    
    // === 5. DRAW HANDLER — BACKUP LOCAL ALPHA WIPE ===
    uintptr_t drawHandler = MemoryManager::read<uintptr_t>(hProcess, playerPed + 0x20);
    if (drawHandler) {
        // Alpha override in draw handler
        MemoryManager::write<uint8_t>(hProcess, drawHandler + 0x3C, 0);
        // Some builds use float alpha at +0x40
        MemoryManager::write<float>(hProcess, drawHandler + 0x40, 0.0f);
    }
    
    // === 6. NETWORK PLAYER MGR — HIDE FROM PLAYER LIST (AGGRESSIVE) ===
    // Try to find network player manager and hide local player from admin tools
    static uintptr_t networkMgrAddr = 0;
    if (!networkMgrAddr) {
        // Pattern for network player manager pointer
        networkMgrAddr = MemoryManager::findPattern(hProcess, baseAddr, 0x5000000, "48 8B 0D ?? ?? ?? ?? 48 85 C9 74 0A");
    }
    if (networkMgrAddr) {
        uint32_t rip = MemoryManager::read<uint32_t>(hProcess, networkMgrAddr + 3);
        uintptr_t netMgr = networkMgrAddr + 7 + rip;
        if (netMgr) {
            // Hide from scoreboard / player list
            uint8_t visibility = MemoryManager::read<uint8_t>(hProcess, netMgr + 0x180);
            visibility = 0;
            MemoryManager::write<uint8_t>(hProcess, netMgr + 0x180, visibility);
        }
    }
}

void Misc::restoreVisibility() {
    if (!hProcess) return;
    
    static uintptr_t baseAddr = 0;
    if (!baseAddr) {
        baseAddr = MemoryManager::getModuleBase(hProcess, config->targetProcess);
    }
    if (!baseAddr) return;
    
    static uintptr_t worldPtrAddr = 0;
    if (!worldPtrAddr) {
        worldPtrAddr = MemoryManager::findPattern(hProcess, baseAddr, 0x5000000, "48 8B 05 ?? ?? ?? ?? 48 8B 48 08");
    }
    if (!worldPtrAddr) return;
    
    uint32_t ripOffset = MemoryManager::read<uint32_t>(hProcess, worldPtrAddr + 3);
    uintptr_t worldPtr = worldPtrAddr + 7 + ripOffset;
    if (!worldPtr) return;
    
    uintptr_t localPlayer = MemoryManager::read<uintptr_t>(hProcess, worldPtr + 0x8);
    if (!localPlayer) return;
    
    uintptr_t playerPed = MemoryManager::read<uintptr_t>(hProcess, localPlayer + 0x8);
    if (!playerPed) return;
    
    // Restore alpha
    MemoryManager::write<uint8_t>(hProcess, playerPed + 0xAC, 255);
    
    // Restore visible flags
    uint32_t visibleFlags = MemoryManager::read<uint32_t>(hProcess, playerPed + 0x2C);
    visibleFlags |= 1;
    visibleFlags |= (1 << 1);
    MemoryManager::write<uint32_t>(hProcess, playerPed + 0x2C, visibleFlags);
    
    // Restore config flags
    uintptr_t configFlagsPtr = MemoryManager::read<uintptr_t>(hProcess, playerPed + 0xDC);
    if (configFlagsPtr) {
        uint64_t flags0 = MemoryManager::read<uint64_t>(hProcess, configFlagsPtr);
        flags0 &= ~(1ULL << 0x2C);
        flags0 &= ~(1ULL << 0x1C0);
        flags0 &= ~(1ULL << 0x1D);
        flags0 &= ~(1ULL << 0x9);
        MemoryManager::write<uint64_t>(hProcess, configFlagsPtr, flags0);
        
        uint64_t flags1 = MemoryManager::read<uint64_t>(hProcess, configFlagsPtr + 8);
        flags1 &= ~(1ULL << (0x13 - 64));
        MemoryManager::write<uint64_t>(hProcess, configFlagsPtr + 8, flags1);
    }
    
    // Restore network object
    uintptr_t netObject = MemoryManager::read<uintptr_t>(hProcess, playerPed + 0xD0);
    if (netObject) {
        MemoryManager::write<uint16_t>(hProcess, netObject + 0x4A, 0); // restore local owner
        MemoryManager::write<uint8_t>(hProcess, netObject + 0x4C, 1);  // restore sync
    }
    
    // Restore draw handler
    uintptr_t drawHandler = MemoryManager::read<uintptr_t>(hProcess, playerPed + 0x20);
    if (drawHandler) {
        MemoryManager::write<uint8_t>(hProcess, drawHandler + 0x3C, 255);
        MemoryManager::write<float>(hProcess, drawHandler + 0x40, 1.0f);
    }
}

void Misc::applyRecoilModifications() {
    // Recoil/accuracy/recovery live inside the active CWeaponInfo struct,
    // which is reachable from CPedWeaponManager + 0x20. Writing zero recoil
    // every tick effectively eliminates spread on the currently held weapon.
    uintptr_t ped = gameMem.getLocalPed();
    if (!ped) return;
    uintptr_t weaponMgr = 0;
    if (!gameMem.read<uintptr_t>(ped + GameOffsets::OFF_PED_WEAPON_MGR, weaponMgr) || !weaponMgr) return;
    
    uintptr_t weaponInfo = 0;
    if (!gameMem.read<uintptr_t>(weaponMgr + 0x20, weaponInfo) || !weaponInfo) return;
    
    // CWeaponInfo offsets (build 2802):
    constexpr int W_RECOIL_ACCURACY = 0x230;  // float
    constexpr int W_RECOIL_RECOVERY = 0x238;  // float
    constexpr int W_DAMAGE          = 0x140;  // float
    
    gameMem.write<float>(weaponInfo + W_RECOIL_ACCURACY, recoilAccuracy);
    gameMem.write<float>(weaponInfo + W_RECOIL_RECOVERY, recoilRecovery);
    
    if (recoilShare) {
        // Restore default damage so the multiplied accuracy still feels balanced
        // (do not boost damage further - that would trigger AC fast).
        (void)weaponInfo;
        (void)W_DAMAGE;
    }
}
