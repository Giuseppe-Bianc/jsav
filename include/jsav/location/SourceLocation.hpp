/*
 * Created by gbian on 20/02/2026.
 * Copyright (c) 2026 All rights reserved.
 */

#pragma once

#include "../headers.hpp"

namespace jsv {

    /// Represents a specific position in source code with line/column information.
    ///
    /// Stores both human-readable (line/column) and machine-oriented (byte offset)
    /// positioning data. Useful for error reporting, debugging information, and
    /// source mapping.
    ///
    /// # Indexing Conventions
    /// - `line`: 1-indexed line number (first line is line 1)
    /// - `column`: 1-indexed column number (first character in line is column 1)
    /// - `absolute_pos`: 0-indexed byte offset from start of source
    ///
    /// # Ordering
    /// Implements lexicographic ordering based on:
    /// 1. Line number
    /// 2. Column number
    /// 3. Byte offset
    ///
    /// This ordering matches how humans read source code (top-to-bottom, left-to-right).
    class SourceLocation {
    public:
        /// Line number in source file (1-indexed).
        std::size_t line = 0;

        /// Column position in line (1-indexed, byte-based).
        std::size_t column = 0;

        /// Absolute byte offset from start of source (0-indexed).
        std::size_t absolute_pos = 0;

        /// Default constructor — all fields zero-initialized.
        constexpr SourceLocation() noexcept = default;

        /// Creates a new source location with specified position data.
        ///
        /// @param line         1-indexed line number
        /// @param column       1-indexed column number
        /// @param absolute_pos 0-indexed byte offset
        ///
        /// @example
        /// auto loc = SourceLocation(3, 5, 20);
        /// assert(loc.line == 3);
        /// assert(loc.column == 5);
        /// assert(loc.absolute_pos == 20);
        constexpr SourceLocation(std::size_t p_line, std::size_t p_column, std::size_t p_absolute_pos) noexcept
          : line{p_line}, column{p_column}, absolute_pos{p_absolute_pos} {}

        /// Lexicographic ordering: line → column → absolute_pos.
        /// Automatically generates ==, !=, <, <=, >, >= from a single definition.
        [[nodiscard]] constexpr std::strong_ordering operator<=>(const SourceLocation &other) const noexcept = default;

        /// Format: `line:column (offset: absolute_pos)`
        /// Used by both std::formatter and fmt::formatter below.
        [[nodiscard]] std::string to_string() const;

        friend std::ostream &operator<<(std::ostream &os, const SourceLocation &loc);
    };
}  // namespace jsv

// -------------------------------------------------------------------------
// std::hash
// -------------------------------------------------------------------------
namespace std {
    template <> struct hash<jsv::SourceLocation> {
        [[nodiscard]] std::size_t operator()(const jsv::SourceLocation &loc) const noexcept;
    };

    // -------------------------------------------------------------------------
    // std::formatter  (C++23 <format>)
    // -------------------------------------------------------------------------
    template <> struct formatter<jsv::SourceLocation> : formatter<string> {
        template <typename FormatContext> auto format(const jsv::SourceLocation &loc, FormatContext &ctx) const {
            return formatter<string>::format(loc.to_string(), ctx);
        }
    };
}  // namespace std

// -------------------------------------------------------------------------
// fmt::formatter  (fmtlib)
// -------------------------------------------------------------------------
template <> struct fmt::formatter<jsv::SourceLocation> : fmt::formatter<std::string> {
    template <typename FormatContext> auto format(const jsv::SourceLocation &loc, FormatContext &ctx) const {
        return fmt::formatter<std::string>::format(loc.to_string(), ctx);
    }
};