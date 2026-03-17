/*
 * Created by gbian on 17/03/2026.
 * Copyright (c) 2026 All rights reserved.
 */

#pragma once

// clang-format off
#include "jsav/ast/Node.hpp"
#include "jsav/ast/Statements.hpp"
// clang-format on

namespace jsv {

    // ============================================================
    // Program: il nodo radice dell'AST
    // ============================================================
    class Program final : public Node {
    public:
        explicit Program(std::vector<StmtPtr> statements, SourceSpan loc = {})
          : Node(NodeKind::Program, loc), statements_{std::move(statements)} {}

        [[nodiscard]] const std::vector<StmtPtr> &statements() const noexcept { return statements_; }

        [[nodiscard]] static constexpr bool classof(const Node *n) { return n->kind() == NodeKind::Program; }

    private:
        std::vector<StmtPtr> statements_;
    };

}  // namespace jsv
