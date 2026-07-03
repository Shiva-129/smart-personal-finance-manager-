#include "finance/Commands.h"
#include "finance/Utils.h"

#include <ctime>
#include <iomanip>
#include <iostream>

using namespace finance::services;
using namespace finance::models;
using namespace finance::utils;

// ══════════════════════════════════════════════════════════════════════
// 1.  REGISTER
// ══════════════════════════════════════════════════════════════════════

class RegisterCmd : public finance::commands::ICommand {
    AuthenticationService& a_;
public:
    explicit RegisterCmd(AuthenticationService& a) : a_(a) {}
    std::string name() const override { return "register"; }
    std::string description() const override { return "Create a new user account"; }
    std::string usage() const override { return "register <username> <password> [display-name]"; }
    void execute(const std::vector<std::string>& args) override {
        if(args.size()<2){std::cout<<"Usage: "<<usage()<<"\n";return;}
        auto u=a_.registerUser(args[0],args[1],args.size()>2?args[2]:args[0]);
        std::cout<<"User '"<<u.username()<<"' registered.\n";
    }
};

// ══════════════════════════════════════════════════════════════════════
// 2.  LOGIN
// ══════════════════════════════════════════════════════════════════════

class LoginCmd : public finance::commands::ICommand {
    AuthenticationService& a_;
public:
    explicit LoginCmd(AuthenticationService& a) : a_(a) {}
    std::string name() const override { return "login"; }
    std::string description() const override { return "Log in with username and password"; }
    std::string usage() const override { return "login <username> <password>"; }
    void execute(const std::vector<std::string>& args) override {
        if(args.size()<2){std::cout<<"Usage: "<<usage()<<"\n";return;}
        auto u=a_.login(args[0],args[1]);
        std::cout<<"Welcome, "<<u.displayName()<<"!\n";
    }
};

// ══════════════════════════════════════════════════════════════════════
// 3.  LOGOUT
// ══════════════════════════════════════════════════════════════════════

class LogoutCmd : public finance::commands::ICommand {
    AuthenticationService& a_;
public:
    explicit LogoutCmd(AuthenticationService& a) : a_(a) {}
    std::string name() const override { return "logout"; }
    std::string description() const override { return "Log out the current user"; }
    std::string usage() const override { return "logout"; }
    void execute(const std::vector<std::string>&) override { a_.logout(); std::cout<<"Logged out.\n"; }
};

// ══════════════════════════════════════════════════════════════════════
// 4.  CHANGE-PASSWORD
// ══════════════════════════════════════════════════════════════════════

class ChangePwCmd : public finance::commands::ICommand {
    AuthenticationService& a_;
public:
    explicit ChangePwCmd(AuthenticationService& a) : a_(a) {}
    std::string name() const override { return "change-password"; }
    std::string description() const override { return "Change your password"; }
    std::string usage() const override { return "change-password <old> <new>"; }
    void execute(const std::vector<std::string>& args) override {
        if(args.size()<2){std::cout<<"Usage: "<<usage()<<"\n";return;}
        a_.changePassword(args[0],args[1]); std::cout<<"Password changed.\n";
    }
};

// ══════════════════════════════════════════════════════════════════════
// Helper: check logged in
// ══════════════════════════════════════════════════════════════════════

static bool requireLogin(AuthenticationService& a) {
    if(!a.isLoggedIn()){std::cout<<"Please login first.\n";return false;}
    return true;
}

static std::string userId(AuthenticationService& a) { return a.currentUser()->id(); }

// ══════════════════════════════════════════════════════════════════════
// 5.  ADD-ACCOUNT
// ══════════════════════════════════════════════════════════════════════

