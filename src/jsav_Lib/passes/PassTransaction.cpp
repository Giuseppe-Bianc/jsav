/*
 * Created by gbian on 25/04/2026.
 * Copyright (c) 2026 All rights reserved.
 */
// NOLINTBEGIN(*-include-cleaner)
#include "jsav/passes/PassTransaction.hpp"

namespace jsv {

    namespace {
        [[nodiscard]] CompileError make_transaction_error(std::string_view message) {
            const SourceLocation start(1, 1, 0);
            const SourceLocation end(1, 1, 0);
            const SourceSpan span("ir.transaction", start, end);
            return CompileError::IrGeneratorError(std::nullopt, message, span, std::nullopt);
        }
    }  // namespace

    PassTransaction::PassTransaction(IrUnit source_ir) : committed_ir_(source_ir), working_ir_(vnd_move(source_ir)) {}

    const IrUnit &PassTransaction::current() const noexcept { return committed_ir_; }

    IrUnit &PassTransaction::working_copy() noexcept { return working_ir_; }

    const IrUnit &PassTransaction::working_copy() const noexcept { return working_ir_; }

    PassExecutionStatus PassTransaction::commit(const PassResult<PassInvariantReport> &post_validation) {
        errors_.clear();

        if(!post_validation.has_value()) {
            errors_ = post_validation.error();
            rollback();
            return {false, true, errors_};
        }

        if(!post_validation.value().all_valid()) {
            errors_.push_back(make_transaction_error("Invariant validation failed in PassTransaction::commit"));
            rollback();
            return {false, true, errors_};
        }

        committed_ir_ = working_ir_;
        committed_ = true;
        rolled_back_ = false;
        return {true, false, {}};
    }

    void PassTransaction::rollback() noexcept {
        working_ir_ = committed_ir_;
        committed_ = false;
        rolled_back_ = true;
    }

    bool PassTransaction::is_committed() const noexcept { return committed_; }

    bool PassTransaction::is_rolled_back() const noexcept { return rolled_back_; }

    const ErrorBatch &PassTransaction::errors() const noexcept { return errors_; }
}  // namespace jsv
// NOLINTEND(*-include-cleaner)
