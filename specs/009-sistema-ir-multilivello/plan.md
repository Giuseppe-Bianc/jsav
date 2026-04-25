# Implementation Plan: Verifiable Multi-Level IR System

**Branch**: `009-sistema-ir-multilivello` | **Date**: 2026-04-21 | **Spec**: `specs/009-sistema-ir-multilivello/spec.md`
**Glossary**: `specs/009-sistema-ir-multilivello/GLOSSARY.md`
**Input**: Feature specification from `/specs/009-sistema-ir-multilivello/spec.md`

**Note**: This template is filled in by the `/speckit.plan` command. See `.specify/templates/plan-template.md` for the execution workflow.

## Summary

Implement a multi-level IR system (HIR/MIR/LIR) in C++23 with MSVC 2026, without introducing new dependencies, based on a modular monolith with a **PassTransaction** mechanism (working copy + atomic post-validation commit). MIR SSA and PHI correctness is guaranteed by a canonical reaching-definitions criterion with eager minimization. Errors are modeled uniformly with `CompileError` and `std::expected<T, std::vector<CompileError>>` for deterministic batch validations. Traceability across levels uses immutable and deterministic global IDs derived from canonical structural paths. The test plan adopts a strict pyramid: compile-time (`test/constexpr_tests.cpp`), constexpr debug (`test/constexpr_tests.cpp`), runtime (`test/tests.cpp`) with edge/corner-case coverage on irreducible CFGs, nominal type versioning, and strict no-reorder on may-alias.

## Technical Context

**Language/Version**: C++23 (MSVC Visual Studio 2026 toolset; GCC13+/Clang16+ compatibility target)  
**Primary Dependencies**: fmtlib 12.1.0, spdlog 1.17.0, CLI11 2.6.1, Catch2 3.14.0 (already approved)  
**Storage**: In-memory graph model (HIR/MIR/LIR + analysis artifacts) with persistence only through existing input/output files  
**Testing**: Catch2 3.14.0; `STATIC_REQUIRE` compile-time, runtime debug constexpr, runtime functional + integration  
**Target Platform**: Windows 11 (MSVC 2026) as baseline; Linux/macOS as portable target
**Project Type**: Compiler (library + CLI) in a modular monolith  
**Performance Goals**: Support up to 100k instructions/function and 2M/module with deterministic bit-identical output and full per-pass validation  
**Constraints**: No new dependencies; unified error model via `CompileError`; deterministic single-thread passes; strict no-reorder for may-alias unless formally proven; zero-warning build  
**Scale/Scope**: End-to-end IR core feature (model, validation, analysis, transformations, tests, CI gates)

### Technology Stack

- Core language/toolchain: C++23 + CMake 4.2+ + Ninja + MSVC 2026
- Logging/formatting/CLI/test: `spdlog 1.17.0`, `fmtlib 12.1.0`, `CLI11 2.6.1`, `Catch2 3.14.0`
- Quality tools: `clang-tidy`, `cppcheck`, `AddressSanitizer`, `UndefinedBehaviorSanitizer`, `lizard`, `gcovr`
- Version lock strategy: explicit pinning in `Dependencies.cmake` + `cpm-package-lock.cmake`

### Architecture Pattern

Semantic modular monolith (not microservices), with isolated modules for: IR model, validation, dataflow/CFG analysis, transformations/lowering, and pass orchestration. This choice is motivated by the compiler domain, team, and cohesive codebase: it reduces operational overhead and maximizes transactional consistency and determinism. Architecture review triggers: stable exceedance of the organizational complexity threshold (team >15), systematic exceedance of per-function complexity thresholds (CCN >15), or unmanageable coupling between core subsystems with evidence that a distributed boundary lowers total cost.

### Libraries & Dependencies

- Core:

  - `fmtlib 12.1.0`: robust and portable formatting; `std::format` alternative is not fully uniform on target toolchains.
  - `spdlog 1.17.0`: structured multi-sink logging for pass/analysis auditing; custom logger alternative rejected due to maintenance cost.
  - `CLI11 2.6.1`: stable CLI parsing for pipeline/pass execution; manual parser alternative rejected due to lower robustness on edge inputs.
- Dev/Test:
  - `Catch2 3.14.0`: unified framework for the compile-time/runtime test pyramid; multi-framework alternative rejected due to complexity.
- Optional: none (project constraint: no additional external dependencies).

### Data Management

