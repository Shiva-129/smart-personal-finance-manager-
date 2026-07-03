/**
 * @file    test_storage.cpp
 * @brief   Unit tests for JsonRepository using a temporary file.
 */

#include <gtest/gtest.h>
#include <filesystem>
#include <fstream>

#include "finance/models/User.h"
#include "finance/storage/JsonRepository.h"

using namespace finance::storage;
using namespace finance::models;

/// Helper: path to a temporary test file.
static std::string tempPath(const char* name)
{
    return std::string(std::filesystem::temp_directory_path()) + "/" + name;
}

// ── JsonRepository<User> Tests ──────────────────────────────────────

class JsonRepositoryTest : public ::testing::Test {
protected:
    void SetUp() override
    {
        path_ = tempPath("test_users.json");
        // Clean up any leftover from a previous failed test.
        std::filesystem::remove(path_);
        repo_ = std::make_unique<JsonRepository<User>>(path_);
    }

    void TearDown() override
    {
        repo_.reset();
        std::filesystem::remove(path_);
    }

    std::string                       path_;
    std::unique_ptr<JsonRepository<User>> repo_;
};

TEST_F(JsonRepositoryTest, EmptyWhenNoFile)
{
    EXPECT_EQ(repo_->count(), 0UL);
    EXPECT_TRUE(repo_->loadAll().empty());
}

TEST_F(JsonRepositoryTest, SaveAndLoadSingleItem)
{
    User u("id1", "alice", "hash1", "Alice");
    repo_->save(u);

    auto loaded = repo_->loadAll();
    ASSERT_EQ(loaded.size(), 1UL);
    EXPECT_EQ(loaded[0].username(), "alice");
}

TEST_F(JsonRepositoryTest, SaveAndFindById)
{
    User u("id2", "bob", "hash2", "Bob");
    repo_->save(u);

    auto opt = repo_->findById("id2");
    ASSERT_TRUE(opt.has_value());
    EXPECT_EQ(opt->username(), "bob");
}

TEST_F(JsonRepositoryTest, FindByIdReturnsNulloptForMissing)
{
    auto opt = repo_->findById("nonexistent");
    EXPECT_FALSE(opt.has_value());
}

TEST_F(JsonRepositoryTest, RemoveExistingItem)
{
    User u("id3", "carol", "hash3", "Carol");
    repo_->save(u);

    EXPECT_TRUE(repo_->remove("id3"));
    EXPECT_EQ(repo_->count(), 0UL);
}

TEST_F(JsonRepositoryTest, RemoveNonexistentItemReturnsFalse)
{
    EXPECT_FALSE(repo_->remove("no_such_id"));
}

TEST_F(JsonRepositoryTest, SaveAllReplacesContents)
{
    repo_->save(User("a", "u1", "h1", "U1"));
    repo_->save(User("b", "u2", "h2", "U2"));

    std::vector<User> fresh = {
        User("c", "u3", "h3", "U3")
    };
    repo_->saveAll(fresh);
    EXPECT_EQ(repo_->count(), 1UL);
    EXPECT_TRUE(repo_->findById("a") == std::nullopt);
    EXPECT_TRUE(repo_->findById("c") != std::nullopt);
}

TEST_F(JsonRepositoryTest, PersistsToDisk)
{
    User u("id_persist", "dave", "hash4", "Dave");
    repo_->save(u);

    // Create a new repository reading the same file.
    JsonRepository<User> repo2(path_);
    EXPECT_EQ(repo2.count(), 1UL);
    auto opt = repo2.findById("id_persist");
    ASSERT_TRUE(opt.has_value());
    EXPECT_EQ(opt->username(), "dave");
}

TEST_F(JsonRepositoryTest, FindWithPredicate)
{
    repo_->save(User("a", "alice", "h1", "A"));
    repo_->save(User("b", "bob",   "h2", "B"));
    repo_->save(User("c", "alice2", "h3", "C"));

    auto results = repo_->find([](const User& u) {
        return u.username().find("alice") != std::string::npos;
    });
    EXPECT_EQ(results.size(), 2UL);
}

TEST_F(JsonRepositoryTest, UpdateExistingItem)
{
    User u("id_upd", "eve", "old_hash", "Eve");
    repo_->save(u);

    User updated("id_upd", "eve", "new_hash", "Eve Updated");
    repo_->save(updated);

    auto opt = repo_->findById("id_upd");
    ASSERT_TRUE(opt.has_value());
    EXPECT_EQ(opt->displayName(), "Eve Updated");
    EXPECT_EQ(opt->passwordHash(), "new_hash");
}

TEST_F(JsonRepositoryTest, HandlesCorruptedFile)
{
    // Write garbage to the file.
    std::ofstream ofs(path_);
    ofs << "this is not json";
    ofs.close();

    // Should not throw — returns empty.
    EXPECT_NO_THROW({
        EXPECT_EQ(repo_->count(), 0UL);
    });
}
