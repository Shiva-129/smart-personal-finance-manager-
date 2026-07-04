// ══════════════════════════════════════════════════════════════════════
// test_models.cpp — Consolidated unit tests for all model classes
//   Accounts, Transactions, Budget, Goals, Categories, User, Notification
// ══════════════════════════════════════════════════════════════════════

#include <gtest/gtest.h>
#include "finance/Models.h"
#include "finance/Utils.h"

using namespace finance::models;
using namespace finance::utils;

// ── Helpers ─────────────────────────────────────────────────────────

static std::string makeId() { return UUID::generate(); }

// ══════════════════════════════════════════════════════════════════════
// Account Tests
// ══════════════════════════════════════════════════════════════════════

TEST(AccountTest, CreateAndAccess)
{
    CashAccount acc(makeId(), "Wallet", "INR", "user1");
    EXPECT_EQ(acc.name(), "Wallet");
    EXPECT_EQ(acc.balance(), 0.0);
    EXPECT_EQ(acc.currency(), "INR");
    EXPECT_EQ(acc.accountTypeName(), "Cash");
    EXPECT_EQ(acc.status(), AccountStatus::ACTIVE);
}

TEST(AccountTest, DepositIncreasesBalance)
{
    CashAccount acc(makeId(), "Test", "INR", "u1");
    acc.deposit(1000.0);
    EXPECT_DOUBLE_EQ(acc.balance(), 1000.0);

    acc.deposit(500.0);
    EXPECT_DOUBLE_EQ(acc.balance(), 1500.0);
}

TEST(AccountTest, DepositThrowsOnNonPositive)
{
    CashAccount acc(makeId(), "Test", "INR", "u1");
    EXPECT_THROW(acc.deposit(0),    InvalidInputException);
    EXPECT_THROW(acc.deposit(-100), InvalidInputException);
}

TEST(AccountTest, WithdrawDecreasesBalance)
{
    CashAccount acc(makeId(), "Test", "INR", "u1");
    acc.deposit(1000.0);
    acc.withdraw(300.0);
    EXPECT_DOUBLE_EQ(acc.balance(), 700.0);
}

TEST(AccountTest, WithdrawThrowsOnInsufficientFunds)
{
    CashAccount acc(makeId(), "Test", "INR", "u1");
    acc.deposit(100.0);
    EXPECT_THROW(acc.withdraw(200.0), NegativeBalanceException);
}

TEST(AccountTest, WithdrawThrowsOnNonPositive)
{
    CashAccount acc(makeId(), "Test", "INR", "u1");
    EXPECT_THROW(acc.withdraw(0),    InvalidInputException);
    EXPECT_THROW(acc.withdraw(-50),  InvalidInputException);
}

TEST(AccountTest, FreezeAndClose)
{
    CashAccount acc(makeId(), "Test", "INR", "u1");
    acc.deposit(100.0);

    acc.freeze();
    EXPECT_EQ(acc.status(), AccountStatus::FROZEN);
    EXPECT_THROW(acc.deposit(50.0),  FinanceException);
    EXPECT_THROW(acc.withdraw(10.0), FinanceException);

    acc.close();
    EXPECT_EQ(acc.status(), AccountStatus::CLOSED);
    EXPECT_THROW(acc.deposit(50.0),  FinanceException);
    EXPECT_THROW(acc.withdraw(10.0), FinanceException);
}

TEST(CreditCardTest, WithdrawGoesNegative)
{
    CreditCardAccount cc(makeId(), "Card", "INR", "u1");
    cc.setCreditLimit(5000.0);

    cc.withdraw(2000.0);
    EXPECT_DOUBLE_EQ(cc.balance(), -2000.0);
}

TEST(CreditCardTest, WithdrawRespectsCreditLimit)
{
    CreditCardAccount cc(makeId(), "Card", "INR", "u1");
    cc.setCreditLimit(5000.0);

    cc.withdraw(3000.0);
    EXPECT_THROW(cc.withdraw(3000.0), CreditLimitException);
}

TEST(CreditCardTest, DepositAfterWithdraw)
{
    CreditCardAccount cc(makeId(), "Card", "INR", "u1");
    cc.setCreditLimit(5000.0);
    cc.withdraw(2000.0);

    cc.deposit(1000.0);
    EXPECT_DOUBLE_EQ(cc.balance(), -1000.0);
}

TEST(SavingsAccountTest, InterestRate)
{
    SavingsAccount sa(makeId(), "Savings", "INR", "u1");
    sa.setInterestRate(4.5);
    EXPECT_DOUBLE_EQ(sa.interestRate(), 4.5);
}

