/**
 * @file Log.hpp
 * @brief Header file for logging functionality using the SPDLOG library.
 *
 * @details This file provides macros and initialization functions for logging
 * using the SPDLOG library. The logging macros cover various logging levels
 * including trace, debug, info, warn, error, and critical.
 *
 * The initialization macro sets up the logging configuration with default settings,
 * including a console logger with colored output and a custom log pattern.
 *
 * @defgroup Logging Macros
 * @brief Macros for logging messages at various levels.
 * @{
 *
 * @section Overview
 * This module provides a set of macros for logging messages at different severity levels.
 * The macros wrap around the corresponding functions provided by the SPDLOG library,
 * making it easy to integrate logging into your application.
 *
 * @section Logging Levels
 * - LTRACE: Logs trace messages, which are the most detailed and typically used for debugging.
 * - LDEBUG: Logs debug messages, useful during development and testing.
 * - LINFO: Logs informational messages about the application's state.
 * - LWARN: Logs warning messages that indicate potential issues.
 * - LERROR: Logs error messages for non-critical errors.
 * - LCRITICAL: Logs critical messages for severe errors requiring immediate attention.
 *
 * @section Initialization
 * The logging system must be initialized before any logging can occur.
 * Use the INIT_LOG() macro to set up the logging configuration with default settings.
 * This includes setting a default pattern for log messages and creating a console logger.
 *
 * @par Example:
 * @code{.cpp}
 * INIT_LOG();
 * LINFO("Logging system initialized.");
 * @endcode
 * }.
 */
#pragma once
// NOLINTBEGIN(*-include-cleaner)

// clang-format off
#include "disableWarn.hpp"
#include <iostream>
#include "format.hpp"
// clang-format on

/** \cond */
DISABLE_WARNINGS_PUSH(
    4005 4201 4459 4514 4625 4626 4820 6244 6285 6385 6386 26409 26415 26418 26429 26432 26437 26438 26440 26446 26447 26450 26451 26455 26457 26459 26460 26461 26467 26472 26473 26474 26475 26481 26482 26485 26490 26491 26493 26494 26495 26496 26497 26498 26800 26814 26818 26826)
/** \endcond */

/**
 * @brief Sets the active logging level to TRACE.
 *
 * This define configures SPDLOG to accept all logging levels from TRACE and above.
 * It must be defined before including spdlog headers to enable trace-level logging.
 */
#define SPDLOG_ACTIVE_LEVEL SPDLOG_LEVEL_TRACE
DISABLE_CLANG_WARNINGS_PUSH("-Wunused-result")
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>
DISABLE_CLANG_WARNINGS_POP()

DISABLE_WARNINGS_POP()
/** \cond */
#ifndef _MSC_VER
/**
 * @brief Disable GCC warning about uninitialized variables for spdlog.
 */
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wuninitialized"
#include <spdlog/details/null_mutex.h>
#pragma GCC diagnostic pop
#endif
/** \endcond */

/**
 * @brief Macro for logging trace messages using SPDLOG_TRACE.
 *
 * @param ... Variable arguments to be formatted and logged.
 *
 * @details Use this macro to log detailed tracing information for debugging purposes.
 * This macro is a wrapper around the SPDLOG_TRACE macro provided by the spdlog library.
 * Trace messages are the most verbose and are typically enabled only during intensive debugging.
 *
 * @par Example:
 * @code{.cpp}
 * LTRACE("Entering function {} with parameter {}", funcName, param);
 * @endcode
 *
 * @see spdlog::trace
 */
#define LTRACE(...) SPDLOG_TRACE(__VA_ARGS__)

/**
 * @brief Macro for logging debug messages using SPDLOG_DEBUG.
 *
 * @param ... Variable arguments to be formatted and logged.
 *
 * @details Use this macro to log debug information helpful during development and testing.
 * This macro is a wrapper around the SPDLOG_DEBUG macro provided by the spdlog library.
 * Debug messages provide more detail than info messages but less than trace messages.
 *
 * @par Example:
 * @code{.cpp}
 * LDEBUG("Processing item {} of {}", currentIndex, totalCount);
 * @endcode
 *
 * @see spdlog::debug
 */
