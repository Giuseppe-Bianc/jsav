# Feature Specification: Hindley-Milner Type Checker with Constraint Solver

**Feature Branch**: `008-type-checker-constraint-solver`
**Created**: giovedì 2 aprile 2026
**Status**: Draft
**Input**: User description: "Build a type checker for the jsav compiler that implements a Typed AST + constraint solver architecture. The type checker accepts a Raw AST and produces a fully typed AST along with a collection of compile errors. The system uses Hindley-Milner style constraint-based type inference with union-find for efficient constraint solving.

Core Objectives

The type checker must accomplish the following:

1. Transform Raw AST to Typed AST: Every AST node receives resolved type information through a systematic typing process
2. Generate and Solve Type Constraints: Infer types through constraint generation and unification-based solving
3. Report Type Errors: Collect all type errors (not fail-fast) to provide comprehensive diagnostic feedback
4. Support Parametric Polymorphism: Enable generic functions through type variables and constraint solving
5. Maintain Type Safety: Ensure all operations are type-correct according to the language's type system

Key Entities and Relationships

Primary Entities:

1. Raw AST - Input structure with untyped expressions and statements
2. Typed AST - Output structure where every node carries resolved type information (TypePtr)
3. Type Variables - Placeholder types (?T, ?R, etc.) representing unknown types during inference
4. Constraints - Equations between types (?T = Int→?R, ?R = Bool) generated during type checking
5. Substitution - Mapping from type variables to concrete types produced by the solver (S = [?T↦Int→Bool, ?R↦Bool])
6. Symbol Table - Environment mapping identifiers to their type schemes during name resolution
7. CompileError - Type errors with source locations, error codes, and helpful messages

Entity Relationships:

- Raw AST → (Name Resolution) → Resolved AST with Symbol Table
- Resolved AST → (Constraint Generation) → Typed AST (partially typed) + Constraint Set
- Constraint Set → (Union-Find Solver) → Substitution
- Typed AST + Substitution → (Zonking) → Fully Typed AST
- CompileError collection occurs at all phases

User-Facing Functionality

Type Checker Interface:

Users (compiler developers) will:

1. Invoke the type checker on a parsed AST and receive a std::pair<TypedAST, std::vector<CompileError>>
2. Inspect typed nodes to access resolved type information via node_type() method on every TypedNode
3. Review comprehensive error reports with source locations, error codes, and helpful suggestions for all type violations
4. Query type information for debugging and analysis through the typed AST structure

Error Reporting:

Users will see:

- All type errors collected in a single pass (not just the first error)
- Source spans pinpointing exact error locations
- Error codes from the existing E2xxx semantic analysis range (E2001-E2999), with new constraint solver codes E2033-E2036
- Helpful messages suggesting fixes (e.g., "did you mean to cast?")
- Type mismatch details showing expected vs. actual types

Type System Rules

Primitive Types:

- Signed integers: i8, i16, i32, i64
- Unsigned integers: u8, u16, u32, u64
- Floating-point: f32, f64
- Other primitives: char, string, bool, void, nullptr

Compound Types:

- Arrays: [T; N] where T is element type and N is compile-time size
- Vectors: Vec<T> for dynamic arrays

Type Constraints:

1. Binary Operations: Both operands must have identical types (no implicit promotions); result type depends on operation

- Arithmetic (+, -, *, /): Operands must have identical types; result has that same type
- Comparison (==, !=, <, >, <=, >=): Operands must have identical types; result is bool
- Logical (&&, ||): Operands must be bool; result is bool

2. Unary Operations: Operand type must match operation requirements

- Negation (-): Operand must be numeric; result has same type
- Logical not (!): Operand must be bool; result is bool

3. Assignment: Left-hand side must be mutable lvalue; right-hand side type must match or be convertible

4. Function Calls: Argument types must match parameter types (or be inferable); return type is function's return type

5. Control Flow:

- if condition must be bool
- Both branches of if expression must have compatible types (join type)
- return expression type must match function's declared return type

6. Arrays: All elements must have the same type; size must be compile-time constant

Constraint Generation and Solving Workflow