class AddAcctCmd : public finance::commands::ICommand {
    AuthenticationService& a_; AccountService& ac_;
public:
    AddAcctCmd(AuthenticationService& a, AccountService& ac) : a_(a), ac_(ac) {}
    std::string name() const override { return "add-account"; }
    std::string description() const override { return "Create account: Cash/Savings/Current/Credit Card/Wallet"; }
    std::string usage() const override { return "add-account <type> <name> [currency]"; }
    void execute(const std::vector<std::string>& args) override {
        if(!requireLogin(a_))return; if(args.size()<2){std::cout<<"Usage: "<<usage()<<"\n";return;}
        auto* a=ac_.createAccount(userId(a_),args[0],args[1],args.size()>2?args[2]:"INR");
        std::cout<<"Account '"<<a->name()<<"' ("<<a->accountTypeName()<<") created.\n";
    }
};

// ══════════════════════════════════════════════════════════════════════
// 6.  LIST-ACCOUNTS
// ══════════════════════════════════════════════════════════════════════

class ListAcctsCmd : public finance::commands::ICommand {
    AuthenticationService& a_; AccountService& ac_;
public:
    ListAcctsCmd(AuthenticationService& a, AccountService& ac) : a_(a), ac_(ac) {}
    std::string name() const override { return "list-accounts"; }
    std::string description() const override { return "Show your accounts"; }
    std::string usage() const override { return "list-accounts"; }
    void execute(const std::vector<std::string>&) override {
        if(!requireLogin(a_))return;
        auto accts=ac_.getAccountsByUser(userId(a_)); if(accts.empty()){std::cout<<"No accounts.\n";return;}
        std::cout<<"\n── Accounts ──\n";
        for(auto*a:accts)std::cout<<"  "<<a->name()<<" ("<<a->accountTypeName()<<") Balance: "<<std::fixed<<std::setprecision(2)<<a->balance()<<" "<<a->currency()<<"\n";
        std::cout<<"\n";
    }
};

// ══════════════════════════════════════════════════════════════════════
// 7.  FREEZE-ACCOUNT
// ══════════════════════════════════════════════════════════════════════

class FreezeAcctCmd : public finance::commands::ICommand {
    AccountService& ac_;
public:
    explicit FreezeAcctCmd(AccountService& ac) : ac_(ac) {}
    std::string name() const override { return "freeze-account"; }
    std::string description() const override { return "Freeze an account"; }
    std::string usage() const override { return "freeze-account <account-id>"; }
    void execute(const std::vector<std::string>& a) override {
        if(a.empty()){std::cout<<"Usage: "<<usage()<<"\n";return;} ac_.freezeAccount(a[0]); std::cout<<"Frozen.\n";
    }
};

// ══════════════════════════════════════════════════════════════════════
// 8.  CLOSE-ACCOUNT
// ══════════════════════════════════════════════════════════════════════

class CloseAcctCmd : public finance::commands::ICommand {
    AccountService& ac_;
public:
    explicit CloseAcctCmd(AccountService& ac) : ac_(ac) {}
    std::string name() const override { return "close-account"; }
    std::string description() const override { return "Close an account"; }
    std::string usage() const override { return "close-account <account-id>"; }
    void execute(const std::vector<std::string>& a) override {
        if(a.empty()){std::cout<<"Usage: "<<usage()<<"\n";return;} ac_.closeAccount(a[0]); std::cout<<"Closed.\n";
    }
};

// ══════════════════════════════════════════════════════════════════════
// 9.  ADD-INCOME
// ══════════════════════════════════════════════════════════════════════

class IncomeCmd : public finance::commands::ICommand {
    AuthenticationService& a_; TransactionService& t_;
public:
    IncomeCmd(AuthenticationService& a, TransactionService& t) : a_(a), t_(t) {}
    std::string name() const override { return "add-income"; }
    std::string description() const override { return "Record income"; }
    std::string usage() const override { return "add-income <amount> <desc> <category-id> <account-id> [goal-id]"; }
    void execute(const std::vector<std::string>& a) override {
        if(!requireLogin(a_))return; if(a.size()<4){std::cout<<"Usage: "<<usage()<<"\n";return;}
        t_.addIncome(userId(a_),std::stod(a[0]),a[1],a[2],a[3],{},a.size()>4?a[4]:"");
        std::cout<<"Income recorded.\n";
    }
};