#define LDEBUG(...) SPDLOG_DEBUG(__VA_ARGS__)

/**
 * @brief Macro for logging informational messages using SPDLOG_INFO.
 *
 * @param ... Variable arguments to be formatted and logged.
 *
 * @details Use this macro to log general information about the application's state.
 * This macro is a wrapper around the SPDLOG_INFO macro provided by the spdlog library.
 * Info messages represent normal operational events that don't require special attention.
 *
 * @par Example:
 * @code{.cpp}
 * LINFO("Application started successfully");
 * @endcode
 *
 * @see spdlog::info
 */
#define LINFO(...) SPDLOG_INFO(__VA_ARGS__)

/**
 * @brief Macro for logging warning messages using SPDLOG_WARN.
 *
 * @param ... Variable arguments to be formatted and logged.
 *
 * @details Use this macro to log non-critical warnings that might indicate potential issues.
 * This macro is a wrapper around the SPDLOG_WARN macro provided by the spdlog library.
 * Warning messages indicate situations that may become problematic if not addressed.
 *
 * @par Example:
 * @code{.cpp}
 * LWARN("Configuration file not found, using defaults");
 * @endcode
 *
 * @see spdlog::warn
 */
#define LWARN(...) SPDLOG_WARN(__VA_ARGS__)

/**
 * @brief Macro for logging error messages using SPDLOG_ERROR.
 *
 * @param ... Variable arguments to be formatted and logged.
 *
 * @details Use this macro to log errors that do not prevent the application from continuing.
 * This macro is a wrapper around the SPDLOG_ERROR macro provided by the spdlog library.
 * Error messages indicate problems that need attention but don't halt execution.
 *
 * @par Example:
 * @code{.cpp}
 * LERROR("Failed to connect to database: {}", errorMessage);
 * @endcode
 *
 * @see spdlog::error
 */
#define LERROR(...) SPDLOG_ERROR(__VA_ARGS__)

/**
 * @brief Macro for logging critical messages using SPDLOG_CRITICAL.
 *
 * @param ... Variable arguments to be formatted and logged.
 *
 * @details Use this macro to log critical errors that require immediate attention.
 * This macro is a wrapper around the SPDLOG_CRITICAL macro provided by the spdlog library.
 * Critical messages indicate severe errors that may prevent the application from continuing.
 *
 * @par Example:
 * @code{.cpp}
 * LCRITICAL("Out of memory, application cannot continue");
 * @endcode
 *
 * @see spdlog::critical
 */
#define LCRITICAL(...) SPDLOG_CRITICAL(__VA_ARGS__)

/**
 * @brief Gets the current timestamp with millisecond precision.
 *
 * @return A std::string containing the current timestamp in the format
 *         "YYYY-MM-DD HH:MM:SS.mmm".
 *
 * @details This function retrieves the current system time and formats it
 *         as a string with millisecond precision. It is used internally
 *         by the error handler to timestamp error messages.
 *
 * @note The function uses std::chrono for time retrieval and formatting.
 *
 * @par Example:
 * @code{.cpp}
 * std::string timestamp = get_current_timestamp();
 * // Output: "2024-01-15 14:30:45.123"
 * @endcode
 */
[[nodiscard]] inline std::string get_current_timestamp() {
    const auto now = ch::floor<ch::milliseconds>(ch::system_clock::now());
    return FORMAT("{:%Y-%m-%d %H:%M:%S}", now);
}

// clang-format off
/**
 * @brief Custom error handler for spdlog internal errors.
 *
 * @param[in] msg The error message from spdlog.
 *
 * @details This function is called when spdlog encounters an internal error.
 *         It outputs detailed error information including:
 *         - Timestamp of the error
 *         - Thread ID where the error occurred
 *         - The error message itself
 *         - A note indicating the error originated from spdlog internals
 *
 * @note The function writes to std::cerr for error output.
 */
