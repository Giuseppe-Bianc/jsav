#pragma once
// NOLINTBEGIN(*-include-cleaner, *-identifier-length)
#include "disableWarn.hpp"
// clang-format off
/** \cond */
DISABLE_WARNINGS_PUSH(
    4005 4201 4459 4514 4625 4626 4820
    6244 6285 6385 6386 26409 26415 26418
    26429 26432 26437 26438 26440 26465
    26446 26447 26450 26451 26455 26457
    26459 26460 26461 26467 26472 26473
    26474 26475 26481 26482 26485 26490
    26491 26493 26494 26495 26496 26497
    26498 26800 26814 26818 26826)
/** \endcond */
#include <any>
#include <algorithm>
#include <array>
#include <atomic>
#include <cassert>
#include <complex>
#include <chrono>
#include <cmath>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <execution>
#include <filesystem>
#include <fstream>
#include <iomanip>
//#include <iostream>
#include <iterator>
#include <initializer_list>
#include <limits>
#include <map>
#include <memory>
#include <memory_resource>
//#include <new>
#include <numbers>
#include <ostream>
#include <optional>
#include <random>
#include <ranges>
//#include <set>
#include <source_location>
#include <sstream>
//#include <stack>
#include <stdexcept>
#include <string>
#include <string_view>
#include <type_traits>
//#include <unordered_set>
#include <utility>
#include <variant>
#include <vector>
// clang-format on
#include "cast/casts.hpp"
// #include "glm_matld.hpp"
// #include "glm_prety_string_cast.hpp"
#include "move.hpp"
// This file will be generated automatically when you run the CMake
// configuration step. It creates a namespace called `Vandior`. You can modify
// the source template at `configured_files/config.hpp.in`.
#include <internal_use_only/config.hpp>
// Restore warning levels.
DISABLE_WARNINGS_POP()
// DISABLE_WARNINGS_PUSH(26476 26446 26482 26497 26472 26440 26447 26490 26481 26429 26493 26438 26455 26432 26496 26485 26819)
// #include <nlohmann/json.hpp>
// DISABLE_WARNINGS_POP()

/**
 * @namespace fs
 * @brief Namespace alias for std::filesystem.
 *
 * Provides convenient access to filesystem operations without the std:: prefix.
 */
namespace fs = std::filesystem;  // NOLINT(*-unused-alias-decls)

/**
 * @namespace ch
 * @brief Namespace alias for std::chrono.
 *
 * Provides convenient access to time-related operations without the std:: prefix.
 */
namespace ch = std::chrono;  // NOLINT(*-unused-alias-decls)

DISABLE_WARNINGS_PUSH(26481)
/**
 * @brief Positive infinity value for long double precision.
 *
 * This constant represents positive infinity for long double type,
 * initialized using std::numeric_limits.
 */
static inline constexpr long double NINFINITY = std::numeric_limits<long double>::infinity();

/**
 * @brief Mathematical constant π (pi) for long double precision.
 *
 * This constant represents the mathematical constant π (pi) for long double type,
 * initialized using std::numbers::pi_v. The value is approximately 3.14159...
 *
 * @note The variable name uses 'PI' despite the NOLINT comment about confusable identifiers.
 */
static inline constexpr long double PI = std::numbers::pi_v<long double>;  // NOLINT(*-confusable-identifiers)

/**
 * @brief Mathematical constant 2π (two pi) for long double precision.
 *
 * This constant represents twice the value of π (pi), approximately 6.28318...
 * Useful for full circle calculations in radians.
 */
static inline constexpr long double TWO_PI = 2 * PI;

/**
 * @brief Mathematical constant π/2 (half pi) for long double precision.
 *
 * This constant represents half the value of π (pi), approximately 1.57079...
 * Useful for quarter circle or right angle calculations in radians.
 */
static inline constexpr long double HALF_PI = PI / 2;

/**
 * @brief Newline character as a C-string literal.
 *
 * Represents the Unix/Linux newline character '\n' as a const char pointer.
 */
static inline constexpr const auto *CNL = "\n";

/**
 * @brief Carriage return character as a C-string literal.
 *
 * Represents the classic Mac OS carriage return '\r' as a const char pointer.
 */
static inline constexpr const auto *CCR = "\r";

/**
 * @brief Newline character constant.
 *
 * Represents the Unix/Linux newline character '\n' as a char constant.
 */
