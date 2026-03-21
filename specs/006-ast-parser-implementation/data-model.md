# Data Model: AST Parser

**Date**: 2026-03-21
**Branch**: `006-ast-parser-implementation`
**Status**: Complete

---

## Overview

This document defines the Abstract Syntax Tree (AST) data model for the jsav compiler parser. The AST represents the syntactic structure of source code as a tree of nodes, where each node corresponds to a language construct (expression, statement, declaration). The data model is organized into three categories:

1. **Base Types**: Common interfaces and enums shared by all nodes
2. **Expression Nodes**: 16 types representing all expression constructs
3. **Statement Nodes**: 11 types representing all statement and declaration constructs

---

## Base Types

### NodeKind (Enum)

**Purpose**: Discriminated union tag for AST node type identification.

```cpp
enum class NodeKind {
    // Expressions (16 types)
    IntegerLiteral,
    FloatLiteral,
    StringLiteral,
    BoolLiteral,
    NullLiteral,
    Identifier,
    UnaryExpr,
    BinaryExpr,
    TernaryExpr,
    CallExpr,
    IndexExpr,
    MemberExpr,
    AssignExpr,
    CastExpr,
    ArrayLiteral,
    GroupingExpr,
    
    // Statements (11 types)
    ExprStmt,
    VarDecl,
    FuncDecl,
    ReturnStmt,
    IfStmt,
    WhileStmt,
    ForStmt,
    BlockStmt,
    BreakStmt,
    ContinueStmt,
    PrintStmt,
    
    // Top-level
    Program
};
```

**Source**: Derived from spec.md FR-002, FR-003

---

### SourceLocation (Struct)

**Purpose**: Tracks source code position for error reporting and debugging.

```cpp
struct SourceLocation {
    std::string_view file;      // Source file path (non-owning)
    int line;                    // 1-based line number
    int column;                  // 1-based column number
    int length;                  // Span length in characters
    
    constexpr SourceLocation() noexcept = default;
    constexpr SourceLocation(std::string_view file, int line, int column, int length) noexcept;
};
```

**Validation Rules**:
- `line >= 1`
- `column >= 1`
- `length >= 0`

**Source**: Derived from spec.md FR-008 (error reporting with source location)

---

### Node (Base Class)

**Purpose**: Abstract base class for all AST nodes.

```cpp
class Node {
public:
    [[nodiscard]] virtual NodeKind kind() const noexcept = 0;
    [[nodiscard]] virtual SourceLocation location() const noexcept = 0;
    virtual ~Node() = default;
    
    // Disable copying
    Node(const Node&) = delete;
    Node& operator=(const Node&) = delete;
    
    // Enable moving
    Node(Node&&) noexcept = default;
    Node& operator=(Node&&) noexcept = default;
};
```

**Memory Ownership**: Non-copyable, movable. Derived classes own child nodes via std::unique_ptr.

---

### Expr (Base Class for Expressions)

**Purpose**: Base class for all expression nodes.

```cpp
class Expr : public Node {
public:
    ~Expr() override = default;
};
```

---

### Stmt (Base Class for Statements)

**Purpose**: Base class for all statement nodes.

```cpp
class Stmt : public Node {
public:
    ~Stmt() override = default;
};
```

---

### Program (Root Node)

**Purpose**: Root of AST. Contains all top-level declarations.

```cpp
class Program : public Node {
public:
    using Ptr = std::unique_ptr<Program>;
    
    explicit Program(std::vector<StmtPtr> declarations, SourceLocation location = {});
    
    [[nodiscard]] NodeKind kind() const noexcept override { return NodeKind::Program; }
    [[nodiscard]] SourceLocation location() const noexcept override { return location_; }
    [[nodiscard]] const std::vector<StmtPtr>& declarations() const noexcept { return declarations_; }
    
private:
    std::vector<StmtPtr> declarations_;
    SourceLocation location_;
};
```

**Relationships**:
- Contains 0..n `StmtPtr` (top-level declarations)
- Owns all child statements via std::unique_ptr

**Source**: Derived from spec.md FR-001 (Program node as root)

---

## Expression Nodes (16 Types)

### IntegerLiteral

**Purpose**: Represents integer literal values (e.g., `42`, `0xFF`, `0b1010`).

```cpp
class IntegerLiteral : public Expr {
public:
    using Ptr = std::unique_ptr<IntegerLiteral>;
    
    explicit IntegerLiteral(std::int64_t value, SourceLocation location);
    
    [[nodiscard]] NodeKind kind() const noexcept override { return NodeKind::IntegerLiteral; }
    [[nodiscard]] SourceLocation location() const noexcept override { return location_; }
    [[nodiscard]] std::int64_t value() const noexcept { return value_; }
    
private:
    std::int64_t value_;
    SourceLocation location_;
};
```

