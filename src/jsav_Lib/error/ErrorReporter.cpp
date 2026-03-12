// NOLINTBEGIN(*-include-cleaner)
#include "jsav/error/ErrorReporter.hpp"

namespace jsv {

    // =========================================================================
    // ANSI colour codes
    // =========================================================================
    namespace ansi {
        inline constexpr std::string_view kReset = "\033[0m";
        inline constexpr std::string_view kRedBold = "\033[1;31m";
        inline constexpr std::string_view kYellow = "\033[33m";
        inline constexpr std::string_view kBlue = "\033[34m";
        inline constexpr std::string_view kBlueBold = "\033[1;34m";
        inline constexpr std::string_view kCyan = "\033[36m";
        inline constexpr std::string_view kGreen = "\033[32m";
    }  // namespace ansi

    // =========================================================================
    // Construction
    // =========================================================================

    ErrorReporter::ErrorReporter(const LineTracker &line_tracker, bool use_color) noexcept
      : line_tracker_(line_tracker), use_color_(use_color) {}

    // =========================================================================
    // Public API
    // =========================================================================

    std::string ErrorReporter::report_errors(const std::vector<CompileError> &errors) const {
        std::string output;
        output.reserve(errors.size() * 512);
        for(const CompileError &err : errors) { output += report_error(err); }
        return output;
    }

    std::string ErrorReporter::report_error(const CompileError &err) const {
        switch(err.kind()) {
        case CompileError::Kind::LexerError:
            return format_spanned_error("LEX", err);
            // Future kinds -- uncomment as they are added to the enum.
            // case CompileError::Kind::SyntaxError:      return format_spanned_error("SYNTAX",  err);
            // case CompileError::Kind::TypeError:        return format_spanned_error("TYPE",    err);
            // case CompileError::Kind::IrGeneratorError: return format_spanned_error("IR GEN",  err);
            // case CompileError::Kind::AsmGeneratorError:return format_simple_error ("ASM GEN", err);
            // case CompileError::Kind::IoError:          return format_simple_error ("I/O",     err);
        }
        return {};
    }

    // =========================================================================
    // Private — error formatters
    // =========================================================================

    // -------------------------------------------------------------------------
    // format_simple_error
    // Equivalent to the Rust `format_simple_error` free function.
    //
    // Output:  ERROR [E4001] ASM GEN: istruzione assembly non valida\n
    // -------------------------------------------------------------------------
    std::string ErrorReporter::format_simple_error(std::string_view category, const CompileError &error) const {
        std::string out;
        out.reserve(128);

        out += red_bold("ERROR");

        if(error.error_code().has_value()) {
            out += ' ';
            out += red_bold(fmt::format("[{}]", jsv::code(*error.error_code())));
            out += ' ';
        } else {
            out += ' ';
        }

        out += red_bold(category);
        out += ": ";
        out += yellow(error.message());
        out += '\n';

        return out;
    }

    // -------------------------------------------------------------------------
    // format_spanned_error
    // Equivalent to the Rust `ErrorReporter::format_error` method.
    //
    // Output:
    //   ERROR [E0005] LEX: letterale stringa non terminato
    //   Location: main.jsv:3:12
    //      3 │ var s = "hello
    //        │            ^
    //   help: Aggiungere virgolette doppie di chiusura
    //
    // -------------------------------------------------------------------------
    std::string ErrorReporter::format_spanned_error(std::string_view category, const CompileError &error) const {
        const SourceSpan &span = error.span();
        const std::size_t start_line = span.start.line;
        const std::size_t start_col = span.start.column;
        const std::size_t end_line = span.end.line;
        const std::size_t end_col = span.end.column;

        const std::string_view source_line = line_tracker_.get_line(start_line);

        // Estimate capacity.
        const std::size_t help_len = [&] {
            auto h = error.help();
            return h.has_value() ? (*h)->size() + 20 : 0;
        }();
        std::string out;
        out.reserve(100 + error.message().size() + category.size() + source_line.size() + help_len + 50);

        // ------------------------------------------------------------------
        // Header:  ERROR [EXXXX] CATEGORY: message
        // ------------------------------------------------------------------
        out += red_bold("ERROR");

        if(error.error_code().has_value()) {
            out += ' ';
            out += red_bold(fmt::format("[{}]", jsv::code(*error.error_code())));
            out += ' ';
        } else {
            out += ' ';
        }

        out += red_bold(category);
        out += ": ";
        out += yellow(error.message());
        out += '\n';

        // ------------------------------------------------------------------
        // Location:  Location: <span>
        // ------------------------------------------------------------------
        out += blue("Location:");
        out += ' ';
        out += cyan(fmt::format("{}", span));  // relies on SourceSpan's formatter
        out += '\n';

        // ------------------------------------------------------------------
        // Source context (only when we have the line text)
        // ------------------------------------------------------------------
        if(!source_line.empty()) {
            // "   3 │ var s = \"hello"
            out += fmt::format("{:4} \u2502 {}\n", start_line, source_line);

            // Build the underline ('^' characters).
            const std::size_t start_offset = (start_col > 0) ? start_col - 1 : 0;
            std::string underline;

            if(start_line == end_line) {
                // Single-line error: pad, then ^^^^…
                const std::size_t length = std::max(end_col - start_col, std::size_t{1});
                underline = fmt::format("{:{}}{}", "", start_offset, std::string(length, '^'));
            } else {
                // Multi-line: single caret at the start column
                underline = fmt::format("{:{}}^", "", start_offset);
            }

            // "     │ ^^^^"
            out += fmt::format("     \u2502 {}\n", red_bold(underline));

            // Multi-line annotation
            if(start_line != end_line) {
                out += fmt::format("     \u2502 {} (error spans lines {}-{})\n", blue("..."), start_line, end_line);
            }
        }

        // ------------------------------------------------------------------
        // Optional help line
        // ------------------------------------------------------------------
        auto help_opt = error.help();
        if(help_opt.has_value()) {
            out += blue_bold("help:");
            out += ' ';
            out += green(**help_opt);
            out += '\n';
        }

        // Blank separator between consecutive errors.
        out += '\n';
        return out;
    }

    // =========================================================================
    // Private — colour helpers
    // =========================================================================

    std::string ErrorReporter::colorize(std::string_view text, std::string_view ansi_code) const {
        if(!use_color_) { return std::string(text); }
        std::string s;
        s.reserve(ansi_code.size() + text.size() + ansi::kReset.size());
        s += ansi_code;
        s += text;
        s += ansi::kReset;
        return s;
    }

    std::string ErrorReporter::red_bold(std::string_view t) const { return colorize(t, ansi::kRedBold); }
    std::string ErrorReporter::yellow(std::string_view t) const { return colorize(t, ansi::kYellow); }
    std::string ErrorReporter::blue(std::string_view t) const { return colorize(t, ansi::kBlue); }
    std::string ErrorReporter::blue_bold(std::string_view t) const { return colorize(t, ansi::kBlueBold); }
    std::string ErrorReporter::cyan(std::string_view t) const { return colorize(t, ansi::kCyan); }
    std::string ErrorReporter::green(std::string_view t) const { return colorize(t, ansi::kGreen); }

}  // namespace jsv
// NOLINTEND(*-include-cleaner)