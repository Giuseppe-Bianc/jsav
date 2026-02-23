/*
 * Created by gbian on 23/02/2026.
 * Copyright (c) 2026 All rights reserved.
 */

#include "jsav/lexer/Token.hpp"

namespace jsv {

    std::string Token::to_string() const {
        return FORMAT("{}({})", to_string(m_kind), m_text);
    }

    std::ostream &operator<<(std::ostream &os, const Token &token) {
        return os << token.to_string();
    }

}  // namespace jsv