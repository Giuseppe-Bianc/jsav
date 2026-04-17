/*
 * Created by gbian on 2 aprile 2026.
 * Copyright (c) 2026 All rights reserved.
 */
// NOLINTBEGIN(*-include-cleaner, *-identifier-length)
#include "jsav/typechecker/UnionFind.hpp"

namespace jsv {

    void UnionFind::make_set(TypeVarId var) { nodes_.try_emplace(var, Node{var, 0u}); }

    TypeVarId UnionFind::find(TypeVarId var) {
        // PERF: iterative two-pass path compression replaces the previous recursive
        // implementation.  The recursive version risked stack overflow on long chains
        // and performed three hash lookups per stack frame.  Two passes over the chain
        // achieve identical O(α(n)) amortised complexity with O(1) stack usage and
        // one map lookup per step.

        // Pass 1 — walk up to the root without modifying parent pointers.
        TypeVarId root = var;
        while(nodes_.at(root).parent != root) { root = nodes_.at(root).parent; }

        // Pass 2 — full path compression: point every node in the chain directly to root.
        while(nodes_.at(var).parent != root) {
            const TypeVarId next = nodes_.at(var).parent;
            nodes_.at(var).parent = root;
            var = next;
        }

        return root;
    }

    void UnionFind::unite(TypeVarId x, TypeVarId y) {
        const TypeVarId root_x = find(x);
        const TypeVarId root_y = find(y);

        if(root_x == root_y) { return; }

        // Union by rank — attach the shorter tree under the taller one.
        auto &nx = nodes_.at(root_x);
        auto &ny = nodes_.at(root_y);

        if(nx.rank < ny.rank) {
            nx.parent = root_y;
        } else if(nx.rank > ny.rank) {
            ny.parent = root_x;
        } else {
            ny.parent = root_x;
            ++nx.rank;
        }
    }

    bool UnionFind::same_set(TypeVarId x, TypeVarId y) {
        if(!nodes_.contains(x) || !nodes_.contains(y)) { return false; }
        return find(x) == find(y);
    }

    std::size_t UnionFind::size() const noexcept { return nodes_.size(); }

}  // namespace jsv

// NOLINTEND(*-include-cleaner, *-identifier-length)