- Primary data model: typed in-memory IR graph (Module -> Function -> Block -> Instruction -> Value/Type).
- Schema approach: explicit C++ schema with strong invariants for each IR level and HIR->MIR->LIR derivation relationships.
- Data flow strategy:
  - reaching definitions with iterative sparse bitsets for canonical PHI placement;
  - dominance/liveness/dependence aligned with the stable canonical key;
  - deterministically ordered output (module/function/block/instruction-index/operand-index).

### State Management

- IR runtime state managed with a transactional working copy at function/pass granularity.
- Atomic commit only after full validation; full rollback on any failure.
- Error state centralized in `std::vector<CompileError>` aggregated per pass (batch-per-pass).

### Deployment Strategy

- Hosting/Execution: native C++ project distributed as a cross-platform binary built via CMake presets.
- CI/CD: GitHub Actions with pipeline gates `lint -> static-analysis -> build -> test -> sanitizers -> complexity -> coverage`.
- Environment parity: configurations entirely via CMake Presets; environment variables for paths/flags; no secrets in the repository.
- Branch mapping: `main` for production/release, feature branches for development, mandatory PR gating.

### Development Workflow

- Build tools: CMake + Ninja with dedicated MSVC/GCC/Clang presets.
- Testing framework: Catch2 with a pyramid:
  - compile-time in `test/constexpr_tests.cpp` (`STATIC_REQUIRE`)
  - constexpr debug in `test/constexpr_tests.cpp`
  - full runtime tests in `test/tests.cpp`
- Code organization: separated headers/public API in `include/jsav/`, implementations in `src/jsav_Lib/`, and shared core in `include/jsavCore/` + `src/jsav_Core_lib/`.
- Quality gates: zero warnings, clean clang-tidy/cppcheck, clean ASan/UBSan, lizard threshold pass, gcovr >= 95%.

## Constitution Check

*GATE: Must pass before Phase 0 research. Re-check after Phase 1 design.*

### Pre-Design Gate Evaluation

- I. Platform Independence: PASS - design based on C++23/STL and portable abstractions.
- II. Visual Studio 2026 Compatibility: PASS - MSVC 2026 baseline and supported C++23 features.
- III. C++ Core Guidelines Compliance: PASS - RAII ownership, `std::expected`, no raw ownership, unified `CompileError`.
- IV. TDD Red-Green + Test Pyramid: PASS - planned constexpr/debug/runtime tests with edge/corner coverage.
- V. Dependency Management: PASS - exclusive use of approved, version-pinned dependencies.
- VI. Documentation Standards: PASS - structured plan/research/design artifacts with explicit glossary deliverable.
- VII. Algorithmic Design Excellence: PASS - formal choice of iterative RD dataflow + PHI minimization with complexity analysis.
- VIII. STL Algorithm Exclusivity: PASS - STL preference for traversal/filter/transform where semantically equivalent.

Gate result: PASS (no unjustified violations).

## Project Structure

### Documentation (this feature)

```text
specs/009-sistema-ir-multilivello/
├── plan.md              # This file (/speckit.plan command output)
├── research.md          # Phase 0 output (/speckit.plan command)
├── data-model.md        # Phase 1 output (/speckit.plan command)
├── GLOSSARY.md          # Phase 1 output: canonical terminology for this feature
├── quickstart.md        # Phase 1 output (/speckit.plan command)
├── contracts/           # Phase 1 output (/speckit.plan command)
└── tasks.md             # Phase 2 output (/speckit.tasks command - NOT created by /speckit.plan)
```

### Source Code (repository root)

