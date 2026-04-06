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
        bool compile = false;
        // bool run = false;
        // bool clean = false;
        // bool create_cmake = false;
        app.add_flag("--version, -v", show_version, "Show version information");
        app.add_flag("--compile, -c", compile, "Compile the resulting code");
        // app.add_flag("--run, -r", run, "Compile the resulting code and execute it");
        // app.add_flag("--clean, -x", clean, "Clean before building");
        // app.add_flag("--cmake, -m", create_cmake, "Create a CMakeLists.txt file");
        CLI11_PARSE(app, argc, argv)
        if(show_version) {
            LINFO("{}", jsav::cmake::project_version);
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
            LERROR("Lexer produced {} error(s)", errors.size());
            const std::string diagnostic = reporter.report_errors(errors);
            // fmt::print(stderr, "{}", diagnostic);
            fmt::print("{}", diagnostic);
        }

        jsv::AstPrinter tree_printer;
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
            LERROR("Type checker produced {} error(s)", type_errors.size());
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