// ══════════════════════════════════════════════════════════════════════
// 10. ADD-EXPENSE
// ══════════════════════════════════════════════════════════════════════

class ExpenseCmd : public finance::commands::ICommand {
    AuthenticationService& a_; TransactionService& t_;
public:
    ExpenseCmd(AuthenticationService& a, TransactionService& t) : a_(a), t_(t) {}
    std::string name() const override { return "add-expense"; }
    std::string description() const override { return "Record an expense"; }
    std::string usage() const override { return "add-expense <amount> <desc> <category-id> <account-id>"; }
    void execute(const std::vector<std::string>& a) override {
        if(!requireLogin(a_))return; if(a.size()<4){std::cout<<"Usage: "<<usage()<<"\n";return;}
        t_.addExpense(userId(a_),std::stod(a[0]),a[1],a[2],a[3]);
        std::cout<<"Expense recorded.\n";
    }
};

// ══════════════════════════════════════════════════════════════════════
// 11. TRANSFER
// ══════════════════════════════════════════════════════════════════════

class TransferCmd : public finance::commands::ICommand {
    AuthenticationService& a_; TransactionService& t_;
public:
    TransferCmd(AuthenticationService& a, TransactionService& t) : a_(a), t_(t) {}
    std::string name() const override { return "transfer"; }
    std::string description() const override { return "Transfer between accounts"; }
    std::string usage() const override { return "transfer <amount> <desc> <from-account> <to-account>"; }
    void execute(const std::vector<std::string>& a) override {
        if(!requireLogin(a_))return; if(a.size()<4){std::cout<<"Usage: "<<usage()<<"\n";return;}
        t_.addTransfer(userId(a_),std::stod(a[0]),a[1],a[2],a[3]); std::cout<<"Transfer done.\n";
    }
};

// ══════════════════════════════════════════════════════════════════════
// 12. LIST-TRANSACTIONS
// ══════════════════════════════════════════════════════════════════════

class ListTxnsCmd : public finance::commands::ICommand {
    AuthenticationService& a_; TransactionService& t_;
public:
    ListTxnsCmd(AuthenticationService& a, TransactionService& t) : a_(a), t_(t) {}
    std::string name() const override { return "list-transactions"; }
    std::string description() const override { return "Show your transactions"; }
    std::string usage() const override { return "list-transactions [account-id]"; }
    void execute(const std::vector<std::string>& a) override {
        if(!requireLogin(a_))return;
        auto txns=a.empty()?t_.getTransactionsByUser(userId(a_)):t_.getTransactionsByAccount(a[0]);
        if(txns.empty()){std::cout<<"No transactions.\n";return;}
        std::cout<<"\n── Transactions ──\n";
        for(auto*t:txns)std::cout<<"  ["<<t->date()<<"] "<<std::setw(8)<<std::left<<t->transactionTypeName()<<" "<<std::fixed<<std::setprecision(2)<<t->amount()<<"  "<<t->description()<<"\n";
        std::cout<<"\n";
    }
};

// ══════════════════════════════════════════════════════════════════════
// 13. EDIT-TRANSACTION
// ══════════════════════════════════════════════════════════════════════

class EditTxnCmd : public finance::commands::ICommand {
    TransactionService& t_;
public:
    explicit EditTxnCmd(TransactionService& t) : t_(t) {}
    std::string name() const override { return "edit-transaction"; }
    std::string description() const override { return "Edit a transaction"; }
    std::string usage() const override { return "edit-transaction <id> <amount> <desc> <category-id>"; }
    void execute(const std::vector<std::string>& a) override {
        if(a.size()<4){std::cout<<"Usage: "<<usage()<<"\n";return;}
        t_.editTransaction(a[0],std::stod(a[1]),a[2],a[3]); std::cout<<"Updated.\n";
    }
};

// ══════════════════════════════════════════════════════════════════════
// 14. DELETE-TRANSACTION
// ══════════════════════════════════════════════════════════════════════