```text
include/
├── jsav/
│   ├── ir/
│   │   ├── Module.hpp
│   │   ├── Function.hpp
│   │   ├── BasicBlock.hpp
│   │   ├── Instruction.hpp
│   │   ├── Value.hpp
│   │   ├── Type.hpp
│   │   └── PhiNode.hpp
│   ├── analysis/
│   │   ├── ReachingDefinitions.hpp
│   │   ├── Dominance.hpp
│   │   ├── Liveness.hpp
│   │   └── Dependence.hpp
│   ├── passes/
│   │   ├── Pass.hpp
│   │   ├── PassResult.hpp
│   │   ├── PassContext.hpp
│   │   ├── PassTransaction.hpp
│   │   ├── PassPipeline.hpp
│   │   ├── SsaConstructionPass.hpp
│   │   ├── PhiMaintenancePass.hpp
│   │   ├── BlockRewritePass.hpp
│   │   ├── HirToMirLowering.hpp
│   │   └── MirToLirLowering.hpp
│   └── validation/
│       ├── IrValidator.hpp
│       ├── SsaValidator.hpp
│       ├── PhiValidator.hpp
│       ├── TypeValidator.hpp
│       ├── UseDefValidator.hpp
│       ├── FormalProofChecker.hpp
│       └── MemoryValidator.hpp

src/
├── jsav_Lib/
│   ├── ir/
│   │   ├── IrCommon.cpp
│   │   ├── Module.cpp
│   │   ├── Function.cpp
│   │   ├── BasicBlock.cpp
│   │   ├── Instruction.cpp
│   │   ├── Value.cpp
│   │   ├── Type.cpp
│   │   └── PhiNode.cpp
│   ├── analysis/
│   │   ├── CanonicalOrder.cpp
│   │   ├── ReachingDefinitions.cpp
│   │   ├── Dominance.cpp
│   │   ├── Liveness.cpp
│   │   ├── Dependence.cpp
│   │   ├── Alias.cpp
│   │   ├── InternalProofGenerator.cpp
│   │   └── DerivationMap.cpp
│   ├── passes/
│   │   ├── PassResult.cpp
│   │   ├── PassContext.cpp
│   │   ├── PassTransaction.cpp
│   │   ├── HirToMirLowering.cpp
│   │   ├── SsaConstructionPass.cpp
│   │   ├── PhiMaintenancePass.cpp
│   │   ├── BlockRewritePass.cpp
│   │   ├── MirToLirLowering.cpp
│   │   └── PassPipeline.cpp
│   └── validation/
│       ├── IrValidator.cpp
│       ├── SsaValidator.cpp
│       ├── PhiValidator.cpp
│       ├── TypeValidator.cpp
│       ├── UseDefValidator.cpp
│       ├── FormalProofChecker.cpp
│       └── MemoryValidator.cpp
├── jsav_Core_lib/
└── jsav/

test/
├── constexpr_tests.cpp
└── tests.cpp
```

**Structure Decision**: Single compiler project (library + CLI) with internal modular separation for IR/analysis/passes/validation. No decomposition into distributed services.

## Complexity Tracking

> **Fill ONLY if Constitution Check has violations that must be justified**

| Violation | Why Needed | Simpler Alternative Rejected Because |
|-----------|------------|-------------------------------------|
| Principle V (Dependency Management) | FR-023 requires SHA-256 for nominal type versioning to ensure 2^128+ collision resistance and deterministic structural hashing. | Adding an external crypto library is rejected to keep the dependency surface minimal per Principle V. A custom implementation in `jsavCore` is selected, with a mandatory security and performance review (Task T019a) to mitigate Principle III risks. |
| Principle VIII (STL Algorithm Exclusivity) | FR-025 and SC-012 require supporting up to 100k instructions per function. Reaching Definitions analysis using `std::vector<bool>` or `std::set` exhibits O(N^2) memory or cache-unfriendly access patterns at this scale. A custom Sparse Bitset is required for bit-parallel dataflow. | `std::vector<bool>` lacks bitwise operations across containers and has high memory overhead for sparse sets. `std::dynamic_bitset` (Boost) is rejected to avoid new dependencies (Principle V). A custom implementation enables O(1) bit-parallel operations and cache-efficient sparse representation. |

## Phase 0 Research Plan

Objective: consolidate final technical decisions with no remaining `NEEDS CLARIFICATION` items, with particular focus on the RD-based SSA/PHI algorithm, deterministic batch error model, and formal proof criteria for no-reorder exceptions.

Expected output: `research.md`.

## Phase 1 Design Plan

Objective: formalize the data model, contracts, and executable quickstart aligned with project constraints.

Expected outputs:

- `data-model.md`
- `GLOSSARY.md`
- `contracts/ir-pass-contract.md`
- `contracts/cli-contract.md`
- `quickstart.md`

## Post-Design Constitution Re-Check

- I. Platform Independence: PASS
- II. Visual Studio 2026 Compatibility: PASS
- III. C++ Core Guidelines Compliance: PASS
- IV. TDD Red-Green + Test Pyramid: PASS
- V. Dependency Management: PASS
- VI. Documentation Standards: PASS
- VII. Algorithmic Design Excellence: PASS
- VIII. STL Algorithm Exclusivity: PASS

Final result: PASS (no unjustified violations after design).
