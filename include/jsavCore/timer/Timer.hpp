// NOLINTBEGIN(*-include-cleaner, *-identifier-length, *-special-member-functions)
#pragma once
#if defined(__GNUC__) && (__GNUC__ >= 11)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wuseless-cast"
#endif
#include "../Log.hpp"
#include "../disableWarn.hpp"
#include "../format.hpp"
#include "../headersCore.hpp"
#include "TimerConstats.hpp"
#include "Times.hpp"
#include "timeFactors.hpp"
// On GCC < 4.8, the following define is often missing. Since
// this library only uses sleep_for, this should be safe
#if defined(__GNUC__) && !defined(__clang__) && __GNUC__ < 5 && __GNUC_MINOR__ < 8
#define _GLIBCXX_USE_NANOSLEEP
#endif

namespace vnd {
    DISABLE_WARNINGS_PUSH(26447 26455)

    /**
     * @brief High-precision timer class for measuring code execution time.
     *
     * @details The Timer class provides a flexible and accurate way to measure the execution
     *          time of code segments. It uses std::chrono::high_resolution_clock for maximum
     *          precision and supports multiple output formats (Simple, Big, Compact, Detailed,
     *          Block, Minimal).
     *
     *          The timer starts automatically upon construction and continues running until
     *          make_time() or to_string() is called. The class also supports benchmarking
     *          via the time_it() method, which executes a function multiple times to reach
     *          a target duration for more accurate measurements.
     *
     *          For automatic logging upon destruction, see AutoTimer which inherits from Timer.
     *
     * @note All copy/move operations are deleted to prevent accidental timer resets.
     * @see AutoTimer, Times, ValueLabel
     *
     * @example
     * ```cpp
     * Timer timer("My Operation");
     * // ... code to measure ...
     * LINFO(timer.to_string());  // Outputs: "My Operation: Time = 1.234ms"
     * ```
     */
    class Timer {
    protected:
        /**
         * @brief Function type for custom time formatting.
         *
         * @details Alias for std::function that takes a title string, title length padding,
         *          and a ValueLabel, returning a formatted std::string. Used to store
         *          the custom print function for timer output.
         */
        using time_print_t = std::function<std::string(std::string, std::size_t, ValueLabel)>;

        std::string title_;         ///< The title/description for this timer measurement.
        std::size_t title_lenpadd;  ///< Title length plus padding for aligned output.
        time_print_t time_print_;   ///< The formatting function for timer output.
        time_point start_;          ///< The time point when the timer was started.
        std::size_t cycles{1};      ///< Number of cycles for averaged timing measurements.

    public:
        ///@name Standard Print Functions
        ///@{

        /**
         * @brief Default print function producing simple one-line output.
         *
         * @details Formats the timer output as: "title: Time = value"
         *          This is the default print function used by Timer if no custom
         *          formatter is specified.
         *
         * @param[in] title The timer title string.
         * @param[in] title_lenpadd The padded title length (unused in this formatter).
         * @param[in] time The ValueLabel containing the formatted time value.
         * @return Formatted string with title and time.
         *
         * @see simpleFormat
         */
        static std::string Simple(const std::string &title, [[maybe_unused]] std::size_t title_lenpadd, const ValueLabel &time) {
            return FORMAT(simpleFormat, title, time);
        }

        /**
         * @brief Elaborate print function producing bordered multi-line output.
         *
         * @details Creates a visually distinct output block with borders and centered
         *          content. The output includes separator lines above and below the
         *          title-time section.
         *
         * @param[in] title The timer title string.
         * @param[in] title_lenpadd The padded title length for alignment.
         * @param[in] time The ValueLabel containing the formatted time value.
         * @return Formatted multi-line string with decorative borders.
         *
         * @see bigFormat, bigTimesFormat, bigTitleTimeFormat
         */
        static std::string Big(const std::string &title, std::size_t title_lenpadd, const ValueLabel &time) {
            const auto times = FORMAT(bigTimesFormat, time);
            const auto tot_len = title_lenpadd + times.length() + 3;
            const auto title_time_section = FORMAT(bigTitleTimeFormat, title, title_lenpadd - 4, times, times.length() + 1);
            return FORMAT(bigFormat, "", tot_len, title_time_section);
        }

