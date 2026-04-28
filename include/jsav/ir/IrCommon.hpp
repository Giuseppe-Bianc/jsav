/*
 * Created by gbian on 25/04/2026.
 * Copyright (c) 2026 All rights reserved.
 */

#pragma once

#include "../headers.hpp"

namespace jsv {

    enum class IrLevel {
        Hir,
        Mir,
        Lir,
    };

    enum class PassKind {
        Validation,
        Analysis,
        Transformation,
        Lowering,
    };

    struct CanonicalKey {
        std::string module;
        std::string function;
        std::string block;
        std::size_t instruction_index{};
        std::size_t operand_index{};

        [[nodiscard]] std::strong_ordering operator<=>(const CanonicalKey &other) const noexcept = default;
        [[nodiscard]] bool operator==(const CanonicalKey &other) const noexcept = default;
        [[nodiscard]] std::string to_string() const;
    };

    struct IrUnit {
        std::string module_name;
        std::vector<std::string> operations;

        [[nodiscard]] bool operator==(const IrUnit &other) const noexcept = default;
        [[nodiscard]] std::string canonical_key() const;
    };

    [[nodiscard]] constexpr std::string_view to_string(const IrLevel level) noexcept {
        switch(level) {
        case IrLevel::Hir:
            return "HIR";
        case IrLevel::Mir:
            return "MIR";
        case IrLevel::Lir:
            return "LIR";
        }
        return "UNKNOWN";
    }

    [[nodiscard]] constexpr std::string_view to_string(const PassKind kind) noexcept {
        switch(kind) {
        case PassKind::Validation:
            return "Validation";
        case PassKind::Analysis:
            return "Analysis";
        case PassKind::Transformation:
            return "Transformation";
        case PassKind::Lowering:
            return "Lowering";
        }
        return "Unknown";
    }

}  // namespace jsv
