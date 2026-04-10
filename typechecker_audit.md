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

- **TypeChecker** → `ConstraintSet`, `ConstraintSolver`, `SymbolTable`, `Substitution`, `TypeScheme`, `TypeVariable`,
  `ErrorType`, AST (`Program`, `TypedProgram`, `Expressions`, `Statements`), error (`CompileError`).
- **ConstraintSolver** → `UnionFind`, `Substitution`, `Constraint`, `ErrorType`, `CompileError`.
- **Constraint** → `TypePtr` (da `Type.hpp`), `SourceSpan`.
- **Substitution** → `TypePtr`, `TypeVariable`.
- **SymbolTable** → `TypeScheme`.
- **TypeScheme** → `TypePtr`, `TypeVariable`.
- **TypeVariable** → `TypeBase` (da `Type.hpp`).
- **UnionFind** → nessuna dipendenza interna al typechecker (solo STL).
- **ErrorType** → `TypeBase` (da `Type.hpp`).

### 1.3 Architectural Coherence Evaluation

La decomposizione in sistemi segue **principi di separazione delle responsabilità generalmente sani**. La pipeline a 4
fasi (name resolution → constraint generation → constraint solving → zonking) è un'architettura consolidata per i type
checker constraint-based e la sua implementazione rispetta questa struttura.

**Punti di coerenza:**

1. I sistemi di basso livello (`UnionFind`, `TypeVariable`, `ErrorType`, `Substitution`, `Constraint`) sono **autonomi e
   coesi** — ciascuno gestisce esattamente una responsabilità.
2. La gerarchia di dipendenze è **aciclica**: i sistemi foglia (`UnionFind`, `TypeVariable`, `ErrorType`) non dipendono
   da nessun altro sistema del typechecker; i sistemi intermedi (`Substitution`, `TypeScheme`, `SymbolTable`,
   `Constraint`) dipendono solo dai foglia; i sistemi radice (`ConstraintSolver`, `TypeChecker`) orchestrazione e
   consumo.
3. L'uso di `TypePtr = std::shared_ptr<const TypeBase>` come tipo uniforme per tutti i riferimenti a tipi è **coerente**
   attraverso tutti i sistemi.

**Punti di incoerenza:**

1. **DEF-001**: Il sistema `ConstraintSolver` implementa la logica di unificazione strutturale ricorsiva (
   `ConstraintSolver::unify`) che duplica parzialmente la ricorsione già presente in `Substitution::apply`. Sebbene i
   due metodi abbiano scopi diversi (unificazione vs applicazione di sostituzione), la duplicazione della logica di
   dispatch per `TypeKind::Array` e `TypeKind::Vector` (`ConstraintSolver.cpp:109–123` vs `Substitution.cpp:33–49`) è
   una duplicazione di responsabilità che richiede manutenzione sincronizzata.
2. **DEF-002**: Il `TypeChecker` espone `type_expr` e `type_stmt` come metodi pubblici (`TypeChecker.hpp:62–77`)
   esclusivamente per "unit testing". Questo rompe l'incapsulamento della pipeline — un chiamante esterno può invocare
   constraint generation su espressioni isolate bypassando name resolution, producendo risultati semanticamente
   inconsistenti.
3. **DEF-003**: Il sistema `TypeChecker` gestisce sia la name resolution che la constraint generation nella stessa
   classe. Sebbene funzionale, questa accoppiamento significa che la SymbolTable e il ConstraintSet sono campi della
   stessa istanza (`TypeChecker.hpp:84–85`), impedendo il riuso indipendente di ciascun fase.
4. **DEF-004**: La funzione `parse_type_annotation` (`TypeChecker.cpp:19–36`) implementa un parser di annotazioni di
   tipo ad-hoc all'interno del file di implementazione del TypeChecker. Questa funzionalità è logicamente parte del
   sottosistema di rappresentazione dei tipi (`Type.hpp`) ma risiede nel file più grande, creando una responsabilità
   fuori luogo.

### 1.4 Cross-Cutting Concerns Assessment

| Concern                            | TypeChecker                                                                                                         | ConstraintSolver                                                                         | Substitution                  | TypeScheme         | SymbolTable                                              | Uniformità                  |
|------------------------------------|---------------------------------------------------------------------------------------------------------------------|------------------------------------------------------------------------------------------|-------------------------------|--------------------|----------------------------------------------------------|-----------------------------|
| **Propagazione errori**            | Raccolta in `std::vector<CompileError>` (`TypeChecker.hpp:85`); errori accumulati e restituiti in `TypeCheckResult` | Restituisce `std::expected<void, CompileError>` da `unify()` (`ConstraintSolver.hpp:72`) | Nessun errore (mappa totale)  | Nessun errore      | `std::nullopt` per lookup fallito (`SymbolTable.hpp:53`) | **INCOERENTE**              |
| **Risoluzione simboli**            | Delega a `SymbolTable::lookup`                                                                                      | Non applicabile                                                                          | Non applicabile               | Non applicabile    | Ricerca scope-inner → outer (`SymbolTable.cpp:21–27`)    | Uniforme (un solo punto)    |
| **Gestione scope**                 | `push_scope`/`pop_scope` chiamati esplicitamente                                                                    | Non applicabile                                                                          | Non applicabile               | Non applicabile    | Stack di `unordered_map` (`SymbolTable.hpp:65`)          | Uniforme                    |
| **Rappresentazione tipi**          | `TypePtr` ovunque; zonk crea nuove istanze `shared_ptr`                                                             | `TypePtr` per unificazione                                                               | `TypePtr` per bindings        | `TypePtr` per body | `TypeScheme` wrappa `TypePtr`                            | **Uniforme**                |
| **ErrorType (silent unification)** | Inserito per undeclared identifiers (`TypeChecker.cpp:506`)                                                         | Unificazione con ErrorType silenziosamente succeeds (`ConstraintSolver.cpp:48–49`)       | Non menzionato esplicitamente | Non menzionato     | Non applicabile                                          | **PARZIALMENTE INCOERENTE** |

**DEF-005 — Incoerenza nella propagazione degli errori:** Il `TypeChecker` accumula errori in un vettore mutabile (
`errors_`), il `ConstraintSolver` usa `std::expected<T, E>` per segnalazione immediata, e la `SymbolTable` restituisce
`std::nullopt` senza contesto d'errore. Un chiamante della `SymbolTable::lookup` non può distinguere tra "simbolo non
trovato" e "errore interno". La `SymbolTable` dovrebbe restituire `std::expected<TypeScheme, CompileError>` per coerenza
con il resto della pipeline.

**DEF-006 — ErrorType non propagate attraverso Substitution:** Il metodo `Substitution::apply` (
`Substitution.cpp:21–54`) gestisce `TypeVariable`, `ArrayType`, `VectorType`, `CustomType` e `default`, ma non menziona
esplicitamente `TypeKind::Error` nel suo switch. Sebbene il caso `default` restituisca il tipo invariato (corretto per
ErrorType), l'assenza di una gestione esplicita rende il comportamento dipendente dall'ordine del default branch,
vulnerabile a future modifiche.

---

## Phase 2 — Per-System Deep Analysis

### System: TypeChecker (Orchestratore Principale)

#### 2.1 System Overview

Il `TypeChecker` è il sistema centrale dell'intera pipeline di type checking. Implementa un algoritmo constraint-based
Hindley-Milner a 4 fasi: (1) name resolution tramite `SymbolTable`, (2) constraint generation tramite traversata
dell'AST non tipizzato, (3) constraint solving delegato a `ConstraintSolver`, (4) zonking (applicazione della
sostituzione all'AST tipizzato).

#### 2.2 Internal Module Organization

Il sistema è organizzato in un singolo header (`TypeChecker.hpp`) e un singolo file di implementazione (
`TypeChecker.cpp`, 1180 righe). L'header espone la classe `TypeChecker` e la struct `TypeCheckResult`. L'implementazione
contiene:

- Funzioni helper statiche (`parse_type_annotation`, `zonk_type`) nelle righe 19–65
- Implementazione di `check()` (entry point) alle righe 68–89
- Fasi 1–4 separate in blocchi commentati (righe 92–187)
- `type_expr()` per constraint generation delle espressioni (righe 389–872)
- `type_stmt()` per constraint generation delle istruzioni (righe 874–1178)

**DEF-007**: Il file `TypeChecker.cpp` (1180 righe) viola la soglia di complessità cognitiva. Il metodo `type_expr`
supera 480 righe con profondità di annidamento fino a 5 livelli. La soglia raccomandata è ≤100 righe per funzione (
AGENTS.md §7).

#### 2.3 Intra-System Dependency Analysis

All'interno del sistema `TypeChecker`, le dipendenze sono:

- `check()` → `resolve_names()`, `generate_constraints()`, `solve_constraints()`, `zonk()`
- `resolve_names()` → `resolve_names_stmt()`
- `generate_constraints()` → `type_stmt()`
- `type_stmt()` → `type_expr()` (per espressioni contenute nelle istruzioni)
- `zonk()` → `zonk_stmt_full()` → `zonk_expr_full()`, `zonk_block_full()`

Non esistono dipendenze circolari. Il grafo è un DAG con `check()` come radice.

#### 2.4 Logical Flow

1. **Reset**: `check()` azzera `symbols_`, `constraints_`, `errors_`, `message_storage_`, `typed_stmts_` (
   `TypeChecker.cpp:69–74`)
2. **Name Resolution**: `resolve_names(program)` crea lo scope globale, poi per ogni statement chiama
   `resolve_names_stmt()`, che registra funzioni, variabili, e blocchi nella `SymbolTable` con type variable fresche (
   `TypeChecker.cpp:95–147`)
3. **Constraint Generation**: `generate_constraints(program)` itera sugli statement chiamando `type_stmt()`, che per
   ogni nodo AST produce il corrispondente nodo tipizzato e aggiunge vincoli al `ConstraintSet` (
   `TypeChecker.cpp:150–155`)
4. **Constraint Solving**: `solve_constraints()` crea un `ConstraintSolver` e risolve tutti i vincoli, producendo una
   `Substitution` (`TypeChecker.cpp:158–161`)
5. **Zonking**: `zonk(subst)` applica la sostituzione a tutti gli statement tipizzati, producendo il `TypedProgram`
   finale (`TypeChecker.cpp:164–180`)

#### 2.5 Critical Points

**DEF-008 — Name resolution incompleta per FuncDecl**: In `resolve_names_stmt`, i parametri delle funzioni ricevono type
annotation fresche se non annotate (`TypeChecker.cpp:118`), ma il tipo della funzione stessa è una type variable
fresca (`TypeChecker.cpp:108`) che non viene mai raffinata con la signature effettiva. Durante la constraint generation,
il tipo della funzione non viene collegato ai suoi parametri e tipo di ritorno — il vincolo è solo indiretto attraverso
le chiamate.

**DEF-009 — Gestione MainStmt nella name resolution**: Il caso `NodeKind::MainStmt` in `resolve_names_stmt` (
`TypeChecker.cpp:124–127`) registra "main" con tipo `void_()`, ma non risolve i nomi nel body del `MainStmt`. Se il body
contiene dichiarazioni di variabili, queste non vengono registrate nella SymbolTable prima della constraint generation,
producendo falsi positivi "Undeclared identifier" per variabili dichiarate nel body di main.

**DEF-010 — Zonk block silenziosamente scarta statement**: In `zonk_block_full` (`TypeChecker.cpp:370–383`), quando
`zonk_stmt_full` restituisce `nullptr`, il commento dice "Can't move from const — skip (original kept by callee)".
Questo significa che statement non zonkati vengono persi silenziosamente dall'output senza errore. Il blocco risultante
è incompleto.