TEST(AccountSerializationTest, CashRoundTrip)
{
    CashAccount original(makeId(), "My Cash", "INR", "u1");
    original.deposit(500.0);

    auto json = original.toJson();
    auto restored = Account::fromJson(json);

    EXPECT_EQ(original.id(),       restored->id());
    EXPECT_EQ(original.name(),     restored->name());
    EXPECT_EQ(original.balance(),  restored->balance());
    EXPECT_EQ(original.currency(), restored->currency());
    EXPECT_EQ(original.accountTypeName(), restored->accountTypeName());
}

TEST(AccountSerializationTest, CreditCardRoundTrip)
{
    CreditCardAccount original(makeId(), "Platinum", "INR", "u1");
    original.setCreditLimit(50000.0);
    original.setDueAmount(12000.0);
    original.setDueDate("2026-08-15");
    original.withdraw(3000.0);

    auto json = original.toJson();
    auto restored = Account::fromJson(json);

    EXPECT_EQ(original.id(),       restored->id());
    EXPECT_EQ(original.balance(),  restored->balance());
    EXPECT_EQ(original.accountTypeName(), restored->accountTypeName());
}

TEST(AccountSerializationTest, FactoryCreatesCorrectTypes)
{
    auto cashJson = CashAccount(makeId(), "C", "INR", "u1").toJson();
    auto savJson  = SavingsAccount(makeId(), "S", "INR", "u1").toJson();
    auto curJson  = CurrentAccount(makeId(), "Cu", "INR", "u1").toJson();
    auto ccJson   = CreditCardAccount(makeId(), "CC", "INR", "u1").toJson();
    auto walJson  = WalletAccount(makeId(), "W", "INR", "u1").toJson();

    EXPECT_EQ(Account::fromJson(cashJson)->accountTypeName(), "Cash");
    EXPECT_EQ(Account::fromJson(savJson)->accountTypeName(),  "Savings");
    EXPECT_EQ(Account::fromJson(curJson)->accountTypeName(),  "Current");
    EXPECT_EQ(Account::fromJson(ccJson)->accountTypeName(),   "Credit Card");
    EXPECT_EQ(Account::fromJson(walJson)->accountTypeName(),  "Wallet");
}

TEST(AccountTest, StatusStringConversion)
{
    EXPECT_EQ(accountStatusToString(AccountStatus::ACTIVE), "active");
    EXPECT_EQ(accountStatusToString(AccountStatus::FROZEN), "frozen");
    EXPECT_EQ(accountStatusToString(AccountStatus::CLOSED), "closed");

    EXPECT_EQ(accountStatusFromString("active"), AccountStatus::ACTIVE);
    EXPECT_EQ(accountStatusFromString("frozen"), AccountStatus::FROZEN);
    EXPECT_EQ(accountStatusFromString("closed"), AccountStatus::CLOSED);
    EXPECT_EQ(accountStatusFromString("unknown"), AccountStatus::ACTIVE);
}

// ══════════════════════════════════════════════════════════════════════
// Transaction Tests
// ══════════════════════════════════════════════════════════════════════

TEST(IncomeTest, CreateAndAccess)
{
    Income inc(makeId(), 50000.0, "Monthly salary",
               "cat_salary", "acc_bank", "user1");
    EXPECT_EQ(inc.amount(), 50000.0);
    EXPECT_EQ(inc.description(), "Monthly salary");
    EXPECT_EQ(inc.transactionTypeName(), "Income");
    EXPECT_FALSE(inc.date().empty());
    EXPECT_FALSE(inc.time().empty());
}

TEST(IncomeTest, SerializationRoundTrip)
{
    Income original(makeId(), 7500.0, "Freelance project",
                    "cat_freelance", "acc_main", "user2",
                    "2026-07-04", "14:30:00");
    original.setTags({"work", "urgent"});
    original.setNotes("Invoice #42");

    auto json = original.toJson();
    std::unique_ptr<Transaction> restored = Transaction::fromJson(json);

    EXPECT_EQ(original.id(),          restored->id());
    EXPECT_EQ(original.amount(),      restored->amount());
    EXPECT_EQ(original.description(), restored->description());
    EXPECT_EQ(original.transactionTypeName(), restored->transactionTypeName());
    ASSERT_EQ(original.tags().size(), restored->tags().size());
    EXPECT_EQ(original.tags()[0],     restored->tags()[0]);
}

