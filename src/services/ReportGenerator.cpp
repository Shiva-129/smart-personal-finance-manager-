#include "finance/services/ReportGenerator.h"
#include "finance/utils/DateUtils.h"

#include <algorithm>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <map>
#include <numeric>
#include <sstream>

namespace finance::services {

ReportGenerator::ReportGenerator(storage::StorageManager& storage)
    : storage_(storage)
{
}

// ── Report builders ─────────────────────────────────────────────────

ReportGenerator::Report ReportGenerator::buildReport(
    const std::vector<models::Transaction*>& txns,
    const std::string& label) const
{
    Report r;
    r.periodLabel = label;

    // Category aggregators.
    std::map<std::string, double> catIncome;
    std::map<std::string, double> catExpense;
    std::map<std::string, double> accSummary;

    double maxExpense = 0.0;
    double totalExpenses = 0.0;
    int expenseCount = 0;

    for (const auto* txn : txns) {
        ++r.totalTransactions;

        if (txn->transactionTypeName() == "Income") {
            r.totalIncome += txn->amount();
            catIncome[txn->categoryId()] += txn->amount();
        } else if (txn->transactionTypeName() == "Expense") {
            r.totalExpense += txn->amount();
            catExpense[txn->categoryId()] += txn->amount();
            if (txn->amount() > maxExpense) maxExpense = txn->amount();
            totalExpenses += txn->amount();
            ++expenseCount;
        }

        accSummary[txn->accountId()] += txn->amount();
    }

    r.netSavings = r.totalIncome - r.totalExpense;
    r.highestExpense = maxExpense;
    r.averageSpending = expenseCount > 0 ? totalExpenses / expenseCount : 0.0;
    r.accountSummary = accSummary;

    // Top expense categories (sorted by amount descending).
    std::vector<std::pair<std::string, double>> sortedCats(
        catExpense.begin(), catExpense.end());
    std::sort(sortedCats.begin(), sortedCats.end(),
              [](const auto& a, const auto& b) { return a.second > b.second; });

    // Take top 5.
    size_t limit = std::min(sortedCats.size(), size_t{5});
    r.topCategories.assign(sortedCats.begin(), sortedCats.begin() + limit);

    // Full category summary (combined income + expense).
    r.categorySummary = catExpense;
    for (const auto& [cat, amt] : catIncome) {
        r.categorySummary[cat] += amt;
    }

    return r;
}

// ── Period generators ───────────────────────────────────────────────

ReportGenerator::Report ReportGenerator::generateDaily(
    const std::string& userId)
{
    std::string today = utils::DateUtils::today();
    auto allTxns = storage_.loadTransactions();

    std::vector<models::Transaction*> filtered;
    for (auto& t : allTxns) {
        if (t->userId() == userId && t->date() == today) {
            filtered.push_back(t.get());
        }
    }

    return buildReport(filtered, "Daily Report - " + today);
}

ReportGenerator::Report ReportGenerator::generateWeekly(
    const std::string& userId)
{
    // Compute the start of the current week (Monday).
    auto today = utils::DateUtils::parseDate(utils::DateUtils::today());
    auto weekday = std::chrono::duration_cast<std::chrono::hours>(
        today - std::chrono::system_clock::time_point{}).count() / 24;
    // Approximate: subtract days since Monday.
    // This is simplified — a proper implementation would use
    // std::chrono::weekday from C++20 or manual calculation.
    int daysFromMonday = 3; // Wednesday approximation for simplicity.
    // Actually, use parseDate to get the date and compute.
    auto [y, m, d] = utils::DateUtils::extractYMD(utils::DateUtils::today());
    // Rough Monday: subtract day-of-week. Using tm_wday = 0 (Sunday)..6 (Saturday).
    std::tm tm = {};
    tm.tm_year = y - 1900;
    tm.tm_mon  = m - 1;
    tm.tm_mday = d;
    std::mktime(&tm);  // Normalize and get tm_wday.
    int dow = tm.tm_wday;  // 0=Sun, 1=Mon, ..., 6=Sat
    int daysSinceMonday = (dow == 0) ? 6 : (dow - 1);

    // Monday is `daysSinceMonday` days ago.
    auto monday = utils::DateUtils::parseDate(utils::DateUtils::today());
    monday -= std::chrono::hours(24 * daysSinceMonday);
    auto sunday = monday + std::chrono::hours(24 * 6);

    std::string monStr = utils::DateUtils::formatDate(monday);
    std::string sunStr = utils::DateUtils::formatDate(sunday);

    auto allTxns = storage_.loadTransactions();
    std::vector<models::Transaction*> filtered;
    for (auto& t : allTxns) {
        if (t->userId() == userId &&
            t->date() >= monStr && t->date() <= sunStr) {
            filtered.push_back(t.get());
        }
    }

    return buildReport(filtered, "Weekly Report - " + monStr + " to " + sunStr);
}

ReportGenerator::Report ReportGenerator::generateMonthly(
    const std::string& userId, int year, int month)
{
    std::string datePrefix = std::to_string(year) + "-" +
                             (month < 10 ? "0" : "") + std::to_string(month);

    auto allTxns = storage_.loadTransactions();
    std::vector<models::Transaction*> filtered;
    for (auto& t : allTxns) {
        if (t->userId() == userId &&
            t->date().substr(0, 7) == datePrefix) {
            filtered.push_back(t.get());
        }
    }

    std::string label = "Monthly Report - " +
                        utils::DateUtils::monthName(month) + " " +
                        std::to_string(year);
    return buildReport(filtered, label);
}

ReportGenerator::Report ReportGenerator::generateYearly(
    const std::string& userId, int year)
{
    std::string yearStr = std::to_string(year);

    auto allTxns = storage_.loadTransactions();
    std::vector<models::Transaction*> filtered;
    for (auto& t : allTxns) {
        if (t->userId() == userId &&
            t->date().substr(0, 4) == yearStr) {
            filtered.push_back(t.get());
        }
    }

    return buildReport(filtered,
                        "Yearly Report - " + yearStr);
}

ReportGenerator::Report ReportGenerator::generateForRange(
    const std::string& userId,
    const std::string& dateFrom, const std::string& dateTo)
{
    auto allTxns = storage_.loadTransactions();
    std::vector<models::Transaction*> filtered;
    for (auto& t : allTxns) {
        if (t->userId() == userId &&
            t->date() >= dateFrom && t->date() <= dateTo) {
            filtered.push_back(t.get());
        }
    }

    return buildReport(filtered,
                        "Custom Report - " + dateFrom + " to " + dateTo);
}

// ── Export (Strategy pattern) ───────────────────────────────────────

std::string ReportGenerator::exportToCsv(const Report& r) const
{
    std::ostringstream csv;
    csv << "Period," << r.periodLabel << "\n";
    csv << "Total Income," << r.totalIncome << "\n";
    csv << "Total Expense," << r.totalExpense << "\n";
    csv << "Net Savings," << r.netSavings << "\n";
    csv << "Highest Expense," << r.highestExpense << "\n";
    csv << "Average Spending," << r.averageSpending << "\n";
    csv << "Total Transactions," << r.totalTransactions << "\n\n";

    csv << "Category,Amount\n";
    for (const auto& [cat, amt] : r.categorySummary) {
        csv << cat << "," << amt << "\n";
    }
    csv << "\n";

    csv << "Account,Amount\n";
    for (const auto& [acc, amt] : r.accountSummary) {
        csv << acc << "," << amt << "\n";
    }

    return csv.str();
}

std::string ReportGenerator::exportToJson(const Report& r) const
{
    nlohmann::json j;
    j["period"]           = r.periodLabel;
    j["total_income"]     = r.totalIncome;
    j["total_expense"]    = r.totalExpense;
    j["net_savings"]      = r.netSavings;
    j["highest_expense"]  = r.highestExpense;
    j["average_spending"] = r.averageSpending;
    j["total_transactions"] = r.totalTransactions;

    nlohmann::json cats = nlohmann::json::object();
    for (const auto& [cat, amt] : r.categorySummary) {
        cats[cat] = amt;
    }
    j["category_summary"] = cats;

    nlohmann::json accs = nlohmann::json::object();
    for (const auto& [acc, amt] : r.accountSummary) {
        accs[acc] = amt;
    }
    j["account_summary"] = accs;

    return j.dump(2);
}

std::string ReportGenerator::exportToTxt(const Report& r) const
{
    std::ostringstream txt;
    txt << "╔══════════════════════════════════════════╗\n";
    txt << "║  " << std::left << std::setw(37) << r.periodLabel << "║\n";
    txt << "╚══════════════════════════════════════════╝\n\n";
    txt << "  Total Income:       " << std::fixed << std::setprecision(2)
        << r.totalIncome << "\n";
    txt << "  Total Expense:      " << r.totalExpense << "\n";
    txt << "  Net Savings:        " << r.netSavings << "\n";
    txt << "  Highest Expense:    " << r.highestExpense << "\n";
    txt << "  Average Spending:   " << r.averageSpending << "\n";
    txt << "  Total Transactions: " << r.totalTransactions << "\n\n";

    txt << "  ── Top Categories ──\n";
    for (const auto& [cat, amt] : r.topCategories) {
        txt << "    " << cat << ": " << amt << "\n";
    }

    return txt.str();
}

void ReportGenerator::saveToFile(const std::string& content,
                                  const std::string& filePath) const
{
    // Ensure parent directory exists.
    std::filesystem::path dir = std::filesystem::path(filePath).parent_path();
    if (!dir.empty() && !std::filesystem::exists(dir)) {
        std::filesystem::create_directories(dir);
    }

    std::ofstream ofs(filePath);
    if (ofs.is_open()) {
        ofs << content;
    }
}

}  // namespace finance::services
