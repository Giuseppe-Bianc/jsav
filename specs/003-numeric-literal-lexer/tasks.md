# Tasks: Riconoscimento Completo dei Literal Numerici nel Lexer

**Input**: Design documents from `/specs/003-numeric-literal-lexer/`
**Prerequisites**: plan.md, spec.md, research.md, data-model.md, contracts/numeric-token-contract.md

**Tests**: Inclusi — il progetto adotta il workflow TDD (Red-Green-Refactor) come da Constitution Principle IV.

**Organization**: Task organizzati per user story per consentire implementazione e test indipendenti di ciascuna storia.

## Format: `[ID] [P?] [Story] Description`

- **[P]**: Può essere eseguito in parallelo (file diversi, nessuna dipendenza)
- **[Story]**: A quale user story appartiene il task (es. US1, US2, US3)
- Percorsi file esatti inclusi nelle descrizioni

## Path Conventions

- **Header**: `include/jsav/lexer/Lexer.hpp`
- **Source**: `src/jsav_Lib/lexer/Lexer.cpp`
- **Tests runtime**: `test/tests.cpp`
- **Tests constexpr**: `test/constexpr_tests.cpp`

---

## Phase 1: Setup

**Purpose**: Preparazione branch e verifica build esistente

- [ ] T001 Verificare che il branch `003-numeric-literal-lexer` sia attivo e che la build compili senza errori con `cmake -S . -B ./build -Djsav_ENABLE_IPO:BOOL=OFF -Djsav_ENABLE_CPPCHECK:BOOL=OFF -DFMT_PEDANTIC:BOOL=ON -Djsav_ENABLE_SANITIZER_ADDRESS:BOOL=OFF`
- [ ] T002 Eseguire la suite di test esistente (`ninja tests relaxed_constexpr_tests && ctest -R "unittests|relaxed_constexpr" --output-on-failure`) e verificare che tutti i test passino (baseline di regressione)

---

## Phase 2: Foundational (Blocking Prerequisites)

**Purpose**: Dichiarazioni helper privati nel header e modifica entry point `next_token()` — prerequisiti che bloccano tutte le user story

**⚠️ CRITICAL**: Nessun lavoro sulle user story può iniziare prima del completamento di questa fase

- [ ] T003 Aggiornare il doccomment di `scan_numeric_literal()` in `include/jsav/lexer/Lexer.hpp` per riflettere il nuovo comportamento trailing-dot (rimuovere riferimento al vecchio split `Numeric + Dot`) come da ricerca R1
- [ ] T004 [P] Dichiarare il metodo helper privato `void try_scan_exponent()` in `include/jsav/lexer/Lexer.hpp` con doccomment per G2 (come da data-model.md Method Signatures)
- [ ] T005 [P] Dichiarare il metodo helper privato `void try_scan_type_suffix()` in `include/jsav/lexer/Lexer.hpp` con doccomment per G3 (come da data-model.md Method Signatures)
- [ ] T006 [P] Dichiarare il metodo helper privato `[[nodiscard]] bool match_width_suffix()` in `include/jsav/lexer/Lexer.hpp` con doccomment (come da data-model.md Method Signatures)
- [ ] T007 Modificare `next_token()` in `src/jsav_Lib/lexer/Lexer.cpp` per aggiungere il branch leading-dot: se `peek_byte() == '.'` e `std::isdigit(peek_byte(1))` allora invocare `scan_numeric_literal(start)` prima di `scan_operator_or_punctuation()` come da ricerca R2
- [ ] T008 Formattare i file modificati con `clang-format -i include/jsav/lexer/Lexer.hpp src/jsav_Lib/lexer/Lexer.cpp`
- [ ] T009 Build e verifica che i test esistenti continuino a passare (regressione baseline)
- [ ] T010 [P] [FR-027] Verificare assenza di utilizzo regex in `src/jsav_Lib/lexer/Lexer.cpp`:
  - Verificare che `#include <regex>` NON sia presente
  - Verificare che `std::regex`, `std::regex_match`, `std::regex_search`, `std::regex_replace` NON siano utilizzati
  - Metodo: eseguire `grep -r "include <regex>" src/jsav_Lib/lexer/Lexer.cpp` e `grep -r "std::regex" src/jsav_Lib/lexer/Lexer.cpp`
  - Entrambi i comandi devono restituire exit code 1 (nessun match trovato)

