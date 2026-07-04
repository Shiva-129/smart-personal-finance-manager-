# ── Finance Library (static) ──────────────────────────────────────────
# Source files listed explicitly — consolidated into 6 implementation files.
set(FINANCE_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/Utils.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Models.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Storage.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Services.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/CommandHandlers.cpp
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

# ── Executable ───────────────────────────────────────────────────────
add_executable(finance_manager main.cpp)

target_link_libraries(finance_manager
    PRIVATE
        finance_lib
)

target_include_directories(finance_manager
    PRIVATE
        ${CMAKE_SOURCE_DIR}/include
)
