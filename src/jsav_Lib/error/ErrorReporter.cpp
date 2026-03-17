/*
 * Created by gbian on 09/03/2026.
 * Copyright (c) 2026 All rights reserved.
 */

// NOLINTBEGIN(*-include-cleaner, *-uppercase-literal-suffix, *-uppercase-literal-suffix, *-identifier-length)
#include "jsav/error/ErrorReporter.hpp"

namespace jsv {
    constexpr auto ERROR_EST_SIZE = 256u;

    // ---------------------------------------------------------------------------
    // Helpers – local to this translation unit
    // ---------------------------------------------------------------------------

    namespace details {
        /// Builds the "[Exxxx] " fragment that optionally prefixes the category.
        /// Returns a single space when @p code is absent (mirrors the Rust
        /// `code.map_or_else(|| " ".to_string(), |c| format!(" [{}] ", …))` call).
        [[nodiscard]] std::string error_code_fragment(std::optional<ErrorCode> code) {
            if(!code.has_value()) { return " "; }
            return FORMAT(" [{}] ", ansi::red_bold(std::string{jsv::code(code.value())}));
        }

    }  // namespace details

    // ---------------------------------------------------------------------------
    // ErrorReporter::format_simple_error
    // ---------------------------------------------------------------------------
    //
    // Rust equivalent:
    //   fn format_simple_error(error_type, message, code) -> String {
    //       format!("{}{}{}: {}\n",
    //           style("ERROR").red().bold(),
    //           code.map_or_else(|| " ", |c| format!(" [{}] ", style(c).red().bold())),
    //           style(error_type).red(),
    //           style(message).yellow())
    //   }
    //
    // ---------------------------------------------------------------------------
    std::string ErrorReporter::format_simple_error(std::string_view error_type, std::string_view msg, std::optional<ErrorCode> code) {
        return FORMAT("{}{}{}: {}\n", ansi::red_bold("ERROR"), details::error_code_fragment(code), ansi::red(error_type),
                      ansi::yellow(msg));
    }

    // ---------------------------------------------------------------------------
    // ErrorReporter::build_underline
    // ---------------------------------------------------------------------------
    //
    // Builds the caret underline for a single source line annotation.
    //
    //   • Single-line span  →  <leading spaces> + <caret_count × '^'>
    //     Uses marker_extents() for Unicode-aware column calculation.
    //     Falls back to byte-based arithmetic on decoding failure.
    //
    //   • Multi-line span   →  <leading spaces> + '^'
    //
    // ---------------------------------------------------------------------------
    std::string ErrorReporter::build_underline(std::string_view source_line, const SourceSpan &span) const {
        const std::size_t start_col = span.start.column;
        const std::size_t end_col = span.end.column;

        // --- Multi-line span: single caret at start position -------------------
        if(span.start.line != span.end.line) {
            const std::size_t start_offset = (start_col > 0u) ? (start_col - 1u) : 0u;
            const std::string underline = FORMAT("{:>{}}^", "", start_offset);
            return config_.ansi_color ? ansi::red_bold(underline) : underline;
        }

        // --- Single-line span: Unicode-aware marker extents --------------------
        const std::size_t start_column = (start_col > 0) ? start_col : 1;
        const std::size_t end_column = (end_col > 0) ? end_col : start_column + 1;

        const std::size_t start_byte_in_line = start_column - 1;
        const std::size_t end_byte_in_line = end_column - 1;

        auto extents_result = marker_extents(source_line, start_byte_in_line, end_byte_in_line, config_.tab_stop_width);

        // Fallback for encoding errors: use byte-based calculation
        if(!extents_result.has_value()) {
            const std::size_t start_offset = (start_col > 0u) ? (start_col - 1u) : 0u;
            const std::size_t length = (end_col > start_col) ? (end_col - start_col) : 1u;
            return FORMAT("{:>{}}{:^>{}}", "", start_offset, "", length);
        }

        const auto [leading_spaces, caret_count] = *extents_result;
        std::string underline = FORMAT("{:>{}}", "", leading_spaces);

        if(config_.ansi_color) {
            for(std::size_t i = 0; i < caret_count; ++i) { underline += ansi::red_bold("^"); }
        } else {
            underline.append(caret_count, '^');
        }

        return underline;
    }

    // ---------------------------------------------------------------------------
    // ErrorReporter::append_encoding_note
    // ---------------------------------------------------------------------------
    //
    // FR-025, FR-026: For UTF-8 validation errors, appends a `note:` block
    // with byte offset and line number.  Performs a case-insensitive keyword
    // scan of the error message; silently returns when no encoding-related
    // keyword is found or when the source line is empty.
    //
    // ---------------------------------------------------------------------------
    void ErrorReporter::append_encoding_note(std::string &output, std::string_view msg, const SourceSpan &span,
                                             std::string_view source_line) {
        if(source_line.empty()) { return; }

        std::string msg_lower(msg);
        std::ranges::transform(msg_lower, msg_lower.begin(), [](unsigned char c) { return C_C(std::tolower(c)); });

        const bool is_encoding = msg_lower.find("utf-8") != std::string::npos || msg_lower.find("encoding") != std::string::npos ||
                                 msg_lower.find("null byte") != std::string::npos || msg_lower.find("overlong") != std::string::npos ||
                                 msg_lower.find("surrogate") != std::string::npos;
        if(!is_encoding) { return; }

        // Build enhanced error message with Unicode code point if applicable
        std::string enhanced(msg);
        if(msg_lower.find("null byte") != std::string::npos) {
            enhanced += " (U+0000)";
        } else if(msg_lower.find("surrogate") != std::string::npos) {
            enhanced += " (U+D800–U+DFFF)";
        }

        const std::size_t byte_offset = span.start.absolute_pos;
        const std::size_t start_line = span.start.line;

        auto out = std::back_inserter(output);
        FORMAT_TO(out, "     │\n");
        FORMAT_TO(out, "     │ {} {} {}, {} {}\n", ansi::blue("note:"), enhanced, ansi::cyan("at byte offset"),
                  ansi::yellow(FORMAT("{}", byte_offset)), ansi::cyan(FORMAT("line {}", start_line)));
    }

