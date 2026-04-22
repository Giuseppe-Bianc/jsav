# Implementation Plan: Sistema IR Multi-Livello Verificabile

**Branch**: `009-sistema-ir-multilivello` | **Date**: 2026-04-21 | **Spec**: `specs/009-sistema-ir-multilivello/spec.md`
**Input**: Feature specification from `/specs/009-sistema-ir-multilivello/spec.md`

**Note**: This template is filled in by the `/speckit.plan` command. See `.specify/templates/plan-template.md` for the execution workflow.

## Summary

Implementare un sistema IR multi-livello (HIR/MIR/LIR) in C++23 con MSVC 2026, senza introdurre nuove dipendenze, basato su monolite modulare con meccanismo di **PassTransaction** (working copy + commit atomico post-validazione). La correttezza MIR SSA e PHI e garantita da criterio canonico reaching-definitions con minimizzazione eager. Gli errori sono modellati in modo uniforme con `CompileError` e `std::expected<T, std::vector<CompileError>>` per validazioni batch deterministiche. La tracciabilita tra livelli usa ID globali immutabili e deterministici derivati da percorsi strutturali canonici. Il piano di test adotta piramide rigorosa: compile-time (`test/constexpr_tests.cpp`), debug constexpr (`test/constexpr_tests.cpp`), runtime (`test/tests.cpp`) con copertura edge/corner case su CFG non riducibili, versionamento nominale tipi e strict no-reorder su may-alias.

## Technical Context

<!--
  ACTION REQUIRED: Replace the content in this section with the technical details
  for the project. The structure here is presented in advisory capacity to guide
  the iteration process.
-->

**Language/Version**: C++23 (MSVC Visual Studio 2026 toolset; GCC13+/Clang16+ compatibility target)  
**Primary Dependencies**: fmtlib 12.1.0, spdlog 1.17.0, CLI11 2.6.1, Catch2 3.14.0 (gia approvate)  
**Storage**: In-memory graph model (HIR/MIR/LIR + analysis artifacts) con persistenza solo tramite file input/output gia esistenti  
**Testing**: Catch2 3.14.0; `STATIC_REQUIRE` compile-time, runtime debug constexpr, runtime functional + integration  
**Target Platform**: Windows 11 (MSVC 2026) come baseline; Linux/macOS come target portabile
**Project Type**: Compiler (library + CLI) in monolite modulare  
**Performance Goals**: Supportare fino a 100k istruzioni/funzione e 2M/modulo con output deterministico bit-identico e validazione completa per pass  
**Constraints**: Nessuna nuova dipendenza; errore unificato via `CompileError`; pass single-thread deterministici; strict no-reorder per may-alias salvo prova formale; zero warning build  
**Scale/Scope**: Feature IR core end-to-end (modello, validazione, analisi, trasformazioni, test, CI gates)

### Technology Stack

- Core language/toolchain: C++23 + CMake 4.2+ + Ninja + MSVC 2026
- Logging/formatting/CLI/test: `spdlog 1.17.0`, `fmtlib 12.1.0`, `CLI11 2.6.1`, `Catch2 3.14.0`
- Quality tools: `clang-tidy`, `cppcheck`, `AddressSanitizer`, `UndefinedBehaviorSanitizer`, `lizard`, `gcovr`
- Version lock strategy: pin esplicito in `Dependencies.cmake` + `cpm-package-lock.cmake`

### Architecture Pattern

Monolite modulare semantico (non microservizi), con moduli isolati per: modello IR, validazione, analisi dataflow/CFG, trasformazioni/lowering, orchestrazione pass. Scelta motivata da dominio compiler, team e codice coeso: riduce overhead operativo e massimizza consistenza transazionale e determinismo. Trigger di revisione architetturale: superamento stabile della soglia di complessita organizzativa (team >15) o superamento sistematico delle soglie di complessità per funzione (CCN >15) o coupling non gestibile tra sottosistemi principali con evidenza che un boundary distribuito riduce costo totale.

### Libraries & Dependencies

- Core:

  - `fmtlib 12.1.0`: formatting robusto e portabile; alternativa `std::format` non pienamente uniforme su toolchain target.
  - `spdlog 1.17.0`: logging strutturato multi-sink per audit pass/analisi; alternativa custom logger scartata per costo manutenzione.
  - `CLI11 2.6.1`: parsing CLI stabile per pipeline/pass; alternativa parser manuale scartata per robustezza inferiore su edge input.
- Dev/Test:
  - `Catch2 3.14.0`: framework unificato per compile-time/runtime test pyramid; alternativa framework multipli scartata per complessita.
- Optional: nessuna (vincolo progetto: no dipendenze esterne aggiuntive).

### Data Management

- Modello dati primario: grafo IR tipizzato in memoria (Modulo -> Funzione -> Blocco -> Istruzione -> Valore/Tipo).
- Schema approach: schema esplicito C++ con invarianti forti per ciascun livello IR e relazioni di derivazione HIR->MIR->LIR.
- Data flow strategy:
  - reaching definitions con sparse bitset iterativo per PHI placement canonico;
  - dominanza/liveness/dependence allineate alla chiave canonica stabile;
  - output ordinati deterministicamente (modulo/funzione/blocco/idx-istruzione/idx-operando).

