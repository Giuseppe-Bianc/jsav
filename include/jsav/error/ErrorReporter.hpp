/*
 * Created by gbian on 12/03/2026.
 * Copyright (c) 2026 All rights reserved.
 */

// NOLINTBEGIN(*-include-cleaner)
#pragma once

#include "../headers.hpp"
#include "../location/LineTracker.hpp"
#include "CompileError.hpp"

namespace jsv {

    // ---------------------------------------------------------------------------
    /// @brief ANSI terminal colour / style helpers
    ///
    /// Thin wrappers that apply ANSI escape sequences so that diagnostic output
    /// mirrors the coloured output produced by the Rust `console::style` calls
    /// in the original `ErrorReporter`.  Each helper returns a `std::string` so
    /// it can be composed freely with `FORMAT()` / `fmt`.
    ///
    /// Reset is always appended after the styled text so colours never "bleed"
    /// into the next token on the terminal line.
    // ---------------------------------------------------------------------------
    namespace ansi {

        inline constexpr std::string_view kReset = "\x1b[0m";
        inline constexpr std::string_view kBold = "\x1b[1m";
        inline constexpr std::string_view kRed = "\x1b[31m";
        inline constexpr std::string_view kYellow = "\x1b[33m";
        inline constexpr std::string_view kBlue = "\x1b[34m";
        inline constexpr std::string_view kCyan = "\x1b[36m";
        inline constexpr std::string_view kGreen = "\x1b[32m";

        /// Apply @p escape_code (an ANSI prefix) around @p text, then reset.
        /// Uses only `{}` specifiers so it is compatible with both the
        /// `fmt::format` and `std::format` backends exposed by FORMAT().
        [[nodiscard]] inline std::string styled(std::string_view text, std::string_view escape_code) {
            return FORMAT("{}{}{}", escape_code, text, kReset);
        }

        // --- convenience wrappers matching Rust `style(x).<colour>()` names ---
        // All calls use plain `{}` specifiers → valid for both fmt and std::format.

        /// @{ Plain-colour variants – equivalent of `style(x).red()` etc.
        [[nodiscard]] inline std::string red(std::string_view text) { return styled(text, kRed); }
        [[nodiscard]] inline std::string yellow(std::string_view text) { return styled(text, kYellow); }
        [[nodiscard]] inline std::string blue(std::string_view text) { return styled(text, kBlue); }
        [[nodiscard]] inline std::string cyan(std::string_view text) { return styled(text, kCyan); }
        [[nodiscard]] inline std::string green(std::string_view text) { return styled(text, kGreen); }
        /// @}

        /// @{ Bold+colour variants – equivalent of `style(x).red().bold()` etc.
        /// Four `{}` placeholders: bold-esc, colour-esc, text, reset – all
        /// standard specifiers, no fmt-only extension used.
        [[nodiscard]] inline std::string red_bold(std::string_view text) { return FORMAT("{}{}{}{}", kBold, kRed, text, kReset); }
        [[nodiscard]] inline std::string blue_bold(std::string_view text) { return FORMAT("{}{}{}{}", kBold, kBlue, text, kReset); }
        /// @}

    }  // namespace ansi

