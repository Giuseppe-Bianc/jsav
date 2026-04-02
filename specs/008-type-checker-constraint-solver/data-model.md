# Data Model: Type Checker Constraint Solver

## Entity: TypeCheckerResult

- Purpose: Output envelope of type checker pass.
- Fields:
  - `typed_program: TypedProgram`
  - `errors: std::vector<CompileError>`
- Validation:
  - `errors` sorted by source location for deterministic reporting.
  - `typed_program` must remain structurally isomorphic to input `Program` except for typed wrappers.

## Entity: TypeVariable

- Purpose: Represents unknown type during inference.
- Fields:
  - `id: uint32_t` (sequential)
  - `display_name: std::string` (`T{id}` or `?T{id}` for diagnostics)
  - `origin_span: SourceSpan`
  - `state: {Unbound, Bound}`
- Validation:
  - ID unique within one typeCheck invocation.
  - Unbound state after solving emits E2036.
- State transitions:
  - `Unbound -> Bound` via successful unification.

## Entity: Constraint

- Purpose: Equation produced by constraint generation.
- Fields:
  - `id: uint32_t` (`C1`, `C2`, ...)
  - `lhs: TypeExpr`
  - `rhs: TypeExpr`
  - `origin_node_kind: NodeKind`
  - `origin_span: SourceSpan`
  - `category: {Operation, Assignment, Call, ControlFlow, Collection, Return, Annotation}`
- Validation:
  - Both sides non-null type expressions.
  - Origin span must be valid when derived from AST node.

## Entity: TypeExpr (solver internal representation)

- Purpose: Normalized type representation used by unifier.
- Variants:
  - `Concrete(TypePtr)`
  - `Variable(TypeVariableId)`
  - `Function(std::vector<TypeExpr> params, TypeExpr ret)`
  - `Array(TypeExpr element, size_t size)`
  - `Vector(TypeExpr element)`
  - `Error`
- Validation:
  - Function signature arity >= 0.
  - Array size compile-time constant.

## Entity: SubstitutionMap

- Purpose: Final mapping from type variables to solved types.
- Fields:
  - `bindings: std::unordered_map<TypeVariableId, TypeExpr>`
- Validation:
  - No cyclic bindings after occurs check.
  - Each key appears at most once.

## Entity: UnionFindState

- Purpose: Efficient equivalence tracking for variables and composite merges.
- Fields:
  - `parent: std::vector<uint32_t>`
  - `rank: std::vector<uint8_t>`
  - `repr_type: std::vector<std::optional<TypeExpr>>`
- Operations:
  - `find(id)` with path compression
  - `union(a, b)` with union by rank
  - `bind(root, type)`
- Validation:
  - Parent references in bounds.
  - Root representative stable after compression.

## Entity: TypeEnvironment

- Purpose: Lexical scope map from identifiers to type schemes.
- Fields:
  - `scopes: std::vector<std::unordered_map<std::string, TypeScheme>>`
- Operations:
  - `pushScope()`, `popScope()`
  - `declare(name, scheme)`
  - `lookup(name)` from innermost to outermost
- Validation:
  - Supports shadowing (inner scope wins).

## Entity: TypeScheme

- Purpose: Polymorphic function/identifier type with quantification.
- Fields:
  - `quantified_vars: std::vector<TypeVariableId>`
  - `body: TypeExpr`
- Validation:
  - Instantiation produces fresh variables per call site.

## Entity: CompileError (type-checking view)

- Purpose: Structured diagnostic emitted by checker and solver.
- Fields:
  - `code: ErrorCode` (includes E2033-E2036)
  - `span: SourceSpan`
  - `message: std::string`
  - `expected: std::optional<std::string>`
  - `actual: std::optional<std::string>`
  - `help: std::optional<std::string>`
  - `root_cause_hint: std::optional<SourceSpan>`
- Validation:
  - Message never empty.
  - Span set whenever AST location is available.

## Entity: TypedNodeBinding

- Purpose: Temporary link from raw nodes to inferred type expressions before zonking.
- Fields:
  - `node_ptr: const Node*`
  - `inferred: TypeExpr`
- Validation:
  - One binding per visited AST node.

## Relation Map

- `Program` (raw AST root) contains `Stmt` graph (`Statements.hpp`) and `Expr` graph (`Expressions.hpp`).
- Constraint generation visits each raw node and creates:
  - `TypedNodeBinding`
  - zero or more `Constraint`
- Solver consumes all `Constraint` and outputs `SubstitutionMap` plus additional `CompileError`.
- Zonking applies `SubstitutionMap` to produce concrete `TypePtr` on all typed nodes, then builds `TypedProgram`.
- `TypeEnvironment` and `TypeScheme` drive identifier resolution and polymorphic call instantiation during generation.

## Node Coverage Contract (Raw AST)

The generator/solver pipeline must handle all current `NodeKind` values from `include/jsav/ast/NodeKind.hpp`:

- Expressions: literals, identifier, unary/binary/ternary, call/index/member, assign/cast, array/grouping.
- Statements: expr, var decl, func decl, return, if/while/for, block, break/continue, main.
- Top-level: program.

## Invariants

- Every typed node in final `TypedProgram` has non-null `node_type()`.
- No unresolved type variable remains without corresponding E2036 diagnostic.
- Error accumulation is monotonic: no phase deletes previously recorded errors.
- `ErrorType` propagation preserves traversal continuity while preventing invalid hard-fail paths.