class DeleteTxnCmd : public finance::commands::ICommand {
    TransactionService& t_;
public:
    explicit DeleteTxnCmd(TransactionService& t) : t_(t) {}
    std::string name() const override { return "delete-transaction"; }
    std::string description() const override { return "Delete a transaction (undoable)"; }
    std::string usage() const override { return "delete-transaction <id>"; }
    void execute(const std::vector<std::string>& a) override {
        if(a.empty()){std::cout<<"Usage: "<<usage()<<"\n";return;}
        t_.deleteTransaction(a[0]); std::cout<<"Deleted. Use 'undo' to restore.\n";
    }
};

// ══════════════════════════════════════════════════════════════════════
// 15. UNDO
// ══════════════════════════════════════════════════════════════════════

class UndoCmd : public finance::commands::ICommand {
    TransactionService& t_;
public:
    explicit UndoCmd(TransactionService& t) : t_(t) {}
    std::string name() const override { return "undo"; }
    std::string description() const override { return "Undo last delete"; }
    std::string usage() const override { return "undo"; }
    void execute(const std::vector<std::string>&) override {
        std::cout<<(t_.undoDelete()?"Undone.\n":"Nothing to undo.\n");
    }
};

// ══════════════════════════════════════════════════════════════════════
// 16. SET-BUDGET
// ══════════════════════════════════════════════════════════════════════

class SetBudgetCmd : public finance::commands::ICommand {
    AuthenticationService& a_; BudgetService& b_;
public:
    SetBudgetCmd(AuthenticationService& a, BudgetService& b) : a_(a), b_(b) {}
    std::string name() const override { return "set-budget"; }
    std::string description() const override { return "Set a monthly budget"; }
    std::string usage() const override { return "set-budget <category-id> <limit> [year] [month]"; }
    void execute(const std::vector<std::string>& a) override {
        if(!requireLogin(a_))return; if(a.size()<2){std::cout<<"Usage: "<<usage()<<"\n";return;}
        std::time_t t=std::time(nullptr); std::tm*now=std::localtime(&t);
        int y=a.size()>2?std::stoi(a[2]):(now->tm_year+1900);
        int m=a.size()>3?std::stoi(a[3]):(now->tm_mon+1);
        b_.setBudget(userId(a_),a[0],y,m,std::stod(a[1]));
        std::cout<<"Budget set.\n";
    }
};

// ══════════════════════════════════════════════════════════════════════
// 17. SHOW-BUDGET
// ══════════════════════════════════════════════════════════════════════

class ShowBudgetCmd : public finance::commands::ICommand {
    AuthenticationService& a_; BudgetService& b_;
public:
    ShowBudgetCmd(AuthenticationService& a, BudgetService& b) : a_(a), b_(b) {}
    std::string name() const override { return "show-budget"; }
    std::string description() const override { return "Show budgets for a month"; }
    std::string usage() const override { return "show-budget [year] [month]"; }
    void execute(const std::vector<std::string>& a) override {
        if(!requireLogin(a_))return;
        std::time_t t=std::time(nullptr); std::tm*now=std::localtime(&t);
        int y=a.size()>0?std::stoi(a[0]):(now->tm_year+1900);
        int m=a.size()>1?std::stoi(a[1]):(now->tm_mon+1);
        auto budgets=b_.getBudgets(userId(a_),y,m);
        if(budgets.empty()){std::cout<<"No budgets for "<<DateUtils::monthName(m)<<" "<<y<<"\n";return;}
        std::cout<<"\n── Budgets: "<<DateUtils::monthName(m)<<" "<<y<<" ──\n";
        for(auto&b:budgets)std::cout<<"  Category:"<<b.categoryId()<<" Budget:"<<std::fixed<<std::setprecision(2)<<b.limitAmount()<<" Spent:"<<b.spentAmount()<<" ("<<(int)b.percentageUsed()<<"%)\n";
    }
};

// ══════════════════════════════════════════════════════════════════════
// 18. CREATE-GOAL
// ══════════════════════════════════════════════════════════════════════

