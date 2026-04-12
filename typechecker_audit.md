# Audit dell'Implementazione del Type Checker

## Fase 1 — Analisi dell'Insieme dei Sistemi

### 1.1 Enumerazione dei Sistemi

Di seguito viene elencrato ogni sistema presente nell'albero del codice del type checker (`include/jsav/typechecker/`, `src/jsav_Lib/typechecker/`).

---

**a) `TypeChecker`** — Orchestratore principale del type checking

- **Responsabilità primaria**: eseguire l'intera pipeline di type checking su un programma AST non tipizzato, producendo un AST completamente tipizzato (`TypedProgram`) e una lista di errori (`CompileError`). La pipeline è articolata in quattro fasi: (1) name resolution, (2) constraint generation, (3) constraint solving, (4) zonking.
- **Ruolo rispetto agli altri sistemi**: è il sistema **core** che consuma tutti gli altri. Riceve input dal parser (AST non tipizzato), utilizza `SymbolTable` per la name resolution, `ConstraintSet` + `ConstraintSolver` per l'unificazione, `Substitution` per lo zonking, e `TypeVariable`/`ErrorType` per la generazione e gestione dei tipi. Tutti gli altri sistemi sono **a monte** (forniscono servizi), `TypeChecker` è il consumatore finale.

---

**b) `ConstraintSolver`** — Risolutore di vincoli tramite unificazione

- **Responsabilità primaria**: risolvere un insieme di vincoli di tipo (`ConstraintSet`) producendo una mappa di sostituzione (`Substitution`) che unifichi tutte le coppie di tipi vincolate. Utilizza `UnionFind` per l'unificazione efficiente con path compression e union by rank.
- **Ruolo**: sistema **midstream**. Riceve vincoli dal `TypeChecker` (Fase 2), produce una `Substitution` consumata dal `TypeChecker` nella Fase 4 (zonking). Dipende da `UnionFind`, `Substitution`, `TypeVisitor`, `ErrorType`.

---

**c) `SymbolTable`** — Tabella dei simboli con scope annidati

- **Responsabilità primaria**: gestire il mapping identificatore → `TypeScheme` con supporto per scope annidati, shadowing e contesto di ritorno delle funzioni. Implementa uno stack di scope (`std::vector<std::unordered_map<...>>`).
- **Ruolo**: sistema **upstream**. Utilizzato dal `TypeChecker` durante la name resolution (Fase 1) e la constraint generation (Fase 2) per risolvere identifier e recuperare contesti di tipo. Dipende da `TypeScheme`.

---

**d) `TypeScheme`** — Schema di tipo polimorfico (∀)

- **Responsabilità primaria**: rappresentare tipi polimorfici con variabili quantificate universalmente (es. `∀T. T → T`). Fornisce `instantiate()` per creare istanze monomorfiche con fresh type variables e `mono()` per creare scheme monomorfici.
- **Ruolo**: sistema **di supporto**. Utilizzato da `SymbolTable` per i binding e da `TypeChecker` durante la risoluzione degli identifier. Dipende da `TypeVariable`.

---

**e) `TypeVariable`** — Variabile di tipo (?T1, ?T2, …)

- **Responsabilità primaria**: rappresentare tipi sconosciuti durante l'inferenza. Ogni istanza ha un `TypeVarId` univoco generato da un contatore thread-local.
- **Ruolo**: sistema **fondazionale**. Utilizzato da `TypeChecker`, `ConstraintSolver`, `Substitution`, `TypeScheme`, `UnionFind`. È il mattone fondamentale dell'inferenza.

---

**f) `Constraint` / `ConstraintSet`** — Vincoli di uguaglianza tra tipi

- **Responsabilità primaria**: `Constraint` rappresenta un'uguaglianza `lhs = rhs` con origine sorgente e motivazione. `ConstraintSet` è un accumulatore ordinato che assegna ID univoci e fornisce accesso sequenziale e per-ID.
- **Ruolo**: sistema **midstream**. Generato dal `TypeChecker` (Fase 2), consumato dal `ConstraintSolver` (Fase 3).

---

**g) `Substitution`** — Mappatura variabili di tipo → tipi risolti

- **Responsabilità primaria**: memorizzare e applicare sostituzioni di type variables a tipi concreti. Include un cache persistente (`apply_cache_`) keyed su `const TypeBase*` per ottimizzare chiamate ripetute di `apply()`.
- **Ruolo**: sistema **midstream/downstream**. Prodotto dal `ConstraintSolver`, consumato dal `TypeChecker` durante lo zonking (Fase 4).

---

**h) `UnionFind`** — Struttura disjoint-set per unificazione

- **Responsabilità primaria**: implementare disjoint-set union con path compression e union by rank per operazioni O(α(n)). Traccia le classi di equivalenza di type variables unificate.
- **Ruolo**: sistema **fondazionale di supporto**. Utilizzato internamente dal `ConstraintSolver`. Non esposto al `TypeChecker` direttamente.

---

**i) `TypeVisitor`** — Interfaccia visitor per tipi composti

- **Responsabilità primaria**: fornire dispatch strutturale su `ArrayType` e `VectorType` senza duplicare logica `switch-on-TypeKind`. Pattern visitor con metodi virtuali puri.
- **Ruolo**: sistema **trasversale**. Utilizzato da `ConstraintSolver` (occurs-check, unificazione) e `Substitution` (apply). Fornisce infrastruttura di traversata.

---

**j) `ErrorType`** — Tipo errore singleton per error recovery

- **Responsabilità primaria**: rappresentare un tipo errore che si unifica silenziosamente con qualsiasi tipo, prevenendo errori a cascata da una singola causa radice. Singleton accessibile tramite `error_type()`.
- **Ruolo**: sistema **trasversale di supporto**. Utilizzato dal `TypeChecker` per inserire placeholder negli errori e dal `ConstraintSolver` per unificazione silenziosa.

---

### 1.2 Mappa delle Dipendenze tra Sistemi

#### a) Diagramma ASCII

```text
                         ┌─────────────────┐
                         │  TypeChecker    │  (core orchestrator)
                         │  (Fasi 1-4)     │
                         └────────┬────────┘
                                  │
          ┌───────────────────────┼────────────────────────┐
          │                       │                        │
          ▼                       ▼                        ▼
  ┌───────────────┐      ┌─────────────────┐      ┌──────────────────┐
  │ SymbolTable   │      │ ConstraintSet   │      │  Substitution    │
  │ (scope mgmt)  │      │ (constraint     │      │  (type var →     │
  │               │      │  accumulator)   │      │   type map)      │
  └───────┬───────┘      └───────┬─────────┘      └────────┬─────────┘
          │                      │                          │
          ▼                      ▼                          │
  ┌───────────────┐      ┌─────────────────┐               │
  │ TypeScheme    │      │ConstraintSolver │               │
  │ (polymorphic  │      │ (unification)   │───────────────┘
  │  types)       │      └───────┬─────────┘
  └───────┬───────┘              │
          │                      │
          ▼                      ▼
  ┌───────────────┐      ┌─────────────────┐
  │ TypeVariable  │◄─────│   UnionFind     │
  │ (type vars)   │      │ (disjoint-set)  │
  └───────┬───────┘      └─────────────────┘
          │
          ▼
  ┌───────────────┐      ┌─────────────────┐
  │  ErrorType    │◄─────│  TypeVisitor    │
  │ (error recov.)│      │ (compound type  │
  └───────────────┘      │  dispatch)      │
                         └─────────────────┘
```

#### b) Classificazione Upstream / Downstream

| Sistema | Posizione | Motivazione |
|---------|-----------|-------------|
| `TypeVariable` | **Upstream** (fondazionale) | Fornisce il tipo primitivo dell'inferenza; tutti gli altri sistemi lo consumano |
| `ErrorType` | **Upstream** (fondazionale) | Tipo base utilizzato da TypeChecker e ConstraintSolver |
| `TypeVisitor` | **Upstream** (infrastruttura) | Interfaccia di traversata; nessun altro sistema dipende da lui direttamente, ma è chiamato da più sistemi |
| `UnionFind` | **Upstream** (supporto) | Usato solo da `ConstraintSolver` internamente |
| `TypeScheme` | **Midstream** | Dipende da `TypeVariable`, è consumato da `SymbolTable` e `TypeChecker` |
| `SymbolTable` | **Midstream** | Dipende da `TypeScheme`; consumato da `TypeChecker` (Fase 1 e 2) |
| `Constraint` / `ConstraintSet` | **Midstream** | Generato da `TypeChecker` (Fase 2), consumato da `ConstraintSolver` (Fase 3) |
| `Substitution` | **Downstream** | Prodotto da `ConstraintSolver`, consumato da `TypeChecker` (Fase 4) |
| `ConstraintSolver` | **Midstream** | Dipende da `UnionFind`, `Substitution`, `TypeVisitor`, `ErrorType`; produce `Substitution` |
| `TypeChecker` | **Downstream** (core) | Consuma tutti gli altri sistemi; orchestratore finale |

