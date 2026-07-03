#ifndef FINANCE_FINANCEMANAGER_H
#define FINANCE_FINANCEMANAGER_H

#include "finance/commands/CommandParser.h"
#include "finance/services/AccountService.h"
#include "finance/services/AuthenticationService.h"
#include "finance/services/BudgetService.h"
#include "finance/services/GoalService.h"
#include "finance/services/NotificationService.h"
#include "finance/services/ReportGenerator.h"
#include "finance/services/SearchEngine.h"
#include "finance/services/TransactionService.h"
#include "finance/storage/StorageManager.h"

#include <memory>

namespace finance {

/**
 * @brief Top-level application orchestrator.
 *
 * Initialises the storage layer, creates all services, registers all
 * commands, and runs the interactive REPL loop.
 */
class FinanceManager {
public:
    FinanceManager();
    ~FinanceManager();

    /// Initialise all subsystems.
    void initialize();

    /// Run the main REPL loop until the user exits.
    void run();

private:
    /// Register all command handlers.
    void registerCommands();

    // ── Subsystems (order matters for construction/destruction) ───
    storage::StorageManager              storage_;
    commands::CommandParser              parser_;

    // Services (owned by FinanceManager, created in initialize()).
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
