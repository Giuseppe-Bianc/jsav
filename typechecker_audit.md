# Audit dell'Implementazione del Type Checker

## Phase 1 — System Ensemble Analysis

### 1.1 System Enumeration

Il codebase del type checker comprende **9 sistemi** distinti, ciascuno responsabile di una specifica funzione
all'interno della pipeline di inferenza e verifica dei tipi.

| # | File Header            | Sistema                                        | File Implementazione   | Responsabilità Primaria                                                                         |
|---|------------------------|------------------------------------------------|------------------------|-------------------------------------------------------------------------------------------------|
| 1 | `TypeChecker.hpp`      | **TypeChecker** (orchestratore principale)     | `TypeChecker.cpp`      | Coordina l'intera pipeline: name resolution, constraint generation, constraint solving, zonking |
| 2 | `ConstraintSolver.hpp` | **ConstraintSolver** (motore di unificazione)  | `ConstraintSolver.cpp` | Risolve vincoli di uguaglianza tra tipi tramite union-find unification                          |
| 3 | `Constraint.hpp`       | **Constraint** (rappresentazione vincoli)      | `Constraint.cpp`       | Definisce la struttura `Constraint` e il contenitore `ConstraintSet`                            |
| 4 | `Substitution.hpp`     | **Substitution** (mappatura variabili→tipi)    | `Substitution.cpp`     | Gestisce la sostituzione di type variable con tipi concreti                                     |
| 5 | `SymbolTable.hpp`      | **SymbolTable** (gestione scope e simboli)     | `SymbolTable.cpp`      | Mappa identificatori a TypeScheme con supporto per scope annidati e shadowing                   |
| 6 | `TypeScheme.hpp`       | **TypeScheme** (tipi polimorfici)              | `TypeScheme.cpp`       | Rappresenta tipi quantificati universalmente (∀T. body) con istanziazione                       |
| 7 | `TypeVariable.hpp`     | **TypeVariable** (variabili di tipo)           | `TypeVariable.cpp`     | Rappresenta type variable (?T1, ?T2, ...) con generazione di ID univoci                         |
| 8 | `UnionFind.hpp`        | **UnionFind** (struttura disjoint-set)         | `UnionFind.cpp`        | Implementa disjoint-set con path compression e union by rank per unificazione efficiente        |
| 9 | `ErrorType.hpp`        | **ErrorType** (tipo errore per error recovery) | `ErrorType.cpp`        | Singleton tipo errore che si unifica silenziosamente con qualsiasi tipo                         |

### 1.2 Inter-System Dependency Map

```
                    ┌──────────────────────────────────────────────────────┐
                    │                  TypeChecker                         │
                    │  (orchestra: resolve_names → generate_constraints →  │
                    │               solve_constraints → zonk)              │
                    └──────┬──────────────────────────────┬────────────────┘
                           │                              │
              ┌────────────▼─────────────┐    ┌────────────▼─────────────┐
              │    ConstraintSet         │    │     SymbolTable          │
              │  (accumula vincoli)      │    │  (gestione scope)        │
              │         │                │    │         │                │
              │         ▼                │    │         ▼                │
              │    Constraint            │    │     TypeScheme           │
              │  (lhs = rhs @ location)  │    │  (∀vars. body)           │
              └────────────┬─────────────┘    └────────────┬─────────────┘
                           │                               │
                           │              ┌────────────────┼────────────┐
                           │              ▼                ▼            │
                    ┌──────▼───────┐  ┌──────────┐  ┌──────────────┐    │
                    │ Constraint-  │  │ Type     │  │ TypeVariable │    │
                    │ Solver       │  │Ptr       │  │ (fresh IDs)  │    │
                    │              │  │ (Type.hpp│  │              │    │
                    │  ┌────────┐  │  │  fuori   │  └──────┬───────┘    │
                    │  │Union-  │  │  │  sistema)│         │            │
                    │  │Find    │  │  └──────────┘         │            │
                    │  └────────┘  │                       │            │
                    │  ┌────────┐  │                       │            │
                    │  │Substi- │  │                       │            │
                    │  │tution  │  │                       │            │
                    │  └────────┘  │                       │            │
                    │  ┌────────┐  │                       │            │
                    │  │Error-  │  │                       │            │
                    │  │Type    │  │                       │            │
                    │  └────────┘  │                       │            │
                    └──────────────┘                       │            │
                                                           │            │
                    ┌──────────────────────────────────────┼────────────┘
                    │            Dipendenze esterne        │
                    ▼                                      ▼
              ┌───────────────┐                    ┌──────────────┐
              │ CompileError  │                    │ AST Types    │
              │ ErrorReporter │                    │ (Type.hpp,   │
              │ ErrorCode     │                    │  Node.hpp)   │
              │ SourceSpan    │                    └──────────────┘
              └───────────────┘
```

**Dipendenze dirette per sistema:**

| Sistema              | Dipende da                                                                                                                                                                                  |
|----------------------|---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------|
| **TypeChecker**      | `ConstraintSet`, `ConstraintSolver`, `SymbolTable`, `Substitution`, `TypeScheme`, `TypeVariable`, `ErrorType`, AST (`Program`, `TypedProgram`, `Expressions`, `Statements`), `CompileError` |
| **ConstraintSolver** | `UnionFind`, `Substitution`, `Constraint`, `ErrorType`, `CompileError`                                                                                                                      |
| **Constraint**       | `TypePtr` (da `Type.hpp`), `SourceSpan`                                                                                                                                                     |
| **Substitution**     | `TypePtr`, `TypeVariable`                                                                                                                                                                   |
| **SymbolTable**      | `TypeScheme`                                                                                                                                                                                |
| **TypeScheme**       | `TypePtr`, `TypeVariable`                                                                                                                                                                   |
| **TypeVariable**     | `TypeBase` (da `Type.hpp`)                                                                                                                                                                  |
| **UnionFind**        | Nessuna dipendenza interna al typechecker (solo STL)                                                                                                                                        |
| **ErrorType**        | `TypeBase` (da `Type.hpp`)                                                                                                                                                                  |

Il grafo delle dipendenze è un **DAG aciclico**. I sistemi foglia (`UnionFind`, `TypeVariable`, `ErrorType`) non
dipendono da nessun altro sistema del typechecker; i sistemi intermedi (`Substitution`, `TypeScheme`, `SymbolTable`,
`Constraint`) dipendono solo dai foglia; i sistemi radice (`ConstraintSolver`, `TypeChecker`) orchestrazione e consumo.

### 1.3 Architectural Coherence Evaluation

La decomposizione in sistemi segue **principi di separazione delle responsabilità generalmente sani**. La pipeline a 4
fasi (name resolution → constraint generation → constraint solving → zonking) è un'architettura consolidata per i type
checker constraint-based e la sua implementazione rispetta questa struttura.

**Punti di coerenza:**

1. I sistemi di basso livello (`UnionFind`, `TypeVariable`, `ErrorType`, `Substitution`, `Constraint`) sono **autonomi e
   coesi** — ciascuno gestisce esattamente una responsabilità.
2. La gerarchia di dipendenze è **aciclica** come documentato nella §1.2.
3. L'uso di `TypePtr = std::shared_ptr<const TypeBase>` come tipo uniforme per tutti i riferimenti a tipi è **coerente**
   attraverso tutti i sistemi.
4. I singleton per tipi primitivi (`PrimitiveType::i32()`, ecc.) e per `ErrorType` (`error_type()`) sono implementati
   correttamente con variabili statiche thread-safe.

**Punti di incoerenza:**

**DEF-001** — Duplicazione della logica di dispatch per tipi composti: Il sistema `ConstraintSolver` implementa la
logica di unificazione strutturale ricorsiva (`ConstraintSolver::unify` in `ConstraintSolver.cpp:47–127`) che duplica
parzialmente la ricorsione già presente in `Substitution::applyImpl` (`Substitution.cpp:27–54`). Sebbene i due metodi
abbiano scopi diversi (unificazione vs applicazione di sostituzione), la duplicazione della logica di dispatch per
`TypeKind::Array` e `TypeKind::Vector` (`ConstraintSolver.cpp:109–119` vs `Substitution.cpp:33–49`) è una duplicazione
di responsabilità che richiede manutenzione sincronizzata. Ogni nuovo tipo composto aggiunto a `Type.hpp` richiede
modifiche in tre punti: `Substitution::applyImpl`, `ConstraintSolver::unify`, e `ConstraintSolver::occurs_in`.

**DEF-002** — API pubblica rompe l'incapsulamento della pipeline: Il `TypeChecker` espone `type_expr` e `type_stmt` come
metodi pubblici (`TypeChecker.hpp:62–77`) esclusivamente per "unit testing". Questo rompe l'incapsulamento della
pipeline — un chiamante esterno può invocare constraint generation su espressioni isolate bypassando name resolution,
producendo risultati semanticamente inconsistenti. I simboli `TypeChecker::type_expr` e `TypeChecker::type_stmt` non
dovrebbero essere visibili al di fuori della classe.

**DEF-003** — Accoppiamento name resolution / constraint generation: Il sistema `TypeChecker` gestisce sia la name
resolution che la constraint generation nella stessa classe. Sebbene funzionale, questo accoppiamento significa che la
SymbolTable e il ConstraintSet sono campi della stessa istanza (`TypeChecker.hpp:84–85`), impedendo il riuso
indipendente di ciascuna fase. Un pipeline più modulare separerebbe le due fasi in classi distinte.

**DEF-004** — Parser di annotazioni di tipo fuori luogo: La funzione `parse_type_annotation` (`TypeChecker.cpp:19–36`)
implementa un parser di annotazioni di tipo ad-hoc all'interno del file di implementazione del TypeChecker. Questa
funzionalità è logicamente parte del sottosistema di rappresentazione dei tipi (`Type.hpp`) ma risiede nel file più
grande, creando una responsabilità fuori luogo. Inoltre, il parser è una catena di `if/else` su stringhe letterali — non
è estensibile e non gestisce tipi composti (es. `array<i32, 10>`).

### 1.4 Cross-Cutting Concerns Assessment

| Concern                             | TypeChecker                                                    | ConstraintSolver                                                                         | Substitution                                           | TypeScheme                                 | SymbolTable                                              | Uniformità                  |
|-------------------------------------|----------------------------------------------------------------|------------------------------------------------------------------------------------------|--------------------------------------------------------|--------------------------------------------|----------------------------------------------------------|-----------------------------|
| **Propagazione errori**             | Accumula in `std::vector<CompileError>` (`TypeChecker.hpp:85`) | Restituisce `std::expected<void, CompileError>` da `unify()` (`ConstraintSolver.hpp:72`) | Nessun errore (mappa totale)                           | Nessun errore                              | `std::nullopt` per lookup fallito (`SymbolTable.hpp:53`) | **INCOERENTE**              |
| **Risoluzione simboli**             | Delega a `SymbolTable::lookup`                                 | Non applicabile                                                                          | Non applicabile                                        | Non applicabile                            | Ricerca scope-inner → outer (`SymbolTable.cpp:21–27`)    | Uniforme (un solo punto)    |
| **Gestione scope**                  | `push_scope`/`pop_scope` chiamati esplicitamente               | Non applicabile                                                                          | Non applicabile                                        | Non applicabile                            | Stack di `unordered_map` (`SymbolTable.hpp:65`)          | Uniforme                    |
| **Rappresentazione tipi**           | `TypePtr` ovunque; zonk crea nuove istanze `shared_ptr`        | `TypePtr` per unificazione                                                               | `TypePtr` per bindings                                 | `TypePtr` per body                         | `TypeScheme` wrappa `TypePtr`                            | **Uniforme**                |
| **ErrorType (silent unification)**  | Inserito per undeclared identifiers (`TypeChecker.cpp:506`)    | Unificazione con ErrorType silenziosamente succeeds (`ConstraintSolver.cpp:48–49`)       | Non menzionato esplicitamente nel switch               | Non menzionato                             | Non applicabile                                          | **PARZIALMENTE INCOERENTE** |
| **Gestione CustomType parametrico** | Non gestito                                                    | Non confronta nomi nel caso `default`                                                    | Restituisce tipo invariato nel `case TypeKind::Custom` | Istanziamento incompleto per body composti | Non applicabile                                          | **INCOERENTE**              |

**DEF-005 — Incoerenza nella propagazione degli errori:** Il `TypeChecker` accumula errori in un vettore mutabile (
`errors_`), il `ConstraintSolver` usa `std::expected<T, E>` per segnalazione immediata, e la `SymbolTable` restituisce
`std::nullopt` senza contesto d'errore. Un chiamante della `SymbolTable::lookup` non può distinguere tra "simbolo non
trovato" e "errore interno". La `SymbolTable` dovrebbe restituire `std::expected<TypeScheme, CompileError>` per coerenza
con il resto della pipeline.

