# Type Checker Implementation Audit

## Phase 1 — System Ensemble Analysis

### 1.1 Enumerazione dei Sistemi

Il type checker di **jsav** comprende **11 header** e **10 file di implementazione** nel modulo `typechecker`, più **1 header** e **1 implementazione** nel modulo `ast/Type`, organizzati in cinque sistemi principali:

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
| `include/jsav/typechecker/TypeChecker.hpp` | `src/jsav_Lib/typechecker/TypeChecker.cpp` | `TypeChecker` — pipeline completa (resolve → constraints → solve → zonk), 1238 righe |

**Sistemi dipendenti esterni** (non parte del type checker ma consumati):

| Sistema                | Responsabilità                                                      |
|------------------------|---------------------------------------------------------------------|
| `TypedNode.hpp` e soci | Nodi AST tipizzati (`TypedExpr`, `TypedStmt`, `TypedProgram`)       |
| `CompileError.hpp`     | Tipo errore strutturato con `ErrorCode`, `SourceSpan`               |
| `Expressions.hpp`      | Nodi espressione non tipizzati (`IntegerLiteral`, `CallExpr`, ecc.) |
| `Statements.hpp`       | Nodi statement non tipizzati (`VarDecl`, `FuncDecl`, ecc.)          |
| `error_codes.hpp`      | 36 codici errore E2001–E2036 per errori semantici/tipo              |

### 1.2 Mappa delle Dipendenze Inter-Sistema