**Checkpoint**: Foundation pronta — l'implementazione delle user story può iniziare

---

## Phase 3: User Story 1 — Riconoscimento numeri interi e decimali di base (Priority: P1) 🎯 MVP

**Goal**: Il lexer riconosce correttamente numeri interi semplici (`0`, `1`, `42`, `007`), decimali con parte intera e frazionaria (`1.0`, `3.14`), decimali con punto finale (`3.`, `42.`), e numeri con sola parte frazionaria (`.5`, `.14`, `.0`). Il testo del token è preservato senza normalizzazione.

**Independent Test**: Fornire al lexer stringhe con ciascuna forma numerica di base e verificare tipo `Numeric`, testo esatto e coordinate di posizione.

### Tests for User Story 1 ⚠️

> **NOTE: Scrivere questi test PRIMA dell'implementazione. Verificare che FALLISCANO (RED phase) prima di implementare.**

- [ ] T011 [P] [US1] Scrivere TEST_CASE per interi semplici (`0`, `1`, `42`, `007`) verificando `TokenKind::Numeric` e testo esatto in `test/tests.cpp`; includere SECTION che verifica coordinate di posizione (`span.start`, `span.end`, riga, colonna) su almeno 5 token rappresentativi (FR-025)
- [ ] T012 [P] [US1] Scrivere TEST_CASE per decimali con parte intera e frazionaria (`1.0`, `3.14`, `0.5`) verificando testo esatto in `test/tests.cpp`
- [ ] T013 [P] [US1] Scrivere TEST_CASE per decimali con punto finale (`3.`, `42.`) verificando che il punto sia incluso nel token `Numeric("3.")` in `test/tests.cpp`
- [ ] T014 [P] [US1] Scrivere TEST_CASE per numeri con sola parte frazionaria (`.5`, `.14`, `.0`) verificando token `Numeric(".5")` in `test/tests.cpp`
- [ ] T015 [P] [US1] Scrivere TEST_CASE per edge cases: punto isolato (`.`) → non Numeric, punto seguito da non-cifra (`.abc`) → non Numeric in `test/tests.cpp`
- [ ] T016 [P] [US1] Scrivere test constexpr in `test/constexpr_tests.cpp` CONTEMPORANEAMENTE ai test runtime (Constitution IV: TDD test-first):
  - Definire funzioni `consteval` che invocano il lexer su input di base (`42`, `3.14`, `3.`, `.5`)
  - Usare `STATIC_REQUIRE` per verificare a compile-time che il token prodotto abbia tipo `TokenKind::Numeric` e testo esatto
  - **Workflow TDD**: Scrivere PRIMA dell'implementazione, verificare che NON compili (RED perché lexer non è ancora constexpr), implementare rendendo i metodi constexpr, verificare che compili (GREEN)

### Implementation for User Story 1

- [ ] T017 [US1] Riscrivere il corpo di `scan_numeric_literal()` in `src/jsav_Lib/lexer/Lexer.cpp` per implementare il gruppo G1 ramo A: consumo cifre intere, consumo opzionale del punto decimale e cifre frazionarie — il punto finale (`3.`) DEVE essere incluso nel token come da FR-003 e R1
- [ ] T018 [US1] Implementare in `scan_numeric_literal()` il gruppo G1 ramo B in `src/jsav_Lib/lexer/Lexer.cpp`: quando l'entry point è un punto (da `next_token()`), consumare il punto e le cifre frazionarie successive
- [ ] T019 [US1] Rimuovere/sostituire il commento legacy sul vecchio trailing-dot behavior in `src/jsav_Lib/lexer/Lexer.cpp` (righe ~245-248) come da R1
- [ ] T020 [US1] Formattare con `clang-format -i src/jsav_Lib/lexer/Lexer.cpp test/tests.cpp test/constexpr_tests.cpp`
- [ ] T021 [US1] Eseguire `ninja tests relaxed_constexpr_tests && ctest -R "unittests|relaxed_constexpr" --output-on-failure` — tutti i test US1 devono passare e nessuna regressione

**Checkpoint**: User Story 1 completa — numeri di base riconosciuti correttamente, testati indipendentemente