inline void my_error_handler(const std::string& msg) {
    fmt::print(stderr,
        "Error occurred:\n  Timestamp: {}\n  Thread ID: {}\n  Message:   {}\n  Note: Error originated within spdlog internals.\n",
        get_current_timestamp(),
        std::this_thread::get_id(),
        msg);
}
// clang-format on

/**
 * @brief Sets up the default logger with console sinks.
 *
 * @details This function configures the default spdlog logger with:
 *          - A stdout sink for trace, debug, and info level messages (colored output)
 *          - A custom log pattern: "[HH:MM:SS level] message"
 *          - Minimum log level set to trace (all messages are logged)
 *
 * @note The logger is created as a shared pointer and set as the default logger.
 * @note The stderr sink is commented out but available for future use.
 *
 * @throws spdlog::spdlog_ex If logger initialization fails.
 *
 * @par Example:
 * @code{.cpp}
 * setup_logger();
 * LINFO("Logger configured");
 * @endcode
 */
inline void setup_logger() {
    std::vector<spdlog::sink_ptr> sinks;

    // Console sink (colored, accepts all log levels starting from trace)
    const auto stdout_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
    stdout_sink->set_level(spdlog::level::trace);  // Log all levels (trace and above)
    // Stderr sink (colored, for warn to critical levels)
    const auto stderr_sink = std::make_shared<spdlog::sinks::stderr_color_sink_mt>();
    stderr_sink->set_level(spdlog::level::warn);  // Log warn and above (warn, error, critical)

    // Add sinks to the logger, ensuring no duplicate output for same log level
    sinks.push_back(stdout_sink);
    // sinks.push_back(stderr_sink);

    // Create logger with the defined sinks
    const auto logger = std::make_shared<spdlog::logger>("main", sinks.begin(), sinks.end());
    logger->set_pattern(R"(%^[%T %l] %v%$)");  // Log pattern
    logger->set_level(spdlog::level::trace);   // Minimum log level (trace)

    // Set this logger as the default logger
    spdlog::set_default_logger(logger);
}

/**
 * @brief Initialize the logging system with default configurations.
 *
 * @details This macro initializes the logging system with a default pattern and
 *         creates a console logger. It performs the following steps:
 *         1. Sets the custom error handler (my_error_handler)
 *         2. Calls setup_logger() to configure the default logger
 *         3. Catches and reports any exceptions during initialization
 *
 * If the initialization fails, it outputs an error message to stderr.
 * The default pattern used is "[HH:MM:SS level] message" with colored output.
 *
 * @pre The spdlog library must be available.
 * @post The default logger is configured and ready for use.
 *
 * @par Example:
 * @code{.cpp}
 * int main() {
 *     INIT_LOG();
 *     LINFO("Application starting...");
 *     // ... application logic ...
 *     return 0;
 * }
 * @endcode
 *
 * @see setup_logger
 * @see spdlog::set_pattern
 * @see spdlog::stdout_color_mt
 * @see spdlog::set_default_logger
 */
#define INIT_LOG()                                                                                                                         \
    do {                                                                                                                                   \
        spdlog::set_error_handler(my_error_handler);                                                                                       \
        try {                                                                                                                              \
            setup_logger();                                                                                                                \
        } catch(const spdlog::spdlog_ex &ex) {                                                                                             \
            std::cerr << "Logger initialization failed: " << ex.what() << '\n';                                                            \
        } catch(const std::exception &e) { std::cerr << "Unhandled exception: " << e.what() << '\n'; } catch(...) {                        \
            std::cerr << "An unknown error occurred. Logger initialization failed.\n";                                                     \
        }                                                                                                                                  \
    } while(0)

/// @}
// NOLINTEND(*-include-cleaner)
