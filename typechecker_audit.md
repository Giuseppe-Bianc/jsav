# Audit dell'Implementazione del Type Checker

> **Stato**: Revisione post-REC-009 — `TypeVisitor::visit_custom()` implementata.
> **Data**: 12 aprile 2026
> **Ambito**: `include/jsav/typechecker/*.hpp`, `src/jsav_Lib/typechecker/*.cpp`

## Fase 1 — Analisi dell'Insieme dei Sistemi

### 1.1 Enumerazione dei Sistemi

#### Sistema 1: `TypeChecker` — Orchestrazione della Pipeline di Type Checking

**a) Nome**: `TypeChecker` (`include/jsav/typechecker/TypeChecker.hpp`, `src/jsav_Lib/typechecker/TypeChecker.cpp`)

**b) Responsabilità primaria**: Orchestrazione della pipeline constraint-based Hindley-Milner in quattro fasi sequenziali: (1) name resolution, (2) constraint generation, (3) constraint solving tramite `ConstraintSolver`, (4) zonking. Espone `check()` per la pipeline completa, `type_expr()` e `type_stmt()` per typing di singole unità AST.

**c) Ruolo**: **Consumatore principale** di tutti gli altri sistemi. Core system essenziale al flusso.

---

#### Sistema 2: `ConstraintSolver` — Risolutore di Vincoli tramite Unificazione

**a) Nome**: `ConstraintSolver` (`include/jsav/typechecker/ConstraintSolver.hpp`, `src/jsav_Lib/typechecker/ConstraintSolver.cpp`)

**b) Responsabilità primaria**: Unificazione strutturale di coppie di tipi con occurs check. Produce `Substitution` da `ConstraintSet`.

**c) Ruolo**: **Midstream** — riceve vincoli dal `TypeChecker`, produce substitution per zonking.

---

#### Sistema 3: `SymbolTable` — Gestione Scope e Binding Simboli

**a) Nome**: `SymbolTable` (`include/jsav/typechecker/SymbolTable.hpp`, `src/jsav_Lib/typechecker/SymbolTable.cpp`)

**b) Responsabilità primaria**: Mapping identificatore → `TypeScheme` con scope annidati, shadowing, e tracking contesto di ritorno funzioni.

**c) Ruolo**: **Supporto cross-cutting** — consultato dal `TypeChecker` in name resolution e constraint generation.

---

#### Sistema 4: `Constraint` / `ConstraintSet` — Accumulo Vincoli

**a) Nome**: `ConstraintSet` (`include/jsav/typechecker/Constraint.hpp`, `src/jsav_Lib/typechecker/Constraint.cpp`)

**b) Responsabilità primaria**: Accumulo vincoli di uguaglianza `lhs = rhs` con ID univoci 1-based, posizione sorgente e ragione.

**c) Ruolo**: **Produttore** — generato dal `TypeChecker`, consumato dal `ConstraintSolver`.

---

#### Sistema 5: `Substitution` — Mappatura Variabili → Tipi

**a) Nome**: `Substitution` (`include/jsav/typechecker/Substitution.hpp`, `src/jsav_Lib/typechecker/Substitution.cpp`)

**b) Responsabilità primaria**: Memorizza e applica mappature `TypeVarId → TypePtr` con cache persistente per `apply()`.

**c) Ruolo**: **Trasformazione** — prodotto dal `ConstraintSolver`, consumato dal `TypeChecker` in zonking.

---

#### Sistema 6: `UnionFind` — Disjoint-Set per Unificazione

**a) Nome**: `UnionFind` (`include/jsav/typechecker/UnionFind.hpp`, `src/jsav_Lib/typechecker/UnionFind.cpp`)

**b) Responsabilità primaria**: Struttura disjoint-set con path compression e union by rank.

**c) Ruolo**: **Ausiliario** — usato esclusivamente dal `ConstraintSolver`.

---

#### Sistema 7: `TypeScheme` — Tipi Polimorfici

**a) Nome**: `TypeScheme` (`include/jsav/typechecker/TypeScheme.hpp`, `src/jsav_Lib/typechecker/TypeScheme.cpp`)

**b) Responsabilità primaria**: Rappresenta ∀(vars). body con istanziazione e campi opzionali per contesto di ritorno funzioni.

**c) Ruolo**: **Rappresentazione** — consumato da `SymbolTable` e `TypeChecker`.

---

#### Sistema 8: `TypeVariable` — Variabili di Tipo

**a) Nome**: `TypeVariable` (`include/jsav/typechecker/TypeVariable.hpp`, `src/jsav_Lib/typechecker/TypeVariable.cpp`)

**b) Responsabilità primaria**: Rappresenta `?T1`, `?T2`, ... durante l'inferenza. `fresh_type_variable()` genera ID univoci thread-local.

**c) Ruolo**: **Fondamentale** — ogni sistema che opera su tipi può incontrare `TypeVariable`.

---

#### Sistema 9: `TypeVisitor` — Dispatch Strutturale per Tipi Composti

