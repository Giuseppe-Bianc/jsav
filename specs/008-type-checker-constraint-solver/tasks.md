# Tasks: Hindley-Milner Type Checker with Constraint Solver

**Input**: Design documents from `/specs/008-type-checker-constraint-solver/`
**Prerequisites**: plan.md ✓, spec.md ✓, research.md ✓, data-model.md ✓, quickstart.md ✓

**Tests**: TDD approach mandated by constitution Principle IV — tests written FIRST in `test/tests.cpp`

**Organization**: Tasks grouped by user story for independent implementation and testing.

## Format: `[ID] [P?] [Story] Description`

- **[P]**: Can run in parallel (different files, no dependencies)
- **[Story]**: Which user story this task belongs to (e.g., US1, US2, US3)
- Include exact file paths in descriptions

## Path Conventions

- **Headers**: `include/jsav/typechecker/` (new module)
- **Sources**: `src/jsav_Lib/typechecker/` (new directory)
- **Tests**: `test/tests.cpp` (existing file)
- **Existing AST**: `include/jsav/ast/` (modify Type.hpp only)

---

## Phase 1: Setup (Shared Infrastructure)

**Purpose**: Create typechecker module structure and extend existing type system

- [ ] T001 Create directory `include/jsav/typechecker/` for type checker headers
- [ ] T002 Create directory `src/jsav_Lib/typechecker/` for type checker implementations
- [ ] T003 [P] Extend TypeKind enum with `TypeVar` and `Error` values in `include/jsav/ast/Type.hpp`
- [ ] T004 [P] Add typechecker source directory to CMake in `src/jsav_Lib/CMakeLists.txt`
- [ ] T004a [P] Verify TypedAst.hpp compatibility: confirm TypedNode, TypedExpr, TypedStmt, TypedProgram interfaces in `include/jsav/ast/` support type checker integration (FR-019)

---

## Phase 2: Foundational (Blocking Prerequisites)

**Purpose**: Core data structures that ALL user stories depend on — MUST complete before story work

**⚠️ CRITICAL**: No user story work can begin until this phase is complete

### Tests for Foundational Components

- [ ] T005 [P] Add TypeVariable tests in `test/tests.cpp`: fresh_type_variable uniqueness, id() accessor, to_string() format (?T1)
- [ ] T006 [P] Add ErrorType tests in `test/tests.cpp`: singleton behavior, classof() predicate, kind() returns TypeKind::Error
- [ ] T007 [P] Add UnionFind tests in `test/tests.cpp`: make_set, find with path compression, unite with union by rank, same_set
- [ ] T008 [P] Add Substitution tests in `test/tests.cpp`: bind, lookup, contains, apply to type variables and compound types
- [ ] T009 [P] Add SymbolTable tests in `test/tests.cpp`: push_scope, pop_scope, define, lookup with shadowing, depth()
- [ ] T010 [P] Add TypeScheme tests in `test/tests.cpp`: mono() creation, instantiate() generates fresh variables
- [ ] T011 [P] Add Constraint tests in `test/tests.cpp`: ConstraintSet.add returns sequential IDs (C1, C2), get by ID, constraints() iteration

### Compile-Time Tests (constexpr — per Constitution Principle IV, three-tier test pyramid)

> **Rationale**: Per Constitution Principle IV, constexpr-capable pure functions and value types MUST receive `STATIC_REQUIRE` coverage in `test/constexpr_tests.cpp` (and `test/relaxed_constexpr_tests.cpp` for debugging). The following components are constexpr-capable:

- [ ] T011a [P] Add constexpr test in `test/constexpr_tests.cpp`: `STATIC_REQUIRE` TypeScheme::mono() creates monomorphic scheme with empty quantified_vars
- [ ] T011b [P] Add constexpr test in `test/constexpr_tests.cpp`: `STATIC_REQUIRE` TypeVariable constexpr properties (TypeVarId is std::size_t, sizeof checks)
- [ ] T011c [P] Add constexpr test in `test/constexpr_tests.cpp`: `STATIC_REQUIRE` ErrorCode enum values (E2033=2033, E2034=2034, E2035=2035, E2036=2036) — once ErrorCode enum is defined in data-model.md
- [ ] T011d [P] Add constexpr test in `test/constexpr_tests.cpp`: `STATIC_REQUIRE` Constraint struct is trivially copyable and sizeof is as expected for value-type semantics

### Implementation for Foundational Components

- [ ] T012 [P] Create TypeVariable class in `include/jsav/typechecker/TypeVariable.hpp` with id(), to_string(), classof()
- [ ] T013 [P] Implement fresh_type_variable() with TLS counter in `src/jsav_Lib/typechecker/TypeVariable.cpp`
- [ ] T014 [P] Create ErrorType class in `include/jsav/typechecker/ErrorType.hpp` with classof()
- [ ] T015 [P] Implement error_type() singleton in `src/jsav_Lib/typechecker/ErrorType.cpp`
- [ ] T016 [P] Create UnionFind class in `include/jsav/typechecker/UnionFind.hpp` with make_set, find, unite, same_set
- [ ] T017 [P] Implement UnionFind with path compression and union by rank in `src/jsav_Lib/typechecker/UnionFind.cpp`
- [ ] T018 [P] Create Substitution class in `include/jsav/typechecker/Substitution.hpp` with bind, lookup, apply, contains
- [ ] T019 [P] Implement Substitution.apply() for all TypeKind variants in `src/jsav_Lib/typechecker/Substitution.cpp`
- [ ] T020 [P] Create SymbolTable class in `include/jsav/typechecker/SymbolTable.hpp` with push_scope, pop_scope, define, lookup
- [ ] T021 [P] Implement SymbolTable with stack-based scopes in `src/jsav_Lib/typechecker/SymbolTable.cpp`
- [ ] T022 [P] Create TypeScheme struct in `include/jsav/typechecker/TypeScheme.hpp` with quantified_vars, body, instantiate(), mono()
- [ ] T023 [P] Implement TypeScheme.instantiate() in `src/jsav_Lib/typechecker/TypeScheme.cpp`
- [ ] T024 [P] Create Constraint struct and ConstraintSet class in `include/jsav/typechecker/Constraint.hpp`
- [ ] T025 [P] Implement ConstraintSet.add() with sequential ID generation in `src/jsav_Lib/typechecker/Constraint.cpp`

**Checkpoint**: Foundation ready — all data structures tested and implemented. User story work can begin.

---

## Phase 3: User Story 1 — Type Check Well-Typed Programs (Priority: P1) 🎯 MVP

**Goal**: Invoke type checker on well-typed Raw AST and receive fully typed AST with all types resolved

**Independent Test**: Provide well-typed AST (e.g., `fn add(x: i32, y: i32) -> i32 { x + y }`), verify output has concrete types on every node and error vector is empty

### Tests for User Story 1

