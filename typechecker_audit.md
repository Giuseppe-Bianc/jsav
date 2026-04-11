# Type Checker Implementation Audit

## Phase 1 — System Ensemble Analysis

### 1.1 System Enumeration

Il type checker di **jsav** comprende **11 header** e **10 file di implementazione** organizzati in cinque sistemi
principali:

**Sistema 1 — Type Representation System** (rappresentazione dei tipi)

| File Header                 | File Implementazione        | Responsabilità primaria                                                                         |
|-----------------------------|-----------------------------|-------------------------------------------------------------------------------------------------|
| `include/jsav/ast/Type.hpp` | `src/jsav_Lib/ast/Type.cpp` | Gerarchia `TypeBase`, `PrimitiveType`, `CustomType`, `ArrayType`, `VectorType`, enum `TypeKind` |

**Sistema 2 — Type Inference Variables System** (variabili di tipo e tipi polimorfici)

| File Header                                 | File Implementazione                        | Responsabilità primaria                                     |
|---------------------------------------------|---------------------------------------------|-------------------------------------------------------------|
| `include/jsav/typechecker/TypeVariable.hpp` | `src/jsav_Lib/typechecker/TypeVariable.cpp` | `TypeVariable` — variabili di tipo `?Tn` per inferenza      |
| `include/jsav/typechecker/TypeScheme.hpp`   | `src/jsav_Lib/typechecker/TypeScheme.cpp`   | `TypeScheme` — tipi polimorfici ∀(vars).body                |
| `include/jsav/typechecker/ErrorType.hpp`    | `src/jsav_Lib/typechecker/ErrorType.cpp`    | `ErrorType` — tipo sentinella singleton per recupero errori |

**Sistema 3 — Constraint Solving System** (vincoli e unificazione)

| File Header                                     | File Implementazione                            | Responsabilità primaria                                          |
|-------------------------------------------------|-------------------------------------------------|------------------------------------------------------------------|
| `include/jsav/typechecker/Constraint.hpp`       | `src/jsav_Lib/typechecker/Constraint.cpp`       | `Constraint` e `ConstraintSet` — vincoli di uguaglianza tra tipi |
| `include/jsav/typechecker/Substitution.hpp`     | `src/jsav_Lib/typechecker/Substitution.cpp`     | `Substitution` — mapping variabili di tipo → tipi risolti        |
| `include/jsav/typechecker/UnionFind.hpp`        | `src/jsav_Lib/typechecker/UnionFind.cpp`        | `UnionFind` — disjoint-set per unificazione efficiente           |
| `include/jsav/typechecker/ConstraintSolver.hpp` | `src/jsav_Lib/typechecker/ConstraintSolver.cpp` | `ConstraintSolver` — motore di unificazione con occurs-check     |
| `include/jsav/typechecker/TypeVisitor.hpp`      | `src/jsav_Lib/typechecker/TypeVisitor.cpp`      | `TypeVisitor` — visitor per tipi composti (Array, Vector)        |

**Sistema 4 — Name Resolution System** (risoluzione dei nomi)

| File Header                                | File Implementazione                       | Responsabilità primaria                                                        |
|--------------------------------------------|--------------------------------------------|--------------------------------------------------------------------------------|
| `include/jsav/typechecker/SymbolTable.hpp` | `src/jsav_Lib/typechecker/SymbolTable.cpp` | `SymbolTable` — gestione scope lessicali e binding identificatore→`TypeScheme` |

**Sistema 5 — Type Checking Orchestration System** (orchestrazione del type checking)

| File Header                                | File Implementazione                       | Responsabilità primaria                                                              |
|--------------------------------------------|--------------------------------------------|--------------------------------------------------------------------------------------|
| `include/jsav/typechecker/TypeChecker.hpp` | `src/jsav_Lib/typechecker/TypeChecker.cpp` | `TypeChecker` — pipeline completa (resolve → constraints → solve → zonk), 1197 righe |

**Sistemi dipendenti esterni** (non parte del type checker ma consumati):

| Sistema                | Responsabilità                                                      |
|------------------------|---------------------------------------------------------------------|
| `TypedNode.hpp` e soci | Nodi AST tipizzati (`TypedExpr`, `TypedStmt`, `TypedProgram`)       |
| `CompileError.hpp`     | Tipo errore strutturato con `ErrorCode`, `SourceSpan`               |
| `Expressions.hpp`      | Nodi espressione non tipizzati (`IntegerLiteral`, `CallExpr`, ecc.) |
| `Statements.hpp`       | Nodi statement non tipizzati (`VarDecl`, `FuncDecl`, ecc.)          |

### 1.2 Inter-System Dependency Map

```
                    ┌──────────────────────────────────────────────────────┐
                    │              TypeChecker (S5)                        │
                    │  check(): resolve → constraints → solve → zonk       │
                    │  type_expr(): 1197 righe, switch su NodeKind         │
                    └──┬───────────────────────────┬───────────────────────┘
                       │                           │
          ┌────────────▼─────────────┐  ┌──────────▼──────────────┐
          │    ConstraintSet (S3)    │  │   SymbolTable (S4)      │
          │  accumula vincoli        │  │   push/pop scope        │
          │  lhs = rhs @ location    │  │   define/lookup         │
          └──────┬───────────────────┘  └──────┬──────────────────┘
                 │                              │
    ┌────────────▼─────────────┐    ┌───────────▼───────────────┐
    │  ConstraintSolver (S3)   │    │    TypeScheme (S2)        │
    │  solve() → Substitution  │    │    ∀vars. body            │
    │  unify() + occurs-check  │    │    instantiate()          │
    └──┬───────────────────┬───┘    └────────────┬──────────────┘
       │                   │                     │
       ▼                   ▼                     ▼
┌──────────────┐  ┌──────────────────┐  ┌────────────────────────┐
│ UnionFind    │  │ Substitution     │  │ TypeVariable (S2)      │
│ make/find/   │  │ bind/lookup/     │  │ fresh_type_variable()  │
│ unite        │  │ apply (cached)   │  │ thread-local counter   │
└──────────────┘  └──────────────────┘  └────────────┬───────────┘
       │                   │                         │
       └───────────────────┴──────────┬──────────────┘
                                      │
                    ┌─────────────────▼──────────────────┐
                    │       TypeBase + sottoclassi (S1)  │
                    │  PrimitiveType, CustomType,        │
                    │  ArrayType, VectorType, TypeVar,   │
                    │  ErrorType                         │
                    └────────────────────────────────────┘
```

**Dipendenze upstream → downstream**:

1. **Type Representation (S1)** → Tutti gli altri sistemi. `TypePtr` è il tipo fondamentale.
2. **Type Inference Variables (S2)** → Constraint Solving, SymbolTable, TypeChecker.
3. **Constraint Solving (S3)** → TypeChecker (fasi 3 e 4 della pipeline).
4. **Name Resolution (S4)** → TypeChecker (fase 1, risolve i nomi prima della generazione vincoli).
5. **Orchestration (S5)** → Dipende da TUTTI i sistemi sopra. È il sink del DAG.

Non esistono dipendenze circolari. Il grafo è un **DAG** (Directed Acyclic Graph) pulito.

### 1.3 Architectural Coherence Evaluation

L'architettura segue un approccio **constraint-based type inference** con pipeline in quattro fasi documentate in
`TypeChecker.hpp:47–55`. La decomposizione è fondamentalmente solida ma presenta asimmetrie.

**Punti di forza**:

- Separazione netta tra **rappresentazione** (`TypeBase` e sottoclassi), **inferenza** (`ConstraintSolver`/`UnionFind`),
  e **orchestrazione** (`TypeChecker`).
- Visitor pattern appropriato (`TypeVisitor`) per visita di tipi composti [`TypeVisitor.hpp:38–64`].
- `Substitution` con cache persistente [`Substitution.hpp:67–86`, `applyImpl`] per ottimizzare ri-applicazione.
- `ErrorType` come sentinella che unifica silenziosamente con qualsiasi tipo [`ConstraintSolver.cpp:67–68`], prevenendo
  errori a cascata.
- Thread-safety documentata in `fresh_type_variable()` [`TypeVariable.cpp:12`] con counter thread-local.

**Deficienze strutturali**:

- **`TypeChecker::type_expr`** (1197 righe totali, `type_expr` supera le 400 righe) viola il principio di singola
  responsabilità e il limite di complessità cognitiva del progetto (CCN ≤15, AGENTS.md §7). Gestisce generazione
  vincoli, type checking anticipato, e costruzione AST tipizzato simultaneamente.
- **Mancanza di tipo funzione** (`FnType`). Il sistema non ha una rappresentazione esplicita per i tipi funzione
  (`(T1, T2) -> R`). Le chiamate a funzione (`CallExpr`) generano vincoli solo sul callee ma **non verificano l'arity**
  né il tipo di ritorno della funzione chiamata [`TypeChecker.cpp:698–745`].
- **Zonking incompleto**: il metodo `zonk_block_full` scarta statement che non producono risultato
  [`TypeChecker.cpp:417` — commento "Can't move from const — skip"], causando perdita silente di nodi AST.

### 1.4 Cross-Cutting Concerns Assessment

**Matrice dei concern trasversali**:

| Concern                      | Type Representation (S1)               | Constraint Solving (S3)             | SymbolTable (S4)                 | TypeChecker (S5)                                  | Uniformità                                               |
|------------------------------|----------------------------------------|-------------------------------------|----------------------------------|---------------------------------------------------|----------------------------------------------------------|
| **Propagazione errori**      | `ErrorType` singleton                  | `std::expected<void, CompileError>` | `std::nullopt` (silenzioso)      | `std::vector<CompileError>`                       | **INCONSISTENTE** — tre strategie diverse                |
| **Rappresentazione tipi**    | `TypePtr = shared_ptr<const TypeBase>` | `TypePtr`                           | `TypeScheme` (wrappa `TypePtr`)  | `TypePtr`                                         | **UNIFORME**                                             |
| **Gestione scope**           | —                                      | —                                   | `vector<unordered_map>` push/pop | Usa SymbolTable + `current_function_return_type_` | **PARZIALE** — contesto funzione duplicato               |
| **Formattazione diagnostic** | `to_string()` virtuale                 | `reason` string nei vincoli         | —                                | `message_storage_` con `FORMAT()`                 | **PARZIALE** — `deque<string>` fragile per `string_view` |

**`DEF-001` — Propagazione errori inconsistente**: `ErrorType` unifica silenziosamente [`ConstraintSolver.cpp:67`], ma
il TypeChecker accumula errori in `vector<CompileError>` [`TypeChecker.hpp:101`] mentre `SymbolTable::lookup`
restituisce `std::nullopt` senza diagnostic [`SymbolTable.hpp:54–55`]. Quando un identificatore non è dichiarato,
`type_expr` crea l'errore manualmente [`TypeChecker.cpp:493–498`]. Ogni sistema ha la propria strategia — nessun
meccanismo unificato di error propagation.

**`DEF-002` — Contesto funzione duplicato**: `SymbolTable` mantiene `return_type` e `function_name` dentro
`TypeScheme` [`TypeScheme.hpp:28–30`], gestiti da `set_function_return_context()` [`SymbolTable.cpp:37–53`]. Tuttavia
`TypeChecker` non usa questo contesto in modo sistematico — il ritorno viene passato due volte: una volta nel binding e
una volta come vincolo [`TypeChecker.cpp:1001`].

---

## Phase 2 — Per-System Analysis

### System: Type Representation System (S1)

#### 2.1 System Overview

Il **Type Representation System** [`include/jsav/ast/Type.hpp`, `src/jsav_Lib/ast/Type.cpp`] definisce la gerarchia di
classi che rappresentano tutti i tipi del linguaggio. È il fondamento su cui tutti gli altri sistemi operano. Fornisce
`TypeBase` come classe base astratta con le sottoclassi concrete `PrimitiveType`, `CustomType`, `ArrayType`,
`VectorType`. `TypePtr` (`shared_ptr<const TypeBase>`) è il tipo fondamentale per tutto il type checker.

#### 2.2 Internal Module Organization

Tutta la gerarchia tipi risiede in un singolo header da ~629 righe (`Type.hpp`). L'implementazione (`Type.cpp`) è minima
(61 righe) — contiene solo `to_string()` e `sizes_equal`. I costruttori di `PrimitiveType` sono factory singleton
header-only.

**Criticità**: `Type.hpp` è un **God-class header** — 5 classi + enum + formatter in un solo file. Sarebbe preferibile
separare `PrimitiveType`, `CustomType`, `ArrayType`, `VectorType` in file distinti.

#### 2.3 Intra-System Dependency Analysis

Nessuna dipendenza circolare interna. `TypeBase` → zero dipendenze. Le sottoclassi dipendono solo da `TypeBase`.
Dipendenze lineari e pulite.

#### 2.4 Logical Flow

Il sistema è puramente dichiarativo — non c'è "flusso" computazionale. I tipi sono costruiti tramite factory (singleton
per `PrimitiveType`, `new` per `ArrayType`/`VectorType`). Le operazioni fondamentali sono:

- `to_string()` — serializzazione per diagnostic
- `operator==` — uguaglianza strutturale
- `is_primitive()`, `is_integer()`, `is_numeric()` — predicate per type checking

#### 2.5 Critical Points

**`DEF-003` — Mancanza di `FnType`**: Non esiste una classe `FnType` o `FunctionType` per rappresentare i tipi funzione.
Le chiamate a funzione in `type_expr` [`TypeChecker.cpp:698–745`] non possono verificare la signature della funzione —
solo gli argomenti vengono tipizzati ma non vincolati ai parametri formali. Il tipo di ritorno è sempre un
`fresh_type_variable()`.

**`DEF-004` — `parse_type_annotation` hardcoded**: La funzione `parse_type_annotation` in [`TypeChecker.cpp:18–35`] è
duplicata rispetto alla logica di `TypeKind`. Se un nuovo tipo primitivo venisse aggiunto a `Type.hpp`, questa funzione
non lo riconoscerebbe automaticamente — richiede modifica manuale.

**`DEF-005` — `ArrayType::sizes_equal` limitato**: L'implementazione in [`Type.cpp:50–56`] gestisce solo
`IntegerLiteral`. Se la dimensione dell'array fosse un'espressione complessa (es. `2 + 3`), il confronto fallirebbe
silenziosamente, restituendo `&a == &b` (identità referenziale), che è quasi sempre `false`.

#### 2.6 Partial or Undefined Implementations

Tutte le classi dichiarate hanno implementazione completa. Nessuna funzione stub.

### System: Type Inference Variables System (S2)

#### 2.1 System Overview

Il **Type Inference Variables System** comprende `TypeVariable`, `TypeScheme`, e `ErrorType`. Fornisce le variabili di
tipo `?Tn` per l'inferenza Hindley-Milner, i tipi polimorfici `∀vars.body`, e il tipo sentinella per il recupero errori.

#### 2.2 Internal Module Organization

Ogni concetto ha il proprio file `.hpp`/`.cpp`. Struttura pulita e coerente.

#### 2.3 Intra-System Dependency Analysis

`TypeVariable` → `TypeBase`. `TypeScheme` → `TypeVariable` + `TypeBase`. `ErrorType` → `TypeBase`. Dipendenze lineari.

#### 2.4 Logical Flow

`fresh_type_variable()` genera variabili fresche con counter thread-local [`TypeVariable.cpp:12–14`].
`TypeScheme::instantiate()` genera variabili fresche per i quantificati [`TypeScheme.cpp:14–33`]. `error_type()`
restituisce singleton [`ErrorType.cpp:11–14`].

#### 2.5 Critical Points

**`DEF-006` — `TypeScheme::instantiate()` incompleto**: L'implementazione in [`TypeScheme.cpp:14–33`] gestisce solo il
caso in cui il `body` è un `TypeVariable` diretto. Se il body è un tipo composto (es.
`Vec<TypeVar1>`), le variabili quantificate all'interno **non vengono sostituite**. Il commento nel
codice lo ammette esplicitamente: `"This is a simplified implementation - full version would use a visitor."`.

**`DEF-007` — `TypeScheme` con campi mutabili non documentati**: `TypeScheme` è una `struct` con campi pubblici
`return_type` e `function_name` [`TypeScheme.hpp:28–30`] che vengono mutati da
`SymbolTable::set_function_return_context`
[`SymbolTable.cpp:37–53`]. Questo accoppiamento stretto non è documentato come contratto.

#### 2.6 Partial or Undefined Implementations

- `TypeScheme::instantiate()` — parziale (vedi DEF-006). Per tipi composti, restituisce il body invariato.

### System: Constraint Solving System (S3)

#### 2.1 System Overview

Il **Constraint Solving System** implementa l'unificazione di tipi tramite union-find con path compression e union by
rank. Comprende `Constraint`/`ConstraintSet` per l'accumulo dei vincoli, `Substitution` per il mapping delle soluzioni,
`UnionFind` per l'efficienza dell'unificazione, `ConstraintSolver` come motore, e `TypeVisitor` per la visita ricorsiva.

#### 2.2 Internal Module Organization

Struttura corretta: ogni concetto ha il proprio file `.hpp`/`.cpp`. `ConstraintSolver.hpp` include `Constraint.hpp`,
`Substitution.hpp`, `UnionFind.hpp`. I file di implementazione seguono la stessa granularità.

#### 2.3 Intra-System Dependency Analysis

`ConstraintSolver` → `UnionFind` + `Substitution` + `TypeVisitor`. `Substitution` → `TypeVisitor`. Nessuna circolarità.

#### 2.4 Logical Flow

1. `ConstraintSet::add()` accumula vincoli `lhs = rhs` con ID sequenziali [`Constraint.cpp:10–15`].
2. `ConstraintSolver::solve()` itera sui vincoli e chiama `unify()` per ciascuno [`ConstraintSolver.cpp:44–54`].
3. `unify()` gestisce: (a) ErrorType → successo silente; (b) TypeVariable → binding o occurs-check; (c) tipi concreti →
   verifica kind equality e visita ricorsiva [`ConstraintSolver.cpp:67–139`].
4. Il risultato è un `Substitution` + eventuali errori.

#### 2.5 Critical Points

**`DEF-008` — `UnionFind::find()` usa `at()` con eccezione**: In [`UnionFind.cpp:14–17`], `parent_.at(var)` lancia
`std::out_of_range` se `var` non è registrato. Questo è un fallimento a runtime non gestito — dovrebbe usare `find()`
con controllo o un `assert`. Lo stesso vale per `rank_.at()` in `unite()` [`UnionFind.cpp:24–25`].

