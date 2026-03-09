/*
 * Created by gbian on 09/03/2026.
 * Copyright (c) 2026 All rights reserved.
 */

// NOLINTBEGIN(*-diagnostic-double-promotion, *-pro-bounds-constant-array-index, *-identifier-length)
#pragma once

#include "cast/casts.hpp"
#include "format.hpp"

static inline constexpr std::array<std::string_view, 5> UNITS = {"B", "KB", "MB", "GB", "TB"};
static inline constexpr std::size_t UNIT_LEN = UNITS.size() - 1;
static inline constexpr long double UNIT_DIVIDER = 1024.0L;
struct FormattedSize {
    long double value;
    std::string_view unit;

    [[nodiscard]] std::string to_string() const { return FORMAT("{} {}", value, unit); }
};

[[nodiscard]] constexpr FormattedSize format_size(std::size_t bytes) noexcept {
    auto size = C_LD(bytes);
    std::size_t unit = 0;

    while(size >= UNIT_DIVIDER && unit < UNIT_LEN) {
        size /= UNIT_DIVIDER;
        ++unit;
    }

    return {.value = size, .unit = UNITS[unit]};
}

template <> struct std::formatter<FormattedSize> : std::formatter<std::string> {
    template <typename FormatContext> auto format(const FormattedSize &fs, FormatContext &ctx) const {
        return std::formatter<std::string>::format(fs.to_string(), ctx);
    }
};
template <> struct fmt::formatter<FormattedSize> : fmt::formatter<std::string> {
    template <typename FormatContext> auto format(const FormattedSize &fs, FormatContext &ctx) const {
        return fmt::formatter<std::string>::format(fs.to_string(), ctx);
    }
};
// NOLINTEND(*-diagnostic-double-promotion, *-pro-bounds-constant-array-index, *-identifier-length)