#include <catch2/catch_test_macros.hpp>
// clang-format off
// NOLINTBEGIN(*-include-cleaner, *-avoid-magic-numbers, *-magic-numbers, *-unchecked-optional-access, *-avoid-do-while, *-use-anonymous-namespace, *-qualified-auto, *-suspicious-stringview-data-usage, *-err58-cpp, *-function-cognitive-complexity, *-macro-usage, *-unnecessary-copy-initialization)
// clang-format on


#include <jsav/jsav.hpp>

/*
TEST_CASE("Factorials are computed with constexpr", "[factorial]")
{
  STATIC_REQUIRE(factorial_constexpr(0) == 1);
  STATIC_REQUIRE(factorial_constexpr(1) == 1);
  STATIC_REQUIRE(factorial_constexpr(2) == 2);
  STATIC_REQUIRE(factorial_constexpr(3) == 6);
  STATIC_REQUIRE(factorial_constexpr(10) == 3628800);
}*/

TEST_CASE("mock test", "[mock]") {
    STATIC_REQUIRE(1 + 2 == 3);
}

// clang-format off
// NOLINTEND(*-include-cleaner, *-avoid-magic-numbers, *-magic-numbers, *-unchecked-optional-access, *-avoid-do-while, *-use-anonymous-namespace, *-qualified-auto, *-suspicious-stringview-data-usage, *-err58-cpp, *-function-cognitive-complexity, *-macro-usage, *-unnecessary-copy-initialization)
// clang-format on
