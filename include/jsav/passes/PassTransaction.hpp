/*
 * Created by gbian on 25/04/2026.
 * Copyright (c) 2026 All rights reserved.
 */
// NOLINTBEGIN(*-include-cleaner)
#pragma once

#include "Pass.hpp"
#include "PassContext.hpp"

namespace jsv {

    class PassTransaction final {
    public:
        explicit PassTransaction(IrUnit source_ir);

        [[nodiscard]] const IrUnit &current() const noexcept;
        [[nodiscard]] IrUnit &working_copy() noexcept;
        [[nodiscard]] const IrUnit &working_copy() const noexcept;

        [[nodiscard]] PassExecutionStatus commit(const PassResult<PassInvariantReport> &post_validation);
        void rollback() noexcept;

        [[nodiscard]] bool is_committed() const noexcept;
        [[nodiscard]] bool is_rolled_back() const noexcept;
        [[nodiscard]] const ErrorBatch &errors() const noexcept;

    private:
        IrUnit committed_ir_;
        IrUnit working_ir_;
        ErrorBatch errors_{};
        bool committed_{false};
        bool rolled_back_{false};
    };

}  // namespace jsv

// NOLINTEND(*-include-cleaner)