Phase 1: Name Resolution (Pre-Type Checking)

- Users can resolve identifiers to declarations using the symbol table
- Unresolved identifiers produce CompileError before type checking begins

Phase 2: Constraint Generation

- Users can traverse the AST and generate type constraints for each node
- Each expression produces a fresh type variable for its result type
- Constraints are equations: type_of_lhs = type_of_rhs or argument_type = parameter_type
- Example: For f(x), generate ?F = (?X → ?R) and type(x) = ?X

Phase 3: Constraint Solving (Unification)

- Users can solve constraints using union-find algorithm
- Occurs check prevents infinite types (?T = ?T → ?T fails)
- Produces substitution mapping type variables to concrete types
- Unification failures produce CompileError

Phase 4: Zonking (Apply Substitution)

- Users can apply the substitution to all type variables in the Typed AST
- Every type variable is replaced with its concrete type from the substitution
- Result is a fully resolved Typed AST with no remaining type variables

Expected Behaviors

Successful Type Checking:

- Input: Well-typed Raw AST
- Output: Typed AST where every node has a concrete (non-variable) type
- Error vector is empty
- All type variables have been resolved through unification

Type Error Scenarios:

- Input: AST with type violations (e.g., 3 + "hello")
- Output: Typed AST (possibly partial) with error nodes
- Error vector contains TypeError entries with:
    - Error code (e.g., E001: type mismatch)
    - Source span showing error location
    - Message: "cannot add i32 and string"
    - Optional help: "consider converting one operand to match the other type"

Polymorphic Functions:

- Input: Generic function like fn id<T>(x: T) -> T { x }
- Output: Function type scheme with quantified type variables
- Each call site instantiates fresh type variables
- Constraints solved independently per call site

Scope Boundaries

In Scope:

- Constraint-based type inference for all AST node types in NodeKind.hpp
- Union-find data structure for efficient constraint solving
- Type variable generation and management
- Substitution application (zonking) across the entire AST
- Comprehensive error collection with source locations
- Integration with existing TypedAst.hpp infrastructure
- Support for primitive types, arrays, and vectors as defined in Type.hpp

Out of Scope (This Version):

- Type annotations in source code (parser must handle these)
- Advanced type system features (subtyping, type classes, traits)
- Type-directed name resolution (handled in separate name resolution phase)
- Incremental type checking (full re-check on each compilation)
- Parallel type checking (single-threaded implementation)

Future Considerations:

- Bidirectional type checking for better error messages
- Type holes for partial type annotations
- Constraint simplification before solving for performance
- Type error prioritization and suppression for better UX

Data Flow Architecture

Source Code
│
├─[Lexer]────────────────► Token Stream
│
├─[Parser]───────────────► Raw AST (untyped)
│
├─[Name Resolution]──────► Resolved AST + Symbol Table
│  (identifiers bound to declarations)
│
├─[Constraint Generation]► Typed AST (with type variables)
│  Γ ⊢ e : ?T
│  Constraint Set C = {?T = Int→?R, ?R = Bool}
│
├─[Constraint Solver]────► Substitution S
│ (Union-Find)  S = [?T↦Int→Bool, ?R↦Bool]
│  unify(C), occurs check
│
├─[Zonking]──────────────► Typed AST (fully resolved)
│  apply S to all type variables
│  every node: Ty = concrete type
│
└─[Error Collection]─────► std::vector<CompileError>
(accumulated at all phases)

Output Contract

The type checker provides the following interface:

 1 std::pair<TypedProgram, std::vector<CompileError>> typeCheck(const Program &rawAST);

Where:

- TypedProgram contains the fully resolved Typed AST with all type variables substituted
- std::vector<CompileError> contains all type errors encountered (empty if successful)
- Errors are sorted by source location for deterministic reporting
- Typed AST nodes are accessible via TypedVisitor.hpp pattern matching
"

## User Scenarios & Testing

### User Story 1 - Type Check Well-Typed Programs (Priority: P1)

As a compiler developer, I want to invoke the type checker on a well-typed Raw AST and receive a fully typed AST with all type information resolved, so that I can proceed to subsequent compilation phases (optimization, code generation) with complete type safety guarantees.

