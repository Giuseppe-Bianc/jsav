# Quickstart: Implementing the Type Checker Feature

## 1. Build Preconditions

- Use existing project toolchain and keep compatibility with Visual Studio 2026.
- Do not add external dependencies.
- Keep solver state fully in memory.

## 2. Red-Green Entry Point

1. Add failing runtime tests in `test/tests.cpp` for one minimal scenario:
   - valid arithmetic: `i32 + i32` resolves to `i32`
   - invalid arithmetic: `i32 + string` emits E2034 and keeps traversal active
2. Run tests and confirm failures.
3. Implement minimal code to pass these first tests.

## 3. Implementation Order

1. Define checker context/state:
   - fresh type variable generator
   - constraint ID generator
   - error collector
   - type environment with lexical scopes
2. Implement constraint generation over Raw AST (`Program`, `Expr`, `Stmt`) using `NodeKind` dispatch.
3. Implement unifier (union-find + occurs check).
4. Implement substitution export and zonking into fully typed nodes.
5. Implement error propagation with `ErrorType` placeholders.
6. Wire top-level API returning `std::pair<TypedProgram, std::vector<CompileError>>`.

## 4. Runtime Test Matrix (in test/tests.cpp)

- Happy path:
  - function declaration + return type consistency
  - nested binary expressions carry concrete intermediate types
  - arrays and vectors typed with consistent element types
- Error accumulation:
  - multiple independent mismatches all reported
  - branch condition non-bool, return mismatch, assignment mismatch in one program
- Polymorphism:
  - identity generic instantiated at multiple call sites with independent type vars
- Solver correctness:
  - occurs check detects recursive type and emits E2035
  - unresolved variables emit E2036
- Determinism:
  - errors ordered by source location

## 5. Logging Verification

- At trace level: per-constraint and per-unification details.
- At debug level: per-function/typecheck phase summaries.
- At info level: overall counts and completion summary.
- Ensure logger output uses existing project logging format configuration.

## 6. Local Validation Loop

1. Build project.
2. Run runtime tests covering this feature in `test/tests.cpp`.
3. Run full test suite before finishing.
4. Confirm no new dependency was introduced and no platform-specific API leaked.

## 7. Definition of Done

- `typeCheck(const Program&)` returns typed program + full error vector.
- Typed AST has concrete types after zonking.
- All required E2033-E2036 paths are test-covered in runtime tests.
- All new runtime tests are located in `test/tests.cpp`.
