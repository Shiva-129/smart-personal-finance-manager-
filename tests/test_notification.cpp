/**
 * @file    test_notification.cpp
 * @brief   Unit tests for Notification model.
 */

#include <gtest/gtest.h>
#include "finance/models/Notification.h"

using namespace finance::models;

TEST(NotificationTest, CreateAndAccess)
{
    Notification n("nid1", "user1", NotificationType::BUDGET_EXCEEDED,
                   "Budget exceeded for Food");
    EXPECT_EQ(n.id(), "nid1");
    EXPECT_EQ(n.userId(), "user1");
    EXPECT_EQ(n.type(), NotificationType::BUDGET_EXCEEDED);
    EXPECT_EQ(n.message(), "Budget exceeded for Food");
    EXPECT_FALSE(n.isRead());
    EXPECT_FALSE(n.createdAt().empty());
}

TEST(NotificationTest, MarkRead)
{
    Notification n("nid", "u1", NotificationType::INFO, "Test");
    EXPECT_FALSE(n.isRead());
    n.markRead();
    EXPECT_TRUE(n.isRead());
}

TEST(NotificationTest, SerializationRoundTrip)
{
    Notification original("nid2", "user2", NotificationType::GOAL_ACHIEVED,
                          "Congratulations! Goal achieved!");
    original.markRead();

    auto json = original.toJson();
    auto restored = Notification::fromJson(json);

    EXPECT_EQ(original.id(),       restored.id());
    EXPECT_EQ(original.userId(),   restored.userId());
    EXPECT_EQ(original.type(),     restored.type());
    EXPECT_EQ(original.message(),  restored.message());
    EXPECT_EQ(original.isRead(),   restored.isRead());
}

TEST(NotificationTest, TypeStringConversion)
{
    EXPECT_EQ(Notification::typeToString(NotificationType::BUDGET_EXCEEDED),   "budget_exceeded");
    EXPECT_EQ(Notification::typeToString(NotificationType::GOAL_ACHIEVED),     "goal_achieved");
    EXPECT_EQ(Notification::typeToString(NotificationType::GOAL_PROGRESS),     "goal_progress");
    EXPECT_EQ(Notification::typeToString(NotificationType::RECURRING_PAYMENT), "recurring_payment");
    EXPECT_EQ(Notification::typeToString(NotificationType::LOW_BALANCE),       "low_balance");
    EXPECT_EQ(Notification::typeToString(NotificationType::INFO),              "info");

    EXPECT_EQ(Notification::typeFromString("budget_exceeded"),   NotificationType::BUDGET_EXCEEDED);
    EXPECT_EQ(Notification::typeFromString("goal_achieved"),     NotificationType::GOAL_ACHIEVED);
    EXPECT_EQ(Notification::typeFromString("goal_progress"),     NotificationType::GOAL_PROGRESS);
    EXPECT_EQ(Notification::typeFromString("recurring_payment"), NotificationType::RECURRING_PAYMENT);
    EXPECT_EQ(Notification::typeFromString("low_balance"),       NotificationType::LOW_BALANCE);
    EXPECT_EQ(Notification::typeFromString("info"),              NotificationType::INFO);
    EXPECT_EQ(Notification::typeFromString("unknown"),           NotificationType::INFO);
}
