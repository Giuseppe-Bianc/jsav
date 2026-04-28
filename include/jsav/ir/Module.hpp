#pragma once

#include "GlobalEntityId.hpp"
#include "IrCommon.hpp"
#include "Type.hpp"

#include <map>
#include <memory>
#include <string>
#include <vector>

namespace jsv {

    class Function;

    /// Function signature: immutable description of a function's interface
    struct FunctionSignature {
        std::vector<TypeRef> param_types;
        TypeRef return_type;

        /// Compare signatures for equality (used in validation)
        [[nodiscard]] bool operator==(const FunctionSignature &other) const noexcept {
            return param_types == other.param_types && return_type == other.return_type;
        }
    };

    /// Type definition version: nominal type with deterministic versioning via SHA-256
    struct TypeDefVersion {
        std::string type_identity;          // Fully qualified type name
        std::string version_id;             // Deterministic version from SHA-256 hash
        std::string layout_spec;            // Binary layout specification
        std::string equivalence_rule;       // "nominal" for this implementation
    };

    /// Module metadata: optional annotations and cross-module traceability
    struct ModuleMetadata {
        std::string source_file;            // Source file path (if any)
        std::string compilation_timestamp;  // ISO 8601 timestamp
        std::map<std::string, std::string> custom_attributes;  // User-defined metadata
    };

    /// @brief Module entity: global IR context and validation boundary (US1, MVPs1)
    /// @invariant module_id must be globally unique following GlobalEntityId format
    /// @invariant name must be non-empty and contain only valid identifier characters
    /// @invariant All TypeRef and FunctionRef within the module resolve to type_table or function_table
    /// @invariant Bidirectional consistency: functions vector and function_table contain same set of names
    /// @invariant No forward declarations; all referenced entities must be defined within the module
    class Module {
    public:
        // ── Construction ──────────────────────────────────────────────────────────
        /// Create a module with a unique ID derived from its canonical path
        explicit Module(std::string module_name) noexcept;

        // ── Value semantics
        Module(const Module &) = delete;
        Module &operator=(const Module &) = delete;
        Module(Module &&) noexcept = default;
        Module &operator=(Module &&) noexcept = default;
        ~Module() = default;

        // ── Accessors (immutable after construction) ──────────────────────────────

        /// Get the module's globally unique ID
        [[nodiscard]] const GlobalEntityId &id() const noexcept { return module_id_; }

        /// Get the module's name
        [[nodiscard]] std::string_view name() const noexcept { return name_; }

        /// Get all functions in the module (const access)
        [[nodiscard]] const std::vector<std::unique_ptr<Function>> &functions() const noexcept { return functions_; }

        /// Get the function table (name -> signature mapping)
        [[nodiscard]] const std::map<std::string, FunctionSignature> &function_table() const noexcept {
            return function_table_;
        }

        /// Get the type table (name -> type definition mapping)
        [[nodiscard]] const std::map<std::string, TypeDefVersion> &type_table() const noexcept { return type_table_; }

        // ── Mutation (transactional context) ───────────────────────────────────────

        /// Add a function to the module; validates and maintains bidirectional consistency
        /// Returns std::expected with error details if validation fails
        [[nodiscard]] std::expected<void, std::vector<CompileError>> add_function(
            std::unique_ptr<Function> func) noexcept;

        /// Register a user-defined type in the module; validates type layout
        /// Returns std::expected with error details if validation fails
        [[nodiscard]] std::expected<void, std::vector<CompileError>>
        register_type(std::string type_name, TypeDefVersion type_def) noexcept;

        // ── Validation ────────────────────────────────────────────────────────────

        /// Validate module structural invariants:
        /// - Bidirectional function consistency
        /// - Type table completeness
        /// - No forward references
        [[nodiscard]] std::expected<void, std::vector<CompileError>> validate() const noexcept;

    private:
        GlobalEntityId module_id_;
        std::string name_;
        std::vector<std::unique_ptr<Function>> functions_;
        std::map<std::string, FunctionSignature> function_table_;
        std::map<std::string, TypeDefVersion> type_table_;
        ModuleMetadata metadata_;
    };

}  // namespace jsv
