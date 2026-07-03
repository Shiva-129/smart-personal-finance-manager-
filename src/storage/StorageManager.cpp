#include "finance/storage/StorageManager.h"
#include "finance/utils/Logger.h"

#include <filesystem>
#include <fstream>

namespace finance::storage {

StorageManager::StorageManager(std::string dataDir)
    : dataDir_(std::move(dataDir))
{
}

void StorageManager::initialize()
{
    namespace fs = std::filesystem;

    if (!fs::exists(dataDir_)) {
        fs::create_directories(dataDir_);
    }

    auto& log = utils::Logger::instance();
    log.info("Storage initialized at: " + dataDir_);
}

// ── Concrete-type repository accessors (lazy creation) ──────────────

IRepository<models::User>& StorageManager::users()
{
    if (!users_) {
        users_ = std::make_unique<JsonRepository<models::User>>(
            dataDir_ + "/users.json");
    }
    return *users_;
}

IRepository<models::Category>& StorageManager::categories()
{
    if (!categories_) {
        categories_ = std::make_unique<JsonRepository<models::Category>>(
            dataDir_ + "/categories.json");
    }
    return *categories_;
}

IRepository<models::Budget>& StorageManager::budgets()
{
    if (!budgets_) {
        budgets_ = std::make_unique<JsonRepository<models::Budget>>(
            dataDir_ + "/budgets.json");
    }
    return *budgets_;
}

IRepository<models::Goal>& StorageManager::goals()
{
    if (!goals_) {
        goals_ = std::make_unique<JsonRepository<models::Goal>>(
            dataDir_ + "/goals.json");
    }
    return *goals_;
}

IRepository<models::Notification>& StorageManager::notifications()
{
    if (!notifications_) {
        notifications_ = std::make_unique<JsonRepository<models::Notification>>(
            dataDir_ + "/notifications.json");
    }
    return *notifications_;
}

// ── Polymorphic-type storage helpers ─────────────────────────────────

std::vector<std::unique_ptr<models::Account>> StorageManager::loadAccounts()
{
    std::string path = dataDir_ + "/accounts.json";
    std::vector<std::unique_ptr<models::Account>> result;

    std::ifstream ifs(path);
    if (!ifs.is_open()) return result;

    try {
        nlohmann::json j;
        ifs >> j;
        if (!j.is_array()) return result;

        for (const auto& elem : j) {
            auto acc = models::Account::fromJson(elem);
            if (acc) result.push_back(std::move(acc));
        }
    } catch (const std::exception&) {
        // Corrupted file — return empty.
    }

    return result;
}

void StorageManager::saveAccounts(
    const std::vector<std::unique_ptr<models::Account>>& accounts)
{
    std::string path = dataDir_ + "/accounts.json";

    // Ensure directory exists.
    std::filesystem::path dir = std::filesystem::path(path).parent_path();
    if (!dir.empty() && !std::filesystem::exists(dir)) {
        std::filesystem::create_directories(dir);
    }

    nlohmann::json j = nlohmann::json::array();
    for (const auto& acc : accounts) {
        j.push_back(acc->toJson());
    }

    std::ofstream ofs(path);
    if (ofs.is_open()) {
        ofs << j.dump(2) << std::endl;
    }
}

std::vector<std::unique_ptr<models::Transaction>> StorageManager::loadTransactions()
{
    std::string path = dataDir_ + "/transactions.json";
    std::vector<std::unique_ptr<models::Transaction>> result;

    std::ifstream ifs(path);
    if (!ifs.is_open()) return result;

    try {
        nlohmann::json j;
        ifs >> j;
        if (!j.is_array()) return result;

        for (const auto& elem : j) {
            auto txn = models::Transaction::fromJson(elem);
            if (txn) result.push_back(std::move(txn));
        }
    } catch (const std::exception&) {
        // Corrupted file — return empty.
    }

    return result;
}

void StorageManager::saveTransactions(
    const std::vector<std::unique_ptr<models::Transaction>>& txns)
{
    std::string path = dataDir_ + "/transactions.json";

    std::filesystem::path dir = std::filesystem::path(path).parent_path();
    if (!dir.empty() && !std::filesystem::exists(dir)) {
        std::filesystem::create_directories(dir);
    }

    nlohmann::json j = nlohmann::json::array();
    for (const auto& txn : txns) {
        j.push_back(txn->toJson());
    }

    std::ofstream ofs(path);
    if (ofs.is_open()) {
        ofs << j.dump(2) << std::endl;
    }
}

void StorageManager::flushAll()
{
    if (users_)         users_->flush();
    if (categories_)    categories_->flush();
    if (budgets_)       budgets_->flush();
    if (goals_)         goals_->flush();
    if (notifications_) notifications_->flush();

    // Polymorphic types are written immediately on save — nothing to flush.
}

}  // namespace finance::storage
