#include "NetworkClient.h"
#include <iostream>

NetworkClient::NetworkClient() : hInternet(nullptr), hConnect(nullptr), connected(false) {
}

NetworkClient::~NetworkClient() {
    disconnect();
}

bool NetworkClient::initialize() {
    hInternet = InternetOpenA("GameCheat", INTERNET_OPEN_TYPE_PRECONFIG, nullptr, nullptr, 0);
    return hInternet != nullptr;
}

bool NetworkClient::connectToServer(const std::string& host, int port) {
    std::lock_guard<std::mutex> lock(mutex);
    
    if (hConnect) {
        InternetCloseHandle(hConnect);
        hConnect = nullptr;
    }
    
    hConnect = InternetConnectA(hInternet, host.c_str(), port, 
        nullptr, nullptr, INTERNET_SERVICE_HTTP, 0, 0);
    
    connected = (hConnect != nullptr);
    return connected;
}

void NetworkClient::disconnect() {
    std::lock_guard<std::mutex> lock(mutex);
    
    if (hConnect) {
        InternetCloseHandle(hConnect);
        hConnect = nullptr;
    }
    
    if (hInternet) {
        InternetCloseHandle(hInternet);
        hInternet = nullptr;
    }
    
    connected = false;
}

std::string NetworkClient::readResponse(HINTERNET hRequest) {
    std::string response;
    char buffer[4096];
    DWORD bytesRead;
    
    while (InternetReadFile(hRequest, buffer, sizeof(buffer) - 1, &bytesRead) && bytesRead > 0) {
        buffer[bytesRead] = '\0';
        response += buffer;
    }
    
    return response;
}

std::string NetworkClient::sendHTTPRequest(const std::string& url, const std::string& data) {
    std::lock_guard<std::mutex> lock(mutex);
    
    if (!hConnect || !connected) {
        return "";
    }
    
    HINTERNET hRequest = HttpOpenRequestA(hConnect, data.empty() ? "GET" : "POST", 
        url.c_str(), nullptr, nullptr, nullptr, 0, 0);
    
    if (!hRequest) {
        return "";
    }
    
    if (!data.empty()) {
        const char* headers = "Content-Type: application/json\r\n";
        HttpSendRequestA(hRequest, headers, -1, (LPVOID)data.c_str(), data.length());
    } else {
        HttpSendRequestA(hRequest, nullptr, 0, nullptr, 0);
    }
    
    std::string response = readResponse(hRequest);
    InternetCloseHandle(hRequest);
    
    return response;
}

std::string NetworkClient::sendGETRequest(const std::string& url) {
    return sendHTTPRequest(url, "");
}

std::string NetworkClient::sendPOSTRequest(const std::string& url, const std::string& data) {
    return sendHTTPRequest(url, data);
}

bool NetworkClient::sendAsyncRequest(const std::string& url, const std::string& data, ResponseCallback callback) {
    // Simple async implementation using thread
    std::thread([this, url, data, callback]() {
        std::string response = sendHTTPRequest(url, data);
        callback(response);
    }).detach();
    
    return true;
}

bool NetworkClient::isConnected() const {
    return connected;
}
