# Tasks: Verifiable Multi-Level IR System

**Input**: Design documents from `/specs/009-sistema-ir-multilivello/`
**Prerequisites**: plan.md, spec.md, research.md, data-model.md, contracts/, quickstart.md

**Tests**: Included and mandatory for this feature (test pyramid: compile-time, relaxed constexpr, runtime).

**Organization**: Tasks are grouped by user story to allow independent implementation and testing.

## Format: `[ID] [P?] [Story] Description`

- **[P]**: executable in parallel (different files, no incomplete dependency)
- **[Story]**: user story target (US1, US2, US3)
- Each task includes the exact file path

## Phase 1: Setup (Shared Infrastructure)

**Purpose**: Align structure, build, and quality gates to the multi-level IR domain.

- [X] T001 Update IR/analysis/passes/validation source registration in src/jsav_Lib/CMakeLists.txt
- [X] T002 Update install/export of new module headers in CMakeLists.txt
- [X] T003 Add skeleton directories and placeholder CMake include entries in src/jsav_Lib/CMakeLists.txt
- [X] T004 [P] Add CI stages section (lint->analysis->build->test->sanitizers->complexity->coverage) in .github/workflows/ci.yml
- [X] T005 [P] Update coverage threshold to >=95% in gcovr.cfg
- [X] T005a [P] Create initial canonical terminology glossary in specs/009-sistema-ir-multilivello/GLOSSARY.md and reference it from feature docs
- US3: T075 and T077 in parallel (Alias analysis header/cpp and DerivationMap header).
- Polish: T081 and T082 in parallel (documentation updates).

---

## Phase 2: Foundational (Blocking Prerequisites)

**Purpose**: Shared contracts and foundational primitives blocking all user stories.

**CRITICAL**: No US1/US2/US3 implementation before this phase is complete.

**Execution rule (Red-Green)**: complete foundational tests T006-T008 before implementation tasks T009-T019b.

### Tests First (Red)

- [X] T006 [P] Add foundational runtime tests in test/tests.cpp to verify: (1) commit() success on valid IR, (2) full IR state rollback() on pass failure, (3) persistence of aggregated errors in PassResult.
- [X] T007 [P] Add foundational relaxed constexpr tests in test/constexpr_tests.cpp
- [X] T008 [P] Add foundational constexpr tests on canonical key/deterministic IDs in test/constexpr_tests.cpp

### Implementation (Green/Refactor)

- [X] T009 Define base IR level/pass kind types and canonical key in include/jsav/ir/IrCommon.hpp
- [X] T010 Implement canonical PassResult error type with CompileError batch in include/jsav/passes/PassResult.hpp
- [X] T011 Implement IPass contract and PassInvariantReport in include/jsav/passes/Pass.hpp
- [X] T012 Implement deterministic PassContext and canonical config in include/jsav/passes/PassContext.hpp
- [X] T013 Implement deterministic immutable global ID entities in include/jsav/ir/GlobalEntityId.hpp
- [X] T014 Implement ID generator from canonical structural path in src/jsav_Lib/ir/GlobalEntityId.cpp
- [X] T015 Implement **PassTransaction** mechanism (working-copy/commit/rollback) in include/jsav/passes/PassTransaction.hpp
- [X] T016 Implement **PassTransaction** engine in src/jsav_Lib/passes/PassTransaction.cpp
- [X] T017 Implement canonical ordering utilities for reports/errors in include/jsav/analysis/CanonicalOrder.hpp
- [X] T018 Implement canonical ordering utilities for reports/errors in src/jsav_Lib/analysis/CanonicalOrder.cpp
- [X] T019 Integrate unified error policy (CompileError only) including hierarchical canonical key support in include/jsav/error/CompileError.hpp
- [X] T019a [P] Security and Performance Review of Custom SHA-256 Implementation (aligned with Principle III and VIII)
- [X] T019b [P] Implement custom SHA-256 utility for nominal versioning in include/jsavCore/util/Sha256.hpp and src/jsav_Core_lib/util/Sha256.cpp

**Checkpoint**: Foundation ready - user stories can begin.

---

## Phase 3: User Story 1 - IR Construction and Validation (Priority: P1) MVP

