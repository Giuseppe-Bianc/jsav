# Tasks: Sistema IR Multi-Livello Verificabile

**Input**: Design documents from `/specs/009-sistema-ir-multilivello/`
**Prerequisites**: plan.md, spec.md, research.md, data-model.md, contracts/, quickstart.md

**Tests**: Inclusi e obbligatori per questa feature (test pyramid: compile-time, relaxed constexpr, runtime).

**Organization**: Tasks raggruppati per user story per consentire implementazione e test indipendenti.

## Format: `[ID] [P?] [Story] Description`

- **[P]**: eseguibile in parallelo (file diversi, nessuna dipendenza incompleta)
- **[Story]**: user story target (US1, US2, US3)
- Ogni task include il file path esatto

## Phase 1: Setup (Shared Infrastructure)

**Purpose**: Allineare struttura, build e quality gate al dominio IR multi-livello.

- [ ] T001 Aggiornare registrazione sorgenti IR/analysis/passes/validation in src/jsav_Lib/CMakeLists.txt
- [ ] T002 Aggiornare install/export header nuovi moduli in CMakeLists.txt
- [ ] T003 Aggiungere directory skeleton e placeholder CMake include in src/jsav_Lib/CMakeLists.txt
- [ ] T004 [P] Aggiungere sezione CI stages (lint->analysis->build->test->sanitizers->complexity->coverage) in .github/workflows/ci.yml
- [ ] T005 [P] Aggiornare soglia copertura >=95% in gcovr.cfg

---

## Phase 2: Foundational (Blocking Prerequisites)

**Purpose**: Contratti e primitive comuni bloccanti per tutte le user story.

**CRITICAL**: Nessuna implementazione US1/US2/US3 prima del completamento di questa fase.

- [ ] T006 Definire tipi base IR level/pass kind e chiave canonica in include/jsav/ir/IrCommon.hpp
- [ ] T007 Implementare risultato errore canonico PassResult con CompileError batch in include/jsav/passes/PassResult.hpp
- [ ] T008 Implementare contratto IPass e PassInvariantReport in include/jsav/passes/Pass.hpp
- [ ] T009 Implementare PassContext deterministico e config canonica in include/jsav/passes/PassContext.hpp
- [ ] T010 Implementare entita ID globali immutabili deterministici in include/jsav/ir/GlobalEntityId.hpp
- [ ] T011 Implementare generatore ID da percorso strutturale canonico in src/jsav_Lib/ir/GlobalEntityId.cpp
- [ ] T012 Implementare transazione working-copy/commit/rollback in include/jsav/passes/PassTransaction.hpp
- [ ] T013 Implementare motore transazionale pass in src/jsav_Lib/passes/PassTransaction.cpp
- [ ] T014 Implementare utility ordinamento canonico report/errori in include/jsav/analysis/CanonicalOrder.hpp
- [ ] T015 Implementare utility ordinamento canonico report/errori in src/jsav_Lib/analysis/CanonicalOrder.cpp
- [ ] T016 Integrare policy errore uniforme (solo CompileError) in include/jsav/error/CompileError.hpp
- [ ] T017 [P] Aggiungere test runtime fondazionale su PassResult/transaction rollback in test/tests.cpp
- [ ] T018 [P] Aggiungere test constexpr fondazionale su chiave canonica/ID deterministici in test/constexpr_tests.cpp
- [ ] T019 [P] Aggiungere test relaxed constexpr fondazionale in test/constexpr_tests.cpp

**Checkpoint**: Foundation pronta - le user story possono iniziare.

---

## Phase 3: User Story 1 - Costruzione e Validazione IR (Priority: P1) MVP

**Goal**: Costruire IR HIR/MIR/LIR validabile con errori batch localizzati e nessuno stato intermedio invalido persistente.

**Independent Test**: Costruire modulo con casi validi/invalidi a ogni livello; la validazione accetta solo i validi e fallisce con CompileError batch deterministico sui non validi.

### Tests for User Story 1

