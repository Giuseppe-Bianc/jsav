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
     * Implements disjoint-set with iterative path compression (full two-pass)
     * and union by rank for O(α(n)) amortised time per operation.
     *
     * ### Layout change vs. original
     * `parent` and `rank` are now co-located in a single `Node` record stored
     * in one `unordered_map<TypeVarId, Node>`.  This eliminates the second
     * independent hash-map lookup that the previous dual-map design required on
     * every `make_set`, `find`, and `unite` call.
     *
     * ### Const-correctness note
     * `find()` and `same_set()` cannot be `const`-qualified: path compression
     * rewrites `Node::parent` entries as a side effect of every lookup. This
     * preserves logical equality (the representative does not change) while
     * mutating physical state. See original rationale in the class comment.
     */
    class UnionFind {
    public:
        /// Create a new singleton set containing only @p var.
        void make_set(TypeVarId var);

        /**
         * @brief Find the representative of the set containing @p var.
         *
         * Uses iterative two-pass path compression (find root, then flatten).
         * Avoids the recursive stack-overflow risk of the previous implementation
         * while achieving the same O(α(n)) amortised complexity.
         *
         * @param var Must have been previously registered via make_set().
         * @return Canonical representative of the set.
         */
        [[nodiscard]] TypeVarId find(TypeVarId var);

        /// Union the sets containing @p x and @p y (union by rank).
        void unite(TypeVarId x, TypeVarId y);

        /**
         * @brief Check if two variables belong to the same equivalence class.
         * Returns false if either variable was never registered.
         */
        [[nodiscard]] bool same_set(TypeVarId x, TypeVarId y);

        /// Number of elements tracked.
        [[nodiscard]] std::size_t size() const noexcept;

    private:
        // PERF: merged parent + rank into one record — single hash lookup per
        // make_set / find / unite call instead of two separate map lookups.
        struct Node {
            TypeVarId parent;
            std::uint8_t rank{0};
        };

        std::unordered_map<TypeVarId, Node> nodes_;
    };

}  // namespace jsv
