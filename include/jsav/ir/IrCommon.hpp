/*
 * Created by gbian on 25/04/2026.
 * Copyright (c) 2026 All rights reserved.
 */

#pragma once


#include "../headers.hpp"

namespace jsv {

    /// IR levels in the multi-level IR system
    enum class IrLevel {
        HIR,  ///< High-level IR - closest to source semantics
        MIR,  ///< Mid-level IR - after initial lowering
        LIR,  ///< Low-level IR - machine-close IR
    };

    /// Pass kind classification
    enum class PassKind {
        Analysis,         ///< Read-only analysis pass
        Transformation,   ///< Transformation that may modify IR
        Optimization,     ///< Optimization pass
        Lowering,         ///< Lowering from one IR level to another
        Validation,       ///< Validation pass
        Codegen,          ///< Code generation pass
    };

    /// Canonical key format for deterministic ordering
    struct CanonicalKey {
        std::string_view module_name;
        std::string_view function_name;
        std::uint64_t block_index;
        std::uint64_t instruction_index;
        std::string_view entity_type;  ///< "module", "function", "block", "instruction", etc.

        /// Lexicographic comparison for deterministic ordering
        bool operator<(const CanonicalKey& other) const noexcept;
        bool operator==(const CanonicalKey& other) const noexcept;
        bool operator!=(const CanonicalKey& other) const noexcept;
    };

}  // namespace jsv