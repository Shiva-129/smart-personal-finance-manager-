/**
 * @file    test_categories.cpp
 * @brief   Unit tests for Category model.
 */

#include <gtest/gtest.h>
#include "finance/Models.h"

using namespace finance::models;

TEST(CategoryTest, DefaultsCreatesExpectedCount)
{
    auto cats = Category::defaults();
    // 5 income + 11 expense = 16 default categories.
    EXPECT_EQ(cats.size(), 16UL);
}

TEST(CategoryTest, DefaultsAreMarkedDefault)
{
    auto cats = Category::defaults();
    for (const auto& c : cats) {
        EXPECT_TRUE(c.isDefault());
    }
}

TEST(CategoryTest, DefaultsHaveDeterministicIds)
{
    auto first  = Category::defaults();
    auto second = Category::defaults();
    ASSERT_EQ(first.size(), second.size());
    for (size_t i = 0; i < first.size(); ++i) {
        EXPECT_EQ(first[i].id(), second[i].id());
        EXPECT_EQ(first[i].name(), second[i].name());
    }
}

TEST(CategoryTest, SerializationRoundTrip)
{
    Category original("test-id", "Groceries", CategoryType::EXPENSE, false);
    auto json = original.toJson();
    auto restored = Category::fromJson(json);

    EXPECT_EQ(original.id(),   restored.id());
    EXPECT_EQ(original.name(), restored.name());
    EXPECT_EQ(original.type(), restored.type());
    EXPECT_EQ(original.isDefault(), restored.isDefault());
}

TEST(CategoryTest, TypeToStringAndBack)
{
    EXPECT_EQ(Category::typeToString(CategoryType::INCOME),  "income");
    EXPECT_EQ(Category::typeToString(CategoryType::EXPENSE), "expense");
    EXPECT_EQ(Category::typeToString(CategoryType::BOTH),    "both");

    EXPECT_EQ(Category::typeFromString("income"),  CategoryType::INCOME);
    EXPECT_EQ(Category::typeFromString("expense"), CategoryType::EXPENSE);
    EXPECT_EQ(Category::typeFromString("both"),    CategoryType::BOTH);
    EXPECT_EQ(Category::typeFromString("unknown"), CategoryType::EXPENSE); // default
}

TEST(CategoryTest, SetNameAndType)
{
    Category cat("id", "Old", CategoryType::EXPENSE);
    cat.setName("New");
    cat.setType(CategoryType::INCOME);

    EXPECT_EQ(cat.name(), "New");
    EXPECT_EQ(cat.type(), CategoryType::INCOME);
}
