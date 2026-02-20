/*
 * Created by gbian on 20/02/2026.
 * Copyright (c) 2026 All rights reserved.
 */

#pragma once

#include "../headers.hpp"
#include "SourceLocation.hpp"

namespace jsv {

    /// Represents a contiguous range of source code in a specific file.
    ///
    /// Spans track:
    /// - Source file path
    /// - Start position (inclusive)
    /// - End position (exclusive)
    ///
    /// Used for error reporting, source mapping, and semantic analysis.
    class SourceSpan {
    public:
        /// Path to source file (shared, immutable — mirrors Rust's Arc<str>)
        std::shared_ptr<const std::string> file_path;

        /// Starting position of the span (inclusive)
        SourceLocation start;

        /// Ending position of the span (exclusive)
        SourceLocation end;

        /// Default constructor — empty path, zero positions.
        SourceSpan() noexcept;

        /// Creates a new source span covering a specific range.
        ///
        /// @param file_path  Shared pointer to source file path string
        /// @param start      Starting position (inclusive)
        /// @param end        Ending position (exclusive)
        SourceSpan(
            std::shared_ptr<const std::string> file_path,
            const SourceLocation& start,
            const SourceLocation& end) noexcept;

        /// Merges another span into this one in-place.
        /// Only merges if spans are from the same file.
        void merge(const SourceSpan &other) noexcept;

        /// Creates a new span that combines this span with another.
        /// Returns std::nullopt if spans are from different files.
        [[nodiscard]] std::optional<SourceSpan> merged(const SourceSpan &other) const;

        /// Lexicographic ordering: file_path (by value) → start → end.
        [[nodiscard]] std::strong_ordering operator<=>(const SourceSpan &other) const noexcept;
        [[nodiscard]] bool operator==(const SourceSpan &other) const noexcept;

        /// Format: `[truncated_path]:line [sl]:column [sc] - line [el]:column [ec]`
        /// Shared by operator<<, std::formatter and fmt::formatter.
        [[nodiscard]] std::string to_string() const;

        friend std::ostream &operator<<(std::ostream &os, const SourceSpan &span);
    };

    /// Truncates a path to show only the last `depth` components.
    [[nodiscard]] std::string truncate_path(const std::filesystem::path &path, std::size_t depth);

    /// Abstract interface for types that carry a source span.
    class HasSpan {
    public:
        virtual ~HasSpan() = default;
        [[nodiscard]] virtual const SourceSpan &span() const noexcept = 0;
    };

}  // namespace jsv

// -------------------------------------------------------------------------
// std::hash
// -------------------------------------------------------------------------
namespace std {
    template <>
    struct hash<jsv::SourceSpan> {
        [[nodiscard]] std::size_t operator()(const jsv::SourceSpan &s) const noexcept;
    };

    // -------------------------------------------------------------------------
    // std::formatter  (C++23 <format>)
    // -------------------------------------------------------------------------
    template <>
    struct formatter<jsv::SourceSpan> : formatter<string> {
        template <typename FormatContext>
        auto format(const jsv::SourceSpan &span, FormatContext &ctx) const {
            return formatter<string>::format(span.to_string(), ctx);
        }
    };
}  // namespace std

// -------------------------------------------------------------------------
// fmt::formatter  (fmtlib)
// -------------------------------------------------------------------------
template <>
struct fmt::formatter<jsv::SourceSpan> : fmt::formatter<std::string> {
    template <typename FormatContext>
    auto format(const jsv::SourceSpan &span, FormatContext &ctx) const {
        return fmt::formatter<std::string>::format(span.to_string(), ctx);
    }
};