- [ ] T020 [US1] Aggiungere test compile-time su invarianti Value/Type immutabili in test/constexpr_tests.cpp
- [ ] T021 [US1] Aggiungere test compile-time su regole base CFG (entry unico, terminatore) in test/constexpr_tests.cpp
- [ ] T022 [US1] Aggiungere test runtime su validazione CFG invalido (terminatore mancante, archi incoerenti) in test/tests.cpp
- [ ] T023 [US1] Aggiungere test runtime su uso senza definizione raggiungibile in test/tests.cpp
- [ ] T024 [US1] Aggiungere test runtime su reporting batch-per-pass con verifiche skipped annotate in test/tests.cpp
- [ ] T025 [US1] Aggiungere test relaxed constexpr di debug su invarianti compile-time in test/constexpr_tests.cpp

### Implementation for User Story 1

- [ ] T026 [P] [US1] Implementare entita Module in include/jsav/ir/Module.hpp
- [ ] T027 [P] [US1] Implementare entita Function in include/jsav/ir/Function.hpp
- [ ] T028 [P] [US1] Implementare entita BasicBlock in include/jsav/ir/BasicBlock.hpp
- [ ] T029 [P] [US1] Implementare entita Instruction in include/jsav/ir/Instruction.hpp
- [ ] T030 [P] [US1] Implementare entita Value e use-site tracking in include/jsav/ir/Value.hpp
- [ ] T031 [P] [US1] Implementare Type system base + versionamento nominale in include/jsav/ir/Type.hpp
- [ ] T032 [P] [US1] Implementare nodo PHI e incoming map in include/jsav/ir/PhiNode.hpp
- [ ] T033 [US1] Implementare validator CFG in include/jsav/validation/IrValidator.hpp
- [ ] T034 [US1] Implementare validator CFG in src/jsav_Lib/validation/IrValidator.cpp
- [ ] T035 [US1] Implementare validator type compatibility/nominal equivalence in include/jsav/validation/TypeValidator.hpp
- [ ] T036 [US1] Implementare validator type compatibility/nominal equivalence in src/jsav_Lib/validation/TypeValidator.cpp
- [ ] T037 [US1] Implementare validator use-def e dipendenze base in include/jsav/validation/UseDefValidator.hpp
- [ ] T038 [US1] Implementare validator use-def e dipendenze base in src/jsav_Lib/validation/UseDefValidator.cpp
- [ ] T039 [US1] Integrare orchestrazione validazione post-pass con CompileError batch in src/jsav_Lib/passes/PassPipeline.cpp

**Checkpoint**: US1 completa e testabile in modo indipendente.

---

## Phase 4: User Story 2 - Trasformazioni Semantiche HIR/MIR/LIR (Priority: P2)

**Goal**: Eseguire lowering HIR->MIR->LIR preservando semantica osservabile su valori/memoria e regole strict no-reorder.

**Independent Test**: Eseguire pipeline di lowering su input con CFG complesso/memoria; output valido e semanticamente equivalente o failure esplicita con rollback completo.

### Tests for User Story 2

- [ ] T040 [US2] Aggiungere test runtime HIR->MIR semantic equivalence su valori/memoria in test/tests.cpp
- [ ] T041 [US2] Aggiungere test runtime MIR->LIR con salti espliciti e semantica preservata in test/tests.cpp
- [ ] T042 [US2] Aggiungere test runtime strict no-reorder su may-alias senza prova formale in test/tests.cpp
- [ ] T043 [US2] Aggiungere test runtime eccezione no-reorder con prova formale valida in test/tests.cpp
- [ ] T044 [US2] Aggiungere test runtime rollback completo su pass fallito in test/tests.cpp
- [ ] T045 [US2] Aggiungere test corner case CFG non riducibile durante lowering in test/tests.cpp

### Implementation for User Story 2

