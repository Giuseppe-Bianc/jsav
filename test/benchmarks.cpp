// clang-format off
// NOLINTBEGIN(*-include-cleaner, *-avoid-magic-numbers, *-magic-numbers, *-avoid-do-while,
//             *-use-anonymous-namespace, *-err58-cpp, *-function-cognitive-complexity, *-macro-usage)
// clang-format on

// Catch2 community rules applied here:
//   • [!benchmark] tag  — built-in Catch2 tag that skips these test cases during a normal
//                         `ctest` / `./benchmarks` run; they only execute when the user
//                         explicitly passes [!benchmark] or --benchmark-samples N.
//   • BENCHMARK lambdas must return a value — without a return the compiler is free to
//     treat the entire body as dead code and optimise it away, producing meaningless timings.
//   • This file has no REQUIRE assertions — benchmarks measure, tests verify.
//   • This target is NOT registered with catch_discover_tests in CMakeLists.txt because
//     CTest cannot enumerate individual BENCHMARK() invocations; registering the executable
//     that way produces spurious failures and misleading test counts.

#include "testsConstanst.hpp"
#include <catch2/benchmark/catch_benchmark.hpp>
#include <catch2/catch_test_macros.hpp>

// ==========================================================================
// Lexer performance benchmarks
// Run with:  ./benchmarks [!benchmark]
//        or: ./benchmarks  [!benchmark] --benchmark-samples 2500 --benchmark-warmup-time 3
// ==========================================================================

