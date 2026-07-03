#ifndef FINANCE_STORAGE_STORAGEMANAGER_H
#define FINANCE_STORAGE_STORAGEMANAGER_H

#include "finance/models/Account.h"
#include "finance/models/Budget.h"
#include "finance/models/Category.h"
#include "finance/models/Goal.h"
#include "finance/models/Notification.h"
#include "finance/models/Transaction.h"
#include "finance/models/User.h"
#include "finance/storage/IRepository.h"
#include "finance/storage/JsonRepository.h"

#include <memory>
#include <string>
#include <vector>

namespace finance::storage {

/**
 * @brief Coordinates all repositories.
 *
 * Provides a single point of access for loading / saving every entity type.
 * Each repository is lazily created on first access.
 *
 * Concrete types (User, Category, Budget, Goal, Notification) use the
 * generic JsonRepository<T>.  Polymorphic types (Account, Transaction)
 * are serialised directly via their factory methods since they cannot
 * be stored by value.
 *
 * To swap JSON storage for SQLite, replace the JsonRepository
 * construction in each accessor — callers only depend on IRepository<T>.
 */
class StorageManager {
public:
    explicit StorageManager(std::string dataDir);

    /// Initialise the data directory.
    void initialize();

    // ── Concrete-type repositories (via IRepository<T>) ────────────
    IRepository<models::User>&         users();
    IRepository<models::Category>&     categories();
    IRepository<models::Budget>&       budgets();
    IRepository<models::Goal>&         goals();
    IRepository<models::Notification>& notifications();

    // ── Polymorphic-type storage (Account / Transaction) ────────────
    // These types are abstract; their factories produce derived
    // instances, so we store/load raw JSON arrays through helpers.

    std::vector<std::unique_ptr<models::Account>>       loadAccounts();
    void                                                saveAccounts(
        const std::vector<std::unique_ptr<models::Account>>& accounts);

    std::vector<std::unique_ptr<models::Transaction>>   loadTransactions();
    void                                                saveTransactions(
        const std::vector<std::unique_ptr<models::Transaction>>& txns);

    /// Persist all repositories to disk.
    void flushAll();

    /// Get the data directory path.
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

#endif  // FINANCE_STORAGE_STORAGEMANAGER_H
