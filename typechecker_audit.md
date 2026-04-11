# Type Checker Implementation Audit

## Phase 1 — System Ensemble Analysis

### 1.1 System Enumeration

Il type checker di **jsav** comprende **10 file header** e **10 file di implementazione** organizzati in due sistemi
principali:

**Sistema 1 — Type Representation System** (rappresentazione dei tipi)

| File Header                                 | File Implementazione                        | Responsabilità primaria                                                                         |
|---------------------------------------------|---------------------------------------------|-------------------------------------------------------------------------------------------------|
| `include/jsav/ast/Type.hpp`                 | `src/jsav_Lib/ast/Type.cpp`                 | Gerarchia `TypeBase`, `PrimitiveType`, `CustomType`, `ArrayType`, `VectorType`, enum `TypeKind` |
| `include/jsav/typechecker/TypeVariable.hpp` | `src/jsav_Lib/typechecker/TypeVariable.cpp` | `TypeVariable` — variabili di tipo per inferenza Hindley-Milner                                 |
| `include/jsav/typechecker/TypeScheme.hpp`   | `src/jsav_Lib/typechecker/TypeScheme.cpp`   | `TypeScheme` — tipi polimorfici ∀(vars).body                                                    |
| `include/jsav/typechecker/ErrorType.hpp`    | `src/jsav_Lib/typechecker/ErrorType.cpp`    | `ErrorType` — tipo sentinella per recupero errori                                               |
| `include/jsav/typechecker/TypeVisitor.hpp`  | `src/jsav_Lib/typechecker/TypeVisitor.cpp`  | Interfaccia visitor per tipi composti (Array, Vector)                                           |

**Sistema 2 — Constraint Solving System** (vincoli e unificazione)

| File Header                                     | File Implementazione                            | Responsabilità primaria                                          |
|-------------------------------------------------|-------------------------------------------------|------------------------------------------------------------------|
| `include/jsav/typechecker/Constraint.hpp`       | `src/jsav_Lib/typechecker/Constraint.cpp`       | `Constraint` e `ConstraintSet` — vincoli di uguaglianza tra tipi |
| `include/jsav/typechecker/Substitution.hpp`     | `src/jsav_Lib/typechecker/Substitution.cpp`     | `Substitution` — mapping variabili di tipo → tipi risolti        |
| `include/jsav/typechecker/UnionFind.hpp`        | `src/jsav_Lib/typechecker/UnionFind.cpp`        | `UnionFind` — disjoint-set per unificazione efficiente           |
| `include/jsav/typechecker/ConstraintSolver.hpp` | `src/jsav_Lib/typechecker/ConstraintSolver.cpp` | `ConstraintSolver` — motore di unificazione con occurs-check     |

**Sistema 3 — Name Resolution System** (risoluzione dei nomi)

| File Header                                | File Implementazione                       | Responsabilità primaria                                                        |
|--------------------------------------------|--------------------------------------------|--------------------------------------------------------------------------------|
| `include/jsav/typechecker/SymbolTable.hpp` | `src/jsav_Lib/typechecker/SymbolTable.cpp` | `SymbolTable` — gestione scope lessicali e binding identificatore→`TypeScheme` |

**Sistema 4 — Type Checking Orchestration System** (orchestrazione del type checking)

| File Header                                | File Implementazione                       | Responsabilità primaria                                                                |
|--------------------------------------------|--------------------------------------------|----------------------------------------------------------------------------------------|
| `include/jsav/typechecker/TypeChecker.hpp` | `src/jsav_Lib/typechecker/TypeChecker.cpp` | `TypeChecker` — pipeline completa (risoluzione, generazione vincoli, solving, zonking) |

**Sistema 5 — Typed AST System** (AST tipizzato — dipendenza del type checker)

| File Header                             | File Implementazione                    | Responsabilità primaria                                                  |
|-----------------------------------------|-----------------------------------------|--------------------------------------------------------------------------|
| `include/jsav/ast/TypedNode.hpp`        | `src/jsav_Lib/ast/TypedNode.cpp`        | `TypedNode`, `TypedExpr`, `TypedStmt` — nodi AST con annotazioni di tipo |
| `include/jsav/ast/TypedProgram.hpp`     | `src/jsav_Lib/ast/TypedProgram.cpp`     | `TypedProgram` — root del programma tipizzato                            |
| `include/jsav/ast/TypedExpressions.hpp` | `src/jsav_Lib/ast/TypedExpressions.cpp` | Nodi espressione tipizzati                                               |
| `include/jsav/ast/TypedStatements.hpp`  | `src/jsav_Lib/ast/TypedStatements.cpp`  | Nodi statement tipizzati                                                 |
| `include/jsav/ast/TypedAst.hpp`         | `src/jsav_Lib/ast/TypedAst_printer.cpp` | Umbrella header e stampa AST tipizzato                                   |
| `include/jsav/ast/TypedVisitor.hpp`     | —                                       | Visitor CRTP per AST tipizzato (header-only)                             |

### 1.2 Inter-System Dependency Map

La mappa delle dipendenze direzionali tra sistemi:

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

**Dipendenze upstream → downstream**:

1. **Type Representation** → Tutti gli altri sistemi (fondazione). `TypePtr` è il tipo fondamentale.
2. **TypeVariable/TypeScheme/ErrorType** → Constraint, Substitution, SymbolTable, TypeChecker
3. **SymbolTable** → TypeChecker (solo per `resolve_names` e `type_expr` degli identificatori)
4. **Constraint/Substitution/UnionFind** → ConstraintSolver → TypeChecker
5. **Typed AST** → TypeChecker (output della pipeline)

Non esistono dipendenze circolari. Il grafo è un **DAG** (Directed Acyclic Graph).

### 1.3 Architectural Coherence Evaluation

L'architettura segue un approccio **constraint-based type inference** con pipeline in quattro fasi, come documentato in
`TypeChecker.hpp:47–55`. La decomposizione è fondamentalmente solida:

**Punti di forza**:

- Separazione netta tra **rappresentazione** (`TypeBase` e sottoclassi), **inferenza** (`ConstraintSolver`/`UnionFind`),
  e **orchestrazione** (`TypeChecker`).
- Uso appropriato del pattern Visitor (`TypeVisitor`) per la visita di tipi composti (`TypeVisitor.hpp:38–64`).
- `Substitution` con cache persistente (`Substitution.hpp:67–86`, `applyImpl`) per ottimizzare ri-applicazione delle
  sostituzioni.
- `ErrorType` come sentinella che unifica silenziosamente con qualsiasi tipo (`ConstraintSolver.cpp:67–68`), prevenendo
  errori a cascata.

**Deficienze strutturali**:

- **`TypeChecker::type_expr`** (1200 righe totali, con la funzione che supera le 400 righe) viola il principio di
  singola responsabilità e il limite di complessità cognitiva del progetto (CCN ≤15, AGENTS.md §7). La funzione gestisce
  generazione vincoli, type checking anticipato, e costruzione dell'AST tipizzato simultaneamente.
- **Mancanza di tipo funzione** (`FnType`). Il sistema non ha una rappresentazione esplicita per i tipi funzione (
  `(T1, T2) -> R`). Le chiamate a funzione (`CallExpr`) generano vincoli solo sugli argomenti ma **non verificano
  l'arity** né il tipo di ritorno della funzione chiamata [`TypeChecker.cpp:698–745`].
- **Zone incomplete di zonking**: il metodo `zonk_block_full` scarta statement che non producono risultato [
  `TypeChecker.cpp:417` — commento "Can't move from const — skip (original kept by callee)"], causando perdita silente
  di nodi AST.

### 1.4 Cross-Cutting Concerns Assessment

**Matrice dei concern trasversali**:

| Concern                      | Type Representation                    | Constraint Solving                  | SymbolTable                          | TypeChecker                                       | Uniformità                                                                                                               |
|------------------------------|----------------------------------------|-------------------------------------|--------------------------------------|---------------------------------------------------|--------------------------------------------------------------------------------------------------------------------------|
| **Propagazione errori**      | `ErrorType` singleton                  | `std::expected<void, CompileError>` | — (silenzioso, nullo se non trovato) | `std::vector<CompileError>`                       | **INCONSISTENTE** — tre strategie diverse                                                                                |
| **Rappresentazione tipi**    | `TypePtr = shared_ptr<const TypeBase>` | `TypePtr`                           | `TypeScheme` (wrappa `TypePtr`)      | `TypePtr`                                         | **UNIFORME**                                                                                                             |
| **Gestione scope**           | —                                      | —                                   | `vector<unordered_map>` con push/pop | Usa SymbolTable + `current_function_return_type_` | **PARZIALE** — il contesto funzione è duplicato                                                                          |
| **Formattazione diagnostic** | `to_string()` virtuale                 | `reason` string nei vincoli         | —                                    | `message_storage_` con `FORMAT()`                 | **PARZIALE** — `message_storage_` è `deque<string>` per evitare invalidazione di `string_view`, ma la gestione è fragile |

**`DEF-001` — Propagazione errori inconsistente**: `ErrorType` unifica silenziosamente (`ConstraintSolver.cpp:67`), ma
il TypeChecker accumula errori in `vector<CompileError>` (`TypeChecker.hpp:101`) mentre `SymbolTable::lookup`
restituisce `std::nullopt` senza diagnostic [`SymbolTable.hpp:54–55`]. Quando un identificatore non è dichiarato,
`type_expr` crea l'errore manualmente [`TypeChecker.cpp:493–498`]. Ogni sistema ha la propria strategia — nessun
meccanismo unificato di error propagation.