**DEF-006 — ErrorType non gestito esplicitamente in Substitution:** Il metodo `Substitution::applyImpl` (
`Substitution.cpp:27–54`) gestisce `TypeVariable`, `ArrayType`, `VectorType`, `CustomType` e `default`, ma non menziona
esplicitamente `TypeKind::Error` nel suo switch. Sebbene il caso `default` restituisca il tipo invariato (corretto per
ErrorType, che è un singleton senza sottostruttura), l'assenza di una gestione esplicita rende il comportamento
dipendente dall'ordine del default branch, vulnerabile a future modifiche.

**DEF-007 — CustomType non confronta nomi nell'unificazione:** Il caso `default` dello switch in
`ConstraintSolver::unify` (`ConstraintSolver.cpp:119–122`) assume che due `CustomType` con lo stesso `kind()` siano
uguali. Ma `CustomType` ha un campo `name_` — due CustomType diversi (es. `Foo` e `Bar`) hanno entrambi
`kind() == TypeKind::Custom` ma nomi diversi. L'unificazione dovrebbe confrontare i nomi.

**DEF-008 — TypeScheme::instantiate incompleto per tipi composti:** Il metodo `TypeScheme::instantiate` (
`TypeScheme.cpp:14–35`) gestisce il caso in cui il body è una TypeVariable diretta, ma restituisce il body invariato per
tipi composti (ArrayType, VectorType, CustomType). Questo significa che un tipo polimorfico con body `Array<T>` non
sostituisce `T` con una variabile fresca.

---

## Phase 2 — Per-System Deep Analysis

### System: TypeChecker (Orchestratore Principale)

#### 2.1 System Overview

Il `TypeChecker` è il sistema centrale dell'intera pipeline di type checking. Implementa un algoritmo constraint-based
Hindley-Milner a 4 fasi: (1) name resolution tramite `SymbolTable`, (2) constraint generation tramite traversata
dell'AST non tipizzato, (3) constraint solving delegato a `ConstraintSolver`, (4) zonking (applicazione della
sostituzione all'AST tipizzato). L'entry point è `TypeChecker::check(const Program&)` che coordina l'intera sequenza.

#### 2.2 Internal Module Organization

Il sistema è organizzato in un singolo header (`TypeChecker.hpp`, 92 righe) e un singolo file di implementazione (
`TypeChecker.cpp`, 1190 righe). L'header espone la classe `TypeChecker`, la struct `TypeCheckResult`, e dichiara i
metodi delle 4 fasi come privati. L'implementazione contiene:

- Funzioni helper statiche (`parse_type_annotation`, `zonk_type`) nelle righe 19–65
- Implementazione di `check()` (entry point) alle righe 68–89
- Fasi 1–4 separate in blocchi commentati (righe 92–187)
- `type_expr()` per constraint generation delle espressioni (righe 389–872, ~480 righe)
- `type_stmt()` per constraint generation delle istruzioni (righe 874–1190, ~316 righe)

**DEF-009** — Violazione soglia complessità cognitiva: Il file `TypeChecker.cpp` (1190 righe) e i metodi `type_expr` (~
480 righe) e `type_stmt` (~316 righe) violano sistematicamente la soglia di complessità cognitiva. La soglia
raccomandata è ≤100 righe per funzione (AGENTS.md §7). La profondità di annidamento in `type_expr` raggiunge 5 livelli (
caso `BinaryExpr` con gestione string/char/numeric/bitwise).

#### 2.3 Intra-System Dependency Analysis

All'interno del sistema `TypeChecker`, le dipendenze sono:

- `check()` → `resolve_names()`, `generate_constraints()`, `solve_constraints()`, `zonk()`
- `resolve_names()` → `resolve_names_stmt()`
- `generate_constraints()` → `type_stmt()`
- `type_stmt()` → `type_expr()` (per espressioni contenute nelle istruzioni)
- `zonk()` → `zonk_stmt_full()` → `zonk_expr_full()`, `zonk_block_full()`

Non esistono dipendenze circolari. Il grafo è un DAG con `check()` come radice e i metodi `type_*` come foglie della
constraint generation.

#### 2.4 Logical Flow

1. **Reset** (`TypeChecker.cpp:69–74`): `check()` azzera `symbols_`, `constraints_`, `errors_`, `message_storage_`,
   `typed_stmts_`.
2. **Name Resolution** (`TypeChecker.cpp:95–147`): `resolve_names(program)` crea lo scope globale, poi per ogni
   statement chiama `resolve_names_stmt()`, che registra funzioni, variabili, e blocchi nella `SymbolTable` con type
   variable fresche.
3. **Constraint Generation** (`TypeChecker.cpp:150–155`): `generate_constraints(program)` itera sugli statement
   chiamando `type_stmt()`, che per ogni nodo AST produce il corrispondente nodo tipizzato e aggiunge vincoli al
   `ConstraintSet`.
4. **Constraint Solving** (`TypeChecker.cpp:158–161`): `solve_constraints()` crea un `ConstraintSolver` e risolve tutti
   i vincoli, producendo una `Substitution`.
5. **Zonking** (`TypeChecker.cpp:164–180`): `zonk(subst)` applica la sostituzione a tutti gli statement tipizzati,
   producendo il `TypedProgram` finale.

#### 2.5 Critical Points

**DEF-010 — Name resolution incompleta per FuncDecl**: In `resolve_names_stmt`, i parametri delle funzioni ricevono type
annotation fresche se non annotate (`TypeChecker.cpp:118`), ma il tipo della funzione stessa è una type variable
fresca (`TypeChecker.cpp:108`) che non viene mai raffinata con la signature effettiva. Durante la constraint generation,
il tipo della funzione non viene collegato ai suoi parametri e tipo di ritorno — il vincolo è solo indiretto attraverso
le chiamate.

**DEF-011 — Gestione MainStmt incompleta nella name resolution**: Il caso `NodeKind::MainStmt` in `resolve_names_stmt` (
`TypeChecker.cpp:124–127`) registra "main" con tipo `void_()`, risolve i nomi nel body del `MainStmt` tramite cast a
`BlockStmt`. Tuttavia, se il body non è un `BlockStmt` (caso raro ma possibile), il fallback
`resolve_names_stmt(ms->body())` (`TypeChecker.cpp:126`) gestisce correttamente il caso. Nessun difetto rilevato qui —
il flusso è corretto.

**DEF-012 — Zonk block silenziosamente scarta statement**: In `zonk_block_full` (`TypeChecker.cpp:370–383`), quando
`zonk_stmt_full` restituisce `nullptr`, il commento dice "Can't move from const — skip (original kept by callee)".
Questo significa che statement non zonkati vengono persi silenziosamente dall'output senza errore. Il blocco risultante
è incompleto e il programma tipizzato perde statement.

**DEF-013 — CallExpr non vincola signature della funzione**: In `type_expr` per `CallExpr` (`TypeChecker.cpp:689–730`),
il callee viene tipato e gli argomenti vengono tipati, ma **non viene generato alcun vincolo** che colleghi il tipo del
callee a una signature di funzione con i tipi degli argomenti e il tipo di ritorno. Il commento alle righe 716–722
ammette esplicitamente: "This is a simplification — a real implementation would use function types". Di conseguenza, le
chiamate a funzione non vengono verificate per arity o tipo degli argomenti. Qualsiasi espressione può essere chiamata
con qualsiasi argomento senza errore.

**DEF-014 — MemberExpr totalmente non implementato**: `type_expr` per `MemberExpr` (`TypeChecker.cpp:843–848`) crea una
type variable fresca per il risultato ma non genera alcun vincolo sull'oggetto o sul membro. Non c'è lookup del membro,
né verifica che l'oggetto abbia quel membro. Qualsiasi accesso `.member` è silenziosamente accettato.

**DEF-015 — CastExpr non genera vincoli di compatibilità**: In `type_expr` per `CastExpr` (`TypeChecker.cpp:850–862`),
il tipo target viene parsato ma non viene generato alcun vincolo tra il tipo dell'operando e il tipo target. Il cast è
quindi una operazione puramente sintattica senza verifica di compatibilità.

**DEF-016 — ArrayLiteral restituisce nullptr su errore**: Quando il primo elemento di un array literal non può essere
tipato, `type_expr` restituisce `nullptr` (`TypeChecker.cpp:739`). Questo propaga `nullptr` attraverso `type_stmt` per
`ExprStmt` che crea un placeholder, ma molti altri punti di chiamata non gestiscono `nullptr` (es.
`typed_init->node_type()` in `VarDecl` alle righe 946).