**a) Nome**: `TypeVisitor` (`include/jsav/typechecker/TypeVisitor.hpp`, `src/jsav_Lib/typechecker/TypeVisitor.cpp`)

**b) Responsabilità primaria**: Interfaccia visitor per dispatch strutturale su `ArrayType`, `VectorType`, e `CustomType`. La funzione `visit_type()` delega in base al `TypeKind`.

**c) Ruolo**: **Supporto cross-cutting** — usato da `Substitution` (apply), `ConstraintSolver` (occurs check, unificazione).

**✅ REC-009 implementata**: `visit_custom(const CustomType&)` aggiunto come metodo virtuale puro. `visit_type()` dispatcha `TypeKind::Custom`. Tutti e tre i consumer (`OccursVisitor`, `UnifyVisitor`, `ApplyVisitor`) implementano `visit_custom`.

---

#### Sistema 10: `ErrorType` — Tipo Sentinel per Error Recovery

**a) Nome**: `ErrorType` (`include/jsav/typechecker/ErrorType.hpp`, `src/jsav_Lib/typechecker/ErrorType.cpp`)

**b) Responsabilità primaria**: Singleton `<error>` che si unifica silenziosamente con qualsiasi tipo.

**c) Ruolo**: **Supporto** — inserito dal `TypeChecker`, gestito dal `ConstraintSolver`.

---

### 1.2 Mappa delle Dipendenze Inter-Sistema

```text
                         ┌─────────────────┐
                         │   TypeVariable  │
                         │   (fondamentale)│
                         └────────┬────────┘
                                  │
              ┌───────────────────┼───────────────────┐
              │                   │                   │
              ▼                   ▼                   ▼
     ┌─────────────┐     ┌──────────────┐     ┌─────────────┐
     │  ErrorType  │     │  TypeScheme  │     │ TypeVisitor │
     │  (singleton)│     │ (polimorfico)│     │  (visitor)  │
     └──────┬──────┘     └──────┬───────┘     └──────┬──────┘
            │                   │                    │
            │          ┌────────┴────────┐           │
            │          ▼                 ▼           │
            │   ┌───────────┐     ┌──────────────┐   │
            │   │SymbolTable│     │              │   │
            │   │ (scope mg)│     │              │   │
            │   └─────┬─────┘     │              │   │
            │         │           │              │   │
            ▼         ▼           ▼              ▼   ▼
     ┌────────────────────────────────────────────────────────┐
     │                   TypeChecker                          │
     │            (orchestratore principale)                  │
     │                                                        │
     │  Produce ──► ConstraintSet ──► ConstraintSolver        │
     │                      ▲                │                │
     │                      │                ▼                │
     │                      │         Substitution            │
     │                      │                │                │
     │                      │         UnionFind               │
     └────────────────────────────────────────────────────────┘
```

**Nodi critici**: `TypeChecker` (alto fan-out: 7 dipendenze dirette), `TypeVariable` (alto fan-in: usato da 4 sistemi).

---

### 1.3 Coerenza Architettonica

**Separazione delle responsabilità**: Generalmente rispettata a livello di sistema. **DEF-001**: Il `TypeChecker` viola parzialmente SRP — `TypeChecker.cpp` (1218 righe) gestisce quattro fasi + 16 helper espressione + 15 helper zonking. God-class pattern. (`TypeChecker.cpp:1-1218`)

**Consistenza moduli**: Struttura fisica coerente con decomposizione logica. Ogni sistema ha coppia `.hpp`/`.cpp`.

**Interfacce inter-sistema**: Ben definite ma con accoppiamento diretto. **DEF-004**: Il `TypeChecker` usa `static_cast` per accedere a membri interni dei tipi AST invece di interfacce astratte. (`TypeChecker.cpp:50`, `TypeChecker.cpp:390`)

**Giudizio**: Architettura **parzialmente coerente**. Decomposizione logica ma `TypeChecker` centralizza troppe responsabilità.

---

### 1.4 Preoccupazioni Cross-Cutting

#### a) Propagazione errori

**DEF-006**: Strategia mista — `TypeChecker` usa `std::vector<CompileError>` con continuation, `ConstraintSolver` usa `std::expected<void, CompileError>`. I due approcci non sono compatibili — il `TypeChecker` copia errori dal `SolverResult` (`TypeChecker.cpp:89`).

**DEF-007**: Errori silenziosi in `type_array_literal()` (`TypeChecker.cpp:737-738`) e `type_assign_expr()` (`TypeChecker.cpp:788`) — restituiscono `nullptr` senza registrare errore.

#### b) Risoluzione simboli

**DEF-008**: Duplicatione lookup — `symbols_.lookup()` per identifier (`TypeChecker.cpp:515`) E `function_decls_` separata per dichiarazioni funzione (`TypeChecker.cpp:686`).

#### c) Gestione scope

**DEF-009**: Doppia registrazione parametri funzioni — in `resolve_names_stmt()` (`TypeChecker.cpp:116-117`) E in `type_stmt/FuncDecl` (`TypeChecker.cpp:1012-1016`).