```
                    ┌──────────────────────────────────────────────────────┐
                    │              TypeChecker (S5)                        │
                    │  check(): resolve → constraints → solve → zonk       │
                    │  type_expr(): switch su NodeKind, ~430 righe         │
                    │  type_stmt(): switch su NodeKind, ~300 righe         │
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

**Classificazione upstream → downstream**:

| Sistema                           | Posizione         | Descrizione                                                                |
|-----------------------------------|-------------------|----------------------------------------------------------------------------|
| **Type Representation (S1)**      | Upstream          | Fornisce `TypePtr` — il tipo fondamentale usato da tutti gli altri sistemi |
| **Type Inference Variables (S2)** | Midstream         | Variabili di tipo e schemi polimorfici, consumati da S3, S4, S5            |
| **Constraint Solving (S3)**       | Midstream         | Unificazione e sostituzione, consumata da S5 nella fase 3 e 4              |
| **Name Resolution (S4)**          | Upstream          | Symbol table, consumata da S5 nella fase 1 e 2                             |
| **Orchestration (S5)**            | Downstream (sink) | Dipende da TUTTI i sistemi sopra — è il punto di convergenza del DAG       |

Non esistono dipendenze circolari. Il grafo è un **DAG** (Directed Acyclic Graph) pulito.

**Nodi critici**:

- **Alto fan-in**: `TypeBase` (S1) — tutti i sistemi dipendono da essa. Single point of failure concettuale, ma mitigato dall'essere una gerarchia immutabile.
- **Alto fan-out**: `TypeChecker` (S5) — dipende da 10+ componenti. Segnale di accoppiamento eccessivo.

### 1.3 Valutazione della Coerenza Architetturale

L'architettura segue un approccio **constraint-based type inference** con pipeline in quattro fasi documentate in `TypeChecker.hpp:47–55`. La decomposizione è fondamentalmente solida ma presenta asimmetrie significative.

**Punti di forza**:

- **Separazione netta** tra rappresentazione (`TypeBase`), inferenza (`ConstraintSolver`/`UnionFind`), e orchestrazione (`TypeChecker`).
- **Visitor pattern** appropriato (`TypeVisitor`) per visita di tipi composti [`TypeVisitor.hpp:38–64`].
- **`Substitution` con cache persistente** [`Substitution.hpp:67–86`, `applyImpl`] per ottimizzare ri-applicazione — O(1) dopo il primo attraversamento.
- **`ErrorType` come sentinella** che unifica silenziosamente con qualsiasi tipo [`ConstraintSolver.cpp:67–68`], prevenendo errori a cascata.
- **Thread-safety documentata** in `fresh_type_variable()` [`TypeVariable.cpp:12`] con counter thread-local.

**Deficienze strutturali**:

- **`TypeChecker::type_expr`** (~430 righe effettive nel corpo dello switch, su 1238 totali del file) viola il principio di singola responsabilità e il limite di complessità cognitiva del progetto (CCN ≤15, AGENTS.md §7). Gestisce generazione vincoli, type checking anticipato, e costruzione AST tipizzato simultaneamente.
- **Mancanza di tipo funzione** (`FnType`). Il sistema non ha una rappresentazione esplicita per i tipi funzione (`(T1, T2) -> R`). Le chiamate a funzione (`CallExpr`) ora verificano l'arity e i parametri [`TypeChecker.cpp:716–745`], ma il tipo del callee resta una variabile fresca non raffinata.
- **Zonking incompleto**: il metodo `zonk_block_full` scarta statement che non producono risultato [`TypeChecker.cpp:417` — commento "Can't move from const — skip"], causando perdita silente di nodi AST.

### 1.4 Valutazione dei Concern Trasversali

**Matrice dei concern trasversali**:

| Concern                      | Type Rep. (S1)                         | Constraint Solving (S3)             | SymbolTable (S4)                 | TypeChecker (S5)                                  | Uniformità                      |
|------------------------------|----------------------------------------|-------------------------------------|----------------------------------|---------------------------------------------------|---------------------------------|
| **Propagazione errori**      | `ErrorType` singleton                  | `std::expected<void, CompileError>` | `std::nullopt` (silenzioso)      | `std::vector<CompileError>`                       | **INCONSISTENTE** — 4 strategie |
| **Rappresentazione tipi**    | `TypePtr = shared_ptr<const TypeBase>` | `TypePtr`                           | `TypeScheme` (wrappa `TypePtr`)  | `TypePtr`                                         | **UNIFORME**                    |
| **Gestione scope**           | —                                      | —                                   | `vector<unordered_map>` push/pop | Usa SymbolTable + `current_function_return_type_` | **PARZIALE**                    |
| **Formattazione diagnostic** | `to_string()` virtuale                 | `reason` string nei vincoli         | —                                | `message_storage_` con `FORMAT()`                 | **PARZIALE**                    |

**`DEF-001` — Propagazione errori inconsistente**: `ErrorType` unifica silenziosamente [`ConstraintSolver.cpp:67–68`], ma il TypeChecker accumula errori in `vector<CompileError>` [`TypeChecker.hpp:101`] mentre `SymbolTable::lookup` restituisce `std::nullopt` senza diagnostic [`SymbolTable.hpp:54–55`]. Quando un identificatore non è dichiarato, `type_expr` crea l'errore manualmente [`TypeChecker.cpp:511–516`]. Ogni sistema ha la propria strategia — nessun meccanismo unificato di error propagation.

**`DEF-002` — Contesto funzione duplicato**: `SymbolTable` mantiene `return_type` e `function_name` dentro `TypeScheme` [`TypeScheme.hpp:28–30`], gestiti da `set_function_return_context()` [`SymbolTable.cpp:37–53`]. Tuttavia `TypeChecker` non usa questo contesto in modo sistematico — il ritorno viene passato due volte: una volta nel binding e una volta come vincolo [`TypeChecker.cpp:1014`].

**`DEF-003` — Mancanza di `FnType`**: Non esiste una classe `FnType` o `FunctionType` per rappresentare i tipi funzione. Le chiamate a funzione in `type_expr` [`TypeChecker.cpp:698–764`] ora verificano l'arity e generano vincoli sui parametri, ma il tipo del callee resta una variabile fresca non collegata alla signature. Il tipo di ritorno viene recuperato dalla `FuncDecl` ma non vincolato al callee type.

---

## Phase 2 — Per-System Analysis

### System: Type Representation System (S1)

#### 2.1 System Overview

Il **Type Representation System** [`include/jsav/ast/Type.hpp`, `src/jsav_Lib/ast/Type.cpp`] definisce la gerarchia di classi che rappresentano tutti i tipi del linguaggio. È il fondamento su cui tutti gli altri sistemi operano. Fornisce `TypeBase` come classe base astratta con le sottoclassi concrete `PrimitiveType`, `CustomType`, `ArrayType`, `VectorType`. `TypePtr` (`shared_ptr<const TypeBase>`) è il tipo fondamentale per tutto il type checker.

**Scopo**: Rappresentare tipi primitivi (i8–i64, u8–u64, f32, f64, bool, string, char, void, nullptr), tipi personalizzati (nomi utente), tipi array (con espressione dimensione), tipi vector (dinamici), variabili di tipo (`TypeVariable`), e tipo errore (`ErrorType`).

**Scope esplicito**: Solo definizione della gerarchia tipi. Non include parsing di annotazioni (hardcoded in `TypeChecker.cpp:18–35`), né operazioni di unificazione (delegate a S3).

**Posizione nella pipeline**: Upstream assoluto — tutti gli altri sistemi ricevono `TypePtr` come input o producono `TypePtr` come output.

**Contesto di attivazione**: Costruzione on-demand tramite factory. `PrimitiveType` usa singleton header-only; `ArrayType`/`VectorType` usano `std::make_shared`.

#### 2.2 Internal Module Organization

| File       | Tipo            | Righe | Scopo dichiarato              | Scopo effettivo                                    |
|------------|-----------------|-------|-------------------------------|----------------------------------------------------|
| `Type.hpp` | Header          | ~629  | Gerarchia tipi completa       | 5 classi + enum + formatter — **God-class header** |
| `Type.cpp` | Implementazione | 61    | `to_string()` e `sizes_equal` | Solo 2 funzioni — minimo ma coerente               |

**Criticità**: `Type.hpp` è un **God-class header** — 5 classi + enum `TypeKind` + `formatter<TypeBase>` in un solo file. Sarebbe preferibile separare `PrimitiveType`, `CustomType`, `ArrayType`, `VectorType` in file distinti per ridurre le dipendenze di compilazione.

**Verdetto**: Decomposizione fisica incoerente con la decomposition logica. Ogni sottoclasse concettuale dovrebbe avere il proprio file `.hpp`/`.cpp`.

#### 2.3 Intra-System Dependency Analysis

Dipendenze lineari e pulite: `TypeBase` → zero dipendenze interne. Le sottoclassi dipendono solo da `TypeBase`. Nessuna circolarità. `PrimitiveType` è auto-contenuto con singleton. `ArrayType` e `VectorType` condividono la logica `sizes_equal` in `Type.cpp`.

#### 2.4 Logical Flow

Il sistema è puramente dichiarativo — non c'è "flusso" computazionale. I tipi sono costruiti tramite factory:

- `PrimitiveType::i32()` → singleton `const TypePtr`
- `ArrayType::make(elem, size)` → `std::make_shared<ArrayType>`
- `fresh_type_variable()` → `std::make_shared<TypeVariable>` (thread-local counter)

Le operazioni fondamentali sono:

- `to_string()` — serializzazione per diagnostic (override virtuale)
- `operator==` — uguaglianza strutturale (override virtuale)
- `is_primitive()`, `is_integer()`, `is_numeric()` — predicate inline

#### 2.5 Critical Points

**`DEF-004` — `parse_type_annotation` hardcoded**: La funzione `parse_type_annotation` in [`TypeChecker.cpp:18–35`] è hardcoded con un if-else chain. Se un nuovo tipo primitivo venisse aggiunto a `Type.hpp`, questa funzione non lo riconoscerebbe automaticamente — richiede modifica manuale in due file separati.

**`DEF-005` — `ArrayType::sizes_equal` limitato**: L'implementazione in [`Type.cpp:50–56`] gestisce solo `IntegerLiteral`. Se la dimensione dell'array fosse un'espressione complessa (es. `2 + 3`), il confronto fallirebbe silenziosamente, restituendo `&a == &b` (identità referenziale), che è quasi sempre `false`.

#### 2.6 Partial or Undefined Implementations

Tutte le classi dichiarate hanno implementazione completa. Nessuna funzione stub.

### System: Type Inference Variables System (S2)

#### 2.1 System Overview

Il **Type Inference Variables System** comprende `TypeVariable`, `TypeScheme`, e `ErrorType`. Fornisce le variabili di tipo `?Tn` per l'inferenza Hindley-Milner, i tipi polimorfici `∀vars.body`, e il tipo sentinella per il recupero errori.

**Scopo**: Rappresentare l'incertezza durante l'inferenza (TypeVariable), la polimorfismo parametrico (TypeScheme), e il fallback per errori (ErrorType).

**Posizione nella pipeline**: Midstream — S2 riceve `TypePtr` da S1 e li arricchisce con metadati di inferenza. Consumato da S3 (unificazione), S4 (symbol table), S5 (constraint generation).

#### 2.2 Internal Module Organization

| File               | Tipo            | Righe | Scopo                                                   |
|--------------------|-----------------|-------|---------------------------------------------------------|
| `TypeVariable.hpp` | Header          | 106   | Classe `TypeVariable`, funzione `fresh_type_variable()` |
| `TypeVariable.cpp` | Implementazione | 24    | `to_string()` e counter thread-local                    |
| `TypeScheme.hpp`   | Header          | 82    | Struct `TypeScheme` con `instantiate()` e `mono()`      |
| `TypeScheme.cpp`   | Implementazione | 46    | Implementazione `instantiate()` — parziale              |
| `ErrorType.hpp`    | Header          | 53    | Classe `ErrorType` singleton                            |
| `ErrorType.cpp`    | Implementazione | 19    | `to_string()` e funzione `error_type()`                 |

Struttura pulita e coerente. Ogni concetto ha il proprio file.

#### 2.3 Intra-System Dependency Analysis

`TypeVariable` → `TypeBase`. `TypeScheme` → `TypeVariable` + `TypeBase`. `ErrorType` → `TypeBase`. Dipendenze lineari, nessuna circolarità.

#### 2.4 Logical Flow

`fresh_type_variable()` genera variabili fresche con counter thread-local [`TypeVariable.cpp:12–14`]. `TypeScheme::instantiate()` genera variabili fresche per i quantificati [`TypeScheme.cpp:14–33`]. `error_type()` restituisce singleton [`ErrorType.cpp:11–14`].

#### 2.5 Critical Points

**`DEF-006` — `TypeScheme::instantiate()` incompleto**: L'implementazione in [`TypeScheme.cpp:14–33`] gestisce solo il caso in cui il `body` è un `TypeVariable` diretto. Se il body è un tipo composto (es. `Vec<TypeVar1>`), le variabili quantificate all'interno **non vengono sostituite**. Il commento nel codice lo ammette esplicitamente: `"This is a simplified implementation - full version would use a visitor."`.

**`DEF-007` — `TypeScheme` con campi mutabili non documentati**: `TypeScheme` è una `struct` con campi pubblici `return_type` e `function_name` [`TypeScheme.hpp:28–30`] che vengono mutati da `SymbolTable::set_function_return_context` [`SymbolTable.cpp:37–53`]. Questo accoppiamento stretto non è documentato come contratto. La mutabilità di una struct che dovrebbe essere immutabile (il body è `TypePtr = shared_ptr<const T>`) è un'incoerenza concettuale.

#### 2.6 Partial or Undefined Implementations

- `TypeScheme::instantiate()` — **parziale** (vedi DEF-006). Per tipi composti, restituisce il body invariato. Non crash, ma produce type inference errata per funzioni polimorfiche con signature composte.

### System: Constraint Solving System (S3)

#### 2.1 System Overview

Il **Constraint Solving System** implementa l'unificazione di tipi tramite union-find con path compression e union by rank. Comprende `Constraint`/`ConstraintSet` per l'accumulo dei vincoli, `Substitution` per il mapping delle soluzioni, `UnionFind` per l'efficienza dell'unificazione, `ConstraintSolver` come motore, e `TypeVisitor` per la visita ricorsiva.

**Scopo**: Ricevere un `ConstraintSet` di uguaglianze `lhs = rhs` e produrre un `Substitution` che risolva tutte le variabili di tipo, o un insieme di `CompileError` per vincoli irrisolvibili.

**Posizione nella pipeline**: Fase 3 della pipeline TypeChecker. Riceve vincoli da S5, produce `SolverResult` con `Substitution` + errori.

#### 2.2 Internal Module Organization

| File                   | Tipo            | Righe | Scopo                                   |
|------------------------|-----------------|-------|-----------------------------------------|
| `Constraint.hpp`       | Header          | 143   | `Constraint`, `ConstraintSet`           |
| `Constraint.cpp`       | Implementazione | 28    | Metodi `ConstraintSet`                  |
| `Substitution.hpp`     | Header          | 158   | `Substitution` con cache persistente    |
| `Substitution.cpp`     | Implementazione | 74    | `bind`, `lookup`, `apply`, `applyImpl`  |
| `UnionFind.hpp`        | Header          | 73    | Disjoint-set con path compression       |
| `UnionFind.cpp`        | Implementazione | 47    | `make_set`, `find`, `unite`, `same_set` |
| `ConstraintSolver.hpp` | Header          | 106   | `ConstraintSolver`, `SolverResult`      |
| `ConstraintSolver.cpp` | Implementazione | 149   | `solve`, `unify`, `occurs_in`           |
| `TypeVisitor.hpp`      | Header          | 91    | Visitor per tipi composti               |
| `TypeVisitor.cpp`      | Implementazione | 29    | `visit_type` dispatch                   |

Struttura corretta: ogni concetto ha il proprio file `.hpp`/`.cpp`. `ConstraintSolver.hpp` include `Constraint.hpp`, `Substitution.hpp`, `UnionFind.hpp`.

#### 2.3 Intra-System Dependency Analysis

Grafo dipendenze:

```
ConstraintSolver → UnionFind + Substitution + TypeVisitor + ErrorType
Substitution → TypeVisitor + ErrorType
Constraint → TypePtr (S1)
UnionFind → nessuna dipendenza interna
TypeVisitor → TypePtr (S1)
```

Nessuna circolarità. Il grafo è un DAG pulito.

**Accoppiamento**: `ConstraintSolver::unify` usa `UnifyVisitor` locale (struct interna al `.cpp`) — accoppiamento stretto ma intenzionale e contenuto.

#### 2.4 Logical Flow

1. `ConstraintSet::add()` accumula vincoli `lhs = rhs` con ID sequenziali [`Constraint.cpp:10–15`].
2. `ConstraintSolver::solve()` itera sui vincoli e chiama `unify()` per ciascuno [`ConstraintSolver.cpp:44–54`].
3. `unify()` gestisce: (a) ErrorType → successo silente; (b) TypeVariable → binding o occurs-check; (c) tipi concreti → verifica kind equality e visita ricorsiva [`ConstraintSolver.cpp:67–139`].
4. Il risultato è un `Substitution` + eventuali errori.

#### 2.5 Critical Points

**`DEF-008` — `UnionFind::find()` usa `at()` con eccezione**: In [`UnionFind.cpp:14–17`], `parent_.at(var)` lancia `std::out_of_range` se `var` non è registrato. Questo è un fallimento a runtime non gestito — dovrebbe usare `find()` con controllo o un `assert`. Lo stesso vale per `rank_.at()` in `unite()` [`UnionFind.cpp:24–25`].

**`DEF-009` — `UnifyVisitor` non gestisce tutti i casi composti**: In [`ConstraintSolver.cpp:26–35`], `UnifyVisitor` gestisce solo `visit_array` e `visit_vector`. Se `t1` è `CustomType`, il visitor non viene dispatchato e `visitor.result` rimane `std::nullopt`, portando a `value_or(success)` [`ConstraintSolver.cpp:139`]. Questo è corretto per `CustomType` perché il kind check è già stato fatto prima — ma se venisse aggiunto un nuovo tipo composto a `TypeKind`, il visitor lo ignorerebbe silenziosamente.

#### 2.6 Partial or Undefined Implementations

Tutte le funzioni dichiarate sono implementate. `ConstraintSolver::occurs_in` è completa.

### System: Name Resolution System (S4)

#### 2.1 System Overview

Il **SymbolTable** gestisce binding identificatore→`TypeScheme` con scope annidati. Supporta shadowing, lookup dall'interno verso l'esterno, e definizione nel scope corrente.

**Scopo**: Mantenere una mappa degli identificatori dichiarati durante la name resolution, con supporto per scope lessicali annidati (globale → funzione → blocco).

**Posizione nella pipeline**: Fase 1 della pipeline TypeChecker. Popolato da `resolve_names()`, consultato da `type_expr()` e `type_stmt()`.

#### 2.2 Internal Module Organization

| File              | Tipo            | Righe | Scopo                                        |
|-------------------|-----------------|-------|----------------------------------------------|
| `SymbolTable.hpp` | Header          | 111   | Classe `SymbolTable` con hash personalizzato |
| `SymbolTable.cpp` | Implementazione | 73    | Tutte le funzioni membro                     |

Struttura minimale e coerente.

#### 2.3 Intra-System Dependency Analysis

Nessuna dipendenza interna oltre a `TypeScheme`. Dipendenze esterne: `<unordered_map>`, `<string_view>`, `<vector>`.

#### 2.4 Logical Flow

`push_scope()` crea un nuovo `unordered_map` nel vector `scopes_`. `define()` inserisce nel back. `lookup()` itera in ordine inverso (dall'interno all'esterno) [`SymbolTable.cpp:20–26`]. `set_function_return_context()` cerca il contesto funzione più recente nello scope corrente [`SymbolTable.cpp:37–53`].

#### 2.5 Critical Points

**`DEF-010` — `StringHash` con `string_view` e ownership**: `SymbolTable` usa `unordered_map<std::string_view, TypeScheme, StringHash>` [`SymbolTable.hpp:69`]. Le `string_view` come chiave puntano a stringhe esterne. Se la stringa originale viene deallocata, la chiave diventa dangling. Questo è sicuro finché i nomi degli identificatori vivono abbastanza, ma è una **precondizione non documentata**.

**`DEF-011` — `define()` crea scope implicitamente**: Se `define()` viene chiamato senza scope attivo, crea implicitamente un scope [`SymbolTable.cpp:16–17`]. Questo comportamento nascosto maschera bug di chiamante che dimentica `push_scope()`.

#### 2.6 Partial or Undefined Implementations

Completo. Nessuna funzione dichiarata senza implementazione.

### System: Type Checking Orchestration System (S5)

#### 2.1 System Overview

Il **TypeChecker** è l'orchestratore della pipeline di type checking. Espone `check()` come entry point principale che esegue: (1) name resolution, (2) constraint generation, (3) constraint solving, (4) zonking. Espone anche `type_expr()` e `type_stmt()` pubblicamente per unit testing.

**Scopo**: Coordinare l'intera pipeline di type checking da un AST non tipizzato a un AST completamente tipizzato.

**Posizione nella pipeline**: È il consumer finale — riceve `Program` non tipizzato, restituisce `TypeCheckResult` con `TypedProgram` + errori.

**Contesto di attivazione**: Istanziato una volta per unità di compilazione in `main.cpp`. Stateful — mantiene `symbols_`, `constraints_`, `errors_`, `typed_stmts_` come membro.

#### 2.2 Internal Module Organization

| File              | Tipo            | Righe | Scopo                                          |
|-------------------|-----------------|-------|------------------------------------------------|
| `TypeChecker.hpp` | Header          | 121   | Dichiarazione `TypeChecker`, `TypeCheckResult` |
| `TypeChecker.cpp` | Implementazione | 1238  | Tutta la logica di type checking               |

**Criticità**: `TypeChecker.cpp` è il **file più grande** del type checker. 1238 righe includono:

- `parse_type_annotation` (statica, 18 righe)
- `zonk_type` (statica, 24 righe)
- `check()` (19 righe)
- `resolve_names`/`resolve_names_stmt` (~70 righe)
- `generate_constraints` (7 righe)
- `solve_constraints` (5 righe)
- `zonk`/`zonk_stmt_full`/`zonk_expr_full`/`zonk_block_full` (~270 righe)
- `type_expr` (~430 righe) — **la funzione più grande**
- `type_stmt` (~300 righe)

#### 2.3 Intra-System Dependency Analysis

`TypeChecker` dipende da TUTTI gli altri sistemi. È il punto di convergenza del DAG. Dipendenze: S1 (TypeBase), S2 (TypeVariable, TypeScheme, ErrorType), S3 (Constraint, ConstraintSolver), S4 (SymbolTable), più AST nodes e error handling.

#### 2.4 Logical Flow

`TypeChecker::check()` [`TypeChecker.cpp:70–88`]:

1. Reset stato interno.
2. `resolve_names(program)` — popola `symbols_`.
3. `generate_constraints(program)` — chiama `type_stmt()` per ogni statement, accumula vincoli in `constraints_`.
4. `solve_constraints()` — crea `ConstraintSolver` temporaneo, risolve.
5. `zonk(subst)` — applica la sostituzione all'AST tipizzato.

#### 2.5 Critical Points

**`DEF-013` — `type_expr` per `CallExpr` con gestione signature parziale**: In [`TypeChecker.cpp:698–764`], la chiamata a funzione tipizza il callee e gli argomenti e ORA verifica l'arity e genera vincoli sui parametri. Tuttavia il tipo del callee resta una variabile fresca non vincolata alla signature della funzione. Il tipo di ritorno viene preso dalla `FuncDecl` ma non c'è vincolo che colleghi il callee type alla funzione signature.

**`DEF-014` — `type_stmt` per `VarDecl` multi-variable semplificato**: In [`TypeChecker.cpp:975–1006`], le dichiarazioni multi-variabili (`let a, b, c = 1, 2, 3`) vengono semplificate a una singola `TypedVarDecl` per la prima variabile. Le altre vengono registrate nella SymbolTable ma **non compaiono nell'AST tipizzato**.

**`DEF-015` — `zonk_block_full` perde statement**: In [`TypeChecker.cpp:413–421`], quando `zonk_stmt_full` restituisce `nullptr` per uno statement in un blocco, lo statement viene silenziosamente scartato ("Can't move from const — skip"). Questo corrompe l'AST tipizzato.

**`DEF-016` — `message_storage_` fragile**: Il `deque<std::string>` in [`TypeChecker.hpp:102`] possiede le stringhe dei messaggi d'errore. I `CompileError` contengono `string_view` su queste stringhe. Se `message_storage_` viene riallocato durante l'inserimento, i `string_view` già memorizzati negli `errors_` **potrebbero** diventare dangling — sebbene `deque` garantisca stabilità degli iteratori, la documentazione non esplicita questa garanzia come invariant.

**`DEF-018` — Type checking anticipato duplica logica del solver**: In `type_expr`, per `BinaryExpr` e `UnaryExpr`, il codice esegue controlli anticipati sui tipi concreti (es. `!lhs_type->is_numeric()`) [`TypeChecker.cpp:550–560`] che **duplicano** la logica che il solver esegue già. Se il solver fallisce, l'errore viene riportato due volte — una volta dal check anticipato e una volta dal solver.

#### 2.6 Partial or Undefined Implementations

Tutte le funzioni dichiarate in `TypeChecker.hpp` sono implementate in `TypeChecker.cpp`. Non ci sono stub.

---

## Phase 3 — Per-Component Exhaustive Analysis

### System: Type Representation (S1) › Component: TypeBase

#### 3.1 Responsibility Statement

`TypeBase` è la classe base astratta che fornisce il discriminante `TypeKind` e l'interfaccia comune (`to_string()`, `operator==`) per tutte le rappresentazioni di tipo nel sistema.

#### 3.2 Class Structure

| Membro               | Tipo                                      | Visibilità  | Semantica                       |
|----------------------|-------------------------------------------|-------------|---------------------------------|
| `kind_`              | `TypeKind`                                | `private`   | Discriminante del tipo concreto |
| `kind()`             | `constexpr TypeKind() const noexcept`     | `public`    | Getter per `kind_`              |
| `TypeBase(TypeKind)` | Costruttore esplicito constexpr           | `protected` | Inizializza `kind_`             |
| `~TypeBase()`        | Distruttore virtuale                      | `public`    | Polimorfismo sicuro             |
| `to_string()`        | `virtual std::string() const = 0`         | `public`    | Serializzazione per diagnostic  |
| `operator==`         | `virtual bool(const TypeBase&) const = 0` | `public`    | Uguaglianza strutturale         |
| `is_primitive()`     | `bool() const noexcept`                   | `public`    | Predicate inline                |
| `is_numeric()`       | `bool() const noexcept`                   | `public`    | Predicate inline                |
| `is_integer()`       | `bool() const noexcept`                   | `public`    | Predicate inline                |

**Ereditarietà**: `TypeBase` è base per `PrimitiveType`, `CustomType`, `ArrayType`, `VectorType`, `TypeVariable`, `ErrorType`. Ereditarietà pubblica singola. Nessun problema di diamond.

#### 3.3 Interface Analysis

| Metodo        | Signature                                                  | Precondizioni  | Postcondizioni                       | Contract                                          |
|---------------|------------------------------------------------------------|----------------|--------------------------------------|---------------------------------------------------|
| `kind()`      | `constexpr TypeKind kind() const noexcept`                 | Nessuna        | Restituisce il `TypeKind` del tipo   | Pura, const, noexcept                             |
| `to_string()` | `virtual std::string to_string() const = 0`                | Nessuna        | Restituisce rappresentazione stringa | Pure virtual — ogni sottoclasse deve implementare |
| `operator==`  | `virtual bool operator==(const TypeBase& other) const = 0` | `other` valido | true se strutturalmente uguali       | Pure virtual — dispatch dinamico                  |

Nessuna discrepanza tra `.hpp` e `.cpp`.

#### 3.4 Implementation Logic

`TypeBase` è puramente astratta — nessuna logica nel `.cpp`. I metodi `is_primitive()`, `is_numeric()`, `is_integer()` sono inline in `.hpp`. Complessità: O(1) — switch su enum. Nessun loop, nessuna ricorsione.

#### 3.5 Error Handling Evaluation

Nessun errore gestito a questo livello — è una classe base astratta. I predicate inline restituiscono semplicemente booleani.

#### 3.6 Type Consistency Audit

`TypePtr = std::shared_ptr<const TypeBase>` — tipo immutabile per costruzione. Nessun cast unsafe. Nessun mismatch tra dichiarazione e definizione.

#### 3.7 Inter-Component Interaction

`TypeBase` è il punto di convergenza di TUTTI i sistemi. Ogni componente che manipola tipi dipende da `TypeBase`. L'accoppiamento è inevitabile ma mitigato dal fatto che `TypeBase` è stabile (aggiunte rare) e immutabile.

#### 3.8 Optimization Opportunities

**Strutturale**: `Type.hpp` come God-class header. **Raccomandazione**: Separare in `TypeBase.hpp`, `PrimitiveType.hpp`, `CustomType.hpp`, `ArrayType.hpp`, `VectorType.hpp`.

---

### System: Type Representation (S1) › Component: PrimitiveType

#### 3.1 Responsibility Statement

`PrimitiveType` rappresenta i tipi primitivi del linguaggio (interi, float, booleani, stringhe, ecc.) come singleton immutabili per ottimizzare memoria e confrontare tipi per identità referenziale.

#### 3.2 Class Structure

| Membro                                         | Tipo                      | Visibilità | Semantica                            |
|------------------------------------------------|---------------------------|------------|--------------------------------------|
| `primitive_type_`                              | `TypeKind`                | `private`  | Quale tipo primitivo rappresenta     |
| `i8()`, `i16()`, ..., `f64()`, `bool_()`, ecc. | `static const TypePtr&()` | `public`   | Factory singleton per ogni primitivo |

Tutti i metodi singleton seguono il pattern Meyers singleton thread-safe.

#### 3.3 Interface Analysis

| Metodo           | Signature                                               | Precondizioni  | Postcondizioni                                 | Contract                             |
|------------------|---------------------------------------------------------|----------------|------------------------------------------------|--------------------------------------|
| `i8()`–`void_()` | `static std::shared_ptr<const PrimitiveType> name()`    | Nessuna        | Restituisce shared_ptr a singleton immutabile | Pura, thread-safe (Meyers singleton) |
| `to_string()`    | `std::string to_string() const override`                | Nessuna        | Nome del tipo (es. "i32", "bool")              | Override di `TypeBase`               |
| `operator==`     | `bool operator==(const TypeBase& other) const override` | `other` valido | true se `other.kind() == primitive_type_`      | Uguaglianza per kind                 |

Nessuna discrepanza.

#### 3.4 Implementation Logic

Singleton pattern con variabile statica locale (Meyers singleton). Thread-safe per costruzione in C++11+. Complessità O(1) per accesso.

#### 3.5 Error Handling Evaluation

Nessun errore possibile — i singleton sono sempre inizializzati correttamente.

#### 3.6 Type Consistency Audit

`TypeKind` è un enum class — nessuna conversione implicita rischiosa. I singleton restituiscono `std::shared_ptr<const PrimitiveType>` — coerente con il resto del sistema.

#### 3.7 Inter-Component Interaction

Consumato da TUTTI i sistemi che creano tipi concreti. L'identità referenziale dei singleton permette confronti `==` per puntatore invece che per valore in molti casi.

#### 3.8 Optimization Opportunities

Nessuna ottimizzazione necessaria — il design singleton è già ottimale per memoria e performance.

---

### System: Type Representation (S1) › Component: ArrayType

#### 3.1 Responsibility Statement

`ArrayType` rappresenta i tipi array con dimensione fissata a compile-time, memorizzando il tipo degli elementi e un'espressione per la dimensione.

#### 3.2 Class Structure

| Membro          | Tipo      | Visibilità | Semantica                                                   |
|-----------------|-----------|------------|-------------------------------------------------------------|
| `element_type_` | `TypePtr` | `private`  | Tipo degli elementi dell'array                              |
| `size_expr_`    | `ExprPtr` | `private`  | Espressione della dimensione (tipicamente `IntegerLiteral`) |

#### 3.3 Interface Analysis

| Metodo           | Signature                         | Precondizioni | Postcondizioni                         | Contract        |
|------------------|-----------------------------------|---------------|----------------------------------------|-----------------|
| `element_type()` | `const std::shared_ptr<const TypeBase>&() const noexcept` | Nessuna | Riferimento al tipo elemento | Const, noexcept |
| `size_expr()`    | `const std::shared_ptr<const Expr>&() const noexcept` | Nessuna | Riferimento all'espressione dimensione | Const, noexcept |

#### 3.4 Implementation Logic

Costruttore semplice che inizializza `element_type_` e `size_expr_`. `to_string()` produce formato `"ArrayType<element_type>[size]"`. `operator==` confronta `element_type` e chiama `sizes_equal`.

`sizes_equal` in [`Type.cpp:50–56`] confronta due `ExprPtr`:

```cpp
bool ArrayType::sizes_equal(const Expr &a, const Expr &b) noexcept {
    if(const auto *ia = node_dyn_cast<const IntegerLiteral>(&a)) {
        if(const auto *ib = node_dyn_cast<const IntegerLiteral>(&b)) { return ia->value() == ib->value(); }
    }
    return &a == &b;  // Fallisce silenziosamente per espressioni non letterali
}
```

#### 3.5 Error Handling Evaluation

`sizes_equal` fallisce silenziosamente per espressioni non letterali — restituisce `false` anche se due espressioni sono semanticamente equivalenti (es. `2 + 1` vs `3`).

#### 3.6 Type Consistency Audit

Nessun problema. `ExprPtr` è usato correttamente.

#### 3.7 Inter-Component Interaction

Consumato da `Substitution::applyImpl`, `ConstraintSolver::UnifyVisitor`, `TypeChecker::zonk_expr_full`. Tutti gestiscono `ArrayType` esplicitamente.

#### 3.8 Optimization Opportunities

**DEF-005 (ripreso)**: `sizes_equal` dovrebbe usare un visitor AST per confrontare espressioni strutturalmente, non solo letterali.

---

### System: Type Representation (S1) › Component: VectorType

#### 3.1 Responsibility Statement

`VectorType` rappresenta i tipi vettore dinamici (tipo elemento senza dimensione fissa).

#### 3.2 Class Structure

| Membro          | Tipo      | Visibilità | Semantica                       |
|-----------------|-----------|------------|---------------------------------|
| `element_type_` | `TypePtr` | `private`  | Tipo degli elementi del vettore |

#### 3.3 Interface Analysis

| Metodo           | Signature                         | Precondizioni | Postcondizioni               | Contract        |
|------------------|-----------------------------------|---------------|------------------------------|-----------------|
| `element_type()` | `const std::shared_ptr<const TypeBase>&() const noexcept` | Nessuna | Riferimento al tipo elemento | Const, noexcept |

#### 3.4 Implementation Logic

Semplice wrapper su `element_type_`. `to_string()` produce `"VectorType<element_type>"`. `operator==` delega al confronto degli element type.

#### 3.5 Error Handling Evaluation

Nessun errore possibile.

#### 3.6 Type Consistency Audit

Nessun problema.

#### 3.7 Inter-Component Interaction

Simile ad `ArrayType`, gestito da `Substitution`, `ConstraintSolver`, `TypeChecker`.

#### 3.8 Optimization Opportunities

Nessuna ottimizzazione necessaria — classe minimale e corretta.

---

### System: Type Representation (S1) › Component: CustomType

#### 3.1 Responsibility Statement

`CustomType` rappresenta i tipi definiti dall'utente (classi, struct, enum) memorizzando il nome non qualificato e il namespace.

#### 3.2 Class Structure

| Membro       | Tipo          | Visibilità | Semantica                    |
|--------------|---------------|------------|------------------------------|
| `name_`      | `std::shared_ptr<const std::string>` | `private` | Nome del tipo custom |

#### 3.3 Interface Analysis

| Metodo         | Signature                             | Precondizioni | Postcondizioni | Contract        |
|----------------|---------------------------------------|---------------|----------------|-----------------|
| `name()`       | `std::string_view() const noexcept`   | Nessuna       | Nome del tipo  | Const, noexcept |

#### 3.4 Implementation Logic

`to_string()` produce `"name"`. `operator==` confronta nome tramite dereferenziazione dello shared_ptr.

#### 3.5 Error Handling Evaluation

Nessun errore possibile.

#### 3.6 Type Consistency Audit

Nessun problema.

#### 3.7 Inter-Component Interaction

`CustomType` è il tipo meno gestito nel solver — `UnifyVisitor` non lo visita esplicitamente, si affida al kind check preliminare.

#### 3.8 Optimization Opportunities

Nessuna ottimizzazione necessaria.

---

### System: Type Inference Variables (S2) › Component: TypeVariable

#### 3.1 Responsibility Statement

`TypeVariable` rappresenta le variabili di tipo (`?T1`, `?T2`, ...) generate durante l'inferenza per esprimere incertezza sul tipo di un'espressione.

#### 3.2 Class Structure

| Membro | Tipo                   | Visibilità | Semantica                              |
|--------|------------------------|------------|----------------------------------------|
| `id_`  | `TypeVarId` (`size_t`) | `private`  | Identificatore univoco della variabile |

#### 3.3 Interface Analysis

| Metodo                    | Signature                                       | Precondizioni  | Postcondizioni                                   | Contract            |
|---------------------------|-------------------------------------------------|----------------|--------------------------------------------------|---------------------|
| `TypeVariable(TypeVarId)` | Costruttore constexpr esplicito                 | `id > 0`       | Inizializza `id_`                                | Constexpr, noexcept |
| `id()`                    | `TypeVarId() const noexcept`                    | Nessuna        | Restituisce `id_`                                | Constexpr, noexcept |
| `classof()`               | `static bool(const TypeBase*) noexcept`         | Nessuna        | true se puntatore a `TypeVariable`               | LLVM-style RTTI     |
| `operator==`              | `bool(const TypeBase&) const noexcept override` | `other` valido | true se `other` è `TypeVariable` con stesso `id` | Uguaglianza per ID  |

#### 3.4 Implementation Logic

`to_string()` produce `"?T{id}"`. `fresh_type_variable()` usa counter thread-local [`TypeVariable.cpp:12–14`] — thread-safe ma **non resetta** tra unità di compilazione. IDs crescono indefinitamente.

#### 3.5 Error Handling Evaluation

Nessun errore gestito. Se `id_` fosse 0 (non documentato come invalido ma implicato dall'invariante), il comportamento è indefinito.

#### 3.6 Type Consistency Audit

Nessun problema.

#### 3.7 Inter-Component Interaction

`TypeVariable` è il tipo più manipolato nel constraint solver. `Substitution::bind`, `UnionFind::make_set`, `ConstraintSolver::unify` operano tutti su `TypeVarId`.

#### 3.8 Optimization Opportunities

**Performance**: `fresh_type_variable()` non resetta il counter. Per compilazioni multiple nella stessa istanza del compilatore, gli IDs crescono indefinitamente. Soluzione: esporre `reset_type_var_counter()` per il TypeChecker.

---

### System: Type Inference Variables (S2) › Component: TypeScheme

#### 3.1 Responsibility Statement

`TypeScheme` rappresenta i tipi polimorfici con quantificazione universale (`∀vars.body`) usati per il let-polimorfismo Hindley-Milner.

#### 3.2 Class Structure

| Membro            | Tipo                         | Visibilità | Semantica                                |
|-------------------|------------------------------|------------|------------------------------------------|
| `quantified_vars` | `std::vector<TypeVarId>`     | `public`   | Variabili di tipo quantificate           |
| `body`            | `TypePtr`                    | `public`   | Corpo del tipo con riferimenti alle vars |
| `is_const`        | `bool`                       | `public`   | Se il binding è immutabile               |
| `return_type`     | `std::optional<TypePtr>`     | `public`   | Tipo di ritorno (per funzioni)           |
| `function_name`   | `std::optional<std::string>` | `public`   | Nome funzione (per diagnostic)           |

#### 3.3 Interface Analysis

| Metodo          | Signature                                                               | Precondizioni | Postcondizioni             | Contract                                  |
|-----------------|-------------------------------------------------------------------------|---------------|----------------------------|-------------------------------------------|
| `instantiate()` | `TypePtr() const`                                                       | Nessuna       | Tipo con variabili fresche | **PARZIALE** — non gestisce tipi composti |
| `mono()`        | `static TypeScheme(TypePtr, bool, optional<TypePtr>, optional<string>)` | Nessuna       | Scheme monomorfico         | Factory corretta                          |

#### 3.4 Implementation Logic

`instantiate()` [`TypeScheme.cpp:14–33`]:

```cpp
TypePtr TypeScheme::instantiate() const {
    if(quantified_vars.empty()) { return body; }
    std::unordered_map<TypeVarId, TypePtr> fresh_vars;
    for(auto qvar : quantified_vars) { fresh_vars[qvar] = fresh_type_variable(); }
    if(const auto *tv = dynamic_cast<const TypeVariable *>(body.get())) {
        auto it = fresh_vars.find(tv->id());
        if(it != fresh_vars.end()) { return it->second; }
        return body;
    }
    return body;  // BUG: tipi composti non gestiti
}
```

Il commento ammette: `"This is a simplified implementation - full version would use a visitor."`.

#### 3.5 Error Handling Evaluation

Nessun errore gestito. Per tipi composti con variabili quantificate interne, il risultato è **sbagliato** — le variabili non vengono sostituite, portando a type inference errata.

#### 3.6 Type Consistency Audit

Nessun mismatch formale, ma il comportamento è semanticamente errato per tipi composti.

#### 3.7 Inter-Component Interaction

`TypeScheme::instantiate()` è chiamato da `TypeChecker::type_expr` per `Identifier` [`TypeChecker.cpp:519`]. Se l'identificatore ha un tipo polimorfico composto, l'istanziazione è errata.

#### 3.8 Optimization Opportunities

**Critico**: `instantiate()` deve essere riscritto con un visitor che attraversa ricorsivamente il body e sostituisce tutte le occorrenze delle variabili quantificate. Vedi **REC-006**.

---

### System: Type Inference Variables (S2) › Component: ErrorType

#### 3.1 Responsibility Statement

`ErrorType` è un tipo sentinella singleton che unifica silenziosamente con qualsiasi tipo per prevenire errori a cascata dopo un errore di tipo rilevato.

#### 3.2 Class Structure

Classe vuota oltre a `TypeBase`. Singleton tramite `error_type()`.

#### 3.3 Interface Analysis

| Metodo         | Signature                                       | Precondizioni | Postcondizioni                            | Contract                        |
|----------------|-------------------------------------------------|---------------|-------------------------------------------|---------------------------------|
| `error_type()` | `TypePtr() noexcept`                            | Nessuna       | Restituisce singleton condiviso           | Thread-safe (Meyers)            |
| `operator==`   | `bool(const TypeBase&) const noexcept override` | Nessuna       | true se `other.kind() == TypeKind::Error` | Tutti gli ErrorType sono uguali |

#### 3.4 Implementation Logic

Singleton [`ErrorType.cpp:11–14`]. `to_string()` restituisce `"<error>"`.

#### 3.5 Error Handling Evaluation

`ErrorType` è il meccanismo di error recovery. Unifica silenziosamente con qualsiasi tipo [`ConstraintSolver.cpp:67–68`] — questo è intenzionale.

#### 3.6 Type Consistency Audit

Nessun problema.

#### 3.7 Inter-Component Interaction

Consumato da `ConstraintSolver::unify`, `TypeChecker::type_expr` (per identificatori non dichiarati), `TypeChecker::type_stmt` (default per statement non supportati).

#### 3.8 Optimization Opportunities

Nessuna ottimizzazione necessaria.

---

### System: Constraint Solving (S3) › Component: Constraint e ConstraintSet

#### 3.1 Responsibility Statement

`Constraint` rappresenta un vincolo di uguaglianza tra due tipi (`lhs = rhs`) con metadati di origine, mentre `ConstraintSet` accumula vincoli con ID sequenziali per il solver.

#### 3.2 Class Structure

`Constraint` è una struct con 5 campi pubblici: `id`, `lhs`, `rhs`, `origin`, `reason`.

`ConstraintSet` ha:
| Membro | Tipo | Visibilità | Semantica |
|--------|------|------------|-----------|
| `constraints_` | `std::vector<Constraint>` | `private` | Vincoli in ordine di inserimento |
| `next_id_` | `ConstraintId` (inizializzato a 1) | `private` | Prossimo ID da assegnare |

#### 3.3 Interface Analysis

| Metodo          | Signature                                                 | Precondizioni | Postcondizioni                  | Contract                         |
|-----------------|-----------------------------------------------------------|---------------|---------------------------------|----------------------------------|
| `add()`         | `ConstraintId(TypePtr, TypePtr, SourceSpan, string_view)` | Nessuna       | Vincolo aggiunto, ID restituito | ID 1-based                       |
| `constraints()` | `const vector<Constraint>&() const noexcept`              | Nessuna       | Riferimento ai vincoli          | Valido fino a prossima mutazione |
| `get()`         | `const Constraint*(ConstraintId) const noexcept`          | Nessuna       | Puntatore o nullptr             | Ricerca lineare O(n)             |
| `size()`        | `size_t() const noexcept`                                 | Nessuna       | Numero vincoli                  |                                  |

#### 3.4 Implementation Logic

`add()` assegna ID incrementale e push_back [`Constraint.cpp:10–15`]. `get()` usa `std::ranges::find` — ricerca lineare O(n). Per set grandi di vincoli, questo è un collo di bottiglia.

#### 3.5 Error Handling Evaluation

Nessun errore gestito — `ConstraintSet` è un contenitore passivo.

#### 3.6 Type Consistency Audit

Nessun problema.

#### 3.7 Inter-Component Interaction

`ConstraintSet` è popolato da `TypeChecker::generate_constraints` e consumato da `ConstraintSolver::solve`.

#### 3.8 Optimization Opportunities

**Performance**: `get()` è O(n). Se il solver o il diagnostic emitter cercano vincoli per ID frequentemente, converrebbe un `unordered_map<ConstraintId, Constraint>`.

---

### System: Constraint Solving (S3) › Component: UnionFind

#### 3.1 Responsibility Statement

`UnionFind` implementa la struttura disjoint-set con path compression e union by rank per tracciare le classi di equivalenza delle variabili di tipo durante l'unificazione.

#### 3.2 Class Structure

| Membro    | Tipo                                  | Visibilità | Semantica              |
|-----------|---------------------------------------|------------|------------------------|
| `parent_` | `unordered_map<TypeVarId, TypeVarId>` | `private`  | Mappa figlio→genitore  |
| `rank_`   | `unordered_map<TypeVarId, uint8_t>`   | `private`  | Rank per union by rank |

#### 3.3 Interface Analysis

| Metodo       | Signature                    | Precondizioni       | Postcondizioni             | Contract                |
|--------------|------------------------------|---------------------|----------------------------|-------------------------|
| `make_set()` | `void(TypeVarId)`            | Nessuna             | Crea insieme singleton     | Idempotente             |
| `find()`     | `TypeVarId(TypeVarId)`       | `var` registrato    | Restituisce rappresentante | Path compression        |
| `unite()`    | `void(TypeVarId, TypeVarId)` | Entrambi registrati | Unisce insiemi             | Union by rank           |
| `same_set()` | `bool(TypeVarId, TypeVarId)` | Nessuna             | true se stesso insieme     | Gestisce non-registrati |

#### 3.4 Implementation Logic

`find()` [`UnionFind.cpp:14–17`] usa path compression ricorsiva:

```cpp
TypeVarId UnionFind::find(TypeVarId var) {
    if(parent_.at(var) != var) { parent_[var] = find(parent_.at(var)); }
    return parent_.at(var);
}
```

**Problema**: `parent_.at(var)` lancia `std::out_of_range` se `var` non è registrato. Questo è un crash a runtime non gestito.

#### 3.5 Error Handling Evaluation

**DEF-008 (ripreso)**: `find()` e `unite()` usano `at()` senza try-catch. Se una variabile non viene registrata con `make_set()` prima di `find()`, il programma crash con `std::out_of_range`.

#### 3.6 Type Consistency Audit

Nessun problema di tipo. `TypeVarId` è `size_t` — coerente ovunque.

#### 3.7 Inter-Component Interaction

`UnionFind` è usato esclusivamente da `ConstraintSolver::unify`. Nessun altro componente lo consulta direttamente.

#### 3.8 Optimization Opportunities

**Robustezza**: Sostituire `at()` con `find()` + `assert` o fallback. Vedi **REC-009**.

---

### System: Constraint Solving (S3) › Component: Substitution

#### 3.1 Responsibility Statement

`Substitution` memorizza il mapping da variabili di tipo a tipi risolti, con cache persistente per ottimizzare ri-applicazioni successive.

#### 3.2 Class Structure

| Membro         | Tipo                                              | Visibilità | Semantica              |
|----------------|---------------------------------------------------|------------|------------------------|
| `bindings_`    | `unordered_map<TypeVarId, TypePtr>`               | `private`  | Mapping variabile→tipo |
| `apply_cache_` | `mutable unordered_map<const TypeBase*, TypePtr>` | `private`  | Cache risultati apply  |

#### 3.3 Interface Analysis

| Metodo        | Signature                                     | Precondizioni | Postcondizioni                       | Contract                      |
|---------------|-----------------------------------------------|---------------|--------------------------------------|-------------------------------|
| `bind()`      | `void(TypeVarId, TypePtr)`                    | Nessuna       | Binding registrato, cache invalidata | Sovrascrive binding esistenti |
| `lookup()`    | `optional<TypePtr>(TypeVarId) const noexcept` | Nessuna       | Tipo bound o nullopt                 |                               |
| `apply()`     | `TypePtr(const TypePtr&) const`               | Nessuna       | Tipo con variabili sostituite        | O(1) se in cache              |
| `applyImpl()` | `TypePtr(const TypePtr&) const`               | Nessuna       | Worker ricorsivo                     | Usa cache                     |
| `contains()`  | `bool(TypeVarId) const noexcept`              | Nessuna       | true se bound                        |                               |
| `size()`      | `size_t() const noexcept`                     | Nessuna       | Numero binding                       |                               |

#### 3.4 Implementation Logic

`applyImpl()` [`Substitution.cpp:44–63`]:

1. Controlla cache — se hit, restituisci.
2. Se `TypeVariable`, cerca in `bindings_`; se trovato, ricorsivamente `applyImpl` sul bound.
3. Se tipo composto, usa `ApplyVisitor` per visitare e ricostruire.
4. Memoizza e restituisci.

La cache è keyed su `const TypeBase*` — punta all'input originale, non al resolved. Questo è corretto perché l'input è immutabile.

#### 3.5 Error Handling Evaluation

Nessun errore gestito. Se un binding punta a un tipo con variabili non risolte, `apply` le lascia così come sono.

#### 3.6 Type Consistency Audit

Nessun problema. `mutable` su `apply_cache_` è giustificato — è un'ottimizzazione che non cambia lo stato osservabile.

#### 3.7 Inter-Component Interaction

`Substitution` è prodotto da `ConstraintSolver::solve` e consumato da `TypeChecker::zonk` e `zonk_type`.

#### 3.8 Optimization Opportunities

La cache persistente è ben progettata. Nessuna ottimizzazione necessaria.

---

### System: Constraint Solving (S3) › Component: ConstraintSolver

#### 3.1 Responsibility Statement

`ConstraintSolver` è il motore di unificazione che processa vincoli producendo una sostituzione risolutiva o errori per vincoli irrisolvibili.

#### 3.2 Class Structure

| Membro          | Tipo           | Visibilità | Semantica                     |
|-----------------|----------------|------------|-------------------------------|
| `union_find_`   | `UnionFind`    | `private`  | Traccia classi di equivalenza |
| `substitution_` | `Substitution` | `private`  | Accumula binding              |

#### 3.3 Interface Analysis

| Metodo        | Signature                                                    | Precondizioni | Postcondizioni                    | Contract               |
|---------------|--------------------------------------------------------------|---------------|-----------------------------------|------------------------|
| `solve()`     | `SolverResult(const ConstraintSet&)`                         | Nessuna       | Substitution + errori             | Resetta stato interno  |
| `unify()`     | `expected<void, CompileError>(TypePtr, TypePtr, Constraint)` | Tipi non-null | Void o errore                     | Occurs-check incluso   |
| `occurs_in()` | `static bool(TypeVarId, TypePtr, Substitution)`              | Nessuna       | true se variabile occorre nel tipo | Previste tipi infiniti |

#### 3.4 Implementation Logic

`solve()` [`ConstraintSolver.cpp:44–54`]: itera vincoli, chiama `unify`, accumula errori.

`unify()` [`ConstraintSolver.cpp:67–139`]:

1. ErrorType → successo silente.
2. Null type → errore E2034.
3. TypeVariable vs TypeVariable → occurs-check, bind, unite in union-find.
4. TypeVariable vs concreto → occurs-check, bind.
5. Concreto vs TypeVariable → swap e ricorsione.
6. Concreto vs concreto → kind check, poi `UnifyVisitor` per tipi composti.

#### 3.5 Error Handling Evaluation

Errori strutturati con `std::expected<void, CompileError>`. ErrorType silenziosamente ignorato — intenzionale. Occurs-check produce E2035. Type mismatch produce E2034.

#### 3.6 Type Consistency Audit

Nessun problema.

#### 3.7 Inter-Component Interaction

`ConstraintSolver` è invocato da `TypeChecker::solve_constraints` come oggetto temporaneo. Non condivide stato con altri solver.

#### 3.8 Optimization Opportunities

**Performance**: `solve()` crea un `ConstraintSolver` temporaneo ad ogni chiamata. Se la pipeline venisse eseguita più volte (es. incremental type checking), il riutilizzo del solver eviterebbe riallocazioni.

---

### System: Constraint Solving (S3) › Component: TypeVisitor

#### 3.1 Responsibility Statement

`TypeVisitor` fornisce un'interfaccia di visitor per la visita strutturale dei tipi composti (Array, Vector), evitando duplicazione della logica switch-on-TypeKind.

#### 3.2 Class Structure

| Membro           | Tipo                                  | Visibilità | Semantica    |
|------------------|---------------------------------------|------------|--------------|
| `~TypeVisitor()` | Distruttore virtuale                  | `public`   | Polimorfismo |
| `visit_array()`  | `virtual void(const ArrayType&) = 0`  | `public`   | Caso Array   |
| `visit_vector()` | `virtual void(const VectorType&) = 0` | `public`   | Caso Vector  |

#### 3.3 Interface Analysis

| Funzione                              | Signature | Precondizioni | Postcondizioni                    | Contract               |
|---------------------------------------|-----------|---------------|-----------------------------------|------------------------|
| `visit_type(TypeBase&, TypeVisitor&)` | `void`    | Nessuna       | Chiama metodo visitor appropriato | Dispatch su `TypeKind` |
| `visit_type(TypePtr, TypeVisitor&)`   | `void`    | Nessuna       | De-referenzia e dispatch          | Null-safe              |

#### 3.4 Implementation Logic

`visit_type` [`TypeVisitor.cpp:11–22`]: switch su `type.kind()` → dispatch al metodo virtuale appropriato. Per tipi non composti (Primitive, Custom, TypeVar, Error), è un no-op.

#### 3.5 Error Handling Evaluation

Nessun errore gestito — è un meccanismo di dispatch puro.

#### 3.6 Type Consistency Audit

Nessun problema.

#### 3.7 Inter-Component Interaction

Usato da `Substitution::applyImpl`, `ConstraintSolver::occurs_in`, `ConstraintSolver::unify`.

#### 3.8 Optimization Opportunities

**Estensibilità**: Se nuovi tipi composti vengono aggiunti a `TypeKind`, `visit_type` e `TypeVisitor` devono essere estesi. Questo è un punto di fragilità — un pattern CRTP o un visitor generico ridurrebbe il rischio.

---

### System: Name Resolution (S4) › Component: SymbolTable

#### 3.1 Responsibility Statement

`SymbolTable` gestisce binding identificatore→`TypeScheme` con scope lessicali annidati, supportando shadowing e lookup dall'interno verso l'esterno.

#### 3.2 Class Structure

| Membro       | Tipo                                                         | Visibilità | Semantica                         |
|--------------|--------------------------------------------------------------|------------|-----------------------------------|
| `scopes_`    | `vector<unordered_map<string_view, TypeScheme, StringHash>>` | `private`  | Stack di scope                    |
| `StringHash` | Struct con `is_transparent`                                  | `private`  | Hash per `string_view` eterogeneo |

#### 3.3 Interface Analysis

| Metodo                          | Signature                                      | Precondizioni | Postcondizioni                            | Contract                     |
|---------------------------------|------------------------------------------------|---------------|-------------------------------------------|------------------------------|
| `push_scope()`                  | `void()`                                       | Nessuna       | Nuovo scope vuoto                         |                              |
| `pop_scope()`                   | `void()`                                       | `depth() > 0` | Scope rimosso                             | Silent no-op se vuoto        |
| `define()`                      | `void(string_view, TypeScheme)`                | Nessuna       | Binding nel scope corrente                | Crea scope se vuoto          |
| `lookup()`                      | `optional<TypeScheme>(string_view) const`      | Nessuna       | Primo binding trovato o nullopt           | Dall'interno all'esterno     |
| `defined_in_current_scope()`    | `bool(string_view) const`                      | Nessuna       | true se nel scope corrente                |                              |
| `depth()`                       | `size_t() const noexcept`                      | Nessuna       | Numero scope attivi                       |                              |
| `set_function_return_context()` | `void(TypePtr, string)`                        | Nessuna       | Aggiorna return_type del binding funzione | Cerca contesto più recente   |
| `get_function_return_context()` | `optional<pair<TypePtr, string_view>>() const` | Nessuna       | Contesto funzione più vicina              | Dall'interno all'esterno     |

#### 3.4 Implementation Logic

`lookup()` [`SymbolTable.cpp:20–26`]:

```cpp
for(const auto &scope : std::ranges::reverse_view(scopes_)) {
    auto found = scope.find(name);
    if(found != scope.end()) { return found->second; }
}
return std::nullopt;
```

`set_function_return_context()` [`SymbolTable.cpp:37–53`]: inserisce un marker `__function_context__` nello scope corrente. Cerca dal più interno al più esterno.

#### 3.5 Error Handling Evaluation

`pop_scope()` è silent no-op se vuoto — dovrebbe assertare o lanciare. `define()` crea scope implicitamente se vuoto — comportamento nascosto.

#### 3.6 Type Consistency Audit

`string_view` come chiave — **DEF-010**: lifetime delle stringhe non garantita dal SymbolTable. Se il chiamante dealloca la stringa originale, la chiave diventa dangling.

#### 3.7 Inter-Component Interaction

Consultato da `TypeChecker::type_expr` per `Identifier` e da `type_stmt` per `VarDecl` e `FuncDecl`.

#### 3.8 Optimization Opportunities

**Robustezza**: `pop_scope()` dovrebbe assertare se vuoto. `define()` non dovrebbe creare scope implicitamente. Vedi **REC-011**.

---

### System: Type Checking Orchestration (S5) › Component: TypeChecker

#### 3.1 Responsibility Statement

`TypeChecker` orchestra l'intera pipeline di type checking — name resolution, constraint generation, constraint solving, e zonking — trasformando un AST non tipizzato in un AST completamente tipizzato.

#### 3.2 Class Structure

| Membro             | Tipo                                     | Visibilità | Semantica                                         |
|--------------------|------------------------------------------|------------|---------------------------------------------------|
| `symbols_`         | `SymbolTable`                            | `private`  | Symbol table corrente                             |
| `constraints_`     | `ConstraintSet`                          | `private`  | Vincoli accumulati                                |
| `errors_`          | `vector<CompileError>`                   | `private`  | Errori raccolti                                   |
| `message_storage_` | `deque<std::string>`                     | `private`  | Proprietario delle stringhe dei messaggi          |
| `typed_stmts_`     | `vector<TypedStmtPtr>`                   | `private`  | Statement tipizzati durante constraint generation |
| `function_decls_`  | `unordered_map<string, const FuncDecl*>` | `private`  | Mapping nome→dichiarazione funzione               |
| `loop_depth_`      | `size_t` (inizializzato a 0)             | `private`  | Profondità nesting loop                           |

#### 3.3 Interface Analysis

| Metodo        | Signature                         | Precondizioni      | Postcondizioni        | Contract                 |
|---------------|-----------------------------------|--------------------|-----------------------|--------------------------|
| `check()`     | `TypeCheckResult(const Program&)` | Programma valido   | TypedProgram + errori | Resetta stato interno    |
| `type_expr()` | `TypedExprPtr(const Expr&)`       | Espressione valida | Espressione tipizzata | Esposto per unit testing |
| `type_stmt()` | `TypedStmtPtr(const Stmt&)`       | Statement valido   | Statement tipizzato   | Esposto per unit testing |

#### 3.4 Implementation Logic

La pipeline in `check()` [`TypeChecker.cpp:70–88`] è lineare e ben strutturata. La complessità è concentrata in `type_expr()` (~430 righe di switch) e `type_stmt()` (~300 righe di switch).

`type_expr()` gestisce 17+ casi di `NodeKind`. I più complessi:

- `BinaryExpr`: controlli anticipati per numerici, booleani, bitwise, stringhe
- `CallExpr`: verifica arity, vincoli parametri, tipo di ritorno da FuncDecl
- `ArrayLiteral`: inferenza tipo elemento da primo elemento, consistenza
- `AssignExpr`: controllo immutabilità

`type_stmt()` gestisce 11 casi. I più complessi:

- `FuncDecl`: scope funzione, parametri, body, vincolo return type
- `ReturnStmt`: validazione contro contesto funzione
- `VarDecl` multi: semplificato a prima variabile

#### 3.5 Error Handling Evaluation

Errori accumulati in `errors_` con `message_storage_` come proprietario stringhe. `deque` garantisce stabilità iteratori ma la documentazione non lo dichiara esplicitamente.

**DEF-015 (ripreso)**: `zonk_block_full` scarta statement che restituiscono nullptr.

**DEF-018 (ripreso)**: Controlli anticipati duplicano logica del solver — errori doppi.

#### 3.6 Type Consistency Audit

Nessun problema formale.

#### 3.7 Inter-Component Interaction

`TypeChecker` è il consumer di TUTTI gli altri sistemi. Coordina S1–S4.

#### 3.8 Optimization Opportunities

**Strutturale**: `type_expr` e `type_stmt` devono essere refattorizzati. Ogni caso dello switch dovrebbe essere una funzione separata. Vedi **REC-001**.

**Performance**: `parse_type_annotation` è hardcoded — dovrebbe essere una lookup table o un metodo su `TypeKind`. Vedi **REC-004**.

---

## Phase 4 — Prioritized Recommendations

### 4.1 Recommendation Register

#### REC-001

**Title**: Refattorizzare `type_expr` in funzioni separate per caso NodeKind

**Deficiency Addressed**: **Phase 2 §2.5 DEF-014/DEF-018** — `TypeChecker::type_expr` è una funzione di ~430 righe con switch su 17+ NodeKind, violando il limite CCN ≤15 e il principio di singola responsabilità. I controlli anticipati duplicano la logica del solver.

**Description**: **Change entry point**: `src/jsav_Lib/typechecker/TypeChecker.cpp`, metodo `TypeChecker::type_expr`. Estrarre ogni caso dello switch in una funzione membro privata dedicata (es. `type_binary_expr`, `type_call_expr`, `type_array_literal`). Ogni funzione riceve il puntatore al nodo specifico e restituisce `TypedExprPtr`. Lo switch nel `type_expr` diventa un dispatcher di 2-3 righe per caso. Questo riduce la complessità cognitiva da CCN >50 a CCN <10 per funzione. I controlli anticipati (es. `!lhs_type->is_numeric()`) dovrebbero essere rimossi o documentati come "early error" separati dal solver, per evitare errori duplicati.

**Feasibility Score**: 4 — Esecutibile nello sprint corrente; richiede refactoring meccanico senza cambiamenti architetturali.

**Expected ROI**: 5 — Impatto trasformativo: riduce CCN da >50 a <10, rendendo il codice testabile, leggibile e manutenibile.

**Implementation Effort**: 3 — 2–6 settimane; ~17 funzioni da estrarre, ciascuna con test di regressione.

**Priority Rank**: (4 × 2) + (5 × 2) + (3 × 1) = 8 + 10 + 3 = **21**

**Estimated Implementation Time**: 2–4 settimane

**Required Resources**:

- Ruoli: 1 senior C++ engineer con esperienza in refactoring
- Tools: clang-tidy per verifica CCN, test suite esistente per regressione
- Access: nessuno speciale
- External: nessuno

**Effectiveness Indicators**:

1. CCN di ogni funzione estratta ≤15 come riportato da `lizard` dopo il refactoring.
2. Zero regressioni: tutti i test esistenti passano dopo il refactoring.
3. Lunghezza massima funzione in `TypeChecker.cpp` ≤100 righe.

---

#### REC-002

**Title**: Correggere perdita statement in `zonk_block_full`

**Deficiency Addressed**: **Phase 2 §2.5 DEF-015** — `zonk_block_full` scarta silenziosamente statement quando `zonk_stmt_full` restituisce `nullptr`, corrompendo l'AST tipizzato.

**Description**: **Change entry point**: `src/jsav_Lib/typechecker/TypeChecker.cpp`, metodo `TypeChecker::zonk_block_full` (linee 413–421). Quando `zonk_stmt_full` restituisce `nullptr`, invece di scartare lo statement, registrare un errore `CompileError::TypeError(E2034, "Failed to zonk statement", ...)` e inserire un placeholder `TypedExprStmt` con `ErrorType`. Questo previene la corruzione silente dell'AST e rende l'errore visibile all'utente.

**Feasibility Score**: 5 — Immediatamente eseguibile; modifica di ~10 righe in una singola funzione.

**Expected ROI**: 4 — Impatto significativo: previene corruzione AST silente in una code path frequentemente esercitata.

**Implementation Effort**: 5 — Minimale: ore, non giorni.

**Priority Rank**: (5 × 2) + (4 × 2) + (5 × 1) = 10 + 8 + 5 = **23**

**Estimated Implementation Time**: 2–4 ore

**Required Resources**:

- Ruoli: 1 C++ engineer
- Tools: nessuno speciale
- Access: nessuno
- External: nessuno

**Effectiveness Indicators**:

1. Zero statement persi durante zonking verificato con test che includono statement che producono nullptr.
2. Errore E2034 correttamente riportato per statement non zoncabili.

---

#### REC-003

**Title**: Introdurre tipo funzione `FnType` per signature esplicite

**Deficiency Addressed**: **Phase 1 §1.4 DEF-003** — Mancanza di `FnType` impedisce la rappresentazione esplicita dei tipi funzione, limitando la verifica delle chiamate.

**Description**: **Change entry point**: `include/jsav/ast/Type.hpp`. Aggiungere classe `FnType` con campi `return_type: TypePtr`, `param_types: std::vector<TypePtr>`, e `is_variadic: bool`. Aggiornare `TypeKind` con `Fn`. Aggiornare `visit_type` in `TypeVisitor.hpp/cpp` con `visit_fn`. Aggiornare `Substitution::applyImpl`, `ConstraintSolver::UnifyVisitor`, `TypeChecker::zonk` per gestire `FnType`. In `type_expr` per `CallExpr`, vincolare il callee type alla signature `FnType` invece di usare una variabile fresca.

**Feasibility Score**: 3 — Richiede coordinamento: modifiche a S1, S3, S5 simultaneamente.

**Expected ROI**: 5 — Impatto trasformativo: abilita type checking completo delle chiamate funzione, incluyendo arity e tipo di ritorno.

**Implementation Effort**: 1 — Very high: multi-settimana, redesign architetturale.

**Priority Rank**: (3 × 2) + (5 × 2) + (1 × 1) = 6 + 10 + 1 = **17**

**Estimated Implementation Time**: 3–6 settimane

**Required Resources**:

- Ruoli: 1 senior compiler engineer
- Tools: test suite estesa per chiamate funzione
- Access: nessuno
- External: nessuno

**Effectiveness Indicators**:

1. Test per chiamate con arity errato producono errore E2028.
2. Test per mismatch tipo parametro/argomento producono errore di tipo.
3. `FnType` appare in `to_string()` per signature funzione.

---

#### REC-004

**Title**: Centralizzare parsing annotazioni tipo in `Type.hpp`

**Deficiency Addressed**: **Phase 2 §2.5 DEF-004** — `parse_type_annotation` è hardcoded in `TypeChecker.cpp` con if-else chain, richiedendo modifica manuale per nuovi tipi.

**Description**: **Change entry point**: `include/jsav/ast/Type.hpp`. Aggiungere funzione `parse_type_annotation(std::string_view)` come funzione libera namespace `jsv` o metodo statico su `PrimitiveType`. Usare una `std::unordered_map<std::string_view, std::function<TypePtr()>>` o un array ordinato + binary search per mapping nome→factory. Rimuovere la duplicata da `TypeChecker.cpp` e includere la nuova funzione.

**Feasibility Score**: 5 — Immediatamente eseguibile; refactoring localizzato.

**Expected ROI**: 3 — Impatto moderato: migliora manutenibilità e riduce duplicazione.

**Implementation Effort**: 5 — Minimale: poche ore.

**Priority Rank**: (5 × 2) + (3 × 2) + (5 × 1) = 10 + 6 + 5 = **21**

**Estimated Implementation Time**: 4–8 ore

**Required Resources**:

- Ruoli: 1 C++ engineer
- Tools: nessuno
- Access: nessuno
- External: nessuno

**Effectiveness Indicators**:

1. Unica definizione di mapping stringa→tipo in tutto il codebase.
2. Nuovo tipo primitivo richiede modifica in un solo punto.

---

#### REC-005

**Title**: Sostituire `at()` con `find()` in `UnionFind` per prevenire crash

**Deficiency Addressed**: **Phase 2 §2.5 DEF-008** — `UnionFind::find()` e `unite()` usano `unordered_map::at()` che lancia `std::out_of_range` per chiavi non registrate.

**Description**: **Change entry point**: `src/jsav_Lib/typechecker/UnionFind.cpp`. In `find()`, sostituire `parent_.at(var)` con `parent_.find(var)` + `assert(it != parent_.end())`. In `unite()`, sostituire `rank_.at(root_x)` con `rank_.find(root_x)->second`. Questo trasforma un'eccezione non gestita in un assertion failure (in Debug) o undefined behavior controllata (in Release, ma con invariant violato esplicitamente documentato).

**Feasibility Score**: 5 — Immediatamente eseguibile; poche righe.

**Expected ROI**: 4 — Impatto significativo: previene crash a runtime per bug del chiamante.

**Implementation Effort**: 5 — Minimale: ore.

**Priority Rank**: (5 × 2) + (4 × 2) + (5 × 1) = 10 + 8 + 5 = **23**

**Estimated Implementation Time**: 1–2 ore

**Required Resources**:

- Ruoli: 1 C++ engineer
- Tools: nessuno
- Access: nessuno
- External: nessuno

**Effectiveness Indicators**:

1. Zero `std::out_of_range` durante esecuzione test con sanitizer enabled.
2. Assertion failure con messaggio chiaro per variabili non registrate.

---

#### REC-006

**Title**: Completare `TypeScheme::instantiate()` con visitor di sostituzione

**Deficiency Addressed**: **Phase 2 §2.5 DEF-006** — `TypeScheme::instantiate()` non gestisce tipi composti, lasciando variabili quantificate non sostituite.

**Description**: **Change entry point**: `src/jsav_Lib/typechecker/TypeScheme.cpp`. Scrivere un visitor (es. `InstantiateVisitor`) che attraversa ricorsivamente il `body` e sostituisce ogni `TypeVariable` il cui ID è in `quantified_vars` con la corrispondente variabile fresca. Per `ArrayType` e `VectorType`, visitare ricorsivamente l'element type. Per `CustomType` e `PrimitiveType`, restituire invariato. Il risultato è un nuovo tipo con tutte le occorrenze delle variabili quantificate sostituite.

**Feasibility Score**: 4 — Esecutibile con preparazione minima; richiede understanding del visitor pattern già esistente.

**Expected ROI**: 5 — Impatto trasformativo: abilita polimorfismo corretto per signature composte.

**Implementation Effort**: 4 — Low: sotto due settimane.

**Priority Rank**: (4 × 2) + (5 × 2) + (4 × 1) = 8 + 10 + 4 = **22**

**Estimated Implementation Time**: 3–5 giorni

**Required Resources**:

- Ruoli: 1 C++ engineer con conoscenza Hindley-Milner
- Tools: test per polimorfismo composto
- Access: nessuno
- External: nessuno

**Effectiveness Indicators**:

1. Test per `∀T. Vec<T>` istanziato produce `Vec<?Tfresh>` con variabile fresca.
2. Zero variabili quantificate non sostituite in programmi polimorfici.

---

#### REC-007

**Title**: Unificare strategia error propagation con `DiagnosticBag`

**Deficiency Addressed**: **Phase 1 §1.4 DEF-001** — Quattro strategie diverse di propagazione errori (`ErrorType`, `std::expected`, `std::nullopt`, `vector<CompileError>`).

**Description**: **Change entry point**: `include/jsav/error/CompileError.hpp`. Creare classe `DiagnosticBag` con metodi `add(CompileError)`, `errors()`, `has_errors()`. Modificare `TypeChecker` per usare `DiagnosticBag` invece di `vector<CompileError>` + `message_storage_`. `SymbolTable::lookup` dovrebbe restituire `std::expected<TypeScheme, DiagnosticRef>` invece di `std::nullopt`. Questo centralizza la gestione errori in un unico componente con interfaccia stabile.

**Feasibility Score**: 2 — Richiede modifica a S2, S4, S5 simultaneamente; coordinamento significativo.

**Expected ROI**: 4 — Impatto significativo: coerenza architetturale e manutenibilità.

**Implementation Effort**: 1 — Very high: redesign multi-sistema.

**Priority Rank**: (2 × 2) + (4 × 2) + (1 × 1) = 4 + 8 + 1 = **13**

**Estimated Implementation Time**: 4–8 settimane

**Required Resources**:

- Ruoli: 1 senior compiler engineer + 1 QA
- Tools: test regression estesa
- Access: nessuno
- External: nessuno

**Effectiveness Indicators**:

1. Single entry point per aggiunta errori: `DiagnosticBag::add()`.
2. Zero uso di `std::nullopt` silente per errori di lookup.

---

#### REC-008

**Title**: Documentare invariant di `message_storage_` e `string_view` lifetime

**Deficiency Addressed**: **Phase 2 §2.5 DEF-016** — `message_storage_` (`deque<string>`) possiede stringhe referenziate da `string_view` in `CompileError`, ma l'invariante non è documentato.

**Description**: **Change entry point**: `include/jsav/typechecker/TypeChecker.hpp`, campo `message_storage_`. Aggiungere commento Doxygen che documenta l'invariante: "`deque` garantisce stabilità degli indirizzi degli elementi dopo inserimento (a differenza di `vector`). I `string_view` in `errors_` puntano a `message_storage_.back()`. Non usare `vector` per `message_storage_`." Aggiungere `static_assert` o test che verifichi la stabilità degli iteratori `deque`.

**Feasibility Score**: 5 — Immediatamente eseguibile; solo documentazione.

**Expected ROI**: 2 — Impatto minore: previene bug futuri da refactoring errato.

**Implementation Effort**: 5 — Minimale: ore.

**Priority Rank**: (5 × 2) + (2 × 2) + (5 × 1) = 10 + 4 + 5 = **19**

**Estimated Implementation Time**: 1–2 ore

**Required Resources**:

- Ruoli: 1 C++ engineer
- Tools: nessuno
- Access: nessuno
- External: nessuno

**Effectiveness Indicators**:

1. Campo `message_storage_` documentato con Doxygen.
2. Test che verifica stabilità `string_view` dopo 1000 inserimenti.

---

#### REC-009

**Title**: Esporre `reset_type_var_counter()` per compilazioni multiple

**Deficiency Addressed**: **Phase 3 §3.8 (TypeVariable)** — `fresh_type_variable()` usa counter thread-local che non resetta, causando IDs crescenti indefinitamente.

**Description**: **Change entry point**: `include/jsav/typechecker/TypeVariable.hpp`, funzione `fresh_type_variable()`. Aggiungere funzione `void reset_type_var_counter() noexcept` che resetta il counter thread-local a 0. Chiamare da `TypeChecker::check()` all'inizio della pipeline. Questo garantisce IDs ripartono da 1 per ogni unità di compilazione.

**Feasibility Score**: 5 — Immediatamente eseguibile; aggiunta di una funzione.

**Expected ROI**: 2 — Impatto minore: utile solo per compilazioni multiple nello stesso processo.

**Implementation Effort**: 5 — Minimale: ore.

**Priority Rank**: (5 × 2) + (2 × 2) + (5 × 1) = 10 + 4 + 5 = **19**

**Estimated Implementation Time**: 1–2 ore

**Required Resources**:

- Ruoli: 1 C++ engineer
- Tools: nessuno
- Access: nessuno
- External: nessuno

**Effectiveness Indicators**:

1. IDs variabili tipo ripartono da 1 dopo ogni chiamata a `check()`.
2. Test per compilazioni multiple nello stesso processo.

---

#### REC-010

**Title**: Correggere `ArrayType::sizes_equal` per espressioni non letterali

**Deficiency Addressed**: **Phase 2 §2.5 DEF-005** — `sizes_equal` gestisce solo `IntegerLiteral`, fallendo silenziosamente per espressioni complesse.

**Description**: **Change entry point**: `src/jsav_Lib/ast/Type.cpp`, funzione `ArrayType::sizes_equal`. Implementare confronto strutturale AST ricorsivo: se entrambe sono `IntegerLiteral`, confrontare valori; se entrambe sono `BinaryExpr`, confrontare operator e operandi ricorsivamente; altrimenti restituire `false` con warning. In alternativa, valutare le espressioni a compile-time se constexpr.

**Feasibility Score**: 3 — Richiede implementazione di visitor AST per confronto strutturale.

**Expected ROI**: 3 — Impatto moderato: migliora correttezza per array con dimensioni complesse.

**Implementation Effort**: 3 — Moderate: 2–6 settimane.

**Priority Rank**: (3 × 2) + (3 × 2) + (3 × 1) = 6 + 6 + 3 = **15**

**Estimated Implementation Time**: 1–3 settimane

**Required Resources**:

- Ruoli: 1 C++ engineer
- Tools: AST visitor per confronto strutturale
- Access: nessuno
- External: nessuno

**Effectiveness Indicators**:

1. `ArrayType` con dimensioni `2+1` e `3` considerati uguali.
2. Zero fallimenti silenziosi per espressioni non letterali.

---

#### REC-011

**Title**: Eliminare creazione implicita scope in `SymbolTable::define()`

**Deficiency Addressed**: **Phase 2 §2.5 DEF-011** — `define()` crea scope implicitamente se vuoto, mascherando bug del chiamante.

**Description**: **Change entry point**: `src/jsav_Lib/typechecker/SymbolTable.cpp`, metodo `SymbolTable::define`. Rimuovere il blocco `if(scopes_.empty()) { scopes_.emplace_back(); }`. Aggiungere `assert(!scopes_.empty() && "push_scope() must be called before define()")`. Questo forza il chiamante a esplicitare la gestione degli scope.

**Feasibility Score**: 5 — Immediatamente eseguibile; modifica di 2 righe.

**Expected ROI**: 3 — Impatto moderato: previene bug sottili da scope management errato.

**Implementation Effort**: 5 — Minimale: ore.

**Priority Rank**: (5 × 2) + (3 × 2) + (5 × 1) = 10 + 6 + 5 = **21**

**Estimated Implementation Time**: 1–2 ore

**Required Resources**:

- Ruoli: 1 C++ engineer
- Tools: nessuno
- Access: nessuno
- External: nessuno

**Effectiveness Indicators**:

1. Assertion failure se `define()` chiamato senza `push_scope()`.
2. Zero scope creati implicitamente in produzione.

---

#### REC-012

**Title**: Separare `Type.hpp` in header modulari per sottoclasse

**Deficiency Addressed**: **Phase 2 §2.2** — `Type.hpp` è un God-class header con 5 classi + enum + formatter.

**Description**: **Change entry point**: `include/jsav/ast/Type.hpp`. Creare file separati: `TypeBase.hpp`, `PrimitiveType.hpp`, `CustomType.hpp`, `ArrayType.hpp`, `VectorType.hpp`. Ogni file include `TypeBase.hpp` e dichiara una sola classe. Creare `Type.hpp` come umbrella header che include tutti. Aggiornare tutti gli `#include "Type.hpp"` nei file del type checker (nessun cambiamento per i consumer).

