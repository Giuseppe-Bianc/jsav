/*
 * Created by gbian on 2 aprile 2026.
 * Copyright (c) 2026 All rights reserved.
 */
#pragma once

// clang-format off
#include "../headers.hpp"
// clang-format on

namespace jsv {

    /// @brief Unique identifier for a type variable in the unification system.
    using TypeVarId = std::size_t;

    /**
     * @brief Union-Find data structure for type variable unification.
     *
     * Implements disjoint-set with path compression and union by rank
     * for O(α(n)) amortized time per operation.
     *
     * Used during constraint solving to track which type variables
     * have been unified.
     *
     * ### Const-correctness note
     * `find()` and `same_set()` are conceptually read-only queries but cannot
     * be `const`-qualified: path compression rewrites `parent_` entries as a
     * side effect of every lookup. This is an implementation detail that
     * preserves logical equality (the representative does not change) while
     * mutating physical state. Marking `parent_` as `mutable` would allow
     * `const` methods but is intentionally avoided here — it would hide
     * real mutations from thread-safety analysis tools. Callers should treat
     * `find()` / `same_set()` as logically non-mutating despite the
     * non-`const` signature.
     */
    class UnionFind {
    public:
        /// Create a new set containing only this element.
        void make_set(TypeVarId var);

        /**
         * @brief Find the representative of the set containing @p var.
         *
         * Applies path compression on every call, flattening the tree for
         * amortised O(α(n)) performance. Path compression rewrites internal
         * parent pointers, so this method cannot be `const` even though the
         * logical (observable) representative is unchanged.
         *
         * @param var Must have been previously registered via make_set().
         * @return Canonical representative of the set.
         */
        [[nodiscard]] TypeVarId find(TypeVarId var);

        /// Union the sets containing @p x and @p y (union by rank).
        void unite(TypeVarId x, TypeVarId y);

        /**
         * @brief Check if two variables belong to the same equivalence class.
         *
         * Delegates to `find()` for both arguments, so path compression
         * applies and the method cannot be `const`. See `find()` for details.
         */
        [[nodiscard]] bool same_set(TypeVarId x, TypeVarId y);

        /// Number of elements tracked.
        [[nodiscard]] std::size_t size() const noexcept;

    private:
        std::unordered_map<TypeVarId, TypeVarId> parent_;
        std::unordered_map<TypeVarId, std::uint8_t> rank_;
    };

}  // namespace jsv
