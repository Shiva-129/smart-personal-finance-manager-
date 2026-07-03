#include "finance/models/Account.h"
#include "finance/utils/DateUtils.h"
#include "finance/utils/Exceptions.h"
#include "finance/utils/UUID.h"

namespace finance::models {

// ── Account base ────────────────────────────────────────────────────

Account::Account(std::string id, std::string name, const std::string& currency,
                 const std::string& userId)
    : id_(std::move(id))
    , name_(std::move(name))
    , currency_(currency)
    , userId_(userId)
    , createdAt_(utils::DateUtils::timestamp())
{
}

void Account::deposit(double amount)
{
    if (amount <= 0) {
        throw utils::InvalidInputException("Deposit amount must be positive");
    }
    if (status_ == AccountStatus::CLOSED) {
        throw utils::FinanceException("Cannot deposit into a closed account");
    }
    if (status_ == AccountStatus::FROZEN) {
        throw utils::FinanceException("Cannot deposit into a frozen account");
    }
    balance_ += amount;
}

void Account::withdraw(double amount)
{
    if (amount <= 0) {
        throw utils::InvalidInputException("Withdrawal amount must be positive");
    }
    if (status_ == AccountStatus::CLOSED) {
        throw utils::FinanceException("Cannot withdraw from a closed account");
    }
    if (status_ == AccountStatus::FROZEN) {
        throw utils::FinanceException("Cannot withdraw from a frozen account");
    }
    if (amount > balance_) {
        throw utils::NegativeBalanceException(name_);
    }
    balance_ -= amount;
}

void Account::freeze()
{
    status_ = AccountStatus::FROZEN;
}

void Account::close()
{
    status_ = AccountStatus::CLOSED;
}

nlohmann::json Account::toJson() const
{
    return {
        {"id",         id_},
        {"name",       name_},
        {"balance",    balance_},
        {"currency",   currency_},
        {"user_id",    userId_},
        {"created_at", createdAt_},
        {"status",     accountStatusToString(status_)},
        {"type",       accountTypeName()}
    };
}

std::unique_ptr<Account> Account::fromJson(const nlohmann::json& j)
{
    std::string type = j.value("type", "Cash");
    std::unique_ptr<Account> acc;

    if (type == "Cash")          acc = std::make_unique<CashAccount>();
    else if (type == "Savings")  acc = std::make_unique<SavingsAccount>();
    else if (type == "Current")  acc = std::make_unique<CurrentAccount>();
    else if (type == "Credit Card") acc = std::make_unique<CreditCardAccount>();
    else if (type == "Wallet")   acc = std::make_unique<WalletAccount>();
    else                          acc = std::make_unique<CashAccount>();

    acc->id_        = j.value("id", "");
    acc->name_      = j.value("name", "");
    acc->balance_   = j.value("balance", 0.0);
    acc->currency_  = j.value("currency", "INR");
    acc->userId_    = j.value("user_id", "");
    acc->createdAt_ = j.value("created_at", "");
    acc->status_    = accountStatusFromString(j.value("status", "active"));

    // Derived-type-specific fields via virtual dispatch (no static_cast).
    acc->loadDerivedFields(j);

    return acc;
}

// ── CashAccount ─────────────────────────────────────────────────────

std::unique_ptr<Account> CashAccount::clone() const
{
    return std::make_unique<CashAccount>(*this);
}

nlohmann::json CashAccount::toJson() const
{
    auto j = Account::toJson();
    j["type"] = "Cash";
    return j;
}

// ── SavingsAccount ──────────────────────────────────────────────────

std::unique_ptr<Account> SavingsAccount::clone() const
{
    return std::make_unique<SavingsAccount>(*this);
}

nlohmann::json SavingsAccount::toJson() const
{
    auto j = Account::toJson();
    j["type"]          = "Savings";
    j["interest_rate"] = interestRate_;
    return j;
}

void SavingsAccount::loadDerivedFields(const nlohmann::json& j)
{
    if (j.contains("interest_rate")) {
        interestRate_ = j["interest_rate"].get<double>();
    }
}

// ── CurrentAccount ──────────────────────────────────────────────────

std::unique_ptr<Account> CurrentAccount::clone() const
{
    return std::make_unique<CurrentAccount>(*this);
}

nlohmann::json CurrentAccount::toJson() const
{
    auto j = Account::toJson();
    j["type"] = "Current";
    return j;
}

// ── CreditCardAccount ───────────────────────────────────────────────

std::unique_ptr<Account> CreditCardAccount::clone() const
{
    return std::make_unique<CreditCardAccount>(*this);
}

nlohmann::json CreditCardAccount::toJson() const
{
    auto j = Account::toJson();
    j["type"]         = "Credit Card";
    j["credit_limit"] = creditLimit_;
    j["due_amount"]   = dueAmount_;
    j["due_date"]     = dueDate_;
    return j;
}

void CreditCardAccount::loadDerivedFields(const nlohmann::json& j)
{
    if (j.contains("credit_limit")) creditLimit_ = j["credit_limit"].get<double>();
    if (j.contains("due_amount"))   dueAmount_   = j["due_amount"].get<double>();
    if (j.contains("due_date"))     dueDate_     = j["due_date"].get<std::string>();
}

void CreditCardAccount::withdraw(double amount)
{
    if (amount <= 0) {
        throw utils::InvalidInputException("Withdrawal amount must be positive");
    }
    if (status_ == AccountStatus::CLOSED) {
        throw utils::FinanceException("Cannot withdraw from a closed account");
    }
    if (status_ == AccountStatus::FROZEN) {
        throw utils::FinanceException("Cannot withdraw from a frozen account");
    }
    // Credit card: balance goes negative up to the credit limit.
    if (balance_ - amount < -creditLimit_) {
        throw utils::CreditLimitException(name_);
    }
    balance_ -= amount;
}

// ── WalletAccount ───────────────────────────────────────────────────

std::unique_ptr<Account> WalletAccount::clone() const
{
    return std::make_unique<WalletAccount>(*this);
}

nlohmann::json WalletAccount::toJson() const
{
    auto j = Account::toJson();
    j["type"] = "Wallet";
    return j;
}

// ── Utilities ───────────────────────────────────────────────────────

std::string accountStatusToString(AccountStatus s)
{
    switch (s) {
        case AccountStatus::ACTIVE: return "active";
        case AccountStatus::FROZEN: return "frozen";
        case AccountStatus::CLOSED: return "closed";
    }
    return "active";
}

AccountStatus accountStatusFromString(const std::string& s)
{
    if (s == "frozen") return AccountStatus::FROZEN;
    if (s == "closed") return AccountStatus::CLOSED;
    return AccountStatus::ACTIVE;
}

}  // namespace finance::models