static inline constexpr auto NL = '\n';

/**
 * @brief Carriage return character constant.
 *
 * Represents the classic Mac OS carriage return '\r' as a char constant.
 */
static inline constexpr auto CR = '\r';

/**
 * @brief Period/dot character constant.
 *
 * Represents the period character '.' as a char constant.
 */
static inline constexpr char PNT = '.';

/**
 * @brief Scientific notation exponent character constant.
 *
 * Represents the exponent character 'E' used in scientific notation as a char constant.
 */
static inline constexpr char ECR = 'E';

/**
 * @brief Carriage return followed by newline as a C-string literal.
 *
 * Represents the Windows-style line ending "\r\n" as a const char pointer.
 */
static inline constexpr const auto *CRNL = "\r\n";

/**
 * @brief Horizontal tab character constant.
 *
 * Represents the tab character '\t' as a char constant.
 */
static inline constexpr char CTAB = '\t';

#ifdef _WIN32  // Windows
/**
 * @brief Platform-specific newline sequence.
 *
 * Defines the appropriate newline sequence for the current platform.
 * On Windows, this is set to carriage return followed by newline ("\r\n").
 */
static inline constexpr const auto *NEWL = CRNL;  // Windows
#elif defined macintosh                           // OS 9
/**
 * @brief Platform-specific newline sequence.
 *
 * Defines the appropriate newline sequence for the current platform.
 * On classic Mac OS (OS 9), this is set to carriage return ("\r").
 */
static inline constexpr const auto *NEWL = &CCR;  // Classic Mac OS
#elif defined __unix__                            // Linux and Unix-like systems
/**
 * @brief Platform-specific newline sequence.
 *
 * Defines the appropriate newline sequence for the current platform.
 * On Linux and Unix-like systems, this is set to newline ("\n").
 */
static inline constexpr const auto *NEWL = CNL;  // Linux and Unix
#elif defined __APPLE__                           // macOS
/**
 * @brief Platform-specific newline sequence.
 *
 * Defines the appropriate newline sequence for the current platform.
 * On macOS, this is set to newline ("\n").
 */
static inline constexpr const auto *NEWL = CNL;  // macOS
#elif defined __VMS                               // OpenVMS
/**
 * @brief Platform-specific newline sequence.
 *
 * Defines the appropriate newline sequence for the current platform.
 * On OpenVMS, this is set to carriage return followed by newline ("\r\n").
 */
static inline constexpr const auto *NEWL = CRNL;  // OpenVMS
#elif defined __FreeBSD__                         // FreeBSD
/**
 * @brief Platform-specific newline sequence.
 *
 * Defines the appropriate newline sequence for the current platform.
 * On FreeBSD, this is set to newline ("\n").
 */
static inline constexpr const auto *NEWL = CNL;  // FreeBSD
#else
/**
 * @brief Platform-specific newline sequence (default case).
 *
 * Defines the appropriate newline sequence for the current platform.
 * For unspecified platforms, defaults to newline ("\n").
 */
static inline constexpr const auto *NEWL = CNL;  // Default case
#endif
DISABLE_WARNINGS_POP()

/**
 * @brief Comma as a string view constant.
 *
 * A constexpr string view containing a single comma character.
 */
static inline constexpr std::string_view comma = ",";

/**
 * @brief Colon as a string view constant.
 *
 * A constexpr string view containing a single colon character.
 */
static inline constexpr std::string_view colon = ":";

/**
 * @brief Build folder name constant.
 *
 * A constexpr string view containing the default build folder name "vnbuild".
 * Used for identifying build output directories.
 */
static inline constexpr std::string_view VANDIOR_BUILDFOLDER = "vnbuild";

/**
 * @brief Comma character constant.
 *
 * A constexpr char containing a comma character.
 */
static inline constexpr auto commacr = ',';

/**
 * @brief Colon character constant.
 *
 * A constexpr char containing a colon character.
 */
static inline constexpr auto coloncr = ':';

/**
 * @brief Forward slash character constant.
 *
 * A constexpr char containing a forward slash character, commonly used in paths.
 */
static inline constexpr auto slashcr = '/';

/**
 * @brief Asterisk character constant.
 *
 * A constexpr char containing an asterisk (star) character.
 */
static inline constexpr auto starcr = '*';

