#pragma once

#include "GlobalEntityId.hpp"
#include "IrCommon.hpp"

#include <optional>
#include <string>
#include <vector>

namespace jsv {

    // Forward declarations
    struct TypeRef;
    struct ValueRef;
    struct ValueDef;
    struct ConstraintSet;

    /// Memory effect classification for instructions
    enum class MemoryEffectKind {
        None,       // No memory effect
        Read,       // Reads from memory
        Write,      // Writes to memory
        ReadWrite   // Both reads and writes
    };

    /// @brief Instruction entity: atomic computation/control/memory operation (US1, MVP)
    /// @invariant Operands are compatible with opcode
    /// @invariant Result type is consistent with type_constraints
    /// @invariant Memory accesses are marked and orderable by dependency
    class Instruction {
    public:
        // ── Construction ──────────────────────────────────────────────────────────
        /// Create an instruction with a unique ID and the specified opcode
        Instruction(OpCode op, std::vector<ValueRef> operands, std::optional<ValueDef> result) noexcept;

        // ── Value semantics
        Instruction(const Instruction &) = delete;
        Instruction &operator=(const Instruction &) = delete;
        Instruction(Instruction &&) noexcept = default;
        Instruction &operator=(Instruction &&) noexcept = default;
        ~Instruction() = default;

        // ── Accessors (immutable structure) ───────────────────────────────────────

        /// Get the instruction's globally unique ID
        [[nodiscard]] const GlobalEntityId &id() const noexcept { return instruction_id_; }

        /// Get the operation code
        [[nodiscard]] OpCode opcode() const noexcept { return opcode_; }

        /// Get all operands
        [[nodiscard]] const std::vector<ValueRef> &operands() const noexcept { return operands_; }

        /// Get the result value (if any)
        [[nodiscard]] const std::optional<ValueDef> &result() const noexcept { return result_; }

        /// Get memory effect kind
        [[nodiscard]] MemoryEffectKind memory_effect() const noexcept { return memory_effect_; }

        /// Check if this is a terminator instruction (control-flow)
        [[nodiscard]] bool is_terminator() const noexcept;

        // ── Validation ────────────────────────────────────────────────────────────

        /// Validate instruction structural invariants:
        /// - Operands compatible with opcode
        /// - Result type consistent with constraints
        /// - Memory effects properly classified
        [[nodiscard]] std::expected<void, std::vector<CompileError>> validate() const noexcept;

    private:
        GlobalEntityId instruction_id_;
        OpCode opcode_;
        std::vector<ValueRef> operands_;
        std::optional<ValueDef> result_;
        MemoryEffectKind memory_effect_;
        ConstraintSet *type_constraints_;  // Non-owning; owned by Pass context
    };

    // ─────────────────────────────────────────────────────────────────────────────
    // Helper structures for Instruction
    // ─────────────────────────────────────────────────────────────────────────────

    /// Reference to a Value (operand in an instruction)
    struct ValueRef {
        GlobalEntityId value_id;

        /// Equality check (used in operand matching)
        [[nodiscard]] bool operator==(const ValueRef &other) const noexcept { return value_id == other.value_id; }
    };

    /// Definition of a Value (result from an instruction)
    struct ValueDef {
        GlobalEntityId value_id;
        TypeRef value_type;
    };

    /// Type constraint set for an instruction result
    struct ConstraintSet {
        // Placeholder for type constraint data
        // Will include information about permitted result types based on operands
    };

}  // namespace jsv
