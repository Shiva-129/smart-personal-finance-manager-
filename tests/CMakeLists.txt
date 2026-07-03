# ── Collect all test source files ─────────────────────────────────────
file(GLOB_RECURSE TEST_SOURCES
    "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"
)

# ── Test Executable ───────────────────────────────────────────────────
add_executable(finance_tests ${TEST_SOURCES})

target_link_libraries(finance_tests
    PRIVATE
        finance_lib
        GTest::gtest_main
        GTest::gmock_main
)

target_include_directories(finance_tests
    PRIVATE
        ${CMAKE_SOURCE_DIR}/include
)

# ── Register tests with CTest ─────────────────────────────────────────
include(GoogleTest)
gtest_add_tests(
    TARGET      finance_tests
    SOURCES     ${TEST_SOURCES}
    TEST_LIST   finance_test_list
)
set_tests_properties(${finance_test_list} PROPERTIES TIMEOUT 10)