        /**
         * @brief Compact print function producing minimal bracketed output.
         *
         * @details Formats the timer output as: "[title]time"
         *          Useful for concise logging where space is limited.
         *
         * @param[in] title The timer title string.
         * @param[in] title_lenpadd The padded title length (unused in this formatter).
         * @param[in] time The ValueLabel containing the formatted time value.
         * @return Formatted string with title in brackets followed by time.
         *
         * @see compactFormat
         */
        static std::string Compact(const std::string &title, [[maybe_unused]] std::size_t title_lenpadd, const ValueLabel &time) {
            return FORMAT(compactFormat, title, time);
        }

        /**
         * @brief Detailed print function producing verbose human-readable output.
         *
         * @details Formats the timer output as: "Timer 'title' measured a duration of time"
         *          Provides the most explicit description of the measurement.
         *
         * @param[in] title The timer title string.
         * @param[in] title_lenpadd The padded title length (unused in this formatter).
         * @param[in] time The ValueLabel containing the formatted time value.
         * @return Formatted descriptive string.
         *
         * @see detailedFormat
         */
        static std::string Detailed(const std::string &title, [[maybe_unused]] std::size_t title_lenpadd, const ValueLabel &time) {
            return FORMAT(detailedFormat, title, time);
        }

        /**
         * @brief Creates a decorative pattern string for block output.
         *
         * @details Generates a repeated character pattern (using '=') divided into
         *          four sections by '|' characters. The pattern length is derived
         *          from title_lenpadd divided by 4.
         *
         * @param[in] title_lenpadd The padded title length used to calculate pattern width.
         * @return Formatted pattern string (e.g., "====|====|====|====").
         *
         * @see blockPatternFormat
         */
        static std::string createPattern(const std::size_t title_lenpadd) {
            const auto ntlenpadd = title_lenpadd / 4;
            return FORMAT(blockPatternFormat, "*", ntlenpadd);
        }

        /**
         * @brief Block style print function producing decorative bordered output.
         *
         * @details Creates a multi-line output with pattern separators above, below,
         *          and between content sections. The title and time are displayed
         *          in separate centered sections within the block.
         *
         * @param[in] title The timer title string.
         * @param[in] title_lenpadd The padded title length for alignment.
         * @param[in] time The ValueLabel containing the formatted time value.
         * @return Formatted multi-line string with decorative block borders.
         *
         * @see blockFormat, createPattern()
         */
        static std::string Block(const std::string &title, std::size_t title_lenpadd, const ValueLabel &time) {
            const auto patternf = createPattern(title_lenpadd);
            const auto times = FORMAT(blockTimesFormat, time);
            return FORMAT(blockFormat, patternf, title_lenpadd, title, times);
        }

        /**
         * @brief Minimal print function producing simplest output.
         *
         * @details Formats the timer output as: "title - time"
         *          The most basic formatter, taking pre-formatted time string.
         *
         * @param[in] title The timer title string.
         * @param[in] time The pre-formatted time string (not ValueLabel).
         * @return Formatted string with title and time separated by " - ".
         *
         * @see minimalFormat
         */
        static std::string Minimal(const std::string &title, const std::string &time) { return FORMAT(minimalFormat, title, time); }
        ///@}

