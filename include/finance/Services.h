#ifndef FINANCE_SERVICES_H
#define FINANCE_SERVICES_H

#include "Storage.h"

#include <memory>
#include <stack>
#include <string>
#include <vector>

namespace finance::services {

// Forward declarations
class AccountService;
class BudgetService;
class GoalService;
class NotificationService;
class TransactionService;

// ══════════════════════════════════════════════════════════════════════
// NotificationService
// ══════════════════════════════════════════════════════════════════════

class NotificationService {
public:
    explicit NotificationService(storage::IRepository<models::Notification>& repo);
    void notify(const std::string& userId, models::NotificationType type, const std::string& message);
    std::vector<models::Notification> getNotifications(const std::string& userId) const;
    std::vector<models::Notification> getUnread(const std::string& userId) const;
    void markAsRead(const std::string& notificationId);
    void markAllAsRead(const std::string& userId);
    size_t unreadCount(const std::string& userId) const;
private:
    storage::IRepository<models::Notification>& repo_;
};

// ══════════════════════════════════════════════════════════════════════
// AuthenticationService
// ══════════════════════════════════════════════════════════════════════

class AuthenticationService {
public:
    explicit AuthenticationService(storage::IRepository<models::User>& userRepo);
    models::User registerUser(const std::string& username, const std::string& password,
                               const std::string& displayName = {});
    models::User login(const std::string& username, const std::string& password);
    void changePassword(const std::string& oldPassword, const std::string& newPassword);
    void logout();
    const std::optional<models::User>& currentUser() const { return currentUser_; }
    bool isLoggedIn() const { return currentUser_.has_value(); }
private:
    static std::string hashPassword(const std::string& password, const std::string& salt);
    static bool verifyPassword(const std::string& password, const std::string& salt, const std::string& storedHash);
    storage::IRepository<models::User>& userRepo_;
    std::optional<models::User> currentUser_;
};

// ══════════════════════════════════════════════════════════════════════
// AccountService
// ══════════════════════════════════════════════════════════════════════

class AccountService {
public:
    explicit AccountService(storage::StorageManager& storage);
    models::Account* createAccount(const std::string& userId, const std::string& type,
                                    const std::string& name, const std::string& currency = "INR");
    void deleteAccount(const std::string& accountId);
    void freezeAccount(const std::string& accountId);
    void closeAccount(const std::string& accountId);
    void transfer(const std::string& fromAccountId, const std::string& toAccountId,
                  double amount, const std::string& description = {});
    std::vector<models::Account*> getAccountsByUser(const std::string& userId);
    models::Account* getAccount(const std::string& accountId);
    const std::vector<std::unique_ptr<models::Account>>& allAccounts() const;
    void saveAccounts();
private:
    void loadAccounts();
    storage::StorageManager& storage_;
    std::vector<std::unique_ptr<models::Account>> accounts_;
};

// ══════════════════════════════════════════════════════════════════════
// BudgetService
// ══════════════════════════════════════════════════════════════════════

class BudgetService {
public:
    BudgetService(storage::StorageManager& storage, NotificationService& notificationService);
    models::Budget setBudget(const std::string& userId, const std::string& categoryId,
                              int year, int month, double limitAmount);
    void removeBudget(const std::string& budgetId);
    void checkBudget(const models::Transaction& expenseTxn);
    std::vector<models::Budget> getBudgets(const std::string& userId, int year, int month) const;
    std::optional<models::Budget> getBudget(const std::string& userId, const std::string& categoryId,
                                              int year, int month) const;
    void recalculate(const std::string& userId, int year, int month);
private:
    storage::StorageManager& storage_;
    NotificationService& notificationService_;
};

// ══════════════════════════════════════════════════════════════════════
// GoalService
// ══════════════════════════════════════════════════════════════════════

class GoalService {
public:
    GoalService(storage::IRepository<models::Goal>& goalRepo, NotificationService& notificationService);
    models::Goal createGoal(const std::string& userId, const std::string& name,
                             double targetAmount, const std::string& deadline = {});
    void contribute(const std::string& goalId, double amount);
    void removeGoal(const std::string& goalId);
    std::vector<models::Goal> getGoals(const std::string& userId) const;
    std::optional<models::Goal> getGoal(const std::string& goalId) const;
private:
    storage::IRepository<models::Goal>& goalRepo_;
    NotificationService& notificationService_;
};

// ══════════════════════════════════════════════════════════════════════
// TransactionService
// ══════════════════════════════════════════════════════════════════════

class TransactionService {
public:
    TransactionService(storage::StorageManager& storage, AccountService& accountService,
                        BudgetService& budgetService, GoalService& goalService,
                        NotificationService& notificationService);

