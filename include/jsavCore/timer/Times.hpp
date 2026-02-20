// NOLINTBEGIN(*-include-cleaner, *-identifier-length, *-special-member-functions, *-convert-member-functions-to-static)
#pragma once

#include "timeFactors.hpp"

DISABLE_WARNINGS_PUSH(26447 26481)  // NOLINT(*-avoid-non-const-global-variables, *-avoid-magic-numbers, *-magic-numbers)

namespace vnd {
    /**
     * @brief Stores time values in multiple units (seconds, milliseconds, microseconds, nanoseconds).
     *
     * @details This class provides a convenient way to store and access a time duration in multiple
     *          units simultaneously. Given a time value in nanoseconds, it automatically computes
     *          the equivalent values in seconds, milliseconds, and microseconds. All values are
     *          stored with long double precision to maintain accuracy across conversions.
     *
     *          The class is designed to be lightweight and copyable, with all operations being
     *          constexpr-compatible for potential compile-time evaluation.
     *
     * @note All time values are stored as long double for maximum precision.
     * @see Times, ValueLabel
     */
    class TimeValues {
    public:
        /**
         * @brief Default constructor initializing all time values to zero.
         *
         * @details Creates a TimeValues instance with seconds, millis, micro, and nano
         *          all initialized to 0.0.
         */
        constexpr TimeValues() noexcept = default;

        /**
         * @brief Constructs TimeValues from a nanoseconds value.
         *
         * @details Converts the provided nanoseconds value into seconds, milliseconds,
         *          and microseconds using the conversion factors (SECONDSFACTOR,
         *          MILLISECONDSFACTOR, MICROSECONDSFACTOR). All conversions maintain
         *          long double precision.
         *
         * @param[in] nanoseconds_ The time value in nanoseconds.
         *
         * @pre nanoseconds_ >= 0 (though not enforced, negative values will propagate)
         * @post seconds = nanoseconds_ / SECONDSFACTOR
         * @post millis = nanoseconds_ / MILLISECONDSFACTOR
         * @post micro = nanoseconds_ / MICROSECONDSFACTOR
         * @post nano = nanoseconds_
         */
        explicit constexpr TimeValues(const long double nanoseconds_) noexcept
          : seconds(nanoseconds_ / SECONDSFACTOR), millis(nanoseconds_ / MILLISECONDSFACTOR), micro(nanoseconds_ / MICROSECONDSFACTOR),
            nano(nanoseconds_) {}

        /**
         * @brief Constructs TimeValues from explicit values for each unit.
         *
         * @details Allows direct specification of time values in each unit without
         *          automatic conversion. This can be useful when time values are
         *          obtained from different sources or need to be set explicitly.
         *
         * @param[in] seconds_ The time value in seconds.
         * @param[in] millis_ The time value in milliseconds.
         * @param[in] micro_ The time value in microseconds.
         * @param[in] nano_ The time value in nanoseconds.
         *
         * @note The provided values are stored as-is without consistency checking
         *       (i.e., they don't need to represent the same duration).
         */
        // NOLINTNEXTLINE(*-easily-swappable-parameters)
        constexpr TimeValues(long double seconds_, long double millis_, long double micro_, long double nano_) noexcept
          : seconds(seconds_), millis(millis_), micro(micro_), nano(nano_) {}

        ///@name Copy and Move Operations
        ///@{
        /**
         * @brief Copy constructor.
         *
         * @details Creates a new TimeValues instance as a copy of another.
         */
        TimeValues(const TimeValues &other) noexcept = default;

        /**
         * @brief Move constructor.
         *
         * @details Creates a new TimeValues instance by moving from another.
         */
        TimeValues(TimeValues &&other) noexcept = default;

        /**
         * @brief Copy assignment operator.
         *
         * @details Copies all time values from another TimeValues instance.
         * @return Reference to this TimeValues.
         */
        TimeValues &operator=(const TimeValues &other) noexcept = default;

        /**
         * @brief Move assignment operator.
         *
         * @details Moves all time values from another TimeValues instance.
         * @return Reference to this TimeValues.
         */
        TimeValues &operator=(TimeValues &&other) noexcept = default;
        ///@}

        ///@name Time Value Accessors
        ///@{
        /**
         * @brief Gets the time value in seconds.
         *
         * @return The time value in seconds as long double.
         */
        [[nodiscard]] constexpr long double get_seconds() const noexcept { return seconds; }

