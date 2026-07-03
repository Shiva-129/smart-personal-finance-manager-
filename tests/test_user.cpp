/**
 * @file    test_user.cpp
 * @brief   Unit tests for User model.
 */

#include <gtest/gtest.h>
#include "finance/models/User.h"
#include "finance/utils/UUID.h"

using namespace finance::models;

TEST(UserTest, CreateAndAccess)
{
    User u("uid1", "johndoe", "hash123", "John Doe");
    EXPECT_EQ(u.id(), "uid1");
    EXPECT_EQ(u.username(), "johndoe");
    EXPECT_EQ(u.passwordHash(), "hash123");
    EXPECT_EQ(u.displayName(), "John Doe");
    EXPECT_TRUE(u.isActive());
    EXPECT_FALSE(u.createdAt().empty());
}

TEST(UserTest, DisplayNameDefaultsToUsername)
{
    User u("uid2", "janedoe", "hash456", "");
    EXPECT_EQ(u.displayName(), "janedoe");
}

TEST(UserTest, SerializationRoundTrip)
{
    User original("uid3", "bob", "salt:hash", "Bob Smith");
    original.setLastLoginAt("2026-07-04T10:30:00");

    auto json = original.toJson();
    auto restored = User::fromJson(json);

    EXPECT_EQ(original.id(),           restored.id());
    EXPECT_EQ(original.username(),     restored.username());
    EXPECT_EQ(original.passwordHash(), restored.passwordHash());
    EXPECT_EQ(original.displayName(),  restored.displayName());
    EXPECT_EQ(original.isActive(),     restored.isActive());
}

TEST(UserTest, SetActive)
{
    User u("id", "test", "hash", "Test");
    u.setActive(false);
    EXPECT_FALSE(u.isActive());
    u.setActive(true);
    EXPECT_TRUE(u.isActive());
}