/**
 * @brief Underscore character constant.
 *
 * A constexpr char containing an underscore character.
 */
static inline constexpr auto underscore = '_';

/**
 * @brief Zero digit character constant.
 *
 * A constexpr char containing the character '0'.
 */
static inline constexpr auto zerocr = '0';

/**
 * @brief Seven digit character constant.
 *
 * A constexpr char containing the character '7'.
 */
static inline constexpr auto sevencr = '7';

/**
 * @brief Plus sign character constant.
 *
 * A constexpr char containing a plus sign character.
 */
static inline constexpr auto plusscr = '+';

/**
 * @brief Minus sign character constant.
 *
 * A constexpr char containing a minus sign (hyphen) character.
 */
static inline constexpr auto minuscs = '-';

// NOLINTBEGIN(*-macro-usage)
/**
 * @brief Pauses program execution and waits for user input.
 *
 * This macro outputs an informational message prompting the user to press Enter,
 * then waits for user input by calling std::cin.ignore(). It is useful for
 * pausing console applications to allow users to read output before the program exits.
 *
 * @pre The standard input stream (std::cin) must be available and not in an error state.
 * @post Program execution resumes after the user presses Enter.
 *
 * @note Requires the logging system to be initialized (LINFO macro must be available).
 *
 * @par Example:
 * @code{.cpp}
 * int main() {
 *     // ... program logic ...
 *     SYSPAUSE();
 *     return 0;
 * }
 * @endcode
 */
#define SYSPAUSE()                                                                                                                         \
    do {                                                                                                                                   \
        LINFO("Press enter to exit...");                                                                                                   \
        std::cin.ignore();                                                                                                                 \
    } while(0);

/**
 * @brief Checks if a number is evenly divisible by a divisor.
 *
 * This constexpr function determines whether the dividend `n` is evenly
 * divisible by the divisor `d` by checking if the remainder of the division is zero.
 *
 * @tparam T The type of the dividend (must satisfy std::integral concept).
 * @tparam U The type of the divisor (must satisfy std::integral concept).
 * @param[in] n The dividend to check.
 * @param[in] d The divisor to check against.
 * @return true if `n` is evenly divisible by `d` (i.e., `n % d == 0`), false otherwise.
 *
 * @note This function is marked noexcept as it performs only arithmetic operations.
 * @note Both parameters must be integral types (int, long, size_t, etc.).
 *
 * @par Example:
 * @code{.cpp}
 * constexpr bool result1 = is_divisor(10, 2);  // true
 * constexpr bool result2 = is_divisor(10, 3);  // false
 * @endcode
 */
[[nodiscard]] static constexpr auto is_divisor(std::integral auto n, std::integral auto d) noexcept -> bool { return n % d == 0; }

/**
 * @brief Finds all divisors of a given number.
 *
 * This constexpr function computes all positive divisors of the input number `num`
 * by iterating from 1 to the square root of `num` and collecting both the divisor
 * and its complementary quotient when divisibility is found.
 *
 * @tparam T The type of the number (must satisfy std::integral concept).
 * @param[in] num The number for which to find divisors.
 * @return A std::vector<T> containing all positive divisors of `num` in ascending order.
 *         Returns an empty vector if `num` is less than 1.
 *
 * @pre `num` should be a positive integer (>= 1) for meaningful results.
 * @post The returned vector contains divisors sorted in ascending order.
 *
 * @note The function uses std::views::iota for efficient iteration.
 * @note Time complexity is O(sqrt(n)) where n is the input number.
 *
 * @par Example:
 * @code{.cpp}
 * constexpr auto divisors = find_divisors(12);  // {1, 2, 3, 4, 6, 12}
 * @endcode
 */
template <std::integral T> [[nodiscard]] constexpr auto find_divisors(T num) noexcept -> std::vector<T> {
    if(num < 1) {
        return {};  // Handle edge case where num is less than 1.
    }
    T num_sqrt = T(std::sqrt(num));
    std::vector<T> divisors;
    divisors.reserve(num_sqrt);

    for(const T &val : std::views::iota(T(1), num_sqrt + 1)) {
        T numBval = num / val;
        if(is_divisor(num, val)) {
            divisors.emplace_back(val);
            if(val != numBval) { divisors.emplace_back(numBval); }
        }
    }

    std::ranges::sort(divisors);

    return divisors;
}