        /**
         * @brief Gets the time value in milliseconds.
         *
         * @return The time value in milliseconds as long double.
         */
        [[nodiscard]] constexpr long double get_millis() const noexcept { return millis; }

        /**
         * @brief Gets the time value in microseconds.
         *
         * @return The time value in microseconds as long double.
         */
        [[nodiscard]] constexpr long double get_micro() const noexcept { return micro; }

        /**
         * @brief Gets the time value in nanoseconds.
         *
         * @return The time value in nanoseconds as long double.
         */
        [[nodiscard]] constexpr long double get_nano() const noexcept { return nano; }
        ///@}

    private:
        long double seconds{};  ///< Time value in seconds.
        long double millis{};   ///< Time value in milliseconds.
        long double micro{};    ///< Time value in microseconds.
        long double nano{};     ///< Time value in nanoseconds.
    };

    /**
     * @brief Formats and converts time values to human-readable strings.
     *
     * @details This class takes a time value with its associated unit label and provides
     *          methods to convert it into formatted strings. For standard units (seconds,
     *          milliseconds, microseconds), it performs smart formatting that breaks down
     *          the value into composite units (e.g., "1s,234ms,567us,890ns").
     *
     *          The class uses std::chrono duration types for precise conversions and
     *          rounding to ensure accurate representation across different time scales.
     *
     * @note Uses long double for internal time value storage.
     * @see TimeValues, Times
     */
    class ValueLabel {
    public:
        /**
         * @brief Default constructor initializing time value to zero and label to empty.
         *
         * @details Creates a ValueLabel instance with timeVal = 0.0 and an empty timeLabel.
         */
        constexpr ValueLabel() noexcept = default;

        /**
         * @brief Constructs ValueLabel with a specific time value and unit label.
         *
         * @details Stores the provided time value and its associated unit label.
         *          The label should be one of: "s", "ms", "us", or "ns" for proper
         *          formatting by toString().
         *
         * @param[in] time_val The numeric time value.
         * @param[in] time_label The unit label (e.g., "s", "ms", "us", "ns").
         *
         * @pre time_label should be a valid unit identifier for meaningful toString() output.
         */
        // NOLINTNEXTLINE(*-easily-swappable-parameters)
        constexpr ValueLabel(const long double time_val, std::string_view time_label) noexcept : timeVal(time_val), timeLabel(time_label) {}

        ///@name Copy and Move Operations
        ///@{
        /**
         * @brief Copy constructor.
         *
         * @details Creates a new ValueLabel instance as a copy of another.
         */
        ValueLabel(const ValueLabel &other) noexcept = default;

        /**
         * @brief Move constructor.
         *
         * @details Creates a new ValueLabel instance by moving from another.
         */
        ValueLabel(ValueLabel &&other) noexcept = default;

        /**
         * @brief Copy assignment operator.
         *
         * @details Copies time value and label from another ValueLabel instance.
         * @return Reference to this ValueLabel.
         */
        ValueLabel &operator=(const ValueLabel &other) noexcept = default;

        /**
         * @brief Move assignment operator.
         *
         * @details Moves time value and label from another ValueLabel instance.
         * @return Reference to this ValueLabel.
         */
        ValueLabel &operator=(ValueLabel &&other) noexcept = default;
        ///@}

        ///@name Time Transformation Methods
        ///@{
        /**
         * @brief Transforms a time value in microseconds to a composite string.
         *
         * @details Converts the input time value (in microseconds) into a formatted
         *          string showing microseconds and nanoseconds components. Uses
         *          std::chrono duration types for precise conversion.
         *
         * @param[in] inputTimeMicro The time value in microseconds.
         * @return Formatted string in the format "Xus,Yns".
         *
         * @example Input: 1234.567 → Output: "1234us,567ns"
         */
        [[nodiscard]] std::string transformTimeMicro(const long double inputTimeMicro) const noexcept {
            using namespace std::chrono;

            const auto durationmicros = microlld(inputTimeMicro);

            const auto durationUs = duration_cast<microseconds>(durationmicros);
            const auto durationNs = round<nanoseconds>(durationmicros - durationUs);

            return FORMAT("{}us,{}ns", C_LD(durationUs.count()), C_LD(durationNs.count()));
        }

