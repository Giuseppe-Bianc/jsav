# Data Model: Hindley-Milner Type Checker with Constraint Solver

**Feature**: 008-type-checker-constraint-solver
**Date**: 2026-04-02
**Phase**: 1 — Design

## Overview

This document defines the entities, relationships, and data structures for the type checker implementation. All types use C++23 standard library exclusively.

---

## Entity Definitions

### 1. TypeVariable

Represents an unknown type during constraint-based inference.

```cpp
namespace jsv {

/// Unique identifier for type variables
using TypeVarId = std::size_t;

/// Type variable representation (e.g., ?T1, ?T2)
class TypeVariable final : public TypeBase {
public:
    explicit TypeVariable(TypeVarId id) 
        : TypeBase{TypeKind::TypeVar}, id_{id} {}

    [[nodiscard]] constexpr TypeVarId id() const noexcept { return id_; }
    
    /// Format as string (e.g., "?T42")
    [[nodiscard]] std::string to_string() const;
    
    [[nodiscard]] static constexpr bool classof(const TypeBase* t) noexcept {
        return t && t->kind() == TypeKind::TypeVar;
    }

private:
    TypeVarId id_;
};

/// Generate a fresh type variable with unique ID (thread-local counter)
[[nodiscard]] TypePtr fresh_type_variable() noexcept;

}  // namespace jsv
```

**Spec Reference**: FR-003 — "System MUST generate fresh type variables for expressions without explicit type annotations"

---

### 2. ErrorType

Placeholder type representing a type error for error recovery.

```cpp
namespace jsv {

/// Error type for error recovery during type checking
class ErrorType final : public TypeBase {
public:
    ErrorType() : TypeBase{TypeKind::Error} {}
    
    [[nodiscard]] static constexpr bool classof(const TypeBase* t) noexcept {
        return t && t->kind() == TypeKind::Error;
    }
};

/// Singleton error type instance
[[nodiscard]] TypePtr error_type() noexcept;

}  // namespace jsv
```

**Spec Reference**: FR-023 — "System MUST insert an ErrorType placeholder when a type error is detected"

---

### 3. Constraint

Represents a type equation generated during type checking.

```cpp
namespace jsv {

/// Unique identifier for constraints (C1, C2, C3, ...)
using ConstraintId = std::size_t;

/// Type constraint: lhs = rhs
struct Constraint {
    ConstraintId id;              ///< Unique constraint ID
    TypePtr lhs;                  ///< Left-hand side type
    TypePtr rhs;                  ///< Right-hand side type
    SourceSpan origin;            ///< Source location for error reporting
    std::string_view reason;      ///< Generation context (e.g., "binary +")
};

/// Constraint set for accumulating constraints during generation
class ConstraintSet {
public:
    /// Add a new constraint and return its ID
    [[nodiscard]] ConstraintId add(TypePtr lhs, TypePtr rhs, 
                                    SourceSpan origin,
                                    std::string_view reason);
    
    /// Get all constraints
    [[nodiscard]] const std::vector<Constraint>& constraints() const noexcept;
    
    /// Get constraint by ID
    [[nodiscard]] const Constraint* get(ConstraintId id) const noexcept;
    
    /// Number of constraints
    [[nodiscard]] std::size_t size() const noexcept;

private:
    std::vector<Constraint> constraints_;
    ConstraintId next_id_{1};
};

}  // namespace jsv
```

**Spec Reference**: FR-004 — "System MUST generate type constraints as equations between types"

---

### 4. Substitution

Mapping from type variables to concrete types produced by the solver.

```cpp
namespace jsv {

/// Substitution: maps type variables to their resolved types
class Substitution {
public:
    /// Bind a type variable to a type
    void bind(TypeVarId var, TypePtr type);
    
    /// Lookup the binding for a type variable
    [[nodiscard]] std::optional<TypePtr> lookup(TypeVarId var) const noexcept;
    
    /// Apply substitution to a type (resolve all type variables)
    [[nodiscard]] TypePtr apply(const TypePtr& type) const;
    
    /// Check if a variable is bound
    [[nodiscard]] bool contains(TypeVarId var) const noexcept;
    
    /// Number of bindings
    [[nodiscard]] std::size_t size() const noexcept;

private:
    std::unordered_map<TypeVarId, TypePtr> bindings_;
};

}  // namespace jsv
```

**Spec Reference**: FR-009 — "System MUST apply the computed substitution to all type variables"

---

### 5. TypeScheme

Quantified type representation for polymorphic functions.

