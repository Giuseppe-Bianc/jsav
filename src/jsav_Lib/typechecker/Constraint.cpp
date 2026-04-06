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

[[nodiscard]] const std::vector<Constraint> &ConstraintSet::constraints() const noexcept { return constraints_; }

[[nodiscard]] const Constraint *ConstraintSet::get(ConstraintId id) const noexcept {
    const auto it = std::ranges::find(constraints_, id, &Constraint::id);
    return it != constraints_.end() ? &*it : nullptr;
}

[[nodiscard]] std::size_t ConstraintSet::size() const noexcept { return constraints_.size(); }

}  // namespace jsv