TEST(IncomeTest, ClonePreservesData)
{
    Income original(makeId(), 3000.0, "Gift", "cat_gift", "acc1", "u1");
    auto clone = original.clone();

    EXPECT_EQ(original.id(),       clone->id());
    EXPECT_EQ(original.amount(),   clone->amount());
    EXPECT_EQ(original.description(), clone->description());
}

TEST(ExpenseTest, CreateAndAccess)
{
    Expense exp(makeId(), 1200.0, "Lunch",
                "cat_food", "acc_wallet", "user1");
    EXPECT_EQ(exp.transactionTypeName(), "Expense");
    EXPECT_FALSE(exp.isRecurring());
}

TEST(ExpenseTest, RecurringFlag)
{
    Expense exp(makeId(), 15000.0, "Rent",
                "cat_rent", "acc_bank", "user1");
    exp.setRecurring(true);
    exp.setRecurrenceRule("monthly");
    EXPECT_TRUE(exp.isRecurring());
    EXPECT_EQ(exp.recurrenceRule(), "monthly");
}

TEST(ExpenseTest, SerializationRecurring)
{
    Expense original(makeId(), 999.0, "Netflix",
                     "cat_entertainment", "acc_cc", "u1",
                     "2026-07-01", "10:00:00");
    original.setRecurring(true);
    original.setRecurrenceRule("monthly");

    auto json = original.toJson();
    auto restored = Transaction::fromJson(json);
    ASSERT_NE(restored, nullptr);
    EXPECT_EQ(restored->transactionTypeName(), "Expense");

    EXPECT_TRUE(restored->transactionTypeName() == "Expense");
}

TEST(ExpenseTest, Mutators)
{
    Expense exp(makeId(), 100.0, "Old desc", "cat1", "acc1", "u1");
    exp.setAmount(200.0);
    exp.setDescription("New desc");
    exp.setCategoryId("cat2");

    EXPECT_DOUBLE_EQ(exp.amount(), 200.0);
    EXPECT_EQ(exp.description(), "New desc");
    EXPECT_EQ(exp.categoryId(), "cat2");
}

TEST(TransferTest, CreateAndAccess)
{
    Transfer tr(makeId(), 5000.0, "Savings transfer",
                "", "acc_savings", "user1");
    tr.setToAccountId("acc_checking");
    EXPECT_EQ(tr.transactionTypeName(), "Transfer");
    EXPECT_EQ(tr.toAccountId(), "acc_checking");
}

TEST(TransferTest, SerializationRoundTrip)
{
    Transfer original(makeId(), 10000.0, "To savings",
                      "", "acc_checking", "u1");
    original.setToAccountId("acc_savings");

    auto json = original.toJson();
    auto restored = Transaction::fromJson(json);

    EXPECT_EQ(original.transactionTypeName(), restored->transactionTypeName());
    EXPECT_EQ(original.amount(), restored->amount());
}

TEST(TransactionFactoryTest, CreatesCorrectTypes)
{
    auto incomeJson   = Income(makeId(), 100, "", "c", "a", "u").toJson();
    auto expenseJson  = Expense(makeId(), 100, "", "c", "a", "u").toJson();
    auto transferJson = Transfer(makeId(), 100, "", "c", "a", "u").toJson();

    EXPECT_EQ(Transaction::fromJson(incomeJson)->transactionTypeName(),   "Income");
    EXPECT_EQ(Transaction::fromJson(expenseJson)->transactionTypeName(),  "Expense");
    EXPECT_EQ(Transaction::fromJson(transferJson)->transactionTypeName(), "Transfer");
}

TEST(TransactionFactoryTest, UnknownTypeDefaultsToExpense)
{
    nlohmann::json j = {
        {"id", "x"}, {"amount", 50.0}, {"description", "fallback"},
        {"type", "UnknownType"}
    };
    auto txn = Transaction::fromJson(j);
    EXPECT_EQ(txn->transactionTypeName(), "Expense");
}

// ══════════════════════════════════════════════════════════════════════
// Budget Tests
// ══════════════════════════════════════════════════════════════════════

TEST(BudgetTest, CreateAndAccess)
{
    Budget b("budget1", "cat_food", "user1", 2026, 7, 6000.0);
    EXPECT_EQ(b.id(), "budget1");
    EXPECT_EQ(b.categoryId(), "cat_food");
    EXPECT_EQ(b.userId(), "user1");
    EXPECT_EQ(b.year(), 2026);
    EXPECT_EQ(b.month(), 7);
    EXPECT_DOUBLE_EQ(b.limitAmount(), 6000.0);
    EXPECT_DOUBLE_EQ(b.spentAmount(), 0.0);
}

