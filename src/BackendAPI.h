#pragma once

#include <string>
#include <functional>
#include <memory>

struct LicenseData {
    std::string key;
    std::string hwid;
    std::string telegramID;
    std::string username;
    int durationDays;
    std::string createdAt;
    std::string expiresAt;
    bool active;
    bool hwidBound;
};

struct APIResponse {
    bool success;
    std::string message;
    std::string data;
    int statusCode;
};

class BackendAPI {
public:
    BackendAPI();
    ~BackendAPI();
    
    void initialize(const std::string& apiEndpoint, const std::string& apiKey);
    
    // License operations
    APIResponse validateLicense(const std::string& key);
    APIResponse activateLicense(const std::string& key, const std::string& hwid);
    APIResponse checkLicenseStatus(const std::string& key);
    APIResponse bindHWID(const std::string& key, const std::string& hwid);
    APIResponse unbindHWID(const std::string& key);
    
    // User operations
    APIResponse registerUser(const std::string& telegramID, const std::string& username, const std::string& hwid);
    APIResponse loginUser(const std::string& telegramID, const std::string& hwid);
    APIResponse getUserInfo(const std::string& telegramID);
    
    // Admin operations
    APIResponse createLicense(const std::string& adminID, const std::string& userID, int durationDays);
    APIResponse revokeLicense(const std::string& adminID, const std::string& key);
    APIResponse extendLicense(const std::string& adminID, const std::string& key, int additionalDays);
    APIResponse getAllLicenses(const std::string& adminID);
    APIResponse getAllUsers(const std::string& adminID);
    
    // Statistics
    APIResponse getStatistics(const std::string& adminID);
    APIResponse getLicenseUsage(const std::string& key);
    
    // Health check
    APIResponse healthCheck();
    
private:
    std::string apiEndpoint;
    std::string apiKey;
    
    std::string sendRequest(const std::string& endpoint, const std::string& method = "GET", 
                            const std::string& jsonData = "");
    APIResponse parseResponse(const std::string& response);
    std::string generateAuthToken();
};