```cpp
namespace jsv {

/// Type scheme: ∀(vars). body
/// Example: ∀T. T → T for identity function
struct TypeScheme {
    std::vector<TypeVarId> quantified_vars;  ///< Bound type variables
    TypePtr body;                             ///< Type body with references to vars
    
    /// Instantiate with fresh type variables
    [[nodiscard]] TypePtr instantiate() const;
    
    /// Create monomorphic scheme (no quantified variables)
    [[nodiscard]] static TypeScheme mono(TypePtr type);
};

}  // namespace jsv
```

**Spec Reference**: FR-010 — "System MUST support parametric polymorphism by allowing function definitions to have type schemes"

---

### 6. SymbolTable

Scoped environment mapping identifiers to their type schemes.

```cpp
namespace jsv {

/// Symbol table with lexical scoping
class SymbolTable {
public:
    /// Enter a new scope
    void push_scope();
    
    /// Exit the current scope
    void pop_scope();
    
    /// Define a symbol in the current scope
    void define(std::string_view name, TypeScheme scheme);
    
    /// Lookup a symbol (searches from innermost to outermost scope)
    [[nodiscard]] std::optional<TypeScheme> lookup(std::string_view name) const;
    
    /// Check if a symbol exists in current scope only
    [[nodiscard]] bool defined_in_current_scope(std::string_view name) const;
    
    /// Current scope depth
    [[nodiscard]] std::size_t depth() const noexcept;

private:
    std::vector<std::unordered_map<std::string, TypeScheme>> scopes_;
};

}  // namespace jsv
```

**Spec Reference**: FR-020 — "System MUST perform name resolution as a pre-type-checking phase"

---

### 7. UnionFind

Efficient disjoint-set data structure for constraint solving.

```cpp
namespace jsv {

/// Union-Find data structure for type variable unification
class UnionFind {
public:
    /// Create a new set containing only this element
    void make_set(TypeVarId var);
    
    /// Find the representative of the set containing var (with path compression)
    [[nodiscard]] TypeVarId find(TypeVarId var);
    
    /// Union the sets containing x and y (with union by rank)
    void unite(TypeVarId x, TypeVarId y);
    
    /// Check if two variables are in the same set
    [[nodiscard]] bool same_set(TypeVarId x, TypeVarId y);
    
    /// Number of elements tracked
    [[nodiscard]] std::size_t size() const noexcept;

private:
    std::unordered_map<TypeVarId, TypeVarId> parent_;
    std::unordered_map<TypeVarId, std::uint8_t> rank_;
};

}  // namespace jsv
```

**Spec Reference**: FR-005 — "System MUST solve constraints using a union-find based unification algorithm"

---

### 8. ConstraintSolver

Solves type constraints using unification.

```cpp
namespace jsv {

/// Result of constraint solving
struct SolverResult {
    Substitution substitution;           ///< Variable → Type mappings
    std::vector<CompileError> errors;    ///< Unification errors
};

/// Constraint solver using union-find unification
class ConstraintSolver {
public:
    /// Solve all constraints and produce substitution
    [[nodiscard]] SolverResult solve(const ConstraintSet& constraints);

private:
    /// Unify two types, producing substitution entries
    [[nodiscard]] std::expected<void, CompileError> 
    unify(const TypePtr& t1, const TypePtr& t2, const Constraint& constraint);
    
    /// Occurs check: does var occur in type?
    [[nodiscard]] bool occurs_in(TypeVarId var, const TypePtr& type);
    
    UnionFind union_find_;
    Substitution substitution_;
};

}  // namespace jsv
```

**Spec Reference**: FR-006 — "System MUST perform occurs check during unification to prevent infinite types"

---

### 9. TypeChecker

Main type checking interface.

```cpp
namespace jsv {

/// Type checking result
struct TypeCheckResult {
    std::unique_ptr<TypedProgram> typed_ast;  ///< Fully typed AST
    std::vector<CompileError> errors;          ///< All type errors collected
};

/// Main type checker class
class TypeChecker {
public:
    /// Type check a program, producing typed AST and errors
    [[nodiscard]] TypeCheckResult check(const Program& program);

private:
    /// Phase 1: Name resolution (populate symbol table)
    void resolve_names(const Program& program);
    
    /// Phase 2: Constraint generation (traverse AST, emit constraints)
    void generate_constraints(const Program& program);
    
    /// Phase 3: Constraint solving (unification)
    SolverResult solve_constraints();
    
    /// Phase 4: Zonking (apply substitution to typed AST)
    std::unique_ptr<TypedProgram> zonk(const Substitution& subst);
    
    /// Type a single expression (recursive)
    [[nodiscard]] TypedExprPtr type_expr(const Expr& expr);
    
    /// Type a single statement (recursive)
    [[nodiscard]] TypedStmtPtr type_stmt(const Stmt& stmt);
    
    SymbolTable symbols_;
    ConstraintSet constraints_;
    std::vector<CompileError> errors_;
};

}  // namespace jsv
```

