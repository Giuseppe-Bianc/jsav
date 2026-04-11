/*
 * Created by gbian on 11 aprile 2026.
 * Copyright (c) 2026 All rights reserved.
 */

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
        default:
            break;
        }
    }

    void visit_type(const TypePtr &type, TypeVisitor &visitor) {
        if(type) { visit_type(*type, visitor); }
    }

}  // namespace jsv