#### c) Nodi critici

- **Alto fan-in** (`TypeVariable`, `ErrorType`, `Substitution`): molti sistemi dipendono da questi. `TypeVariable` è il **collo di bottiglia potenziale** — se il contatore thread-local o la gestione degli ID presenta difetti, l'intero sistema di inferenza è compromesso.
- **Alto fan-out** (`TypeChecker`): dipende da tutti gli altri sistemi. Questo è accettabile per un orchestratore, ma rende il `TypeChecker` un **god class** potenziale (1213 righe nel `.cpp`).

### 1.3 Valutazione della Coerenza Architettonica

#### a) Separazione delle responsabilità

**Giudizio: parzialmente coerente.**

I sistemi sono ben delineati a livello concettuale: `SymbolTable` gestisce scope, `ConstraintSolver` gestisce unificazione, `Substitution` gestisce mappature, `TypeChecker` orchestra. Tuttavia:

- **DEF-001**: `TypeChecker` viola il Single Responsibility Principle a livello di sistema. Gestisce name resolution, constraint generation, constraint solving E zonking in un'unica classe di 1213 righe. La classe ha 18+ metodi privati (uno per tipo di espressione) + 14+ metodi di zonking + 6+ metodi di gestione statement. (`TypeChecker.hpp:54–92`)
- **DEF-002**: `TypeChecker::zonk_type()` è una funzione statica libera che duplica parzialmente la logica di `Substitution::apply()`. Entrambe ricorsivamente risolvono type variables in tipi composti. (`TypeChecker.cpp:44–69` vs `Substitution.cpp:48–63`) — *Parzialmente mitigato da REC-005: tutti i `dynamic_cast` in `zonk_type()` sono stati sostituiti con `classof()` + `static_cast`, ma la duplicazione logica persiste (vedi REC-002).*

#### b) Coerenza dell'organizzazione dei moduli

**Giudizio: coerente.**

La struttura fisica (`include/jsav/typechecker/` + `src/jsav_Lib/typechecker/`) rispecchia fedelmente la decompositione logica. Ogni sistema ha il proprio file `.hpp`/`.cpp` dedicato. I naming convention sono uniformi: `PascalCase` per classi/struct, file nominati dopo il concetto primario.

#### c) Qualità dei confini inter-sistema

**Giudizio: parzialmente definito.**

- Le interfacce sono esplicite e tipizzate: `TypeChecker::check()` riceve `const Program&` e restituisce `TypeCheckResult`. `ConstraintSolver::solve()` riceve `const ConstraintSet&` e restituisce `SolverResult`.
- **DEF-003**: `TypeChecker` accede direttamente ai membri interni di `TypedFuncDecl`, `TypedVarDecl`, ecc. durante lo zonking, tramite `static_cast`. Non esistono interfacce astratte di zonking; il `TypeChecker` conosce la struttura interna di ogni tipo tipizzato. (`TypeChecker.cpp:194–320`)
- **DEF-004**: `ConstraintSolver::unify()` è dichiarato `public` nell'header (`ConstraintSolver.hpp:83`) ma è concepito come metodo interno del solver. L'esposizione pubblica permette a chiamate esterne di invocare unificazione isolata, potenzialmente bypassando il ciclo di solving.

**Giudizio sintetico**: architettura **parzialmente coerente**. La decompositione a livello di file è buona, ma il `TypeChecker` centralizza troppe responsabilità e esistono duplicazioni funzionali tra sottosistemi.

### 1.4 Analisi dei Preoccupazioni Trasversali (Cross-Cutting Concerns)

#### Matrice delle preoccupazioni trasversali

| Preoccupazione | TypeChecker | ConstraintSolver | SymbolTable | Substitution | Uniformità? |
|---|---|---|---|---|---|
| **Propagazione errori** | `std::vector<CompileError>` | `std::expected<void, CompileError>` per unify; `std::vector<CompileError>` in SolverResult | N/A (non genera errori) | N/A | ❌ **Inconsistente** |
| **Risoluzione simboli** | Delega a `SymbolTable` | N/A | Implementazione diretta | N/A | ✅ Centralizzato |
| **Gestione scope** | Delega a `SymbolTable` | N/A | Implementazione diretta | N/A | ✅ Centralizzato |
| **Rappresentazione tipi** | `TypePtr` = `shared_ptr<const TypeBase>` | `TypePtr` | `TypePtr` in `TypeScheme` | `TypePtr` | ✅ Centralizzato |
| **Unificazione/normalizzazione** | `zonk_type()` (libera) | `unify()` + visitor | N/A | `apply()` + visitor | ❌ **Frammentato** |

#### a) Propagazione errori

**DEF-005**: Inconsistenza nel modello di errore. Il `TypeChecker` raccoglie errori in un `std::vector<CompileError>` (side-effect su membro `errors_`), mentre il `ConstraintSolver` usa `std::expected<void, CompileError>` per `unify()` ma poi accumula in `SolverResult::errors` come `std::vector`. Il risultato è che il `TypeChecker` deve fare merge manuale: `errors_.insert(errors_.end(), solver_result.errors.begin(), ...)`. (`TypeChecker.cpp:85`)

**DEF-006**: `ErrorType` permette unificazione silenziosa con qualsiasi tipo, ma il `TypeChecker` continua a generare vincoli anche dopo aver inserito `ErrorType` come tipo di un'espressione. Questo può mascherare errori reali. Ad esempio, `type_identifier()` restituisce `error_type()` per identifier non dichiarati, ma i vincoli successivi potrebbero unificare `ErrorType` con tipi concreti senza segnalare il problema. (`TypeChecker.cpp:521–526`, `ConstraintSolver.cpp:69–71`)

#### b) Risoluzione simboli

Centralizzata nel `SymbolTable`. Nessun sistema duplica la logica di lookup. ✅

#### c) Gestione scope

Centralizzata nel `SymbolTable` tramite stack di `unordered_map`. Il `TypeChecker` chiama `push_scope()`/`pop_scope()` correttamente. ✅

Tuttavia:

- **DEF-007**: Il contesto di ritorno delle funzioni (`set_function_return_context`) utilizza un "marker sintetico" con chiave `"__function_context__"` nello scope. Questo è un hack che inquina lo spazio dei nomi dei simboli. Un nome di variabile reale `"__function_context__"` nel linguaggio sorgente causerebbe collisione. (`SymbolTable.cpp:37–43`)

#### d) Rappresentazione dei tipi

Unificata su `TypePtr = std::shared_ptr<const TypeBase>`. Tutti i sistemi usano lo stesso tipo. ✅

Tuttavia:

- **DEF-008**: La normalizzazione/unificazione è frammentata. `Substitution::apply()` risolve type variables ricorsivamente con caching. `TypeChecker::zonk_type()` fa la stessa cosa ma senza caching e con `dynamic_cast` invece del visitor pattern. Due implementazioni parallele della stessa operazione. (`TypeChecker.cpp:44–69` vs `Substitution.cpp:48–63`) — *Parzialmente mitigato da REC-005: `dynamic_cast` eliminato, ma la duplicazione funzionale persiste (vedi REC-002).*

---

## Fase 2 — Analisi per Sistema

### Sistema: `TypeChecker`

#### 2.1 Panoramica del Sistema

**Scopo**: `TypeChecker` è l'orchestratore della pipeline di type checking Hindley-Milner basato su vincoli. Trasforma un `Program` (AST non tipizzato) in un `TypedProgram` (AST completamente tipizzato), producendo nel contempo una lista di `CompileError`. Risolve quattro problemi distinti: (1) name resolution — popolare la tabella dei simboli con binding di identificatori, (2) constraint generation — traversare l'AST e generare vincoli di uguaglianza tra tipi, (3) constraint solving — delegare al `ConstraintSolver` per ottenere una sostituzione unificante, (4) zonking — applicare la sostituzione all'AST tipizzato per risolvere le variabili di tipo residue.

