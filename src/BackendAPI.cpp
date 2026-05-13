#include "BackendAPI.h"
#include <iostream>
#include <sstream>
#include <random>
#include <windows.h>
#include <wininet.h>

#pragma comment(lib, "wininet.lib")

BackendAPI::BackendAPI() {
}

BackendAPI::~BackendAPI() {
}

void BackendAPI::initialize(const std::string& apiEndpoint, const std::string& apiKey) {
    this->apiEndpoint = apiEndpoint;
    this->apiKey = apiKey;
    
    std::cout << "BackendAPI initialized: " << apiEndpoint << std::endl;
}

APIResponse BackendAPI::validateLicense(const std::string& key) {
    std::string endpoint = apiEndpoint + "/api/license/validate";
    std::string jsonData = "{\"key\":\"" + key + "\"}";
    std::string response = sendRequest(endpoint, "POST", jsonData);
    return parseResponse(response);
}

APIResponse BackendAPI::activateLicense(const std::string& key, const std::string& hwid) {
    std::string endpoint = apiEndpoint + "/api/license/activate";
    std::string jsonData = "{\"key\":\"" + key + "\",\"hwid\":\"" + hwid + "\"}";
    std::string response = sendRequest(endpoint, "POST", jsonData);
    return parseResponse(response);
}

APIResponse BackendAPI::checkLicenseStatus(const std::string& key) {
    std::string endpoint = apiEndpoint + "/api/license/" + key + "/status";
    std::string response = sendRequest(endpoint, "GET");
    return parseResponse(response);
}

APIResponse BackendAPI::bindHWID(const std::string& key, const std::string& hwid) {
    std::string endpoint = apiEndpoint + "/api/license/" + key + "/bind";
    std::string jsonData = "{\"hwid\":\"" + hwid + "\"}";
    std::string response = sendRequest(endpoint, "POST", jsonData);
    return parseResponse(response);
}

APIResponse BackendAPI::unbindHWID(const std::string& key) {
    std::string endpoint = apiEndpoint + "/api/license/" + key + "/unbind";
    std::string response = sendRequest(endpoint, "POST");
    return parseResponse(response);
}

APIResponse BackendAPI::registerUser(const std::string& telegramID, const std::string& username, const std::string& hwid) {
    std::string endpoint = apiEndpoint + "/api/user/register";
    std::string jsonData = "{\"telegram_id\":\"" + telegramID + "\",\"username\":\"" + username + "\",\"hwid\":\"" + hwid + "\"}";
    std::string response = sendRequest(endpoint, "POST", jsonData);
    return parseResponse(response);
}

APIResponse BackendAPI::loginUser(const std::string& telegramID, const std::string& hwid) {
    std::string endpoint = apiEndpoint + "/api/user/login";
    std::string jsonData = "{\"telegram_id\":\"" + telegramID + "\",\"hwid\":\"" + hwid + "\"}";
    std::string response = sendRequest(endpoint, "POST", jsonData);
    return parseResponse(response);
}

APIResponse BackendAPI::getUserInfo(const std::string& telegramID) {
    std::string endpoint = apiEndpoint + "/api/user/" + telegramID;
    std::string response = sendRequest(endpoint, "GET");
    return parseResponse(response);
}

APIResponse BackendAPI::createLicense(const std::string& adminID, const std::string& userID, int durationDays) {
    std::string endpoint = apiEndpoint + "/api/admin/license/create";
    std::string jsonData = "{\"admin_id\":\"" + adminID + "\",\"user_id\":\"" + userID + "\",\"duration_days\":" + std::to_string(durationDays) + "}";
    std::string response = sendRequest(endpoint, "POST", jsonData);
    return parseResponse(response);
}

APIResponse BackendAPI::revokeLicense(const std::string& adminID, const std::string& key) {
    std::string endpoint = apiEndpoint + "/api/admin/license/revoke";
    std::string jsonData = "{\"admin_id\":\"" + adminID + "\",\"key\":\"" + key + "\"}";
    std::string response = sendRequest(endpoint, "POST", jsonData);
    return parseResponse(response);
}

