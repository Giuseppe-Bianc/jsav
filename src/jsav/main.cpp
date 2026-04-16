/*
 * Created by gbian on 19/02/2026.
 * Copyright (c) 2026 All rights reserved.
 */
// NOLINTBEGIN(*-include-cleaner, *-env33-c)
#include "Costanti.hpp"
// clang-format off
#ifdef _WIN32
#include <windows.h>
#endif
// clang-format on
DISABLE_WARNINGS_PUSH(
    4005 4201 4459 4514 4625 4626 4820 6244 6285 6385 6386 26408 26409 26415 26418 26426 26429 26432 26437 26438 26440 26446 26447 26450 26451 26455 26457 26459 26460 26461 26462 26467 26472 26473 26474 26475 26481 26482 26485 26490 26491 26493 26494 26495 26496 26497 26498 26800 26814 26818 26821 26826 26827)
#include <CLI/CLI.hpp>
#include <iostream>
#include <string>

DISABLE_WARNINGS_POP()

namespace {

    struct MemberInfo {
        std::string_view name;
        std::string_view type;
        std::size_t offset;
        std::size_t size;
        std::size_t alignment;
    };

    template <typename T, std::size_t N> void print_layout(std::string_view type_name, const std::array<MemberInfo, N> &members) {
        if constexpr(!std::is_standard_layout_v<T>) {
            fmt::print("\n=== {} ===\n\n", type_name);
            fmt::print("// Layout unavailable: type is not standard-layout, so offsetof-based analysis is not well-defined.\n");
            return;
        }

        fmt::print("\n=== {} ===\n\n", type_name);
        fmt::print("[Layout Calculation: {}]\n", type_name);

        std::size_t wasted_padding = 0;
        std::size_t previous_end = 0;

        for(const auto &member : members) {
            const std::size_t padding_before = member.offset - previous_end;
            wasted_padding += padding_before;

            fmt::print("  offset {} : {} -> size {}, align {}, padding_before {}, ends at {}\n", member.offset, member.name, member.size,
                       member.alignment, padding_before, member.offset + member.size);

            previous_end = member.offset + member.size;
        }

        const std::size_t raw_end = previous_end;
        const std::size_t type_align = alignof(T);
        const std::size_t trailing_padding = (type_align - (raw_end % type_align)) % type_align;
        wasted_padding += trailing_padding;

        fmt::print("  raw_end = {}, type_align = {}, trailing_padding = {}\n", raw_end, type_align, trailing_padding);
        fmt::print("  sizeof({}) = {}, alignof({}) = {}, wasted = {}\n\n", type_name, sizeof(T), type_name, alignof(T), wasted_padding);

        fmt::print("Member Layout:\n");
        fmt::print("┌─────────────────────┬───────────────────────┬──────────────┬────────┬─────────────────┐\n");
        fmt::print("│ Member              │ Type                  │ Size (bytes) │ Offset │ Padding before  │\n");
        fmt::print("├─────────────────────┼───────────────────────┼──────────────┼────────┼─────────────────┤\n");

        previous_end = 0;
        for(const auto &member : members) {
            const std::size_t padding_before = member.offset - previous_end;
            fmt::print("│ {:<19} │ {:<21} │ {:>12} │ {:>6} │ {:>15} │\n", member.name, member.type, member.size, member.offset,
                       padding_before);
            previous_end = member.offset + member.size;
        }

        fmt::print("├─────────────────────┼───────────────────────┼──────────────┼────────┼─────────────────┤\n");
        fmt::print("│ {:<19} │ {:<21} │ {:>12} │ {:>6} │ {:>15} │\n", "(trailing padding)", "—", trailing_padding, raw_end, "—");
        fmt::print("└─────────────────────┴───────────────────────┴──────────────┴────────┴─────────────────┘\n\n");

        fmt::print("Size:                 {} bytes\n", sizeof(T));
        fmt::print("Alignment:            {} bytes\n", alignof(T));
        fmt::print("Wasted padding space: {} bytes\n", wasted_padding);
    }

