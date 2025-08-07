#include "Logger.h"
#include "Config.h"
#include <iostream>
#include <iomanip>
#include <chrono>
#include <sstream>

Logger::Logger(const std::string& filename, bool console, LogLevel level) 
    : consoleOutput(console), currentLogLevel(level) {
    
    if (!filename.empty()) {
        logFile.open(filename, std::ios::app);
        
        if (!logFile.is_open()) {
            std::cerr << "Warning: Could not open log file: " << filename << std::endl;
        }
    }
}

Logger::~Logger() {
    if (logFile.is_open()) {
        logFile.close();
    }
}

void Logger::configure(const Config& config) {
    if (logFile.is_open()) {
        logFile.close();
    }
    
    consoleOutput = config.isConsoleLoggingEnabled();
    currentLogLevel = config.getLogLevel();
    
    const std::string& filename = config.getLogFile();
    if (!filename.empty()) {
        logFile.open(filename, std::ios::app);
        
        if (!logFile.is_open()) {
            std::cerr << "Warning: Could not open log file: " << filename << std::endl;
        }
    }
}

void Logger::debug(const std::string& message) {
    writeLog(LogLevel::DEBUG, message);
}

void Logger::info(const std::string& message) {
    writeLog(LogLevel::INFO, message);
}

void Logger::warning(const std::string& message) {
    writeLog(LogLevel::WARNING, message);
}

void Logger::error(const std::string& message) {
    writeLog(LogLevel::ERROR, message);
}

void Logger::writeLog(LogLevel level, const std::string& message) {
    if (!shouldLog(level)) return;
    
    std::string timestamp = getCurrentTimestamp();
    std::string levelStr = logLevelToString(level);
    std::string logEntry = "[" + levelStr + "] [" + timestamp + "] " + message;
    
    if (consoleOutput) {
        switch (level) {
            case LogLevel::ERROR:
                std::cout << "\033[31m" << logEntry << "\033[0m" << std::endl;
                break;
            case LogLevel::WARNING:
                std::cout << "\033[33m" << logEntry << "\033[0m" << std::endl;
                break;
            case LogLevel::DEBUG:
                std::cout << "\033[36m" << logEntry << "\033[0m" << std::endl;
                break;
            default:
                std::cout << logEntry << std::endl;
                break;
        }
    }
    
    if (logFile.is_open()) {
        logFile << logEntry << std::endl;
        logFile.flush();
    }
}

std::string Logger::getCurrentTimestamp() {
    auto now = std::chrono::system_clock::now();
    auto time_t = std::chrono::system_clock::to_time_t(now);
    
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()) % 1000;
    
    std::stringstream ss;
    ss << std::put_time(std::localtime(&time_t), "%Y-%m-%d %H:%M:%S");
    ss << "." << std::setfill('0') << std::setw(3) << ms.count();
    
    return ss.str();
}

std::string Logger::logLevelToString(LogLevel level) {
    switch (level) {
        case LogLevel::DEBUG: return "DEBUG";
        case LogLevel::INFO: return "INFO";
        case LogLevel::WARNING: return "WARNING";
        case LogLevel::ERROR: return "ERROR";
        default: return "UNKNOWN";
    }
}

bool Logger::shouldLog(LogLevel level) const {
    return static_cast<int>(level) >= static_cast<int>(currentLogLevel);
}
