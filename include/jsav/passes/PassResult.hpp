/*
 * Created by gbian on 26/04/2026.
 * Copyright (c) 2026 All rights reserved.
 */
// NOLINTBEGIN(*-include-cleaner)
#pragma once

#include "../error/CompileError.hpp"

namespace jsv {

    using ErrorBatch = std::vector<CompileError>;

    template <typename TResult> using PassResult = std::expected<TResult, ErrorBatch>;

    struct PassExecutionStatus {
        bool committed{false};
        bool rolled_back{false};
        ErrorBatch errors{};

        [[nodiscard]] bool succeeded() const noexcept { return committed && !rolled_back && errors.empty(); }
    };

}  // namespace jsv

// NOLINTEND(*-include-cleaner)