class CreateGoalCmd : public finance::commands::ICommand {
    AuthenticationService& a_; GoalService& g_;
public:
    CreateGoalCmd(AuthenticationService& a, GoalService& g) : a_(a), g_(g) {}
    std::string name() const override { return "create-goal"; }
    std::string description() const override { return "Create a savings goal"; }
    std::string usage() const override { return "create-goal <name> <target> [deadline]"; }
    void execute(const std::vector<std::string>& a) override {
        if(!requireLogin(a_))return; if(a.size()<2){std::cout<<"Usage: "<<usage()<<"\n";return;}
        auto g=g_.createGoal(userId(a_),a[0],std::stod(a[1]),a.size()>2?a[2]:"");
        std::cout<<"Goal '"<<g.name()<<"' created. ID: "<<g.id()<<"\n";
    }
};

// ══════════════════════════════════════════════════════════════════════
// 19. CONTRIBUTE-GOAL
// ══════════════════════════════════════════════════════════════════════

class ContributeGoalCmd : public finance::commands::ICommand {
    GoalService& g_;
public:
    explicit ContributeGoalCmd(GoalService& g) : g_(g) {}
    std::string name() const override { return "contribute-goal"; }
    std::string description() const override { return "Contribute to a goal"; }
    std::string usage() const override { return "contribute-goal <goal-id> <amount>"; }
    void execute(const std::vector<std::string>& a) override {
        if(a.size()<2){std::cout<<"Usage: "<<usage()<<"\n";return;}
        g_.contribute(a[0],std::stod(a[1])); std::cout<<"Contributed.\n";
    }
};

// ══════════════════════════════════════════════════════════════════════
// 20. SHOW-GOALS
// ══════════════════════════════════════════════════════════════════════

class ShowGoalsCmd : public finance::commands::ICommand {
    AuthenticationService& a_; GoalService& g_;
public:
    ShowGoalsCmd(AuthenticationService& a, GoalService& g) : a_(a), g_(g) {}
    std::string name() const override { return "show-goals"; }
    std::string description() const override { return "Show your goals"; }
    std::string usage() const override { return "show-goals"; }
    void execute(const std::vector<std::string>&) override {
        if(!requireLogin(a_))return;
        auto goals=g_.getGoals(userId(a_)); if(goals.empty()){std::cout<<"No goals.\n";return;}
        std::cout<<"\n── Goals ──\n";
        for(auto&g:goals){int p=(int)g.progressPercentage();
            std::cout<<"  "<<g.name()<<" Target:"<<std::fixed<<std::setprecision(2)<<g.targetAmount()<<" Current:"<<g.currentAmount()<<" "<<p<<"%\n";}
    }
};

// ══════════════════════════════════════════════════════════════════════
// 21. SHOW-NOTIFICATIONS
// ══════════════════════════════════════════════════════════════════════

class NotifsCmd : public finance::commands::ICommand {
    AuthenticationService& a_; NotificationService& n_;
public:
    NotifsCmd(AuthenticationService& a, NotificationService& n) : a_(a), n_(n) {}
    std::string name() const override { return "notifications"; }
    std::string description() const override { return "Show notifications"; }
    std::string usage() const override { return "notifications"; }
    void execute(const std::vector<std::string>&) override {
        if(!requireLogin(a_))return;
        auto ns=n_.getNotifications(userId(a_)); if(ns.empty()){std::cout<<"No notifications.\n";return;}
        std::cout<<"\n── Notifications ──\n";
        for(auto&n:ns)std::cout<<"  ["<<n.createdAt()<<"] "<<(n.isRead()?" ":"●")<<" "<<n.message()<<"\n";
        n_.markAllAsRead(userId(a_));
    }
};

// ══════════════════════════════════════════════════════════════════════
// 22. SEARCH
// ══════════════════════════════════════════════════════════════════════

