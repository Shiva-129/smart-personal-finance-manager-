#ifndef FINANCE_UTILS_SETTINGS_H
#define FINANCE_UTILS_SETTINGS_H

#include <nlohmann/json.hpp>
#include <string>

namespace finance::utils {

/**
 * @brief Thread-safe singleton application settings.
 *
 * Settings are persisted to a JSON file (data/settings.json by default).
 */
class Settings {
public:
    /// Access the global settings instance.
    static Settings& instance();

    /// Load settings from the default path.
    void load();

    /// Load settings from a specific path.
    void loadFrom(const std::string& path);

    /// Save current settings to disk.
    void save() const;

    /// Save to a specific path.
    void saveTo(const std::string& path) const;

    // ── Accessors ───────────────────────────────────────────────────

    std::string dataDir()        const { return get("data_dir", std::string("data")); }
    std::string logFile()        const { return get("log_file", std::string("data/finance.log")); }
    std::string defaultCurrency() const { return get("default_currency", std::string("INR")); }
    std::string dateFormat()     const { return get("date_format", std::string("YYYY-MM-DD")); }
    int         logLevel()       const { return get("log_level", 1); }  // 0=DEBUG,1=INFO,2=WARN,3=ERROR

    /// Generic getter with default.
    std::string get(const std::string& key, const std::string& defaultValue) const;
    int         get(const std::string& key, int defaultValue) const;
    bool        get(const std::string& key, bool defaultValue) const;

    /// Generic setter.
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

#endif  // FINANCE_UTILS_SETTINGS_H
