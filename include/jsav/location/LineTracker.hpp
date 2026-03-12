/*
 * Created by gbian on 12/03/2026.
 * Copyright (c) 2026 All rights reserved.
 */

#pragma once

/*
 * LineTracker.hpp
 * Indexes a source-code string_view into individual lines so that the error
 * reporter can retrieve any line by its 1-based line number.
 *
 * Lifetime contract
 * ─────────────────
 * LineTracker does NOT own the source text.  It stores a std::string_view
 * and all `get_line` results are views into that same buffer.  The caller
 * must guarantee that the underlying character array outlives both the
 * LineTracker and every string_view it returns.
 *
 * This matches the Lexer's own convention: the Lexer receives a string_view
 * of the full source and never copies it.
 */

#pragma once

#include "../headers.hpp"

namespace jsv {

    // -------------------------------------------------------------------------
    /// @class LineTracker
    /// @brief Splits a non-owning source view into indexed lines for
    ///        diagnostic display.
    ///
    /// Construction is O(n) in the length of the source.  All subsequent
    /// `get_line` calls are O(1) and return string_views into the original
    /// buffer — zero allocations after construction.
    ///
    /// Line numbers are **1-based** throughout, matching compiler convention.
    // -------------------------------------------------------------------------
    class LineTracker {
    public:
        /// @brief Construct an empty tracker (no source).
        LineTracker() = default;

        /// @brief Build a tracker from a source string_view.
        /// @param source  Non-owning view of the full source text.
        ///                The pointed-to memory must outlive this object.
        explicit LineTracker(std::string_view source);

        // --- Queries ---------------------------------------------------------

        /// @brief Return the text of a single source line (1-based).
        /// @param line_number  1-based line number.
        /// @return The line text without its trailing newline, or an empty
        ///         string_view when `line_number` is out of range.
        [[nodiscard]] std::string_view get_line(std::size_t line_number) const noexcept;

        /// @brief Total number of lines in the source.
        [[nodiscard]] std::size_t line_count() const noexcept;

        /// @brief True when no source has been loaded.
        [[nodiscard]] bool empty() const noexcept;

    private:
        /// Non-owning view of the full source text.
        std::string_view source_;

        /// Views into `source_`, one entry per source line (newlines stripped).
        std::vector<std::string_view> lines_;

        /// Walks `source_` once and populates `lines_`.
        void index_lines();
    };

}  // namespace jsv