    // ---------------------------------------------------------------------------
    // ErrorReporter::format_spanned_error
    // ---------------------------------------------------------------------------
    //
    // Rust equivalent: `fn format_error(&self, category, message, span, help, code)`
    //
    // Full output (with source line available):
    //
    //   ERROR [E0001] LEX: <message>
    //   Location: <SourceSpan>
    //      3 │ let x = @bad;
    //        │         ^
    //   help: <help>
    //
    // Multi-line span:
    //
    //     20 │ /* unterminated comment
    //        │ ^
    //        │ ... (error spans lines 20-25)
    //
    // When the LineTracker returns an empty view for the line (e.g. mock errors
    // whose line numbers don't correspond to the loaded source) the source-line
    // block is silently omitted, matching the Rust `unwrap_or_default` behaviour.
    //
    // ---------------------------------------------------------------------------
    std::string ErrorReporter::format_spanned_error(std::string_view category, const CompileError &error) const {
        const std::optional<ErrorCode> error_code = error.error_code();
        const std::string_view msg = error.message();
        const SourceSpan &span = error.span();
        const std::string_view source_line = line_tracker_.get_line(span.start.line);

        std::string output;
        output.reserve(ERROR_EST_SIZE + msg.size() + source_line.size());
        auto out = std::back_inserter(output);

        // --- Header: ERROR [Exxxx] CATEGORY: message ----------------------------
        //             Location: <span>
        FORMAT_TO(out, "{}{}{}: {}\n{} {}\n", ansi::red_bold("ERROR"), details::error_code_fragment(error_code), ansi::red(category),
                  ansi::yellow(msg), ansi::blue("Location:"), ansi::cyan(FORMAT("{}", span)));

        // --- Source-line block (only when the tracker found the line) -----------
        if(!source_line.empty()) {
            FORMAT_TO(out, "{:4} │ {}\n", span.start.line, source_line);
            FORMAT_TO(out, "     │ {}\n", build_underline(source_line, span));

            if(span.start.line != span.end.line) {
                FORMAT_TO(out, "     │ {} (error spans lines {}-{})\n", ansi::blue("..."), span.start.line, span.end.line);
            }
        }

        // --- Encoding error note (FR-025, FR-026) ------------------------------
        append_encoding_note(output, msg, span, source_line);

        // --- Help line (optional) -----------------------------------------------
        if(const auto help_opt = error.help(); help_opt.has_value()) {
            FORMAT_TO(out, "{} {}\n", ansi::blue_bold("help:"), ansi::green(**help_opt));
        }

        return output;
    }

    // ---------------------------------------------------------------------------
    // ErrorReporter::report_errors
    // ---------------------------------------------------------------------------
    //
    // Rust equivalent:
    //   pub fn report_errors(&self, errors: Vec<CompileError>) -> String { … }
    //
    // Iterates over every error, dispatches to the appropriate formatter, and
    // concatenates the results into a single diagnostic string.
    //
    // ---------------------------------------------------------------------------
    std::string ErrorReporter::report_errors(std::span<const CompileError> errors) const {
        static constexpr std::size_t k_per_error_budget = 256u;
        std::string output;
        output.reserve(errors.size() * k_per_error_budget);  // rough per-error budget

        for(const CompileError &error : errors) {
            switch(error.kind()) {
            case CompileError::Kind::LexerError:
                output += format_spanned_error("LEX", error);
                break;

                // --- Future kinds (currently commented-out in CompileError.hpp) ---
                // case CompileError::Kind::SyntaxError:
                //     output += format_spanned_error("SYNTAX", error);
                //     break;
                // case CompileError::Kind::TypeError:
                //     output += format_spanned_error("TYPE", error);
                //     break;
                // case CompileError::Kind::IrGeneratorError:
                //     output += format_spanned_error("IR GEN", error);
                //     break;
                // case CompileError::Kind::AsmGeneratorError:
                //     output += format_simple_error("ASM GEN", error.message(), error.error_code());
                //     break;
                // case CompileError::Kind::IoError:
                //     output += format_simple_error("I/O", error.message(), std::nullopt);
                //     break;

            default:
                // Safety net: treat unknown kinds as simple errors so that new
                // variants added to the enum are at least reported rather than
                // silently swallowed.
                output += format_simple_error("UNKNOWN", error.message(), error.error_code());
                break;
            }
        }

        return output;
    }

}  // namespace jsv

// NOLINTEND(*-include-cleaner, *-uppercase-literal-suffix, *-uppercase-literal-suffix, *-identifier-length)