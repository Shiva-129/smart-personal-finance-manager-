#include "finance/FinanceManager.h"
#include "finance/utils/DateUtils.h"

#include <ctime>
#include <iomanip>
#include <iostream>
#include <sstream>

namespace finance::commands {

// ── Forward declarations registered in FinanceManager::registerCommands() ──

// Each command is a small class defined below and registered by name.

// ══════════════════════════════════════════════════════════════════════
// 1.  REGISTER
// ══════════════════════════════════════════════════════════════════════

class RegisterCommand : public ICommand {
    services::AuthenticationService& auth_;
public:
    explicit RegisterCommand(services::AuthenticationService& a) : auth_(a) {}
    std::string name() const override { return "register"; }
    std::string description() const override { return "Create a new user account"; }
    std::string usage() const override { return "register <username> <password> [display-name]"; }
    void execute(const std::vector<std::string>& args) override {
        if (args.size() < 2) { std::cout << "Usage: " << usage() << "\n"; return; }
        std::string display = args.size() > 2 ? args[2] : args[0];
        auto user = auth_.registerUser(args[0], args[1], display);
        std::cout << "User '" << user.username() << "' registered successfully.\n";
    }
};

// ══════════════════════════════════════════════════════════════════════
// 2.  LOGIN
// ══════════════════════════════════════════════════════════════════════

class LoginCommand : public ICommand {
    services::AuthenticationService& auth_;
public:
    explicit LoginCommand(services::AuthenticationService& a) : auth_(a) {}
    std::string name() const override { return "login"; }
    std::string description() const override { return "Log in with username and password"; }
    std::string usage() const override { return "login <username> <password>"; }
    void execute(const std::vector<std::string>& args) override {
        if (args.size() < 2) { std::cout << "Usage: " << usage() << "\n"; return; }
        auto user = auth_.login(args[0], args[1]);
        std::cout << "Welcome back, " << user.displayName() << "!\n";
    }
};

// ══════════════════════════════════════════════════════════════════════
// 3.  LOGOUT
// ══════════════════════════════════════════════════════════════════════

class LogoutCommand : public ICommand {
    services::AuthenticationService& auth_;
public:
    explicit LogoutCommand(services::AuthenticationService& a) : auth_(a) {}
    std::string name() const override { return "logout"; }
    std::string description() const override { return "Log out the current user"; }
    std::string usage() const override { return "logout"; }
    void execute(const std::vector<std::string>&) override {
        auth_.logout();
        std::cout << "Logged out.\n";
    }
};

// ══════════════════════════════════════════════════════════════════════
// 4.  CHANGE-PASSWORD
// ══════════════════════════════════════════════════════════════════════

class ChangePasswordCommand : public ICommand {
    services::AuthenticationService& auth_;
public:
    explicit ChangePasswordCommand(services::AuthenticationService& a) : auth_(a) {}
    std::string name() const override { return "change-password"; }
    std::string description() const override { return "Change the current user's password"; }
    std::string usage() const override { return "change-password <old> <new>"; }
    void execute(const std::vector<std::string>& args) override {
        if (args.size() < 2) { std::cout << "Usage: " << usage() << "\n"; return; }
        auth_.changePassword(args[0], args[1]);
        std::cout << "Password changed successfully.\n";
    }
};

// ══════════════════════════════════════════════════════════════════════
// 5.  ADD-ACCOUNT
// ══════════════════════════════════════════════════════════════════════

class AddAccountCommand : public ICommand {
    services::AuthenticationService& auth_;
    services::AccountService& acc_;
public:
    AddAccountCommand(services::AuthenticationService& a, services::AccountService& ac)
        : auth_(a), acc_(ac) {}
    std::string name() const override { return "add-account"; }
    std::string description() const override { return "Create a new account (Cash/Savings/Current/Credit Card/Wallet)"; }
    std::string usage() const override { return "add-account <type> <name> [currency]"; }
    void execute(const std::vector<std::string>& args) override {
        if (!auth_.isLoggedIn()) { std::cout << "Please login first.\n"; return; }
        if (args.size() < 2) { std::cout << "Usage: " << usage() << "\n"; return; }
        std::string currency = args.size() > 2 ? args[2] : "INR";
        auto* acc = acc_.createAccount(auth_.currentUser()->id(), args[0], args[1], currency);
        std::cout << "Account '" << acc->name() << "' (" << acc->accountTypeName()
                  << ") created. ID: " << acc->id() << "\n";
    }
};

// ══════════════════════════════════════════════════════════════════════
// 6.  LIST-ACCOUNTS
// ══════════════════════════════════════════════════════════════════════

class ListAccountsCommand : public ICommand {
    services::AuthenticationService& auth_;
    services::AccountService& acc_;
public:
    ListAccountsCommand(services::AuthenticationService& a, services::AccountService& ac)
        : auth_(a), acc_(ac) {}
    std::string name() const override { return "list-accounts"; }
    std::string description() const override { return "Show all accounts for the current user"; }
    std::string usage() const override { return "list-accounts"; }
    void execute(const std::vector<std::string>&) override {
        if (!auth_.isLoggedIn()) { std::cout << "Please login first.\n"; return; }
        auto accounts = acc_.getAccountsByUser(auth_.currentUser()->id());
        if (accounts.empty()) { std::cout << "No accounts found.\n"; return; }
        std::cout << "\n── Accounts ──────────────────────────────\n";
        for (const auto* a : accounts) {
            std::cout << "  " << a->name() << " (" << a->accountTypeName() << ")\n"
                      << "    Balance: " << std::fixed << std::setprecision(2)
                      << a->balance() << " " << a->currency() << "\n"
                      << "    Status:  " << (a->status() == models::AccountStatus::ACTIVE ? "Active"
                                              : a->status() == models::AccountStatus::FROZEN ? "Frozen" : "Closed")
                      << "\n    ID:      " << a->id() << "\n\n";
        }
    }
};

// ══════════════════════════════════════════════════════════════════════
// 7.  FREEZE-ACCOUNT / CLOSE-ACCOUNT
// ══════════════════════════════════════════════════════════════════════

class FreezeAccountCommand : public ICommand {
    services::AccountService& acc_;
public:
    explicit FreezeAccountCommand(services::AccountService& a) : acc_(a) {}
    std::string name() const override { return "freeze-account"; }
    std::string description() const override { return "Freeze an account by ID"; }
    std::string usage() const override { return "freeze-account <account-id>"; }
    void execute(const std::vector<std::string>& args) override {
        if (args.empty()) { std::cout << "Usage: " << usage() << "\n"; return; }
        acc_.freezeAccount(args[0]);
        std::cout << "Account frozen.\n";
    }
};

class CloseAccountCommand : public ICommand {
    services::AccountService& acc_;
public:
    explicit CloseAccountCommand(services::AccountService& a) : acc_(a) {}
    std::string name() const override { return "close-account"; }
    std::string description() const override { return "Close an account by ID"; }
    std::string usage() const override { return "close-account <account-id>"; }
    void execute(const std::vector<std::string>& args) override {
        if (args.empty()) { std::cout << "Usage: " << usage() << "\n"; return; }
        acc_.closeAccount(args[0]);
        std::cout << "Account closed.\n";
    }
};

// ══════════════════════════════════════════════════════════════════════
// 8.  ADD-INCOME
// ══════════════════════════════════════════════════════════════════════

class AddIncomeCommand : public ICommand {
    services::AuthenticationService& auth_;
    services::TransactionService& txn_;
public:
    AddIncomeCommand(services::AuthenticationService& a, services::TransactionService& t)
        : auth_(a), txn_(t) {}
    std::string name() const override { return "add-income"; }
    std::string description() const override { return "Record an income transaction"; }
    std::string usage() const override { return "add-income <amount> <description> <category-id> <account-id> [goal-id] [tags...]"; }
    void execute(const std::vector<std::string>& args) override {
        if (!auth_.isLoggedIn()) { std::cout << "Please login first.\n"; return; }
        if (args.size() < 4) { std::cout << "Usage: " << usage() << "\n"; return; }
        double amount = std::stod(args[0]);
        std::string goalId = args.size() > 4 ? args[4] : "";
        std::vector<std::string> tags;
        for (size_t i = (args.size() > 4 ? 5 : 4); i < args.size(); ++i) tags.push_back(args[i]);
        auto* t = txn_.addIncome(auth_.currentUser()->id(), amount, args[1], args[2], args[3], {}, goalId, tags);
        std::cout << "Income recorded: " << args[1] << " (" << amount << ")\n";
    }
};

// ══════════════════════════════════════════════════════════════════════
// 9.  ADD-EXPENSE
// ══════════════════════════════════════════════════════════════════════

class AddExpenseCommand : public ICommand {
    services::AuthenticationService& auth_;
    services::TransactionService& txn_;
public:
    AddExpenseCommand(services::AuthenticationService& a, services::TransactionService& t)
        : auth_(a), txn_(t) {}
    std::string name() const override { return "add-expense"; }
    std::string description() const override { return "Record an expense transaction"; }
    std::string usage() const override { return "add-expense <amount> <description> <category-id> <account-id> [recurring] [rule] [tags...]"; }
    void execute(const std::vector<std::string>& args) override {
        if (!auth_.isLoggedIn()) { std::cout << "Please login first.\n"; return; }
        if (args.size() < 4) { std::cout << "Usage: " << usage() << "\n"; return; }
        double amount = std::stod(args[0]);
        bool recurring = args.size() > 4 && args[4] == "true";
        std::string rule = args.size() > 5 ? args[5] : "";
        std::vector<std::string> tags;
        for (size_t i = (args.size() > 5 ? 6 : (args.size() > 4 ? 5 : 4)); i < args.size(); ++i) tags.push_back(args[i]);
        auto* t = txn_.addExpense(auth_.currentUser()->id(), amount, args[1], args[2], args[3], {}, recurring, rule, tags);
        std::cout << "Expense recorded: " << args[1] << " (" << amount << ")\n";
    }
};

// ══════════════════════════════════════════════════════════════════════
// 10. TRANSFER
// ══════════════════════════════════════════════════════════════════════

class TransferCommand : public ICommand {
    services::AuthenticationService& auth_;
    services::TransactionService& txn_;
public:
    TransferCommand(services::AuthenticationService& a, services::TransactionService& t)
        : auth_(a), txn_(t) {}
    std::string name() const override { return "transfer"; }
    std::string description() const override { return "Transfer money between accounts"; }
    std::string usage() const override { return "transfer <amount> <description> <from-account-id> <to-account-id>"; }
    void execute(const std::vector<std::string>& args) override {
        if (!auth_.isLoggedIn()) { std::cout << "Please login first.\n"; return; }
        if (args.size() < 4) { std::cout << "Usage: " << usage() << "\n"; return; }
        double amount = std::stod(args[0]);
        txn_.addTransfer(auth_.currentUser()->id(), amount, args[1], args[2], args[3]);
        std::cout << "Transfer completed: " << args[1] << " (" << amount << ")\n";
    }
};

// ══════════════════════════════════════════════════════════════════════
// 11. LIST-TRANSACTIONS
// ══════════════════════════════════════════════════════════════════════

class ListTransactionsCommand : public ICommand {
    services::AuthenticationService& auth_;
    services::TransactionService& txn_;
public:
    ListTransactionsCommand(services::AuthenticationService& a, services::TransactionService& t)
        : auth_(a), txn_(t) {}
    std::string name() const override { return "list-transactions"; }
    std::string description() const override { return "Show transactions (optionally filter by account)"; }
    std::string usage() const override { return "list-transactions [account-id]"; }
    void execute(const std::vector<std::string>& args) override {
        if (!auth_.isLoggedIn()) { std::cout << "Please login first.\n"; return; }
        std::vector<models::Transaction*> txns;
        if (args.empty()) {
            txns = txn_.getTransactionsByUser(auth_.currentUser()->id());
        } else {
            txns = txn_.getTransactionsByAccount(args[0]);
        }
        if (txns.empty()) { std::cout << "No transactions found.\n"; return; }
        std::cout << "\n── Transactions ───────────────────────────\n";
        for (const auto* t : txns) {
            std::cout << "  [" << t->date() << "] " << std::setw(8) << std::left
                      << t->transactionTypeName() << " "
                      << std::fixed << std::setprecision(2) << t->amount()
                      << "  " << t->description() << "\n";
        }
        std::cout << std::endl;
    }
};

// ══════════════════════════════════════════════════════════════════════
// 12. EDIT-TRANSACTION
// ══════════════════════════════════════════════════════════════════════

class EditTransactionCommand : public ICommand {
    services::TransactionService& txn_;
public:
    explicit EditTransactionCommand(services::TransactionService& t) : txn_(t) {}
    std::string name() const override { return "edit-transaction"; }
    std::string description() const override { return "Edit a transaction's amount, description, category"; }
    std::string usage() const override { return "edit-transaction <id> <amount> <description> <category-id>"; }
    void execute(const std::vector<std::string>& args) override {
        if (args.size() < 4) { std::cout << "Usage: " << usage() << "\n"; return; }
        txn_.editTransaction(args[0], std::stod(args[1]), args[2], args[3]);
        std::cout << "Transaction updated.\n";
    }
};

// ══════════════════════════════════════════════════════════════════════
// 13. DELETE-TRANSACTION
// ══════════════════════════════════════════════════════════════════════

class DeleteTransactionCommand : public ICommand {
    services::TransactionService& txn_;
public:
    explicit DeleteTransactionCommand(services::TransactionService& t) : txn_(t) {}
    std::string name() const override { return "delete-transaction"; }
    std::string description() const override { return "Delete a transaction (can be undone)"; }
    std::string usage() const override { return "delete-transaction <transaction-id>"; }
    void execute(const std::vector<std::string>& args) override {
        if (args.empty()) { std::cout << "Usage: " << usage() << "\n"; return; }
        txn_.deleteTransaction(args[0]);
        std::cout << "Transaction deleted. Use 'undo' to restore.\n";
    }
};

// ══════════════════════════════════════════════════════════════════════
// 14. UNDO
// ══════════════════════════════════════════════════════════════════════

class UndoCommand : public ICommand {
    services::TransactionService& txn_;
public:
    explicit UndoCommand(services::TransactionService& t) : txn_(t) {}
    std::string name() const override { return "undo"; }
    std::string description() const override { return "Undo the most recent delete"; }
    std::string usage() const override { return "undo"; }
    void execute(const std::vector<std::string>&) override {
        if (txn_.undoDelete()) {
            std::cout << "Last delete undone.\n";
        } else {
            std::cout << "Nothing to undo.\n";
        }
    }
};

// ══════════════════════════════════════════════════════════════════════
// 15. SET-BUDGET
// ══════════════════════════════════════════════════════════════════════

class SetBudgetCommand : public ICommand {
    services::AuthenticationService& auth_;
    services::BudgetService& budget_;
public:
    SetBudgetCommand(services::AuthenticationService& a, services::BudgetService& b)
        : auth_(a), budget_(b) {}
    std::string name() const override { return "set-budget"; }
    std::string description() const override { return "Set a monthly budget for a category"; }
    std::string usage() const override { return "set-budget <category-id> <limit> [year] [month]"; }
    void execute(const std::vector<std::string>& args) override {
        if (!auth_.isLoggedIn()) { std::cout << "Please login first.\n"; return; }
        if (args.size() < 2) { std::cout << "Usage: " << usage() << "\n"; return; }
        // Get current year/month as defaults.
        std::time_t t = std::time(nullptr);
        std::tm* now = std::localtime(&t);
        int year  = args.size() > 2 ? std::stoi(args[2]) : (now->tm_year + 1900);
        int month = args.size() > 3 ? std::stoi(args[3]) : (now->tm_mon + 1);
        auto budget = budget_.setBudget(auth_.currentUser()->id(), args[0], year, month, std::stod(args[1]));
        std::cout << "Budget set for category " << args[0] << ": "
                  << budget.limitAmount() << " ("
                  << finance::utils::DateUtils::monthName(month) << " " << year << ")\n";
    }
};

// ══════════════════════════════════════════════════════════════════════
// 16. SHOW-BUDGET
// ══════════════════════════════════════════════════════════════════════

class ShowBudgetCommand : public ICommand {
    services::AuthenticationService& auth_;
    services::BudgetService& budget_;
public:
    ShowBudgetCommand(services::AuthenticationService& a, services::BudgetService& b)
        : auth_(a), budget_(b) {}
    std::string name() const override { return "show-budget"; }
    std::string description() const override { return "Show budgets for a month"; }
    std::string usage() const override { return "show-budget [year] [month]"; }
    void execute(const std::vector<std::string>& args) override {
        if (!auth_.isLoggedIn()) { std::cout << "Please login first.\n"; return; }
        std::time_t t = std::time(nullptr);
        std::tm* now = std::localtime(&t);
        int year  = args.size() > 0 ? std::stoi(args[0]) : (now->tm_year + 1900);
        int month = args.size() > 1 ? std::stoi(args[1]) : (now->tm_mon + 1);
        auto budgets = budget_.getBudgets(auth_.currentUser()->id(), year, month);
        if (budgets.empty()) { std::cout << "No budgets set for " << finance::utils::DateUtils::monthName(month) << " " << year << "\n"; return; }
        std::cout << "\n── Budgets for " << finance::utils::DateUtils::monthName(month) << " " << year << " ──\n";
        for (const auto& b : budgets) {
            double pct = b.percentageUsed();
            std::cout << "  Category: " << b.categoryId() << "\n"
                      << "    Budget:    " << std::fixed << std::setprecision(2) << b.limitAmount() << "\n"
                      << "    Spent:     " << b.spentAmount() << " (" << static_cast<int>(pct) << "%)\n"
                      << "    Remaining: " << b.remaining() << "\n";
            if (b.isExceeded()) std::cout << "    ⚠ OVER BUDGET!\n";
            std::cout << "\n";
        }
    }
};

// ══════════════════════════════════════════════════════════════════════
// 17. CREATE-GOAL
// ══════════════════════════════════════════════════════════════════════

class CreateGoalCommand : public ICommand {
    services::AuthenticationService& auth_;
    services::GoalService& goal_;
public:
    CreateGoalCommand(services::AuthenticationService& a, services::GoalService& g)
        : auth_(a), goal_(g) {}
    std::string name() const override { return "create-goal"; }
    std::string description() const override { return "Create a new savings goal"; }
    std::string usage() const override { return "create-goal <name> <target-amount> [deadline]"; }
    void execute(const std::vector<std::string>& args) override {
        if (!auth_.isLoggedIn()) { std::cout << "Please login first.\n"; return; }
        if (args.size() < 2) { std::cout << "Usage: " << usage() << "\n"; return; }
        std::string deadline = args.size() > 2 ? args[2] : "";
        auto goal = goal_.createGoal(auth_.currentUser()->id(), args[0], std::stod(args[1]), deadline);
        std::cout << "Goal '" << goal.name() << "' created (target: " << goal.targetAmount() << "). ID: " << goal.id() << "\n";
    }
};

// ══════════════════════════════════════════════════════════════════════
// 18. CONTRIBUTE-GOAL
// ══════════════════════════════════════════════════════════════════════

class ContributeGoalCommand : public ICommand {
    services::GoalService& goal_;
public:
    explicit ContributeGoalCommand(services::GoalService& g) : goal_(g) {}
    std::string name() const override { return "contribute-goal"; }
    std::string description() const override { return "Contribute money to a savings goal"; }
    std::string usage() const override { return "contribute-goal <goal-id> <amount>"; }
    void execute(const std::vector<std::string>& args) override {
        if (args.size() < 2) { std::cout << "Usage: " << usage() << "\n"; return; }
        goal_.contribute(args[0], std::stod(args[1]));
        std::cout << "Contributed to goal.\n";
    }
};

// ══════════════════════════════════════════════════════════════════════
// 19. SHOW-GOALS
// ══════════════════════════════════════════════════════════════════════

class ShowGoalsCommand : public ICommand {
    services::AuthenticationService& auth_;
    services::GoalService& goal_;
public:
    ShowGoalsCommand(services::AuthenticationService& a, services::GoalService& g)
        : auth_(a), goal_(g) {}
    std::string name() const override { return "show-goals"; }
    std::string description() const override { return "Show all savings goals"; }
    std::string usage() const override { return "show-goals"; }
    void execute(const std::vector<std::string>&) override {
        if (!auth_.isLoggedIn()) { std::cout << "Please login first.\n"; return; }
        auto goals = goal_.getGoals(auth_.currentUser()->id());
        if (goals.empty()) { std::cout << "No goals found.\n"; return; }
        std::cout << "\n── Goals ──────────────────────────────────\n";
        for (const auto& g : goals) {
            int pct = static_cast<int>(g.progressPercentage());
            std::cout << "  " << g.name() << "\n"
                      << "    Target:  " << std::fixed << std::setprecision(2) << g.targetAmount() << "\n"
                      << "    Current: " << g.currentAmount() << "\n"
                      << "    Progress:" << std::string(pct / 10, '#')  // progress bar
                                         << std::string(10 - pct / 10, '-')
                                         << " " << pct << "%\n"
                      << "    Status:  " << (g.isAchieved() ? "✓ ACHIEVED!" : "In progress") << "\n\n";
        }
    }
};

// ══════════════════════════════════════════════════════════════════════
// 20. SHOW-NOTIFICATIONS
// ══════════════════════════════════════════════════════════════════════

class ShowNotificationsCommand : public ICommand {
    services::AuthenticationService& auth_;
    services::NotificationService& notif_;
public:
    ShowNotificationsCommand(services::AuthenticationService& a, services::NotificationService& n)
        : auth_(a), notif_(n) {}
    std::string name() const override { return "show-notifications"; }
    std::string description() const override { return "Show notifications and alerts"; }
    std::string usage() const override { return "show-notifications"; }
    void execute(const std::vector<std::string>&) override {
        if (!auth_.isLoggedIn()) { std::cout << "Please login first.\n"; return; }
        auto notifs = notif_.getNotifications(auth_.currentUser()->id());
        if (notifs.empty()) { std::cout << "No notifications.\n"; return; }
        std::cout << "\n── Notifications ──────────────────────────\n";
        for (const auto& n : notifs) {
            std::cout << "  [" << n.createdAt() << "] "
                      << (n.isRead() ? " " : "●") << " "
                      << n.message() << "\n";
        }
        // Mark all as read.
        notif_.markAllAsRead(auth_.currentUser()->id());
        std::cout << std::endl;
    }
};

// ══════════════════════════════════════════════════════════════════════
// 21. SEARCH
// ══════════════════════════════════════════════════════════════════════

class SearchCommand : public ICommand {
    services::AuthenticationService& auth_;
    services::SearchEngine& search_;
public:
    SearchCommand(services::AuthenticationService& a, services::SearchEngine& s)
        : auth_(a), search_(s) {}
    std::string name() const override { return "search"; }
    std::string description() const override { return "Search transactions (filters: date-from date-to amount-min amount-max description)"; }
    std::string usage() const override { return "search [date-from] [date-to] [amount-min] [amount-max] [description]"; }
    void execute(const std::vector<std::string>& args) override {
        if (!auth_.isLoggedIn()) { std::cout << "Please login first.\n"; return; }
        services::SearchEngine::Criteria c;
        if (args.size() > 0 && args[0] != "*") c.dateFrom    = args[0];
        if (args.size() > 1 && args[1] != "*") c.dateTo      = args[1];
        if (args.size() > 2 && args[2] != "*") c.amountMin   = std::stod(args[2]);
        if (args.size() > 3 && args[3] != "*") c.amountMax   = std::stod(args[3]);
        if (args.size() > 4 && args[4] != "*") c.description = args[4];
        auto results = search_.search(auth_.currentUser()->id(), c);
        if (results.empty()) { std::cout << "No matching transactions.\n"; return; }
        std::cout << "\n── Search Results (" << results.size() << ") ────────\n";
        for (const auto* t : results) {
            std::cout << "  [" << t->date() << "] " << std::setw(8) << std::left
                      << t->transactionTypeName() << " "
                      << std::fixed << std::setprecision(2) << t->amount()
                      << "  " << t->description() << "\n";
        }
        std::cout << std::endl;
    }
};

// ══════════════════════════════════════════════════════════════════════
// 22. GENERATE-REPORT
// ══════════════════════════════════════════════════════════════════════

class GenerateReportCommand : public ICommand {
    services::AuthenticationService& auth_;
    services::ReportGenerator& report_;
public:
    GenerateReportCommand(services::AuthenticationService& a, services::ReportGenerator& r)
        : auth_(a), report_(r) {}
    std::string name() const override { return "generate-report"; }
    std::string description() const override { return "Generate a report (daily/weekly/monthly/yearly)"; }
    std::string usage() const override { return "generate-report <daily|weekly|monthly|yearly> [year] [month]"; }
    void execute(const std::vector<std::string>& args) override {
        if (!auth_.isLoggedIn()) { std::cout << "Please login first.\n"; return; }
        if (args.empty()) { std::cout << "Usage: " << usage() << "\n"; return; }
        std::time_t t = std::time(nullptr);
        std::tm* now = std::localtime(&t);
        int year  = args.size() > 1 ? std::stoi(args[1]) : (now->tm_year + 1900);
        int month = args.size() > 2 ? std::stoi(args[2]) : (now->tm_mon + 1);

        services::ReportGenerator::Report r;
        if (args[0] == "daily")    r = report_.generateDaily(auth_.currentUser()->id());
        else if (args[0] == "weekly")  r = report_.generateWeekly(auth_.currentUser()->id());
        else if (args[0] == "monthly") r = report_.generateMonthly(auth_.currentUser()->id(), year, month);
        else if (args[0] == "yearly")  r = report_.generateYearly(auth_.currentUser()->id(), year);
        else { std::cout << "Invalid period. Use: daily, weekly, monthly, yearly\n"; return; }

        std::cout << "\n" << report_.exportToTxt(r) << std::endl;
    }
};

// ══════════════════════════════════════════════════════════════════════
// 23. EXPORT-REPORT
// ══════════════════════════════════════════════════════════════════════

class ExportReportCommand : public ICommand {
    services::AuthenticationService& auth_;
    services::ReportGenerator& report_;
public:
    ExportReportCommand(services::AuthenticationService& a, services::ReportGenerator& r)
        : auth_(a), report_(r) {}
    std::string name() const override { return "export-report"; }
    std::string description() const override { return "Export a report to file (csv/json/txt)"; }
    std::string usage() const override { return "export-report <csv|json|txt> <daily|weekly|monthly|yearly> <filepath> [year] [month]"; }
    void execute(const std::vector<std::string>& args) override {
        if (!auth_.isLoggedIn()) { std::cout << "Please login first.\n"; return; }
        if (args.size() < 3) { std::cout << "Usage: " << usage() << "\n"; return; }
        std::time_t t = std::time(nullptr);
        std::tm* now = std::localtime(&t);
        int year  = args.size() > 3 ? std::stoi(args[3]) : (now->tm_year + 1900);
        int month = args.size() > 4 ? std::stoi(args[4]) : (now->tm_mon + 1);

        services::ReportGenerator::Report r;
        if (args[1] == "daily")    r = report_.generateDaily(auth_.currentUser()->id());
        else if (args[1] == "weekly")  r = report_.generateWeekly(auth_.currentUser()->id());
        else if (args[1] == "monthly") r = report_.generateMonthly(auth_.currentUser()->id(), year, month);
        else if (args[1] == "yearly")  r = report_.generateYearly(auth_.currentUser()->id(), year);
        else { std::cout << "Invalid period.\n"; return; }

        std::string content;
        if (args[0] == "csv")       content = report_.exportToCsv(r);
        else if (args[0] == "json") content = report_.exportToJson(r);
        else if (args[0] == "txt")  content = report_.exportToTxt(r);
        else { std::cout << "Invalid format.\n"; return; }

        report_.saveToFile(content, args[2]);
        std::cout << "Report exported to " << args[2] << "\n";
    }
};

// ══════════════════════════════════════════════════════════════════════
// 24. PROCESS-RECURRING
// ══════════════════════════════════════════════════════════════════════

class ProcessRecurringCommand : public ICommand {
    services::TransactionService& txn_;
public:
    explicit ProcessRecurringCommand(services::TransactionService& t) : txn_(t) {}
    std::string name() const override { return "process-recurring"; }
    std::string description() const override { return "Generate recurring transactions that are due"; }
    std::string usage() const override { return "process-recurring"; }
    void execute(const std::vector<std::string>&) override {
        size_t count = txn_.processRecurring();
        std::cout << "Generated " << count << " recurring transaction(s).\n";
    }
};

}  // namespace finance::commands