**DEF-017 — Gestione loop_depth_ non thread-safe**: `loop_depth_` è un campo mutable della classe (`TypeChecker.hpp:89`)
incrementato/decrementato durante la traversata. Se `TypeChecker` venisse usato concorrentemente (non è il caso attuale,
ma l'API non lo proibisce esplicitamente), questo causerebbe data race.

**DEF-018 — type_stmt per multi-var decl semplificato a single-var**: In `type_stmt` per `VarDecl` con multipli nomi (
`TypeChecker.cpp:972–1004`), il commento dice "Multi-variable declaration — simplify: create one TypedVarDecl with first
name". Il codice itera su tutti i nomi e li registra nella SymbolTable correttamente, ma restituisce un singolo
`TypedVarDecl` solo per il primo nome. Gli altri nomi sono persi nell'AST tipizzato.

#### 2.6 Partial or Undefined Implementations

| Metodo/Dichiarazione                | Stato        | Dettaglio                                                                                                                                                                                                                                      |
|-------------------------------------|--------------|------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------|
| `TypeChecker::check`                | **Completo** | Pipeline completa a 4 fasi                                                                                                                                                                                                                     |
| `TypeChecker::resolve_names`        | **Completo** | Crea scope globale, itera statement                                                                                                                                                                                                            |
| `TypeChecker::resolve_names_stmt`   | **Parziale** | Gestisce `FuncDecl`, `MainStmt`, `VarDecl`, `BlockStmt`. Caso `default` fa `break` silenzioso — `ReturnStmt`, `BreakStmt`, `ContinueStmt`, `IfStmt`, `WhileStmt`, `ForStmt` non risolvono nomi nei loro sottostatement (`TypeChecker.cpp:145`) |
| `TypeChecker::generate_constraints` | **Completo** | Delega pura a `type_stmt`                                                                                                                                                                                                                      |
| `TypeChecker::solve_constraints`    | **Completo** | Delega pura a `ConstraintSolver`                                                                                                                                                                                                               |
| `TypeChecker::zonk`                 | **Completo** | Itera `typed_stmts_`, chiama `zonk_stmt_full`                                                                                                                                                                                                  |
| `TypeChecker::zonk_stmt_full`       | **Parziale** | Gestisce 12 `NodeKind`. Caso `default` restituisce `nullptr` (`TypeChecker.cpp:283`)                                                                                                                                                           |
| `TypeChecker::zonk_expr_full`       | **Parziale** | Gestisce 17 `NodeKind`. Caso `default` restituisce `nullptr` (`TypeChecker.cpp:386`)                                                                                                                                                           |
| `TypeChecker::zonk_block_full`      | **Parziale** | Scarta statement che `zonk_stmt_full` restituisce `nullptr` (`TypeChecker.cpp:378`)                                                                                                                                                            |
| `TypeChecker::type_expr`            | **Parziale** | `MemberExpr` non genera vincoli (DEF-014), `CallExpr` non vincola signature (DEF-013), `CastExpr` non genera vincoli (DEF-015)                                                                                                                 |
| `TypeChecker::type_stmt`            | **Parziale** | Multi-var decl semplificato a single-var (DEF-018)                                                                                                                                                                                             |
| `parse_type_annotation` (statica)   | **Parziale** | Gestisce solo primitivi, non tipi composti (`TypeChecker.cpp:19–36`)                                                                                                                                                                           |
| `zonk_type` (statica)               | **Parziale** | Gestisce `TypeVariable`, `ArrayType`, `VectorType`. `CustomType` e `PrimitiveType` restituiscono invariati — corretto ma `CustomType` parametrico futuro richiederebbe ricorsione                                                              |

---

### System: ConstraintSolver (Motore di Unificazione)

#### 2.1 System Overview

Il `ConstraintSolver` implementa l'algoritmo di unificazione basato su union-find. Riceve un `ConstraintSet` e produce
una `Substitution` che mappa type variable a tipi concreti, o errori di unificazione. È il motore inferenziale che
risolve i vincoli generati dalla fase 2.

#### 2.2 Internal Module Organization

Header (`ConstraintSolver.hpp`, 80 righe) e implementazione (`ConstraintSolver.cpp`, 129 righe) compatti. L'header
definisce `SolverResult` e la classe `ConstraintSolver`. L'implementazione contiene `solve()`, `occurs_in()`, e
`unify()`. Design coerente e ben strutturato.

#### 2.3 Intra-System Dependency Analysis

Dipendenze interne: `solve()` → `unify()` per ogni constraint. `unify()` → `occurs_in()` per occurs check. `unify()` →
`UnionFind::make_set`, `UnionFind::unite`. `unify()` → `Substitution::bind`. Nessuna dipendenza circolare.

#### 2.4 Logical Flow

1. `solve()` inizializza `union_find_` e `substitution_` vuoti (`ConstraintSolver.cpp:12–14`)
2. Per ogni constraint nel set, chiama `unify(lhs, rhs, constraint)` (`ConstraintSolver.cpp:16–18`)
3. `unify()` gestisce: ErrorType (successo silenzioso righe 48–49), null type (errore righe 51–54), type variable
   unification con occurs check (righe 57–86), concrete type equality ricorsivo per Array/Vector (righe 100–122)
4. Gli errori sono raccolti in `SolverResult::errors` e restituiti al chiamante

#### 2.5 Critical Points

**DEF-007** (ripetuta dalla §1.4) — Unificazione incompleta per CustomType: Il caso `default` dello switch in
`unify()` (`ConstraintSolver.cpp:119–122`) assume che due `CustomType` con lo stesso `kind()` siano uguali. Due
CustomType diversi (es. `Foo` e `Bar`) si unificano senza errore.

**DEF-019 — Unificazione Array ignora size**: Quando unifica due `ArrayType` (`ConstraintSolver.cpp:109–115`), il solver
unifica solo i tipi elemento, ignorando completamente le espressioni di dimensione. Questo significa che `[i32; 3]` si
unifica con `[i32; 5]` senza errore.

**DEF-020 — occurs_in non gestisce CustomType**: Il metodo `occurs_in` (`ConstraintSolver.cpp:24–44`) ricorre in `Array`
e `Vector` ma non in `CustomType`. Se un CustomType contenesse type variable come parametri generici (estensione
futura), l'occurs check fallirebbe silenziosamente.

**DEF-021 — UnionFind duplica lavoro di Substitution**: Il solver mantiene sia `union_find_` che `substitution_` come
stati paralleli (`ConstraintSolver.hpp:75–76`). Dopo l'unificazione di due type variable, sia `union_find_.unite()` che
`substitution_.bind()` vengono chiamati. Questo è ridondante perché la substitution da sola sarebbe sufficiente per la
fase di zonk. L'UnionFind è usato solo internamente ma il suo mantenimento in parallelo alla Substitution introduce
overhead O(n) non necessario.

#### 2.6 Partial or Undefined Implementations

| Metodo                        | Stato        | Dettaglio                                                                        |
|-------------------------------|--------------|----------------------------------------------------------------------------------|
| `ConstraintSolver::solve`     | **Completo** | Itera constraints, chiama unify                                                  |
| `ConstraintSolver::occurs_in` | **Parziale** | Gestisce TypeVariable, Array, Vector. Non gestisce CustomType parametrico futuro |
| `ConstraintSolver::unify`     | **Parziale** | CustomType non confronta nomi (DEF-007), Array ignora size (DEF-019)             |

---

### System: Constraint (Rappresentazione Vincoli)

#### 2.1 System Overview

Il sistema `Constraint` fornisce la struttura dati per rappresentare vincoli di uguaglianza tra tipi (`lhs = rhs`) e il
contenitore `ConstraintSet` per accumularli durante la constraint generation.

#### 2.2 Internal Module Organization

Header (`Constraint.hpp`) definisce `ConstraintId`, `Constraint` (struct), e `ConstraintSet` (classe). Implementazione (
`Constraint.cpp`) con metodi inline-like. Design minimale e coerente.

#### 2.3 Intra-System Dependency Analysis

Nessuna dipendenza interna. `ConstraintSet` dipende solo da STL (`std::vector`, `std::ranges`).

#### 2.4 Logical Flow

1. `add()` crea un nuovo `Constraint` con ID incrementale e lo aggiunge al vettore interno (`Constraint.cpp:10–14`)
2. `constraints()` restituisce riferimento const al vettore (`Constraint.cpp:16`)
3. `get()` cerca linearmente per ID (`Constraint.cpp:18–21`)
4. `size()` restituisce la cardinalità (`Constraint.cpp:23`)

#### 2.5 Critical Points

**DEF-022 — Lookup lineare O(n) per ID**: `ConstraintSet::get()` usa `std::ranges::find` con complessità O(n) (
`Constraint.cpp:19`). Se il numero di vincoli cresce (centinaia/migliaia per programmi grandi), questo diventa un collo
di bottiglia. Un `std::unordered_map<ConstraintId, Constraint>` o un accesso diretto per indice sarebbe più efficiente.

**DEF-023 — Nessun metodo di rimozione**: `ConstraintSet` non supporta la rimozione di vincoli risolti. Se un sistema
futuro volesse implementare constraint solving incrementale o backtracking, l'API sarebbe insufficiente.

#### 2.6 Partial or Undefined Implementations

Tutti i metodi dichiarati sono implementati. Nessuna funzione stub.

---

### System: Substitution (Mappatura Variabili→Tipi)

#### 2.1 System Overview

Il sistema `Substitution` gestisce la mappatura da type variable a tipi concreti. Viene prodotto dal `ConstraintSolver`
e consumato dalla fase di zonking del `TypeChecker` per risolvere tutti i type variable nell'AST tipizzato.

#### 2.2 Internal Module Organization

Header (`Substitution.hpp`, 112 righe) e implementazione (`Substitution.cpp`, 57 righe). L'header documenta
estensivamente il caching persistente (`apply_cache_`). Design pulito.

#### 2.3 Intra-System Dependency Analysis

Nessuna dipendenza interna oltre a `TypePtr` e `TypeVariable`. `applyImpl` è il worker ricorsivo chiamato da `apply`.

#### 2.4 Logical Flow

1. `bind()` registra un'associazione e invalida il cache (`Substitution.cpp:10–13`)
2. `apply()` delega a `applyImpl` (`Substitution.cpp:25`)
3. `applyImpl()` controlla il cache, se miss risolve ricorsivamente: TypeVariable → lookup bindings, Array/Vector →
   ricorsione su elemento, CustomType/default → invariato (`Substitution.cpp:27–54`)
4. Il risultato viene memorizzato nel cache prima del ritorno

#### 2.5 Critical Points

**DEF-006** (ripetuta dalla §1.4) — ErrorType non gestito esplicitamente: Lo switch in `applyImpl` non ha un
`case TypeKind::Error`, affidandosi al `default`. Corretto ma fragile.

**DEF-024 — Cache non thread-safe**: Il campo `apply_cache_` è `mutable` e viene modificato da `applyImpl` durante
chiamate a `apply`. Se lo stesso oggetto `Substitution` venisse usato concorrentemente da più thread (possibile durante
zonking parallelo), si verificherebbe una data race su `apply_cache_`.

#### 2.6 Partial or Undefined Implementations

Tutti i metodi dichiarati sono implementati. `applyImpl` gestisce i tipi correntemente supportati ma è parziale per
estensioni future (CustomType parametrico).

---

### System: SymbolTable (Gestione Scope e Simboli)

#### 2.1 System Overview

Il sistema `SymbolTable` gestisce la mappa da identificatori a `TypeScheme` con supporto per scope annidati. Implementa
lo shadowing: binding di scope interni nascondono quelli di scope esterni con lo stesso nome.

#### 2.2 Internal Module Organization

Header (`SymbolTable.hpp`, 72 righe) e implementazione (`SymbolTable.cpp`, 34 righe). Design compatto e coerente. Usa
`std::vector<std::unordered_map<...>>` per lo stack di scope.

#### 2.3 Intra-System Dependency Analysis

Nessuna dipendenza interna oltre a `TypeScheme`.

#### 2.4 Logical Flow

1. `push_scope()` crea un nuovo `unordered_map` vuoto e lo pusha (`SymbolTable.cpp:10`)
2. `define()` inserisce o aggiorna nel back scope (`SymbolTable.cpp:15–18`)
3. `lookup()` cerca dall'inner all'outer scope (`SymbolTable.cpp:21–27`)
4. `pop_scope()` rimuove lo scope più interno (`SymbolTable.cpp:12–14`)

#### 2.5 Critical Points

**DEF-005** (ripetuta dalla §1.4) — `lookup` restituisce `std::nullopt` senza contesto: Il chiamante non può distinguere
tra "simbolo non trovato" e un eventuale errore futuro.

**DEF-025 — `pop_scope` non verifica precondizione**: `pop_scope` controlla `if(!scopes_.empty())` (
`SymbolTable.cpp:13`) ma non segnala errore se lo stack è vuoto. Silenziosamente non fa nulla. Il commento nell'header
dice `@pre depth() > 0` ma l'implementazione non enforcement.

#### 2.6 Partial or Undefined Implementations

Tutti i metodi dichiarati sono implementati e completi.

---

### System: TypeScheme (Tipi Polimorfici)

#### 2.1 System Overview

Il sistema `TypeScheme` rappresenta tipi polimorfici quantificati universalmente (∀T. body). Supporta l'istanziazione
con type variable fresche per l'uso durante l'inferenza.

#### 2.2 Internal Module Organization

Header (`TypeScheme.hpp`, 56 righe) e implementazione (`TypeScheme.cpp`, 37 righe). Struct semplice con metodo
`instantiate()` e factory `mono()`.

#### 2.3 Intra-System Dependency Analysis

Dipende da `TypePtr` e `TypeVariable`. `instantiate()` chiama `fresh_type_variable()`.

#### 2.4 Logical Flow

1. `mono()` crea uno scheme monomorfico senza variabili quantificate (`TypeScheme.cpp:10`)
2. `instantiate()` genera fresh type variables per ogni variabile quantificata e le sostituisce nel body (
   `TypeScheme.cpp:14–35`)

#### 2.5 Critical Points

**DEF-008** (ripetuta dalla §1.4) — Istanziamento incompleto per tipi composti: `TypeScheme::instantiate` gestisce solo
il caso in cui il body è una TypeVariable diretta. Per ArrayType, VectorType, CustomType, restituisce il body invariato
senza sostituire le variabili quantificate al suo interno. Questo rompe l'inferenza polimorfica per funzioni che
restituiscono tipi composti contenenti variabili quantificate.

#### 2.6 Partial or Undefined Implementations

| Metodo                    | Stato        | Dettaglio                                                                                 |
|---------------------------|--------------|-------------------------------------------------------------------------------------------|
| `TypeScheme::mono`        | **Completo** | Factory per scheme monomorfico                                                            |
| `TypeScheme::instantiate` | **Parziale** | Gestisce solo TypeVariable diretta, non tipi composti con variabili quantificate annidate |

---

### System: TypeVariable (Variabili di Tipo)

#### 2.1 System Overview

Il sistema `TypeVariable` rappresenta le variabili di tipo (?T1, ?T2, ...) usate durante l'inferenza. Fornisce un
generatore di ID univoci thread-local.

#### 2.2 Internal Module Organization

Header (`TypeVariable.hpp`, 82 righe) e implementazione (`TypeVariable.cpp`, 18 righe). Classe `TypeVariable` estende
`TypeBase`. Funzione `fresh_type_variable()` come factory.

#### 2.3 Intra-System Dependency Analysis

Dipende da `TypeBase` (da `Type.hpp`). `fresh_type_variable()` usa un counter `thread_local`.

#### 2.4 Logical Flow

1. `fresh_type_variable()` incrementa il counter thread-local e crea una nuova `TypeVariable` (`TypeVariable.cpp:17`)
2. `to_string()` formatta come `?T{id}` (`TypeVariable.cpp:10`)

#### 2.5 Critical Points

Nessun punto critico rilevato. Il sistema è minimale e correttamente implementato. Il counter `thread_local` garantisce
unicità per-thread.

#### 2.6 Partial or Undefined Implementations

Tutti i metodi dichiarati sono implementati e completi.

---

### System: UnionFind (Struttura Disjoint-Set)

#### 2.1 System Overview

Il sistema `UnionFind` implementa la struttura dati disjoint-set con path compression e union by rank per unificazione
efficiente O(α(n)).

#### 2.2 Internal Module Organization

Header (`UnionFind.hpp`, 52 righe) e implementazione (`UnionFind.cpp`, 40 righe). Design standard con due mappe (
`parent_`, `rank_`).

#### 2.3 Intra-System Dependency Analysis

Nessuna dipendenza interna al typechecker. Solo STL (`unordered_map`, `uint8_t`).

#### 2.4 Logical Flow

1. `make_set()` crea un singleton set (`UnionFind.cpp:10–13`)
2. `find()` applica path compression ricorsivo (`UnionFind.cpp:15–19`)
3. `unite()` unisce per rank (`UnionFind.cpp:21–33`)
4. `same_set()` confronta rappresentanti (`UnionFind.cpp:35–38`)

#### 2.5 Critical Points

**DEF-026 — `find()` usa `at()` con eccezioni**: `UnionFind::find` usa `parent_.at(var)` (`UnionFind.cpp:16`) che lancia
`std::out_of_range` se `var` non è stato registrato con `make_set`. Questo è un comportamento non documentato — il
chiamante deve garantire che `var` esista. In pratica, `ConstraintSolver::unify` chiama `make_set` prima di `find`,
quindi è sicuro, ma l'API non garantisce questa precondizione.

#### 2.6 Partial or Undefined Implementations

Tutti i metodi dichiarati sono implementati e completi.

---

### System: ErrorType (Tipo Errore per Error Recovery)

#### 2.1 System Overview

Il sistema `ErrorType` fornisce un tipo placeholder che si unifica silenziosamente con qualsiasi altro tipo, prevenendo
errori a cascata da una singola causa radice.

#### 2.2 Internal Module Organization

Header (`ErrorType.hpp`, 40 righe) e implementazione (`ErrorType.cpp`, 14 righe). Singleton tramite `error_type()`.

#### 2.3 Intra-System Dependency Analysis

Dipende da `TypeBase` (da `Type.hpp`).

#### 2.4 Logical Flow

1. `error_type()` restituisce un'istanza singleton condivisa (`ErrorType.cpp:10–13`)
2. `to_string()` restituisce `"<error>"` (`ErrorType.cpp:6`)
3. `operator==` confronta il `kind()` — tutti gli ErrorType sono uguali tra loro

#### 2.5 Critical Points

Nessun punto critico. Il design singleton è appropriato per un tipo sentinella.

#### 2.6 Partial or Undefined Implementations

Tutti i metodi dichiarati sono implementati e completi.

---

## Phase 3 — Per-Component Analysis

### System: TypeChecker › Component: TypeChecker (classe principale)

#### 3.1 Responsibility Statement

Il componente `TypeChecker` è esclusivamente responsabile per l'orchestrazione della pipeline di type checking: name
resolution, constraint generation, constraint solving e zonking di un programma AST da non tipato a completamente
tipizzato.

#### 3.2 Class Structure

| Membro                          | Tipo                         | Visibilità | Semantica                                                                                  |
|---------------------------------|------------------------------|------------|--------------------------------------------------------------------------------------------|
| `symbols_`                      | `SymbolTable`                | `private`  | Tabella dei simboli con scope annidati                                                     |
| `constraints_`                  | `ConstraintSet`              | `private`  | Insieme di vincoli di tipo accumulati                                                      |
| `errors_`                       | `std::vector<CompileError>`  | `private`  | Errori di tipo raccolti durante il checking                                                |
| `message_storage_`              | `std::deque<std::string>`    | `private`  | Proprietario delle stringhe per `CompileError::message_` (previene dangling `string_view`) |
| `typed_stmts_`                  | `std::vector<TypedStmtPtr>`  | `private`  | Statement tipizzati prodotti durante constraint generation                                 |
| `current_function_return_type_` | `std::optional<TypePtr>`     | `private`  | Tipo di ritorno atteso della funzione corrente                                             |
| `current_function_name_`        | `std::optional<std::string>` | `private`  | Nome della funzione corrente per messaggi d'errore                                         |
| `loop_depth_`                   | `std::size_t`                | `private`  | Profondità di annidamento nei loop (per validazione break/continue)                        |

**Ereditarietà**: `TypeChecker` non eredita da nessuna classe base. Composizione con tutti i sistemi del typechecker.

#### 3.3 Interface Analysis

| Metodo      | Signature                                                     | Precondizioni                                              | Postcondizioni                      | Contratto                                            |
|-------------|---------------------------------------------------------------|------------------------------------------------------------|-------------------------------------|------------------------------------------------------|
| `check`     | `[[nodiscard]] TypeCheckResult check(const Program &program)` | `program` valido                                           | Restituisce `TypedProgram` + errori | Esegue l'intera pipeline 4 fasi                      |
| `type_expr` | `[[nodiscard]] TypedExprPtr type_expr(const Expr &expr)`      | `expr` valido; name resolution già eseguita per identifier | Restituisce `TypedExpr` o `nullptr` | Genera vincoli per un'espressione (esposto per test) |
| `type_stmt` | `[[nodiscard]] TypedStmtPtr type_stmt(const Stmt &stmt)`      | `stmt` valido; name resolution già eseguita                | Restituisce `TypedStmt`             | Genera vincoli per uno statement (esposto per test)  |

#### 3.4 Implementation Logic

Il metodo `check` (`TypeChecker.cpp:68–89`) esegue sequenzialmente: reset stato, resolve_names, generate_constraints,
solve_constraints, zonk. Ogni fase produce output consumato dalla successiva. Gli errori del solver vengono aggiunti al
vettore `errors_`.

Il metodo `type_expr` (`TypeChecker.cpp:389–872`) è un grande switch su `expr.kind()`. Ogni caso: (a) tipa i
sotto-espressioni ricorsivamente, (b) genera vincoli tramite `constraints_.add()`, (c) controlla condizioni d'errore
immediate (es. tipo non numerico per operatori aritmetici), (d) costruisce il nodo tipizzato corrispondente. I casi
`BinaryExpr` (righe 497–678) sono i più complessi con gestione separata per operatori aritmetici, logici, bitwise, e
string/char concatenation.

Il metodo `type_stmt` (`TypeChecker.cpp:915–1190`) segue lo stesso pattern: switch su `stmt.kind()`, tipa
sotto-elementi, genera vincoli, gestisce errori, costruisce statement tipizzato. I casi `FuncDecl` e `ReturnStmt`
gestiscono lo stack `current_function_return_type_` per validare i return.

#### 3.5 Error Handling Evaluation

La gestione errori nel `TypeChecker` è **basata su accumulo**: ogni errore rilevato viene pushato in `errors_` e la
procedura continua (error recovery). Questo è coerente con l'obiettivo di un type checker che riporta tutti gli errori
in una sola passata.

**Errori rilevati**: undeclared identifiers (E2033), type mismatch per operatori aritmetici (E2013), type mismatch per
operatori logici (E2012), type mismatch per operatori bitwise (E2011), return type mismatch (E2007), return da void
function (E2006), break/continue fuori loop (E2009, E2010), array element type mismatch (E2021), array vuoto (E2020),
const assignment (E2024), index non-integer (E2030), non-array indexing (E2031), negazione non numerica (E2018), logical
not non boolean (E2019).

**Errori NON rilevati**: chiamata a funzione con arity sbagliata (DEF-013), accesso a membro inesistente (DEF-014), cast
incompatibile (DEF-015), statement persi durante zonking (DEF-012). Questi sono silenzi accettati come limitazioni
dell'implementazione corrente.

#### 3.6 Type Consistency Audit

I tipi sono usati coerentemente: `TypePtr` (shared_ptr<const TypeBase>) ovunque. Non ci sono cast impliciti pericolosi.
I `static_cast<const TypeDecl *>(&stmt)` sono sicuri perché preceduti da check su `stmt.kind()`. L'uso di `dynamic_cast`
è presente in `zonk_type` (`TypeChecker.cpp:41–59`) ma è accettabile per il codice di zonking che opera su tipi noti.

**DEF-027 — Cast non verificati in `type_expr`**: In alcuni casi, `static_cast` viene usato dopo aver già creato il nodo
tipizzato. Es. `const auto *ident = static_cast<const TypedIdentifier *>(target_typed.get())` (`TypeChecker.cpp:810`) —
se `target_typed` non è un `TypedIdentifier`, il cast è undefined behavior. Un `dynamic_cast` con check nullo sarebbe
più sicuro.

#### 3.7 Inter-Component Interaction

Il `TypeChecker` interagisce con:

- **SymbolTable**: per lookup e definizione di simboli durante name resolution e constraint generation
- **ConstraintSet**: per accumulo vincoli durante constraint generation
- **ConstraintSolver**: per risoluzione vincoli (delega pura)
- **Substitution**: consumata durante zonking
- **ErrorType**: inserito come placeholder per undeclared identifiers
- **TypeVariable**: generato per espressioni senza tipo esplicito

L'accoppiamento è diretto — il `TypeChecker` conosce le API interne di tutti questi componenti. Questo è inevitabile per
un orchestratore ma rende il `TypeChecker` un "God class" che centralizza troppe responsabilità.

#### 3.8 Optimization Opportunities

1. **DEF-009** (ripetuta) — `type_expr` (480 righe) e `type_stmt` (316 righe) dovrebbero essere suddivisi in funzioni
   più piccole per caso o per famiglia di casi (es. `type_binary_expr`, `type_call_expr`, `type_var_decl`,
   `type_func_decl`).
2. **DEF-012** (ripetuta) — `zonk_block_full` dovrebbe preservare statement non zonkati invece di scartarli
   silenziosamente.
3. **DEF-028 — `message_storage_` cresce indefinitamente**: Il `deque` delle stringhe d'errore non viene mai compattato.
   Per programmi con migliaia di errori, la memoria cresce proporzionalmente.
4. **DEF-029 — `parse_type_annotation` non cacheata**: La funzione viene chiamata ripetutamente per le stesse
   annotazioni. Una cache `unordered_map<string_view, TypePtr>` eliminerebbe le ripetizioni.

---

### System: TypeChecker › Component: resolve_names_stmt

#### 3.1 Responsibility Statement

Il componente `resolve_names_stmt` è esclusivamente responsabile per la registrazione di dichiarazioni (funzioni,
variabili, blocchi) nella `SymbolTable` durante la fase di name resolution, senza tipizzazione.

#### 3.2 Class Structure

Non è una classe separata ma un metodo privato di `TypeChecker`. Opera sui campi `symbols_` del `TypeChecker`.

#### 3.3 Interface Analysis

| Metodo               | Signature                                   | Precondizioni                         | Postcondizioni                 |
|----------------------|---------------------------------------------|---------------------------------------|--------------------------------|
| `resolve_names_stmt` | `void resolve_names_stmt(const Stmt &stmt)` | `symbols_` ha almeno uno scope attivo | Simboli definiti in `symbols_` |

#### 3.4 Implementation Logic

Switch su `stmt.kind()`. Per `FuncDecl`: registra funzione con type variable fresca, pusha scope, registra parametri,
risolve body, poppa scope. Per `MainStmt`: registra "main", pusha scope, risolve body, poppa. Per `VarDecl`: registra
ogni nome con type variable fresca. Per `BlockStmt`: pusha/poppa scope risolvendo statement interni. Default: break
silenzioso.

#### 3.5 Error Handling Evaluation

Nessun errore riportato — la name resolution è silenziosa sui fallimenti. Se un simbolo è già definito (shadowing),
viene sovrascritto senza warning. Questo è corretto per il language design ma potrebbe nascondere bug dell'utente (
ridefinizione accidentale).