**DEF-011 — CallExpr non vincola signature**: In `type_expr` per `CallExpr` (`TypeChecker.cpp:689–730`), il callee viene
tipato e gli argomenti vengono tipati, ma **non viene generato alcun vincolo** che colleghi il tipo del callee a una
signature di funzione con i tipi degli argomenti e il tipo di ritorno. Il commento alle righe 716–722 ammette
esplicitamente: "This is a simplification — a real implementation would use function types". Di conseguenza, le chiamate
a funzione non vengono verificate per arity o tipo degli argomenti.

**DEF-012 — MemberExpr totalmente non implementato**: `type_expr` per `MemberExpr` (`TypeChecker.cpp:843–848`) crea una
type variable fresca per il risultato ma non genera alcun vincolo sull'oggetto o sul membro. Non c'è lookup del membro,
né verifica che l'oggetto abbia quel membro.

**DEF-013 — CastExpr non genera vincoli**: In `type_expr` per `CastExpr` (`TypeChecker.cpp:850–862`), il tipo target
viene parsato ma non viene generato alcun vincolo tra il tipo dell'operando e il tipo target. Il cast è quindi una
operazione puramente sintattica senza verifica di compatibilità.

**DEF-014 — ArrayLiteral restituisce nullptr su errore**: Quando il primo elemento di un array literal non può essere
tipato, `type_expr` restituisce `nullptr` (`TypeChecker.cpp:739`). Questo propaga nullptr attraverso `type_stmt` per
`ExprStmt` che crea un placeholder, ma molti altri punti di chiamata non gestiscono nullptr (es.
`typed_init->node_type()` in `VarDecl` alle righe 946).

