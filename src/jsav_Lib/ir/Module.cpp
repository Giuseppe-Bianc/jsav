// clang-format off
// NOLINTBEGIN(*-include-cleaner, *-avoid-magic-numbers, *-magic-numbers, *-owning-memory, *-pro-bounds-array-to-pointer-decay)
// clang-format on

#include "jsav/ir/Module.hpp"
#include "jsav/ir/Function.hpp"
#include "jsav/ir/GlobalEntityId.hpp"

namespace jsv {

    // ─────────────────────────────────────────────────────────────────────────────
    // Module Implementation
    // ─────────────────────────────────────────────────────────────────────────────

    Module::Module(std::string module_name) noexcept
        : name_(std::move(module_name)) {
        // Generate deterministic global ID from module name
        const std::string canonical_path = name_;
        module_id_ = GlobalEntityId::from_structural_path("module/" + canonical_path);
    }

    std::expected<void, std::vector<CompileError>> Module::add_function(
        std::unique_ptr<Function> func) noexcept {
        if(!func) {
            return std::unexpected(std::vector<CompileError>{
                CompileError("Module::add_function", "nullptr function passed", "E001")});
        }

        // Check for duplicates
        const auto func_name = func->name();
        for(const auto &existing : functions_) {
            if(existing->name() == func_name) {
                return std::unexpected(std::vector<CompileError>{
                    CompileError("Module::add_function",
                                 fmt::format("Function '{}' already exists in module '{}'", func_name, name_),
                                 "E002")});
            }
        }

        // Add to function table
        function_table_[std::string(func_name)] = func->signature();

        // Add to functions vector
        functions_.push_back(std::move(func));

        return {};
    }

    std::expected<void, std::vector<CompileError>> Module::register_type(
        std::string type_name, TypeDefVersion type_def) noexcept {
        // Check for duplicates
        if(type_table_.contains(type_name)) {
            return std::unexpected(std::vector<CompileError>{
                CompileError("Module::register_type",
                             fmt::format("Type '{}' already registered in module '{}'", type_name, name_),
                             "E003")});
        }

        // Validate type definition
        if(type_def.type_identity.empty() || type_def.version_id.empty() ||
           type_def.equivalence_rule != "nominal") {
            return std::unexpected(std::vector<CompileError>{
                CompileError("Module::register_type",
                             fmt::format("Invalid type definition for '{}': incomplete or invalid rule", type_name),
                             "E004")});
        }

        type_table_[type_name] = type_def;
        return {};
    }

    std::expected<void, std::vector<CompileError>> Module::validate() const noexcept {
        std::vector<CompileError> errors;

        // Validate bidirectional function consistency
        for(const auto &func : functions_) {
            const auto func_name = func->name();
            if(!function_table_.contains(std::string(func_name))) {
                errors.emplace_back("Module::validate",
                                   fmt::format("Function '{}' in vector but not in table", func_name), "V001");
            }
        }

        for(const auto &[name, sig] : function_table_) {
            bool found = false;
            for(const auto &func : functions_) {
                if(func->name() == name) {
                    found = true;
                    break;
                }
            }
            if(!found) {
                errors.emplace_back("Module::validate",
                                   fmt::format("Function '{}' in table but not in vector", name), "V002");
            }
        }

        // Validate each function
        for(const auto &func : functions_) {
            if(auto result = func->validate(); !result) {
                const auto &func_errors = result.error();
                errors.insert(errors.end(), func_errors.begin(), func_errors.end());
            }
        }

        if(!errors.empty()) {
            return std::unexpected(errors);
        }

        return {};
    }

}  // namespace jsv

// clang-format off
// NOLINTEND(*-include-cleaner, *-avoid-magic-numbers, *-magic-numbers, *-owning-memory, *-pro-bounds-array-to-pointer-decay)
// clang-format on
