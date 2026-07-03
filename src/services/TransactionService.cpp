#include "finance/services/TransactionService.h"
#include "finance/utils/DateUtils.h"
#include "finance/utils/Exceptions.h"
#include "finance/utils/UUID.h"

#include <algorithm>
#include <sstream>

namespace finance::services {

TransactionService::TransactionService(
    storage::StorageManager& storage,
    AccountService& accountService,
    BudgetService& budgetService,
    GoalService& goalService,
    NotificationService& notificationService)
    : storage_(storage)
    , accountService_(accountService)
    , budgetService_(budgetService)
    , goalService_(goalService)
    , notificationService_(notificationService)
{
    loadTransactions();
}

void TransactionService::loadTransactions()
{
    txns_ = storage_.loadTransactions();
}

void TransactionService::saveTransactions()
{
    storage_.saveTransactions(txns_);
}

// ── CRUD ────────────────────────────────────────────────────────────

models::Transaction* TransactionService::addIncome(
    const std::string& userId, double amount,
    const std::string& description, const std::string& categoryId,
    const std::string& accountId, const std::string& date,
    const std::string& goalId, const std::vector<std::string>& tags)
{
    if (amount <= 0) {
        throw utils::InvalidInputException("Income amount must be positive");
    }

    std::string id = utils::UUID::generate();
    auto txn = std::make_unique<models::Income>(
        id, amount, description, categoryId, accountId, userId, date);

    txn->setTags(tags);

    // Deposit into the account.
    auto* acc = accountService_.getAccount(accountId);
    if (!acc) throw utils::NotFoundException("Account", accountId);
    acc->deposit(amount);
    accountService_.saveAccounts();

    models::Transaction* ptr = txn.get();
    txns_.push_back(std::move(txn));
    saveTransactions();

    // Contribute to goal if specified.
    if (!goalId.empty()) {
        goalService_.contribute(goalId, amount);
    }

    return ptr;
}

models::Transaction* TransactionService::addExpense(
    const std::string& userId, double amount,
    const std::string& description, const std::string& categoryId,
    const std::string& accountId, const std::string& date,
    bool isRecurring, const std::string& recurrenceRule,
    const std::vector<std::string>& tags)
{
    if (amount <= 0) {
        throw utils::InvalidInputException("Expense amount must be positive");
    }

    std::string id = utils::UUID::generate();
    auto txn = std::make_unique<models::Expense>(
        id, amount, description, categoryId, accountId, userId, date);
    txn->setRecurring(isRecurring);
    txn->setRecurrenceRule(recurrenceRule);
    txn->setTags(tags);

    // Withdraw from the account.
    auto* acc = accountService_.getAccount(accountId);
    if (!acc) throw utils::NotFoundException("Account", accountId);
    acc->withdraw(amount);
    accountService_.saveAccounts();

    models::Transaction* ptr = txn.get();
    txns_.push_back(std::move(txn));
    saveTransactions();

    // Check budgets.
    budgetService_.checkBudget(*ptr);

    return ptr;
}

models::Transaction* TransactionService::addTransfer(
    const std::string& userId, double amount,
    const std::string& description,
    const std::string& fromAccountId, const std::string& toAccountId,
    const std::string& date)
{
    if (amount <= 0) {
        throw utils::InvalidInputException("Transfer amount must be positive");
    }

    std::string id = utils::UUID::generate();
    auto txn = std::make_unique<models::Transfer>(
        id, amount, description, "", fromAccountId, userId, date);
    txn->setToAccountId(toAccountId);

    // Move money between accounts.
    accountService_.transfer(fromAccountId, toAccountId, amount, description);

    models::Transaction* ptr = txn.get();
    txns_.push_back(std::move(txn));
    saveTransactions();

    return ptr;
}

void TransactionService::editTransaction(
    const std::string& transactionId,
    double newAmount, const std::string& newDescription,
    const std::string& newCategoryId)
{
    auto it = std::find_if(txns_.begin(), txns_.end(),
        [&](const auto& t) { return t->id() == transactionId; });
    if (it == txns_.end()) {
        throw utils::NotFoundException("Transaction", transactionId);
    }

    (*it)->setAmount(newAmount);
    (*it)->setDescription(newDescription);
    (*it)->setCategoryId(newCategoryId);
    saveTransactions();
}

void TransactionService::deleteTransaction(const std::string& transactionId)
{
    auto it = std::find_if(txns_.begin(), txns_.end(),
        [&](const auto& t) { return t->id() == transactionId; });
    if (it == txns_.end()) {
        throw utils::NotFoundException("Transaction", transactionId);
    }

    // Push onto undo stack before removing.
    undoStack_.push(std::move(*it));
    txns_.erase(it);
    saveTransactions();
}

bool TransactionService::undoDelete()
{
    if (undoStack_.empty()) return false;

    auto txn = std::move(undoStack_.top());
    undoStack_.pop();

    txns_.push_back(std::move(txn));
    saveTransactions();
    return true;
}

// ── Queries ─────────────────────────────────────────────────────────

std::vector<models::Transaction*> TransactionService::getTransactionsByUser(
    const std::string& userId) const
{
    std::vector<models::Transaction*> result;
    for (const auto& t : txns_) {
        if (t->userId() == userId) result.push_back(t.get());
    }
    return result;
}

std::vector<models::Transaction*> TransactionService::getTransactionsByAccount(
    const std::string& accountId) const
{
    std::vector<models::Transaction*> result;
    for (const auto& t : txns_) {
        if (t->accountId() == accountId) {
            result.push_back(t.get());
        }
        // Also check transfers where this is the destination.
        if (t->transactionTypeName() == "Transfer") {
            auto* tr = static_cast<const models::Transfer*>(t.get());
            if (tr->toAccountId() == accountId) {
                result.push_back(t.get());
            }
        }
    }
    return result;
}

std::vector<models::Transaction*> TransactionService::getTransactionsByCategory(
    const std::string& categoryId) const
{
    std::vector<models::Transaction*> result;
    for (const auto& t : txns_) {
        if (t->categoryId() == categoryId) result.push_back(t.get());
    }
    return result;
}

models::Transaction* TransactionService::getTransaction(
    const std::string& txnId) const
{
    auto it = std::find_if(txns_.begin(), txns_.end(),
        [&](const auto& t) { return t->id() == txnId; });
    return (it != txns_.end()) ? it->get() : nullptr;
}

// ── Recurring ───────────────────────────────────────────────────────

size_t TransactionService::processRecurring()
{
    size_t count = 0;
    std::string today = utils::DateUtils::today();
    auto [curYear, curMonth, curDay] = utils::DateUtils::extractYMD(today);

    for (const auto& txn : txns_) {
        if (txn->transactionTypeName() != "Expense") continue;
        auto* exp = static_cast<const models::Expense*>(txn.get());
        if (!exp->isRecurring() || exp->recurrenceRule().empty()) continue;

        auto [txYear, txMonth, txDay] = utils::DateUtils::extractYMD(txn->date());
        bool shouldGenerate = false;

        if (exp->recurrenceRule() == "monthly") {
            int monthsSince = (curYear - txYear) * 12 + (curMonth - txMonth);
            shouldGenerate = (monthsSince >= 1);
        } else if (exp->recurrenceRule() == "yearly") {
            shouldGenerate = (curYear > txYear && curMonth == txMonth && curDay >= txDay);
        } else if (exp->recurrenceRule() == "weekly") {
            auto daysDiff = utils::DateUtils::daysBetween(txn->date(), today);
            shouldGenerate = (daysDiff >= 7);
        }

        if (shouldGenerate) {
            auto recurring = std::make_unique<models::Expense>(
                utils::UUID::generate(),
                txn->amount(), txn->description(),
                txn->categoryId(), txn->accountId(),
                txn->userId(), today);
            recurring->setRecurring(true);
            recurring->setRecurrenceRule(exp->recurrenceRule());
            recurring->setTags(txn->tags());

            models::Transaction* ptr = recurring.get();
            txns_.push_back(std::move(recurring));
            saveTransactions();

            notificationService_.notify(
                txn->userId(),
                models::NotificationType::RECURRING_PAYMENT,
                "Recurring expense generated: " + txn->description() +
                " (" + std::to_string(txn->amount()) + ")");
            ++count;
        }
    }

    return count;
}

}  // namespace finance::services
