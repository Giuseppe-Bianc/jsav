// clang-format off
// NOLINTBEGIN(*-include-cleaner, *-avoid-magic-numbers, *-magic-numbers, *-owning-memory)
// clang-format on

#include "jsav/ir/BasicBlock.hpp"
#include "jsav/ir/Instruction.hpp"
#include "jsav/ir/GlobalEntityId.hpp"

namespace jsv {

    // ─────────────────────────────────────────────────────────────────────────────
    // BasicBlock Implementation
    // ─────────────────────────────────────────────────────────────────────────────

    BasicBlock::BasicBlock(std::string block_name) noexcept {
        // Generate deterministic global ID from block name
        const std::string canonical_path = "block/" + block_name;
        block_id_ = GlobalEntityId::from_structural_path(canonical_path);
    }

    const Instruction *BasicBlock::terminator() const noexcept {
        if(instructions_.empty()) {
            return nullptr;
        }
        return instructions_.back().get();
    }

    std::expected<void, std::vector<CompileError>> BasicBlock::add_instruction(
        std::unique_ptr<Instruction> instr) noexcept {
        if(!instr) {
            return std::unexpected(std::vector<CompileError>{
                CompileError("BasicBlock::add_instruction", "nullptr instruction", "BB001")});
        }

        // Cannot add after terminator
        if(!instructions_.empty() && instructions_.back()->is_terminator()) {
            return std::unexpected(std::vector<CompileError>{
                CompileError("BasicBlock::add_instruction", "Cannot add instruction after terminator", "BB002")});
        }

        instructions_.push_back(std::move(instr));
        return {};
    }

    std::expected<void, std::vector<CompileError>> BasicBlock::set_terminator(
        std::unique_ptr<Instruction> term) noexcept {
        if(!term) {
            return std::unexpected(std::vector<CompileError>{
                CompileError("BasicBlock::set_terminator", "nullptr terminator", "BB003")});
        }

        if(!term->is_terminator()) {
            return std::unexpected(std::vector<CompileError>{
                CompileError("BasicBlock::set_terminator", "Instruction is not a terminator", "BB004")});
        }

        // Remove existing terminator if present
        if(!instructions_.empty() && instructions_.back()->is_terminator()) {
            instructions_.pop_back();
        }

        instructions_.push_back(std::move(term));
        return {};
    }

    void BasicBlock::add_predecessor(const GlobalEntityId &pred_id) noexcept {
        // Check for duplicates
        for(const auto &existing : predecessors_) {
            if(existing == pred_id) {
                return;  // Already exists
            }
        }
        predecessors_.push_back(pred_id);
    }

    void BasicBlock::add_successor(const GlobalEntityId &succ_id) noexcept {
        // Check for duplicates
        for(const auto &existing : successors_) {
            if(existing == succ_id) {
                return;  // Already exists
            }
        }
        successors_.push_back(succ_id);
    }

    std::expected<void, std::vector<CompileError>> BasicBlock::validate() const noexcept {
        std::vector<CompileError> errors;

        // Must have at least one instruction (terminator)
        if(instructions_.empty()) {
            errors.emplace_back("BasicBlock::validate", "Block has no instructions", "BBV001");
            return std::unexpected(errors);
        }

        // Last instruction must be a terminator
        if(!instructions_.back()->is_terminator()) {
            errors.emplace_back("BasicBlock::validate", "Block's last instruction is not a terminator", "BBV002");
        }

        // Check PHI nodes are at the beginning
        size_t phi_count = 0;
        for(const auto &instr : instructions_) {
            // Note: is_phi() not yet implemented; placeholder
            phi_count++;
        }

        if(!errors.empty()) {
            return std::unexpected(errors);
        }

        return {};
    }

}  // namespace jsv

// clang-format off
// NOLINTEND(*-include-cleaner, *-avoid-magic-numbers, *-magic-numbers, *-owning-memory)
// clang-format on
