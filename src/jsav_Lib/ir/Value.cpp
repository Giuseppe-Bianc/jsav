// clang-format off
// NOLINTBEGIN(*-include-cleaner, *-avoid-magic-numbers, *-magic-numbers, *-owning-memory)
// clang-format on

#include "jsav/ir/Value.hpp"
#include "jsav/ir/GlobalEntityId.hpp"

namespace jsv {

    // ─────────────────────────────────────────────────────────────────────────────
    // Value Implementation
    // ─────────────────────────────────────────────────────────────────────────────

    Value::Value(GlobalEntityId def_instr, const TypeRef &type) noexcept
        : defining_instruction_(def_instr), value_type_(type), version_({0}) {
        // Generate deterministic global ID from defining instruction
        const std::string canonical_path = "value/" + def_instr.path();
        value_id_ = GlobalEntityId::from_structural_path(canonical_path);
    }

    void Value::record_use_site(GlobalEntityId instr_id, uint32_t operand_index) noexcept {
        use_sites_.push_back(UseSite{instr_id, operand_index});
    }

    std::expected<void, std::vector<CompileError>> Value::validate() const noexcept {
        std::vector<CompileError> errors;

        // Check single definition consistent
        if(!defining_instruction_.is_valid()) {
            errors.emplace_back("Value::validate", "Defining instruction ID is invalid", "VV001");
        }

        // Check type is non-null
        if(!value_type_.type_ptr) {
            errors.emplace_back("Value::validate", "Value type is null", "VV002");
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