TEST(BudgetTest, Remaining)
{
    Budget b("id", "cat", "u", 2026, 1, 5000.0);
    b.setSpentAmount(3200.0);
    EXPECT_DOUBLE_EQ(b.remaining(), 1800.0);
}

TEST(BudgetTest, PercentageUsed)
{
    Budget b("id", "cat", "u", 2026, 1, 1000.0);
    EXPECT_DOUBLE_EQ(b.percentageUsed(), 0.0);

    b.setSpentAmount(250.0);
    EXPECT_DOUBLE_EQ(b.percentageUsed(), 25.0);

    b.setSpentAmount(1000.0);
    EXPECT_DOUBLE_EQ(b.percentageUsed(), 100.0);
}

TEST(BudgetTest, IsExceeded)
{
    Budget b("id", "cat", "u", 2026, 1, 1000.0);
    EXPECT_FALSE(b.isExceeded());

    b.setSpentAmount(1000.0);
    EXPECT_FALSE(b.isExceeded());

    b.setSpentAmount(1000.01);
    EXPECT_TRUE(b.isExceeded());
}

TEST(BudgetTest, AddSpending)
{
    Budget b("id", "cat", "u", 2026, 1, 5000.0);
    b.addSpending(1000.0);
    EXPECT_DOUBLE_EQ(b.spentAmount(), 1000.0);
    b.addSpending(500.0);
    EXPECT_DOUBLE_EQ(b.spentAmount(), 1500.0);
}

TEST(BudgetTest, SerializationRoundTrip)
{
    Budget original("bid", "cat_food", "u1", 2026, 7, 6000.0);
    original.setSpentAmount(4200.0);

    auto json = original.toJson();
    auto restored = Budget::fromJson(json);

    EXPECT_EQ(original.id(),         restored.id());
    EXPECT_EQ(original.categoryId(), restored.categoryId());
    EXPECT_EQ(original.year(),       restored.year());
    EXPECT_EQ(original.month(),      restored.month());
    EXPECT_DOUBLE_EQ(original.limitAmount(), restored.limitAmount());
    EXPECT_DOUBLE_EQ(original.spentAmount(), restored.spentAmount());
}

TEST(BudgetTest, PercentageUsedWithZeroLimit)
{
    Budget b("id", "cat", "u", 2026, 1, 0.0);
    EXPECT_DOUBLE_EQ(b.percentageUsed(), 0.0);
}

// ══════════════════════════════════════════════════════════════════════
// Goal Tests
// ══════════════════════════════════════════════════════════════════════

TEST(GoalTest, CreateAndAccess)
{
    Goal g("goal1", "user1", "Buy Laptop", 100000.0);
    EXPECT_EQ(g.name(), "Buy Laptop");
    EXPECT_DOUBLE_EQ(g.targetAmount(), 100000.0);
    EXPECT_DOUBLE_EQ(g.currentAmount(), 0.0);
    EXPECT_EQ(g.status(), GoalStatus::IN_PROGRESS);
    EXPECT_FALSE(g.isAchieved());
}

TEST(GoalTest, ProgressPercentage)
{
    Goal g("id", "u", "Test", 1000.0);
    EXPECT_DOUBLE_EQ(g.progressPercentage(), 0.0);

    g.addAmount(250.0);
    EXPECT_DOUBLE_EQ(g.progressPercentage(), 25.0);

    g.addAmount(250.0);
    EXPECT_DOUBLE_EQ(g.progressPercentage(), 50.0);
}

TEST(GoalTest, ProgressCapsAt100)
{
    Goal g("id", "u", "Test", 1000.0);
    g.addAmount(1500.0);
    EXPECT_DOUBLE_EQ(g.progressPercentage(), 100.0);
}

TEST(GoalTest, MarkAchieved)
{
    Goal g("id", "u", "Test", 1000.0);
    g.addAmount(1000.0);
    g.markAchieved();
    EXPECT_TRUE(g.isAchieved());
    EXPECT_EQ(g.status(), GoalStatus::ACHIEVED);
}

TEST(GoalTest, AddAmount)
{
    Goal g("id", "u", "Test", 5000.0);
    g.addAmount(2000.0);
    EXPECT_DOUBLE_EQ(g.currentAmount(), 2000.0);
    g.addAmount(1500.0);
    EXPECT_DOUBLE_EQ(g.currentAmount(), 3500.0);
}

