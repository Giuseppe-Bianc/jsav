/*
 * Created by gbian on 2 aprile 2026.
 * Copyright (c) 2026 All rights reserved.
 */
#include "jsav/typechecker/TypeVariable.hpp"
#include "jsavCore/format.hpp"

namespace jsv {

std::string TypeVariable::to_string() const { return FORMAT("?T{}", id_); }

namespace {
    /// Thread-local counter for generating unique type variable IDs
    std::size_t next_type_var_id() noexcept {
        static thread_local std::size_t counter = 0;
        return ++counter;
    }
}  // namespace

TypePtr fresh_type_variable() noexcept { return std::make_shared<TypeVariable>(next_type_var_id()); }

}  // namespace jsv
