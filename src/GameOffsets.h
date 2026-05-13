#pragma once

// =============================================================================
// GTA5 / RAGE engine offsets (used by Majestic RP, RAGE:MP, alt:V)
// Reference build: GTA5 1.0.2802 (October 2022, last stable for RAGE:MP/Majestic)
// alt:V uses the same GTA5 process so most offsets are identical.
//
// All offsets are auto-recoverable via the patterns below; if a pattern fails
// to scan, the hard-coded offset is used as a last-resort fallback.
// =============================================================================

namespace GameOffsets {

// ===== Patterns (signature scanning) =====
// CPedFactory: mov rax,[rip+disp]; mov rcx,[rax+8]
constexpr const char* PAT_WORLD_PTR     = "48 8B 05 ?? ?? ?? ?? 48 8B 48 08";
// Camera/Viewport pointer
constexpr const char* PAT_VIEWPORT_PTR  = "48 8B 05 ?? ?? ?? ?? 48 8D 95";
// CReplayInterfaceManager (entity pools)
constexpr const char* PAT_REPLAY_IFACE  = "48 8D 0D ?? ?? ?? ?? 48 8B D7 E8 ?? ?? ?? ?? 48 8D 8B";
// Network player manager
constexpr const char* PAT_NETWORK_MGR   = "48 8B 0D ?? ?? ?? ?? 48 85 C9 74 0A";
// Globals (CGlobalIndices) for build 2802
constexpr const char* PAT_GAME_STATE    = "83 3D ?? ?? ?? ?? ?? 75 17 8B 43 20";

// ===== CPedFactory / World structure =====
constexpr int OFF_WORLD_TO_LOCAL_PLAYER = 0x08;  // CPedFactory + 0x8 -> CPed* localPlayer

// ===== CPed (player ped) offsets =====
constexpr int OFF_PED_NAVIGATION        = 0x30;   // ptr CNavigation
constexpr int OFF_PED_VISIBLE_FLAGS     = 0x2C;   // u32 visibility/render flags
constexpr int OFF_PED_DRAW_HANDLER      = 0x48;   // CDrawHandler*
constexpr int OFF_PED_ALPHA             = 0xAC;   // u8 alpha (0..255)
constexpr int OFF_PED_HEALTH            = 0x280;  // float
constexpr int OFF_PED_MAX_HEALTH        = 0x2A0;  // float
constexpr int OFF_PED_ARMOUR            = 0x150C; // float (build 2802)
constexpr int OFF_PED_VEHICLE           = 0xD30;  // CVehicle* (when in vehicle)
constexpr int OFF_PED_NETWORK_OBJECT    = 0xD0;   // CNetObjPed*
constexpr int OFF_PED_CONFIG_FLAGS      = 0xDC;   // ptr to u64 array (config flags)
constexpr int OFF_PED_WEAPON_MGR        = 0x10B8; // CPedWeaponManager*
constexpr int OFF_PED_BONE_MATRIX       = 0x60;   // float[3] world position fallback
constexpr int OFF_PED_PLAYER_INFO       = 0x10A8; // CPlayerInfo*
constexpr int OFF_PED_NAME_HASH         = 0x18;   // u32 (model hash)

// CNavigation -> position (Vector3)
constexpr int OFF_NAV_POSITION          = 0x50;   // Vector3 fvec3 (current world pos)

// CPed bones (head/neck for aimbot)
constexpr int OFF_PED_BONE_PTR          = 0x410;  // ptr to bone matrix array
constexpr int BONE_HEAD                 = 12;     // bone index for head (skel slot)
constexpr int BONE_NECK                 = 9;
constexpr int BONE_SPINE                = 6;

// ===== CPlayerInfo =====
constexpr int OFF_PINFO_NAME            = 0x7C;   // char[32] player name
constexpr int OFF_PINFO_RUN_SPEED       = 0xCC8;  // float run multiplier (build 2802)
constexpr int OFF_PINFO_SWIM_SPEED      = 0xCCC;  // float swim multiplier
constexpr int OFF_PINFO_STAMINA         = 0xCD0;  // float stamina

// ===== CVehicle (when ped in vehicle) =====
constexpr int OFF_VEH_HEALTH            = 0x840;
constexpr int OFF_VEH_ENGINE_HEALTH     = 0x8C8;
constexpr int OFF_VEH_NAVIGATION        = 0x30;
constexpr int OFF_VEH_GRAVITY           = 0xC5C;

// ===== Camera / Viewport =====
constexpr int OFF_VIEWPORT_VIEW_MATRIX  = 0x40;   // float[16] world->view
constexpr int OFF_VIEWPORT_FOV          = 0x150;  // float
constexpr int OFF_VIEWPORT_SCREEN_W     = 0x180;  // int
constexpr int OFF_VIEWPORT_SCREEN_H     = 0x184;  // int
constexpr int OFF_VIEWPORT_CAMERA_POS   = 0x200;  // Vector3

// ===== CPed config flag indices (used for invisibility / godmode hints) =====
constexpr int CPED_FLAG_INVISIBLE       = 0x2C;
constexpr int CPED_FLAG_NO_COLLISION    = 0x09;
constexpr int CPED_FLAG_DONT_NETWORK    = 0x1C0;
constexpr int CPED_FLAG_GOD             = 0x1F4;  // not god-mode itself, but cooperates
constexpr int CPED_FLAG_NO_RAGDOLL      = 0x4F;

// ===== Replay Interface (entity pools) =====
constexpr int OFF_REPLAY_PED_INTERFACE  = 0x18;   // CReplayInterfacePed*
constexpr int OFF_REPLAY_VEH_INTERFACE  = 0x10;   // CReplayInterfaceVehicle*
constexpr int OFF_PED_INTERFACE_LIST    = 0x100;  // ptr to CPed*[] array
constexpr int OFF_PED_INTERFACE_COUNT   = 0x110;  // u32 count
constexpr int OFF_VEH_INTERFACE_LIST    = 0x100;
constexpr int OFF_VEH_INTERFACE_COUNT   = 0x110;

// ===== Limits =====
constexpr int MAX_ENTITIES_SCAN         = 256;    // safe upper bound

} // namespace GameOffsets
