#include "finance/services/BudgetService.h"
#include "finance/utils/DateUtils.h"
#include "finance/utils/Exceptions.h"
#include "finance/utils/UUID.h"

#include <algorithm>
#include <numeric>
#include <sstream>

namespace finance::services {

BudgetService::BudgetService(storage::StorageManager& storage,
                               NotificationService& notificationService)
    : storage_(storage)
    , notificationService_(notificationService)
{
}

models::Budget BudgetService::setBudget(
    const std::string& userId, const std::string& categoryId,
    int year, int month, double limitAmount)
{
    if (limitAmount <= 0) {
        throw utils::InvalidInputException("Budget limit must be positive");
    }
    if (month < 1 || month > 12) {
        throw utils::InvalidInputException("Invalid month");
    }

    // Check if a budget already exists for this category/month.
    auto existing = storage_.budgets().find(
        [&](const models::Budget& b) {
            return b.userId() == userId &&
                   b.categoryId() == categoryId &&
                   b.year() == year &&
                   b.month() == month;
        });

    if (!existing.empty()) {
        // Update existing.
        auto budget = existing.front();
        budget.setLimitAmount(limitAmount);
        storage_.budgets().save(budget);
        return budget;
    }

    // Create new.
    std::string id = utils::UUID::generate();
    models::Budget budget(id, categoryId, userId, year, month, limitAmount);
    storage_.budgets().save(budget);
    return budget;
}

void BudgetService::removeBudget(const std::string& budgetId)
{
    if (!storage_.budgets().remove(budgetId)) {
        throw utils::NotFoundException("Budget", budgetId);
    }
}

void BudgetService::checkBudget(const models::Transaction& expenseTxn)
{
    // Extract year and month from the transaction date.
    auto [year, month, day] = utils::DateUtils::extractYMD(expenseTxn.date());

    auto opt = getBudget(expenseTxn.userId(), expenseTxn.categoryId(), year, month);
    if (!opt) return;  // No budget set for this category.

    auto budget = *opt;
    budget.addSpending(expenseTxn.amount());
    storage_.budgets().save(budget);

    // Notify if exceeded.
    if (budget.isExceeded()) {
        std::string msg = "Budget exceeded for category " +
                          expenseTxn.categoryId() + ": " +
                          std::to_string(static_cast<int>(budget.percentageUsed())) +
                          "% used (" +
                          std::to_string(static_cast<int>(budget.spentAmount())) +
                          " / " +
                          std::to_string(static_cast<int>(budget.limitAmount())) +
                          ")";
        notificationService_.notify(
            expenseTxn.userId(),
            models::NotificationType::BUDGET_EXCEEDED,
            msg);
    } else if (budget.percentageUsed() >= 80.0) {
        // Warn at 80%.
        std::string msg = "Budget warning for category " +
                          expenseTxn.categoryId() + ": " +
                          std::to_string(static_cast<int>(budget.percentageUsed())) +
                          "% used";
        notificationService_.notify(
            expenseTxn.userId(),
            models::NotificationType::BUDGET_EXCEEDED,
            msg);
    }
}

std::vector<models::Budget> BudgetService::getBudgets(
    const std::string& userId, int year, int month) const
{
    return storage_.budgets().find(
        [&](const models::Budget& b) {
            return b.userId() == userId &&
                   b.year() == year &&
                   b.month() == month;
        });
}

std::optional<models::Budget> BudgetService::getBudget(
    const std::string& userId, const std::string& categoryId,
    int year, int month) const
{
    auto results = storage_.budgets().find(
        [&](const models::Budget& b) {
            return b.userId() == userId &&
                   b.categoryId() == categoryId &&
                   b.year() == year &&
                   b.month() == month;
        });
    if (results.empty()) return std::nullopt;
    return results.front();
}

void BudgetService::recalculate(const std::string& userId, int year, int month)
{
    // Get all budgets for this user/month.
    auto budgets = getBudgets(userId, year, month);

    // Get all expenses for this user/month.
    auto allTxns = storage_.loadTransactions();
    double totalSpent = 0.0;

    for (const auto& txn : allTxns) {
        auto [ty, tm, td] = utils::DateUtils::extractYMD(txn->date());
        if (txn->userId() == userId && txn->transactionTypeName() == "Expense" &&
            ty == year && tm == month) {
            // Find matching budget.
            for (auto& b : budgets) {
                if (b.categoryId() == txn->categoryId()) {
                    b.setSpentAmount(b.spentAmount() + txn->amount());
                }
            }
        }
    }

    // Save updated budgets.
    for (const auto& b : budgets) {
        storage_.budgets().save(b);
    }
}

}  // namespace finance::services