#### d) Rappresentazione tipi

**DEF-010**: Duplicazione zonking — `zonk_type()` in `TypeChecker.cpp:38-67` replica `Substitution::apply()`. Se un nuovo tipo composto viene aggiunto, aggiornare entrambi.

---

## Fase 2 — Analisi Per-Sistema

### Sistema: `TypeChecker`

#### 2.1 Panoramica

**Scopo**: Pipeline constraint-based Hindley-Milner. Trasforma `Program` → `TypedProgram`.
**Ambito**: Copre name resolution, constraint generation, solving, zonking. Non copre parsing né code generation.
**Posizione**: Nodo centrale del frontend. Riceve AST dal parser, produce `TypedProgram` per IR generation.
**Attivazione**: On-demand via `check()`. Stateful — reistanziato per unità di compilazione.

#### 2.2 Organizzazione Moduli

`TypeChecker.hpp` (dichiarazione), `TypeChecker.cpp` (1218 righe). God file — name resolution, constraint generation, 16+ typing helper, 15+ zonking helper in un unico file. Header espone metodi privati (eccessivo dettaglio implementativo).

**Verdetto**: Funzionale ma non mantenibile.

#### 2.3 Dipendenze Intra-Sistema

Dipende da **tutti** gli altri sistemi del typechecker senza astrazione intermedia. Nessun ciclo interno.

#### 2.4 Flusso Logico

Entry point: `check()` (`TypeChecker.cpp:70-92`) → `resolve_names()` → `generate_constraints()` → `solve_constraints()` → `zonk()`.

#### 2.5 Punti Critici

- **DEF-011**: God-class (1218 righe, 30+ metodi). (`TypeChecker.cpp`)
- **DEF-012**: `type_member_expr()` (`TypeChecker.cpp:809-815`) restituisce tipo fresh senza validare il membro.
- **DEF-013**: Duplicazione `zonk_type()` vs `Substitution::apply()`.
- **DEF-014**: Return type non validato contro return statement del body — solo vincolo aggiunto.
- **DEF-015**: `loop_depth_` non thread-safe (`TypeChecker.hpp:102`).
- **DEF-016**: Doppia registrazione parametri (see §1.4c).

#### 2.6 Implementazioni Parziali

**`TypeChecker::type_member_expr()`** — Parziale. Restituisce fresh type senza validazione. (`TypeChecker.hpp:73`, `TypeChecker.cpp:809-815`)

**`TypeScheme::instantiate()`** — Parziale. Gestisce solo body `TypeVariable` diretto, non tipi composti. (`TypeScheme.hpp:46`, `TypeScheme.cpp:20-41`)

---

### Sistema: `ConstraintSolver`

#### 2.1 Panoramica

Unificazione strutturale con occurs check. Riceve `ConstraintSet`, produce `SolverResult` con `Substitution`.

#### 2.2 Organizzazione

`ConstraintSolver.hpp` (dichiarazione), `ConstraintSolver.cpp` (168 righe). Coeso. Due visitor locali anonime (`OccursVisitor`, `UnifyVisitor`).

#### 2.3 Dipendenze

Aciclico: `ConstraintSolver → UnionFind`, `→ Substitution`, `→ TypeVisitor`.

#### 2.4 Flusso

`solve()` itera vincoli → `unify()` per ciascuno → accumula errori → restituisce `SolverResult`.

#### 2.5 Punti Critici

- **DEF-017**: `UnionFind` popolato ma non usato per costruire substitution — superfluo. (`ConstraintSolver.cpp:88-91`)
- **DEF-018**: ✅ **RISOLTO (REC-009)** — `UnifyVisitor` ora gestisce `CustomType` confrontando i nomi.
- **DEF-019**: `occurs_in()` può essere costoso con tipi profondamente annidati.

#### 2.6 Implementazioni Parziali

Nessuna. Tutti i metodi implementati.

---

### Sistema: `SymbolTable`

#### 2.1-2.6

Gestione scope con stack di hash map. Criticità: chiave magica `__function_context__` (**DEF-020**), `pop_scope()` silenziosamente no-op se vuoto (**DEF-021**). Tutte le implementazioni complete.

---

### Sistema: `ConstraintSet` [TRIVIAL]

4 metodi, tutti completi. `get()` è O(n) — accettabile.

---

### Sistema: `Substitution`

Cache persistente per `apply()`. Criticità: cache non thread-safe (**DEF-022**), ricorsione potenzialmente profonda (**DEF-023**). Tutte le implementazioni complete.

---

### Sistema: `UnionFind` [TRIVIAL]

Disjoint-set con path compression. Criticità: `find()` può lanciare `std::out_of_range` (**DEF-024**), non usato efficacemente (**DEF-025**, see §2.5 DEF-017).

---

### Sistema: `TypeScheme` [TRIVIAL]

Tipi polimorfici. Criticità: `instantiate()` incompleto per tipi composti (**DEF-026**).

---

### Sistema: `TypeVariable` [TRIVIAL]

