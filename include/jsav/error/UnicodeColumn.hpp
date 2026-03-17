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
    /// This function checks environment variables in priority order to determine
    /// if ANSI color output should be enabled for error markers. The detection order is:
    ///   1. `NO_COLOR` disables color if set (highest priority)
    ///   2. `COLORTERM` set to "truecolor" or "24bit" enables 24-bit color
    ///   3. `TERM` containing "color", "xterm", "screen", or "tmux" enables color
    ///   4. Otherwise, color is disabled (default)
    ///
    /// @return true if ANSI color is supported, false otherwise.
    /// @threadsafe Yes (reads environment variables only)
    /// @platform Cross-platform (Windows, Linux, macOS)
    /// @see ErrorDisplayConfig, make_display_config
    [[nodiscard]] bool detect_ansi_color() noexcept;

    // ---------------------------------------------------------------------------
    /// @brief Creates an ErrorDisplayConfig with environment-detected defaults.
    ///
    /// This function returns an ErrorDisplayConfig struct with:
    ///   - tab_stop_width = 8 (default, matches GCC/Clang/MSVC)
    ///   - ansi_color = detect_ansi_color() (auto-detected)
    ///
    /// @return ErrorDisplayConfig with recommended settings for the current environment.
    /// @threadsafe Yes (pure function)
    /// @see detect_ansi_color, ErrorDisplayConfig
    [[nodiscard]] ErrorDisplayConfig make_display_config() noexcept;

    // ---------------------------------------------------------------------------
    /// @brief Computes the 1-based visual column for a given byte offset in a UTF-8 line.
    ///
    /// This function walks the line from the start up to the specified byte offset,
    /// counting Unicode code points (not bytes) and expanding tabs according to the
    /// specified tab stop width. Handles BOM at file start, invalid UTF-8, null bytes,
    /// and enforces a line length limit of 10,000 code points.
    ///
    /// @param line         Source line content (UTF-8 encoded).
    /// @param byte_offset  Byte offset within line (0-based).
    /// @param tab_stop_width Tab stop width in columns (default 8, must be > 0).
    ///
    /// @return std::expected<std::size_t, std::string>
    ///   - Success: 1-based visual column (code point count, tabs expanded)
    ///   - Error: Error message string for invalid UTF-8, null byte, or line too long
    ///
    /// @throws None (returns std::expected for errors)
    /// @threadsafe Yes (pure function, no mutation)
    ///
    /// @see marker_extents, ErrorDisplayConfig
    [[nodiscard]] std::expected<std::size_t, std::string> visual_column(std::string_view line, std::size_t byte_offset,
                                                                        std::size_t tab_stop_width = 8) noexcept;

    // ---------------------------------------------------------------------------
    /// @brief Computes (leading_spaces, caret_count) for Unicode error marker display.
    ///
    /// This function calculates the number of leading spaces and carets needed to
    /// visually mark an error span in a UTF-8 encoded line. It uses code point-based
    /// counting, expands tabs, and enforces a minimum of 1 caret. Handles invalid UTF-8,
    /// null bytes, and line length limits. Tabs in the error span contribute their full
    /// expanded width to caret_count.
    ///
    /// @param line         Source line content (UTF-8 encoded).
    /// @param start_byte   Start byte offset within line (inclusive, 0-based).
    /// @param end_byte     End byte offset within line (exclusive, 0-based).
    /// @param tab_stop_width Tab stop width in columns (default 8, must be > 0).
    ///
    /// @return std::expected<std::pair<std::size_t, std::size_t>, std::string>
    ///   - Success: {leading_spaces, caret_count}
    ///   - Error: Error message string for invalid UTF-8, null byte, or line too long
    ///
    /// @throws None (returns std::expected for errors)
    /// @threadsafe Yes (pure function, no mutation)
    ///
    /// @note caret_count is always >= 1 (FR-013)
    /// @note Tabs in the error span contribute their expanded width (FR-010)
    /// @see visual_column, ErrorDisplayConfig
    [[nodiscard]] std::expected<std::pair<std::size_t, std::size_t>, std::string> marker_extents(std::string_view line,
                                                                                                 std::size_t start_byte,
                                                                                                 std::size_t end_byte,
                                                                                                 std::size_t tab_stop_width = 8) noexcept;

}  // namespace jsv
