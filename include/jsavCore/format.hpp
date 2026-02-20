/**
 * @file format.hpp
 * @brief A set of macros for convenient use of the fmt library.
 *
 * @details This file provides macros for easy integration with the fmt library,
 * a modern C++ formatting library. It includes macros for formatting strings,
 * pointers, and joining containers with delimiters.
 *
 * The macros provided here wrap around fmt::format, fmt::ptr, and fmt::join
 * to provide a more concise syntax for common formatting operations.
 *
 * @note This file requires the fmt library to be available as a dependency.
 * @note FMT_PTR and FMT_JOIN are only compatible with FORMAT(), not FORMATST(),
 *       as std::format does not provide equivalents to fmt::ptr and fmt::join.
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
 *
 * This macro wraps the fmt::format function for convenient string formatting.
 * It provides a type-safe alternative to printf-style formatting with support
 * for custom types, named arguments, and modern C++ features.
 *
 * @param ... The format string and arguments.
 * @return The formatted std::string.
 *
 * @par Example:
 * @code{.cpp}
 * std::string result = FORMAT("Hello, {}! You have {} messages.", name, count);
 * @endcode
 *
 * @see FORMATST
 * @see FMT_PTR
 * @see FMT_JOIN
 */
#define FORMAT(...) fmt::format(__VA_ARGS__)

#ifdef __cpp_lib_format
/**
 * @def FORMATST(...)
 * @brief Macro for formatting strings using std::format.
 *
 * This macro wraps the std::format function for convenient string formatting
 * when the C++20 std::format feature is available. It provides the same
 * functionality as FORMAT() but uses the standard library implementation.
 *
 * @param ... The format string and arguments.
 * @return The formatted std::string.
 *
 * @warning Custom fmt::formatter specializations are not compatible with std::format.
 *          Behavior may differ slightly from the fmt library fallback.
 *
 * @par Example:
 * @code{.cpp}
 * std::string result = FORMATST("Value: {}", 42);
 * @endcode
 *
 * @see FORMAT
 */
#define FORMATST(...) std::format(__VA_ARGS__)
#else
/**
 * @def FORMATST(...)
 * @brief Macro for formatting strings using the fmt library instead of std::format.
 *
 * This macro wraps the FORMAT() function (which uses fmt::format) when
 * std::format is not available. It provides a consistent interface
 * regardless of whether std::format is supported.
 *
 * @param ... The format string and arguments.
 * @return The formatted std::string.
 *
 * @par Example:
 * @code{.cpp}
 * std::string result = FORMATST("Value: {}", 42);
 * @endcode
 *
 * @see FORMAT
 */
#define FORMATST(...) FORMAT(__VA_ARGS__)
#endif

/**
 * @def FMT_PTR(ptr)
 * @brief Macro for formatting pointers using the fmt library.
 *
 * This macro wraps the fmt::ptr function for formatting pointers.
 * It produces a hexadecimal representation of the pointer address.
 *
 * @param ptr The pointer to be formatted.
 * @return A wrapper object suitable for use with FORMAT() to produce
 *         a formatted pointer string.
 *
 * @note FMT_PTR is only compatible with FORMAT(), not FORMATST(),
 *       as std::format does not provide an equivalent to fmt::ptr.
 *
 * @par Example:
 * @code{.cpp}
 * int value = 42;
 * std::string result = FORMAT("Address: {}", FMT_PTR(&value));
 * // Output: "Address: 0x7fff5fbff6ac" (example)
 * @endcode
 */
#define FMT_PTR(ptr) fmt::ptr(ptr)

/**
 * @def FMT_JOIN(container, delimiter)
 * @brief Macro for joining container elements with a delimiter using the fmt library.
 *
 * This macro wraps the fmt::join function for joining container elements
 * with a specified delimiter. It is useful for formatting ranges, vectors,
 * lists, and other iterable containers.
 *
 * @param container The container to be joined.
 * @param delimiter The delimiter to be used between elements.
 * @return A view object suitable for use with FORMAT() to produce a joined string.
 *
 * @note FMT_JOIN is only compatible with FORMAT(), not FORMATST(),
 *       as std::format does not provide an equivalent to fmt::join.
 *
 * @par Example:
 * @code{.cpp}
 * std::vector<int> numbers = {1, 2, 3, 4, 5};
 * std::string result = FORMAT("Numbers: {}", FMT_JOIN(numbers, ", "));
 * // Output: "Numbers: 1, 2, 3, 4, 5"
 * @endcode
 */
#define FMT_JOIN(container, delimiter) fmt::join(container, delimiter)
// NOLINTEND(*-include-cleaner, *-macro-usage)
