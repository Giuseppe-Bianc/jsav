/*
 * Created by gbian on 09/10/2024.
 */

#pragma once

#include "../headersCore.hpp"

namespace vnd {
    /**
     * @brief Format string for simple timer output.
     *
     * @details Used by Timer::Simple() to produce output in the format:
     *          "title: Time = value"
     */
    static inline constexpr auto simpleFormat = "{}: Time = {}";

    /**
     * @brief Format string for time value in big output style.
     *
     * @details Used by Timer::Big() to format the time portion of the output.
     */
    static inline constexpr auto bigTimesFormat = "Time = {}";

    /**
     * @brief Format string for title and time alignment in big output style.
     *
     * @details Used by Timer::Big() to create a bordered section containing
     *          the title and time value with proper padding.
     * @param arg0 The title string
     * @param arg1 The width for title padding
     * @param arg2 The formatted time string
     * @param arg3 The width for time padding
     */
    static inline constexpr auto bigTitleTimeFormat = "|{0: ^{1}}|{2: ^{3}}|";

    /**
     * @brief Format string for complete big output style with borders.
     *
     * @details Used by Timer::Big() to create a fully bordered output block
     *          with separator lines above and below the content.
     */
    static inline constexpr auto bigFormat = "\n{0:-^{1}}\n{2}\n{0:-^{1}}";

    /**
     * @brief Format string for compact timer output.
     *
     * @details Used by Timer::Compact() to produce output in the format:
     *          "[title]time"
     */
    static inline constexpr auto compactFormat = "[{}]{}";

    /**
     * @brief Format string for detailed timer output.
     *
     * @details Used by Timer::Detailed() to produce a verbose, human-readable
     *          description of the measured duration.
     */
    static inline constexpr auto detailedFormat = "Timer '{}' measured a duration of {}";

    /**
     * @brief Format string for creating block pattern separators.
     *
     * @details Used by Timer::Block() to generate decorative separator lines
     *          composed of repeated characters.
     */
    static inline constexpr auto blockPatternFormat = "{0:=^{1}}|{0:=^{1}}|{0:=^{1}}|{0:=^{1}}";

    /**
     * @brief Format string for time value in block output style.
     *
     * @details Used by Timer::Block() to format the time portion of block output.
     */
    static inline constexpr auto blockTimesFormat = "Time:{}";

    /**
     * @brief Format string for complete block output style with decorative borders.
     *
     * @details Used by Timer::Block() to create a visually distinct block with
     *          pattern separators and centered content.
     */
    static inline constexpr auto blockFormat = "\n{0}\n{2: ^{1}}\n{0}\n{3: ^{1}}\n{0}";

    /**
     * @brief Format string for minimal timer output.
     *
     * @details Used by Timer::Minimal() to produce output in the format:
     *          "title - time"
     */
    static inline constexpr auto minimalFormat = "{} - {}";

    /**
     * @brief Format string for time_it() method output.
     *
     * @details Used by Timer::time_it() to report the measured time and
     *          number of iterations performed.
     */
    static inline constexpr auto timeItFormat = "{} for {} tries";

    /**
     * @brief Default padding added to timer titles for alignment.
     *
     * @details This constant value (10) is added to the title length to ensure
     *          consistent spacing in formatted output across different timer
     *          instances with varying title lengths.
     */
    static inline constexpr std::size_t TILEPADDING = 10;

    /**
     * @brief High-resolution clock type for precise time measurements.
     *
     * @details Alias for std::chrono::high_resolution_clock, providing the
     *          highest precision clock available on the platform. Used by
     *          Timer class for capturing start and end time points.
     */
    using clock = ch::high_resolution_clock;

    /**
     * @brief Time point type for marking specific moments in time.
     *
     * @details Alias for std::chrono::time_point based on the high_resolution_clock.
     *          Represents a specific point in time measured from the clock's epoch.
     */
    using time_point = ch::time_point<clock>;

    /**
     * @brief Duration type representing nanoseconds with long double precision.
     *
     * @details Alias for std::chrono::duration with nanosecond resolution and
     *          long double representation. Used for high-precision time measurements
     *          that preserve fractional nanoseconds.
     */
    using nanolld = ch::duration<long double, std::nano>;

    /**
     * @brief Duration type representing microseconds with long double precision.
     *
     * @details Alias for std::chrono::duration with microsecond resolution and
     *          long double representation. Used for time measurements in the
     *          microsecond range.
     */
    using microlld = ch::duration<long double, std::micro>;

    /**
     * @brief Duration type representing milliseconds with long double precision.
     *
     * @details Alias for std::chrono::duration with millisecond resolution and
     *          long double representation. Used for time measurements in the
     *          millisecond range.
     */
    using millilld = ch::duration<long double, std::milli>;

    /**
     * @brief Duration type representing seconds with long double precision.
     *
     * @details Alias for std::chrono::duration with second resolution and
     *          long double representation. Used for time measurements in the
     *          second range.
     */
    using seclld = ch::duration<long double>;
}  // namespace vnd