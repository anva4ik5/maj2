#include "GameMemory.h"
#include "MemoryManager.h"
#include <iostream>
#include <psapi.h>

bool GameMemory::initialize(HANDLE process, const std::string& mod) {
    if (!process) return false;
    hProcess   = process;
    moduleName = mod;

    // Find module base + size
    HMODULE mods[1024];
    DWORD needed = 0;
    if (!EnumProcessModules(hProcess, mods, sizeof(mods), &needed)) {
        std::cerr << "[GameMemory] EnumProcessModules failed" << std::endl;
        return false;
    }

    int count = (int)(needed / sizeof(HMODULE));
    for (int i = 0; i < count; ++i) {
        char name[MAX_PATH];
        if (!GetModuleFileNameExA(hProcess, mods[i], name, sizeof(name))) continue;
        std::string sname = name;
        if (sname.find(moduleName) != std::string::npos) {
            MODULEINFO info{};
            if (GetModuleInformation(hProcess, mods[i], &info, sizeof(info))) {
                moduleBase = (uintptr_t)info.lpBaseOfDll;
                moduleSize = info.SizeOfImage;
            }
            break;
        }
    }

    if (!moduleBase) {
        std::cerr << "[GameMemory] Module '" << moduleName << "' not found" << std::endl;
        return false;
    }

    return resolvePointers();
}

uintptr_t GameMemory::resolveRipRelative(const char* pattern, int dispOffset, int instructionLen) {
    if (!moduleBase) return 0;
    uintptr_t patAddr = MemoryManager::findPattern(hProcess, moduleBase, moduleSize, pattern);
    if (!patAddr) return 0;

    int32_t disp = 0;
    SIZE_T n = 0;
    if (!ReadProcessMemory(hProcess, (LPCVOID)(patAddr + dispOffset), &disp, sizeof(disp), &n) || n != 4)
        return 0;

    return patAddr + instructionLen + disp;
}

bool GameMemory::resolvePointers() {
    using namespace GameOffsets;

    cachedWorldPtr    = resolveRipRelative(PAT_WORLD_PTR, 3, 7);
    cachedViewportPtr = resolveRipRelative(PAT_VIEWPORT_PTR, 3, 7);
    cachedReplayPtr   = resolveRipRelative(PAT_REPLAY_IFACE, 3, 7);
    cachedNetworkMgr  = resolveRipRelative(PAT_NETWORK_MGR, 3, 7);

    if (!cachedWorldPtr) {
        std::cerr << "[GameMemory] WARNING: world pattern not found - patterns may have changed" << std::endl;
    }

    // Read screen dimensions from viewport if available
    if (cachedViewportPtr) {
        uintptr_t vp = 0;
        if (read<uintptr_t>(cachedViewportPtr, vp) && vp) {
            int w = 0, h = 0;
            if (read<int>(vp + OFF_VIEWPORT_SCREEN_W, w) && w > 0)  cachedScreenW = w;
            if (read<int>(vp + OFF_VIEWPORT_SCREEN_H, h) && h > 0) cachedScreenH = h;
        }
    }

    return cachedWorldPtr != 0;
}

uintptr_t GameMemory::getLocalPed() {
    if (!cachedWorldPtr) return 0;
    uintptr_t worldStruct = 0;
    if (!read<uintptr_t>(cachedWorldPtr, worldStruct) || !worldStruct) return 0;

    uintptr_t ped = 0;
    if (!read<uintptr_t>(worldStruct + GameOffsets::OFF_WORLD_TO_LOCAL_PLAYER, ped)) return 0;
    return ped;
}