APIResponse BackendAPI::extendLicense(const std::string& adminID, const std::string& key, int additionalDays) {
    std::string endpoint = apiEndpoint + "/api/admin/license/extend";
    std::string jsonData = "{\"admin_id\":\"" + adminID + "\",\"key\":\"" + key + "\",\"additional_days\":" + std::to_string(additionalDays) + "}";
    std::string response = sendRequest(endpoint, "POST", jsonData);
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

// JSON string escape helper
static std::string jsonEscape(const std::string& s) {
    std::string out;
    out.reserve(s.size() + 8);
    for (char c : s) {
        switch (c) {
            case '"':  out += "\\\""; break;
            case '\\': out += "\\\\"; break;
            case '\b': out += "\\b"; break;
            case '\f': out += "\\f"; break;
            case '\n': out += "\\n"; break;
            case '\r': out += "\\r"; break;
            case '\t': out += "\\t"; break;
            default:
                if ((unsigned char)c < 0x20) {
                    char buf[8];
                    snprintf(buf, sizeof(buf), "\\u%04x", (unsigned char)c);
                    out += buf;
                } else {
                    out += c;
                }
        }
    }
    return out;
}

// Extract a top-level JSON string field by name from a flat object response
static std::string extractStringField(const std::string& json, const std::string& field) {
    std::string needle = "\"" + field + "\"";
    size_t pos = json.find(needle);
    if (pos == std::string::npos) return "";
    pos = json.find(':', pos);
    if (pos == std::string::npos) return "";
    pos++;
    while (pos < json.size() && (json[pos] == ' ' || json[pos] == '\t')) pos++;
    if (pos >= json.size() || json[pos] != '"') return "";
    pos++;
    std::string out;
    while (pos < json.size() && json[pos] != '"') {
        if (json[pos] == '\\' && pos + 1 < json.size()) {
            char next = json[pos + 1];
            if (next == '"' || next == '\\' || next == '/') { out += next; pos += 2; continue; }
            if (next == 'n') { out += '\n'; pos += 2; continue; }
            if (next == 't') { out += '\t'; pos += 2; continue; }
            if (next == 'r') { out += '\r'; pos += 2; continue; }
        }
        out += json[pos++];
    }
    return out;
}

static bool extractBoolField(const std::string& json, const std::string& field) {
    std::string needle = "\"" + field + "\"";
    size_t pos = json.find(needle);
    if (pos == std::string::npos) return false;
    pos = json.find(':', pos);
    if (pos == std::string::npos) return false;
    pos++;
    while (pos < json.size() && (json[pos] == ' ' || json[pos] == '\t')) pos++;
    return json.compare(pos, 4, "true") == 0;
}

AuthResult BackendAPI::registerStart(const std::string& login, const std::string& password,
                                     const std::string& telegramID, const std::string& hwid) {
    AuthResult r;
    std::string url = apiEndpoint + "/api/auth/register/start";
    std::string body = "{\"login\":\"" + jsonEscape(login) +
                       "\",\"password\":\"" + jsonEscape(password) +
                       "\",\"telegram_id\":\"" + jsonEscape(telegramID) +
                       "\",\"hwid\":\"" + jsonEscape(hwid) + "\"}";
    std::string resp = sendRequest(url, "POST", body);
    r.success = extractBoolField(resp, "success");
    r.message = extractStringField(resp, "message");
    r.sessionId = extractStringField(resp, "session_id");
    r.requireCode = r.success;
    return r;
}

AuthResult BackendAPI::registerConfirm(const std::string& sessionId, const std::string& code) {
    AuthResult r;
    std::string url = apiEndpoint + "/api/auth/register/confirm";
    std::string body = "{\"session_id\":\"" + jsonEscape(sessionId) +
                       "\",\"code\":\"" + jsonEscape(code) + "\"}";
    std::string resp = sendRequest(url, "POST", body);
    r.success = extractBoolField(resp, "success");
    r.message = extractStringField(resp, "message");
    r.token = extractStringField(resp, "token");
    r.login = extractStringField(resp, "login");
    r.telegramId = extractStringField(resp, "telegram_id");
    return r;
}

AuthResult BackendAPI::login(const std::string& loginVal, const std::string& password, const std::string& hwid) {
    AuthResult r;
    std::string url = apiEndpoint + "/api/auth/login";
    std::string body = "{\"login\":\"" + jsonEscape(loginVal) +
                       "\",\"password\":\"" + jsonEscape(password) +
                       "\",\"hwid\":\"" + jsonEscape(hwid) + "\"}";
    std::string resp = sendRequest(url, "POST", body);
    r.success = extractBoolField(resp, "success");
    r.message = extractStringField(resp, "message");
    r.requireCode = extractBoolField(resp, "require_2fa");
    r.token = extractStringField(resp, "token");
    r.sessionId = extractStringField(resp, "session_id");
    r.login = extractStringField(resp, "login");
    r.telegramId = extractStringField(resp, "telegram_id");
    return r;
}

AuthResult BackendAPI::loginConfirm(const std::string& sessionId, const std::string& code) {
    AuthResult r;
    std::string url = apiEndpoint + "/api/auth/login/confirm";
    std::string body = "{\"session_id\":\"" + jsonEscape(sessionId) +
                       "\",\"code\":\"" + jsonEscape(code) + "\"}";
    std::string resp = sendRequest(url, "POST", body);
    r.success = extractBoolField(resp, "success");
    r.message = extractStringField(resp, "message");
    r.token = extractStringField(resp, "token");
    return r;
}

AuthResult BackendAPI::verifyToken(const std::string& token, const std::string& hwid) {
    AuthResult r;
    std::string url = apiEndpoint + "/api/auth/verify";
    std::string body = "{\"token\":\"" + jsonEscape(token) +
                       "\",\"hwid\":\"" + jsonEscape(hwid) + "\"}";
    std::string resp = sendRequest(url, "POST", body);
    r.success = extractBoolField(resp, "success");
    r.message = extractStringField(resp, "message");
    r.token = token;
    r.login = extractStringField(resp, "login");
    r.telegramId = extractStringField(resp, "telegram_id");
    return r;
}

bool BackendAPI::logout(const std::string& token) {
    std::string url = apiEndpoint + "/api/auth/logout";
    std::string body = "{\"token\":\"" + jsonEscape(token) + "\"}";
    std::string resp = sendRequest(url, "POST", body);
    return extractBoolField(resp, "success");
}

// Parse URL into host, port, and path. Supports http:// and https://
static bool parseURL(const std::string& url, std::string& host, int& port, std::string& path) {
    std::string temp = url;
    bool https = false;
    
    if (temp.substr(0, 8) == "https://") {
        https = true;
        temp = temp.substr(8);
    } else if (temp.substr(0, 7) == "http://") {
        temp = temp.substr(7);
    }
    
    size_t slashPos = temp.find('/');
    if (slashPos == std::string::npos) {
        host = temp;
        path = "/";
    } else {
        host = temp.substr(0, slashPos);
        path = temp.substr(slashPos);
    }
    
    size_t colonPos = host.find(':');
    if (colonPos != std::string::npos) {
        port = std::stoi(host.substr(colonPos + 1));
        host = host.substr(0, colonPos);
    } else {
        port = https ? INTERNET_DEFAULT_HTTPS_PORT : INTERNET_DEFAULT_HTTP_PORT;
    }
    
    return https;
}

std::string BackendAPI::sendRequest(const std::string& endpoint, const std::string& method, 
                                    const std::string& jsonData) {
    std::string host, path;
    int port;
    bool https = parseURL(endpoint, host, port, path);
    
    HINTERNET hInternet = InternetOpenA("GameCheat", INTERNET_OPEN_TYPE_PRECONFIG, nullptr, nullptr, 0);
    if (!hInternet) {
        std::cerr << "InternetOpenA failed" << std::endl;
        return "";
    }
    
    HINTERNET hConnect = InternetConnectA(hInternet, host.c_str(), port, nullptr, nullptr, INTERNET_SERVICE_HTTP, 0, 0);
    if (!hConnect) {
        InternetCloseHandle(hInternet);
        return "";
    }
    
    DWORD flags = 0;
    if (https) {
        flags |= INTERNET_FLAG_SECURE;
        flags |= INTERNET_FLAG_IGNORE_CERT_CN_INVALID;
        flags |= INTERNET_FLAG_IGNORE_CERT_DATE_INVALID;
    }
    flags |= INTERNET_FLAG_RELOAD;
    flags |= INTERNET_FLAG_NO_CACHE_WRITE;
    
    HINTERNET hRequest = HttpOpenRequestA(hConnect, method.c_str(), path.c_str(), nullptr, nullptr, nullptr, flags, 0);
    if (!hRequest) {
        InternetCloseHandle(hConnect);
        InternetCloseHandle(hInternet);
        return "";
    }
    
    // Build headers with API key
    std::string headers = "Content-Type: application/json\r\n";
    if (!apiKey.empty()) {
        headers += "X-API-Key: " + apiKey + "\r\n";
    }
    
    BOOL sendResult = FALSE;
    if (!jsonData.empty()) {
        sendResult = HttpSendRequestA(hRequest, headers.c_str(), (DWORD)headers.length(), (LPVOID)jsonData.c_str(), (DWORD)jsonData.length());
    } else {
        sendResult = HttpSendRequestA(hRequest, headers.c_str(), (DWORD)headers.length(), nullptr, 0);
    }
    
    if (!sendResult) {
        InternetCloseHandle(hRequest);
        InternetCloseHandle(hConnect);
        InternetCloseHandle(hInternet);
        return "";
    }
    
    // Read response
    std::string response;
    char buffer[4096];
    DWORD bytesRead;
    while (InternetReadFile(hRequest, buffer, sizeof(buffer) - 1, &bytesRead) && bytesRead > 0) {
        buffer[bytesRead] = '\0';
        response += buffer;
    }
    
    InternetCloseHandle(hRequest);
    InternetCloseHandle(hConnect);
    InternetCloseHandle(hInternet);
    
    return response;
}

APIResponse BackendAPI::parseResponse(const std::string& response) {
    APIResponse apiResponse;
    apiResponse.success = false;
    apiResponse.statusCode = 0;
    
    if (response.empty()) {
        apiResponse.message = "No response from server";
        return apiResponse;
    }
    
    // Simple JSON parsing without external library
    // Check for success field
    if (response.find("\"success\":true") != std::string::npos ||
        response.find("\"success\": true") != std::string::npos) {
        apiResponse.success = true;
    }
    
    // Extract message if present
    size_t msgPos = response.find("\"message\":\"");
    if (msgPos != std::string::npos) {
        msgPos += 11;
        size_t msgEnd = response.find("\"", msgPos);
        if (msgEnd != std::string::npos) {
            apiResponse.message = response.substr(msgPos, msgEnd - msgPos);
        }
    }
    
    // Extract data if present
    size_t dataPos = response.find("\"data\":");
    if (dataPos != std::string::npos) {
        dataPos += 7;
        // Find matching brace
        int braceCount = 0;
        size_t dataStart = std::string::npos;
        for (size_t i = dataPos; i < response.length(); i++) {
            if (response[i] == '{') {
                if (braceCount == 0) dataStart = i;
                braceCount++;
            } else if (response[i] == '}') {
                braceCount--;
                if (braceCount == 0 && dataStart != std::string::npos) {
                    apiResponse.data = response.substr(dataStart, i - dataStart + 1);
                    break;
                }
            }
        }
    }
    
    if (apiResponse.message.empty()) {
        apiResponse.message = apiResponse.success ? "Request successful" : "Request failed";
    }
    
    return apiResponse;
}

std::string BackendAPI::generateAuthToken() {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, 35);
    
    const char charset[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
    std::string token;
    
    for (int i = 0; i < 32; i++) {
        token += charset[dis(gen)];
    }
    
    return token;
}
