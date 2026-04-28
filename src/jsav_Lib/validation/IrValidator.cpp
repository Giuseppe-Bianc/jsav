/*
 * Created by gbian on 25/04/2026.
 * Copyright (c) 2026 All rights reserved.
 */
// NOLINTBEGIN(*-include-cleaner, *-avoid-magic-numbers, *-magic-numbers)
#include "jsav/validation/IrValidator.hpp"

namespace jsv {

    namespace {
        [[nodiscard]] CompileError make_error(std::string_view code, std::string_view context,
                                              std::string_view message) {
            const SourceLocation location(1, 1, 0);
            const SourceSpan span("ir.validation", location, location);
            return CompileError::IrGeneratorError(std::nullopt, FORMAT("[{}] {}: {}", code, context, message), span,
                                                  std::nullopt);
        }
    }  // namespace

    // ─────────────────────────────────────────────────────────────────────────────
    // IrValidator Implementation (T033-T039)
    // ─────────────────────────────────────────────────────────────────────────────

    std::expected<void, std::vector<CompileError>> IrValidator::validate_cfg(const Function &func) noexcept {
        std::vector<CompileError> errors;

        // Delegate to individual validators
        if(auto result = validate_reachability(func); !result) {
            const auto &reachability_errors = result.error();
            errors.insert(errors.end(), reachability_errors.begin(), reachability_errors.end());
        }

        if(auto result = validate_terminators(func); !result) {
            const auto &terminator_errors = result.error();
            errors.insert(errors.end(), terminator_errors.begin(), terminator_errors.end());
        }

        if(auto result = validate_edge_consistency(func); !result) {
            const auto &edge_errors = result.error();
            errors.insert(errors.end(), edge_errors.begin(), edge_errors.end());
        }

        if(!errors.empty()) {
            return std::unexpected(errors);
        }

        return {};
    }

    std::expected<void, std::vector<CompileError>> IrValidator::validate_reachability(
        const Function &func) noexcept {
        std::vector<CompileError> errors;

        // Check entry block exists
        if(func.entry_block_id().path().empty()) {
            errors.push_back(make_error("CFG001", "IrValidator::validate_reachability", "No entry block set in function"));
            return std::unexpected(errors);
        }

        // BFS from entry block to mark reachable blocks
        std::vector<bool> reachable(func.blocks().size(), false);
        std::vector<const BasicBlock *> queue;

        // Find entry block and start BFS
        const BasicBlock *entry = nullptr;
        for(size_t i = 0; i < func.blocks().size(); ++i) {
            if(func.blocks()[i]->id() == func.entry_block_id()) {
                entry = func.blocks()[i].get();
                reachable[i] = true;
                queue.push_back(entry);
                break;
            }
        }

        if(!entry) {
            errors.push_back(make_error("CFG002", "IrValidator::validate_reachability",
                                        "Entry block not found in blocks vector"));
            return std::unexpected(errors);
        }

        // BFS traversal
        while(!queue.empty()) {
            const auto current = queue.back();
            queue.pop_back();

            for(const auto &succ_id : current->successors()) {
                for(size_t i = 0; i < func.blocks().size(); ++i) {
                    if(func.blocks()[i]->id() == succ_id && !reachable[i]) {
                        reachable[i] = true;
                        queue.push_back(func.blocks()[i].get());
                        break;
                    }
                }
            }
        }

        // Check all blocks are reachable
        for(size_t i = 0; i < func.blocks().size(); ++i) {
            if(!reachable[i]) {
                errors.push_back(make_error("CFG003", "IrValidator::validate_reachability",
                                            FORMAT("Block {} is not reachable from entry", func.blocks()[i]->id().path())));
            }
        }

        if(!errors.empty()) {
            return std::unexpected(errors);
        }

        return {};
    }

    std::expected<void, std::vector<CompileError>> IrValidator::validate_terminators(
        const Function &func) noexcept {
        std::vector<CompileError> errors;

        for(const auto &block : func.blocks()) {
            if(!block->terminator()) {
                errors.push_back(make_error("CFG004", "IrValidator::validate_terminators",
                                            FORMAT("Block {} has no terminator", block->id().path())));
            }
        }

        if(!errors.empty()) {
            return std::unexpected(errors);
        }

        return {};
    }

    std::expected<void, std::vector<CompileError>> IrValidator::validate_edge_consistency(
        const Function &func) noexcept {
        std::vector<CompileError> errors;

        for(const auto &block_a : func.blocks()) {
            // For each successor of block_a, verify it has block_a as predecessor
            for(const auto &succ_id : block_a->successors()) {
                bool found = false;
                for(const auto &block_b : func.blocks()) {
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
                    errors.push_back(make_error("CFG005", "IrValidator::validate_edge_consistency",
                                                FORMAT("Edge {} -> {} is not bidirectional", block_a->id().path(),
                                                       succ_id.path())));
                }
            }
        }

        if(!errors.empty()) {
            return std::unexpected(errors);
        }

        return {};
    }

}  // namespace jsv
// NOLINTEND(*-include-cleaner, *-avoid-magic-numbers, *-magic-numbers)