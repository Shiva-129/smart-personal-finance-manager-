#ifndef FINANCE_SERVICES_REPORTGENERATOR_H
#define FINANCE_SERVICES_REPORTGENERATOR_H

#include "finance/storage/StorageManager.h"

#include <map>
#include <string>
#include <vector>

namespace finance::services {

/**
 * @brief Generates financial reports and exports them in multiple formats.
 *
 * Uses the Strategy pattern for export: each format (CSV, JSON, TXT)
 * is a separate method that formats the same Report data structure.
 */
class ReportGenerator {
public:
    /// Structured report data.
    struct Report {
        std::string                                  periodLabel;
        double                                       totalIncome  = 0.0;
        double                                       totalExpense = 0.0;
        double                                       netSavings   = 0.0;
        std::vector<std::pair<std::string, double>>  topCategories;
        double                                       highestExpense   = 0.0;
        double                                       averageSpending  = 0.0;
        std::map<std::string, double>                accountSummary;
        std::map<std::string, double>                categorySummary;
        int                                          totalTransactions = 0;
    };

    explicit ReportGenerator(storage::StorageManager& storage);

    // ── Report generation ───────────────────────────────────────────

    Report generateDaily(const std::string& userId);
    Report generateWeekly(const std::string& userId);
    Report generateMonthly(const std::string& userId, int year, int month);
    Report generateYearly(const std::string& userId, int year);

    /// Generate a report for an arbitrary date range.
    Report generateForRange(const std::string& userId,
                             const std::string& dateFrom,
                             const std::string& dateTo);

    // ── Export (Strategy pattern) ───────────────────────────────────

    std::string exportToCsv(const Report& report) const;
    std::string exportToJson(const Report& report) const;
    std::string exportToTxt(const Report& report) const;

    /// Save an export string to a file.
    void saveToFile(const std::string& content,
                    const std::string& filePath) const;

private:
    /// Build a report from a filtered list of transactions.
    Report buildReport(const std::vector<models::Transaction*>& txns,
                        const std::string& label) const;

    storage::StorageManager& storage_;
};

}  // namespace finance::services

#endif  // FINANCE_SERVICES_REPORTGENERATOR_H