class SearchCmd : public finance::commands::ICommand {
    AuthenticationService& a_; SearchEngine& s_;
public:
    SearchCmd(AuthenticationService& a, SearchEngine& s) : a_(a), s_(s) {}
    std::string name() const override { return "search"; }
    std::string description() const override { return "Search transactions"; }
    std::string usage() const override { return "search [from] [to] [min] [max] [desc]"; }
    void execute(const std::vector<std::string>& a) override {
        if(!requireLogin(a_))return;
        SearchEngine::Criteria c;
        if(a.size()>0&&a[0]!="*")c.dateFrom=a[0]; if(a.size()>1&&a[1]!="*")c.dateTo=a[1];
        if(a.size()>2&&a[2]!="*")c.amountMin=std::stod(a[2]); if(a.size()>3&&a[3]!="*")c.amountMax=std::stod(a[3]);
        if(a.size()>4&&a[4]!="*")c.description=a[4];
        auto r=s_.search(userId(a_),c); if(r.empty()){std::cout<<"No matches.\n";return;}
        std::cout<<"\n── Results ("<<r.size()<<") ──\n";
        for(auto*t:r)std::cout<<"  ["<<t->date()<<"] "<<t->transactionTypeName()<<" "<<std::fixed<<std::setprecision(2)<<t->amount()<<" "<<t->description()<<"\n";
    }
};

// ══════════════════════════════════════════════════════════════════════
// 23. GENERATE-REPORT
// ══════════════════════════════════════════════════════════════════════

class GenReportCmd : public finance::commands::ICommand {
    AuthenticationService& a_; ReportGenerator& r_;
public:
    GenReportCmd(AuthenticationService& a, ReportGenerator& r) : a_(a), r_(r) {}
    std::string name() const override { return "report"; }
    std::string description() const override { return "Generate report: daily/weekly/monthly/yearly"; }
    std::string usage() const override { return "report <daily|weekly|monthly|yearly> [year] [month]"; }
    void execute(const std::vector<std::string>& a) override {
        if(!requireLogin(a_))return; if(a.empty()){std::cout<<"Usage: "<<usage()<<"\n";return;}
        std::time_t t=std::time(nullptr); std::tm*now=std::localtime(&t);
        int y=a.size()>1?std::stoi(a[1]):(now->tm_year+1900);
        int m=a.size()>2?std::stoi(a[2]):(now->tm_mon+1);
        ReportGenerator::Report rep;
        if(a[0]=="daily")rep=r_.generateDaily(userId(a_));
        else if(a[0]=="weekly")rep=r_.generateWeekly(userId(a_));
        else if(a[0]=="monthly")rep=r_.generateMonthly(userId(a_),y,m);
        else if(a[0]=="yearly")rep=r_.generateYearly(userId(a_),y);
        else{std::cout<<"Invalid period.\n";return;}
        std::cout<<"\n"<<r_.exportToTxt(rep)<<"\n";
    }
};

// ══════════════════════════════════════════════════════════════════════
// 24. EXPORT-REPORT
// ══════════════════════════════════════════════════════════════════════

class ExportReportCmd : public finance::commands::ICommand {
    AuthenticationService& a_; ReportGenerator& r_;
public:
    ExportReportCmd(AuthenticationService& a, ReportGenerator& r) : a_(a), r_(r) {}
    std::string name() const override { return "export"; }
    std::string description() const override { return "Export report to file: export <csv|json|txt> <daily|weekly|monthly|yearly> <file> [year] [month]"; }
    std::string usage() const override { return "export <csv|json|txt> <period> <file> [year] [month]"; }
    void execute(const std::vector<std::string>& a) override {
        if(!requireLogin(a_))return; if(a.size()<3){std::cout<<"Usage: "<<usage()<<"\n";return;}
        std::time_t t=std::time(nullptr); std::tm*now=std::localtime(&t);
        int y=a.size()>3?std::stoi(a[3]):(now->tm_year+1900);
        int m=a.size()>4?std::stoi(a[4]):(now->tm_mon+1);
        ReportGenerator::Report rep;
        if(a[1]=="daily")rep=r_.generateDaily(userId(a_));
        else if(a[1]=="weekly")rep=r_.generateWeekly(userId(a_));
        else if(a[1]=="monthly")rep=r_.generateMonthly(userId(a_),y,m);
        else if(a[1]=="yearly")rep=r_.generateYearly(userId(a_),y);
        else{std::cout<<"Invalid period.\n";return;}
        std::string c; if(a[0]=="csv")c=r_.exportToCsv(rep); else if(a[0]=="json")c=r_.exportToJson(rep); else if(a[0]=="txt")c=r_.exportToTxt(rep); else{std::cout<<"Invalid format.\n";return;}
        r_.saveToFile(c,a[2]); std::cout<<"Exported to "<<a[2]<<"\n";
    }
};