**Fields**:
- `value_`: Integer value (64-bit signed)
- `location_`: Source location of literal

**Source**: spec.md FR-002

---

### FloatLiteral

**Purpose**: Represents floating-point literal values (e.g., `3.14`, `2.5e-3`).

```cpp
class FloatLiteral : public Expr {
public:
    using Ptr = std::unique_ptr<FloatLiteral>;
    
    explicit FloatLiteral(double value, SourceLocation location);
    
    [[nodiscard]] NodeKind kind() const noexcept override { return NodeKind::FloatLiteral; }
    [[nodiscard]] SourceLocation location() const noexcept override { return location_; }
    [[nodiscard]] double value() const noexcept { return value_; }
    
private:
    double value_;
    SourceLocation location_;
};
```

**Fields**:
- `value_`: Floating-point value (64-bit double)
- `location_`: Source location of literal

**Source**: spec.md FR-002

---

### StringLiteral

**Purpose**: Represents string literal values (e.g., `"hello\nworld"`).

```cpp
class StringLiteral : public Expr {
public:
    using Ptr = std::unique_ptr<StringLiteral>;
    
    explicit StringLiteral(std::string value, SourceLocation location);
    
    [[nodiscard]] NodeKind kind() const noexcept override { return NodeKind::StringLiteral; }
    [[nodiscard]] SourceLocation location() const noexcept override { return location_; }
    [[nodiscard]] const std::string& value() const noexcept { return value_; }
    
private:
    std::string value_;
    SourceLocation location_;
};
```

**Fields**:
- `value_`: String value (with escape sequences processed)
- `location_`: Source location of literal

**Source**: spec.md FR-002

---

### BoolLiteral

**Purpose**: Represents boolean literal values (`true`, `false`).

```cpp
class BoolLiteral : public Expr {
public:
    using Ptr = std::unique_ptr<BoolLiteral>;
    
    explicit BoolLiteral(bool value, SourceLocation location);
    
    [[nodiscard]] NodeKind kind() const noexcept override { return NodeKind::BoolLiteral; }
    [[nodiscard]] SourceLocation location() const noexcept override { return location_; }
    [[nodiscard]] bool value() const noexcept { return value_; }
    
private:
    bool value_;
    SourceLocation location_;
};
```

**Fields**:
- `value_`: Boolean value
- `location_`: Source location of literal

**Source**: spec.md FR-002

---

### NullLiteral

**Purpose**: Represents null literal value (`null`).

```cpp
class NullLiteral : public Expr {
public:
    using Ptr = std::unique_ptr<NullLiteral>;
    
    explicit NullLiteral(SourceLocation location);
    
    [[nodiscard]] NodeKind kind() const noexcept override { return NodeKind::NullLiteral; }
    [[nodiscard]] SourceLocation location() const noexcept override { return location_; }
    
private:
    SourceLocation location_;
};
```

**Fields**:
- `location_`: Source location of literal

**Source**: spec.md FR-002

---

### Identifier

**Purpose**: Represents identifier references (e.g., variable names, function names).

```cpp
class Identifier : public Expr {
public:
    using Ptr = std::unique_ptr<Identifier>;
    
    explicit Identifier(std::string name, SourceLocation location);
    
    [[nodiscard]] NodeKind kind() const noexcept override { return NodeKind::Identifier; }
    [[nodiscard]] SourceLocation location() const noexcept override { return location_; }
    [[nodiscard]] const std::string& name() const noexcept { return name_; }
    
private:
    std::string name_;
    SourceLocation location_;
};
```

**Fields**:
- `name_`: Identifier text
- `location_`: Source location of identifier

**Source**: spec.md FR-002

---

### UnaryExpr

**Purpose**: Represents unary operator expressions (e.g., `-x`, `!flag`, `~mask`).

```cpp
class UnaryExpr : public Expr {
public:
    using Ptr = std::unique_ptr<UnaryExpr>;
    
    UnaryExpr(TokenKind op, ExprPtr operand, SourceLocation location);
    
    [[nodiscard]] NodeKind kind() const noexcept override { return NodeKind::UnaryExpr; }
    [[nodiscard]] SourceLocation location() const noexcept override { return location_; }
    [[nodiscard]] TokenKind op() const noexcept { return op_; }
    [[nodiscard]] const ExprPtr& operand() const noexcept { return operand_; }
    
private:
    TokenKind op_;         // TokenKind::Minus, TokenKind::Bang, TokenKind::Tilde, etc.
    ExprPtr operand_;      // Owned sub-expression
    SourceLocation location_;
};
```