    template <typename T> void print_layout_unavailable(std::string_view type_name, std::string_view reason) {
        fmt::print("\n=== {} ===\n\n", type_name);
        fmt::print("// {}\n", reason);
        fmt::print("Size:                 {} bytes\n", sizeof(T));
        fmt::print("Alignment:            {} bytes\n", alignof(T));
        fmt::print("Wasted padding space: n/a\n");
    }

#define MEMBER(TYPE, FIELD, FIELD_TYPE)                                                                                                    \
    MemberInfo {                                                                                                                           \
        #FIELD, FIELD_TYPE, offsetof(TYPE, FIELD), sizeof(std::remove_reference_t<decltype(((TYPE *)nullptr)->FIELD)>),                    \
            alignof(std::remove_reference_t<decltype(((TYPE *)nullptr)->FIELD)>)                                                           \
    }

    void print_layout_report() {
        print_layout<jsv::SourceLocation>(
            "jsv::SourceLocation",
            std::to_array<MemberInfo>({MEMBER(jsv::SourceLocation, line, "std::size_t"), MEMBER(jsv::SourceLocation, column, "std::size_t"),
                                       MEMBER(jsv::SourceLocation, absolute_pos, "std::size_t")}));

        print_layout<jsv::SourceSpan>("jsv::SourceSpan", std::to_array<MemberInfo>({MEMBER(jsv::SourceSpan, file_path, "std::string_view"),
                                                                                    MEMBER(jsv::SourceSpan, start, "jsv::SourceLocation"),
                                                                                    MEMBER(jsv::SourceSpan, end, "jsv::SourceLocation")}));

        print_layout<jsv::unicode::Utf8DecodeResult>(
            "jsv::unicode::Utf8DecodeResult",
            std::to_array<MemberInfo>({MEMBER(jsv::unicode::Utf8DecodeResult, codepoint, "char32_t"),
                                       MEMBER(jsv::unicode::Utf8DecodeResult, byte_length, "std::uint8_t"),
                                       MEMBER(jsv::unicode::Utf8DecodeResult, status, "jsv::unicode::Utf8Status")}));

        print_layout<jsv::unicode::CodepointRange>("jsv::unicode::CodepointRange",
                                                   std::to_array<MemberInfo>({MEMBER(jsv::unicode::CodepointRange, first, "char32_t"),
                                                                              MEMBER(jsv::unicode::CodepointRange, last, "char32_t")}));

        print_layout<jsv::ErrorDisplayConfig>("jsv::ErrorDisplayConfig",
                                              std::to_array<MemberInfo>({MEMBER(jsv::ErrorDisplayConfig, tab_stop_width, "std::size_t"),
                                                                         MEMBER(jsv::ErrorDisplayConfig, ansi_color, "bool")}));

        print_layout<jsv::ErrorInfo>(
            "jsv::ErrorInfo", std::to_array<MemberInfo>(
                                  {MEMBER(jsv::ErrorInfo, code, "const char *"), MEMBER(jsv::ErrorInfo, numeric_code, "uint16_t"),
                                   MEMBER(jsv::ErrorInfo, severity, "jsv::Severity"), MEMBER(jsv::ErrorInfo, phase, "jsv::CompilerPhase"),
                                   MEMBER(jsv::ErrorInfo, message, "const char *"), MEMBER(jsv::ErrorInfo, explanation, "const char *"),
                                   MEMBER(jsv::ErrorInfo, suggestions, "std::span<const char *const>")}));

        print_layout<SizeSystem>(
            "SizeSystem", std::to_array<MemberInfo>({MEMBER(SizeSystem, name, "std::string_view"), MEMBER(SizeSystem, base, "long double"),
                                                     MEMBER(SizeSystem, prefixes, "std::array<std::string_view, 6>")}));

        print_layout<FormattedSize>("FormattedSize", std::to_array<MemberInfo>({MEMBER(FormattedSize, value, "long double"),
                                                                                MEMBER(FormattedSize, suffix, "std::string_view")}));

        print_layout<FileSizeInfo>("FileSizeInfo", std::to_array<MemberInfo>({MEMBER(FileSizeInfo, bytes, "uintmax_t")}));

        print_layout<FormattedSizePair>("FormattedSizePair", std::to_array<MemberInfo>({MEMBER(FormattedSizePair, si, "FormattedSize"),
                                                                                        MEMBER(FormattedSizePair, iec, "FormattedSize")}));

        print_layout_unavailable<FileSizeReport>(
            "FileSizeReport", "Layout unavailable: contains reference members; field storage is implementation-defined in this report.");

        print_layout_unavailable<vnd::Timer>(
            "vnd::Timer", "Layout unavailable: non-standard-layout class with private/protected members and deleted special members.");
        print_layout_unavailable<vnd::AutoTimer>(
            "vnd::AutoTimer",
            "Layout unavailable: class hierarchy with base subobject; member-level offsetof analysis is not exposed here.");
        print_layout_unavailable<jsv::Token>("jsv::Token",
                                             "Layout unavailable: members are private; this report does not bypass access control.");
        print_layout_unavailable<jsv::LineTracker>("jsv::LineTracker",
                                                   "Layout unavailable: members are private; this report does not bypass access control.");
        print_layout_unavailable<jsv::ErrorReporter>(
            "jsv::ErrorReporter", "Layout unavailable: members are private; this report does not bypass access control.");
        print_layout_unavailable<jsv::Lexer>("jsv::Lexer",
                                             "Layout unavailable: members are private; this report does not bypass access control.");
        print_layout_unavailable<jsv::Parser>("jsv::Parser",
                                              "Layout unavailable: members are private; this report does not bypass access control.");
        print_layout_unavailable<jsv::TypeChecker>("jsv::TypeChecker",
                                                   "Layout unavailable: members are private; this report does not bypass access control.");
        print_layout_unavailable<jsv::TypedAstPrinter>(
            "jsv::TypedAstPrinter",
            "Layout unavailable: visitor-based class hierarchy; member-level offsetof analysis is not exposed here.");
        print_layout_unavailable<jsv::TypeCheckResult>(
            "jsv::TypeCheckResult", "Layout unavailable: non-standard-layout aggregate containing complex non-standard-layout members.");
    }

#undef MEMBER

}  // namespace

