/*
 * Created by gbian on 25/04/2026.
 * Copyright (c) 2026 All rights reserved.
 */

// NOLINTBEGIN(*-include-cleaner)
#pragma once

#include "../error/CompileError.hpp"
#include "../ir/GlobalEntityId.hpp"

namespace jsv {

    struct CompileErrorCanonicalLess {
        [[nodiscard]] bool operator()(const CompileError &lhs, const CompileError &rhs) const noexcept;
    };

    [[nodiscard]] std::vector<CompileError> sort_compile_errors_canonical(std::vector<CompileError> errors);
    [[nodiscard]] bool is_canonical_error_order(std::span<const CompileError> errors) noexcept;

}  // namespace jsv
// NOLINTEND(*-include-cleaner)
