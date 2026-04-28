// clang-format off
// NOLINTBEGIN(*-include-cleaner)
// clang-format on

#include "jsav/validation/SsaValidator.hpp"

#include <map>
#include <vector>

namespace jsv {

    namespace {
        struct DefinitionLocation {
            std::size_t block_index{};
            std::size_t instruction_index{};
        };

        [[nodiscard]] CompileError make_error(std::string_view code, std::string_view message) {
            const SourceLocation location(1, 1, 0);
            const SourceSpan span("ir.validation", location, location);
            return CompileError::IrGeneratorError(std::nullopt, FORMAT("[{}] {}", code, message), span, std::nullopt);
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
                            errors.push_back(make_error("SSA001",
                                                        FORMAT("Value {} is defined more than once", result->value_id.path())));
                        }
                    }
                }
            }

            return definitions;
        }

        [[nodiscard]] std::vector<std::vector<std::size_t>> compute_dominators(const Function &func) {
            std::vector<std::vector<std::size_t>> dominators(func.blocks().size());
            if(func.blocks().empty()) {
                return dominators;
            }

            const auto entry_it = std::find_if(func.blocks().begin(), func.blocks().end(),
                                               [&func](const auto &block) { return block->id() == func.entry_block_id(); });
            if(entry_it == func.blocks().end()) {
                return dominators;
            }

            const std::size_t entry_index = static_cast<std::size_t>(std::distance(func.blocks().begin(), entry_it));

            for(std::size_t block_index = 0; block_index < func.blocks().size(); ++block_index) {
                dominators[block_index].reserve(func.blocks().size());
                for(std::size_t candidate = 0; candidate < func.blocks().size(); ++candidate) {
                    dominators[block_index].push_back(candidate);
                }
            }

            dominators[entry_index].clear();
            dominators[entry_index].push_back(entry_index);

            bool changed = true;
            while(changed) {
                changed = false;
                for(std::size_t block_index = 0; block_index < func.blocks().size(); ++block_index) {
                    if(block_index == entry_index) {
                        continue;
                    }

                    const auto &block = func.blocks()[block_index];
                    if(block->predecessors().empty()) {
                        continue;
                    }

                    std::vector<std::size_t> candidate = dominators[block_index];
                    candidate.clear();

                    bool first = true;
                    for(const auto &pred_id : block->predecessors()) {
                        const auto pred_it = std::find_if(func.blocks().begin(), func.blocks().end(),
                                                          [&pred_id](const auto &candidate_block) {
                                                              return candidate_block->id() == pred_id;
                                                          });
                        if(pred_it == func.blocks().end()) {
                            continue;
                        }

                        const std::size_t pred_index = static_cast<std::size_t>(std::distance(func.blocks().begin(), pred_it));
                        if(first) {
                            candidate = dominators[pred_index];
                            first = false;
                        } else {
                            std::vector<std::size_t> intersection;
                            for(const auto dominator : candidate) {
                                if(std::find(dominators[pred_index].begin(), dominators[pred_index].end(), dominator) !=
                                   dominators[pred_index].end()) {
                                    intersection.push_back(dominator);
                                }
                            }
                            candidate = std::move(intersection);
                        }
                    }

                    if(std::find(candidate.begin(), candidate.end(), block_index) == candidate.end()) {
                        candidate.push_back(block_index);
                    }

                    std::sort(candidate.begin(), candidate.end());
                    if(candidate != dominators[block_index]) {
                        dominators[block_index] = std::move(candidate);
                        changed = true;
                    }
                }
            }

            return dominators;
        }

        [[nodiscard]] bool dominates(const std::vector<std::vector<std::size_t>> &dominators,
                                     std::size_t dominator_index, std::size_t block_index) {
            const auto &dominator_set = dominators[block_index];
            return std::find(dominator_set.begin(), dominator_set.end(), dominator_index) != dominator_set.end();
        }
    }  // namespace

    std::expected<void, std::vector<CompileError>> SsaValidator::validate_single_definition(
        const Function &func) noexcept {
        std::vector<CompileError> errors;

        (void)collect_definitions(func, errors);

        if(!errors.empty()) {
            return std::unexpected(errors);
        }

        return {};
    }

    std::expected<void, std::vector<CompileError>> SsaValidator::validate_dominance(
        const Function &func) noexcept {
        std::vector<CompileError> errors;

        const auto definitions = collect_definitions(func, errors);
        const auto dominators = compute_dominators(func);

        for(std::size_t block_index = 0; block_index < func.blocks().size(); ++block_index) {
            const auto &block = func.blocks()[block_index];
            const auto &instructions = block->instructions();

            for(std::size_t instruction_index = 0; instruction_index < instructions.size(); ++instruction_index) {
                const auto &instruction = instructions[instruction_index];
                for(const auto &operand : instruction->operands()) {
                    const auto def_it = definitions.find(operand.value_id);
                    if(def_it == definitions.end()) {
                        continue;
                    }

                    const auto [def_block_index, def_instruction_index] = def_it->second;
                    const bool same_block = def_block_index == block_index;
                    const bool dominates_current_block = same_block || dominates(dominators, def_block_index, block_index);
                    const bool defined_before_use = !same_block || def_instruction_index < instruction_index;

                    if(!(dominates_current_block && defined_before_use)) {
                        errors.push_back(make_error("SSA002",
                                                    FORMAT("Value {} used in block {} is not dominated by its definition",
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
// NOLINTEND(*-include-cleaner)
// clang-format on
