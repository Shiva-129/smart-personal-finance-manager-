#include <gtest/gtest.h>
#include "finance/Utils.h"

using namespace finance::utils;

TEST(UUIDTest, GenerateReturnsValidFormat) {
    auto id = UUID::generate();
    EXPECT_TRUE(UUID::isValid(id));
    EXPECT_EQ(id.size(), 36UL);
}

TEST(UUIDTest, GenerateProducesUniqueIds) {
    EXPECT_NE(UUID::generate(), UUID::generate());
}

TEST(UUIDTest, IsValidRejectsBadStrings) {
    EXPECT_FALSE(UUID::isValid(""));
    EXPECT_FALSE(UUID::isValid("not-a-uuid"));
}

TEST(DateUtilsTest, TodayReturnsValidFormat) {
    auto t = DateUtils::today();
    EXPECT_EQ(t.size(), 10UL);
    EXPECT_EQ(t[4], '-');
}

TEST(DateUtilsTest, ParseInvalidDateReturnsDefault) {
    auto tp = DateUtils::parseDate("not-a-date");
    DateUtils::TimePoint defaultTp{};
    EXPECT_EQ(tp, defaultTp);
}

TEST(DateUtilsTest, ExtractYMD) {
    auto [y, m, d] = DateUtils::extractYMD("2026-07-04");
    EXPECT_EQ(y, 2026); EXPECT_EQ(m, 7); EXPECT_EQ(d, 4);
}

TEST(DateUtilsTest, MonthName) {
    EXPECT_EQ(DateUtils::monthName(1), "January");
    EXPECT_EQ(DateUtils::monthName(13), "Unknown");
}

TEST(DateUtilsTest, DaysBetween) {
    EXPECT_EQ(DateUtils::daysBetween("2026-01-01", "2026-01-10"), 9);
}

TEST(DateUtilsTest, IsFuture) {
    EXPECT_TRUE(DateUtils::isFuture("2099-12-31"));
    EXPECT_FALSE(DateUtils::isFuture("1999-01-01"));
}
