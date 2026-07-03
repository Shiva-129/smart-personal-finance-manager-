#include "finance/services/AccountService.h"
#include "finance/utils/DateUtils.h"
#include "finance/utils/Exceptions.h"
#include "finance/utils/UUID.h"

#include <algorithm>

namespace finance::services {

AccountService::AccountService(storage::StorageManager& storage)
    : storage_(storage)
{
    loadAccounts();
}

void AccountService::loadAccounts()
{
    accounts_ = storage_.loadAccounts();
}

void AccountService::saveAccounts()
{
    storage_.saveAccounts(accounts_);
}

// ── Factory helpers ─────────────────────────────────────────────────

static std::unique_ptr<models::Account> createAccountOfType(
    const std::string& type, const std::string& id,
    const std::string& name, const std::string& currency,
    const std::string& userId)
{
    if (type == "Cash")        return std::make_unique<models::CashAccount>(
        id, name, currency, userId);
    if (type == "Savings")     return std::make_unique<models::SavingsAccount>(
        id, name, currency, userId);
    if (type == "Current")     return std::make_unique<models::CurrentAccount>(
        id, name, currency, userId);
    if (type == "Credit Card") return std::make_unique<models::CreditCardAccount>(
        id, name, currency, userId);
    if (type == "Wallet")      return std::make_unique<models::WalletAccount>(
        id, name, currency, userId);

    throw utils::InvalidInputException("Unknown account type: " + type);
}

// ── Core operations ─────────────────────────────────────────────────

models::Account* AccountService::createAccount(
    const std::string& userId, const std::string& type,
    const std::string& name, const std::string& currency)
{
    if (name.empty()) {
        throw utils::InvalidInputException("Account name cannot be empty");
    }

    std::string id = utils::UUID::generate();
    auto acc = createAccountOfType(type, id, name, currency, userId);

    models::Account* ptr = acc.get();
    accounts_.push_back(std::move(acc));
    saveAccounts();
    return ptr;
}

void AccountService::deleteAccount(const std::string& accountId)
{
    auto it = std::find_if(accounts_.begin(), accounts_.end(),
        [&](const auto& acc) { return acc->id() == accountId; });
    if (it == accounts_.end()) {
        throw utils::NotFoundException("Account", accountId);
    }
    accounts_.erase(it);
    saveAccounts();
}

void AccountService::freezeAccount(const std::string& accountId)
{
    auto* acc = getAccount(accountId);
    if (!acc) throw utils::NotFoundException("Account", accountId);
    acc->freeze();
    saveAccounts();
}

void AccountService::closeAccount(const std::string& accountId)
{
    auto* acc = getAccount(accountId);
    if (!acc) throw utils::NotFoundException("Account", accountId);
    acc->close();
    saveAccounts();
}

void AccountService::transfer(const std::string& fromAccountId,
                               const std::string& toAccountId,
                               double amount,
                               const std::string& description)
{
    if (amount <= 0) {
        throw utils::InvalidInputException("Transfer amount must be positive");
    }
    if (fromAccountId == toAccountId) {
        throw utils::InvalidInputException("Cannot transfer to the same account");
    }

    auto* from = getAccount(fromAccountId);
    auto* to   = getAccount(toAccountId);
    if (!from) throw utils::NotFoundException("Source account", fromAccountId);
    if (!to)   throw utils::NotFoundException("Destination account", toAccountId);

    from->withdraw(amount);
    to->deposit(amount);
    saveAccounts();
}

std::vector<models::Account*> AccountService::getAccountsByUser(
    const std::string& userId)
{
    std::vector<models::Account*> result;
    for (auto& acc : accounts_) {
        if (acc->userId() == userId) {
            result.push_back(acc.get());
        }
    }
    return result;
}

models::Account* AccountService::getAccount(const std::string& accountId)
{
    auto it = std::find_if(accounts_.begin(), accounts_.end(),
        [&](const auto& acc) { return acc->id() == accountId; });
    return (it != accounts_.end()) ? it->get() : nullptr;
}

const std::vector<std::unique_ptr<models::Account>>&
AccountService::allAccounts() const
{
    return accounts_;
}

}  // namespace finance::services
