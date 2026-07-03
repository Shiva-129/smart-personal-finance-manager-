#ifndef FINANCE_SERVICES_ACCOUNTSERVICE_H
#define FINANCE_SERVICES_ACCOUNTSERVICE_H

#include "finance/models/Account.h"
#include "finance/models/Transaction.h"
#include "finance/storage/StorageManager.h"

#include <memory>
#include <string>
#include <vector>

namespace finance::services {

/**
 * @brief Business logic for account management.
 *
 * Handles creating, modifying, freezing, closing accounts, and
 * transferring money between them.
 */
class AccountService {
public:
    explicit AccountService(storage::StorageManager& storage);

    /// Create a new account of the given type.
    /// type: "Cash", "Savings", "Current", "Credit Card", "Wallet"
    models::Account* createAccount(const std::string& userId,
                                    const std::string& type,
                                    const std::string& name,
                                    const std::string& currency = "INR");

    /// Delete an account and all its transactions.
    void deleteAccount(const std::string& accountId);

    /// Freeze an account (prevents transactions).
    void freezeAccount(const std::string& accountId);

    /// Close an account (irreversible).
    void closeAccount(const std::string& accountId);

    /// Transfer money between two accounts.  Creates two Transfer transactions.
    void transfer(const std::string& fromAccountId,
                   const std::string& toAccountId,
                   double amount,
                   const std::string& description = {});

    /// Get all accounts owned by a user.
    std::vector<models::Account*> getAccountsByUser(
        const std::string& userId);

    /// Get a single account by id (returns null if not found).
    models::Account* getAccount(const std::string& accountId);

    /// Get all accounts (for reports, etc.).
    const std::vector<std::unique_ptr<models::Account>>& allAccounts() const;

    /// Persist the in-memory account list to storage.
    /// Public so that TransactionService (and others) can persist
    /// balance changes made through getAccount() -> deposit/withdraw.
    void saveAccounts();

private:
    /// Reload the in-memory account list from storage.
    void loadAccounts();

    storage::StorageManager&                       storage_;
    std::vector<std::unique_ptr<models::Account>>  accounts_;
};

}  // namespace finance::services

#endif  // FINANCE_SERVICES_ACCOUNTSERVICE_H
