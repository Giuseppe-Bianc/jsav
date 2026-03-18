/*
 * Created by gbian on 17/03/2026.
 * Copyright (c) 2026 All rights reserved.
 */

#include "jsav/ast/Node.hpp"

namespace jsv {

    [[nodiscard]] std::string_view Node::kind_name() const noexcept { return node_kind_name(kind_); }

    // ============================================================
    // Safe casting utilities (LLVM-style) - template instantiations
    // ============================================================
    // Note: Template definitions must be visible at instantiation site.
    // These are kept inline in the header for now due to their generic nature.
    // If specific instantiations are needed, they can be moved here.

}  // namespace jsv