    models::Transaction* addIncome(const std::string& userId, double amount,
                                    const std::string& description, const std::string& categoryId,
                                    const std::string& accountId, const std::string& date = {},
                                    const std::string& goalId = {}, const std::vector<std::string>& tags = {});
    models::Transaction* addExpense(const std::string& userId, double amount,
                                     const std::string& description, const std::string& categoryId,
                                     const std::string& accountId, const std::string& date = {},
                                     bool isRecurring = false, const std::string& recurrenceRule = {},
                                     const std::vector<std::string>& tags = {});
    models::Transaction* addTransfer(const std::string& userId, double amount,
                                      const std::string& description, const std::string& fromAccountId,
                                      const std::string& toAccountId, const std::string& date = {});
    void editTransaction(const std::string& transactionId, double newAmount,
                          const std::string& newDescription, const std::string& newCategoryId);
    void deleteTransaction(const std::string& transactionId);
    bool undoDelete();
    std::vector<models::Transaction*> getTransactionsByUser(const std::string& userId) const;
    std::vector<models::Transaction*> getTransactionsByAccount(const std::string& accountId) const;
    std::vector<models::Transaction*> getTransactionsByCategory(const std::string& categoryId) const;
    models::Transaction* getTransaction(const std::string& txnId) const;
    size_t processRecurring();
private:
    void loadTransactions();
    void saveTransactions();
    storage::StorageManager& storage_;
    AccountService& accountService_;
    BudgetService& budgetService_;
    GoalService& goalService_;
    NotificationService& notificationService_;
    std::vector<std::unique_ptr<models::Transaction>> txns_;
    std::stack<std::unique_ptr<models::Transaction>> undoStack_;
};

// ══════════════════════════════════════════════════════════════════════
// ReportGenerator
// ══════════════════════════════════════════════════════════════════════

class ReportGenerator {
public:
    struct Report {
        std::string periodLabel;
        double totalIncome = 0.0, totalExpense = 0.0, netSavings = 0.0;
        double highestExpense = 0.0, averageSpending = 0.0;
        int totalTransactions = 0;
        std::vector<std::pair<std::string, double>> topCategories;
        std::map<std::string, double> accountSummary, categorySummary;
    };

    explicit ReportGenerator(storage::StorageManager& storage);

    Report generateDaily(const std::string& userId);
    Report generateWeekly(const std::string& userId);
    Report generateMonthly(const std::string& userId, int year, int month);
    Report generateYearly(const std::string& userId, int year);
    Report generateForRange(const std::string& userId, const std::string& dateFrom, const std::string& dateTo);

    std::string exportToCsv(const Report& r) const;
    std::string exportToJson(const Report& r) const;
    std::string exportToTxt(const Report& r) const;
    void saveToFile(const std::string& content, const std::string& filePath) const;

private:
    Report buildReport(const std::vector<models::Transaction*>& txns, const std::string& label) const;
    storage::StorageManager& storage_;
};

// ══════════════════════════════════════════════════════════════════════
// SearchEngine
// ══════════════════════════════════════════════════════════════════════

class SearchEngine {
public:
    struct Criteria {
        std::optional<std::string> dateFrom, dateTo;
        std::optional<double> amountMin, amountMax;
        std::optional<std::string> categoryId, description, tag, accountId, transactionType;
    };

    explicit SearchEngine(storage::StorageManager& storage);
    std::vector<models::Transaction*> search(const std::string& userId, const Criteria& c) const;

private:
    storage::StorageManager& storage_;
};

}  // namespace finance::services

#endif  // FINANCE_SERVICES_H