**Goal**: Build validatable HIR/MIR/LIR IR with localized batch errors and no persistent invalid intermediate states.

**Independent Test**: Build a module with valid/invalid cases at each level; validation accepts only valid ones and fails with deterministic CompileError batch on invalid ones.

### Tests for User Story 1

- [X] T020 [US1] Add relaxed constexpr debug tests for compile-time invariants in test/constexpr_tests.cpp
- [X] T021 [US1] Add compile-time tests for immutable Value/Type invariants in test/constexpr_tests.cpp
- [X] T022 [US1] Add compile-time tests for base CFG rules (single entry, terminator) in test/constexpr_tests.cpp
- [X] T023 [US1] Add runtime test for invalid CFG validation (missing terminator, inconsistent edges) in test/tests.cpp
- [X] T024 [US1] Add runtime test for use without reachable definition in test/tests.cpp
- [X] T025 [US1] Add runtime test for batch-per-pass reporting with annotated skipped checks in test/tests.cpp

### Implementation for User Story 1

- [X] T026 [P] [US1] Implement Module entity in include/jsav/ir/Module.hpp
- [X] T027 [P] [US1] Implement Function entity in include/jsav/ir/Function.hpp
- [X] T028 [P] [US1] Implement BasicBlock entity in include/jsav/ir/BasicBlock.hpp
- [X] T029 [P] [US1] Implement Instruction entity in include/jsav/ir/Instruction.hpp
- [X] T030 [P] [US1] Implement Value entity and use-site tracking in include/jsav/ir/Value.hpp
- [X] T031 [P] [US1] Implement base Type system + nominal versioning with SHA-256 hashing and canonical binary serialization in include/jsav/ir/Type.hpp
- [X] T032 [P] [US1] Implement PHI node and incoming map in include/jsav/ir/PhiNode.hpp
- [X] T032a [US1] Implement Module entity in src/jsav_Lib/ir/Module.cpp
- [X] T032b [US1] Implement Function entity in src/jsav_Lib/ir/Function.cpp
- [X] T032c [US1] Implement BasicBlock entity in src/jsav_Lib/ir/BasicBlock.cpp
- [X] T032d [US1] Implement Instruction entity in src/jsav_Lib/ir/Instruction.cpp
- [X] T032e [US1] Implement Value entity and use-site tracking in src/jsav_Lib/ir/Value.cpp
- [X] T032f [US1] Implement base Type system in src/jsav_Lib/ir/Type.cpp
- [X] T032g [US1] Implement PHI node in src/jsav_Lib/ir/PhiNode.cpp
- [X] T033 [US1] Implement CFG validator in include/jsav/validation/IrValidator.hpp
- [X] T034 [US1] Implement CFG validator in src/jsav_Lib/validation/IrValidator.cpp
- [X] T035 [US1] Implement type compatibility/nominal equivalence validator in include/jsav/validation/TypeValidator.hpp
- [X] T036 [US1] Implement type compatibility/nominal equivalence validator with SHA-256 hashing and canonical binary serialization in src/jsav_Lib/validation/TypeValidator.cpp
- [X] T037 [US1] use-def and base dependency validator in include/jsav/validation/UseDefValidator.hpp
- [X] T038 [US1] use-def and base dependency validator in src/jsav_Lib/validation/UseDefValidator.cpp
- [X] T038a [US1] SSA validator (single definition, dominance) in include/jsav/validation/SsaValidator.hpp
- [X] T038b [US1] SSA validator (single definition, dominance) in src/jsav_Lib/validation/SsaValidator.cpp
- [X] T038c [US1] PHI validator (one operand per predecessor) in include/jsav/validation/PhiValidator.hpp
- [X] T038d [US1] PHI validator (one operand per predecessor) in src/jsav_Lib/validation/PhiValidator.cpp
- [X] T039 [US1] Integrate post-pass validation orchestration with CompileError batch in src/jsav_Lib/passes/PassPipeline.cpp

**Checkpoint**: US1 complete and independently testable.

---

## Phase 4: User Story 2 - Semantic HIR/MIR/LIR Transformations (Priority: P2)