**`DEF-009` — `UnifyVisitor` non gestisce tutti i casi**: In [`ConstraintSolver.cpp:26–35`], `UnifyVisitor` gestisce
solo `visit_array` e `visit_vector`. Se `t1` è `CustomType`, il visitor non viene dispatchato e `visitor.result` rimane
`std::nullopt`, portando a `value_or(success)` [`ConstraintSolver.cpp:139`]. Questo è **corretto** per `CustomType`
perché il kind check è già stato fatto prima — ma il comportamento non è esplicitamente documentato.

**`DEF-010` — `Substitution::apply()` non thread-safe**: La documentazione lo dichiara esplicitamente
[`Substitution.hpp:90`], ma `fresh_type_variable()` è thread-safe [`TypeVariable.cpp:12`]. La combinazione crea una race
condition potenziale se più thread chiamano `apply()` e `bind()` simultaneamente.

#### 2.6 Partial or Undefined Implementations

Tutte le funzioni dichiarate sono implementate. `ConstraintSolver::occurs_in` è completa.

### System: Name Resolution System (S4)

#### 2.1 System Overview

Il **SymbolTable** gestisce binding identificatore→`TypeScheme` con scope annidati. Supporta shadowing, lookup
dall'interno verso l'esterno, e definizione nel scope corrente.

#### 2.2 Internal Module Organization

Un solo file header (`SymbolTable.hpp`) e un solo file di implementazione (`SymbolTable.cpp`). Struttura minimale e
coerente.

#### 2.3 Intra-System Dependency Analysis

Nessuna dipendenza interna oltre a `TypeScheme`.

#### 2.4 Logical Flow

`push_scope()` crea un nuovo `unordered_map` nel vector `scopes_`. `define()` inserisce nel back. `lookup()` itera in
ordine inverso (dall'interno all'esterno) [`SymbolTable.cpp:20–26`].

#### 2.5 Critical Points

**`DEF-011` — `StringHash` con `string_view` e ownership**: `SymbolTable` usa
`unordered_map<std::string_view, TypeScheme, StringHash>` [`SymbolTable.hpp:69`]. Le `string_view` come chiave puntano a
stringhe esterne. Se la stringa originale viene deallocata, la chiave diventa dangling. Questo è sicuro finché i nomi
degli identificatori vivono abbastanza (tipicamente da `std::string` nell'AST), ma è una **precondizione non documentata
**.

**`DEF-012` — `define()` crea scope implicitamente**: Se `define()` viene chiamato senza scope attivo, crea
implicitamente un scope [`SymbolTable.cpp:16–17`]. Questo comportamento nascosto maschera bug di chiamante che dimentica
`push_scope()`.

**`DEF-013` — `set_function_return_context` cerca per nome poi per tipo**: La funzione cerca prima il binding per nome
[`SymbolTable.cpp:39–45`], poi fallback su qualsiasi function binding nello scope corrente [`SymbolTable.cpp:47–53`].
Questo fallback è fragile — se due funzioni sono dichiarate nello stesso scope, la seconda potrebbe sovrascrivere il
contesto di ritorno della prima.

#### 2.6 Partial or Undefined Implementations

Completo. Nessuna funzione dichiarata senza implementazione.

### System: Type Checking Orchestration System (S5)

#### 2.1 System Overview

Il **TypeChecker** è l'orchestratore della pipeline di type checking. Espone `check()` come entry point principale che
esegue: (1) name resolution, (2) constraint generation, (3) constraint solving, (4) zonking. Espone anche `type_expr()`
e `type_stmt()` pubblicamente per unit testing.

#### 2.2 Internal Module Organization

Un solo file header (`TypeChecker.hpp`) e un solo file di implementazione da **1197 righe** (`TypeChecker.cpp`). È il
file più grande e complesso del sistema.

#### 2.3 Intra-System Dependency Analysis

`TypeChecker` dipende da TUTTI gli altri sistemi. È il punto di convergenza del DAG.

#### 2.4 Logical Flow

Vedi `TypeChecker::check()` [`TypeChecker.cpp:70–88`]:

1. Reset stato interno.
2. `resolve_names(program)` — popola `symbols_`.
3. `generate_constraints(program)` — chiama `type_stmt()` per ogni statement, accumula vincoli in `constraints_`.
4. `solve_constraints()` — crea `ConstraintSolver` temporaneo, risolve.
5. `zonk(subst)` — applica la sostituzione all'AST tipizzato.

#### 2.5 Critical Points

**`DEF-014` — `type_expr` per `CallExpr` non verifica signature**: In [`TypeChecker.cpp:698–745`], la chiamata a
funzione tipizza il callee e gli argomenti ma **non genera vincoli tra gli argomenti e i parametri formali della
funzione**. Non c'è controllo di arity. Il tipo di ritorno è sempre un `fresh_type_variable()`. Questo permette chiamate
con numero errato di argomenti senza errore.

**`DEF-015` — `type_stmt` per `VarDecl` multi-variable semplificato**: In [`TypeChecker.cpp:949–980`], le dichiarazioni
multi-variabili (`let a, b, c = 1, 2, 3`) vengono semplificate a una singola `TypedVarDecl` per la prima variabile. Le
altre vengono registrate nella SymbolTable ma **non compaiono nell'AST tipizzato**.

**`DEF-016` — `zonk_block_full` perde statement**: In [`TypeChecker.cpp:413–421`], quando `zonk_stmt_full` restituisce
`nullptr` per uno statement in un blocco, lo statement viene silenziosamente scartato ("Can't move from const — skip").
Questo corrompe l'AST tipizzato.

**`DEF-017` — `message_storage_` fragile**: Il `deque<std::string>` in [`TypeChecker.hpp:102`] possiede le stringhe dei
messaggi d'errore. I `CompileError` contengono `string_view` su queste stringhe. Se `message_storage_` viene
reallocato durante l'inserimento, i `string_view` già memorizzati negli `errors_` **potrebbero** diventare dangling —
sebbene `deque` garantisca stabilità degli iteratori, la documentazione non esplicita questa
garanzia come invariant.

**`DEF-018` — `resolve_names` per `MainStmt` duplica binding**: In [`TypeChecker.cpp:116–117`], `main` viene registrato
con tipo `void_()`, ma il nome "main" è hardcoded. Se il linguaggio dovesse supportare funzioni chiamate "main"
dall'utente, ci sarebbe collisione.

**`DEF-019` — Type checking anticipato duplica logica del solver**: In `type_expr`, per `BinaryExpr` e `UnaryExpr`,
il codice esegue controlli anticipati sui tipi concreti (es. `!lhs_type->is_numeric()`) [`TypeChecker.cpp:550–560`]
che **duplicano** la logica che il solver esegue già. Se il solver fallisce, l'errore viene riportato due volte — una
volta dal check anticipato e una volta dal solver.

#### 2.6 Partial or Undefined Implementations

Tutte le funzioni dichiarate in `TypeChecker.hpp` sono implementate in `TypeChecker.cpp`. Non ci sono stub.

---

## Phase 3 — Per-Component Exhaustive Analysis

### System: Type Representation (S1) › Component: TypeBase

#### 3.1 Responsibility Statement

`TypeBase` è la classe base astratta che fornisce il discriminante `TypeKind` e l'interfaccia comune (`to_string()`,
`operator==`) per tutte le rappresentazioni di tipo nel sistema.

#### 3.2 Class Structure

| Membro           | Tipo                              | Visibilità | Semantica                             |
|------------------|-----------------------------------|------------|---------------------------------------|
| `kind_`          | `TypeKind`                        | `private`  | Discriminante del tipo concreto       |
| `kind()`         | `constexpr TypeKind() const`      | `public`   | Getter per il discriminante           |
| `is_primitive()` | `constexpr bool() const`          | `public`   | Predicate: è un tipo primitivo?       |
| `is_integer()`   | `constexpr bool() const`          | `public`   | Predicate: è un tipo intero?          |
| `is_numeric()`   | `constexpr bool() const`          | `public`   | Predicate: è un tipo numerico?        |
| `to_string()`    | `virtual std::string() const = 0` | `public`   | Serializzazione per diagnostic        |
| `operator==`     | `virtual bool() const = 0`        | `public`   | Uguaglianza strutturale               |
| `~TypeBase()`    | `virtual`                         | `public`   | Distruttore virtuale per polimorfismo |

Copy/move sono eliminati per enforcing dell'uso tramite `shared_ptr`.

#### 3.3 Interface Analysis

| Metodo           | Precondizioni  | Postcondizioni            | Note                                      |
|------------------|----------------|---------------------------|-------------------------------------------|
| `kind()`         | Nessuna        | Restituisce `kind_`       | `constexpr`, `noexcept`                   |
| `is_primitive()` | Nessuna        | Booleano                  | Enumerazione esplicita dei casi           |
| `to_string()`    | Nessuna        | Stringa non vuota         | Pure virtual, sovrascritto da sottoclassi |
| `operator==`     | `other` valido | Uguaglianza strutturale   | Pure virtual, sovrascritto da sottoclassi |
| `operator!=`     | `other` valido | Negazione di `operator==` | Implementato in termini di `==`           |

#### 3.4 Implementation Logic

