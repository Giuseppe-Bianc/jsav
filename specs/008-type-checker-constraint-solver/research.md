# Research: Hindley-Milner Type Checker with Constraint Solver

**Feature**: 008-type-checker-constraint-solver
**Date**: 2026-04-02
**Phase**: 0 — Research & Unknown Resolution

## Overview

This document captures research decisions for implementing a Hindley-Milner style constraint-based type checker for the jsav compiler. All decisions are scoped to C++23 standard library features supported by Visual Studio 2026.

---

## Research Topic 1: Union-Find Data Structure Implementation

**Unknown**: How to implement efficient union-find for type variable unification in C++23 without external dependencies?

### Decision

Implement a custom union-find data structure using:

- `std::vector<std::size_t>` for parent array (path compression via `find` operation)
- `std::vector<std::uint8_t>` for rank array (union by rank optimization)
- Type variables map to sequential indices via `std::unordered_map<TypeVarId, std::size_t>`

### Rationale

- Union-find achieves O(α(n)) amortized time per operation (where α is inverse Ackermann function)
- Path compression and union by rank are essential for near-constant-time performance
- Standard library containers provide all necessary functionality
- No external library needed; pure C++23 implementation

### Alternatives Considered

1. **Naive linked representation** — Rejected: O(n) find operations unacceptable for 100K constraints
2. **External library (Boost.DisjointSets)** — Rejected: Constitution Principle V prohibits new dependencies
3. **std::map for parent tracking** — Rejected: O(log n) lookups slower than O(1) vector indexing

---

## Research Topic 2: Type Variable Representation and Uniqueness

**Unknown**: How to guarantee globally unique type variable identifiers across scopes and function instantiations?

### Decision

Use thread-local sequential counter for type variable ID generation:

```cpp
inline std::size_t fresh_type_var_id() noexcept {
    static thread_local std::size_t counter = 0;
    return ++counter;
}
```

Type variable naming: `?T1`, `?T2`, `?T3`, ... (prefix `?T` + sequential ID)

### Rationale

- Thread-local storage (TLS) ensures no race conditions without synchronization overhead
- Sequential IDs guarantee uniqueness within a compilation unit
- Simple implementation with predictable, debuggable variable names
- Spec explicitly requires TLS approach: "Sequential counter with thread-local storage (TLS)"

### Alternatives Considered

1. **Global atomic counter** — Rejected: Unnecessary synchronization; type checker is explicitly single-threaded
2. **UUID-based IDs** — Rejected: Overkill; sequential IDs sufficient for single-threaded model
3. **Scope-qualified naming** — Rejected: Adds complexity without benefit; fresh IDs per call site handle instantiation

---

## Research Topic 3: Constraint Representation

**Unknown**: How to efficiently represent type constraints for unification?

### Decision

Represent constraints as tagged pairs with unique IDs:

```cpp
struct Constraint {
    std::size_t id;           // C1, C2, C3...
    TypePtr lhs;              // Left-hand side type
    TypePtr rhs;              // Right-hand side type
    SourceSpan origin;        // Source location for error reporting
    std::string_view reason;  // Human-readable generation context
};
```

Store constraints in `std::vector<Constraint>` for sequential processing.

### Rationale

- Flat vector allows efficient iteration during solving
- Constraint IDs enable precise error reporting ("Unification failure at constraint C17")
- SourceSpan origin provides accurate error locations
- Reason field aids debugging ("from binary expression +")

### Alternatives Considered

1. **Implicit constraints via direct unification** — Rejected: Loses diagnostic context; errors harder to trace
2. **Constraint graph structure** — Rejected: Unnecessary complexity; sequential constraints sufficient
3. **std::deque for dynamic insertion** — Rejected: No insertion during solving; vector is simpler and faster

---

## Research Topic 4: Error Type Propagation Strategy

**Unknown**: How to continue type checking after encountering an error without cascading false positives?

### Decision

Introduce `ErrorType` as a special type kind that:

1. Is compatible with all types during unification (never fails)
2. Propagates through expressions silently
3. Does not generate new error reports when combined with other ErrorTypes
4. Is tracked via TypeKind::Error enum value

```cpp
// In Type.hpp (add to TypeKind enum)
enum class TypeKind : std::uint8_t {
    // ... existing types ...
    Error,  // Type error placeholder
};
```

### Rationale