/**
 * @brief Extracts and removes leading tab characters from a string view.
 *
 * This function identifies all consecutive tab characters at the beginning of the
 * input string view and returns the substring starting from the first non-tab character.
 * If the string contains only tabs or is empty, returns the original string view.
 *
 * @param[in] input The input string view from which to extract leading tabs.
 * @return A string view containing the input string without leading tab characters.
 *         If the input consists entirely of tabs, returns an empty string view.
 *
 * @note This function is marked [[nodiscard]] to ensure the return value is used.
 * @note This function is marked noexcept as it performs only view operations without allocation.
 * @note The function does not modify the original string view.
 *
 * @par Example:
 * @code{.cpp}
 * constexpr std::string_view input = "\t\tHello";
 * constexpr auto result = extractTabs(input);  // "Hello"
 * @endcode
 */
[[nodiscard]] static constexpr std::string_view extractTabs(const std::string_view &input) noexcept {
    const auto pos = input.find_first_not_of(CTAB);
    return pos == std::string_view::npos ? input : input.substr(0, pos);
}

/**
 * @brief Converts a preprocessing token to a string literal.
 *
 * This macro converts its argument into a string literal by stringifying it.
 * The conversion happens during the preprocessing phase, before compilation.
 *
 * @param x The token or expression to stringify.
 * @return A string literal representation of the argument.
 *
 * @note This is a two-level macro; use TOSTRING() for most applications.
 * @note Useful for generating error messages, debug output, or metadata.
 *
 * @par Example:
 * @code{.cpp}
 * #define VALUE 42
 * const char* str = STRINGIFY(VALUE);  // Expands to "VALUE", not "42"
 * @endcode
 *
 * @see TOSTRING
 */
#define STRINGIFY(x) #x

/**
 * @brief Converts a preprocessing token to a string literal (two-level macro).
 *
 * This macro is an alias for STRINGIFY(x) that enables proper macro expansion
 * before stringification. It converts the provided parameter into a string literal
 * by first expanding any macros, then stringifying the result.
 *
 * @param x The token or expression to stringify.
 * @return A string literal representation of the argument after macro expansion.
 *
 * @note Use this macro instead of STRINGIFY() when the argument is itself a macro.
 *
 * @par Example:
 * @code{.cpp}
 * #define VALUE 42
 * const char* str = TOSTRING(VALUE);  // Expands to "42"
 * @endcode
 *
 * @see STRINGIFY
 */
#define TOSTRING(x) STRINGIFY(x)

/**
 * @brief Creates a unique pointer to an object of the specified type.
 *
 * This macro creates a std::unique_ptr to an object of the specified type,
 * optionally forwarding arguments to its constructor. It provides a concise
 * syntax for creating unique pointers with automatic memory management.
 *
 * @param type The type of the object to create.
 * @param ... The arguments to forward to the constructor of the object.
 * @return A std::unique_ptr<type> pointing to the newly created object.
 *
 * @note The created object will be automatically deleted when the unique_ptr goes out of scope.
 * @note Perfect forwarding is used to preserve value categories of constructor arguments.
 *
 * @par Example:
 * @code{.cpp}
 * auto ptr = MAKE_UNIQUE(MyClass, arg1, arg2);
 * @endcode
 *
 * @see MAKE_SHARED
 */
#define MAKE_UNIQUE(type, ...) std::make_unique<type>(__VA_ARGS__)

/**
 * @brief Creates a shared pointer to an object of the specified type.
 *
 * This macro creates a std::shared_ptr to an object of the specified type,
 * optionally forwarding arguments to its constructor. It provides a concise
 * syntax for creating shared pointers with reference-counted memory management.
 *
 * @param type The type of the object to create.
 * @param ... The arguments to forward to the constructor of the object.
 * @return A std::shared_ptr<type> pointing to the newly created object.
 *
 * @note The created object will be deleted when all shared_ptr instances pointing to it are destroyed.
 * @note Perfect forwarding is used to preserve value categories of constructor arguments.
 *
 * @par Example:
 * @code{.cpp}
 * auto ptr = MAKE_SHARED(MyClass, arg1, arg2);
 * @endcode
 *
 * @see MAKE_UNIQUE
 */
#define MAKE_SHARED(type, ...) std::make_shared<type>(__VA_ARGS__)