TEST(GoalTest, SerializationRoundTrip)
{
    Goal original("gid", "u1", "Emergency Fund", 50000.0, "2026-12-31");
    original.addAmount(12000.0);

    auto json = original.toJson();
    auto restored = Goal::fromJson(json);

    EXPECT_EQ(original.id(),             restored.id());
    EXPECT_EQ(original.name(),           restored.name());
    EXPECT_DOUBLE_EQ(original.targetAmount(), restored.targetAmount());
    EXPECT_DOUBLE_EQ(original.currentAmount(), restored.currentAmount());
    EXPECT_EQ(original.deadline(),       restored.deadline());
    EXPECT_EQ(original.status(),         restored.status());
}

TEST(GoalTest, StatusStringConversion)
{
    EXPECT_EQ(Goal::statusToString(GoalStatus::IN_PROGRESS), "in_progress");
    EXPECT_EQ(Goal::statusToString(GoalStatus::ACHIEVED),    "achieved");
    EXPECT_EQ(Goal::statusToString(GoalStatus::CANCELLED),   "cancelled");

    EXPECT_EQ(Goal::statusFromString("in_progress"), GoalStatus::IN_PROGRESS);
    EXPECT_EQ(Goal::statusFromString("achieved"),    GoalStatus::ACHIEVED);
    EXPECT_EQ(Goal::statusFromString("cancelled"),   GoalStatus::CANCELLED);
    EXPECT_EQ(Goal::statusFromString("unknown"),     GoalStatus::IN_PROGRESS);
}

TEST(GoalTest, ProgressWithZeroTarget)
{
    Goal g("id", "u", "Test", 0.0);
    EXPECT_DOUBLE_EQ(g.progressPercentage(), 0.0);
}

// ══════════════════════════════════════════════════════════════════════
// Category Tests
// ══════════════════════════════════════════════════════════════════════

TEST(CategoryTest, DefaultsCreatesExpectedCount)
{
    auto cats = Category::defaults();
    EXPECT_EQ(cats.size(), 16UL);
}

TEST(CategoryTest, DefaultsAreMarkedDefault)
{
    auto cats = Category::defaults();
    for (const auto& c : cats) {
        EXPECT_TRUE(c.isDefault());
    }
}

TEST(CategoryTest, DefaultsHaveDeterministicIds)
{
    auto first  = Category::defaults();
    auto second = Category::defaults();
    ASSERT_EQ(first.size(), second.size());
    for (size_t i = 0; i < first.size(); ++i) {
        EXPECT_EQ(first[i].id(), second[i].id());
        EXPECT_EQ(first[i].name(), second[i].name());
    }
}

TEST(CategoryTest, SerializationRoundTrip)
{
    Category original("test-id", "Groceries", CategoryType::EXPENSE, false);
    auto json = original.toJson();
    auto restored = Category::fromJson(json);

    EXPECT_EQ(original.id(),   restored.id());
    EXPECT_EQ(original.name(), restored.name());
    EXPECT_EQ(original.type(), restored.type());
    EXPECT_EQ(original.isDefault(), restored.isDefault());
}

TEST(CategoryTest, TypeToStringAndBack)
{
    EXPECT_EQ(Category::typeToString(CategoryType::INCOME),  "income");
    EXPECT_EQ(Category::typeToString(CategoryType::EXPENSE), "expense");
    EXPECT_EQ(Category::typeToString(CategoryType::BOTH),    "both");

    EXPECT_EQ(Category::typeFromString("income"),  CategoryType::INCOME);
    EXPECT_EQ(Category::typeFromString("expense"), CategoryType::EXPENSE);
    EXPECT_EQ(Category::typeFromString("both"),    CategoryType::BOTH);
    EXPECT_EQ(Category::typeFromString("unknown"), CategoryType::EXPENSE);
}

TEST(CategoryTest, SetNameAndType)
{
    Category cat("id", "Old", CategoryType::EXPENSE);
    cat.setName("New");
    cat.setType(CategoryType::INCOME);

    EXPECT_EQ(cat.name(), "New");
    EXPECT_EQ(cat.type(), CategoryType::INCOME);
}

// ══════════════════════════════════════════════════════════════════════
// User Tests
// ══════════════════════════════════════════════════════════════════════