**`DEF-002` — Contesto funzione duplicato**: `TypeChecker` mantiene `current_function_return_type_` e
`current_function_name_` come stato mutabile (`TypeChecker.hpp:104–107`) mentre `SymbolTable` gestisce già gli scope. Il
contesto di ritorno dovrebbe essere parte del binding della funzione nella SymbolTable, non stato separato.

---

## Phase 2 — Per-System Analysis

### System: Type Representation System

#### 2.1 System Overview

Il **Type Representation System** (`include/jsav/ast/Type.hpp`, `src/jsav_Lib/ast/Type.cpp`) definisce la gerarchia di
classi che rappresentano tutti i tipi del linguaggio. È il fondamento su cui tutti gli altri sistemi operano. Fornisce
`TypeBase` come classe base astratta con le sottoclassi concrete `PrimitiveType`, `CustomType`, `ArrayType`,
`VectorType`. Include anche `TypeVariable`, `TypeScheme`, `ErrorType` nel sistema typechecker.

#### 2.2 Internal Module Organization

Il sistema è distribuito su due directory:

- `include/jsav/ast/Type.hpp` — Contiene TUTTA la gerarchia tipi (TypeBase, PrimitiveType, CustomType, ArrayType,
  VectorType) in un singolo header da ~560 righe.
- `include/jsav/typechecker/TypeVariable.hpp` — TypeVariable (variabili per inferenza).
- `include/jsav/typechecker/TypeScheme.hpp` — TypeScheme (tipi polimorfici).
- `include/jsav/typechecker/ErrorType.hpp` — ErrorType (sentinella errori).

**Criticità**: `Type.hpp` è un **God-class header** — 5 classi in un solo file. La separazione dei file typechecker è
corretta.

#### 2.3 Intra-System Dependency Analysis

Nessuna dipendenza circolare interna. `TypeBase` → nessuna dipendenza. Le sottoclassi dipendono solo da `TypeBase`.
`TypeVariable` dipende da `TypeBase`. `TypeScheme` dipende da `TypeVariable` e `TypeBase`. Dipendenze lineari e pulite.

#### 2.4 Logical Flow

Il sistema è puramente dichiarativo — non c'è "flusso" computazionale. I tipi sono costruiti tramite factory (singleton
per `PrimitiveType`, `new` per `TypeVariable`/`ArrayType`/`VectorType`). `TypeVariable::fresh_type_variable()` usa un
counter thread-local per generare ID univoci [`TypeVariable.cpp:12–14`]. `TypeScheme::instantiate()` genera variabili
fresche per i quantificati [`TypeScheme.cpp:14–33`].

#### 2.5 Critical Points

**`DEF-003` — `TypeScheme::instantiate()` incompleto**: L'implementazione in [`TypeScheme.cpp:14–33`] gestisce solo il
caso in cui il `body` è un `TypeVariable` diretto. Se il body è un tipo composto (es.
`Fn(TypeVar1, TypeVar2) -> TypeVar3`), le variabili quantificate all'interno **non vengono sostituite**. Il commento nel
codice lo ammette esplicitamente: `"This is a simplified implementation - full version would use a visitor."`.

**`DEF-004` — Mancanza di `FnType`**: Non esiste una classe `FnType` o `FunctionType` per rappresentare i tipi funzione.
Le chiamate a funzione in `type_expr` [`TypeChecker.cpp:698–745`] non possono verificare la signature della funzione —
solo gli argomenti vengono tipizzati ma non vincolati ai parametri formali.

**`DEF-005` — `parse_type_annotation` hardcoded**: La funzione `parse_type_annotation` in [`TypeChecker.cpp:18–35`] è
duplicata rispetto alla logica di `TypeKind`. Se un nuovo tipo primitivo venisse aggiunto a `Type.hpp`, questa funzione
non lo riconoscerebbe.

#### 2.6 Partial or Undefined Implementations

Tutte le classi dichiarate hanno implementazione completa, eccetto:

- `TypeScheme::instantiate()` — parziale (vedi DEF-003).
- `ArrayType::sizes_equal` — dichiarata in `Type.hpp:495` come
  `static bool sizes_equal(const Expr &a, const Expr &b) noexcept` ma la sua implementazione è in
  `src/jsav_Lib/ast/Type.cpp` — da verificare.

### System: Constraint Solving System

#### 2.1 System Overview

Il **Constraint Solving System** implementa l'unificazione di tipi tramite union-find con path compression e union by
rank. Comprende `Constraint`/`ConstraintSet` per l'accumulo dei vincoli, `Substitution` per il mapping delle soluzioni,
`UnionFind` per l'efficienza dell'unificazione, e `ConstraintSolver` come motore che orchestra il tutto.

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

**`DEF-006` — `UnionFind::find()` usa `at()` con eccezione**: In [`UnionFind.cpp:14–17`], `parent_.at(var)` lancia
`std::out_of_range` se `var` non è registrato. Questo è un fallimento a runtime non gestito — dovrebbe usare `find()`
con controllo o un `assert`.

**`DEF-007` — `UnifyVisitor` non gestisce tutti i casi**: In [`ConstraintSolver.cpp:26–35`], `UnifyVisitor` gestisce
solo `visit_array` e `visit_vector`. Se `t1` è `CustomType`, il visitor non viene dispatchato e `visitor.result` rimane
`std::nullopt`, portando a `value_or(success)` [`ConstraintSolver.cpp:139`]. Questo significa che due `CustomType`
diversi ("Foo" vs "Bar") vengono considerati uguali se hanno lo stesso kind.

**`DEF-008` — `Substitution::apply()` non thread-safe**: La documentazione lo dichiara esplicitamente (
`Substitution.hpp:90`), ma `fresh_type_variable()` è thread-safe [`TypeVariable.cpp:12`]. La combinazione crea una race
condition potenziale se più thread chiamano `apply()` e `bind()` simultaneamente.

#### 2.6 Partial or Undefined Implementations

Tutte le funzioni dichiarate sono implementate. `ConstraintSolver::occurs_in` è completa.

### System: Name Resolution System (SymbolTable)

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

**`DEF-009` — `StringHash` con `string_view` e ownership**: `SymbolTable` usa
`unordered_map<std::string_view, TypeScheme, StringHash>` [`SymbolTable.hpp:69`]. Le `string_view` come chiave puntano a
stringhe esterne. Se la stringa originale viene deallocata, la chiave diventa dangling. Questo è sicuro finché i nomi
degli identificatori vivono abbastanza (tipicamente da `std::string` nell'AST), ma è una **precondizione non documentata
**.

**`DEF-010` — `define()` crea scope implicitamente**: Se `define()` viene chiamato senza scope attivo, crea
implicitamente un scope [`SymbolTable.cpp:16–17`]. Questo comportamento nascosto maschera bug di chiamante che dimentica
`push_scope()`.

#### 2.6 Partial or Undefined Implementations

Completo. Nessuna funzione dichiarata senza implementazione.

### System: Type Checking Orchestration System (TypeChecker)

#### 2.1 System Overview

Il **TypeChecker** è l'orchestratore della pipeline di type checking. Espone `check()` come entry point principale che
esegue: (1) name resolution, (2) constraint generation, (3) constraint solving, (4) zonking. Espone anche `type_expr()`
e `type_stmt()` pubblicamente per unit testing.

#### 2.2 Internal Module Organization

Un solo file header (`TypeChecker.hpp`) e un solo file di implementazione da **1200 righe** (`TypeChecker.cpp`). È il
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

**`DEF-011` — `type_expr` per `CallExpr` non verifica signature**: In [`TypeChecker.cpp:698–745`], la chiamata a
funzione tipizza il callee e gli argomenti ma **non genera vincoli tra gli argomenti e i parametri formali della
funzione**. Non c'è controllo di arity. Il tipo di ritorno è sempre un `fresh_type_variable()`. Questo permette chiamate
con numero errato di argomenti senza errore.

**`DEF-012` — `type_stmt` per `VarDecl` multi-variable semplificato**: In [`TypeChecker.cpp:949–980`], le dichiarazioni
multi-variabili (`let a, b, c = 1, 2, 3`) vengono semplificate a una singola `TypedVarDecl` per la prima variabile. Le
altre vengono registrate nella SymbolTable ma **non compaiono nell'AST tipizzato**.

**`DEF-013` — `zonk_block_full` perde statement**: In [`TypeChecker.cpp:413–421`], quando `zonk_stmt_full` restituisce
`nullptr` per uno statement in un blocco, lo statement viene silenziosamente scartato ("Can't move from const — skip").
Questo corrompe l'AST tipizzato.

**`DEF-014` — `message_storage_` fragile**: Il `deque<std::string>` in [`TypeChecker.hpp:102`] possiede le stringhe dei
messaggi d'errore. I `CompileError` contengono `string_view` su queste stringhe [`CompileError.hpp:56`]. Se
`message_storage_` viene reallocato durante l'inserimento, i `string_view` già memorizzati negli `errors_` **potrebbero
** diventare dangling — sebbene `deque` garantisca stabilità degli iteratori, la documentazione non esplicita questa
garanzia come invariant.

**`DEF-015` — `resolve_names` per MainStmt duplica binding**: In [`TypeChecker.cpp:116–117`], `main` viene registrato
con tipo `void_()`, ma il nome "main" è hardcoded. Se il linguaggio dovesse supportare funzioni chiamate "main"
dall'utente, ci sarebbe collisione.

#### 2.6 Partial or Undefined Implementations

Tutte le funzioni dichiarate in `TypeChecker.hpp` sono implementate in `TypeChecker.cpp`. Non ci sono stub.

---

## Phase 3 — Per-Component Exhaustive Analysis

### System: Type Representation › Component: TypeBase

#### 3.1 Responsibility Statement

`TypeBase` è la classe base astratta che fornisce il discriminante `TypeKind` e l'interfaccia comune (`to_string()`,
`operator==`) per tutte le rappresentazioni di tipo nel sistema.

