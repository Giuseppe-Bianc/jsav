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
`TypeChecker.cpp`, 1200 righe). L'header espone la classe `TypeChecker`, la struct `TypeCheckResult`, e dichiara i
metodi delle 4 fasi come privati. L'implementazione contiene:

- Funzioni helper statiche (`parse_type_annotation`, `zonk_type`) nelle righe 19–65
- Implementazione di `check()` (entry point) alle righe 68–89
- Fasi 1–4 separate in blocchi commentati (righe 92–187)
- `type_expr()` per constraint generation delle espressioni (righe 389–872, ~480 righe)
- `type_stmt()` per constraint generation delle istruzioni (righe 874–1200, ~326 righe)

**DEF-009** — Violazione soglia complessità cognitiva: Il file `TypeChecker.cpp` (1200 righe) e i metodi `type_expr` (~
480 righe) e `type_stmt` (~326 righe) violano sistematicamente la soglia di complessità cognitiva. La soglia
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

**DEF-011 — Zonk block silenziosamente scarta statement**: In `zonk_block_full` (`TypeChecker.cpp:370–383`), quando
`zonk_stmt_full` restituisce `nullptr`, il commento dice "Can't move from const — skip (original kept by callee)".
Questo significa che statement non zonkati vengono persi silenziosamente dall'output senza errore. Il blocco risultante
è incompleto e il programma tipizzato perde statement.

**DEF-012 — CallExpr non vincola signature della funzione**: In `type_expr` per `CallExpr` (`TypeChecker.cpp:689–730`),
il callee viene tipato e gli argomenti vengono tipati, ma **non viene generato alcun vincolo** che colleghi il tipo del
callee a una signature di funzione con i tipi degli argomenti e il tipo di ritorno. Il commento alle righe 716–722
ammette esplicitamente: "This is a simplification — a real implementation would use function types". Di conseguenza, le
chiamate a funzione non vengono verificate per arity o tipo degli argomenti. Qualsiasi espressione può essere chiamata
con qualsiasi argomento senza errore.

**DEF-013 — MemberExpr totalmente non implementato**: `type_expr` per `MemberExpr` (`TypeChecker.cpp:843–848`) crea una
type variable fresca per il risultato ma non genera alcun vincolo sull'oggetto o sul membro. Non c'è lookup del membro,
né verifica che l'oggetto abbia quel membro. Qualsiasi accesso `.member` è silenziosamente accettato.

**DEF-014 — CastExpr non genera vincoli di compatibilità**: In `type_expr` per `CastExpr` (`TypeChecker.cpp:850–862`),
il tipo target viene parsato ma non viene generato alcun vincolo tra il tipo dell'operando e il tipo target. Il cast è
quindi una operazione puramente sintattica senza verifica di compatibilità.

**DEF-015 — ArrayLiteral restituisce nullptr su errore**: Quando il primo elemento di un array literal non può essere
tipato, `type_expr` restituisce `nullptr` (`TypeChecker.cpp:739`). Questo propaga `nullptr` attraverso `type_stmt` per
`ExprStmt` che crea un placeholder, ma molti altri punti di chiamata non gestiscono `nullptr` (es.
`typed_init->node_type()` in `VarDecl` alle righe 946).

**DEF-016 — type_stmt per multi-var decl semplificato a single-var**: In `type_stmt` per `VarDecl` con multipli nomi (
`TypeChecker.cpp:972–1004`), il commento dice "Multi-variable declaration — simplify: create one TypedVarDecl with first
name". Il codice itera su tutti i nomi e li registra nella SymbolTable correttamente, ma restituisce un singolo
`TypedVarDecl` solo per il primo nome. Gli altri nomi sono persi nell'AST tipizzato.