**Fields**:
- `op_`: Unary operator token kind
- `operand_`: Operand expression (owned)
- `location_`: Source location of operator

**Relationships**:
- Owns one `ExprPtr` (operand)

**Source**: spec.md FR-002

---

### BinaryExpr

**Purpose**: Represents binary operator expressions (e.g., `a + b`, `x * y`).

```cpp
class BinaryExpr : public Expr {
public:
    using Ptr = std::unique_ptr<BinaryExpr>;
    
    BinaryExpr(ExprPtr left, TokenKind op, ExprPtr right, SourceLocation location);
    
    [[nodiscard]] NodeKind kind() const noexcept override { return NodeKind::BinaryExpr; }
    [[nodiscard]] SourceLocation location() const noexcept override { return location_; }
    [[nodiscard]] TokenKind op() const noexcept { return op_; }
    [[nodiscard]] const ExprPtr& left() const noexcept { return left_; }
    [[nodiscard]] const ExprPtr& right() const noexcept { return right_; }
    
private:
    ExprPtr left_;         // Owned left operand
    TokenKind op_;         // Binary operator token kind
    ExprPtr right_;        // Owned right operand
    SourceLocation location_;
};
```

**Fields**:
- `left_`: Left operand expression (owned)
- `op_`: Binary operator token kind
- `right_`: Right operand expression (owned)
- `location_`: Source location of operator

**Relationships**:
- Owns two `ExprPtr` (left and right operands)

**Source**: spec.md FR-002, research.md Decision 1 (operator precedence)

---

### TernaryExpr

**Purpose**: Represents ternary conditional expressions (e.g., `cond ? thenVal : elseVal`).

```cpp
class TernaryExpr : public Expr {
public:
    using Ptr = std::unique_ptr<TernaryExpr>;
    
    TernaryExpr(ExprPtr condition, ExprPtr thenBranch, ExprPtr elseBranch, SourceLocation location);
    
    [[nodiscard]] NodeKind kind() const noexcept override { return NodeKind::TernaryExpr; }
    [[nodiscard]] SourceLocation location() const noexcept override { return location_; }
    [[nodiscard]] const ExprPtr& condition() const noexcept { return condition_; }
    [[nodiscard]] const ExprPtr& thenBranch() const noexcept { return thenBranch_; }
    [[nodiscard]] const ExprPtr& elseBranch() const noexcept { return elseBranch_; }
    
private:
    ExprPtr condition_;    // Owned condition expression
    ExprPtr thenBranch_;   // Owned then-branch expression
    ExprPtr elseBranch_;   // Owned else-branch expression
    SourceLocation location_;
};
```

**Fields**:
- `condition_`: Condition expression (owned)
- `thenBranch_`: Then-branch expression (owned)
- `elseBranch_`: Else-branch expression (owned)
- `location_`: Source location of `?` token

**Relationships**:
- Owns three `ExprPtr` (condition, then-branch, else-branch)

**Source**: spec.md FR-002

---

### CallExpr

**Purpose**: Represents function call expressions (e.g., `func(arg1, arg2)`).

```cpp
class CallExpr : public Expr {
public:
    using Ptr = std::unique_ptr<CallExpr>;
    
    CallExpr(ExprPtr callee, std::vector<ExprPtr> arguments, SourceLocation location);
    
    [[nodiscard]] NodeKind kind() const noexcept override { return NodeKind::CallExpr; }
    [[nodiscard]] SourceLocation location() const noexcept override { return location_; }
    [[nodiscard]] const ExprPtr& callee() const noexcept { return callee_; }
    [[nodiscard]] const std::vector<ExprPtr>& arguments() const noexcept { return arguments_; }
    
private:
    ExprPtr callee_;                    // Owned callee expression (function)
    std::vector<ExprPtr> arguments_;    // Owned argument expressions
    SourceLocation location_;
};
```

**Fields**:
- `callee_`: Callee expression (owned, typically Identifier or MemberExpr)
- `arguments_`: Vector of argument expressions (owned)
- `location_`: Source location of `(` token

**Relationships**:
- Owns one `ExprPtr` (callee)
- Owns 0..n `ExprPtr` (arguments)

**Source**: spec.md FR-002

---

### IndexExpr

**Purpose**: Represents array indexing expressions (e.g., `array[index]`).

```cpp
class IndexExpr : public Expr {
public:
    using Ptr = std::unique_ptr<IndexExpr>;
    
    IndexExpr(ExprPtr array, ExprPtr index, SourceLocation location);
    
    [[nodiscard]] NodeKind kind() const noexcept override { return NodeKind::IndexExpr; }
    [[nodiscard]] SourceLocation location() const noexcept override { return location_; }
    [[nodiscard]] const ExprPtr& array() const noexcept { return array_; }
    [[nodiscard]] const ExprPtr& index() const noexcept { return index_; }
    
private:
    ExprPtr array_;      // Owned array expression
    ExprPtr index_;      // Owned index expression
    SourceLocation location_;
};
```

