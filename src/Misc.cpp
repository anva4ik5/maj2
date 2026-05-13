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
    // TODO: Reset player HP to max
    // MemoryManager::write<int>(hProcess, PLAYER_HP_OFFSET, MAX_HP);
}

void Misc::resetArmour() {
    // TODO: Reset player armour to max
    // MemoryManager::write<int>(hProcess, PLAYER_ARMOUR_OFFSET, MAX_ARMOUR);
}

void Misc::teleportToWaypoint() {
    // TODO: Teleport player to waypoint
    // Vector3 waypointPos = readWaypointPosition();
    // MemoryManager::write<Vector3>(hProcess, PLAYER_POSITION_OFFSET, waypointPos);
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
    // TODO: Kill player
    // MemoryManager::write<int>(hProcess, PLAYER_HP_OFFSET, 0);
}

void Misc::applyDamage(float amount) {
    // TODO: Apply damage to target entity
    // MemoryManager::write<int>(hProcess, TARGET_HP_OFFSET, currentHP - amount);
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
    // TODO: Disable collision for vehicles
    // This would involve patching game functions or modifying vehicle collision flags
}

void Misc::applyObjectsNoCollision() {
    // TODO: Disable collision for objects
}

void Misc::applyNoclip() {
    // TODO: Enable noclip mode
    // This would involve disabling collision checks for local player
}

void Misc::applySlideRun() {
    // TODO: Enable slide running
    // Modify movement speed and friction
}

void Misc::applyFastRun() {
    // TODO: Enable fast running
    // Modify movement speed multiplier
}

void Misc::applyGodMode() {
    // TODO: Enable god mode
    // Prevent damage by patching damage functions or setting HP to max continuously
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
    // TODO: Apply recoil modifications
    // Modify recoil pattern, accuracy, and recovery speed
}
