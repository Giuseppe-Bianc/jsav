/*
 * Created by gbian on 11 aprile 2026.
 * Copyright (c) 2026 All rights reserved.
 */
// NOLINTBEGIN(*-include-cleaner, *-pro-type-static-cast-downcast)
// clang-format off
#include "jsav/typechecker/TypeVisitor.hpp"
// clang-format on

namespace jsv {

    void visit_type(const TypeBase &type, TypeVisitor &visitor) {
        switch(type.kind()) {
        case TypeKind::Array:
            visitor.visit_array(static_cast<const ArrayType &>(type));
            break;
        case TypeKind::Vector:
            visitor.visit_vector(static_cast<const VectorType &>(type));
            break;
        case TypeKind::Custom:
            visitor.visit_custom(static_cast<const CustomType &>(type));
            break;
        default:
            break;
        }
    }

    void visit_type(const TypePtr &type, TypeVisitor &visitor) {
        if(type) { visit_type(*type, visitor); }
    }

}  // namespace jsv

// NOLINTEND(*-include-cleaner, *-pro-type-static-cast-downcast)