---

## Phase 4: User Story 2 — Riconoscimento notazione scientifica (Priority: P2)

**Goal**: Il lexer riconosce il gruppo esponente G2 (`[eE][+-]?\d+`) immediatamente dopo G1. Se l'esponente è incompleto (`1e`, `1e+`), il marcatore e il segno non vengono consumati.

**Independent Test**: Fornire stringhe con notazione scientifica valida e non valida, verificare token prodotti.

### Tests for User Story 2 ⚠️

> **NOTE: Scrivere questi test PRIMA dell'implementazione. Verificare che FALLISCANO (RED phase) prima di implementare.**

- [ ] T022 [P] [US2] Scrivere TEST_CASE per esponenti validi (`1e10`, `3.14E+2`, `2.5e-3`, `.5E10`) verificando singolo token Numeric in `test/tests.cpp`
- [ ] T023 [P] [US2] Scrivere TEST_CASE per esponenti non validi: `1e` → `Numeric("1")` + token `e`; `1e+` → `Numeric("1")` + `e` + `+`; `1E-` → `Numeric("1")` + `E` + `-` in `test/tests.cpp`
- [ ] T024 [P] [US2] Scrivere test constexpr in `test/constexpr_tests.cpp` CONTEMPORANEAMENTE ai test runtime (Constitution IV: TDD test-first):
  - Definire funzioni `consteval` che verificano notazione scientifica valida (`1e10`, `3.14E+2`, `2.5e-3`) a compile-time
  - Usare `STATIC_REQUIRE` per verificare che esponenti incompleti (`1e`, `1e+`, `1E-`) producano token separati
  - **Workflow TDD**: Scrivere PRIMA dell'implementazione, verificare RED, implementare `try_scan_exponent()` come `constexpr`, verificare GREEN

### Implementation for User Story 2

- [ ] T025 [US2] Implementare `try_scan_exponent()` in `src/jsav_Lib/lexer/Lexer.cpp` con pattern save/restore di `m_pos` e `m_column`: tentare consumo `e`/`E`, opzionale `+`/`-`, cifre obbligatorie; rollback completo se cifre assenti come da R4
- [ ] T026 [US2] Aggiungere la chiamata a `try_scan_exponent()` alla fine di G1 in `scan_numeric_literal()` in `src/jsav_Lib/lexer/Lexer.cpp`
- [ ] T027 [US2] Rimuovere il vecchio codice di consumo esponente incondizionato (se ancora presente) in `src/jsav_Lib/lexer/Lexer.cpp`
- [ ] T028 [US2] Formattare con `clang-format -i src/jsav_Lib/lexer/Lexer.cpp test/tests.cpp test/constexpr_tests.cpp`
- [ ] T029 [US2] Eseguire `ninja tests relaxed_constexpr_tests && ctest -R "unittests|relaxed_constexpr" --output-on-failure` — tutti i test US1+US2 devono passare

**Checkpoint**: User Story 2 completa — notazione scientifica riconosciuta, esponenti incompleti gestiti correttamente

---

## Phase 5: User Story 3 — Riconoscimento suffissi di tipo (Priority: P3)

**Goal**: Il lexer riconosce i suffissi di tipo G3 (`d`/`D`, `f`/`F`, `u`/`U` con opzionale width, `i`/`I` con width obbligatoria). Maximal munch: suffissi composti hanno priorità. `f`/`F` non forma composti. `i`/`I` da solo non è suffisso.

**Independent Test**: Fornire cifre seguite da tutti i suffissi ammessi e verificare tokenizzazione corretta.

### Tests for User Story 3 ⚠️

> **NOTE: Scrivere questi test PRIMA dell'implementazione. Verificare che FALLISCANO (RED phase) prima di implementare.**

