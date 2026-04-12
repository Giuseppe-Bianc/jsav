/*
 * Created by gbian on 2 aprile 2026.
 * Copyright (c) 2026 All rights reserved.
 */
// NOLINTBEGIN(*-include-cleaner)
#include "jsav/typechecker/ErrorType.hpp"

namespace jsv {

    std::string ErrorType::to_string() const { return "<error>"; }

    TypePtr ErrorType::clone() const noexcept { return error_type(); }

    TypePtr error_type() noexcept {
        static const auto instance = std::make_shared<ErrorType>();
        return instance;
    }

}  // namespace jsv

// NOLINTEND(*-include-cleaner)