**Ambito**: Il sistema opera su AST completi (`Program`). Non gestisce parsing (a monte), né generazione di codice (a valle). Non esegue type checking incrementale — ogni chiamata a `check()` ricrea tutti gli stati da zero.

**Posizione nella pipeline**: Si colloca tra il parser (input: `Program`) e il code generator (output: `TypedProgram`). È il sistema centrale del front-end semantico.

**Contesto di attivazione**: Istanziato una volta per unità di compilazione. `check()` è il punto di ingresso principale; `type_expr()` e `type_stmt()` sono esposti pubblicamente per unit testing ma sono concettualmente interni alla pipeline.

**Stato**: Stateful all'interno di una singola chiamata a `check()`. I membri `symbols_`, `constraints_`, `errors_`, `typed_stmts_` vengono resettati all'inizio di ogni chiamata.

#### 2.2 Organizzazione Interna dei Moduli

**Inventario file**:
- `TypeChecker.hpp` — Dichiarazione della classe con tutti i metodi pubblici e privati
- `TypeChecker.cpp` — Implementazione completa (1213 righe)

**Confini dei moduli**: Tutto il type checking risiede in un singolo file. Questo è un **god file** — concentra la logica di 4 fasi distinte + typing helpers per 16 tipi di espressione + typing helpers per 10+ tipi di statement + zonking per tutti i nodi tipizzati.

**Organizzazione header**: L'header espone `type_expr()` e `type_stmt()` come metodi pubblici, ma questi sono concettualmente interni. L'header è minimale nelle dipendenze (include solo headers necessari), ma espone troppo dell'interfaccia interna.

**Verdetto**: La decompositione fisica è **inadeguata**. Un file da 1213 righe con responsabilità multiple ostacola la manutenibilità. La logica di name resolution, constraint generation, constraint solving e zonking dovrebbe essere separata in moduli distinti.

#### 2.3 Analisi delle Dipendenze Intra-Sistema

Il `TypeChecker` non ha dipendenze intra-sistema nel senso tradizionale (è un singolo file). Ma internamente:

- I metodi di typing delle espressioni (`type_binary_expr`, `type_call_expr`, ecc.) dipendono tutti da `type_expr()` come dispatch.
- `type_stmt()` dispatcha verso la logica specifica per ogni statement kind.
- `zonk_stmt_full()` e `zonk_expr_full()` dipendono da `zonk_type()` (funzione libera).
- **Ciclo logico**: `type_call_expr` → `type_expr` → `type_identifier` → `SymbolTable::lookup` → `TypeScheme::instantiate` → `fresh_type_variable`. Non è un ciclo nel grafo delle dipendenze fisiche, ma crea una catena di chiamate profonda.

**Accoppiamento stretto**: `TypeChecker` accede direttamente ai campi privati di `FuncDecl`, `VarDecl`, `BinaryExpr`, ecc. tramite `static_cast`. Questo accoppiamento è inevitabile data l'architettura AST, ma rende il `TypeChecker` fragile rispetto a cambiamenti nell'AST.

**DEF-009**: La funzione libera `parse_type_annotation()` (`TypeChecker.cpp:19–35`) è duplicata concettualmente — la stessa logica di parsing di stringhe tipo dovrebbe risiedere in un modulo dedicato, non come helper locale del type checker.

#### 2.4 Flusso Logico

**Punto di ingresso**: `TypeChecker::check(const Program&)` (`TypeChecker.cpp:72–89`).

**Elaborazione input**:

1. Reset dello stato: `symbols_`, `constraints_`, `errors_`, `typed_stmts_` vengono azzerati.
2. **Fase 1 — Name Resolution**: `resolve_names(program)` (`TypeChecker.cpp:92–97`) entra nello scope globale e itera su tutti gli statement. `resolve_names_stmt()` dispatcha su `NodeKind`: per `FuncDecl`, registra il nome funzione con tipo fresh e crea uno scope per parametri e corpo; per `VarDecl`, crea binding fresh per ogni nome; per `BlockStmt`, push/pop scope.

3. **Fase 2 — Constraint Generation**: `generate_constraints(program)` (`TypeChecker.cpp:158–163`) chiama `type_stmt()` su ogni statement. `type_stmt()` dispatcha su `NodeKind` e per ogni espressione chiama `type_expr()`, che a sua volta dispatcha sul tipo di espressione. Ogni typing helper genera vincoli tramite `constraints_.add()`.

4. **Fase 3 — Constraint Solving**: `solve_constraints()` (`TypeChecker.cpp:166–169`) crea un `ConstraintSolver` e chiama `solve()`. Gli errori del solver vengono mergeati in `errors_`.

5. **Fase 4 — Zonking**: `zonk(subst)` (`TypeChecker.cpp:172–189`) applica la sostituzione a ogni statement tipizzato, producendo l'AST finale.

**Produzione output**: Restituisce `TypeCheckResult{TypedProgram, errors}`.

**Gestione errori**: Gli errori vengono accumulati in `errors_` durante tutte le fasi. Il type checking non si ferma al primo errore — continua processando l'intero programma (error recovery tramite `ErrorType`).

#### 2.5 Punti Critici

- **DEF-010** (`TypeChecker.cpp:292–296`): `zonk_block_full()` scarta statement zonked che risultano `nullptr` senza registrare errori. Se `zonk_stmt_full()` restituisce `nullptr` per un statement valido, questo scompare silenziosamente dall'AST finale.
- **DEF-011** (`TypeChecker.cpp:573–607`): `type_binary_expr()` ha complessità ciclomatica elevata (NOLINT comment conferma `*-function-cognitive-complexity`). La logica per `Add` con stringhe/char è duplicata: il check `is_numeric()` viene fatto due volte — una nel blocco principale e una nel blocco post-switch per bitwise.
- **DEF-012** (`TypeChecker.cpp:685–689`): `type_call_expr()` non gestisce il caso in cui il callee non sia un `Identifier`. Se il callee è un'espressione complessa (es. `(f)(x)` o `obj.method(x)`), il tipo di ritorno è semplicemente una fresh type variable senza vincoli. Questo significa che chiamate tramite espressioni non-identificatore non type-checkano correttamente.
- **DEF-013** (`TypeChecker.cpp:755–765`): `type_array_literal()` restituisce `nullptr` se il primo elemento non può essere tipizzato, invece di propagare `error_type()`. Questo può causare crash nei chiamanti che dereferenziano il risultato senza check nullo.
- **DEF-014** (`TypeChecker.cpp:108–120`): Durante la name resolution delle `FuncDecl`, il tipo di ritorno del parametro `param.type_annotation` usa direttamente il campo `type_annotation` che è un `std::optional<std::string>` — se l'annotazione è una stringa non riconosciuta da `parse_type_annotation()`, viene creata una fresh type variable, ma il binding nel symbol table non tiene traccia del fatto che l'annotazione era malformata.
- **DEF-015** (`TypeChecker.cpp:328–334`): Nel default case di `zonk_stmt_full`, viene creato un `TypedExprStmt` fittizio con `"unknown"` come identificatore. Questo placeholder potrebbe confondere le fasi downstream che si aspettano statement validi.
- **DEF-016** (`TypeChecker.cpp:1009–1015`): `type_member_expr()` assegna sempre una fresh type variable al risultato, senza vincoli. Questo significa che l'accesso a membri di struct/oggetti non viene type-checkato affatto.

#### 2.6 Implementazioni Parziali o Non Definite

**Inventario completo**:

| Dichiarazione | File | Stato | Impatto |
|---|---|---|---|
| `TypeScheme::instantiate()` | `TypeScheme.hpp:74` / `TypeScheme.cpp:19–37` | **Parziale** | Per tipi composti (Array, Vector), restituisce il body senza sostituire le variabili quantificate. L'inferenza polimorfica è incompleta per tipi non banali. |
| `TypeChecker::type_member_expr()` | `TypeChecker.hpp:89` / `TypeChecker.cpp:850–857` | **Parziale** | Non genera vincoli per i membri; restituisce sempre fresh type variable. |
| `TypeChecker::type_cast_expr()` | `TypeChecker.hpp:90` / `TypeChecker.cpp:859–866` | **Parziale** | Non genera vincoli tra operand e target type; non verifica la validità del cast. |
| `Zone_stmt_full` default case | `TypeChecker.cpp:328–334` | **Parziale** | Gestisce statement non supportati con placeholder invece di fallire esplicitamente. |

**Dettaglio su `TypeScheme::instantiate()`**:

