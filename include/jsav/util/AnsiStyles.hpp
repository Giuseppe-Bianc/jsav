/*
 * Created by gbian on 18/03/2026.
 * Copyright (c) 2026 All rights reserved.
 */

#pragma once

#include "jsav/headers.hpp"

namespace jsv::ansi {
    /// @{ ANSI escape code constants
    inline constexpr std::string_view kReset = "\x1b[0m";
    inline constexpr std::string_view kBold = "\x1b[1m";
    inline constexpr std::string_view kRed = "\x1b[31m";
    inline constexpr std::string_view kYellow = "\x1b[33m";
    inline constexpr std::string_view kBlue = "\x1b[34m";
    inline constexpr std::string_view kCyan = "\x1b[36m";
    inline constexpr std::string_view kGreen = "\x1b[32m";
    /// @}

    /// @brief Apply @p escape_code (an ANSI prefix) around @p text, then reset.
    ///
    /// Uses only `{}` specifiers so it is compatible with both the
    /// `fmt::format` and `std::format` backends exposed by FORMAT().
    ///
    /// @param text        The text to style.
    /// @param escape_code The ANSI escape sequence to apply.
    /// @return Styled text with reset appended.
    [[nodiscard]] std::string styled(std::string_view text, std::string_view escape_code);

    // --- convenience wrappers matching Rust `style(x).<colour>()` names ---

    /// @{ Plain-colour variants – equivalent of `style(x).red()` etc.
    [[nodiscard]] std::string red(std::string_view text);
    [[nodiscard]] std::string yellow(std::string_view text);
    [[nodiscard]] std::string blue(std::string_view text);
    [[nodiscard]] std::string cyan(std::string_view text);
    [[nodiscard]] std::string green(std::string_view text);
    /// @}

    /// @{ Bold+colour variants – equivalent of `style(x).red().bold()` etc.
    [[nodiscard]] std::string red_bold(std::string_view text);
    [[nodiscard]] std::string blue_bold(std::string_view text);
    /// @}
}  // namespace jsv::ansi
