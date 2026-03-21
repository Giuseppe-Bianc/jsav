# Quick Start Guide: AST Parser

**Date**: 2026-03-21
**Branch**: `006-ast-parser-implementation`
**Status**: Complete

---

## Overview

This guide provides quick start examples for using the AST Parser module in the jsav compiler. The parser transforms token streams from the Lexer into typed Abstract Syntax Trees (AST) while collecting syntax errors.

**Target Audience**: Compiler developers implementing or extending the parser module.

**Prerequisites**:
- C++23 compatible compiler (GCC 14+, Clang 18+, MSVC 2022+)
- Existing Lexer module producing `std::vector<Token>`
- CMake 4.2+ build system

---

## Basic Usage

### Example 1: Parse Complete Program

```cpp
#include "jsav/headers.hpp"
#include "jsav/parser/Parser.hpp"

int main() {
    // Step 1: Create lexer (assumes existing Lexer module)
    std::string source = R"(
        var x = 42;
        fun add(a: i32, b: i32) -> i32 {
            return a + b;
        }
        print add(1, 2);
    )";
    
    Lexer lexer(source);
    std::vector<Token> tokens = lexer.tokenize();
    
    // Step 2: Create parser with token stream
    Parser parser(tokens);
    
    // Step 3: Parse into AST
    auto [ast, errors] = parser.parse();
    
    // Step 4: Check for errors
    if (!errors.empty()) {
        for (const auto& error : errors) {
            std::print("Error: {} at {}:{}\n", 
                      error.message(), 
                      error.location().line,
                      error.location().column);
        }
        return 1;
    }
    
    // Step 5: Use AST (ast is std::unique_ptr<Program>)
    std::print("Parsed {} top-level declarations\n", 
              ast->declarations().size());
    
    return 0;
}
```

**Output**:
```text
Parsed 3 top-level declarations
```

---

### Example 2: Parse Expression with Precedence

```cpp
#include "jsav/parser/Parser.hpp"

int main() {
    // Token stream for: 1 + 2 * 3
    std::vector<Token> tokens = {
        Token{TokenKind::Integer, "1", {0, 1, 1}},
        Token{TokenKind::Plus, "+", {0, 1, 3}},
        Token{TokenKind::Integer, "2", {0, 1, 5}},
        Token{TokenKind::Star, "*", {0, 1, 7}},
        Token{TokenKind::Integer, "3", {0, 1, 9}},
        Token{TokenKind::Eof, "", {0, 1, 10}}
    };
    
    Parser parser(tokens);
    auto [ast, errors] = parser.parse();
    
    // AST structure: Program → ExprStmt → BinaryExpr(+)
    //   left: IntegerLiteral(1)
    //   right: BinaryExpr(*)
    //     left: IntegerLiteral(2)
    //     right: IntegerLiteral(3)
    
    return 0;
}
```

**AST Structure**:
```
Program
└── ExprStmt
    └── BinaryExpr(+)
        ├── IntegerLiteral(1)
        └── BinaryExpr(*)
            ├── IntegerLiteral(2)
            └── IntegerLiteral(3)
```

---

### Example 3: Error Detection and Recovery

```cpp
#include "jsav/parser/Parser.hpp"

int main() {
    // Token stream with syntax errors: var x = (1 + 2;
    std::vector<Token> tokens = {
        Token{TokenKind::Var, "var", {0, 1, 1}},
        Token{TokenKind::Identifier, "x", {0, 1, 5}},
        Token{TokenKind::Equal, "=", {0, 1, 7}},
        Token{TokenKind::LeftParen, "(", {0, 1, 9}},
        Token{TokenKind::Integer, "1", {0, 1, 10}},
        Token{TokenKind::Plus, "+", {0, 1, 12}},
        Token{TokenKind::Integer, "2", {0, 1, 14}},
        Token{TokenKind::Semicolon, ";", {0, 1, 15}},
        Token{TokenKind::Eof, "", {0, 1, 16}}
    };
    
    Parser parser(tokens);
    auto [ast, errors] = parser.parse();
    
    // Errors collected:
    // - E0101: Unbalanced parentheses at line 1, column 9
    // Parser recovered and continued parsing
    
    std::print("Collected {} errors\n", errors.size());
    for (const auto& error : errors) {
        std::print("  [{}] {} at {}:{}\n",
                  error.code(),
                  error.message(),
                  error.location().line,
                  error.location().column);
    }
    
    return 0;
}
```

