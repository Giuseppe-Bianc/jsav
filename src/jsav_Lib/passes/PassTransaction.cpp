/*
 * Created by gbian on 25/04/2026.
 * Copyright (c) 2026 All rights reserved.
 */
// NOLINTBEGIN(*-include-cleaner)
#include "jsav/passes/PassTransaction.hpp"

namespace jsv {

    // Simplified IrUnit for initial implementation
    // Full implementation will replace this
    class IrUnit {
    public:
        IrUnit() = default;
        virtual ~IrUnit() = default;
        IrUnit(const IrUnit&) = default;
        IrUnit& operator=(const IrUnit&) = default;
    };

    PassTransaction::PassTransaction(const IrUnit& input)
        : state_(State::Open), pending_errors_() {
        // Create deep copy of input for original
        original_ = std::make_unique<IrUnit>(input);
        // Create working copy
        working_copy_ = std::make_unique<IrUnit>(input);
    }

    IrUnit& PassTransaction::working_copy() noexcept {
        return *working_copy_;
    }

    const IrUnit& PassTransaction::original() const noexcept {
        return *original_;
    }

    PassResult<IrUnit> PassTransaction::commit() noexcept {
        if (state_ != State::Open) {
            ErrorBatch errors;
            return std::unexpected(errors);
        }

        // In production: validate postconditions and merge working copy
        state_ = State::Committed;
        return *working_copy_;
    }

    void PassTransaction::rollback() noexcept {
        if (state_ == State::Open) {
            working_copy_ = std::make_unique<IrUnit>(*original_);
            state_ = State::RolledBack;
        }
    }

    bool PassTransaction::has_pending_changes() const noexcept {
        return state_ == State::Open;
    }

    bool PassTransaction::is_committed() const noexcept {
        return state_ == State::Committed;
    }

    PassTransaction::State PassTransaction::state() const noexcept {
        return state_;
    }

}  // namespace jsv
// NOLINTEND(*-include-cleaner)