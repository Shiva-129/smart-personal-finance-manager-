#ifndef FINANCE_MODELS_H
#define FINANCE_MODELS_H

#include "Utils.h"

#include <memory>
#include <nlohmann/json.hpp>
#include <string>
#include <vector>

namespace finance::models {

// ══════════════════════════════════════════════════════════════════════
// User
// ══════════════════════════════════════════════════════════════════════

class User {
public:
    User() = default;
    User(std::string id, std::string username, std::string passwordHash,
         std::string displayName = {});

    const std::string& id()           const { return id_; }
    const std::string& username()     const { return username_; }
    const std::string& passwordHash() const { return passwordHash_; }
    const std::string& displayName()  const { return displayName_; }
    const std::string& createdAt()    const { return createdAt_; }
    bool               isActive()     const { return isActive_; }

    void setPasswordHash(const std::string& h) { passwordHash_ = h; }
    void setDisplayName(const std::string& n)  { displayName_ = n; }
    void setLastLoginAt(const std::string& ts) { lastLoginAt_ = ts; }
    void setActive(bool a) { isActive_ = a; }

    nlohmann::json toJson() const;
    static User fromJson(const nlohmann::json& j);

private:
    std::string id_, username_, passwordHash_, displayName_, createdAt_, lastLoginAt_;
    bool isActive_ = true;
};

// ══════════════════════════════════════════════════════════════════════
// Category
// ══════════════════════════════════════════════════════════════════════

enum class CategoryType { INCOME, EXPENSE, BOTH };

class Category {
public:
    Category() = default;
    Category(std::string id, std::string name, CategoryType type, bool isDefault = false);

    const std::string& id() const { return id_; }
    const std::string& name() const { return name_; }
    CategoryType       type() const { return type_; }
    bool               isDefault() const { return isDefault_; }
    void setName(const std::string& n) { name_ = n; }
    void setType(CategoryType t) { type_ = t; }

    nlohmann::json toJson() const;
    static Category fromJson(const nlohmann::json& j);
    static std::vector<Category> defaults();
    static std::string typeToString(CategoryType t);
    static CategoryType typeFromString(const std::string& s);

private:
    std::string id_, name_;
    CategoryType type_ = CategoryType::EXPENSE;
    bool isDefault_ = false;
};

// ══════════════════════════════════════════════════════════════════════
// Account (abstract base + 5 derived types)
// ══════════════════════════════════════════════════════════════════════

enum class AccountStatus { ACTIVE, FROZEN, CLOSED };

class Account {
public:
    Account() = default;
    Account(std::string id, std::string name, const std::string& currency, const std::string& userId);
    virtual ~Account() = default;

    const std::string& id()       const { return id_; }
    const std::string& name()     const { return name_; }
    double             balance()  const { return balance_; }
    const std::string& currency() const { return currency_; }
    const std::string& userId()   const { return userId_; }
    const std::string& createdAt() const { return createdAt_; }
    AccountStatus      status()   const { return status_; }

    void setName(const std::string& n) { name_ = n; }
    void setStatus(AccountStatus s) { status_ = s; }

    virtual void deposit(double amount);
    virtual void withdraw(double amount);
    void freeze();
    void close();