**Output**:
```text
Collected 1 errors
  [E0101] Unbalanced parentheses at 1:9
```

---

### Example 4: Context Validation (Break Outside Loop)

```cpp
#include "jsav/parser/Parser.hpp"

int main() {
    // Token stream: break; (outside loop)
    std::vector<Token> tokens = {
        Token{TokenKind::Break, "break", {0, 1, 1}},
        Token{TokenKind::Semicolon, ";", {0, 1, 6}},
        Token{TokenKind::Eof, "", {0, 1, 7}}
    };
    
    Parser parser(tokens);
    auto [ast, errors] = parser.parse();
    
    // Error detected: E0304 - Break outside loop
    std::print("Errors: {}\n", errors.size());  // Prints: 1
    
    return 0;
}
```

**Output**:
```text
Errors: 1
```

---

## Module Architecture

### Parser Module Components

```
Parser (Orchestrator)
├── Maintains parser state (tokens, index, errors, panic mode)
├── Token navigation (advance, peek, check, match, expect)
├── Error handling (report_error, synchronize)
└── Context management (push_context, pop_context, RAII guards)

ExpressionParser (Pratt Parsing)
├── Binding power table (12 levels)
├── Prefix parse table (literals, identifiers, unary operators)
├── Infix parse table (binary operators, calls, indexing)
└── parse_expression(min_precedence) main loop

StatementParser (Recursive Descent)
├── parse_statement() dispatch
├── parse_var_declaration()
├── parse_function_declaration()
├── parse_if_statement(), parse_while_statement(), parse_for_statement()
├── parse_return_statement(), parse_break_statement(), parse_continue_statement()
└── parse_block(), parse_print_statement(), parse_expression_statement()
```

### Dependency Flow

```text
Lexer → Parser → AST
  ↓       ↓       ↓
Token   State   Node
Vector  Mgmt    Tree
```

**Key Constraints**:
- Unidirectional dependency (no cycles)
- Parser holds non-owning reference to tokens
- AST owns all node memory via std::unique_ptr

---

## Common Patterns

### Pattern 1: AST Traversal

```cpp
void visitExpr(const Expr* expr) {
    switch (expr->kind()) {
        case NodeKind::BinaryExpr: {
            const auto* bin = static_cast<const BinaryExpr*>(expr);
            std::print("Binary operator: {}\n", bin->op());
            visitExpr(bin->left().get());
            visitExpr(bin->right().get());
            break;
        }
        case NodeKind::IntegerLiteral: {
            const auto* lit = static_cast<const IntegerLiteral*>(expr);
            std::print("Integer: {}\n", lit->value());
            break;
        }
        // ... handle other expression types
        default:
            break;
    }
}
```

### Pattern 2: Error Handling

```cpp
auto [ast, errors] = parser.parse();

if (!errors.empty()) {
    // Group errors by severity
    std::vector<CompileError> fatalErrors;
    std::vector<CompileError> warnings;
    
    for (const auto& error : errors) {
        if (error.code() >= E0400) {
            fatalErrors.push_back(error);
        } else {
            warnings.push_back(error);
        }
    }
    
    // Report fatal errors first
    for (const auto& error : fatalErrors) {
        reportFatal(error);
    }
    
    // Exit if too many fatal errors
    if (fatalErrors.size() > 10) {
        std::print("Compilation aborted due to excessive errors\n");
        return 1;
    }
}
```

### Pattern 3: Context-Aware Parsing

```cpp
// Inside StatementParser::parse_while_statement()
auto condition = expression_parser_.parse_expression();
auto body = parse_statement();

// Context validation happens automatically via RAII
// WhileStmt constructor doesn't need explicit context check
return std::make_unique<WhileStmt>(
    std::move(condition),
    std::move(body),
    location
);

// ContextGuard automatically pops loop context on scope exit
```

---

## Extension Examples

### Adding a New Operator (Modulo %)

**Step 1**: Add TokenKind (in lexer)

```cpp
// lexer/Token.hpp
enum class TokenKind {
    // ... existing tokens
    Mod,  // Add % token
};
```

**Step 2**: Register in binding power table

```cpp
// ExpressionParser.cpp
constexpr static std::array<BindingPower, 256> bindingPowers = [] {
    std::array<BindingPower, 256> bp{};
    // ... existing operators
    bp[static_cast<size_t>(TokenKind::Mod)] = {7, 8};  // Same as * /
    return bp;
}();
```

