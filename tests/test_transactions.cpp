/**
 * @file    test_transactions.cpp
 * @brief   Unit tests for Transaction hierarchy and factory.
 */

#include <gtest/gtest.h>
#include "finance/Models.h"
#include "finance/Utils.h"

using namespace finance::models;
using namespace finance::utils;

static std::string makeId() { return UUID::generate(); }

// ── Income Tests ────────────────────────────────────────────────────

TEST(IncomeTest, CreateAndAccess)
{
    Income inc(makeId(), 50000.0, "Monthly salary",
               "cat_salary", "acc_bank", "user1");
    EXPECT_EQ(inc.amount(), 50000.0);
    EXPECT_EQ(inc.description(), "Monthly salary");
    EXPECT_EQ(inc.transactionTypeName(), "Income");
    EXPECT_FALSE(inc.date().empty());
    EXPECT_FALSE(inc.time().empty());
}

TEST(IncomeTest, SerializationRoundTrip)
{
    Income original(makeId(), 7500.0, "Freelance project",
                    "cat_freelance", "acc_main", "user2",
                    "2026-07-04", "14:30:00");
    original.setTags({"work", "urgent"});
    original.setNotes("Invoice #42");

    auto json = original.toJson();
    std::unique_ptr<Transaction> restored = Transaction::fromJson(json);

    EXPECT_EQ(original.id(),          restored->id());
    EXPECT_EQ(original.amount(),      restored->amount());
    EXPECT_EQ(original.description(), restored->description());
    EXPECT_EQ(original.transactionTypeName(), restored->transactionTypeName());
    ASSERT_EQ(original.tags().size(), restored->tags().size());
    EXPECT_EQ(original.tags()[0],     restored->tags()[0]);
}

TEST(IncomeTest, ClonePreservesData)
{
    Income original(makeId(), 3000.0, "Gift", "cat_gift", "acc1", "u1");
    auto clone = original.clone();

    EXPECT_EQ(original.id(),       clone->id());
    EXPECT_EQ(original.amount(),   clone->amount());
    EXPECT_EQ(original.description(), clone->description());
}

// ── Expense Tests ───────────────────────────────────────────────────

TEST(ExpenseTest, CreateAndAccess)
{
    Expense exp(makeId(), 1200.0, "Lunch",
                "cat_food", "acc_wallet", "user1");
    EXPECT_EQ(exp.transactionTypeName(), "Expense");
    EXPECT_FALSE(exp.isRecurring());
}

TEST(ExpenseTest, RecurringFlag)
{
    Expense exp(makeId(), 15000.0, "Rent",
                "cat_rent", "acc_bank", "user1");
    exp.setRecurring(true);
    exp.setRecurrenceRule("monthly");
    EXPECT_TRUE(exp.isRecurring());
    EXPECT_EQ(exp.recurrenceRule(), "monthly");
}

TEST(ExpenseTest, SerializationRecurring)
{
    Expense original(makeId(), 999.0, "Netflix",
                     "cat_entertainment", "acc_cc", "u1",
                     "2026-07-01", "10:00:00");
    original.setRecurring(true);
    original.setRecurrenceRule("monthly");

    auto json = original.toJson();
    auto restored = Transaction::fromJson(json);
    ASSERT_NE(restored, nullptr);
    EXPECT_EQ(restored->transactionTypeName(), "Expense");

    // dynamic_cast would be better but we don't have RTTI issues here.
    // We rely on factory producing the correct type.
    EXPECT_TRUE(restored->transactionTypeName() == "Expense");
}

TEST(ExpenseTest, Mutators)
{
    Expense exp(makeId(), 100.0, "Old desc", "cat1", "acc1", "u1");
    exp.setAmount(200.0);
    exp.setDescription("New desc");
    exp.setCategoryId("cat2");

    EXPECT_DOUBLE_EQ(exp.amount(), 200.0);
    EXPECT_EQ(exp.description(), "New desc");
    EXPECT_EQ(exp.categoryId(), "cat2");
}

// ── Transfer Tests ──────────────────────────────────────────────────

TEST(TransferTest, CreateAndAccess)
{
    Transfer tr(makeId(), 5000.0, "Savings transfer",
                "", "acc_savings", "user1");
    tr.setToAccountId("acc_checking");
    EXPECT_EQ(tr.transactionTypeName(), "Transfer");
    EXPECT_EQ(tr.toAccountId(), "acc_checking");
}

TEST(TransferTest, SerializationRoundTrip)
{
    Transfer original(makeId(), 10000.0, "To savings",
                      "", "acc_checking", "u1");
    original.setToAccountId("acc_savings");

    auto json = original.toJson();
    auto restored = Transaction::fromJson(json);

    EXPECT_EQ(original.transactionTypeName(), restored->transactionTypeName());
    EXPECT_EQ(original.amount(), restored->amount());
}

// ── Factory Tests ───────────────────────────────────────────────────

TEST(TransactionFactoryTest, CreatesCorrectTypes)
{
    auto incomeJson   = Income(makeId(), 100, "", "c", "a", "u").toJson();
    auto expenseJson  = Expense(makeId(), 100, "", "c", "a", "u").toJson();
    auto transferJson = Transfer(makeId(), 100, "", "c", "a", "u").toJson();

    EXPECT_EQ(Transaction::fromJson(incomeJson)->transactionTypeName(),   "Income");
    EXPECT_EQ(Transaction::fromJson(expenseJson)->transactionTypeName(),  "Expense");
    EXPECT_EQ(Transaction::fromJson(transferJson)->transactionTypeName(), "Transfer");
}

TEST(TransactionFactoryTest, UnknownTypeDefaultsToExpense)
{
    nlohmann::json j = {
        {"id", "x"}, {"amount", 50.0}, {"description", "fallback"},
        {"type", "UnknownType"}
    };
    auto txn = Transaction::fromJson(j);
    EXPECT_EQ(txn->transactionTypeName(), "Expense");
}