**Goal**: Execute HIR->MIR->LIR lowering while preserving observable semantics on values/memory and strict no-reorder rules.

**Independent Test**: Execute lowering pipeline on inputs with complex CFG/memory behavior; output is valid and semantically equivalent or explicit failure with full rollback.

### Tests for User Story 2

- [ ] T040 [US2] Add runtime test for HIR->MIR semantic equivalence on values/memory in test/tests.cpp
- [ ] T041 [US2] Add runtime test for MIR->LIR with explicit jumps and preserved semantics in test/tests.cpp
- [ ] T042 [US2] Add runtime test for strict no-reorder on may-alias without formal proof in test/tests.cpp
- [ ] T043 [US2] Add runtime test for no-reorder exception with valid formal proof in test/tests.cpp
- [ ] T044 [US2] Add runtime test for full rollback on failed pass in test/tests.cpp
- [ ] T045 [US2] Add corner-case test for non-reducible CFG during lowering in test/tests.cpp

### Implementation for User Story 2

- [ ] T046 [US2] Implement iterative reaching-definitions dataflow (sparse bitset) per Principle VIII in include/jsav/analysis/ReachingDefinitions.hpp
- [ ] T047 [US2] Implement iterative reaching-definitions dataflow (sparse bitset) per Principle VIII in src/jsav_Lib/analysis/ReachingDefinitions.cpp
- [ ] T048 [US2] Implement SSA builder with canonical RD-based PHI placement in include/jsav/passes/SsaConstructionPass.hpp
- [ ] T049 [US2] Implement SSA builder with canonical RD-based PHI placement in src/jsav_Lib/passes/SsaConstructionPass.cpp
- [ ] T050 [US2] Implement eager PHI minimization/pruning on CFG updates in include/jsav/passes/PhiMaintenancePass.hpp
- [ ] T051 [US2] Implement eager PHI minimization/pruning on CFG updates in src/jsav_Lib/passes/PhiMaintenancePass.cpp
- [ ] T052 [US2] Implement transactional HIR->MIR lowering in include/jsav/passes/HirToMirLowering.hpp
- [ ] T053 [US2] Implement transactional HIR->MIR lowering in src/jsav_Lib/passes/HirToMirLowering.cpp
- [ ] T054 [US2] Implement transactional MIR->LIR lowering in include/jsav/passes/MirToLirLowering.hpp
- [ ] T055 [US2] Implement transactional MIR->LIR lowering in src/jsav_Lib/passes/MirToLirLowering.cpp
- [ ] T056 [US2] Implement strict no-reorder memory/alias validator in include/jsav/validation/MemoryValidator.hpp
- [ ] T057 [US2] Implement strict no-reorder memory/alias validator in src/jsav_Lib/validation/MemoryValidator.cpp
- [ ] T058 [US2] Implement rewrite-safe block elimination policy in include/jsav/passes/BlockRewritePass.hpp
- [ ] T059 [US2] Implement rewrite-safe block elimination policy in src/jsav_Lib/passes/BlockRewritePass.cpp
- [ ] T060 [US2] Implement ProofWitness structure for formal independence proofs in include/jsav/analysis/ProofWitness.hpp
- [ ] T061 [US2] Implement FormalProofChecker for independence certificate validation in include/jsav/validation/FormalProofChecker.hpp
- [ ] T061a [US2] Implement InternalProofGenerator for independence proof generation (FR-030b) in src/jsav_Lib/analysis/InternalProofGenerator.cpp

**Checkpoint**: US1 and US2 work and are independently verifiable.

---

## Phase 5: User Story 3 - Deterministic Analyses and Traceability (Priority: P3)

**Goal**: Provide deterministic analyses (dominance/RD/liveness/dependence) and full HIR->MIR->LIR traceability with immutable IDs.

**Independent Test**: Rerun analyses and pipeline on same input/config; obtain bit-identical outputs ordered by canonical key with verifiable derivation relations.

### Tests for User Story 3