    // ---------------------------------------------------------------------------
    /// @class ErrorReporter
    /// @brief Formats compiler diagnostics into human-readable, coloured strings.
    ///
    /// Direct C++ port of the Rust `ErrorReporter` (`src/error/error_reporter.rs`).
    ///
    /// Holds a `LineTracker` built from the source text so that every spanned
    /// error can display the offending source line together with a caret underline,
    /// exactly as the Rust version does:
    ///
    /// @code
    ///   ERROR [E0001] LEX: carattere '@' non riconosciuto
    ///   Location: src/example.jsv:line 3:column 5 - line 3:column 6
    ///      3 │ let x = @bad;
    ///        │         ^
    ///   help: …
    /// @endcode
    ///
    /// For multi-line spans only the first line is shown; a `...` note is
    /// appended to indicate that the error continues:
    ///
    /// @code
    ///      20 │ /* unterminated
    ///         │ ^
    ///         │ ... (error spans lines 20-25)
    /// @endcode
    ///
    /// ### Error kinds and their format
    ///
    /// | CompileError::Kind          | category   | location | source line | help     |
    /// |-----------------------------|------------|:--------:|:-----------:|:--------:|
    /// | LexerError                  | `"LEX"`    | yes      | yes         | optional |
    /// | SyntaxError *(future)*      | `"SYNTAX"` | yes      | yes         | optional |
    /// | TypeError *(future)*        | `"TYPE"`   | yes      | yes         | optional |
    /// | IrGeneratorError *(future)* | `"IR GEN"` | yes      | yes         | optional |
    /// | AsmGeneratorError *(future)*| `"ASM GEN"`| no       | no          | no       |
    /// | IoError *(future)*          | `"I/O"`    | no       | no          | no       |
    // ---------------------------------------------------------------------------
    class ErrorReporter {
    public:
        /// @brief Construct a reporter backed by @p line_tracker.
        ///
        /// Mirrors Rust's `ErrorReporter::new(line_tracker: LineTracker)`.
        /// The `LineTracker` holds a non-owning `string_view` of the source;
        /// the caller must guarantee the source buffer outlives this object.
        ///
        /// @param line_tracker  A fully-constructed `LineTracker` for the source
        ///                      being compiled.
        explicit ErrorReporter(const LineTracker &line_tracker) noexcept : line_tracker_(line_tracker) {}

        ErrorReporter() = delete;
        ErrorReporter(const ErrorReporter &) = delete;
        ErrorReporter &operator=(const ErrorReporter &) = delete;
        ErrorReporter(ErrorReporter &&) = default;
        ErrorReporter &operator=(ErrorReporter &&) = default;
        ~ErrorReporter() = default;

        // -----------------------------------------------------------------------
        /// @brief Formats every error in @p errors into a single diagnostic string.
        ///
        /// The returned string is ready to be written to `stderr` or forwarded to
        /// a logger.  Each error is separated by a blank line for readability.
        ///
        /// @param errors  Span of compile errors to format (non-owning).
        /// @return A fully formatted, ANSI-coloured diagnostic string.
        // -----------------------------------------------------------------------
        [[nodiscard]] std::string report_errors(std::span<const CompileError> errors) const;

        // -----------------------------------------------------------------------
        /// @brief Convenience overload accepting a `std::vector`.
        // -----------------------------------------------------------------------
        [[nodiscard]] std::string report_errors(const std::vector<CompileError> &errors) const {
            return report_errors(std::span<const CompileError>{errors});
        }

    private:
        // -----------------------------------------------------------------------
        // Internal helpers
        // -----------------------------------------------------------------------

        /// @brief Formats a single error that carries a `SourceSpan` (Lexer /
        ///        Syntax / Type / IR variants).
        ///
        /// Output structure (mirrors `format_error` in the Rust source):
        /// @code
        ///   ERROR [E0001] LEX: <message>
        ///   Location: <file>:<line>:<col>
        ///   help: <help>          ← only when help is present
        /// @endcode
        ///
        /// @param category  Label shown after the error code (e.g. `"LEX"`).
        /// @param error     The compile error to format.
        [[nodiscard]] std::string format_spanned_error(std::string_view category, const CompileError &error) const;

        /// @brief Formats a simple, location-less error (AsmGen / I/O variants).
        ///
        /// Output structure (mirrors `format_simple_error` in the Rust source):
        /// @code
        ///   ERROR [E4001] ASM GEN: <message>
        /// @endcode
        ///
        /// @param error_type  Short label (e.g. `"ASM GEN"`, `"I/O"`).
        /// @param msg         Error message text.
        /// @param code        Optional error code for display.
        [[nodiscard]] static std::string format_simple_error(std::string_view error_type, std::string_view msg,
                                                             std::optional<ErrorCode> code);

        /// The line index built from the source text — used to retrieve the
        /// offending source line for display in `format_spanned_error`.
        LineTracker line_tracker_;
    };

}  // namespace jsv
// NOLINTEND(*-include-cleaner)