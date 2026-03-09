/*
 * Created by gbian on 09/03/2026.
 * Copyright (c) 2026 All rights reserved.
 */

#pragma once
// NOLINTBEGIN(*-diagnostic-double-promotion, *-pro-bounds-constant-array-index, *-identifier-length)

#include "cast/casts.hpp"
#include "format.hpp"

namespace jsv {
    static inline constexpr std::array<std::string_view, 5> UNITS = {"B", "KB", "MB", "GB", "TB"};
    static inline constexpr std::size_t UNIT_LEN = UNITS.size() - 1;
    static inline constexpr long double UNIT_DIVIDER = 1024.0L;
}  // namespace jsv

struct FormattedSize {
    long double value;
    std::string_view unit;
};

[[nodiscard]] constexpr FormattedSize format_size(std::size_t bytes) noexcept {
    auto size = C_LD(bytes);
    std::size_t unit = 0;

    while(size >= jsv::UNIT_DIVIDER && unit < jsv::UNIT_LEN) {
        size /= jsv::UNIT_DIVIDER;
        ++unit;
    }

    return {.value = size, .unit = jsv::UNITS[unit]};
}

template <> struct std::formatter<FormattedSize> : std::formatter<std::string> {
    template <typename FormatContext> auto format(const FormattedSize &fs, FormatContext &ctx) const {
        auto out = ctx.out();
        out = std::format_to(out, "{} {}", fs.value, fs.unit);
        return out;
    }
};
template <> struct fmt::formatter<FormattedSize> : fmt::formatter<std::string> {
    template <typename FormatContext> auto format(const FormattedSize &fs, FormatContext &ctx) const {
        return fmt::format_to(ctx.out(), "{} {}", fs.value, fs.unit);
    }
};
// NOLINTEND(*-diagnostic-double-promotion, *-pro-bounds-constant-array-index, *-identifier-length)