### State Management

- Stato runtime IR gestito per working copy transazionale a granularita funzione/pass.
- Commit atomico solo dopo validazione completa; rollback totale su qualunque failure.
- Stato errori centralizzato in `std::vector<CompileError>` aggregato per pass (batch-per-pass).

### Deployment Strategy

- Hosting/Execution: progetto nativo C++ distribuito come binario cross-platform buildato via CMake presets.
- CI/CD: GitHub Actions con pipeline gate `lint -> static-analysis -> build -> test -> sanitizers -> complexity -> coverage`.
- Environment parity: configurazioni interamente via CMake Presets; variabili ambiente per path/flag; nessun segreto nel repository.
- Branch mapping: `main` produzione/rilascio, branch feature per sviluppo, PR gating obbligatorio.

### Development Workflow

- Build tools: CMake + Ninja con preset dedicati MSVC/GCC/Clang.
- Testing framework: Catch2 con piramide:
  - compile-time in `test/constexpr_tests.cpp` (`STATIC_REQUIRE`)
  - debug constexpr in `test/constexpr_tests.cpp`
  - runtime completi in `test/tests.cpp`
- Code organization: separazione header/public API in `include/jsav/`, implementazioni in `src/jsav_Lib/` e core condiviso in `include/jsavCore/` + `src/jsav_Core_lib/`.
- Quality gates: zero warnings, clang-tidy/cppcheck clean, ASan/UBSan clean, lizard threshold pass, gcovr >= 95%.

## Constitution Check

*GATE: Must pass before Phase 0 research. Re-check after Phase 1 design.*

### Pre-Design Gate Evaluation

- I. Platform Independence: PASS - design basato su C++23/STL e astrazioni portabili.
- II. Visual Studio 2026 Compatibility: PASS - baseline MSVC 2026 e feature C++23 supportate.
- III. C++ Core Guidelines Compliance: PASS - ownership RAII, `std::expected`, no raw ownership, `CompileError` unificato.
- IV. TDD Red-Green + Test Pyramid: PASS - test constexpr/debug/runtime previsti con edge/corner coverage.
- V. Dependency Management: PASS - uso esclusivo dipendenze approvate, version-pinned.
- VI. Documentation Standards: PASS - artifact di piano/research/design strutturati.
- VII. Algorithmic Design Excellence: PASS - scelta formale di dataflow iterativo RD + minimizzazione PHI con analisi complessita.
- VIII. STL Algorithm Exclusivity: PASS - preferenza STL per traversal/filter/transform dove semanticamente equivalente.

Esito gate: PASS (nessuna violazione non giustificata).

## Project Structure

### Documentation (this feature)

```text
specs/009-sistema-ir-multilivello/
├── plan.md              # This file (/speckit.plan command output)
├── research.md          # Phase 0 output (/speckit.plan command)
├── data-model.md        # Phase 1 output (/speckit.plan command)
├── quickstart.md        # Phase 1 output (/speckit.plan command)
├── contracts/           # Phase 1 output (/speckit.plan command)
└── tasks.md             # Phase 2 output (/speckit.tasks command - NOT created by /speckit.plan)
```

### Source Code (repository root)
<!--
  ACTION REQUIRED: Replace the placeholder tree below with the concrete layout
  for this feature. Delete unused options and expand the chosen structure with
  real paths (e.g., apps/admin, packages/something). The delivered plan must
  not include Option labels.
-->

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
│   │   ├── PassPipeline.hpp
│   │   ├── HirToMirLowering.hpp
│   │   └── MirToLirLowering.hpp
│   └── validation/
│       ├── IrValidator.hpp
│       ├── SsaValidator.hpp
│       ├── PhiValidator.hpp
│       ├── TypeValidator.hpp
│       └── MemoryValidator.hpp

src/
├── jsav_Lib/
│   ├── ir/
│   ├── analysis/
│   ├── passes/
│   └── validation/
├── jsav_Core_lib/
└── jsav/

tests/
├── constexpr_tests.cpp
└── tests.cpp
```

**Structure Decision**: Singolo progetto compiler (library + CLI) con separazione modulare interna per IR/analysis/passes/validation. Nessuna scomposizione in servizi distribuiti.

## Complexity Tracking

> **Fill ONLY if Constitution Check has violations that must be justified**

| Violation | Why Needed | Simpler Alternative Rejected Because |
|-----------|------------|-------------------------------------|
| Nessuna | N/A | N/A |

## Phase 0 Research Plan

Obiettivo: consolidare decisioni tecniche finali senza `NEEDS CLARIFICATION` residue, con particolare focus su algoritmo RD-based SSA/PHI, error model batch deterministico e criteri di prova formale per eccezioni no-reorder.

Output previsto: `research.md`.

## Phase 1 Design Plan

Obiettivo: formalizzare modello dati, contratti e quickstart eseguibile allineato ai vincoli del progetto.

Output previsti:

- `data-model.md`
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

Esito finale: PASS (nessuna violazione non giustificata dopo design).