**Step 3**: Add parse function

```cpp
ExprPtr ExpressionParser::parseModulo(ExprPtr left) {
    auto right = parse_expression(7);  // Right binding power
    return std::make_unique<BinaryExpr>(
        std::move(left),
        TokenKind::Mod,
        std::move(right),
        location
    );
}
```

**Step 4**: Register in infix table

```cpp
// ExpressionParser constructor
infix_parse_table_[TokenKind::Mod] = &ExpressionParser::parseModulo;
```

---

### Adding a New Statement (Switch)

**Step 1**: Add NodeKind

```cpp
// NodeKind.hpp
enum class NodeKind {
    // ... existing nodes
    SwitchStmt,
};
```

**Step 2**: Create node class

```cpp
// Statements.hpp
class SwitchStmt : public Stmt {
public:
    using Ptr = std::unique_ptr<SwitchStmt>;
    
    SwitchStmt(ExprPtr condition, 
               std::vector<CaseClause> cases,
               StmtPtr defaultCase,
               SourceLocation location);
    
    // ... implement kind(), location(), accessors
};
```

**Step 3**: Implement parser

```cpp
// StatementParser.cpp
StmtPtr StatementParser::parse_switch_statement() {
    expect(TokenKind::LeftParen, "Expected '(' after switch");
    auto condition = expression_parser_.parse_expression();
    expect(TokenKind::RightParen, "Expected ')' after condition");
    
    expect(TokenKind::LeftBrace, "Expected '{' before switch body");
    auto cases = parse_case_clauses();
    auto defaultCase = parse_default_clause();
    expect(TokenKind::RightBrace, "Expected '}' after switch body");
    
    return std::make_unique<SwitchStmt>(
        std::move(condition),
        std::move(cases),
        std::move(defaultCase),
        location
    );
}
```

**Step 4**: Add dispatch case

```cpp
// StatementParser::parse_statement()
if (match(TokenKind::Switch)) {
    return parse_switch_statement();
}
```

---

## Testing Examples

### Unit Test: Expression Precedence

```cpp
#include <catch2/catch_test_macros.hpp>

TEST_CASE("Parser respects operator precedence", "[parser][expressions]") {
    std::vector<Token> tokens = {
        Token{TokenKind::Integer, "1", {}},
        Token{TokenKind::Plus, "+", {}},
        Token{TokenKind::Integer, "2", {}},
        Token{TokenKind::Star, "*", {}},
        Token{TokenKind::Integer, "3", {}},
        Token{TokenKind::Eof, "", {}}
    };
    
    Parser parser(tokens);
    auto [ast, errors] = parser.parse();
    
    REQUIRE(errors.empty());
    REQUIRE(ast->declarations().size() == 1);
    
    // Verify AST structure: 1 + (2 * 3)
    const auto* exprStmt = dynamic_cast<const ExprStmt*>(ast->declarations()[0].get());
    REQUIRE(exprStmt != nullptr);
    
    const auto* add = dynamic_cast<const BinaryExpr*>(&exprStmt->expression());
    REQUIRE(add != nullptr);
    REQUIRE(add->op() == TokenKind::Plus);
    
    const auto* mul = dynamic_cast<const BinaryExpr*>(&add->right());
    REQUIRE(mul != nullptr);
    REQUIRE(mul->op() == TokenKind::Star);
}
```

### Unit Test: Error Detection

```cpp
TEST_CASE("Parser detects unbalanced parentheses", "[parser][errors]") {
    std::vector<Token> tokens = {
        Token{TokenKind::LeftParen, "(", {0, 1, 1}},
        Token{TokenKind::Integer, "1", {0, 1, 2}},
        Token{TokenKind::Plus, "+", {0, 1, 3}},
        Token{TokenKind::Integer, "2", {0, 1, 4}},
        Token{TokenKind::Eof, "", {0, 1, 5}}
    };
    
    Parser parser(tokens);
    auto [ast, errors] = parser.parse();
    
    REQUIRE(errors.size() == 1);
    REQUIRE(errors[0].code() == E0101);
    REQUIRE(errors[0].location().column == 1);
}
```

### Constexpr Test: Binding Power Constants

