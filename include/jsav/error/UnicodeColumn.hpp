/*
 * Created by gbian on 16/03/2026.
 * Copyright (c) 2026 All rights reserved.
 */

#pragma once

#include "../headers.hpp"

namespace jsv {

/// @brief Configuration for tab expansion and ANSI color output in error markers.
///
/// This struct controls the visual appearance of error markers (carets) in
/// compiler error messages. It allows customization of tab stop width and
/// enables/disables ANSI color escape codes.
///
/// @threadsafe Yes (immutable after construction)
/// @since 0.5.0 (feature 005-fix-unicode-error-reporting)
struct ErrorDisplayConfig {
    /// @brief Tab stop width in columns (default: 8).
    ///
    /// Controls how tab characters (U+0009) are expanded when calculating
    /// visual column positions. The default value of 8 matches GCC, Clang,
    /// and MSVC behavior.
    ///
    /// @note Must be > 0. Division by zero occurs if tab_stop_width == 0.
    /// @see FR-018, FR-010
    std::size_t tab_stop_width = 8;

    /// @brief Enable ANSI color escape codes for error markers (default: false).
    ///
    /// When true, error markers (carets) are output with ANSI color escape
    /// codes (red color). When false, plain '^' characters are used.
    ///
    /// @note Auto-detected by detect_ansi_color() based on environment variables.
    /// @note Respects NO_COLOR environment variable (overrides to false).
    /// @see NFR-003, detect_ansi_color()
    bool ansi_color = false;
};

/// @brief Detect ANSI color support from environment variables.
///
/// Detects ANSI color support by checking environment variables in the
/// following priority order:
/// 1. NO_COLOR (any non-empty value) → returns false (user disabled color)
/// 2. COLORTERM="truecolor" or "24bit" → returns true
/// 3. TERM contains "color", "xterm", "screen", or "tmux" → returns true
/// 4. Default → returns false (conservative fallback)
///
/// @return true if terminal supports ANSI color escape codes, false otherwise.
/// @threadsafe Yes (reads environment variables via std::getenv)
/// @note Respects NO_COLOR standard (https://no-color.org/)
/// @see NFR-003, ErrorDisplayConfig
[[nodiscard]] bool detect_ansi_color() noexcept;

/// @brief Load default ErrorDisplayConfig from environment.
///
/// Creates an ErrorDisplayConfig with default tab_stop_width (8) and
/// auto-detected ansi_color based on detect_ansi_color().
///
/// @return ErrorDisplayConfig with environment-detected settings.
/// @threadsafe Yes
/// @see detect_ansi_color(), ErrorDisplayConfig
[[nodiscard]] ErrorDisplayConfig make_display_config() noexcept;

/// @brief Return the 1-based visual column of byte_offset inside line.
///
/// Calculates the visual column position by counting Unicode code points
/// (not bytes) from the start of the line. Handles UTF-8 decoding, tab
/// expansion, BOM skipping, and validates the input for encoding errors.
///
/// @param line Source line content (UTF-8 encoded).
/// @param byte_offset Byte offset within line (0-based).
/// @param tab_stop_width Tab stop width in columns (default 8, must be > 0).
/// @return 1-based visual column on success, or error message on failure.
///
/// @error "Invalid UTF-8 sequence at byte offset {offset}" - Invalid UTF-8 detected
/// @error "Null byte (U+0000) at byte offset {offset}" - Null byte detected
/// @error "Source line exceeds 10,000 code points" - Line length limit exceeded
///
/// @threadsafe Yes (pure function, no mutation)
/// @see FR-003, FR-004, FR-018, FR-019, FR-020, FR-027
[[nodiscard]] std::expected<std::size_t, std::string>
visual_column(std::string_view line, std::size_t byte_offset,
              std::size_t tab_stop_width = 8) noexcept;

/// @brief Return (leading_spaces, caret_count) for error marker row.
///
/// Calculates the number of leading spaces and carets needed to display
/// the error marker row in compiler error messages. Uses code point-based
/// column calculation (not byte-based) for correct Unicode alignment.
///
/// @param line Source line content (UTF-8 encoded).
/// @param start_byte Start byte offset within line (inclusive, 0-based).
/// @param end_byte End byte offset within line (exclusive, 0-based).
/// @param tab_stop_width Tab stop width in columns (default 8, must be > 0).
/// @return Pair of (leading_spaces, caret_count) on success, or error message on failure.
///
/// @error "Invalid UTF-8 sequence at byte offset {offset}" - Invalid UTF-8 detected
/// @error "Null byte (U+0000) at byte offset {offset}" - Null byte detected
/// @error "Source line exceeds 10,000 code points" - Line length limit exceeded
///
/// @note caret_count is guaranteed to be at least 1 (FR-013).
/// @threadsafe Yes (pure function, no mutation)
/// @see FR-005, FR-006, FR-010, FR-013, FR-018
[[nodiscard]] std::expected<std::pair<std::size_t, std::size_t>, std::string>
marker_extents(std::string_view line, std::size_t start_byte, std::size_t end_byte,
               std::size_t tab_stop_width = 8) noexcept;

}  // namespace jsv