**Fields**:
- `array_`: Array expression (owned)
- `index_`: Index expression (owned)
- `location_`: Source location of `[` token

**Relationships**:
- Owns two `ExprPtr` (array and index)

**Source**: spec.md FR-002

---

### MemberExpr

**Purpose**: Represents member access expressions (e.g., `object.member`).

```cpp
class MemberExpr : public Expr {
public:
    using Ptr = std::unique_ptr<MemberExpr>;
    
    MemberExpr(ExprPtr object, std::string memberName, SourceLocation location);
    
    [[nodiscard]] NodeKind kind() const noexcept override { return NodeKind::MemberExpr; }
    [[nodiscard]] SourceLocation location() const noexcept override { return location_; }
    [[nodiscard]] const ExprPtr& object() const noexcept { return object_; }
    [[nodiscard]] const std::string& memberName() const noexcept { return memberName_; }
    
private:
    ExprPtr object_;     // Owned object expression
    std::string memberName_;  // Member name (copied for convenience)
    SourceLocation location_;
};
```

**Fields**:
- `object_`: Object expression (owned)
- `memberName_`: Member name text
- `location_`: Source location of `.` token

**Relationships**:
- Owns one `ExprPtr` (object)

**Source**: spec.md FR-002

---

### AssignExpr

**Purpose**: Represents assignment expressions (e.g., `x = 5`, `arr[0] = val`).

```cpp
class AssignExpr : public Expr {
public:
    using Ptr = std::unique_ptr<AssignExpr>;
    
    AssignExpr(ExprPtr target, TokenKind op, ExprPtr value, SourceLocation location);
    
    [[nodiscard]] NodeKind kind() const noexcept override { return NodeKind::AssignExpr; }
    [[nodiscard]] SourceLocation location() const noexcept override { return location_; }
    [[nodiscard]] TokenKind op() const noexcept { return op_; }
    [[nodiscard]] const ExprPtr& target() const noexcept { return target_; }
    [[nodiscard]] const ExprPtr& value() const noexcept { return value_; }
    
private:
    TokenKind op_;       // Assignment operator (=, +=, -=, etc.)
    ExprPtr target_;     // Owned target expression (lvalue)
    ExprPtr value_;      // Owned value expression
    SourceLocation location_;
};
```

**Fields**:
- `op_`: Assignment operator token kind
- `target_`: Target expression (owned, must be lvalue)
- `value_`: Value expression (owned)
- `location_`: Source location of assignment operator

**Relationships**:
- Owns two `ExprPtr` (target and value)

**Source**: spec.md FR-002, research.md Decision 1 (right-associative assignment)

---

### CastExpr

**Purpose**: Represents type cast expressions (e.g., `i32(x)`, `float(value)`).

```cpp
class CastExpr : public Expr {
public:
    using Ptr = std::unique_ptr<CastExpr>;
    
    CastExpr(Type targetType, ExprPtr expression, SourceLocation location);
    
    [[nodiscard]] NodeKind kind() const noexcept override { return NodeKind::CastExpr; }
    [[nodiscard]] SourceLocation location() const noexcept override { return location_; }
    [[nodiscard]] Type targetType() const noexcept { return targetType_; }
    [[nodiscard]] const ExprPtr& expression() const noexcept { return expression_; }
    
private:
    Type targetType_;    // Target type (from type system)
    ExprPtr expression_; // Owned expression to cast
    SourceLocation location_;
};
```

**Fields**:
- `targetType_`: Target type for cast
- `expression_`: Expression to cast (owned)
- `location_`: Source location of cast

**Relationships**:
- Owns one `ExprPtr` (expression)

**Source**: spec.md FR-002

---

### ArrayLiteral

**Purpose**: Represents array literal expressions (e.g., `[1, 2, 3]`).

```cpp
class ArrayLiteral : public Expr {
public:
    using Ptr = std::unique_ptr<ArrayLiteral>;
    
    explicit ArrayLiteral(std::vector<ExprPtr> elements, SourceLocation location);
    
    [[nodiscard]] NodeKind kind() const noexcept override { return NodeKind::ArrayLiteral; }
    [[nodiscard]] SourceLocation location() const noexcept override { return location_; }
    [[nodiscard]] const std::vector<ExprPtr>& elements() const noexcept { return elements_; }
    
private:
    std::vector<ExprPtr> elements_;  // Owned element expressions
    SourceLocation location_;
};
```