        /**
         * @brief Transforms a time value in milliseconds to a composite string.
         *
         * @details Converts the input time value (in milliseconds) into a formatted
         *          string showing milliseconds, microseconds, and nanoseconds components.
         *          Uses std::chrono duration types for precise conversion and rounding.
         *
         * @param[in] inputTimeMilli The time value in milliseconds.
         * @return Formatted string in the format "Xms,Yus,Zns".
         *
         * @example Input: 1.234567 → Output: "1ms,234us,567ns"
         */
        [[nodiscard]] std::string transformTimeMilli(const long double inputTimeMilli) const noexcept {
            using namespace std::chrono;

            const auto durationmils = millilld(inputTimeMilli);

            const auto durationMs = duration_cast<milliseconds>(durationmils);
            const auto durationUs = round<microseconds>(durationmils - durationMs);
            const auto durationNs = round<nanoseconds>(durationmils - durationMs - durationUs);

            return FORMAT("{}ms,{}us,{}ns", C_LD(durationMs.count()), C_LD(durationUs.count()), C_LD(durationNs.count()));
        }

        /**
         * @brief Transforms a time value in seconds to a composite string.
         *
         * @details Converts the input time value (in seconds) into a formatted
         *          string showing seconds, milliseconds, microseconds, and nanoseconds
         *          components. Uses std::chrono duration types for precise conversion
         *          and rounding.
         *
         * @param[in] inputTimeSeconds The time value in seconds.
         * @return Formatted string in the format "Xs,Yms,Zus,Wns".
         *
         * @example Input: 1.234567890 → Output: "1s,234ms,567us,890ns"
         */
        [[nodiscard]] std::string transformTimeSeconds(const long double inputTimeSeconds) const noexcept {
            using namespace std::chrono;

            const auto durationSecs = seclld(inputTimeSeconds);

            const auto durationSec = duration_cast<seconds>(durationSecs);
            const auto durationMs = round<milliseconds>(durationSecs - durationSec);
            const auto durationUs = round<microseconds>(durationSecs - durationSec - durationMs);
            const auto durationNs = round<nanoseconds>(durationSecs - durationSec - durationMs - durationUs);

            return FORMAT("{}s,{}ms,{}us,{}ns", C_LD(durationSec.count()), C_LD(durationMs.count()), C_LD(durationUs.count()),
                          C_LD(durationNs.count()));
        }
        ///@}

        /**
         * @brief Converts the stored time value to a formatted string.
         *
         * @details Selects the appropriate transformation method based on the stored
         *          timeLabel. For "s", "ms", and "us", uses the corresponding composite
         *          transformation. For other labels (e.g., "ns"), returns a simple
         *          "value label" format.
         *
         * @return Formatted time string.
         *
         * @pre timeLabel should be set to a valid unit for meaningful output.
         * @throws May throw if FORMAT macro or underlying string operations fail.
         *
         * @see transformTimeSeconds(), transformTimeMilli(), transformTimeMicro()
         */
        [[nodiscard]] std::string toString() const noexcept {
            if(timeLabel == "s") { return transformTimeSeconds(timeVal); }
            if(timeLabel == "ms") { return transformTimeMilli(timeVal); }
            if(timeLabel == "us") { return transformTimeMicro(timeVal); }
            return FORMAT("{} {}", timeVal, timeLabel);
        }

    private:
        long double timeVal{};           ///< The numeric time value.
        std::string_view timeLabel{""};  ///< The unit label (e.g., "s", "ms", "us", "ns").
    };

    /**
     * @brief High-level time representation with automatic unit selection.
     *
     * @details This class wraps TimeValues and provides intelligent formatting to select
     *          the most human-readable time unit based on the magnitude of the duration.
     *          It automatically chooses between seconds, milliseconds, microseconds, or
     *          nanoseconds depending on which produces a value greater than 1.0.
     *
     *          The class also supports custom labels for each time unit, allowing for
     *          localization or alternative naming conventions.
     *
     * @note Default labels are: "s", "ms", "us", "ns"
     * @see TimeValues, ValueLabel, Timer
     */
    class Times {
    public:
        /**
         * @brief Default constructor.
         *
         * @details Creates a Times instance with zero time value and default labels.
         */
        Times() noexcept = default;

        /**
         * @brief Constructs Times from a nanoseconds value.
         *
         * @details Creates a TimeValues from the nanoseconds and stores it with default
         *          unit labels.
         *
         * @param[in] nanoseconds_ The time duration in nanoseconds.
         */
        explicit Times(const long double nanoseconds_) noexcept : values(nanoseconds_) {}

        /**
         * @brief Constructs Times from a TimeValues object.
         *
         * @details Wraps an existing TimeValues with default unit labels.
         *
         * @param[in] time_values The TimeValues to wrap.
         */
        explicit Times(const TimeValues &time_values) noexcept : values(time_values) {}