- ErrorType acts as "poison" — once introduced, it flows through without generating cascading errors
- Spec explicitly requires: "Insert ErrorType placeholder when type error detected"
- Allows single-pass complete error collection (FR-007)
- Typed AST remains structurally valid even with errors

### Alternatives Considered

1. **Null/empty type pointer** — Rejected: Requires null checks everywhere; error-prone
2. **std::optional<TypePtr>** — Rejected: Changes all type signatures; invasive change
3. **Exception-based error propagation** — Rejected: Violates constitution error handling pattern

---

## Research Topic 5: Symbol Table Scope Management

**Unknown**: How to implement lexical scoping with variable shadowing for the symbol table?

### Decision

Implement stack-based scope management:

```cpp
class SymbolTable {
public:
    void push_scope();
    void pop_scope();
    void define(std::string_view name, TypeScheme scheme);
    std::optional<TypeScheme> lookup(std::string_view name) const;
private:
    std::vector<std::unordered_map<std::string, TypeScheme>> scopes_;
};
```

- `push_scope()` adds new empty scope
- `pop_scope()` removes innermost scope
- `lookup()` searches from innermost to outermost (shadowing behavior)
- `define()` always adds to innermost scope

### Rationale

- Stack-based scopes naturally implement lexical scoping
- Inner-to-outer search implements shadowing: inner declarations hide outer ones
- Spec requires: "Shadowing: Inner scope declarations hide outer scope bindings"
- O(1) define, O(depth) lookup — acceptable for typical scope depths (<10)

### Alternatives Considered

1. **Single flat map with scope prefixes** — Rejected: Complex name mangling; harder to pop scopes
2. **Persistent data structure** — Rejected: Overkill; mutation within scope is fine
3. **std::map for scopes** — Rejected: Slower than unordered_map; string keys work well with hashing

---

## Research Topic 6: Type Scheme Instantiation for Polymorphism

**Unknown**: How to represent and instantiate polymorphic type schemes (∀T. T → T)?

### Decision

Represent type schemes as:

```cpp
struct TypeScheme {
    std::vector<TypeVarId> quantified_vars;  // Bound type variables
    TypePtr body;                             // Type with references to bound vars
};
```

Instantiation process:

1. For each quantified variable, generate a fresh type variable
2. Substitute all occurrences in body with fresh variables
3. Return the substituted type

### Rationale

- Explicit quantification list clearly separates bound from free variables
- Fresh instantiation per call site prevents cross-contamination (SC-005)
- Simple substitution traversal for instantiation
- Aligns with textbook Hindley-Milner implementation

### Alternatives Considered

1. **De Bruijn indices** — Rejected: Harder to debug; variable names lost
2. **Lazy instantiation** — Rejected: Complicates unification; eager is simpler
3. **No type schemes (monomorphic only)** — Rejected: Spec requires parametric polymorphism (FR-010)

---

## Research Topic 7: Occurs Check Implementation

**Unknown**: How to efficiently implement the occurs check to prevent infinite types?

### Decision

Implement occurs check as a recursive type traversal:

```cpp
bool occurs_in(TypeVarId var, const TypePtr& type, const Substitution& subst) {
    // Apply current substitution first
    TypePtr resolved = subst.apply(type);
    
    if (auto* tv = type_as<TypeVariable>(resolved)) {
        return tv->id() == var;
    }
    // Recurse into compound types (arrays, functions, etc.)
    if (auto* arr = type_as<ArrayType>(resolved)) {
        return occurs_in(var, arr->element_type(), subst);
    }
    // ... handle other compound types
    return false;
}
```

Call occurs check before binding a type variable in unification.

### Rationale

- Prevents infinite types like `?T = ?T → ?T`
- Must be performed before each variable binding to catch recursion early
- Spec requires: "Occurs check prevents infinite types" (FR-006)
- Error code E2035 for occurs check failure

### Alternatives Considered

1. **Post-unification cycle detection** — Rejected: Detects too late; may produce invalid substitution
2. **Depth-limited recursion** — Rejected: May miss deep cycles; occurs check is necessary

---

## Research Topic 8: Zonking Strategy

**Unknown**: How to apply substitution to produce fully-typed AST efficiently?

### Decision

Implement zonking as a typed AST traversal that replaces type variables:

