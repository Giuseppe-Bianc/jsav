/*
 * Created by gbian on 2 aprile 2026.
 * Copyright (c) 2026 All rights reserved.
 */
#include "jsav/typechecker/ErrorType.hpp"
#include "jsavCore/format.hpp"

namespace jsv {

std::string ErrorType::to_string() const { return "<error>"; }

TypePtr error_type() noexcept {
    static const auto instance = std::make_shared<ErrorType>();
    return instance;
}

}  // namespace jsv
