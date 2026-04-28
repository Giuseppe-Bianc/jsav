#pragma once

#include "GlobalEntityId.hpp"
#include "IrCommon.hpp"

#include <vector>

namespace jsv {

    // Forward declarations
    struct ValueRef;

    /// Incoming edge for a PHI node: (predecessor_block_id, value_id)
    struct PhiIncoming {
        GlobalEntityId predecessor_block_id;
        GlobalEntityId value_id;

        [[nodiscard]] bool operator==(const PhiIncoming &other) const noexcept {
            return predecessor_block_id == other.predecessor_block_id && value_id == other.value_id;
        }
    };

    /// @brief PHI node: definition convergence in SSA (US1, MVP)
    /// @invariant One operand for each current predecessor of the block
    /// @invariant Each incoming value reaches the block from its associated predecessor
    /// @invariant Eager pruning on unreachable predecessors (FR-006, FR-007)
    class PhiNode {
    public:
        // ── Construction ──────────────────────────────────────────────────────────
        /// Create a PHI node with a unique ID and target value
        PhiNode(GlobalEntityId target_val, std::vector<PhiIncoming> incoming) noexcept;

        // ── Value semantics
        PhiNode(const PhiNode &) = delete;
        PhiNode &operator=(const PhiNode &) = delete;
        PhiNode(PhiNode &&) noexcept = default;
        PhiNode &operator=(PhiNode &&) noexcept = default;
        ~PhiNode() = default;

        // ── Accessors (immutable structure, except during minimization) ───────────

        /// Get the PHI node's globally unique ID
        [[nodiscard]] const GlobalEntityId &id() const noexcept { return phi_id_; }

        /// Get the target value (value defined by this PHI)
        [[nodiscard]] const GlobalEntityId &target_value() const noexcept { return target_value_; }

        /// Get all incoming edges (predecessor_block_id, value_id pairs)
        [[nodiscard]] const std::vector<PhiIncoming> &incoming() const noexcept { return incoming_; }

        // ── Mutation (minimization and pruning) ────────────────────────────────────

        /// Remove an incoming edge by predecessor block ID
        /// (Called during eager PHI pruning on unreachable predecessors)
        void remove_incoming(const GlobalEntityId &pred_block_id) noexcept;

        /// Update an incoming value for a predecessor
        void update_incoming(const GlobalEntityId &pred_block_id, GlobalEntityId new_value_id) noexcept;

        /// Minimize trivial PHI nodes (all operands are the same)
        /// Returns the unique value_id if all incoming edges point to the same value, else std::nullopt
        [[nodiscard]] std::optional<GlobalEntityId> try_minimize() const noexcept;

        // ── Validation ────────────────────────────────────────────────────────────

        /// Validate PHI node structural invariants:
        /// - Exactly one operand per current predecessor
        /// - All incoming values are valid references
        /// - No self-referential cycles without intervening values
        [[nodiscard]] std::expected<void, std::vector<CompileError>>
        validate(const std::vector<GlobalEntityId> &current_predecessors) const noexcept;

    private:
        GlobalEntityId phi_id_;
        GlobalEntityId target_value_;
        std::vector<PhiIncoming> incoming_;
    };

}  // namespace jsv