**Fields**:
- `elements_`: Vector of element expressions (owned)
- `location_`: Source location of `[` token

**Relationships**:
- Owns 0..n `ExprPtr` (elements)

**Source**: spec.md FR-002

---

### GroupingExpr

**Purpose**: Represents parenthesized grouping expressions (e.g., `(1 + 2) * 3`).

```cpp
class GroupingExpr : public Expr {
public:
    using Ptr = std::unique_ptr<GroupingExpr>;
    
    explicit GroupingExpr(ExprPtr expression, SourceLocation location);
    
    [[nodiscard]] NodeKind kind() const noexcept override { return NodeKind::GroupingExpr; }
    [[nodiscard]] SourceLocation location() const noexcept override { return location_; }
    [[nodiscard]] const ExprPtr& expression() const noexcept { return expression_; }
    
private:
    ExprPtr expression_;  // Owned grouped expression
    SourceLocation location_;
};
```

**Fields**:
- `expression_`: Grouped expression (owned)
- `location_`: Source location of `(` token

**Relationships**:
- Owns one `ExprPtr` (expression)

**Source**: spec.md FR-002

---

## Statement Nodes (11 Types)

### ExprStmt

**Purpose**: Represents expression statements (expression followed by semicolon).

```cpp
class ExprStmt : public Stmt {
public:
    using Ptr = std::unique_ptr<ExprStmt>;
    
    explicit ExprStmt(ExprPtr expression);
    
    [[nodiscard]] NodeKind kind() const noexcept override { return NodeKind::ExprStmt; }
    [[nodiscard]] SourceLocation location() const noexcept override { return expression_->location(); }
    [[nodiscard]] const ExprPtr& expression() const noexcept { return expression_; }
    
private:
    ExprPtr expression_;  // Owned expression
};
```

**Fields**:
- `expression_`: Expression to evaluate (owned)

**Relationships**:
- Owns one `ExprPtr` (expression)

**Source**: spec.md FR-003

---

### VarDecl

**Purpose**: Represents variable declarations (e.g., `var x = 42`, `const PI = 3.14`).

```cpp
class VarDecl : public Stmt {
public:
    using Ptr = std::unique_ptr<VarDecl>;
    
    VarDecl(std::string name, std::optional<Type> type, ExprPtr initializer, bool isConst, SourceLocation location);
    
    [[nodiscard]] NodeKind kind() const noexcept override { return NodeKind::VarDecl; }
    [[nodiscard]] SourceLocation location() const noexcept override { return location_; }
    [[nodiscard]] const std::string& name() const noexcept { return name_; }
    [[nodiscard]] std::optional<Type> type() const noexcept { return type_; }
    [[nodiscard]] const ExprPtr& initializer() const noexcept { return initializer_; }
    [[nodiscard]] bool isConst() const noexcept { return isConst_; }
    
private:
    std::string name_;           // Variable name
    std::optional<Type> type_;   // Optional type annotation
    ExprPtr initializer_;        // Owned initializer expression
    bool isConst_;               // true for 'const', false for 'var'
    SourceLocation location_;
};
```

**Fields**:
- `name_`: Variable name
- `type_`: Optional type annotation
- `initializer_`: Initializer expression (owned)
- `isConst_`: true for const declarations
- `location_`: Source location of declaration

**Validation Rules**:
- If `isConst_` is true, `initializer_` must not be null (spec.md E0201)

**Relationships**:
- Owns one `ExprPtr` (initializer)

**Source**: spec.md FR-003, E0201

---

### FuncDecl

**Purpose**: Represents function declarations (e.g., `fun add(a: i32, b: i32) -> i32 { ... }`).

```cpp
struct Parameter {
    std::string name;
    std::optional<Type> type;
    SourceLocation location;
};

class FuncDecl : public Stmt {
public:
    using Ptr = std::unique_ptr<FuncDecl>;
    
    FuncDecl(std::string name, 
             std::vector<Parameter> params,
             std::optional<Type> returnType,
             BlockStmtPtr body,
             SourceLocation location);
    
    [[nodiscard]] NodeKind kind() const noexcept override { return NodeKind::FuncDecl; }
    [[nodiscard]] SourceLocation location() const noexcept override { return location_; }
    [[nodiscard]] const std::string& name() const noexcept { return name_; }
    [[nodiscard]] const std::vector<Parameter>& params() const noexcept { return params_; }
    [[nodiscard]] std::optional<Type> returnType() const noexcept { return returnType_; }
    [[nodiscard]] const BlockStmtPtr& body() const noexcept { return body_; }
    
private:
    std::string name_;                // Function name
    std::vector<Parameter> params_;   // Parameters (name, type, location)
    std::optional<Type> returnType_;  // Optional return type
    BlockStmtPtr body_;               // Owned function body
    SourceLocation location_;
};
```