- [ ] T046 [US2] Implementare reaching definitions dataflow iterativo (sparse bitset) in include/jsav/analysis/ReachingDefinitions.hpp
- [ ] T047 [US2] Implementare reaching definitions dataflow iterativo (sparse bitset) in src/jsav_Lib/analysis/ReachingDefinitions.cpp
- [ ] T048 [US2] Implementare builder SSA con placement PHI canonico RD-based in include/jsav/passes/SsaConstructionPass.hpp
- [ ] T049 [US2] Implementare builder SSA con placement PHI canonico RD-based in src/jsav_Lib/passes/SsaConstructionPass.cpp
- [ ] T050 [US2] Implementare minimizzazione/pruning eager PHI su update CFG in include/jsav/passes/PhiMaintenancePass.hpp
- [ ] T051 [US2] Implementare minimizzazione/pruning eager PHI su update CFG in src/jsav_Lib/passes/PhiMaintenancePass.cpp
- [ ] T052 [US2] Implementare lowering HIR->MIR transazionale in include/jsav/passes/HirToMirLowering.hpp
- [ ] T053 [US2] Implementare lowering HIR->MIR transazionale in src/jsav_Lib/passes/HirToMirLowering.cpp
- [ ] T054 [US2] Implementare lowering MIR->LIR transazionale in include/jsav/passes/MirToLirLowering.hpp
- [ ] T055 [US2] Implementare lowering MIR->LIR transazionale in src/jsav_Lib/passes/MirToLirLowering.cpp
- [ ] T056 [US2] Implementare validator memoria/alias strict no-reorder in include/jsav/validation/MemoryValidator.hpp
- [ ] T057 [US2] Implementare validator memoria/alias strict no-reorder in src/jsav_Lib/validation/MemoryValidator.cpp
- [ ] T058 [US2] Implementare policy rewrite-safe block elimination in include/jsav/passes/BlockRewritePass.hpp
- [ ] T059 [US2] Implementare policy rewrite-safe block elimination in src/jsav_Lib/passes/BlockRewritePass.cpp

**Checkpoint**: US1 e US2 funzionano e sono verificabili indipendentemente.

---

## Phase 5: User Story 3 - Analisi Deterministiche e Tracciabilita (Priority: P3)

**Goal**: Fornire analisi deterministiche (dominanza/RD/liveness/dependence) e tracciabilita completa HIR->MIR->LIR con ID immutabili.

**Independent Test**: Rieseguire analisi e pipeline su stessi input/config; ottenere output bit-identici ordinati con chiave canonica e relazioni di derivazione verificabili.

### Tests for User Story 3

- [ ] T060 [US3] Aggiungere test runtime determinismo dominanza su riesecuzioni identiche in test/tests.cpp
- [ ] T061 [US3] Aggiungere test runtime determinismo reaching definitions/liveness/dependence in test/tests.cpp
- [ ] T062 [US3] Aggiungere test runtime ordinamento totale report/errori con chiave canonica in test/tests.cpp
- [ ] T063 [US3] Aggiungere test runtime tracciabilita derivazione HIR->MIR->LIR in test/tests.cpp
- [ ] T064 [US3] Aggiungere test edge case aggiornamento PHI dopo predecessore non raggiungibile in test/tests.cpp
- [ ] T065 [US3] Aggiungere test corner case ridefinizione tipo utente con mantenimento binding versione precedente in test/tests.cpp

### Implementation for User Story 3

- [ ] T066 [US3] Implementare analisi dominanza in include/jsav/analysis/Dominance.hpp
- [ ] T067 [US3] Implementare analisi dominanza in src/jsav_Lib/analysis/Dominance.cpp
- [ ] T068 [US3] Implementare analisi liveness in include/jsav/analysis/Liveness.hpp
- [ ] T069 [US3] Implementare analisi liveness in src/jsav_Lib/analysis/Liveness.cpp
- [ ] T070 [US3] Implementare analisi dependence in include/jsav/analysis/Dependence.hpp
- [ ] T071 [US3] Implementare analisi dependence in src/jsav_Lib/analysis/Dependence.cpp
- [ ] T072 [US3] Implementare modello relazioni di derivazione tra livelli in include/jsav/ir/DerivationMap.hpp
- [ ] T073 [US3] Implementare modello relazioni di derivazione tra livelli in src/jsav_Lib/ir/DerivationMap.cpp
- [ ] T074 [US3] Integrare output report deterministici machine-readable in src/jsav/main.cpp
- [ ] T075 [US3] Integrare comandi CLI validate/lower/analyze/pipeline in src/jsav/main.cpp

**Checkpoint**: Tutte le user story sono complete e testabili indipendentemente.

---

