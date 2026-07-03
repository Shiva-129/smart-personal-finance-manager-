/**
 * @file    test_budget.cpp
 * @brief   Unit tests for Budget model.
 */

#include <gtest/gtest.h>
#include "finance/models/Budget.h"

using namespace finance::models;

TEST(BudgetTest, CreateAndAccess)
{
    Budget b("budget1", "cat_food", "user1", 2026, 7, 6000.0);
    EXPECT_EQ(b.id(), "budget1");
    EXPECT_EQ(b.categoryId(), "cat_food");
    EXPECT_EQ(b.userId(), "user1");
    EXPECT_EQ(b.year(), 2026);
    EXPECT_EQ(b.month(), 7);
    EXPECT_DOUBLE_EQ(b.limitAmount(), 6000.0);
    EXPECT_DOUBLE_EQ(b.spentAmount(), 0.0);
}

TEST(BudgetTest, Remaining)
{
    Budget b("id", "cat", "u", 2026, 1, 5000.0);
    b.setSpentAmount(3200.0);
    EXPECT_DOUBLE_EQ(b.remaining(), 1800.0);
}

TEST(BudgetTest, PercentageUsed)
{
    Budget b("id", "cat", "u", 2026, 1, 1000.0);
    EXPECT_DOUBLE_EQ(b.percentageUsed(), 0.0);

    b.setSpentAmount(250.0);
    EXPECT_DOUBLE_EQ(b.percentageUsed(), 25.0);

    b.setSpentAmount(1000.0);
    EXPECT_DOUBLE_EQ(b.percentageUsed(), 100.0);
}

TEST(BudgetTest, IsExceeded)
{
    Budget b("id", "cat", "u", 2026, 1, 1000.0);
    EXPECT_FALSE(b.isExceeded());

    b.setSpentAmount(1000.0);
    EXPECT_FALSE(b.isExceeded()); // exactly at limit

    b.setSpentAmount(1000.01);
    EXPECT_TRUE(b.isExceeded());
}

TEST(BudgetTest, AddSpending)
{
    Budget b("id", "cat", "u", 2026, 1, 5000.0);
    b.addSpending(1000.0);
    EXPECT_DOUBLE_EQ(b.spentAmount(), 1000.0);
    b.addSpending(500.0);
    EXPECT_DOUBLE_EQ(b.spentAmount(), 1500.0);
}

TEST(BudgetTest, SerializationRoundTrip)
{
    Budget original("bid", "cat_food", "u1", 2026, 7, 6000.0);
    original.setSpentAmount(4200.0);

    auto json = original.toJson();
    auto restored = Budget::fromJson(json);

    EXPECT_EQ(original.id(),         restored.id());
    EXPECT_EQ(original.categoryId(), restored.categoryId());
    EXPECT_EQ(original.year(),       restored.year());
    EXPECT_EQ(original.month(),      restored.month());
    EXPECT_DOUBLE_EQ(original.limitAmount(), restored.limitAmount());
    EXPECT_DOUBLE_EQ(original.spentAmount(), restored.spentAmount());
}

TEST(BudgetTest, PercentageUsedWithZeroLimit)
{
    Budget b("id", "cat", "u", 2026, 1, 0.0);
    EXPECT_DOUBLE_EQ(b.percentageUsed(), 0.0);
}
