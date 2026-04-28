// clang-format off
// NOLINTBEGIN(*-include-cleaner, *-avoid-magic-numbers, *-magic-numbers, *-owning-memory)
// clang-format on

#include "jsav/ir/Instruction.hpp"
#include "jsav/ir/GlobalEntityId.hpp"

namespace jsv {

    // ─────────────────────────────────────────────────────────────────────────────
    // Instruction Implementation
    // ─────────────────────────────────────────────────────────────────────────────

    Instruction::Instruction(OpCode op, std::vector<ValueRef> operands,
                             std::optional<ValueDef> result) noexcept
        : opcode_(op), operands_(std::move(operands)), result_(result), memory_effect_(MemoryEffectKind::None),
          type_constraints_(nullptr) {
        // Generate deterministic global ID
        const std::string canonical_path =
            fmt::format("instr/{}", static_cast<int>(op));  // Simplified; should include context
        instruction_id_ = GlobalEntityId::from_structural_path(canonical_path);
    }

    bool Instruction::is_terminator() const noexcept {
        // Check if opcode is a control terminator (Br, BrCond, Return, etc.)
        // Placeholder: detailed opcode checking needed
        return opcode_ == OpCode::BrUnconditional || opcode_ == OpCode::BrConditional ||
               opcode_ == OpCode::Return;
    }

    std::expected<void, std::vector<CompileError>> Instruction::validate() const noexcept {
        std::vector<CompileError> errors;

        // Check operands compatibility with opcode
        // Placeholder: operand type validation needed

        // Check result type consistency
        // Placeholder: result type validation needed

        // Check memory effect classification
        // All memory operations should have memory_effect != MemoryEffectKind::None
        // Placeholder: memory effect validation needed

        if(!errors.empty()) {
            return std::unexpected(errors);
        }

        return {};
    }

}  // namespace jsv

// clang-format off
// NOLINTEND(*-include-cleaner, *-avoid-magic-numbers, *-magic-numbers, *-owning-memory)
// clang-format on
