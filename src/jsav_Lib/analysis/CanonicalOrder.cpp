/*
 * Created by gbian on 25/04/2026.
 * Copyright (c) 2026 All rights reserved.
 */
// NOLINTBEGIN(*-include-cleaner)

#include "jsav/analysis/CanonicalOrder.hpp"

namespace jsv {

    bool CompileErrorCanonicalLess::operator()(const CompileError &lhs, const CompileError &rhs) const noexcept {
        if(lhs.span().file_path != rhs.span().file_path) { return lhs.span().file_path < rhs.span().file_path; }
        if(lhs.span().start != rhs.span().start) { return lhs.span().start < rhs.span().start; }
        if(lhs.span().end != rhs.span().end) { return lhs.span().end < rhs.span().end; }
        if(lhs.error_code().has_value() != rhs.error_code().has_value()) { return lhs.error_code().has_value(); }
        return lhs.message() < rhs.message();
    }

    std::vector<CompileError> sort_compile_errors_canonical(std::vector<CompileError> errors) {
        std::ranges::sort(errors, CompileErrorCanonicalLess{});
        return errors;
    }

    bool is_canonical_error_order(const std::span<const CompileError> errors) noexcept {
        return std::ranges::is_sorted(errors, CompileErrorCanonicalLess{});
    }

}  // namespace jsv
// NOLINTEND(*-include-cleaner)
