#ifndef FINANCE_UTILS_LOGGER_H
#define FINANCE_UTILS_LOGGER_H

#include <fstream>
#include <mutex>
#include <string>

namespace finance::utils {

/// Log severity levels.
enum class LogLevel {
    DEBUG,
    INFO,
    WARNING,
    ERROR
};

/**
 * @brief Thread-safe singleton logger.
 *
 * Writes timestamped messages to the console and optionally to a log file.
 */
class Logger {
public:
    /// Access the global logger instance.
    static Logger& instance();

    /// Set the minimum log level (messages below this level are suppressed).
    void setLevel(LogLevel level);

    /// Enable / disable file logging.
    void setLogFile(const std::string& filePath);

    /// Log a message at the given level.
    void log(LogLevel level, const std::string& message);

    /// Convenience wrappers.
    void debug(const std::string& msg) { log(LogLevel::DEBUG, msg); }
    void info(const std::string& msg)  { log(LogLevel::INFO, msg); }
    void warn(const std::string& msg)  { log(LogLevel::WARNING, msg); }
    void error(const std::string& msg) { log(LogLevel::ERROR, msg); }

    // Non-copyable.
    Logger(const Logger&) = delete;
    Logger& operator=(const Logger&) = delete;

private:
    Logger() = default;
    ~Logger();

    static std::string levelToString(LogLevel level);

    LogLevel        level_ = LogLevel::INFO;
    std::ofstream   file_;
    std::mutex      mutex_;
};

}  // namespace finance::utils

#endif  // FINANCE_UTILS_LOGGER_H
