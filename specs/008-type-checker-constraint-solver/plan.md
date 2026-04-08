# Implementation Plan: Hindley-Milner Type Checker with Constraint Solver

**Branch**: `008-type-checker-constraint-solver` | **Date**: 2026-04-02 | **Spec**: [spec.md](./spec.md)
**Input**: Feature specification from `/specs/008-type-checker-constraint-solver/spec.md`

## Summary

Build a type checker for the jsav compiler that transforms Raw AST to Typed AST using Hindley-Milner style constraint-based type inference. The system generates type constraints during AST traversal, solves them using union-find unification, and produces a fully typed AST with comprehensive error collection. Key components: type variables, constraint generation, union-find solver with occurs check, zonking (substitution application), and parametric polymorphism support.

## Technical Context

**Language/Version**: C++23 (Visual Studio 2026 compatible features only)
**Primary Dependencies**: spdlog (existing, per Constitution Principle V) — no additional external dependencies
**Storage**: N/A (all in-memory processing)
**Testing**: Catch2 (existing infrastructure) — runtime tests in `test/tests.cpp`
**Target Platform**: Windows/Linux/macOS (cross-platform via C++23 standard library)
**Project Type**: Compiler module (library component of jsav compiler)
**Performance Goals**: 10,000+ AST nodes in <5 seconds (SC-004), 100K constraints in <50MB (SC-009)
**Constraints**: Single-threaded, no additional external dependencies beyond spdlog (existing) and Catch2 (test-only), graceful degradation on resource limits
**Scale/Scope**: Support all 28 NodeKind variants, 17 primitive/compound types, 4 new error codes (E2033-E2036)

## Constitution Check

*GATE: Must pass before Phase 0 research. Re-check after Phase 1 design.*

### Pre-Design Gate Evaluation

| Principle | Status | Justification |
|-----------|--------|---------------|
| **I. Platform Independence** | ✅ Compliant | Uses C++23 standard library exclusively; no OS-specific APIs |
| **II. Visual Studio 2026 Compatibility** | ✅ Compliant | Only C++23 features verified as fully supported by MSVC 2026 |
| **III. C++ Core Guidelines Compliance** | ✅ Compliant | `std::unique_ptr` for AST ownership, `std::expected` for errors, const correctness enforced |
| **III.a Ownership Semantics** | ✅ Compliant | TypePtr = `std::shared_ptr<TypeBase>` (existing), unique_ptr for typed nodes |
| **III.b Pervasive Const Correctness** | ✅ Compliant | All read-only parameters will be const&, visitor methods const |
| **III.c Conscious Move Semantics** | ✅ Compliant | Typed AST nodes move-only; noexcept move operations |
| **III.d Structured Error Handling** | ✅ Compliant | CompileError collection, no exceptions for type errors |
| **IV. Test-Driven Development** | ✅ Compliant | Three-tier test pyramid per Constitution Principle IV: (1) `test/constexpr_tests.cpp` — `STATIC_REQUIRE` for constexpr-capable pure functions and value types (TypeScheme::mono(), TypeVariable properties, ErrorCode enum values, Constraint struct). (2) `test/relaxed_constexpr_tests.cpp` — runtime debugging version of constexpr tests. (3) `test/tests.cpp` — runtime tests for all type checking logic, error reporting, and polymorphism via `REQUIRE` assertions. All tests written FIRST following Red-Green-Refactor. |
| **V. Dependency Management** | ✅ Compliant | No new dependencies; uses existing spdlog for logging |
| **VI. Documentation Standards** | ✅ Compliant | Doxygen comments for all public interfaces |
| **VII. Algorithmic Design Excellence** | ✅ Compliant | Union-find with path compression (O(α(n)) amortized) |

### Gate Result: **PASS** — Proceed to Phase 0 research.

## Project Structure

### Documentation (this feature)

```text
specs/008-type-checker-constraint-solver/
├── plan.md              # This file
├── research.md          # Phase 0 output: algorithm decisions
├── data-model.md        # Phase 1 output: entity definitions
├── quickstart.md        # Phase 1 output: usage examples
└── tasks.md             # Phase 2 output (/speckit.tasks command)
```

### Source Code (repository root)

