// clang-format off
// NOLINTBEGIN(*-include-cleaner)
// clang-format on

#include "jsav/validation/PhiValidator.hpp"

#include "jsav/validation/SsaValidator.hpp"

#include <vector>

namespace jsv {

    namespace {
        [[nodiscard]] CompileError make_error(std::string_view code, std::string_view message) {
            const SourceLocation location(1, 1, 0);
            const SourceSpan span("ir.validation", location, location);
            return CompileError::IrGeneratorError(std::nullopt, FORMAT("[{}] {}", code, message), span, std::nullopt);
        }
    }  // namespace

    std::expected<void, std::vector<CompileError>> PhiValidator::validate_phi_nodes(
        const Function &func) noexcept {
        std::vector<CompileError> errors;

        if(auto result = SsaValidator::validate_single_definition(func); !result) {
            const auto &ssa_errors = result.error();
            errors.insert(errors.end(), ssa_errors.begin(), ssa_errors.end());
        }

        if(auto result = SsaValidator::validate_dominance(func); !result) {
            const auto &ssa_errors = result.error();
            errors.insert(errors.end(), ssa_errors.begin(), ssa_errors.end());
        }

        for(const auto &block : func.blocks()) {
            if(block->predecessors().empty()) {
                continue;
            }

            if(block->instructions().empty()) {
                errors.push_back(make_error("PHI001",
                                            FORMAT("Block {} cannot satisfy PHI placement without instructions",
                                                   block->id().path())));
            }
        }

        if(!errors.empty()) {
            return std::unexpected(errors);
        }

        return {};
    }

}  // namespace jsv

// clang-format off
// NOLINTEND(*-include-cleaner)
// clang-format on
