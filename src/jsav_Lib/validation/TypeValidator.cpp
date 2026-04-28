// clang-format off
// NOLINTBEGIN(*-include-cleaner)
// clang-format on

#include "jsav/validation/TypeValidator.hpp"

namespace jsv {

    bool TypeValidator::types_equivalent(const TypeBase &t1, const TypeBase &t2) noexcept {
        if(t1.kind() != t2.kind()) {
            return false;
        }

        if(t1.kind() == TypeKind::UserDefined) {
            const auto *lhs = dynamic_cast<const UserDefinedType *>(&t1);
            const auto *rhs = dynamic_cast<const UserDefinedType *>(&t2);
            if(lhs == nullptr || rhs == nullptr) {
                return false;
            }

            return lhs->is_equivalent_to(*rhs);
        }

        return t1.canonical_name() == t2.canonical_name();
    }

    bool TypeValidator::types_compatible(const TypeBase &from, const TypeBase &to) noexcept {
        return types_equivalent(from, to);
    }

}  // namespace jsv

// clang-format off
// NOLINTEND(*-include-cleaner)
// clang-format on