**Why this priority**: This is the core functionality of the type checker. Without the ability to successfully type-check valid programs, the entire type system is non-functional. This represents the minimum viable product for the type checker.

**Independent Test**: Can be fully tested by providing a well-typed Raw AST (e.g., `fn add(x: i32, y: i32) -> i32 { x + y }`) and verifying that the output Typed AST has concrete types on every node and the error vector is empty.

**Acceptance Scenarios**:

1. **Given** a Raw AST with explicit type annotations and valid type-correct expressions, **When** the type checker is invoked, **Then** the output Typed AST has concrete (non-variable) types on every node.
2. **Given** a Raw AST with function calls matching declared signatures, **When** the type checker processes it, **Then** all function call sites have resolved argument and return types.
3. **Given** a Raw AST with nested expressions (e.g., `(a + b) * (c - d)`), **When** the type checker processes it, **Then** every intermediate expression node carries its computed type.
4. **Given** a well-typed program with arrays and vectors, **When** the type checker processes it, **Then** all compound types are fully resolved with correct element types and sizes.

---

### User Story 2 - Comprehensive Type Error Reporting (Priority: P2)

As a compiler developer, I want the type checker to collect ALL type errors in a single pass (not fail-fast) with source locations, error codes, and helpful messages, so that users can fix multiple type violations in one edit-compile cycle.

**Why this priority**: Error reporting quality directly impacts developer productivity. A type checker that stops at the first error forces multiple compilation cycles, slowing development. However, this builds on top of the core type-checking functionality, making it P2.

**Independent Test**: Can be fully tested by providing a Raw AST with multiple type violations (e.g., `3 + "hello"`, `true && 5`, `return 42` in a `void` function) and verifying that the error vector contains all violations with proper source spans and error codes.

**Acceptance Scenarios**:

1. **Given** a Raw AST with multiple type mismatches, **When** the type checker processes it, **Then** all type errors are collected and returned (not just the first one).
2. **Given** a Raw AST with a type error, **When** the type checker reports it, **Then** the error includes a source span pinpointing the exact error location.
3. **Given** a Raw AST with incompatible operand types (e.g., `i32 + string`), **When** the type checker reports it, **Then** the error message shows both the expected and actual types.
4. **Given** a Raw AST with an error that has a common fix (e.g., missing cast), **When** the type checker reports it, **Then** the error includes a helpful suggestion ("did you mean to cast?").

---

### User Story 3 - Parametric Polymorphism Support (Priority: P3)

As a compiler developer, I want the type checker to support generic functions with type variables and constraint solving, so that users can write polymorphic functions like `fn id<T>(x: T) -> T { x }` that work with multiple concrete types.

**Why this priority**: Parametric polymorphism is a powerful feature for code reuse, but it requires sophisticated constraint generation and solving. It builds on top of basic type checking and error reporting, making it P3. The core type checker can function without generics.

**Independent Test**: Can be fully tested by defining a generic function (e.g., `fn id<T>(x: T) -> T { x }`) and calling it with different concrete types (e.g., `id(42)`, `id("hello")`), then verifying that each call site has correctly instantiated type variables.

**Acceptance Scenarios**:

1. **Given** a generic function definition with type parameters, **When** the type checker processes it, **Then** the function receives a type scheme with quantified type variables.
2. **Given** a generic function called with different concrete types, **When** the type checker processes each call site, **Then** fresh type variables are instantiated and solved independently per call.
3. **Given** a generic function with constraint violations (e.g., `id(42)` assigned to `string`), **When** the type checker processes it, **Then** a type error is reported at the call site.

---

### Edge Cases

