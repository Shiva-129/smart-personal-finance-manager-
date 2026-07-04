/**
 * @file    main.cpp
 * @brief   Entry point for the Smart Personal Finance & Expense Manager.
 *
 * A production-quality offline personal finance manager built with modern
 * C++17, Clean Architecture, and SOLID principles.
 */

#include "finance/FinanceManager.h"
#include "finance/Utils.h"

int main()
{
    try {
        finance::FinanceManager app;
        app.initialize();
        app.run();
    } catch (const std::exception& e) {
        finance::utils::Logger::instance().error(
            std::string("Fatal error: ") + e.what());
        return 1;
    }

    return 0;
}
