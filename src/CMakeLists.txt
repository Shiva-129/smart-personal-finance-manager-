# ── Finance Library (static) ──────────────────────────────────────────
# Source files are listed explicitly here. As new modules are added in
# each phase, their source files are appended to FINANCE_SOURCES.
set(FINANCE_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/finance_lib.cpp

    # ── Phase 1 — Utils ────────────────────────────────────────
    ${CMAKE_CURRENT_SOURCE_DIR}/utils/Logger.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/utils/UUID.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/utils/DateUtils.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/utils/Settings.cpp

    # ── Phase 1 — Models ───────────────────────────────────────
    ${CMAKE_CURRENT_SOURCE_DIR}/models/User.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/models/Category.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/models/Account.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/models/Transaction.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/models/Budget.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/models/Goal.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/models/Notification.cpp

    # ── Phase 2 — Storage ───────────────────────────────────────
    ${CMAKE_CURRENT_SOURCE_DIR}/storage/StorageManager.cpp

    # ── Phase 3 — Services ──────────────────────────────────────
    ${CMAKE_CURRENT_SOURCE_DIR}/services/AccountService.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/services/AuthenticationService.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/services/BudgetService.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/services/GoalService.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/services/NotificationService.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/services/ReportGenerator.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/services/SearchEngine.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/services/TransactionService.cpp

    # ── Phase 4 — Commands ──────────────────────────────────────
    ${CMAKE_CURRENT_SOURCE_DIR}/commands/CommandParser.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/commands/CommandRegistry.cpp

    # ── Phase 4 — Orchestrator ────────────────────────────────
    ${CMAKE_CURRENT_SOURCE_DIR}/FinanceManager.cpp
)

add_library(finance_lib STATIC ${FINANCE_SOURCES})

target_include_directories(finance_lib
    PUBLIC
        ${CMAKE_SOURCE_DIR}/include
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(finance_lib
    PUBLIC
        nlohmann_json::nlohmann_json
)

# ── Executable ─────────────────────────────────────────────────────────
add_executable(finance_manager
    main.cpp
)

target_link_libraries(finance_manager
    PRIVATE
        finance_lib
)

target_include_directories(finance_manager
    PRIVATE
        ${CMAKE_SOURCE_DIR}/include
)