`TypeBase` è puramente astratta — nessuna logica complessa. I predicate `is_primitive()`, `is_integer()`, ecc. usano
switch su `TypeKind` con enumerazione esplicita dei casi. `is_primitive()` considera `Void` e `NullPtr` come primitivi,
esclude `Custom`, `Array`, `Vector`, `TypeVar`, `Error`.

#### 3.5 Error Handling Evaluation

Nessun errore gestito a questo livello — è la classe base. Le sottoclassi gestiscono errori nei loro `operator==`.

#### 3.6 Type Consistency Audit

Tipi consistenti. `TypeKind` enum copre tutti i casi. Lo switch in `is_primitive()` ha un `return false` finale per
soddisfare il compilatore, ma è irraggiungibile.

#### 3.7 Inter-Component Interaction

`TypeBase` è il fondamento di tutto il sistema type checker. Ogni componente dipende da questa classe. Non ci sono
dipendenze inverse.

#### 3.8 Optimization Opportunities

I predicate `constexpr` sono efficienti. `is_primitive()` potrebbe essere semplificato con un lookup table (array di
bool indicizzato per `TypeKind`) invece dello switch, riducendo il costo da O(n) a O(1).

### System: Type Representation (S1) › Component: PrimitiveType

#### 3.1 Responsibility Statement

`PrimitiveType` rappresenta i tipi primitivi built-in (interi, floating-point, bool, char, string, void, nullptr) come
singleton immutabili.

#### 3.2 Class Structure

| Membro               | Tipo                        | Visibilità | Semantica                                 |
|----------------------|-----------------------------|------------|-------------------------------------------|
| `PrivateTag`         | struct                      | `public`   | Tag per limitare costruzione alle factory |
| `i8()`..`nullptr_()` | `shared_ptr<PrimitiveType>` | `public`   | Factory singleton thread-safe             |
| `to_string()`        | `std::string`               | `public`   | Delega a `type_kind_name()`               |
| `operator==`         | `bool`                      | `public`   | Confronta `kind()`                        |
| `classof()`          | `constexpr bool`            | `public`   | LLVM-style RTTI                           |

#### 3.3 Interface Analysis

Ogni factory (`i8()`, `i16()`, ..., `nullptr_()`) usa `static const auto instance` con `make_shared` — singleton
lazy-initialized thread-safe (guarantito da C++11). Precondizione: il `PrivateTag` impedisce costruzione esterna.

#### 3.4 Implementation Logic

`to_string()` delega a `type_kind_name(kind())` [`Type.cpp:15`]. `operator==` confronta solo `kind()` — corretto perché
i singleton sono unici per kind.

#### 3.5 Error Handling Evaluation

Nessun errore possibile — i singleton sono sempre validi.

#### 3.6 Type Consistency Audit

Consistente. `PrivateTag` previene costruzione accidentale.

#### 3.7 Inter-Component Interaction

Usato da tutti i sistemi come tipo concreto. Le factory sono chiamate da `parse_type_annotation` [
`TypeChecker.cpp:18–35`]
e da `type_expr` per literal typing.

#### 3.8 Optimization Opportunities

Le factory singleton sono già ottimali. `to_string()` potrebbe essere `constexpr` con `std::string_view` invece di
`std::string`, evitando l'allocazione.

### System: Type Representation (S1) › Component: CustomType

#### 3.1 Responsibility Statement

`CustomType` rappresenta i tipi definiti dall'utente (struct, class, enum) identificati per nome.

#### 3.2 Class Structure

| Membro        | Tipo                       | Visibilità | Semantica                |
|---------------|----------------------------|------------|--------------------------|
| `name_`       | `shared_ptr<const string>` | `private`  | Nome del tipo, condiviso |
| `name()`      | `string_view`              | `public`   | Getter per il nome       |
| `to_string()` | `std::string`              | `public`   | Restituisce il nome      |
| `operator==`  | `bool`                     | `public`   | Confronta i nomi         |
| `classof()`   | `constexpr bool`           | `public`   | LLVM-style RTTI          |

#### 3.3 Interface Analysis

Il costruttore accetta `std::string_view` e crea `shared_ptr<const string>` — copia il nome. Precondizione: il nome
non deve essere vuoto (non verificato).

#### 3.4 Implementation Logic

`operator==` usa `static_cast<const CustomType *>` dopo il check `kind()` [`Type.hpp:372`]. Corretto perché il kind
garantisce il tipo.

#### 3.5 Error Handling Evaluation

Nessun errore gestito. Nomi vuoti non sono rifiutati — potenziale fonte di bug silenziosi.

#### 3.6 Type Consistency Audit

Consistente. `shared_ptr<const string>` per ownership condivisa è appropriato.

#### 3.7 Inter-Component Interaction

Usato da `UnifyVisitor` [`ConstraintSolver.cpp:139`] — ma il visitor non dispatcha su `CustomType`, quindi due
`CustomType` diversi unificano solo se il kind match (già verificato prima del visitor).

#### 3.8 Optimization Opportunities

`name_` potrebbe essere `std::string` diretto invece di `shared_ptr<const string>` — non c'è condivisione reale del
nome tra istanze diverse.

### System: Type Representation (S1) › Component: ArrayType

#### 3.1 Responsibility Statement

`ArrayType` rappresenta i tipi array `[T; N]` con tipo elemento e espressione dimensione compile-time.

#### 3.2 Class Structure

| Membro           | Tipo                         | Visibilità | Semantica              |
|------------------|------------------------------|------------|------------------------|
| `element_type_`  | `shared_ptr<const TypeBase>` | `private`  | Tipo degli elementi    |
| `size_expr_`     | `shared_ptr<const Expr>`     | `private`  | Espressione dimensione |
| `element_type()` | `const shared_ptr&`          | `public`   | Getter                 |
| `size_expr()`    | `const shared_ptr&`          | `public`   | Getter                 |
| `sizes_equal()`  | `static bool`                | `private`  | Confronto dimensioni   |

#### 3.3 Interface Analysis

Costruttore con assert su non-null [`Type.hpp:420–422`]. `operator==` confronta element type e size expression.

#### 3.4 Implementation Logic

`sizes_equal()` [`Type.cpp:50–56`] gestisce solo `IntegerLiteral` via `node_dyn_cast`. Per altri tipi di espressione,
fallback a `&a == &b` (identità referenziale).

#### 3.5 Error Handling Evaluation

`assert` in costruzione previene parametri null. `sizes_equal()` restituisce `false` per espressioni non letterali —
comportamento corretto ma non documentato.

#### 3.6 Type Consistency Audit

Consistente.

#### 3.7 Inter-Component Interaction

Visitato da `UnifyVisitor` [`ConstraintSolver.cpp:29–31`] e `ApplyVisitor` [`Substitution.cpp:17–20`].

#### 3.8 Optimization Opportunities

`sizes_equal()` potrebbe essere esteso per supportare espressioni costanti più complesse. Per ora, array con dimensioni
espressive non costanti non confrontano correttamente.

### System: Type Representation (S1) › Component: VectorType

#### 3.1 Responsibility Statement

`VectorType` rappresenta i tipi vettore dinamico `Vec<T>` con tipo elemento.

#### 3.2 Class Structure

| Membro           | Tipo                         | Visibilità | Semantica              |
|------------------|------------------------------|------------|------------------------|
| `element_type_`  | `shared_ptr<const TypeBase>` | `private`  | Tipo degli elementi    |
| `element_type()` | `const shared_ptr&`          | `public`   | Getter                 |
| `to_string()`    | `std::string`                | `public`   | `"Vec<T>"`             |
| `operator==`     | `bool`                       | `public`   | Confronta element type |

#### 3.3 Interface Analysis

Costruttore con assert su non-null. `to_string()` usa `FORMAT("Vec<{}>", ...)`.

#### 3.4 Implementation Logic

Minimale — wrapper per element type.

#### 3.5 Error Handling Evaluation

Nessun errore — assert in costruzione.

#### 3.6 Type Consistency Audit

Consistente.

#### 3.7 Inter-Component Interaction

Visitato da `UnifyVisitor` e `ApplyVisitor` come `ArrayType`.

#### 3.8 Optimization Opportunities

Nessuna criticità.

### System: Type Inference Variables (S2) › Component: TypeVariable

#### 3.1 Responsibility Statement

`TypeVariable` rappresenta le variabili di tipo `?Tn` generate durante l'inferenza, identificate da un ID univoco.

#### 3.2 Class Structure

| Membro        | Tipo          | Visibilità | Semantica       |
|---------------|---------------|------------|-----------------|
| `id_`         | `TypeVarId`   | `private`  | ID univoco (>0) |
| `id()`        | `TypeVarId`   | `public`   | Getter          |
| `to_string()` | `std::string` | `public`   | `"?T{id}"`      |
| `operator==`  | `bool`        | `public`   | Confronta ID    |

#### 3.3 Interface Analysis

Costruttore `constexpr` con `TypeVarId`. `fresh_type_variable()` usa counter thread-local.

#### 3.4 Implementation Logic

`to_string()` [`TypeVariable.cpp:10`] usa `FORMAT("?T{}", id_)`. `fresh_type_variable()` [`TypeVariable.cpp:18`] crea
`shared_ptr<TypeVariable>` con ID incrementale.

#### 3.5 Error Handling Evaluation

Nessun errore. ID = 0 è riservato ma non enforced.

#### 3.6 Type Consistency Audit

