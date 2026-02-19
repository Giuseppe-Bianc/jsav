/*
* Created by gbian on 19/02/2026.
* Copyright (c) 2026 All rights reserved.
*/
// NOLINTBEGIN(*-include-cleaner, *-avoid-pragma-once)
#pragma once

#include "jsav/jsav.hpp"

// NOLINTNEXTLINE(bugprone-exception-escape, readability-function-cognitive-complexity)
#ifdef _WIN32  // Windows
#ifdef __MINGW32__
constexpr std::string_view filename = R"(..\..\vn_files\input.vn)";  // windows mingw form editor, use this when building for mingw
#elifdef __clang__
constexpr std::string_view filename = R"(..\..\..\vn_files\input.vn)";  // windows mingw form editor, use this when building for clang
#else
constexpr std::string_view filename = R"(..\..\..\vn_files\input.vn)";
#endif
#elif defined __unix__  // Linux and Unix-like systems
// constexpr std::string_view filename = "../../../vn_files/input.vn";  // Linux and Unix  form editor
constexpr std::string_view filename = "../../vn_files/input.vn";  // Linux and Unix
#endif

// NOLINTEND(*-include-cleaner, *-avoid-pragma-once)