## Phase 6: Polish & Cross-Cutting Concerns

**Purpose**: Hardening, quality gates finali e validazione quickstart end-to-end.

- [ ] T076 [P] Aggiornare documentazione tecnica IR/SSA/PHI in README.md
- [ ] T077 [P] Aggiornare quickstart con comandi reali di esecuzione pipeline in specs/009-sistema-ir-multilivello/quickstart.md
- [ ] T078 Aggiungere target/verifica lizard nel workflow CI in .github/workflows/ci.yml
- [ ] T079 Aggiungere target/verifica gcovr >=95% nel workflow CI in .github/workflows/ci.yml
- [ ] T080 Eseguire validazione finale quickstart e allineare comandi in specs/009-sistema-ir-multilivello/quickstart.md

---

## Dependencies & Execution Order

### Phase Dependencies

- Phase 1 (Setup): inizio immediato.
- Phase 2 (Foundational): dipende da Phase 1, blocca tutte le user story.
- Phase 3 (US1): dipende da Phase 2.
- Phase 4 (US2): dipende da Phase 2 e dai contratti/error model fondazionali.
- Phase 5 (US3): dipende da Phase 2; integra risultati di US1/US2 ma resta testabile indipendentemente.
- Phase 6 (Polish): dipende da completamento delle story richieste.

### User Story Dependencies

- US1 (P1): nessuna dipendenza su altre story, solo su Foundational.
- US2 (P2): usa invarianti/validatori base di US1 ma puo essere validata con suite indipendente di trasformazione.
- US3 (P3): usa pipeline/IR gia presenti, ma i suoi test di determinismo e tracciabilita sono indipendenti.

### Within Each User Story

- Test prima dell'implementazione (Red -> Green -> Refactor).
- Header/contratti prima delle implementazioni cpp.
- Analisi/validatori prima di integrazione CLI.
- Checkpoint di story prima del passaggio alla successiva.

---

## Parallel Opportunities

- Setup: T004 e T005 in parallelo.
- Foundational: T017, T018, T019 in parallelo dopo T006-T016.
- US1: T022 e T025 in parallelo; T026-T032 in parallelo.
- US2: T052 e T054 possono iniziare in parallelo dopo T046-T051.
- US3: T066, T068, T070 in parallelo.
- Polish: T076 e T077 in parallelo; T078 e T079 in parallelo.

---

## Parallel Example: User Story 1

```text
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
Task: T060 [US3] deterministic dominance test in test/tests.cpp
Task: T061 [US3] deterministic RD/liveness/dependence test in test/tests.cpp
Task: T062 [US3] canonical ordering test in test/tests.cpp
Task: T063 [US3] derivation traceability test in test/tests.cpp

Task: T066 [US3] Dominance.hpp/cpp
Task: T068 [US3] Liveness.hpp/cpp
Task: T070 [US3] Dependence.hpp/cpp
```

---

## Implementation Strategy

### MVP First (US1 only)

1. Completare Phase 1 (Setup).
2. Completare Phase 2 (Foundational).
3. Completare Phase 3 (US1).
4. Validare US1 in isolamento con test constexpr + runtime.
5. Stabilizzare error model `CompileError` e commit atomico come baseline per le fasi successive.

### Incremental Delivery

1. Foundation pronta (Phase 1+2).
2. Delivery US1 (validazione robusta).
3. Delivery US2 (lowering semantico + policy memoria).
4. Delivery US3 (analisi deterministiche + tracciabilita).
5. Polish finale e quality gate CI completi.

### Parallel Team Strategy

1. Team allinea foundation insieme.
2. Dopo foundation:
   - Dev A su validatori/US1.
   - Dev B su lowering/US2.
   - Dev C su analisi/tracciabilita/US3.
3. Integrazione continua su pipeline con test gate obbligatori.

---

## Notes

- Tutti i task mantengono il vincolo: nessuna nuova dipendenza esterna.
- `CompileError` e `std::expected<T, std::vector<CompileError>>` sono obbligatori in ogni componente.
- Edge case e corner case sono esplicitamente coperti nei task test.
- I task con `[P]` sono paralleli solo se non introducono conflitti sugli stessi file.
