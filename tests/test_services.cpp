// ══════════════════════════════════════════════════════════════════════
// test_services.cpp — Consolidated tests for SearchEngine + ReportGenerator
// ══════════════════════════════════════════════════════════════════════

#include <gtest/gtest.h>
#include <filesystem>

#include "finance/Services.h"
#include "finance/Storage.h"
#include "finance/Utils.h"

using namespace finance::services;
using namespace finance::storage;
using namespace finance::models;
using namespace finance::utils;

// ══════════════════════════════════════════════════════════════════════
// SearchEngine Tests
// ══════════════════════════════════════════════════════════════════════

class SearchEngineTest : public ::testing::Test {
protected:
    void SetUp() override
    {
        dataDir_ = std::filesystem::temp_directory_path() / ("fin_test_" + UUID::generate());
        storage_ = std::make_unique<StorageManager>(dataDir_.string());
        storage_->initialize();

        auto store = [&](const std::string& type, double amount,
                         const std::string& desc, const std::string& cat,
                         const std::string& acc, const std::string& date,
                         const std::vector<std::string>& tags = {}) {
            auto txn = [&]() -> std::unique_ptr<Transaction> {
                if (type == "Income")
                    return std::make_unique<Income>(UUID::generate(), amount, desc, cat, acc, "user1", date);
                if (type == "Expense")
                    return std::make_unique<Expense>(UUID::generate(), amount, desc, cat, acc, "user1", date);
                return std::make_unique<Expense>(UUID::generate(), amount, desc, cat, acc, "user1", date);
            }();
            txn->setTags(tags);
            txns_.push_back(std::move(txn));
        };

        store("Income",  50000.0, "Salary",         "cat_salary", "acc_bank",  "2026-07-01");
        store("Expense", 1200.0,  "Lunch",           "cat_food",   "acc_wallet","2026-07-02", {"food", "daily"});
        store("Expense", 500.0,   "Bus fare",        "cat_travel", "acc_wallet","2026-07-03");
        store("Expense", 15000.0, "Rent",            "cat_rent",   "acc_bank",  "2026-07-01");
        store("Income",  5000.0,  "Freelance work",  "cat_freelance","acc_bank","2026-07-05");

        storage_->saveTransactions(txns_);

        engine_ = std::make_unique<SearchEngine>(*storage_);
    }

    void TearDown() override
    {
        engine_.reset();
        txns_.clear();
        storage_.reset();
        std::filesystem::remove_all(dataDir_);
    }

    std::filesystem::path                   dataDir_;
    std::unique_ptr<StorageManager>         storage_;
    std::unique_ptr<SearchEngine>           engine_;
    std::vector<std::unique_ptr<Transaction>> txns_;
};

TEST_F(SearchEngineTest, FindAllReturnsAll)
{
    SearchEngine::Criteria c;
    auto results = engine_->search("user1", c);
    EXPECT_EQ(results.size(), 5UL);
}

TEST_F(SearchEngineTest, FilterByDateRange)
{
    SearchEngine::Criteria c;
    c.dateFrom = "2026-07-03";
    c.dateTo   = "2026-07-05";
    auto results = engine_->search("user1", c);
    EXPECT_EQ(results.size(), 2UL);
}

TEST_F(SearchEngineTest, FilterByAmountRange)
{
    SearchEngine::Criteria c;
    c.amountMin = 10000.0;
    auto results = engine_->search("user1", c);
    EXPECT_EQ(results.size(), 2UL);
}

TEST_F(SearchEngineTest, FilterByDescription)
{
    SearchEngine::Criteria c;
    c.description = "lunch";
    auto results = engine_->search("user1", c);
    EXPECT_EQ(results.size(), 1UL);
    EXPECT_EQ(results[0]->description(), "Lunch");
}

TEST_F(SearchEngineTest, FilterByTransactionType)
{
    SearchEngine::Criteria c;
    c.transactionType = "Income";
    auto results = engine_->search("user1", c);
    EXPECT_EQ(results.size(), 2UL);
}

TEST_F(SearchEngineTest, FilterByTag)
{
    SearchEngine::Criteria c;
    c.tag = "food";
    auto results = engine_->search("user1", c);
    EXPECT_EQ(results.size(), 1UL);
}

TEST_F(SearchEngineTest, CombinedFilters)
{
    SearchEngine::Criteria c;
    c.dateFrom     = "2026-06-01";
    c.dateTo       = "2026-12-31";
    c.transactionType = "Expense";
    c.amountMin    = 1000.0;
    auto results = engine_->search("user1", c);
    EXPECT_EQ(results.size(), 2UL);
}

TEST_F(SearchEngineTest, NoMatchReturnsEmpty)
{
    SearchEngine::Criteria c;
    c.description = "nonexistent";
    auto results = engine_->search("user1", c);
    EXPECT_TRUE(results.empty());
}

// ══════════════════════════════════════════════════════════════════════
// ReportGenerator Tests
// ══════════════════════════════════════════════════════════════════════

class ReportTest : public ::testing::Test {
protected:
    void SetUp() override
    {
        dataDir_ = std::filesystem::temp_directory_path() / ("fin_rpt_" + UUID::generate());
        storage_ = std::make_unique<StorageManager>(dataDir_.string());
        storage_->initialize();

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

        add("Income",  60000.0, "Salary",    "cat_salary", "acc_bank",  "2026-07-01");
        add("Expense", 15000.0, "Rent",      "cat_rent",   "acc_bank",  "2026-07-01");
        add("Expense",  2000.0, "Groceries", "cat_food",   "acc_wallet","2026-07-05");
        add("Expense",   500.0, "Dinner",    "cat_food",   "acc_wallet","2026-07-10");
        add("Income",   3000.0, "Freelance", "cat_freelance","acc_bank","2026-07-15");
        add("Expense",  1000.0, "Old expense","cat_other", "acc_wallet", "2026-06-20");

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

    EXPECT_DOUBLE_EQ(r.totalIncome,  63000.0);
    EXPECT_DOUBLE_EQ(r.totalExpense, 17500.0);
    EXPECT_DOUBLE_EQ(r.netSavings,   45500.0);
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
    EXPECT_DOUBLE_EQ(r.averageSpending, 17500.0 / 3.0);
}

TEST_F(ReportTest, MonthlyReportTopCategories)
{
    auto r = report_->generateMonthly("user1", 2026, 7);
    ASSERT_FALSE(r.topCategories.empty());
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
    EXPECT_EQ(r.totalTransactions, 6);
}
