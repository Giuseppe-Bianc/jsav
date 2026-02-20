/**
 * @file format.hpp
 * @brief A set of macros for convenient use of the fmt library.
 * This file provides macros for easy integration with the fmt library, a modern C++ formatting library.
 * It includes macros for formatting strings, pointers, and joining containers with delimiters.
 * @note This file requires the fmt library to be available as a dependency (headers are included here).
 */
#pragma once

// NOLINTBEGIN(*-include-cleaner, *-macro-usage)
#ifdef __cpp_lib_format
#include <format>
#endif
DISABLE_CLANG_WARNINGS_PUSH("-Wunused-result")
DISABLE_GCC_WARNINGS_PUSH("-Wstringop-overflow")
#include <fmt/core.h>
#include <fmt/format.h>
#include <fmt/ranges.h>
#include <fmt/std.h>
DISABLE_CLANG_WARNINGS_POP()
DISABLE_GCC_WARNINGS_POP()

/**
 * @def FORMAT(...)
 * @brief Macro for formatting strings using the fmt library.
 * This macro wraps the fmt::format function for convenient string formatting.
 * @param ... The format string and arguments.
 * @return The formatted string.
 */
#define FORMAT(...) fmt::format(__VA_ARGS__)

#ifdef __cpp_lib_format
/**
 * @def FORMATST(...)
 * @brief Macro for formatting strings using the std::format.
 * This macro wraps the std::format function for convenient string formatting.
 * @param ... The format string and arguments.
 * @return The formatted string.
 * @warning Custom fmt::formatter specializations are not compatible with std::format.
 *          Behavior may differ slightly from the fmt library fallback.
 */
#define FORMATST(...) std::format(__VA_ARGS__)
#else
/**
 * @def FORMATST(...)
 * @brief Macro for formatting strings using the fmt library instead of std::format.
 * This macro wraps the format function for convenient string formatting.
 * @param ... The format string and arguments.
 * @return The formatted string.
 */
#define FORMATST(...) FORMAT(__VA_ARGS__)
#endif

/**
 * @note FMT_PTR and FMT_JOIN are only compatible with FORMAT(), not FORMATST(),
 *       as std::format does not provide equivalents to fmt::ptr and fmt::join.
 */

/**
 * @def FMT_PTR(ptr)
 * @brief Macro for formatting pointers using the fmt library.
 * This macro wraps the fmt::ptr function for formatting pointers.
 * @param ptr The pointer to be formatted.
 * @return A wrapper object suitable for use with FORMAT() to produce a formatted pointer string.
 */
#define FMT_PTR(ptr) fmt::ptr(ptr)

/**
 * @def FMT_JOIN(container, delimiter)
 * @brief Macro for joining container elements with a delimiter using the fmt library.
 * This macro wraps the fmt::join function for joining container elements with a specified delimiter.
 * @param container The container to be joined.
 * @param delimiter The delimiter to be used between elements.
 * @return A view object suitable for use with FORMAT() to produce a joined string.
 */
#define FMT_JOIN(container, delimiter) fmt::join(container, delimiter)
// NOLINTEND(*-include-cleaner, *-macro-usage)