- [ ] T026 [P] [US1] Add test TypeChecker_IntegerLiteral_ReturnsI32Type in `test/tests.cpp`
- [ ] T027 [P] [US1] Add test TypeChecker_BooleanLiteral_ReturnsBoolType in `test/tests.cpp`
- [ ] T028 [P] [US1] Add test TypeChecker_StringLiteral_ReturnsStringType in `test/tests.cpp`
- [ ] T029 [P] [US1] Add test TypeChecker_BinaryAdd_InfersOperandTypes in `test/tests.cpp`
- [ ] T030 [P] [US1] Add test TypeChecker_BinaryComparison_ReturnsBool in `test/tests.cpp`
- [ ] T031 [P] [US1] Add test TypeChecker_UnaryNegate_PreservesType in `test/tests.cpp`
- [ ] T032 [P] [US1] Add test TypeChecker_VarDecl_MatchesAnnotation in `test/tests.cpp`
- [ ] T033 [P] [US1] Add test TypeChecker_FunctionDecl_BuildsFunctionType in `test/tests.cpp`
- [ ] T034 [P] [US1] Add test TypeChecker_FunctionCall_ResolvesReturnType in `test/tests.cpp`
- [ ] T035 [P] [US1] Add test TypeChecker_IfExpression_JoinsBranchTypes in `test/tests.cpp`
- [ ] T036 [P] [US1] Add test TypeChecker_ArrayLiteral_InfersElementType in `test/tests.cpp`
- [ ] T036a [P] [US1] Add test TypeChecker_Assignment_MutableLvalueRequired in `test/tests.cpp`: verify assignment to immutable binding produces error
- [ ] T036b [P] [US1] Add test TypeChecker_Assignment_TypeMismatch in `test/tests.cpp`: verify RHS type must match LHS type exactly (e.g., `i32 = string` → error E2034)
- [ ] T037 [P] [US1] Add test TypeChecker_NestedExpressions_AllNodesTyped in `test/tests.cpp`
- [ ] T038 [P] [US1] Add test TypeChecker_WellTypedProgram_EmptyErrorVector in `test/tests.cpp`

### Implementation for User Story 1

- [ ] T039 [US1] Create ConstraintSolver class in `include/jsav/typechecker/ConstraintSolver.hpp` with solve(), unify(), occurs_in()
- [ ] T040 [US1] Implement ConstraintSolver.unify() for primitive types in `src/jsav_Lib/typechecker/ConstraintSolver.cpp`
- [ ] T041 [US1] Implement ConstraintSolver.unify() for type variables in `src/jsav_Lib/typechecker/ConstraintSolver.cpp`
- [ ] T042 [US1] Implement ConstraintSolver.occurs_in() recursive check in `src/jsav_Lib/typechecker/ConstraintSolver.cpp`
- [ ] T043 [US1] Create TypeChecker class in `include/jsav/typechecker/TypeChecker.hpp` with check() returning `std::pair<TypedProgram, std::vector<CompileError>>`, type_expr(), type_stmt()
- [ ] T044 [US1] Implement TypeChecker.resolve_names() for symbol table population in `src/jsav_Lib/typechecker/TypeChecker.cpp`
- [ ] T045 [US1] Implement TypeChecker.generate_constraints() for literals (integer, bool, string, char) in `src/jsav_Lib/typechecker/TypeChecker.cpp`
- [ ] T046 [US1] Implement TypeChecker.generate_constraints() for binary expressions in `src/jsav_Lib/typechecker/TypeChecker.cpp`
- [ ] T047 [US1] Implement TypeChecker.generate_constraints() for unary expressions in `src/jsav_Lib/typechecker/TypeChecker.cpp`
- [ ] T048 [US1] Implement TypeChecker.generate_constraints() for variable declarations in `src/jsav_Lib/typechecker/TypeChecker.cpp`
- [ ] T049 [US1] Implement TypeChecker.generate_constraints() for function declarations in `src/jsav_Lib/typechecker/TypeChecker.cpp`
- [ ] T050 [US1] Implement TypeChecker.generate_constraints() for function calls in `src/jsav_Lib/typechecker/TypeChecker.cpp`
- [ ] T051 [US1] Implement TypeChecker.generate_constraints() for if/else statements in `src/jsav_Lib/typechecker/TypeChecker.cpp`
- [ ] T052 [US1] Implement TypeChecker.generate_constraints() for return statements in `src/jsav_Lib/typechecker/TypeChecker.cpp`
- [ ] T053 [US1] Implement TypeChecker.generate_constraints() for array literals in `src/jsav_Lib/typechecker/TypeChecker.cpp`
- [ ] T053a [US1] Implement TypeChecker.generate_constraints() for assignment statements in `src/jsav_Lib/typechecker/TypeChecker.cpp`: generate constraint LHS_type = RHS_type; verify LHS is mutable lvalue; on mismatch, insert ErrorType and report E2034 per FR-014
- [ ] T054 [US1] Implement TypeChecker.solve_constraints() wrapper in `src/jsav_Lib/typechecker/TypeChecker.cpp`
- [ ] T055 [US1] Implement TypeChecker.zonk() for substitution application in `src/jsav_Lib/typechecker/TypeChecker.cpp`
- [ ] T056 [US1] Implement TypeChecker.check() returning `std::pair<TypedProgram, std::vector<CompileError>>` per spec.md Output Contract, orchestrating all phases in `src/jsav_Lib/typechecker/TypeChecker.cpp`
- [ ] T057 [US1] Add spdlog trace logging for constraint generation in `src/jsav_Lib/typechecker/TypeChecker.cpp`
- [ ] T058 [US1] Add spdlog debug logging for unification steps in `src/jsav_Lib/typechecker/ConstraintSolver.cpp`

