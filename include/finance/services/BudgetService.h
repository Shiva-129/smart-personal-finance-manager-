#ifndef FINANCE_SERVICES_BUDGETSERVICE_H
#define FINANCE_SERVICES_BUDGETSERVICE_H

#include "finance/models/Budget.h"
#include "finance/models/Transaction.h"
#include "finance/services/NotificationService.h"
#include "finance/storage/StorageManager.h"

#include <memory>
#include <string>
#include <vector>

namespace finance::services {

/**
 * @brief Manages monthly budgets per category and generates alerts.
 *
 * Acts as an Observer: when TransactionService records an expense,
 * it calls checkBudget() which updates spending totals and fires
 * a notification if the budget is exceeded.
 */
class BudgetService {
public:
    BudgetService(storage::StorageManager& storage,
                   NotificationService& notificationService);

    /// Set (or update) a monthly budget for a category.
    models::Budget setBudget(const std::string& userId,
                              const std::string& categoryId,
                              int year, int month,
                              double limitAmount);

    /// Remove a budget.
    void removeBudget(const std::string& budgetId);

    /// Called after an expense is added — updates spent amount and
    /// fires a notification if the budget is exceeded.
    void checkBudget(const models::Transaction& expenseTxn);

    /// Get all budgets for a user in a given month.
    std::vector<models::Budget> getBudgets(const std::string& userId,
                                            int year, int month) const;

    /// Get a specific budget (by category, user, month).
    std::optional<models::Budget> getBudget(const std::string& userId,
                                             const std::string& categoryId,
                                             int year, int month) const;

    /// Recalculate spent amounts for all budgets of a user in a month
    /// from the transaction history (e.g. after editing/deleting a txn).
    void recalculate(const std::string& userId, int year, int month);

private:
    storage::StorageManager& storage_;
    NotificationService&     notificationService_;
};

}  // namespace finance::services

#endif  // FINANCE_SERVICES_BUDGETSERVICE_H
