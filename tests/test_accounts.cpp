/**
 * @file    test_accounts.cpp
 * @brief   Unit tests for Account hierarchy.
 */

#include <gtest/gtest.h>
#include "finance/models/Account.h"
#include "finance/utils/Exceptions.h"
#include "finance/utils/UUID.h"

using namespace finance::models;
using namespace finance::utils;

// ── Helper ──────────────────────────────────────────────────────────

static std::string makeId() { return UUID::generate(); }

// ── Account base ────────────────────────────────────────────────────

TEST(AccountTest, CreateAndAccess)
{
    CashAccount acc(makeId(), "Wallet", "INR", "user1");
    EXPECT_EQ(acc.name(), "Wallet");
    EXPECT_EQ(acc.balance(), 0.0);
    EXPECT_EQ(acc.currency(), "INR");
    EXPECT_EQ(acc.accountTypeName(), "Cash");
    EXPECT_EQ(acc.status(), AccountStatus::ACTIVE);
}

TEST(AccountTest, DepositIncreasesBalance)
{
    CashAccount acc(makeId(), "Test", "INR", "u1");
    acc.deposit(1000.0);
    EXPECT_DOUBLE_EQ(acc.balance(), 1000.0);

    acc.deposit(500.0);
    EXPECT_DOUBLE_EQ(acc.balance(), 1500.0);
}

TEST(AccountTest, DepositThrowsOnNonPositive)
{
    CashAccount acc(makeId(), "Test", "INR", "u1");
    EXPECT_THROW(acc.deposit(0),    InvalidInputException);
    EXPECT_THROW(acc.deposit(-100), InvalidInputException);
}

TEST(AccountTest, WithdrawDecreasesBalance)
{
    CashAccount acc(makeId(), "Test", "INR", "u1");
    acc.deposit(1000.0);
    acc.withdraw(300.0);
    EXPECT_DOUBLE_EQ(acc.balance(), 700.0);
}

TEST(AccountTest, WithdrawThrowsOnInsufficientFunds)
{
    CashAccount acc(makeId(), "Test", "INR", "u1");
    acc.deposit(100.0);
    EXPECT_THROW(acc.withdraw(200.0), NegativeBalanceException);
}

TEST(AccountTest, WithdrawThrowsOnNonPositive)
{
    CashAccount acc(makeId(), "Test", "INR", "u1");
    EXPECT_THROW(acc.withdraw(0),    InvalidInputException);
    EXPECT_THROW(acc.withdraw(-50),  InvalidInputException);
}

TEST(AccountTest, FreezeAndClose)
{
    CashAccount acc(makeId(), "Test", "INR", "u1");
    acc.deposit(100.0);

    acc.freeze();
    EXPECT_EQ(acc.status(), AccountStatus::FROZEN);
    EXPECT_THROW(acc.deposit(50.0),  FinanceException);
    EXPECT_THROW(acc.withdraw(10.0), FinanceException);

    acc.close();
    EXPECT_EQ(acc.status(), AccountStatus::CLOSED);
    EXPECT_THROW(acc.deposit(50.0),  FinanceException);
    EXPECT_THROW(acc.withdraw(10.0), FinanceException);
}

// ── CreditCardAccount ───────────────────────────────────────────────

TEST(CreditCardTest, WithdrawGoesNegative)
{
    CreditCardAccount cc(makeId(), "Card", "INR", "u1");
    cc.setCreditLimit(5000.0);

    cc.withdraw(2000.0);
    EXPECT_DOUBLE_EQ(cc.balance(), -2000.0);
}

TEST(CreditCardTest, WithdrawRespectsCreditLimit)
{
    CreditCardAccount cc(makeId(), "Card", "INR", "u1");
    cc.setCreditLimit(5000.0);

    cc.withdraw(3000.0);
    EXPECT_THROW(cc.withdraw(3000.0), CreditLimitException);
}

TEST(CreditCardTest, DepositAfterWithdraw)
{
    CreditCardAccount cc(makeId(), "Card", "INR", "u1");
    cc.setCreditLimit(5000.0);
    cc.withdraw(2000.0);

    cc.deposit(1000.0);
    EXPECT_DOUBLE_EQ(cc.balance(), -1000.0);
}

// ── SavingsAccount ──────────────────────────────────────────────────

TEST(SavingsAccountTest, InterestRate)
{
    SavingsAccount sa(makeId(), "Savings", "INR", "u1");
    sa.setInterestRate(4.5);
    EXPECT_DOUBLE_EQ(sa.interestRate(), 4.5);
}

// ── Serialization / Factory ─────────────────────────────────────────

TEST(AccountSerializationTest, CashRoundTrip)
{
    CashAccount original(makeId(), "My Cash", "INR", "u1");
    original.deposit(500.0);

    auto json = original.toJson();
    auto restored = Account::fromJson(json);

    EXPECT_EQ(original.id(),       restored->id());
    EXPECT_EQ(original.name(),     restored->name());
    EXPECT_EQ(original.balance(),  restored->balance());
    EXPECT_EQ(original.currency(), restored->currency());
    EXPECT_EQ(original.accountTypeName(), restored->accountTypeName());
}

TEST(AccountSerializationTest, CreditCardRoundTrip)
{
    CreditCardAccount original(makeId(), "Platinum", "INR", "u1");
    original.setCreditLimit(50000.0);
    original.setDueAmount(12000.0);
    original.setDueDate("2026-08-15");
    original.withdraw(3000.0);

    auto json = original.toJson();
    auto restored = Account::fromJson(json);

    EXPECT_EQ(original.id(),       restored->id());
    EXPECT_EQ(original.balance(),  restored->balance());
    EXPECT_EQ(original.accountTypeName(), restored->accountTypeName());
}

TEST(AccountSerializationTest, FactoryCreatesCorrectTypes)
{
    auto cashJson = CashAccount(makeId(), "C", "INR", "u1").toJson();
    auto savJson  = SavingsAccount(makeId(), "S", "INR", "u1").toJson();
    auto curJson  = CurrentAccount(makeId(), "Cu", "INR", "u1").toJson();
    auto ccJson   = CreditCardAccount(makeId(), "CC", "INR", "u1").toJson();
    auto walJson  = WalletAccount(makeId(), "W", "INR", "u1").toJson();

    EXPECT_EQ(Account::fromJson(cashJson)->accountTypeName(), "Cash");
    EXPECT_EQ(Account::fromJson(savJson)->accountTypeName(),  "Savings");
    EXPECT_EQ(Account::fromJson(curJson)->accountTypeName(),  "Current");
    EXPECT_EQ(Account::fromJson(ccJson)->accountTypeName(),   "Credit Card");
    EXPECT_EQ(Account::fromJson(walJson)->accountTypeName(),  "Wallet");
}

TEST(AccountTest, StatusStringConversion)
{
    EXPECT_EQ(accountStatusToString(AccountStatus::ACTIVE), "active");
    EXPECT_EQ(accountStatusToString(AccountStatus::FROZEN), "frozen");
    EXPECT_EQ(accountStatusToString(AccountStatus::CLOSED), "closed");

    EXPECT_EQ(accountStatusFromString("active"), AccountStatus::ACTIVE);
    EXPECT_EQ(accountStatusFromString("frozen"), AccountStatus::FROZEN);
    EXPECT_EQ(accountStatusFromString("closed"), AccountStatus::CLOSED);
    EXPECT_EQ(accountStatusFromString("unknown"), AccountStatus::ACTIVE);
}
