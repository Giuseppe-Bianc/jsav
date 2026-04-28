#pragma once

#include "GlobalEntityId.hpp"
#include "IrCommon.hpp"

#include <memory>
#include <vector>

namespace jsv {

    // Forward declarations
    class Instruction;

    /// @brief BasicBlock entity: CFG node with instruction sequence (US1, MVP)
    /// @invariant Terminator is mandatory and last element in instructions vector
    /// @invariant All PHI nodes must appear consecutively at the beginning
    /// @invariant Block must have at least one instruction (the terminator)
    /// @invariant Operand SSA references follow dominance and local ordering rules
    /// @invariant Predecessor/successor sets are bidirectionally consistent
    class BasicBlock {
    public:
        // ── Construction ──────────────────────────────────────────────────────────
        /// Create a basic block with a unique ID derived from its canonical path
        explicit BasicBlock(std::string block_name) noexcept;

        // ── Value semantics
        BasicBlock(const BasicBlock &) = delete;
        BasicBlock &operator=(const BasicBlock &) = delete;
        BasicBlock(BasicBlock &&) noexcept = default;
        BasicBlock &operator=(BasicBlock &&) noexcept = default;
        ~BasicBlock() = default;

        // ── Accessors (immutable structure) ───────────────────────────────────────

        /// Get the block's globally unique ID
        [[nodiscard]] const GlobalEntityId &id() const noexcept { return block_id_; }

        /// Get all instructions in the block (including terminator)
        [[nodiscard]] const std::vector<std::unique_ptr<Instruction>> &instructions() const noexcept {
            return instructions_;
        }

        /// Get predecessor block IDs
        [[nodiscard]] const std::vector<GlobalEntityId> &predecessors() const noexcept { return predecessors_; }

        /// Get successor block IDs
        [[nodiscard]] const std::vector<GlobalEntityId> &successors() const noexcept { return successors_; }

        /// Get the terminator instruction (const access)
        [[nodiscard]] const Instruction *terminator() const noexcept;

        // ── Mutation ──────────────────────────────────────────────────────────────

        /// Add a non-terminator instruction to the block
        /// Returns std::expected with error if block already has terminator at end
        [[nodiscard]] std::expected<void, std::vector<CompileError>> add_instruction(
            std::unique_ptr<Instruction> instr) noexcept;

        /// Set the terminator instruction (must have control-flow opcode)
        /// Replaces any existing terminator; validates opcode
        /// Returns std::expected with error if opcode is not a control terminator
        [[nodiscard]] std::expected<void, std::vector<CompileError>>
        set_terminator(std::unique_ptr<Instruction> term) noexcept;

        /// Add a predecessor block ID (maintaining bidirectional consistency)
        void add_predecessor(const GlobalEntityId &pred_id) noexcept;

        /// Add a successor block ID (maintaining bidirectional consistency)
        void add_successor(const GlobalEntityId &succ_id) noexcept;

        // ── Validation ────────────────────────────────────────────────────────────

        /// Validate block structural invariants:
        /// - Has exactly one terminator (last instruction)
        /// - All PHI nodes at the beginning
        /// - At least one instruction
        /// - Operands satisfy local SSA ordering
        [[nodiscard]] std::expected<void, std::vector<CompileError>> validate() const noexcept;

    private:
        GlobalEntityId block_id_;
        std::vector<std::unique_ptr<Instruction>> instructions_;
        std::vector<GlobalEntityId> predecessors_;
        std::vector<GlobalEntityId> successors_;
    };

}  // namespace jsv