// ══════════════════════════════════════════════════════════════════════
// Registration (in the finance namespace, not finance::commands)
// ══════════════════════════════════════════════════════════════════════

void finance::FinanceManager::registerCommands()
{
    using namespace finance::commands;

    // Quick exit if no auth service (defensive).
    if (!authService_) return;

    parser_.registerCommand(std::make_unique<RegisterCommand>(*authService_));
    parser_.registerCommand(std::make_unique<LoginCommand>(*authService_));
    parser_.registerCommand(std::make_unique<LogoutCommand>(*authService_));
    parser_.registerCommand(std::make_unique<ChangePasswordCommand>(*authService_));

    if (accountService_) {
        parser_.registerCommand(std::make_unique<AddAccountCommand>(*authService_, *accountService_));
        parser_.registerCommand(std::make_unique<ListAccountsCommand>(*authService_, *accountService_));
        parser_.registerCommand(std::make_unique<FreezeAccountCommand>(*accountService_));
        parser_.registerCommand(std::make_unique<CloseAccountCommand>(*accountService_));
    }

    if (transactionService_) {
        parser_.registerCommand(std::make_unique<AddIncomeCommand>(*authService_, *transactionService_));
        parser_.registerCommand(std::make_unique<AddExpenseCommand>(*authService_, *transactionService_));
        parser_.registerCommand(std::make_unique<TransferCommand>(*authService_, *transactionService_));
        parser_.registerCommand(std::make_unique<ListTransactionsCommand>(*authService_, *transactionService_));
        parser_.registerCommand(std::make_unique<EditTransactionCommand>(*transactionService_));
        parser_.registerCommand(std::make_unique<DeleteTransactionCommand>(*transactionService_));
        parser_.registerCommand(std::make_unique<UndoCommand>(*transactionService_));
        parser_.registerCommand(std::make_unique<ProcessRecurringCommand>(*transactionService_));
    }

    if (budgetService_) {
        parser_.registerCommand(std::make_unique<SetBudgetCommand>(*authService_, *budgetService_));
        parser_.registerCommand(std::make_unique<ShowBudgetCommand>(*authService_, *budgetService_));
    }

    if (goalService_) {
        parser_.registerCommand(std::make_unique<CreateGoalCommand>(*authService_, *goalService_));
        parser_.registerCommand(std::make_unique<ContributeGoalCommand>(*goalService_));
        parser_.registerCommand(std::make_unique<ShowGoalsCommand>(*authService_, *goalService_));
    }

    if (notificationService_) {
        parser_.registerCommand(std::make_unique<ShowNotificationsCommand>(*authService_, *notificationService_));
    }

    if (searchEngine_) {
        parser_.registerCommand(std::make_unique<SearchCommand>(*authService_, *searchEngine_));
    }

    if (reportGenerator_) {
        parser_.registerCommand(std::make_unique<GenerateReportCommand>(*authService_, *reportGenerator_));
        parser_.registerCommand(std::make_unique<ExportReportCommand>(*authService_, *reportGenerator_));
    }
}
