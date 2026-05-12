#include "AdminDetector.h"
#include <windows.h>
#include <iostream>

AdminDetector::AdminDetector() 
    : initialized(false), detectionRadius(100.0f), spectatorCheckInterval(5), autoDisableOnAdmin(false) {
}

AdminDetector::~AdminDetector() {
}

void AdminDetector::initialize() {
    initialized = true;
    std::cout << "AdminDetector initialized" << std::endl;
}

void AdminDetector::update() {
    if (!initialized) return;
    
    static DWORD64 lastCheck = 0;
    DWORD64 currentTime = GetTickCount64();
    
    if (currentTime - lastCheck >= static_cast<DWORD64>(spectatorCheckInterval) * 1000) {
        lastCheck = currentTime;
        
        if (isAdminNearby()) {
            if (warningCallback) {
                warningCallback();
            }
            
            if (autoDisableOnAdmin) {
                std::cout << "Админ обнаружен! Отключение функций чита..." << std::endl;
            }
        }
    }
}

bool AdminDetector::isAdminNearby() {
    return checkForAdminInRadius();
}

bool AdminDetector::isAdminWatching() {
    return checkForAdminSpectator();
}

bool AdminDetector::isSpectating() {
    return checkForAdminSpectator();
}

std::vector<AdminInfo> AdminDetector::getNearbyAdmins() {
    std::vector<AdminInfo> nearby;
    scanForAdmins();
    
    for (const auto& admin : knownAdmins) {
        if (admin.distance <= detectionRadius) {
            nearby.push_back(admin);
        }
    }
    
    return nearby;
}

std::vector<AdminInfo> AdminDetector::getAllAdmins() {
    return knownAdmins;
}

void AdminDetector::setWarningCallback(std::function<void()> callback) {
    warningCallback = callback;
}

void AdminDetector::setAutoDisableOnAdmin(bool enable) {
    autoDisableOnAdmin = enable;
}

void AdminDetector::addKnownAdmin(int id, const std::string& name) {
    AdminInfo info;
    info.id = id;
    info.name = name;
    info.isAdmin = true;
    info.isModerator = false;
    info.isDeveloper = false;
    info.distance = 0.0f;
    
    knownAdmins.push_back(info);
}

void AdminDetector::removeKnownAdmin(int id) {
    knownAdmins.erase(
        std::remove_if(knownAdmins.begin(), knownAdmins.end(),
            [id](const AdminInfo& info) { return info.id == id; }),
        knownAdmins.end()
    );
}

void AdminDetector::clearKnownAdmins() {
    knownAdmins.clear();
}

void AdminDetector::setDetectionRadius(float radius) {
    detectionRadius = radius;
}

void AdminDetector::setSpectatorCheckInterval(int seconds) {
    spectatorCheckInterval = seconds;
}

bool AdminDetector::checkForAdminSpectator() {
    return monitorAdminPackets();
}

bool AdminDetector::checkForAdminInRadius() {
    scanForAdmins();
    
    for (const auto& admin : knownAdmins) {
        if (admin.distance <= detectionRadius) {
            return true;
        }
    }
    
    return false;
}

bool AdminDetector::checkForAdminCommands() {
    return false;
}

void AdminDetector::scanForAdmins() {
    // Placeholder for admin scanning
}

bool AdminDetector::isAdminEntity(int entityId) {
    return false;
}

bool AdminDetector::monitorAdminPackets() {
    return detectAdminTeleport();
}

bool AdminDetector::detectAdminTeleport() {
    return false;
}
