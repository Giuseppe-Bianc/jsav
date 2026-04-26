/*
 * Created by gbian on 26/04/2026.
 * Copyright (c) 2026 All rights reserved.
 */
// NOLINTBEGIN(*-include-cleaner)
#pragma once

#include "../error/CompileError.hpp"

namespace jsv {

    /// Batch error result type for passes
    using ErrorBatch = std::vector<CompileError>;

    /// Result type for pass operations - success with value or failure with error batch
    template <typename TResult>
    using PassResult = std::expected<TResult, ErrorBatch>;

    /// Convenience specialization for void operations
    using PassResultVoid = PassResult<void>;

}  // namespace jsv

// NOLINTEND(*-include-cleaner)
