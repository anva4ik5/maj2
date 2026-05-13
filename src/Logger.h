#pragma once

#include <string>
#include <fstream>
#include <vector>
#include <mutex>
#include <chrono>
#include <iomanip>
#include <sstream>

enum class LogLevel {
    INFO,
    WARNING,
    ERR,
    DEBUG
};

class Logger {
public:
    static Logger& getInstance();
    
    void log(LogLevel level, const std::string& message);
    void info(const std::string& message);
    void warning(const std::string& message);
    void err(const std::string& message);
    void debug(const std::string& message);
    
    void setLogFile(const std::string& filename);
    void enableConsole(bool enabled);
    void enableFile(bool enabled);
    
    std::vector<std::string> getLogs() const;
    void clearLogs();
    
    std::string getCurrentTime();
    
private:
    Logger();
    ~Logger();
    
    std::ofstream logFile;
    std::string logFilename;
    bool consoleEnabled;
    bool fileEnabled;
    std::mutex mutex;
    
    std::vector<std::string> logs;
    
    std::string levelToString(LogLevel level);
};

class ServerLogger {
public:
    ServerLogger();
    ~ServerLogger();
    
    void initialize(const std::string& serverAddress);
    void log(const std::string& message);
    void logEvent(const std::string& eventName, const std::string& data);
    void dumpServer();
    
    void setEnabled(bool enabled) { this->enabled = enabled; }
    bool isEnabled() const { return enabled; }
    
private:
    std::string serverAddress;
    bool enabled;
    std::mutex mutex;
    
    void sendToServer(const std::string& data);
};

class EventLogger {
public:
    EventLogger();
    ~EventLogger();
    
    void initialize();
    void logEvent(const std::string& event);
    void logPlayerAction(const std::string& action);
    void logNetworkEvent(const std::string& event);
    void logGameEvent(const std::string& event);
    
    void setEnabled(bool enabled) { this->enabled = enabled; }
    bool isEnabled() const { return enabled; }
    
    std::vector<std::string> getEvents() const;
    void clearEvents();
    
private:
    bool enabled;
    std::vector<std::string> events;
    std::mutex mutex;
};
    std::mutex mutex;
};
