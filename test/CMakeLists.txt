cmake_minimum_required(VERSION 3.15...4.2)

project(CmakeConfigPackageTests
        VERSION ${PROJECT_VERSION}
        LANGUAGES CXX)

# ---- Test as standalone project the exported config package ----

if (PROJECT_IS_TOP_LEVEL OR TEST_INSTALLED_VERSION)
    enable_testing()

    find_package(jsav CONFIG REQUIRED) # for intro, project_options, ...

    if (NOT TARGET jsav_options)
        message(FATAL_ERROR "Required config package not found!")
        return() # be strictly paranoid for Template Janitor github action! CK
    endif ()
endif ()

# ---- Dependencies ----

include(${Catch2_SOURCE_DIR}/extras/Catch.cmake)

# Provide a simple smoke test to make sure that the CLI works and can display a --help message
add_test(NAME cli.has_help COMMAND jsav --help)

# Provide a test to verify that the version being reported from the application
# matches the version given to CMake. This will be important once you package
# your program. Real world shows that this is the kind of simple mistake that is easy
# to make, but also easy to test for.
add_test(NAME cli.version_matches COMMAND jsav --version)
set_tests_properties(cli.version_matches PROPERTIES PASS_REGULAR_EXPRESSION "${PROJECT_VERSION}")

# ---- Unit tests ----

add_executable(tests tests.cpp)
target_link_libraries(
        tests
        PRIVATE jsav::jsav_warnings
        jsav::jsav_options
        jsav::jsav_lib
        Catch2::Catch2WithMain)

jsav_configure_linker(tests)

set_target_properties(tests PROPERTIES CXX_SCAN_FOR_MODULES OFF)
# Enable /bigobj flag for MSVC to handle large test files
target_compile_options(tests PRIVATE $<$<CXX_COMPILER_ID:MSVC>:/bigobj>)
if (WIN32 AND BUILD_SHARED_LIBS)
    add_custom_command(
            TARGET tests
            PRE_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_RUNTIME_DLLS:tests> $<TARGET_FILE_DIR:tests>
            COMMAND_EXPAND_LISTS)
endif ()

catch_discover_tests(
        tests
        TEST_PREFIX
        "unittests."
        REPORTER
        XML
        OUTPUT_DIR
        .
        OUTPUT_PREFIX
        "unittests."
        OUTPUT_SUFFIX
        .xml)

# ---- Constexpr tests ----

add_executable(constexpr_tests constexpr_tests.cpp)
target_link_libraries(
        constexpr_tests
        PRIVATE jsav::jsav_warnings
        jsav::jsav_options
        jsav::jsav_lib
        Catch2::Catch2WithMain)

jsav_configure_linker(constexpr_tests)
set_target_properties(constexpr_tests PROPERTIES CXX_SCAN_FOR_MODULES OFF)

catch_discover_tests(
        constexpr_tests
        TEST_PREFIX
        "constexpr."
        REPORTER
        XML
        OUTPUT_DIR
        .
        OUTPUT_PREFIX
        "constexpr."
        OUTPUT_SUFFIX
        .xml)

add_executable(relaxed_constexpr_tests constexpr_tests.cpp)
target_link_libraries(
        relaxed_constexpr_tests
        PRIVATE jsav::jsav_warnings
        jsav::jsav_options
        jsav::jsav_lib
        Catch2::Catch2WithMain)
target_compile_definitions(relaxed_constexpr_tests PRIVATE -DCATCH_CONFIG_RUNTIME_STATIC_REQUIRE)

jsav_configure_linker(relaxed_constexpr_tests)
set_target_properties(relaxed_constexpr_tests PROPERTIES CXX_SCAN_FOR_MODULES OFF)

catch_discover_tests(
        relaxed_constexpr_tests
        TEST_PREFIX
        "relaxed_constexpr."
        REPORTER
        XML
        OUTPUT_DIR
        .
        OUTPUT_PREFIX
        "relaxed_constexpr."
        OUTPUT_SUFFIX
        .xml)

# ---- Benchmarks ----
#
# Catch2 community guidance:
#   • Do NOT pass the benchmarks executable to catch_discover_tests.
#     CTest enumerates TEST_CASEs, not individual BENCHMARK() calls, so the
#     executable would appear as a single opaque CTest entry.  More importantly,
#     CTest treats any non-zero exit code as a failure; benchmark runs that time
#     out or exceed a sample count produce non-zero exits and trigger false
#     failures in CI.
#   • Benchmark TEST_CASEs are tagged [!benchmark].  Catch2 skips them during
#     a plain `./benchmarks` invocation; they only run when the user explicitly
#     requests the tag:
#
#         ./benchmarks [!benchmark]
#         ./benchmarks [!benchmark] --benchmark-samples 100
#
#   • The target is still built as part of the default build so that compilation
#     errors are caught early, but it is excluded from CTest via EXCLUDE_FROM_ALL
#     on the add_test entry below (the single smoke-test that just prints --help).

add_executable(benchmarks benchmarks.cpp)
target_link_libraries(
        benchmarks
        PRIVATE jsav::jsav_warnings
        jsav::jsav_options
        jsav::jsav_lib
        Catch2::Catch2WithMain)

jsav_configure_linker(benchmarks)
set_target_properties(benchmarks PROPERTIES CXX_SCAN_FOR_MODULES OFF)

if (WIN32 AND BUILD_SHARED_LIBS)
    add_custom_command(
            TARGET benchmarks
            PRE_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_RUNTIME_DLLS:benchmarks> $<TARGET_FILE_DIR:benchmarks>
            COMMAND_EXPAND_LISTS)
endif ()

# Register a single smoke-test so CI can verify the executable at least starts
# without crashing, while the [!benchmark] tag ensures no benchmark body runs.
# EXCLUDE_FROM_ALL keeps this out of the default `ctest` suite.
add_test(NAME benchmarks.smoke COMMAND benchmarks --list-tests)

set_tests_properties(benchmarks.smoke PROPERTIES LABELS "benchmark" EXCLUDE_FROM_ALL TRUE)