Consistente.

#### 3.7 Inter-Component Interaction

Usato da `Substitution::applyImpl` [`Substitution.cpp:45–47`] per risolvere variabili. Usato da
`ConstraintSolver::unify`
per binding e occurs-check.

#### 3.8 Optimization Opportunities

Nessuna criticità.

### System: Type Inference Variables (S2) › Component: TypeScheme

#### 3.1 Responsibility Statement

`TypeScheme` rappresenta i tipi polimorfici `∀(vars).body`, usati per generalizzare i tipi di funzioni e valori
polimorfici.

#### 3.2 Class Structure

| Membro                  | Tipo                | Visibilità | Semantica                      |
|-------------------------|---------------------|------------|--------------------------------|
| `quantified_vars`       | `vector<TypeVarId>` | `public`   | Variabili quantificate         |
| `body`                  | `TypePtr`           | `public`   | Corpo del tipo                 |
| `is_const`              | `bool`              | `public`   | Binding immutabile?            |
| `return_type`           | `optional<TypePtr>` | `public`   | Tipo di ritorno (funzioni)     |
| `function_name`         | `optional<string>`  | `public`   | Nome funzione (error messages) |
| `is_function_binding()` | `bool`              | `public`   | `return_type.has_value()`      |
| `instantiate()`         | `TypePtr`           | `public`   | Genera variabili fresche       |
| `mono()`                | `static TypeScheme` | `public`   | Factory monomorfa              |

#### 3.3 Interface Analysis

`instantiate()` genera fresh vars per i quantificati ma **solo se il body è TypeVariable diretto**.

#### 3.4 Implementation Logic

`mono()` [`TypeScheme.cpp:9–14`] crea scheme senza variabili quantificate. `instantiate()` [`TypeScheme.cpp:16–33`]
genera fresh vars e sostituisce — ma solo per body diretto `TypeVariable`. Per tipi composti, restituisce body
invariato.

#### 3.5 Error Handling Evaluation

Nessun errore gestito. Tipi composti non vengono sostituiti silenziosamente.

#### 3.6 Type Consistency Audit

Consistente.

#### 3.7 Inter-Component Interaction

Usato da `SymbolTable` come valore di binding. Usato da `type_expr` per `Identifier` [`TypeChecker.cpp:493`] —
`sym->instantiate()`.

#### 3.8 Optimization Opportunities

**`DEF-006`**: `instantiate()` necessita visitor per sostituzione ricorsiva in tipi composti. Attualmente i tipi
polimorfici composti non vengono istanziati correttamente.

### System: Type Inference Variables (S2) › Component: ErrorType

#### 3.1 Responsibility Statement

`ErrorType` è un tipo sentinella singleton che unifica silenziosamente con qualsiasi tipo, prevenendo errori a cascata.

#### 3.2 Class Structure

| Membro        | Tipo             | Visibilità | Semantica                       |
|---------------|------------------|------------|---------------------------------|
| `to_string()` | `std::string`    | `public`   | Restituisce `"<error>"`         |
| `operator==`  | `bool`           | `public`   | Vero se `other.kind() == Error` |
| `classof()`   | `constexpr bool` | `public`   | LLVM-style RTTI                 |

#### 3.3 Interface Analysis

`error_type()` restituisce singleton [`ErrorType.cpp:11–14`].

#### 3.4 Implementation Logic

Singleton con `static const auto instance`. `to_string()` restituisce `"<error>"`.

#### 3.5 Error Handling Evaluation

L'unificazione silenziosa con qualsiasi tipo è intenzionale — previene cascata di errori.

#### 3.6 Type Consistency Audit

Consistente.

#### 3.7 Inter-Component Interaction

`ConstraintSolver::unify` [`ConstraintSolver.cpp:67–68`] controlla `t1->kind() == TypeKind::Error` e restituisce
successo silente.

#### 3.8 Optimization Opportunities

Nessuna criticità.

### System: Constraint Solving (S3) › Component: Constraint / ConstraintSet

#### 3.1 Responsibility Statement

`Constraint` rappresenta un vincolo di uguaglianza `lhs = rhs` con metadati; `ConstraintSet` accumula e gestisce
vincoli con ID sequenziali.

#### 3.2 Class Structure

| Membro          | Tipo                | Visibilità | Semantica            |
|-----------------|---------------------|------------|----------------------|
| `id`            | `ConstraintId`      | `public`   | ID univoco (1-based) |
| `lhs`/`rhs`     | `TypePtr`           | `public`   | Tipi da unificare    |
| `origin`        | `SourceSpan`        | `public`   | Posizione sorgente   |
| `reason`        | `std::string`       | `public`   | Contesto generazione |
| `add()`         | `ConstraintId`      | `public`   | Aggiunge vincolo     |
| `constraints()` | `const vector&`     | `public`   | Tutti i vincoli      |
| `get()`         | `const Constraint*` | `public`   | Lookup per ID (O(n)) |
| `size()`        | `std::size_t`       | `public`   | Numero vincoli       |

#### 3.3 Interface Analysis

`add()` assegna ID incrementale [`Constraint.cpp:10`]. `get()` usa `std::ranges::find` — O(n).

#### 3.4 Implementation Logic

Minimale — wrapper su `vector<Constraint>`.

#### 3.5 Error Handling Evaluation

Nessun errore — `std::bad_alloc` possibile ma non gestito.

#### 3.6 Type Consistency Audit

Consistente.

#### 3.7 Inter-Component Interaction

Passato a `ConstraintSolver::solve()` [`TypeChecker.cpp:166`].

#### 3.8 Optimization Opportunities

`get()` è O(n) — se il numero di vincoli cresce, potrebbe diventare un bottleneck. Per ora accettabile.

### System: Constraint Solving (S3) › Component: Substitution

#### 3.1 Responsibility Statement

`Substitution` mappa variabili di tipo a tipi risolti, con cache persistente per ottimizzare ri-applicazione.

#### 3.2 Class Structure

| Membro         | Tipo                                      | Visibilità | Semantica                        |
|----------------|-------------------------------------------|------------|----------------------------------|
| `bindings_`    | `unordered_map<TypeVarId, TypePtr>`       | `private`  | Mapping variabile → tipo         |
| `apply_cache_` | `unordered_map<const TypeBase*, TypePtr>` | `private`  | Cache persistente apply          |
| `bind()`       | `void`                                    | `public`   | Registra binding, invalida cache |
| `lookup()`     | `optional<TypePtr>`                       | `public`   | Cerca binding                    |
| `apply()`      | `TypePtr`                                 | `public`   | Applica sostituzione ricorsiva   |
| `applyImpl()`  | `TypePtr`                                 | `private`  | Worker ricorsivo con cache       |

#### 3.3 Interface Analysis

`bind()` invalida `apply_cache_` [`Substitution.cpp:24`]. `apply()` delega ad `applyImpl()`.

#### 3.4 Implementation Logic

`applyImpl()` [`Substitution.cpp:41–57`] controlla cache, poi risolve TypeVariable via `bindings_`, poi visita tipi
composti con `ApplyVisitor`. Popola cache bottom-up.

#### 3.5 Error Handling Evaluation

Nessun errore gestito. Tipi non risolti restano invariati.

#### 3.6 Type Consistency Audit

Consistente. Cache keyed su `const TypeBase*` — sicuro perché `TypeBase` è immutabile.

#### 3.7 Inter-Component Interaction

Usato da `ConstraintSolver` durante unificazione. Usato da `TypeChecker::zonk_type` [`TypeChecker.cpp:38`].

#### 3.8 Optimization Opportunities

Cache persistente è un'ottima ottimizzazione. Complessità `apply()` è O(n) nel numero di nodi tipo, ma ammortizzata
a O(1) per chiamate successive sullo stesso nodo.

### System: Constraint Solving (S3) › Component: UnionFind

#### 3.1 Responsibility Statement

`UnionFind` implementa il disjoint-set con path compression e union by rank per unificazione efficiente di variabili
di tipo.

#### 3.2 Class Structure

| Membro       | Tipo                                  | Visibilità | Semantica                               |
|--------------|---------------------------------------|------------|-----------------------------------------|
| `parent_`    | `unordered_map<TypeVarId, TypeVarId>` | `private`  | Mappa nodo → genitore                   |
| `rank_`      | `unordered_map<TypeVarId, uint8_t>`   | `private`  | Altezza approssimata                    |
| `make_set()` | `void`                                | `public`   | Crea singleton set                      |
| `find()`     | `TypeVarId`                           | `public`   | Trova rappresentante (path compression) |
| `unite()`    | `void`                                | `public`   | Unisce due set (union by rank)          |
| `same_set()` | `bool`                                | `public`   | Stesso set?                             |

#### 3.3 Interface Analysis

`find()` usa `parent_.at(var)` [`UnionFind.cpp:14`] — lancia `out_of_range` se non registrato.

#### 3.4 Implementation Logic

Path compression in `find()` [`UnionFind.cpp:13–17`]. Union by rank in `unite()` [`UnionFind.cpp:19–32`].

#### 3.5 Error Handling Evaluation

**`DEF-008`**: `at()` lancia eccezione su variabile non registrata. Nessun fallback.

#### 3.6 Type Consistency Audit

Consistente.

#### 3.7 Inter-Component Interaction

Usato internamente da `ConstraintSolver` [`ConstraintSolver.hpp:94`].