- **What happens when type variables cannot be resolved?** The type checker reports an "unresolved type variable" error with the source location where the ambiguity originates.
- **How does the system handle recursive types?** The occurs check in unification detects infinite types (e.g., `?T = ?T → ?T`) and reports a "recursive type" error.
- **What happens with undeclared identifiers?** Name resolution (pre-type-checking phase) reports "undefined variable" errors before constraint generation begins.
- **How does the system handle type mismatches in array literals?** All elements must have the same type; if they differ, the type checker reports "array element type mismatch" showing the expected vs. actual element types.
- **What happens when function arguments don't match parameter count?** The type checker reports "argument count mismatch" showing expected vs. provided argument count.
- **How does the system handle implicit type conversions?** The type checker does NOT perform implicit conversions; all type mismatches are reported as errors. Explicit casts are required.
- **How does the system handle mixed-type arithmetic operations?** The type checker enforces strict type matching: operands must have identical types. No implicit promotions are performed (e.g., `i32 + i64` is an error requiring explicit cast).

## Requirements

### Functional Requirements

- **FR-001**: System MUST accept a Raw AST (untyped) as input and produce a `std::pair<TypedProgram, std::vector<CompileError>>` as output.
- **FR-002**: System MUST assign a resolved type (TypePtr) to every AST node in the output Typed AST through systematic constraint generation and solving.
- **FR-003**: System MUST generate fresh type variables for expressions without explicit type annotations during constraint generation.
- **FR-004**: System MUST generate type constraints as equations between types (e.g., `?T = Int → ?R`, `?R = Bool`) for all expressions and statements.
- **FR-005**: System MUST solve constraints using a union-find based unification algorithm that produces a substitution mapping type variables to concrete types.
- **FR-006**: System MUST perform occurs check during unification to prevent infinite types and report recursive type errors when detected.
- **FR-007**: System MUST collect ALL type errors encountered during type checking (not fail-fast) to provide comprehensive diagnostic feedback.
- **FR-008**: System MUST report type errors with source spans, error codes, expected vs. actual types, and helpful fix suggestions.
- **FR-009**: System MUST apply the computed substitution to all type variables in the Typed AST (zonking) to produce a fully resolved AST with no remaining type variables.
- **FR-010**: System MUST support parametric polymorphism by allowing function definitions to have type schemes with quantified type variables.
- **FR-011**: System MUST instantiate fresh type variables for each call site of a generic function and solve constraints independently per call.
- **FR-012**: System MUST enforce binary operation type rules: arithmetic operands must have identical types (no implicit promotions), result type matches operand type; comparison operands must match, result is bool; logical operands must be bool, result is bool.
- **FR-013**: System MUST enforce unary operation type rules: negation requires numeric operand, logical not requires bool operand.
- **FR-014**: System MUST enforce assignment type rules: left-hand side must be mutable lvalue, right-hand side type must match or be convertible.
- **FR-015**: System MUST enforce control flow type rules: if conditions must be bool, both branches of if expressions must have compatible types (join type), return expressions must match function's declared return type.
- **FR-016**: System MUST enforce array type rules: all elements must have the same type, size must be compile-time constant.
- **FR-017**: System MUST support primitive types (i8, i16, i32, i64, u8, u16, u32, u64, f32, f64, char, string, bool, void, nullptr) as defined in Type.hpp.
- **FR-018**: System MUST support compound types (arrays `[T; N]`, vectors `Vec<T>`) as defined in Type.hpp.
- **FR-019**: System MUST integrate with existing TypedAst.hpp infrastructure (TypedNode, TypedExpr, TypedStmt, TypedProgram).
- **FR-020**: System MUST perform name resolution as a pre-type-checking phase, resolving identifiers to declarations using a symbol table.
- **FR-021**: System MUST provide debug logging via spdlog for constraint generation, unification steps, and substitution application to aid troubleshooting of type inference failures.

### Key Entities

