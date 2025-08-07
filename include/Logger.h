#pragma once
#include <string>
#include <fstream>
// #include <mutex>  // Commented out for now - we'll add threading later

/**
 * Logger class - handles all logging functionality
 * Similar to Java's logging frameworks but simpler
 * Thread-safe for concurrent access
 */
class Logger {
private:
    std::ofstream logFile;
    // std::mutex logMutex;  // Commented out for now
    bool consoleOutput;

public:
    Logger(const std::string& filename = "", bool console = true);
    ~Logger();
    
    // Different log levels (like Log4j)
    void info(const std::string& message);
    void warning(const std::string& message);
    void error(const std::string& message);
    void debug(const std::string& message);
    
private:
    void writeLog(const std::string& level, const std::string& message);
    std::string getCurrentTimestamp();
};