**Feasibility Score**: 4 — Esecutibile con preparazione: richiede aggiornamento include path.

**Expected ROI**: 3 — Impatto moderato: riduce dipendenze di compilazione.

**Implementation Effort**: 3 — Moderate: 2–6 settimane per testare tutti i path include.

**Priority Rank**: (4 × 2) + (3 × 2) + (3 × 1) = 8 + 6 + 3 = **17**

**Estimated Implementation Time**: 1–2 settimane

**Required Resources**:

- Ruoli: 1 C++ engineer
- Tools: include-what-you-use per verifica
- Access: nessuno
- External: nessuno

**Effectiveness Indicators**:

1. Zero includenti rotti dopo refactoring.
2. Tempo di compilazione ridotto del 5%+ per file che include solo una sottoclasse.

---

### 4.2 Summary Priority Table

| Rank | ID                  | Title                                              | Feasibility | ROI | Effort | Composite Score | Est. Time |
|------|---------------------|----------------------------------------------------|-------------|-----|--------|-----------------|-----------|
| 1    | [REC-002](#rec-002) | Correggere perdita statement in `zonk_block_full`  | 5           | 4   | 5      | 23              | 2–4 hrs   |
| 2    | [REC-005](#rec-005) | Sostituire `at()` con `find()` in `UnionFind`      | 5           | 4   | 5      | 23              | 1–2 hrs   |
| 3    | [REC-006](#rec-006) | Completare `TypeScheme::instantiate()` con visitor | 4           | 5   | 4      | 22              | 3–5 days  |
| 4    | [REC-001](#rec-001) | Refattorizzare `type_expr` in funzioni separate    | 4           | 5   | 3      | 21              | 2–4 wks   |
| 5    | [REC-004](#rec-004) | Centralizzare parsing annotazioni tipo             | 5           | 3   | 5      | 21              | 4–8 hrs   |
| 6    | [REC-011](#rec-011) | Eliminare creazione implicita scope in `define()`  | 5           | 3   | 5      | 21              | 1–2 hrs   |
| 7    | [REC-008](#rec-008) | Documentare invariant `message_storage_`           | 5           | 2   | 5      | 19              | 1–2 hrs   |
| 8    | [REC-009](#rec-009) | Esporre `reset_type_var_counter()`                 | 5           | 2   | 5      | 19              | 1–2 hrs   |
| 9    | [REC-003](#rec-003) | Introdurre tipo funzione `FnType`                  | 3           | 5   | 1      | 17              | 3–6 wks   |
| 10   | [REC-012](#rec-012) | Separare `Type.hpp` in header modulari             | 4           | 3   | 3      | 17              | 1–2 wks   |
| 11   | [REC-010](#rec-010) | Correggere `ArrayType::sizes_equal`                | 3           | 3   | 3      | 15              | 1–3 wks   |
| 12   | [REC-007](#rec-007) | Unificare error propagation con `DiagnosticBag`    | 2           | 4   | 1      | 13              | 4–8 wks   |

---

## Appendix — Deficiency-to-Recommendation Traceability

| DEF ID  | Finding                                           | Recommendation           |
|---------|---------------------------------------------------|--------------------------|
| DEF-001 | Propagazione errori inconsistente                 | REC-007                  |
| DEF-002 | Contesto funzione duplicato                       | (mitigato da REC-001)    |
| DEF-003 | Mancanza di `FnType`                              | REC-003                  |
| DEF-004 | `parse_type_annotation` hardcoded                 | REC-004                  |
| DEF-005 | `ArrayType::sizes_equal` limitato                 | REC-010                  |
| DEF-006 | `TypeScheme::instantiate()` incompleto            | REC-006                  |
| DEF-007 | `TypeScheme` campi mutabili non documentati       | (mitigato da REC-006)    |
| DEF-008 | `UnionFind::find()` usa `at()` con eccezione      | REC-005                  |
| DEF-009 | `UnifyVisitor` non gestisce tutti i casi composti | (mitigato da REC-003)    |
| DEF-010 | `StringHash` con `string_view` e ownership        | (documentare in REC-008) |
| DEF-011 | `define()` crea scope implicitamente              | REC-011                  |
| DEF-013 | `CallExpr` con gestione signature parziale        | REC-003                  |
| DEF-014 | `VarDecl` multi-variable semplificato             | (fuori scope — feature)  |
| DEF-015 | `zonk_block_full` perde statement                 | REC-002                  |
| DEF-016 | `message_storage_` fragile                        | REC-008                  |
| DEF-018 | Type checking anticipato duplica logica           | REC-001                  |