**Fields**:
- `name_`: Function name
- `params_`: Vector of parameters (each with name, optional type, location)
- `returnType_`: Optional return type
- `body_`: Function body block (owned)
- `location_`: Source location of function name

**Validation Rules**:
- Parameter names must be unique within parameter list (spec.md E0204)
- Body must not be null (spec.md E0205)

**Relationships**:
- Owns one `BlockStmtPtr` (body)

**Source**: spec.md FR-003, E0204, E0205

---

### ReturnStmt

**Purpose**: Represents return statements (e.g., `return value;`, `return;`).

```cpp
class ReturnStmt : public Stmt {
public:
    using Ptr = std::unique_ptr<ReturnStmt>;
    
    ReturnStmt(ExprPtr value, SourceLocation location);
    
    [[nodiscard]] NodeKind kind() const noexcept override { return NodeKind::ReturnStmt; }
    [[nodiscard]] SourceLocation location() const noexcept override { return location_; }
    [[nodiscard]] const ExprPtr& value() const noexcept { return value_; }
    
private:
    ExprPtr value_;      // Owned return value (null for bare 'return')
    SourceLocation location_;
};
```

**Fields**:
- `value_`: Return value expression (owned, may be null for bare `return`)
- `location_`: Source location of `return` keyword

**Validation Rules**:
- Must appear inside function context (spec.md E0306)

**Relationships**:
- May own one `ExprPtr` (value, optional)

**Source**: spec.md FR-003, E0306

---

### IfStmt

**Purpose**: Represents if statements (e.g., `if (cond) { ... } else { ... }`).

```cpp
class IfStmt : public Stmt {
public:
    using Ptr = std::unique_ptr<IfStmt>;
    
    IfStmt(ExprPtr condition, StmtPtr thenBranch, StmtPtr elseBranch, SourceLocation location);
    
    [[nodiscard]] NodeKind kind() const noexcept override { return NodeKind::IfStmt; }
    [[nodiscard]] SourceLocation location() const noexcept override { return location_; }
    [[nodiscard]] const ExprPtr& condition() const noexcept { return condition_; }
    [[nodiscard]] const StmtPtr& thenBranch() const noexcept { return thenBranch_; }
    [[nodiscard]] const StmtPtr& elseBranch() const noexcept { return elseBranch_; }
    
private:
    ExprPtr condition_;   // Owned condition expression
    StmtPtr thenBranch_;  // Owned then-branch statement
    StmtPtr elseBranch_;  // Owned else-branch statement (may be null)
    SourceLocation location_;
};
```

**Fields**:
- `condition_`: Condition expression (owned)
- `thenBranch_`: Then-branch statement (owned)
- `elseBranch_`: Else-branch statement (owned, may be null for if without else)
- `location_`: Source location of `if` keyword

**Validation Rules**:
- Condition must not be null (spec.md E0301)

**Relationships**:
- Owns three `StmtPtr`/`ExprPtr` (condition, then-branch, else-branch)

**Source**: spec.md FR-003, E0301

---

### WhileStmt

**Purpose**: Represents while loop statements (e.g., `while (cond) { body; }`).

```cpp
class WhileStmt : public Stmt {
public:
    using Ptr = std::unique_ptr<WhileStmt>;
    
    WhileStmt(ExprPtr condition, StmtPtr body, SourceLocation location);
    
    [[nodiscard]] NodeKind kind() const noexcept override { return NodeKind::WhileStmt; }
    [[nodiscard]] SourceLocation location() const noexcept override { return location_; }
    [[nodiscard]] const ExprPtr& condition() const noexcept { return condition_; }
    [[nodiscard]] const StmtPtr& body() const noexcept { return body_; }
    
private:
    ExprPtr condition_;   // Owned condition expression
    StmtPtr body_;        // Owned body statement
    SourceLocation location_;
};
```

**Fields**:
- `condition_`: Condition expression (owned)
- `body_`: Body statement (owned)
- `location_`: Source location of `while` keyword

**Validation Rules**:
- Condition must not be null (spec.md E0302)
- Body establishes loop context for break/continue validation

**Relationships**:
- Owns two `StmtPtr`/`ExprPtr` (condition, body)

**Source**: spec.md FR-003, E0302

---

### ForStmt

**Purpose**: Represents for loop statements (e.g., `for (init; cond; incr) { body; }`).

