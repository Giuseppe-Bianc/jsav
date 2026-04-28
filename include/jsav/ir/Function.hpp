#pragma once

#include "GlobalEntityId.hpp"
#include "IrCommon.hpp"
#include "Module.hpp"

#include <memory>
#include <string>
#include <vector>

namespace jsv {

    class BasicBlock;
    class Module;

    struct ControlFlowGraph;
    struct SsaIndex;

    /// @brief Function entity: primary pass unit and SSA domain (US1, MVP)
    /// @invariant Single entry block required
    /// @invariant All blocks must end with control terminator
    /// @invariant All blocks reachable from entry_block via CFG edges
    /// @invariant CFG edges bidirectionally consistent (A->B iff B has A as predecessor)
    /// @invariant Every Value use is dominated by its defining instruction (MIR requirement)
    class Function {
    public:
        // ── Construction ──────────────────────────────────────────────────────────
        /// Create a function with a unique ID derived from its canonical path
        /// and the provided signature
        Function(std::string function_name, const FunctionSignature &sig) noexcept;

        // ── Value semantics
        Function(const Function &) = delete;
        Function &operator=(const Function &) = delete;
        Function(Function &&) noexcept = default;
        Function &operator=(Function &&) noexcept = default;
        ~Function() = default;

        // ── Accessors (immutable after construction) ──────────────────────────────

        /// Get the function's globally unique ID
        [[nodiscard]] const GlobalEntityId &id() const noexcept { return function_id_; }

        /// Get the function's name
        [[nodiscard]] std::string_view name() const noexcept { return name_; }

        /// Get the function's signature
        [[nodiscard]] const FunctionSignature &signature() const noexcept { return signature_; }

        /// Get the entry block ID
        [[nodiscard]] const GlobalEntityId &entry_block_id() const noexcept { return entry_block_id_; }

        /// Get all basic blocks in the function
        [[nodiscard]] const std::vector<std::unique_ptr<BasicBlock>> &blocks() const noexcept { return blocks_; }

        /// Get the control flow graph
        [[nodiscard]] const ControlFlowGraph &cfg() const noexcept { return *cfg_; }

        /// Get SSA construction index
        [[nodiscard]] const SsaIndex &ssa_index() const noexcept { return *ssa_index_; }

        // ── Mutation (transactional context) ───────────────────────────────────────

        /// Add a basic block to the function
        /// Returns std::expected with error details if validation fails
        [[nodiscard]] std::expected<void, std::vector<CompileError>> add_block(
            std::unique_ptr<BasicBlock> block) noexcept;

        /// Set the entry block (must exist in blocks vector)
        /// Returns std::expected with error details if block not found
        [[nodiscard]] std::expected<void, std::vector<CompileError>>
        set_entry_block(const GlobalEntityId &block_id) noexcept;

        // ── Validation ────────────────────────────────────────────────────────────

        /// Validate function structural invariants:
        /// - Single entry block exists and is reachable
        /// - All blocks have terminators
        /// - CFG bidirectional consistency
        /// - All uses dominated by definitions (MIR only)
        [[nodiscard]] std::expected<void, std::vector<CompileError>> validate() const noexcept;

    private:
        GlobalEntityId function_id_;
        std::string name_;
        FunctionSignature signature_;
        GlobalEntityId entry_block_id_;
        std::vector<std::unique_ptr<BasicBlock>> blocks_;
        std::unique_ptr<ControlFlowGraph> cfg_;
        std::unique_ptr<SsaIndex> ssa_index_;
    };

    // ─────────────────────────────────────────────────────────────────────────────
    // Helper structures for Function
    // ─────────────────────────────────────────────────────────────────────────────

    /// Control flow graph: CFG node-edge structure with bidirectional consistency
    struct ControlFlowGraph {
        // Nodes: references to basic blocks (via their IDs)
        // Edges: stored in BasicBlock::predecessors and BasicBlock::successors
        // This struct serves as a logical grouping for CFG analysis output

        /// Verify bidirectional edge consistency across all blocks
        [[nodiscard]] std::expected<void, std::vector<CompileError>>
        validate_edge_consistency(const std::vector<std::unique_ptr<BasicBlock>> &blocks) const noexcept;
    };

    /// SSA construction state: tracks reaching definitions for canonical PHI placement
    struct SsaIndex {
        // Placeholder for SSA bookkeeping data
        // Will include reaching definitions bitsets, PHI placement strategy, etc.
        bool is_valid{false};
    };

}  // namespace jsv
