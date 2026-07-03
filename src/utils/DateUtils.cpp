#include "finance/utils/DateUtils.h"

#include <array>
#include <cstdio>
#include <ctime>
#include <iomanip>
#include <sstream>

namespace finance::utils {

// ── Format helpers ──────────────────────────────────────────────────

std::string DateUtils::today()
{
    return formatDate(Clock::now());
}

std::string DateUtils::now()
{
    return formatTime(Clock::now());
}

std::string DateUtils::timestamp()
{
    return formatTimestamp(Clock::now());
}

std::string DateUtils::formatDate(const TimePoint& tp)
{
    auto tt = Clock::to_time_t(tp);
    auto tm = *std::localtime(&tt);
    std::ostringstream oss;
    oss << std::put_time(&tm, "%Y-%m-%d");
    return oss.str();
}

std::string DateUtils::formatTime(const TimePoint& tp)
{
    auto tt = Clock::to_time_t(tp);
    auto tm = *std::localtime(&tt);
    std::ostringstream oss;
    oss << std::put_time(&tm, "%H:%M:%S");
    return oss.str();
}

std::string DateUtils::formatTimestamp(const TimePoint& tp)
{
    auto tt = Clock::to_time_t(tp);
    auto tm = *std::localtime(&tt);
    std::ostringstream oss;
    oss << std::put_time(&tm, "%Y-%m-%dT%H:%M:%S");
    return oss.str();
}

// ── Parsing ─────────────────────────────────────────────────────────

DateUtils::TimePoint DateUtils::parseDate(const std::string& dateStr)
{
    std::tm tm = {};
    tm.tm_isdst = -1;  // let the system determine DST
    std::istringstream ss(dateStr);
    ss >> std::get_time(&tm, "%Y-%m-%d");
    if (ss.fail()) {
        return TimePoint{};
    }
    return Clock::from_time_t(std::mktime(&tm));
}

DateUtils::TimePoint DateUtils::parseTimestamp(const std::string& ts)
{
    std::tm tm = {};
    tm.tm_isdst = -1;  // let the system determine DST
    std::istringstream ss(ts);
    ss >> std::get_time(&tm, "%Y-%m-%dT%H:%M:%S");
    if (ss.fail()) {
        return TimePoint{};
    }
    return Clock::from_time_t(std::mktime(&tm));
}

// ── Extraction ──────────────────────────────────────────────────────

std::tuple<int, int, int> DateUtils::extractYMD(const std::string& dateStr)
{
    int y = 0, m = 0, d = 0;
    std::sscanf(dateStr.c_str(), "%d-%d-%d", &y, &m, &d);
    return {y, m, d};
}

std::string DateUtils::monthName(int month)
{
    static const std::array<std::string, 12> names = {
        "January", "February", "March", "April", "May", "June",
        "July", "August", "September", "October", "November", "December"
    };
    if (month < 1 || month > 12) return "Unknown";
    return names[month - 1];
}

// ── Comparators ─────────────────────────────────────────────────────

int DateUtils::daysBetween(const std::string& start, const std::string& end)
{
    auto tp1 = parseDate(start);
    auto tp2 = parseDate(end);
    auto diff = tp2 - tp1;
    return static_cast<int>(std::chrono::duration_cast<std::chrono::hours>(diff).count() / 24);
}

bool DateUtils::isFuture(const std::string& dateStr)
{
    auto tp = parseDate(dateStr);
    return tp > Clock::now();
}

bool DateUtils::isPast(const std::string& dateStr)
{
    auto tp = parseDate(dateStr);
    return tp < Clock::now();
}

}  // namespace finance::utils