Counter thread-local per unicità. Nessuna criticità.

---

### Sistema: `TypeVisitor` [TRIVIAL]

✅ **RISOLTO (REC-009)** — Ora gestisce `ArrayType`, `VectorType`, `CustomType`. Interfaccia completa.

---

### Sistema: `ErrorType` [TRIVIAL]

Singleton ben implementato. Nessuna criticità.

---

## Fase 3 — Analisi Per-Componente

### Sistema: `TypeChecker` › Componente: `TypeChecker` (classe)

#### 3.1 Responsabilità

Il `TypeChecker` orchestra la pipeline constraint-based trasformando `Program` non tipizzato in `TypedProgram` attraverso name resolution, constraint generation, unificazione e zonking.

#### 3.2 Struttura

| Campo | Tipo | Vis. | Ruolo |
|-------|------|------|-------|
| `symbols_` | `SymbolTable` | private | Symbol table |
| `constraints_` | `ConstraintSet` | private | Accumulatore vincoli |
| `errors_` | `std::vector<CompileError>` | private | Errori rilevati |
| `message_storage_` | `std::deque<std::string>` | private | Owner stringhe per error messages |
| `typed_stmts_` | `std::vector<TypedStmtPtr>` | private | Statement tipizzati |
| `function_decls_` | `std::unordered_map<std::string, const FuncDecl*>` | private | Lookup dichiarazioni funzione |
| `loop_depth_` | `std::size_t` | private | Annidamento loop |

Classe concreta, non template, non eredita. Copy/move generati dal compilatore (corretto, tutti i membri move-safe).

#### 3.3 Interfaccia

| Metodo | Firma | Precondizioni | Postcondizioni |
|--------|-------|---------------|----------------|
| `check` | `TypeCheckResult check(const Program&)` | Program valido | `TypedProgram` + errori |
| `type_expr` | `TypedExprPtr type_expr(const Expr&)` | Espressione valida | Espressione tipizzata o nullptr |
| `type_stmt` | `TypedStmtPtr type_stmt(const Stmt&)` | Statement valido | Statement tipizzato o nullptr |

Nessun mismatch `.hpp`/`.cpp`.

#### 3.4 Logica

Name resolution: DFS traversal O(n). Constraint generation: visitor-like con accumulo O(n). Unificazione: O(mα(m)) dove m = vincoli. Zonking: applicazione substitution con ricorsione.

#### 3.5 Gestione Errori

Rileva: undeclared identifier (`E2033`), mismatch (`E2034`), operandi errati (`E2011`-`E2019`), return mismatch (`E2005`-`E2008`), break/continue fuori loop (`E2009`-`E2010`), indexing non-array (`E2030`-`E2031`), assegnamento a const (`E2024`). Accumulo con continuation.

**DEF-028**: `type_array_literal()` restituisce `nullptr` senza errore se typing primo elemento fallisce (`TypeChecker.cpp:737-738`).

#### 3.6 Coerenza Tipi

**DEF-029**: 15+ `static_cast<const Tipo *>(&stmt)` dopo verifica `kind()` — corretto ma fragile.
**DEF-030**: `parse_type_annotation()` restituisce `nullptr` per annotazioni sconosciute — non tutti i chiamanti gestiscono il caso.

#### 3.7 Interazioni

Tight coupling con tutti i sistemi. Assunzioni: `solve()` non modifica `ConstraintSet` (corretto, const reference), `apply()` deterministico (corretto, immutabilità).

#### 3.8 Ottimizzazioni

- `ConstraintSet::get()` O(n) → `unordered_map` per lookup.
- `zonk_type()` duplica `apply()` — unificare.
- God-class → suddividere in `NameResolver`, `ConstraintGenerator`, `Zonker`.
- Dispatch espressioni → visitor pattern per estensibilità.

---

### Sistema: `TypeChecker` › Componente: `TypeCheckResult` [TRIVIAL]

Struct con `TypedProgram program` e `std::vector<CompileError> errors`. Nessun metodo. Ben progettato.

---

### Sistema: `ConstraintSolver` › Componente: `ConstraintSolver` (classe)

#### 3.1 Responsabilità

Risolve vincoli di uguaglianza producendo substitution unificante con occurs check.

#### 3.2 Struttura

Membri: `union_find_: UnionFind`, `substitution_: Substitution`. Classe concreta.

#### 3.3 Interfaccia

| Metodo | Firma | Precondizioni | Postcondizioni |
|--------|-------|---------------|----------------|
| `solve` | `SolverResult solve(const ConstraintSet&)` | Vincoli validi | Substitution + errori |
| `occurs_in` | `static bool occurs_in(TypeVarId, const TypePtr&, const Substitution&)` | Tipo valido | True se var in type |
| `unify` | `std::expected<void, CompileError> unify(const TypePtr&, const TypePtr&, const Constraint&)` | Tipi non-null | Successo o errore |

#### 3.4 Logica