```text
include/jsav/
├── ast/                        # Existing - Raw and Typed AST (already exists)
│   ├── Node.hpp               # Base AST node
│   ├── Type.hpp               # Type system definitions
│   ├── TypedNode.hpp          # Typed AST base
│   ├── TypedExpressions.hpp   # Typed expression nodes
│   ├── TypedStatements.hpp    # Typed statement nodes
│   └── TypedProgram.hpp       # Typed program root
├── typechecker/               # NEW - Type checker module
│   ├── TypeChecker.hpp        # Main type checker interface
│   ├── TypeVariable.hpp       # Type variable representation (?T1, ?T2, ...)
│   ├── Constraint.hpp         # Type constraint representation
│   ├── ConstraintSolver.hpp   # Union-find based solver
│   ├── Substitution.hpp       # Type variable → Type mapping
│   ├── SymbolTable.hpp        # Scoped identifier → TypeScheme mapping
│   ├── TypeScheme.hpp         # Quantified type for polymorphism (∀T. T → T)
│   └── ErrorType.hpp          # Error type placeholder for error recovery

src/jsav_Lib/
├── typechecker/               # NEW - Implementation
│   ├── TypeChecker.cpp        # Main entry point
│   ├── TypeVariable.cpp       # Type variable counter (TLS)
│   ├── Constraint.cpp         # Constraint generation
│   ├── ConstraintSolver.cpp   # Union-find unification
│   ├── Substitution.cpp       # Zonking implementation
│   └── SymbolTable.cpp        # Scope management

test/
└── tests.cpp                  # Runtime tests (add type checker tests here)
```

**Structure Decision**: Single module under `include/jsav/typechecker/` and `src/jsav_Lib/typechecker/` following existing project conventions. No separate library — integrated into `jsav_Lib`. Tests added to existing `test/tests.cpp` file as specified.

## Complexity Tracking

> No constitution violations requiring justification. Design is compliant.

---

## Post-Design Constitution Re-Evaluation

### Phase 1 Design Review

| Principle | Status | Post-Design Notes |
|-----------|--------|-------------------|
| **I. Platform Independence** | ✅ Confirmed | data-model.md uses only std:: types (vector, unordered_map, shared_ptr, expected) |
| **II. Visual Studio 2026 Compatibility** | ✅ Confirmed | All C++23 features used (std::expected, constexpr, concepts) are VS2026-supported |
| **III.a Ownership Semantics** | ✅ Confirmed | TypeVariable (value type), Constraint (ID + refs), TypeScheme (value type) — no raw pointers |
| **III.b Pervasive Const Correctness** | ✅ Confirmed | Entity interfaces show const methods; no mutable state in type representations |
| **III.c Conscious Move Semantics** | ✅ Confirmed | Union-find parent_ vector supports efficient moves |
| **III.d Structured Error Handling** | ✅ Confirmed | TypeCheckResult with errors vector; ErrorType for silent propagation |
| **IV. Test-Driven Development** | ✅ Confirmed | quickstart.md examples directly translate to Catch2 TEST_CASEs in test/tests.cpp |
| **V. Dependency Management** | ✅ Confirmed | No new deps introduced; uses existing TypePtr, CompileError, spdlog |
| **VI. Documentation Standards** | ✅ Confirmed | data-model.md includes Doxygen-style descriptions for all entities |
| **VII. Algorithmic Design Excellence** | ✅ Confirmed | Union-find with path compression + union by rank = O(α(n)) amortized |

### Gate Result: **PASS** — Design is constitution-compliant. Ready for /speckit.tasks.

---

## Generated Artifacts Summary

| Artifact | Path | Description |
|----------|------|-------------|
| **research.md** | `specs/008-type-checker-constraint-solver/research.md` | Phase 0: Algorithm decisions (union-find, type variables, constraints, error propagation) |
| **data-model.md** | `specs/008-type-checker-constraint-solver/data-model.md` | Phase 1: Entity definitions (9 entities with C++ interfaces) |
| **quickstart.md** | `specs/008-type-checker-constraint-solver/quickstart.md` | Phase 1: Usage examples and test patterns |
| **plan.md** | `specs/008-type-checker-constraint-solver/plan.md` | This file: Implementation plan with structure and constitution checks |
