#include "finance/Utils.h"

#include <algorithm>
#include <array>
#include <chrono>
#include <cstdio>
#include <ctime>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <random>
#include <regex>
#include <sstream>

namespace finance::utils {

// ══════════════════════════════════════════════════════════════════════
// UUID
// ══════════════════════════════════════════════════════════════════════

std::string UUID::generate()
{
    static thread_local std::mt19937_64 rng(std::random_device{}());
    uint64_t hi = rng(), lo = rng();
    hi &= ~0xf000ULL; hi |= 0x4000ULL;
    lo &= ~0xc000000000000000ULL; lo |= 0x8000000000000000ULL;
    std::array<char, 37> buf{};
    std::snprintf(buf.data(), buf.size(), "%08lx-%04lx-%04lx-%04lx-%012lx",
        static_cast<unsigned long>(hi >> 32),
        static_cast<unsigned long>((hi >> 16) & 0xffff),
        static_cast<unsigned long>(hi & 0xffff),
        static_cast<unsigned long>(lo >> 48),
        static_cast<unsigned long>(lo & 0xffffffffffffULL));
    return buf.data();
}

bool UUID::isValid(const std::string& uuid)
{
    static const std::regex re(
        "^[0-9a-f]{8}-[0-9a-f]{4}-4[0-9a-f]{3}-[89ab][0-9a-f]{3}-[0-9a-f]{12}$",
        std::regex::icase);
    return std::regex_match(uuid, re);
}

// ══════════════════════════════════════════════════════════════════════
// DateUtils
// ══════════════════════════════════════════════════════════════════════

std::string DateUtils::today() { return formatDate(Clock::now()); }
std::string DateUtils::now()   { return formatTime(Clock::now()); }
std::string DateUtils::timestamp() { return formatTimestamp(Clock::now()); }

std::string DateUtils::formatDate(const TimePoint& tp) {
    auto tt = Clock::to_time_t(tp);
    auto tm = *std::localtime(&tt);
    std::ostringstream oss; oss << std::put_time(&tm, "%Y-%m-%d"); return oss.str();
}

std::string DateUtils::formatTime(const TimePoint& tp) {
    auto tt = Clock::to_time_t(tp);
    auto tm = *std::localtime(&tt);
    std::ostringstream oss; oss << std::put_time(&tm, "%H:%M:%S"); return oss.str();
}

std::string DateUtils::formatTimestamp(const TimePoint& tp) {
    auto tt = Clock::to_time_t(tp);
    auto tm = *std::localtime(&tt);
    std::ostringstream oss; oss << std::put_time(&tm, "%Y-%m-%dT%H:%M:%S"); return oss.str();
}

DateUtils::TimePoint DateUtils::parseDate(const std::string& dateStr) {
    std::tm tm = {}; tm.tm_isdst = -1;
    std::istringstream ss(dateStr); ss >> std::get_time(&tm, "%Y-%m-%d");
    if (ss.fail()) return TimePoint{};
    return Clock::from_time_t(std::mktime(&tm));
}

DateUtils::TimePoint DateUtils::parseTimestamp(const std::string& ts) {
    std::tm tm = {}; tm.tm_isdst = -1;
    std::istringstream ss(ts); ss >> std::get_time(&tm, "%Y-%m-%dT%H:%M:%S");
    if (ss.fail()) return TimePoint{};
    return Clock::from_time_t(std::mktime(&tm));
}

std::tuple<int,int,int> DateUtils::extractYMD(const std::string& d) {
    int y=0,m=0,day=0; std::sscanf(d.c_str(),"%d-%d-%d",&y,&m,&day); return {y,m,day};
}

std::string DateUtils::monthName(int month) {
    static const std::array<std::string,12> n{"January","February","March","April","May","June",
        "July","August","September","October","November","December"};
    return (month<1||month>12) ? "Unknown" : n[month-1];
}

int DateUtils::daysBetween(const std::string& s, const std::string& e) {
    auto diff = parseDate(e) - parseDate(s);
    return static_cast<int>(std::chrono::duration_cast<std::chrono::hours>(diff).count()/24);
}

bool DateUtils::isFuture(const std::string& d) { return parseDate(d) > Clock::now(); }
bool DateUtils::isPast(const std::string& d)  { return parseDate(d) < Clock::now(); }

// ══════════════════════════════════════════════════════════════════════
// Logger
// ══════════════════════════════════════════════════════════════════════

Logger& Logger::instance() { static Logger inst; return inst; }
Logger::~Logger() { if (file_.is_open()) file_.close(); }

void Logger::setLevel(LogLevel l) { std::lock_guard<std::mutex> lock(mutex_); level_ = l; }

void Logger::setLogFile(const std::string& p) {
    std::lock_guard<std::mutex> lock(mutex_);
    if (file_.is_open()) file_.close();
    file_.open(p, std::ios::app);
}

void Logger::log(LogLevel level, const std::string& msg) {
    std::lock_guard<std::mutex> lock(mutex_);
    if (level < level_) return;
    auto now = DateUtils::Clock::to_time_t(DateUtils::Clock::now());
    auto tm = *std::localtime(&now);
    std::ostringstream oss;
    oss << std::put_time(&tm, "%Y-%m-%dT%H:%M:%S") << " [" << levelToString(level) << "] " << msg;
    std::string f = oss.str();
    std::cout << f << std::endl;
    if (file_.is_open()) file_ << f << std::endl;
}

std::string Logger::levelToString(LogLevel l) {
    switch(l) {
        case LogLevel::DEBUG: return "DEBUG";
        case LogLevel::INFO: return "INFO";
        case LogLevel::WARNING: return "WARN";
        case LogLevel::ERROR: return "ERROR";
    }
    return "UNKNOWN";
}

// ══════════════════════════════════════════════════════════════════════
// Settings
// ══════════════════════════════════════════════════════════════════════

Settings& Settings::instance() { static Settings inst; return inst; }
void Settings::load() { loadFrom(filePath_); }

void Settings::loadFrom(const std::string& path) {
    filePath_ = path;
    std::ifstream ifs(path);
    if (ifs.is_open()) { try { ifs >> data_; } catch(...) { data_ = nlohmann::json::object(); } }
    else data_ = nlohmann::json::object();
}

void Settings::save() const { saveTo(filePath_); }
void Settings::saveTo(const std::string& path) const {
    std::ofstream ofs(path);
    if (ofs.is_open()) ofs << data_.dump(4) << std::endl;
}

std::string Settings::get(const std::string& k, const std::string& d) const {
    auto it = data_.find(k); return (it != data_.end() && it->is_string()) ? it->get<std::string>() : d;
}
int Settings::get(const std::string& k, int d) const {
    auto it = data_.find(k); return (it != data_.end() && it->is_number_integer()) ? it->get<int>() : d;
}
bool Settings::get(const std::string& k, bool d) const {
    auto it = data_.find(k); return (it != data_.end() && it->is_boolean()) ? it->get<bool>() : d;
}
void Settings::set(const std::string& k, const std::string& v) { data_[k] = v; }
void Settings::set(const std::string& k, int v) { data_[k] = v; }
void Settings::set(const std::string& k, bool v) { data_[k] = v; }

}  // namespace finance::utils
