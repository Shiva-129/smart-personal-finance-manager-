#include "finance/utils/Logger.h"

#include <algorithm>
#include <chrono>
#include <ctime>
#include <iomanip>
#include <iostream>
#include <sstream>

namespace finance::utils {

Logger& Logger::instance()
{
    static Logger inst;
    return inst;
}

Logger::~Logger()
{
    if (file_.is_open()) {
        file_.close();
    }
}

void Logger::setLevel(LogLevel level)
{
    std::lock_guard<std::mutex> lock(mutex_);
    level_ = level;
}

void Logger::setLogFile(const std::string& filePath)
{
    std::lock_guard<std::mutex> lock(mutex_);
    if (file_.is_open()) {
        file_.close();
    }
    file_.open(filePath, std::ios::app);
}

void Logger::log(LogLevel level, const std::string& message)
{
    std::lock_guard<std::mutex> lock(mutex_);
    if (level < level_) return;

    // Build timestamp.
    auto now = std::chrono::system_clock::now();
    auto tt  = std::chrono::system_clock::to_time_t(now);
    auto tm  = *std::localtime(&tt);

    std::ostringstream oss;
    oss << std::put_time(&tm, "%Y-%m-%dT%H:%M:%S")
        << " [" << levelToString(level) << "] " << message;

    std::string formatted = oss.str();

    // Console.
    std::cout << formatted << std::endl;

    // File.
    if (file_.is_open()) {
        file_ << formatted << std::endl;
    }
}

std::string Logger::levelToString(LogLevel level)
{
    switch (level) {
        case LogLevel::DEBUG:   return "DEBUG";
        case LogLevel::INFO:    return "INFO";
        case LogLevel::WARNING: return "WARN";
        case LogLevel::ERROR:   return "ERROR";
    }
    return "UNKNOWN";
}

}  // namespace finance::utils
