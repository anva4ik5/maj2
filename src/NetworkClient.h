#pragma once

#include <windows.h>
#include <wininet.h>
#include <string>
#include <mutex>
#include <functional>

#pragma comment(lib, "wininet.lib")

class NetworkClient {
public:
    using ResponseCallback = std::function<void(const std::string&)>;
    
    NetworkClient();
    ~NetworkClient();
    
    bool initialize();
    bool connectToServer(const std::string& host, int port);
    void disconnect();
    
    std::string sendHTTPRequest(const std::string& url, const std::string& data = "");
    std::string sendGETRequest(const std::string& url);
    std::string sendPOSTRequest(const std::string& url, const std::string& data);
    
    bool sendAsyncRequest(const std::string& url, const std::string& data, ResponseCallback callback);
    
    bool isConnected() const;
    
private:
    HINTERNET hInternet;
    HINTERNET hConnect;
    std::mutex mutex;
    bool connected;
    
    std::vector<std::thread> asyncThreads;
    
    std::string readResponse(HINTERNET hRequest);
};
