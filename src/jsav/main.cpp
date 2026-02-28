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
// NOLINTBEGIN(*-diagnostic-double-promotion, *-pro-bounds-constant-array-index, *-identifier-length)
static inline constexpr std::array<std::string_view, 5> UNITS = {"B", "KB", "MB", "GB", "TB"};
static inline constexpr std::size_t UNIT_LEN = UNITS.size() - 1;
static inline constexpr long double UNIT_DIVIDER = 1024.0L;
struct FormattedSize {
    long double value;
    std::string_view unit;

    [[nodiscard]] std::string to_string() const { return FORMAT("{} {}", value, unit); }
};

[[nodiscard]] constexpr FormattedSize format_size(std::size_t bytes) noexcept {
    auto size = C_LD(bytes);
    std::size_t unit = 0;

    while(size >= UNIT_DIVIDER && unit < UNIT_LEN) {
        size /= UNIT_DIVIDER;
        ++unit;
    }

    return {.value = size, .unit = UNITS[unit]};
}

template <> struct std::formatter<FormattedSize> : std::formatter<std::string> {
    template <typename FormatContext> auto format(const FormattedSize &fs, FormatContext &ctx) const {
        return std::formatter<std::string>::format(fs.to_string(), ctx);
    }
};
template <> struct fmt::formatter<FormattedSize> : fmt::formatter<std::string> {
    template <typename FormatContext> auto format(const FormattedSize &fs, FormatContext &ctx) const {
        return fmt::formatter<std::string>::format(fs.to_string(), ctx);
    }
};
// NOLINTEND(*-diagnostic-double-promotion, *-pro-bounds-constant-array-index, *-identifier-length)
DISABLE_WARNINGS_PUSH(26461 26821)
// static inline constexpr auto sequence = std::views::iota(0, 9999);
// NOLINTNEXTLINE(*-function-cognitive-complexity, *-exception-escape)
auto main(int argc, const char *const argv[]) -> int {
#ifdef _WIN32
    // Set UTF-8 code page for Windows console
    SetConsoleOutputCP(CP_UTF8);

    // Optional: enable virtual terminal processing for better Unicode/emoji support
    if(HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE); hOut != INVALID_HANDLE_VALUE && hOut != nullptr) {
        DWORD dwMode = 0;
        if(GetConsoleMode(hOut, &dwMode) != 0) {
            dwMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
            SetConsoleMode(hOut, dwMode);  // Failure here is non-fatal for UTF-8 output
        }
    }
#endif
    // NOLINTNEXTLINE
    INIT_LOG();

    LINFO("UTF-8 test: àèìòù ñ ü ß → ✓ 日本語 🎉");
    LINFO("Project: {}", jsav::cmake::project_name);
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

        const vnd::AutoTimer compilationTime("Total Execution");
        const vnd::Timer timer(FORMAT("Processing file {}", porfilename));
        const auto str = vnd::readFromFile(porfilename);
        const auto processing_time = timer.to_string();
        LINFO(processing_time);

        [[maybe_unused]] const std::string_view code(str);
        const auto size_bytes = str.size();
        const auto fsz = format_size(size_bytes);
        LINFO("{} total of bytes read: {}", porfilename, fsz);
        const jsv::SourceLocation start{0, 0, 0};
        const jsv::SourceLocation end{0, 1, 1};
        const auto sf_p = MAKE_SHARED(const std::string, porfilename);
        const jsv::SourceSpan source_span{sf_p, start, end};
        LINFO(source_span);
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
