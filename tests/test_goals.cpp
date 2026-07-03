/**
 * @file    test_goals.cpp
 * @brief   Unit tests for Goal model.
 */

#include <gtest/gtest.h>
#include "finance/models/Goal.h"

using namespace finance::models;

TEST(GoalTest, CreateAndAccess)
{
    Goal g("goal1", "user1", "Buy Laptop", 100000.0);
    EXPECT_EQ(g.name(), "Buy Laptop");
    EXPECT_DOUBLE_EQ(g.targetAmount(), 100000.0);
    EXPECT_DOUBLE_EQ(g.currentAmount(), 0.0);
    EXPECT_EQ(g.status(), GoalStatus::IN_PROGRESS);
    EXPECT_FALSE(g.isAchieved());
}

TEST(GoalTest, ProgressPercentage)
{
    Goal g("id", "u", "Test", 1000.0);
    EXPECT_DOUBLE_EQ(g.progressPercentage(), 0.0);

    g.addAmount(250.0);
    EXPECT_DOUBLE_EQ(g.progressPercentage(), 25.0);

    g.addAmount(250.0);
    EXPECT_DOUBLE_EQ(g.progressPercentage(), 50.0);
}

TEST(GoalTest, ProgressCapsAt100)
{
    Goal g("id", "u", "Test", 1000.0);
    g.addAmount(1500.0);
    EXPECT_DOUBLE_EQ(g.progressPercentage(), 100.0);
}

TEST(GoalTest, MarkAchieved)
{
    Goal g("id", "u", "Test", 1000.0);
    g.addAmount(1000.0);
    g.markAchieved();
    EXPECT_TRUE(g.isAchieved());
    EXPECT_EQ(g.status(), GoalStatus::ACHIEVED);
}

TEST(GoalTest, AddAmount)
{
    Goal g("id", "u", "Test", 5000.0);
    g.addAmount(2000.0);
    EXPECT_DOUBLE_EQ(g.currentAmount(), 2000.0);
    g.addAmount(1500.0);
    EXPECT_DOUBLE_EQ(g.currentAmount(), 3500.0);
}

TEST(GoalTest, SerializationRoundTrip)
{
    Goal original("gid", "u1", "Emergency Fund", 50000.0, "2026-12-31");
    original.addAmount(12000.0);

    auto json = original.toJson();
    auto restored = Goal::fromJson(json);

    EXPECT_EQ(original.id(),             restored.id());
    EXPECT_EQ(original.name(),           restored.name());
    EXPECT_DOUBLE_EQ(original.targetAmount(), restored.targetAmount());
    EXPECT_DOUBLE_EQ(original.currentAmount(), restored.currentAmount());
    EXPECT_EQ(original.deadline(),       restored.deadline());
    EXPECT_EQ(original.status(),         restored.status());
}

TEST(GoalTest, StatusStringConversion)
{
    EXPECT_EQ(Goal::statusToString(GoalStatus::IN_PROGRESS), "in_progress");
    EXPECT_EQ(Goal::statusToString(GoalStatus::ACHIEVED),    "achieved");
    EXPECT_EQ(Goal::statusToString(GoalStatus::CANCELLED),   "cancelled");

    EXPECT_EQ(Goal::statusFromString("in_progress"), GoalStatus::IN_PROGRESS);
    EXPECT_EQ(Goal::statusFromString("achieved"),    GoalStatus::ACHIEVED);
    EXPECT_EQ(Goal::statusFromString("cancelled"),   GoalStatus::CANCELLED);
    EXPECT_EQ(Goal::statusFromString("unknown"),     GoalStatus::IN_PROGRESS);
}

TEST(GoalTest, ProgressWithZeroTarget)
{
    Goal g("id", "u", "Test", 0.0);
    EXPECT_DOUBLE_EQ(g.progressPercentage(), 0.0);
}
