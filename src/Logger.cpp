#include "Logger.h"
#include <iostream>
#include <iomanip>
#include <chrono>
#include <sstream>

Logger::Logger(const std::string& filename, bool console) 
    : consoleOutput(console) {
    
    if (!filename.empty()) {
        logFile.open(filename, std::ios::app);
        
        if (!logFile.is_open()) {
            std::cerr << "Warning: Could not open log file: " << filename << std::endl;
        }
    }
}

Logger::~Logger() {
}

void Logger::info(const std::string& message) {
    writeLog("INFO", message);
}

void Logger::warning(const std::string& message) {
    writeLog("WARNING", message);
}

void Logger::error(const std::string& message) {
    writeLog("ERROR", message);
}

void Logger::debug(const std::string& message) {
    writeLog("DEBUG", message);
}

void Logger::writeLog(const std::string& level, const std::string& message) {
    std::string timestamp = getCurrentTimestamp();
    std::string logEntry = "[" + level + "] [" + timestamp + "] " + message;
    
    if (consoleOutput) {
        std::cout << logEntry << std::endl;
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
