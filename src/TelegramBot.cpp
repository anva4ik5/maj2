#include "TelegramBot.h"
#include <iostream>
#include <sstream>
#include <random>
#include <thread>
#include <chrono>

TelegramBot::TelegramBot() : polling(false) {
}

TelegramBot::~TelegramBot() {
    stopPolling();
}

void TelegramBot::initialize(const std::string& botToken, const std::string& adminID) {
    this->botToken = botToken;
    this->adminID = adminID;
    
    // curl_global_init(CURL_GLOBAL_ALL); // Disabled
    
    std::cout << "TelegramBot initialized" << std::endl;
}

bool TelegramBot::sendMessage(const std::string& chatID, const std::string& message) {
    // Disabled - requires json library
    std::cout << "Sending message to " << chatID << ": " << message << std::endl;
    
    return true;
}

bool TelegramBot::sendMessageWithKeyboard(const std::string& chatID, const std::string& message, 
                                           const std::string& keyboardJSON) {
    // Disabled - requires json library
    std::cout << "Sending message with keyboard to " << chatID << std::endl;
    
    return true;
}

bool TelegramBot::answerCallbackQuery(const std::string& callbackQueryID, const std::string& text) {
    // Disabled - requires json library
    std::cout << "Answering callback query: " << callbackQueryID << std::endl;
    
    return true;
}

bool TelegramBot::isAdmin(const std::string& userID) {
    return userID == adminID;
}

bool TelegramBot::generateLicenseKey(const std::string& adminID, const std::string& userID, 
                                     int durationDays, std::string& licenseKey) {
    if (!isAdmin(adminID)) {
        std::cout << "Unauthorized: Only admin can generate license keys" << std::endl;
        return false;
    }
    
    licenseKey = generateKey();
    
    std::string message = "🔑 <b>New License Key Generated</b>\n\n";
    message += "👤 User ID: " + userID + "\n";
    message += "⏱ Duration: " + std::to_string(durationDays) + " days\n";
    message += "🔑 Key: <code>" + licenseKey + "</code>\n\n";
    message += "Please share this key with the user securely.";
    
    sendMessage(adminID, message);
    
    return true;
}

bool TelegramBot::revokeLicenseKey(const std::string& adminID, const std::string& licenseKey) {
    if (!isAdmin(adminID)) {
        return false;
    }
    
    // In production, this would call the backend API to revoke the key
    std::string message = "🔑 License key revoked: " + licenseKey;
    sendMessage(adminID, message);
    
    return true;
}

bool TelegramBot::extendLicense(const std::string& adminID, const std::string& licenseKey, int additionalDays) {
    if (!isAdmin(adminID)) {
        return false;
    }
    
    // In production, this would call the backend API to extend the key
    std::string message = "🔑 License extended: " + licenseKey + " (+ " + std::to_string(additionalDays) + " days)";
    sendMessage(adminID, message);
    
    return true;
}

void TelegramBot::startPolling() {
    polling = true;
    
    std::thread([this]() {
        int lastUpdateID = 0;
        
        while (polling) {
            std::string url = "https://api.telegram.org/bot" + botToken + "/getUpdates";
            if (lastUpdateID > 0) {
                url += "?offset=" + std::to_string(lastUpdateID + 1);
            }
            
            std::string response = sendRequest(url);
            
            // Disabled JSON parsing - requires nlohmann/json
            if (!response.empty()) {
                std::cout << "Received update from Telegram" << std::endl;
            }
            
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }
    }).detach();
}

void TelegramBot::stopPolling() {
    polling = false;
}

std::string TelegramBot::generateKey() {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, 35);
    
    const char charset[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
    std::string key = "";
    
    for (int i = 0; i < 16; i++) {
        if (i > 0 && i % 4 == 0) {
            key += "-";
        }
        key += charset[dis(gen)];
    }
    
    return key;
}

std::string TelegramBot::sendRequest(const std::string& endpoint, const std::string& jsonData) {
    // Disabled - requires curl library
    std::cout << "Telegram API disabled - requires curl" << std::endl;
    return "";
}