        /**
         * @brief Constructs Times with custom unit labels.
         *
         * @details Creates a Times instance with a TimeValues and custom labels for
         *          each time unit. This allows for localization or alternative naming.
         *
         * @param[in] time_values The TimeValues containing the time in multiple units.
         * @param[in] labelseconds_ Label for seconds (default: "s").
         * @param[in] labelmillis_ Label for milliseconds (default: "ms").
         * @param[in] labelmicro_ Label for microseconds (default: "us").
         * @param[in] labelnano_ Label for nanoseconds (default: "ns").
         */
        // NOLINTNEXTLINE(*-easily-swappable-parameters)
        Times(const TimeValues &time_values, std::string_view labelseconds_, std::string_view labelmillis_, std::string_view labelmicro_,
              std::string_view labelnano_) noexcept
          : values(time_values), labelseconds(labelseconds_), labelmillis(labelmillis_), labelmicro(labelmicro_), labelnano(labelnano_) {}

        ///@name Copy and Move Operations
        ///@{
        /**
         * @brief Copy constructor.
         *
         * @details Creates a new Times instance as a copy of another.
         */
        Times(const Times &other) noexcept = default;

        /**
         * @brief Move constructor.
         *
         * @details Creates a new Times instance by moving from another.
         */
        Times(Times &&other) noexcept = default;

        /**
         * @brief Copy assignment operator.
         *
         * @details Copies time values and labels from another Times instance.
         * @return Reference to this Times.
         */
        Times &operator=(const Times &other) noexcept = default;

        /**
         * @brief Move assignment operator.
         *
         * @details Moves time values and labels from another Times instance.
         * @return Reference to this Times.
         */
        Times &operator=(Times &&other) noexcept = default;
        ///@}

        /**
         * @brief Gets the most relevant time unit as a formatted ValueLabel.
         *
         * @details Automatically selects the most appropriate time unit based on magnitude:
         *          - If seconds > 1.0: returns value in seconds with label "s"
         *          - Else if millis > 1.0: returns value in milliseconds with label "ms"
         *          - Else if micro > 1.0: returns value in microseconds with label "us"
         *          - Otherwise: returns value in nanoseconds with label "ns"
         *
         *          This ensures the returned value is always in a human-readable range
         *          (typically between 1 and 1000 of the selected unit).
         *
         * @return ValueLabel containing the most relevant time value and its unit label.
         *
         * @note The selection threshold is strictly > 1.0 for each unit.
         */
        [[nodiscard]] ValueLabel getRelevantTimeframe() const noexcept {
            const auto seconds = values.get_seconds();
            const auto millis = values.get_millis();
            const auto micro = values.get_micro();

            if(seconds > 1.0L) {  // seconds NOLINT(*-branch-clone)
                return {seconds, labelseconds};
            } else if(millis > 1.0L) {  // millis
                return {millis, labelmillis};
            } else if(micro > 1.0L) {  // micros
                return {micro, labelmicro};
            }
            return {values.get_nano(), labelnano};  // nanos
        }

    private:
        TimeValues values;                   ///< Time values in multiple units.
        std::string_view labelseconds{"s"};  ///< Label for seconds unit.
        std::string_view labelmillis{"ms"};  ///< Label for milliseconds unit.
        std::string_view labelmicro{"us"};   ///< Label for microseconds unit.
        std::string_view labelnano{"ns"};    ///< Label for nanoseconds unit.
    };
    DISABLE_WARNINGS_POP()
}  // namespace vnd

/**
 * @brief Formatter specialization for ValueLabel using fmt library.
 *
 * @details This template specialization enables fmt library to format ValueLabel
 *          objects by delegating to ValueLabel::toString(). This allows ValueLabel
 *          to be used directly with fmt::format(), fmt::print(), and the FORMAT
 *          macro.
 *
 * @note Marked with \cond to exclude from Doxygen documentation (implementation detail).
 * \cond
 */
template <> struct fmt::formatter<vnd::ValueLabel> : fmt::formatter<std::string_view> {
    auto format(const vnd::ValueLabel &val, format_context &ctx) const -> format_context::iterator {
        return fmt::formatter<std::string_view>::format(val.toString(), ctx);
    }
};
/** \endcond */

// NOLINTEND(*-include-cleaner, *-identifier-length, *-special-member-functions, *-convert-member-functions-to-static)