cmake_minimum_required(VERSION 3.16)
project(FinanceManager VERSION 1.0.0 LANGUAGES CXX)

# ── C++ Standard ───────────────────────────────────────────────────────
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# ── Build Options ──────────────────────────────────────────────────────
option(BUILD_TESTS "Build unit tests" ON)

# ── Dependencies (via FetchContent) ────────────────────────────────────
include(FetchContent)

# nlohmann/json — header-only JSON library
FetchContent_Declare(
    nlohmann_json
    GIT_REPOSITORY  https://github.com/nlohmann/json.git
    GIT_TAG         v3.11.3
    GIT_SHALLOW     TRUE
)
FetchContent_MakeAvailable(nlohmann_json)

# GoogleTest — unit testing framework
if(BUILD_TESTS)
    FetchContent_Declare(
        googletest
        GIT_REPOSITORY  https://github.com/google/googletest.git
        GIT_TAG         v1.15.2
        GIT_SHALLOW     TRUE
    )
    set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
    FetchContent_MakeAvailable(googletest)
    enable_testing()
endif()

# ── Subdirectories ────────────────────────────────────────────────────
add_subdirectory(src)

if(BUILD_TESTS)
    add_subdirectory(tests)
endif()
