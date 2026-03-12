/*
 * Created by gbian on 12/03/2026.
 * Copyright (c) 2026 All rights reserved.
 */

#pragma once

#include "../headers.hpp"
#include "../location/LineTracker.hpp"
#include "CompileError.hpp"

namespace jsv {

    // -------------------------------------------------------------------------
    /// @class ErrorReporter
    /// @brief Formats a list of CompileError objects into human-readable
    ///        diagnostic strings, optionally with ANSI colour.
    ///
    /// The reporter holds a const reference to a LineTracker so it can pull
    /// the source line for each error span.  The LineTracker must therefore
    /// outlive the ErrorReporter.
    // -------------------------------------------------------------------------
    class ErrorReporter {
    public:
        // --- Construction ----------------------------------------------------

        /// @brief Create a reporter backed by the given line tracker.
        /// @param line_tracker  Indexed source lines (must outlive this object).
        /// @param use_color     Emit ANSI colour escapes (default: true).
        explicit ErrorReporter(const LineTracker &line_tracker, bool use_color = true) noexcept;

        // --- Primary API -----------------------------------------------------

        /// @brief Format every error in `errors` and return the complete output.
        /// @param errors  The errors produced by a compilation pipeline stage.
        /// @return A multi-line string ready for printing to stderr / a log.
        [[nodiscard]] std::string report_errors(const std::vector<CompileError> &errors) const;

        /// @brief Format a single CompileError.
        [[nodiscard]] std::string report_error(const CompileError &error) const;

    private:
        // --- Internal helpers ------------------------------------------------

        /// Format an error that carries a source span (LEX, SYNTAX, …).
        [[nodiscard]] std::string format_spanned_error(std::string_view category, const CompileError &error) const;

        /// Format an error that has no span (ASM GEN, I/O).
        [[nodiscard]] std::string format_simple_error(std::string_view category, const CompileError &error) const;

        // --- Colour helpers --------------------------------------------------

        /// Wrap `text` with an ANSI SGR sequence when colour is enabled.
        [[nodiscard]] std::string colorize(std::string_view text, std::string_view ansi_code) const;
        // Convenience colour wrappers matching the Rust `console::style` calls.
        [[nodiscard]] std::string red_bold(std::string_view t) const;
        [[nodiscard]] std::string yellow(std::string_view t) const;
        [[nodiscard]] std::string blue(std::string_view t) const;
        [[nodiscard]] std::string blue_bold(std::string_view t) const;
        [[nodiscard]] std::string cyan(std::string_view t) const;
        [[nodiscard]] std::string green(std::string_view t) const;

        // --- Data members ----------------------------------------------------
        const LineTracker &line_tracker_;
        bool use_color_;
    };

}  // namespace jsv