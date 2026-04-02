# Contract: Internal Type Checker API

## Context

This contract defines the internal interface between semantic analysis and downstream compiler phases. It consumes the raw AST from `include/jsav/ast` and produces fully-typed AST output plus aggregated diagnostics.

## API Surface

```cpp
std::pair<TypedProgram, std::vector<CompileError>> typeCheck(const Program& raw_ast);
```

## Input Contract

- Input root node type: `jsv::Program` (`include/jsav/ast/Program.hpp`).
- Input tree may contain any `NodeKind` defined in `include/jsav/ast/NodeKind.hpp`.
- `SourceSpan` should be present when parser produced location info; checker must preserve spans.
- Name resolution phase must be executed before type checking.

## Output Contract

- `first` (`TypedProgram`):
  - structurally aligned with input program statements.
  - all typed nodes carry non-null `node_type()` after zonking.
  - in presence of errors, partial typing is allowed but must use `ErrorType` placeholders where needed.
- `second` (`std::vector<CompileError>`):
  - contains all discovered type errors (non fail-fast).
  - includes E2xxx semantic codes and E2033-E2036 solver codes.
  - sorted by source location for deterministic diagnostics.

## Behavioral Guarantees

1. Constraint generation traverses full AST and creates equations for all type-relevant operations.
2. Unification uses occurs check to reject recursive types.
3. Polymorphic call sites instantiate fresh type variables per invocation.
4. Zonking applies final substitution across entire typed AST.
5. Checker remains single-threaded and in-memory.

## Error Code Mapping

- E2033: Constraint generation error
- E2034: Unification failure (type mismatch)
- E2035: Occurs check failure (recursive type)
- E2036: Unresolved type variable after solving

## Logging Contract

- trace: constraint and unify step details
- debug: per-function summary and substitutions
- info: global statistics and final summary
- no structured metrics API; diagnostics are log-based only

## Compatibility

- Interface is internal-only; no backward compatibility guarantee across compiler versions.
- Downstream phases must be rebuilt with each compiler version.
