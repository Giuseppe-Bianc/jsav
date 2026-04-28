/*
 * Created by gbian on 25/04/2026.
 * Copyright (c) 2026 All rights reserved.
 */
// NOLINTBEGIN(*-include-cleaner)
#include "jsav/ir/IrCommon.hpp"

namespace jsv {

    std::string CanonicalKey::to_string() const {
        return FORMAT("{}/{}/{}#{}:{}", module, function, block, instruction_index, operand_index);
    }

    std::string IrUnit::canonical_key() const { return FORMAT("{}:{}", module_name, operations.size()); }

}  // namespace jsv
// NOLINTEND(*-include-cleaner)