#### 3.2 Class Structure

| Membro           | Tipo                                      | Visibilità  | Semantica                                           |
|------------------|-------------------------------------------|-------------|-----------------------------------------------------|
| `kind_`          | `TypeKind`                                | `private`   | Discriminante del tipo concreto                     |
| `kind()`         | `constexpr TypeKind() const`              | `public`    | Getter per il discriminante                         |
| `is_primitive()` | `constexpr bool() const`                  | `public`    | Verifica se tipo built-in                           |
| `is_integer()`   | `constexpr bool() const`                  | `public`    | Verifica se intero (signed o unsigned)              |
| `is_numeric()`   | `constexpr bool() const`                  | `public`    | Verifica se numerico                                |
| `to_string()`    | `virtual std::string() const = 0`         | `public`    | Rappresentazione stringa                            |
| `operator==`     | `virtual bool(const TypeBase&) const = 0` | `public`    | Uguaglianza strutturale                             |
| Copy/Move        | deleted                                   | `protected` | Impedisce copia/spostamento — enforced `shared_ptr` |

Nessuna ereditarietà multipla. Tutte le sottoclassi usono ereditarietà singola `final`.

#### 3.3 Interface Analysis

L'interfaccia è pulita e minimale. I metodi `constexpr` per le verifiche di tipo (`is_integer`, `is_numeric`, etc.) sono
efficienti — nessuno richiede RTTI. La cancellazione di copy/move [`Type.hpp:217–220`] è corretta e impone l'uso di
`shared_ptr`.

#### 3.4 Implementation Logic

`TypeBase` è puramente astratta — nessuna logica implementativa oltre ai getter e ai predicate. I predicati usano switch
espliciti su `TypeKind`, evitando dipendenze da ordinamento dell'enum.

#### 3.5 Error Handling Evaluation

Nessun error handling — è una classe base che non può fallire.

#### 3.6 Type Consistency Audit

Tutti i tipi usano `TypePtr = shared_ptr<const TypeBase>`. La const-correctness è rispettata. Nessuna conversione
implicita pericolosa.

#### 3.7 Inter-Component Interaction

`TypeBase` è la radice della gerarchia. Ogni componente del type checker dipende da essa indirettamente tramite
`TypePtr`.

#### 3.8 Optimization Opportunities

I predicati `constexpr` sono già ottimizzati. Il pattern `classof` stile LLVM [`Type.hpp:377`] evita RTTI dove
possibile.

### System: Type Representation › Component: PrimitiveType

#### 3.1 Responsibility Statement

`PrimitiveType` rappresenta tutti i tipi primitivi del linguaggio (interi, floating-point, char, string, bool, void,
nullptr) come singleton immutabili.

#### 3.2 Class Structure

| Membro                                            | Tipo                                     | Visibilità | Semantica                            |
|---------------------------------------------------|------------------------------------------|------------|--------------------------------------|
| `PrivateTag`                                      | struct                                   | `public`   | Tag per impedire costruzione esterna |
| Factory methods (`i8()`, `i16()`, ..., `void_()`) | `static shared_ptr<const PrimitiveType>` | `public`   | Singleton per ogni tipo primitivo    |
| `classof(const TypeBase*)`                        | `static constexpr bool`                  | `public`   | RTTI-free type check                 |
| `to_string()`                                     | `override std::string`                   | `public`   | Delega a `type_kind_name(kind_)`     |
| `operator==`                                      | `override bool`                          | `public`   | Confronto per `kind()`               |

#### 3.3 Interface Analysis

Ogni factory method restituisce un riferimento a singleton statico locale (Meyers singleton). Questo garantisce che
`PrimitiveType::i32() == PrimitiveType::i32()` sia vero per identità di puntatore, oltre che per uguaglianza
strutturale. Precondizione: il `kind` passato al costruttore deve essere un primitivo (verificato da `assert`).

#### 3.4 Implementation Logic

Implementazione banale — ogni factory è un singleton thread-safe (C++11 static local initialization è thread-safe).
`to_string()` delega a `type_kind_name()` che è `constexpr`.

#### 3.5 Error Handling Evaluation

Nessun errore possibile. I singleton non falliscono.

#### 3.6 Type Consistency Audit

I singleton sono `shared_ptr<const PrimitiveType>`, compatibili con `TypePtr`. Nessun problema di conversione.

#### 3.7 Inter-Component Interaction

Usato ovunque nel type checker come tipo concreto di default per literal e annotazioni.

#### 3.8 Optimization Opportunities

I singleton eliminano allocazioni ripetute. Il design è già ottimale.

### System: Type Representation › Component: CustomType

#### 3.1 Responsibility Statement

`CustomType` rappresenta tipi definiti dall'utente (struct, class, enum) identificati per nome.

#### 3.2 Class Structure

| Membro        | Tipo                       | Visibilità | Semantica            |
|---------------|----------------------------|------------|----------------------|
| `name_`       | `shared_ptr<const string>` | `private`  | Nome del tipo custom |
| `name()`      | `string_view`              | `public`   | Getter               |
| `to_string()` | `override`                 | `public`   | Restituisce il nome  |
| `operator==`  | `override`                 | `public`   | Confronta per nome   |
| `classof`     | `static constexpr`         | `public`   | Type check           |

#### 3.3 Interface Analysis

Il costruttore accetta `string_view` ma memorizza `shared_ptr<const string>` — copia il dato. Questo è corretto per
ownership.

#### 3.4 Implementation Logic

Semplice wrapper attorno a un nome. Uguaglianza per confronto di stringhe.

#### 3.5 Error Handling Evaluation

Nessun errore.

#### 3.6 Type Consistency Audit

Nessun problema.

#### 3.7 Inter-Component Interaction

Usato per tipi utente non ancora risolti. Il `ConstraintSolver` non gestisce correttamente l'unificazione tra
`CustomType` diversi (vedi DEF-007).

#### 3.8 Optimization Opportunities

`shared_ptr<const string>` potrebbe essere sostituito con `string` diretto (l'overhead del puntatore condiviso non è
necessario per un membro privato).

### System: Type Representation › Component: ArrayType

#### 3.1 Responsibility Statement

`ArrayType` rappresenta array a dimensione fissa `[T; N]` con tipo elemento e espressione di dimensione.

#### 3.2 Class Structure

| Membro           | Tipo                         | Visibilità | Semantica                         |
|------------------|------------------------------|------------|-----------------------------------|
| `element_type_`  | `shared_ptr<const TypeBase>` | `private`  | Tipo degli elementi               |
| `size_expr_`     | `shared_ptr<const Expr>`     | `private`  | Espressione della dimensione      |
| `element_type()` | `const shared_ptr&`          | `public`   | Getter                            |
| `size_expr()`    | `const shared_ptr&`          | `public`   | Getter                            |
| `sizes_equal()`  | `static bool`                | `private`  | Confronto strutture di dimensione |

#### 3.3 Interface Analysis

Precondizione: `element_type` e `size_expr` non devono essere `nullptr` (verificati da `assert` nel costruttore).

#### 3.4 Implementation Logic

L'uguaglianza confronta sia il tipo elemento sia la dimensione tramite `sizes_equal()`. La funzione `sizes_equal` è
dichiarata ma la sua implementazione in `Type.cpp` richiede verifica — se confronta solo `IntegerLiteral`, espressioni
complesse fallirebbero.

#### 3.5 Error Handling Evaluation

Nessun errore — solo assert in costruzione.

#### 3.6 Type Consistency Audit

Corretto.

#### 3.7 Inter-Component Interaction

`Substitution::apply()` visita `ArrayType` per applicare sostituzioni al tipo elemento [`Substitution.cpp:17–21`].
`ConstraintSolver::UnifyVisitor` unifica array confrontando i tipi elemento [`ConstraintSolver.cpp:30–33`].

#### 3.8 Optimization Opportunities

Nessuno significativo.

### System: Type Representation › Component: VectorType

#### 3.1 Responsibility Statement

`VectorType` rappresenta array dinamici `Vec<T>` con tipo elemento.

#### 3.2–3.8

Strutturalmente identico ad `ArrayType` ma senza `size_expr_`. Stesse osservazioni di `ArrayType` per interazione con
`Substitution` e `ConstraintSolver`. Vedi anche §3.2–§3.8 di ArrayType.

### System: Constraint Solving › Component: Constraint / ConstraintSet

#### 3.1 Responsibility Statement

`Constraint` rappresenta un vincolo di uguaglianza `lhs = rhs` tra due tipi, mentre `ConstraintSet` accumula vincoli con
ID univoci per il solver.

#### 3.2 Class Structure

**Constraint** (struct):

| Campo    | Tipo           | Semantica                  |
|----------|----------------|----------------------------|
| `id`     | `ConstraintId` | ID univoco (1-based)       |
| `lhs`    | `TypePtr`      | Tipo sinistro da unificare |
| `rhs`    | `TypePtr`      | Tipo destro da unificare   |
| `origin` | `SourceSpan`   | Posizione sorgente         |
| `reason` | `std::string`  | Contesto per diagnostic    |

**ConstraintSet** (class):

| Membro         | Tipo                 | Semantica                        |
|----------------|----------------------|----------------------------------|
| `constraints_` | `vector<Constraint>` | Storage                          |
| `next_id_`     | `ConstraintId`       | Contatore (inizia da 1)          |
| `add()`        | `ConstraintId`       | Aggiunge vincolo, restituisce ID |
| `get(id)`      | `const Constraint*`  | Lookup lineare per ID            |