- [ ] T030 [P] [US3] Scrivere TEST_CASE per suffissi singolo-carattere (`42u`, `42U`, `1.0F`, `1.0f`, `10d`, `10D`) verificando testo nel token Numeric in `test/tests.cpp`
- [ ] T031 [P] [US3] Scrivere TEST_CASE per suffissi composti validi (`255u8`, `1000i32`, `50i16`, `50I16`, `100U32`) verificando testo nel token Numeric in `test/tests.cpp`
- [ ] T032 [P] [US3] Scrivere TEST_CASE per edge cases suffissi: `1i` → `Numeric("1")` + `i`; `1u64` → `Numeric("1u")` + `64`; `5f32` → `Numeric("5f")` + `32`; `1i64` → `Numeric("1")` + token separati in `test/tests.cpp`
- [ ] T033 [P] [US3] Scrivere test constexpr in `test/constexpr_tests.cpp` CONTEMPORANEAMENTE ai test runtime (Constitution IV: TDD test-first):
  - Definire funzioni `consteval` che verificano suffissi validi (`42u`, `1.0F`, `255u8`, `1000i32`) a compile-time
  - Usare `STATIC_REQUIRE` per edge cases (`1i`, `1u64`, `5f32`) che producono token separati
  - **Workflow TDD**: Scrivere PRIMA dell'implementazione, verificare RED, implementare `try_scan_type_suffix()` e `match_width_suffix()` come `constexpr`, verificare GREEN

### Implementation for User Story 3

- [ ] T034 [US3] Implementare `match_width_suffix()` in `src/jsav_Lib/lexer/Lexer.cpp`: confronto con priorità `32` → `16` → `8` con advance se match, return false senza advance se nessun match come da FR-017 e R6
- [ ] T035 [US3] Implementare `try_scan_type_suffix()` in `src/jsav_Lib/lexer/Lexer.cpp`: riconoscimento `d`/`D` e `f`/`F` come singoli; `u`/`U` con tentativo width (bare se width non valida); `i`/`I` con width obbligatoria (non consumare se width assente) come da FR-011–FR-017 e R6
- [ ] T036 [US3] Aggiungere la chiamata a `try_scan_type_suffix()` dopo `try_scan_exponent()` in `scan_numeric_literal()` in `src/jsav_Lib/lexer/Lexer.cpp`
- [ ] T037 [US3] Formattare con `clang-format -i src/jsav_Lib/lexer/Lexer.cpp test/tests.cpp test/constexpr_tests.cpp`
- [ ] T038 [US3] Eseguire `ninja tests relaxed_constexpr_tests && ctest -R "unittests|relaxed_constexpr" --output-on-failure` — tutti i test US1+US2+US3 devono passare

**Checkpoint**: User Story 3 completa — suffissi di tipo riconosciuti con maximal munch

---

## Phase 6: User Story 4 — Pattern completo G1-G2-G3 combinato (Priority: P4)

**Goal**: Il lexer riconosce la combinazione dei tre gruppi nell'ordine G1 → G2 → G3, contigui e senza separatori, producendo un singolo token Numeric.

**Independent Test**: Fornire stringhe che combinano tutti e tre i gruppi e verificare il token risultante.

### Tests for User Story 4 ⚠️

> **NOTE: Scrivere questi test PRIMA dell'implementazione. Verificare che FALLISCANO (RED phase) prima di implementare.**

- [ ] T039 [P] [US4] Scrivere TEST_CASE per combinazioni G1+G2+G3: `1.5e10f` → `Numeric("1.5e10f")`, `2.0E-3d` → `Numeric("2.0E-3d")`, `1e2u16` → `Numeric("1e2u16")`, `.5e1i32` → `Numeric(".5e1i32")` in `test/tests.cpp`
- [ ] T040 [P] [US4] Scrivere test constexpr in `test/constexpr_tests.cpp` CONTEMPORANEAMENTE ai test runtime (Constitution IV: TDD test-first):
  - Definire funzioni `consteval` che verificano combinazioni complete (`1.5e10f`, `2.0E-3d`, `1e2u16`, `.5e1i32`) a compile-time
  - Usare `STATIC_REQUIRE` per verificare che G1→G2→G3 producano singolo token con testo esatto
  - **Workflow TDD**: Scrivere PRIMA della verifica finale, verificare che il pattern completo sia `constexpr`-compatibile
- [ ] T041 [P] [US4] Scrivere TEST_CASE per opzionalità gruppi (FR-019): verificare che solo G1 sia obbligatorio:
  - `42` → `Numeric("42")` (solo G1)
  - `42e10` → `Numeric("42e10")` (G1 + G2)
  - `42u` → `Numeric("42u")` (G1 + G3)
  - `42e10u` → `Numeric("42e10u")` (G1 + G2 + G3)
  - Verificare che G2 e G3 siano opzionali ma G1 sia richiesto