        /**
         * @brief Standard constructor initializing timer with title and print function.
         *
         * @details Constructs a Timer that starts measuring time immediately. The title
         *          is used to identify the measurement in output, and the print function
         *          determines the output format. Default print function is Timer::Simple.
         *
         * @param[in] title The title/description for this timer (default: "Timer").
         * @param[in] time_print The formatting function (default: Timer::Simple).
         *
         * @post Timer is running from the moment of construction.
         * @post title_ = title, title_lenpadd = title.length() + TILEPADDING
         *
         * @example
         * ```cpp
         * Timer t("Database Query");  // Starts timing immediately
         * ```
         */
        explicit Timer(const std::string &title = "Timer", const time_print_t &time_print = Simple)
          : title_(title), title_lenpadd(title.length() + TILEPADDING), time_print_(time_print), start_(clock::now()) {}

        ///@name Copy and Move Operations (Deleted)
        ///@{
        /**
         * @brief Copy constructor deleted to prevent accidental timer duplication.
         *
         * @details Copying a timer would be ambiguous (should the copy continue timing
         *          or start fresh?), so this operation is explicitly deleted.
         */
        Timer(const Timer &) = delete;

        /**
         * @brief Copy assignment operator deleted.
         *
         * @details Prevents assignment from another Timer instance.
         */
        Timer &operator=(const Timer &) = delete;

        /**
         * @brief Move constructor deleted.
         *
         * @details Prevents moving a Timer instance.
         */
        Timer(Timer &&) = delete;

        /**
         * @brief Move assignment operator deleted.
         *
         * @details Prevents move assignment from another Timer instance.
         */
        Timer &operator=(Timer &&) = delete;
        ///@}

        /**
         * @brief Measures execution time of a callable over multiple iterations.
         *
         * @details Executes the provided function object repeatedly until either:
         *          - At least MFACTOR (100) iterations have been performed, AND
         *          - The total elapsed time reaches or exceeds target_time
         *
         *          Returns the average time per iteration. The timer's start point
         *          is saved and restored after the measurement to avoid affecting
         *          subsequent measurements.
         *
         * @param[in] f The function/callable to benchmark.
         * @param[in] target_time The target total duration in seconds (default: 1.0).
         * @return Formatted string showing average time and number of iterations.
         *
         * @pre f must be a valid callable (std::function<void()>).
         * @post Timer's start_ is restored to its value before the call.
         *
         * @throws May throw if f throws an exception during execution.
         * @throws May throw if FORMAT or string operations fail.
         *
         * @note Uses quiet_NaN() for total_time initialization before the loop.
         * @see MFACTOR
         *
         * @example
         * ```cpp
         * Timer timer;
         * auto result = timer.time_it([]() {
         *     // Code to benchmark
         * }, 0.5);  // Target 0.5 seconds
         * ```
         */
        [[nodiscard]] std::string time_it(const std::function<void()> &f, long double target_time = 1) {
            const time_point start = start_;
            [[maybe_unused]] auto total_time = std::numeric_limits<long double>::quiet_NaN();

            start_ = clock::now();
            std::size_t n = 0;
            do {  // NOLINT(*-avoid-do-while)
                f();
                total_time = ch::duration_cast<nanolld>(clock::now() - start_).count();
            } while(n++ < MFACTOR && total_time < target_time);
            const auto total_timef = C_LD(total_time / C_LD(n));
            std::string out = FORMAT(timeItFormat, make_time_str(total_timef), std::to_string(n));
            start_ = start;
            return out;
        }

        /**
         * @brief Gets the elapsed time in nanoseconds since timer start.
         *
         * @details Calculates the duration from the timer's start_ time point to
         *          the current time using high_resolution_clock. The result is
         *          returned as a long double representing nanoseconds.
         *
         * @return Elapsed time in nanoseconds as long double.
         *
         * @note Despite the name "make_time", the return value is in nanoseconds, not seconds.
         * @see make_time_str(), multi_time()
         */
        [[nodiscard]] long double make_time() const noexcept { return ch::duration_cast<nanolld>(clock::now() - start_).count(); }

