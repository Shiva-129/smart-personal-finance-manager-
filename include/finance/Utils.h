#ifndef FINANCE_UTILS_H
#define FINANCE_UTILS_H

// ══════════════════════════════════════════════════════════════════════
// Exceptions
// ══════════════════════════════════════════════════════════════════════

#include <chrono>
#include <fstream>
#include <mutex>
#include <nlohmann/json.hpp>
#include <stdexcept>
#include <string>
#include <tuple>

namespace finance::utils {

class FinanceException : public std::runtime_error {
public:
    explicit FinanceException(const std::string& msg) : std::runtime_error(msg) {}
};
class InvalidInputException : public FinanceException {
public:
    explicit InvalidInputException(const std::string& d)
        : FinanceException("Invalid input: " + d) {}
};
class NegativeBalanceException : public FinanceException {
public:
    explicit NegativeBalanceException(const std::string& n)
        : FinanceException("Insufficient funds: " + n) {}
};
class NotFoundException : public FinanceException {
public:
    explicit NotFoundException(const std::string& t, const std::string& id = "")
        : FinanceException("Not found: " + t + (id.empty() ? "" : " [" + id + "]")) {}
};
class DuplicateException : public FinanceException {
public:
    explicit DuplicateException(const std::string& t, const std::string& n)
        : FinanceException("Duplicate " + t + ": " + n) {}
};
class AuthenticationException : public FinanceException {
public:
    explicit AuthenticationException(const std::string& d)
        : FinanceException("Authentication failed: " + d) {}
};
class CreditLimitException : public FinanceException {
public:
    explicit CreditLimitException(const std::string& n)
        : FinanceException("Credit limit exceeded: " + n) {}
};
class FileIOException : public FinanceException {
public:
    explicit FileIOException(const std::string& p)
        : FinanceException("File I/O error: " + p) {}
};
class CorruptedDataException : public FinanceException {
public:
    explicit CorruptedDataException(const std::string& d)
        : FinanceException("Corrupted data: " + d) {}
};

// ══════════════════════════════════════════════════════════════════════
// UUID v4 Generator
// ══════════════════════════════════════════════════════════════════════

class UUID {
public:
    static std::string generate();
    static bool isValid(const std::string& uuid);
};

// ══════════════════════════════════════════════════════════════════════
// Date / Time Utilities
// ══════════════════════════════════════════════════════════════════════

class DateUtils {
public:
    using Clock     = std::chrono::system_clock;
    using TimePoint = Clock::time_point;

    static std::string today();
    static std::string now();
    static std::string timestamp();
    static std::string formatDate(const TimePoint& tp);
    static std::string formatTime(const TimePoint& tp);
    static std::string formatTimestamp(const TimePoint& tp);
    static TimePoint   parseDate(const std::string& dateStr);
    static TimePoint   parseTimestamp(const std::string& ts);
    static std::tuple<int, int, int> extractYMD(const std::string& dateStr);
    static std::string monthName(int month);
    static int         daysBetween(const std::string& start, const std::string& end);
    static bool        isFuture(const std::string& dateStr);
    static bool        isPast(const std::string& dateStr);
};

// ══════════════════════════════════════════════════════════════════════
// Logger (Singleton, thread-safe)
// ══════════════════════════════════════════════════════════════════════

enum class LogLevel { DEBUG, INFO, WARNING, ERROR };

class Logger {
public:
    static Logger& instance();
    void setLevel(LogLevel level);
    void setLogFile(const std::string& filePath);
    void log(LogLevel level, const std::string& message);
    void debug(const std::string& msg) { log(LogLevel::DEBUG, msg); }
    void info(const std::string& msg)  { log(LogLevel::INFO, msg); }
    void warn(const std::string& msg)  { log(LogLevel::WARNING, msg); }
    void error(const std::string& msg) { log(LogLevel::ERROR, msg); }

    Logger(const Logger&) = delete;
    Logger& operator=(const Logger&) = delete;

private:
    Logger() = default;
    ~Logger();

    static std::string levelToString(LogLevel level);
    LogLevel      level_ = LogLevel::INFO;
    std::ofstream file_;
    std::mutex    mutex_;
};

// ══════════════════════════════════════════════════════════════════════
// Settings (Singleton, JSON-backed)
// ══════════════════════════════════════════════════════════════════════

class Settings {
public:
    static Settings& instance();

    void load();
    void loadFrom(const std::string& path);
    void save() const;
    void saveTo(const std::string& path) const;

    std::string dataDir()         const { return get("data_dir",        std::string("data")); }
    std::string logFile()         const { return get("log_file",        std::string("data/finance.log")); }
    std::string defaultCurrency() const { return get("default_currency", std::string("INR")); }
    std::string dateFormat()      const { return get("date_format",     std::string("YYYY-MM-DD")); }
    int         logLevel()        const { return get("log_level", 1); }

    std::string get(const std::string& key, const std::string& def) const;
    int  get(const std::string& key, int def) const;
    bool get(const std::string& key, bool def) const;
    void set(const std::string& key, const std::string& value);
    void set(const std::string& key, int value);
    void set(const std::string& key, bool value);

    Settings(const Settings&) = delete;
    Settings& operator=(const Settings&) = delete;

private:
    Settings() = default;
    std::string     filePath_ = "data/settings.json";
    nlohmann::json  data_ = nlohmann::json::object();
};

}  // namespace finance::utils

#endif  // FINANCE_UTILS_H