TEST_CASE("Benchmark_LargeAsciiFile", "[!benchmark][lexer][performance]") {
    // Build the source string once outside the hot loop; Catch2 executes
    // the BENCHMARK lambda many times for statistical accuracy.
    std::string src;
    src.reserve(std::size_t{10'000} * 8);
    for(int i = 0; i < 10'000; ++i) {
        src += 'x';
        src += std::to_string(i);
        src += ' ';
    }

    // Return the token vector so the compiler cannot eliminate the call.
    BENCHMARK("Tokenize 10K ASCII tokens") {
        jsv::Lexer bench_lex{src, "bench.vn"};
        return bench_lex.tokenize();  // return value prevents dead-code elimination
    };
}

TEST_CASE("Benchmark_MixedUnicodeFile", "[!benchmark][lexer][performance]") {
    // Mix of ASCII identifiers, CJK, Cyrillic, and string literals with emoji.
    std::string src;
    src.reserve(5'000);
    for(int i = 0; i < 100; ++i) {
        src += 'a';
        src += std::to_string(i);
        src += ' ';                           // ASCII identifier
        src += "\xe5\x8f\x98\xe9\x87\x8f ";   // 変量  (CJK, 3-byte sequences)
        src += "\xd0\xb0\xd0\xb1\xd0\xb2 ";   // абв   (Cyrillic, 2-byte sequences)
        src += "\"hello\xf0\x9f\x98\x80\" ";  // string literal containing emoji
    }

    BENCHMARK("Tokenize mixed Unicode") {
        jsv::Lexer bench_lex{src, "bench.vn"};
        return bench_lex.tokenize();  // return value prevents dead-code elimination
    };
}

// ==========================================================================
// LineTracker performance benchmarks
// Run with:  ./benchmarks [!benchmark]
// ==========================================================================

TEST_CASE("LineTracker construction benchmarks", "[!benchmark][LineTracker][performance]") {
    SECTION("Single line construction") {
        constexpr std::string_view source = "Hello, World!";

        BENCHMARK("Single line") { return jsv::LineTracker(source); };
    }

    SECTION("100 lines construction") {
        std::string source;
        source.reserve(100 * 20);
        for(int i = 0; i < 100; ++i) { source += "Line " + std::to_string(i) + "\n"; }

        BENCHMARK("100 lines") { return jsv::LineTracker(source); };
    }

    SECTION("1000 lines construction") {
        std::string source;
        source.reserve(1000 * 20);
        for(int i = 0; i < 1000; ++i) { source += "Line " + std::to_string(i) + "\n"; }

        BENCHMARK("1000 lines") { return jsv::LineTracker(source); };
    }

    SECTION("10000 lines construction") {
        std::string source;
        source.reserve(10000 * 20);
        for(int i = 0; i < 10000; ++i) { source += "Line " + std::to_string(i) + "\n"; }

        BENCHMARK("10000 lines") { return jsv::LineTracker(source); };
    }
}

TEST_CASE("LineTracker get_line access benchmarks", "[!benchmark][LineTracker][performance]") {
    SECTION("O(1) line access - 1000 lines") {
        std::string source;
        source.reserve(1000 * 20);
        for(int i = 0; i < 1000; ++i) { source += "Line " + std::to_string(i) + "\n"; }
        const jsv::LineTracker tracker(source);

        BENCHMARK("Sequential access 1000 lines") {
            std::size_t total = 0;
            for(std::size_t i = 1; i <= 1000; ++i) { total += tracker.get_line(i).size(); }
            return total;
        };
    }

    SECTION("O(1) line access - random access pattern") {
        std::string source;
        source.reserve(10000 * 20);
        for(int i = 0; i < 10000; ++i) { source += "Line " + std::to_string(i) + "\n"; }
        const jsv::LineTracker tracker(source);

        // Deterministic "random" access pattern for reproducibility
        BENCHMARK("Random access 10000 lines") {
            std::size_t total = 0;
            for(std::size_t i = 0; i < 10000; ++i) {
                // Pseudo-random but deterministic access pattern
                const std::size_t idx = (i * 7919 + 104729) % 10000 + 1;
                total += tracker.get_line(idx).size();
            }
            return total;
        };
    }
}

TEST_CASE("LineTracker Windows vs Unix newlines benchmark", "[!benchmark][LineTracker][performance]") {
    SECTION("Unix newlines (LF)") {
        std::string source;
        source.reserve(1000 * 20);
        for(int i = 0; i < 1000; ++i) { source += "Line " + std::to_string(i) + "\n"; }

        BENCHMARK("Parse Unix LF newlines") { return jsv::LineTracker(source); };
    }

    SECTION("Windows newlines (CRLF)") {
        std::string source;
        source.reserve(1000 * 21);  // Extra byte for \r
        for(int i = 0; i < 1000; ++i) { source += "Line " + std::to_string(i) + "\r\n"; }

        BENCHMARK("Parse Windows CRLF newlines") { return jsv::LineTracker(source); };
    }
}

// ==========================================================================
// ErrorReporter performance benchmarks
// Run with:  ./benchmarks [!benchmark]
// ==========================================================================

TEST_CASE("ErrorReporter spanned error formatting benchmark", "[!benchmark][ErrorReporter][performance]") {
    constexpr std::string_view source = "let x = 5;\nlet y = 10;\nlet z = 15;";
    const jsv::LineTracker tracker(source);
    const jsv::ErrorReporter reporter(tracker);
    const jsv::SourceSpan span("test.cpp", jsv::SourceLocation(2, 5, 13), jsv::SourceLocation(2, 10, 18));
    const jsv::CompileError error = jsv::CompileError::LexerError(std::nullopt, "Test error message"sv, span, std::nullopt);

    BENCHMARK("Format spanned error") { return reporter.report_errors(std::vector{error}); };
}

TEST_CASE("ErrorReporter multiple errors benchmark", "[!benchmark][ErrorReporter][performance]") {
    constexpr std::string_view source = "let x = 5;\nlet y = 10;\nlet z = 15;";
    const jsv::LineTracker tracker(source);
    const jsv::ErrorReporter reporter(tracker);

    SECTION("Report 10 errors") {
        std::vector<jsv::CompileError> errors;
        errors.reserve(10);
        for(int i = 0; i < 10; ++i) {
            const jsv::SourceSpan span("test.cpp", jsv::SourceLocation(1, 1, 0), jsv::SourceLocation(1, 5, 4));
            errors.emplace_back(jsv::CompileError::LexerError(std::nullopt, "Error"sv, span, std::nullopt));
        }

        BENCHMARK("Report 10 errors") { return reporter.report_errors(errors); };
    }

    SECTION("Report 100 errors") {
        std::vector<jsv::CompileError> errors;
        errors.reserve(100);
        for(int i = 0; i < 100; ++i) {
            const jsv::SourceSpan span("test.cpp", jsv::SourceLocation(1 + (C_ST(i) / 10), 1, 0),
                                       jsv::SourceLocation(1 + (C_ST(i) / 10), 5, 4));
            errors.emplace_back(jsv::CompileError::LexerError(std::nullopt, "Error"sv, span, std::nullopt));
        }

        BENCHMARK("Report 100 errors") { return reporter.report_errors(errors); };
    }
}

TEST_CASE("ErrorReporter with help messages benchmark", "[!benchmark][ErrorReporter][performance]") {
    constexpr std::string_view source = "let x = 5;";
    const jsv::LineTracker tracker(source);
    const jsv::ErrorReporter reporter(tracker);

    std::vector<jsv::CompileError> errors;
    errors.reserve(10);
    for(int i = 0; i < 10; ++i) {
        const jsv::SourceSpan span("test.cpp", jsv::SourceLocation(1, 1, 0), jsv::SourceLocation(1, 5, 4));
        errors.emplace_back(jsv::CompileError::LexerError(std::nullopt, "Error"sv, span, std::string("This is a helpful suggestion")));
    }

    BENCHMARK("Report 10 errors with help") { return reporter.report_errors(errors); };
}

// ==========================================================================
// Integration benchmarks: LineTracker + ErrorReporter
// Run with:  ./benchmarks [!benchmark]
// ==========================================================================

TEST_CASE("Integration benchmark: Complete error reporting pipeline", "[!benchmark][LineTracker][ErrorReporter][integration]") {
    // Realistic source code with multiple potential error locations
    constexpr std::string_view source_code = R"(fn main() {
    let x = 5;
    let y = @invalid;
    let z = 10;
    /* unterminated comment
       spans multiple lines
    return x + y + z;
})";

    BENCHMARK("Full pipeline: tokenize + error report") {
        const jsv::LineTracker tracker(source_code);
        const jsv::ErrorReporter reporter(tracker);

        const jsv::SourceSpan span("example.jsv", jsv::SourceLocation(3, 13, 25), jsv::SourceLocation(3, 14, 26));
        const jsv::CompileError error = jsv::CompileError::LexerError(jsv::ErrorCode::E0001, "Unrecognized character '@'"sv, span,
                                                                      std::string("Remove the '@' character"));

        return reporter.report_errors(std::vector{error});
    };
}

// clang-format off
// NOLINTEND(*-include-cleaner, *-avoid-magic-numbers, *-magic-numbers, *-avoid-do-while,
//           *-use-anonymous-namespace, *-err58-cpp, *-function-cognitive-complexity, *-macro-usage)
// clang-format on