- **Raw AST**: Input structure containing untyped expressions and statements produced by the parser. Every node lacks type information (node_type() returns nullptr).
- **Typed AST**: Output structure where every node carries resolved type information (TypePtr). Produced by applying the substitution from constraint solving to the initial typed AST with type variables.
- **Type Variables**: Placeholder types (`?T`, `?R`, etc.) representing unknown types during inference. Created fresh for each expression without explicit type annotation.
- **Constraints**: Equations between types generated during type checking (e.g., `?T = Int → ?R`, `argument_type = parameter_type`). Represent the type relationships that must be satisfied.
- **Substitution**: Mapping from type variables to concrete types produced by the unification solver (e.g., `S = [?T ↦ Int → Bool, ?R ↦ Bool]`). Applied to eliminate type variables.
- **Symbol Table**: Environment mapping identifiers to their type schemes during name resolution. Maintains scope hierarchy for variable and function lookups.
- **CompileError**: Type errors with source locations (SourceSpan), error codes (e.g., E001: type mismatch), and helpful messages suggesting fixes.
- **Type Scheme**: Quantified type representation for generic functions (e.g., `∀T. T → T`). Instantiated with fresh type variables at each call site.
- **Union-Find Data Structure**: Efficient disjoint-set data structure for tracking type variable equivalences during constraint solving. Supports near-constant-time union and find operations with path compression and union by rank.

## Success Criteria

### Measurable Outcomes

- **SC-001**: Type checker successfully processes well-typed programs with 100% type resolution (all nodes have concrete types, zero unresolved type variables).
- **SC-002**: Type checker detects and reports 100% of type errors in ill-typed programs (no false negatives) when validated against a comprehensive test suite of known type violations.
- **SC-003**: Error messages enable users to fix type errors in one edit cycle: 90% of type errors include specific fix suggestions that, when applied, resolve the error.
- **SC-004**: Type checker handles programs with 10,000+ AST nodes without performance degradation (completes type checking in under 5 seconds on standard development hardware).
- **SC-005**: Constraint solver correctly handles polymorphic functions: generic functions called with 10+ different concrete types all resolve correctly without cross-contamination.
- **SC-006**: Error reports are actionable: 100% of type errors include source spans pinpointing the exact error location and show expected vs. actual types.
- **SC-007**: Type checker collects all errors in a single pass: programs with 5+ independent type errors report all errors, not just the first one.
- **SC-008**: Occurs check correctly prevents infinite types: all recursive type definitions (e.g., `?T = ?T → ?T`) are detected and reported as errors.

## Assumptions

- **Name resolution is a separate pre-phase**: The type checker assumes identifiers have been resolved to declarations in a prior name resolution phase. Unresolved identifiers produce errors before constraint generation.
- **Type annotations are provided by the parser**: Function parameter types, return types, and variable declaration types are present in the Raw AST from parsing. Type inference fills in missing types for intermediate expressions.
- **No implicit type conversions**: The type system does not perform implicit coercions (e.g., int to float). All type conversions must be explicit via cast expressions.
- **Single-threaded type checking**: The implementation is single-threaded. Parallel type checking is out of scope for this version.
- **Full re-check on each compilation**: The type checker does not support incremental type checking. The entire program is re-checked on each compilation.
- **Standard development environment**: Users have stable internet connectivity and a C++23-compliant compiler (GCC 13+, Clang 16+, or MSVC 2022+) as specified in the project build requirements.
- **Existing infrastructure is available**: The type checker builds on existing TypedAst.hpp, Type.hpp, and NodeKind.hpp infrastructure. No changes to these base files are required beyond their current design.
- **Scope boundaries**: Advanced type system features (subtyping, type classes, traits, bidirectional type checking, type holes) are out of scope for this version.
- **Logging infrastructure available**: The type checker integrates with the project's existing spdlog-based logging system for debug tracing and constraint solving diagnostics.
- **Error code taxonomy**: Type checker uses the existing E2xxx semantic analysis error codes (E2001-E2999) from error_codes.hpp; four new codes (E2033-E2036) will be added for constraint solver-specific errors (unification failure, occurs check, unresolved type variable, constraint generation error).

## Clarifications

### Session 2026-04-02

- Q: What type promotion rules should the type checker enforce for mixed-type arithmetic operations? → A: No promotions - operands must have identical types; all mixed-type operations are errors requiring explicit casts
- Q: Should the type checker include debug logging/tracing capabilities for constraint solving? → A: Runtime configurable logging - spdlog-based logging with runtime level control (trace/debug/info); logs to file/console like the rest of jsav compiler
- Q: How should type checker error codes be structured and categorized? → A: Use your existing E2xxx range and taxonomy as-is, add ~4 new codes for constraint solver errors starting at E2033