**Checkpoint**: User Story 1 complete — well-typed programs produce fully typed AST with empty error vector.

---

## Phase 4: User Story 2 — Comprehensive Type Error Reporting (Priority: P2)

**Goal**: Collect ALL type errors in single pass with source locations, error codes, and helpful messages

**Independent Test**: Provide AST with multiple type violations (`3 + "hello"`, `true && 5`, `return 42` in void function), verify all errors collected with proper spans and codes

### Tests for User Story 2

- [ ] T059 [P] [US2] Add test TypeChecker_TypeMismatch_ReportsE2034 in `test/tests.cpp`
- [ ] T060 [P] [US2] Add test TypeChecker_MultipleErrors_CollectsAll in `test/tests.cpp`
- [ ] T061 [P] [US2] Add test TypeChecker_ErrorSpan_PointsToExactLocation in `test/tests.cpp`
- [ ] T062 [P] [US2] Add test TypeChecker_ErrorMessage_ShowsExpectedVsActual in `test/tests.cpp`
- [ ] T063 [P] [US2] Add test TypeChecker_BinaryTypeMismatch_SuggestsCast in `test/tests.cpp`
- [ ] T064 [P] [US2] Add test TypeChecker_ReturnTypeMismatch_ReportsError in `test/tests.cpp`
- [ ] T065 [P] [US2] Add test TypeChecker_IfConditionNotBool_ReportsError in `test/tests.cpp`
- [ ] T066 [P] [US2] Add test TypeChecker_ArrayElementMismatch_ReportsError in `test/tests.cpp`
- [ ] T067 [P] [US2] Add test TypeChecker_FunctionArgCountMismatch_ReportsError in `test/tests.cpp`
- [ ] T068 [P] [US2] Add test TypeChecker_ErrorType_PropagatesSilently in `test/tests.cpp`
- [ ] T069 [P] [US2] Add test TypeChecker_CascadingError_HintsRootCause in `test/tests.cpp`

### Implementation for User Story 2

- [ ] T070 [US2] Implement CompileError creation with E2034 code for unification failures in `src/jsav_Lib/typechecker/ConstraintSolver.cpp`
- [ ] T071 [US2] Implement error message formatting with expected vs actual types in `src/jsav_Lib/typechecker/ConstraintSolver.cpp`
- [ ] T072 [US2] Implement fix suggestion generation ("did you mean to cast?") in `src/jsav_Lib/typechecker/ConstraintSolver.cpp`
- [ ] T073 [US2] Implement ErrorType insertion on type error detection in `src/jsav_Lib/typechecker/TypeChecker.cpp`
- [ ] T074 [US2] Implement ErrorType silent propagation (unifies with any type) in `src/jsav_Lib/typechecker/ConstraintSolver.cpp`
- [ ] T075 [US2] Implement error collection (append to vector, never fail-fast) in `src/jsav_Lib/typechecker/TypeChecker.cpp`
- [ ] T076 [US2] Implement cascading error annotation ("may be consequence of line X") in `src/jsav_Lib/typechecker/TypeChecker.cpp`
- [ ] T077 [US2] Implement E2033 (constraint generation error) reporting in `src/jsav_Lib/typechecker/TypeChecker.cpp`
- [ ] T078 [US2] Implement E2035 (occurs check failure) reporting in `src/jsav_Lib/typechecker/ConstraintSolver.cpp`
- [ ] T079 [US2] Implement E2036 (unresolved type variable) reporting in `src/jsav_Lib/typechecker/TypeChecker.cpp`
- [ ] T080 [US2] Sort errors by source location before returning in `src/jsav_Lib/typechecker/TypeChecker.cpp`