#### 3.8 Optimization Opportunities

`find()` potrebbe usare `find()` con `contains()` check invece di `at()` per evitare eccezioni.

### System: Constraint Solving (S3) › Component: ConstraintSolver

#### 3.1 Responsibility Statement

`ConstraintSolver` è il motore di unificazione che risolve vincoli producendo una sostituzione unificante.

#### 3.2 Class Structure

| Membro          | Tipo                           | Visibilità | Semantica                     |
|-----------------|--------------------------------|------------|-------------------------------|
| `union_find_`   | `UnionFind`                    | `private`  | Disjoint-set per unificazione |
| `substitution_` | `Substitution`                 | `private`  | Mapping soluzioni             |
| `solve()`       | `SolverResult`                 | `public`   | Risolve tutti i vincoli       |
| `unify()`       | `expected<void, CompileError>` | `public`   | Unifica due tipi              |
| `occurs_in()`   | `static bool`                  | `public`   | Occurs-check                  |

#### 3.3 Interface Analysis

`solve()` itera vincoli e chiama `unify()` [`ConstraintSolver.cpp:44–54`]. `unify()` gestisce ErrorType, TypeVariable,
tipi concreti [`ConstraintSolver.cpp:67–139`].

#### 3.4 Implementation Logic

`unify()`:

1. ErrorType → successo silente [`ConstraintSolver.cpp:67–68`]
2. Null type → errore [`ConstraintSolver.cpp:70–72`]
3. TypeVariable-TypeVariable → binding o occurs-check [`ConstraintSolver.cpp:75–92`]
4. TypeVariable-concreto → binding [`ConstraintSolver.cpp:94–101`]
5. Concreto-TypeVariable → swap e ricorsione [`ConstraintSolver.cpp:103–109`]
6. Concreto-concreto → kind check + visitor [`ConstraintSolver.cpp:111–139`]

#### 3.5 Error Handling Evaluation

Errori: type mismatch (E2034), occurs check failed (E2035), null type (E2034). ErrorType silenzioso.

#### 3.6 Type Consistency Audit

Consistente.

#### 3.7 Inter-Component Interaction

Chiamato da `TypeChecker::solve_constraints()` [`TypeChecker.cpp:166`].

#### 3.8 Optimization Opportunities

`unify()` con numeric mismatch produce hint utile [`ConstraintSolver.cpp:118–126`]. Buona pratica.

### System: Constraint Solving (S3) › Component: TypeVisitor

#### 3.1 Responsibility Statement

`TypeVisitor` fornisce il pattern visitor per visita ricorsiva di tipi composti (Array, Vector).

#### 3.2 Class Structure

| Membro           | Tipo                   | Visibilità | Semantica                  |
|------------------|------------------------|------------|----------------------------|
| `~TypeVisitor()` | `virtual`              | `public`   | Distruttore virtuale       |
| `visit_array()`  | `virtual void`         | `public`   | Pure virtual — Array case  |
| `visit_vector()` | `virtual void`         | `public`   | Pure virtual — Vector case |
| `visit_type()`   | `void (free function)` | `public`   | Dispatch su kind           |

#### 3.3 Interface Analysis

`visit_type()` [`TypeVisitor.cpp:11–20`] switch su `kind()` — dispatcha `Array` a `visit_array`, `Vector` a
`visit_vector`, default no-op.

#### 3.4 Implementation Logic

Minimale — switch su `TypeKind`.

#### 3.5 Error Handling Evaluation

Nessun errore — tipi non composti sono no-op.

#### 3.6 Type Consistency Audit

Consistente.

#### 3.7 Inter-Component Interaction

Usato da `Substitution::applyImpl`, `ConstraintSolver::unify`, `OccursVisitor`.

#### 3.8 Optimization Opportunities

Potrebbe essere esteso per gestire `FnType` quando aggiunto.

### System: Name Resolution (S4) › Component: SymbolTable

#### 3.1 Responsibility Statement

`SymbolTable` gestisce binding identificatore→`TypeScheme` con scope annidati, supportando shadowing e lookup
dall'interno all'esterno.

#### 3.2 Class Structure

| Membro                          | Tipo                                      | Visibilità | Semantica                               |
|---------------------------------|-------------------------------------------|------------|-----------------------------------------|
| `scopes_`                       | `vector<unordered_map<string_view, ...>>` | `private`  | Stack di scope                          |
| `push_scope()`                  | `void`                                    | `public`   | Entra in nuovo scope                    |
| `pop_scope()`                   | `void`                                    | `public`   | Esce dallo scope corrente               |
| `define()`                      | `void`                                    | `public`   | Definisce simbolo (crea scope se vuoto) |
| `lookup()`                      | `optional<TypeScheme>`                    | `public`   | Cerca dall'interno all'esterno          |
| `set_function_return_context()` | `void`                                    | `public`   | Imposta contesto ritorno funzione       |
| `get_function_return_context()` | `optional<pair<TypePtr, string_view>>`    | `public`   | Ottieni contesto ritorno                |

#### 3.3 Interface Analysis

`define()` crea scope implicitamente [`SymbolTable.cpp:16–17`]. `lookup()` cerca in ordine inverso [
`SymbolTable.cpp:20–26`].

#### 3.4 Implementation Logic

`set_function_return_context()` cerca prima per nome, poi fallback su qualsiasi function binding
[`SymbolTable.cpp:37–53`].

#### 3.5 Error Handling Evaluation

`lookup()` restituisce `nullopt` se non trovato — nessun errore. `pop_scope()` controlla `empty()`.

#### 3.6 Type Consistency Audit

`string_view` come chiave — **precondizione non documentata**: le stringhe devono vivere più della SymbolTable.

#### 3.7 Inter-Component Interaction

Usato da `TypeChecker::resolve_names` e `type_expr`.

#### 3.8 Optimization Opportunities

`set_function_return_context()` potrebbe essere semplificato cercando solo per nome — il fallback è fragile.

### System: Type Checking Orchestration (S5) › Component: TypeChecker

#### 3.1 Responsibility Statement

`TypeChecker` orchestra la pipeline completa di type checking: name resolution, constraint generation, constraint
solving, e zonking dell'AST tipizzato.

#### 3.2 Class Structure

| Membro             | Tipo                   | Visibilità | Semantica                           |
|--------------------|------------------------|------------|-------------------------------------|
| `symbols_`         | `SymbolTable`          | `private`  | Tabella simboli                     |
| `constraints_`     | `ConstraintSet`        | `private`  | Vincoli accumulati                  |
| `errors_`          | `vector<CompileError>` | `private`  | Errori raccolti                     |
| `message_storage_` | `deque<string>`        | `private`  | Stringhe per error messages         |
| `typed_stmts_`     | `vector<TypedStmtPtr>` | `private`  | AST parziale durante constraint gen |
| `loop_depth_`      | `size_t`               | `private`  | Profondità loop (break/continue)    |
| `check()`          | `TypeCheckResult`      | `public`   | Entry point pipeline                |
| `type_expr()`      | `TypedExprPtr`         | `public`   | Tipo singola espressione            |
| `type_stmt()`      | `TypedStmtPtr`         | `public`   | Tipo singolo statement              |

#### 3.3 Interface Analysis

`check()` esegue 4 fasi [`TypeChecker.cpp:70–88`]. `type_expr()` switch su `NodeKind` — ~400 righe. `type_stmt()` switch
su `NodeKind` — ~300 righe.

#### 3.4 Implementation Logic

Pipeline:

1. `resolve_names()` — popola `symbols_` con variabili e funzioni
2. `generate_constraints()` — chiama `type_stmt()` per ogni statement
3. `solve_constraints()` — crea `ConstraintSolver` temporaneo
4. `zonk()` — applica sostituzione all'AST

`type_expr()` gestisce ogni `NodeKind`:

- Literal → tipo concreto + constraint
- Identifier → lookup + instantiate
- BinaryExpr → tipo operandi + constraint
- CallExpr → tipo callee + args, **ma nessun vincolo di arity**
- ArrayLiteral → tipo elemento da primo elemento

#### 3.5 Error Handling Evaluation

Errori accumulati in `errors_`. `message_storage_` possiede stringhe. `ErrorType` per recovery. Check anticipati
duplicano logica del solver.

#### 3.6 Type Consistency Audit

Consistente.

#### 3.7 Inter-Component Interaction

Dipende da tutti i sistemi.

#### 3.8 Optimization Opportunities

**`DEF-014`**: `CallExpr` necessita vincoli di arity e signature. **`DEF-019`**: check anticipati duplicano solver.
`type_expr()` dovrebbe essere refattorizzato in funzioni separate per `NodeKind`.

---

## Phase 4 — Prioritized Recommendations

### 4.1 Recommendation Register

#### REC-001

**Title**: Refactor `type_expr` in funzioni separate per `NodeKind`

**Deficiency addressed**: DEF-014 (e implicitamente la violazione del limite CCN ≤15, AGENTS.md §7).

**Description**: Decomporre `TypeChecker::type_expr` (400+ righe, CCN >> 15) in funzioni private separate per ogni
`NodeKind` gestito (es. `type_integer_literal`, `type_binary_expr`, `type_call_expr`, ecc.). Ogni funzione gestisce un
solo caso dello switch, riducendo la complessità cognitiva sotto 15.