    virtual std::string accountTypeName() const = 0;
    virtual nlohmann::json toJson() const;
    static std::unique_ptr<Account> fromJson(const nlohmann::json& j);
    virtual std::unique_ptr<Account> clone() const = 0;
    virtual void loadDerivedFields(const nlohmann::json& j) {}

protected:
    std::string   id_, name_, currency_, userId_, createdAt_;
    double        balance_ = 0.0;
    AccountStatus status_ = AccountStatus::ACTIVE;
    void setBalance(double b) { balance_ = b; }
};

class CashAccount : public Account {
public:
    using Account::Account;
    std::string accountTypeName() const override { return "Cash"; }
    std::unique_ptr<Account> clone() const override;
    nlohmann::json toJson() const override;
};
class SavingsAccount : public Account {
public:
    using Account::Account;
    std::string accountTypeName() const override { return "Savings"; }
    std::unique_ptr<Account> clone() const override;
    nlohmann::json toJson() const override;
    void loadDerivedFields(const nlohmann::json& j) override;
    double interestRate() const { return interestRate_; }
    void setInterestRate(double r) { interestRate_ = r; }
private:
    double interestRate_ = 0.0;
};
class CurrentAccount : public Account {
public:
    using Account::Account;
    std::string accountTypeName() const override { return "Current"; }
    std::unique_ptr<Account> clone() const override;
    nlohmann::json toJson() const override;
};
class CreditCardAccount : public Account {
public:
    using Account::Account;
    std::string accountTypeName() const override { return "Credit Card"; }
    std::unique_ptr<Account> clone() const override;
    nlohmann::json toJson() const override;
    void loadDerivedFields(const nlohmann::json& j) override;
    double creditLimit() const { return creditLimit_; }
    double dueAmount()   const { return dueAmount_; }
    std::string dueDate() const { return dueDate_; }
    void setCreditLimit(double l) { creditLimit_ = l; }
    void setDueAmount(double a)   { dueAmount_ = a; }
    void setDueDate(const std::string& d) { dueDate_ = d; }
    void withdraw(double amount) override;
private:
    double creditLimit_ = 0.0, dueAmount_ = 0.0;
    std::string dueDate_;
};
class WalletAccount : public Account {
public:
    using Account::Account;
    std::string accountTypeName() const override { return "Wallet"; }
    std::unique_ptr<Account> clone() const override;
    nlohmann::json toJson() const override;
};

std::string accountStatusToString(AccountStatus s);
AccountStatus accountStatusFromString(const std::string& s);

// ══════════════════════════════════════════════════════════════════════
// Transaction (abstract base + 3 derived types)
// ══════════════════════════════════════════════════════════════════════

class Transaction {
public:
    Transaction() = default;
    Transaction(std::string id, double amount, std::string description,
                std::string categoryId, std::string accountId,
                std::string userId, std::string date = {}, std::string time = {});
    virtual ~Transaction() = default;

    const std::string& id()           const { return id_; }
    double             amount()       const { return amount_; }
    const std::string& description()  const { return description_; }
    const std::string& categoryId()   const { return categoryId_; }
    const std::string& accountId()    const { return accountId_; }
    const std::string& userId()       const { return userId_; }
    const std::string& date()         const { return date_; }
    const std::string& time()         const { return time_; }
    const std::vector<std::string>& tags() const { return tags_; }
    const std::string& notes()        const { return notes_; }

    void setAmount(double a)          { amount_ = a; }
    void setDescription(const std::string& d) { description_ = d; }
    void setCategoryId(const std::string& c)  { categoryId_ = c; }
    void setDate(const std::string& d) { date_ = d; }
    void setTime(const std::string& t) { time_ = t; }
    void setTags(const std::vector<std::string>& t) { tags_ = t; }
    void setNotes(const std::string& n) { notes_ = n; }

    virtual std::string transactionTypeName() const = 0;
    virtual nlohmann::json toJson() const;
    static std::unique_ptr<Transaction> fromJson(const nlohmann::json& j);
    virtual std::unique_ptr<Transaction> clone() const = 0;
    virtual void loadDerivedFields(const nlohmann::json& j) {}

protected:
    std::string id_, description_, categoryId_, accountId_, userId_, date_, time_, notes_;
    double amount_ = 0.0;
    std::vector<std::string> tags_;
};

class Income : public Transaction {
public:
    using Transaction::Transaction;
    std::string transactionTypeName() const override { return "Income"; }
    std::unique_ptr<Transaction> clone() const override;
    nlohmann::json toJson() const override;
};

class Expense : public Transaction {
public:
    using Transaction::Transaction;
    std::string transactionTypeName() const override { return "Expense"; }
    std::unique_ptr<Transaction> clone() const override;
    nlohmann::json toJson() const override;
    void loadDerivedFields(const nlohmann::json& j) override;
    bool isRecurring() const { return isRecurring_; }
    void setRecurring(bool r) { isRecurring_ = r; }
    std::string recurrenceRule() const { return recurrenceRule_; }
    void setRecurrenceRule(const std::string& r) { recurrenceRule_ = r; }
private:
    bool isRecurring_ = false;
    std::string recurrenceRule_;
};

class Transfer : public Transaction {
public:
    using Transaction::Transaction;
    std::string transactionTypeName() const override { return "Transfer"; }
    std::unique_ptr<Transaction> clone() const override;
    nlohmann::json toJson() const override;
    void loadDerivedFields(const nlohmann::json& j) override;
    const std::string& toAccountId() const { return toAccountId_; }
    void setToAccountId(const std::string& id) { toAccountId_ = id; }
private:
    std::string toAccountId_;
};

// ══════════════════════════════════════════════════════════════════════
// Budget
// ══════════════════════════════════════════════════════════════════════

class Budget {
public:
    Budget() = default;
    Budget(std::string id, std::string categoryId, std::string userId,
           int year, int month, double limitAmount);