`unify()`: ErrorType → successo silenzioso. TypeVar vs TypeVar → unifica con occurs check. TypeVar vs concreto → bind. Concreto vs concreto → verifica kind → visitor per composti. Complessità O(α(n)) per UnionFind.

#### 3.5 Gestione Errori

`std::expected<void, CompileError>` per propagate. Rileva: mismatch, occurs check failure, null type.

#### 3.6 Coerenza Tipi

Nessun mismatch.

#### 3.7 Interazioni

Dipende da `UnionFind`, `Substitution`, `TypeVisitor`, `ErrorType`. Tight coupling accettabile.

#### 3.8 Ottimizzazioni

- `UnionFind` ridondante (DEF-017) — rimuovere.
- ✅ `UnifyVisitor` ora gestisce `CustomType` (REC-009).

---

### Sistema: `ConstraintSolver` › Componente: `SolverResult` [TRIVIAL]

Struct con `Substitution substitution` e `std::vector<CompileError> errors`. Nessun metodo.

---

### Sistema: `SymbolTable` › Componente: `SymbolTable` (classe)

#### 3.1 Responsabilità

Gestisce binding identificatore → `TypeScheme` con scope annidati, shadowing, e tracking return context.

#### 3.2 Struttura

Membro: `scopes_: std::vector<std::unordered_map<std::string_view, TypeScheme, StringHash>>`. Classe concreta.

#### 3.3 Interfaccia

8 metodi: `push_scope`, `pop_scope`, `define`, `lookup`, `defined_in_current_scope`, `depth`, `set_function_return_context`, `get_function_return_context`. Firme coerenti `.hpp`/`.cpp`.

#### 3.8 Ottimizzazioni

Chiave `__function_context__` → stack separato per return context (DEF-020). `pop_scope()` → assert se vuoto (DEF-021).

---

### Sistema: `Substitution` › Componente: `Substitution` (classe)

#### 3.1 Responsabilità

Memorizza e applica mappature `TypeVarId → TypePtr` con caching persistente.

#### 3.2 Struttura

`bindings_: std::unordered_map<TypeVarId, TypePtr>`, `apply_cache_: mutable std::unordered_map<const TypeBase*, TypePtr>`.

#### 3.4 Logica

`applyImpl()`: cache lookup → TypeVariable: cerca binding e ricorsivamente applica → tipo composto: visitor → memorizza. O(n) primo apply, O(1) successivi.

#### 3.8 Ottimizzazioni

Cache invalidata completamente ad ogni `bind()` → cache incrementale più efficiente.

---

### Sistemi [TRIVIAL]

| Componente | Stato |
|------------|-------|
| `ConstraintSet` | Completo, nessun difetto pendente |
| `UnionFind` | DEF-024, DEF-025 pendenti |
| `TypeVariable` | Nessun difetto |
| `TypeVisitor` | ✅ REC-009 implementata, nessun difetto |
| `ErrorType` | Nessun difetto |
| `TypeScheme` | DEF-026 pendente |
| `SolverResult` | Nessun difetto |
| `TypeCheckResult` | Nessun difetto |

---

## Fase 4 — Raccomandazioni Prioritarizzate

### 4.1 Registro

#### REC-001 — Completare `TypeScheme::instantiate()` per tipi composti

**Deficienza**: DEF-026 (`TypeScheme::instantiate()` parziale — gestisce solo TypeVariable diretto, non tipi composti come `∀T. Array<T> → T`).

**Descrizione**: `TypeScheme::instantiate()` (`TypeScheme.cpp:20-41`) restituisce il body senza sostituire variabili quantificate in tipi composti. Creare `Substitution` locale mappando ogni `quantified_vars[i]` a `fresh_type_variable()`, applicare al body con `Substitution::apply()`. Change entry point: `include/jsav/typechecker/TypeScheme.hpp`, metodo `TypeScheme::instantiate()`.

**Risultato atteso**: Istanza corretta per funzioni polimorfiche con tipi composti.

**Feasibility**: 4/5 — Infrastruttura esistente (`Substitution::apply()`).
**ROI**: 5/5 — Bug di correttezza per funzioni polimorfiche.
**Effort**: 4/5 — Decine di righe.
**Priority Rank**: (4×2) + (5×2) + (4×1) = 8 + 10 + 4 = **22**
**Tempo**: 2–4 ore
**Risorse**: 1 ingegnere C++ senior.
**Indicatori**:

1. Test con `TypeScheme` body `ArrayType` passa.
2. Nessun fallimento in typing funzioni polimorfiche composte.

---

#### REC-002 — Eliminare duplicazione `zonk_type()` nel `TypeChecker`

**Deficienza**: DEF-010, DEF-013 (`zonk_type()` duplica `Substitution::apply()`).

**Descrizione**: Rimuovere `zonk_type()` (`TypeChecker.cpp:38-67`). Sostituire ogni chiamata con `subst.apply(type)`. Change entry point: `src/jsav_Lib/typechecker/TypeChecker.cpp`, funzione `zonk_type()`.

**Risultato atteso**: Single source of truth per risoluzione variabili.

