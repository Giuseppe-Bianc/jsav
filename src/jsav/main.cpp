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
/*namespace vnd {
    // NOLINTNEXTLINE(*-use-anonymous-namespace)
    static auto timeTokenizer(Tokenizer &tokenizer, std::vector<TokenVec> &tokens) -> void {
        tokens.clear();
#ifdef INDEPT
        const AutoTimer timer("tokenization");
#endif
        tokens = tokenizer.tokenize();
    }
    // NOLINTNEXTLINE(*-use-anonymous-namespace)
    void count_total_num_tokens(const std::vector<vnd::TokenVec> &tokens) {
        vnd::AutoTimer const timer("Counting total number of tokens");
        const std::size_t totalTokenSize = std::accumulate(tokens.begin(), tokens.end(), std::size_t{0},
                                                           [](std::size_t sum, const vnd::TokenVec &inner) { return sum + inner.size(); });
        LINFO("num tokens {}", totalTokenSize);
    }

}  // namespace vnd

*/

// ============================================================
// Helper: costruisce AST manualmente (senza parser)
// ============================================================
std::unique_ptr<jsv::Program> build_manual_ast() {
    using namespace jsv;
    std::vector<StmtPtr> stmts;

    // var x: int = 42;
    stmts.push_back(std::make_unique<VarDecl>("x", std::optional<std::string>{"int"}, std::make_unique<IntegerLiteral>(42), false));

    // const pi = 3.14;
    stmts.push_back(std::make_unique<VarDecl>("pi", std::nullopt, std::make_unique<FloatLiteral>(3.14), true));

    // Multi-variable declaration: var a2, b2: i64 = 12, 21;
    {
        std::vector<std::string> names{"a2", "b2"};
        std::vector<ExprPtr> initializers;
        initializers.reserve(2);
        initializers.push_back(std::make_unique<IntegerLiteral>(12));
        initializers.push_back(std::make_unique<IntegerLiteral>(21));
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
        std::vector<FuncParam> params{{"a", "int"}, {"b", "int"}};

        stmts.push_back(std::make_unique<FuncDecl>("add", std::move(params), std::optional<std::string>{"int"}, std::move(body)));
    }

    // print add(x, 8);
    {
        std::vector<ExprPtr> args;
        args.push_back(std::make_unique<Identifier>("x"));
        args.push_back(std::make_unique<IntegerLiteral>(8));

        stmts.push_back(std::make_unique<PrintStmt>(std::make_unique<CallExpr>(std::make_unique<Identifier>("add"), std::move(args))));
    }

    // Grouping expression example: (a2 + b2) * 2
    {
        // var result: int = ((a2 + b2) * 2);
        auto grouping_inner = std::make_unique<GroupingExpr>(
            std::make_unique<BinaryExpr>(BinaryOp::Add, std::make_unique<Identifier>("a2"), std::make_unique<Identifier>("b2")));

        auto grouping_outer = std::make_unique<GroupingExpr>(
            std::make_unique<BinaryExpr>(BinaryOp::Mul, std::move(grouping_inner), std::make_unique<IntegerLiteral>(2)));

        stmts.push_back(std::make_unique<VarDecl>("result", std::optional<std::string>{"int"}, std::move(grouping_outer), false));

        // print result;
        stmts.push_back(std::make_unique<PrintStmt>(std::make_unique<Identifier>("result")));
    }

    return std::make_unique<Program>(std::move(stmts));
}

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
        /*if(clean) {
            const auto folderPath = vnd::GetBuildFolder(fs::path(porfilename));
            LINFO("Cleaning the project");
#ifdef INDEPT
            const vnd::Timer timer("Cleaning of the project");
#endif
            auto folderDeleted = vnd::FolderDeletionResult::deleteFolder(folderPath);
#ifdef INDEPT
            const auto folder_delition_time = timer.to_string();
#endif
            if(folderDeleted.success()) {
#ifdef INDEPT
                LINFO(folder_delition_time);
#else

                LINFO("Cleaning of the project done");
#endif
            }
        }
        */

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

        for(jsv::Token token : tokens) { LINFO("{}", token); }
        // -----------------------------------------------------------------
        // SourceLocation(line, column, absolute_pos)   ← actual constructor
        // SourceSpan(file_path, start, end)            ← actual constructor
        //
        // file_path is a string_view: we use porfilename which is already in scope
        // (or a literal if a completely autonomous mock is desired).
        // -----------------------------------------------------------------
        const std::string_view mock_file = "src/example.jsv";

        // --- Error 1 ─ invalid token (E0001), single-line span -------
        //   line 3, col 5, offset 42  →  line 3, col 6, offset 43
        const jsv::SourceLocation e1_start{3, 5, 42};
        const jsv::SourceLocation e1_end{3, 6, 43};
        const jsv::SourceSpan e1_span{mock_file, e1_start, e1_end};

        // --- Error 2 ─ unterminated string (E0005), with help --------------
        //   line 7, col 12, offset 110  →  line 7, col 13, offset 111
        const jsv::SourceLocation e2_start{7, 12, 110};
        const jsv::SourceLocation e2_end{7, 13, 111};
        const jsv::SourceSpan e2_span{mock_file, e2_start, e2_end};

        // --- Error 3 ─ invalid escape sequence (E0007), with help ------
        //   line 15, col 3, offset 280  →  line 15, col 5, offset 282
        const jsv::SourceLocation e3_start{15, 3, 280};
        const jsv::SourceLocation e3_end{15, 5, 282};
        const jsv::SourceSpan e3_span{mock_file, e3_start, e3_end};

        // --- Error 4 ─ unterminated multi-line comment (E0008) -----------
        //   starts line 20 col 1 offset 400 → ends line 25 col 1 offset 520
        const jsv::SourceLocation e4_start{20, 1, 400};
        const jsv::SourceLocation e4_end{25, 1, 520};
        const jsv::SourceSpan e4_span{mock_file, e4_start, e4_end};

        // -----------------------------------------------------------------
        // Building CompileError via factory
        // -----------------------------------------------------------------
        std::vector<jsv::CompileError> mock_errors;

        // E0001 – no help
        mock_errors.push_back(jsv::CompileError::LexerError(jsv::ErrorCode::E0001, "unrecognized '@' character", e1_span, std::nullopt));

        // // E0005 – with help
        // mock_errors.push_back(jsv::CompileError::LexerError(jsv::ErrorCode::E0005, "unterminated string starting with '\"'", e2_span,
        //                                                     std::string{R"(add '"' at the end of the literal: "hello world")"}));
        //
        // // E0007 – with help containing backslash (raw string for safety)
        // mock_errors.push_back(jsv::CompileError::LexerError(jsv::ErrorCode::E0007, R"(invalid escape sequence '\q')", e3_span,
        //                                                     std::string{R"(valid sequences: \n \t \\ \" \' \0 \u{XXXX})"}));
        //
        // // E0008 – multi-line span, with help
        // mock_errors.push_back(jsv::CompileError::LexerError(jsv::ErrorCode::E0008, "unterminated multi-line comment '/*'", e4_span,
        //                                                     std::string{"add '*/' to close the comment"}));

        const std::string diagnostic = reporter.report_errors(mock_errors);

        // Colored diagnostics to stderr (compiler convention).
        // Alternative with project logger: LERROR("{}", diagnostic);
        // fmt::print(stderr, "{}", diagnostic);
        fmt::print("{}", diagnostic);

        auto program = build_manual_ast();

        std::cout << "--- Tree view ---\n";
        jsv::AstPrinter tree_printer;
        tree_printer.print(*program);
        // LINFO("{}", code);
        /*vnd::Tokenizer tokenizer{code, porfilename};
        std::vector<vnd::TokenVec> tokens;
        vnd::timeTokenizer(tokenizer, tokens);
        LINFO("num tokens {}", tokens.size());
        LINFO("Input:\n{}", code);
        vnd::Parser parser{code, "input.vn"};
        for(const auto progrmamAST = vnd::timeParse(parser); const auto &statement : progrmamAST) {
            const auto &node = statement.get_root();
            const auto &token = statement.get_token();
            if(token.getType() == vnd::TokenType::UNKNOWN) { LINFO("the statement is not generated by any token"); }
            if(node != nullptr) {
                LINFO("{}", token);
                prettyPrint(*node);
            } else {
                LINFO("EMPTY STATMENT generated from {}", token.compat_to_string());
            }
        }
        vnd::Transpiler transpiler{code, porfilename, create_cmake};
        LINFO("transpiled code: \n{}", transpiler.transpile());*/
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
