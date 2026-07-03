/**
 * @file    test_search.cpp
 * @brief   Unit tests for SearchEngine criteria matching.
 *
 * These tests create a StorageManager with a temp data directory,
 * seed a few transactions, and verify that SearchEngine correctly
 * filters them.
 */

#include <gtest/gtest.h>
#include <filesystem>

#include "finance/services/SearchEngine.h"
#include "finance/storage/StorageManager.h"
#include "finance/utils/UUID.h"

using namespace finance::services;
using namespace finance::storage;
using namespace finance::models;
using namespace finance::utils;

class SearchEngineTest : public ::testing::Test {
protected:
    void SetUp() override
    {
        // Use a unique temp directory for each test.
        dataDir_ = std::filesystem::temp_directory_path() / ("fin_test_" + UUID::generate());
        storage_ = std::make_unique<StorageManager>(dataDir_.string());
        storage_->initialize();

        // Seed some transactions.
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
        // Manually clear pointers before storage resets.
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
    EXPECT_EQ(results.size(), 2UL); // 50000 + 15000
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
    EXPECT_EQ(results.size(), 2UL); // Lunch (1200) + Rent (15000)
}

TEST_F(SearchEngineTest, NoMatchReturnsEmpty)
{
    SearchEngine::Criteria c;
    c.description = "nonexistent";
    auto results = engine_->search("user1", c);
    EXPECT_TRUE(results.empty());
}