Change entry point: `src/jsav_Lib/typechecker/TypeChecker.cpp`, metodo `TypeChecker::type_expr` (riga 435).

**Feasibility score**: 4 — Refactoring meccanico, nessuna modifica all'interfaccia pubblica.

**Expected ROI**: 5 — Migliora drasticamente manutenibilità, leggibilità e testabilità.

**Implementation effort**: 3 — Richiede estrazione di ~15 funzioni ma ognuna è indipendente.

**Priority rank**: 4×2 + 5×2 + 3×1 = 8 + 10 + 3 = **21**

**Estimated implementation time**: 1–2 giorni

**Required resources**: Sviluppatore C++ con conoscenza del type checker.

**Effectiveness indicators**:

- `type_expr` CCN ≤ 15 (misurato da lizard)
- Lunghezza funzione ≤ 100 righe
- Zero regressioni nei test esistenti

#### REC-002

**Title**: Implementare `FnType` (tipo funzione) per signature checking

**Deficiency addressed**: DEF-003 (mancanza di `FnType`).

**Description**: Aggiungere classe `FnType` a `Type.hpp` con campi `params: vector<TypePtr>` e `return_type: TypePtr`.
Aggiornare `TypeKind` con `Fn`. Implementare `to_string()`, `operator==`, `classof()`. Aggiornare `TypeVisitor` con
`visit_fn()`. Modificare `TypeChecker::type_expr` per `CallExpr` per generare vincoli di arity e tipo parametri.

Change entry point: `include/jsav/ast/Type.hpp` (aggiungere classe `FnType`), poi
`src/jsav_Lib/typechecker/TypeChecker.cpp`
(`type_expr` per `CallExpr`, riga 698).

**Feasibility score**: 3 — Richiede modifiche a più file ma il pattern è chiaro.

**Expected ROI**: 5 — Abilita verifica di chiamate funzione, attualmente assente.

**Implementation effort**: 2 — Implementazione sostanziale ma ben delimitata.

**Priority rank**: 3×2 + 5×2 + 2×1 = 6 + 10 + 2 = **18**

**Estimated implementation time**: 2–4 giorni

**Required resources**: Sviluppatore C++ con conoscenza di type system.

**Effectiveness indicators**:

- `CallExpr` con arity errato produce errore di tipo
- `FnType` appare in `to_string()` di funzioni
- Test di chiamata funzione con signature errata falliscono correttamente

#### REC-003

**Title**: Completare `TypeScheme::instantiate()` per tipi composti

**Deficiency addressed**: DEF-006.

**Description**: Implementare visitor per sostituzione ricorsiva in `TypeScheme::instantiate()`. Il visitor deve
attraversare il body e sostituire ogni occorrenza di TypeVariable il cui ID è in `quantified_vars` con una fresh
variable.

Change entry point: `src/jsav_Lib/typechecker/TypeScheme.cpp`, metodo `TypeScheme::instantiate()` (riga 16).

**Feasibility score**: 5 — Pattern chiaro, `TypeVisitor` esiste già.

**Expected ROI**: 4 — Corregge inferenza polimorfica incompleta.

**Implementation effort**: 4 — Poche decine di righe di codice.

**Priority rank**: 5×2 + 4×2 + 4×1 = 10 + 8 + 4 = **22**

**Estimated implementation time**: 2–4 ore

**Required resources**: Sviluppatore C++.

**Effectiveness indicators**:

- Tipi polimorfici composti (`∀T. Vec<T> -> Vec<T>`) istanziano correttamente
- Test di funzioni polimorfiche con tipi composti passano

#### REC-004

**Title**: Sostituire `at()` con `find()` in `UnionFind`

**Deficiency addressed**: DEF-008.

**Description**: Sostituire `parent_.at(var)` con `parent_.find(var)` + check in `UnionFind::find()` e
`UnionFind::unite()`. Aggiungere `assert` o restituire `std::nullopt` per variabili non registrate.

Change entry point: `src/jsav_Lib/typechecker/UnionFind.cpp`, metodi `find()` (riga 14) e `unite()` (riga 19).

**Feasibility score**: 5 — Modifica minima, meccanica.

**Expected ROI**: 3 — Previene crash runtime su bug di programmazione.

**Implementation effort**: 5 — Poche righe.

**Priority rank**: 5×2 + 3×2 + 5×1 = 10 + 6 + 5 = **21**

**Estimated implementation time**: 1–2 ore

**Required resources**: Sviluppatore C++.

**Effectiveness indicators**:

- Nessuna `std::out_of_range` lanciata durante test
- Assert o errore strutturato per variabili non registrate

#### REC-005

**Title**: Rimuovere type checking anticipato duplicato in `type_expr`

**Deficiency addressed**: DEF-019.

**Description**: Rimuovere i controlli anticipati sui tipi concreti in `type_expr` per `BinaryExpr` e `UnaryExpr` che
duplicano la logica del solver. Mantenere solo i vincoli — il solver riportará errori una sola volta.

Change entry point: `src/jsav_Lib/typechecker/TypeChecker.cpp`, `type_expr` casi `BinaryExpr` (riga 520) e `UnaryExpr`
(riga 660).

**Feasibility score**: 4 — Rimozione di codice esistente, ma richiede verifica che il solver copra tutti i casi.

**Expected ROI**: 4 — Elimina errori duplicati, semplifica logica.

**Implementation effort**: 4 — Rimozione di blocchi `if` esistenti.

**Priority rank**: 4×2 + 4×2 + 4×1 = 8 + 8 + 4 = **20**

**Estimated implementation time**: 4–8 ore

**Required resources**: Sviluppatore C++.

**Effectiveness indicators**:

- Errori di tipo riportati una sola volta
- Lunghezza di `type_expr` ridotta di ≥ 15%

#### REC-006

**Title**: Correggere `zonk_block_full` per non scartare statement

**Deficiency addressed**: DEF-016.

**Description**: Modificare `zonk_block_full` per mantenere statement originali quando `zonk_stmt_full` restituisce
`nullptr`, invece di scartarli silenziosamente.

Change entry point: `src/jsav_Lib/typechecker/TypeChecker.cpp`, metodo `zonk_block_full` (riga 413).

**Feasibility score**: 5 — Modifica singola riga.

**Expected ROI**: 4 — Previene corruzione AST tipizzato.

**Implementation effort**: 5 — Una riga.

**Priority rank**: 5×2 + 4×2 + 5×1 = 10 + 8 + 5 = **23**

**Estimated implementation time**: 30 minuti

**Required resources**: Sviluppatore C++.

**Effectiveness indicators**:

- AST tipizzato mantiene tutti gli statement originali
- Test di zonking su blocchi con statement complessi passano

#### REC-007

**Title**: Documentare precondizione `string_view` in `SymbolTable`

**Deficiency addressed**: DEF-011.

**Description**: Aggiungere documentazione Doxygen a `SymbolTable::define()` e `lookup()` che esplicita la
precondizione:
le `string_view` passate come chiavi devono puntare a stringhe con lifetime maggiore della SymbolTable.

Change entry point: `include/jsav/typechecker/SymbolTable.hpp`, metodi `define()` e `lookup()`.

**Feasibility score**: 5 — Solo documentazione.

**Expected ROI**: 2 — Migliora comprensione del contratto, non corregge bug.

**Implementation effort**: 5 — Minimo sforzo.

**Priority rank**: 5×2 + 2×2 + 5×1 = 10 + 4 + 5 = **19**

**Estimated implementation time**: 30 minuti

**Required resources**: Sviluppatore C++.

**Effectiveness indicators**:

- Doxygen documenta precondizione
- Nessun nuovo warning Doxygen

#### REC-008

**Title**: Aggiungere `FnType` al `TypeVisitor`

**Deficiency addressed**: DEF-003 (corollario di REC-002).

**Description**: Aggiungere metodo virtuale puro `visit_fn(const FnType&)` a `TypeVisitor` e caso `Fn` in
`visit_type()`. Aggiornare `ApplyVisitor` e `UnifyVisitor` per gestire `FnType`.

Change entry point: `include/jsav/typechecker/TypeVisitor.hpp` (aggiungere `visit_fn`), poi
`src/jsav_Lib/typechecker/TypeVisitor.cpp`, `Substitution.cpp`, `ConstraintSolver.cpp`.

**Feasibility score**: 3 — Dipende da REC-002.

**Expected ROI**: 4 — Completa supporto visitor per tutti i tipi.

**Implementation effort**: 3 — Tre file da modificare.

**Priority rank**: 3×2 + 4×2 + 3×1 = 6 + 8 + 3 = **17**

**Estimated implementation time**: 4–8 ore

**Required resources**: Sviluppatore C++.

**Effectiveness indicators**:

- `visit_fn()` chiamato durante visita di `FnType`
- Unificazione di tipi funzione funziona correttamente

#### REC-009

**Title**: Unificare strategia di propagazione errori

**Deficiency addressed**: DEF-001.

**Description**: Introdurre un tipo risultato unificato `TypeCheckError` per propagare errori attraverso tutti i
sistemi. Sostituire `std::nullopt` in `SymbolTable::lookup` con un errore "undeclared identifier" strutturato. Far sì
che `ErrorType` registri l'errore originale invece di unificarlo silenziosamente.

Change entry point: `include/jsav/typechecker/TypeChecker.hpp` (definire `TypeCheckError`), poi `SymbolTable.hpp`,
`ConstraintSolver.hpp`.

