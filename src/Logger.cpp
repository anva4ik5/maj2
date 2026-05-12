#include "Logger.h"
#include <iostream>
#include <ctime>

// Logger implementation
Logger& Logger::getInstance() {
    static Logger instance;
    return instance;
}

Logger::Logger() : consoleEnabled(true), fileEnabled(false) {
}

Logger::~Logger() {
    if (logFile.is_open()) {
        logFile.close();
    }
}

void Logger::log(LogLevel level, const std::string& message) {
    std::lock_guard<std::mutex> lock(mutex);
    
    std::string timeStr = getCurrentTime();
    std::string levelStr = levelToString(level);
    std::string logEntry = "[" + timeStr + "] [" + levelStr + "] " + message;
    
    logs.push_back(logEntry);
    
    if (consoleEnabled) {
        std::cout << logEntry << std::endl;
    }
    
    if (fileEnabled && logFile.is_open()) {
        logFile << logEntry << std::endl;
        logFile.flush();
    }
}

void Logger::info(const std::string& message) {
    log(LogLevel::INFO, message);
}

void Logger::warning(const std::string& message) {
    log(LogLevel::WARNING, message);
}

void Logger::error(const std::string& message) {
    log(LogLevel::ERROR, message);
}

void Logger::debug(const std::string& message) {
    log(LogLevel::DEBUG, message);
}

void Logger::setLogFile(const std::string& filename) {
    std::lock_guard<std::mutex> lock(mutex);
    
    if (logFile.is_open()) {
        logFile.close();
    }
    
    logFilename = filename;
    logFile.open(filename, std::ios::app);
    
    if (logFile.is_open()) {
        fileEnabled = true;
    } else {
        fileEnabled = false;
    }
}

void Logger::enableConsole(bool enabled) {
    consoleEnabled = enabled;
}

void Logger::enableFile(bool enabled) {
    fileEnabled = enabled;
}

std::vector<std::string> Logger::getLogs() const {
    return logs;
}

void Logger::clearLogs() {
    std::lock_guard<std::mutex> lock(mutex);
    logs.clear();
}

std::string Logger::getCurrentTime() {
    auto now = std::chrono::system_clock::now();
    auto time = std::chrono::system_clock::to_time_t(now);
    
    std::stringstream ss;
    ss << std::put_time(std::localtime(&time), "%Y-%m-%d %H:%M:%S");
    return ss.str();
}

std::string Logger::levelToString(LogLevel level) {
    switch (level) {
        case LogLevel::INFO: return "INFO";
        case LogLevel::WARNING: return "WARNING";
        case LogLevel::ERROR: return "ERROR";
        case LogLevel::DEBUG: return "DEBUG";
        default: return "UNKNOWN";
    }
}

// ServerLogger implementation
ServerLogger::ServerLogger() : enabled(false) {
}

ServerLogger::~ServerLogger() {
}

void ServerLogger::initialize(const std::string& serverAddress) {
    this->serverAddress = serverAddress;
}

void ServerLogger::log(const std::string& message) {
    if (!enabled) return;
    
    std::lock_guard<std::mutex> lock(mutex);
    sendToServer(message);
}

void ServerLogger::logEvent(const std::string& eventName, const std::string& data) {
    if (!enabled) return;
    
    std::string message = "{\"event\":\"" + eventName + "\",\"data\":\"" + data + "\"}";
    log(message);
}

void ServerLogger::dumpServer() {
    if (!enabled) return;
    
    std::string message = "{\"action\":\"dump_server\"}";
    log(message);
}

void ServerLogger::sendToServer(const std::string& data) {
    // TODO: Send data to server using NetworkClient
    // This would require access to NetworkClient instance
    std::cout << "[ServerLogger] Sending to server: " << data << std::endl;
}

// EventLogger implementation
EventLogger::EventLogger() : enabled(false) {
}

EventLogger::~EventLogger() {
}

void EventLogger::initialize() {
}

void EventLogger::logEvent(const std::string& event) {
    if (!enabled) return;
    
    std::lock_guard<std::mutex> lock(mutex);
    
    std::string time = Logger::getInstance().getCurrentTime();
    std::string logEntry = "[" + time + "] " + event;
    
    events.push_back(logEntry);
    Logger::getInstance().info(logEntry);
}

void EventLogger::logPlayerAction(const std::string& action) {
    logEvent("[PLAYER] " + action);
}

void EventLogger::logNetworkEvent(const std::string& event) {
    logEvent("[NETWORK] " + event);
}

void EventLogger::logGameEvent(const std::string& event) {
    logEvent("[GAME] " + event);
}

std::vector<std::string> EventLogger::getEvents() const {
    return events;
}

void EventLogger::clearEvents() {
    std::lock_guard<std::mutex> lock(mutex);
    events.clear();
}
