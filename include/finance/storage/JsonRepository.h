#ifndef FINANCE_STORAGE_JSONREPOSITORY_H
#define FINANCE_STORAGE_JSONREPOSITORY_H

#include "finance/storage/IRepository.h"

#include <algorithm>
#include <filesystem>
#include <fstream>
#include <mutex>
#include <nlohmann/json.hpp>
#include <optional>
#include <string>
#include <vector>

namespace finance::storage {

/**
 * @brief JSON-file-backed implementation of IRepository<T>.
 *
 * Thread-safe: all public methods acquire the internal mutex.
 * Lazy-loaded: the file is only read on first access.
 */
template<typename T>
class JsonRepository : public IRepository<T> {
public:
    using Predicate = typename IRepository<T>::Predicate;

    explicit JsonRepository(std::string filePath)
        : filePath_(std::move(filePath))
        , loaded_(false)
    {
    }

    // ── IRepository implementation ──────────────────────────────────

    std::vector<T> loadAll() override
    {
        std::lock_guard<std::mutex> lock(mutex_);
        ensureLoaded();
        return items_;   // copy under lock
    }

    void saveAll(const std::vector<T>& items) override
    {
        std::lock_guard<std::mutex> lock(mutex_);
        items_ = items;
        loaded_ = true;
        persist();
    }

    std::optional<T> findById(const std::string& id) override
    {
        std::lock_guard<std::mutex> lock(mutex_);
        ensureLoaded();
        auto it = std::find_if(items_.begin(), items_.end(),
                               [&](const T& item) { return item.id() == id; });
        if (it != items_.end()) return *it;  // copy under lock
        return std::nullopt;
    }

    void save(const T& item) override
    {
        std::lock_guard<std::mutex> lock(mutex_);
        ensureLoaded();

        auto it = std::find_if(items_.begin(), items_.end(),
                               [&](const T& existing) { return existing.id() == item.id(); });
        if (it != items_.end()) {
            *it = item;
        } else {
            items_.push_back(item);
        }
        persist();
    }

    bool remove(const std::string& id) override
    {
        std::lock_guard<std::mutex> lock(mutex_);
        ensureLoaded();

        auto it = std::find_if(items_.begin(), items_.end(),
                               [&](const T& item) { return item.id() == id; });
        if (it == items_.end()) return false;

        items_.erase(it);
        persist();
        return true;
    }

    std::vector<T> find(const Predicate& predicate) override
    {
        std::lock_guard<std::mutex> lock(mutex_);
        ensureLoaded();

        std::vector<T> result;
        std::copy_if(items_.begin(), items_.end(),
                     std::back_inserter(result), predicate);
        return result;  // copy under lock
    }

    size_t count() override
    {
        std::lock_guard<std::mutex> lock(mutex_);
        ensureLoaded();
        return items_.size();
    }

    void reload() override
    {
        std::lock_guard<std::mutex> lock(mutex_);
        loaded_ = false;
        loadFromDisk();
    }

    void flush() override
    {
        std::lock_guard<std::mutex> lock(mutex_);
        persist();
    }

private:
    void ensureLoaded()
    {
        if (!loaded_) {
            loadFromDisk();
            loaded_ = true;
        }
    }

    void loadFromDisk()
    {
        items_.clear();
        std::ifstream ifs(filePath_);
        if (!ifs.is_open()) return;

        try {
            nlohmann::json j;
            ifs >> j;
            if (!j.is_array()) return;

            for (const auto& elem : j) {
                items_.push_back(T::fromJson(elem));
            }
        } catch (const std::exception&) {
            items_.clear();
        }
    }

    void persist()
    {
        // Ensure parent directory exists — C++17 filesystem API.
        std::filesystem::path dir = std::filesystem::path(filePath_).parent_path();
        if (!dir.empty() && !std::filesystem::exists(dir)) {
            std::filesystem::create_directories(dir);
        }

        nlohmann::json j = nlohmann::json::array();
        for (const auto& item : items_) {
            j.push_back(item.toJson());
        }

        std::ofstream ofs(filePath_);
        if (ofs.is_open()) {
            ofs << j.dump(2) << std::endl;
        }
    }

    std::string         filePath_;
    std::vector<T>      items_;
    bool                loaded_;
    std::mutex          mutex_;
};

}  // namespace finance::storage

#endif  // FINANCE_STORAGE_JSONREPOSITORY_H
