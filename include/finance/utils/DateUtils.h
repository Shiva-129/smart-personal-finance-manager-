#ifndef FINANCE_UTILS_DATEUTILS_H
#define FINANCE_UTILS_DATEUTILS_H

#include <chrono>
#include <string>
#include <tuple>

namespace finance::utils {

/**
 * @brief Date and time utility functions.
 *
 * All dates are stored as ISO-8601 strings:  "YYYY-MM-DD"
 * All times are stored as             :  "HH:MM:SS"
 * Combined timestamps                 :  "YYYY-MM-DDTHH:MM:SS"
 */
class DateUtils {
public:
    using Clock     = std::chrono::system_clock;
    using TimePoint = Clock::time_point;

    // ── Format helpers ──────────────────────────────────────────────

    /// Return current date as "YYYY-MM-DD".
    static std::string today();

    /// Return current time as "HH:MM:SS".
    static std::string now();

    /// Return current date-time as "YYYY-MM-DDTHH:MM:SS".
    static std::string timestamp();

    /// Format a time_point as "YYYY-MM-DD".
    static std::string formatDate(const TimePoint& tp);

    /// Format a time_point as "HH:MM:SS".
    static std::string formatTime(const TimePoint& tp);

    /// Format a time_point as "YYYY-MM-DDTHH:MM:SS".
    static std::string formatTimestamp(const TimePoint& tp);

    // ── Parsing ─────────────────────────────────────────────────────

    /// Parse "YYYY-MM-DD" to a time_point.  Returns epoch on failure.
    static TimePoint parseDate(const std::string& dateStr);

    /// Parse "YYYY-MM-DDTHH:MM:SS" to a time_point.
    static TimePoint parseTimestamp(const std::string& ts);

    // ── Extraction ──────────────────────────────────────────────────

    /// Extract {year, month, day} from a date string.
    static std::tuple<int, int, int> extractYMD(const std::string& dateStr);

    /// Get the month name (e.g. "January").
    static std::string monthName(int month);

    // ── Comparators ─────────────────────────────────────────────────

    static int daysBetween(const std::string& start, const std::string& end);
    static bool isFuture(const std::string& dateStr);
    static bool isPast(const std::string& dateStr);

private:
    DateUtils() = default;
};

}  // namespace finance::utils

#endif  // FINANCE_UTILS_DATEUTILS_H
