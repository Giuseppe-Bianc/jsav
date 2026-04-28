// clang-format off
// NOLINTBEGIN(*-include-cleaner, *-avoid-magic-numbers, *-magic-numbers, *-owning-memory)
// clang-format on

#include "jsav/ir/Function.hpp"
#include "jsav/ir/BasicBlock.hpp"
#include "jsav/ir/GlobalEntityId.hpp"

namespace jsv {

    // ─────────────────────────────────────────────────────────────────────────────
    // Function Implementation
    // ─────────────────────────────────────────────────────────────────────────────

    Function::Function(std::string function_name, const FunctionSignature &sig) noexcept
        : name_(std::move(function_name)), signature_(sig), cfg_(std::make_unique<ControlFlowGraph>()),
          ssa_index_(std::make_unique<SsaIndex>()) {
        // Generate deterministic global ID from function name
        const std::string canonical_path = "function/" + name_;
        function_id_ = GlobalEntityId::from_structural_path(canonical_path);
        // entry_block_id will be set later via set_entry_block()
    }

    std::expected<void, std::vector<CompileError>> Function::add_block(
        std::unique_ptr<BasicBlock> block) noexcept {
        if(!block) {
            return std::unexpected(std::vector<CompileError>{
                CompileError("Function::add_block", "nullptr block passed", "F001")});
        }

        blocks_.push_back(std::move(block));
        return {};
    }

    std::expected<void, std::vector<CompileError>> Function::set_entry_block(
        const GlobalEntityId &block_id) noexcept {
        // Verify block exists
        bool found = false;
        for(const auto &block : blocks_) {
            if(block->id() == block_id) {
                found = true;
                break;
            }
        }

        if(!found) {
            return std::unexpected(std::vector<CompileError>{
                CompileError("Function::set_entry_block", "Block ID not found in function blocks", "F002")});
        }

        entry_block_id_ = block_id;
        return {};
    }

    std::expected<void, std::vector<CompileError>> Function::validate() const noexcept {
        std::vector<CompileError> errors;

        // Check entry block exists
        if(entry_block_id_.path().empty()) {
            errors.emplace_back("Function::validate", "No entry block set", "FV001");
        }

        // Check all blocks have terminators and are reachable
        for(const auto &block : blocks_) {
            if(block->terminator() == nullptr) {
                errors.emplace_back("Function::validate",
                                   fmt::format("Block {} has no terminator", block->id().path()), "FV002");
            }
        }

        // Validate CFG edge consistency
        if(auto result = cfg_->validate_edge_consistency(blocks_); !result) {
            const auto &cfg_errors = result.error();
            errors.insert(errors.end(), cfg_errors.begin(), cfg_errors.end());
        }

        if(!errors.empty()) {
            return std::unexpected(errors);
        }

        return {};
    }

    // ─────────────────────────────────────────────────────────────────────────────
    // ControlFlowGraph Implementation
    // ─────────────────────────────────────────────────────────────────────────────

    std::expected<void, std::vector<CompileError>>
    ControlFlowGraph::validate_edge_consistency(const std::vector<std::unique_ptr<BasicBlock>> &blocks) const
        noexcept {
        std::vector<CompileError> errors;

        // Verify bidirectional consistency for all edges
        for(const auto &block_a : blocks) {
            // Check successors: for each successor, block_a must be in its predecessors
            for(const auto &succ_id : block_a->successors()) {
                bool found = false;
                for(const auto &block_b : blocks) {
                    if(block_b->id() == succ_id) {
                        // Check if block_a is in block_b's predecessors
                        for(const auto &pred_id : block_b->predecessors()) {
                            if(pred_id == block_a->id()) {
                                found = true;
                                break;
                            }
                        }
                        break;
                    }
                }

                if(!found) {
                    errors.emplace_back("CFG::validate_edge_consistency",
                                       fmt::format("Edge {} -> {} not bidirectional", block_a->id().path(),
                                                   succ_id.path()),
                                       "CFG001");
                }
            }
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
