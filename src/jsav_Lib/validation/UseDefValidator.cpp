// clang-format off
// NOLINTBEGIN(*-include-cleaner, *-avoid-magic-numbers)
// clang-format on

#include "jsav/validation/UseDefValidator.hpp"

#include <map>
#include <vector>

namespace jsv {

    namespace {
        struct DefinitionLocation {
            std::size_t block_index{};
            std::size_t instruction_index{};
        };

        [[nodiscard]] CompileError make_error(std::string_view validator, std::string_view code,
                                              std::string_view message) {
            const SourceLocation location(1, 1, 0);
            const SourceSpan span("ir.validation", location, location);
            return CompileError::IrGeneratorError(std::nullopt, FORMAT("[{}] {}: {}", code, validator, message), span,
                                               std::nullopt);
        }

        [[nodiscard]] std::optional<DefinitionLocation> find_definition(
            const std::map<GlobalEntityId, DefinitionLocation> &definitions, const GlobalEntityId &value_id) {
            const auto it = definitions.find(value_id);
            if(it == definitions.end()) {
                return std::nullopt;
            }
            return it->second;
        }

        [[nodiscard]] std::map<GlobalEntityId, DefinitionLocation> collect_definitions(const Function &func,
                                                                                       std::vector<CompileError> &errors) {
            std::map<GlobalEntityId, DefinitionLocation> definitions;

            for(std::size_t block_index = 0; block_index < func.blocks().size(); ++block_index) {
                const auto &block = func.blocks()[block_index];
                const auto &instructions = block->instructions();
                for(std::size_t instruction_index = 0; instruction_index < instructions.size(); ++instruction_index) {
                    const auto &instruction = instructions[instruction_index];
                    if(const auto &result = instruction->result(); result.has_value()) {
                        const auto [it, inserted] = definitions.emplace(result->value_id, DefinitionLocation{block_index, instruction_index});
                        if(!inserted) {
                            errors.push_back(make_error("UseDefValidator::validate_use_def_chain", "UD002",
                                                        FORMAT("Value {} is defined more than once", result->value_id.path())));
                        }
                    }
                }
            }

            return definitions;
        }
    }  // namespace

    std::expected<void, std::vector<CompileError>> UseDefValidator::validate_use_def_chain(
        const Function &func) noexcept {
        std::vector<CompileError> errors;

        const auto definitions = collect_definitions(func, errors);

        for(std::size_t block_index = 0; block_index < func.blocks().size(); ++block_index) {
            const auto &block = func.blocks()[block_index];
            const auto &instructions = block->instructions();

            for(std::size_t instruction_index = 0; instruction_index < instructions.size(); ++instruction_index) {
                const auto &instruction = instructions[instruction_index];

                if(const auto &result = instruction->result(); result.has_value() && !result->value_id.is_valid()) {
                    errors.push_back(make_error("UseDefValidator::validate_use_def_chain", "UD001",
                                                FORMAT("Instruction {} in block {} has an invalid result ID",
                                                       instruction->id().path(), block->id().path())));
                }

                for(const auto &operand : instruction->operands()) {
                    if(const auto def = find_definition(definitions, operand.value_id); !def.has_value()) {
                        errors.push_back(make_error("UseDefValidator::validate_use_def_chain", "UD003",
                                                    FORMAT("Operand {} in block {} references an undefined value",
                                                           operand.value_id.path(), block->id().path())));
                    }
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
// NOLINTEND(*-include-cleaner, *-avoid-magic-numbers)
// clang-format on
