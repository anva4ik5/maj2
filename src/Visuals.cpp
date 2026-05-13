#include "Visuals.h"
#include <cmath>

Visuals::Visuals(DirectXOverlay* ov)
    : overlay(ov), hProcess(nullptr), enabled(false) {
}

Visuals::~Visuals() {
}

void Visuals::initialize(HANDLE process) {
    hProcess = process;
    config = &ConfigManager::getInstance().getConfig();
    
    // Load config values
    enabled = config->enableESP;
    skeleton = config->skeleton;
    distance = config->distance;
    snapline = config->snapline;
    name = config->name;
    weapons = config->weapons;
    box = config->box;
    staticID = config->staticID;
    familyID = config->familyID;
    fraction = config->fraction;
    hpBar = config->hpBar;
    ammoBar = config->ammoBar;
    anim = config->anim;
    radar = config->radar;
    admin = config->admin;
    localplayer = config->localplayer;
    objects = config->objects;
    vehicles = config->vehicles;
}

void Visuals::render() {
    if (!overlay || !overlay->isInitialized() || !enabled) return;
    
    overlay->beginFrame();
    
    // Update entities
    updateEntities();
    
    // Render each entity
    for (auto& entity : entities) {
        if (skeleton) renderSkeleton(&entity);
        if (distance) renderDistance(&entity);
        if (snapline) renderSnapline(&entity);
        if (name) renderName(&entity);
        if (weapons) renderWeapons(&entity);
        if (box) renderBox(&entity);
        if (staticID) renderStaticID(&entity);
        if (familyID) renderFamilyID(&entity);
        if (fraction) renderFraction(&entity);
        if (hpBar) renderHPBar(&entity);
        if (ammoBar) renderAmmoBar(&entity);
        if (anim) renderAnim(&entity);
        if (admin) renderAdmin(&entity);
    }
    
    if (localplayer) renderLocalPlayer();
    if (objects) renderObjects();
    if (vehicles) renderVehicles();
    if (radar) renderRadar();
    
    overlay->endFrame();
}

void Visuals::updateEntities() {
    entities.clear();
    
    // TODO: Read entities from game memory
    // Similar to Aimbot::updateEntities()
}

void Visuals::renderSkeleton(Entity* entity) {
    // TODO: Render skeleton lines
    // Requires bone positions from game memory
    // overlay->drawLine(headPos, neckPos, color);
    // overlay->drawLine(neckPos, chestPos, color);
    // etc.
}

void Visuals::renderDistance(Entity* entity) {
    Vec2 screenPos = worldToScreen(entity->position);
    if (screenPos.x < 0 || screenPos.y < 0) return;
    
    float dist = entity->position.distanceTo(Vector3()); // Should be local player position
    std::string distText = std::to_string((int)dist) + "m";
    
    overlay->drawText(distText, {screenPos.x, screenPos.y - 20}, {1, 1, 1, 1}, 14.0f);
}

void Visuals::renderSnapline(Entity* entity) {
    Vec2 screenPos = worldToScreen(entity->position);
    if (screenPos.x < 0 || screenPos.y < 0) return;
    
    Vec2 screenCenter = {static_cast<float>(overlay->getWidth()) / 2.0f, static_cast<float>(overlay->getHeight())};
    overlay->drawLine(screenCenter, screenPos, {1, 0, 0, 0.5f}, 1.0f);
}

void Visuals::renderName(Entity* entity) {
    Vec2 screenPos = worldToScreen(entity->headPosition);
    if (screenPos.x < 0 || screenPos.y < 0) return;
    
    overlay->drawText(entity->name, {screenPos.x, screenPos.y - 35}, {1, 1, 1, 1}, 16.0f);
}

void Visuals::renderWeapons(Entity* entity) {
    Vec2 screenPos = worldToScreen(entity->position);
    if (screenPos.x < 0 || screenPos.y < 0) return;
    
    overlay->drawText(entity->weapon, {screenPos.x, screenPos.y + 20}, {0.5, 0.5, 1, 1}, 12.0f);
}

void Visuals::renderBox(Entity* entity) {
    Vec2 screenPos = worldToScreen(entity->position);
    if (screenPos.x < 0 || screenPos.y < 0) return;
    
    Vec2 boxSize = {50, 100}; // Should be calculated based on distance
    Vec2 boxPos = {screenPos.x - boxSize.x / 2, screenPos.y - boxSize.y};
    
    overlay->drawBox(boxPos, boxSize, {1, 1, 1, 0.8f}, 1.5f);
}

void Visuals::renderStaticID(Entity* entity) {
    // TODO: Render static ID
}

void Visuals::renderFamilyID(Entity* entity) {
    // TODO: Render family ID
}

void Visuals::renderFraction(Entity* entity) {
    // TODO: Render fraction/gang info
}

void Visuals::renderHPBar(Entity* entity) {
    Vec2 screenPos = worldToScreen(entity->headPosition);
    if (screenPos.x < 0 || screenPos.y < 0) return;
    
    float hpPercent = (float)entity->health / 100.0f; // Assuming max HP is 100
    Vec2 barSize = {50, 5};
    Vec2 barPos = {screenPos.x - barSize.x / 2, screenPos.y - 45};
    
    // Background
    overlay->drawFilledBox(barPos, barSize, {0, 0, 0, 0.5f});
    
    // Health
    Vec2 healthSize = {barSize.x * hpPercent, barSize.y};
    Color healthColor = getHealthColor(entity->health, 100);
    overlay->drawFilledBox(barPos, healthSize, healthColor);
}

void Visuals::renderAmmoBar(Entity* entity) {
    // TODO: Render ammo bar
}

void Visuals::renderAnim(Entity* entity) {
    // TODO: Render animation info
}

void Visuals::renderRadar() {
    // TODO: Render 2D radar
    // Show entities relative to local player
}

void Visuals::renderAdmin(Entity* entity) {
    // TODO: Render admin indicator
}

void Visuals::renderLocalPlayer() {
    // TODO: Render local player info
}

void Visuals::renderObjects() {
    // TODO: Render objects
}

void Visuals::renderVehicles() {
    // TODO: Render vehicles
}

Vec2 Visuals::worldToScreen(Vector3 worldPos) {
    // TODO: Implement world-to-screen transformation
    // Requires view matrix and projection matrix from game
    return Vec2();
}

Color Visuals::getTeamColor(int team) {
    // Return color based on team
    switch (team) {
        case 0: return Color(1, 0, 0, 1); // Red
        case 1: return Color(0, 1, 0, 1); // Green
        case 2: return Color(0, 0, 1, 1); // Blue
        default: return Color(1, 1, 1, 1); // White
    }
}

Color Visuals::getHealthColor(int health, int maxHealth) {
    float percent = (float)health / maxHealth;
    if (percent > 0.5f) return Color(0, 1, 0, 1);
    if (percent > 0.25f) return Color(1, 1, 0, 1);
    return Color(1, 0, 0, 1);
}