**Checkpoint**: User Story 2 complete — all type errors collected with actionable messages and source locations.

---

## Phase 5: User Story 3 — Parametric Polymorphism Support (Priority: P3)

**Goal**: Support generic functions with type variables and per-call-site constraint solving

**Independent Test**: Define generic function `fn id<T>(x: T) -> T { x }`, call with different types (`id(42)`, `id("hello")`), verify each call site has correctly instantiated types

### Tests for User Story 3

- [ ] T081 [P] [US3] Add test TypeChecker_GenericFunction_CreatesTypeScheme in `test/tests.cpp`
- [ ] T082 [P] [US3] Add test TypeChecker_GenericCall_InstantiatesFreshVars in `test/tests.cpp`
- [ ] T083 [P] [US3] Add test TypeChecker_GenericIdentity_ResolvesToConcreteType in `test/tests.cpp`
- [ ] T084 [P] [US3] Add test TypeChecker_MultipleCalls_IndependentInstantiation in `test/tests.cpp`
- [ ] T085 [P] [US3] Add test TypeChecker_GenericConstraintViolation_ReportsAtCallSite in `test/tests.cpp`
- [ ] T086 [P] [US3] Add test TypeChecker_NestedGenericCalls_ResolveCorrectly in `test/tests.cpp`
- [ ] T087 [P] [US3] Add test TypeChecker_TenDifferentTypes_NoCrossContamination in `test/tests.cpp`

### Implementation for User Story 3

- [ ] T088 [US3] Implement TypeScheme creation for generic function declarations in `src/jsav_Lib/typechecker/TypeChecker.cpp`
- [ ] T089 [US3] Implement generalization (collect free type variables) in `src/jsav_Lib/typechecker/TypeChecker.cpp`
- [ ] T090 [US3] Implement TypeScheme.instantiate() with fresh variable substitution in `src/jsav_Lib/typechecker/TypeScheme.cpp`
- [ ] T091 [US3] Implement per-call-site constraint generation for generic functions in `src/jsav_Lib/typechecker/TypeChecker.cpp`
- [ ] T092 [US3] Implement constraint solving isolation per call site in `src/jsav_Lib/typechecker/TypeChecker.cpp`
- [ ] T093 [US3] Implement type parameter lookup in function scope in `src/jsav_Lib/typechecker/SymbolTable.cpp`

**Checkpoint**: User Story 3 complete — generic functions work with independent per-call-site instantiation.

---

## Phase 6: Polish & Cross-Cutting Concerns

**Purpose**: Improvements affecting all user stories

- [ ] T094 [P] Add Doxygen documentation to all public headers in `include/jsav/typechecker/`
- [ ] T095 [P] Add integration test for 10K+ node program in `test/tests.cpp`
- [ ] T096 [P] Add memory bounds test (100K constraints < 50MB) in `test/tests.cpp`
- [ ] T097 Run quickstart.md validation — verify all code examples compile and pass
- [ ] T098 Add spdlog info logging for overall type checking statistics in `src/jsav_Lib/typechecker/TypeChecker.cpp`
- [ ] T099 Code cleanup: ensure all functions have `[[nodiscard]]` and `constexpr` where appropriate
- [ ] T100 Verify zero clang-tidy warnings in typechecker module