bool GameMemory::readPedSnapshot(uintptr_t pedAddr, GamePlayer& out, bool isLocal) {
    using namespace GameOffsets;
    if (!pedAddr) return false;
    out.pedAddr = pedAddr;
    out.isLocal = isLocal;

    // Position via CNavigation
    uintptr_t nav = 0;
    if (read<uintptr_t>(pedAddr + OFF_PED_NAVIGATION, nav) && nav) {
        read<float>(nav + OFF_NAV_POSITION + 0,  out.position.x);
        read<float>(nav + OFF_NAV_POSITION + 4,  out.position.y);
        read<float>(nav + OFF_NAV_POSITION + 8,  out.position.z);
    }

    // Head position via bone matrix (fallback to position + 0.65 z)
    out.headPosition = out.position;
    out.headPosition.z += 0.65f;

    read<float>(pedAddr + OFF_PED_HEALTH,     out.health);
    read<float>(pedAddr + OFF_PED_MAX_HEALTH, out.maxHealth);
    read<float>(pedAddr + OFF_PED_ARMOUR,     out.armour);

    uintptr_t veh = 0;
    if (read<uintptr_t>(pedAddr + OFF_PED_VEHICLE, veh) && veh) {
        out.inVehicle = true;
    }

    return true;
}

bool GameMemory::readLocalPlayer(GamePlayer& out) {
    uintptr_t ped = getLocalPed();
    if (!ped) return false;
    return readPedSnapshot(ped, out, /*isLocal=*/true);
}

int GameMemory::readPedList(GamePlayer* outArr, int maxCount) {
    using namespace GameOffsets;
    if (!outArr || maxCount <= 0 || !cachedReplayPtr) return 0;

    uintptr_t replayBase = 0;
    if (!read<uintptr_t>(cachedReplayPtr, replayBase) || !replayBase) return 0;

    uintptr_t pedIface = 0;
    if (!read<uintptr_t>(replayBase + OFF_REPLAY_PED_INTERFACE, pedIface) || !pedIface) return 0;

    uintptr_t pedListPtr = 0;
    int       pedCount   = 0;
    read<uintptr_t>(pedIface + OFF_PED_INTERFACE_LIST,  pedListPtr);
    read<int>(pedIface       + OFF_PED_INTERFACE_COUNT, pedCount);

    if (!pedListPtr || pedCount <= 0) return 0;
    if (pedCount > MAX_ENTITIES_SCAN) pedCount = MAX_ENTITIES_SCAN;

    uintptr_t localPed = getLocalPed();

    int written = 0;
    for (int i = 0; i < pedCount && written < maxCount; ++i) {
        uintptr_t ped = 0;
        if (!read<uintptr_t>(pedListPtr + i * 0x10, ped) || !ped) continue;
        if (ped == localPed) continue;

        GamePlayer p;
        if (readPedSnapshot(ped, p, false)) {
            if (p.health > 0.0f) {
                outArr[written++] = p;
            }
        }
    }
    return written;
}

bool GameMemory::worldToScreen(const GameVec3& world, float& sx, float& sy) {
    using namespace GameOffsets;
    if (!cachedViewportPtr) return false;
    uintptr_t vp = 0;
    if (!read<uintptr_t>(cachedViewportPtr, vp) || !vp) return false;

    float m[16] = {0};
    SIZE_T n = 0;
    if (!ReadProcessMemory(hProcess, (LPCVOID)(vp + OFF_VIEWPORT_VIEW_MATRIX),
                           m, sizeof(m), &n) || n != sizeof(m)) {
        return false;
    }

    // Standard column-major transform: clip = view * world(homogeneous)
    float clipX = m[0]*world.x + m[4]*world.y + m[8]*world.z  + m[12];
    float clipY = m[1]*world.x + m[5]*world.y + m[9]*world.z  + m[13];
    float clipW = m[3]*world.x + m[7]*world.y + m[11]*world.z + m[15];

    if (clipW < 0.001f) return false; // behind camera

    float ndcX = clipX / clipW;
    float ndcY = clipY / clipW;

    sx = (ndcX * 0.5f + 0.5f) * (float)cachedScreenW;
    sy = (1.0f - (ndcY * 0.5f + 0.5f)) * (float)cachedScreenH;
    return true;
}