std::string toLower(std::string str) {
    std::ranges::transform(str, str.begin(), [](unsigned char chr) -> char { return C_C(std::tolower(C_UC(chr))); });
    return str;
}

bool hasExtensionVN(const fs::path &pth) { return toLower(pth.extension().string()) == ".vn"; }

DISABLE_WARNINGS_PUSH(26461 26821)
// static inline constexpr auto sequence = std::views::iota(0, 9999);
// NOLINTNEXTLINE(*-function-cognitive-complexity, *-exception-escape)
auto main(int argc, const char *const argv[]) -> int {
    // NOLINTNEXTLINE
    INIT_LOG();
    const vnd::AutoTimer compilationTime("Total Execution");
#ifdef _WIN32
    const vnd::Timer winConsoleTimer("Windows console setup");
    // Set UTF-8 code page for Windows console
    SetConsoleCP(CP_UTF8);
    SetConsoleOutputCP(CP_UTF8);

    // Enable virtual terminal processing (ANSI escape codes) on both
    // stdout and stderr — each handle requires its own SetConsoleMode call.
    for(const DWORD handle_id : {STD_OUTPUT_HANDLE, STD_ERROR_HANDLE}) {
        // NOLINTNEXTLINE(*-identifier-length)
        if(HANDLE h = GetStdHandle(handle_id); h != INVALID_HANDLE_VALUE && h != nullptr) {
            DWORD dwMode = 0;
            if(GetConsoleMode(h, &dwMode) != 0) {
                dwMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
                SetConsoleMode(h, dwMode);  // non-fatal if this fails
            }
        }
    }
    LINFO("{}", winConsoleTimer);
#endif
    try {
        CLI::App app{FORMAT("{} version {}", jsav::cmake::project_name, jsav::cmake::project_version)};  // NOLINT(*-include-cleaner)
        // std::optional<std::string> message;  // NOLINT(*-include-cleaner)
        std::optional<std::string> path;
        // app.add_option("-m,--message", message, "A message to print back out");
        app.add_option("-i,--input", path, "The input file");
        bool show_version = false;
        bool show_layout_report = false;
        bool compile = false;
        // bool run = false;
        // bool clean = false;
        // bool create_cmake = false;
        app.add_flag("--version, -v", show_version, "Show version information");
        app.add_flag("--layout-report", show_layout_report, "Print a memory layout report for selected public types");
        app.add_flag("--compile, -c", compile, "Compile the resulting code");
        // app.add_flag("--run, -r", run, "Compile the resulting code and execute it");
        // app.add_flag("--clean, -x", clean, "Clean before building");
        // app.add_flag("--cmake, -m", create_cmake, "Create a CMakeLists.txt file");
        CLI11_PARSE(app, argc, argv)
        if(show_version) {
            LINFO("{}", jsav::cmake::project_version);
            return EXIT_SUCCESS;
        }
        if(show_layout_report) {
            print_layout_report();
            return EXIT_SUCCESS;
        }
        const auto porfilename = fs::canonical(fs::path(path.value_or(filename.data())).lexically_normal()).string();

        if(!hasExtensionVN(fs::path(porfilename))) {
            LERROR("File {} does not have the expected .vn extension", porfilename);
            return EXIT_FAILURE;
        }

        const vnd::Timer timer(FORMAT("Processing file {}", porfilename));
        const auto str = vnd::readFromFile(porfilename);
        const auto processing_time = timer.to_string();
        LINFO(processing_time);

        const std::string_view code(str);
        // LineTracker indexes the source once (O(n)); all get_line calls are O(1).
        // ErrorReporter takes ownership — source buffer (str) must outlive reporter.
        const jsv::ErrorReporter reporter{jsv::LineTracker{code}};
        const auto size_bytes = str.size();
        const FileSizeReport report{
            .info = {.bytes = size_bytes},
            .si_sys = kSI,
            .iec_sys = kIEC,
        };
        LINFO("File: {}\n{}", porfilename, report);
        jsv::Lexer lexer{code, porfilename};
        const vnd::Timer tokenizationTimer("Tokenization");
        const auto [tokens, errors] = lexer.tokenize();
        LINFO("{}", tokenizationTimer);
        LINFO("num tokens {}", tokens.size());

        // for(jsv::Token token : tokens) { LINFO("{}", token); }
        if(!errors.empty()) {
            const std::string diagnostic = reporter.report_errors(errors);
            // fmt::print(stderr, "{}", diagnostic);
            fmt::print("{}", diagnostic);
        }

        // jsv::AstPrinter tree_printer;
        jsv::TypedAstPrinter typed_tree_printer;
        jsv::Parser parser{tokens};
        const vnd::Timer parsingTimer("Parsing");
        const auto [parsed_program, parse_errors] = parser.parse();
        LINFO("{}", parsingTimer);
        if(!parse_errors.empty()) {
            const std::string diagnostic = reporter.report_errors(parse_errors);
            fmt::print("{}", diagnostic);
        }

        // Type checking phase
        jsv::TypeChecker type_checker;
        const vnd::Timer typeCheckingTimer("Type Checking");
        auto [typed_program, type_errors] = type_checker.check(*parsed_program);
        LINFO("{}", typeCheckingTimer);
        if(!type_errors.empty()) {
            const std::string diagnostic = reporter.report_errors(type_errors);
            fmt::print("{}", diagnostic);
        }

        // Print both untyped and typed ASTs
        // tree_printer.print(*parsed_program); // Uncomment to print untyped AST

        const vnd::Timer typedAstPrintTimer("TypedAST Printing");
        typed_tree_printer.print(typed_program);
        LINFO("{}", typedAstPrintTimer);
    } catch(const std::exception &e) {
        // Handle any other types of exceptions
        LERROR("Unhandled exception in main: {}", e.what());
    } catch(...) {
        // Handle any other types of exceptions
        LERROR("An unknown error occurred while creating the folder.");
    }
    return EXIT_SUCCESS;  // Return appropriate exit code
}

DISABLE_WARNINGS_POP()
// NOLINTEND(*-include-cleaner, *-env33-c)