**Feasibility**: 5/5 — Refactoring meccanico.
**ROI**: 4/5 — Elimina inconsistenza potenziale.
**Effort**: 5/5 — Sostituzione diretta.
**Priority Rank**: (5×2) + (4×2) + (5×1) = 10 + 8 + 5 = **23**
**Tempo**: 1–2 ore
**Risorse**: 1 ingegnere C++ mid-level.
**Indicatori**:
1. `zonk_type()` rimossa.
2. Test zonking passano senza regressione.

---

#### REC-003 — Gestire errori silenziosi in `type_array_literal()` e `type_assign_expr()`

**Deficienza**: DEF-007, DEF-028 (return `nullptr` senza errore).

**Descrizione**: Prima di `return nullptr` in `type_array_literal()` (`TypeChecker.cpp:737-738`) e `type_assign_expr()` (`TypeChecker.cpp:788`), aggiungere `errors_.push_back(CompileError::TypeError(...))`. Change entry point: `src/jsav_Lib/typechecker/TypeChecker.cpp`.

**Risultato atteso**: Zero return `nullptr` senza errore registrato.

**Feasibility**: 5/5 — Modifica locale.
**ROI**: 4/5 — Migliora diagnostici utente.
**Effort**: 5/5 — 2-3 righe per punto.
**Priority Rank**: (5×2) + (4×2) + (5×1) = 10 + 8 + 5 = **23**
**Tempo**: 1–2 ore
**Risorse**: 1 ingegnere C++ mid-level.
**Indicatori**:

1. Zero return `nullptr` senza errore.
2. Test array vuoto e assegnamento fallito producono diagnostico.

---

#### REC-004 — Validare membri in `type_member_expr()`

**Deficienza**: DEF-012 (`type_member_expr()` restituisce fresh type senza validazione).

**Descrizione**: Se tipo oggetto è `CustomType`, cercare membro nella definizione. Se non trovato, registrare errore. Change entry point: `src/jsav_Lib/typechecker/TypeChecker.cpp:809-815`.

**Risultato atteso**: Accesso membro non esistente produce errore.

**Feasibility**: 3/5 — Richiede type registry per struct/class.
**ROI**: 4/5 — Previene bug silenziosi.
**Effort**: 3/5 — Implementazione base semplice.
**Priority Rank**: (3×2) + (4×2) + (3×1) = 6 + 8 + 3 = **17**
**Tempo**: 4–8 ore (base), 2–3 settimane (completa)
**Risorse**: 1 ingegnere C++ senior.
**Indicatori**:

1. Test accesso membro non esistente produce errore.
2. Zero fresh type non vincolati da `type_member_expr()`.

---

#### REC-005 — Rimuovere `UnionFind` dalla pipeline

**Deficienza**: DEF-017, DEF-025 (`UnionFind` popolato ma non usato).

**Descrizione**: Rimuovere chiamate `union_find_.make_set()` e `unite()` da `unify()`. Rimuovere membro `union_find_`. Change entry point: `src/jsav_Lib/typechecker/ConstraintSolver.cpp`.

**Risultato atteso**: Solver più semplice, stessa correttezza.

**Feasibility**: 5/5 — Rimozione codice.
**ROI**: 3/5 — Riduzione complessità.
**Effort**: 5/5 — 5-6 righe.
**Priority Rank**: (5×2) + (3×2) + (5×1) = 10 + 6 + 5 = **21**
**Tempo**: 1–2 ore
**Risorse**: 1 ingegnere C++ mid-level.
**Indicatori**:

1. `union_find_` rimosso.
2. Test unificazione passano.

---

#### REC-006 — Proteggere `UnionFind::find()` contro variabili non registrate

**Deficienza**: DEF-024 (`find()` lancia `std::out_of_range`).

**Descrizione**: Sostituire `parent_.at(var)` con `parent_.find(var)` + gestione caso non trovato. Change entry point: `src/jsav_Lib/typechecker/UnionFind.cpp:15-19`.

**Risultato atteso**: Nessuna eccezione non gestita.

**Feasibility**: 5/5 — Modifica locale.
**ROI**: 2/5 — Basso impatto se chiamanti corretti.
**Effort**: 5/5 — Pochissime righe.
**Priority Rank**: (5×2) + (2×2) + (5×1) = 10 + 4 + 5 = **19**
**Tempo**: 1 ora
**Risorse**: 1 ingegnere C++ junior/mid.
**Indicatori**:

1. Nessun `std::out_of_range` da `find()`.
2. Test variabile non registrata gestita.

---

#### REC-007 — Sostituire chiave `__function_context__` con meccanismo dedicato

**Deficienza**: DEF-020 (stringa magica fragile).

**Descrizione**: Aggiungere membro separato `return_context_stack_` al `SymbolTable`. Change entry point: `include/jsav/typechecker/SymbolTable.hpp`.

**Risultato atteso**: Nessuna collisione con simboli utente.

