# Phase 0 Research - Verifiable Multi-Level IR System

## Decision 1: Canonical stack and toolchain

- Decision: Use C++23 with MSVC 2026 as baseline, CMake 4.2+, and Ninja.
- Rationale: Consistency with the constitution (VS 2026 compatibility), reproducible builds, and a toolchain already standard in the repository.
- Alternatives considered: Clang-only toolchain (rejected: not the team baseline), GCC-only (rejected: lower alignment with the primary environment).

## Decision 2: Modular monolith architecture

- Decision: Keep a modular monolith architecture with IR/analysis/passes/validation subsystems.
- Rationale: Highly cohesive compiler domain, simpler pass transactions, lower operational cost than distributed systems.
- Alternatives considered: Microservices (rejected: unjustified network/observability overhead), dynamic runtime plugins (rejected: ABI complexity).

## Decision 3: Allowed dependencies and pinning

- Decision: Use only fmtlib 12.1.0, spdlog 1.17.0, CLI11 2.6.1, Catch2 3.14.0 with the existing version lock.
- Rationale: Explicit project constraint (no new dependencies), stability, and auditability.
- Alternatives considered: Adding external dataflow/graph libraries (rejected: constraint violation + integration cost).

## Decision 4: Unified error model

- Decision: All recoverable failures use CompileError and std::expected<T, std::vector<CompileError>>.
- Rationale: Supports deterministic batch-per-pass reporting (FR-026), simplifies cross-module error handling.
- Alternatives considered: Exceptions for validation (rejected: difficult deterministic batching), numeric error codes (rejected: insufficient context).

## Decision 5: Canonical SSA/PHI

- Decision: Canonical PHI placement based on reaching definitions (iterative dataflow with sparse bitsets), with eager pruning/minimization on CFG updates.
- Rationale: Direct alignment with FR-006/007/009 and fewer redundant PHIs versus pure dominance.
- Alternatives considered: Dominance frontier as primary criterion (rejected: over-approximation), incremental SSA without final RD verification (rejected: non-canonical).

## Decision 6: Cross-level traceability

- Decision: Immutable global IDs, deterministic from canonical structural paths (module/function/block/index/entity-type) with explicit derivation relations.
- Rationale: Robust HIR->MIR->LIR auditing and stable ordering for output/errors/reports.
- Alternatives considered: Random UUIDs (rejected: non-deterministic), global runtime counters (rejected: execution-order dependency).

## Decision 7: Data and transactional state

- Decision: Typed in-memory graph model with transactional working copy per function/pass; atomic commit only after validation.
- Rationale: Guarantees FR-022/SC-008 and prevents persistent invalid intermediate states.
- Alternatives considered: In-place mutation with undo log (rejected: larger bug surface), full-module snapshot for each pass (rejected: excessive memory cost).

## Decision 8: Strict no-reorder memory strategy

- Decision: In the presence of may-alias, reordering is forbidden unless formal proof exists (verifiable certificate or reproducible internal proof with full log).
- Rationale: Preserves observable memory equivalence (FR-028/030) and reduces semantic regressions.
- Alternatives considered: Permissive reordering heuristics (rejected: risk of non-determinism/unsoundness).

## Decision 9: Test pyramid and coverage

- Decision: Use three levels: constexpr compile-time, relaxed constexpr debug runtime, full runtime; include systematic edge/corner cases.
- Rationale: Balances formal compile-time verification and dynamic behavior on normal/anomalous conditions.
- Alternatives considered: Runtime-only tests (rejected: loss of compile-time guarantees), property-based fuzzing only (rejected: does not replace deterministic tests).

## Decision 10: CI/CD quality gates

- Decision: GitHub Actions with mandatory gates: zero-warning build, tests, clang-tidy/cppcheck, ASan/UBSan, lizard, gcovr >= 95%.
- Rationale: Automatic enforcement of constitution rules + SC success criteria.
- Alternatives considered: Local-only validation (rejected: environment drift), non-binding coverage (rejected: risk of untested branches).

## Decision 11: Environment configuration

- Decision: Configurations injected via CMake Presets and environment variables, no secrets in the repository.
- Rationale: Dev/CI/release environment parity and configuration traceability.
- Alternatives considered: Hardcoded config (rejected: anti-pattern), versioned .env files (rejected: security risk).
