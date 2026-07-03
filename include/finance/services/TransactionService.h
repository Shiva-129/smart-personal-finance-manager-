#ifndef FINANCE_SERVICES_TRANSACTIONSERVICE_H
#define FINANCE_SERVICES_TRANSACTIONSERVICE_H

#include "finance/models/Transaction.h"
#include "finance/services/AccountService.h"
#include "finance/services/BudgetService.h"
#include "finance/services/GoalService.h"
#include "finance/services/NotificationService.h"
#include "finance/storage/StorageManager.h"

#include <memory>
#include <stack>
#include <string>
#include <vector>

namespace finance::services {

/**
 * @brief Business logic for financial transactions.
 *
 * Supports full CRUD, undo (via a Command-pattern undo stack), and
 * automatic generation of recurring entries.
 *
 * Notifies BudgetService and GoalService when relevant transactions
 * are created.
 */
class TransactionService {
public:
    TransactionService(storage::StorageManager& storage,
                        AccountService& accountService,
                        BudgetService& budgetService,
                        GoalService& goalService,
                        NotificationService& notificationService);

    // ── CRUD ────────────────────────────────────────────────────────

    /// Add an income transaction.  Optionally allocate to a savings goal.
    models::Transaction* addIncome(const std::string& userId,
                                    double amount,
                                    const std::string& description,
                                    const std::string& categoryId,
                                    const std::string& accountId,
                                    const std::string& date = {},
                                    const std::string& goalId = {},
                                    const std::vector<std::string>& tags = {});

    /// Add an expense transaction.  Checks budgets automatically.
    models::Transaction* addExpense(const std::string& userId,
                                     double amount,
                                     const std::string& description,
                                     const std::string& categoryId,
                                     const std::string& accountId,
                                     const std::string& date = {},
                                     bool isRecurring = false,
                                     const std::string& recurrenceRule = {},
                                     const std::vector<std::string>& tags = {});

    /// Add a transfer between two accounts.
    models::Transaction* addTransfer(const std::string& userId,
                                      double amount,
                                      const std::string& description,
                                      const std::string& fromAccountId,
                                      const std::string& toAccountId,
                                      const std::string& date = {});

    /// Edit an existing transaction.
    void editTransaction(const std::string& transactionId,
                          double newAmount,
                          const std::string& newDescription,
                          const std::string& newCategoryId);

    /// Delete a transaction (pushes onto undo stack).
    void deleteTransaction(const std::string& transactionId);

    /// Undo the most recent delete.
    bool undoDelete();

    // ── Queries ─────────────────────────────────────────────────────

    std::vector<models::Transaction*> getTransactionsByUser(
        const std::string& userId) const;

    std::vector<models::Transaction*> getTransactionsByAccount(
        const std::string& accountId) const;

    std::vector<models::Transaction*> getTransactionsByCategory(
        const std::string& categoryId) const;

    models::Transaction* getTransaction(const std::string& txnId) const;

    // ── Recurring ───────────────────────────────────────────────────

    /// Check all recurring expenses and auto-generate due entries.
    size_t processRecurring();

private:
    void loadTransactions();
    void saveTransactions();

    storage::StorageManager&                       storage_;
    AccountService&                                accountService_;
    BudgetService&                                 budgetService_;
    GoalService&                                   goalService_;
    NotificationService&                           notificationService_;

    std::vector<std::unique_ptr<models::Transaction>> txns_;

    /// Undo stack for deleted transactions (Command pattern).
    std::stack<std::unique_ptr<models::Transaction>> undoStack_;
};

}  // namespace finance::services

#endif  // FINANCE_SERVICES_TRANSACTIONSERVICE_H
