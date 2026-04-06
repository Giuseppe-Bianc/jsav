/*
 * Created by gbian on 2 aprile 2026.
 * Copyright (c) 2026 All rights reserved.
 */
#include "jsav/typechecker/Constraint.hpp"

namespace jsv {

ConstraintId ConstraintSet::add(TypePtr lhs, TypePtr rhs, SourceSpan origin, std::string_view reason) {
    auto id = next_id_++;
    constraints_.push_back(Constraint{.id = id, .lhs = std::move(lhs), .rhs = std::move(rhs), .origin = std::move(origin), .reason = std::string{reason}});
    return id;
}

const std::vector<Constraint>& ConstraintSet::constraints() const noexcept { return constraints_; }

const Constraint* ConstraintSet::get(ConstraintId id) const noexcept {
    for(const auto& c : constraints_) {
        if(c.id == id) { return &c; }
    }
    return nullptr;
}

std::size_t ConstraintSet::size() const noexcept { return constraints_.size(); }

}  // namespace jsv