```cpp
class ForStmt : public Stmt {
public:
    using Ptr = std::unique_ptr<ForStmt>;
    
    ForStmt(StmtPtr initializer,
            ExprPtr condition,
            ExprPtr increment,
            StmtPtr body,
            SourceLocation location);
    
    [[nodiscard]] NodeKind kind() const noexcept override { return NodeKind::ForStmt; }
    [[nodiscard]] SourceLocation location() const noexcept override { return location_; }
    [[nodiscard]] const StmtPtr& initializer() const noexcept { return initializer_; }
    [[nodiscard]] const ExprPtr& condition() const noexcept { return condition_; }
    [[nodiscard]] const ExprPtr& increment() const noexcept { return increment_; }
    [[nodiscard]] const StmtPtr& body() const noexcept { return body_; }
    
private:
    StmtPtr initializer_;   // Owned initializer statement (may be null)
    ExprPtr condition_;     // Owned condition expression (may be null)
    ExprPtr increment_;     // Owned increment expression (may be null)
    StmtPtr body_;          // Owned body statement
    SourceLocation location_;
};
```

**Fields**:
- `initializer_`: Initializer statement (owned, may be null)
- `condition_`: Condition expression (owned, may be null)
- `increment_`: Increment expression (owned, may be null)
- `body_`: Body statement (owned)
- `location_`: Source location of `for` keyword

**Relationships**:
- Owns four `StmtPtr`/`ExprPtr` (initializer, condition, increment, body)

**Source**: spec.md FR-003

---

### BlockStmt

**Purpose**: Represents block statements (e.g., `{ stmt1; stmt2; }`).

```cpp
class BlockStmt : public Stmt {
public:
    using Ptr = std::unique_ptr<BlockStmt>;
    
    explicit BlockStmt(std::vector<StmtPtr> statements, SourceLocation location);
    
    [[nodiscard]] NodeKind kind() const noexcept override { return NodeKind::BlockStmt; }
    [[nodiscard]] SourceLocation location() const noexcept override { return location_; }
    [[nodiscard]] const std::vector<StmtPtr>& statements() const noexcept { return statements_; }
    
private:
    std::vector<StmtPtr> statements_;  // Owned statements in order
    SourceLocation location_;
};
```

**Fields**:
- `statements_`: Vector of statements in execution order (owned)
- `location_`: Source location of `{` token

**Relationships**:
- Owns 0..n `StmtPtr` (statements)
- Establishes block context for variable scoping

**Source**: spec.md FR-003

---

### BreakStmt

**Purpose**: Represents break statements (e.g., `break;`).

```cpp
class BreakStmt : public Stmt {
public:
    using Ptr = std::unique_ptr<BreakStmt>;
    
    explicit BreakStmt(SourceLocation location);
    
    [[nodiscard]] NodeKind kind() const noexcept override { return NodeKind::BreakStmt; }
    [[nodiscard]] SourceLocation location() const noexcept override { return location_; }
    
private:
    SourceLocation location_;
};
```

**Fields**:
- `location_`: Source location of `break` keyword

**Validation Rules**:
- Must appear inside loop context (spec.md E0304)

**Source**: spec.md FR-003, E0304

---

### ContinueStmt

**Purpose**: Represents continue statements (e.g., `continue;`).

```cpp
class ContinueStmt : public Stmt {
public:
    using Ptr = std::unique_ptr<ContinueStmt>;
    
    explicit ContinueStmt(SourceLocation location);
    
    [[nodiscard]] NodeKind kind() const noexcept override { return NodeKind::ContinueStmt; }
    [[nodiscard]] SourceLocation location() const noexcept override { return location_; }
    
private:
    SourceLocation location_;
};
```

**Fields**:
- `location_`: Source location of `continue` keyword

**Validation Rules**:
- Must appear inside loop context (spec.md E0305)

**Source**: spec.md FR-003, E0305

---

### PrintStmt

**Purpose**: Represents print statements (e.g., `print expression;`).

```cpp
class PrintStmt : public Stmt {
public:
    using Ptr = std::unique_ptr<PrintStmt>;
    
    explicit PrintStmt(ExprPtr expression);
    
    [[nodiscard]] NodeKind kind() const noexcept override { return NodeKind::PrintStmt; }
    [[nodiscard]] SourceLocation location() const noexcept override { return expression_->location(); }
    [[nodiscard]] const ExprPtr& expression() const noexcept { return expression_; }
    
private:
    ExprPtr expression_;  // Owned expression to print
};
```

**Fields**:
- `expression_`: Expression to print (owned)

**Validation Rules**:
- Expression must not be null (spec.md E0308)

**Relationships**:
- Owns one `ExprPtr` (expression)

**Source**: spec.md FR-003, E0308

---

## Type System Integration