- **Posizione dichiarazione**: `TypeScheme.hpp:74`
- **Stato**: Parziale — gestisce solo il caso in cui `body` sia direttamente un `TypeVariable` (`TypeScheme.cpp:31–34`). Per `ArrayType`, `VectorType`, `CustomType`, restituisce `body` invariato (`TypeScheme.cpp:36`).
- **Impatto**: Le funzioni polimorfiche con tipi di ritorno composti (es. `∀T. [T; N]`) non vengono istanziate correttamente. Ogni referenza alla funzione riutilizza lo stesso tipo invece di creare fresh variables.
- **Dipendenze interessate**: `TypeChecker::type_identifier()` chiama `instantiate()` (`TypeChecker.cpp:525`). Se l'identifier referenzia una funzione con tipo composto polimorfico, il tipo non viene correttamente rinnovato.

---

### Sistema: `ConstraintSolver`

#### 2.1 Panoramica del Sistema

**Scopo**: Risolvere vincoli di uguaglianza tra tipi tramite unificazione strutturale con occurs-check. Produce una `Substitution` che mappa type variables ai loro tipi risolti.

**Ambito**: Opera esclusivamente su vincoli e tipi. Non conosce l'AST, non conosce i simboli. Il suo unico compito è l'unificazione.

**Posizione nella pipeline**: Tra constraint generation (Fase 2) e zonking (Fase 4).

**Contesto di attivazione**: Istanziato da `TypeChecker::solve_constraints()` per ogni chiamata a `check()`. Stateful durante il solving, ma senza stato residuo tra chiamate diverse.

#### 2.2 Organizzazione Interna dei Moduli

**Inventario file**:
- `ConstraintSolver.hpp` — Dichiarazione con `solve()`, `unify()`, `occurs_in()`
- `ConstraintSolver.cpp` — Implementazione con visitor locali (`OccursVisitor`, `UnifyVisitor`)

**Confini**: Il file è coerente. I visitor locali sono definiti come struct anonimi nel `.cpp`, nascosti all'interfaccia pubblica. Buona separazione.

**Verdetto**: Organizzazione **logica e coerente**.

#### 2.3 Analisi delle Dipendenze Intra-Sistema

- `solve()` → `unify()` per ogni constraint
- `unify()` → `occurs_in()` per occurs-check
- `occurs_in()` → `visit_type()` → `OccursVisitor`
- `unify()` → `visit_type()` → `UnifyVisitor` → `unify()` (ricorsione)

**Ciclo**: `UnifyVisitor::visit_array()` → `solver.unify()` → `visit_type()` → `UnifyVisitor` per tipi annidati. Questo è un ciclo ricorsivo intenzionale (unificazione strutturale), non un difetto.

**DEF-017**: `UnionFind` viene utilizzato per track delle classi di equivalenza ma le sue informazioni **non sono sincronizzate con la `Substitution`**. `unify()` chiama sia `union_find_.unite()` che `substitution_.bind()` separatamente. Se un bug diverge i due, l'unificazione produce risultati inconsistenti. (`ConstraintSolver.cpp:89–92`)

#### 2.4 Flusso Logico

**Punto di ingresso**: `ConstraintSolver::solve(const ConstraintSet&)` (`ConstraintSolver.cpp:42–51`).

1. Per ogni constraint nel set, chiama `unify(lhs, rhs, constraint)`.
2. `unify()` gestisce: ErrorType (successo silenzioso), null type (errore E2034), type variable unification (con occurs-check), concrete type equality (con visitor per tipi composti).
3. Se tv1 = tv2: occurs-check, poi unite + bind.
4. Se tv1 = concrete: occurs-check, poi bind.
5. Se concrete = concrete: check kind match, poi visitor per unificazione strutturale.

**Output**: `SolverResult{substitution, errors}`.

#### 2.5 Punti Critici

- **DEF-018** (`ConstraintSolver.cpp:104–108`): L'occur-check fallisce se `t2` contiene `tv1` indirettamente attraverso una sostituzione esistente, ma `occurs_in()` usa `subst.apply(type)` che risolve solo il primo livello. Per tipi profondamente annidati con catene di sostituzioni, l'occurs-check potrebbe non rilevare cicli.
- **DEF-019** (`ConstraintSolver.cpp:66–68`): `occurs_in()` usa `dynamic_cast<const TypeVariable*>` invece di `TypeVariable::classof()`. Questo è inconsistente con il resto del codice che usa `classof()` per RTTI. — ✅ **Risolto da REC-005**.
- **DEF-020** (`ConstraintSolver.cpp:120–139`): Il mismatch di tipi numerici genera un messaggio di errore che suggerisce un cast, ma il solver non tenta alcuna coercizione implicita. Questo è corretto dal punto di vista del tipo, ma il messaggio è fuorviante perché implica che un cast esplicito risolverebbe il problema a livello di vincoli (non è così — il vincolo rimane insoddisfatto).

#### 2.6 Implementazioni Parziali o Non Definite

| Dichiarazione | File | Stato | Impatto |
|---|---|---|---|
| `UnifyVisitor` per tipi Custom/Primitive | `ConstraintSolver.cpp:27–35` | **Parziale** | Gestisce solo Array e Vector. `CustomType` non ha un handler nel visitor — l'unificazione di tipi personalizzati cade nel caso default `value_or({})` che restituisce successo silenzioso. |

---

### Sistema: `SymbolTable`

#### 2.1 Panoramica

**Scopo**: Gestire binding nome → `TypeScheme` con scope annidati e shadowing.

**Ambito**: Solo gestione scope e lookup. Non esegue type checking.

**Posizione**: Upstream, consultato dal `TypeChecker` durante name resolution e constraint generation.

**Contesto**: Push/pop scope guidati dal `TypeChecker`. Stateful durante il checking di un programma.

#### 2.2 Organizzazione Interna

**Inventario**: `SymbolTable.hpp`, `SymbolTable.cpp` (56 righe). File compatto e coerente.

**Verdetto**: Eccellente.

#### 2.3 Dipendenze Intra-Sistema

Nessuna dipendenza interna significativa. Dipende da `TypeScheme` esternamente.

#### 2.4 Flusso Logico

`push_scope()` → emplace_back di un nuovo `unordered_map`. `define()` → `insert_or_assign` nello scope corrente. `lookup()` → reverse iteration sugli scope. `pop_scope()` → pop_back.

#### 2.5 Punti Critici

- **DEF-007** (già citato): Il marker `"__function_context__"` inquina lo spazio dei nomi. (`SymbolTable.cpp:37–43`)
- **DEF-021** (`SymbolTable.cpp:17–19`): `define()` crea uno scope vuoto se lo stack è vuoto. Questo maschera un errore del chiamante che dovrebbe aver chiamato `push_scope()` prima. Sarebbe meglio un `assert(!scopes_.empty())`.

#### 2.6 Implementazioni Parziali

Nessuna. Tutte le dichiarazioni sono implementate completamente.

---

### Sistema: `TypeScheme`

#### 2.1 Panoramica

**Scopo**: Rappresentare tipi polimorfici con variabili quantificate universalmente.

#### 2.2 Organizzazione

`TypeScheme.hpp` (definisce struct), `TypeScheme.cpp` (implementa `mono()` e `instantiate()`). Compatto.

#### 2.5 Punti Critici

- **DEF-010** (già citato): `instantiate()` è incompleto per tipi composti.

---

### Sistema: `TypeVariable`

#### 2.1 Panoramica

**Scopo**: Rappresentare variabili di tipo `?T1`, `?T2` con ID univoci.

#### 2.5 Punti Critici

Nessun punto critico significativo. Implementazione solida e minimale.

---

### Sistema: `Constraint` / `ConstraintSet`

#### 2.1 Panoramica

**Scopo**: Accumulare vincoli di uguaglianza con ID, origine sorgente e motivazione.

#### 2.5 Punti Critici

- **DEF-022** (`Constraint.cpp:17–19`): `ConstraintSet::get()` esegue una ricerca lineare O(n). Per grandi codebase con migliaia di vincoli, questo diventa un collo di bottiglia. Un `unordered_map<ConstraintId, Constraint>` sarebbe O(1).

---

### Sistema: `Substitution`

#### 2.1 Panoramica

**Scopo**: Mappare type variables → tipi risolti con caching persistente per `apply()`.

#### 2.5 Punti Critici