### Implementation for User Story 4

- [ ] T042 [US4] Verificare in `scan_numeric_literal()` in `src/jsav_Lib/lexer/Lexer.cpp` che il flusso G1 → `try_scan_exponent()` → `try_scan_type_suffix()` sia correttamente concatenato e produca un singolo token per combinazioni come `1.5e10f` e `1e2u16`
- [ ] T043 [US4] Eseguire `ninja tests relaxed_constexpr_tests && ctest -R "unittests|relaxed_constexpr" --output-on-failure` — tutti i test US1+US2+US3+US4 devono passare

**Checkpoint**: User Story 4 completa — pattern G1→G2→G3 funzionante end-to-end

---

## Phase 7: User Story 5 — Regola maximal munch e confini di token (Priority: P5)

**Goal**: Il lexer applica maximal munch, `+`/`-` sono parte del token solo dentro G2, spazi/operatori/delimitatori/fine file/non-ASCII terminano correttamente il literal. I caratteri newline (`\n`, `\r`, `\r\n`) terminano incondizionatamente il token numerico anche a pattern G1→G2→G3 incompleto (FR-028).

**Independent Test**: Fornire input con numeri adiacenti ad altri token e verificare la corretta separazione. Fornire input con newline interposti (es. `"42\n10"`) e verificare che il newline termini il primo token e il secondo numero inizi un nuovo token.

### Tests for User Story 5 ⚠️

> **NOTE: Scrivere questi test PRIMA dell'implementazione. Verificare che FALLISCANO (RED phase) prima di implementare.**

- [ ] T044 [P] [US5] Scrivere TEST_CASE per confini di token: `-42` → `-` + `Numeric("42")`; `42 u8` → `Numeric("42")` + `u8`; `3.14+2` → `Numeric("3.14")` + `+` + `Numeric("2")`; `1e2+3` → `Numeric("1e2")` + `+` + `Numeric("3")` in `test/tests.cpp`
- [ ] T045 [P] [US5] Scrivere TEST_CASE per terminazione su non-ASCII e fine file in `test/tests.cpp`
- [ ] T046 [P] [US5] Scrivere TEST_CASE per terminazione newline (FR-028): `"42\n10"` → `Numeric("42")` + newline token + `Numeric("10")`; `"3.14\r\n2.5"` → `Numeric("3.14")` + newline token + `Numeric("2.5")`; `"1e2\r3"` → `Numeric("1e2")` + newline token + `Numeric("3")` in `test/tests.cpp`
- [ ] T047 [P] [US5] Scrivere test constexpr in `test/constexpr_tests.cpp` CONTEMPORANEAMENTE ai test runtime (Constitution IV: TDD test-first):
  - Definire funzioni `consteval` che verificano confini di token (`-42` → `-` + `42`, `42 u8` → `42` + `u8`) a compile-time
  - Usare `STATIC_REQUIRE` per maximal munch e terminazione newline
  - **Workflow TDD**: Scrivere PRIMA della verifica finale, verificare che la logica di confinamento sia `constexpr`-compatibile

### Implementation for User Story 5

- [ ] T048 [US5] Verificare in `scan_numeric_literal()` in `src/jsav_Lib/lexer/Lexer.cpp` che la terminazione del token avvenga correttamente al primo carattere non consumabile (spazi, operatori, delimitatori, EOF, non-ASCII) e che `+`/`-` non vengano consumati fuori dal contesto G2
- [ ] T049 [US5] Verificare in `scan_numeric_literal()` in `src/jsav_Lib/lexer/Lexer.cpp` che i caratteri `\n`, `\r` terminino incondizionatamente il token numerico anche se G1→G2→G3 sarebbe continuabile; il carattere newline NON DEVE essere consumato dal token Numeric ma deve rimanere nel flusso per il prossimo token come da FR-028
- [ ] T050 [US5] Eseguire `ninja tests relaxed_constexpr_tests && ctest -R "unittests|relaxed_constexpr" --output-on-failure` — tutti i test US1–US5 devono passare

**Checkpoint**: User Story 5 completa — maximal munch e confini di token corretti

---

## Phase 8: Polish & Cross-Cutting Concerns