- [ ] T062 [US3] Add runtime test for dominance determinism on identical reruns in test/tests.cpp
- [ ] T063 [US3] Add runtime test for reaching-definitions/liveness/dependence determinism in test/tests.cpp
- [ ] T064 [US3] Add runtime test for total report/error ordering with canonical key in test/tests.cpp
- [ ] T065 [US3] Add runtime test for HIR->MIR->LIR derivation traceability in test/tests.cpp
- [ ] T066 [US3] Add edge-case test for PHI updates after unreachable predecessor in test/tests.cpp
- [ ] T067 [US3] Add corner-case test for user type redefinition with previous version binding preserved in test/tests.cpp
- [ ] T068 [US3] Add runtime test for alias-analysis determinism in test/tests.cpp
- [ ] T068a [US3] Add runtime test to verify bit-identical IR output and report determinism (SC-010, SC-016) in test/tests.cpp

### Implementation for User Story 3

- [ ] T069 [US3] Implement dominance analysis per Principle VIII in include/jsav/analysis/Dominance.hpp
- [ ] T070 [US3] Implement dominance analysis per Principle VIII in src/jsav_Lib/analysis/Dominance.cpp
- [ ] T071 [US3] Implement liveness analysis in include/jsav/analysis/Liveness.hpp
- [ ] T072 [US3] Implement liveness analysis in src/jsav_Lib/analysis/Liveness.cpp
- [ ] T073 [US3] Implement dependence analysis in include/jsav/analysis/Dependence.hpp
- [ ] T074 [US3] Implement dependence analysis in src/jsav_Lib/analysis/Dependence.cpp
- [ ] T075 [US3] Implement alias analysis in include/jsav/analysis/Alias.hpp
- [ ] T076 [US3] Implement alias analysis in src/jsav_Lib/analysis/Alias.cpp
- [ ] T077 [US3] Implement derivation-relation model across levels in include/jsav/ir/DerivationMap.hpp
- [ ] T078 [US3] Implement derivation-relation model across levels in src/jsav_Lib/ir/DerivationMap.cpp

### Integration (Final Step of Phase 5)

- [ ] T079 [US3] Integrate deterministic machine-readable report output in src/jsav/main.cpp
- [ ] T080 [US3] Integrate CLI commands validate/lower/analyze/pipeline in src/jsav/main.cpp

**Checkpoint**: All user stories are complete and independently testable.

---

## Phase 6: Polish & Cross-Cutting Concerns

**Purpose**: Hardening, final quality gates, and end-to-end quickstart validation.

- [ ] T081 [P] Update technical IR/SSA/PHI documentation in README.md
- [ ] T082 [P] Update quickstart with real pipeline execution commands in specs/009-sistema-ir-multilivello/quickstart.md
- [ ] T082a [P] Update and maintain canonical terminology glossary in specs/009-sistema-ir-multilivello/GLOSSARY.md
- [ ] T083 Add lizard target/check in CI workflow in .github/workflows/ci.yml
- [ ] T084 Add gcovr >=95% target/check in CI workflow in .github/workflows/ci.yml
- [ ] T085 Execute final quickstart validation and align commands in specs/009-sistema-ir-multilivello/quickstart.md
- [ ] T086 Define target-scale benchmark suite (100k instructions/function, 2M/module) in test/benchmarks.cpp
- [ ] T087 Integrate scale benchmarks in CI: validate that validation and analysis on 100k instructions/function complete within [2.0s] and with memory peak < [1GB] (aligned with FR-025 and SC-012).
- [ ] T088 [P] Verify compliance with Principle VIII (STL Algorithm Exclusivity) for all algorithm/data-structure implementations (e.g., Reaching Definitions, Dominance).

---

## Dependencies & Execution Order

### Phase Dependencies

- Phase 1 (Setup): immediate start.
- Phase 2 (Foundational): depends on Phase 1, blocks all user stories.
- Phase 3 (US1): depends on Phase 2.
- Phase 4 (US2): depends on Phase 2 and foundational contracts/error model.
- Phase 5 (US3): depends on Phase 2; integrates US1/US2 outcomes but remains independently testable.
- Phase 6 (Polish): depends on completion of required stories.

### User Story Dependencies

- US1 (P1): no dependency on other stories, only on Foundational.
- US2 (P2): uses base invariants/validators from US1 but can be validated with an independent transformation suite.
- US3 (P3): uses existing pipeline/IR, but its determinism and traceability tests are independent.