---

## Dependencies & Execution Order

### Phase Dependencies

- **Setup (Phase 1)**: No dependencies — can start immediately
- **Foundational (Phase 2)**: Depends on Setup — BLOCKS all user stories
- **User Story 1 (Phase 3)**: Depends on Foundational completion
- **User Story 2 (Phase 4)**: Depends on Foundational; can start in parallel with US1
- **User Story 3 (Phase 5)**: Depends on Foundational; can start in parallel with US1/US2
- **Polish (Phase 6)**: Depends on all user stories being complete

### User Story Dependencies

- **User Story 1 (P1)**: Independent after Foundational — core type checking
- **User Story 2 (P2)**: Independent after Foundational — error reporting builds on same infrastructure
- **User Story 3 (P3)**: Independent after Foundational — polymorphism uses same solver

### Within Each User Story

- Tests MUST be written and FAIL before implementation (TDD)
- ConstraintSolver before TypeChecker (solver is a dependency)
- Constraint generation before solving before zonking (pipeline order)

### Parallel Opportunities

**Phase 1 Setup**:

```text
T001, T002 → then T003, T004 in parallel
```

**Phase 2 Foundational** (all tests can run in parallel, all implementations in parallel):

```text
Tests:  T005, T006, T007, T008, T009, T010, T011 (parallel)
Impls:  T012-T025 (parallel after tests pass)
```

**Phase 3-5 User Stories** (can run in parallel after Foundational):

```text
US1 Tests:  T026-T038 (parallel)
US1 Impls:  T039-T042 → T043-T058 (solver then type checker)

US2 Tests:  T059-T069 (parallel)
US2 Impls:  T070-T080 (sequential within, parallel to US1)

US3 Tests:  T081-T087 (parallel)
US3 Impls:  T088-T093 (sequential within, parallel to US1/US2)
```

---

## Implementation Strategy

### MVP First (User Story 1 Only)

1. Complete Phase 1: Setup (T001-T004)
2. Complete Phase 2: Foundational (T005-T025)
3. Complete Phase 3: User Story 1 (T026-T058)
4. **STOP and VALIDATE**: Run `ninja tests && ctest -R typechecker`
5. MVP delivers: type checker that handles well-typed programs

### Incremental Delivery

1. Setup + Foundational → Infrastructure ready
2. Add User Story 1 → Test independently → MVP complete
3. Add User Story 2 → Test independently → Error reporting added
4. Add User Story 3 → Test independently → Polymorphism added
5. Polish → Production-ready type checker

### Parallel Team Strategy

With multiple developers after Foundational:

- Developer A: User Story 1 (core type checking)
- Developer B: User Story 2 (error reporting)
- Developer C: User Story 3 (polymorphism)

---

## Task Summary

| Phase | Description | Task Count | Parallel Opportunities |
|-------|-------------|------------|------------------------|
| Phase 1 | Setup | 5 | T003, T004, T004a parallel |
| Phase 2 | Foundational | 25 | All tests parallel; all impls parallel |
| Phase 3 | User Story 1 | 36 | 15 tests parallel; solver→typechecker |
| Phase 4 | User Story 2 | 22 | 11 tests parallel; sequential impls |
| Phase 5 | User Story 3 | 13 | 7 tests parallel; sequential impls |
| Phase 6 | Polish | 7 | T094-T096 parallel |
| **Total** | | **108** | |

---

## Notes

- All tests in `test/tests.cpp` following existing Catch2 patterns
- Test naming: `TypeChecker_[Scenario]_[ExpectedResult]`
- Test tags: `[typechecker]`, `[constraint]`, `[unification]`, `[polymorphism]`
- TDD: Write test → Verify FAIL → Implement → Verify PASS → Refactor
- Commit after each task or logical group
- Stop at any checkpoint to validate story independently