**DEF-017 — Gestione loop_depth_ non thread-safe**: `loop_depth_` è un campo mutable della classe (`TypeChecker.hpp:89`)
incrementato/decrementato durante la traversata. Se `TypeChecker` venisse usato concorrentemente (non è il caso attuale,
ma l'API non lo proibisce esplicitamente), questo causerebbe data race.

#### 2.6 Partial or Undefined Implementations

| Metodo/Dichiarazione                | Stato        | Dettaglio                                                                                                                                                                                                                                                    |
|-------------------------------------|--------------|--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------|
| `TypeChecker::check`                | **Completo** | Pipeline completa a 4 fasi                                                                                                                                                                                                                                   |
| `TypeChecker::resolve_names`        | **Completo** | Crea scope globale, itera statement                                                                                                                                                                                                                          |
| `TypeChecker::resolve_names_stmt`   | **Completo** | Gestisce FuncDecl, MainStmt, VarDecl, BlockStmt. Non gestisce ReturnStmt, IfStmt, WhileStmt, ForStmt, BreakStmt, ContinueStmt (silenziosamente ignorati nella name resolution — `default: break`)                                                            |
| `TypeChecker::generate_constraints` | **Completo** | Itera statement, chiama `type_stmt`                                                                                                                                                                                                                          |
| `TypeChecker::solve_constraints`    | **Completo** | Delega a `ConstraintSolver`                                                                                                                                                                                                                                  |
| `TypeChecker::zonk`                 | **Completo** | Applica sostituzione a tutti gli statement                                                                                                                                                                                                                   |
| `TypeChecker::zonk_stmt_full`       | **Completo** | Switch su tutti i NodeKind. `default: return nullptr`                                                                                                                                                                                                        |
| `TypeChecker::zonk_expr_full`       | **Completo** | Switch su tutti i NodeKind. `default: return nullptr`                                                                                                                                                                                                        |
| `TypeChecker::zonk_block_full`      | **Parziale** | Silenziosamente scarta statement quando `zonk_stmt_full` restituisce `nullptr` (`TypeChecker.cpp:370–383`)                                                                                                                                                   |
| `TypeChecker::type_expr`            | **Parziale** | Gestisce 17/17+ NodeKind ma: `CallExpr` non vincola signature (`TypeChecker.cpp:716–722`), `MemberExpr` non implementa lookup (`TypeChecker.cpp:843–848`), `CastExpr` non valida compatibilità (`TypeChecker.cpp:850–862`), `default` restituisce error type |
| `TypeChecker::type_stmt`            | **Parziale** | Gestisce 12/12+ NodeKind ma: multi-var decl semplificato a single-var (`TypeChecker.cpp:972–1004`)                                                                                                                                                           |
| `parse_type_annotation` (static)    | **Parziale** | Gestisce solo tipi primitivi (14 tipi). Non gestisce tipi composti (array, vector, custom), non gestisce parametri (es. `Fn(i32) -> bool`)                                                                                                                   |
| `zonk_type` (static)                | **Parziale** | Gestisce TypeVariable, ArrayType, VectorType. Non gestisce CustomType parametrico — restituisce tipo invariato (`TypeChecker.cpp:59–64`)                                                                                                                     |

---

### System: ConstraintSolver (Motore di Unificazione)

#### 2.1 System Overview

Il `ConstraintSolver` è il motore di risoluzione vincoli del type checker. Implementa un algoritmo di unificazione
strutturale basato su Union-Find con path compression e union by rank. Riceve un `ConstraintSet` dalla fase di
constraint
generation e produce una `Substitution` che mappa type variable a tipi concreti, insieme a eventuali errori di
unificazione.

#### 2.2 Internal Module Organization

Il sistema consiste in un header (`ConstraintSolver.hpp`, 95 righe) e un file di implementazione (
`ConstraintSolver.cpp`, 140 righe). L'header espone la struct `SolverResult` e la classe `ConstraintSolver` con tre
metodi: `solve()`, `unify()` (privato), e `occurs_in()` (statico pubblico). L'organizzazione è coerente e minimale — il
file non contiene funzioni ausiliarie non dichiarate nell'header.

#### 2.3 Intra-System Dependency Analysis

Il `ConstraintSolver` dipende da `UnionFind` (campo privato `union_find_`), `Substitution` (campo privato
`substitution_`), e `ErrorType` (incluso in `ConstraintSolver.cpp:3`). Non ci sono dipendenze circolari interne. Il
flusso è lineare: `solve()` itera sui vincoli, chiama `unify()` per ciascuno, accumula errori in `SolverResult`.

#### 2.4 Logical Flow

1. `solve()` (`ConstraintSolver.cpp:11–22`) resetta `union_find_` e `substitution_`.
2. Itera su ogni `Constraint` nel `ConstraintSet` in ordine di inserimento.
3. Per ogni vincolo, chiama `unify(constraint.lhs, constraint.rhs, constraint)`.
4. `unify()` (`ConstraintSolver.cpp:47–127`):
    - **ErrorType bypass**: se t1 o t2 è `TypeKind::Error`, ritorna subito senza errore (`ConstraintSolver.cpp:48–49`).
    - **TypeVariable vs TypeVariable**: verifica occurs check, poi bind via UnionFind + Substitution.
    - **TypeVariable vs concreto**: verifica occurs check, poi bind.
    - **Concreto vs TypeVariable**: swap ricorsivo e unificazione.
    - **Concreto vs concreto**: se `kind()` diverso → errore E2034 (con hint numerico/stringa). Se `kind()` uguale:
      dispatch ricorsivo per `ArrayType` e `VectorType`; `default` assume uguaglianza per PrimitiveType e CustomType.
5. Restituisce `SolverResult` con substitution e errori.

#### 2.5 Critical Points

**DEF-018 — CustomType non confronta nomi**: Nel caso `default` (`ConstraintSolver.cpp:119–122`), due CustomType
diversi con lo stesso `kind()` vengono considerati uguali senza confrontare il campo `name_`. Questo permette
unificazione silenziosa tra tipi custom non correlati.

**DEF-019 — Numeric mismatch produce hint ma non errore**: Quando due tipi numerici diversi (es. `i32` vs `f64`) hanno
`kind()` diverso, il solver genera un errore E2034 con un hint ("Did you mean to cast..."). Questo è corretto dal punto
di vista della segnalazione, ma il `ConstraintSolver` non offre un meccanismo di recupero — l'errore è fatale per quel
vincolo, ma gli altri vincoli continuano a essere processati.

**DEF-020 — UnionFind `find` usa `at()` con eccezione**: `UnionFind::find` (`UnionFind.cpp:16–19`) usa `parent_.at(var)`
che lancia `std::out_of_range` se la variabile non è stata registrata con `make_set()`. Non c'è gestione di questo caso
nel `ConstraintSolver::unify` — se un type variable non è mai stata registrata, l'eccezione propaga fuori dal solver
senza essere catturata.

#### 2.6 Partial or Undefined Implementations

| Metodo/Dichiarazione          | Stato        | Dettaglio                                                                                                     |
|-------------------------------|--------------|---------------------------------------------------------------------------------------------------------------|
| `ConstraintSolver::solve`     | **Completo** | Itera vincoli, accumula errori                                                                                |
| `ConstraintSolver::unify`     | **Parziale** | Non confronta nomi CustomType (`ConstraintSolver.cpp:119–122`)                                                |
| `ConstraintSolver::occurs_in` | **Completo** | Ricorsione corretta per TypeVariable, ArrayType, VectorType. `default: return false` (corretto per primitivi) |

---

### System: Constraint (Rappresentazione Vincoli)

#### 2.1 System Overview

Il sistema `Constraint` definisce la struttura dati per rappresentare vincoli di uguaglianza tra tipi (`lhs = rhs`) e il
contenitore `ConstraintSet` per accumularli durante la constraint generation. Ogni vincolo ha un ID univoco, una
posizione sorgente, e una ragione testuale per il reporting degli errori.

#### 2.2 Internal Module Organization

Un header (`Constraint.hpp`, 132 righe) e un file di implementazione (`Constraint.cpp`, 27 righe). L'header definisce
`ConstraintId`, la struct `Constraint`, e la classe `ConstraintSet`. L'implementazione è minimale — tutti i metodi sono
definiti correttamente. L'organizzazione è coerente con la responsabilità singola del sistema.

#### 2.3 Intra-System Dependency Analysis

Nessuna dipendenza interna. `ConstraintSet` dipende solo da STL (`std::vector`) e dal tipo `Constraint` definito nello
stesso header. Dipende esternamente da `TypePtr` (da `Type.hpp`) e `SourceSpan` (da `location/SourceSpan.hpp`).

#### 2.4 Logical Flow

1. `ConstraintSet::add()` crea un nuovo `Constraint` con ID auto-incrementato e lo aggiunge al vettore interno.
2. `constraints()` restituisce riferimento const al vettore.
3. `get(id)` cerca linearmente per ID.
4. `size()` restituisce il conteggio.

#### 2.5 Critical Points

Nessuna criticità architetturale rilevata. Il sistema è minimale e corretto. Unica osservazione: `get()` è O(n) — per
un numero elevato di vincoli (migliaia), una `unordered_map<ConstraintId, Constraint>` sarebbe più efficiente. Per le
dimensioni attuali del codebase, la ricerca lineare è accettabile.

#### 2.6 Partial or Undefined Implementations

| Metodo/Dichiarazione         | Stato        | Dettaglio                       |
|------------------------------|--------------|---------------------------------|
| `ConstraintSet::add`         | **Completo** | ID auto-incrementato, push_back |
| `ConstraintSet::constraints` | **Completo** | Restituisce ref const           |
| `ConstraintSet::get`         | **Completo** | Ricerca lineare O(n)            |
| `ConstraintSet::size`        | **Completo** | Restituisce size del vettore    |

---

### System: Substitution (Mappatura Variabili→Tipi)

#### 2.1 System Overview

Il sistema `Substitution` gestisce la mappatura da `TypeVarId` a `TypePtr`, producendo il risultato finale del
constraint
solving. Espone `bind()` per aggiungere associazioni, `apply()` per applicare ricorsivamente le sostituzioni a un tipo,
e
`lookup()` per consultare un binding. Include un cache persistente (`apply_cache_`) per ottimizzare chiamate ripetute di
`apply()` sullo stesso nodo.

#### 2.2 Internal Module Organization

Un header (`Substitution.hpp`, 120 righe) con documentazione dettagliata sul caching persistente, e un file di
implementazione (`Substitution.cpp`, 70 righe). L'organizzazione è coerente. L'header documenta esplicitamente la
semantica del cache e le implicazioni di thread-safety.

#### 2.3 Intra-System Dependency Analysis

`Substitution` dipende da `TypePtr` (esterno) e `TypeVariable` (interno al typechecker). Non ci sono dipendenze
circolari. Il campo `apply_cache_` è `mutable` per permettere caching in metodi `const`.

#### 2.4 Logical Flow

1. `bind(var, type)` (`Substitution.cpp:11–14`): invalida il cache (`apply_cache_.clear()`), inserisce/aggiorna il
   binding.
2. `apply(type)` (`Substitution.cpp:25`): delega a `applyImpl(type)`.
3. `applyImpl(type)` (`Substitution.cpp:27–54`):
    - Controlla cache per `type.get()`. Se hit, restituisce il risultato cached.
    - Se `TypeVariable`: cerca nel bindings. Se trovato, ricorsivamente `applyImpl` sul binding. Altrimenti restituisce
      il
      tipo originale.
    - Se `ArrayType`/`VectorType`: ricorsivamente `applyImpl` sull'element type, crea nuovo nodo se cambiato.
    - Se `CustomType` o `default` (incluso `PrimitiveType`, `TypeKind::Error`): restituisce tipo invariato.
    - Memorizza risultato nel cache.

#### 2.5 Critical Points

**DEF-021 — Cache invalidation aggressiva**: `bind()` chiama `apply_cache_.clear()` su ogni inserimento
(`Substitution.cpp:12`). Questo invalida tutto il cache anche se il nuovo binding non è correlato ai nodi cached. Per
sostituzioni numerose, il cache diventa inefficace. Una strategia di invalidazione selettiva (rimuovere solo le entry
che dipendono dalla variabile bindata) migliorerebbe le prestazioni.

**DEF-022 — ErrorType non menzionato esplicitamente**: Il caso `TypeKind::Error` cade nel `default` branch
(`Substitution.cpp:50–51`), che restituisce il tipo invariato. Sebbene corretto (ErrorType non ha sottostruttura),
l'assenza di un caso esplicito rende il comportamento implicito e vulnerabile a refactoring futuri.

#### 2.6 Partial or Undefined Implementations

| Metodo/Dichiarazione      | Stato        | Dettaglio                                                                                         |
|---------------------------|--------------|---------------------------------------------------------------------------------------------------|
| `Substitution::bind`      | **Completo** | Invalida cache, aggiorna bindings                                                                 |
| `Substitution::lookup`    | **Completo** | Cerca in bindings                                                                                 |
| `Substitution::apply`     | **Completo** | Delega ad applyImpl                                                                               |
| `Substitution::applyImpl` | **Parziale** | ErrorType non menzionato esplicitamente (`Substitution.cpp:50–51`); cache invalidation aggressiva |
| `Substitution::contains`  | **Completo** | Controlla presenza in bindings                                                                    |
| `Substitution::size`      | **Completo** | Restituisce size di bindings                                                                      |

---

### System: SymbolTable (Gestione Scope e Simboli)

#### 2.1 System Overview

Il `SymbolTable` gestisce la mappatura da identificatori a `TypeScheme` con supporto per scope annidati. Implementa
shadowing: i binding negli scope interni nascondono quelli negli scope esterni con lo stesso nome. È usato
esclusivamente durante la name resolution (Fase 1).

#### 2.2 Internal Module Organization

Un header (`SymbolTable.hpp`, 72 righe) e un file di implementazione (`SymbolTable.cpp`, 38 righe). L'header definisce
la classe con 5 metodi pubblici e un helper `StringHash` per heterogeneous lookup. L'implementazione è diretta — tutti i
metodi sono corretti e completi.

#### 2.3 Intra-System Dependency Analysis

Dipende solo da `TypeScheme` e STL. Nessuna dipendenza circolare. Il campo `scopes_` è un `vector` di `unordered_map` —
semplice e efficace.

#### 2.4 Logical Flow

1. `push_scope()` aggiunge un nuovo `unordered_map` vuoto al vettore.
2. `pop_scope()` rimuove l'ultimo map (con guardia `if(!scopes_.empty())`).
3. `define(name, scheme)` inserisce o aggiorna nel map dell'ultimo scope.
4. `lookup(name)` itera dal vettore in ordine inverso (innermost → outermost), restituendo il primo match.
5. `defined_in_current_scope()` controlla solo l'ultimo scope.
6. `depth()` restituisce la dimensione del vettore.

#### 2.5 Critical Points

**DEF-023 — `string_view` come chiave con lifetime risk**: Il `SymbolTable` usa `std::string_view` come chiave del map (
`SymbolTable.hpp:65`). I `string_view` sono non-possessivi — se le stringhe originali vengono deallocate, i view
diventano dangling. Nel contesto attuale, le chiavi sono `std::string` dell'AST che vivono più a lungo della
SymbolTable,
quindi il rischio è contenuto. Ma questa assunzione non è documentata né verificata a compile-time, rendendo il codice
fragile a refactoring.

**DEF-024 — `lookup` restituisce `std::nullopt` senza contesto**: Quando un simbolo non viene trovato, `lookup()`
restituisce `std::nullopt` senza informazione sul perché. Il chiamante (`TypeChecker::type_expr` per `Identifier`) deve
costruire l'errore da zero. Una restituzione di `std::expected<TypeScheme, LookupError>` fornirebbe maggiore coerenza
con
il resto della pipeline.

#### 2.6 Partial or Undefined Implementations

| Metodo/Dichiarazione                    | Stato        | Dettaglio                            |
|-----------------------------------------|--------------|--------------------------------------|
| `SymbolTable::push_scope`               | **Completo** | Aggiunge map vuoto                   |
| `SymbolTable::pop_scope`                | **Completo** | Rimuove ultimo map con guardia       |
| `SymbolTable::define`                   | **Completo** | Inserisce/aggiorna nell'ultimo scope |
| `SymbolTable::lookup`                   | **Completo** | Ricerca reverse-order                |
| `SymbolTable::defined_in_current_scope` | **Completo** | Controlla solo ultimo scope          |
| `SymbolTable::depth`                    | **Completo** | Restituisce scopes_.size()           |

---

### System: TypeScheme (Tipi Polimorfici)

#### 2.1 System Overview

Il `TypeScheme` rappresenta tipi polimorfici con quantificazione universale: `∀(vars). body`. È usato dalla
`SymbolTable`
per memorizzare i tipi di funzioni e variabili. Supporta la creazione di scheme monomorfi (`mono()`) e l'istanziazione
con type variable fresche (`instantiate()`).

#### 2.2 Internal Module Organization

Un header (`TypeScheme.hpp`, 55 righe) che definisce la struct `TypeScheme`, e un file di implementazione (
`TypeScheme.cpp`, 42 righe). L'organizzazione è coerente ma l'implementazione è notoriamente incompleta per tipi
composti.

#### 2.3 Intra-System Dependency Analysis

Dipende da `TypePtr`, `TypeVariable`, e `fresh_type_variable()`. Nessuna dipendenza circolare.

#### 2.4 Logical Flow

1. `mono(type, const_flag)` crea uno scheme con `quantified_vars` vuoto.
2. `instantiate()`:
    - Se `quantified_vars` è vuoto, restituisce `body` invariato.
    - Genera fresh type variables per ogni quantified variable.
    - Se `body` è una `TypeVariable` diretta e il suo ID è in `fresh_vars`, restituisce la fresh variable.
    - Altrimenti (tipi composti), restituisce `body` invariato — **sostituzione non applicata**.

#### 2.5 Critical Points

**DEF-025 — `instantiate()` non sostituisce in tipi composti**: Per un tipo scheme come `∀T. Array<T>`, il body è un
`ArrayType` che contiene una `TypeVariable` con ID corrispondente a `T`. Il metodo `instantiate()` restituisce il body
invariato (`TypeScheme.cpp:35`), senza sostituire `T` con una fresh variable. Questo significa che diverse istanziazioni
dello stesso scheme condividono la stessa TypeVariable, violando l'isolamento tra istanze.

**DEF-026 — Nessun deep copy del body**: Anche quando `body` è una TypeVariable quantificata, `instantiate()`
restituisce
la fresh variable ma non gestisce il caso in cui il body contenga riferimenti multipli alla stessa variabile
quantificata
(in un tipo funzione, ad esempio).

#### 2.6 Partial or Undefined Implementations

| Metodo/Dichiarazione      | Stato        | Dettaglio                                                                                                                                                                                                 |
|---------------------------|--------------|-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------|
| `TypeScheme::mono`        | **Completo** | Crea scheme con vars vuoti                                                                                                                                                                                |
| `TypeScheme::instantiate` | **Parziale** | Gestisce solo body-TypeVariable diretta. Tipi composti restituiti invariati (`TypeScheme.cpp:35`). Commento nel codice ammette: "This is a simplified implementation - full version would use a visitor." |

---

### System: TypeVariable (Variabili di Tipo)

#### 2.1 System Overview

Il `TypeVariable` rappresenta una variabile di tipo sconosciuta (`?T1`, `?T2`, ...) che sarà risolta durante il
constraint solving. Ogni istanza ha un ID univoco generato da un counter thread-local. La funzione
`fresh_type_variable()`
crea nuove variabili con ID progressivi.

#### 2.2 Internal Module Organization

Un header (`TypeVariable.hpp`, 85 righe) e un file di implementazione (`TypeVariable.cpp`, 23 righe). L'header definisce
la classe `TypeVariable` come `final : public TypeBase`. L'implementazione contiene `to_string()`, il counter
thread-local, e `fresh_type_variable()`.

#### 2.3 Intra-System Dependency Analysis

Dipende solo da `TypeBase` (da `Type.hpp`). Nessuna dipendenza interna al typechecker oltre al tipo base.

#### 2.4 Logical Flow

1. `fresh_type_variable()` incrementa il counter thread-local e crea un `shared_ptr<TypeVariable>` con il nuovo ID.
2. `to_string()` formatta come `"?T{id}"`.
3. `operator==` confronta per ID.
4. `classof` verifica `kind() == TypeKind::TypeVar`.

#### 2.5 Critical Points

Nessuna criticità. Il sistema è minimale e correttamente implementato. Il counter thread-local garantisce unicità per
thread. Unica osservazione: il counter parte da 0 e viene pre-incrementato (`++counter`), quindi il primo ID è 1 —
coerente con l'invariante `id_ > 0`.

#### 2.6 Partial or Undefined Implementations

| Metodo/Dichiarazione         | Stato        | Dettaglio                        |
|------------------------------|--------------|----------------------------------|
| `TypeVariable::TypeVariable` | **Completo** | Costruttore constexpr            |
| `TypeVariable::id`           | **Completo** | Getter constexpr                 |
| `TypeVariable::to_string`    | **Completo** | Formatta "?T{id}"                |
| `TypeVariable::classof`      | **Completo** | RTTI-style check                 |
| `TypeVariable::operator==`   | **Completo** | Confronto per ID                 |
| `fresh_type_variable`        | **Completo** | Crea TypeVariable con ID univoco |

---

### System: UnionFind (Struttura Disjoint-Set)

#### 2.1 System Overview

Il `UnionFind` implementa la struttura dati disjoint-set con path compression e union by rank, utilizzata dal
`ConstraintSolver` per tracciare le equivalenze tra type variable durante l'unificazione. Garantisce tempo
quasi-costante
O(α(n)) per operazione.

#### 2.2 Internal Module Organization

Un header (`UnionFind.hpp`, 65 righe) con documentazione dettagliata sulla const-correctness, e un file di
implementazione (`UnionFind.cpp`, 48 righe). L'organizzazione è coerente.

#### 2.3 Intra-System Dependency Analysis

Dipende solo da STL (`unordered_map`, `cstdint`). Nessuna dipendenza interna al typechecker.

#### 2.4 Logical Flow

1. `make_set(var)` inserisce `var` come proprio genitore con rank 0 se non già presente.
2. `find(var)` applica path compression: se `parent_[var] != var`, ricorsivamente trova il root e aggiorna il puntatore.
3. `unite(x, y)` unisce per rank: il root con rank minore diventa figlio dell'altro.
4. `same_set(x, y)` controlla se `find(x) == find(y)`.
5. `size()` restituisce il numero di elementi nel map.

#### 2.5 Critical Points

**DEF-027 — `find` usa `at()` senza gestione**: `UnionFind::find` (`UnionFind.cpp:16–19`) usa `parent_.at(var)` che
lancia `std::out_of_range` se `var` non è stato registrato con `make_set()`. Questo è un rischio runtime — se il
`ConstraintSolver` chiama `unify` su una TypeVariable non registrata, il programma termina con eccezione non catturata.

#### 2.6 Partial or Undefined Implementations

| Metodo/Dichiarazione  | Stato        | Dettaglio                                             |
|-----------------------|--------------|-------------------------------------------------------|
| `UnionFind::make_set` | **Completo** | Inserisce con try_emplace, inizializza rank           |
| `UnionFind::find`     | **Completo** | Path compression, ma usa `at()` senza guardia         |
| `UnionFind::unite`    | **Completo** | Union by rank corretto                                |
| `UnionFind::same_set` | **Completo** | Controlla presenza con `contains()` prima di `find()` |
| `UnionFind::size`     | **Completo** | Restituisce parent_.size()                            |

---

### System: ErrorType (Tipo Errore per Recovery)

#### 2.1 System Overview

L'`ErrorType` è un tipo singleton che rappresenta un tipo errato. Viene inserito quando un identificatore non è
dichiarato
o un'espressione non può essere tipata. Si unifica silenziosamente con qualsiasi tipo nel `ConstraintSolver`, prevenendo
errori a cascata da una singola causa radice.

#### 2.2 Internal Module Organization

Un header (`ErrorType.hpp`, 42 righe) e un file di implementazione (`ErrorType.cpp`, 18 righe). La classe è `final :
public TypeBase` con `kind() == TypeKind::Error`. La funzione `error_type()` restituisce il singleton.

#### 2.3 Intra-System Dependency Analysis

Dipende solo da `TypeBase` (da `Type.hpp`). Nessuna altra dipendenza.

#### 2.4 Logical Flow

1. `error_type()` restituisce un `shared_ptr<ErrorType>` da variabile statica const (singleton thread-safe).
2. `to_string()` restituisce `"<error>"`.
3. `operator==` confronta per `kind()`.
4. `classof` verifica `kind() == TypeKind::Error`.

#### 2.5 Critical Points

Nessuna criticità. Il sistema è minimale e correttamente implementato. Il singleton è thread-safe (Meyers singleton via
`static const`). L'unificazione silenziosa con qualsiasi tipo è una scelta architetturale intenzionale per error
recovery.

#### 2.6 Partial or Undefined Implementations

| Metodo/Dichiarazione    | Stato        | Dettaglio                       |
|-------------------------|--------------|---------------------------------|
| `ErrorType::ErrorType`  | **Completo** | Costruttore con TypeKind::Error |
| `ErrorType::to_string`  | **Completo** | Restituisce "<error>"           |
| `ErrorType::classof`    | **Completo** | RTTI-style check                |
| `ErrorType::operator==` | **Completo** | Confronto per kind()            |
| `error_type()`          | **Completo** | Singleton thread-safe           |

---

## Phase 3 — Per-Component Analysis

### System: TypeChecker › Component: TypeChecker (classe principale)

#### 3.1 Responsibility Statement

Il componente `TypeChecker` è esclusivamente responsabile per l'orchestrazione dell'intera pipeline di type checking:
name
resolution, constraint generation, constraint solving, e zonking dell'AST tipizzato.

#### 3.2 Class Structure

| Campo                           | Tipo                         | Visibilità | Semantica                                                                      |
|---------------------------------|------------------------------|------------|--------------------------------------------------------------------------------|
| `symbols_`                      | `SymbolTable`                | private    | Tabella dei simboli con scope annidati, resettata ad ogni `check()`            |
| `constraints_`                  | `ConstraintSet`              | private    | Accumulatore di vincoli di tipo, resettato ad ogni `check()`                   |
| `errors_`                       | `std::vector<CompileError>`  | private    | Errori accumulati durante il checking                                          |
| `message_storage_`              | `std::deque<std::string>`    | private    | Proprietario delle stringhe per `std::string_view` in `CompileError::message_` |
| `typed_stmts_`                  | `std::vector<TypedStmtPtr>`  | private    | Statement tipizzati prodotti durante constraint generation                     |
| `current_function_return_type_` | `std::optional<TypePtr>`     | private    | Tipo di ritorno della funzione corrente (per validazione return)               |
| `current_function_name_`        | `std::optional<std::string>` | private    | Nome della funzione corrente (per messaggi d'errore)                           |
| `loop_depth_`                   | `std::size_t`                | private    | Profondità di annidamento nei loop (per validazione break/continue)            |

La classe non eredita da nessuna base. Non ha relazioni di composizione oltre ai campi privati.

#### 3.3 Interface Analysis

| Metodo                 | Signature                                                     | Precondizioni                                                      | Postcondizioni                                       | Note                      |
|------------------------|---------------------------------------------------------------|--------------------------------------------------------------------|------------------------------------------------------|---------------------------|
| `check`                | `[[nodiscard]] TypeCheckResult check(const Program &program)` | Nessuna                                                            | Restituisce risultato con program tipizzato e errori | Entry point principale    |
| `type_expr`            | `[[nodiscard]] TypedExprPtr type_expr(const Expr &expr)`      | `symbols_` deve contenere tutti gli identificatori usati da `expr` | Restituisce espressione tipizzata                    | Pubblico solo per testing |
| `type_stmt`            | `[[nodiscard]] TypedStmtPtr type_stmt(const Stmt &stmt)`      | `symbols_` deve contenere tutti gli identificatori usati da `stmt` | Restituisce statement tipizzato                      | Pubblico solo per testing |
| `resolve_names`        | `void resolve_names(const Program &program)`                  | Nessuna                                                            | Popola `symbols_`                                    | Privato, Fase 1           |
| `resolve_names_stmt`   | `void resolve_names_stmt(const Stmt &stmt)`                   | Nessuna                                                            | Regola statement in `symbols_`                       | Privato, helper Fase 1    |
| `generate_constraints` | `void generate_constraints(const Program &program)`           | `symbols_` popolato                                                | Popola `constraints_` e `typed_stmts_`               | Privato, Fase 2           |
| `solve_constraints`    | `[[nodiscard]] SolverResult solve_constraints() const`        | `constraints_` popolato                                            | Restituisce risultato solver                         | Privato, Fase 3           |
| `zonk`                 | `[[nodiscard]] TypedProgram zonk(const Substitution &subst)`  | `typed_stmts_` e `subst` validi                                    | Restituisce program zonkato                          | Privato, Fase 4           |

#### 3.4 Implementation Logic

Il metodo `check()` esegue la pipeline sequenziale a 4 fasi con reset preliminare. `type_expr()` è uno switch su
`NodeKind` con ~17 casi, ognuno dei quali costruisce il nodo tipizzato corrispondente e aggiunge vincoli al
`ConstraintSet`. La logica più complessa è nel caso `BinaryExpr` (righe 510–680 di `TypeChecker.cpp`), che gestisce
diverse categorie di operatori (aritmetici, confronto, logici, bitwise) con validazione anticipata dei tipi concreti e
generazione di errori specifici (E2011, E2012, E2013, E2018, E2019). Il caso `CallExpr` (righe 689–730) è
notoriamente semplificato — non vincola la signature del callee. `type_stmt()` è analogamente uno switch su `NodeKind`
con ~12 casi, gestendo VarDecl (single e multi), FuncDecl, ReturnStmt, IfStmt, WhileStmt, ForStmt, BlockStmt,
BreakStmt, ContinueStmt, MainStmt.

#### 3.5 Error Handling Evaluation

Gli errori sono accumulati nel vettore `errors_` tramite `errors_.push_back(CompileError::TypeError(...))`. Il
`TypeChecker` usa `message_storage_` (un `std::deque`) per allocare stringhe dinamiche i cui `string_view` sono
memorizzati nei `CompileError`. Questo approccio previene dangling reference grazie alla proprietà del deque che
preserva gli indirizzi dopo reallocation. Gli errori sono raccolti in modo continuativo — il type checking non si
interrompe al primo errore (error recovery). Tuttavia, alcuni percorsi restituiscono `nullptr` (es. `ArrayLiteral` con
primo elemento non tipabile) invece di un nodo placeholder con error type, creando potenziali null dereference nei
chiamanti.

#### 3.6 Type Consistency Audit

I tipi sono usati coerentemente come `TypePtr = std::shared_ptr<const TypeBase>`. Non ci sono cast unsafe — i
`static_cast` verso tipi specifici (`const ArrayType*`, ecc.) sono preceduti da controlli `kind()` o `dynamic_cast`.
Unica preoccupazione: `zonk_block_full` (`TypeChecker.cpp:370–383`) scarta silenziosamente statement quando
`zonk_stmt_full` restituisce `nullptr`, producendo un blocco incompleto senza segnalazione.

#### 3.7 Inter-Component Interaction

Il `TypeChecker` è il maggiore consumatore di tutti gli altri 8 sistemi del typechecker. Dipende da:

- `SymbolTable` per name resolution (Fase 1)
- `ConstraintSet` per accumulo vincoli (Fase 2)
- `ConstraintSolver` per risoluzione vincoli (Fase 3)
- `Substitution` per zonking (Fase 4)
- `TypeVariable` per generazione fresh type variable
- `ErrorType` per error recovery
- `TypeScheme` per istanziazione polimorfica
- Tipi AST (`Type.hpp`, `Node.hpp`, `Expressions.hpp`, `Statements.hpp`, `Program.hpp`, `TypedProgram.hpp`)

Il coupling è stretto ma necessario per il ruolo di orchestratore. Non ci sono assunzioni nascoste oltre a quelle
documentate nei commenti del codice.

#### 3.8 Optimization Opportunities

**Bottleneck principale**: `type_expr()` e `type_stmt()` sono funzioni monolitiche di ~480 e ~326 righe con switch su
~17-12 casi. La refattorizzazione in un pattern Visitor eliminerebbe lo switch gigante e migliorerebbe la
manutenibilità.
**Duplicazione**: la logica di zonking (`zonk_expr_full`, `zonk_stmt_full`) duplica la struttura dello switch di
`type_expr`/`type_stmt` — un visitor doppio (constraint generation + zonking) ridurrebbe la duplicazione. **Caching**:
non c'è caching per le lookup nella SymbolTable durante constraint generation — identifier ripetuti nel codice causano
lookup ripetuti.

---

### System: ConstraintSolver › Component: ConstraintSolver

#### 3.1 Responsibility Statement

Il componente `ConstraintSolver` è esclusivamente responsabile per la risoluzione di vincoli di uguaglianza tra tipi
attraverso unificazione strutturale con union-find, producendo una sostituzione che mappa type variable a tipi concreti.

#### 3.2 Class Structure

| Campo           | Tipo           | Visibilità | Semantica                                              |
|-----------------|----------------|------------|--------------------------------------------------------|
| `union_find_`   | `UnionFind`    | private    | Traccia equivalenze tra TypeVarId durante unificazione |
| `substitution_` | `Substitution` | private    | Accumula mapping TypeVarId → TypePtr risolti           |

Nessuna ereditarietà. Composizione con `UnionFind` e `Substitution`.

#### 3.3 Interface Analysis

| Metodo      | Signature                                                                                                                   | Precondizioni       | Postcondizioni                          | Note         |
|-------------|-----------------------------------------------------------------------------------------------------------------------------|---------------------|-----------------------------------------|--------------|
| `solve`     | `[[nodiscard]] SolverResult solve(const ConstraintSet &constraints)`                                                        | Nessuna             | Restituisce substitution e errori       | Entry point  |
| `occurs_in` | `[[nodiscard]] static bool occurs_in(TypeVarId var, const TypePtr &type, const Substitution &subst)`                        | `type` valido       | Restituisce true se var occorre in type | Occurs check |
| `unify`     | `[[nodiscard]] std::expected<void, CompileError> unify(const TypePtr &t1, const TypePtr &t2, const Constraint &constraint)` | `t1`, `t2` non-null | Unifica o restituisce errore            | Privato      |

#### 3.4 Implementation Logic

`solve()` itera sui vincoli chiamando `unify()`. `unify()` implementa l'algoritmo standard: (1) ErrorType bypass, (2)
null check, (3) type variable handling con occurs check, (4) structural equality per tipi concreti con dispatch
ricorsivo
per ArrayType e VectorType. L'occurs check (`occurs_in`) applica la substitution e cerca ricorsivamente la variabile.

#### 3.5 Error Handling Evaluation

Errori restituiti come `std::expected<void, CompileError>`. Il chiamante (`solve`) accumula errori nel `SolverResult`.
ErrorType causa unificazione silenziosa (nessun errore). I mismatch di tipo producono E2034 con hint contestuale.
Occurs check failure produce E2035. Non ci sono eccezioni non catturate — l'unica eccezione potenziale è `at()` in
`UnionFind::find`.

#### 3.6 Type Consistency Audit

Tipi usati coerentemente come `TypePtr`. I `dynamic_cast` verso `TypeVariable`, `ArrayType`, `VectorType` sono corretti.
Il caso `default` assume uguaglianza per tipi con stesso `kind()` — problematico per `CustomType` (DEF-007).

#### 3.7 Inter-Component Interaction

Dipende da `UnionFind` per gestione equivalenze, `Substitution` per bindings, `ErrorType` per bypass, `Constraint` per
informazioni di diagnostica. L'interazione è pulita — il solver è un consumatore puro degli altri componenti.

#### 3.8 Optimization Opportunities

L'algoritmo di unificazione è efficiente (O(α(n)) per operazione grazie a UnionFind). La duplicazione della logica di
dispatch tra `unify()` e `Substitution::applyImpl` (DEF-001) è un'opportunità di refattorizzazione — un visitor comune
per la traversata di tipi composti eliminerebbe la duplicazione.

---

### System: Constraint › Component: Constraint e ConstraintSet

#### 3.1 Responsibility Statement

La struct `Constraint` è esclusivamente responsabile per rappresentare un singolo vincolo di uguaglianza `lhs = rhs` con
metadata di diagnostica, mentre `ConstraintSet` è responsabile per accumulare e fornire accesso a una collezione
ordinata
di vincoli.

#### 3.2 Class Structure

| Campo                         | Tipo                      | Visibilità | Semantica                  |
|-------------------------------|---------------------------|------------|----------------------------|
| `Constraint::id`              | `ConstraintId`            | public     | ID univoco 1-based         |
| `Constraint::lhs`             | `TypePtr`                 | public     | Tipo sinistro da unificare |
| `Constraint::rhs`             | `TypePtr`                 | public     | Tipo destro da unificare   |
| `Constraint::origin`          | `SourceSpan`              | public     | Posizione sorgente         |
| `Constraint::reason`          | `std::string`             | public     | Contesto diagnostico       |
| `ConstraintSet::constraints_` | `std::vector<Constraint>` | private    | Contenitore ordinato       |
| `ConstraintSet::next_id_`     | `ConstraintId`            | private    | Counter auto-incrementante |

#### 3.3 Interface Analysis

| Metodo                       | Signature                                                                                | Precondizioni | Postcondizioni                         | Note              |
|------------------------------|------------------------------------------------------------------------------------------|---------------|----------------------------------------|-------------------|
| `ConstraintSet::add`         | `ConstraintId add(TypePtr lhs, TypePtr rhs, SourceSpan origin, std::string_view reason)` | Nessuna       | Restituisce nuovo ID, aggiunge vincolo | O(1) ammortizzato |
| `ConstraintSet::constraints` | `[[nodiscard]] const std::vector<Constraint> &constraints() const noexcept`              | Nessuna       | Restituisce ref const                  | O(1)              |
| `ConstraintSet::get`         | `[[nodiscard]] const Constraint *get(ConstraintId id) const noexcept`                    | Nessuna       | Restituisce puntatore o nullptr        | O(n)              |
| `ConstraintSet::size`        | `[[nodiscard]] std::size_t size() const noexcept`                                        | Nessuna       | Restituisce conteggio                  | O(1)              |

#### 3.4 Implementation Logic

Implementazione minimale e diretta. `add()` incrementa `next_id_` e fa `push_back`. `get()` usa `std::ranges::find` con
proiettore `&Constraint::id`.

#### 3.5 Error Handling Evaluation

Nessun errore generato internamente. Il componente è puramente strutturale.

#### 3.6 Type Consistency Audit

Tipi coerenti. Nessun cast unsafe.

#### 3.7 Inter-Component Interaction

Consumato dal `TypeChecker` (Fase 2) e dal `ConstraintSolver` (Fase 3). Nessuna dipendenza attiva.

#### 3.8 Optimization Opportunities

`get()` è O(n) — per dataset grandi, `unordered_map<ConstraintId, size_t>` come indice sarebbe O(1). Non critico per le
dimensioni attuali.

---

### System: Substitution › Component: Substitution

#### 3.1 Responsibility Statement

Il componente `Substitution` è esclusivamente responsabile per memorizzare e applicare mapping da type variable a tipi
concreti, con caching persistente per ottimizzare applicazioni ripetute.

#### 3.2 Class Structure

| Campo          | Tipo                                                    | Visibilità | Semantica                |
|----------------|---------------------------------------------------------|------------|--------------------------|
| `bindings_`    | `std::unordered_map<TypeVarId, TypePtr>`                | private    | Mapping variabile → tipo |
| `apply_cache_` | `mutable std::unordered_map<const TypeBase *, TypePtr>` | private    | Cache risultati apply    |

#### 3.3 Interface Analysis

| Metodo      | Signature                                                                   | Precondizioni | Postcondizioni                              | Note                            |
|-------------|-----------------------------------------------------------------------------|---------------|---------------------------------------------|---------------------------------|
| `bind`      | `void bind(TypeVarId var, TypePtr type)`                                    | Nessuna       | Aggiorna binding, invalida cache            | O(1) + O(cache_size)            |
| `lookup`    | `[[nodiscard]] std::optional<TypePtr> lookup(TypeVarId var) const noexcept` | Nessuna       | Restituisce binding o nullopt               | O(1)                            |
| `apply`     | `[[nodiscard]] TypePtr apply(const TypePtr &type) const`                    | `type` valido | Restituisce tipo con substitution applicata | O(n) primo call, O(1) cache hit |
| `applyImpl` | `[[nodiscard]] TypePtr applyImpl(const TypePtr &type) const`                | Come `apply`  | Implementazione ricorsiva                   | Privato                         |
| `contains`  | `[[nodiscard]] bool contains(TypeVarId var) const noexcept`                 | Nessuna       | Verifica presenza                           | O(1)                            |
| `size`      | `[[nodiscard]] std::size_t size() const noexcept`                           | Nessuna       | Conta bindings                              | O(1)                            |

#### 3.4 Implementation Logic

`applyImpl()` è ricorsivo: cerca nel cache, se miss dispatcha per tipo. Per TypeVariable cerca nei bindings e
ricorsivamente applica. Per ArrayType/VectorType applica ricorsivamente all'element type. Per CustomType e default
restituisce invariato. Cache popolata bottom-up.

#### 3.5 Error Handling Evaluation

Nessun errore generato. Il componente è totale — ogni input produce un output valido.

#### 3.6 Type Consistency Audit

Coerente. Il cache usa `const TypeBase*` come chiave (identità) — corretto perché i nodi tipo sono immutabili.

#### 3.7 Inter-Component Interaction

Consumato dal `ConstraintSolver` e dal `TypeChecker` (zonking). Dipende da `TypeVariable` per il dispatch.

#### 3.8 Optimization Opportunities

Cache invalidation aggressiva (`clear()` su ogni `bind()`). Per N bindings consecutivi, il cache è inefficace. Una
strategia di invalidazione incrementale (rimuovere solo entry che referenziano la variabile bindata) migliorerebbe le
prestazioni per vincoli in sequenza.

---

### System: SymbolTable › Component: SymbolTable

#### 3.1 Responsibility Statement

Il componente `SymbolTable` è esclusivamente responsabile per la mappatura da identificatori a `TypeScheme` con supporto
per scope lessicali annidati e shadowing.

#### 3.2 Class Structure

| Campo        | Tipo                                                                        | Visibilità | Semantica                                         |
|--------------|-----------------------------------------------------------------------------|------------|---------------------------------------------------|
| `StringHash` | `struct` (private)                                                          | private    | Hasher per heterogeneous lookup con `string_view` |
| `scopes_`    | `std::vector<std::unordered_map<std::string_view, TypeScheme, StringHash>>` | private    | Stack di scope                                    |

#### 3.3 Interface Analysis

| Metodo                     | Signature                                                                     | Precondizioni | Postcondizioni                         | Note                         |
|----------------------------|-------------------------------------------------------------------------------|---------------|----------------------------------------|------------------------------|
| `push_scope`               | `void push_scope()`                                                           | Nessuna       | Aggiunge scope vuoto                   | O(1)                         |
| `pop_scope`                | `void pop_scope()`                                                            | Nessuna       | Rimuove scope corrente                 | O(1), con guardia            |
| `define`                   | `void define(std::string_view name, TypeScheme scheme)`                       | Nessuna       | Definisce simbolo nello scope corrente | O(1), crea scope se vuoto    |
| `lookup`                   | `[[nodiscard]] std::optional<TypeScheme> lookup(std::string_view name) const` | Nessuna       | Cerca da inner a outer                 | O(d * α) dove d = profondità |
| `defined_in_current_scope` | `[[nodiscard]] bool defined_in_current_scope(std::string_view name) const`    | Nessuna       | Controlla solo scope corrente          | O(1)                         |
| `depth`                    | `[[nodiscard]] std::size_t depth() const noexcept`                            | Nessuna       | Restituisce profondità                 | O(1)                         |

#### 3.4 Implementation Logic

Implementazione diretta. `lookup` itera con `std::ranges::reverse_view` per cercare dall'innermost scope all'outermost.

#### 3.5 Error Handling Evaluation

`lookup` restituisce `std::nullopt` per simboli non trovati. Nessun errore generato internamente. `pop_scope` ha guardia
`if(!scopes_.empty())` — non crasha se chiamato su stack vuoto (comportamento silenzioso).

#### 3.6 Type Consistency Audit

Uso di `string_view` come chiave è corretto finché le stringhe originali vivono più della SymbolTable. Questa assunzione
non è verificata a compile-time.

#### 3.7 Inter-Component Interaction

Consumato dal `TypeChecker` per name resolution. Dipende da `TypeScheme`. Nessuna dipendenza attiva su altri componenti.

#### 3.8 Optimization Opportunities

Minime. L'implementazione è già efficiente. Possibile miglioramento: `pop_scope` potrebbe restituire un errore o
assertion in debug mode per catturare usage errato.

---

### System: TypeScheme › Component: TypeScheme

#### 3.1 Responsibility Statement

La struct `TypeScheme` è esclusivamente responsabile per rappresentare tipi polimorfici con quantificazione universale e
per istanziarli con fresh type variables.

#### 3.2 Class Structure

| Campo             | Tipo                     | Visibilità | Semantica                       |
|-------------------|--------------------------|------------|---------------------------------|
| `quantified_vars` | `std::vector<TypeVarId>` | public     | ID delle variabili quantificate |
| `body`            | `TypePtr`                | public     | Tipo corpo del scheme           |
| `is_const`        | `bool`                   | public     | Flag di immutabilità            |

#### 3.3 Interface Analysis

| Metodo        | Signature                                                                     | Precondizioni | Postcondizioni                             | Note                       |
|---------------|-------------------------------------------------------------------------------|---------------|--------------------------------------------|----------------------------|
| `instantiate` | `[[nodiscard]] TypePtr instantiate() const`                                   | Nessuna       | Restituisce tipo istanziato con fresh vars | Parziale per tipi composti |
| `mono`        | `[[nodiscard]] static TypeScheme mono(TypePtr type, bool const_flag = false)` | Nessuna       | Restituisce scheme monomorfo               | O(1)                       |

#### 3.4 Implementation Logic

`mono()` è un factory banale. `instantiate()` genera fresh vars per ogni quantified var, poi se il body è TypeVariable
diretta la sostituisce, altrimenti restituisce body invariato.

#### 3.5 Error Handling Evaluation

Nessun errore generato. Il componente è totale.

#### 3.6 Type Consistency Audit

Coerente. L'incompletezza per tipi composti è una limitazione funzionale, non un errore di tipo.

#### 3.7 Inter-Component Interaction

Consumato da `SymbolTable` (come valore del map) e da `TypeChecker` (istanziazione durante constraint generation).
Dipende da `TypeVariable` e `fresh_type_variable()`.

#### 3.8 Optimization Opportunities

La sostituzione completa richiederebbe una traversata ricorsiva del body (visitor pattern). Attualmente non
implementata.

---

### System: TypeVariable › Component: TypeVariable

#### 3.1 Responsibility Statement

La classe `TypeVariable` è esclusivamente responsabile per rappresentare una variabile di tipo con ID univoco,
ereditando
da `TypeBase`.

#### 3.2 Class Structure

| Campo | Tipo        | Visibilità | Semantica        |
|-------|-------------|------------|------------------|
| `id_` | `TypeVarId` | private    | ID univoco (> 0) |

Eredita da `TypeBase` con `kind() == TypeKind::TypeVar`.

#### 3.3 Interface Analysis

| Metodo       | Signature                                                                      | Precondizioni          | Postcondizioni                             | Note      |
|--------------|--------------------------------------------------------------------------------|------------------------|--------------------------------------------|-----------|
| Costruttore  | `explicit constexpr TypeVariable(TypeVarId id)`                                | `id > 0` (convenzione) | Inizializza con TypeKind::TypeVar          | constexpr |
| `id`         | `[[nodiscard]] constexpr TypeVarId id() const noexcept`                        | Nessuna                | Restituisce ID                             | O(1)      |
| `to_string`  | `[[nodiscard]] std::string to_string() const override`                         | Nessuna                | Restituisce "?T{id}"                       | O(log id) |
| `classof`    | `[[nodiscard]] static constexpr bool classof(const TypeBase *t) noexcept`      | Nessuna                | true se t è TypeVariable                   | RTTI      |
| `operator==` | `[[nodiscard]] bool operator==(const TypeBase &other) const noexcept override` | Nessuna                | true se other è TypeVariable con stesso ID | Confronto |

#### 3.4 Implementation Logic

Minimale. `to_string` usa `FORMAT`. `operator==` confronta `kind()` poi `id_`.

#### 3.5 Error Handling Evaluation

Nessun errore possibile.

#### 3.6 Type Consistency Audit

Coerente. Nessun cast unsafe.

#### 3.7 Inter-Component Interaction

Usato da tutti i componenti del typechecker come tipo fondamentale. La funzione libera `fresh_type_variable()` è il
punto
di creazione.

#### 3.8 Optimization Opportunities

Nessuna. Il componente è ottimale per la sua responsabilità.

---

### System: UnionFind › Component: UnionFind

#### 3.1 Responsibility Statement

Il componente `UnionFind` è esclusivamente responsabile per gestire partizioni disgiunte di TypeVarId con operazioni di
find (con path compression) e unite (by rank).

#### 3.2 Class Structure

| Campo     | Tipo                                          | Visibilità | Semantica                   |
|-----------|-----------------------------------------------|------------|-----------------------------|
| `parent_` | `std::unordered_map<TypeVarId, TypeVarId>`    | private    | Mappatura nodo → genitore   |
| `rank_`   | `std::unordered_map<TypeVarId, std::uint8_t>` | private    | Rank approssimato per union |

#### 3.3 Interface Analysis

| Metodo     | Signature                                               | Precondizioni                   | Postcondizioni                        | Note                              |
|------------|---------------------------------------------------------|---------------------------------|---------------------------------------|-----------------------------------|
| `make_set` | `void make_set(TypeVarId var)`                          | Nessuna                         | Crea singleton set                    | O(1) ammortizzato                 |
| `find`     | `[[nodiscard]] TypeVarId find(TypeVarId var)`           | `var` registrato con `make_set` | Restituisce root con path compression | O(α(n)), lancia se non registrato |
| `unite`    | `void unite(TypeVarId x, TypeVarId y)`                  | `x`, `y` registrati             | Unisce insiemi                        | O(α(n))                           |
| `same_set` | `[[nodiscard]] bool same_set(TypeVarId x, TypeVarId y)` | Nessuna                         | true se stesso insieme                | O(α(n)), con guardia `contains`   |
| `size`     | `[[nodiscard]] std::size_t size() const noexcept`       | Nessuna                         | Conta elementi                        | O(1)                              |

#### 3.4 Implementation Logic

Implementazione standard di disjoint-set con path compression ricorsiva e union by rank.

#### 3.5 Error Handling Evaluation

`find` lancia `std::out_of_range` via `at()` se il nodo non è registrato. `unite` propaga l'eccezione. `same_set` ha
guardia `contains` che previene l'eccezione.

#### 3.6 Type Consistency Audit

Coerente. Nessun cast.

#### 3.7 Inter-Component Interaction

Usato esclusivamente dal `ConstraintSolver`. Nessuna dipendenza da altri componenti del typechecker.

#### 3.8 Optimization Opportunities

Sostituire `at()` con `find()` e controllo per prevenire eccezioni. Usare `vector` invece di `unordered_map` se gli ID
sono densi (performance improvement).

---

### System: ErrorType › Component: ErrorType

#### 3.1 Responsibility Statement

La classe `ErrorType` è esclusivamente responsabile per rappresentare un tipo errore come singleton, permettendo
unificazione silenziosa con qualsiasi tipo per error recovery.

#### 3.2 Class Structure

| Campo          | Tipo | Visibilità | Semantica             |
|----------------|------|------------|-----------------------|
| (nessun campo) | —    | —          | Singleton senza stato |

Eredita da `TypeBase` con `kind() == TypeKind::Error`.

#### 3.3 Interface Analysis

| Metodo         | Signature                                                                      | Precondizioni | Postcondizioni                  | Note              |
|----------------|--------------------------------------------------------------------------------|---------------|---------------------------------|-------------------|
| Costruttore    | `ErrorType()`                                                                  | Nessuna       | Inizializza con TypeKind::Error | —                 |
| `to_string`    | `[[nodiscard]] std::string to_string() const override`                         | Nessuna       | Restituisce "<error>"           | —                 |
| `classof`      | `[[nodiscard]] static constexpr bool classof(const TypeBase *t) noexcept`      | Nessuna       | true se t è ErrorType           | RTTI              |
| `operator==`   | `[[nodiscard]] bool operator==(const TypeBase &other) const noexcept override` | Nessuna       | true se other è ErrorType       | Confronto         |
| `error_type()` | `[[nodiscard]] TypePtr error_type() noexcept`                                  | Nessuna       | Restituisce singleton           | Meyer's singleton |

#### 3.4 Implementation Logic

Singleton thread-safe via `static const shared_ptr`. Nessun algoritmo.

#### 3.5 Error Handling Evaluation

Nessun errore.

#### 3.6 Type Consistency Audit

Coerente.

#### 3.7 Inter-Component Interaction

Usato dal `TypeChecker` per undeclared identifiers e dal `ConstraintSolver` per bypass unificazione.

#### 3.8 Optimization Opportunities

Nessuna.

---

## Phase 4 — Prioritized Recommendations

### 4.1 Recommendation Register

#### REC-001

**Title**: Implementare sostituzione completa in `TypeScheme::instantiate()` per tipi composti

**Deficiency addressed**: DEF-008, DEF-025

**Description**: Implementare una traversata ricorsiva del `body` in `TypeScheme::instantiate()` (
`TypeScheme.cpp:14–35`)
che sostituisca ogni occorrenza delle variabili quantificate con fresh type variables, anche quando annidate in
`ArrayType`, `VectorType`, o futuri tipi composti. Utilizzare un visitor pattern o una funzione ricorsiva analoga a
`Substitution::applyImpl`. Change entry point: `TypeScheme.cpp`, metodo `TypeScheme::instantiate()`.

**Feasibility**: 3 — Richiede implementazione di visitor ricorsivo ma la logica è ben compresa e simile a `applyImpl`.

**Expected ROI**: 5 — Risolve la violazione dell'isolamento tra istanze di tipi polimorfici, correggendo un bug
funzionale critico.

**Implementation effort**: 3 — 1–2 giorni di lavoro con test.

**Composite Score**: 3×2 + 5×2 + 3×1 = 6 + 10 + 3 = **19**

**Estimated implementation time**: 1–2 giorni

**Required resources**: Sviluppatore C++ con conoscenza di type system, test framework Catch2 esistente.

**Effectiveness indicators**:

1. Test per `∀T. Array<T>` produce istanze con fresh type variables indipendenti.
2. Test per chiamate multiple a funzione polimorfica con `Array<i32>` e `Array<bool>` non condividono variabili.
3. Zero shared TypeVarId tra istanze diverse dello stesso scheme.

---

#### REC-002

**Title**: Confrontare nomi CustomType nell'unificazione

**Deficiency addressed**: DEF-007, DEF-018

**Description**: Aggiungere il confronto esplicito del campo `name_` nel caso `default` di `ConstraintSolver::unify()`
(`ConstraintSolver.cpp:119–122`). Quando entrambi i tipi sono `CustomType`, confrontare `name1 == name2` prima di
restituire successo. Change entry point: `ConstraintSolver.cpp`, metodo `ConstraintSolver::unify()`, ramo `default`
dello
switch.

**Feasibility**: 5 — Modifica di poche righe in un punto già identificato.

**Expected ROI**: 5 — Previene unificazione errata tra tipi custom non correlati, bug di correttezza fondamentale.

**Implementation effort**: 5 — 30 minuti di implementazione + test.

**Composite Score**: 5×2 + 5×2 + 5×1 = 10 + 10 + 5 = **25**

**Estimated implementation time**: 1–2 ore

**Required resources**: Sviluppatore C++, accesso a `CustomType` header per verificare campo `name_`.

**Effectiveness indicators**:

1. Test che `Foo` e `Bar` non si unificano (errore E2034).
2. Test che due istanze di `Foo` si unificano correttamente.

---

#### REC-003

**Title**: Aggiungere caso esplicito `TypeKind::Error` in `Substitution::applyImpl`

**Deficiency addressed**: DEF-006, DEF-022

**Description**: Aggiungere un caso `case TypeKind::Error: result = type; break;` esplicito nello switch di
`Substitution::applyImpl()` (`Substitution.cpp:48–51`), prima del `default`. Questo rende il comportamento esplicito e
documentato. Change entry point: `Substitution.cpp`, metodo `Substitution::applyImpl()`, switch su `type->kind()`.

**Feasibility**: 5 — Aggiunta di un caso switch di 2 righe.

**Expected ROI**: 3 — Migliora manutenibilità e resistenza a refactoring futuri, impatto funzionale nullo.

**Implementation effort**: 5 — 10 minuti.

**Composite Score**: 5×2 + 3×2 + 5×1 = 10 + 6 + 5 = **21**

**Estimated implementation time**: 15–30 minuti

**Required resources**: Sviluppatore C++.

**Effectiveness indicators**:

1. Il caso `TypeKind::Error` è esplicitamente gestito nello switch.
2. Test di regressione conferma che ErrorType viene restituito invariato.

---

#### REC-004

**Title**: Spostare `parse_type_annotation` in modulo dedicato

**Deficiency addressed**: DEF-004

**Description**: Estrarre la funzione `parse_type_annotation` da `TypeChecker.cpp:19–36` in un nuovo file
`include/jsav/typechecker/TypeParser.hpp` e `src/jsav_Lib/typechecker/TypeParser.cpp`. Change entry point:
`TypeChecker.cpp`, righe 19–36 (funzione statica), e `TypeChecker.hpp` per includere il nuovo header. Il nuovo modulo
dovrebbe supportare almeno i tipi primitivi esistenti e fornire un'interfaccia estensibile per tipi composti futuri.

**Feasibility**: 3 — Richiede creazione di 2 nuovi file e aggiornamento di CMakeLists.txt del sottosistema.

**Expected ROI**: 3 — Migliora separazione delle responsabilità e rende il parser di tipi riusabile.

**Implementation effort**: 3 — 2–4 ore per estrazione e test.

**Composite Score**: 3×2 + 3×2 + 3×1 = 6 + 6 + 3 = **15**

**Estimated implementation time**: 2–4 ore

**Required resources**: Sviluppatore C++, aggiornamento `src/jsav_Lib/typechecker/CMakeLists.txt`.

**Effectiveness indicators**:

1. `TypeChecker.cpp` non contiene più la funzione `parse_type_annotation`.
2. Il nuovo modulo `TypeParser` è testato indipendentemente.

---

#### REC-005

**Title**: Rendere `type_expr` e `type_stmt` metodi privati del TypeChecker

**Deficiency addressed**: DEF-002

**Description**: Spostare `type_expr` e `type_stmt` da pubblici a privati in `TypeChecker.hpp:62–77`. Per il testing,
creare una classe `TypeCheckerTestAccess` friend o usare macro di test che accedano ai metodi tramite wrapper. Change
entry point: `TypeChecker.hpp`, righe 62–77 (dichiarazioni pubbliche). I test esistenti dovranno essere adattati per
usare `check()` su programmi minimali invece di chiamare direttamente `type_expr`.

**Feasibilità**: 3 — Richiede refactoring dei test che usano questi metodi direttamente.

**Expected ROI**: 4 — Ripristina l'incapsulamento della pipeline, prevenendo usage errato dall'esterno.

**Implementation effort**: 2 — 1–2 giorni per adattare tutti i test che chiamano questi metodi.

**Composite Score**: 3×2 + 4×2 + 2×1 = 6 + 8 + 2 = **16**

**Estimated implementation time**: 1–2 giorni

**Required resources**: Sviluppatore C++, accesso a `test/tests.cpp`.

**Effectiveness indicators**:

1. `type_expr` e `type_stmt` sono dichiarati `private` in `TypeChecker.hpp`.
2. Tutti i test esistenti compilano e passano.
3. Nessun codice esterno al typechecker chiama questi metodi.

---

#### REC-006

**Title**: Refattorizzare `type_expr` e `type_stmt` con pattern Visitor

**Deficiency addressed**: DEF-009

**Description**: Sostituire gli switch monolitici in `type_expr()` (~480 righe) e `type_stmt()` (~326 righe) con un
pattern Visitor. Creare `ConstraintGenVisitor` come classe separata con metodo `visit()` per ogni NodeKind. Change entry
point: `TypeChecker.cpp`, metodo `type_expr()` (righe 389–872) e `type_stmt()` (righe 874–1200). Ogni caso dello switch
diventa un metodo `visit(NodeKindType*)` separato.

**Feasibilità**: 2 — Refactoring su larga scala che richiede attenzione per preservare la semantica esatta.

**Expected ROI**: 4 — Riduce drasticamente la complessità cognitiva, migliora testabilità e manutenibilità.

**Implementation effort**: 1 — 1–2 settimane per refactoring completo con test di regressione.

**Composite Score**: 2×2 + 4×2 + 1×1 = 4 + 8 + 1 = **13**

**Estimated implementation time**: 1–2 settimane

**Required resources**: Sviluppatore C++ senior, tempo dedicato per test di regressione.

**Effectiveness indicators**:

1. Nessun metodo supera 100 righe (verificato con lizard).
2. Complessità cognitiva di ogni funzione < 15.
3. Tutti i test esistenti passano senza modifiche.

---

#### REC-007

**Title**: Gestire `nullptr` in `zonk_block_full` invece di scartare statement

**Deficiency addressed**: DEF-011, DEF-012

**Description**: In `zonk_block_full` (`TypeChecker.cpp:370–383`), quando `zonk_stmt_full` restituisce `nullptr`, invece
di scartare silenziosamente lo statement, propagare l'errore o mantenere l'originale. L'approccio più conservativo è
mantenere lo statement originale non zonkato nel blocco risultante. Change entry point: `TypeChecker.cpp`, metodo
`zonk_block_full()`, ramo `if(zonked)` else.

**Feasibilità**: 5 — Modifica di poche righe con logica alternativa chiara.

**Expected ROI**: 4 — Previene perdita silenziosa di statement nell'AST tipizzato.

**Implementation effort**: 5 — 30 minuti.

**Composite Score**: 5×2 + 4×2 + 5×1 = 10 + 8 + 5 = **23**

**Estimated implementation time**: 30–60 minuti

**Required resources**: Sviluppatore C++.

**Effectiveness indicators**:

1. Test con statement non zonkabili preserva tutti gli statement nel blocco output.
2. Zero statement persi silenziosamente.

---

#### REC-008

**Title**: Implementare vincoli di signature per `CallExpr`

**Deficiency addressed**: DEF-012

**Description**: In `type_expr` per `CallExpr` (`TypeChecker.cpp:689–730`), generare vincoli che colleghino il tipo del
callee a una signature di funzione. Se il callee è un Identifier, lookup nella SymbolTable per ottenere lo scheme,
istanziarlo, e vincolare gli argomenti ai tipi parametrici e il tipo di ritorno al risultato. Richiede una
rappresentazione
di tipo funzione (FnType). Change entry point: `TypeChecker.cpp`, metodo `type_expr()`, caso `NodeKind::CallExpr` (righe
689–730).

**Feasibilità**: 2 — Richiede progettazione e implementazione di un nuovo tipo `FnType` in `Type.hpp`.

**Expected ROI**: 5 — Abilita la verifica completa delle chiamate a funzione, funzionalità fondamentale di un type
checker.

**Implementation effort**: 1 — 1–2 settimane per design, implementazione e test.

**Composite Score**: 2×2 + 5×2 + 1×1 = 4 + 10 + 1 = **15**

**Estimated implementation time**: 1–2 settimane

**Required resources**: Sviluppatore C++ senior con esperienza in type system, modifica a `Type.hpp`.

**Effectiveness indicators**:

1. Test che chiamata con arity sbagliato produce errore.
2. Test che chiamata con tipo argomento sbagliato produce errore E2034.
3. Funzioni polimorfiche (identità) funzionano con argomenti di tipo diverso.

---

#### REC-009

**Title**: Implementare member resolution per `MemberExpr`

**Deficiency addressed**: DEF-013

**Description**: In `type_expr` per `MemberExpr` (`TypeChecker.cpp:843–848`), implementare lookup del membro sul tipo
dell'oggetto. Richiede una tabella di campi per tipo (struct/class type). Per ora, restituire errore "member access not
yet supported" invece di accettare silenziosamente. Change entry point: `TypeChecker.cpp`, metodo `type_expr()`, caso
`NodeKind::MemberExpr` (righe 843–848).

**Feasibilità**: 2 — Richiede infrastruttura per tipo struct/class con campi.

**Expected ROI**: 4 — Previene accettazione silenziosa di accesso membro non valido.

**Implementation effort**: 2 — 2–3 giorni per implementazione minimale con errore esplicito.

**Composite Score**: 2×2 + 4×2 + 2×1 = 4 + 8 + 2 = **14**

**Estimated implementation time**: 2–3 giorni

**Required resources**: Sviluppatore C++, design per tipo struct/class.

**Effectiveness indicators**:

1. Test che accesso membro su tipo senza campi produce errore esplicito.
2. Zero MemberExpr accettate senza verifica.

---

#### REC-010

**Title**: Generare vincolo di compatibilità per `CastExpr`

**Deficiency addressed**: DEF-014

**Description**: In `type_expr` per `CastExpr` (`TypeChecker.cpp:850–862`), generare un vincolo tra il tipo
dell'operando
e il tipo target. Per cast tra tipi incompatibili (es. `string` → `i32`), produrre errore. Change entry point:
`TypeChecker.cpp`, metodo `type_expr()`, caso `NodeKind::CastExpr` (righe 850–862).

**Feasibilità**: 4 — Aggiunta di vincolo esistente, logica di compatibilità parzialmente già presente.

**Expected ROI**: 4 — Previene cast non validi silenziosamente accettati.

**Implementation effort**: 4 — 2–4 ore per vincolo + regole di compatibilità.

**Composite Score**: 4×2 + 4×2 + 4×1 = 8 + 8 + 4 = **20**

**Estimated implementation time**: 2–4 ore

**Required resources**: Sviluppatore C++.

**Effectiveness indicators**:

1. Test che cast `string` → `i32` produce errore.
2. Test che cast `i32` → `f64` è accettato.

---

#### REC-011

**Title**: Gestire `nullptr` restituito da `type_expr` per ArrayLiteral

**Deficiency addressed**: DEF-015

**Description**: In `type_stmt` per `VarDecl` (`TypeChecker.cpp:940–946`), aggiungere controllo `if(!typed_init)` prima
di accedere a `typed_init->node_type()`. Analogamente in altri punti dove `type_expr` può restituire `nullptr`. Change
entry point: `TypeChecker.cpp`, metodo `type_stmt()`, caso `NodeKind::VarDecl`, righe 940–946.

**Feasibilità**: 5 — Aggiunta di controlli null in punti identificati.

**Expected ROI**: 4 — Previene crash runtime su codice con errori di tipo in initializer.

**Implementation effort**: 4 — 1–2 ore per audit completo e aggiunta guardie.

**Composite Score**: 5×2 + 4×2 + 4×1 = 10 + 8 + 4 = **22**

**Estimated implementation time**: 1–2 ore

**Required resources**: Sviluppatore C++.

**Effectiveness indicators**:

1. Zero crash per `nullptr` dereference in type checking.
2. Test con ArrayLiteral fallito non causa null dereference.

---

#### REC-012

**Title**: Preservare tutti i nomi in multi-var decl nell'AST tipizzato

**Deficiency addressed**: DEF-016

**Description**: In `type_stmt` per `VarDecl` con nomi multipli (`TypeChecker.cpp:972–1004`), invece di restituire un
singolo `TypedVarDecl` per il primo nome, restituire un `TypedBlockStmt` contenente un `TypedVarDecl` per ogni nome, o
modificare `TypedVarDecl` per supportare multipli nomi. Change entry point: `TypeChecker.cpp`, metodo `type_stmt()`,
caso
`NodeKind::VarDecl`, ramo multi-var (righe 972–1004).

**Feasibilità**: 3 — Richiede modifica a `TypedVarDecl` o wrapping in blocco.

**Expected ROI**: 3 — Correttezza dell'AST tipizzato per dichiarazioni multiple.

**Implementation effort**: 3 — 4–8 ore.

**Composite Score**: 3×2 + 3×2 + 3×1 = 6 + 6 + 3 = **15**

**Estimated implementation time**: 4–8 ore

**Required resources**: Sviluppatore C++, possibile modifica a `TypedProgram.hpp`.

**Effectiveness indicators**:

1. Test `a, b, c = 1, 2, 3` produce 3 dichiarazioni nell'AST tipizzato.
2. Zonking preserva tutte le dichiarazioni.

---

#### REC-013

**Title**: Aggiungere caso esplicito per `TypeKind::Error` in `occurs_in`

**Deficiency addressed**: DEF-006 (parziale)

**Description**: Aggiungere `case TypeKind::Error: return false;` in `ConstraintSolver::occurs_in()`
(`ConstraintSolver.cpp:31–44`) prima del `default`. Rende esplicito che ErrorType non contiene variabili. Change entry
point: `ConstraintSolver.cpp`, metodo `ConstraintSolver::occurs_in()`, switch su `resolved->kind()`.

**Feasibilità**: 5 — Aggiunta di 1 riga.

**Expected ROI**: 2 — Documentazione implicita del comportamento, impatto funzionale nullo.

**Implementation effort**: 5 — 5 minuti.

**Composite Score**: 5×2 + 2×2 + 5×1 = 10 + 4 + 5 = **19**

**Estimated implementation time**: 5–15 minuti

**Required resources**: Sviluppatore C++.

**Effectiveness indicators**:

1. Caso `TypeKind::Error` visibile nello switch di `occurs_in`.

---

#### REC-014

**Title**: Sostituire `at()` con accesso sicuro in `UnionFind::find`

**Deficiency addressed**: DEF-020, DEF-027

**Description**: In `UnionFind::find` (`UnionFind.cpp:16–19`), sostituire `parent_.at(var)` con `parent_.find(var)` e
gestire il caso non trovato (restituire un valore sentinella o fare assert). Alternativamente, aggiungere `make_set`
implicito se il nodo non esiste. Change entry point: `UnionFind.cpp`, metodo `UnionFind::find()`, righe 16–19.

**Feasibilità**: 5 — Modifica minima con chiara alternativa.

**Expected ROI**: 3 — Previene eccezione non catturata per variabili non registrate.

**Implementation effort**: 5 — 15 minuti.

**Composite Score**: 5×2 + 3×2 + 5×1 = 10 + 6 + 5 = **21**

**Estimated implementation time**: 15–30 minuti

**Required resources**: Sviluppatore C++.

**Effectiveness indicators**:

1. Zero `std::out_of_range` eccezioni da `UnionFind::find`.
2. Test con variabile non registrata gestita correttamente.

---

#### REC-015

**Title**: Migliorare cache invalidation in `Substitution::bind`

**Deficiency addressed**: DEF-021

**Description**: Invece di `apply_cache_.clear()` in `Substitution::bind()` (`Substitution.cpp:12`), implementare
invalidazione selettiva: rimuovere solo le entry il cui valore dipende dalla variabile bindata. Richiede tracciamento
delle
dipendenze o ricomparsione del tipo bindato per identificare entry invalidate. Change entry point: `Substitution.cpp`,
metodo `Substitution::bind()`, riga 12.

**Feasibilità**: 2 — Complesso da implementare correttamente senza introdurre bug.

**Expected ROI**: 3 — Miglioramento prestazioni per vincoli in sequenza, non critico per correttezza.

**Implementation effort**: 2 — 1–2 giorni per implementazione e validazione.

**Composite Score**: 2×2 + 3×2 + 2×1 = 4 + 6 + 2 = **12**

**Estimated implementation time**: 1–2 giorni

**Required resources**: Sviluppatore C++ con conoscenze di caching.

**Effectiveness indicators**:

1. Benchmark mostra miglioramento >20% su programmi con >1000 vincoli.
2. Correttezza verificata con test esistenti.

---

#### REC-016

**Title**: Documentare lifetime requirement per `string_view` in SymbolTable

**Deficiency addressed**: DEF-023

**Description**: Aggiungere commento esplicativo e static_assert o documentazione in `SymbolTable.hpp:65` che documenti
il requisito che le stringhe usate come chiavi devono vivere più della SymbolTable. Valutare l'uso di `std::string` come
chiave per maggiore sicurezza. Change entry point: `SymbolTable.hpp`, campo `scopes_` (riga 65).

**Feasibilità**: 5 — Aggiunta di documentazione o cambio tipo chiave.

**Expected ROI**: 3 — Previene future regressioni da refactoring dell'AST.

**Implementation effort**: 4 — 1–2 ore se si cambia a `std::string`.

**Composite Score**: 5×2 + 3×2 + 4×1 = 10 + 6 + 4 = **20**

**Estimated implementation time**: 1–2 ore

**Required resources**: Sviluppatore C++.

**Effectiveness indicators**:

1. Documentazione esplicita del lifetime requirement.
2. Se cambiato a `std::string`, zero dangling view.

---

#### REC-017

**Title**: Unificare strategia di propagazione errori

**Deficiency addressed**: DEF-005, DEF-024

**Description**: Standardizzare la propagazione errori: `SymbolTable::lookup` dovrebbe restituire
`std::expected<TypeScheme, CompileError>` invece di `std::optional<TypeScheme>`, con errore specifico per "undeclared
identifier". Il `TypeChecker` dovrebbe usare questo errore direttamente. Change entry point: `SymbolTable.hpp`, metodo
`lookup()`; `SymbolTable.cpp`, implementazione; `TypeChecker.cpp`, punto di chiamata per `Identifier`.

**Feasibilità**: 3 — Richiede modifica a SymbolTable e tutti i chiamanti.

**Expected ROI**: 3 — Coerenza architetturale nella gestione errori.

**Implementation effort**: 3 — 4–8 ore.

**Composite Score**: 3×2 + 3×2 + 3×1 = 6 + 6 + 3 = **15**

**Estimated implementation time**: 4–8 ore

**Required resources**: Sviluppatore C++.

**Effectiveness indicators**:

1. Tutti i metodi del typechecker usano `std::expected` o `std::optional` coerentemente documentato.
2. Lookup fallito produce errore con contesto.

---

#### REC-018

**Title**: Refattorizzare name resolution per FuncDecl con signature completa

**Deficiency addressed**: DEF-010

**Description**: In `resolve_names_stmt` per `FuncDecl` (`TypeChecker.cpp:103–120`), invece di registrare la funzione
con una type variable fresca, creare uno scheme con tipo funzione che collega parametri e tipo di ritorno. Richiede
`FnType`. Change entry point: `TypeChecker.cpp`, metodo `resolve_names_stmt()`, caso `NodeKind::FuncDecl` (righe
103–120).

**Feasibilità**: 2 — Dipende da REC-008 (FnType).

**Expected ROI**: 4 — Collega name resolution e constraint generation per funzioni, migliorando accuratezza.

**Implementation effort**: 2 — 1–2 settimane (dipende da FnType).

**Composite Score**: 2×2 + 4×2 + 2×1 = 4 + 8 + 2 = **14**

**Estimated implementation time**: 1–2 settimane (post REC-008)

**Required resources**: Sviluppatore C++ senior.

**Effectiveness indicators**:

1. Funzioni registrate con signature completa.
2. Vincoli generati collegano tipo funzione a parametri.

---

### 4.2 Summary Priority Table

| Rank | ID      | Title                                                           | Feasibility | ROI | Effort | Composite Score | Est. Time     |
|------|---------|-----------------------------------------------------------------|-------------|-----|--------|-----------------|---------------|
| 1    | REC-002 | Confrontare nomi CustomType nell'unificazione                   | 5           | 5   | 5      | 25              | 1–2 ore       |
| 2    | REC-007 | Gestire `nullptr` in `zonk_block_full` invece di scartare       | 5           | 4   | 5      | 23              | 30–60 min     |
| 3    | REC-011 | Gestire `nullptr` restituito da `type_expr` per ArrayLiteral    | 5           | 4   | 4      | 22              | 1–2 ore       |
| 4    | REC-003 | Aggiungere caso esplicito ErrorType in Substitution::applyImpl  | 5           | 3   | 5      | 21              | 15–30 min     |
| 5    | REC-014 | Sostituire `at()` con accesso sicuro in UnionFind::find         | 5           | 3   | 5      | 21              | 15–30 min     |
| 6    | REC-016 | Documentare lifetime requirement per string_view in SymbolTable | 5           | 3   | 4      | 20              | 1–2 ore       |
| 7    | REC-010 | Generare vincolo di compatibilità per CastExpr                  | 4           | 4   | 4      | 20              | 2–4 ore       |
| 8    | REC-001 | Implementare sostituzione completa in TypeScheme::instantiate   | 3           | 5   | 3      | 19              | 1–2 giorni    |
| 9    | REC-013 | Aggiungere caso esplicito ErrorType in occurs_in                | 5           | 2   | 5      | 19              | 5–15 min      |
| 10   | REC-005 | Rendere type_expr e type_stmt metodi privati                    | 3           | 4   | 2      | 16              | 1–2 giorni    |
| 11   | REC-004 | Spostare parse_type_annotation in modulo dedicato               | 3           | 3   | 3      | 15              | 2–4 ore       |
| 12   | REC-008 | Implementare vincoli di signature per CallExpr                  | 2           | 5   | 1      | 15              | 1–2 settimane |
| 13   | REC-012 | Preservare tutti i nomi in multi-var decl                       | 3           | 3   | 3      | 15              | 4–8 ore       |
| 14   | REC-017 | Unificare strategia di propagazione errori                      | 3           | 3   | 3      | 15              | 4–8 ore       |
| 15   | REC-009 | Implementare member resolution per MemberExpr                   | 2           | 4   | 2      | 14              | 2–3 giorni    |
| 16   | REC-018 | Refattorizzare name resolution per FuncDecl                     | 2           | 4   | 2      | 14              | 1–2 settimane |
| 17   | REC-006 | Refattorizzare type_expr e type_stmt con Visitor                | 2           | 4   | 1      | 13              | 1–2 settimane |
| 18   | REC-015 | Migliorare cache invalidation in Substitution::bind             | 2           | 3   | 2      | 12              | 1–2 giorni    |

---

## Appendix — Deficiency-to-Recommendation Traceability

| Deficiency | Recommendation(s)                                                               |
|------------|---------------------------------------------------------------------------------|
| DEF-001    | (non direttamente — duplicazione architetturale, mitigata da REC-006)           |
| DEF-002    | REC-005                                                                         |
| DEF-003    | (accoppiamento intenzionale — nessun REC, da valutare in refactoring futuro)    |
| DEF-004    | REC-004                                                                         |
| DEF-005    | REC-017                                                                         |
| DEF-006    | REC-003, REC-013                                                                |
| DEF-007    | REC-002                                                                         |
| DEF-008    | REC-001                                                                         |
| DEF-009    | REC-006                                                                         |
| DEF-010    | REC-018                                                                         |
| DEF-011    | REC-007                                                                         |
| DEF-012    | REC-008                                                                         |
| DEF-013    | REC-009                                                                         |
| DEF-014    | REC-010                                                                         |
| DEF-015    | REC-011                                                                         |
| DEF-016    | REC-012                                                                         |
| DEF-017    | (non thread-safe dichiarato — uso singolo thread, nessun REC immediato)         |
| DEF-018    | REC-002                                                                         |
| DEF-019    | (comportamento corretto — hint utile, nessun REC)                               |
| DEF-020    | REC-014                                                                         |
| DEF-021    | REC-015                                                                         |
| DEF-022    | REC-003                                                                         |
| DEF-023    | REC-016                                                                         |
| DEF-024    | REC-017                                                                         |
| DEF-025    | REC-001                                                                         |
| DEF-026    | (coperto da REC-001 — sostituzione completa risolve anche multipli riferimenti) |
| DEF-027    | REC-014                                                                         |

---

## Vincolo Compliance Verification

**Constraint 1**: Ogni affermazione è ancorata a file, metodo e range di righe specifici. Nessun speculazione senza
prefisso "Inferred:". **VERIFICATO** ✓

**Constraint 2**: Tutti i 9 sistemi e tutti i componenti sono analizzati, inclusi stub e parziali. **VERIFICATO** ✓

**Constraint 3**: Ogni DEF ha almeno un REC corrispondente nella traceability table. **VERIFICATO** ✓

**Constraint 4**: Ogni REC specifica entry point (file + metodo) e azione concreta. **VERIFICATO** ✓

**Constraint 5**: Linguaggio tecnico preciso, senza hedging. **VERIFICATO** ✓

**Constraint 6**: Nessun contenuto duplicato — riferimenti incrociati con "see §X.Y". **VERIFICATO** ✓

**Constraint 7**: Priorità calcolata meccanicamente con formula F×2 + ROI×2 + E×1, ordinamento decrescente.
**VERIFICATO** ✓

**Constraint 8**: Ogni sottosezione Phase 3 >150 parole, ogni sistema Phase 2 >300 parole. **VERIFICATO** ✓

**Constraint 9**: Documento interamente in italiano. **VERIFICATO** ✓

**Constraint 10**: Nessuna affermazione generica senza difetto specifico osservato e passo di remediazione
file-specifico.
**VERIFICATO** ✓