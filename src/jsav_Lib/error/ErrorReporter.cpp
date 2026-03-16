/*
 * Created by gbian on 09/03/2026.
 * Copyright (c) 2026 All rights reserved.
 */

// NOLINTBEGIN(*-include-cleaner, *-uppercase-literal-suffix, *-uppercase-literal-suffix)
#include "jsav/error/ErrorReporter.hpp"
#include "jsav/error/UnicodeColumn.hpp"

namespace jsv {

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
    // Unicode-aware column calculation:
    // Uses marker_extents() to calculate visual column positions by counting
    // Unicode code points (not bytes), ensuring correct marker alignment for
    // multi-byte characters. ANSI color is applied when config_.ansi_color is true.
    //
    // ---------------------------------------------------------------------------
    std::string ErrorReporter::format_spanned_error(std::string_view category, const CompileError &error) const {
        const std::optional<ErrorCode> error_code = error.error_code();
        const std::string_view msg = error.message();
        const SourceSpan &span = error.span();

        const std::size_t start_line = span.start.line;
        const std::size_t end_line = span.end.line;

        // Retrieve the offending source line (O(1), zero-copy view).
        // Mirrors: `self.line_tracker.get_line(start_line).unwrap_or_default()`
        const std::string_view source_line = line_tracker_.get_line(start_line);

        // Calculate byte offsets within the line for marker_extents()
        // SourceLocation::absolute_pos is the byte offset from the start of the source file
        // We need the byte offset from the start of the line
        // line_start_absolute = source_line.data() - line_tracker_.source().data()
        // start_byte_in_line = span.start.absolute_pos - line_start_absolute
        const std::size_t line_start_absolute = line_tracker_.source().data() - source_line.data();
        const std::size_t start_byte = span.start.absolute_pos - line_start_absolute;
        const std::size_t end_byte = span.end.absolute_pos - line_start_absolute;

        // Use UnicodeColumn::marker_extents() for Unicode-aware column calculation
        auto extents_result = marker_extents(source_line, start_byte, end_byte, config_.tab_stop_width);

        // Handle encoding errors (invalid UTF-8, null bytes, etc.)
        if(!extents_result.has_value()) {
            // Encoding error - display with byte-based calculation as fallback
            const std::size_t estimated = 100u + msg.size() + category.size() + (error_code.has_value() ? 12u : 0u)
                                          + 40u + source_line.size() + 20u + 20u;

            std::string output;
            output.reserve(estimated);
            auto out = std::back_inserter(output);

            FORMAT_TO(out, "{}{}{}: {}\n{} {}\n", ansi::red_bold("ERROR"), details::error_code_fragment(error_code),
                      ansi::red(category), ansi::yellow(msg), ansi::blue("Location:"), ansi::cyan(FORMAT("{}", span)));

            if(!source_line.empty()) {
                FORMAT_TO(out, "{:4} │ {}\n", start_line, source_line);
                // Byte-based fallback for encoding errors
                const std::size_t start_offset = start_byte;
                FORMAT_TO(out, "     │ {:>{}}^\n", "", start_offset);
            }

            // Add encoding error note with byte offset information
            const std::string error_msg = extents_result.error();
            FORMAT_TO(out, "{} {} encoding error: {}\n", ansi::blue("note:"), ansi::yellow(category), error_msg);

            if(const auto help_opt = error.help(); help_opt.has_value()) {
                FORMAT_TO(out, "{} {}\n", ansi::blue_bold("help:"), ansi::green(**help_opt));
            }

            return output;
        }

        const auto [leading_spaces, caret_count] = extents_result.value();

        const std::size_t estimated = 100u + msg.size() + category.size() + (error_code.has_value() ? 12u : 0u)  // "[Exxxx] "
                                      + 40u                                                                      // location line
                                      + source_line.size() + 20u                                                 // source line row
                                      + leading_spaces + caret_count + 10u                                       // underline row
                                      + (start_line != end_line ? 40u : 0u)                                      // multi-line note
                                      + (error.help().has_value() ? 20u : 0u);                                   // help line

        std::string output;
        output.reserve(estimated);
        auto out = std::back_inserter(output);

        // --- Header: ERROR [Exxxx] CATEGORY: message ----------------------------
        //             Location: <span>
        FORMAT_TO(out, "{}{}{}: {}\n{} {}\n", ansi::red_bold("ERROR"), details::error_code_fragment(error_code), ansi::red(category),
                  ansi::yellow(msg), ansi::blue("Location:"), ansi::cyan(FORMAT("{}", span)));

        // --- Source-line block (FR-012: show marker even for empty lines) -------
        // Check if line exists (line_number is valid) rather than if it's non-empty
        // Empty lines should still show the marker row per FR-012
        if(start_line <= line_tracker_.line_count()) {
            // "{start_line:4} │ {source_line}"
            // Rust: writeln!(output, "{start_line:4} │ {source_line}");
            FORMAT_TO(out, "{:4} │ {}\n", start_line, source_line);

            // Build the underline string with Unicode-aware positioning
            // Create caret string with leading spaces
            std::string underline(leading_spaces, ' ');
            underline.append(caret_count, '^');

            // Apply ANSI color if enabled (NFR-003)
            if(config_.ansi_color) {
                // Red carets: \033[31m<carets>\033[0m
                FORMAT_TO(out, "     │ {}\n", ansi::red_bold(underline));
            } else {
                // Plain carets (monochrome fallback)
                FORMAT_TO(out, "     │ {}\n", ansi::red_bold(underline));
            }

            // Multi-line note: "     │ ... (error spans lines X-Y)"
            if(start_line != end_line) { FORMAT_TO(out, "     │ {} (error spans lines {}-{})\n", ansi::blue("..."), start_line, end_line); }
        }

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

// NOLINTEND(*-include-cleaner, *-uppercase-literal-suffix, *-uppercase-literal-suffix)