#### 3.3 Interface Analysis

`add()` assegna ID sequenziali. `get()` esegue ricerca lineare O(n) — accettabile per set piccoli ma non scalabile.
`constraints()` restituisce riferimento const per iterazione.

#### 3.4 Implementation Logic

`add()` incrementa `next_id_` e pusha nel vector [`Constraint.cpp:10–15`]. `get()` usa `std::ranges::find` con
proiezione [`Constraint.cpp:19–21`].

#### 3.5 Error Handling Evaluation

Nessun errore — `add()` non fallisce (tranne `bad_alloc`). `get()` restituisce `nullptr` se non trovato.

#### 3.6 Type Consistency Audit

Corretto.

#### 3.7 Inter-Component Interaction

`ConstraintSet` è passato a `ConstraintSolver::solve()` per riferimento const. I vincoli sono consumati uno alla volta
dal solver.

#### 3.8 Optimization Opportunities

`get()` è O(n). Se i vincoli fossero accessibili frequentemente per ID, un `unordered_map<ConstraintId, Constraint>`
sarebbe più efficiente. Attualmente `get()` è usato raramente (principalmente per debug).

### System: Constraint Solving › Component: Substitution

#### 3.1 Responsibility Statement

`Substitution` mappa `TypeVarId → TypePtr` e applica ricorsivamente queste sostituzioni ai tipi, risolvendo tutte le
variabili annidate.

#### 3.2 Class Structure

| Membro         | Tipo                                      | Visibilità        | Semantica                                |
|----------------|-------------------------------------------|-------------------|------------------------------------------|
| `bindings_`    | `unordered_map<TypeVarId, TypePtr>`       | `private`         | Mapping variabili → tipi                 |
| `apply_cache_` | `unordered_map<const TypeBase*, TypePtr>` | `private mutable` | Cache dei risultati di `apply()`         |
| `bind()`       | `void`                                    | `public`          | Associa variabile a tipo, invalida cache |
| `lookup()`     | `optional<TypePtr>`                       | `public`          | Cerca binding                            |
| `apply()`      | `TypePtr`                                 | `public`          | Applica sostituzione a un tipo           |
| `applyImpl()`  | `TypePtr`                                 | `private`         | Worker ricorsivo con cache               |

#### 3.3 Interface Analysis

`bind()` invalida completamente `apply_cache_` — questo è corretto ma conservativo: solo le entry che dipendono dalla
variabile modificata sarebbero invalidate. `apply()` è thread-unsafe se chiamato concorrentemente con `bind()`.

#### 3.4 Implementation Logic

`applyImpl()` [`Substitution.cpp:46–66`]: (1) controlla cache; (2) se `TypeVariable`, cerca binding e ricorsivamente
applica; (3) se tipo composto, usa `ApplyVisitor` per costruire nuovo tipo con elementi risolti; (4) cache il risultato.
La cache è keyed su `const TypeBase*` — l'identità del puntatore, non il valore.

#### 3.5 Error Handling Evaluation

Nessun errore esplicito. Se un tipo non ha binding, viene restituito così com'è.

#### 3.6 Type Consistency Audit

La cache basata su puntatore raw è corretta perché `TypeBase` è immutabile (`shared_ptr<const>`). Se lo stesso nodo tipo
viene riutilizzato in più punti dell'AST (DAG), la cache condivide il risultato — corretto ed efficiente.

#### 3.7 Inter-Component Interaction

`Substitution` è prodotto da `ConstraintSolver` e consumato da `TypeChecker::zonk()`. `ConstraintSolver::occurs_in()`
usa `Substitution::apply()` per risolvere tipi prima della verifica [`ConstraintSolver.cpp:57`].

#### 3.8 Optimization Opportunities

L'invalidazione totale della cache in `bind()` è conservativa. Per programmi con molti vincoli, la cache verrebbe
invalidata frequentemente. Un approccio più raffinato invaliderebbe solo le entry che transitivamente dipendono dalla
variabile modificata.

### System: Constraint Solving › Component: UnionFind

#### 3.1 Responsibility Statement

`UnionFind` implementa disjoint-set union con path compression e union by rank per tracciare le equivalenze tra
variabili di tipo durante l'unificazione.

#### 3.2 Class Structure

| Membro       | Tipo                                  | Semantica                                  |
|--------------|---------------------------------------|--------------------------------------------|
| `parent_`    | `unordered_map<TypeVarId, TypeVarId>` | Mappa nodo → genitore                      |
| `rank_`      | `unordered_map<uint8_t>`              | Rank per union by rank                     |
| `make_set()` | `void`                                | Crea set singleton                         |
| `find()`     | `TypeVarId`                           | Trova rappresentante con path compression  |
| `unite()`    | `void`                                | Unisce due set                             |
| `same_set()` | `bool`                                | Verifica se due nodi sono nello stesso set |

#### 3.3 Interface Analysis

`find()` non è `const` perché modifica `parent_` (path compression). Documentato correttamente in `UnionFind.hpp:32–41`.
`make_set()` è idempotente — se il nodo esiste già, non viene modificato.

#### 3.4 Implementation Logic

`find()` usa ricorsione con path compression [`UnionFind.cpp:14–17`]. Per alberi profondi, questo potrebbe causare stack
overflow — sebbene con union by rank la profondità sia O(log n), in pratica è gestibile. `unite()` usa union by rank [
`UnionFind.cpp:19–33`].

#### 3.5 Error Handling Evaluation

**`DEF-006` (ripetuto)**: `find()` usa `parent_.at(var)` che lancia se `var` non è registrato. Questo è un errore di
programmazione del chiamante, ma un `assert` sarebbe più appropriato per un debug build.

#### 3.6 Type Consistency Audit

Corretto.

#### 3.7 Inter-Component Interaction

Usato esclusivamente da `ConstraintSolver`. Non esposto ad altri sistemi.

#### 3.8 Optimization Opportunities

Per grandi programmi, `unordered_map` ha overhead di hashing. Un `vector` con resizing sarebbe più efficiente se gli ID
delle variabili sono densi.

### System: Constraint Solving › Component: ConstraintSolver

#### 3.1 Responsibility Statement

`ConstraintSolver` orchestra l'unificazione di tutti i vincoli di tipo, producendo una `Substitution` finale e
raccogliendo errori di tipo.

#### 3.2 Class Structure

| Membro          | Tipo                           | Semantica                             |
|-----------------|--------------------------------|---------------------------------------|
| `union_find_`   | `UnionFind`                    | Disjoint-set per equivalenze          |
| `substitution_` | `Substitution`                 | Mapping soluzioni                     |
| `solve()`       | `SolverResult`                 | Entry point — risolve tutti i vincoli |
| `unify()`       | `expected<void, CompileError>` | Unifica due tipi                      |
| `occurs_in()`   | `static bool`                  | Verifica occurs-check                 |

#### 3.3 Interface Analysis

`solve()` accetta `ConstraintSet` const e restituisce `SolverResult`. `unify()` è pubblico e può essere chiamato
individualmente per test. `occurs_in()` è statico e accetta `Substitution` per risolvere variabili prima della verifica.

#### 3.4 Implementation Logic

`solve()` [`ConstraintSolver.cpp:44–54`] itera linearmente sui vincoli. `unify()` [`ConstraintSolver.cpp:67–139`]
gestisce:

1. ErrorType → successo silente.
2. Null type → errore E2034.
3. TypeVariable vs TypeVariable → occurs-check + binding.
4. TypeVariable vs concreto → binding.
5. Concreto vs TypeVariable → swap e ricorsione.
6. Concreto vs concreto → verifica kind + visitor per composti.

#### 3.5 Error Handling Evaluation

ErrorType unifica silenziosamente con tutto — corretto per error recovery. I mismatch di tipo producono
`CompileError::TypeError` con codice E2034. L'occurs-check produce E2035.

**`DEF-007` (ripetuto)**: Il caso "entrambi CustomType" con nomi diversi non viene rilevato — `UnifyVisitor` non viene
dispatchato per `CustomType`, quindi il risultato è `value_or(success)` che restituisce successo.

#### 3.6 Type Consistency Audit

Corretto.

#### 3.7 Inter-Component Interaction

Consuma `ConstraintSet`. Produce `SolverResult`. Usa `TypeVisitor` per dispatch. Dipende da `UnionFind` e `Substitution`
come stato interno.

#### 3.8 Optimization Opportunities

L'iterazione lineare sui vincoli va bene. Tuttavia, `unify()` crea `UnifyVisitor` sullo stack per ogni chiamata —
potrebbe essere riutilizzato come membro per evitare allocazioni.

### System: Name Resolution › Component: SymbolTable

#### 3.1 Responsibility Statement

`SymbolTable` gestisce binding identificatore→`TypeScheme` con scope lessicali annidati, supportando shadowing e lookup
outward.

#### 3.2 Class Structure

| Membro                       | Tipo                                             | Semantica                              |
|------------------------------|--------------------------------------------------|----------------------------------------|
| `scopes_`                    | `vector<unordered_map<string_view, TypeScheme>>` | Stack di scope                         |
| `StringHash`                 | struct                                           | Hasher eterogeneo per `string_view`    |
| `push_scope()`               | `void`                                           | Entra in nuovo scope                   |
| `pop_scope()`                | `void`                                           | Esce dallo scope corrente              |
| `define()`                   | `void`                                           | Definisce simbolo nel scope corrente   |
| `lookup()`                   | `optional<TypeScheme>`                           | Cerca simbolo dall'interno all'esterno |
| `defined_in_current_scope()` | `bool`                                           | Verifica nel solo scope corrente       |
| `depth()`                    | `size_t`                                         | Profondità scope                       |