/**
 * @brief Gets the index of the active alternative in a std::variant.
 *
 * This macro retrieves the index of the currently active alternative in a std::variant.
 * The index corresponds to the zero-based position of the type in the variant's template parameter list.
 *
 * @param var The std::variant to inspect.
 * @return The zero-based index of the active alternative, or std::variant::valueless_by_exception
 *         if the variant is valueless.
 *
 * @pre The variant must not be in a valueless_by_exception state for meaningful results.
 *
 * @par Example:
 * @code{.cpp}
 * std::variant<int, double, std::string> v = 42;
 * auto idx = GET_VARIANT_INDEX(v);  // Returns 0
 * @endcode
 */
#define GET_VARIANT_INDEX(var) var.index()

/**
 * @brief Extracts a value of the specified type from a std::variant.
 *
 * This macro retrieves the value stored in a std::variant by specifying the target type.
 * It uses std::get<type> internally, which throws std::bad_variant_access if the
 * active alternative does not match the requested type.
 *
 * @param var The std::variant to extract the value from.
 * @param type The type of value to extract (must match the active alternative).
 * @return A reference to the value of the specified type stored in the variant.
 *
 * @throws std::bad_variant_access if the active alternative does not match `type`.
 *
 * @par Example:
 * @code{.cpp}
 * std::variant<int, double> v = 3.14;
 * auto val = GET_VARIANT_TYPE(v, double);  // Returns 3.14
 * @endcode
 */
#define GET_VARIANT_TYPE(var, type) std::get<type>(var)

/**
 * @brief Generates the full name and version information of the generator.
 *
 * This macro formats a string containing the generator's project name, version,
 * and Git short SHA commit hash. It uses the FORMAT() macro for string formatting
 * and retrieves version information from the Vandior::cmake namespace.
 *
 * @return A formatted std::string containing the generator's full name, version,
 *         and Git commit hash in the format: "{project_name} v{version} git sha: {short_sha}"
 *
 * @pre The Vandior::cmake namespace must be available with project_name, project_version,
 *      and git_short_sha defined.
 * @pre The FORMAT() macro must be available (requires format.hpp to be included).
 *
 * @par Example:
 * @code{.cpp}
 * std::string fullName = GENERATOR_FULLNAME;
 * // Example output: "jsav v1.0.0 git sha: abc1234"
 * @endcode
 *
 * @see GENERATOR_VERSION
 */
#define GENERATOR_FULLNAME                                                                                                                 \
    FORMAT("{} v{} git sha: {}", Vandior::cmake::project_name, Vandior::cmake::project_version, Vandior::cmake::git_short_sha)

/**
 * @brief Generates the version string of the generator.
 *
 * This macro formats a string containing the generator's version and Git short SHA commit hash.
 * It uses the FORMAT() macro for string formatting and retrieves version information
 * from the Vandior::cmake namespace.
 *
 * @return A formatted std::string containing the generator's version and Git commit hash
 *         in the format: "v{version} git sha: {short_sha}"
 *
 * @pre The Vandior::cmake namespace must be available with project_name, project_version,
 *      and git_short_sha defined.
 * @pre The FORMAT() macro must be available (requires format.hpp to be included).
 *
 * @par Example:
 * @code{.cpp}
 * std::string version = GENERATOR_VERSION;
 * // Example output: "v1.0.0 git sha: abc1234"
 * @endcode
 *
 * @see GENERATOR_FULLNAME
 */
#define GENERATOR_VERSION                                                                                                                  \
    FORMAT("v{} git sha: {}", Vandior::cmake::project_name, Vandior::cmake::project_version, Vandior::cmake::git_short_sha)

// NOLINTEND(*-macro-usage)

/**
 * @typedef StringPair
 * @brief A type alias for a pair of strings.
 *
 * This type alias defines a pair of std::string objects using std::pair from
 * the C++ Standard Library. It is useful for storing two associated strings together,
 * such as key-value pairs, name-description pairs, or any two related string values.
 *
 * @par Example:
 * @code{.cpp}
 * StringPair config = {"key", "value"};
 * StringPair person = {"John Doe", "Software Engineer"};
 * @endcode
 */
using StringPair = std::pair<std::string, std::string>;

