#pragma once
#include <string>
#include <fstream>

enum class LogLevel {
    DEBUG,
    INFO,
    WARNING,
    ERROR
};

class Config;

class Logger {
private:
    std::ofstream logFile;
    bool consoleOutput;
    LogLevel currentLogLevel;

public:
    Logger(const std::string& filename = "", bool console = true, LogLevel level = LogLevel::INFO);
    ~Logger();
    
    void configure(const Config& config);
    
    void debug(const std::string& message);
    void info(const std::string& message);
    void warning(const std::string& message);
    void error(const std::string& message);
    
    void setLogLevel(LogLevel level) { currentLogLevel = level; }
    LogLevel getLogLevel() const { return currentLogLevel; }
    
private:
    void writeLog(LogLevel level, const std::string& message);
    std::string getCurrentTimestamp();
    std::string logLevelToString(LogLevel level);
    bool shouldLog(LogLevel level) const;
};
