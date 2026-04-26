/*
 * Created by gbian on 26/04/2026.
 * Copyright (c) 2026 All rights reserved.
 */
// NOLINTBEGIN(*-include-cleaner)
#pragma once

#include "PassResult.hpp"
#include "../ir/IrCommon.hpp"


namespace jsv {

    // Forward declarations
    class IrUnit;
    struct PassContext;

    /// Report on pass invariant validation
    struct PassInvariantReport {
        bool cfgValid;      ///< Control flow graph invariants satisfied
        bool ssaValid;      ///< SSA properties maintained
        bool phiValid;      ///< PHI node invariants satisfied
        bool typesValid;    ///< Type system invariants satisfied
        bool memoryValid;   ///< Memory ordering invariants satisfied
    };

    /// Granularity of pass application
    enum class IrGranularity {
        Module,    ///< Pass operates on entire module
        Function,  ///< Pass operates on individual functions
    };

    /// Ownership semantics for pass output
    enum class OutputOwnershipSemantics {
        TransferOwnership,  ///< Pass output is owned by caller
        DeepCopy,          ///< Pass returns a copy
    };

    /// Ordering relation between passes
    enum class OrderingRelation {
        MustRunAfter,      ///< This pass must run after the other
        MustRunBefore,     ///< This pass must run before the other
        PreferRunAfter,    ///< This pass should preferably run after the other
        PreferRunBefore,   ///< This pass should preferably run before the other
        MutuallyExclusive, ///< These passes cannot run together
    };

    /// Ordering constraint for pass dependencies
    struct OrderingConstraint {
        std::string_view otherPass;
        OrderingRelation relation;
        bool conditional;
    };

    /// Analysis key for composition and invalidation tracking
    using AnalysisKey = std::string_view;

    /// Interface for IR passes
    class IPass {
    public:
        virtual ~IPass() = default;

        /// Get the pass name
        virtual std::string_view name() const noexcept = 0;

        /// Get the pass kind
        virtual PassKind kind() const noexcept = 0;

        /// Get required passes that must run before this one
        virtual std::vector<std::string_view> getRequiredPasses() const {
            return {};
        }

        /// Get ordering constraints relative to other passes
        virtual std::vector<OrderingConstraint> getOrderingConstraints() const {
            return {};
        }

        /// Get analysis results required by this pass
        virtual std::vector<AnalysisKey> getRequiredAnalyses() const {
            return {};
        }

        /// Get analyses preserved by this pass
        virtual std::vector<AnalysisKey> getPreservedAnalyses() const {
            return {};
        }

        /// Get analyses invalidated by this pass
        virtual std::vector<AnalysisKey> getInvalidatedAnalyses() const {
            return {};
        }

        /// Callback when analyses are invalidated
        virtual void onAnalysesInvalidated(std::span<const AnalysisKey> /*invalidated*/) {
        }

        /// Register an analysis result
        virtual void registerAnalysisResult(AnalysisKey /*key*/, const void* /*result*/) {
        }

        /// Deregister an analysis result
        virtual void deregisterAnalysisResult(AnalysisKey /*key*/) {
        }

        /// Check if pass supports incremental execution
        virtual bool supportsIncremental() const noexcept {
            return false;
        }

        /// Get the granularity of this pass
        virtual IrGranularity granularity() const noexcept {
            return IrGranularity::Module;
        }

        /// Get ownership semantics for pass output
        virtual OutputOwnershipSemantics outputOwnershipSemantics() const noexcept {
            return OutputOwnershipSemantics::TransferOwnership;
        }

        /// Create a deep copy of an IR unit
        virtual PassResult<IrUnit> deepCopyIrUnit(const IrUnit& input) const = 0;

        /// Validate preconditions before pass execution
        virtual PassResult<void> validatePreconditions(const IrUnit& input,
                                                       const PassContext& ctx) const = 0;

        /// Run the pass on a working copy
        virtual PassResult<IrUnit> runOnWorkingCopy(const IrUnit& input,
                                                    const PassContext& ctx) const = 0;

        /// Validate postconditions after pass execution
        virtual PassResult<PassInvariantReport> validatePostconditions(const IrUnit& output,
                                                                       const PassContext& ctx) const = 0;
    };

}  // namespace jsv

// NOLINTEND(*-include-cleaner)
