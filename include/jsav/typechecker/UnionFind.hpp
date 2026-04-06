/*
 * Created by gbian on 2 aprile 2026.
 * Copyright (c) 2026 All rights reserved.
 */
#pragma once

// clang-format off
#include <cstdint>
#include <unordered_map>
// clang-format on

namespace jsv {

using TypeVarId = std::size_t;

/**
 * @brief Union-Find data structure for type variable unification.
 *
 * Implements disjoint-set with path compression and union by rank
 * for O(α(n)) amortized time per operation.
 *
 * Used during constraint solving to track which type variables
 * have been unified.
 */
class UnionFind {
public:
    /// Create a new set containing only this element
    void make_set(TypeVarId var);

    /// Find the representative of the set containing var (with path compression)
    [[nodiscard]] TypeVarId find(TypeVarId var);

    /// Union the sets containing x and y (with union by rank)
    void unite(TypeVarId x, TypeVarId y);

    /// Check if two variables are in the same set
    [[nodiscard]] bool same_set(TypeVarId x, TypeVarId y);

    /// Number of elements tracked
    [[nodiscard]] std::size_t size() const noexcept;

private:
    std::unordered_map<TypeVarId, TypeVarId> parent_;
    std::unordered_map<TypeVarId, std::uint8_t> rank_;
};

}  // namespace jsv