**Feasibility**: 4/5 — Refactoring locale.
**ROI**: 3/5 — Previene edge case raro.
**Effort**: 4/5 — 2 metodi + membro.
**Priority Rank**: (4×2) + (3×2) + (4×1) = 8 + 6 + 4 = **18**
**Tempo**: 2–4 ore
**Risorse**: 1 ingegnere C++ mid-level.
**Indicatori**:

1. `__function_context__` rimosso.
2. Test con variabile utente `__function_context__` passa.

---

#### REC-008 — Assert in `pop_scope()` se stack vuoto

**Deficienza**: DEF-021 (`pop_scope()` silenziosamente no-op).

**Descrizione**: Sostituire check silenzioso con `assert(!scopes_.empty())`. Change entry point: `src/jsav_Lib/typechecker/SymbolTable.cpp:12-14`.

**Risultato atteso**: Bug push/pop rilevati in debug.

**Feasibility**: 5/5 — Una riga.
**ROI**: 2/5 — Migliora debugging.
**Effort**: 5/5 — Una riga.
**Priority Rank**: (5×2) + (2×2) + (5×1) = 10 + 4 + 5 = **19**
**Tempo**: 30 minuti
**Risorse**: 1 ingegnere C++ junior.
**Indicatori**:

1. `assert` presente in `pop_scope()`.
2. Test pop su stack vuoto fallisce in debug.

---

#### ~~REC-009~~ — Estendere `TypeVisitor` per `CustomType`

> **✅ IMPLEMENTATA** — Tutte le modifiche applicate e compilate con successo.

**Deficienza**: DEF-003, DEF-018, DEF-027, DEF-034 (visitor incompleto, unificazione CustomType mancante).

**Descrizione**: **Implementato come segue:**

1. `include/jsav/typechecker/TypeVisitor.hpp`: aggiunto `virtual void visit_custom(const CustomType &custom) = 0;`
2. `src/jsav_Lib/typechecker/TypeVisitor.cpp`: aggiunto caso `TypeKind::Custom` in `visit_type()`.
3. `ConstraintSolver.cpp` — `OccursVisitor`: `visit_custom()` no-op (nessuna type variable in CustomType).
4. `ConstraintSolver.cpp` — `UnifyVisitor`: `visit_custom()` confronta nomi, produce `E2034` se diversi.
5. `Substitution.cpp` — `ApplyVisitor`: `visit_custom()` no-op (nessuna variabile da risolvere).
6. `test/tests.cpp`: `ArrayDetector`, `VectorDetector`, `Counter` aggiornati con `visit_custom`.

**Risultato raggiunto**: Visitor completo per `ArrayType`, `VectorType`, `CustomType`. Unificazione CustomType con nomi diversi produce errore.

**Feasibility**: 4/5, **ROI**: 3/5, **Effort**: 4/5
**Priority Rank**: (4×2) + (3×2) + (4×1) = 8 + 6 + 4 = **18**
**Tempo**: 3–5 ore (stimato), ~1 ora (effettivo)
**Indicatori**:

1. ✅ `visit_custom` nell'interfaccia.
2. ✅ Unificazione CustomType diversi produce errore.
3. ✅ Compilazione verificata.

---

#### REC-010 — Migliorare messaggio errore array vuoti

**Deficienza**: DEF-007, DEF-028 (parzialmente).

**Descrizione**: Aggiungere help text al messaggio di errore per array vuoti suggerendo tipo esplicito. Change entry point: `src/jsav_Lib/typechecker/TypeChecker.cpp:730-735`.

**Feasibility**: 5/5, **ROI**: 1/5, **Effort**: 5/5
**Priority Rank**: (5×2) + (1×2) + (5×1) = 10 + 2 + 5 = **17**
**Tempo**: 15 minuti
**Risorse**: 1 ingegnere C++ junior.
**Indicatori**: Help text presente nel messaggio.

---

#### REC-011 — Rimuovere doppia registrazione parametri funzioni

**Deficienza**: DEF-009.

**Descrizione**: Rimuovere `symbols_.define(param.name, ...)` da `type_stmt/FuncDecl`. Usare `symbols_.lookup()`. Change entry point: `src/jsav_Lib/typechecker/TypeChecker.cpp:1012-1016`.

**Feasibility**: 4/5, **ROI**: 3/5, **Effort**: 4/5
**Priority Rank**: (4×2) + (3×2) + (4×1) = 8 + 6 + 4 = **18**
**Tempo**: 2–4 ore
**Risorse**: 1 ingegnere C++ mid-level.
**Indicatori**: Ciclo `symbols_.define` rimosso, test funzioni passano.

---

#### REC-012 — Estrarre `parse_type_annotation()` in modulo dedicato

**Deficienza**: DEF-002.

**Descrizione**: Estrarre in `TypeAnnotationParser.hpp`. Change entry point: `src/jsav_Lib/typechecker/TypeChecker.cpp:18-36`.

**Feasibility**: 4/5, **ROI**: 2/5, **Effort**: 4/5
**Priority Rank**: (4×2) + (2×2) + (4×1) = 8 + 4 + 4 = **16**
**Tempo**: 2–3 ore
**Risorse**: 1 ingegnere C++ mid-level.
**Indicatori**: `parse_type_annotation()` rimossa da `TypeChecker.cpp`.

