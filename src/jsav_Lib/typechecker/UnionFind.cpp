/*
 * Created by gbian on 2 aprile 2026.
 * Copyright (c) 2026 All rights reserved.
 */
#include "jsav/typechecker/UnionFind.hpp"

namespace jsv {

void UnionFind::make_set(TypeVarId var) {
    if(parent_.find(var) == parent_.end()) {
        parent_[var] = var;
        rank_[var] = 0;
    }
}

TypeVarId UnionFind::find(TypeVarId var) {
    // Path compression: point directly to root
    if(parent_.at(var) != var) {
        parent_[var] = find(parent_.at(var));
    }
    return parent_.at(var);
}

void UnionFind::unite(TypeVarId x, TypeVarId y) {
    auto root_x = find(x);
    auto root_y = find(y);

    if(root_x == root_y) { return; }

    // Union by rank
    if(rank_.at(root_x) < rank_.at(root_y)) {
        parent_[root_x] = root_y;
    } else if(rank_.at(root_x) > rank_.at(root_y)) {
        parent_[root_y] = root_x;
    } else {
        parent_[root_y] = root_x;
        rank_[root_x]++;
    }
}

bool UnionFind::same_set(TypeVarId x, TypeVarId y) {
    if(parent_.find(x) == parent_.end() || parent_.find(y) == parent_.end()) { return false; }
    return find(x) == find(y);
}

std::size_t UnionFind::size() const noexcept { return parent_.size(); }

}  // namespace jsv
