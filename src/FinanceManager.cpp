#include "finance/FinanceManager.h"
#include "finance/Utils.h"

#include <iostream>
#include <string>

namespace finance {

FinanceManager::FinanceManager() : storage_("data") {}
FinanceManager::~FinanceManager() = default;

void FinanceManager::initialize() {
    auto& log = utils::Logger::instance();
    storage_.initialize();

    auto& settings = utils::Settings::instance();
    settings.load();
    log.setLevel(static_cast<utils::LogLevel>(settings.logLevel()));
    if (!settings.logFile().empty()) log.setLogFile(settings.logFile());

    log.info("Finance Manager initializing...");

    notificationService_ = std::make_unique<services::NotificationService>(storage_.notifications());
    authService_         = std::make_unique<services::AuthenticationService>(storage_.users());
    accountService_      = std::make_unique<services::AccountService>(storage_);
    budgetService_       = std::make_unique<services::BudgetService>(storage_, *notificationService_);
    goalService_         = std::make_unique<services::GoalService>(storage_.goals(), *notificationService_);
    transactionService_  = std::make_unique<services::TransactionService>(
        storage_, *accountService_, *budgetService_, *goalService_, *notificationService_);
    reportGenerator_     = std::make_unique<services::ReportGenerator>(storage_);
    searchEngine_        = std::make_unique<services::SearchEngine>(storage_);

    registerCommands();
    log.info("Finance Manager ready.");
}

void FinanceManager::run() {
    std::cout << "\n  ╔══════════════════════════════════════════╗\n"
              << "  ║   Smart Personal Finance & Expense Mgr   ║\n"
              << "  ║           Version 1.0.0                  ║\n"
              << "  ╚══════════════════════════════════════════╝\n"
              << "  Type 'help' for commands.\n" << std::endl;

    std::string input;
    while (true) {
        std::cout << (authService_ && authService_->isLoggedIn()
                      ? authService_->currentUser()->username() + "> "
                      : "> ");
        if (!std::getline(std::cin, input)) { std::cout << std::endl; break; }
        if (!parser_.execute(input)) break;
        storage_.flushAll();
    }
    std::cout << "Goodbye!" << std::endl;
}

}  // namespace finance