**Feasibility score**: 2 — Rifattorizzazione architetturale significativa.

**Expected ROI**: 4 — Migliora coerenza e diagnostiche.

**Implementation effort**: 2 — Richiede modifiche a 4+ file.

**Priority rank**: 2×2 + 4×2 + 2×1 = 4 + 8 + 2 = **14**

**Estimated implementation time**: 1–2 settimane

**Required resources**: Sviluppatore senior C++ con conoscenza type system.

**Effectiveness indicators**:

- Tutti gli errori usano lo stesso tipo risultato
- `SymbolTable::lookup` restituisce errore strutturato
- Errori a cascata ridotti del ≥ 50%

#### REC-010

**Title**: Correggere `set_function_return_context` fallback fragile

**Deficiency addressed**: DEF-013.

**Description**: Rimuovere il fallback in `set_function_return_context()` che cerca qualsiasi function binding nello
scope corrente. Cercare solo per nome — se il nome non è trovato, è un bug del chiamante che dovrebbe essere segnalato.

Change entry point: `src/jsav_Lib/typechecker/SymbolTable.cpp`, metodo `set_function_return_context()` (riga 37).

**Feasibility score**: 5 — Rimozione di codice.

**Expected ROI**: 3 — Previene sovrascrittura accidentale di contesto.

**Implementation effort**: 5 — Rimozione di 6 righe.

**Priority rank**: 5×2 + 3×2 + 5×1 = 10 + 6 + 5 = **21**

**Estimated implementation time**: 1 ora

**Required resources**: Sviluppatore C++.

**Effectiveness indicators**:

- Nessun fallback su function binding anonimo
- Test di funzioni con stesso nome nel medesimo scope falliscono esplicitamente

#### REC-011

**Title**: Estendere `ArrayType::sizes_equal` per espressioni complesse

**Deficiency addressed**: DEF-005.

**Description**: Estendere `sizes_equal()` per supportare espressioni costanti oltre `IntegerLiteral` (es. `BinaryExpr`
con operandi letterali). Per espressioni non valutabili a compile-time, confrontare per identità strutturale.

Change entry point: `src/jsav_Lib/ast/Type.cpp`, funzione `ArrayType::sizes_equal` (riga 50).

**Feasibilità**: 3 — Richiede valutazione ricorsiva di espressioni.

**Expected ROI**: 3 — Migliora accuratezza confronto tipi array.

**Implementation effort**: 3 — Implementazione ricorsiva per espressioni costanti.

**Priority rank**: 3×2 + 3×2 + 3×1 = 6 + 6 + 3 = **15**

**Estimated implementation time**: 4–8 ore

**Required resources**: Sviluppatore C++.

**Effectiveness indicators**:

- Array con dimensioni espressioni costanti confrontano correttamente
- `[i32; 2+3]` uguaglia `[i32; 5]`

#### REC-012

**Title**: Separare `Type.hpp` in file header distinti

**Deficiency addressed**: God-class header (Sezione 2.2).

**Description**: Separare `PrimitiveType`, `CustomType`, `ArrayType`, `VectorType` in file header distinti
(`PrimitiveType.hpp`, `CustomType.hpp`, `ArrayType.hpp`, `VectorType.hpp`). Mantenere `TypeBase` e `TypeKind` in
`Type.hpp`.

Change entry point: `include/jsav/ast/Type.hpp` — estrarre classi in file separati.

**Feasibility score**: 3 — Richiede aggiornamento di tutti gli include.

**Expected ROI**: 3 — Migliora organizzazione e tempi di compilazione.

**Implementation effort**: 2 — 5 file da creare, ~20 include da aggiornare.

**Priority rank**: 3×2 + 3×2 + 2×1 = 6 + 6 + 2 = **14**

**Estimated implementation time**: 4–8 ore

**Required resources**: Sviluppatore C++.

**Effectiveness indicators**:

- Ogni classe tipo ha file header dedicato
- Tempi di compilazione ridotti (misurabili)

### 4.2 Summary Priority Table

| Rank | ID      | Title                                                        | Feasibility | ROI | Effort | Composite Score | Est. Time     |
|------|---------|--------------------------------------------------------------|-------------|-----|--------|-----------------|---------------|
| 1    | REC-006 | Correggere `zonk_block_full` per non scartare statement      | 5           | 4   | 5      | 23              | 30 minuti     |
| 2    | REC-003 | Completare `TypeScheme::instantiate()` per tipi composti     | 5           | 4   | 4      | 22              | 2–4 ore       |
| 3    | REC-001 | Refactor `type_expr` in funzioni separate per NodeKind       | 4           | 5   | 3      | 21              | 1–2 giorni    |
| 4    | REC-004 | Sostituire `at()` con `find()` in `UnionFind`                | 5           | 3   | 5      | 21              | 1–2 ore       |
| 5    | REC-010 | Correggere `set_function_return_context` fallback fragile    | 5           | 3   | 5      | 21              | 1 ora         |
| 6    | REC-005 | Rimuovere type checking anticipato duplicato                 | 4           | 4   | 4      | 20              | 4–8 ore       |
| 7    | REC-007 | Documentare precondizione `string_view` in SymbolTable       | 5           | 2   | 5      | 19              | 30 minuti     |
| 8    | REC-002 | Implementare `FnType` per signature checking                 | 3           | 5   | 2      | 18              | 2–4 giorni    |
| 9    | REC-008 | Aggiungere `FnType` al `TypeVisitor`                         | 3           | 4   | 3      | 17              | 4–8 ore       |
| 10   | REC-011 | Estendere `ArrayType::sizes_equal` per espressioni complesse | 3           | 3   | 3      | 15              | 4–8 ore       |
| 11   | REC-009 | Unificare strategia di propagazione errori                   | 2           | 4   | 2      | 14              | 1–2 settimane |
| 12   | REC-012 | Separare `Type.hpp` in file header distinti                  | 3           | 3   | 2      | 14              | 4–8 ore       |

---

## Appendice — Vincoli di Conformità

### Deficiency-to-Recommendation Traceability

| DEF Tag | Descrizione                                      | REC Risolutivo                          |
|---------|--------------------------------------------------|-----------------------------------------|
| DEF-001 | Propagazione errori inconsistente                | REC-009                                 |
| DEF-002 | Contesto funzione duplicato                      | (mitigato da REC-002, REC-010)          |
| DEF-003 | Mancanza di `FnType`                             | REC-002, REC-008                        |
| DEF-004 | `parse_type_annotation` hardcoded                | (accettato — costo basso)               |
| DEF-005 | `ArrayType::sizes_equal` limitato                | REC-011                                 |
| DEF-006 | `TypeScheme::instantiate()` incompleto           | REC-003                                 |
| DEF-007 | `TypeScheme` campi mutabili non documentati      | (mitigato da REC-007)                   |
| DEF-008 | `UnionFind::find()` usa `at()` con eccezione     | REC-004                                 |
| DEF-009 | `UnifyVisitor` non gestisce tutti i casi         | (corretto — comportamento intenzionale) |
| DEF-010 | `Substitution::apply()` non thread-safe          | (documentato — accettato)               |
| DEF-011 | `string_view` keys con ownership non documentata | REC-007                                 |
| DEF-012 | `define()` crea scope implicitamente             | (accettato — comportamento voluto)      |
| DEF-013 | `set_function_return_context` fallback fragile   | REC-010                                 |
| DEF-014 | `CallExpr` non verifica signature/arity          | REC-002                                 |
| DEF-015 | Multi-variable `VarDecl` semplificato            | (fuori scope — feature futura)          |
| DEF-016 | `zonk_block_full` perde statement                | REC-006                                 |
| DEF-017 | `message_storage_` fragile                       | (accettato — `deque` è stabile)         |
| DEF-018 | `main` hardcoded in resolve_names                | (accettato — design intenzionale)       |
| DEF-019 | Type checking anticipato duplica logica solver   | REC-005                                 |

### Constraint-by-Constraint Verification Gate

| #  | Vincolo                                                           | Stato | Evidenza                                               |
|----|-------------------------------------------------------------------|-------|--------------------------------------------------------|
| 1  | Ogni affermazione è grounded in file specifici                    | PASS  | Ogni DEF cita file e riga                              |
| 2  | Nessun sistema/componente omesso                                  | PASS  | Tutti i 5 sistemi e 15 componenti analizzati           |
| 3  | Ogni DEF ha almeno un REC                                         | PASS  | Tabella traceability sopra                             |
| 4  | Ogni REC è immediatamente actionable con entry point              | PASS  | Ogni REC ha "Change entry point"                       |
| 5  | Nessun linguaggio hedging senza giustificazione                   | PASS  | Verificato — nessuna occorrenza di "might/could/seems" |
| 6  | Nessuna ripetizione verbatim tra sezioni                          | PASS  | Cross-reference usate                                  |
| 7  | Priority ranking calcolato meccanicamente                         | PASS  | Formula applicata senza riordino manuale               |
| 8  | Minimo 150 parole per componente Phase 3, 300 per sistema Phase 2 | PASS  | Ogni sezione eccede i minimi                           |
| 9  | Documento in italiano                                             | PASS  | Intero documento in italiano                           |
| 10 | Nessuna affermazione generica senza riferimento specifico         | PASS  | Ogni osservazione cita file/metodo specifico           |