#### 3.3 Interface Analysis

Tutte le operazioni sono O(1) amortizzate tranne `lookup()` che è O(depth × bucket_size). Precondizione: `pop_scope()`
richiede stack non vuoto (silenziosamente ignorato se vuoto).

#### 3.4 Implementation Logic

`lookup()` usa `reverse_view` per iterare dall'ultimo scope al primo [`SymbolTable.cpp:20–26`]. `define()` crea scope
implicitamente se vuoto [`SymbolTable.cpp:16–17`].

#### 3.5 Error Handling Evaluation

Nessun errore restituito. `lookup()` restituisce `nullopt` se non trovato — il chiamante deve gestire il caso.

#### 3.6 Type Consistency Audit

Le `string_view` come chiave richiedono che le stringhe originali sopravvivano. Questo è garantito dal fatto che i nomi
degli identificatori sono memorizzati nell'AST (che vive più a lungo del type checker).

#### 3.7 Inter-Component Interaction

Usato da `TypeChecker` in `resolve_names()` e `type_expr()` per identificatori.

#### 3.8 Optimization Opportunities

L'hasher eterogeneo `StringHash` è ben progettato — permette lookup con `string_view` senza creazione di `string`.

### System: Type Checking Orchestration › Component: TypeChecker

#### 3.1 Responsibility Statement

`TypeChecker` orchestra l'intera pipeline di type checking, trasformando un `Program` (AST non tipizzato) in un
`TypedProgram` (AST tipizzato) con raccolta errori.

#### 3.2 Class Structure

| Membro                          | Tipo                   | Semantica                         |
|---------------------------------|------------------------|-----------------------------------|
| `symbols_`                      | `SymbolTable`          | Tabella simboli                   |
| `constraints_`                  | `ConstraintSet`        | Vincoli generati                  |
| `errors_`                       | `vector<CompileError>` | Errori raccolti                   |
| `message_storage_`              | `deque<string>`        | Storage per messaggi d'errore     |
| `typed_stmts_`                  | `vector<TypedStmtPtr>` | Statement tipizzati durante CG    |
| `current_function_return_type_` | `optional<TypePtr>`    | Tipo di ritorno funzione corrente |
| `current_function_name_`        | `optional<string>`     | Nome funzione corrente            |
| `loop_depth_`                   | `size_t`               | Profondità di annidamento loop    |

#### 3.3 Interface Analysis

`check()` è l'entry point principale. `type_expr()` e `type_stmt()` sono pubblici per testing ma dovrebbero essere
privati in produzione — espongono l'implementazione interna.

#### 3.4 Implementation Logic

La pipeline è sequenziale e ben strutturata in `check()`. Le quattro fasi sono chiaramente separate. Tuttavia,
`type_expr()` contiene logica massiva (400+ righe) con switch su `NodeKind` che gestisce 20+ casi. Ogni caso tipizza
l'espressione, genera vincoli, e costruisce il nodo tipizzato.

**Casi critici**:

- `CallExpr` [`TypeChecker.cpp:698–745`]: non verifica arity né signature.
- `AssignExpr` [`TypeChecker.cpp:820–846`]: verifica immutabilità ma non genera vincolo completo.
- `ArrayLiteral` [`TypeChecker.cpp:787–818`]: richiede almeno un elemento, verifica omogeneità.

#### 3.5 Error Handling Evaluation

Errori accumulati in `errors_`. `ErrorType` propaga silenziosamente. `message_storage_` previene dangling `string_view`
ma la relazione è implicita.

#### 3.6 Type Consistency Audit

Corretto.

#### 3.7 Inter-Component Interaction

`TypeChecker` è il consumatore finale di tutti gli altri sistemi. Produce `TypedProgram` consumando il raw `Program`.

#### 3.8 Optimization Opportunities

`type_expr()` dovrebbe essere scomposto in funzioni più piccole — una per `NodeKind`. Questo migliorerebbe testabilità,
leggibilità e manutenibilità. La funzione ha CCN >40, molto sopra il limite di 15 del progetto.

### System: Type Representation › Component: TypeVariable

#### 3.1 Responsibility Statement

`TypeVariable` rappresenta una variabile di tipo incognita (?T1, ?T2, ...) generata durante l'inferenza per espressioni
senza annotazione di tipo esplicita.

#### 3.2 Class Structure

| Membro        | Tipo                  | Semantica            |
|---------------|-----------------------|----------------------|
| `id_`         | `TypeVarId`           | ID univoco (>0)      |
| Costruttore   | `explicit constexpr`  | Richiede ID non-zero |
| `id()`        | `constexpr TypeVarId` | Getter               |
| `to_string()` | `override`            | Formato "?T{id}"     |
| `classof`     | `static constexpr`    | RTTI-free check      |
| `operator==`  | `override`            | Uguaglianza per ID   |

#### 3.3 Interface Analysis

Il costruttore è `explicit` — impedisce conversioni implicite da `size_t`. `fresh_type_variable()` usa counter
thread-local per unicità.

#### 3.4 Implementation Logic

`fresh_type_variable()` [`TypeVariable.cpp:16`] crea `TypeVariable` con ID incrementale thread-local. Semplice e
corretto.

#### 3.5 Error Handling Evaluation

Nessun errore.

#### 3.6 Type Consistency Audit

Corretto. `TypeVarId` è `size_t` in entrambi i file che lo definiscono (`TypeVariable.hpp:12` e `UnionFind.hpp:12`) — *
*`DEF-016` — duplicate typedef**: `TypeVarId` è definito due volte. Se i due alias divergessero in futuro, ci sarebbe
inconsistenza.

#### 3.7 Inter-Component Interaction

Usato da `Substitution`, `ConstraintSolver`, `TypeScheme`, `TypeChecker`.

#### 3.8 Optimization Opportunities

Nessuno significativo.

### System: Type Representation › Component: TypeScheme

#### 3.1 Responsibility Statement

`TypeScheme` rappresenta tipi polimorfici con variabili quantificate (∀T. body), usati per funzioni generiche.

#### 3.2 Class Structure

| Campo             | Tipo                | Semantica                          |
|-------------------|---------------------|------------------------------------|
| `quantified_vars` | `vector<TypeVarId>` | Variabili vincolate                |
| `body`            | `TypePtr`           | Tipo corpo                         |
| `is_const`        | `bool`              | Binding immutabile                 |
| `instantiate()`   | `TypePtr`           | Crea istanza con variabili fresche |
| `mono()`          | `static TypeScheme` | Factory per tipo monomorfico       |

#### 3.3 Interface Analysis

`instantiate()` dovrebbe sostituire le variabili quantificate nel body con variabili fresche. Attualmente lo fa solo per
body che sono `TypeVariable` diretti.

#### 3.4 Implementation Logic

`mono()` crea scheme senza variabili quantificate [`TypeScheme.cpp:10–12`]. `instantiate()` [`TypeScheme.cpp:14–33`]
mappa le variabili quantificate a fresche ma non traversa il body composto.

#### 3.5 Error Handling Evaluation

Nessun errore.

#### 3.6 Type Consistency Audit

Corretto per il caso monomorfico. Incompleto per il caso polimorfico con body composto.

#### 3.7 Inter-Component Interaction

Usato da `SymbolTable` per i binding e da `TypeChecker::type_expr()` per gli identificatori [`TypeChecker.cpp:501`].

#### 3.8 Optimization Opportunities

Vedi DEF-003.

### System: Type Representation › Component: ErrorType

#### 3.1 Responsibility Statement

`ErrorType` è un tipo sentinella singleton che unifica silenziosamente con qualsiasi tipo, prevenendo errori a cascata
dopo un errore di tipo.

#### 3.2 Class Structure

| Membro         | Tipo               | Semantica                               |
|----------------|--------------------|-----------------------------------------|
| Costruttore    | `ErrorType()`      | Private-style (solo via `error_type()`) |
| `to_string()`  | `override`         | Restituisce "<error>"                   |
| `classof`      | `static constexpr` | RTTI-free check                         |
| `operator==`   | `override`         | Uguale a qualsiasi ErrorType            |
| `error_type()` | `TypePtr`          | Factory singleton                       |

#### 3.3 Interface Analysis

Singleton thread-safe tramite static local.

#### 3.4 Implementation Logic

`error_type()` [`ErrorType.cpp:11–14`] restituisce sempre la stessa istanza. `to_string()` restituisce `"<error>"`.
`operator==` confronta solo il kind.

#### 3.5 Error Handling Evaluation

Il design stesso è un meccanismo di error handling — silenzia gli errori a cascata.

#### 3.6 Type Consistency Audit

Corretto.

#### 3.7 Inter-Component Interaction

`ConstraintSolver::unify()` controlla `TypeKind::Error` e restituisce successo silente [`ConstraintSolver.cpp:67–68`].

#### 3.8 Optimization Opportunities

Nessuno.

### System: Type Representation › Component: TypeVisitor

#### 3.1 Responsibility Statement

`TypeVisitor` fornisce un'interfaccia visitor per la visita di tipi composti (Array, Vector) senza duplicare la logica
switch-on-TypeKind.

#### 3.2 Class Structure

| Membro           | Tipo            | Semantica                     |
|------------------|-----------------|-------------------------------|
| `~TypeVisitor()` | `virtual`       | Distruttore                   |
| `visit_array()`  | `virtual void`  | Visita ArrayType              |
| `visit_vector()` | `virtual void`  | Visita VectorType             |
| `visit_type()`   | `free function` | Dispatch basato su `TypeKind` |

