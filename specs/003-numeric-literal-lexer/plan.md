# Implementation Plan: Riconoscimento Completo dei Literal Numerici nel Lexer

**Branch**: `003-numeric-literal-lexer` | **Date**: 2026-03-03 | **Spec**: [spec.md](spec.md)
**Input**: Feature specification from `/specs/003-numeric-literal-lexer/spec.md`

## Summary

Riscrivere `scan_numeric_literal()` nella classe `Lexer` esistente per implementare il pattern
completo G1→G2→G3 (parte numerica → esponente opzionale → suffisso di tipo opzionale).
Modificare `next_token()` per intercettare i literal con punto iniziale (`.5`) prima del branch
operatori. Decomposizione in helper privati per restare sotto i limiti Lizard (CCN ≤ 15).
Zero nuove dipendenze, zero nuovi file sorgente — modifica localizzata a 4 file esistenti.

## Technical Context

**Language/Version**: C++23 (ISO standard), compilatori MSVC 2026 / GCC / Clang
**Primary Dependencies**: Catch2 3.13.0 (test), fmt 12.1.0, spdlog 1.17.0, CLI11 2.6.1
**Storage**: N/A
**Testing**: Catch2 — tre tier: `constexpr_tests`, `relaxed_constexpr_tests`, `tests`
**Target Platform**: Cross-platform (Windows, Linux, macOS)
**Project Type**: Compiler (componente lexer)
**Performance Goals**: Single-pass O(n) per literal, nessun backtracking non-lineare, no regex runtime
**Constraints**: Lizard CCN ≤ 15, function length ≤ 100 lines, ≤ 6 params per function
**Scale/Scope**: Modifica localizzata in 2 metodi + helper privati nella classe monolitica `Lexer` (namespace `jsv`)

## Constitution Check

*GATE: Must pass before Phase 0 research. Re-check after Phase 1 design.*

| Principio | Stato | Note |
|-----------|-------|------|
| I. Platform Independence | ✅ PASS | Puro C++ standard, zero API OS-specifiche |
| II. VS 2026 Compatibility | ✅ PASS | Solo `std::isdigit`, `char`, `std::string_view` — supporto MSVC completo |
| III. C++ Core Guidelines | ⚠️ RISK | `scan_numeric_literal()` rischia CCN > 15 senza decomposizione in helper |
| III.a Ownership Semantics | ✅ PASS | Nessuna allocazione dinamica, opera su `string_view` |
| III.b Const Correctness | ✅ PASS | Helper saranno `const` dove possibile, parametri `const&` |
| III.c Move Semantics | ✅ N/A | Nessuna risorsa gestita nel codice modificato |
| III.d Error Handling | ✅ PASS | Ritorno via `Token`, nessuna eccezione |
| IV. TDD | ✅ PASS | Workflow Red-Green-Refactor, test prima dell'implementazione |
| V. Dependency Management | ✅ PASS | Zero nuove dipendenze |
| VI. Documentation | ✅ PASS | Doc inline e commenti esplicativi nei metodi |

**Mitigazione rischio III**: Decomposizione obbligatoria di `scan_numeric_literal()` in 3 helper
privati: `try_scan_exponent()` (G2), `try_scan_type_suffix()` (G3), `match_width_suffix()` (utility).
Ciascun metodo avrà CCN ≤ 10.

## Project Structure

### Documentation (this feature)

```text
specs/003-numeric-literal-lexer/
├── plan.md              # This file
├── research.md          # Phase 0 output
├── data-model.md        # Phase 1 output
├── quickstart.md        # Phase 1 output
├── contracts/           # Phase 1 output
├── tasks.md             # Phase 2 output (/speckit.tasks)
└── performance-report.md # Phase 8 output (T053 performance profiling results)
```

### Source Code (repository root)

```text
include/jsav/lexer/
└── Lexer.hpp            # Dichiarazioni helper privati (Task 3)

src/jsav_Lib/lexer/
└── Lexer.cpp            # Rewrite scan_numeric_literal + modifica next_token (Task 1-2)

test/
├── tests.cpp            # Test runtime Catch2 (Task 4)
└── constexpr_tests.cpp  # Test constexpr STATIC_REQUIRE (Task 5)
```

**Structure Decision**: Struttura esistente del progetto jsav — nessun nuovo file, nessuna nuova
directory. Le modifiche sono confinate a 4 file esistenti nel layout già in uso.

## Complexity Tracking

| Rischio | Mitigazione | Alternativa rifiutata perché |
|---------|-------------|------------------------------|
| `scan_numeric_literal()` CCN > 15 | Decomposizione in `try_scan_exponent()`, `try_scan_type_suffix()`, `match_width_suffix()` | Metodo monolitico: supererebbe CCN 15 con i 3 gruppi + edge cases |
| Trailing-dot behavior change | Test di regressione espliciti + aggiornamento commenti | Mantenere split `123.`→`Numeric+Dot`: contraddice la spec FR-003 |
