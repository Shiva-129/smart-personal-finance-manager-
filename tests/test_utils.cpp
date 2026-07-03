/**
 * @file    test_utils.cpp
 * @brief   Unit tests for utility classes: UUID, DateUtils.
 */

#include <gtest/gtest.h>
#include "finance/utils/DateUtils.h"
#include "finance/utils/UUID.h"

using namespace finance::utils;

// ── UUID Tests ──────────────────────────────────────────────────────

TEST(UUIDTest, GenerateReturnsValidFormat)
{
    auto id = UUID::generate();
    EXPECT_TRUE(UUID::isValid(id));
    EXPECT_EQ(id.size(), 36UL);
}

TEST(UUIDTest, GenerateProducesUniqueIds)
{
    auto a = UUID::generate();
    auto b = UUID::generate();
    EXPECT_NE(a, b);
}

TEST(UUIDTest, IsValidRejectsBadStrings)
{
    EXPECT_FALSE(UUID::isValid(""));
    EXPECT_FALSE(UUID::isValid("not-a-uuid"));
    EXPECT_FALSE(UUID::isValid("00000000-0000-0000-0000-00000000000Z")); // bad char
}

// ── DateUtils Tests ─────────────────────────────────────────────────

TEST(DateUtilsTest, TodayReturnsValidFormat)
{
    auto today = DateUtils::today();
    EXPECT_EQ(today.size(), 10UL); // "YYYY-MM-DD"
    EXPECT_EQ(today[4], '-');
    EXPECT_EQ(today[7], '-');
}

TEST(DateUtilsTest, TimestampContainsDateAndTime)
{
    auto ts = DateUtils::timestamp();
    EXPECT_GE(ts.size(), 19UL); // "YYYY-MM-DDTHH:MM:SS"
    EXPECT_EQ(ts[10], 'T');
}

TEST(DateUtilsTest, FormatAndParseRoundTrip)
{
    auto ts = DateUtils::timestamp();
    auto tp = DateUtils::parseTimestamp(ts);
    auto formatted = DateUtils::formatTimestamp(tp);
    EXPECT_EQ(ts, formatted);
}

TEST(DateUtilsTest, ParseInvalidDateReturnsDefault)
{
    auto tp = DateUtils::parseDate("not-a-date");
    // A default-constructed TimePoint is the epoch.
    DateUtils::TimePoint defaultTp{};
    EXPECT_EQ(tp, defaultTp);
}

TEST(DateUtilsTest, ExtractYMD)
{
    auto [y, m, d] = DateUtils::extractYMD("2026-07-04");
    EXPECT_EQ(y, 2026);
    EXPECT_EQ(m, 7);
    EXPECT_EQ(d, 4);
}

TEST(DateUtilsTest, MonthName)
{
    EXPECT_EQ(DateUtils::monthName(1),   "January");
    EXPECT_EQ(DateUtils::monthName(6),   "June");
    EXPECT_EQ(DateUtils::monthName(12),  "December");
    EXPECT_EQ(DateUtils::monthName(13),  "Unknown");
    EXPECT_EQ(DateUtils::monthName(0),   "Unknown");
}

TEST(DateUtilsTest, DaysBetween)
{
    int days = DateUtils::daysBetween("2026-01-01", "2026-01-10");
    EXPECT_EQ(days, 9);
}

TEST(DateUtilsTest, IsFuture)
{
    // 2099 is definitely in the future.
    EXPECT_TRUE(DateUtils::isFuture("2099-12-31"));
    // 1999 is definitely in the past.
    EXPECT_FALSE(DateUtils::isFuture("1999-01-01"));
}