/**
 * @typedef StringPairVec
 * @brief A type alias for a vector of string pairs.
 *
 * This type alias defines a std::vector of StringPair elements. It is useful for
 * storing a collection of associated string pairs, such as configuration settings,
 * metadata, or a list of key-value pairs.
 *
 * @par Example:
 * @code{.cpp}
 * StringPairVec config = {{"name", "John"}, {"age", "30"}, {"city", "New York"}};
 * @endcode
 *
 * @see StringPair
 */
using StringPairVec = std::vector<StringPair>;

/**
 * @typedef StringVec
 * @brief A type alias for a vector of strings.
 *
 * This type alias defines a std::vector of std::string elements. It is useful for
 * storing a list of strings, such as a collection of words, lines from a text file,
 * command-line arguments, or any sequence of string values.
 *
 * @par Example:
 * @code{.cpp}
 * StringVec words = {"hello", "world", "example"};
 * @endcode
 */
using StringVec = std::vector<std::string>;

/**
 * @typedef StrViewVec
 * @brief A type alias for a vector of string views.
 *
 * This type alias defines a std::vector of std::string_view elements. String views
 * provide a non-owning reference to string data, which can be more efficient than
 * copying strings in performance-sensitive operations. This type is useful for
 * storing a list of string views that reference parts of strings without owning them.
 *
 * @note String views do not own their data; ensure the underlying string data
 *       outlives the string views to avoid dangling references.
 *
 * @par Example:
 * @code{.cpp}
 * std::string text = "hello world example";
 * StrViewVec views = {text.substr(0, 5), text.substr(6, 5)};
 * @endcode
 */
using StrViewVec = std::vector<std::string_view>;

/**
 * @typedef OptionalSizeT
 * @brief A type alias for an optional size_t value.
 *
 * This type alias defines a std::optional<size_t> using std::optional from the
 * C++ Standard Library. It represents a value that may or may not be present,
 * useful for functions that may or may not return a valid size or index.
 *
 * @par Example:
 * @code{.cpp}
 * OptionalSizeT findElement(const StringVec& vec, const std::string& target) {
 *     for (size_t i = 0; i < vec.size(); ++i) {
 *         if (vec[i] == target) return i;
 *     }
 *     return std::nullopt;
 * }
 * @endcode
 */
using OptionalSizeT = std::optional<size_t>;

/**
 * @concept StringOrStringView
 * @brief Concept that constrains a type to be std::string or std::string_view.
 *
 * This concept is used to constrain template parameters to ensure that only
 * std::string, std::string_view, or types convertible to them are accepted.
 * It checks for exact type matches as well as types that are convertible to
 * string types or provide string-like interface (data(), size(), length(), iterators).
 *
 * @tparam T The type to check.
 *
 * @par Requirements:
 * The type T must satisfy one of the following:
 * - Be exactly std::string or std::string_view (after removing cv-qualifiers and references)
 * - Be convertible to std::string or std::string_view
 * - Provide string-like interface with:
 *   - begin() and end() returning const iterators
 *   - data() returning const char*
 *   - size() and length() returning integral types
 *
 * @par Example:
 * @code{.cpp}
 * template <StringOrStringView T>
 * void printString(T str) {
 *     std::cout << str << std::endl;
 * }
 *
 * printString(std::string("hello"));  // OK
 * printString(std::string_view("world"));  // OK
 * printString("literal");  // OK (convertible)
 * @endcode
 */
template <typename T>
concept StringOrStringView = std::same_as<std::remove_cvref_t<T>, std::string> || std::same_as<std::remove_cvref_t<T>, std::string_view> ||
                             std::convertible_to<T, std::string> || std::convertible_to<T, std::string_view> || (requires(const T &t) {
                                 { std::ranges::begin(t) } -> std::convertible_to<typename T::const_iterator>;
                                 { std::ranges::end(t) } -> std::convertible_to<typename T::const_iterator>;
                                 { t.data() } -> std::convertible_to<const char *>;
                                 { t.size() } -> std::integral;
                                 { t.length() } -> std::integral;
                             });

constexpr std::size_t golden_ratio_mix = sizeof(std::size_t) == 8 ? 0x9e3779b97f4a7c15ULL  // 64-bit
                                                                  : 0x9e3779b9U;           // 32-bit
inline void hash_combine(std::size_t &seed, std::size_t v) {
    seed ^= std::hash<std::size_t>{}(v) + golden_ratio_mix + (seed << 6) + (seed >> 2);
}
// NOLINTEND(*-include-cleaner, *-identifier-length)