```cpp
#include <catch2/catch_test_macros.hpp>

TEST_CASE("Binding power table has correct precedence levels", "[parser][constexpr]") {
    constexpr auto mulBP = getBindingPower(TokenKind::Star);
    constexpr auto addBP = getBindingPower(TokenKind::Plus);
    
    STATIC_REQUIRE(mulBP.left > addBP.left);  // * binds tighter than +
    STATIC_REQUIRE(mulBP.right == addBP.left + 1);  // Left-assoc
}
```

---

## Build Integration

### CMake Configuration

```cmake
# src/jsav_Lib/CMakeLists.txt

# Add parser sources to jsav_lib target
target_sources(jsav_lib
    PRIVATE
        parser/Parser.cpp
        parser/ExpressionParser.cpp
        parser/StatementParser.cpp
)

# Add include directories
target_include_directories(jsav_lib
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/../../include/jsav/parser
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/parser
)
```

### Header Installation

```cmake
# CMakeLists.txt (root)

install(DIRECTORY include/jsav/parser/
    DESTINATION include/jsav/parser
    FILES_MATCHING PATTERN "*.hpp"
)
```

---

## Troubleshooting

### Issue: Parser Crashes on Deep Nesting

**Symptom**: Stack overflow on deeply nested expressions (e.g., `((((...))))`)

**Solution**:
- Parser uses recursive calls for Pratt parsing and statement descent
- Default stack size is typically 1-8 MB (platform-dependent)
- Mitigation: Increase stack size via linker flags or refactor to use explicit stack

```cmake
# Increase stack size (MSVC)
target_link_options(jsav_lib PRIVATE /STACK:8388608)  # 8 MB

# Increase stack size (GCC/Clang)
target_link_options(jsav_lib PRIVATE -Wl,-z,stack-size=8388608)
```

### Issue: Memory Leak Detected

**Symptom**: AddressSanitizer reports memory leak

**Solution**:
- Verify all AST nodes use std::unique_ptr
- Check for raw `new` statements (should be `std::make_unique`)
- Ensure Program node (AST root) is properly destroyed

```cpp
// Correct: unique_ptr automatically deletes
auto [ast, errors] = parser.parse();
// ast is std::unique_ptr<Program>, destroyed on scope exit

// Incorrect: raw pointer leak
auto* ast = parser.parse().first;  // LEAK!
delete ast;  // Manual cleanup required (error-prone)
```

### Issue: Incorrect Precedence

**Symptom**: Expression `1 + 2 * 3` parses as `(1 + 2) * 3`

**Solution**:
- Verify binding power table has correct values
- Check that multiplication has higher left_bp than addition
- Ensure Pratt loop uses correct min_precedence

```cpp
// Correct binding powers
constexpr static std::array<BindingPower, 256> bindingPowers = [] {
    std::array<BindingPower, 256> bp{};
    bp[static_cast<size_t>(TokenKind::Star)] = {7, 8};   // *
    bp[static_cast<size_t>(TokenKind::Plus)] = {6, 7};   // +
    return bp;
}();
```

---

## Performance Tips

### Tip 1: Minimize Token Copies

```cpp
// Correct: Non-owning reference
Parser parser(tokens);  // tokens is std::vector<Token>&

// Incorrect: Unnecessary copy
Parser parser(std::move(tokens));  // Parser doesn't own tokens
```

### Tip 2: Reserve AST Node Capacity

```cpp
// If approximate token count is known
class Program {
    std::vector<StmtPtr> declarations_;
    
    void reserve(std::size_t estimatedCount) {
        declarations_.reserve(estimatedCount);
    }
};
```

### Tip 3: Profile Before Optimizing

```bash
# Build with RelWithDebInfo for profiling
cmake -DCMAKE_BUILD_TYPE=RelWithDebInfo ..

# Profile with perf (Linux)
perf record --build ./jsav input.vn
perf report

# Profile with Visual Studio Profiler (Windows)
# Debug → Profiler → Performance Explorer
```

---

## Next Steps

After implementing the parser:

1. **Semantic Analysis**: Add type checking, symbol resolution, scope management
2. **Intermediate Representation**: Lower AST to IR for optimization
3. **Code Generation**: Generate target code from optimized IR
4. **Error Recovery Enhancement**: Improve error messages with suggestions
5. **Performance Optimization**: Profile and optimize hot paths

---

## References

- [data-model.md](./data-model.md) — Complete AST node definitions
- [research.md](./research.md) — Algorithmic design decisions
- [spec.md](./spec.md) — Feature requirements
- [AGENTS.md](../../AGENTS.md) — Project architecture and build instructions
