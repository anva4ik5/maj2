#pragma once

#include <windows.h>
#include <string>
#include <cstdint>
#include <cmath>
#include "GameOffsets.h"

struct GameVec3 {
    float x = 0, y = 0, z = 0;
    GameVec3() = default;
    GameVec3(float x_, float y_, float z_) : x(x_), y(y_), z(z_) {}
    float distanceTo(const GameVec3& o) const {
        float dx = x - o.x, dy = y - o.y, dz = z - o.z;
        return std::sqrt(dx*dx + dy*dy + dz*dz);
    }
};

// Player snapshot used by Aimbot/Visuals
struct GamePlayer {
    uintptr_t pedAddr  = 0;
    GameVec3  position;
    GameVec3  headPosition;
    float     health   = 0;
    float     maxHealth= 0;
    float     armour   = 0;
    bool      isLocal  = false;
    bool      inVehicle= false;
    std::string name;
};

class GameMemory {
public:
    // Initialise with the open process handle and game module name (e.g. "GTA5.exe")
    bool initialize(HANDLE process, const std::string& moduleName);

    // Re-resolve patterns. Call once after init or after game restart.
    bool resolvePointers();

    // Accessors -----------------------------------------------------------
    uintptr_t worldPtr()      const { return cachedWorldPtr; }
    uintptr_t viewportPtr()   const { return cachedViewportPtr; }
    uintptr_t replayIfacePtr()const { return cachedReplayPtr; }

    // Local player ----------------------------------------------------------
    uintptr_t getLocalPed();
    bool      readLocalPlayer(GamePlayer& out);

    // Entities --------------------------------------------------------------
    int       readPedList(GamePlayer* outArr, int maxCount);

    // World->Screen ---------------------------------------------------------
    bool      worldToScreen(const GameVec3& world, float& sx, float& sy);
    int       screenWidth()  const { return cachedScreenW; }
    int       screenHeight() const { return cachedScreenH; }

    // Helpers (raw memory) --------------------------------------------------
    template<typename T>
    bool read(uintptr_t addr, T& out) const {
        SIZE_T n = 0;
        return ReadProcessMemory(hProcess, (LPCVOID)addr, &out, sizeof(T), &n) && n == sizeof(T);
    }
    template<typename T>
    bool write(uintptr_t addr, const T& value) {
        SIZE_T n = 0;
        return WriteProcessMemory(hProcess, (LPVOID)addr, &value, sizeof(T), &n) && n == sizeof(T);
    }

    HANDLE getProcessHandle() const { return hProcess; }

private:
    HANDLE hProcess = nullptr;
    std::string moduleName;
    uintptr_t moduleBase = 0;
    size_t    moduleSize = 0;

    uintptr_t cachedWorldPtr     = 0;
    uintptr_t cachedViewportPtr  = 0;
    uintptr_t cachedReplayPtr    = 0;
    uintptr_t cachedNetworkMgr   = 0;
    int       cachedScreenW      = 1920;
    int       cachedScreenH      = 1080;

    // Resolve a RIP-relative pointer (pattern + 7 bytes typical)
    uintptr_t resolveRipRelative(const char* pattern, int dispOffset = 3, int instructionLen = 7);
    bool      readPedSnapshot(uintptr_t pedAddr, GamePlayer& out, bool isLocal = false);
};
