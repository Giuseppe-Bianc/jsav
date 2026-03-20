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

constexpr int kExampleIntValue = 42;
constexpr double kExamplePiValue = 3.14;
constexpr int kMultiVarFirst = 12;
constexpr int kMultiVarSecond = 21;
constexpr int kCallArgValue = 8;
constexpr int kGroupingMultiplier = 2;

// ============================================================
// Helper: costruisce AST manualmente (senza parser)
// ============================================================
std::unique_ptr<jsv::Program> build_manual_ast() {
    using namespace jsv;
    std::vector<StmtPtr> stmts;

    // var x: int = 42;
    stmts.push_back(
        std::make_unique<VarDecl>("x", std::optional<std::string>{"int"}, std::make_unique<IntegerLiteral>(kExampleIntValue), false));

    // const pi = 3.14;
    stmts.push_back(std::make_unique<VarDecl>("pi", std::nullopt, std::make_unique<FloatLiteral>(kExamplePiValue), true));

    // Multi-variable declaration: var a2, b2: i64 = 12, 21;
    {
        std::vector<std::string> names{"a2", "b2"};
        std::vector<ExprPtr> initializers;
        initializers.reserve(2);
        initializers.push_back(std::make_unique<IntegerLiteral>(kMultiVarFirst));
        initializers.push_back(std::make_unique<IntegerLiteral>(kMultiVarSecond));
        stmts.push_back(std::make_unique<VarDecl>(std::move(names), std::optional<std::string>{"i64"}, std::move(initializers), false));
    }

    // fn add(a: int, b: int) -> int {
    //     return a + b;
    // }
    {
        std::vector<StmtPtr> body_stmts;
        body_stmts.push_back(std::make_unique<ReturnStmt>(
            std::make_unique<BinaryExpr>(BinaryOp::Add, std::make_unique<Identifier>("a"), std::make_unique<Identifier>("b"))));

        auto body = std::make_unique<BlockStmt>(std::move(body_stmts));
        std::vector<FuncParam> params{{.name = "a", .type = Type::I32, .loc = {}}, {.name = "b", .type = Type::I32, .loc = {}}};

        stmts.push_back(std::make_unique<FuncDecl>("add", std::move(params), std::optional<Type>{Type::I32}, std::move(body)));
    }

    // print add(x, 8);
    {
        std::vector<ExprPtr> args;
        args.push_back(std::make_unique<Identifier>("x"));
        args.push_back(std::make_unique<IntegerLiteral>(kCallArgValue));

        stmts.push_back(std::make_unique<PrintStmt>(std::make_unique<CallExpr>(std::make_unique<Identifier>("add"), std::move(args))));
    }

    // Grouping expression example: (a2 + b2) * 2
    {
        // var result: int = ((a2 + b2) * 2);
        auto grouping_inner = std::make_unique<GroupingExpr>(
            std::make_unique<BinaryExpr>(BinaryOp::Add, std::make_unique<Identifier>("a2"), std::make_unique<Identifier>("b2")));

        auto grouping_outer = std::make_unique<GroupingExpr>(
            std::make_unique<BinaryExpr>(BinaryOp::Mul, std::move(grouping_inner), std::make_unique<IntegerLiteral>(kGroupingMultiplier)));

        stmts.push_back(std::make_unique<VarDecl>("result", std::optional<std::string>{"int"}, std::move(grouping_outer), false));

        // print result;
        stmts.push_back(std::make_unique<PrintStmt>(std::make_unique<Identifier>("result")));
    }

    return std::make_unique<Program>(std::move(stmts));
}

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

        //for(jsv::Token token : tokens) { LINFO("{}", token); }
        if(!errors.empty()) {
            LERROR("Lexer produced {} error(s)", errors.size());
            const std::string diagnostic = reporter.report_errors(errors);
            // fmt::print(stderr, "{}", diagnostic);
            fmt::print("{}", diagnostic);
        }
    

        auto program = build_manual_ast();

        jsv::AstPrinter tree_printer;
        tree_printer.print(*program);
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
