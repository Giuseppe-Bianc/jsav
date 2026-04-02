# Implementation Plan: Hindley-Milner Type Checker with Constraint Solver

**Branch**: `008-type-checker-constraint-solver` | **Date**: 2026-04-02 | **Spec**: `specs/008-type-checker-constraint-solver/spec.md`
**Input**: Feature specification from `specs/008-type-checker-constraint-solver/spec.md`

## Summary

Implementare un type checker a vincoli per jsav che trasforma il Raw AST (`Program`) in Fully-Typed AST (`TypedProgram`) e raccoglie tutti gli errori di tipo in un singolo pass logico. L'approccio usa inferenza Hindley-Milner, generazione vincoli, unificazione con union-find, zonking finale e propagazione di `ErrorType` per evitare fail-fast.

## Technical Context

**Language/Version**: C++23, limitato a feature supportate da Visual Studio 2026/MSVC  
**Primary Dependencies**: Solo dipendenze già approvate nel repository (fmt/spdlog/CLI11/Catch2); nessuna nuova dipendenza  
**Storage**: N/A (in-memory only per il solver e l'AST typing)  
**Testing**: Catch2; test runtime in `test/tests.cpp`; test constexpr dove applicabile nel percorso test esistente  
**Target Platform**: Windows (MSVC VS2026) con mantenimento portabilità cross-platform già prevista dal progetto  
**Project Type**: Compiler component/library (semantic/type-checking phase)  
**Performance Goals**: End-to-end: 10k+ nodi AST in < 5s su CI runner; target memoria soft: 100k constraint < 50MB  
**Constraints**: No implicit promotions; exact type matching per regole spec; single-threaded non-thread-safe API; nessuna metrica strutturata oltre logging  
**Scale/Scope**: Copertura completa di tutti i NodeKind espressione/statement esistenti nel raw AST (`include/jsav/ast`) e produzione typed AST completo

## Constitution Check

*GATE: Must pass before Phase 0 research. Re-check after Phase 1 design.*

### Pre-Design Gate Evaluation

- Principle I (Platform Independence): **Compliant**. Il piano usa solo C++23 stdlib + dipendenze già approvate e resta OS-independent.
- Principle II (Visual Studio 2026 Compatibility): **Compliant**. Scelte tecniche vincolate a feature C++23 supportate da MSVC VS2026.
- Principle III (C++ Core Guidelines Compliance): **Compliant**. Ownership chiara, nessun raw owning pointer, error handling strutturato e enforcement via test/tooling.
- Principle IV (TDD Red-Green): **Compliant**. Strategia test-first definita, con test runtime in `test/tests.cpp` e flusso incrementale.
- Principle V (Dependency Management): **Compliant**. Nessuna nuova dipendenza.

Gate Result: **PASS** (nessuna violazione non giustificata).

### Post-Design Gate Re-Evaluation

- Principle I (Platform Independence): **Compliant**. Data model e contratti non introducono API platform-specific.
- Principle II (Visual Studio 2026 Compatibility): **Compliant**. Contratti e quickstart impongono validazione build/test su MSVC.
- Principle III (C++ Core Guidelines Compliance): **Compliant**. Entita progettate con ownership esplicita, error accumulation senza eccezioni fail-fast.
- Principle IV (TDD Red-Green): **Compliant**. Quickstart definisce ordine red-green-refactor con focus sui test runtime richiesti.
- Principle V (Dependency Management): **Compliant**. Artefatti non richiedono librerie extra.

Gate Result: **PASS** (nessuna violazione non giustificata).

## Project Structure

### Documentation (this feature)

```text
specs/008-type-checker-constraint-solver/
├── plan.md
├── research.md
├── data-model.md
├── quickstart.md
├── contracts/
|   └── type-checker-api.md
└── tasks.md
```

### Source Code (repository root)

```text
include/
└── jsav/
    ├── ast/
    |   ├── Program.hpp
    |   ├── Expressions.hpp
    |   ├── Statements.hpp
    |   ├── Type.hpp
    |   ├── TypedNode.hpp
    |   └── TypedProgram.hpp
    └── semantic/

src/
├── jsav_Lib/
|   └── semantic/
└── jsav/

test/
├── tests.cpp
└── constexpr_tests.cpp
```

**Structure Decision**: Feature implementata nel modulo compiler/library esistente, integrando il Raw AST reale in `include/jsav/ast` e mantenendo i test runtime centralizzati in `test/tests.cpp`.

## Phase 0: Research Plan

- Consolidare decisioni su inferenza HM, union-find, strategia error recovery (`ErrorType`), zonking e determinismo output errori.
- Validare best practice con vincoli del progetto: MSVC 2026, no dependency addition, memory-only solver state.
- Output: `research.md` completo con decisioni actionable.

## Phase 1: Design Plan

- Definire data model (vincoli, type variable, substitution, error record, environment/symbol table).
- Definire contratto interfaccia type checker (`Program` -> `TypedProgram + errors`) per uso interno tra fasi del compilatore.
- Definire quickstart implementativo con sequenza test-first e target file reali.
- Output: `data-model.md`, `contracts/type-checker-api.md`, `quickstart.md`.

## Phase 2: Task Planning Handoff

- `speckit.tasks` dovra produrre task dipendenti in ordine: infrastruttura type variable/constraint -> constraint generation per NodeKind -> solver/zonking -> error reporting -> integrazione -> test.

## Complexity Tracking

Nessuna violazione costituzionale da giustificare in fase di planning.