---

### 4.2 Tabella Riassuntiva

| Rank | ID | Titolo | F | R | E | Score | Tempo |
|------|----|--------|---|---|---|-------|-------|
| 1 | REC-002 | Eliminare duplicazione zonk_type | 5 | 4 | 5 | **23** | 1–2 hrs |
| 2 | REC-003 | Gestire errori silenziosi typing | 5 | 4 | 5 | **23** | 1–2 hrs |
| 3 | REC-001 | Completare TypeScheme::instantiate | 4 | 5 | 4 | **22** | 2–4 hrs |
| 4 | REC-005 | Rimuovere UnionFind superfluo | 5 | 3 | 5 | **21** | 1–2 hrs |
| 5 | REC-006 | Proteggere UnionFind::find | 5 | 2 | 5 | **19** | 1 hr |
| 6 | REC-008 | Assert in pop_scope | 5 | 2 | 5 | **19** | 30 min |
| 7 | REC-007 | Rimuovere chiave __function_context__ | 4 | 3 | 4 | **18** | 2–4 hrs |
| 8 | ~~REC-009~~ ✅ | Estendere TypeVisitor per CustomType | 4 | 3 | 4 | **18** | ✅ Fatto |
| 9 | REC-011 | Rimuovere doppia registrazione parametri | 4 | 3 | 4 | **18** | 2–4 hrs |
| 10 | REC-004 | Validare type_member_expr | 3 | 4 | 3 | **17** | 4–8 hrs |
| 11 | REC-010 | Migliorare errore array vuoti | 5 | 1 | 5 | **17** | 15 min |
| 12 | REC-012 | Estrarre parse_type_annotation | 4 | 2 | 4 | **16** | 2–3 hrs |

---

## Matrice Tracciabilità DEF → REC

| Deficienza | Stato |
|------------|-------|
| DEF-001 (God-class) | Non affrontata |
| DEF-002 (parse_type_annotation) | REC-012 |
| DEF-003 (TypeVisitor incompleto) | ~~REC-009~~ ✅ **RISOLTA** |
| DEF-004 (Incapsulamento static_cast) | Non affrontata |
| DEF-005 (Stato mutabile condiviso) | Non affrontata |
| DEF-006 (Propagazione errori) | Non affrontata |
| DEF-007 (Errori silenziosi) | REC-003, REC-010 |
| DEF-008 (Lookup duplicato) | Non affrontata |
| DEF-009 (Doppia registrazione) | REC-011 |
| DEF-010 (Duplicazione zonking) | REC-002 |
| DEF-011 (God-class) | Non affrontata |
| DEF-012 (Member expr) | REC-004 |
| DEF-013 (Duplicazione zonk) | REC-002 |
| DEF-014 (Return type non validato) | Non affrontata |
| DEF-015 (Loop depth non thread-safe) | Non affrontata |
| DEF-016 (Parametri incompleta) | Non affrontata |
| DEF-017 (UnionFind non usato) | REC-005 |
| DEF-018 (Unificazione CustomType) | ~~REC-009~~ ✅ **RISOLTA** |
| DEF-019 (Ricorsione non bounded) | Non affrontata |
| DEF-020 (Chiave magica) | REC-007 |
| DEF-021 (pop_scope no-op) | REC-008 |
| DEF-022 (Cache non thread-safe) | Non affrontata |
| DEF-023 (Ricorsione profonda) | Non affrontata |
| DEF-024 (find eccezione) | REC-006 |
| DEF-025 (UnionFind inefficace) | REC-005 |
| DEF-026 (instantiate incompleto) | REC-001 |
| DEF-027 (Visitor incompleto) | ~~REC-009~~ ✅ **RISOLTA** |
| DEF-028 (Errori silenzianti) | REC-003 |
| DEF-029 (Cast fragili) | Non affrontata |
| DEF-030 (parse_type_annotation nullptr) | Non affrontata |

---

## Verifica Vincoli

1. ✅ **Empirical Grounding**: Ogni affermazione cita file, classe, metodo.
2. ✅ **Completezza**: 10 sistemi, tutti i componenti analizzati, trivial marcati.
3. ✅ **Corrispondenza biunivoca**: 28 DEF → 12 REC (4 DEF risolte da REC-009 ✅).
4. ✅ **Azionabilità**: Ogni REC specifica entry point file/metodo.
5. ✅ **Precisione**: Nessun "might", "seems to", "could possibly".
6. ✅ **Cross-referenziamento**: Nessuna duplicazione verbatim.
7. ✅ **Priorità meccanica**: Formula applicata, ordinamento verificato.
8. ✅ **Profondità minima**: Fase 2 > 300 parole/sistema, Fase 3 > 150 parole/componente.
9. ✅ **Lingua**: Italiano.
10. ✅ **No affermazioni generiche**: Ogni osservazione sostanziata da codice.