### Within Each User Story

- Tests before implementation (Red -> Green -> Refactor).
- Headers/contracts before cpp implementations.
- Analyses/validators before CLI integration.
- Story checkpoint before moving to the next one.

---

## Parallel Opportunities

- Setup: T004 and T005 in parallel.
- Foundational: run test flow T006 -> T007 -> T008 first, then execute implementation tasks T009-T019b.
- US1: test flow T025 -> (T020, T021) -> (T022, T023, T024); T026-T032 in parallel.
- US2: T052 and T054 can start in parallel after T046-T051.
- US3: T066, T068, T070 in parallel.
- Polish: T076 and T077 in parallel; T078 and T079 in parallel.
- Scale benchmark: T086 before T087.

---

## Parallel Example: User Story 1

```text
Task: T025 [US1] test relaxed constexpr invarianti compile-time in test/constexpr_tests.cpp
Task: T020 [US1] test compile-time invarianti Value/Type in test/constexpr_tests.cpp
Task: T021 [US1] test compile-time regole CFG in test/constexpr_tests.cpp
Task: T022 [US1] test runtime CFG invalido in test/tests.cpp
Task: T023 [US1] test runtime use-without-def in test/tests.cpp

Task: T026 [US1] Module.hpp
Task: T027 [US1] Function.hpp
Task: T028 [US1] BasicBlock.hpp
Task: T029 [US1] Instruction.hpp
Task: T030 [US1] Value.hpp
Task: T031 [US1] Type.hpp
Task: T032 [US1] PhiNode.hpp
```

## Parallel Example: User Story 2

```text
Task: T040 [US2] semantic equivalence HIR->MIR in test/tests.cpp
Task: T041 [US2] semantic equivalence MIR->LIR in test/tests.cpp
Task: T042 [US2] strict no-reorder without proof in test/tests.cpp
Task: T043 [US2] strict no-reorder with formal proof in test/tests.cpp

Task: T052 [US2] HirToMirLowering.hpp/cpp
Task: T054 [US2] MirToLirLowering.hpp/cpp
```

## Parallel Example: User Story 3

```text
Task: T062 [US3] deterministic dominance test in test/tests.cpp
Task: T063 [US3] deterministic RD/liveness/dependence test in test/tests.cpp
Task: T064 [US3] canonical ordering test in test/tests.cpp
Task: T065 [US3] derivation traceability test in test/tests.cpp

Task: T069-T070 [US3] Dominance.hpp/cpp
Task: T071-T072 [US3] Liveness.hpp/cpp
Task: T073-T074 [US3] Dependence.hpp/cpp
```

---

## Implementation Strategy

### MVP First (US1 only)

1. Complete Phase 1 (Setup).
2. Complete Phase 2 (Foundational).
3. Complete Phase 3 (US1).
4. Validate US1 in isolation with constexpr + runtime tests.
5. Stabilize `CompileError` error model and atomic commit as baseline for subsequent phases.

### Incremental Delivery

1. Foundation ready (Phase 1+2).
2. Deliver US1 (robust validation).
3. Deliver US2 (semantic lowering + memory policy).
4. Deliver US3 (deterministic analyses + traceability).
5. Final polish and complete CI quality gates.

### Parallel Team Strategy

1. Team aligns on foundation together.
2. After foundation:
   - Dev A on validators/US1.
   - Dev B on lowering/US2.
   - Dev C on analyses/traceability/US3.
3. Continuous integration on pipeline with mandatory test gates.

---

## Notes

- All tasks keep the constraint: no new external dependencies.
- `CompileError` and `std::expected<T, std::vector<CompileError>>` are mandatory in every component.
- Edge cases and corner cases are explicitly covered in test tasks.
- Tasks marked with `[P]` are parallel only if they do not introduce conflicts on the same files.- All tasks keep the constraint: no new external dependencies.
- `CompileError` and `std::expected<T, std::vector<CompileError>>` are mandatory in every component.
- Edge cases and corner cases are explicitly covered in test tasks.
- Tasks marked with `[P]` are parallel only if they do not introduce conflicts on the same files.
