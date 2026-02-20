/*
 * Created by gbian on 20/02/2026.
 * Copyright (c) 2026 All rights reserved.
 */

#include "jsav/lexer/SourceLocation.hpp"
namespace jsv {

    std::string SourceLocation::to_string() const { return FORMAT("line {}:column {} (offset: {})", line, column, absolute_pos); }

    std::ostream &operator<<(std::ostream &os, const SourceLocation &loc) { return os << loc.to_string(); }

}  // namespace jsv

namespace std {

    std::size_t hash<jsv::SourceLocation>::operator()(const jsv::SourceLocation &loc) const noexcept {
        std::size_t seed = 0;
        auto combine = [&](std::size_t v) { seed ^= std::hash<std::size_t>{}(v) + 0x9e3779b9 + (seed << 6) + (seed >> 2); };
        combine(loc.line);
        combine(loc.column);
        combine(loc.absolute_pos);
        return seed;
    }

}  // namespace std