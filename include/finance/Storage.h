#ifndef FINANCE_STORAGE_H
#define FINANCE_STORAGE_H

#include "Models.h"

#include <algorithm>
#include <filesystem>
#include <fstream>
#include <memory>
#include <mutex>
#include <nlohmann/json.hpp>
#include <optional>
#include <string>
#include <vector>

namespace finance::storage {

// ══════════════════════════════════════════════════════════════════════
// IRepository<T> — abstract interface for all repositories
// ══════════════════════════════════════════════════════════════════════

template<typename T>
class IRepository {
public:
    using Predicate = std::function<bool(const T&)>;
    virtual ~IRepository() = default;
    virtual std::vector<T> loadAll() = 0;
    virtual void saveAll(const std::vector<T>& items) = 0;
    virtual std::optional<T> findById(const std::string& id) = 0;
    virtual void save(const T& item) = 0;
    virtual bool remove(const std::string& id) = 0;
    virtual std::vector<T> find(const Predicate& predicate) = 0;
    virtual size_t count() = 0;
    virtual void reload() = 0;
    virtual void flush() = 0;
};

// ══════════════════════════════════════════════════════════════════════
// JsonRepository<T> — thread-safe JSON-file implementation
// ══════════════════════════════════════════════════════════════════════

template<typename T>
class JsonRepository : public IRepository<T> {
public:
    using Predicate = typename IRepository<T>::Predicate;

    explicit JsonRepository(std::string filePath)
        : filePath_(std::move(filePath)), loaded_(false) {}

    std::vector<T> loadAll() override {
        std::lock_guard<std::mutex> lock(mutex_);
        ensureLoaded();
        return items_;
    }

    void saveAll(const std::vector<T>& items) override {
        std::lock_guard<std::mutex> lock(mutex_);
        items_ = items;
        loaded_ = true;
        persist();
    }

    std::optional<T> findById(const std::string& id) override {
        std::lock_guard<std::mutex> lock(mutex_);
        ensureLoaded();
        auto it = std::find_if(items_.begin(), items_.end(),
            [&](const T& item) { return item.id() == id; });
        if (it != items_.end()) return *it;
        return std::nullopt;
    }

    void save(const T& item) override {
        std::lock_guard<std::mutex> lock(mutex_);
        ensureLoaded();
        auto it = std::find_if(items_.begin(), items_.end(),
            [&](const T& e) { return e.id() == item.id(); });
        if (it != items_.end()) *it = item;
        else items_.push_back(item);
        persist();
    }

    bool remove(const std::string& id) override {
        std::lock_guard<std::mutex> lock(mutex_);
        ensureLoaded();
        auto it = std::find_if(items_.begin(), items_.end(),
            [&](const T& item) { return item.id() == id; });
        if (it == items_.end()) return false;
        items_.erase(it);
        persist();
        return true;
    }

    std::vector<T> find(const Predicate& pred) override {
        std::lock_guard<std::mutex> lock(mutex_);
        ensureLoaded();
        std::vector<T> result;
        std::copy_if(items_.begin(), items_.end(), std::back_inserter(result), pred);
        return result;
    }

    size_t count() override {
        std::lock_guard<std::mutex> lock(mutex_);
        ensureLoaded();
        return items_.size();
    }

    void reload() override {
        std::lock_guard<std::mutex> lock(mutex_);
        loaded_ = false;
        loadFromDisk();
    }

    void flush() override {
        std::lock_guard<std::mutex> lock(mutex_);
        persist();
    }

private:
    void ensureLoaded() { if (!loaded_) { loadFromDisk(); loaded_ = true; } }

    void loadFromDisk() {
        items_.clear();
        std::ifstream ifs(filePath_);
        if (!ifs.is_open()) return;
        try {
            nlohmann::json j;
            ifs >> j;
            if (!j.is_array()) return;
            for (const auto& elem : j) items_.push_back(T::fromJson(elem));
        } catch (const std::exception&) { items_.clear(); }
    }

    void persist() {
        auto dir = std::filesystem::path(filePath_).parent_path();
        if (!dir.empty() && !std::filesystem::exists(dir))
            std::filesystem::create_directories(dir);
        nlohmann::json j = nlohmann::json::array();
        for (const auto& item : items_) j.push_back(item.toJson());
        std::ofstream ofs(filePath_);
        if (ofs.is_open()) ofs << j.dump(2) << std::endl;
    }

    std::string filePath_;
    std::vector<T> items_;
    bool loaded_;
    std::mutex mutex_;
};

// ══════════════════════════════════════════════════════════════════════
// StorageManager — coordinates 7 repositories
// ══════════════════════════════════════════════════════════════════════

class StorageManager {
public:
    explicit StorageManager(std::string dataDir);

    void initialize();

    // Concrete types (via IRepository<T>)
    IRepository<models::User>&         users();
    IRepository<models::Category>&     categories();
    IRepository<models::Budget>&       budgets();
    IRepository<models::Goal>&         goals();
    IRepository<models::Notification>& notifications();

    // Polymorphic types (abstract base → factory deserialization)
    std::vector<std::unique_ptr<models::Account>>       loadAccounts();
    void saveAccounts(const std::vector<std::unique_ptr<models::Account>>& accounts);
    std::vector<std::unique_ptr<models::Transaction>>   loadTransactions();
    void saveTransactions(const std::vector<std::unique_ptr<models::Transaction>>& txns);

    void flushAll();
    const std::string& dataDir() const { return dataDir_; }

private:
    std::string dataDir_;
    std::unique_ptr<IRepository<models::User>>         users_;
    std::unique_ptr<IRepository<models::Category>>     categories_;
    std::unique_ptr<IRepository<models::Budget>>       budgets_;
    std::unique_ptr<IRepository<models::Goal>>         goals_;
    std::unique_ptr<IRepository<models::Notification>> notifications_;
};

}  // namespace finance::storage

#endif  // FINANCE_STORAGE_H