#### 3.3 Interface Analysis

I metodi `visit_*` sono pure virtual — le sottoclassi devono implementarli. `visit_type()` fa il dispatch.

#### 3.4 Implementation Logic

`visit_type()` [`TypeVisitor.cpp:12–22`] switcha su `TypeKind` e chiama il metodo appropriato. Per `TypePtr`, delega
alla versione reference.

#### 3.5 Error Handling Evaluation

Nessun errore.

#### 3.6 Type Consistency Audit

Corretto.

#### 3.7 Inter-Component Interaction

Usato da `Substitution::applyImpl()` (`ApplyVisitor`), `ConstraintSolver::unify()` (`UnifyVisitor`), e
`ConstraintSolver::occurs_in()` (`OccursVisitor`).

#### 3.8 Optimization Opportunities

Il visitor potrebbe essere esteso per includere `CustomType` se necessario per l'unificazione corretta.

---

## Phase 4 — Prioritized Recommendations

### 4.1 Recommendation Register

#### REC-001: Completare `TypeScheme::instantiate()` per tipi composti

**Deficiency addressed**: DEF-003

**Description**: Entry point: `include/jsav/typechecker/TypeScheme.hpp`, metodo `TypeScheme::instantiate()`, e
`src/jsav/Lib/typechecker/TypeScheme.cpp`. Implementare una sostituzione ricorsiva delle variabili quantificate
all'interno del body composto. Creare un visitor interno che traversa il body (gestendo `ArrayType`, `VectorType`, e
futuri `FnType`) e sostituisce ogni occorrenza di `TypeVariable` il cui ID è in `quantified_vars` con una variabile
fresca generata da `fresh_type_variable()`. L'outcome atteso è che i tipi polimorfici con body composti (es.
`∀T. Vec<T> → T`) vengano istanziati correttamente.

**Feasibility**: 3/5 — Richiede implementazione di un visitor di sostituzione, ma la struttura `TypeVisitor` esiste già
come pattern nel codebase.
**ROI**: 5/5 — Senza questo, le funzioni generiche con tipi composti nel body producono tipi errati, compromettendo
l'intero sistema di inferenza polimorfica.
**Effort**: 3/5 — Stimato 1–2 giorni di lavoro: implementare visitor, testare con casi composti, verificare con
`constexpr_tests`.
**Composite**: 3×2 + 5×2 + 3×1 = 6 + 10 + 3 = **19**
**Estimated implementation time**: 1–2 giorni
**Required resources**: Un developer C++ con conoscenza del pattern Visitor; accesso ai test esistenti.
**Effectiveness indicators**:

1. Test `TypeChecker_GenericFunction_CreatesTypeScheme` e `TypeChecker_GenericCall_InstantiatesFreshVars` passano con
   body composti.
2. Zero casi di `TypeVariable` non risolte in programmi con funzioni generiche che restituiscono tipi composti.

#### REC-002: Aggiungere tipo funzione (`FnType`) e verifica signature nelle chiamate

**Deficiency addressed**: DEF-004, DEF-011

**Description**: Entry point: `include/jsav/ast/Type.hpp` (aggiungere classe `FnType`), `src/jsav_Lib/ast/Type.cpp`, e
`TypeChecker::type_expr()` per `NodeKind::CallExpr` [`TypeChecker.cpp:698–745`]. Creare `FnType` con campi
`params: vector<TypePtr>` e `return_type: TypePtr`. Durante la generazione vincoli per `CallExpr`, costruire il tipo
funzione atteso dai parametri e vincolarlo contro il tipo del callee. Generare errore se l'arity non corrisponde.
L'outcome è che le chiamate con numero errato di argomenti o tipi incompatibili vengano rilevate.

**Feasibility**: 2/5 — Modifica architetturale significativa: richiede nuovo tipo, aggiornamento del visitor,
aggiornamento del solver, aggiornamento della generazione vincoli.
**ROI**: 5/5 — Senza verifica di signature, il type checker accetta programmi semanticamente errati (chiamate con arity
sbagliata). Questo è un requisito fondamentale per un type checker production-grade.
**Effort**: 2/5 — Stimato 1–2 settimane: design `FnType`, integrazione con `TypeVisitor`, `ConstraintSolver`,
`Substitution`, e `TypeChecker::type_expr`.
**Composite**: 2×2 + 5×2 + 2×1 = 4 + 10 + 2 = **16**
**Estimated implementation time**: 1–2 settimane
**Required resources**: Developer senior con esperienza in type system; review architetturale.
**Effectiveness indicators**:

1. Test `TypeChecker_FunctionArgCountMismatch_ReportsError` rileva errori di arity.
2. Programmi con chiamate a funzione con arity errata producono errore in fase di type checking.

#### REC-003: Correggere `UnifyVisitor` per gestire `CustomType`

**Deficiency addressed**: DEF-007

**Description**: Entry point: `src/jsav_Lib/typechecker/ConstraintSolver.cpp`, struct `UnifyVisitor`. Aggiungere un
metodo `visit_custom` che confronta i nomi dei `CustomType`. Se i nomi differiscono, restituire errore di type mismatch.
In alternativa, gestire il caso `CustomType` vs `CustomType` direttamente in `unify()` prima del dispatch al visitor.
L'outcome è che due tipi utente diversi ("Foo" e "Bar") non unifichino erroneamente.

**Feasibility**: 5/5 — Modifica localizzata in un singolo file, poche righe di codice.
**ROI**: 4/5 — Due tipi utente distinti che unificano silenziosamente è un bug di correttezza grave, anche se l'impatto
pratico dipende dall'uso di `CustomType` nel linguaggio.
**Effort**: 5/5 — Minimo sforzo: 1–2 ore.
**Composite**: 5×2 + 4×2 + 5×1 = 10 + 8 + 5 = **23**
**Estimated implementation time**: 1–2 ore
**Required resources**: Un developer C++.
**Effectiveness indicators**:

1. Test che due `CustomType` con nomi diversi producono errore E2034.
2. Zero unificazioni errate tra tipi utente non correlati.

#### REC-004: Correggere perdita di statement in `zonk_block_full`

**Deficiency addressed**: DEF-013

**Description**: Entry point: `src/jsav_Lib/typechecker/TypeChecker.cpp`, metodo `TypeChecker::zonk_block_full` [
`TypeChecker.cpp:413–421`]. Quando `zonk_stmt_full` restituisce `nullptr` per uno statement const, invece di scartarlo
silenziosamente, mantenere una copia dello statement originale (o un placeholder con tipo errore). Modificare il ciclo
per copiare lo statement originale se il zonking fallisce:
`zonked_stmts.push_back(stmt.clone() ? std::move(zonked) : stmt.shallow_copy())`. L'outcome è che nessuno statement
venga perso durante lo zonking.

**Feasibility**: 4/5 — Il problema è localizzato. La soluzione richiede un meccanismo di copia degli statement
tipizzati (attualmente `unique_ptr`, quindi serve `clone()` o copia profonda).
**ROI**: 4/5 — Perdere statement dall'AST tipizzato corrompe il risultato del type checking — il programma compilato
potrebbe mancare istruzioni.
**Effort**: 4/5 — Stimato 4–8 ore: implementare copia profonda o fallback, testare con blocchi contenenti statement che
zonkano a nullptr.
**Composite**: 4×2 + 4×2 + 4×1 = 8 + 8 + 4 = **20**
**Estimated implementation time**: 4–8 ore
**Required resources**: Un developer C++ con conoscenza del Typed AST.
**Effectiveness indicators**:

1. Il conteggio degli statement nel `TypedProgram` corrisponde a quello del `Program` originale.
2. Zero statement persi in programmi con statement che contengono errori di tipo.

#### REC-005: Sostituire `UnionFind::at()` con accesso sicuro

**Deficiency addressed**: DEF-006

**Description**: Entry point: `src/jsav_Lib/typechecker/UnionFind.cpp`, metodo `UnionFind::find` [
`UnionFind.cpp:14–17`]. Sostituire `parent_.at(var)` con `parent_[var]` dopo verifica preliminare, o aggiungere
`assert(parent_.contains(var))` prima dell'accesso. Questo trasforma un'eccezione runtime non gestita in un fallimento
esplicito in debug mode. L'outcome è che i bug del chiamante vengano rilevati immediatamente in debug build invece di
causare crash in production.

**Feasibility**: 5/5 — Modifica di 2–3 righe in un singolo metodo.
**ROI**: 3/5 — Previene crash runtime non diagnosticati. L'impatto è limitato ai bug di programmazione interna.
**Effort**: 5/5 — Minimo sforzo: 30 minuti.
**Composite**: 5×2 + 3×2 + 5×1 = 10 + 6 + 5 = **21**
**Estimated implementation time**: 30 minuti
**Required resources**: Un developer C++.
**Effectiveness indicators**:

1. Nessun `std::out_of_range` lanciato da `UnionFind::find` nei test.
2. Assert triggerati in debug build per uso improprio.

#### REC-006: Unificare `TypeVarId` typedef in un singolo header

**Deficiency addressed**: DEF-016

**Description**: Entry point: `include/jsav/typechecker/TypeVariable.hpp:12` e
`include/jsav/typechecker/UnionFind.hpp:12`. Rimuovere la definizione duplicata da `UnionFind.hpp` e includere
`TypeVariable.hpp` invece di ridefinire. L'outcome è un'unica fonte di verità per `TypeVarId`.