```cpp
TypePtr zonk(const TypePtr& type, const Substitution& subst) {
    TypePtr current = type;
    while (auto* tv = type_as<TypeVariable>(current)) {
        if (auto bound = subst.lookup(tv->id())) {
            current = *bound;
        } else {
            // Unbound variable — return as-is (will be reported as error)
            return current;
        }
    }
    // Recursively zonk compound types
    // ... 
    return current;
}
```

Apply to every node's type in a post-order AST traversal.

### Rationale

- Post-order traversal ensures children are zonked before parents
- Loop handles transitive substitutions (?T → ?R → Int becomes Int)
- Unbound variables detected and reported as E2036 errors
- Produces clean typed AST with no remaining type variables (SC-001)

### Alternatives Considered

1. **Eager substitution during unification** — Rejected: More complex; path compression already optimizes find
2. **Lazy zonking on demand** — Rejected: All types needed fully resolved for downstream phases

---

## Research Topic 9: C++23 Features for Type Checker Implementation

**Unknown**: Which C++23 features should be used for optimal implementation?

### Decision

Leverage the following C++23 features (all supported by VS 2026):

| Feature | Usage | Rationale |
|---------|-------|-----------|
| `std::expected<T, E>` | Return type for functions that can fail | Structured error handling per constitution |
| `std::format` | Error message formatting | Type-safe, modern formatting |
| `std::optional` | Optional type annotations, lookup results | Explicit nullability |
| `std::string_view` | Immutable string parameters | Avoid allocations |
| `std::unreachable()` | Exhaustive switch handling | UB if reached; documents completeness |
| `[[nodiscard]]` | All functions returning values | Prevent ignored returns |
| `constexpr` | Type predicates, simple functions | Compile-time evaluation |
| `thread_local` | Type variable counter | TLS for unique IDs |

### Rationale

- Constitution requires C++23 features verified as MSVC-supported
- These features are well-established in Visual Studio 2022/2026
- Provides clean, modern API surface

### Alternatives Considered

1. **Exception-based errors** — Rejected: Constitution mandates `std::expected`
2. **C-style strings** — Rejected: Constitution mandates `std::string_view`
3. **Output parameters for errors** — Rejected: `std::expected` is more expressive

---

## Research Topic 10: Test Strategy for Type Checker

**Unknown**: How to structure tests for comprehensive type checker validation?

### Decision

Hybrid testing approach in `test/tests.cpp`:

1. **Unit tests** for individual constraint rules:
   - Literal typing (integer → i32, bool → bool, etc.)
   - Binary operation typing (i32 + i32 → i32, comparison → bool)
   - Unary operation typing
   - Assignment type matching

2. **Integration tests** for end-to-end scenarios:
   - Well-typed programs → fully typed AST, empty error vector
   - Ill-typed programs → partial AST, correct error codes

3. **Error reporting tests**:
   - Verify error codes (E2033-E2036)
   - Verify source span accuracy
   - Verify "did you mean" suggestions

Test naming convention: `TypeChecker_[Scenario]_[ExpectedResult]`

### Rationale

- Spec mandates: "Hybrid approach — example-based unit tests + integration tests"
- Tests in `test/tests.cpp` per user requirement
- TDD Red-Green-Refactor cycle per constitution

### Alternatives Considered

1. **Golden file testing only** — Rejected: Spec requires unit tests for constraint rules
2. **Property-based testing** — Rejected: Not specified; complexity not warranted for initial version
3. **Separate test file** — Rejected: User explicitly requested tests in `test/tests.cpp`

---

## Summary: All Unknowns Resolved

| Unknown | Resolution |
|---------|------------|
| Union-find implementation | Custom vector-based with path compression + union by rank |
| Type variable uniqueness | Thread-local sequential counter |
| Constraint representation | Struct with ID, LHS/RHS types, origin, reason |
| Error propagation | ErrorType kind that silently propagates |
| Symbol table scoping | Stack-based scope with inner-to-outer lookup |
| Type scheme instantiation | Explicit quantified vars + fresh variable substitution |
| Occurs check | Recursive traversal before each binding |
| Zonking strategy | Post-order AST traversal with transitive substitution |
| C++23 features | std::expected, std::format, std::optional, std::string_view |
| Test strategy | Unit tests + integration tests in test/tests.cpp |

**Phase 0 Status**: ✅ Complete — All NEEDS CLARIFICATION resolved.