TEST(UserTest, CreateAndAccess)
{
    User u("uid1", "johndoe", "hash123", "John Doe");
    EXPECT_EQ(u.id(), "uid1");
    EXPECT_EQ(u.username(), "johndoe");
    EXPECT_EQ(u.passwordHash(), "hash123");
    EXPECT_EQ(u.displayName(), "John Doe");
    EXPECT_TRUE(u.isActive());
    EXPECT_FALSE(u.createdAt().empty());
}

TEST(UserTest, DisplayNameDefaultsToUsername)
{
    User u("uid2", "janedoe", "hash456", "");
    EXPECT_EQ(u.displayName(), "janedoe");
}

TEST(UserTest, SerializationRoundTrip)
{
    User original("uid3", "bob", "salt:hash", "Bob Smith");
    original.setLastLoginAt("2026-07-04T10:30:00");

    auto json = original.toJson();
    auto restored = User::fromJson(json);

    EXPECT_EQ(original.id(),           restored.id());
    EXPECT_EQ(original.username(),     restored.username());
    EXPECT_EQ(original.passwordHash(), restored.passwordHash());
    EXPECT_EQ(original.displayName(),  restored.displayName());
    EXPECT_EQ(original.isActive(),     restored.isActive());
}

TEST(UserTest, SetActive)
{
    User u("id", "test", "hash", "Test");
    u.setActive(false);
    EXPECT_FALSE(u.isActive());
    u.setActive(true);
    EXPECT_TRUE(u.isActive());
}

// ══════════════════════════════════════════════════════════════════════
// Notification Tests
// ══════════════════════════════════════════════════════════════════════

TEST(NotificationTest, CreateAndAccess)
{
    Notification n("nid1", "user1", NotificationType::BUDGET_EXCEEDED,
                   "Budget exceeded for Food");
    EXPECT_EQ(n.id(), "nid1");
    EXPECT_EQ(n.userId(), "user1");
    EXPECT_EQ(n.type(), NotificationType::BUDGET_EXCEEDED);
    EXPECT_EQ(n.message(), "Budget exceeded for Food");
    EXPECT_FALSE(n.isRead());
    EXPECT_FALSE(n.createdAt().empty());
}

TEST(NotificationTest, MarkRead)
{
    Notification n("nid", "u1", NotificationType::INFO, "Test");
    EXPECT_FALSE(n.isRead());
    n.markRead();
    EXPECT_TRUE(n.isRead());
}

TEST(NotificationTest, SerializationRoundTrip)
{
    Notification original("nid2", "user2", NotificationType::GOAL_ACHIEVED,
                          "Congratulations! Goal achieved!");
    original.markRead();

    auto json = original.toJson();
    auto restored = Notification::fromJson(json);

    EXPECT_EQ(original.id(),       restored.id());
    EXPECT_EQ(original.userId(),   restored.userId());
    EXPECT_EQ(original.type(),     restored.type());
    EXPECT_EQ(original.message(),  restored.message());
    EXPECT_EQ(original.isRead(),   restored.isRead());
}

TEST(NotificationTest, TypeStringConversion)
{
    EXPECT_EQ(Notification::typeToString(NotificationType::BUDGET_EXCEEDED),   "budget_exceeded");
    EXPECT_EQ(Notification::typeToString(NotificationType::GOAL_ACHIEVED),     "goal_achieved");
    EXPECT_EQ(Notification::typeToString(NotificationType::GOAL_PROGRESS),     "goal_progress");
    EXPECT_EQ(Notification::typeToString(NotificationType::RECURRING_PAYMENT), "recurring_payment");
    EXPECT_EQ(Notification::typeToString(NotificationType::LOW_BALANCE),       "low_balance");
    EXPECT_EQ(Notification::typeToString(NotificationType::INFO),              "info");

    EXPECT_EQ(Notification::typeFromString("budget_exceeded"),   NotificationType::BUDGET_EXCEEDED);
    EXPECT_EQ(Notification::typeFromString("goal_achieved"),     NotificationType::GOAL_ACHIEVED);
    EXPECT_EQ(Notification::typeFromString("goal_progress"),     NotificationType::GOAL_PROGRESS);
    EXPECT_EQ(Notification::typeFromString("recurring_payment"), NotificationType::RECURRING_PAYMENT);
    EXPECT_EQ(Notification::typeFromString("low_balance"),       NotificationType::LOW_BALANCE);
    EXPECT_EQ(Notification::typeFromString("info"),              NotificationType::INFO);
    EXPECT_EQ(Notification::typeFromString("unknown"),           NotificationType::INFO);
}