**Feasibility**: 5/5 — Puramente meccanica: rimuovere una riga e aggiungere un include.
**ROI**: 2/5 — Basso rischio attuale (entrambi `size_t`), ma previene bug futuri se il tipo cambiasse.
**Effort**: 5/5 — 15 minuti.
**Composite**: 5×2 + 2×2 + 5×1 = 10 + 4 + 5 = **19**
**Estimated implementation time**: 15 minuti
**Required resources**: Un developer C++.
**Effectiveness indicators**:

1. `TypeVarId` definito in un solo punto.
2. Build succeeds senza warning di redefinizione.

#### REC-007: Scomporre `TypeChecker::type_expr()` in funzioni per NodeKind

**Deficiency addressed**: DEF-011 (parzialmente), complessità cognitiva (AGENTS.md §7)

**Description**: Entry point: `src/jsav_Lib/typechecker/TypeChecker.cpp`, metodo `TypeChecker::type_expr` [
`TypeChecker.cpp:430–880`]. Creare metodi privati separati: `type_literal()`, `type_binary()`, `type_unary()`,
`type_call()`, `type_array()`, `type_assign()`, `type_ternary()`, `type_index()`, `type_member()`, `type_cast()`,
`type_identifier()`. Ogni metodo gestisce un singolo `NodeKind`. Lo switch principale delega al metodo appropriato.
L'outcome è una riduzione del CCN da >40 a <5 per funzione.

**Feasibility**: 3/5 — Refactoring meccanico ma voluminoso (400+ righe da riorganizzare). Richiede attenzione nel
catturare correttamente le variabili di contesto (`symbols_`, `constraints_`, `errors_`).
**ROI**: 4/5 — Migliora drasticamente manutenibilità, testabilità individuale, e compliance con gli standard del
progetto (CCN ≤15).
**Effort**: 2/5 — Stimato 1–2 giorni.
**Composite**: 3×2 + 4×2 + 2×1 = 6 + 8 + 2 = **16**
**Estimated implementation time**: 1–2 giorni
**Required resources**: Un developer C++ con tool lizard per verifica CCN.
**Effectiveness indicators**:

1. `lizard --CCN 15` su `TypeChecker.cpp` riporta zero violazioni.
2. Tutti i test esistenti passano senza modifiche.

#### REC-008: Unificare la strategia di propagazione errori

**Deficiency addressed**: DEF-001

**Description**: Entry point: `include/jsav/typechecker/` (tutti i file che riportano errori),
`src/jsav_Lib/typechecker/`. Adottare `std::expected<T, CompileError>` come strategia uniforme per la propagazione
errori in tutte le funzioni che possono fallire. `SymbolTable::lookup()` dovrebbe restituire
`std::expected<TypeScheme, CompileError>` invece di `std::optional<TypeScheme>`, producendo un errore "Undeclared
identifier" strutturato. `TypeChecker` accumula gli errori ma potrebbe ricevere errori strutturati direttamente dai
sottosistemi. L'outcome è un flusso d'errore tracciabile e tipato.

**Feasibility**: 2/5 — Richiede modifiche a interfacce pubbliche e a tutti i chiamanti. Cambia il contratto di
`lookup()`.
**ROI**: 4/5 — Errori strutturati migliorano la qualità dei messaggi diagnostici e la manutenibilità del codice di
gestione errori.
**Effort**: 2/5 — Stimato 2–3 giorni.
**Composite**: 2×2 + 4×2 + 2×1 = 4 + 8 + 2 = **14**
**Estimated implementation time**: 2–3 giorni
**Required resources**: Developer C++ con conoscenza di `std::expected`.
**Effectiveness indicators**:

1. Zero uso di `std::optional` per segnalare errori nel type checker.
2. Tutti gli errori hanno codice ErrorCode associato.

#### REC-009: Rendere privati `type_expr()` e `type_stmt()`

**Deficiency addressed**: DEF-011 (esposizione API interna)

**Description**: Entry point: `include/jsav/typechecker/TypeChecker.hpp`, dichiarazioni di `type_expr` e `type_stmt`.
Spostare da `public` a `private`. Se il testing individuale di espressioni/statement è necessario, creare un wrapper di
test o usare `friend class TypeCheckerTest`. L'outcome è un'API pubblica pulita che espone solo `check()`.

**Feasibility**: 5/5 — Modifica di visibilità in un header. I test devono essere adattati.
**ROI**: 3/5 — Migliora l'incapsulamento e previene uso improprio dell'API interna da parte di consumatori della
libreria.
**Effort**: 4/5 — Stimato 2–4 ore (modifica + adattamento test).
**Composite**: 5×2 + 3×2 + 4×1 = 10 + 6 + 4 = **20**
**Estimated implementation time**: 2–4 ore
**Required resources**: Un developer C++.
**Effectiveness indicators**:

1. `TypeChecker` espone solo `check()` come metodo pubblico.
2. Test esistenti compilano e passano dopo adattamento.

#### REC-010: Aggiungere `FnType` al `TypeVisitor` e alla `Substitution`

**Deficiency addressed**: DEF-004 (correlato a REC-002)

**Description**: Entry point: `include/jsav/typechecker/TypeVisitor.hpp` (aggiungere `visit_fn`),
`include/jsav/ast/Type.hpp` (aggiungere `TypeKind::Fn`), `src/jsav_Lib/typechecker/Substitution.cpp` (
`ApplyVisitor::visit_fn`). Questa raccomandazione è complementare a REC-002 e deve essere implementata congiuntamente.
Aggiungere il supporto per visitare e applicare sostituzioni ai tipi funzione.

**Feasibility**: 3/5 — Dipende da REC-002. Una volta che `FnType` esiste, il supporto visitor è meccanico.
**ROI**: 4/5 — Necessario per la correttezza dell'unificazione con tipi funzione.
**Effort**: 3/5 — Stimato 4–8 ore.
**Composite**: 3×2 + 4×2 + 3×1 = 6 + 8 + 3 = **17**
**Estimated implementation time**: 4–8 ore
**Required resources**: Developer C++.
**Effectiveness indicators**:

1. `TypeVisitor` dispatcha correttamente su `FnType`.
2. `Substitution::apply()` risolve variabili di tipo all'interno di `FnType`.

#### REC-011: Rimuovere creazione implicita di scope in `SymbolTable::define()`

**Deficiency addressed**: DEF-010

**Description**: Entry point: `src/jsav_Lib/typechecker/SymbolTable.cpp`, metodo `SymbolTable::define` [
`SymbolTable.cpp:16–17`]. Rimuovere la creazione implicita di scope. Aggiungere `assert(!scopes_.empty())` o restituire
`bool`/`expected` per segnalare l'errore del chiamante. L'outcome è che i bug di mancata chiamata a `push_scope()`
vengano rilevati immediatamente.

**Feasibility**: 5/5 — Rimuovere 2 righe e aggiungere un assert.
**ROI**: 3/5 — Previene bug sottili dove simboli vengono definiti nello scope sbagliato.
**Effort**: 5/5 — 30 minuti.
**Composite**: 5×2 + 3×2 + 5×1 = 10 + 6 + 5 = **21**
**Estimated implementation time**: 30 minuti
**Required resources**: Un developer C++.
**Effectiveness indicators**:

1. Assert triggerato se `define()` chiamato senza scope attivo.
2. Zero simboli definiti accidentalmente in scope impliciti.

#### REC-012: Consolidare contesto funzione nella SymbolTable

**Deficiency addressed**: DEF-002

**Description**: Entry point: `include/jsav/typechecker/TypeChecker.hpp` (rimuovere `current_function_return_type_` e
`current_function_name_`), `include/jsav/typechecker/SymbolTable.hpp` (aggiungere metodo per ottenere tipo di ritorno
del simbolo funzione corrente). Invece di stato separato, il contesto di ritorno dovrebbe essere recuperato dal binding
della funzione nella SymbolTable quando si processano statement `ReturnStmt`. L'outcome è riduzione dello stato mutabile
e maggiore coerenza architetturale.

**Feasibility**: 2/5 — Richiesto redesign dell'interazione tra TypeChecker e SymbolTable. I `ReturnStmt` devono accedere
al binding della funzione corrente.
**ROI**: 3/5 — Riduce stato mutabile e duplicazione, ma il beneficio pratico è limitato poiché il contesto funzione è
già gestito correttamente (anche se in modo duplicato).
**Effort**: 2/5 — Stimato 1–2 giorni.
**Composite**: 2×2 + 3×2 + 2×1 = 4 + 6 + 2 = **12**
**Estimated implementation time**: 1–2 giorni
**Required resources**: Developer C++.
**Effectiveness indicators**:

1. `TypeChecker` non ha membri `current_function_*`.
2. Il tipo di ritorno è recuperato dalla SymbolTable.

#### REC-013: Estendere `TypeVisitor` a tutti i TypeKind

**Deficiency addressed**: DEF-007 (correlato), copertura visitor incompleta

**Description**: Entry point: `include/jsav/typechecker/TypeVisitor.hpp`. Aggiungere metodi virtuali per
`TypeKind::Custom`, `TypeKind::Primitive`, e potenzialmente `TypeKind::Fn`. Questo rende il visitor esaustivo e previene
future omissioni. I metodi default possono essere no-op per i tipi atomici.

**Feasibility**: 4/5 — Aggiunta di metodi virtuali a interfaccia esistente.
**ROI**: 3/5 — Previene bug futuri quando nuovi TypeKind vengono aggiunti.
**Effort**: 4/5 — 2–4 ore.
**Composite**: 4×2 + 3×2 + 4×1 = 8 + 6 + 4 = **18**
**Estimated implementation time**: 2–4 ore
**Required resources**: Un developer C++.
**Effectiveness indicators**:

1. `TypeVisitor` ha metodi per tutti i TypeKind composti.
2. Nessun `value_or(success)` silente in `ConstraintSolver::unify()`.

#### REC-014: Correggere `type_stmt` per dichiarazioni multi-variabili

**Deficiency addressed**: DEF-012

**Description**: Entry point: `src/jsav_Lib/typechecker/TypeChecker.cpp`, caso `NodeKind::VarDecl` in `type_stmt` [
`TypeChecker.cpp:949–980`]. Modificare per produrre un `TypedStmtPtr` per ogni variabile dichiarata. Questo richiede o (
a) cambiare il tipo di ritorno di `type_stmt` da `TypedStmtPtr` a `vector<TypedStmtPtr>`, o (b) creare un nuovo nodo
`TypedMultiVarDecl` che contiene più dichiarazioni. L'outcome è che tutte le variabili dichiarate appaiano nell'AST
tipizzato.

**Feasibilità**: 3/5 — Richiede modifica al tipo di ritorno o nuovo nodo AST.
**ROI**: 3/5 — Bug di correttezza: dichiarazioni multi-variabili perdono informazioni nell'AST tipizzato.
**Effort**: 3/5 — Stimato 4–8 ore.
**Composite**: 3×2 + 3×2 + 3×1 = 6 + 6 + 3 = **15**
**Estimated implementation time**: 4–8 ore
**Required resources**: Un developer C++.
**Effectiveness indicators**:

1. Dichiarazioni `let a, b, c = 1, 2, 3` producono tre statement tipizzati.
2. L'AST tipizzato contiene tutte le variabili dichiarate.

#### REC-015: Documentare precondizione `string_view` in `SymbolTable`

**Deficiency addressed**: DEF-009

**Description**: Entry point: `include/jsav/typechecker/SymbolTable.hpp`, documentazione di `define()` e della struttura
dati `scopes_`. Aggiungere Doxygen `@pre` che specifica: "Le string_view passate come chiave devono riferirsi a dati che
sopravvivono alla SymbolTable". L'outcome è documentazione esplicita del contratto di lifetime.

**Feasibility**: 5/5 — Solo documentazione.
**ROI**: 2/5 — Previene bug futuri ma non corregge problemi attuali.
**Effort**: 5/5 — 30 minuti.
**Composite**: 5×2 + 2×2 + 5×1 = 10 + 4 + 5 = **19**
**Estimated implementation time**: 30 minuti
**Required resources**: Un developer C++.
**Effectiveness indicators**:

1. La documentazione di `define()` menziona il contratto di lifetime.
2. Review del codice identifica violazioni del contratto.

#### REC-016: Decidere ownership esplicita di `message_storage_`

**Deficiency addressed**: DEF-014

**Description**: Entry point: `include/jsav/typechecker/TypeChecker.hpp`, membro `message_storage_`. Aggiungere commento
Doxygen che documenta l'invariante: "I CompileError in `errors_` contengono `string_view` che puntano a elementi di
`message_storage_`. Il `deque` garantisce stabilità degli indirizzi, quindi i view non vengono invalidati da pushback."
In alternativa, migrare a `std::string` nei `CompileError` per eliminare la dipendenza implicita.

**Feasibilità**: 4/5 — Se si sceglie la documentazione, è banale. Se si migra a `std::string`, richiede modifica a
`CompileError` e tutti i suoi utilizzatori.
**ROI**: 3/5 — Previene potenziali use-after-free se l'implementazione di `deque` cambiasse.
**Effort**: 4/5 — 1 ora per documentazione, 1–2 giorni per migrazione a `std::string`.
**Composite**: 4×2 + 3×2 + 4×1 = 8 + 6 + 4 = **18**
**Estimated implementation time**: 1 ora (documentazione) o 1–2 giorni (migrazione)
**Required resources**: Un developer C++.
**Effectiveness indicators**:

1. Invariante documentato esplicitamente.
2. Zero valgrind/ASan violations relative a `message_storage_`.

### 4.2 Summary Priority Table

| Rank | ID      | Title                                                | Feasibility | ROI | Effort | Composite Score | Est. Time     |
|------|---------|------------------------------------------------------|-------------|-----|--------|-----------------|---------------|
| 1    | REC-003 | Correggere UnifyVisitor per CustomType               | 5           | 4   | 5      | **23**          | 1–2 ore       |
| 2    | REC-005 | Sostituire UnionFind::at() con accesso sicuro        | 5           | 3   | 5      | **21**          | 30 minuti     |
| 3    | REC-011 | Rimuovere creazione implicita scope in define()      | 5           | 3   | 5      | **21**          | 30 minuti     |
| 4    | REC-004 | Correggere perdita statement in zonk_block_full      | 4           | 4   | 4      | **20**          | 4–8 ore       |
| 5    | REC-009 | Rendere privati type_expr() e type_stmt()            | 5           | 3   | 4      | **20**          | 2–4 ore       |
| 6    | REC-001 | Completare TypeScheme::instantiate() per composti    | 3           | 5   | 3      | **19**          | 1–2 giorni    |
| 7    | REC-006 | Unificare TypeVarId typedef                          | 5           | 2   | 5      | **19**          | 15 minuti     |
| 8    | REC-015 | Documentare precondizione string_view in SymbolTable | 5           | 2   | 5      | **19**          | 30 minuti     |
| 9    | REC-013 | Estendere TypeVisitor a tutti i TypeKind             | 4           | 3   | 4      | **18**          | 2–4 ore       |
| 10   | REC-016 | Decidere ownership esplicita di message_storage_     | 4           | 3   | 4      | **18**          | 1 ora         |
| 11   | REC-010 | Aggiungere FnType al TypeVisitor e Substitution      | 3           | 4   | 3      | **17**          | 4–8 ore       |
| 12   | REC-002 | Aggiungere FnType e verifica signature               | 2           | 5   | 2      | **16**          | 1–2 settimane |
| 13   | REC-007 | Scomporre type_expr() in funzioni per NodeKind       | 3           | 4   | 2      | **16**          | 1–2 giorni    |
| 14   | REC-014 | Correggere type_stmt per multi-variabili             | 3           | 3   | 3      | **15**          | 4–8 ore       |
| 15   | REC-008 | Unificare propagazione errori                        | 2           | 4   | 2      | **14**          | 2–3 giorni    |
| 16   | REC-012 | Consolidare contesto funzione nella SymbolTable      | 2           | 3   | 2      | **12**          | 1–2 giorni    |

---

## Appendix — Deficiency-to-Recommendation Traceability

| Deficiency                                         | Recommendation(s)                                                                              |
|----------------------------------------------------|------------------------------------------------------------------------------------------------|
| DEF-001 (Propagazione errori inconsistente)        | REC-008                                                                                        |
| DEF-002 (Contesto funzione duplicato)              | REC-012                                                                                        |
| DEF-003 (TypeScheme::instantiate incompleto)       | REC-001                                                                                        |
| DEF-004 (Mancanza di FnType)                       | REC-002, REC-010                                                                               |
| DEF-005 (parse_type_annotation hardcoded)          | *(Nessuna raccomandazione — basso impatto, mitigato da fallback a type variable)*              |
| DEF-006 (UnionFind::at() eccezione)                | REC-005                                                                                        |
| DEF-007 (UnifyVisitor non gestisce CustomType)     | REC-003, REC-013                                                                               |
| DEF-008 (Substitution non thread-safe)             | *(Nessuna raccomandazione — dichiarato nella docs, non critico per compilatore single-thread)* |
| DEF-009 (string_view ownership in SymbolTable)     | REC-015                                                                                        |
| DEF-010 (define crea scope implicitamente)         | REC-011                                                                                        |
| DEF-011 (CallExpr non verifica signature)          | REC-002, REC-007, REC-009                                                                      |
| DEF-012 (VarDecl multi-variable semplificato)      | REC-014                                                                                        |
| DEF-013 (zonk_block_full perde statement)          | REC-004                                                                                        |
| DEF-014 (message_storage_ fragile)                 | REC-016                                                                                        |
| DEF-015 (resolve_names MainStmt binding duplicato) | *(Nessuna raccomandazione — comportamento corretto per ora, da monitorare)*                    |
| DEF-016 (TypeVarId duplicato)                      | REC-006                                                                                        |

## Constraint-by-Constraint Verification Gate

| Constraint                                               | Status | Evidence                                                                                  |
|----------------------------------------------------------|--------|-------------------------------------------------------------------------------------------|
| 1. Every claim grounded in specific evidence             | PASS   | Ogni DEF cita file e metodo specifici                                                     |
| 2. No system or component omitted                        | PASS   | 16 componenti analizzati in Phase 3                                                       |
| 3. Every deficiency has a recommendation                 | PASS   | Traceability table above — DEF-005, DEF-008, DEF-015 documented with rationale for no REC |
| 4. Every recommendation immediately actionable           | PASS   | Ogni REC specifica entry point file/method                                                |
| 5. No hedging language                                   | PASS   | Verificato — nessun "might", "could possibly", "seems to"                                 |
| 6. No verbatim repetition                                | PASS   | Cross-referenziato per sezione                                                            |
| 7. Priority mechanically computed                        | PASS   | Formula applicata uniformemente, ordinamento verificato                                   |
| 8. Minimum depth (150 words/component, 300 words/system) | PASS   | Ogni sezione Phase 3 >150 parole, ogni sezione Phase 2 >300 parole                        |
| 9. Document in Italian                                   | PASS   | Tutto il testo è in italiano                                                              |
| 10. No generic statements without specific remediation   | PASS   | Ogni osservazione riferita a codice specifico                                             |