### Type (Enum/Class)

**Purpose**: Represents resolved type annotations in declarations.

```cpp
enum class Type {
    Void,
    Bool,
    I8, I16, I32, I64,
    U8, U16, U32, U64,
    F32, F64,
    String,
    Null,
    Unknown  // For untyped declarations
};
```

**Usage**:
- Used in `VarDecl::type_` field
- Used in `Parameter::type` field
- Used in `FuncDecl::returnType_` field
- Used in `CastExpr::targetType_` field

**Source**: Derived from spec.md type annotation requirements

---

## Memory Ownership Summary

### Ownership Rules

1. **Parent owns children**: All `ExprPtr` and `StmtPtr` fields represent owned pointers
2. **No sharing**: std::unique_ptr exclusively, no std::shared_ptr
3. **No raw ownership**: Raw pointers used only for non-owning observers (e.g., function return references)
4. **Automatic cleanup**: Parent destruction recursively destroys children
5. **Move semantics**: Nodes are movable, not copyable

### Pointer Type Aliases

```cpp
using ExprPtr = std::unique_ptr<Expr>;
using StmtPtr = std::unique_ptr<Stmt>;

// Node-specific aliases for convenience
using IntegerLiteralPtr = std::unique_ptr<IntegerLiteral>;
using BinaryExprPtr = std::unique_ptr<BinaryExpr>;
using VarDeclPtr = std::unique_ptr<VarDecl>;
// ... etc for all 27 node types
```

---

## Validation Rules Summary

### Expression Validation

- **E0101**: Unbalanced parentheses `(` without `)`
- **E0102**: Unbalanced brackets `[` without `]`
- **E0103**: Unbalanced braces `{` without `}`
- **E0104**: Missing operand for binary operator
- **E0105**: Unexpected token in expression
- **E0106**: Missing function call arguments `(` without `)`
- **E0107**: Missing index expression `[` without `]`
- **E0108**: Missing member name after `.`
- **E0109**: Trailing comma in argument list
- **E0110**: Trailing comma in array literal

### Declaration Validation

- **E0201**: Missing initializer for const declaration
- **E0202**: Invalid variable name
- **E0203**: Missing type annotation
- **E0204**: Duplicate parameter name
- **E0205**: Missing function body
- **E0206**: Invalid return type

### Statement Validation

- **E0301**: Missing condition for if statement
- **E0302**: Missing condition for while statement
- **E0303**: Missing semicolon after statement
- **E0304**: Break statement outside loop
- **E0305**: Continue statement outside loop
- **E0306**: Return statement outside function
- **E0307**: Unreachable code after return
- **E0308**: Missing expression for print statement

### Structural Validation

- **E0401**: Unexpected token at top level
- **E0402**: Unexpected end of file

---

## Relationships Diagram

```
Program
├── declarations_: std::vector<StmtPtr>
│   ├── VarDecl
│   │   ├── name_: std::string
│   │   ├── type_: std::optional<Type>
│   │   └── initializer_: ExprPtr → [any Expr]
│   ├── FuncDecl
│   │   ├── params_: std::vector<Parameter>
│   │   ├── returnType_: std::optional<Type>
│   │   └── body_: BlockStmtPtr → BlockStmt
│   │       └── statements_: std::vector<StmtPtr>
│   ├── IfStmt
│   │   ├── condition_: ExprPtr
│   │   ├── thenBranch_: StmtPtr
│   │   └── elseBranch_: StmtPtr (nullable)
│   └── ... (all 11 statement types)
│
Expressions (used in statements):
├── BinaryExpr
│   ├── left_: ExprPtr → [any Expr]
│   ├── op_: TokenKind
│   └── right_: ExprPtr → [any Expr]
├── CallExpr
│   ├── callee_: ExprPtr
│   └── arguments_: std::vector<ExprPtr>
└── ... (all 16 expression types)
```

---

## Extension Points

### Adding New Expression Types

1. Add `NodeKind` enum value
2. Create new class inheriting from `Expr`
3. Add `using Ptr = std::unique_ptr<NewExpr>` alias
4. Implement constructor, `kind()`, `location()` overrides
5. Add parsing logic in `ExpressionParser`
6. Add tests

### Adding New Statement Types

1. Add `NodeKind` enum value
2. Create new class inheriting from `Stmt`
3. Add `using Ptr = std::unique_ptr<NewStmt>` alias
4. Implement constructor, `kind()`, `location()` overrides
5. Add parsing logic in `StatementParser`
6. Add tests

### Adding New Types to Type System

1. Add `Type` enum value
2. Update type resolution logic in semantic analysis (future phase)
3. Update cast validation
4. Add type compatibility checks