// ══════════════════════════════════════════════════════════════════════
// 25. PROCESS-RECURRING
// ══════════════════════════════════════════════════════════════════════

class RecurringCmd : public finance::commands::ICommand {
    TransactionService& t_;
public:
    explicit RecurringCmd(TransactionService& t) : t_(t) {}
    std::string name() const override { return "process-recurring"; }
    std::string description() const override { return "Generate due recurring transactions"; }
    std::string usage() const override { return "process-recurring"; }
    void execute(const std::vector<std::string>&) override {
        std::cout<<"Generated "<<t_.processRecurring()<<" recurring transaction(s).\n";
    }
};

// ══════════════════════════════════════════════════════════════════════
// Registration
// ══════════════════════════════════════════════════════════════════════

namespace finance {

void FinanceManager::registerCommands() {
    if(!authService_)return;
    using namespace commands;

    parser_.registerCommand(std::make_unique<RegisterCmd>(*authService_));
    parser_.registerCommand(std::make_unique<LoginCmd>(*authService_));
    parser_.registerCommand(std::make_unique<LogoutCmd>(*authService_));
    parser_.registerCommand(std::make_unique<ChangePwCmd>(*authService_));

    if(accountService_){
        parser_.registerCommand(std::make_unique<AddAcctCmd>(*authService_,*accountService_));
        parser_.registerCommand(std::make_unique<ListAcctsCmd>(*authService_,*accountService_));
        parser_.registerCommand(std::make_unique<FreezeAcctCmd>(*accountService_));
        parser_.registerCommand(std::make_unique<CloseAcctCmd>(*accountService_));
    }
    if(transactionService_){
        parser_.registerCommand(std::make_unique<IncomeCmd>(*authService_,*transactionService_));
        parser_.registerCommand(std::make_unique<ExpenseCmd>(*authService_,*transactionService_));
        parser_.registerCommand(std::make_unique<TransferCmd>(*authService_,*transactionService_));
        parser_.registerCommand(std::make_unique<ListTxnsCmd>(*authService_,*transactionService_));
        parser_.registerCommand(std::make_unique<EditTxnCmd>(*transactionService_));
        parser_.registerCommand(std::make_unique<DeleteTxnCmd>(*transactionService_));
        parser_.registerCommand(std::make_unique<UndoCmd>(*transactionService_));
        parser_.registerCommand(std::make_unique<RecurringCmd>(*transactionService_));
    }
    if(budgetService_){
        parser_.registerCommand(std::make_unique<SetBudgetCmd>(*authService_,*budgetService_));
        parser_.registerCommand(std::make_unique<ShowBudgetCmd>(*authService_,*budgetService_));
    }
    if(goalService_){
        parser_.registerCommand(std::make_unique<CreateGoalCmd>(*authService_,*goalService_));
        parser_.registerCommand(std::make_unique<ContributeGoalCmd>(*goalService_));
        parser_.registerCommand(std::make_unique<ShowGoalsCmd>(*authService_,*goalService_));
    }
    if(notificationService_){
        parser_.registerCommand(std::make_unique<NotifsCmd>(*authService_,*notificationService_));
    }
    if(searchEngine_){
        parser_.registerCommand(std::make_unique<SearchCmd>(*authService_,*searchEngine_));
    }
    if(reportGenerator_){
        parser_.registerCommand(std::make_unique<GenReportCmd>(*authService_,*reportGenerator_));
        parser_.registerCommand(std::make_unique<ExportReportCmd>(*authService_,*reportGenerator_));
    }
}

}  // namespace finance