        /**
         * @brief Gets the elapsed time as a Times object with named units.
         *
         * @details Converts the current elapsed time (in nanoseconds) into a Times
         *          object, which provides access to the duration in seconds,
         *          milliseconds, microseconds, and nanoseconds simultaneously.
         *
         * @param[in] time The time value in nanoseconds.
         * @return Times object containing the time in multiple units.
         *
         * @see Times, TimeValues
         */
        [[nodiscard]] static Times make_named_times(const long double time) noexcept { return Times{time}; }

        /**
         * @brief Gets the elapsed time as a Times object (convenience wrapper).
         *
         * @details Calls make_time() internally and wraps the result in a Times object.
         *          This is a convenience method for getting multi-unit time representation.
         *
         * @return Times object containing the elapsed time in multiple units.
         *
         * @note Marked as [[maybe_unused]] to suppress warnings when return value is unused.
         * @see make_time(), make_named_times()
         */
        [[maybe_unused]] [[nodiscard]] Times multi_time() const noexcept { return Times{make_time()}; }

        /**
         * @brief Formats the elapsed time as a ValueLabel string.
         *
         * @details Calculates the average time per cycle by dividing the total elapsed
         *          time by the cycles count. This is useful when measuring repeated
         *          operations. The result is formatted using make_time_str(long double).
         *
         * @return ValueLabel containing the formatted time string.
         *
         * @post Uses current cycles value for division (default: 1).
         * @see cycles, make_time_str(long double)
         */
        [[nodiscard]] ValueLabel make_time_str() const noexcept {
            const auto time = C_LD(make_time() / C_LD(cycles));
            return make_time_str(time);
        }

        /**
         * @brief Formats a raw nanoseconds value into a human-readable string.
         *
         * @details Converts the provided time value (in nanoseconds) into the most
         *          appropriate unit using Times::getRelevantTimeframe(). The result
         *          is a ValueLabel that can be directly used with fmt formatting.
         *
         * @param[in] time The time value in nanoseconds.
         * @return ValueLabel containing the formatted time with appropriate unit.
         *
         * @note Marked with LCOV_EXCL_STOP to exclude from coverage reports (trivial getter).
         * @see Times::getRelevantTimeframe()
         */
        // LCOV_EXCL_START
        [[nodiscard]] static ValueLabel make_time_str(const long double time) noexcept {
            return make_named_times(time).getRelevantTimeframe();
        }
        // LCOV_EXCL_STOP

        /**
         * @brief Gets a string representation of the Timer using the configured formatter.
         *
         * @details Calls the stored time_print_ function with the timer's title,
         *          padded title length, and formatted time value. This is the
         *          primary method for obtaining formatted timer output.
         *
         * @return Formatted timer string according to the configured print function.
         *
         * @see time_print_, Simple(), Big(), Compact(), Detailed(), Block(), Minimal()
         *
         * @example
         * ```cpp
         * Timer timer("Operation");
         * std::cout << timer.to_string() << std::endl;
         * ```
         */
        [[nodiscard]] std::string to_string() const noexcept {
            const auto time = make_time_str();
            return std::invoke(time_print_, title_, title_lenpadd, time);
        }

        /**
         * @brief Sets the number of cycles for averaged timing measurements.
         *
         * @details Divides the elapsed time by the specified value when formatting
         *          output. This is useful for measuring the average time of
         *          repeated operations. The operator returns a reference to
         *          enable fluent syntax.
         *
         * @param[in] val The number of cycles to divide by.
         * @return Reference to this Timer for method chaining.
         *
         * @post cycles = val
         *
         * @note Division by zero is not checked and will result in undefined behavior.
         *
         * @example
         * ```cpp
         * Timer timer;
         * for(int i = 0; i < 100; ++i) {
         *     operation();
         * }
         * std::cout << (timer / 100).to_string() << std::endl;  // Average time per operation
         * ```
         */
        Timer &operator/(std::size_t val) noexcept {
            cycles = val;
            return *this;
        }
    };

