/*
 * Created by gbian on 19/02/2026.
 * Copyright (c) 2026 All rights reserved.
 */

#pragma once

// clang-format off
#include "headers.hpp"
#include "fs/fs.hpp"
#include "location/SourceLocation.hpp"
#include "location/SourceSpan.hpp"
#include "location/LineTracker.hpp"
#include "error/error_codes.hpp"
#include "error/CompileError.hpp"
#include "error/ErrorReporter.hpp"
#include "lexer/Token.hpp"
#include "lexer/Lexer.hpp"
#include "lexer/unicode/UnicodeData.hpp"
#include "lexer/unicode/Utf8.hpp"
#include "ast/Node.hpp"
#include "ast/Expressions.hpp"
#include "ast/Statements.hpp"
#include "ast/Program.hpp"
#include "ast/Visitor.hpp"
#include "ast/Ast_printer.hpp"
// clang-format on