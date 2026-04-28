#pragma once

#include "GlobalEntityId.hpp"
#include "IrCommon.hpp"

#include <vector>

namespace jsv {

    // Forward declarations
    struct TypeRef;
    struct UseSite;

    /// SSA version identifier for Value tracking
    struct SsaVersion {
        uint32_t version_number{0};

        [[nodiscard]] bool operator==(const SsaVersion &other) const noexcept {
            return version_number == other.version_number;
        }
    };

    /// Use site: location where a Value is used in an instruction
    struct UseSite {
        GlobalEntityId instruction_id;  // Instruction that uses this value
        uint32_t operand_index;         // Index in the instruction's operands vector

        [[nodiscard]] bool operator==(const UseSite &other) const noexcept {
            return instruction_id == other.instruction_id && operand_index == other.operand_index;
        }
    };

    /// @brief Value entity: immutable SSA entity with single definition (US1, MVP)
    /// @invariant Single definition for each value_id (SSA property)
    /// @invariant Every use is dominated by the definition in MIR
    /// @invariant value_type cannot be changed after construction (immutability)
    class Value {
    public:
        // ── Construction ──────────────────────────────────────────────────────────
        /// Create a value with a unique ID and its defining instruction
        Value(GlobalEntityId def_instr, const TypeRef &type) noexcept;

        // ── Value semantics
        Value(const Value &) = delete;
        Value &operator=(const Value &) = delete;
        Value(Value &&) noexcept = default;
        Value &operator=(Value &&) noexcept = default;
        ~Value() = default;

        // ── Accessors (immutable after construction) ──────────────────────────────

        /// Get the value's globally unique ID
        [[nodiscard]] const GlobalEntityId &id() const noexcept { return value_id_; }

        /// Get the instruction that defines this value
        [[nodiscard]] const GlobalEntityId &defining_instruction() const noexcept { return defining_instruction_; }

        /// Get the type of this value (immutable)
        [[nodiscard]] const TypeRef &type() const noexcept { return value_type_; }

        /// Get the SSA version (incremented on each redefinition in algorithms)
        [[nodiscard]] const SsaVersion &ssa_version() const noexcept { return version_; }

        /// Get all use sites of this value
        [[nodiscard]] const std::vector<UseSite> &use_sites() const noexcept { return use_sites_; }

        // ── Mutation (use-site tracking) ──────────────────────────────────────────

        /// Record a use site (called by instructions that reference this value)
        void record_use_site(GlobalEntityId instr_id, uint32_t operand_index) noexcept;

        /// Clear use sites (useful for transactional rollback)
        void clear_use_sites() noexcept { use_sites_.clear(); }

        // ── Validation ────────────────────────────────────────────────────────────

        /// Validate Value structural invariants:
        /// - Single definition consistent
        /// - Type is non-null
        [[nodiscard]] std::expected<void, std::vector<CompileError>> validate() const noexcept;

    private:
        GlobalEntityId value_id_;
        GlobalEntityId defining_instruction_;
        TypeRef value_type_;
        SsaVersion version_;
        std::vector<UseSite> use_sites_;
    };

}  // namespace jsv
