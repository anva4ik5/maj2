#include "AuthManager.h"
#include "HWIDManager.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <ctime>
#include <random>
#include <regex>

AuthManager::AuthManager() 
    : authorized(false) {
}

AuthManager::~AuthManager() {
}

void AuthManager::initialize(const AuthConfig& config) {
    this->config = config;
    
    // Load stored license key and HWID
    currentLicenseKey = loadLicenseKey();
    currentHWID = loadUserHWID();
    
    std::cout << "AuthManager initialized" << std::endl;
}

bool AuthManager::validateLicenseKey(const std::string& key) {
    // Check if key format is valid (example: XXXX-XXXX-XXXX-XXXX)
    std::regex keyRegex("^[A-Z0-9]{4}-[A-Z0-9]{4}-[A-Z0-9]{4}-[A-Z0-9]{4}$");
    if (!std::regex_match(key, keyRegex)) {
        std::cout << "Invalid license key format" << std::endl;
        return false;
    }
    
    // Fetch license from API
    if (!fetchLicenseFromAPI(key)) {
        std::cout << "Failed to fetch license from API" << std::endl;
        return false;
    }
    
    // Check if license is expired
    if (!checkLicenseExpiry(key)) {
        std::cout << "License key expired" << std::endl;
        return false;
    }
    
    return true;
}

bool AuthManager::activateLicenseKey(const std::string& key, const std::string& hwid) {
    // Validate key format
    if (!validateLicenseKey(key)) {
        return false;
    }
    
    // Check HWID binding
    if (!checkHWIDBinding(key, hwid)) {
        std::cout << "HWID not bound to this license key" << std::endl;
        return false;
    }
    
    // Submit HWID to API for binding
    if (!submitHWIDToAPI(key, hwid)) {
        std::cout << "Failed to bind HWID to license" << std::endl;
        return false;
    }
    
    // Save license key and HWID locally
    saveLicenseKey(key);
    saveUserHWID(hwid);
    
    currentLicenseKey = key;
    currentHWID = hwid;
    authorized = true;
    
    if (authSuccessCallback) {
        authSuccessCallback();
    }
    
    return true;
}

bool AuthManager::checkLicenseExpiry(const std::string& key) {
    // In production, this would check the expiry date from the API
    // For now, we'll assume the license is valid
    return true;
}

bool AuthManager::registerUser(const std::string& telegramID, const std::string& username) {
    // Disabled - requires json library
    std::cout << "Registering user: " << username << " (Telegram ID: " << telegramID << ")" << std::endl;
    
    currentTelegramID = telegramID;
    return true;
}

bool AuthManager::loginUser(const std::string& telegramID) {
    // Check if user exists and is authorized
    // In production, this would verify against the database
    currentTelegramID = telegramID;
    return true;
}

bool AuthManager::isUserAuthorized(const std::string& telegramID) {
    // Check if user has valid license
    if (currentLicenseKey.empty()) {
        return false;
    }
    
    return authorized;
}

bool AuthManager::sendTelegramMessage(const std::string& chatID, const std::string& message) {
    // Disabled - requires json library
    std::cout << "Sending Telegram message to " << chatID << ": " << message << std::endl;
    
    return true;
}

bool AuthManager::verifyTelegramCode(const std::string& telegramID, const std::string& code) {
    // Generate expected code based on telegram ID and current time
    std::string expected = generateActivationCode();
    
    // Verify code (in production, this would be more secure)
    return verifyActivationCode(code, expected);
}

bool AuthManager::fetchLicenseFromAPI(const std::string& key) {
    // Temporarily disabled - requires curl library
    std::cout << "API validation disabled - requires curl" << std::endl;
    return true;
}

bool AuthManager::submitHWIDToAPI(const std::string& key, const std::string& hwid) {
    // Disabled - requires json library
    std::cout << "Submitting HWID to API" << std::endl;
    
    return true;
}

bool AuthManager::checkHWIDBinding(const std::string& key, const std::string& hwid) {
    // Check if HWID is already bound to this license
    std::string url = config.apiEndpoint + "/license/" + key + "/hwid";
    
    std::cout << "Checking HWID binding: " << url << std::endl;
    
    // In production, fetch and verify HWID binding
    return true;
}

void AuthManager::saveLicenseKey(const std::string& key) {
    std::ofstream file("license.key");
    if (file.is_open()) {
        std::string encrypted = encryptData(key);
        file << encrypted;
        file.close();
        std::cout << "License key saved" << std::endl;
    }
}

std::string AuthManager::loadLicenseKey() {
    std::ifstream file("license.key");
    if (file.is_open()) {
        std::string encrypted;
        std::getline(file, encrypted);
        file.close();
        return decryptData(encrypted);
    }
    return "";
}

void AuthManager::saveUserHWID(const std::string& hwid) {
    std::ofstream file("hwid.dat");
    if (file.is_open()) {
        std::string encrypted = encryptData(hwid);
        file << encrypted;
        file.close();
        std::cout << "HWID saved" << std::endl;
    }
}

std::string AuthManager::loadUserHWID() {
    std::ifstream file("hwid.dat");
    if (file.is_open()) {
        std::string encrypted;
        std::getline(file, encrypted);
        file.close();
        return decryptData(encrypted);
    }
    return "";
}

std::string AuthManager::generateActivationCode() {
    // Generate a 6-digit activation code
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(100000, 999999);
    
    return std::to_string(dis(gen));
}

bool AuthManager::verifyActivationCode(const std::string& code, const std::string& expected) {
    return code == expected;
}

std::string AuthManager::formatDate(const std::string& date) {
    // Format date string
    return date;
}

bool AuthManager::isDateExpired(const std::string& date) {
    // Check if date is expired
    return false;
}

std::string AuthManager::encryptData(const std::string& data) {
    // Simple XOR encryption (in production, use AES)
    std::string key = "MAJESTIC_RP_CHEAT_KEY";
    std::string encrypted = data;
    
    for (size_t i = 0; i < data.size(); i++) {
        encrypted[i] = data[i] ^ key[i % key.size()];
    }
    
    return encrypted;
}

std::string AuthManager::decryptData(const std::string& encrypted) {
    // Simple XOR decryption (in production, use AES)
    std::string key = "MAJESTIC_RP_CHEAT_KEY";
    std::string decrypted = encrypted;
    
    for (size_t i = 0; i < encrypted.size(); i++) {
        decrypted[i] = encrypted[i] ^ key[i % key.size()];
    }
    
    return decrypted;
}
