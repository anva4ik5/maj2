#pragma once

#include <string>
#include <memory>
#include <functional>

struct AuthConfig {
    std::string telegramBotToken;
    std::string adminTelegramID;
    std::string apiEndpoint;
    std::string databaseURL;
};

struct LicenseKey {
    std::string key;
    std::string hwid;
    int durationDays;
    std::string expiryDate;
    bool active;
    std::string createdAt;
};

struct User {
    std::string telegramID;
    std::string username;
    std::string hwid;
    std::string licenseKey;
    std::string expiryDate;
    bool isActive;
};

class AuthManager {
public:
    AuthManager();
    ~AuthManager();
    
    void initialize(const AuthConfig& config);
    
    // License key operations
    bool validateLicenseKey(const std::string& key);
    bool activateLicenseKey(const std::string& key, const std::string& hwid);
    bool checkLicenseExpiry(const std::string& key);
    
    // User operations
    bool registerUser(const std::string& telegramID, const std::string& username);
    bool loginUser(const std::string& telegramID);
    bool isUserAuthorized(const std::string& telegramID);
    
    // Telegram integration
    bool sendTelegramMessage(const std::string& chatID, const std::string& message);
    bool verifyTelegramCode(const std::string& telegramID, const std::string& code);
    
    // API operations
    bool fetchLicenseFromAPI(const std::string& key);
    bool submitHWIDToAPI(const std::string& key, const std::string& hwid);
    bool checkHWIDBinding(const std::string& key, const std::string& hwid);
    
    // Local storage
    void saveLicenseKey(const std::string& key);
    std::string loadLicenseKey();
    void saveUserHWID(const std::string& hwid);
    std::string loadUserHWID();
    
    // Getters
    bool isAuthorized() const { return authorized; }
    std::string getCurrentLicenseKey() const { return currentLicenseKey; }
    std::string getCurrentHWID() const { return currentHWID; }
    
    // Callbacks
    void setAuthSuccessCallback(std::function<void()> callback) { authSuccessCallback = callback; }
    void setAuthFailureCallback(std::function<void(const std::string&)> callback) { authFailureCallback = callback; }
    
private:
    AuthConfig config;
    bool authorized;
    std::string currentLicenseKey;
    std::string currentHWID;
    std::string currentTelegramID;
    
    std::function<void()> authSuccessCallback;
    std::function<void(const std::string&)> authFailureCallback;
    
    // Helper functions
    std::string generateActivationCode();
    bool verifyActivationCode(const std::string& code, const std::string& expected);
    std::string formatDate(const std::string& date);
    bool isDateExpired(const std::string& date);
    std::string encryptData(const std::string& data);
    std::string decryptData(const std::string& encrypted);
};
