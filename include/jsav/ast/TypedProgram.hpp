/*
 * Created by gbian on 1 aprile 2026.
 * Copyright (c) 2026 All rights reserved.
 */

#pragma once

// clang-format off
#include "jsav/ast/TypedNode.hpp"
#include "jsav/ast/TypedStatements.hpp"
// clang-format on

namespace jsv {

    // ============================================================
    // Typed Program Node
    // ============================================================

    /**
     * @brief Typed program root node.
     *
     * Represents the complete typed program with all statements
     * and their resolved type information.
     */
    class TypedProgram final : public TypedNode {
    public:
        // cppcheck-suppress passedByValue
        TypedProgram(std::vector<TypedStmtPtr> statements, TypePtr node_type, SourceSpan loc = {})
          : TypedNode{NodeKind::Program, std::move(node_type), loc}, statements_{std::move(statements)} {}

        [[nodiscard]] const std::vector<TypedStmtPtr> &statements() const noexcept { return statements_; }

        [[nodiscard]] static constexpr bool classof(const Node *n) { return n->kind() == NodeKind::Program; }

    private:
        std::vector<TypedStmtPtr> statements_;
    };

}  // namespace jsv
