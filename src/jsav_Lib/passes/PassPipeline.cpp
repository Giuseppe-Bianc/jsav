// clang-format off
// NOLINTBEGIN(*-include-cleaner, *-avoid-magic-numbers)
// clang-format on

#include "jsav/passes/PassPipeline.hpp"

#include "jsav/validation/IrValidator.hpp"
#include "jsav/validation/PhiValidator.hpp"
#include "jsav/validation/SsaValidator.hpp"
#include "jsav/validation/UseDefValidator.hpp"

namespace jsv {

    // ─────────────────────────────────────────────────────────────────────────────
    // PassPipeline Implementation (T039)
    // ─────────────────────────────────────────────────────────────────────────────

    std::expected<void, std::vector<CompileError>> PassPipeline::validate_post_pass(
        const Function &func) noexcept {
        std::vector<CompileError> errors;

        if(auto result = IrValidator::validate_cfg(func); !result) {
            const auto &validation_errors = result.error();
            errors.insert(errors.end(), validation_errors.begin(), validation_errors.end());
        }

        if(auto result = UseDefValidator::validate_use_def_chain(func); !result) {
            const auto &validation_errors = result.error();
            errors.insert(errors.end(), validation_errors.begin(), validation_errors.end());
        }

        if(auto result = SsaValidator::validate_single_definition(func); !result) {
            const auto &validation_errors = result.error();
            errors.insert(errors.end(), validation_errors.begin(), validation_errors.end());
        }

        if(auto result = SsaValidator::validate_dominance(func); !result) {
            const auto &validation_errors = result.error();
            errors.insert(errors.end(), validation_errors.begin(), validation_errors.end());
        }

        if(auto result = PhiValidator::validate_phi_nodes(func); !result) {
            const auto &validation_errors = result.error();
            errors.insert(errors.end(), validation_errors.begin(), validation_errors.end());
        }

        if(!errors.empty()) {
            return std::unexpected(errors);
        }

        return {};
    }

}  // namespace jsv

// clang-format off
// NOLINTEND(*-include-cleaner, *-avoid-magic-numbers)
// clang-format on