**Purpose**: Verifica complessità, regressione completa, formattazione finale e documentazione

- [ ] T051 [P] Eseguire analisi Lizard (`cmake --build build --target lizard`) e verificare che tutti i metodi modificati rispettino CCN ≤ 15 e length ≤ 100 lines (Constitution Principle III)
- [ ] T052 [P] Scrivere performance test in `test/tests.cpp` che definisce il criterio O(n) PRIMA dell'implementazione della profilazione (Constitution IV: Test-First):
  - Includere `#include "jsavCore/timer/Timer.hpp"` nel file di test
  - Definire TEST_CASE `"scan_numeric_literal_scales_linearly"` che:
    - Genera literal numerici di lunghezza 10, 100, 500, 1000 caratteri (sole cifre decimali)
    - Misura tempo di scansione per ciascuna lunghezza usando `vnd::Timer`
    - Verifica criterio: `tempo(1000) / tempo(10) ≤ 150` (dimostra O(n) con 50% overhead)
  - **Questo test DEVE fallire inizialmente** (RED phase) perché la profilazione non è ancora implementata
- [ ] T053 Implementare la profilazione effettiva in `scan_numeric_literal()` per far passare il performance test T052:
  - Utilizzare `vnd::Timer` per misurare tempi di scansione su input di varie lunghezze
  - Documentare risultati in `specs/003-numeric-literal-lexer/performance-report.md`:
    - Tempi misurati per ciascuna lunghezza (10, 100, 500, 1000 caratteri)
    - Calcolo del fattore di scaling `tempo(1000) / tempo(10)`
    - Verdetto: PASS se scaling ≤ 150×, FAIL altrimenti
  - **Criterio di accettazione**: Il performance test T052 DEVE passare (GREEN phase)
- [ ] T054 [P] Eseguire la suite completa di test di regressione (`ctest --output-on-failure`) e verificare che TUTTI i test preesistenti passino senza regressioni (SC-007)
- [ ] T055 Formattare tutti i file modificati con `clang-format -i src/jsav_Lib/lexer/Lexer.cpp include/jsav/lexer/Lexer.hpp test/tests.cpp test/constexpr_tests.cpp`
- [ ] T056 Eseguire validazione quickstart.md: istanziare il quick smoke test con newline (`jsv::Lexer lex{"3.14e+2f\n.5\r1e 42u8\r\n1i", "test.jsav"}`) e verificare che i newline terminino correttamente i token numerici producendo token separati per i numeri su righe diverse
- [ ] T057 Revisione finale del codice: verificare che commenti e doccomment siano aggiornati in `include/jsav/lexer/Lexer.hpp` e `src/jsav_Lib/lexer/Lexer.cpp`

---

## Dependencies & Execution Order

### Phase Dependencies