**Spec Reference**: FR-001 — "System MUST accept a Raw AST as input and produce a `std::pair<TypedProgram, std::vector<CompileError>>`"

---

## Entity Relationships

```text
                    ┌─────────────┐
                    │   Program   │ (Raw AST)
                    └──────┬──────┘
                           │ input
                           ▼
                    ┌─────────────┐
                    │ TypeChecker │
                    └──────┬──────┘
                           │
         ┌─────────────────┼─────────────────┐
         │                 │                 │
         ▼                 ▼                 ▼
  ┌─────────────┐   ┌─────────────┐   ┌─────────────┐
  │ SymbolTable │   │ConstraintSet│   │   Errors    │
  │  (scopes)   │   │ (equations) │   │ (collected) │
  └─────────────┘   └──────┬──────┘   └─────────────┘
                           │
                           ▼
                  ┌─────────────────┐
                  │ConstraintSolver │
                  │   (UnionFind)   │
                  └────────┬────────┘
                           │
                           ▼
                    ┌─────────────┐
                    │ Substitution │
                    │ [?T → Type] │
                    └──────┬──────┘
                           │ apply (zonk)
                           ▼
                    ┌─────────────┐
                    │TypedProgram │ (Typed AST)
                    └─────────────┘
```

---

## State Transitions

### Type Variable Lifecycle

```text
┌─────────────┐     constraint      ┌─────────────┐
│   Created   │ ──────────────────► │  In System  │
│ (fresh ID)  │    generation       │ (equations) │
└─────────────┘                     └──────┬──────┘
                                           │
                                           │ unification
                                           ▼
                              ┌────────────────────────┐
                              │       Bound            │
                              │ (substitution entry)   │
                              └───────────┬────────────┘
                                          │
                                          │ zonking
                                          ▼
                              ┌────────────────────────┐
                              │      Resolved          │
                              │  (concrete type in     │
                              │    Typed AST node)     │
                              └────────────────────────┘
```

### Constraint Solving States

```text
   Constraint C                  After Unify(C)
   ─────────────                 ────────────────
   ?T = i32                      Substitution: ?T → i32
   ?T = ?R                       UnionFind: ?T ∪ ?R (same set)
   i32 = string                  Error: E2034 (type mismatch)
   ?T = ?T → ?R                  Error: E2035 (occurs check)
```

---

## Validation Rules

### Binary Expression Type Rules

| Operation | Operand Constraint | Result Type |
|-----------|-------------------|-------------|
| `+`, `-`, `*`, `/`, `%` | Both operands must have identical numeric type | Same as operands |
| `==`, `!=`, `<`, `>`, `<=`, `>=` | Both operands must have identical type | `bool` |
| `&&`, `\|\|` | Both operands must be `bool` | `bool` |
| `&`, `\|`, `^`, `<<`, `>>` | Both operands must be integer | Same as operands |

### Unary Expression Type Rules

| Operation | Operand Constraint | Result Type |
|-----------|-------------------|-------------|
| `-` (negate) | Operand must be numeric | Same as operand |
| `!` (logical not) | Operand must be `bool` | `bool` |
| `~` (bitwise not) | Operand must be integer | Same as operand |
| `++`, `--` | Operand must be numeric | Same as operand |

### Control Flow Type Rules

| Construct | Constraint |
|-----------|------------|
| `if` condition | Must be `bool` |
| `if` expression branches | Must have identical types |
| `while` condition | Must be `bool` |
| `for` condition | Must be `bool` |
| `return` expression | Must match function return type |

---

## Error Codes

| Code | Category | Description |
|------|----------|-------------|
| E2033 | Constraint Generation | Failed to generate constraint for expression |
| E2034 | Unification Failure | Type mismatch during unification |
| E2035 | Occurs Check | Recursive type detected (infinite type) |
| E2036 | Unresolved Variable | Type variable has no binding after solving |

---

## TypeKind Extension

Add to existing `TypeKind` enum in `Type.hpp`:

```cpp
enum class TypeKind : std::uint8_t {
    // ... existing types ...
    
    // Type inference types
    TypeVar,  // Type variable (?T)
    Error,    // Error type placeholder
};
```

---

## Phase 1 Status

✅ **Complete** — All entities extracted from spec and defined with C++23 interfaces.
