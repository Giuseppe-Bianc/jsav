/*
 * Created by gbian on 14/03/2026.
 * Copyright (c) 2026 All rights reserved.
 */

#pragma once

/**
 * UnicodeColumn.hpp
 * 
 * Unicode-aware visual column calculation for error reporting.
 * 
 * This module provides UTF-8-aware visual column calculation and error marker
 * extent computation. All functions are stateless, pure (no side effects), and
 * operate on std::string_view inputs.
 * 
 * Key features:
 * - Code point-based column counting (not byte-based)
 * - Tab expansion with configurable tab stops
 * - BOM handling (skip at file start)
 * - Null byte rejection
 * - Invalid UTF-8 detection and reporting
 * - Line length limit enforcement (10,000 code points)
 * - ANSI color detection for terminal output
 * 
 * @since 0.5.0 (feature 005-fix-unicode-error-reporting)
 */

#include "../headers.hpp"

namespace jsv {

// ---------------------------------------------------------------------------
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

// ---------------------------------------------------------------------------
/// @brief Detect ANSI color support from environment variables.
///
/// Detects whether the terminal supports ANSI color escape codes by checking
/// environment variables in the following priority order:
/// 
/// 1. NO_COLOR (any non-empty value) → disables color (highest priority)
/// 2. COLORTERM="truecolor" or "24bit" → enables 24-bit color
/// 3. TERM contains "color", "xterm", "screen", or "tmux" → enables color
/// 4. Default → no color (lowest priority)
///
/// @return true if terminal supports ANSI color, false otherwise.
/// 
/// @threadsafe Yes (reads environment variables via std::getenv)
/// @platform Cross-platform (Windows, Linux, macOS)
/// @see NFR-003, ErrorDisplayConfig::ansi_color
[[nodiscard]] bool detect_ansi_color() noexcept;

// ---------------------------------------------------------------------------
/// @brief Load default ErrorDisplayConfig from environment.
///
/// Creates an ErrorDisplayConfig with default values:
/// - tab_stop_width = 8 (default)
/// - ansi_color = detect_ansi_color() (environment-detected)
///
/// @return ErrorDisplayConfig with environment-detected settings.
/// 
/// @threadsafe Yes
/// @see detect_ansi_color(), ErrorDisplayConfig
[[nodiscard]] ErrorDisplayConfig make_display_config() noexcept;

// ---------------------------------------------------------------------------
/// @brief Return the 1-based visual column of byte_offset inside line.
///
/// Calculates the visual column position by counting Unicode code points
/// (not bytes) from the start of the line. Special handling:
/// 
/// - BOM (0xEF 0xBB 0xBF) at file start is skipped in column count
/// - Tab characters are expanded to visual width (tab_stop_width)
/// - Invalid UTF-8 sequences return an error
/// - Null bytes (U+0000) return an error
/// - Line length limit: 10,000 code points maximum
///
/// @param line Source line content (UTF-8 encoded).
/// @param byte_offset Byte offset within line (0-based).
/// @param tab_stop_width Tab stop width in columns (default 8, must be > 0).
/// 
/// @return std::expected<std::size_t, std::string>
///         - Success: 1-based visual column
///         - Error: Error message string for invalid UTF-8, null byte, or line too long
/// 
/// @throws None (returns std::expected for errors)
/// @threadsafe Yes (pure function, no mutation)
/// 
/// @see FR-003, FR-004, FR-018, FR-019, FR-020, FR-025, FR-026, FR-027
/// @see marker_extents()
[[nodiscard]] std::expected<std::size_t, std::string>
visual_column(std::string_view line,
              std::size_t     byte_offset,
              std::size_t     tab_stop_width = 8) noexcept;

// ---------------------------------------------------------------------------
/// @brief Return (leading_spaces, caret_count) for error marker row.
///
/// Calculates the visual extents of an error span for display in error
/// messages. The leading_spaces value positions the first caret, and
/// caret_count determines how many carets to display.
///
/// @param line Source line content (UTF-8 encoded).
/// @param start_byte Start byte offset within line (inclusive, 0-based).
/// @param end_byte End byte offset within line (exclusive, 0-based).
/// @param tab_stop_width Tab stop width in columns (default 8, must be > 0).
///
/// @return std::expected<std::pair<std::size_t, std::size_t>, std::string>
///         - Success: {leading_spaces, caret_count}
///         - Error: Error message string for invalid UTF-8, null byte, or line too long
///
/// @throws None (returns std::expected for errors)
/// @threadsafe Yes (pure function, no mutation)
///
/// @note caret_count is guaranteed to be at least 1 (FR-013)
/// @note Tabs in the error span contribute their expanded width to caret_count (FR-010)
/// 
/// @see FR-005, FR-006, FR-010, FR-012, FR-013, FR-014, FR-018
/// @see visual_column()
[[nodiscard]] std::expected<std::pair<std::size_t, std::size_t>, std::string>
marker_extents(std::string_view line,
               std::size_t     start_byte,
               std::size_t     end_byte,
               std::size_t     tab_stop_width = 8) noexcept;

}  // namespace jsv