- **Setup (Phase 1)**: Nessuna dipendenza — può iniziare immediatamente
- **Foundational (Phase 2)**: Dipende dal completamento di Phase 1 — **BLOCCA** tutte le user story
- **User Story 1 (Phase 3)**: Dipende da Phase 2 — implementa G1, prerequisito per US2/US3/US4
- **User Story 2 (Phase 4)**: Dipende da Phase 3 (G1 deve esistere per agganciare G2)
- **User Story 3 (Phase 5)**: Dipende da Phase 3 (G1 deve esistere per agganciare G3); può procedere in parallelo con US2
- **User Story 4 (Phase 6)**: Dipende da Phase 4 E Phase 5 (richiede G1+G2+G3 tutti implementati)
- **User Story 5 (Phase 7)**: Dipende da Phase 6 (verifica confini sull'intero pattern)
- **Polish (Phase 8)**: Dipende dal completamento di tutte le user story desiderate

### User Story Dependencies

```text
Phase 1 (Setup)
    │
    ▼
Phase 2 (Foundational) ──── BLOCKS ALL ────┐
    │                                        │
    ▼                                        │
Phase 3 (US1: G1 base) ◄────────────────────┘
    │
    ├─────────────┐
    ▼             ▼
Phase 4 (US2)  Phase 5 (US3)    ← possono procedere in parallelo
    │             │
    └──────┬──────┘
           ▼
    Phase 6 (US4: G1+G2+G3)
           │
           ▼
    Phase 7 (US5: confini)
           │
           ▼
    Phase 8 (Polish)
```

### Within Each User Story

1. Test DEVONO essere scritti e DEVONO FALLIRE prima dell'implementazione (TDD Red-Green-Refactor)
2. Implementazione dei metodi helper prima dell'orchestratore
3. Integrazione: chiamata all'helper nell'orchestratore
4. Format → Build → Test verde → Checkpoint

### Parallel Opportunities

- **Phase 2**: T004, T005, T006, T010 possono essere eseguiti in parallelo (dichiarazioni helper e verifica regex in sezioni diverse del header)
- **Phase 3**: T011–T016 possono essere eseguiti in parallelo (test in file diversi o sezioni indipendenti)
- **Phase 4 e Phase 5**: Possono procedere in parallelo (US2 modifica `try_scan_exponent`, US3 modifica `try_scan_type_suffix` — metodi separati)
- **Phase 6**: T039, T040, T041 possono essere eseguiti in parallelo
- **Phase 7**: T044, T045, T046, T047 possono essere eseguiti in parallelo
- **Phase 8**: T051, T052, T054 possono essere eseguiti in parallelo

---

## Parallel Example: User Story 1

```text
# Test in parallelo (tutti su file diversi o sezioni indipendenti):
T011: TEST_CASE interi semplici                    → test/tests.cpp
T012: TEST_CASE decimali con parte frazionaria     → test/tests.cpp
T013: TEST_CASE decimali con punto finale          → test/tests.cpp
T014: TEST_CASE numeri con sola parte frazionaria  → test/tests.cpp
T015: TEST_CASE edge cases punto                   → test/tests.cpp
T016: STATIC_REQUIRE test constexpr                → test/constexpr_tests.cpp
```

## Parallel Example: User Story 2 e 3 in parallelo

```text
# US2 e US3 possono procedere in parallelo dopo US1:
Developer A (US2): T022-T029 → try_scan_exponent() in Lexer.cpp
Developer B (US3): T030-T038 → try_scan_type_suffix() + match_width_suffix() in Lexer.cpp
```

---

## Implementation Strategy

### MVP First (User Story 1 Only)

1. Completare Phase 1: Setup
2. Completare Phase 2: Foundational (CRITICAL — blocca tutte le story)
3. Completare Phase 3: User Story 1 — G1 base
4. **STOP e VALIDARE**: Testare US1 indipendentemente
5. Il lexer riconosce già correttamente tutti i numeri di base

### Incremental Delivery

1. Setup + Foundational → Infrastruttura pronta
2. User Story 1 (G1) → Test indipendenti → **MVP funzionante** 🎯
3. User Story 2 (G2) → Test indipendenti → Notazione scientifica aggiunta
4. User Story 3 (G3) → Test indipendenti → Suffissi di tipo aggiunti
5. User Story 4 (G1+G2+G3) → Test indipendenti → Pattern combinato validato
6. User Story 5 → Test indipendenti → Confini di token validati
7. Polish → Complessità, regressione, documentazione

### Parallel Team Strategy

Con due sviluppatori dopo il completamento di US1:

1. Team completa Setup + Foundational + US1 insieme
2. Dopo US1 completata:
   - **Developer A**: US2 (try_scan_exponent)
   - **Developer B**: US3 (try_scan_type_suffix + match_width_suffix)
3. US4 e US5 sequenziali dopo il merge di US2+US3
4. Polish finale

---

## Notes

- I task [P] operano su file diversi o sezioni indipendenti, senza conflitti
- Il label [Story] mappa ogni task alla user story per tracciabilità
- Ogni user story è testabile indipendentemente al suo checkpoint
- **Workflow TDD**: test RED → implementazione → test GREEN → refactor
- Commit dopo ogni task o gruppo logico
- I file coinvolti sono 5: `Lexer.hpp`, `Lexer.cpp`, `tests.cpp`, `constexpr_tests.cpp`, `performance-report.md` (generated)
- **Total tasks**: 57 (T001–T057), including T010 for FR-027 regex verification
- **Constitution Principle IV**: Test constexpr DEVONO seguire lo stesso workflow test-first dei test runtime — scrivere PRIMA, verificare RED (non compila), implementare constexpr, verificare GREEN
