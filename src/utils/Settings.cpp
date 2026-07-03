#include "finance/utils/Settings.h"

#include <fstream>

namespace finance::utils {

Settings& Settings::instance()
{
    static Settings inst;
    return inst;
}

void Settings::load()
{
    loadFrom(filePath_);
}

void Settings::loadFrom(const std::string& path)
{
    filePath_ = path;
    std::ifstream ifs(path);
    if (ifs.is_open()) {
        try {
            ifs >> data_;
        } catch (...) {
            data_ = nlohmann::json::object();
        }
    } else {
        data_ = nlohmann::json::object();
    }
}

void Settings::save() const
{
    saveTo(filePath_);
}

void Settings::saveTo(const std::string& path) const
{
    std::ofstream ofs(path);
    if (ofs.is_open()) {
        ofs << data_.dump(4) << std::endl;
    }
}

// ── Generic getters ─────────────────────────────────────────────────

std::string Settings::get(const std::string& key, const std::string& defaultValue) const
{
    auto it = data_.find(key);
    if (it != data_.end() && it->is_string()) {
        return it->get<std::string>();
    }
    return defaultValue;
}

int Settings::get(const std::string& key, int defaultValue) const
{
    auto it = data_.find(key);
    if (it != data_.end() && it->is_number_integer()) {
        return it->get<int>();
    }
    return defaultValue;
}

bool Settings::get(const std::string& key, bool defaultValue) const
{
    auto it = data_.find(key);
    if (it != data_.end() && it->is_boolean()) {
        return it->get<bool>();
    }
    return defaultValue;
}

// ── Generic setters ─────────────────────────────────────────────────

void Settings::set(const std::string& key, const std::string& value)
{
    data_[key] = value;
}

void Settings::set(const std::string& key, int value)
{
    data_[key] = value;
}

void Settings::set(const std::string& key, bool value)
{
    data_[key] = value;
}

}  // namespace finance::utils
