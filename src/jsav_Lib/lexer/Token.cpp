/*
 * Created by gbian on 23/02/2026.
 * Copyright (c) 2026 All rights reserved.
 */
// NOLINTBEGIN(*-include-cleaner, *-identifier-length)
#include "jsav/lexer/Token.hpp"

namespace jsv {

    std::string Token::to_string() const { return FORMAT(R"({}("{}") {})", tokenKindToString(m_kind), m_text, m_span); }

    std::ostream &operator<<(std::ostream &os, const Token &token) { return os << token.to_string(); }

}  // namespace jsv
// NOLINTEND(*-include-cleaner, *-identifier-length)