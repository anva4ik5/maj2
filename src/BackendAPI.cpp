#include "BackendAPI.h"
#include <iostream>
#include <sstream>
#include <random>

BackendAPI::BackendAPI() {
}

BackendAPI::~BackendAPI() {
    // curl_global_cleanup(); // Disabled
}

void BackendAPI::initialize(const std::string& apiEndpoint, const std::string& apiKey) {
    this->apiEndpoint = apiEndpoint;
    this->apiKey = apiKey;
    
    std::cout << "BackendAPI initialized: " << apiEndpoint << std::endl;
}

APIResponse BackendAPI::validateLicense(const std::string& key) {
    std::string endpoint = apiEndpoint + "/api/license/validate";
    
    // Disabled - requires json library
    std::string response = sendRequest(endpoint, "POST", "");
    return parseResponse(response);
}

APIResponse BackendAPI::activateLicense(const std::string& key, const std::string& hwid) {
    std::string endpoint = apiEndpoint + "/api/license/activate";
    
    // Disabled - requires json library
    std::string response = sendRequest(endpoint, "POST", "");
    return parseResponse(response);
}

APIResponse BackendAPI::checkLicenseStatus(const std::string& key) {
    std::string endpoint = apiEndpoint + "/api/license/" + key + "/status";
    std::string response = sendRequest(endpoint, "GET");
    return parseResponse(response);
}

APIResponse BackendAPI::bindHWID(const std::string& key, const std::string& hwid) {
    std::string endpoint = apiEndpoint + "/api/license/" + key + "/bind";
    
    // Disabled - requires json library
    std::string response = sendRequest(endpoint, "POST", "");
    return parseResponse(response);
}

APIResponse BackendAPI::unbindHWID(const std::string& key) {
    std::string endpoint = apiEndpoint + "/api/license/" + key + "/unbind";
    std::string response = sendRequest(endpoint, "POST");
    return parseResponse(response);
}

APIResponse BackendAPI::registerUser(const std::string& telegramID, const std::string& username, const std::string& hwid) {
    std::string endpoint = apiEndpoint + "/api/user/register";
    
    // Disabled - requires json library
    std::string response = sendRequest(endpoint, "POST", "");
    return parseResponse(response);
}

APIResponse BackendAPI::loginUser(const std::string& telegramID, const std::string& hwid) {
    std::string endpoint = apiEndpoint + "/api/user/login";
    
    // Disabled - requires json library
    std::string response = sendRequest(endpoint, "POST", "");
    return parseResponse(response);
}

APIResponse BackendAPI::getUserInfo(const std::string& telegramID) {
    std::string endpoint = apiEndpoint + "/api/user/" + telegramID;
    std::string response = sendRequest(endpoint, "GET");
    return parseResponse(response);
}

APIResponse BackendAPI::createLicense(const std::string& adminID, const std::string& userID, int durationDays) {
    std::string endpoint = apiEndpoint + "/api/admin/license/create";
    
    // Disabled - requires json library
    std::string response = sendRequest(endpoint, "POST", "");
    return parseResponse(response);
}

APIResponse BackendAPI::revokeLicense(const std::string& adminID, const std::string& key) {
    std::string endpoint = apiEndpoint + "/api/admin/license/revoke";
    
    json payload;
    payload["admin_id"] = adminID;
    payload["key"] = key;
    
    std::string response = sendRequest(endpoint, "POST", payload.dump());
    return parseResponse(response);
}

APIResponse BackendAPI::extendLicense(const std::string& adminID, const std::string& key, int additionalDays) {
    std::string endpoint = apiEndpoint + "/api/admin/license/extend";
    
    json payload;
    payload["admin_id"] = adminID;
    payload["key"] = key;
    payload["additional_days"] = additionalDays;
    
    std::string response = sendRequest(endpoint, "POST", payload.dump());
    return parseResponse(response);
}

APIResponse BackendAPI::getAllLicenses(const std::string& adminID) {
    std::string endpoint = apiEndpoint + "/api/admin/licenses?admin_id=" + adminID;
    std::string response = sendRequest(endpoint, "GET");
    return parseResponse(response);
}

APIResponse BackendAPI::getAllUsers(const std::string& adminID) {
    std::string endpoint = apiEndpoint + "/api/admin/users?admin_id=" + adminID;
    std::string response = sendRequest(endpoint, "GET");
    return parseResponse(response);
}

APIResponse BackendAPI::getStatistics(const std::string& adminID) {
    std::string endpoint = apiEndpoint + "/api/admin/statistics?admin_id=" + adminID;
    std::string response = sendRequest(endpoint, "GET");
    return parseResponse(response);
}

APIResponse BackendAPI::getLicenseUsage(const std::string& key) {
    std::string endpoint = apiEndpoint + "/api/license/" + key + "/usage";
    std::string response = sendRequest(endpoint, "GET");
    return parseResponse(response);
}

APIResponse BackendAPI::healthCheck() {
    std::string endpoint = apiEndpoint + "/api/health";
    std::string response = sendRequest(endpoint, "GET");
    return parseResponse(response);
}

std::string BackendAPI::sendRequest(const std::string& endpoint, const std::string& method, 
                                    const std::string& jsonData) {
    // Disabled - requires curl library
    std::cout << "Backend API disabled - requires curl" << std::endl;
    return "";
}

APIResponse BackendAPI::parseResponse(const std::string& response) {
    APIResponse apiResponse;
    apiResponse.success = false;
    apiResponse.statusCode = 0;
    
    if (response.empty()) {
        apiResponse.message = "No response from server";
        return apiResponse;
    }
    
    // Disabled JSON parsing - requires nlohmann/json
    apiResponse.success = true;
    apiResponse.message = "Response received";
    
    return apiResponse;
}

std::string BackendAPI::generateAuthToken() {
    // Generate a random auth token
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, 35);
    
    const char charset[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
    std::string token = "";
    
    for (int i = 0; i < 32; i++) {
        token += charset[dis(gen)];
    }
    
    return token;
}
