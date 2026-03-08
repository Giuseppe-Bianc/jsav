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
//        or: ./benchmarks [!benchmark] --benchmark-samples 100
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

// clang-format off
// NOLINTEND(*-include-cleaner, *-avoid-magic-numbers, *-magic-numbers, *-avoid-do-while,
//           *-use-anonymous-namespace, *-err58-cpp, *-function-cognitive-complexity, *-macro-usage)
// clang-format on