    const std::string& id()         const { return id_; }
    const std::string& categoryId() const { return categoryId_; }
    const std::string& userId()     const { return userId_; }
    int  year()  const { return year_; }
    int  month() const { return month_; }
    double limitAmount() const { return limitAmount_; }
    double spentAmount() const { return spentAmount_; }
    double remaining()   const { return limitAmount_ - spentAmount_; }
    double percentageUsed() const;
    bool   isExceeded() const { return spentAmount_ > limitAmount_; }
    void setLimitAmount(double l) { limitAmount_ = l; }
    void setSpentAmount(double s) { spentAmount_ = s; }
    void addSpending(double a)    { spentAmount_ += a; }

    nlohmann::json toJson() const;
    static Budget fromJson(const nlohmann::json& j);

private:
    std::string id_, categoryId_, userId_;
    int year_ = 0, month_ = 0;
    double limitAmount_ = 0.0, spentAmount_ = 0.0;
};

// ══════════════════════════════════════════════════════════════════════
// Goal
// ══════════════════════════════════════════════════════════════════════

enum class GoalStatus { IN_PROGRESS, ACHIEVED, CANCELLED };

class Goal {
public:
    Goal() = default;
    Goal(std::string id, std::string userId, std::string name,
         double targetAmount, std::string deadline = {});

    const std::string& id()      const { return id_; }
    const std::string& userId()  const { return userId_; }
    const std::string& name()    const { return name_; }
    double targetAmount()  const { return targetAmount_; }
    double currentAmount() const { return currentAmount_; }
    const std::string& deadline()  const { return deadline_; }
    const std::string& createdAt() const { return createdAt_; }
    GoalStatus status() const { return status_; }
    double progressPercentage() const;
    bool   isAchieved() const { return status_ == GoalStatus::ACHIEVED; }
    void setName(const std::string& n)   { name_ = n; }
    void setTargetAmount(double a)       { targetAmount_ = a; }
    void setCurrentAmount(double a)      { currentAmount_ = a; }
    void addAmount(double a)             { currentAmount_ += a; }
    void setDeadline(const std::string& d) { deadline_ = d; }
    void setStatus(GoalStatus s) { status_ = s; }
    void markAchieved() { status_ = GoalStatus::ACHIEVED; }

    nlohmann::json toJson() const;
    static Goal fromJson(const nlohmann::json& j);
    static std::string statusToString(GoalStatus s);
    static GoalStatus statusFromString(const std::string& s);

private:
    std::string id_, userId_, name_, deadline_, createdAt_;
    double targetAmount_ = 0.0, currentAmount_ = 0.0;
    GoalStatus status_ = GoalStatus::IN_PROGRESS;
};

// ══════════════════════════════════════════════════════════════════════
// Notification
// ══════════════════════════════════════════════════════════════════════

enum class NotificationType {
    BUDGET_EXCEEDED, GOAL_ACHIEVED, GOAL_PROGRESS, RECURRING_PAYMENT, LOW_BALANCE, INFO
};

class Notification {
public:
    Notification() = default;
    Notification(std::string id, std::string userId, NotificationType type, std::string message);

    const std::string& id()      const { return id_; }
    const std::string& userId()  const { return userId_; }
    NotificationType   type()    const { return type_; }
    const std::string& message() const { return message_; }
    const std::string& createdAt() const { return createdAt_; }
    bool isRead() const { return isRead_; }
    void markRead() { isRead_ = true; }

    nlohmann::json toJson() const;
    static Notification fromJson(const nlohmann::json& j);
    static std::string typeToString(NotificationType t);
    static NotificationType typeFromString(const std::string& s);

private:
    std::string id_, userId_, message_, createdAt_;
    NotificationType type_ = NotificationType::INFO;
    bool isRead_ = false;
};

}  // namespace finance::models

#endif  // FINANCE_MODELS_H