- **DEF-023** (`Substitution.cpp:48–63`): Il caching è keyed su `const TypeBase*`. Se lo stesso tipo logico è rappresentato da due oggetti diversi (duplicazione di nodi), il cache non li riconosce come equivalenti. Questo è tecnicamente corretto (l'identity-based caching è intenzionale), ma può portare a duplicazione di lavoro se l'AST contiene nodi duplicati.

---

### Sistema: `UnionFind`

#### 2.1 Panoramica

**Scopo**: Disjoint-set con path compression e union by rank.

#### 2.5 Punti Critici

- **DEF-024** (`UnionFind.cpp:15–18`): `find()` usa `parent_.at(var)` che lancia `std::out_of_range` se `var` non è stato registrato con `make_set()`. Questo può causare eccezioni non gestite se un caller passa un ID non registrato.

---

### Sistema: `TypeVisitor`

#### 2.1 Panoramica

**Scopo**: Interfaccia visitor per dispatch strutturale su tipi composti.

#### 2.5 Punti Critici

Nessun punto critico. Interfaccia minimale e ben definita.

---

### Sistema: `ErrorType`

#### 2.1 Panoramica

**Scopo**: Tipo errore singleton per error recovery silenziosa.

#### 2.5 Punti Critici

- **DEF-025** (`ErrorType.cpp:13–15`): `error_type()` restituisce un `shared_ptr` da un singleton `static const`. Ogni chiamata incrementa il reference count del singleton. Questo è corretto ma inefficiente — un riferimento `const TypeBase&` sarebbe più appropriato.

---

## Fase 3 — Analisi per Componente

### Sistema: `TypeChecker` › Componente: `TypeChecker` (classe)

#### 3.1 Dichiarazione di Responsabilità

Il componente `TypeChecker` è responsabile di trasformare un AST non tipizzato (`Program`) in un AST completamente tipizzato (`TypedProgram`) attraverso una pipeline a quattro fasi — name resolution, constraint generation, constraint solving e zonking — producendo contestualmente una raccolta di errori di tipo.

⚠️ **Design smell**: Questa è più di una responsabilità. Il componente gestisce sia la trasformazione AST sia la raccolta errori. Idealmente, la raccolta errori dovrebbe essere un componente separato (es. `DiagnosticCollector`).

#### 3.2 Struttura delle Classi

| Campo | Tipo | Visibilità | Ruolo Semantico | Inizializzazione |
|---|---|---|---|---|
| `symbols_` | `SymbolTable` | private | Tabella dei simboli con scope | Default-constructed |
| `constraints_` | `ConstraintSet` | private | Insieme di vincoli di tipo | Default-constructed |
| `errors_` | `std::vector<CompileError>` | private | Raccolta errori accumulati | Default-constructed |
| `message_storage_` | `std::deque<std::string>` | private | Storage proprietario per stringhe `CompileError::message_` (evita dangling `string_view`) | Default-constructed |
| `typed_stmts_` | `std::vector<TypedStmtPtr>` | private | Statement tipizzati durante constraint generation | Default-constructed |
| `function_decls_` | `std::unordered_map<std::string, const FuncDecl*>` | private | Mappa nomi funzione → dichiarazioni per lookup in `CallExpr` | Default-constructed |
| `loop_depth_` | `std::size_t` | private | Profondità di annidamento nei loop (per validazione break/continue) | `= 0` |

**Ereditarietà**: Nessuna. Classe concreta standalone.

**Regola del Cinque**: Distruttore generato dal compilatore (corretto — tutti i membri sono movable). Copy/move: implicitamente generati ma non utilizzati (il TypeChecker è tipicamente usato come istanza stack-local).

#### 3.3 Analisi delle Interfacce

| Metodo | Firma | Precondizioni | Postcondizioni | Contratto |
|---|---|---|---|---|
| `check()` | `TypeCheckResult check(const Program&)` | `program` valido e ben formato | Restituisce `TypedProgram` + errori; stato interno resettato | Idempotente rispetto a chiamate multiple |
| `type_expr()` | `TypedExprPtr type_expr(const Expr&)` | Espressione valida | Restituisce espressione tipizzata o nullptr/error_type | Esposto per testing; concettualmente interno |
| `type_stmt()` | `TypedStmtPtr type_stmt(const Stmt&)` | Statement valido | Restituisce statement tipizzato o placeholder | Esposto per testing; concettualmente interno |

**Discrepanze**: `type_expr()` e `type_stmt()` sono pubblici ma dovrebbero essere privati. La documentazione li descrive come "esposti per unit testing", il quale è un codice smell — i test dovrebbero testare tramite `check()`.

#### 3.4 Logica Implementativa

**Algoritmo principale**: Il type checking segue un approccio **constraint-based Hindley-Milner**:

1. **Name resolution**: Ricorsione strutturale sull'AST. Per ogni nodo, popola il `SymbolTable`. Complessità: O(n) dove n è il numero di nodi AST.
2. **Constraint generation**: Seconda passata ricorsiva. Per ogni espressione, genera vincoli. Complessità: O(n) vincoli generati.
3. **Constraint solving**: Unificazione per ogni vincolo. Con UnionFind, complessità ammortizzata: O(n × α(n)) dove α è la funzione inversa di Ackermann.
4. **Zonking**: Applicazione della sostituzione a ogni nodo tipizzato. Complessità: O(n × d) dove d è la profondità massima delle catene di sostituzione.

**Rami critici**:

- `type_binary_expr()`: 15+ branch per operatore binario. Include logica speciale per Add con stringhe/char.
- `zonk_stmt_full()`: 12 branch per tipo di statement, con ricorsione annidata.

**Pattern ricorsivi**: `type_expr()` e `type_stmt()` sono mutualmente ricorsivi (es. `type_call_expr` → `type_expr` per ogni argomento; `type_stmt(FuncDecl)` → `type_stmt` per ogni statement nel corpo).

#### 3.5 Valutazione della Gestione Errori

**Rilevamento**: Il `TypeChecker` controlla:

- Identifier non dichiarati (`type_identifier`)
- Mismatch di tipo in assegnazioni, return, binary expr
- Occurs-check (delegato al solver)
- Break/continue fuori loop
- Array vuoti
- Numero argomenti funzione

**Rappresentazione**: `CompileError::TypeError()` con `ErrorCode`, messaggio, posizione, hint.

**Propagazione**: Accumulo in `errors_`. Nessuna eccezione — error recovery continua.

**Casi non rilevati**:

- **DEF-013**: `type_array_literal()` restituisce `nullptr` senza errore quando il primo elemento fallisce.
- **DEF-010**: `zonk_block_full()` scarta statement `nullptr` silenziosamente.
- **DEF-016**: `type_member_expr()` non genera errori per membri non risolti.

#### 3.6 Audit della Coerenza dei Tipi

- **DEF-026** (`TypeChecker.cpp:19–35`): `parse_type_annotation()` restituisce `nullptr` per annotazioni sconosciute. Il caller (`type_integer_literal`, `type_cast_expr`, `type_stmt` per `VarDecl`) deve gestire il `nullptr`. Questo è inconsistente — sarebbe meglio restituire `std::expected<TypePtr, ErrorCode>`.
- **DEF-027** (`TypeChecker.cpp:44–69`): `zonk_type()` usa `dynamic_cast` invece di `classof()` + `static_cast`. Questo è più lento e inconsistente con il resto del codice. — ✅ **Risolto da REC-005**.

#### 3.7 Interazioni tra Componenti

Il `TypeChecker` dipende da:

- `SymbolTable` → lookup binding
- `ConstraintSet` → accumulo vincoli
- `ConstraintSolver` → unificazione
- `Substitution` → zonking
- `TypeVariable` → generazione fresh type
- `ErrorType` → placeholder errori

**Accoppiamento**: Forte con l'AST. Il `TypeChecker` conosce ogni variante di `Expr` e `Stmt`. Questo è inevitabile ma rende il componente fragile.

#### 3.8 Opportunità di Ottimizzazione

**Performance**:

- `parse_type_annotation()` usa catena di `if/else`. Sostituire con `std::unordered_map<std::string_view, TypePtr>` per O(1).
- `zonk_type()` duplica `Substitution::apply()`. Eliminare la duplicazione.

**Strutturale**:

- **God class**: Scomporre `TypeChecker` in `NameResolver`, `ConstraintGenerator`, `Zonker`.
- **Duplicazione**: La logica di check numerico in `type_binary_expr()` è duplicata tra il blocco principale e il blocco post-switch.

**Manutenibilità**:

- `type_expr()` e `type_stmt()` hanno complessità cognitiva elevata. Sostituire con pattern Visitor o tabella di dispatch.

---

### Sistema: `ConstraintSolver` › Componente: `ConstraintSolver` (classe)

#### 3.1 Responsabilità

Il componente `ConstraintSolver` è responsabile di risolvere un insieme di vincoli di uguaglianza tra tipi producendo una sostituzione unificante e una lista di errori di unificazione.

#### 3.2 Struttura delle Classi

| Campo | Tipo | Visibilità | Ruolo |
|---|---|---|---|
| `union_find_` | `UnionFind` | private | Traccia classi di equivalenza di type variables |
| `substitution_` | `Substitution` | private | Mappa type variables → tipi risolti |

Nessuna ereditarietà. Classe concreta.

#### 3.3 Analisi delle Interfacce

| Metodo | Firma | Pre | Post | Contratto |
|---|---|---|---|---|
| `solve()` | `SolverResult solve(const ConstraintSet&)` | Vincoli validi | Restituisce substitution + errori | Consuma tutti i vincoli |
| `unify()` | `std::expected<void, CompileError> unify(const TypePtr&, const TypePtr&, const Constraint&)` | Tipi non-null (tranne ErrorType) | Unifica o restituisce errore | Ricorsivo per tipi composti |
| `occurs_in()` | `static bool occurs_in(TypeVarId, const TypePtr&, const Substitution&)` | Tipo valido | true se var occorre in type | Pura funzione |

#### 3.4 Logica Implementativa

**Algoritmo**: Unificazione di Robinson con occurs-check. Per ogni vincolo:

- Se entrambi sono type variables: occurs-check + bind.
- Se uno è type variable: occurs-check + bind alla concrete type.
- Se entrambi sono concrete: check structural equality con visitor.

Complessità: O(n × α(n)) per n vincoli.

#### 3.5 Gestione Errori

`std::expected<void, CompileError>` propaga errori di unificazione in modo esplicito. Good practice.

**DEF-018**: L'occurs-check potrebbe non rilevare cicli indiretti attraverso catene di sostituzioni profonde.

#### 3.6 Coerenza dei Tipi

Tutti i tipi sono `TypePtr`. Coerente.

**DEF-019**: Uso di `dynamic_cast` invece di `classof()` in `occurs_in()`.

#### 3.7 Interazioni

Dipende da `UnionFind` (interna), `Substitution` (interna), `TypeVisitor` (trasversale), `ErrorType` (trasversale).

#### 3.8 Opportunità di Ottimizzazione

- Sostituire `dynamic_cast` con `classof()` + `static_cast`.
- Il visitor `UnifyVisitor` crea un'istanza per ogni chiamata a `unify()`. Potrebbe essere riutilizzato come membro.

---

*(Nota: I componenti rimanenti — SymbolTable, TypeScheme, TypeVariable, Constraint/ConstraintSet, Substitution, UnionFind, TypeVisitor, ErrorType — sono stati analizzati nella Fase 2 e presentano una complessità proporzionalmente minore. Di seguito vengono riportate le analisi Phase 3 per i componenti più significativi.)*

### Sistema: `SymbolTable` › Componente: `SymbolTable`

#### 3.1 Responsabilità

Il componente `SymbolTable` è responsabile di gestire il mapping tra identificatori e i loro `TypeScheme` con supporto per scope annidati e shadowing.

#### 3.2 Struttura

| Campo | Tipo | Visibilità | Ruolo |
|---|---|---|---|
| `scopes_` | `std::vector<std::unordered_map<std::string_view, TypeScheme, StringHash>>` | private | Stack di scope |

#### 3.3 Interfacce

| Metodo | Firma | Pre | Post |
|---|---|---|---|
| `push_scope()` | `void push_scope()` | Nessuna | Nuovo scope vuoto in cima allo stack |
| `pop_scope()` | `void pop_scope()` | `depth() > 0` | Scope rimosso (silenzioso se vuoto) |
| `define()` | `void define(std::string_view, TypeScheme)` | Nessuna | Binding aggiunto/sovrascritto |
| `lookup()` | `std::optional<TypeScheme> lookup(std::string_view)` | Nessuna | Primo binding trovato o nullo |

**DEF-007**: Il marker `"__function_context__"` è un hack.

#### 3.8 Opportunità

- Usare `std::unordered_map<std::string, ...>` invece di `std::string_view` per evitare dangling reference se il caller dealloca la stringa originale. Attualmente funziona perché le stringhe sono string literal o chiavi di mappe con lifetime garantita.

---

### Sistema: `Substitution` › Componente: `Substitution`

#### 3.1 Responsabilità

Il componente `Substitution` è responsabile di memorizzare mappature type-variable → tipo risolto e di applicarle ricorsivamente a tipi arbitrari con caching persistente.

#### 3.2 Struttura

| Campo | Tipo | Visibilità | Ruolo |
|---|---|---|---|
| `bindings_` | `std::unordered_map<TypeVarId, TypePtr>` | private | Mappature attive |
| `apply_cache_` | `mutable std::unordered_map<const TypeBase*, TypePtr>` | private | Cache risultati apply |

#### 3.4 Logica

`applyImpl()`:

1. Check cache → hit = ritorno cached.
2. Se `TypeVariable`: lookup in `bindings_` → ricorsione.
3. Se composto: visitor → costruisce nuovo tipo.
4. Altrimenti: ritorno tipo originale.
5. Memoizza nella cache.

Complessità: O(d) per tipo di profondità d al primo apply; O(1) per chiamate successive (cache hit).

#### 3.6 Coerenza dei Tipi

Tutto coerente. Il caching su `const TypeBase*` è corretto dato che `TypeBase` è immutabile.

#### 3.8 Opportunità

- `apply_cache_` viene invalidato completamente ad ogni `bind()`. Per substitution grandi, questo è costoso. Un approccio incrementale (validare solo i nodi affetti) potrebbe migliorare le prestazioni.

---

## Fase 4 — Raccomandazioni Prioritarie

### 4.1 Registro delle Raccomandazioni

---

#### **REC-001**

**Titolo**: Scomporre `TypeChecker` in moduli dedicati per fase

**Deficienza affrontata**: DEF-001 (Fase 2, §2.1), DEF-002 (Fase 1, §1.3) — `TypeChecker` viola SRP con 1213 righe e 4 responsabilità distinte.

**Descrizione**: Refactorizzare la classe `TypeChecker` in quattro componenti separati: `NameResolver`, `ConstraintGenerator`, `ConstraintSolver` (già esistente ma da integrare meglio), e `Zonker`. Ciascuno avrà un file `.hpp`/`.cpp` dedicato in `include/jsav/typechecker/` e `src/jsav_Lib/typechecker/`. Il `TypeChecker` diventerà un facade che orchestra i quattro componenti.

**Change entry point**: Aprire `TypeChecker.hpp` e identificare i gruppi di metodi per fase: (1) `resolve_names`, `resolve_names_stmt` → `NameResolver`; (2) `generate_constraints`, `type_expr`, `type_stmt` + tutti i typing helper → `ConstraintGenerator`; (3) `solve_constraints` → già in `ConstraintSolver`; (4) `zonk`, `zonk_stmt_full`, `zonk_expr_full`, `zonk_block_full`, `zonk_type` → `Zonker`.

**Criterio di completamento**: Dopo il refactor, nessun file `.cpp` supera le 400 righe. Tutti e quattro i nuovi moduli compilano indipendentemente. I test esistenti passano senza modifiche.

**Feasibility Score**: 2 — Richiede coordinamento significativo: tutti i test che chiamano `type_expr()` e `type_stmt()` pubblicamente dovranno essere aggiornati per usare il nuovo facade.

**Expected ROI**: 5 — Trasformativo: migliora manutenibilità, testabilità e separazione delle responsabilità. Riduce la complessità cognitiva del file principale da 1213 a ~250 righe per modulo.

**Implementation Effort**: 1 — Alto sforzo: 1-3 mesi. Richiede refactoring architetturale con migrazione parallela e testing estensivo.

**Priority Rank**: (2 × 2) + (5 × 2) + (1 × 1) = 4 + 10 + 1 = **15**

**Tempo stimato**: 4–8 settimane

**Risorse richieste**:

- Ruoli: 1 senior compiler engineer + 1 QA engineer per regressione test
- Strumenti: clang-tidy, gcovr per verifica copertura
- Accesso: repository jsav, branch dedicato
- Dipendenze esterne: Nessuna

**Indicatori di efficacia**:

1. Ogni nuovo file `.cpp` ha ≤400 righe dopo il refactor.
2. Zero test falliti dopo la migrazione.
3. Cyclomatic complexity di ogni funzione ≤15 (verificato con lizard).

---

#### **REC-002**

**Titolo**: Unificare `zonk_type()` con `Substitution::apply()`

**Deficienza affrontata**: DEF-002 (Fase 1, §1.3), DEF-008 (Fase 1, §1.4) — Duplicazione della logica di risoluzione type variables tra `zonk_type()` e `Substitution::apply()`.

**Descrizione**: Eliminare la funzione libera `zonk_type()` in `TypeChecker.cpp:44–69` e sostituirla con chiamate a `Substitution::apply()`. Poiché `apply()` già gestisce ricorsione su type variables e tipi composti con caching, `zonk_type()` è ridondante.

**Change entry point**: Aprire `TypeChecker.cpp`, riga 44. Sostituire tutte le chiamate a `zonk_type(subst, type)` con `subst.apply(type)`. Rimuovere la definizione di `zonk_type()`.

**Criterio di completamento**: Tutti i test di zonking passano. Nessun `dynamic_cast` residuo per la risoluzione di type variables nello zonking.

**Feasibility Score**: 5 — Immediatamente eseguibile: entrambi i metodi sono nello stesso codebase e la semantica è identica.

**Expected ROI**: 4 — Significativo: elimina ~25 righe duplicate e aggiunge caching allo zonking (prima assente).

**Implementation Effort**: 5 — Sforzo minimo: poche ore di lavoro.

**Priority Rank**: (5 × 2) + (4 × 2) + (5 × 1) = 10 + 8 + 5 = **23**

**Tempo stimato**: 2–4 ore

**Risorse richieste**:
- Ruoli: 1 engineer
- Strumenti: clang-format, test runner
- Accesso: Nessuno speciale

**Indicatori di efficacia**:
1. Funzione `zonk_type()` rimossa dal codebase.
2. Zero regressioni nei test di zonking.
3. Profiling mostra zero degradazione nelle prestazioni di zonking.

---

#### **REC-003**

**Titolo**: Completare `TypeScheme::instantiate()` per tipi composti

**Deficienza affrontata**: DEF-010 (Fase 2, §2.6) — `TypeScheme::instantiate()` non sostituisce variabili quantificate in `ArrayType`/`VectorType`.

**Descrizione**: Implementare la sostituzione ricorsiva delle variabili quantificate all'interno di tipi composti. Creare un visitor `TypeSchemeInstantiator` che traversa il tipo body e sostituisce ogni occorrenza di una variabile quantificata con una fresh type variable.

**Change entry point**: Aprire `TypeScheme.cpp:19–37`. Dopo il caso `TypeVariable`, aggiungere casi per `ArrayType` e `VectorType` che ricorsivamente chiamano l'instantiation sull'element type.

**Criterio di completamento**: Test per `∀T. [T; N]` produce istanze con fresh type variables diverse ad ogni chiamata.

**Feasibility Score**: 4 — Eseguibile con preparazione minore: richiede creazione di un visitor dedicato.

**Expected ROI**: 5 — Trasformativo: abilita il corretto type checking polimorfico per funzioni che restituiscono array/vector generici.

**Implementation Effort**: 4 — Basso sforzo: sotto due settimane.

**Priority Rank**: (4 × 2) + (5 × 2) + (4 × 1) = 8 + 10 + 4 = **22**

**Tempo stimato**: 1–2 settimane

**Risorse richieste**:

- Ruoli: 1 engineer con conoscenza di Hindley-Milner
- Strumenti: TypeVisitor esistente come base

**Indicatori di efficacia**:

1. Test `TypeChecker_GenericFunction_*` passano con tipi composti.
2. Due chiamate a funzione polimorfica con ritorno `[T]` producono tipi distinti.

---

#### **REC-004**

**Titolo**: Generare vincoli per `type_member_expr` e `type_cast_expr`

**Deficienza affrontata**: DEF-016 (Fase 2, §2.5), TypeScheme incompleto per member access e casting.

**Descrizione**: Attualmente `type_member_expr()` assegna una fresh type variable senza vincoli e `type_cast_expr()` non verifica la validità del cast. Aggiungere: (1) per `type_member_expr()`, generare vincoli tra il tipo dell'oggetto e il tipo del membro richiesto; (2) per `type_cast_expr()`, verificare che il cast sia semanticamente valido (es. numeric→numeric) e generare un errore per cast impossibili.

**Change entry point**: Aprire `TypeChecker.cpp:850–866`. Modificare `type_member_expr()` per lookup del membro nella SymbolTable dell'oggetto e generazione vincoli. Modificare `type_cast_expr()` per validare la coppia source→target.

**Criterio di completamento**: Test con accesso a membro non tipizzato produce errore E2xxx. Test con cast invalido (es. string→bool) produce errore.

**Feasibility Score**: 3 — Richiede conoscenza della struttura dei tipi custom (non ancora implementata nel type checker).

**Expected ROI**: 4 — Significativo: chiude due lacune di type checking che permettono codice mal tipizzato di passare.

**Implementation Effort**: 3 — Moderato: 2-6 settimane.

**Priority Rank**: (3 × 2) + (4 × 2) + (3 × 1) = 6 + 8 + 3 = **17**

**Tempo stimato**: 2–4 settimane

**Risorse richieste**:

- Ruoli: 1 engineer
- Dipendenze: Sistema di tipi custom (strutture/classi) deve essere definito

**Indicatori di efficacia**:

1. Test per member access non risoluto produce errore.
2. Test per cast invalido produce errore.

---

#### ~~**REC-005**~~ ✅ **COMPLETATO**

**Titolo**: Sostituire `dynamic_cast` con `classof()` + `static_cast`

**Stato**: ✅ **COMPLETATO** — 13 occorrenze di `dynamic_cast` sostituite in 4 file. Zero residuali verificati con `grep`. Codice compilato con successo, tutti i test passati.

**File modificati**:
- `TypeScheme.cpp:33` — `TypeVariable`
- `Substitution.cpp:54` — `TypeVariable`
- `ConstraintSolver.cpp:62, 80, 81, 116` — `TypeVariable` × 4
- `TypeChecker.cpp:44, 52, 59, 131, 316, 712, 1192` — `TypeVariable`, `ArrayType`, `VectorType`, `BlockStmt` × 2, `TypedBlockStmt`, `Identifier`

**Deficienza affrontata**: DEF-019 (Fase 2, ConstraintSolver), DEF-027 (Fase 3, TypeChecker) — Uso inconsistente di `dynamic_cast` invece del pattern `classof()`.

**Descrizione**: Sostituire tutti i `dynamic_cast<const TypeVariable*>`, `dynamic_cast<const ArrayType*>`, ecc. con `TypeVariable::classof(ptr) ? static_cast<const TypeVariable*>(ptr) : nullptr`. Questo è più veloce (nessun RTTI runtime) e consistente con il resto del codebase.

**Change entry point**: Aprire `ConstraintSolver.cpp:66`, `Substitution.cpp:50`, `TypeChecker.cpp:47–59`. Sostituire ogni `dynamic_cast`.

**Criterio di completamento**: Zero `dynamic_cast` residui nei file del typechecker (verificabile con `grep`). Tutti i test passano. ✅ **Verificato**.

**Feasibility Score**: 5 — Immediato: pattern meccanico, nessun cambiamento semantico.

**Expected ROI**: 3 — Moderato: migliora performance (RTTI elimination) e consistenza del codice.

**Implementation Effort**: 5 — Minimo: poche ore.

**Priority Rank**: (5 × 2) + (3 × 2) + (5 × 1) = 10 + 6 + 5 = **21**

**Tempo stimato**: 4–8 ore

**Risorse richieste**:

- Ruoli: 1 engineer junior
- Strumenti: grep, clang-tidy

**Indicatori di efficacia**:

1. Zero occorrenze di `dynamic_cast` in `src/jsav_Lib/typechecker/`. ✅ **Verificato**
2. Tutti i test passano senza modifiche. ✅ **Verificato**

---

#### **REC-006**

**Titolo**: Proteggere `UnionFind::find()` da ID non registrati

**Deficienza affrontata**: DEF-024 (Fase 2, UnionFind) — `find()` lancia `std::out_of_range` per ID non registrati.

**Descrizione**: Sostituire `parent_.at(var)` con `parent_.find(var)` e gestire il caso "not found" restituendo un valore sentinella o aggiungendo un `assert`. In alternativa, cambiare il contratto di `find()` per chiamare implicitamente `make_set()` se l'ID non esiste (comportamento "auto-make").

**Change entry point**: Aprire `UnionFind.cpp:15–18`. Sostituire `parent_.at(var)` con lookup sicuro.

**Criterio di completamento**: Test con ID non registrato non lancia eccezione non gestita.

**Feasibility Score**: 5 — Immediato.

**Expected ROI**: 3 — Moderato: previene crash in produzione per bug del caller.

**Implementation Effort**: 5 — Minimo.

**Priority Rank**: (5 × 2) + (3 × 2) + (5 × 1) = 10 + 6 + 5 = **21**

**Tempo stimato**: 1–2 ore

**Risorse richieste**:
- Ruoli: 1 engineer

**Indicatori di efficacia**:
1. Test unitario per `find()` con ID non registrato passa senza eccezioni.

---

#### **REC-007**

**Titolo**: Rimuovere marker `"__function_context__"` dallo SymbolTable

**Deficienza affrontata**: DEF-007 (Fase 1, §1.4), DEF-021 (Fase 2, SymbolTable) — Hack del contesto funzione inquina lo spazio dei nomi.

**Descrizione**: Sostituire il marker `"__function_context__"` con una struttura dedicata `FunctionContext` gestita separatamente dallo SymbolTable. Aggiungere un membro `std::vector<FunctionContext> func_context_stack_` al `TypeChecker` (non al `SymbolTable`). Il `TypeChecker` pusha/popa il contesto funzione in parallelo con lo scope.

**Change entry point**: Aprire `SymbolTable.cpp:37–43` e `SymbolTable.hpp:75–82`. Rimuovere `set_function_return_context` e `get_function_return_context`. Aggiungere `FunctionContext` come membro di `TypeChecker`.

**Criterio di completamento**: Nessun uso di stringhe magiche nello SymbolTable. Test di return type validation passano.

**Feasibility Score**: 4 — Eseguibile con preparazione minore.

**Expected ROI**: 3 — Moderato: migliora la pulizia architetturale e previene collisioni con nomi utente.

**Implementation Effort**: 4 — Basso: sotto due settimane.

**Priority Rank**: (4 × 2) + (3 × 2) + (4 × 1) = 8 + 6 + 4 = **18**

**Tempo stimato**: 1–2 settimane

**Risorse richieste**:
- Ruoli: 1 engineer

**Indicatori di efficacia**:
1. Zero stringhe `"__function_context__"` nel codebase.
2. Tutti i test di return statement validation passano.

---

#### **REC-008**

**Titolo**: Gestire `nullptr` da `type_array_literal()` con error_type

**Deficienza affrontata**: DEF-013 (Fase 2, §2.5) — Restituzione `nullptr` invece di `error_type()`.

**Descrizione**: In `TypeChecker::type_array_literal()`, quando il primo elemento non può essere tipizzato, restituire `std::make_unique<TypedArrayLiteral>(..., error_type(), ...)` invece di `nullptr`. Questo propaga l'errore silenziosamente attraverso la pipeline.

**Change entry point**: Aprire `TypeChecker.cpp:718–720` e `TypeChecker.cpp:732–734`. Sostituire `return nullptr` con return di un `TypedArrayLiteral` con tipo `error_type()`.

**Criterio di completamento**: Nessun `nullptr` restituito dai typing helper. Test con array mal tipizzati non crashano.

**Feasibility Score**: 5 — Immediato.

**Expected ROI**: 4 — Significativo: previene crash da dereferenziazione nullo.

**Implementation Effort**: 5 — Minimo.

**Priority Rank**: (5 × 2) + (4 × 2) + (5 × 1) = 10 + 8 + 5 = **23**

**Tempo stimato**: 1–3 ore

**Risorse richieste**:
- Ruoli: 1 engineer

**Indicatori di efficacia**:
1. Zero `return nullptr` nei typing helper del TypeChecker.
2. Test con array invalidi non producono segmentation fault.

---

#### **REC-009**

**Titolo**: Migliorare `ConstraintSet::get()` a O(1) con unordered_map

**Deficienza affrontata**: DEF-022 (Fase 2, Constraint) — Ricerca lineare O(n) per ID.

**Descrizione**: Sostituire `std::vector<Constraint>` con `std::unordered_map<ConstraintId, Constraint>` in `ConstraintSet`. Mantenere un `std::vector<ConstraintId>` per l'ordinamento di iterazione.

**Change entry point**: Aprire `Constraint.hpp:100–101` e `Constraint.cpp:17–19`. Cambiare il tipo di `constraints_` e aggiornare `add()`, `get()`, `constraints()`.

**Criterio di completamento**: `get()` è O(1). `constraints()` restituisce ancora in ordine di inserimento.

**Feasibility Score**: 5 — Immediato.

**Expected ROI**: 2 — Minore: impatto solo per codebase molto grandi con migliaia di vincoli.

**Implementation Effort**: 4 — Basso: modifica strutturale semplice.

**Priority Rank**: (5 × 2) + (2 × 2) + (4 × 1) = 10 + 4 + 4 = **18**

**Tempo stimato**: 2–4 ore

**Risorse richieste**:
- Ruoli: 1 engineer

**Indicatori di efficacia**:
1. Benchmark mostra O(1) per `get()` su 10K vincoli.

---

#### **REC-010**

**Titolo**: Eliminare esposizione pubblica di `ConstraintSolver::unify()`

**Deficienza affrontata**: DEF-004 (Fase 1, §1.3) — `unify()` è pubblico ma concettualmente privato.

**Descrizione**: Spostare `unify()` nella sezione `private` di `ConstraintSolver.hpp`. Se è necessario per testing, rendere il test class `friend` o esporre un metodo di testing dedicato.

**Change entry point**: Aprire `ConstraintSolver.hpp:83`. Spostare la dichiarazione di `unify()` da `public` a `private`.

**Criterio di completamento**: `unify()` non è accessibile pubblicamente. Test esistenti compilano.

**Feasibility Score**: 5 — Immediato.

**Expected ROI**: 2 — Minore: migliora incapsulamento API.

**Implementation Effort**: 5 — Minimo.

**Priority Rank**: (5 × 2) + (2 × 2) + (5 × 1) = 10 + 4 + 5 = **19**

**Tempo stimato**: 1 ora

**Risorse richieste**:
- Ruoli: 1 engineer

**Indicatori di efficacia**:
1. `ConstraintSolver::unify()` è `private` nell'header.

---

### 4.2 Tabella Riassuntiva delle Priorità

| Rank | ID | Titolo | Feasibility | ROI | Effort | Composite Score | Est. Time |
|------|----|--------|-------------|-----|--------|-----------------|-----------|
| 1 | [REC-002](#rec-002) | Unificare `zonk_type()` con `Substitution::apply()` | 5 | 4 | 5 | 23 | 2–4 hrs |
| 2 | [REC-008](#rec-008) | Gestire `nullptr` da `type_array_literal()` con error_type | 5 | 4 | 5 | 23 | 1–3 hrs |
| 3 | [REC-003](#rec-003) | Completare `TypeScheme::instantiate()` per tipi composti | 4 | 5 | 4 | 22 | 1–2 wks |
| 4 | [~~REC-005~~](#rec-005) ✅ | ~~Sostituire `dynamic_cast` con `classof()` + `static_cast`~~ | 5 | 3 | 5 | 21 | ✅ Done |
| 5 | [REC-006](#rec-006) | Proteggere `UnionFind::find()` da ID non registrati | 5 | 3 | 5 | 21 | 1–2 hrs |
| 6 | [REC-010](#rec-010) | Eliminare esposizione pubblica di `ConstraintSolver::unify()` | 5 | 2 | 5 | 19 | 1 hr |
| 7 | [REC-007](#rec-007) | Rimuovere marker `"__function_context__"` dallo SymbolTable | 4 | 3 | 4 | 18 | 1–2 wks |
| 8 | [REC-009](#rec-009) | Migliorare `ConstraintSet::get()` a O(1) con unordered_map | 5 | 2 | 4 | 18 | 2–4 hrs |
| 9 | [REC-004](#rec-004) | Generare vincoli per `type_member_expr` e `type_cast_expr` | 3 | 4 | 3 | 17 | 2–4 wks |
| 10 | [REC-001](#rec-001) | Scomporre `TypeChecker` in moduli dedicati per fase | 2 | 5 | 1 | 15 | 4–8 wks |

---

*Documento generato il 12 aprile 2026. Audit basato sull'analisi completa di 20 file (10 header + 10 implementazione) nel percorso `include/jsav/typechecker/` e `src/jsav_Lib/typechecker/`.*
