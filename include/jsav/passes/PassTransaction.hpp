/*
 * Created by gbian on 25/04/2026.
 * Copyright (c) 2026 All rights reserved.
 */
// NOLINTBEGIN(*-include-cleaner)
#pragma once

#include "PassResult.hpp"
#include "PassContext.hpp"
#include "../ir/IrCommon.hpp"

namespace jsv {

    // Forward declarations
    class IrUnit;

    /// Transactional IR modification with working copy and atomic commit
    /// Implements working-copy pattern: modifications are isolated until validated and committed
    class PassTransaction {
    public:
        /// Create a new transaction from an input IR unit
        explicit PassTransaction(const IrUnit& input);

        PassTransaction(const PassTransaction&) = delete;
        PassTransaction& operator=(const PassTransaction&) = delete;
        PassTransaction(PassTransaction&&) noexcept = default;
        PassTransaction& operator=(PassTransaction&&) noexcept = default;

        ~PassTransaction() = default;

        /// Get mutable working copy for modifications
        IrUnit& working_copy() noexcept;

        /// Get immutable view of original IR
        const IrUnit& original() const noexcept;

        /// Commit modifications after successful validation
        /// Returns the modified IR unit
        PassResult<IrUnit> commit() noexcept;

        /// Rollback all modifications to original state
        void rollback() noexcept;

        /// Check if transaction has pending modifications
        bool has_pending_changes() const noexcept;

        /// Check if transaction is in committed state
        bool is_committed() const noexcept;

        /// Get the current state of the transaction
        enum class State {
            Open,       ///< Transaction is open for modifications
            Committed,  ///< Transaction successfully committed
            RolledBack, ///< Transaction rolled back to original
            Failed,     ///< Transaction failed validation
        };

        State state() const noexcept;

    private:
        std::unique_ptr<IrUnit> original_;
        std::unique_ptr<IrUnit> working_copy_;
        State state_;
        ErrorBatch pending_errors_;
    };

}  // namespace jsv

// NOLINTEND(*-include-cleaner)