#### 3.6 Type Consistency Audit

Tipi usati correttamente. Le type variable fresche sono generate con `fresh_type_variable()`.

#### 3.7 Inter-Component Interaction

Dipende da `SymbolTable::define`, `SymbolTable::push_scope`, `SymbolTable::pop_scope`. Dipende da
`fresh_type_variable()`.

#### 3.8 Optimization Opportunities

**DEF-030 — Name resolution parziale**: `resolve_names_stmt` non gestisce `IfStmt`, `WhileStmt`, `ForStmt`,
`ReturnStmt`, `BreakStmt`, `ContinueStmt`. Per `IfStmt` e `WhileStmt`, questo significa che le variabili dichiarate nei
loro body non vengono registrate prima della constraint generation, producendo falsi positivi "Undeclared identifier".
Questo è un bug funzionale, non solo una limitazione.

---

### System: TypeChecker › Component: zonk_stmt_full / zonk_expr_full / zonk_block_full

#### 3.1 Responsibility Statement

Questi tre componenti sono esclusivamente responsabili per l'applicazione della sostituzione (zonking) all'AST
tipizzato, producendo il programma finale con tipi concreti.

#### 3.2 Class Structure

Metodi privati di `TypeChecker`. Operano su `TypedStmt`, `TypedExpr`, `TypedBlockStmt` (tipi dell'AST tipizzato).

#### 3.3 Interface Analysis

| Metodo            | Signature                                                                                                               | Precondizioni                  | Postcondizioni                              |
|-------------------|-------------------------------------------------------------------------------------------------------------------------|--------------------------------|---------------------------------------------|
| `zonk_stmt_full`  | `[[nodiscard]] TypedStmtPtr zonk_stmt_full(const Substitution &subst, const TypedStmt &stmt)`                           | `stmt` valido, `subst` valido  | Restituisce statement zonkato o `nullptr`   |
| `zonk_expr_full`  | `[[nodiscard]] TypedExprPtr zonk_expr_full(const Substitution &subst, const TypedExpr &expr)`                           | `expr` valido, `subst` valido  | Restituisce espressione zonkata o `nullptr` |
| `zonk_block_full` | `[[nodiscard]] std::unique_ptr<TypedBlockStmt> zonk_block_full(const Substitution &subst, const TypedBlockStmt &block)` | `block` valido, `subst` valido | Restituisce blocco zonkato                  |

#### 3.4 Implementation Logic

Ogni metodo è uno switch su `kind()` che: (a) chiama `zonk_type` sul tipo del nodo, (b) zonka ricorsivamente i figli, (
c) ricostruisce il nodo tipizzato con i figli zonkati e il tipo risolto. `zonk_block_full` itera sugli statement del
blocco e chiama `zonk_stmt_full` per ciascuno, scartando i risultati `nullptr`.

#### 3.5 Error Handling Evaluation

Gli errori sono silenziosi: `nullptr` viene restituito per casi `default` non gestiti e propagato senza diagnostica.
`zonk_block_full` scarta statement non zonkati (DEF-012).

#### 3.6 Type Consistency Audit

`zonk_type` (`TypeChecker.cpp:41–65`) usa `dynamic_cast` per discriminare i tipi. Questo è meno efficiente di uno switch
su `kind()` ma è corretto. Per coerenza con il resto del codebase che usa `static_cast` dopo `kind()`, `zonk_type`
dovrebbe usare lo stesso pattern.

#### 3.7 Inter-Component Interaction

Dipende da `Substitution::lookup` e `Substitution::apply` (indirettamente tramite `zonk_type`). Ricostruisce nodi
dell'AST tipizzato usando i constructor dei tipi `Typed*`.

#### 3.8 Optimization Opportunities

1. **DEF-031 — `zonk_type` duplica logica di `Substitution::apply`**: Entrambi ricorrono su Array/Vector e risolvono
   TypeVariable. `zonk_type` dovrebbe delegare a `Substitution::apply` invece di reimplementare la ricorsione.
2. **DEF-012** (ripetuta) — `zonk_block_full` dovrebbe preservare statement originali quando `zonk_stmt_full`
   restituisce `nullptr`.

---

### System: ConstraintSolver › Component: ConstraintSolver (classe principale)

#### 3.1 Responsibility Statement

Il componente `ConstraintSolver` è esclusivamente responsabile per la risoluzione di vincoli di uguaglianza tra tipi
tramite unificazione union-find, producendo una `Substitution` o errori.

#### 3.2 Class Structure

| Membro          | Tipo           | Visibilità | Semantica                                          |
|-----------------|----------------|------------|----------------------------------------------------|
| `union_find_`   | `UnionFind`    | `private`  | Struttura disjoint-set per unificazione efficiente |
| `substitution_` | `Substitution` | `private`  | Mappatura type variable → tipo concreto            |

**Ereditarietà**: Nessuna.

#### 3.3 Interface Analysis

| Metodo      | Signature                                                                                                                   | Precondizioni  | Postcondizioni                    |
|-------------|-----------------------------------------------------------------------------------------------------------------------------|----------------|-----------------------------------|
| `solve`     | `[[nodiscard]] SolverResult solve(const ConstraintSet &constraints)`                                                        | Vincoli validi | Restituisce substitution o errori |
| `occurs_in` | `[[nodiscard]] static bool occurs_in(TypeVarId var, const TypePtr &type, const Substitution &subst)`                        | `type` valido  | true se `var` occorre in `type`   |
| `unify`     | `[[nodiscard]] std::expected<void, CompileError> unify(const TypePtr &t1, const TypePtr &t2, const Constraint &constraint)` | Tipi validi    | Unifica o restituisce errore      |

#### 3.4 Implementation Logic

`solve()` itera sui vincoli e chiama `unify()`. `unify()` gestisce: ErrorType (successo), null type (errore),
TypeVariable unification (con occurs check), concrete type equality (ricorsivo per Array/Vector, default per
CustomType). L'occurs check previene tipi infiniti ricorsivi.

#### 3.5 Error Handling Evaluation

Usa `std::expected<void, CompileError>` per propagazione immediata. ErrorType unifica silenziosamente con qualsiasi
tipo (corretto per error recovery). Null type genera errore E2034. Occurs check fallito genera E2035. Type mismatch
genera E2034 con hint per cast.

#### 3.6 Type Consistency Audit

Tipi usati coerentemente. `dynamic_cast` per discriminare TypeVariable — coerente con il resto del typechecker.

#### 3.7 Inter-Component Interaction

Dipende da `UnionFind` (make_set, unite), `Substitution` (bind), `ErrorType` (singleton), `CompileError` (factory).

#### 3.8 Optimization Opportunities

**DEF-021** (ripetuta) — Mantenimento parallelo di UnionFind e Substitution. Si potrebbe eliminare UnionFind e usare
solo Substitution con find/union implementati sulla substitution stessa.

---

### System: Constraint › Component: ConstraintSet

#### 3.1 Responsibility Statement

Il componente `ConstraintSet` è esclusivamente responsabile per l'accumulo sequenziale di vincoli di uguaglianza tra
tipi con identificatori univoci.

#### 3.2 Class Structure

| Membro         | Tipo                      | Visibilità | Semantica                              |
|----------------|---------------------------|------------|----------------------------------------|
| `constraints_` | `std::vector<Constraint>` | `private`  | Contenitore ordinato di vincoli        |
| `next_id_`     | `ConstraintId`            | `private`  | Contatore ID incrementale (parte da 1) |

#### 3.3 Interface Analysis

| Metodo        | Signature                                                                                | Precondizioni | Postcondizioni                 |
|---------------|------------------------------------------------------------------------------------------|---------------|--------------------------------|
| `add`         | `ConstraintId add(TypePtr lhs, TypePtr rhs, SourceSpan origin, std::string_view reason)` | Tipi validi   | Restituisce nuovo ID           |
| `constraints` | `[[nodiscard]] const std::vector<Constraint> &constraints() const noexcept`              | Nessuna       | Riferimento al vettore interno |
| `get`         | `[[nodiscard]] const Constraint *get(ConstraintId id) const noexcept`                    | Nessuna       | Puntatore al vincolo o nullptr |
| `size`        | `[[nodiscard]] std::size_t size() const noexcept`                                        | Nessuna       | Numero di vincoli              |

#### 3.4 Implementation Logic

Implementazione diretta: `add` pusha nel vettore, `get` cerca linearmente, `size` delega a `vector::size`.

#### 3.5 Error Handling Evaluation

Nessun errore — il contenitore non può fallire (eccetto `bad_alloc` per memoria).

#### 3.6 Type Consistency Audit

Coerente. `TypePtr` e `SourceSpan` usati correttamente.

#### 3.7 Inter-Component Interaction

Dipende solo da STL e `TypePtr`. Consumato da `ConstraintSolver::solve`.

#### 3.8 Optimization Opportunities

**DEF-022** (ripetuta) — Lookup O(n) in `get()`. Per grandi constraint set, un `unordered_map` o accesso diretto per
indice (`constraints_[id-1]`) sarebbe O(1).

---

### System: Substitution › Component: Substitution (classe principale)

#### 3.1 Responsibility Statement

Il componente `Substitution` è esclusivamente responsabile per la gestione della mappatura da type variable a tipi
concreti e la loro applicazione ricorsiva a tipi composti.

#### 3.2 Class Structure

| Membro         | Tipo                                                    | Visibilità | Semantica                    |
|----------------|---------------------------------------------------------|------------|------------------------------|
| `bindings_`    | `std::unordered_map<TypeVarId, TypePtr>`                | `private`  | Mappa type variable → tipo   |
| `apply_cache_` | `mutable std::unordered_map<const TypeBase *, TypePtr>` | `private`  | Cache dei risultati di apply |

#### 3.3 Interface Analysis

| Metodo     | Signature                                                                   | Precondizioni | Postcondizioni                            |
|------------|-----------------------------------------------------------------------------|---------------|-------------------------------------------|
| `bind`     | `void bind(TypeVarId var, TypePtr type)`                                    | `type` valido | Associazione registrata, cache invalidata |
| `lookup`   | `[[nodiscard]] std::optional<TypePtr> lookup(TypeVarId var) const noexcept` | Nessuna       | Tipo associato o nullopt                  |
| `apply`    | `[[nodiscard]] TypePtr apply(const TypePtr &type) const`                    | `type` valido | Tipo con variabili risolte                |
| `contains` | `[[nodiscard]] bool contains(TypeVarId var) const noexcept`                 | Nessuna       | true se vincolato                         |
| `size`     | `[[nodiscard]] std::size_t size() const noexcept`                           | Nessuna       | Numero di binding                         |

#### 3.4 Implementation Logic

`bind()` invalida il cache e aggiorna `bindings_`. `apply()` delega a `applyImpl()`. `applyImpl()` controlla il cache,
se miss risolve: TypeVariable → lookup e ricorsione, Array/Vector → ricorsione su elemento, altro → invariato. Cache
populate bottom-up.

#### 3.5 Error Handling Evaluation

Nessun errore — la sostituzione è una mappa totale. Type variable non associate restano invariate.

#### 3.6 Type Consistency Audit

Coerente. `TypePtr` immutabile (`shared_ptr<const TypeBase>`) garantisce che il cache keyed su puntatore grezzo sia
sicuro.

#### 3.7 Inter-Component Interaction

Dipende da `TypePtr`, `TypeVariable`. Consumato da `ConstraintSolver` e `TypeChecker::zonk`.

#### 3.8 Optimization Opportunities

**DEF-024** (ripetuta) — Cache non thread-safe. Se necessario parallelismo, servirebbe un mutex o una cache
thread-local.

---

### System: SymbolTable › Component: SymbolTable (classe principale)

#### 3.1 Responsibility Statement

Il componente `SymbolTable` è esclusivamente responsabile per la gestione di scope annidati e la mappatura da
identificatori a `TypeScheme` con supporto per shadowing.

#### 3.2 Class Structure

| Membro    | Tipo                                                                                         | Visibilità | Semantica      |
|-----------|----------------------------------------------------------------------------------------------|------------|----------------|
| `scopes_` | `std::vector<std::unordered_map<std::string_view, TypeScheme, StringHash, std::equal_to<>>>` | `private`  | Stack di scope |

**StringHash**: hasher eterogeneo che accetta sia `std::string` che `std::string_view` come chiave, evitando allocazioni
per lookup.

#### 3.3 Interface Analysis

| Metodo                     | Signature                                                                     | Precondizioni                    | Postcondizioni                         |
|----------------------------|-------------------------------------------------------------------------------|----------------------------------|----------------------------------------|
| `push_scope`               | `void push_scope()`                                                           | Nessuna                          | Nuovo scope vuoto creato               |
| `pop_scope`                | `void pop_scope()`                                                            | Nessuna (documentata: depth > 0) | Scope rimosso silenziosamente se vuoto |
| `define`                   | `void define(std::string_view name, TypeScheme scheme)`                       | Nessuna (documentata: depth > 0) | Simbolo definito nello scope corrente  |
| `lookup`                   | `[[nodiscard]] std::optional<TypeScheme> lookup(std::string_view name) const` | Nessuna                          | Schema trovato o nullopt               |
| `defined_in_current_scope` | `[[nodiscard]] bool defined_in_current_scope(std::string_view name) const`    | Nessuna                          | true se definito nello scope corrente  |
| `depth`                    | `[[nodiscard]] std::size_t depth() const noexcept`                            | Nessuna                          | Profondità stack                       |

#### 3.4 Implementation Logic

Implementazione diretta. `lookup` itera in reverse (inner→outer). `define` usa `insert_or_assign`.

#### 3.5 Error Handling Evaluation

**DEF-025** (ripetuta) — `pop_scope` e `define` non enforcement delle precondizioni. Se chiamate con stack vuoto,
`pop_scope` non fa nulla, `define` crea implicitamente uno scope (`if(scopes_.empty()) scopes_.emplace_back()`).

#### 3.6 Type Consistency Audit

Coerente. `std::string_view` come chiave richiede attenzione alla lifetime delle stringhe — il chiamante deve garantire
che le stringhe degli identificatori sopravvivano nello scope.

#### 3.7 Inter-Component Interaction

Dipende da `TypeScheme`. Consumato da `TypeChecker::resolve_names` e `TypeChecker::type_expr` (per lookup identifier).

#### 3.8 Optimization Opportunities

Design già efficiente. L'hasher eterogeneo `StringHash` è un'ottima scelta per evitare allocazioni.

---

### System: TypeScheme › Component: TypeScheme (struct)

#### 3.1 Responsibility Statement

Il componente `TypeScheme` è esclusivamente responsabile per la rappresentazione di tipi polimorfici quantificati
universalmente e la loro istanziazione con variabili fresche.

#### 3.2 Class Structure

| Campo             | Tipo                     | Visibilità | Semantica                       |
|-------------------|--------------------------|------------|---------------------------------|
| `quantified_vars` | `std::vector<TypeVarId>` | `public`   | ID delle variabili quantificate |
| `body`            | `TypePtr`                | `public`   | Tipo corpo dello scheme         |
| `is_const`        | `bool`                   | `public`   | Flag di immutabilità            |

#### 3.3 Interface Analysis

| Metodo        | Signature                                                                     | Precondizioni | Postcondizioni                        |
|---------------|-------------------------------------------------------------------------------|---------------|---------------------------------------|
| `mono`        | `[[nodiscard]] static TypeScheme mono(TypePtr type, bool const_flag = false)` | `type` valido | Scheme monomorfico                    |
| `instantiate` | `[[nodiscard]] TypePtr instantiate() const`                                   | Nessuna       | Tipo istanziato con variabili fresche |

#### 3.4 Implementation Logic

`mono()` è una factory. `instantiate()` genera fresh variables per ogni quantified_var, poi sostituisce nel body.
Attualmente gestisce solo TypeVariable diretta.

#### 3.5 Error Handling Evaluation

Nessun errore — l'istanziazione è sempre sicura.

#### 3.6 Type Consistency Audit

**DEF-008** (ripetuta) — Per body composti, le variabili quantificate annidate non vengono sostituite.

#### 3.7 Inter-Component Interaction

Dipende da `TypePtr`, `TypeVariable`, `fresh_type_variable()`. Consumato da `SymbolTable` (come valore) e
`TypeChecker::type_expr` (per instantiate su identifier).

#### 3.8 Optimization Opportunities

**DEF-032 — Serve un visitor per sostituire variabili in tipi composti**: `instantiate()` avrebbe bisogno di un visitor
che traversa ArrayType, VectorType, e futuri tipi generici, sostituendo TypeVariable con ID matching `quantified_vars`.

---

### System: TypeVariable › Component: TypeVariable (classe)

#### 3.1 Responsibility Statement

Il componente `TypeVariable` è esclusivamente responsabile per la rappresentazione di variabili di tipo durante
l'inferenza, con identificatore univoco.

#### 3.2 Class Structure

| Campo | Tipo        | Visibilità | Semantica        |
|-------|-------------|------------|------------------|
| `id_` | `TypeVarId` | `private`  | ID univoco (> 0) |

Eredita da `TypeBase` con `TypeKind::TypeVar`.

#### 3.3 Interface Analysis

| Metodo       | Signature                                                                      | Precondizioni | Postcondizioni   |
|--------------|--------------------------------------------------------------------------------|---------------|------------------|
| ctor         | `explicit constexpr TypeVariable(TypeVarId id)`                                | `id > 0`      | Istanza creata   |
| `id`         | `[[nodiscard]] constexpr TypeVarId id() const noexcept`                        | Nessuna       | Restituisce ID   |
| `to_string`  | `[[nodiscard]] std::string to_string() const override`                         | Nessuna       | Formato `?T{id}` |
| `classof`    | `[[nodiscard]] static constexpr bool classof(const TypeBase *t) noexcept`      | Nessuna       | RTTI check       |
| `operator==` | `[[nodiscard]] bool operator==(const TypeBase &other) const noexcept override` | Nessuna       | Confronta ID     |

#### 3.4 Implementation Logic

Triviale. `to_string()` usa `FORMAT`. `operator==` confronta kind e ID.

#### 3.5 Error Handling Evaluation

Nessun errore possibile.

#### 3.6 Type Consistency Audit

Coerente. `TypeVarId` è `std::size_t`.

#### 3.7 Inter-Component Interaction

Dipende da `TypeBase`. Usato da tutti i sistemi del typechecker.

#### 3.8 Optimization Opportunities

Nessuno significativo. Componente minimale e ben implementato.

---

### System: UnionFind › Component: UnionFind (classe)

#### 3.1 Responsibility Statement

Il componente `UnionFind` è esclusivamente responsabile per la gestione di insiemi disjoint-set con path compression e
union by rank per unificazione efficiente.

#### 3.2 Class Structure

| Campo     | Tipo                                          | Visibilità | Semantica             |
|-----------|-----------------------------------------------|------------|-----------------------|
| `parent_` | `std::unordered_map<TypeVarId, TypeVarId>`    | `private`  | Mappa nodo → genitore |
| `rank_`   | `std::unordered_map<TypeVarId, std::uint8_t>` | `private`  | Rank di ogni nodo     |

#### 3.3 Interface Analysis

| Metodo     | Signature                                               | Precondizioni                   | Postcondizioni         |
|------------|---------------------------------------------------------|---------------------------------|------------------------|
| `make_set` | `void make_set(TypeVarId var)`                          | Nessuna                         | Singleton set creato   |
| `find`     | `[[nodiscard]] TypeVarId find(TypeVarId var)`           | `var` registrato con `make_set` | Rappresentante del set |
| `unite`    | `void unite(TypeVarId x, TypeVarId y)`                  | `x`, `y` registrati             | Set uniti              |
| `same_set` | `[[nodiscard]] bool same_set(TypeVarId x, TypeVarId y)` | `x`, `y` registrati             | true se stesso set     |
| `size`     | `[[nodiscard]] std::size_t size() const noexcept`       | Nessuna                         | Numero di elementi     |

#### 3.4 Implementation Logic

Implementazione standard union-find con path compression ricorsivo e union by rank.

#### 3.5 Error Handling Evaluation

**DEF-026** (ripetuta) — `find()` lancia `std::out_of_range` per variabili non registrate.

#### 3.6 Type Consistency Audit

Coerente.

#### 3.7 Inter-Component Interaction

Usato esclusivamente da `ConstraintSolver::unify`.

#### 3.8 Optimization Opportunities

Nessuno significativo per l'implementazione attuale. Per set molto grandi, `std::vector` invece di `unordered_map`
sarebbe più efficiente se gli ID sono densi.

---

### System: ErrorType › Component: ErrorType (classe)

#### 3.1 Responsibility Statement

Il componente `ErrorType` è esclusivamente responsabile per fornire un tipo sentinella che si unifica silenziosamente
con qualsiasi tipo, prevenendo errori a cascata.

#### 3.2 Class Structure

Nessun campo. Eredita da `TypeBase` con `TypeKind::Error`. Singleton tramite `error_type()`.

#### 3.3 Interface Analysis

| Metodo       | Signature                                                                      | Precondizioni | Postcondizioni                |
|--------------|--------------------------------------------------------------------------------|---------------|-------------------------------|
| ctor         | `ErrorType()`                                                                  | Nessuna       | Istanza creata                |
| `to_string`  | `[[nodiscard]] std::string to_string() const override`                         | Nessuna       | Restituisce `"<error>"`       |
| `classof`    | `[[nodiscard]] static constexpr bool classof(const TypeBase *t) noexcept`      | Nessuna       | RTTI check                    |
| `operator==` | `[[nodiscard]] bool operator==(const TypeBase &other) const noexcept override` | Nessuna       | true se other.kind() == Error |

| Funzione     | Signature                                     | Postcondizioni                  |
|--------------|-----------------------------------------------|---------------------------------|
| `error_type` | `[[nodiscard]] TypePtr error_type() noexcept` | Restituisce singleton condiviso |

#### 3.4 Implementation Logic

Singleton thread-safe tramite variabile statica const con `make_shared`.

#### 3.5 Error Handling Evaluation

Nessun errore.

#### 3.6 Type Consistency Audit

Coerente.

#### 3.7 Inter-Component Interaction

Usato da `TypeChecker::type_expr` (per undeclared identifier), `ConstraintSolver::unify` (per silent unification).

#### 3.8 Optimization Opportunities

Nessuno. Componente minimale e corretto.

---

## Phase 4 — Prioritized Recommendations

### 4.1 Recommendation Register

#### REC-001

**Title**: Implementare istanziamento completo per TypeScheme con visitor

**Deficiency addressed**: DEF-008

**Description**: Sostituire l'implementazione semplificata di `TypeScheme::instantiate` (`TypeScheme.cpp:14–35`) con un
visitor che traversa ricorsivamente il body (ArrayType, VectorType, e tipi composti futuri) sostituendo ogni
TypeVariable il cui ID è in `quantified_vars` con una fresh type variable. Entry point: `TypeScheme::instantiate` in
`TypeScheme.cpp`. Creare una funzione helper
`substitute_type_vars(const TypePtr&, const std::unordered_map<TypeVarId, TypePtr>&)` che ricorre su
ArrayType/VectorType. Expected outcome: funzioni polimorfiche con ritorno composto (es. `Array<T>`) vengono istanziate
correttamente.

**Feasibility score**: 3 — Richiede implementazione di un visitor ricorsivo ma il pattern è ben noto e la struttura dei
tipi è già definita.

**Expected ROI**: 5 — Abilita il polimorfismo per tipi composti, attualmente completamente rotto.

**Implementation effort**: 2 — Stimato 4–8 ore di implementazione e testing.

**Priority rank**: 3×2 + 5×2 + 2×1 = 6 + 10 + 2 = **18**

**Estimated implementation time**: 4–8 ore

**Required resources**: Sviluppatore C++ con conoscenza di visitor pattern, test esistenti per validazione

**Effectiveness indicators**:

1. Test `TypeScheme::instantiate` con body `ArrayType` contenente TypeVariable verifica che l'element type sia una fresh
   variable
2. Zero regressioni nei test di inferenza polimorfica esistenti
3. Coverage del metodo `instantiate` ≥ 90%

---

#### REC-002

**Title**: Correggere name resolution per IfStmt, WhileStmt, ForStmt

**Deficiency addressed**: DEF-030

**Description**: Aggiungere casi per `NodeKind::IfStmt`, `NodeKind::WhileStmt`, `NodeKind::ForStmt` in
`TypeChecker::resolve_names_stmt` (`TypeChecker.cpp:96–147`). Per IfStmt: risolvere nomi in condition, then_branch,
else_branch. Per WhileStmt: risolvere in condition e body. Per ForStmt: pushare scope, risolvere init, condition,
increment, body, poppare scope. Entry point: `TypeChecker::resolve_names_stmt` in `TypeChecker.cpp`. Expected outcome:
variabili dichiarate nei body di if/while/for vengono correttamente registrate, eliminando falsi positivi "Undeclared
identifier".

**Feasibility score**: 5 — Pattern già implementato per FuncDecl e BlockStmt, basta replicare.

**Expected ROI**: 5 — Risolve bug funzionale: variabili in body di if/while/for non vengono trovate.

**Implementation effort**: 5 — Stimato 1–2 ore (3 casi, pattern noto).

**Priority rank**: 5×2 + 5×2 + 5×1 = 10 + 10 + 5 = **25**

**Estimated implementation time**: 1–2 ore

**Required resources**: Sviluppatore C++, suite di test esistente

**Effectiveness indicators**:

1. Test con variabile dichiarata in body di if/while/for e usata successivamente non produce "Undeclared identifier"
2. Tutti i test esistenti continuano a passare
3. Coverage di `resolve_names_stmt` ≥ 95%

---

#### REC-003

**Title**: Generare vincoli di signature per CallExpr

**Deficiency addressed**: DEF-013

**Description**: Implementare la generazione di vincoli per le chiamate a funzione in `TypeChecker::type_expr` caso
`CallExpr` (`TypeChecker.cpp:689–730`). Step: (1) Lookup della funzione nella SymbolTable per ottenere la sua
signature. (2) Se la signature è disponibile, generare vincoli tra i tipi degli argomenti effettivi e i tipi dei
parametri formali. (3) Generare vincolo tra il tipo di ritorno della funzione e il tipo risultante della CallExpr. (4)
Se la funzione non ha signature nota (type variable fresca), creare un tipo funzione temporaneo e unificarlo. Entry
point: `TypeChecker::type_expr` caso `NodeKind::CallExpr` in `TypeChecker.cpp`. Expected outcome: chiamate con arity o
tipo argomenti errati producono errori di tipo.

**Feasibility score**: 2 — Richiede rappresentare tipi funzione (FnType) che non esistono attualmente nel sistema dei
tipi. Necessaria estensione di `Type.hpp`.

**Expected ROI**: 5 — Abilita verifica fondamentale delle chiamate di funzione.

**Implementation effort**: 1 — Stimato 2–3 giorni (design FnType, implementazione vincoli, testing).

**Priority rank**: 2×2 + 5×2 + 1×1 = 4 + 10 + 1 = **15**

**Estimated implementation time**: 2–3 giorni

**Required resources**: Sviluppatore C++ senior con conoscenza di sistemi di tipi, estensione di Type.hpp

**Effectiveness indicators**:

1. Test con chiamata a funzione con numero argomenti sbagliato produce errore
2. Test con argomento di tipo incompatibile produce errore di type mismatch
3. Chiamate corrette continuano a funzionare

---

#### REC-004

**Title**: Implementare vincoli per CastExpr

**Deficiency addressed**: DEF-015

**Description**: In `TypeChecker::type_expr` caso `CastExpr` (`TypeChecker.cpp:850–862`), generare un vincolo tra il
tipo dell'operando e il tipo target parseato. Se il tipo target non è parseabile (sconosciuto), usare type variable
fresca ma emettere warning. Entry point: `TypeChecker::type_expr` caso `NodeKind::CastExpr` in `TypeChecker.cpp`.
Aggiungere
`constraints_.add(operand_type, target_type, cast->location(), "cast: operand must be compatible with target")`.
Expected outcome: cast incompatibili vengono rilevati dal constraint solver.

**Feasibility score**: 5 — Aggiunta di una singola riga di vincolo, già esiste il pattern.

**Expected ROI**: 4 — Migliora significativamente la sicurezza dei cast.

**Implementation effort**: 5 — Stimato 30 minuti.

**Priority rank**: 5×2 + 4×2 + 5×1 = 10 + 8 + 5 = **23**

**Estimated implementation time**: 30 minuti

**Required resources**: Sviluppatore C++

**Effectiveness indicators**:

1. Test con cast `i32` a `string` produce errore
2. Test con cast `i32` a `f64` viene accettato (cast numerico valido)
3. Zero regressioni nei test esistenti

---

#### REC-005

**Title**: Correggere CustomType unification per confrontare nomi

**Deficiency addressed**: DEF-007

**Description**: In `ConstraintSolver::unify` (`ConstraintSolver.cpp:119–122`), sostituire il caso `default` con un
`case TypeKind::Custom` esplicito che confronta i nomi dei due CustomType. Se i nomi differiscono, restituire errore di
type mismatch. Entry point: `ConstraintSolver::unify` in `ConstraintSolver.cpp`. Expected outcome: tipi custom diversi (
`Foo` vs `Bar`) non si unificano.

**Feasibility score**: 5 — Modifica localizzata a uno switch, pattern già esistente per Array/Vector.

**Expected ROI**: 4 — Previene unificazione errata di tipi custom diversi.

**Implementation effort**: 5 — Stimato 15 minuti.

**Priority rank**: 5×2 + 4×2 + 5×1 = 10 + 8 + 5 = **23**

**Estimated implementation time**: 15 minuti

**Required resources**: Sviluppatore C++

**Effectiveness indicators**:

1. Test con unificazione `Foo` vs `Bar` produce errore
2. Test con unificazione `Foo` vs `Foo` succeeds
3. Zero regressioni

---

#### REC-006

**Title**: Preservare statement non zonkati in zonk_block_full

**Deficiency addressed**: DEF-012

**Description**: In `zonk_block_full` (`TypeChecker.cpp:370–383`), quando `zonk_stmt_full` restituisce `nullptr`,
preservare lo statement originale invece di scartarlo. Entry point: `TypeChecker::zonk_block_full` in `TypeChecker.cpp`.
Modificare il ramo else per pushare lo statement originale (`zonked_stmts.push_back(std::move(s))` usando `s` dal blocco
originale). Expected outcome: nessun statement viene perso durante lo zonking.

**Feasibility score**: 5 — Modifica di 2-3 righe.

**Expected ROI**: 4 — Previene perdita silenziosa di codice durante il type checking.

**Implementation effort**: 5 — Stimato 15 minuti.

**Priority rank**: 5×2 + 4×2 + 5×1 = 10 + 8 + 5 = **23**

**Estimated implementation time**: 15 minuti

**Required resources**: Sviluppatore C++

**Effectiveness indicators**:

1. Test con statement che produce nullptr da zonk_stmt_full viene preservato nel blocco output
2. Programmi tipizzati mantengono tutti gli statement originali
3. Zero regressioni

---

#### REC-007

**Title**: Implementare vincoli per MemberExpr

**Deficiency addressed**: DEF-014

**Description**: In `TypeChecker::type_expr` caso `MemberExpr` (`TypeChecker.cpp:843–848`), implementare lookup del
membro. Per ora, generare un vincolo tra il tipo dell'oggetto e il tipo risultato (type variable fresca). In futuro,
quando struct/class saranno supportati, il lookup verificherà l'esistenza del membro. Entry point:
`TypeChecker::type_expr` caso `NodeKind::MemberExpr` in `TypeChecker.cpp`. Expected outcome: MemberExpr produce vincoli
invece di silenzio.

**Feasibility score**: 3 — Richiede un sistema di membri per tipi custom che non esiste ancora. Per ora, generare un
vincolo minimale.

**Expected ROI**: 3 — Migliora la tracciabilità dei vincoli per accessi a membri.

**Implementation effort**: 3 — Stimato 2–4 ore per implementazione minimale.

**Priority rank**: 3×2 + 3×2 + 3×1 = 6 + 6 + 3 = **15**

**Estimated implementation time**: 2–4 ore

**Required resources**: Sviluppatore C++

**Effectiveness indicators**:

1. MemberExpr genera almeno un vincolo
2. Il tipo del risultato è vincolato al tipo dell'oggetto
3. Zero regressioni

---

#### REC-008

**Title**: Unificare gestione errori: SymbolTable restituisce std::expected

**Deficiency addressed**: DEF-005

**Description**: Modificare `SymbolTable::lookup` per restituire `std::expected<TypeScheme, CompileError>` invece di
`std::optional<TypeScheme>`. Entry point: `SymbolTable::lookup` in `SymbolTable.hpp` e `SymbolTable.cpp`. Aggiornare
tutti i chiamanti (`TypeChecker::type_expr` per Identifier, `TypeChecker::resolve_names_stmt`). Expected outcome:
coerenza nella propagazione errori attraverso tutta la pipeline.

**Feasibility score**: 2 — Richiede modifica API e aggiornamento di tutti i chiamanti. Breaking change per eventuali
consumatori esterni.

**Expected ROI**: 3 — Migliora coerenza architetturale ma impatto funzionale limitato (lookup fallito è già gestito dal
chiamante).

**Implementation effort**: 2 — Stimato 1–2 giorni (refactoring + test).

**Priority rank**: 2×2 + 3×2 + 2×1 = 4 + 6 + 2 = **12**

**Estimated implementation time**: 1–2 giorni

**Required resources**: Sviluppatore C++, refactor dei chiamanti

**Effectiveness indicators**:

1. Tutti i lookup falliti restituiscono errore con contesto
2. Zero `std::nullopt` non gestiti
3. Tutti i test passano

---

#### REC-009

**Title**: Decomporre type_expr in funzioni per caso

**Deficiency addressed**: DEF-009

**Description**: Refactoring di `TypeChecker::type_expr` (480 righe) suddividendo ogni caso dello switch in una funzione
dedicata (`type_integer_literal`, `type_binary_expr`, `type_call_expr`, ecc.). Entry point: `TypeChecker::type_expr` in
`TypeChecker.cpp` e dichiarazione in `TypeChecker.hpp`. Expected outcome: ogni funzione ≤100 righe, profondità di
annidamento ≤3, compliance con AGENTS.md §7.

**Feasibility score**: 3 — Refactoring meccanico ma voluminoso. Ogni funzione ha accesso a `this` per `constraints_`,
`symbols_`, `errors_`.

**Expected ROI**: 4 — Migliora drasticamente manutenibilità e leggibilità.

**Implementation effort**: 1 — Stimato 2–3 giorni (20+ funzioni da estrarre, test di regressione).

**Priority rank**: 3×2 + 4×2 + 1×1 = 6 + 8 + 1 = **15**

**Estimated implementation time**: 2–3 giorni

**Required resources**: Sviluppatore C++ senior, tempo per refactoring esteso

**Effectiveness indicators**:

1. Nessuna funzione >100 righe in TypeChecker.cpp
2. lizard reporta CCN ≤15 per tutte le funzioni
3. Tutti i test passano senza modifiche

---

#### REC-010

**Title**: Decomporre type_stmt in funzioni per caso

**Deficiency addressed**: DEF-009

**Description**: Analogamente a REC-009, decomporre `TypeChecker::type_stmt` (316 righe) in funzioni dedicate per ogni
NodeKind. Entry point: `TypeChecker::type_stmt` in `TypeChecker.cpp` e dichiarazione in `TypeChecker.hpp`. Expected
outcome: compliance con AGENTS.md §7.

**Feasibility score**: 3 — Simile a REC-009, leggermente meno voluminoso.

**Expected ROI**: 4 — Stesso beneficio di manutenibilità.

**Implementation effort**: 2 — Stimato 1–2 giorni.

**Priority rank**: 3×2 + 4×2 + 2×1 = 6 + 8 + 2 = **16**

**Estimated implementation time**: 1–2 giorni

**Required resources**: Sviluppatore C++

**Effectiveness indicators**:

1. Nessuna funzione >100 righe
2. lizard CCN ≤15
3. Tutti i test passano

---

#### REC-011

**Title**: Aggiungere case TypeKind::Error esplicito in Substitution::applyImpl

**Deficiency addressed**: DEF-006

**Description**: In `Substitution::applyImpl` (`Substitution.cpp:27–54`), aggiungere
`case TypeKind::Error: result = type; break;` esplicitamente prima del `default`. Entry point: `Substitution::applyImpl`
in `Substitution.cpp`. Expected outcome: comportamento documentato esplicitamente, resilienza a futuri TypeKind aggiunti
dopo Error.

**Feasibility score**: 5 — Aggiunta di 2 righe.

**Expected ROI**: 3 — Migliora resilienza e chiarezza del codice.

**Implementation effort**: 5 — Stimato 5 minuti.

**Priority rank**: 5×2 + 3×2 + 5×1 = 10 + 6 + 5 = **21**

**Estimated implementation time**: 5 minuti

**Required resources**: Sviluppatore C++

**Effectiveness indicators**:

1. Switch in applyImpl ha case esplicito per ogni TypeKind
2. clang-tidy non riporta warnings su switch non esaustivi
3. Test con ErrorType in sostituzione continua a funzionare

---

#### REC-012

**Title**: Delegare zonk_type a Substitution::apply

**Deficiency addressed**: DEF-031

**Description**: Sostituire l'implementazione di `zonk_type` (`TypeChecker.cpp:41–65`) con una delega a
`Substitution::apply`. Entry point: `zonk_type` in `TypeChecker.cpp`. Semplificare a: `return subst.apply(type);`.
Expected outcome: eliminazione della duplicazione della logica di ricorsione su Array/Vector.

**Feasibility score**: 5 — Sostituzione diretta, `Substitution::apply` fa già tutto il lavoro.

**Expected ROI**: 4 — Elimina duplicazione, riduce superficie di bug.

**Implementation effort**: 5 — Stimato 10 minuti.

**Priority rank**: 5×2 + 4×2 + 5×1 = 10 + 8 + 5 = **23**

**Estimated implementation time**: 10 minuti

**Required resources**: Sviluppatore C++

**Effectiveness indicators**:

1. Funzione `zonk_type` delega a `subst.apply(type)`
2. Tutti i test di zonking passano
3. Rimozione di ~25 righe di codice duplicato

---

#### REC-013

**Title**: Validare arity delle chiamate in CallExpr

**Deficiency addressed**: DEF-013

**Description**: Come parte della risoluzione di DEF-013, aggiungere un check immediato: se la funzione è nella
SymbolTable e ha una signature nota, confrontare il numero di argomenti effettivi con il numero di parametri formali.
Entry point: `TypeChecker::type_expr` caso `CallExpr` in `TypeChecker.cpp`. Expected outcome: errore per chiamate con
arity sbagliata.

**Feasibility score**: 3 — Dipende dalla disponibilità di signature nella SymbolTable (attualmente le funzioni hanno
solo type variable fresca).

**Expected ROI**: 4 — Rileva un'importante classe di errori del programmatore.

**Implementation effort**: 3 — Stimato 2–4 ore.

**Priority rank**: 3×2 + 4×2 + 3×1 = 6 + 8 + 3 = **17**

**Estimated implementation time**: 2–4 ore

**Required resources**: Sviluppatore C++

**Effectiveness indicators**:

1. Test con chiamata a funzione 2-param con 3 argomenti produce errore
2. Chiamate con arity corretta continuano a funzionare

---

#### REC-014

**Title**: Gestire nullptr in type_expr per ArrayLiteral

**Deficiency addressed**: DEF-016

**Description**: In `TypeChecker::type_expr` caso `ArrayLiteral` (`TypeChecker.cpp:733–768`), quando `type_expr` del
primo elemento restituisce `nullptr`, invece di propagare `nullptr`, restituire `error_type()` come tipo dell'array e un
ArrayLiteral tipizzato con elementi vuoti o error. Entry point: `TypeChecker::type_expr` caso `NodeKind::ArrayLiteral`
in `TypeChecker.cpp`. Expected outcome: nessun crash per dereferenziazione di nullptr da ArrayLiteral.

**Feasibility score**: 5 — Modifica localizzata a un caso.

**Expected ROI**: 3 — Previene potenziali crash.

**Implementation effort**: 5 — Stimato 15 minuti.

**Priority rank**: 5×2 + 3×2 + 5×1 = 10 + 6 + 5 = **21**

**Estimated implementation time**: 15 minuti

**Required resources**: Sviluppatore C++

**Effectiveness indicators**:

1. Test con ArrayLiteral il cui primo elemento fallisce il typing non crasha
2. L'errore viene registrato nel vettore errors_
3. Zero regressioni

---

#### REC-015

**Title**: Rendere type_expr e type_stmt privati o friend per test

**Deficiency addressed**: DEF-002

**Description**: Cambiare la visibilità di `type_expr` e `type_stmt` da `public` a `private` in `TypeChecker.hpp` (righe
62–77). Per mantenere la testabilità, dichiarare le classi di test come `friend` o creare un wrapper di testing. Entry
point: `TypeChecker.hpp` righe 62–77. Expected outcome: incapsulamento della pipeline preservato, testabilità mantenuta.

**Feasibility score**: 4 — Richiede aggiornamento dei test esistenti per usare il meccanismo friend.

**Expected ROI**: 3 — Migliora incapsulamento API.

**Implementation effort**: 4 — Stimato 1–2 ore.

**Priority rank**: 4×2 + 3×2 + 4×1 = 8 + 6 + 4 = **18**

**Estimated implementation time**: 1–2 ore

**Required resources**: Sviluppatore C++, aggiornamento test

**Effectiveness indicators**:

1. `type_expr` e `type_stmt` non sono accessibili dall'esterno del TypeChecker
2. I test esistenti continuano a compilare e passare
3. Documentazione API aggiornata

---

#### REC-016

**Title**: Aggiungere case TypeKind::Custom a occurs_in

**Deficiency addressed**: DEF-020

**Description**: In `ConstraintSolver::occurs_in` (`ConstraintSolver.cpp:24–44`), aggiungere gestione esplicita per
`TypeKind::Custom`. Per ora, se CustomType ha parametri generici futuri, occorrerebbe ricorrere in essi. Entry point:
`ConstraintSolver::occurs_in` in `ConstraintSolver.cpp`. Expected outcome: occurs check completo per tutti i tipi
composti.

**Feasibility score**: 4 — Aggiunta di un caso switch. CustomType attuale non ha parametri, quindi il case può essere
placeholder.

**Expected ROI**: 3 — Previene bugs futuri quando CustomType diventa parametrico.

**Implementation effort**: 5 — Stimato 10 minuti.

**Priority rank**: 4×2 + 3×2 + 5×1 = 8 + 6 + 5 = **19**

**Estimated implementation time**: 10 minuti

**Required resources**: Sviluppatore C++

**Effectiveness indicators**:

1. Switch in occurs_in ha case per CustomType
2. Test con CustomType contenente TypeVariable (quando supportato) passa occurs check

---

#### REC-017

**Title**: Unificare ArrayType size nell'unificazione

**Deficiency addressed**: DEF-019

**Description**: In `ConstraintSolver::unify` caso `TypeKind::Array` (`ConstraintSolver.cpp:109–115`), aggiungere
confronto delle espressioni di dimensione. Se le size differiscono, restituire errore di type mismatch. Entry point:
`ConstraintSolver::unify` in `ConstraintSolver.cpp`. Expected outcome: `[i32; 3]` non si unifica con `[i32; 5]`.

**Feasibility score**: 3 — Richiede confronto strutturale di espressioni (non banale per espressioni complesse). Per
IntegerLiteral è semplice.

**Expected ROI**: 4 — Rileva errori di dimensione array.

**Implementation effort**: 3 — Stimato 2–4 ore per confronto expr completo.

**Priority rank**: 3×2 + 4×2 + 3×1 = 6 + 8 + 3 = **17**

**Estimated implementation time**: 2–4 ore

**Required resources**: Sviluppatore C++

**Effectiveness indicators**:

1. Test con unificazione array di dimensioni diverse produce errore
2. Array con stessa size e stesso elemento type si unificano
3. Zero regressioni

---

#### REC-018

**Title**: Ottimizzare ConstraintSet::get a O(1)

**Deficiency addressed**: DEF-022

**Description**: Sostituire il lookup lineare in `ConstraintSet::get` (`Constraint.cpp:18–21`) con accesso diretto per
indice: `return (id > 0 && id <= constraints_.size()) ? &constraints_[id - 1] : nullptr;`. Entry point:
`ConstraintSet::get` in `Constraint.cpp`. Expected outcome: lookup O(1) invece di O(n).

**Feasibility score**: 5 — Modifica di una riga, gli ID sono sequenziali per costruzione.

**Expected ROI**: 3 — Migliora performance per grandi constraint set.

**Implementation effort**: 5 — Stimato 5 minuti.

**Priority rank**: 5×2 + 3×2 + 5×1 = 10 + 6 + 5 = **21**

**Estimated implementation time**: 5 minuti

**Required resources**: Sviluppatore C++

**Effectiveness indicators**:

1. Benchmark su programma con 10000+ vincoli mostra miglioramento
2. Tutti i test passano
3. Complessità documentata nell'header

---

#### REC-019

**Title**: Correggere multi-var decl in type_stmt

**Deficiency addressed**: DEF-018

**Description**: In `TypeChecker::type_stmt` caso `VarDecl` con multipli nomi (`TypeChecker.cpp:972–1004`), invece di
semplificare a single-var, creare una struttura `TypedVarDecl` multipla o un nuovo nodo `TypedMultiVarDecl`. Entry
point: `TypeChecker::type_stmt` caso `NodeKind::VarDecl` in `TypeChecker.cpp`. Expected outcome: tutti i nomi multipli
appaiono nell'AST tipizzato.

**Feasibility score**: 2 — Richiede modifica all'AST tipizzato (aggiungere TypedMultiVarDecl o estendere TypedVarDecl).

**Expected ROI**: 3 — Corregge perdita di informazioni nell'AST tipizzato.

**Implementation effort**: 2 — Stimato 1–2 giorni (design AST + implementazione + zonking).

**Priority rank**: 2×2 + 3×2 + 2×1 = 4 + 6 + 2 = **12**

**Estimated implementation time**: 1–2 giorni

**Required resources**: Sviluppatore C++, modifica AST tipizzato

**Effectiveness indicators**:

1. Test con `let a, b, c = 1, 2, 3` produce 3 variabili nell'AST tipizzato
2. Zonking preserva tutte le variabili
3. Zero regressioni

---

#### REC-020

**Title**: Aggiungere assert o例外 per find() su variabile non registrata

**Deficiency addressed**: DEF-026

**Description**: In `UnionFind::find` (`UnionFind.cpp:15–19`), sostituire `parent_.at(var)` con un check che restituisce
un valore sentinel o usa `assert(parent_.contains(var))` per documentare la precondizione. Entry point:
`UnionFind::find` in `UnionFind.cpp`. Expected outcome: errore chiaro invece di eccezione std::out_of_range per uso
improprio.

**Feasibility score**: 5 — Aggiunta di assert o check.

**Expected ROI**: 2 — Migliora debuggabilità ma impatto funzionale minimo (l'uso corrente è corretto).

**Implementation effort**: 5 — Stimato 5 minuti.

**Priority rank**: 5×2 + 2×2 + 5×1 = 10 + 4 + 5 = **19**

**Estimated implementation time**: 5 minuti

**Required resources**: Sviluppatore C++

**Effectiveness indicators**:

1. UnionFind::find contiene assert o check documentato
2. Messaggio d'errore chiaro per uso improprio
3. Zero regressioni

---

### 4.2 Summary Priority Table

| Rank | ID      | Title                                                                | Feasibility | ROI | Effort | Composite Score | Est. Time  |
|------|---------|----------------------------------------------------------------------|-------------|-----|--------|-----------------|------------|
| 1    | REC-002 | Correggere name resolution per IfStmt, WhileStmt, ForStmt            | 5           | 5   | 5      | **25**          | 1–2 ore    |
| 2    | REC-004 | Implementare vincoli per CastExpr                                    | 5           | 4   | 5      | **23**          | 30 minuti  |
| 3    | REC-005 | Correggere CustomType unification per confrontare nomi               | 5           | 4   | 5      | **23**          | 15 minuti  |
| 4    | REC-006 | Preservare statement non zonkati in zonk_block_full                  | 5           | 4   | 5      | **23**          | 15 minuti  |
| 5    | REC-012 | Delegare zonk_type a Substitution::apply                             | 5           | 4   | 5      | **23**          | 10 minuti  |
| 6    | REC-011 | Aggiungere case TypeKind::Error esplicito in Substitution::applyImpl | 5           | 3   | 5      | **21**          | 5 minuti   |
| 7    | REC-014 | Gestire nullptr in type_expr per ArrayLiteral                        | 5           | 3   | 5      | **21**          | 15 minuti  |
| 8    | REC-018 | Ottimizzare ConstraintSet::get a O(1)                                | 5           | 3   | 5      | **21**          | 5 minuti   |
| 9    | REC-016 | Aggiungere case TypeKind::Custom a occurs_in                         | 4           | 3   | 5      | **19**          | 10 minuti  |
| 10   | REC-020 | Aggiungere assert per find() su variabile non registrata             | 5           | 2   | 5      | **19**          | 5 minuti   |
| 11   | REC-001 | Implementare istanziamento completo per TypeScheme con visitor       | 3           | 5   | 2      | **18**          | 4–8 ore    |
| 12   | REC-015 | Rendere type_expr e type_stmt privati o friend per test              | 4           | 3   | 4      | **18**          | 1–2 ore    |
| 13   | REC-013 | Validare arity delle chiamate in CallExpr                            | 3           | 4   | 3      | **17**          | 2–4 ore    |
| 14   | REC-017 | Unificare ArrayType size nell'unificazione                           | 3           | 4   | 3      | **17**          | 2–4 ore    |
| 15   | REC-010 | Decomporre type_stmt in funzioni per caso                            | 3           | 4   | 2      | **16**          | 1–2 giorni |
| 16   | REC-003 | Generare vincoli di signature per CallExpr                           | 2           | 5   | 1      | **15**          | 2–3 giorni |
| 17   | REC-007 | Implementare vincoli per MemberExpr                                  | 3           | 3   | 3      | **15**          | 2–4 ore    |
| 18   | REC-009 | Decomporre type_expr in funzioni per caso                            | 3           | 4   | 1      | **15**          | 2–3 giorni |
| 19   | REC-008 | Unificare gestione errori: SymbolTable restituisce std::expected     | 2           | 3   | 2      | **12**          | 1–2 giorni |
| 20   | REC-019 | Correggere multi-var decl in type_stmt                               | 2           | 3   | 2      | **12**          | 1–2 giorni |

---

### Appendice: Traceability DEF → REC

| Deficiency | Recommendation                                                                                                                                     |
|------------|----------------------------------------------------------------------------------------------------------------------------------------------------|
| DEF-001    | REC-012                                                                                                                                            |
| DEF-002    | REC-015                                                                                                                                            |
| DEF-003    | (accettato come trade-off architetturale — refactoring a TypeChecker separato sarebbe over-engineering allo stadio attuale)                        |
| DEF-004    | (accettato — parser annotazioni sarà sostituito da parser proper quando il language support per tipi composti nelle annotazioni sarà implementato) |
| DEF-005    | REC-008                                                                                                                                            |
| DEF-006    | REC-011                                                                                                                                            |
| DEF-007    | REC-005                                                                                                                                            |
| DEF-008    | REC-001                                                                                                                                            |
| DEF-009    | REC-009, REC-010                                                                                                                                   |
| DEF-010    | (mitigato da REC-003 — vincoli di signature risolveranno il disaccoppiamento)                                                                      |
| DEF-011    | (nessun difetto — flusso corretto)                                                                                                                 |
| DEF-012    | REC-006                                                                                                                                            |
| DEF-013    | REC-003, REC-013                                                                                                                                   |
| DEF-014    | REC-007                                                                                                                                            |
| DEF-015    | REC-004                                                                                                                                            |
| DEF-016    | REC-014                                                                                                                                            |
| DEF-017    | (accettato — documentato come non thread-safe nel commento)                                                                                        |
| DEF-018    | REC-019                                                                                                                                            |
| DEF-019    | REC-017                                                                                                                                            |
| DEF-020    | REC-016                                                                                                                                            |
| DEF-021    | (accettato — UnionFind ottimizza path compression, overhead marginale)                                                                             |
| DEF-022    | REC-018                                                                                                                                            |
| DEF-023    | (accettato — rimozione vincoli non necessaria allo stadio attuale)                                                                                 |
| DEF-024    | (accettato — Substitution non è usato concorrentemente allo stadio attuale)                                                                        |
| DEF-025    | (accettato — comportamento documentato, define crea scope implicitamente)                                                                          |
| DEF-026    | REC-020                                                                                                                                            |
| DEF-027    | (mitigato da REC-009 — decomposizione in funzioni permetterà review puntuale dei cast)                                                             |
| DEF-028    | (accettato — errori non sono tipicamente migliaia in un singolo run)                                                                               |
| DEF-029    | (accettato — parse_type_annotation è chiamato raramente, cache non necessaria)                                                                     |
| DEF-030    | REC-002                                                                                                                                            |
| DEF-031    | REC-012                                                                                                                                            |
| DEF-032    | REC-001                                                                                                                                            |
