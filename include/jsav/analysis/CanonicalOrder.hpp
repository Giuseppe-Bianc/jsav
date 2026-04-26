/*
 * Created by gbian on 25/04/2026.
 * Copyright (c) 2026 All rights reserved.
 */

// NOLINTBEGIN(*-include-cleaner)
#pragma once

#include "../error/CompileError.hpp"
#include "../ir/GlobalEntityId.hpp"
#include <vector>
#include <string_view>

namespace jsv {

    /// Utilities for canonical (deterministic) ordering of reports and errors
    class CanonicalOrder {
    public:
        /// Sort errors by canonical key for deterministic batch reporting
        static void sort_errors_canonical(std::vector<CompileError>& errors) noexcept;

        /// Sort errors by location (source position) for user-friendly output
        static void sort_errors_by_location(std::vector<CompileError>& errors) noexcept;

        /// Get canonical order key for an error (for comparison)
        static std::string_view get_error_key(const CompileError& error) noexcept;

        /// Compare two errors using canonical ordering
        static bool compare_canonical(const CompileError& a, const CompileError& b) noexcept;

        /// Sort entity IDs in canonical order
        static void sort_ids_canonical(std::vector<GlobalEntityId>& ids) noexcept;

        /// Deduplicate errors maintaining canonical order
        static void deduplicate_errors_canonical(std::vector<CompileError>& errors) noexcept;

        /// Verify that a vector of errors is in canonical order
        static bool is_in_canonical_order(const std::vector<CompileError>& errors) noexcept;
    };

}  // namespace jsv
// NOLINTEND(*-include-cleaner)