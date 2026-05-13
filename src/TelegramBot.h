#pragma once

#include <string>
#include <functional>
#include <memory>
#include <atomic>

struct TelegramMessage {
    std::string chatID;
    std::string messageID;
    std::string text;
    std::string username;
    std::string userID;
    long timestamp;
};

class TelegramBot {
public:
    TelegramBot();
    ~TelegramBot();
    
    void initialize(const std::string& botToken, const std::string& adminID);
    
    bool sendMessage(const std::string& chatID, const std::string& message);
    bool sendMessageWithKeyboard(const std::string& chatID, const std::string& message, 
                                  const std::string& keyboardJSON);
    bool answerCallbackQuery(const std::string& callbackQueryID, const std::string& text = "");
    
    bool isAdmin(const std::string& userID);
    
    bool generateLicenseKey(const std::string& adminID, const std::string& userID, 
                           int durationDays, std::string& licenseKey);
    bool revokeLicenseKey(const std::string& adminID, const std::string& licenseKey);
    bool extendLicense(const std::string& adminID, const std::string& licenseKey, int additionalDays);
    
    void startPolling();
    void stopPolling();
    
    void setMessageCallback(std::function<void(const TelegramMessage&)> callback) { messageCallback = callback; }
    void setCallbackQueryCallback(std::function<void(const std::string&, const std::string&)> callback) { callbackQueryCallback = callback; }
    
private:
    std::string botToken;
    std::string adminID;
    std::atomic<bool> polling;
    
    std::function<void(const TelegramMessage&)> messageCallback;
    std::function<void(const std::string&, const std::string&)> callbackQueryCallback;
    
    std::string generateKey();
    std::string sendRequest(const std::string& endpoint, const std::string& jsonData = "");
};
