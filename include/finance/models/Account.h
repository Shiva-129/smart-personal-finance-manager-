#ifndef FINANCE_MODELS_ACCOUNT_H
#define FINANCE_MODELS_ACCOUNT_H

#include <memory>
#include <nlohmann/json.hpp>
#include <string>
#include <vector>

namespace finance::models {

/// Possible states of an account.
enum class AccountStatus {
    ACTIVE,
    FROZEN,
    CLOSED
};

/**
 * @brief Abstract base class for all account types.
 */
class Account {
public:
    Account() = default;
    Account(std::string id, std::string name, const std::string& currency,
            const std::string& userId);
    virtual ~Account() = default;

    // ── Accessors ───────────────────────────────────────────────────
    const std::string& id()         const { return id_; }
    const std::string& name()       const { return name_; }
    double             balance()    const { return balance_; }
    const std::string& currency()   const { return currency_; }
    const std::string& userId()     const { return userId_; }
    const std::string& createdAt()  const { return createdAt_; }
    AccountStatus      status()     const { return status_; }

    // ── Mutators ────────────────────────────────────────────────────
    void setName(const std::string& name)     { name_ = name; }
    void setStatus(AccountStatus s)           { status_ = s; }

    /// Deposit money into the account.
    virtual void deposit(double amount);
    /// Withdraw money (throws NegativeBalanceException if insufficient).
    virtual void withdraw(double amount);
    /// Freeze the account (prevents transactions).
    void freeze();
    /// Close the account.
    void close();

    /// Human-readable account type name (implemented by each derived class).
    virtual std::string accountTypeName() const = 0;

    /// Serialization.
    virtual nlohmann::json toJson() const;
    /// Factory: create the correct derived type from JSON.
    static std::unique_ptr<Account> fromJson(const nlohmann::json& j);

    /// Clone via virtual copy.
    virtual std::unique_ptr<Account> clone() const = 0;

    /// Load derived-type-specific fields from JSON (used by fromJson).
    virtual void loadDerivedFields(const nlohmann::json& j) {}

protected:
    std::string     id_;
    std::string     name_;
    double          balance_ = 0.0;
    std::string     currency_;
    std::string     userId_;
    std::string     createdAt_;
    AccountStatus   status_ = AccountStatus::ACTIVE;

    /// Recompute balance from a list of transactions (used when loading from storage).
    void setBalance(double b) { balance_ = b; }
};

// ── Concrete Account Types ──────────────────────────────────────────

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
    void setInterestRate(double rate) { interestRate_ = rate; }

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

    void setCreditLimit(double limit) { creditLimit_ = limit; }
    void setDueAmount(double amount)  { dueAmount_ = amount; }
    void setDueDate(const std::string& date) { dueDate_ = date; }

    /// Withdraw (borrow) against the credit limit.
    void withdraw(double amount) override;

private:
    double      creditLimit_ = 0.0;
    double      dueAmount_ = 0.0;
    std::string dueDate_;
};

class WalletAccount : public Account {
public:
    using Account::Account;
    std::string accountTypeName() const override { return "Wallet"; }
    std::unique_ptr<Account> clone() const override;
    nlohmann::json toJson() const override;
};

// ── Utilities ───────────────────────────────────────────────────────

std::string accountStatusToString(AccountStatus s);
AccountStatus accountStatusFromString(const std::string& s);

}  // namespace finance::models

#endif  // FINANCE_MODELS_ACCOUNT_H