    /**
     * @brief Timer subclass that automatically logs elapsed time on destruction.
     *
     * @details AutoTimer extends Timer with RAII semantics: when an AutoTimer instance
     *          goes out of scope, its destructor automatically logs the elapsed time
     *          using LINFO(). This is useful for function-level timing without
     *          explicit logging calls.
     *
     *          All copy/move operations are deleted to prevent accidental duplication.
     *
     * @note The destructor catches all exceptions to prevent stack unwinding issues.
     * @see Timer, LINFO
     *
     * @example
     * ```cpp
     * void myFunction() {
     *     AutoTimer timer("myFunction execution time");
     *     // ... function body ...
     * }  // Timer automatically logged here when timer goes out of scope
     * ```
     */
    class AutoTimer : public Timer {
    public:
        /**
         * @brief Inherit constructors from Timer base class.
         *
         * @details Makes all Timer constructors available for AutoTimer.
         */
        using Timer::Timer;

        ///@name Copy and Move Operations (Deleted)
        ///@{
        /**
         * @brief Copy constructor deleted.
         *
         * @details Prevents copying an AutoTimer instance.
         */
        AutoTimer(const AutoTimer &) = delete;

        /**
         * @brief Copy assignment operator deleted.
         *
         * @details Prevents assignment from another AutoTimer instance.
         */
        AutoTimer &operator=(const AutoTimer &) = delete;

        /**
         * @brief Move constructor deleted.
         *
         * @details Prevents moving an AutoTimer instance.
         */
        AutoTimer(AutoTimer &&) = delete;

        /**
         * @brief Move assignment operator deleted.
         *
         * @details Prevents move assignment from another AutoTimer instance.
         */
        AutoTimer &operator=(AutoTimer &&) = delete;
        ///@}

        /**
         * @brief Destructor that logs elapsed time.
         *
         * @details Logs the timer's formatted output using LINFO() upon destruction.
         *          This enables automatic timing of scopes without explicit logging.
         *          All exceptions are caught to prevent issues during stack unwinding.
         *
         * @note Exceptions caught in the destructor are silently ignored.
         * @todo Consider adding a configurable log level or output stream.
         */
        ~AutoTimer() noexcept {
            try {
                LINFO(to_string());
            } catch(...) {  // NOLINT(*-empty-catch)
                // Handle or log the exception as needed
            }
        }
    };
}  // namespace vnd

/**
 * @brief Formatter specialization for Timer using fmt library.
 *
 * @details This template specialization enables fmt library to format Timer
 *          objects by delegating to Timer::to_string(). This allows Timer
 *          to be used directly with fmt::format(), fmt::print(), and the
 *          FORMAT macro.
 *
 * @note Marked with \cond to exclude from Doxygen documentation (implementation detail).
 * \cond
 */
template <> struct fmt::formatter<vnd::Timer> : formatter<std::string_view> {
    auto format(const vnd::Timer &timer, format_context &ctx) const -> format_context::iterator {
        return formatter<std::string_view>::format(timer.to_string(), ctx);
    }
};
/** \endcond */

/**
 * @brief Stream insertion operator for Timer.
 *
 * @details Enables writing Timer objects directly to std::ostream using
 *          the timer's configured print function. This allows syntax like:
 *          std::cout << timer << std::endl;
 *
 * @param[in,out] os The output stream to write to.
 * @param[in] timer The Timer object to format and write.
 * @return Reference to os for chaining.
 *
 * @see Timer::to_string()
 */
inline std::ostream &operator<<(std::ostream &os, const vnd::Timer &timer) { return os << timer.to_string(); }

DISABLE_WARNINGS_POP()
#if defined(__GNUC__) && (__GNUC__ >= 11)
#pragma GCC diagnostic pop
#endif

// NOLINTEND(*-include-cleaner, *-identifier-length, *-special-member-functions)
