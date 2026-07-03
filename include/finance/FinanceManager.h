#ifndef FINANCE_FINANCEMANAGER_H
#define FINANCE_FINANCEMANAGER_H

#include "finance/Commands.h"
#include "finance/Services.h"

#include <memory>

namespace finance {

class FinanceManager {
public:
    FinanceManager();
    ~FinanceManager();
    void initialize();
    void run();

private:
    void registerCommands();

    storage::StorageManager              storage_;
    commands::CommandParser              parser_;

    std::unique_ptr<services::AuthenticationService> authService_;
    std::unique_ptr<services::AccountService>        accountService_;
    std::unique_ptr<services::BudgetService>         budgetService_;
    std::unique_ptr<services::GoalService>           goalService_;
    std::unique_ptr<services::NotificationService>   notificationService_;
    std::unique_ptr<services::TransactionService>    transactionService_;
    std::unique_ptr<services::ReportGenerator>       reportGenerator_;
    std::unique_ptr<services::SearchEngine>          searchEngine_;
};

}  // namespace finance

#endif  // FINANCE_FINANCEMANAGER_H
