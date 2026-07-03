/**
 * @file    test_report.cpp
 * @brief   Unit tests for ReportGenerator.
 */

#include <gtest/gtest.h>
#include <filesystem>

#include "finance/services/ReportGenerator.h"
#include "finance/storage/StorageManager.h"
#include "finance/utils/UUID.h"

using namespace finance::services;
using namespace finance::storage;
using namespace finance::models;
using namespace finance::utils;

class ReportTest : public ::testing::Test {
protected:
    void SetUp() override
    {
        dataDir_ = std::filesystem::temp_directory_path() / ("fin_rpt_" + UUID::generate());
        storage_ = std::make_unique<StorageManager>(dataDir_.string());
        storage_->initialize();

        // Seed different transactions for different months.
        auto add = [&](const std::string& type, double amt,
                       const std::string& desc, const std::string& cat,
                       const std::string& acc, const std::string& date) {
            std::unique_ptr<Transaction> t;
            if (type == "Income")
                t = std::make_unique<Income>(UUID::generate(), amt, desc, cat, acc, "user1", date);
            else
                t = std::make_unique<Expense>(UUID::generate(), amt, desc, cat, acc, "user1", date);
            txns_.push_back(std::move(t));
        };

        // July 2026 transactions
        add("Income",  60000.0, "Salary",    "cat_salary", "acc_bank",  "2026-07-01");
        add("Expense", 15000.0, "Rent",      "cat_rent",   "acc_bank",  "2026-07-01");
        add("Expense",  2000.0, "Groceries", "cat_food",   "acc_wallet","2026-07-05");
        add("Expense",   500.0, "Dinner",    "cat_food",   "acc_wallet","2026-07-10");
        add("Income",   3000.0, "Freelance", "cat_freelance","acc_bank","2026-07-15");

        // June 2026 transaction (shouldn't appear in July report)
        add("Expense", 1000.0, "Old expense", "cat_other", "acc_wallet", "2026-06-20");

        storage_->saveTransactions(txns_);

        report_ = std::make_unique<ReportGenerator>(*storage_);
    }

    void TearDown() override
    {
        report_.reset();
        txns_.clear();
        storage_.reset();
        std::filesystem::remove_all(dataDir_);
    }

    std::filesystem::path                     dataDir_;
    std::unique_ptr<StorageManager>           storage_;
    std::unique_ptr<ReportGenerator>          report_;
    std::vector<std::unique_ptr<Transaction>> txns_;
};

TEST_F(ReportTest, MonthlyReportTotals)
{
    auto r = report_->generateMonthly("user1", 2026, 7);

    EXPECT_DOUBLE_EQ(r.totalIncome,  63000.0);   // 60000 + 3000
    EXPECT_DOUBLE_EQ(r.totalExpense, 17500.0);   // 15000 + 2000 + 500
    EXPECT_DOUBLE_EQ(r.netSavings,   45500.0);   // 63000 - 17500
    EXPECT_EQ(r.totalTransactions, 5);
}

TEST_F(ReportTest, MonthlyReportHighestExpense)
{
    auto r = report_->generateMonthly("user1", 2026, 7);
    EXPECT_DOUBLE_EQ(r.highestExpense, 15000.0);
}

TEST_F(ReportTest, MonthlyReportAverageSpending)
{
    auto r = report_->generateMonthly("user1", 2026, 7);
    // Total expense 17500 / 3 expense transactions
    EXPECT_DOUBLE_EQ(r.averageSpending, 17500.0 / 3.0);
}

TEST_F(ReportTest, MonthlyReportTopCategories)
{
    auto r = report_->generateMonthly("user1", 2026, 7);
    ASSERT_FALSE(r.topCategories.empty());
    // Top category should be Rent (15000) > Food (2500)
    EXPECT_EQ(r.topCategories[0].first, "cat_rent");
    EXPECT_DOUBLE_EQ(r.topCategories[0].second, 15000.0);
}

TEST_F(ReportTest, MonthlyReportIgnoresOtherMonths)
{
    auto r = report_->generateMonthly("user1", 2026, 6);
    EXPECT_EQ(r.totalTransactions, 1);
    EXPECT_DOUBLE_EQ(r.totalExpense, 1000.0);
    EXPECT_DOUBLE_EQ(r.totalIncome, 0.0);
}

TEST_F(ReportTest, ExportToCsv)
{
    auto r = report_->generateMonthly("user1", 2026, 7);
    auto csv = report_->exportToCsv(r);
    EXPECT_FALSE(csv.empty());
    EXPECT_NE(csv.find("Total Income"), std::string::npos);
    EXPECT_NE(csv.find("63000"), std::string::npos);
}

TEST_F(ReportTest, ExportToJson)
{
    auto r = report_->generateMonthly("user1", 2026, 7);
    auto json = report_->exportToJson(r);
    EXPECT_FALSE(json.empty());
    EXPECT_NE(json.find("total_income"), std::string::npos);
    EXPECT_NE(json.find("63000"), std::string::npos);
}

TEST_F(ReportTest, ExportToTxt)
{
    auto r = report_->generateMonthly("user1", 2026, 7);
    auto txt = report_->exportToTxt(r);
    EXPECT_FALSE(txt.empty());
    EXPECT_NE(txt.find("Total Income"), std::string::npos);
}

TEST_F(ReportTest, SaveToFileCreatesFile)
{
    auto r = report_->generateMonthly("user1", 2026, 7);
    auto outPath = (dataDir_ / "test_report.txt").string();
    report_->saveToFile("hello", outPath);

    EXPECT_TRUE(std::filesystem::exists(outPath));
    std::filesystem::remove(outPath);
}

TEST_F(ReportTest, YearlyReport)
{
    auto r = report_->generateYearly("user1", 2026);
    EXPECT_EQ(r.totalTransactions, 6); // all transactions in 2026
}
