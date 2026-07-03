#include "finance/Storage.h"
#include "finance/Utils.h"

#include <filesystem>
#include <fstream>

namespace finance::storage {

StorageManager::StorageManager(std::string dataDir) : dataDir_(std::move(dataDir)) {}

void StorageManager::initialize() {
    namespace fs = std::filesystem;
    if (!fs::exists(dataDir_)) fs::create_directories(dataDir_);
    utils::Logger::instance().info("Storage initialized at: " + dataDir_);
}

// ── Concrete-type repos (lazy creation) ─────────────────────────────

IRepository<models::User>& StorageManager::users() {
    if (!users_) users_=std::make_unique<JsonRepository<models::User>>(dataDir_+"/users.json"); return *users_;
}
IRepository<models::Category>& StorageManager::categories() {
    if (!categories_) categories_=std::make_unique<JsonRepository<models::Category>>(dataDir_+"/categories.json"); return *categories_;
}
IRepository<models::Budget>& StorageManager::budgets() {
    if (!budgets_) budgets_=std::make_unique<JsonRepository<models::Budget>>(dataDir_+"/budgets.json"); return *budgets_;
}
IRepository<models::Goal>& StorageManager::goals() {
    if (!goals_) goals_=std::make_unique<JsonRepository<models::Goal>>(dataDir_+"/goals.json"); return *goals_;
}
IRepository<models::Notification>& StorageManager::notifications() {
    if (!notifications_) notifications_=std::make_unique<JsonRepository<models::Notification>>(dataDir_+"/notifications.json"); return *notifications_;
}

// ── Polymorphic types (Account / Transaction) ───────────────────────

static std::string ensureDir(const std::string& path) {
    auto dir=std::filesystem::path(path).parent_path();
    if(!dir.empty()&&!std::filesystem::exists(dir)) std::filesystem::create_directories(dir);
    return path;
}

std::vector<std::unique_ptr<models::Account>> StorageManager::loadAccounts() {
    std::vector<std::unique_ptr<models::Account>> r;
    std::ifstream ifs(dataDir_+"/accounts.json"); if(!ifs.is_open()) return r;
    try { nlohmann::json j; ifs>>j; if(!j.is_array()) return r;
        for(auto& e:j){auto a=models::Account::fromJson(e); if(a)r.push_back(std::move(a));} }
    catch(const std::exception&){}
    return r;
}

void StorageManager::saveAccounts(const std::vector<std::unique_ptr<models::Account>>& accs) {
    auto p=ensureDir(dataDir_+"/accounts.json"); nlohmann::json j=nlohmann::json::array();
    for(auto& a:accs) j.push_back(a->toJson());
    std::ofstream ofs(p); if(ofs.is_open()) ofs<<j.dump(2)<<std::endl;
}

std::vector<std::unique_ptr<models::Transaction>> StorageManager::loadTransactions() {
    std::vector<std::unique_ptr<models::Transaction>> r;
    std::ifstream ifs(dataDir_+"/transactions.json"); if(!ifs.is_open()) return r;
    try { nlohmann::json j; ifs>>j; if(!j.is_array()) return r;
        for(auto& e:j){auto t=models::Transaction::fromJson(e); if(t)r.push_back(std::move(t));} }
    catch(const std::exception&){}
    return r;
}

void StorageManager::saveTransactions(const std::vector<std::unique_ptr<models::Transaction>>& txns) {
    auto p=ensureDir(dataDir_+"/transactions.json"); nlohmann::json j=nlohmann::json::array();
    for(auto& t:txns) j.push_back(t->toJson());
    std::ofstream ofs(p); if(ofs.is_open()) ofs<<j.dump(2)<<std::endl;
}

void StorageManager::flushAll() {
    if(users_) users_->flush(); if(categories_)categories_->flush();
    if(budgets_)budgets_->flush(); if(goals_)goals_->flush();
    if(notifications_)notifications_->flush();
}

}  // namespace finance::storage
