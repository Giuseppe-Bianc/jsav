/*
 * Created by gbian on 2 aprile 2026.
 * Copyright (c) 2026 All rights reserved.
 */
// NOLINTBEGIN(*-include-cleaner, *-identifier-length)
#include "jsav/typechecker/Constraint.hpp"

namespace jsv {

    ConstraintId ConstraintSet::add(TypePtr lhs, TypePtr rhs, SourceSpan origin, std::string_view reason) {
        auto id = next_id_++;
        index_by_id_.insert_or_assign(id, constraints_.size());
        constraints_.push_back(
            Constraint{.id = id, .lhs = std::move(lhs), .rhs = std::move(rhs), .origin = origin, .reason = std::string{reason}});
        return id;
    }

    [[nodiscard]] const std::vector<Constraint> &ConstraintSet::constraints() const noexcept { return constraints_; }

    [[nodiscard]] const Constraint *ConstraintSet::get(ConstraintId id) const noexcept {
        const auto it = index_by_id_.find(id);
        if (it == index_by_id_.end()) {
            return nullptr;
        }
        return &constraints_[it->second];
    }

    [[nodiscard]] std::size_t ConstraintSet::size() const noexcept { return constraints_.size(); }

}  // namespace jsv

// NOLINTEND(*-include-cleaner, *-identifier-length)