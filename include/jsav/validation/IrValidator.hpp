/*
 * Created by gbian on 25/04/2026.
 * Copyright (c) 2026 All rights reserved.
 */
// NOLINTBEGIN(*-include-cleaner)
#pragma once

#include "jsav/error/CompileError.hpp"
#include "jsav/ir/BasicBlock.hpp"
#include "jsav/ir/Function.hpp"

#include <vector>

namespace jsv {

    /// @brief IR validator: checks structural invariants of HIR/MIR/LIR (T033-T039, US1)
    /// Validates:
    /// - CFG: single entry, all blocks reachable, bidirectional edges, terminators
    /// - Type: compatibility and equivalence (nominal for user types)
    /// - Use-def: uses dominate definitions, SSA properties
    /// - SSA: single definition, dominance relations
    /// - PHI: correct operand count, reachability
    class IrValidator {
    public:
        /// Validate CFG structural invariants
        [[nodiscard]] static std::expected<void, std::vector<CompileError>>
        validate_cfg(const Function &func) noexcept;

        /// Validate that all blocks are reachable from entry
        [[nodiscard]] static std::expected<void, std::vector<CompileError>>
        validate_reachability(const Function &func) noexcept;

        /// Validate terminator instructions on all blocks
        [[nodiscard]] static std::expected<void, std::vector<CompileError>>
        validate_terminators(const Function &func) noexcept;

        /// Validate CFG edge bidirectional consistency
        [[nodiscard]] static std::expected<void, std::vector<CompileError>>
        validate_edge_consistency(const Function &func) noexcept;
    };

}  // namespace jsv
// NOLINTEND(*-include-cleaner)