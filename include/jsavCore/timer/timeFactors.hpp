//
// Created by gbian on 04/06/2024.
//

#pragma once

namespace vnd {
    /**
     * @brief Conversion factor from microseconds to nanoseconds.
     *
     * @details This constant represents the number of nanoseconds in one microsecond (1000).
     *          To convert a value from microseconds to nanoseconds, multiply by this factor.
     *          Despite the name suggesting microseconds-to-seconds, the actual value (1000.0L)
     *          indicates this is the microseconds-to-nanoseconds conversion factor.
     *
     * @note Value: 1000.0L (nanoseconds per microsecond)
     */
    inline static constexpr long double MICROSECONDSFACTOR = 1000.0L;

    /**
     * @brief Conversion factor from milliseconds to nanoseconds.
     *
     * @details This constant represents the number of nanoseconds in one millisecond (1,000,000).
     *          To convert a value from milliseconds to nanoseconds, multiply by this factor.
     *          Despite the name suggesting milliseconds-to-seconds, the actual value (1'000'000.0L)
     *          indicates this is the milliseconds-to-nanoseconds conversion factor.
     *
     * @note Value: 1'000'000.0L (nanoseconds per millisecond)
     */
    inline static constexpr long double MILLISECONDSFACTOR = 1'000'000.0L;

    /**
     * @brief Conversion factor from seconds to nanoseconds.
     *
     * @details This constant represents the number of nanoseconds in one second (1,000,000,000).
     *          To convert a value from seconds to nanoseconds, multiply by this factor.
     *          Despite the name suggesting seconds-to-seconds, the actual value (1'000'000'000.0L)
     *          indicates this is the seconds-to-nanoseconds conversion factor.
     *
     * @note Value: 1'000'000'000.0L (nanoseconds per second)
     */
    inline static constexpr long double SECONDSFACTOR = 1'000'000'000.0L;

    /**
     * @brief Multiplier factor used in Timer::time_it() for iteration control.
     *
     * @details This constant (value: 100) is used internally by the Timer::time_it() method
     *          to set a minimum number of iterations when benchmarking code. The function
     *          will execute the provided callable at least MFACTOR times or until the
     *          target time is reached, whichever comes later.
     *
     * @see Timer::time_it()
     */
    inline static constexpr long MFACTOR = 100;
}  // namespace vnd