**DEF-015 — Gestione loop_depth_ non thread-safe**: `loop_depth_` è un campo mutable della classe (`TypeChecker.hpp:89`)
incrementato/decrementato durante la traversata. Se `TypeChecker` venisse usato concorrentemente (non è il caso attuale,
ma l'API non lo proibisce esplicitamente), questo causerebbe data race.

#### 2.6 Partial or Undefined Implementations

- **`TypeChecker::resolve_names`**: Dichiarato e implementato completo.
- **`TypeChecker::resolve_names_stmt`**: Dichiarato e implementato, ma **parziale** — non gestisce `ReturnStmt`,
  `BreakStmt`, `ContinueStmt`, `IfStmt`, `WhileStmt`, `ForStmt` nella name resolution (caso `default` fa `break`
  silenzioso, `TypeChecker.cpp:145`).
- **`TypeChecker::generate_constraints`**: Dichiarato e implementato.
- **`TypeChecker::solve_constraints`**: Dichiarato e implementato (delega pura).
- **`TypeChecker::zonk`**: Dichiarato e implementato.
- **`TypeChecker::zonk_stmt_full`**: Dichiarato e implementato, ma **parziale** — caso `default` restituisce `nullptr`.
- **`TypeChecker::zonk_expr_full`**: Dichiarato e implementato, ma **parziale** — caso `default` restituisce `nullptr`.
- **`TypeChecker::zonk_block_full`**: Dichiarato e implementato, ma **parziale** — scarta statement che zonk_stmt_full
  restituisce nullptr.
- **`TypeChecker::type_expr`**: Dichiarato e implementato, ma **parziale** — `MemberExpr` non genera vincoli, `CallExpr`
  non vincola signature, `CastExpr` non genera vincoli di compatibilità.
- **`TypeChecker::type_stmt`**: Dichiarato e implementato, ma **parziale** — multi-var decl semplificato a single-var.

---

### System: ConstraintSolver (Motore di Unificazione)

#### 2.1 System Overview

Il `ConstraintSolver` implementa l'algoritmo di unificazione basato su union-find. Riceve un `ConstraintSet` e produce
una `Substitution` che mappa type variable a tipi concreti, o errori di unificazione.

#### 2.2 Internal Module Organization

Header (`ConstraintSolver.hpp`) e implementazione (`ConstraintSolver.cpp`) compatti. L'header definisce `SolverResult` e
la classe `ConstraintSolver`. L'implementazione contiene `solve()`, `occurs_in()`, e `unify()`.

#### 2.3 Intra-System Dependency Analysis

Dipendenze interne: `solve()` → `unify()` per ogni constraint. `unify()` → `occurs_in()` per occurs check. `unify()` →
`UnionFind::make_set`, `UnionFind::unite`. `unify()` → `Substitution::bind`. Nessuna dipendenza circolare.

#### 2.4 Logical Flow

1. `solve()` inizializza `union_find_` e `substitution_` vuoti (`ConstraintSolver.cpp:12–14`)
2. Per ogni constraint nel set, chiama `unify(lhs, rhs, constraint)` (`ConstraintSolver.cpp:16–18`)
3. `unify()` gestisce: ErrorType (successo silenzioso), null type (errore), type variable unification (con occurs
   check), concrete type equality (ricorsivo per Array/Vector) (`ConstraintSolver.cpp:47–127`)
4. Gli errori sono raccolti in `SolverResult::errors` e restituiti al chiamante

#### 2.5 Critical Points

**DEF-016 — Unificazione incompleta per CustomType**: Il caso `default` dello switch in `unify()` (
`ConstraintSolver.cpp:119–122`) assume che due `CustomType` con lo stesso `kind()` siano uguali. Ma `CustomType` ha un
campo `name_` — due CustomType diversi (es. `Foo` e `Bar`) hanno entrambi `kind() == TypeKind::Custom` ma nomi diversi.
L'unificazione dovrebbe confrontare i nomi.

**DEF-017 — Unificazione Array ignora size**: Quando unifica due `ArrayType` (`ConstraintSolver.cpp:109–115`), il solver
unifica solo i tipi elemento, ignorando completamente le espressioni di dimensione. Questo significa che `[i32; 3]` si
unifica con `[i32; 5]` senza errore.

**DEF-018 — occurs_in non gestisce CustomType**: Il metodo `occurs_in` (`ConstraintSolver.cpp:24–44`) ricorre in `Array`
e `Vector` ma non in `CustomType`. Se un CustomType contenesse type variable come parametri generici (estensione
futura), l'occurs check fallirebbe silenziosamente.

**DEF-019 — UnionFind duplica lavoro di Substitution**: Il solver mantiene sia `union_find_` che `substitution_` come
stati paralleli (`ConstraintSolver.hpp:75–76`). Dopo l'unificazione di due type variable, sia `union_find_.unite()` che
`substitution_.bind()` vengono chiamati. Questo è ridondante perché la substitution da sola sarebbe sufficiente per la
fase di zonk.

#### 2.6 Partial or Undefined Implementations

- **`ConstraintSolver::solve`**: Completo.
- **`ConstraintSolver::occurs_in`**: Completo per i tipi attualmente supportati, ma **parziale** per estensioni future (
  CustomType parametrici).
- **`ConstraintSolver::unify`**: Dichiarato e implementato, ma **parziale** — CustomType non confronta nomi, Array
  ignora size.

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

**DEF-020 — Lookup lineare O(n) per ID**: `ConstraintSet::get()` usa `std::ranges::find` con complessità O(n) (
`Constraint.cpp:19`). Se il numero di vincoli cresce (centinaia/migliaia per programmi grandi), questo diventa un collo
di bottiglia. Un `std::unordered_map<ConstraintId, Constraint>` o un accesso diretto per indice sarebbe più efficiente.

**DEF-021 — Nessun metodo di rimozione**: `ConstraintSet` non supporta la rimozione di vincoli risolti. Se un sistema
futuro volesse implementare constraint solving incrementale o backtracking, l'API sarebbe insufficiente.

#### 2.6 Partial or Undefined Implementations

Tutti i metodi dichiarati sono implementati. Nessuna funzione stub.

---

### System: Substitution (Mappatura Variabili→Tipi)

#### 2.1 System Overview

Il sistema `Substitution` gestisce una mappa da `TypeVarId` a `TypePtr`, prodotta dal constraint solver e applicata
durante lo zonk per risolvere le type variable nell'AST tipizzato.

#### 2.2 Internal Module Organization

Header (`Substitution.hpp`) e implementazione (`Substitution.cpp`). Design coerente con interfaccia minimale: `bind`,
`lookup`, `apply`, `contains`, `size`.

#### 2.3 Intra-System Dependency Analysis

Dipende da `TypeVariable` per il dispatching nel metodo `apply()`. Nessuna altra dipendenza interna.

#### 2.4 Logical Flow

1. `bind()` inserisce/sovrascrive nel `unordered_map` (`Substitution.cpp:10`)
2. `lookup()` cerca nel map e restituisce `optional` (`Substitution.cpp:12–16`)
3. `apply()` ricorsivamente sostituisce type variable nel tipo (`Substitution.cpp:21–54`): per TypeVariable cerca nel
   map, per Array/Vector ricorre, per altri tipi restituisce invariato
4. `contains()` e `size()` sono wrapper sul map sottostante

#### 2.5 Critical Points

**DEF-022 — apply non transitive per binding indiretti**: Se A→B e B→C sono binding nella substitution, `apply(A)`
restituisce B (primo lookup) e poi ricorsivamente C. Questo è corretto solo se B è esso stesso un TypeVariable. Se B è
un tipo composto contenente A, si crea ricorsione infinita. La protezione è che il solver non dovrebbe mai creare
cicli (garantita dall'occurs check), ma non c'è assertion difensiva.

**DEF-023 — allocazioni multiple in apply per tipi composti**: Per ogni ArrayType/VectorType che contiene type variable,
`apply()` alloca un nuovo `shared_ptr` (`Substitution.cpp:38–39`, `44–45`). Per AST grandi con molti tipi composti,
questo genera pressione sul garbage collector.

#### 2.6 Partial or Undefined Implementations

Tutti i metodi dichiarati sono implementati.

---

### System: SymbolTable (Gestione Scope e Simboli)

#### 2.1 System Overview

Il sistema `SymbolTable` gestisce l'associazione tra identificatori (nomi di variabili, funzioni) e i loro `TypeScheme`,
con supporto per scope lessicali annidati e shadowing.

#### 2.2 Internal Module Organization

Header (`SymbolTable.hpp`) e implementazione (`SymbolTable.cpp`). Implementazione minimale (~30 righe). Rappresentazione
interna: `std::vector<std::unordered_map<std::string, TypeScheme>>`.

#### 2.3 Intra-System Dependency Analysis

Dipende da `TypeScheme`. Nessuna altra dipendenza interna.

#### 2.4 Logical Flow

1. `push_scope()` aggiunge una nuova mappa vuota (`SymbolTable.cpp:10`)
2. `pop_scope()` rimuove la mappa più interna con guardia `empty()` (`SymbolTable.cpp:12–14`)
3. `define()` inserisce nella mappa più interna, con auto-creazione se vuota (`SymbolTable.cpp:16–19`)
4. `lookup()` cerca dalla mappa più interna alla più esterna (`SymbolTable.cpp:21–27`)
5. `defined_in_current_scope()` controlla solo la mappa più interna (`SymbolTable.cpp:29–32`)

#### 2.5 Critical Points

**DEF-024 — define auto-crea scope se vuoto**: `SymbolTable::define` (`SymbolTable.cpp:17`) chiama
`scopes_.emplace_back()` se `scopes_` è vuoto. Questo maschera errori di chiamata a `define` senza un precedente
`push_scope`, rendendo il bug difficile da diagnosticare.

**DEF-025 — lookup alloca std::string ad ogni ricerca**: `lookup()` costruisce `std::string{name}` ad ogni iterazione
del loop (`SymbolTable.cpp:23`). Per ricerche frequenti (ogni identificatore nell'AST), questo genera allocazioni non
necessarie. Usare `std::string_view` come chiave dell'`unordered_map` eliminerebbe le allocazioni.

**DEF-026 — Nessun rilevamento di ridefinizione**: `define()` usa `insert_or_assign` (`SymbolTable.cpp:18`) che
sovrascrive silenziosamente binding esistenti nello stesso scope. Non c'è warning o errore per ridefinizioni nello
stesso scope, che nella maggior parte dei linguaggi è un errore semantico.

#### 2.6 Partial or Undefined Implementations

Tutti i metodi dichiarati sono implementati.

---

### System: TypeScheme (Tipi Polimorfici)

#### 2.1 System Overview

Il sistema `TypeScheme` rappresenta tipi polimorfici con quantificazione universale (∀T. body). Fornisce il metodo
`instantiate()` per sostituire le variabili quantificate con fresh type variable al momento dell'uso.

#### 2.2 Internal Module Organization

Header (`TypeScheme.hpp`) con struct `TypeScheme` e metodo `mono()`. Implementazione (`TypeScheme.cpp`) con
`instantiate()`. Compatto.

#### 2.3 Intra-System Dependency Analysis

Dipende da `TypeVariable` per la generazione di fresh variable in `instantiate()`.

#### 2.4 Logical Flow

1. `mono()` crea uno scheme senza variabili quantificate (`TypeScheme.cpp:10`)
2. `instantiate()` genera fresh type variable per ogni variabile quantificata e le sostituisce nel body (
   `TypeScheme.cpp:12–34`)

#### 2.5 Critical Points

**DEF-027 — instantiate() non sostituisce in tipi composti**: Il commento alle righe 24–28 di `TypeScheme.cpp` ammette
esplicitamente: "For compound types, we'd need to traverse and replace. This is a simplified implementation." Se il body
di uno scheme polimorfico è un `ArrayType<?T1>` o un tipo funzione, le variabili quantificate all'interno del tipo
composto non vengono sostituite. Solo il caso in cui il body è direttamente un `TypeVariable` è gestito (righe 29–32).

**DEF-028 — quantified_vars non validati**: Non c'è verifica che gli ID in `quantified_vars` corrispondano
effettivamente a type variable presenti nel `body`. Uno scheme potrebbe dichiarare `∀T1. int` dove T1 non appare nel
body — tecnicamente corretto ma potenzialmente indicativo di bug.

#### 2.6 Partial or Undefined Implementations

- **`TypeScheme::instantiate`**: Dichiarato e implementato, ma **parziale** — non sostituisce variabili quantificate in
  tipi composti (ArrayType, VectorType, CustomType parametrici).

---

### System: TypeVariable (Variabili di Tipo)

#### 2.1 System Overview

Il sistema `TypeVariable` fornisce la rappresentazione concreta delle type variable (?T1, ?T2, ...) usate
nell'inferenza, con un contatore thread-local per ID univoci.

#### 2.2 Internal Module Organization

Header (`TypeVariable.hpp`) con classe `TypeVariable` e funzione `fresh_type_variable()`. Implementazione (
`TypeVariable.cpp`) con `to_string()` e il contatore.

#### 2.3 Intra-System Dependency Analysis

Nessuna dipendenza interna. `TypeVariable` estende `TypeBase` (esterno al sistema typechecker).

#### 2.4 Logical Flow

1. `fresh_type_variable()` incrementa un contatore thread-local e crea un nuovo `TypeVariable` con l'ID (
   `TypeVariable.cpp:13–15`)
2. `to_string()` formatta come "?T{id}" (`TypeVariable.cpp:10`)
3. `classof()` e `operator==` forniscono RTTI e uguaglianza

#### 2.5 Critical Points

Nessuna criticità significativa. Il design è corretto e minimale.

**Nota**: Il contatore parte da 0 e viene pre-incrementato (`++counter`), quindi il primo ID è 1. L'invariante
`id_ > 0` (`TypeVariable.hpp:22`) è rispettato.

#### 2.6 Partial or Undefined Implementations

Tutti i metodi dichiarati sono implementati.

---

### System: UnionFind (Disjoint-Set)

#### 2.1 System Overview

Il sistema `UnionFind` implementa la struttura dati disjoint-set con path compression e union by rank per operazioni
quasi-costanti O(α(n)).

#### 2.2 Internal Module Organization

Header (`UnionFind.hpp`) e implementazione (`UnionFind.cpp`). Due `unordered_map` interni: `parent_` e `rank_`.

#### 2.3 Intra-System Dependency Analysis

Nessuna dipendenza da altri sistemi del typechecker. Solo STL.

#### 2.4 Logical Flow

1. `make_set()` crea un singleton (`UnionFind.cpp:10–13`)
2. `find()` applica path compression ricorsivo (`UnionFind.cpp:15–18`)
3. `unite()` unisce per rank (`UnionFind.cpp:20–33`)
4. `same_set()` confronta i rappresentanti (`UnionFind.cpp:35–38`)

#### 2.5 Critical Points

**DEF-029 — find() usa `at()` con eccezione potenziali**: `UnionFind::find` usa `parent_.at(var)` (`UnionFind.cpp:16`),
che lancia `std::out_of_range` se `var` non è stato registrato con `make_set()`. Questo è un failure mode non
documentato nell'interfaccia (`ConstraintSolver.hpp:62` dice solo "Must have been previously registered via make_set()"
ma non specifica il comportamento in caso di violazione).

**DEF-030 — unite() usa `at()` per rank**: Analogamente, `unite()` usa `rank_.at(root_x)` e `rank_.at(root_y)` (
`UnionFind.cpp:24–25`). Se un caller chiama `unite(x, y)` senza `make_set(x)` o `make_set(y)`, l'eccezione è non
gestita.

#### 2.6 Partial or Undefined Implementations

Tutti i metodi dichiarati sono implementati.

---

### System: ErrorType (Tipo Errore per Error Recovery)

#### 2.1 System Overview

Il sistema `ErrorType` fornisce un tipo singleton che si unifica con qualsiasi tipo, implementando l'error recovery
silenzioso per prevenire errori a cascata.

#### 2.2 Internal Module Organization

Header (`ErrorType.hpp`) con classe `ErrorType` e funzione `error_type()`. Implementazione (`ErrorType.cpp`) con
singleton statico.

#### 2.3 Intra-System Dependency Analysis

Nessuna dipendenza interna. Estende `TypeBase`.

#### 2.4 Logical Flow

1. `error_type()` restituisce un'istanza singleton `const` (`ErrorType.cpp:10–13`)
2. `operator==` confronta qualsiasi `ErrorType` con un altro `ErrorType` come uguale
3. `classof()` identifica ErrorType per downcasting

#### 2.5 Critical Points

Nessuna criticità. Il design è corretto. L'uso di un singleton `const` è appropriato dato che ErrorType non ha stato.

#### 2.6 Partial or Undefined Implementations

Tutti i metodi dichiarati sono implementati.

---

## Phase 3 — Per-Component Exhaustive Analysis

### System: TypeChecker › Component: TypeChecker (classe principale)

#### 3.1 Responsibility Statement

Il componente `TypeChecker` è esclusivamente responsabile per l'orchestrazione della pipeline di type checking a 4 fasi,
trasformando un AST non tipizzato (`Program`) in un AST completamente tipizzato (`TypedProgram`) con raccolta degli
errori.

#### 3.2 Class Structure

| Membro                          | Tipo                                                  | Visibilità | Semantica                                                                   |
|---------------------------------|-------------------------------------------------------|------------|-----------------------------------------------------------------------------|
| `symbols_`                      | `SymbolTable`                                         | private    | Gestisce lo scope e le associazioni nome→TypeScheme                         |
| `constraints_`                  | `ConstraintSet`                                       | private    | Accumula i vincoli di uguaglianza tra tipi                                  |
| `errors_`                       | `std::vector<CompileError>`                           | private    | Raccolta degli errori di type checking                                      |
| `message_storage_`              | `std::deque<std::string>`                             | private    | Proprietario delle stringhe per `CompileError::message_` (std::string_view) |
| `typed_stmts_`                  | `std::vector<TypedStmtPtr>`                           | private    | Statement tipizzati prodotti durante la constraint generation               |
| `current_function_return_type_` | `std::optional<TypePtr>`                              | private    | Tipo di ritorno della funzione corrente (per validazione dei return)        |
| `current_function_name_`        | `std::optional<std::string>`                          | private    | Nome della funzione corrente (per messaggi d'errore)                        |
| `loop_depth_`                   | `std::size_t`                                         | private    | Profondità di annidamento nei loop (per validazione break/continue)         |
| `check()`                       | `TypeCheckResult(const Program&)`                     | public     | Entry point della pipeline                                                  |
| `type_expr()`                   | `TypedExprPtr(const Expr&)`                           | public     | Constraint generation per espressioni                                       |
| `type_stmt()`                   | `TypedStmtPtr(const Stmt&)`                           | public     | Constraint generation per statement                                         |
| `resolve_names()`               | `void(const Program&)`                                | private    | Phase 1: name resolution                                                    |
| `resolve_names_stmt()`          | `void(const Stmt&)`                                   | private    | Name resolution ricorsiva per statement                                     |
| `generate_constraints()`        | `void(const Program&)`                                | private    | Phase 2: orchestratore constraint generation                                |
| `solve_constraints()`           | `SolverResult() const`                                | private    | Phase 3: delega a ConstraintSolver                                          |
| `zonk()`                        | `TypedProgram(const Substitution&)`                   | private    | Phase 4: applica sostituzione                                               |
| `zonk_stmt_full()`              | `TypedStmtPtr(const Substitution&, const TypedStmt&)` | private    | Zonk ricorsivo per statement                                                |
| `zonk_expr_full()`              | `TypedExprPtr(const Substitution&, const TypedExpr&)` | private    | Zonk ricorsivo per espressioni                                              |
| `zonk_block_full()`             | `unique_ptr<TypedBlockStmt>(...)`                     | private    | Zonk ricorsivo per blocchi                                                  |

#### 3.3 Interface Analysis

| Metodo        | Signature                               | Precondizioni                        | Postcondizioni                                                | Contratto                                                      |
|---------------|-----------------------------------------|--------------------------------------|---------------------------------------------------------------|----------------------------------------------------------------|
| `check()`     | `TypeCheckResult check(const Program&)` | Nessuna                              | Restituisce `TypeCheckResult` con program tipizzato ed errori | Se `errors` è vuoto, `program` è completamente tipizzato       |
| `type_expr()` | `TypedExprPtr type_expr(const Expr&)`   | `symbols_` popolato con nomi risolti | Restituisce espressione tipizzata o nullptr                   | Aggiungi vincoli a `constraints_` per il tipo dell'espressione |
| `type_stmt()` | `TypedStmtPtr type_stmt(const Stmt&)`   | `symbols_` popolato con nomi risolti | Restituisce statement tipizzato o nullptr                     | Aggiungi vincoli; aggiorna `typed_stmts_` indirettamente       |

**Discrepanza**: `type_expr()` e `type_stmt()` sono dichiarati `public` nell'header (`TypeChecker.hpp:62–77`) ma
logicamente sono dettagli implementativi della constraint generation. Il loro contratto dipende dallo stato interno (
`symbols_`, `constraints_`) che non è parte dell'interfaccia pubblica.

#### 3.4 Implementation Logic

Il metodo `type_expr` implementa constraint generation per ~18 varianti di espressioni attraverso uno switch su
`NodeKind`. Ogni caso:

1. Casta il nodo AST al tipo concreto (static_cast dopo kind check)
2. Ricorsivamente tipa i sotto-nodi
3. Genera vincoli di uguaglianza tra i tipi
4. Costruisce il nodo Typed corrispondente
5. Esegue validazioni anticipate (early check) per errori comuni

Il metodo `type_stmt` segue lo stesso pattern per ~12 varianti di statement, con gestione specifica per:

- `FuncDecl`: push/pop scope, typing parametri e body
- `VarDecl`: gestione multi-var con semplificazione a single-var
- `WhileStmt`/`ForStmt`: incremento/decremento di `loop_depth_`
- `ReturnStmt`: validazione contro `current_function_return_type_`

La funzione helper `parse_type_annotation` (`TypeChecker.cpp:19–36`) mappa stringhe ("i32", "f64", "bool", ...) a
`PrimitiveType` singleton tramite confronti stringa sequenziali.

La funzione `zonk_type` (`TypeChecker.cpp:40–64`) applica ricorsivamente una substitution a un TypePtr, gestendo
TypeVariable (lookup ricorsivo), ArrayType (ricorsione sull'elemento), VectorType (ricorsione sull'elemento).

#### 3.5 Error Handling Evaluation

La gestione errori nel TypeChecker segue un **pattern ibrido**:

1. **Errori rilevati durante type_expr/type_stmt**: Gli errori vengono aggiunti a `errors_` via
   `errors_.push_back(CompileError::TypeError(...))` e il metodo restituisce `nullptr` o un nodo con `error_type()`.
2. **Undeclared identifiers**: Rilevati in `type_expr/Identifier` (`TypeChecker.cpp:500–509`), restituiscono
   `TypedIdentifier` con `error_type()`.
3. **Type mismatch**: Rilevati sia come early check (quando entrambi i tipi sono concreti) sia come vincoli (quando sono
   type variable). Gli early check generano errori immediati con hint contestuali.
4. **Errori dal ConstraintSolver**: Fusi in `errors_` dopo il solving (`TypeChecker.cpp:82–83`).

**DEF-031 — Errori silenziosi in zonk_block_full**: Come notato in §2.5 (DEF-010), quando `zonk_stmt_full` restituisce
`nullptr` in `zonk_block_full`, lo statement viene silenziosamente rimosso dal blocco senza registrare errore.

**DEF-032 — Return di nullptr non gestito dai chiamanti**: Molti punti in `type_stmt` chiamano `type_expr` senza
verificare nullptr prima di accedere a `->node_type()`. Ad esempio, in `VarDecl` (`TypeChecker.cpp:946`),
`typed_init->node_type()` causa undefined behavior se `typed_init` è nullptr.

#### 3.6 Type Consistency Audit

- `TypePtr` è usato coerentemente come `std::shared_ptr<const TypeBase>` in tutta la classe.
- I cast usano `static_cast<const Tipo*>(&stmt)` dopo verifica di `kind()`, evitando RTTI overhead.
- **DEF-033**: `zonk_type` usa `dynamic_cast<const TypeVariable*>` (`TypeChecker.cpp:43`) mentre il resto della codebase
  usa `static_cast` dopo `kind()` check. Questo è inconsistente con il pattern stabilito in `TypeChecker::type_expr` e
  `TypeChecker::zonk_expr_full`.
- **DEF-034**: `current_function_return_type_` è `std::optional<TypePtr>` ma viene dereferenziato con
  `*current_function_return_type_` in `type_stmt/ReturnStmt` (`TypeChecker.cpp:1024`) senza verifica in tutti i branch.
  La verifica esiste (`if(!current_function_return_type_)`) ma il successivo `else if(current_function_return_type_)` è
  ridondante e confusionario.

#### 3.7 Inter-Component Interaction

Il `TypeChecker` interagisce con:

- **SymbolTable**: Chiamate a `push_scope`, `pop_scope`, `define`, `lookup`. L'interazione è pulita — la SymbolTable è
  un'astrazione ben definita.
- **ConstraintSet**: Chiamate a `add` per generare vincoli. L'interazione è diretta.
- **ConstraintSolver**: Creato e usato in `solve_constraints()`. Accoppiamento temporaneo (vive solo per la durata del
  solving).
- **Substitution**: Ricevuta da `SolverResult` e passata a `zonk`, `zonk_type`, `zonk_stmt_full`, `zonk_expr_full`.
- **TypeVariable**: Creata tramite `fresh_type_variable()` per placeholder non annotati.

**DEF-035 — Accoppiamento con AST non tipizzato**: Il `TypeChecker` include direttamente `Expressions.hpp`,
`Statements.hpp`, `NodeKind.hpp` (`TypeChecker.cpp:6–8`). Questo accoppiamento è inevitabile ma significa che ogni
modifica all'AST non tipizzato richiede una ricompilazione del TypeChecker.

#### 3.8 Optimization Opportunities

1. **DEF-036 — `parse_type_annotation` O(n) sequenziale**: La funzione esegue 14 confronti stringa sequenziali. Un
   `std::unordered_map<std::string_view, TypePtr>` o un perfetto hash statico ridurrebbe a O(1).
2. **DEF-037 — `zonk_type` usa dynamic_cast**: Sostituire con switch su `kind()` + `static_cast` come nel resto del
   codicebase.
3. **DEF-038 — `message_storage_` come deque**: L'uso di `std::deque` è motivato dalla necessità di prevenire
   invalidazione di `string_view`, ma un `std::vector<std::string>` con `reserve()` avrebbe migliore località di cache.
   La crescita del deque è frammentata.
4. **DEF-039 — Allocazioni multiple in zonk**: Ogni chiamata a `zonk_expr_full` e `zonk_stmt_full` alloca un nuovo
   `unique_ptr` per il nodo tipizzato, anche quando la substitution non ha cambiato nulla. Un confronto preventivo tra
   tipo prima/dopo zonk eviterebbe allocazioni inutili.

---

### System: TypeChecker › Component: parse_type_annotation (funzione helper)

#### 3.1 Responsibility Statement

La funzione `parse_type_annotation` è esclusivamente responsabile per la mappatura di stringhe di annotazione di tipo (
es. "i32", "f64") alle corrispondenti istanze singleton di `PrimitiveType`.

#### 3.2 Class Structure

Funzione libera statica, nessuna classe associata.

| Parametro | Tipo               | Direzione                       |
|-----------|--------------------|---------------------------------|
| `annot`   | `std::string_view` | input                           |
| Return    | `TypePtr`          | output (nullptr se sconosciuto) |

#### 3.3 Interface Analysis

| Aspetto        | Dettaglio                                                                            |
|----------------|--------------------------------------------------------------------------------------|
| Signature      | `static TypePtr parse_type_annotation(std::string_view annot)`                       |
| Precondizioni  | Nessuna                                                                              |
| Postcondizioni | Restituisce PrimitiveType singleton o nullptr                                        |
| Contratto      | Input deve essere una delle stringhe note; output è il tipo corrispondente o nullptr |

#### 3.4 Implementation Logic

14 confronti `if(annot == "...")` sequenziali, ciascuno che restituisce un singleton PrimitiveType. Se nessun match,
restituisce nullptr.

#### 3.5 Error Handling Evaluation

Nessun errore generato — restituisce nullptr per annotazioni sconosciute, delegando la gestione al chiamante.

#### 3.6 Type Consistency Audit

Tipi restituiti coerentemente `TypePtr` (alias di `shared_ptr<const TypeBase>`). I singleton restituiti sono
`shared_ptr<const PrimitiveType>`, implicitamente convertibili a `TypePtr`.

#### 3.7 Inter-Component Interaction

Usata esclusivamente in `TypeChecker::type_expr` (per IntegerLiteral con suffix e per CastExpr) e
`TypeChecker::type_stmt` (per VarDecl con type annotation). Accoppiamento minimo.

#### 3.8 Optimization Opportunities

Come DEF-036: 14 confronti stringa sequenziali possono essere ottimizzati con lookup table statica o binary search su
array ordinato. Per 14 elementi, un `std::array` ordinato con `std::ranges::lower_bound` sarebbe significativamente più
veloce.

---

### System: TypeChecker › Component: zonk_type (funzione helper)

#### 3.1 Responsibility Statement

La funzione `zonk_type` è esclusivamente responsabile per l'applicazione ricorsiva di una substitution a un TypePtr,
risolvendo tutte le type variable nidificate.

#### 3.2 Class Structure

Funzione libera statica.

| Parametro | Tipo                  | Direzione |
|-----------|-----------------------|-----------|
| `subst`   | `const Substitution&` | input     |
| `type`    | `const TypePtr&`      | input     |
| Return    | `TypePtr`             | output    |

#### 3.3 Interface Analysis

Precondizioni: Nessuna. Postcondizioni: Restituisce il tipo con tutte le type variable risolte, o il tipo originale se
nessuna variabile è bound.

#### 3.4 Implementation Logic

Ricorsione strutturale:

1. Se il tipo è nullptr, restituisce nullptr
2. Se è TypeVariable, cerca nel substitution; se bound, ricorre sul tipo risolto
3. Se è ArrayType, ricorre sull'element_type e ricostruisce se cambiato
4. Se è VectorType, analogamente
5. Default: restituisce il tipo invariato

#### 3.5 Error Handling Evaluation

Nessuna gestione errori — la funzione è totale e non può fallire.

#### 3.6 Type Consistency Audit

**DEF-033** (già citato): Usa `dynamic_cast` invece del pattern `kind()` + `static_cast` usato ovunque.

#### 3.7 Inter-Component Interaction

Chiama `Substitution::lookup`. Usata da `zonk_stmt_full`, `zonk_expr_full`, `zonk_block_full`.

#### 3.8 Optimization Opportunities

Il dispatch basato su `dynamic_cast` è più lento del pattern `kind()` + `static_cast`. Inoltre, la ricorsione potrebbe
essere trasformata in iterazione con stack esplicito per evitare stack overflow su tipi profondamente annidati.

---

### System: ConstraintSolver › Component: ConstraintSolver (classe)

#### 3.1 Responsibility Statement

Il componente `ConstraintSolver` è esclusivamente responsabile per la risoluzione di un insieme di vincoli di
uguaglianza tra tipi, producendo una substitution unificante o errori di unificazione.

#### 3.2 Class Structure

| Membro          | Tipo                                                          | Visibilità | Semantica                                               |
|-----------------|---------------------------------------------------------------|------------|---------------------------------------------------------|
| `union_find_`   | `UnionFind`                                                   | private    | Traccia le equivalence class di type variable unificate |
| `substitution_` | `Substitution`                                                | private    | Mappa type variable ai loro tipi risolti                |
| `solve()`       | `SolverResult(const ConstraintSet&)`                          | public     | Entry point del solving                                 |
| `occurs_in()`   | `static bool(TypeVarId, const TypePtr&, const Substitution&)` | public     | Occurs check per prevenire tipi infiniti                |
| `unify()`       | `expected<void, CompileError>(...)`                           | private    | Unificazione di due tipi                                |

#### 3.3 Interface Analysis

| Metodo        | Signature                                                               | Precondizioni    | Postcondizioni                      |
|---------------|-------------------------------------------------------------------------|------------------|-------------------------------------|
| `solve()`     | `SolverResult solve(const ConstraintSet&)`                              | Nessuna          | Restituisce substitution e/o errori |
| `occurs_in()` | `static bool occurs_in(TypeVarId, const TypePtr&, const Substitution&)` | `type` non nullo | true se `var` appare in `type`      |

#### 3.4 Implementation Logic

`solve()` itera sui constraint chiamando `unify()`. `unify()` implementa l'algoritmo di unificazione standard con
gestione speciale per:

- ErrorType: successo silenzioso
- TypeVariable: binding con occurs check
- Concrete types: confronto strutturale ricorsivo

#### 3.5 Error Handling Evaluation

Usa `std::expected<void, CompileError>` per propagazione errori. Gli errori di tipo `E2034` (type mismatch) e `E2035` (
occurs check) sono generati con hint contestuali. **Nessun errore non catturato** — ogni path di `unify` restituisce
`expected`.

#### 3.6 Type Consistency Audit

Coerente nell'uso di `TypePtr`. I dynamic_cast per TypeVariable sono giustificati dalla necessità di distinguere
TypeVariable da altri tipi nel primo branch dell'unificazione.

#### 3.7 Inter-Component Interaction

Usa `UnionFind` per equivalence tracking e `Substitution` per binding. Dipende da `ErrorType` per error recovery
silenziosa.

#### 3.8 Optimization Opportunities

**DEF-019**: Mantenere sia UnionFind che Substitution è ridondante. La Substitution da sola sarebbe sufficiente.
Rimuovere UnionFind ridurrebbe memoria e tempo di O(α(n)) per operazione a O(1) per lookup diretta.

---

### System: Constraint › Component: Constraint (struct) e ConstraintSet (classe)

#### 3.1 Responsibility Statement

Il componente `Constraint` è esclusivamente responsabile per rappresentare un singolo vincolo di uguaglianza `lhs = rhs`
con metadati (ID, origine, motivazione); `ConstraintSet` accumula e fornisce accesso a un insieme ordinato di vincoli.

#### 3.2 Class Structure

| Membro                         | Tipo                          | Visibilità | Semantica                  |
|--------------------------------|-------------------------------|------------|----------------------------|
| `Constraint::id`               | `ConstraintId`                | public     | ID univoco 1-based         |
| `Constraint::lhs`              | `TypePtr`                     | public     | Tipo left-hand side        |
| `Constraint::rhs`              | `TypePtr`                     | public     | Tipo right-hand side       |
| `Constraint::origin`           | `SourceSpan`                  | public     | Posizione sorgente         |
| `Constraint::reason`           | `std::string`                 | public     | Contesto generazione       |
| `ConstraintSet::constraints_`  | `std::vector<Constraint>`     | private    | Contenitore interno        |
| `ConstraintSet::next_id_`      | `ConstraintId`                | private    | Contatore ID (inizia da 1) |
| `ConstraintSet::add()`         | `ConstraintId(...)`           | public     | Aggiunge vincolo           |
| `ConstraintSet::constraints()` | `const vector&() const`       | public     | Accesso read-only          |
| `ConstraintSet::get()`         | `const Constraint*(id) const` | public     | Lookup per ID              |
| `ConstraintSet::size()`        | `size_t() const`              | public     | Cardinalità                |

#### 3.3 Interface Analysis

Tutte le interfacce sono ben documentate con Doxygen, pre/post condizioni chiare, e esempi d'uso nei commenti.

#### 3.4 Implementation Logic

`add()` incrementa `next_id_`, costruisce `Constraint`, e lo push_back nel vector. `get()` usa `std::ranges::find` con
proiezione sul campo `id`. `size()` delega a `vector::size()`.

#### 3.5 Error Handling Evaluation

Nessun errore possibile — `add()` può lanciare solo `std::bad_alloc` per allocazione fallita.

#### 3.6 Type Consistency Audit

Coerente. `ConstraintId` è `std::size_t`, consistente con `TypeVarId`.

#### 3.7 Inter-Component Interaction

Dipende da `TypePtr` (Type.hpp) e `SourceSpan`. Usato da TypeChecker (generazione) e ConstraintSolver (consumo).

#### 3.8 Optimization Opportunities

**DEF-020**: `get()` O(n) → O(1) con `std::unordered_map` o accesso per indice (gli ID sono sequenziali, quindi
`constraints_[id - 1]`).

---

### System: Substitution › Component: Substitution (classe)

#### 3.1 Responsibility Statement

Il componente `Substitution` è esclusivamente responsabile per la gestione di una mappa di binding da type variable a
tipi concreti e per l'applicazione ricorsiva di questi binding ai tipi.

#### 3.2 Class Structure

| Membro       | Tipo                                     | Visibilità |
|--------------|------------------------------------------|------------|
| `bindings_`  | `std::unordered_map<TypeVarId, TypePtr>` | private    |
| `bind()`     | `void(TypeVarId, TypePtr)`               | public     |
| `lookup()`   | `optional<TypePtr>(TypeVarId) const`     | public     |
| `apply()`    | `TypePtr(const TypePtr&) const`          | public     |
| `contains()` | `bool(TypeVarId) const`                  | public     |
| `size()`     | `size_t() const`                         | public     |

#### 3.3 Interface Analysis

Tutti i metodi documentati con Doxygen e esempi. Contratti chiari.

#### 3.4 Implementation Logic

`apply()` è il metodo più complesso: dispatch su `kind()` del tipo, con ricorsione per TypeVariable, ArrayType,
VectorType.

#### 3.5 Error Handling Evaluation

Nessun errore generato. Funzione totale.

#### 3.6 Type Consistency Audit

Coerente. **DEF-022** (già citato): ricorsione potenzialmente infinita per cicli nei binding (prevenuta dall'occurs
check ma senza assertion difensiva).

#### 3.7 Inter-Component Interaction

Usato da ConstraintSolver (scrittura) e TypeChecker/zonk_type (lettura).

#### 3.8 Optimization Opportunities

**DEF-023**: Allocazioni multiple in `apply()`. Usare small-object optimization o pool di allocazione per i TypePtr
temporanei.

---

### System: SymbolTable › Component: SymbolTable (classe)

#### 3.1 Responsibility Statement

Il componente `SymbolTable` è esclusivamente responsabile per la gestione di scope lessicali annidati e l'associazione
tra identificatori e i loro TypeScheme.

#### 3.2 Class Structure

| Membro                       | Tipo                                        | Visibilità |
|------------------------------|---------------------------------------------|------------|
| `scopes_`                    | `vector<unordered_map<string, TypeScheme>>` | private    |
| `push_scope()`               | `void()`                                    | public     |
| `pop_scope()`                | `void()`                                    | public     |
| `define()`                   | `void(string_view, TypeScheme)`             | public     |
| `lookup()`                   | `optional<TypeScheme>(string_view) const`   | public     |
| `defined_in_current_scope()` | `bool(string_view) const`                   | public     |
| `depth()`                    | `size_t() const`                            | public     |

#### 3.3 Interface Analysis

`pop_scope()` ha precondizione `depth() > 0` ma la viola silenziosamente (guardia `!empty()` invece di assertion).

#### 3.4 Implementation Logic

Stack di hashmap. Lookup reverse-itera dallo scope più interno. Define usa `insert_or_assign`.

#### 3.5 Error Handling Evaluation

**DEF-026**: `define()` sovrascrive silenziosamente. **DEF-024**: auto-creazione scope maschera bug.

#### 3.6 Type Consistency Audit

Coerente. **DEF-025**: allocazioni stringa in lookup.

#### 3.7 Inter-Component Interaction

Usato esclusivamente dal TypeChecker.

#### 3.8 Optimization Opportunities

**DEF-025**: Chiave `std::string_view` per `unordered_map` con custom hasher.

---

### System: TypeScheme › Component: TypeScheme (struct)

#### 3.1 Responsibility Statement

Il componente `TypeScheme` è esclusivamente responsabile per rappresentare tipi polimorfici quantificati e la loro
istanziazione con fresh type variable.

#### 3.2 Class Structure

| Membro            | Tipo                               | Visibilità |
|-------------------|------------------------------------|------------|
| `quantified_vars` | `vector<TypeVarId>`                | public     |
| `body`            | `TypePtr`                          | public     |
| `is_const`        | `bool`                             | public     |
| `instantiate()`   | `TypePtr() const`                  | public     |
| `mono()`          | `static TypeScheme(TypePtr, bool)` | public     |

#### 3.3 Interface Analysis

`instantiate()` non documenta il caso parziale (tipi composti).

#### 3.4 Implementation Logic

Genera fresh vars per ogni quantified var, sostituisce solo se il body è direttamente un TypeVariable. Per tipi
composti, restituisce il body invariato.

#### 3.5 Error Handling Evaluation

Nessun errore. Funzione parziale per tipi composti.

#### 3.6 Type Consistency Audit

Coerente.

#### 3.7 Inter-Component Interaction

Usato da SymbolTable (come valore della mappa) e TypeChecker (lookup e instantiate).

#### 3.8 Optimization Opportunities

**DEF-027**: Implementare visitor-based substitution per tipi composti.

---

### System: TypeVariable › Component: TypeVariable (classe) e fresh_type_variable()

#### 3.1 Responsibility Statement

Il componente `TypeVariable` è esclusivamente responsabile per la rappresentazione di type variable non risolte durante
l'inferenza; `fresh_type_variable()` genera ID univoci thread-safe.

#### 3.2 Class Structure

| Membro                  | Tipo                                         | Visibilità    |
|-------------------------|----------------------------------------------|---------------|
| `id_`                   | `TypeVarId`                                  | private       |
| Costruttore             | `explicit constexpr TypeVariable(TypeVarId)` | public        |
| `id()`                  | `TypeVarId() const`                          | public        |
| `to_string()`           | `string() const`                             | public        |
| `classof()`             | `static bool(const TypeBase*)`               | public        |
| `operator==`            | `bool(const TypeBase&) const`                | public        |
| `fresh_type_variable()` | `TypePtr() noexcept`                         | free function |

#### 3.3 Interface Analysis

Costruttore `explicit` previene conversioni implicite. `to_string()` non è `noexcept` (alloca stringa).

#### 3.4 Implementation Logic

Contatore thread-local con `++counter`. Semplice e corretto.

#### 3.5 Error Handling Evaluation

Nessun errore possibile. `fresh_type_variable()` usa `make_shared` che può lanciare `bad_alloc`.

#### 3.6 Type Consistency Audit

Coerente. `TypeVarId` è `std::size_t`, compatibile con `ConstraintId`.

#### 3.7 Inter-Component Interaction

Usato da TypeChecker, Substitution, TypeScheme, ConstraintSolver.

#### 3.8 Optimization Opportunities

Nessuna ottimizzazione significativa necessaria.

---

### System: UnionFind › Component: UnionFind (classe)

#### 3.1 Responsibility Statement

Il componente `UnionFind` è esclusivamente responsabile per la gestione di insiemi disgiunti di type variable con
operazioni di find (con path compression) e unite (con union by rank).

#### 3.2 Class Structure

| Membro       | Tipo                                  | Visibilità |
|--------------|---------------------------------------|------------|
| `parent_`    | `unordered_map<TypeVarId, TypeVarId>` | private    |
| `rank_`      | `unordered_map<TypeVarId, uint8_t>`   | private    |
| `make_set()` | `void(TypeVarId)`                     | public     |
| `find()`     | `TypeVarId(TypeVarId)`                | public     |
| `unite()`    | `void(TypeVarId, TypeVarId)`          | public     |
| `same_set()` | `bool(TypeVarId, TypeVarId)`          | public     |
| `size()`     | `size_t() const`                      | public     |

#### 3.3 Interface Analysis

`find()` e `same_set()` non sono `const` (documentato nel commento dell'header per mutazione interna di path
compression).

#### 3.4 Implementation Logic

Path compression ricorsivo in `find()`. Union by rank in `unite()`.

#### 3.5 Error Handling Evaluation

**DEF-029, DEF-030**: `at()` lancia `out_of_range` per ID non registrati.

#### 3.6 Type Consistency Audit

Coerente.

#### 3.7 Inter-Component Interaction

Usato esclusivamente da ConstraintSolver.

#### 3.8 Optimization Opportunities

Sostituire `unordered_map` con `std::vector` se gli ID sono densi e sequenziali (come sono, generati da
`fresh_type_variable()`).

---

### System: ErrorType › Component: ErrorType (classe) e error_type()

#### 3.1 Responsibility Statement

Il componente `ErrorType` è esclusivamente responsabile per fornire un tipo sentinel che si unifica con qualsiasi tipo,
prevenendo errori a cascata dopo un errore di tipo rilevato.

#### 3.2 Class Structure

| Membro         | Tipo                           | Visibilità    |
|----------------|--------------------------------|---------------|
| Costruttore    | `ErrorType()`                  | public        |
| `to_string()`  | `string() const`               | public        |
| `classof()`    | `static bool(const TypeBase*)` | public        |
| `operator==`   | `bool(const TypeBase&) const`  | public        |
| `error_type()` | `TypePtr() noexcept`           | free function |

#### 3.3 Interface Analysis

Singleton restituito da `error_type()`. Costante, thread-safe.

#### 3.4 Implementation Logic

Static local `shared_ptr<const ErrorType>`.

#### 3.5 Error Handling Evaluation

Nessun errore.

#### 3.6 Type Consistency Audit

Coerente.

#### 3.7 Inter-Component Interaction

Usato da TypeChecker (undeclared identifiers) e ConstraintSolver (unificazione silenziosa).

#### 3.8 Optimization Opportunities

Nessuna.

---

## Phase 4 — Prioritized Recommendations

### 4.1 Recommendation Register

### REC-001: Implementare sostituzione completa in TypeScheme::instantiate()

**Indirizzo di carenza**: DEF-027 (§3.4 TypeScheme) — `instantiate()` non sostituisce variabili quantificate in tipi
composti.

**Descrizione**: Implementare un visitor o una funzione ricorsiva che attraversi il body del TypeScheme e sostituisca
ogni occorrenza di TypeVariable il cui ID è in `quantified_vars` con una fresh type variable. Change entry point:
`TypeScheme::instantiate()` in `include/jsav/typechecker/TypeScheme.hpp:30` e
`src/jsav_Lib/typechecker/TypeScheme.cpp:12–34`. Creare una funzione helper
`substitute_in_type(const TypePtr&, const unordered_map<TypeVarId, TypePtr>&)` che gestisca tutti i TypeKind. L'outcome
atteso è che schemi polimorfici con body composti (es. `∀T. Vec<T> → T`) vengano istanziati correttamente.

**Feasibility**: 3/5 — Richiede implementazione di visitor ricorsivo ma la struttura Type è già progettata per il
dispatch su kind.

**Expected ROI**: 5/5 — Senza questa correzione, il polimorfismo non funziona per nessun tipo composto, rendendo
l'intero sistema di TypeScheme inefficace per casi d'uso reali.

**Implementation Effort**: 2/5 — 30–50 righe di codice ricorsivo con switch su TypeKind.

**Priority Rank**: 3×2 + 5×2 + 2×1 = 6 + 10 + 2 = **18**

**Estimated Implementation Time**: 2–4 ore

**Required Resources**: Sviluppatore C++ con conoscenza di type system. Test unitari per verificare istanziazione di
ArrayType e VectorType polimorfici.

**Effectiveness Indicators**:

1. Test `STATIC_REQUIRE` in `constexpr_tests.cpp` che verifica l'istanziazione di `∀T. Vec<T>` produce `Vec<?T_nuovo>`
   con ID diverso
2. Zero casi di TypeScheme con variabili quantificate non sostituite rilevati da analisi statica
3. Coverage del branch compound-type in `instantiate()` ≥ 90%

---

### REC-002: Correggere unificazione CustomType per confrontare i nomi

**Indirizzo di carenza**: DEF-016 (§2.5 ConstraintSolver) — CustomType con nomi diversi si unificano silenziosamente.

**Descrizione**: Aggiungere un caso `TypeKind::Custom` nello switch di `ConstraintSolver::unify()` che confronti i nomi
dei due CustomType. Change entry point: `ConstraintSolver::unify()` in
`src/jsav_Lib/typechecker/ConstraintSolver.cpp:109`. Se i nomi differiscono, restituire
`CompileError::TypeError(E2034, ...)`. L'outcome è che `Foo` e `Bar` non si unifichino più.

**Feasibility**: 5/5 — Aggiunta di un caso switch di 5 righe.

**Expected ROI**: 4/5 — Previene unificazione errata tra tipi utente diversi, errore semantico grave.

**Implementation Effort**: 5/5 — Modifica minima, ~5 righe.

**Priority Rank**: 5×2 + 4×2 + 5×1 = 10 + 8 + 5 = **23**

**Estimated Implementation Time**: 30 minuti – 1 ora

**Required Resources**: Nessuna dipendenza aggiuntiva.

**Effectiveness Indicators**:

1. Test case `ConstraintSolver_CustomTypeMismatch_RejectsUnification` che verifica errore per `Foo = Bar`
2. Zero unificazioni CustomType errate nei test esistenti

---

### REC-003: Aggiungere vincolo size nell'unificazione Array

**Indirizzo di carenza**: DEF-017 (§2.5 ConstraintSolver) — Array di dimensioni diverse si unificano senza errore.

**Descrizione**: Nel caso `TypeKind::Array` di `ConstraintSolver::unify()` (
`src/jsav_Lib/typechecker/ConstraintSolver.cpp:109–115`), aggiungere un confronto tra le espressioni di dimensione. Se
le size expression non sono uguali (confronto strutturale tramite `sizes_equal` già esistente in
`ArrayType::operator==`), restituire errore `E2034`. Change entry point: `ConstraintSolver::unify()`.

**Feasibility**: 4/5 — Richiede riutilizzo della logica di confronto esistente in `ArrayType`.

**Expected ROI**: 4/5 — Previene unificazione di array incompatibili, errore di tipo significativo.

**Implementation Effort**: 4/5 — ~10 righe di codice.

**Priority Rank**: 4×2 + 4×2 + 4×1 = 8 + 8 + 4 = **20**

**Estimated Implementation Time**: 1–2 ore

**Required Resources**: Accesso alla funzione `sizes_equal` da `ArrayType` (attualmente private — potrebbe richiedere
friend o funzione libera).

**Effectiveness Indicators**:

1. Test `[i32; 3] = [i32; 5]` produce errore E2034
2. Test `[i32; 3] = [i32; 3]` unifica correttamente

---

### REC-004: Risolvere name resolution incompleta per MainStmt

**Indirizzo di carenza**: DEF-009 (§2.5 TypeChecker) — il body di MainStmt non viene risolto durante la name resolution.

**Descrizione**: Nel caso `NodeKind::MainStmt` di `resolve_names_stmt()` (
`src/jsav_Lib/typechecker/TypeChecker.cpp:124–127`), aggiungere la traversata ricorsiva del body. Change entry point:
`TypeChecker::resolve_names_stmt()`. Se il body è un BlockStmt, iterare sugli statement e chiamare
`resolve_names_stmt()` per ciascuno. Se è un singolo statement, chiamare direttamente.

**Feasibility**: 5/5 — Pattern già implementato per `FuncDecl` e `BlockStmt`.

**Expected ROI**: 4/5 — Previene falsi positivi "Undeclared identifier" per variabili nel body di main.

**Implementation Effort**: 5/5 — ~8 righe di codice.

**Priority Rank**: 5×2 + 4×2 + 5×1 = 10 + 8 + 5 = **23**

**Estimated Implementation Time**: 30 minuti – 1 ora

**Required Resources**: Nessuna.

**Effectiveness Indicators**:

1. Test con variabile dichiarata nel body di MainStmt che viene risolta correttamente
2. Zero falsi positivi E2033 per variabili nel body di main

---

### REC-005: Correggere zonk_block_full per non scartare statement

**Indirizzo di carenza**: DEF-010 (§2.5 TypeChecker) — statement non zonkati vengono persi silenziosamente.

**Descrizione**: In `zonk_block_full()` (`src/jsav_Lib/typechecker/TypeChecker.cpp:370–383`), quando `zonk_stmt_full`
restituisce `nullptr`, invece di saltare silenziosamente, preservare lo statement originale dal blocco (il blocco è
`const` quindi richiede clone) o registrare un errore. Change entry point: `TypeChecker::zonk_block_full()`. La
soluzione più conservativa è aggiungere
`zonked_stmts.push_back(std::make_unique<TypedExprStmt>(..., error_type(), ...))` con errore.

**Feasibility**: 3/5 — Richiede decisione architetturale (errore vs preservazione).

**Expected ROI**: 4/5 — Previene perdita silenziosa di codice durante lo zonk.

**Implementation Effort**: 3/5 — ~10 righe.

**Priority Rank**: 3×2 + 4×2 + 3×1 = 6 + 8 + 3 = **17**

**Estimated Implementation Time**: 1–2 ore

**Required Resources**: Decisione del technical lead sulla strategia (errore vs preservazione).

**Effectiveness Indicators**:

1. Test con statement che produce nullptr da zonk_stmt_full verifica che l'errore sia registrato
2. Zero statement persi nel TypedProgram risultante

---

### REC-006: Implementare vincoli di signature per CallExpr

**Indirizzo di carenza**: DEF-011 (§2.5 TypeChecker) — le chiamate a funzione non verificano arity o tipo degli
argomenti.

**Descrizione**: In `type_expr` per `CallExpr` (`src/jsav_Lib/typechecker/TypeChecker.cpp:689–730`), quando il callee è
un Identifier, recuperare il TypeScheme dalla SymbolTable, istanziarlo, e generare vincoli tra i tipi degli argomenti e
i tipi dei parametri della signature. Richiede una rappresentazione di tipo funzione (es. `FunctionType` con
`param_types: vector<TypePtr>` e `return_type: TypePtr`). Change entry point: `TypeChecker::type_expr()` caso
`NodeKind::CallExpr`. Se la signature non è disponibile (funzione non dichiarata), generare errore E2027.

**Feasibility**: 2/5 — Richiede nuova rappresentazione FunctionType e modifica significativa alla constraint generation.

**Expected ROI**: 5/5 — Senza questo, le chiamate a funzione non sono verificate, rendendo il type checker incompleto
per programmi con funzioni.

**Implementation Effort**: 1/5 — Stimato 1–2 settimane per FunctionType, parsing signature, e vincoli.

**Priority Rank**: 2×2 + 5×2 + 1×1 = 4 + 10 + 1 = **15**

**Estimated Implementation Time**: 1–2 settimane

**Required Resources**: Nuova classe `FunctionType` in `Type.hpp`. Modifica al parser per supportare signature nelle
FuncDecl.

**Effectiveness Indicators**:

1. Test `fn f(x: i32): i32 { x }` e `f("string")` produce errore E2029
2. Test `f()` con numero errato di argomenti produce errore E2028
3. Coverage del branch CallExpr con signature ≥ 85%

---

### REC-007: Generare vincoli per CastExpr

**Indirizzo di carenza**: DEF-013 (§2.5 TypeChecker) — CastExpr non genera vincoli di compatibilità.

**Descrizione**: In `type_expr` per `CastExpr` (`src/jsav_Lib/typechecker/TypeChecker.cpp:850–862`), aggiungere vincolo
tra il tipo dell'operando e il tipo target per le conversioni implicite, o validazione esplicita per cast espliciti.
Change entry point: `TypeChecker::type_expr()` caso `NodeKind::CastExpr`. Per ora, generare vincolo
`constraints_.add(operand_type, target_type, ...)` per conversioni implicite.

**Feasibility**: 5/5 — Aggiunta di una riga di vincolo.

**Expected ROI**: 3/5 — Migliora la correttezza del cast ma non è un errore critico se i cast sono espliciti.

**Implementation Effort**: 5/5 — ~3 righe.

**Priority Rank**: 5×2 + 3×2 + 5×1 = 10 + 6 + 5 = **21**

**Estimated Implementation Time**: 30 minuti

**Required Resources**: Nessuna.

**Effectiveness Indicators**:

1. Test con cast incompatibile (es. `string` → `i32`) registra errore o vincolo
2. Coverage del branch CastExpr ≥ 90%

---

### REC-008: Gestire nullptr in VarDecl dopo type_expr

**Indirizzo di carenza**: DEF-014 (§2.5 TypeChecker) — accesso a `typed_init->node_type()` su possibile nullptr.

**Descrizione**: In `type_stmt` per `VarDecl` (`src/jsav_Lib/typechecker/TypeChecker.cpp:939–966`), aggiungere verifica
`if(!typed_init) { /* gestisci errore */ }` prima di accedere a `typed_init->node_type()`. Change entry point:
`TypeChecker::type_stmt()` caso `NodeKind::VarDecl`. Se `typed_init` è nullptr, registrare errore E2033 e procedere con
fresh type variable.

**Feasibility**: 5/5 — Aggiunta di guardia if.

**Expected ROI**: 5/5 — Previene undefined behavior (crash potenziale).

**Implementation Effort**: 5/5 — ~5 righe.

**Priority Rank**: 5×2 + 5×2 + 5×1 = 10 + 10 + 5 = **25**

**Estimated Implementation Time**: 30 minuti

**Required Resources**: Nessuna.

**Effectiveness Indicators**:

1. AddressSanitizer: zero crash su programmi con errore nel primo elemento di array literal usato come initializer
2. Test con initializer invalido non causa crash

---

### REC-009: Rimuovere UnionFind dal ConstraintSolver

**Indirizzo di carenza**: DEF-019 (§2.5 ConstraintSolver) — duplicazione di stato tra UnionFind e Substitution.

**Descrizione**: Rimuovere `union_find_` da `ConstraintSolver` e usare direttamente la `substitution_` per determinare
l'equivalenza di type variable. Change entry point: `ConstraintSolver` in
`include/jsav/typechecker/ConstraintSolver.hpp:75–76` e `src/jsav_Lib/typechecker/ConstraintSolver.cpp`. Sostituire le
chiamate a `union_find_.make_set()`, `unite()`, `find()` con lookup diretta nella substitution.

**Feasibility**: 3/5 — Richiede riprogettazione della logica di unificazione ma non è complesso.

**Expected ROI**: 3/5 — Riduce complessità e memoria ma non è un bug funzionale.

**Implementation Effort**: 3/5 — ~30 righe da modificare/rimuovere.

**Priority Rank**: 3×2 + 3×2 + 3×1 = 6 + 6 + 3 = **15**

**Estimated Implementation Time**: 2–4 ore

**Required Resources**: Nessuna dipendenza esterna.

**Effectiveness Indicators**:

1. Tutti i test esistenti passano senza UnionFind
2. Riduzione della memoria del solver ≥ 15% misurata con profiler

---

### REC-010: Ottimizzare ConstraintSet::get() da O(n) a O(1)

**Indirizzo di carenza**: DEF-020 (§2.5 Constraint) — lookup lineare per ID.

**Descrizione**: Dato che gli ID sono sequenziali (1, 2, 3, ...), sostituire `std::ranges::find` con accesso diretto per
indice: `return &constraints_[id - 1]` con bounds check. Change entry point: `ConstraintSet::get()` in
`src/jsav_Lib/typechecker/Constraint.cpp:18–21`.

**Feasibility**: 5/5 — Sostituzione di una riga.

**Expected ROI**: 2/5 — Miglioramento prestazionale ma non critico per correttezza.

**Implementation Effort**: 5/5 — 2 righe.

**Priority Rank**: 5×2 + 2×2 + 5×1 = 10 + 4 + 5 = **19**

**Estimated Implementation Time**: 15 minuti

**Required Resources**: Nessuna.

**Effectiveness Indicators**:

1. Benchmark: tempo di lookup per ID ridotto da O(n) a O(1)
2. Tutti i test esistenti passano

---

### REC-011: Rendere type_expr e type_stmt privati nel TypeChecker

**Indirizzo di carenza**: DEF-002 (§1.3) — metodi pubblici rompono incapsulamento.

**Descrizione**: Spostare `type_expr()` e `type_stmt()` da `public` a `private` in `TypeChecker.hpp:62–77`. Se sono
necessari per unit testing, esporli tramite una funzione amica o un metodo `friend` nella classe di test. Change entry
point: `include/jsav/typechecker/TypeChecker.hpp:62`.

**Feasibility**: 4/5 — Richiede aggiornamento dei test che chiamano direttamente questi metodi.

**Expected ROI**: 3/5 — Migliora l'incapsulamento e previene uso improprio dell'API.

**Implementation Effort**: 4/5 — Modifica header e aggiornamento test.

**Priority Rank**: 4×2 + 3×2 + 4×1 = 8 + 6 + 4 = **18**

**Estimated Implementation Time**: 1–2 ore

**Required Resources**: Aggiornamento dei file di test.

**Effectiveness Indicators**:

1. Compilazione fallisce per codice esterno che chiama type_expr direttamente
2. Test esistenti aggiornati e passano

---

### REC-012: Aggiungere gestione esplicita di ErrorType in Substitution::apply

**Indirizzo di carenza**: DEF-006 (§1.4) — ErrorType non menzionato esplicitamente in apply.

**Descrizione**: Aggiungere caso `TypeKind::Error` nello switch di `Substitution::apply()` che restituisca
esplicitamente il tipo invariato. Change entry point: `Substitution::apply()` in
`src/jsav_Lib/typechecker/Substitution.cpp:21–54`. Aggiungere commento che documenta la scelta intenzionale.

**Feasibility**: 5/5 — Aggiunta di un caso switch.

**Expected ROI**: 2/5 — Prevenzione difensiva, non bug corrente.

**Implementation Effort**: 5/5 — 3 righe.

**Priority Rank**: 5×2 + 2×2 + 5×1 = 10 + 4 + 5 = **19**

**Estimated Implementation Time**: 15 minuti

**Required Resources**: Nessuna.

**Effectiveness Indicators**:

1. Codice esplicitamente documentato nel commento del caso ErrorType
2. Nessun cambiamento comportamentale verificato dai test esistenti

---

### REC-013: Implementare MemberExpr con vincoli e lookup

**Indirizzo di carenza**: DEF-012 (§2.5 TypeChecker) — MemberExpr totalmente non implementato.

**Descrizione**: In `type_expr` per `MemberExpr` (`src/jsav_Lib/typechecker/TypeChecker.cpp:843–848`), implementare
lookup del membro sull'oggetto. Richiede una rappresentazione dei tipi record/struct con campi. Change entry point:
`TypeChecker::type_expr()` caso `NodeKind::MemberExpr`. In attesa di RecordType, generare errore E2033 "Member access
not yet supported".

**Feasibility**: 2/5 — Richiede nuovo tipo RecordType e infrastruttura di lookup.

**Expected ROI**: 3/5 — Necessario per supporto a struct/class ma non urgente se il linguaggio target non le ha ancora.

**Implementation Effort**: 1/5 — Stimato 1–2 settimane per implementazione completa.

**Priority Rank**: 2×2 + 3×2 + 1×1 = 4 + 6 + 1 = **11**

**Estimated Implementation Time**: 1–2 settimane (implementazione completa) / 1 ora (errore temporaneo)

**Required Resources**: Nuova classe `RecordType`, modifica al parser, SymbolTable estesa.

**Effectiveness Indicators**:

1. Test con accesso a membro di struct produce errore chiaro o risultato corretto
2. Zero TypeVariable fresche non vincolate da MemberExpr

---

### REC-014: Sostituire dynamic_cast con kind()+static_cast in zonk_type

**Indirizzo di carenza**: DEF-033 (§3.6 TypeChecker) — inconsistenza nel pattern di casting.

**Descrizione**: In `zonk_type()` (`src/jsav_Lib/typechecker/TypeChecker.cpp:43`), sostituire
`dynamic_cast<const TypeVariable*>(type.get())` con
`if(type->kind() == TypeKind::TypeVar) { const auto* tvar = static_cast<const TypeVariable*>(type.get()); ... }`. Change
entry point: `TypeChecker::zonk_type()`.

**Feasibility**: 5/5 — Sostituzione diretta.

**Expected ROI**: 2/5 — Miglioramento prestazionale minore, ma coerenza del codice.

**Implementation Effort**: 5/5 — 3 righe.

**Priority Rank**: 5×2 + 2×2 + 5×1 = 10 + 4 + 5 = **19**

**Estimated Implementation Time**: 15 minuti

**Required Resources**: Nessuna.

**Effectiveness Indicators**:

1. Zero dynamic_cast nel file TypeChecker.cpp verificato da clang-tidy
2. Tutti i test esistenti passano

---

### REC-015: Aggiungere rilevamento ridefinizioni in SymbolTable::define

**Indirizzo di carenza**: DEF-026 (§2.5 SymbolTable) — define sovrascrive silenziosamente.

**Descrizione**: Modificare `SymbolTable::define()` (`src/jsav_Lib/typechecker/SymbolTable.cpp:16–19`) per restituire
`std::optional<TypeScheme>` con il precedente binding se esisteva, permettendo al TypeChecker di generare warning o
errore. Change entry point: `SymbolTable::define()` in `include/jsav/typechecker/SymbolTable.hpp:43` e
`src/jsav_Lib/typechecker/SymbolTable.cpp:16`.

**Feasibility**: 4/5 — Modifica della signature e aggiornamento del chiamante.

**Expected ROI**: 3/5 — Previene bug da ridefinizioni accidentali.

**Implementation Effort**: 3/5 — ~10 righe.

**Priority Rank**: 4×2 + 3×2 + 3×1 = 8 + 6 + 3 = **17**

**Estimated Implementation Time**: 1–2 ore

**Required Resources**: Aggiornamento del chiamante in TypeChecker.

**Effectiveness Indicators**:

1. Test con ridefinizione nello stesso scope genera errore o warning
2. Zero sovrascritture silenziose nei test esistenti

---

### REC-016: Ottimizzare parse_type_annotation con lookup table

**Indirizzo di carenza**: DEF-036 (§3.8 TypeChecker) — 14 confronti stringa sequenziali.

**Descrizione**: Sostituire la catena if-else con `std::array<std::pair<std::string_view, TypePtr>, N>` ordinato e
`std::ranges::lower_bound`. Change entry point: `parse_type_annotation()` in
`src/jsav_Lib/typechecker/TypeChecker.cpp:19–36`.

**Feasibility**: 5/5 — Sostituzione meccanica.

**Expected ROI**: 2/5 — Miglioramento prestazionale minore (chiamata infrequente).

**Implementation Effort**: 4/5 — ~15 righe.

**Priority Rank**: 5×2 + 2×2 + 4×1 = 10 + 4 + 4 = **18**

**Estimated Implementation Time**: 1 ora

**Required Resources**: Nessuna.

**Effectiveness Indicators**:

1. Benchmark: tempo di parse_type_annotation ridotto del 60%+
2. Tutti i test esistenti passano

---

### REC-017: Implementare Name Resolution completa per tutti gli statement

**Indirizzo di carenza**: DEF-008 (§2.5 TypeChecker) — resolve_names_stmt ignora molti statement.

**Descrizione**: Aggiungere casi per `ReturnStmt`, `IfStmt`, `WhileStmt`, `ForStmt`, `BreakStmt`, `ContinueStmt` in
`resolve_names_stmt()` (`src/jsav_Lib/typechecker/TypeChecker.cpp:97–147`). Per IfStmt/WhileStmt/ForStmt, risolvere
ricorsivamente condition e body. Change entry point: `TypeChecker::resolve_names_stmt()`.

**Feasibilità**: 4/5 — Pattern già stabilito per BlockStmt.

**Expected ROI**: 3/5 — Migliora la coerenza della name resolution ma la constraint generation gestisce già questi casi.

**Implementation Effort**: 3/5 — ~30 righe.

**Priority Rank**: 4×2 + 3×2 + 3×1 = 8 + 6 + 3 = **17**

**Estimated Implementation Time**: 2–4 ore

**Required Resources**: Nessuna.

**Effectiveness Indicators**:

1. Tutti i NodeKind gestiti in resolve_names_stmt verificato da test di esaustività
2. Zero casi default non gestiti

---

### REC-018: Prevenire allocazioni stringa in SymbolTable::lookup

**Indirizzo di carenza**: DEF-025 (§2.5 SymbolTable) — allocazioni std::string ad ogni lookup.

**Descrizione**: Cambiare il tipo di `scopes_` da `vector<unordered_map<string, TypeScheme>>` a
`vector<unordered_map<string_view_ref, TypeScheme>>` con wrapper che usa string_view senza copia. Change entry point:
`SymbolTable` in `include/jsav/typechecker/SymbolTable.hpp:65` e `src/jsav_Lib/typechecker/SymbolTable.cpp`.

**Feasibility**: 2/5 — Richiede custom hasher per string_view e gestione della lifetime delle chiavi.

**Expected ROI**: 3/5 — Riduzione allocazioni ma complessità aggiuntiva.

**Implementation Effort**: 2/5 — Stimato 4–8 ore per implementazione sicura.

**Priority Rank**: 2×2 + 3×2 + 2×1 = 4 + 6 + 2 = **12**

**Estimated Implementation Time**: 4–8 ore

**Required Resources**: Custom hasher, attenzione alla lifetime.

**Effectiveness Indicators**:

1. Profiler: zero allocazioni heap durante SymbolTable::lookup
2. Tutti i test esistenti passano

---

### REC-019: Usare std::expected<TypeScheme, CompileError> per SymbolTable::lookup

**Indirizzo di carenza**: DEF-005 (§1.4) — lookup restituisce std::nullopt senza contesto.

**Descrizione**: Cambiare la signature di `SymbolTable::lookup()` da `std::optional<TypeScheme>` a
`std::expected<TypeScheme, CompileError>`. Quando il simbolo non è trovato, restituire errore con codice E2023. Change
entry point: `SymbolTable::lookup()` in `include/jsav/typechecker/SymbolTable.hpp:53` e tutti i punti di chiamata in
TypeChecker.

**Feasibility**: 3/5 — Richiede aggiornamento di tutti i chiamanti.

**Expected ROI**: 3/5 — Uniforma la gestione errori ma non è critico.

**Implementation Effort**: 2/5 — ~20 righe distribuite.

**Priority Rank**: 3×2 + 3×2 + 2×1 = 6 + 6 + 2 = **14**

**Estimated Implementation Time**: 2–4 ore

**Required Resources**: Aggiornamento dei punti di chiamata.

**Effectiveness Indicators**:

1. Tutti i lookup falliti restituiscono CompileError con codice e messaggio
2. Zero `std::nullopt` silenziosi nella pipeline

---

### REC-020: Aggiungere bounds check in UnionFind::find e unite

**Indirizzo di carenza**: DEF-029, DEF-030 (§2.5 UnionFind) — `at()` lancia eccezioni non documentate.

**Descrizione**: Sostituire `parent_.at(var)` con controllo esplicito:
`auto it = parent_.find(var); if(it == parent_.end()) { /* errore o assertion */ }`. In modalità debug, usare `assert`.
Change entry point: `UnionFind::find()` e `UnionFind::unite()` in `src/jsav_Lib/typechecker/UnionFind.cpp:15–33`.

**Feasibility**: 5/5 — Sostituzione diretta.

**Expected ROI**: 3/5 — Previene crash con messaggio oscuro.

**Implementation Effort**: 4/5 — ~10 righe.

**Priority Rank**: 5×2 + 3×2 + 4×1 = 10 + 6 + 4 = **20**

**Estimated Implementation Time**: 1 ora

**Required Resources**: Nessuna.

**Effectiveness Indicators**:

1. Test con ID non registrato produce assertion failure o errore gestito, non eccezione std::out_of_range
2. Zero eccezioni non catturate nei test

---

### REC-021: Rifattorizzare TypeChecker.cpp in file multipli

**Indirizzo di carenza**: DEF-007 (§2.2 TypeChecker) — 1180 righe, type_expr 480+ righe.

**Descrizione**: Suddividere `TypeChecker.cpp` in: `TypeCheckerCore.cpp` (check, resolve_names, zonk),
`TypeCheckerExpr.cpp` (type_expr), `TypeCheckerStmt.cpp` (type_stmt). Change entry point: intero file
`src/jsav_Lib/typechecker/TypeChecker.cpp`. Mantenere la classe TypeChecker unica, spostando le definizioni dei metodi.

**Feasibility**: 4/5 — Rifattorizzazione meccanica senza cambiamenti logici.

**Expected ROI**: 4/5 — Migliora drasticamente manutenibilità e velocità di compilazione incrementale.

**Implementation Effort**: 3/5 — 1–2 giorni di rifattorizzazione.

**Priority Rank**: 4×2 + 4×2 + 3×1 = 8 + 8 + 3 = **19**

**Estimated Implementation Time**: 1–2 giorni

**Required Resources**: Build system update per aggiungere nuovi file al target.

**Effectiveness Indicators**:

1. Nessun file .cpp > 500 righe nel typechecker
2. Nessun metodo > 100 righe verificato da lizard
3. Tutti i test esistenti passano

---

### REC-022: Aggiungere gestione esplicita ErrorType nel ConstraintSolver

**Indirizzo di carenza**: Cross-cutting concern — ErrorType menzionato in ConstraintSolver ma non in
Substitution/TypeScheme.

**Descrizione**: Uniformare la gestione di ErrorType attraverso tutti i sistemi. Aggiungere commento esplicito in
`Substitution::apply()` (già coperto da REC-012) e verificare che `TypeScheme::instantiate()` gestisca ErrorType nel
body (attualmente: sì, perché non è TypeVariable né composto). Change entry point: `Substitution::apply()` e
`TypeScheme::instantiate()`.

**Feasibility**: 5/5 — Documentazione e verifica.

**Expected ROI**: 2/5 — Coerenza architetturale.

**Implementation Effort**: 5/5 — Minimo sforzo.

**Priority Rank**: 5×2 + 2×2 + 5×1 = 10 + 4 + 5 = **19**

**Estimated Implementation Time**: 30 minuti

**Required Resources**: Nessuna.

**Effectiveness Indicators**:

1. Documentazione esplicita per ErrorType in ogni sistema che processa TypePtr
2. Zero comportamenti impliciti non documentati

---

### 4.2 Summary Priority Table

| Rank | ID          | Title                                                             | Feasibility | ROI | Effort | Composite Score | Est. Time     |
|------|-------------|-------------------------------------------------------------------|-------------|-----|--------|-----------------|---------------|
| 1    | **REC-008** | Gestire nullptr in VarDecl dopo type_expr                         | 5           | 5   | 5      | **25**          | 30 min        |
| 2    | **REC-002** | Correggere unificazione CustomType per confrontare i nomi         | 5           | 4   | 5      | **23**          | 30 min – 1 h  |
| 3    | **REC-004** | Risolvere name resolution incompleta per MainStmt                 | 5           | 4   | 5      | **23**          | 30 min – 1 h  |
| 4    | **REC-007** | Generare vincoli per CastExpr                                     | 5           | 3   | 5      | **21**          | 30 min        |
| 5    | **REC-003** | Aggiungere vincolo size nell'unificazione Array                   | 4           | 4   | 4      | **20**          | 1–2 h         |
| 6    | **REC-020** | Aggiungere bounds check in UnionFind::find e unite                | 5           | 3   | 4      | **20**          | 1 h           |
| 7    | **REC-001** | Implementare sostituzione completa in TypeScheme::instantiate()   | 3           | 5   | 2      | **18**          | 2–4 h         |
| 8    | **REC-011** | Rendere type_expr e type_stmt privati nel TypeChecker             | 4           | 3   | 4      | **18**          | 1–2 h         |
| 9    | **REC-016** | Ottimizzare parse_type_annotation con lookup table                | 5           | 2   | 4      | **18**          | 1 h           |
| 10   | **REC-005** | Correggere zonk_block_full per non scartare statement             | 3           | 4   | 3      | **17**          | 1–2 h         |
| 11   | **REC-015** | Aggiungere rilevamento ridefinizioni in SymbolTable::define       | 4           | 3   | 3      | **17**          | 1–2 h         |
| 12   | **REC-017** | Implementare Name Resolution completa per tutti gli statement     | 4           | 3   | 3      | **17**          | 2–4 h         |
| 13   | **REC-010** | Ottimizzare ConstraintSet::get() da O(n) a O(1)                   | 5           | 2   | 5      | **19**          | 15 min        |
| 14   | **REC-012** | Aggiungere gestione esplicita di ErrorType in Substitution::apply | 5           | 2   | 5      | **19**          | 15 min        |
| 15   | **REC-014** | Sostituire dynamic_cast con kind()+static_cast in zonk_type       | 5           | 2   | 5      | **19**          | 15 min        |
| 16   | **REC-021** | Rifattorizzare TypeChecker.cpp in file multipli                   | 4           | 4   | 3      | **19**          | 1–2 giorni    |
| 17   | **REC-022** | Aggiungere gestione esplicita ErrorType nel ConstraintSolver      | 5           | 2   | 5      | **19**          | 30 min        |
| 18   | **REC-006** | Implementare vincoli di signature per CallExpr                    | 2           | 5   | 1      | **15**          | 1–2 sett.     |
| 19   | **REC-009** | Rimuovere UnionFind dal ConstraintSolver                          | 3           | 3   | 3      | **15**          | 2–4 h         |
| 20   | **REC-019** | Usare std::expected per SymbolTable::lookup                       | 3           | 3   | 2      | **14**          | 2–4 h         |
| 21   | **REC-018** | Prevenire allocazioni stringa in SymbolTable::lookup              | 2           | 3   | 2      | **12**          | 4–8 h         |
| 22   | **REC-013** | Implementare MemberExpr con vincoli e lookup                      | 2           | 3   | 1      | **11**          | 1 h – 2 sett. |
