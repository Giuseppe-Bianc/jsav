/*
 * Created by gbian on 17/03/2026.
 * Copyright (c) 2026 All rights reserved.
 */

#include "../../../include/jsav/ast/Statements.hpp"

namespace jsv {
    std::string to_string(Type type) {
        switch(type) {
        case Type::I8:
            return "i8";
        case Type::I16:
            return "i16";
        case Type::I32:
            return "i32";
        case Type::I64:
            return "i64";
        case Type::U8:
            return "u8";
        case Type::U16:
            return "u16";
        case Type::U32:
            return "u32";
        case Type::U64:
            return "u64";
        case Type::F32:
            return "f32";
        case Type::F64:
            return "f64";
        case Type::Char:
            return "char";
        case Type::String:
            return "string";
        case Type::Bool:
            return "bool";
        case Type::Void:
            return "void";
        default:
            return "unknown";
        }
    }
}  // namespace jsv