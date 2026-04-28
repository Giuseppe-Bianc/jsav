#pragma once

#include "jsav/error/CompileError.hpp"
#include "jsav/ir/Type.hpp"

namespace jsv {

    /// @brief Type validator: nominal equivalence and compatibility checks (T035-T036, US1)
    class TypeValidator {
    public:
        /// Validate nominal type equivalence
        [[nodiscard]] static bool types_equivalent(const TypeBase &t1, const TypeBase &t2) noexcept;

        /// Validate type compatibility for assignment
        [[nodiscard]] static bool types_compatible(const TypeBase &from, const TypeBase &to) noexcept;
    };

}  // namespace jsv
