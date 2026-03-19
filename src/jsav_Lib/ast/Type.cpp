/*
 * Created by gbian on 19/03/2026.
 * Copyright (c) 2026 All rights reserved.
 */
// NOLINTBEGIN(*-include-cleaner, *-identifier-length)

#include "jsav/ast/Type.hpp"

namespace jsv {

    // Type implementation (if needed for non-inline methods)

}  // namespace jsv

// -------------------------------------------------------------------------
// std::hash
// -------------------------------------------------------------------------

namespace std {

    std::size_t hash<jsv::Type>::operator()(const jsv::Type &type) const noexcept {
        std::size_t seed = 0;
        jsv::hash_combine(seed, static_cast<std::size_t>(type.kind()));

        switch(type.kind()) {
        case jsv::TypeKind::Custom:
            jsv::hash_combine(seed, std::hash<std::string>{}(type.custom_name()));
            break;
        case jsv::TypeKind::Array:
        case jsv::TypeKind::Vector:
            if(type.element_type_) { jsv::hash_combine(seed, std::hash<jsv::Type>{}(*type.element_type_)); }
            break;
        default:
            break;
        }

        return seed;
    }

    std::size_t hash<jsv::Parameter>::operator()(const jsv::Parameter &param) const noexcept {
        std::size_t seed = 0;
        jsv::hash_combine(seed, std::hash<std::string>{}(param.name));
        jsv::hash_combine(seed, std::hash<jsv::Type>{}(param.type_annotation));
        jsv::hash_combine(seed, std::hash<jsv::SourceSpan>{}(param.span));
        return seed;
    }

}  // namespace std

// NOLINTEND(*-include-cleaner, *-identifier-length)
