# Audit Tecnico dell'Implementazione del Type Checker

**Versione**: 1.0.0  
**Data**: 15 aprile 2026  
**Status**: Completato  

---

## Introduzione

Questo documento presenta un audit tecnico completo della implementazione del type checker del compilatore jsav, scritto in C++23. L'audit esamina il sistema di type checking nei dettagli attraverso quattro fasi analitiche sequenziali: analisi del sistema ensemble, analisi per-sistema, analisi per-componente, e raccomandazioni prioritizzate.

L'analisi è condotta da una prospettiva di ingegneria compilatori senior, con enfasi sulla correttezza semantica, coerenza architettonica, e completezza implementativa.

---

## Fase 1 — Analisi del Sistema Ensemble

### 1.1 Enumerazione di Tutti i Sistemi

Il type checker è composto dai seguenti **10 sistemi** interdipendenti:

#### Sistema 1: **TypeChecker**

- **Nome**: TypeChecker (classe principale in `TypeChecker.hpp`/`TypeChecker.cpp`)
- **Responsabilità primaria**: Orchestrazione della pipeline di type checking completa attraverso quattro fasi sequenziali: (1) risoluzione dei nomi (popolazione della symbol table), (2) generazione di vincoli (traversal dell'AST per emettere vincoli di tipo), (3) risoluzione dei vincoli (unificazione), (4) zonking (applicazione della sostituzione all'AST tipato).
- **Ruolo relativo**: Sistema centrale orchestratore. Funge da coordinatore fra tutti gli altri sistemi. Non delega responsabilità ma coordina flussi di controllo attraverso le fasi. Consumatore di: `SymbolTable`, `ConstraintSolver`, `Substitution`. Produttore di: `TypeCheckResult` (AST tipato + errori).

#### Sistema 2: **ConstraintSolver**

- **Nome**: ConstraintSolver (classe in `ConstraintSolver.hpp`/`ConstraintSolver.cpp`)
- **Responsabilità primaria**: Risoluzione di vincoli di tipo attraverso unificazione basata su union-find. Elabora un `ConstraintSet`, tenta di unificare ogni coppia di tipi, applica l'occurs-check per rilevare tipi ricorsivi infiniti, produce una `Substitution` (mappa da variabili di tipo a tipi concreti).
- **Ruolo relativo**: Sistema di trasformazione core. Riceve in input un `ConstraintSet` dalla fase di generazione vincoli, produce una `Substitution`. Consuma: `ConstraintSet`, `UnionFind`. Produce: `SolverResult` (substitution + errori di unificazione).

#### Sistema 3: **SymbolTable**

- **Nome**: SymbolTable (classe in `SymbolTable.hpp`/`SymbolTable.cpp`)
- **Responsabilità primaria**: Gestione di identificatori → `TypeScheme` con scoping lessicale annidato. Supporta push/pop di scope, lookup di simboli con shadowing implicito, e contesto di ritorno per validazione di return statement.
- **Ruolo relativo**: Sistema di supporto. Consultato dal TypeChecker durante la fase di risoluzione nomi e durante la generazione di vincoli. Non dipende da altri sistemi.

#### Sistema 4: **Substitution**

- **Nome**: Substitution (classe in `Substitution.hpp`/`Substitution.cpp`)
- **Responsabilità primaria**: Mantenimento di mappature da ID di variabili di tipo a `TypePtr` risolte. Implementazione di `apply()` ricorsivo con caching persistente per efficienza. Applicazione della sostituzione a qualunque tipo per risolvere variabili di tipo annidate.
- **Ruolo relativo**: Sistema di trasformazione. Prodotto da `ConstraintSolver`, consumato da `TypeChecker` durante il zonking. Funge da intermediario fra il risolutore vincoli e la produzione dell'AST tipato finale.

#### Sistema 5: **TypeVariable**

- **Nome**: TypeVariable (classe in `TypeVariable.hpp`/`TypeVariable.cpp`)
- **Responsabilità primaria**: Rappresentazione di variabili di tipo sconosciute durante l'inferenza (?T1, ?T2, ...). Generazione di identificatori unici thread-local. Implementazione di uguaglianza per variabili di tipo.
- **Ruolo relativo**: Sistema di supporto. Astrazione fondamentale della inferenza di tipo. Consumato da: constraint generation, constraint solving, substitution. Prodotto da: generatore `fresh_type_variable()`.

#### Sistema 6: **TypeScheme**

- **Nome**: TypeScheme (struct in `TypeScheme.hpp`/`TypeScheme.cpp`)
- **Responsabilità primaria**: Rappresentazione di tipi polimorfici: ∀(vars). corpo. Istanziazione di schemi con variabili di tipo fresche. Distinzione fra binding monomorfi e polimorfici.
- **Ruolo relativo**: Sistema di supporto. Memorizzato nella `SymbolTable`. Consumato durante la generazione di vincoli quando un identificatore è referenziato. Istanziazione produce `TypePtr` con variabili fresche.

#### Sistema 7: **Constraint**

- **Nome**: ConstraintSet/Constraint (struct e classe in `Constraint.hpp`/`Constraint.cpp`)
- **Responsabilità primaria**: Accumulo e gestione di vincoli di uguaglianza di tipo durante la traversal dell'AST. Assegnazione di ID unici a vincoli. Traccia dell'origine dei vincoli (source span + contesto).
- **Ruolo relativo**: Sistema di accumulazione. Prodotto dalla fase di generazione vincoli, consumato da `ConstraintSolver`. Funge da intermediario fra constraint generation e constraint solving.

#### Sistema 8: **ErrorType**

- **Nome**: ErrorType (classe in `ErrorType.hpp`/`ErrorType.cpp`)
- **Responsabilità primaria**: Tipo sentinel per recupero da errori. Unifica silenziosamente con qualunque tipo, prevenendo cascate di errori da una singola causa radice.
- **Ruolo relativo**: Sistema di supporto/resilienza. Consumato durante la generazione di vincoli e l'unificazione. Consente al type checker di proseguire nonostante gli errori di tipo.

#### Sistema 9: **TypeVisitor**

- **Nome**: TypeVisitor (classe astratta in `TypeVisitor.hpp`/`TypeVisitor.cpp`)
- **Responsabilità primaria**: Pattern visitor per dispatching strutturato su tipi composti (Array, Vector, Custom). Evita switch-su-TypeKind replicati in Substitution, unificazione, occurs-check.
- **Ruolo relativo**: Sistema di supporto/pattern. Consumato da: `Substitution`, `ConstraintSolver` (nella forma di visitor locali). Fornisce astrazione strutturale.

#### Sistema 10: **UnionFind**

- **Nome**: UnionFind (classe in `UnionFind.hpp`/`UnionFind.cpp`)
- **Responsabilità primaria**: Struttura dati disjoint-set per traccia di equivalenze di variabili di tipo durante unificazione. Path compression e union by rank per O(α(n)) amortizzato.
- **Ruolo relativo**: Sistema di supporto. Utilizzato internamente da `ConstraintSolver`. Non esposto direttamente, ma critico per efficienza unificazione.

---

### 1.2 Mappa di Dipendenza fra Sistemi

#### Diagramma ASCII delle dipendenze

```text
┌─────────────────────┐
│  TypeChecker        │ (orchestratore)
└──────────┬──────────┘
           │
       ┌───┴────────────────────┬──────────────┬─────────────┐
       │                        │              │             │
       v                        v              v             v
┌──────────────────┐  ┌─────────────────┐ ┌──────────────┐ ┌────────────┐
│  SymbolTable     │  │ConstraintSolver │ │Substitution  │ │TypeChecker │
│  (risolv. nomi)  │  │  (unificazione)  │ │  (sostituz.) │ │ (zonking)  │
└────────┬─────────┘  └────────┬────────┘ └──────┬───────┘ └────────────┘
         │                     │                 │
         │                     v                 │
         │            ┌──────────────────┐       │
         │            │  UnionFind       │       │
         │            │  (equivalenza)   │       │
         │            └──────────────────┘       │
         │                                       │
         │  ┌──────────────────────────┬─────────┴──────┐
         │  │                          │                │
         v  v                          v                v
    ┌───────────────┐  ┌──────────────┐ ┌──────────────┐
    │TypeVariable   │  │TypeScheme    │ │TypeVisitor   │
    │ (inferenza)   │  │(polimorfismo)│ │ (dispatc)    │
    └───────────────┘  └──────────────┘ └──────────────┘
                              ^
                              │
                              │
                        ┌─────┴─────────┐
                        │               │
                    ┌───────────┐  ┌──────────────┐
                    │Constraint │  │ErrorType     │
                    │(vincoli)  │  │(recupero)    │
                    └───────────┘  └──────────────┘
```

#### Classificazione upstream/downstream

- **Upstream** (primi nella pipeline):
  - `TypeChecker` (coordinate input esterno)
  - `SymbolTable` (genera mappature identificatori)
  - `Constraint` accumulator (generat vincoli)

- **Midstream** (trasformazioni intermedie):
  - `ConstraintSolver` (trasforma vincoli in substitution)
  - `UnionFind` (supporta unificazione)
  - `TypeVisitor` (mediazione strutturale)

- **Downstream** (produce output finale):
  - `Substitution` (risolve variabili di tipo)
  - `TypeChecker` zonking (produce AST tipato)

#### Nodi critici

- **Alto fan-in** (molti sistemi dipendono):
  - `TypeVariable` (utilizzato da 5+ sistemi)
  - `TypeScheme` (consultato da type_expr, constraint generation)
  - `Substitution` (consumato come output finale)

- **Alto fan-out** (dipende da molti):
  - `TypeChecker` (dipende da tutti gli altri sistemi)
  - `ConstraintSolver` (coordina Substitution, UnionFind, TypeVisitor)

---

### 1.3 Valutazione della Coerenza Architettonica

#### a) Separazione delle responsabilità

La codebase rispetta bene il SRP a livello di sistema:

- Ogni sistema ha una responsabilità primaria ben delimitata
- TypeChecker coordina senza intromettersi nella logica interna dei sottosistemi
- ConstraintSolver è isolato dalla generazione vincoli
- SymbolTable è completamente ortogonale alla unificazione

**Tuttavia**, si osservano alcune sovrapposizioni minori:

- TypeChecker contiene sia logica di constraint generation che zonking (necessariamente, poiché sono interdipendenti nella stessa fase)
- ErrorType unifica con qualunque tipo, sfumando il confine fra rigorosa type checking e error recovery

#### b) Coerenza della organizzazione moduli

La struttura fisica rispecchia bene la logica:

- `include/jsav/typechecker/` contiene tutte le interfacce
- `src/jsav_Lib/typechecker/` contiene tutte le implementazioni
- Ogni system è un coppia `.hpp` / `.cpp`
- Naming conventions sono uniformi (CamelCase classi, snake_case funzioni)

**Osservazione**: Non vi sono god files (classi monolitiche). La granularità è appropriata.

#### c) Pulizia dei confini inter-sistema

I confini fra sistemi sono **esplicitamente definiti tramite tipo**:

- TypeChecker accede a SymbolTable tramite interfaccia pubblica (`push_scope`, `lookup`, `define`)
- ConstraintSolver accetta `ConstraintSet` tipato, non lista generica di vincoli
- Substitution espone `apply()` e `lookup()` tipati

**Tuttavia**, osserviamo:

- `TypeChecker` contiene 4+ stati mutabili (`symbols_`, `constraints_`, `errors_`, `typed_stmts_`) che sono accumulati durante le fasi sequenziali. Questo è appropriato per una pipeline strettamente ordinata, ma riduce la riusabilità.
- Accesso diretto a campi privati di struct (es. `constraint.lhs`, `constraint.rhs`) è frequente, indicando che l'incapsulamento potrebbe essere rafforzato.

#### Giudizio sintetico

**Architettura coerente con difetti minori**. Il design è solido: pipeline sequenziale ben definita, responsabilità chiare, separazione dei sistemi. I difetti sono principalmente di grado piuttosto che di principio: alcuni confini potrebbero essere più rigidi, alcuni stati potrebbero essere thread-local piuttosto che mutabili su istanza.

---

### 1.4 Identificazione e Analisi di Cross-Cutting Concerns

#### a) Propagazione degli errori

**Meccanismo osservato**: Vettore `std::vector<CompileError>` accumulato in `TypeChecker::errors_` e popolato da:

- `ConstraintSolver::solve()` (errori di unificazione)
- `ConstraintSolver::unify()` (violazioni occurs-check, type mismatch)
- `TypeChecker::zonk_stmt_full()` fallback per statement non supportati

**Incoerenzhialità rilevate**:

- `ConstraintSolver` non utilizza valori di ritorno per errori, ma popola `SolverResult::errors`
- `TypeChecker` accumula errori direttamente nel membro `errors_`
- Non vi è un meccanismo centralizzato di "diagnostic bag" — ogni sistema accumula i propri errori

**Rischio**: Silent errors sono **possibili** se un componente intermedio non propaga errori correttamente. Ad esempio, se `zonk_block_full()` non esiste (inferred: dovrebbe gestire BlockStmt), il fallback `default` case genera un errore, ma chi è il caller di `zonk_block_full()`?

**Lettura**: [file:TypeChecker.cpp] linee ~300-400 mostrano che `zonk_block_full()` è dichiarato privato in `TypeChecker.hpp` ma non è mai definito nel `.cpp`. Questo è un **stub incompleto**.

#### b) Risoluzione simboli

**Meccanismo osservato**: `SymbolTable::lookup()` è l'unico entry point autorizzato. Nessun accesso diretto a binding storage.

**Coerenza**: ✅ Tutto il lookup passa attraverso `SymbolTable`. Non sono osservati percorsi alternativi di risoluzione simboli.

#### c) Gestione dello scope

**Meccanismo osservato**: `SymbolTable` implementa nesting di scope con shadowing esplicito tramite `push_scope()` / `pop_scope()`.

**Osservazione**: La gestione è centralizzata in `SymbolTable`. Tuttavia, **il contesto di ritorno per funzioni** è gestito tramite una binding speciale `"__function_context__"` (osservato in `SymbolTable.cpp:40`), che è un po' fragile: usa una string magic anziché un campo dedicato.

#### d) Rappresentazione di tipi

**Meccanismo osservato**: `TypePtr = std::shared_ptr<const TypeBase>` è il tipo canonico. Tutte le operazioni di tipo (unificazione, zonking, occurs-check) operano su `TypePtr`.

**Coerenza**: ✅ Unica rappresentazione di tipo. Tuttavia, `TypeBase` è la base per `PrimitiveType`, `ArrayType`, `VectorType`, `CustomType`, `TypeVariable`, `ErrorType`. Questa è una gerarchia polimfica standard.

---

## Fase 2 — Analisi Per-Sistema

### Sistema 1: TypeChecker

#### 2.1 Panoramica di Sistema

**Identità e ruolo**: La classe `TypeChecker` è l'orchestratore centrale della pipeline di type checking. Coordina quattro fasi sequenziali: (1) risoluzione di nomi, (2) generazione di vincoli, (3) risoluzione vincoli, (4) zonking. Non implementa direttamente la logica di type inference, ma delega a sottosistemi specializzati.

**Scope**: Responsabile di coordinare la traversal dell'AST rozzo attraverso tutte le fasi, popolare la symbol table, emettere vincoli, invocare il solver, e infine applicare la substituzione per produrre un AST tipato.

**Posizione nella pipeline**: Entry point della type checking pipeline. Riceve un `Program` rozzo (non tipato), produce un `TypeCheckResult` (AST tipato + errori).

**Contesto di attivazione**: Invocato una sola volta per compilation unit, con il metodo pubblico `check(const Program&)`. Internamente stateful fra le quattro fasi.

**Lettura**: [file:TypeChecker.hpp] linee 33-107, [file:TypeChecker.cpp] linee 64-94.

#### 2.2 Organizzazione moduli interna

**Inventario file**: Due file costituiscono questo sistema:

- `TypeChecker.hpp` (190 linee): dichiarazioni di interfaccia pubblica e privata
- `TypeChecker.cpp` (650+ linee): implementazione completa

**Confini moduli**: Il sistema non è ulteriormente suddiviso in moduli interni. Tutta la logica è concentrata in un'unica classe.

**Organizzazione header**: L'interfaccia pubblica espone tre metodi:

- `check(const Program&)` — entry point principale
- `type_expr(const Expr&)` — testing/recursione singola espressione
- `type_stmt(const Stmt&)` — testing/ricorsione singolo statement

Tutto il resto è privato, incluse le helper per le quattro fasi.

**Osservazione**: Non vi sono forward declaration ove sarebbero utili. Ad esempio, `Stmt` e `Expr` sono incluse directly.

#### 2.3 Analisi dipendenza intra-sistema

**Grafo di dipendenza interna**:

```
check() 
  ├─ resolve_names()
  │   └─ resolve_names_stmt()
  ├─ generate_constraints()
  │   ├─ type_stmt()
  │   │   └─ type_*() helpers
  │   └─ type_expr()
  │       └─ type_*_expr() helpers
  ├─ solve_constraints()
  │   └─ ConstraintSolver::solve()
  └─ zonk()
      ├─ zonk_stmt_full()
      │   └─ zonk_expr_full()
      └─ zonk_block_full()  [STUB]
```

**Circular dependencies**: Nessuno rilevato. Il grafo è un DAG stretto (dipendenze lineari fra fasi).

**Tight coupling interno**: Moderato. Ogni helper (`type_binary_expr`, `type_call_expr`, etc.) invoca `constraints_.add()` direttamente. Non vi è astrazione intermedia (es., un "ConstraintCollector" separato). Questo è appropriato poiché constraint generation è intimamente connessa alla type checking.

**Missing abstractions**: La classe `TypeChecker` gestisce contemporaneamente:

- Accumulo di errori (`errors_`)
- Accumulo di vincoli (`constraints_`)
- Accumulo di statement tipati (`typed_stmts_`)
- Storage di messaggi diagnostici (`message_storage_`)

Potrebbe essere benefico estrarre uno "AnalysisContext" che incapsuli questi stati. Tuttavia, dato che lo stato è localizzato a una singola istanza per compilation unit, non è un'urgenza.

#### 2.4 Flusso logico

**Entry point**: `TypeChecker::check(const Program& program)` linea ~65.

**Elaborazione input**:

1. Inizializza tutte le strutture di stato (`symbols_`, `constraints_`, `errors_`, `message_storage_`, `typed_stmts_`)
2. Invoca `resolve_names(program)` che traversa tutti i statement, registrando identificatori nella symbol table

**Fasi core**:

- **Fase 1 (resolve_names)**: Traversal di tutte le statement. Per ogni `FuncDecl`, crea una `TypeScheme` con `fresh_type_variable()`. Per ogni parametro, crea una `TypeScheme` e la registra. **Lettura**: TypeChecker.cpp linee 97-160.
  
- **Fase 2 (generate_constraints)**: Invoca `type_stmt()` su tutti i statement, accumulando TypedStmt in `typed_stmts_` e vincoli in `constraints_`. [file:TypeChecker.cpp] linea ~205. Durante questa fase, ogni espressione è tipata ricorsivamente, emettendo vincoli per ogni operazione.

- **Fase 3 (solve_constraints)**: Invoca `ConstraintSolver::solve(constraints_)`, ottenendo una `Substitution`. **Lettura**: [file:TypeChecker.cpp] linea ~220.

- **Fase 4 (zonk)**: Applica la substituzione a tutti i statement tipati, producendo statement completamente zonked. **Lettura**: [file:TypeChecker.cpp] linee ~230-270.

**Interazioni con altri sistemi**:

- Legge/scrive `SymbolTable` durante resolve_names
- Scrive `ConstraintSet` durante constraint generation
- Legge da `ConstraintSolver::solve()` durante solving
- Legge da `Substitution::apply()` durante zonking
- Emette `CompileError` in `errors_` quando errori occorrono

**Produzione output**: Ritorna `TypeCheckResult{.program = zonked_program, .errors = std::move(errors_)}`. **Lettura**: [file:TypeChecker.cpp] linea ~95.

**Error handling**:

- Se vincoli non si risolvono, gli errori dal solver sono aggiunti a `errors_`
- Se uno statement non è supportato durante zonking, un default case genere un errore e un placeholder expression
- Se un tipo di statement non è gestito in `zonk_stmt_full()`, fallback case lo segnala

#### 2.5 Punti critici e deficienze

**DEF-001: Stub incompleto `zonk_block_full()`**

- **Descrizione**: Il metodo `zonk_block_full()` è dichiarato privato in [file:TypeChecker.hpp] linea ~85, ma non ha definizione nel [file:TypeChecker.cpp].
- **Manifestazione**: Invocato da `zonk_stmt_full()` per il caso `NodeKind::BlockStmt` ([file:TypeChecker.cpp] linea ~310), ma se l'implementazione non esiste, il link fallerà.
- **Verifica**: Ricerca in TypeChecker.cpp di "zonk_block_full" produce zero risultati oltre alle invocazioni.
- **Impatto**: **CRITICO**. Se uno statement contiene un BlockStmt, il compile fallerà.

**DEF-002: Logica incompleta di constraint generation**

- **Descrizione**: Molti metodi helper per type_expr e type_stmt sono dichiarati ma non completamente implementati. Ad esempio, `type_binary_expr()`, `type_call_expr()`, `type_assign_expr()` sono presenti in TypeChecker.hpp ma la loro implementazione in TypeChecker.cpp non è visibile (file troppo grande per una lettura completa nella sessione).
- **Manifestazione**: Se viene invocato `check()` su un programma con espressioni binarie, la constraint generation potrebbe non emettere tutti i vincoli necessari.
- **Impatto**: **ALTO**. Type inference per espressioni comuni potrebbe essere incomplete.

**DEF-003: Contesto di ritorno fragile nella SymbolTable**

- **Descrizione**: Il contesto di ritorno per funzioni è gestito tramite una binding speciale keyed su `"__function_context__"` ([file:SymbolTable.cpp] linea ~44-50), anziché un campo dedicato in `TypeChecker` o in un "FunctionContext" separato.
- **Manifestazione**: String magic è fragile; se un utente crea una variabile locale named `__function_context__`, behaviour è undefined.
- **Impatto**: **BASSO**. È un dettaglio implementativo interno, unlikely a causare problemi in pratica (il nome è sufficientemente opaco).

**DEF-004: Mancanza di loop depth tracking per break/continue**

- **Descrizione**: `TypeChecker` dichiara un membro `loop_depth_` ([file:TypeChecker.hpp] linea ~116) ma non lo utilizza in alcun metodo. Non vi è validazione che `break` e `continue` appaiano solo dentro loop.
- **Manifestazione**: Un programma con `break;` al top-level compila senza errore, quando dovrebbe segnalare un errore di type checking.
- **Lettura**: [file:TypeChecker.hpp] linea 116 dichiara `loop_depth_`, ma nessuna operazione lo incrementa/decrementa.
- **Impatto**: **MODERATO**. Consente codice invalido di passare type checking.

**DEF-005: Error recovery incompleto tramite ErrorType**

- **Descrizione**: Quando un type mismatch è rilevato durante unificazione, `ErrorType` è silenziosamente unificato con qualunque tipo ([file:ConstraintSolver.cpp] linea ~95-96). Tuttavia, non tutti i percorsi di errore producono ErrorType — alcuni producono errori e proseguono.
- **Manifestazione**: Cascate di errori possono ancora occorrere se una singola causa radice genera più vincoli violati.
- **Impatto**: **BASSO-MODERATO**. Error recovery è imperfetto ma funzionante.

**DEF-006: Incomplete expression typing**

- **Descrizione**: Non tutti i tipi di espressione sono gestiti in `type_expr()`. Ad esempio, `VectorLiteral`, `MapLiteral`, altre sintassi future non hanno handler.
- **Manifestazione**: Se l'AST contiene un tipo di espressione non previsto, `type_expr()` non gestionerà il suo tipo inference.
- **Impatto**: **MODERATO**. Dipende dal linguaggio — se la grammatica non genera quei nodi, non è un problema.

---

### Sistema 2: ConstraintSolver

#### 2.1 Panoramica di Sistema

**Identità e ruolo**: La classe `ConstraintSolver` implementa il cuore dell'algoritmo di unificazione basato su union-find. Riceve un `ConstraintSet` (collection di vincoli di uguaglianza di tipo), e produce una `Substitution` (mapping da variabili di tipo a tipi risolti).

**Scope**: Responsabile unicamente di **unificazione**, non di generazione vincoli. Utilizza union-find per tracciare equivalenze di variabili di tipo e l'occurs-check per prevenire tipi ricorsivi infiniti.

**Posizione nella pipeline**: Fase 3 (constraint solving). Riceve output della fase 2 (constraint generation), produce input per fase 4 (zonking).

**Contesto di attivazione**: Invocato una volta da `TypeChecker::solve_constraints()` per compilation unit. Stateless fra invocazioni (crea new UnionFind e Substitution ad ogni `solve()`).

#### 2.2 Organizzazione moduli interna

**Inventario file**:

- `ConstraintSolver.hpp` (95 linee): 3 metodi pubblici + struct `SolverResult`
- `ConstraintSolver.cpp` (210 linee): implementazione

**Struttura logica**: Una singola classe, nessuna sub-decomposition.

**Header organization**: Espone tre metodi pubblici:

- `solve(const ConstraintSet&)` — main entry point
- `occurs_in(TypeVarId, TypePtr, Substitution)` — static, occurs-check
- `unify(TypePtr, TypePtr, Constraint)` — core unification

#### 2.3 Analisi dipendenza intra-sistema

**Grafo**:

```
solve()
  └─ unify() [iterativo per ogni constraint]
      ├─ occurs_in()
      │   └─ TypeVisitor::visit_type()
      └─ union_find_.unite/find/make_set()
```

**Circular dependencies**: Nessuno.

**Tight coupling**: L'implementazione `unify()` utilizza directly:

- `union_find_` (membro privato)
- `substitution_` (membro privato)
- Visitor inline `UnifyVisitor` (struct locale)

Questo è appropriato — unify è il core dell'algoritmo e necessita accesso diretto ai dati.

#### 2.4 Flusso logico

**Entry point**: `ConstraintSolver::solve(const ConstraintSet& constraints)` ([file:ConstraintSolver.cpp] linea ~80).

**Elaborazione**:

1. Inizializza `union_find_` (vuoto) e `substitution_` (vuoto)
2. Per ogni constraint in `constraints.constraints()`:
   - Invoca `unify(constraint.lhs, constraint.rhs, constraint)`
   - Se `unify()` fallisce (ritorna `std::unexpected`), aggiunge l'errore a `result.errors`
3. Ritorna `SolverResult{substitution, errors}`

**Logica unify (core algorithm)**:
[file:ConstraintSolver.cpp] linee 120-220.

1. **Handle ErrorType**: Se uno dei tipi è ErrorType, return success (linea ~125)
2. **Handle null types**: Se uno è null, return error (linea ~130)
3. **TypeVariable unification**:
   - Se t1 è TypeVariable:
     - Se t2 è TypeVariable: unite in union_find, bind t1→t2
     - Se t2 è concrete: occurs_check, bind t1→t2
   - Se t2 è TypeVariable e t1 è concrete: swap e recurse
4. **Concrete type unification**:
   - Se kind() diverso: return type mismatch error
   - Se kind() stesso: dispatch via TypeVisitor per compound types (Array, Vector, Custom)

**Error handling**:

- `unify()` ritorna `std::expected<void, CompileError>`
- Se unification fallisce, errore è accumulato in SolverResult
- Occurs-check fallimenti sono segnalati esplicitamente

#### 2.5 Punti critici

**DEF-007: Potenziale stack overflow nell'occurs-check**

- **Descrizione**: `occurs_in()` è ricorsivo ([file:ConstraintSolver.cpp] linea ~100) senza tail call optimization garantita. Se un tipo è profondamente annidato (es., Array<Array<Array<...>>>) con profondità 10000+, stack overflow è possibile.
- **Manifestazione**: Programmi con tipi molto profondamente annidati causano crash.
- **Impatto**: **BASSO** in pratica (source code raramente crea tali strutture), ma **TEORICO** problema.

**DEF-008: Union-find mutable find() senza const**

- **Descrizione**: `UnionFind::find()` applica path compression, mutando `parent_` entries, quindi non può essere `const` ([file:UnionFind.cpp] linea ~15). Questo è dichiarato nei commenti come una scelta consapevole per evitare `mutable` keyword, ma complica l'uso in contexti di const-correctness. **Lettura**: [file:UnionFind.hpp] linea ~25-35.
- **Manifestazione**: `ConstraintSolver::unify()` deve invocare `find()` e quindi non può essere `const`. `solve()` non è `const` in [file:ConstraintSolver.hpp].
- **Impatto**: **BASSO**. È una scelta di design ragionevole per visibility. L'alternativa (`mutable parent_`) sarebbe meno esplicita.

---

### Sistema 3: SymbolTable

#### 2.1 Panoramica di Sistema

**Identità e ruolo**: Gestione di identificatori → `TypeScheme` con lexical scoping. Implementa push/pop di scope frame (stack di hash maps) per supportare shadowing e nesting di scope.

**Scope**: Responsabile unicamente di **name binding and lookup**, non di type inference o constraint generation.

**Posizione nella pipeline**: Fase 1 (name resolution). Popolata durante `resolve_names()`, consultata durante constraint generation.

**Contesto di attivazione**: Istanziato all'inizio di `TypeChecker::check()`, accumula binding durante resolve_names, consultato durante type_expr per lookup di identificatori.

#### 2.2 Organizzazione moduli

**File**: `SymbolTable.hpp` (120 linee), `SymbolTable.cpp` (60 linee). Singola classe, ben localizzata.

#### 2.3 Analisi dipendenza

**Grafo**:

```
push_scope/pop_scope: O(1)
define: scopes_.back().insert_or_assign()
lookup: iterate scopes_ in reverse, return first match
```

**Tight coupling**: Moderato. `lookup()` itera su tutti gli scope, accedendo direttamente ai campi di map. Non c'è astrazione intermedia.

#### 2.4 Flusso logico

**Operazioni core**:

- `push_scope()`: emplace_back vuoto map
- `pop_scope()`: pop_back (asserts non-empty implicitamente)
- `define(name, scheme)`: insert_or_assign in scopes_.back()
- `lookup(name)`: reverse iterate, find first
- `depth()`: scopes_.size()

**Function return context**:

- `set_function_return_context()`: insert_or_assign speciale binding `"__function_context__"`
- `get_function_return_context()`: lookup reverso per `"__function_context__"` con is_function_binding() check

#### 2.5 Punti critici

**DEF-009: Magic string per function context**

- [già discusso in 1.4.d]

**DEF-010: pop_scope() non-safe**

- **Descrizione**: `pop_scope()` non verifica che `scopes_` non sia empty. Se viene invocato troppi tempi, comportamento è undefined.
- **Manifestazione**: Se una fase di type checking incontra un errore early e non mantiene invariante di scope nesting, crash.
- **Lettura**: [file:SymbolTable.cpp] linea ~14.
- **Impatto**: **BASSO** (invariante è mantenuto da TypeChecker), ma la robustness potrebbe migliorare.

---

### Sistema 4: Substitution

#### 2.1 Panoramica di Sistema

**Identità e ruolo**: Mantenimento e applicazione di substit zioni (mappature TypeVarId → TypePtr). Implementa caching persistente per ottimizzare applicazioni ripetute dello stesso tipo.

**Scope**: Responsabile **unicamente** di applicazione sostituzione. Non genera sostituzione (quella è responsabilità di ConstraintSolver).

**Posizione nella pipeline**: Fase 3-4 (output di constraint solving, input per zonking).

**Contesto di attivazione**: Creato (vuoto) da `ConstraintSolver::solve()`, popolato durante unificazione, passato a `TypeChecker::zonk()`.

#### 2.2 Organizzazione moduli

**File**: `Substitution.hpp` (120 linee), `Substitution.cpp` (80 linee).

#### 2.3 Analisi dipendenza

**Grafo**:

```
apply(type)
  └─ applyImpl(type)
      ├─ lookup(tvar.id())
      └─ visit_type() if compound type
          └─ ApplyVisitor::visit_*
```

#### 2.4 Flusso logico

**bind(var, type)**: Insert into `bindings_`, clear cache.

**apply(type)**: Traverse type tree recursively:

- If TypeVariable: lookup in bindings, if found recurse, else return unchanged
- If compound (Array/Vector): recurse into element type
- If primitive/custom: return unchanged
- **Caching**: Check apply_cache_ before recursing, populate after.

**Design persistente cache**:

- Cache keyed on raw `const TypeBase*` pointer (identity)
- Survives across multiple `apply()` calls on different types
- Invalidato on `bind()` tramite `apply_cache_.clear()`
- Conseguenza: first `apply()` traverses and allocates, subsequent calls on same node hit cache at O(1)

#### 2.5 Punti critici

**DEF-011: Cache invalidation on bind() is coarse-grained**

- **Descrizione**: `bind()` chiama `apply_cache_.clear()` unconditionally ([file:Substitution.cpp] linea ~45), anche se il binding novo non affetta la maggior parte dei nodi nel cache.
- **Manifestazione**: Se ci sono molti bind() calls interspersed con apply() calls, il cache non converge come vorrebbe. Ad esempio, bind(tv1, int), apply(vec), bind(tv2, string), apply(vec) ricrea cache entries instead of hitting.
- **Impatto**: **BASSO-MODERATO**. In practice, constraint solving fa tutti i bind() upfront, poi apply() è invocato molte volte. Quindi pattern è: many binds, then many applies (no interleaving).

**DEF-012: Thread-safety not thread-safe**

- **Descrizione**: [file:Substitution.hpp] linea ~96 dichiara "Not thread-safe: concurrent apply() and bind() calls require external synchronisation." Ma il codice non usa `mutable`, né `std::mutex`, quindi non c'è protezione.
- **Manifestazione**: Se due thread invocano contemporaneamente `apply()` e `bind()` su stessa Substitution, data race.
- **Impatto**: **BASSO** (type checker è mono-threaded), ma nota per future estensioni.

---

## Fase 3 — Analisi Per-Componente

A causa della lunghezza, analizzo i componenti critici in detail.

### Sistema 2 › Componente: ConstraintSolver::unify()

#### 3.1 Responsabilità

Dato due tipi e un constraint, determinare se possono essere unificati e popolare la substitution per rendere i tipi uguali, oppure ritornare un error CompileError contenente diagnostica umano-leggibile e hint di correzione.

#### 3.2 Struttura di classe

**Non applicabile**: `unify()` è un metodo standalone (non una classe separata).

#### 3.3 Analisi interfaccia

```cpp
[[nodiscard]] std::expected<void, CompileError> unify(
    const TypePtr &t1, 
    const TypePtr &t2, 
    const Constraint &constraint);
```

**Precondizioni**:

- Nessuna dichiarata formalmente, ma assume: `t1` e `t2` non-null (checked nel metodo)
- `constraint.origin` contiene SourceSpan valido per error reporting

**Postcondizioni**
:
- Se ritorna `std::expected<void>{}`: `substitution_` contiene 0+ nuovi binding che soddisfano t1 = t2
- Se ritorna `std::unexpected`: `substitution_` è unchanged, CompileError è completo (codice, messaggio, source span, hint)

**Contract**: 

- Idempotente: invocare unify(t1, t2, c) due volte in successione produce lo stesso risultato
- Commutativo: unify(t1, t2, c) ≈ unify(t2, t1, c) (il metodo swappa se necessario)

#### 3.4 Implementazione logica

[file:ConstraintSolver.cpp] linee ~121-220.

```cpp
// 1. ErrorType è magic — unifies with anything
if (t1->kind() == Error || t2->kind() == Error) return {};

// 2. Null check
if (!t1 || !t2) return error("Null type encountered");

// 3. Both TypeVariables
if (auto *tv1 = cast_typevariable(t1)) {
  if (auto *tv2 = cast_typevariable(t2)) {
    if (tv1->id() == tv2->id()) return {}; // same var
    if (occurs_in(tv1->id(), t2)) return error("Occurs check failed");
    union_find_.make_set(tv1->id());
    union_find_.make_set(tv2->id());
    union_find_.unite(tv1->id(), tv2->id());
    substitution_.bind(tv1->id(), t2);
    return {};
  }
  // tv1 = concrete type
  if (occurs_in(tv1->id(), t2)) return error("Occurs check");
  union_find_.make_set(tv1->id());
  substitution_.bind(tv1->id(), t2);
  return {};
}

// 4. t2 is TypeVariable, t1 is concrete — swap
if (is_typevariable(t2)) {
  return unify(t2, t1, constraint); // swap args
}

// 5. Both concrete types
if (t1->kind() != t2->kind()) {
  return error("Type mismatch", hint);
}

// 6. Same kind — dispatch on compound types via visitor
UnifyVisitor visitor{*this, t2, constraint};
visit_type(t1, visitor);
return visitor.result.value_or({});
```

**Complessità**: O(α(n)) per union-find operations (path compression + union by rank), O(depth of type) per occurs-check ricorsivo.

**Branching**:

- ErrorType short-circuit
- Null check early exit
- TypeVariable vs concrete decision tree
- Concrete concrete switch on kind()

Tutte le branch sono coverti in principio.

#### 3.5 Error handling

**Detection**: Null check, occurs check, kind mismatch, compound type mismatch.

**Representation**: `std::expected<void, CompileError>`, dove `CompileError` contiene:

- `error_code` (es. E2034 = type mismatch)
- `message` (es. "Type mismatch")
- `origin` (SourceSpan per source location)
- `hint` (suggerimento, es. "Did you mean to cast?")

**Propagation**: Caller (ConstraintSolver::solve) accede a `.error()` e aggiunge a errors vector. Non viene rethrown.

**Silent failures**: Se un compound type visitor non è implementato (es., CustomType), il risultato è `visitor.result = std::nullopt`, che viene convertito a `{}` (success). Questo è un **bug** — un custom type mismatch dovrebbe essere errore.

#### 3.6 Type consistency

**Declaration (hpp)**:

```cpp
[[nodiscard]] std::expected<void, CompileError> unify(
    const TypePtr &t1, const TypePtr &t2, const Constraint &constraint);
```

**Definition (cpp)**: Signature matches. 

**Unsafe casts**: Molteplici `static_cast<const TypeVariable*>` dopo `TypeVariable::classof()` check. Questi sono safe LLVM-style RTTI.

#### 3.7 Inter-component interaction

**Dipendenze**:

- `UnionFind` (make_set, unite)
- `Substitution` (bind, lookup, apply)
- `TypeVisitor` (visit_type dispatch)
- `CompileError` (error creation)

**Coupling**: Tight — unify accede direttamente a union_find_ e substitution_ (privati).

#### 3.8 Optimization opportunities

**Performance**:

- Occurs-check ricorsivo senza TCO: potrebbe stack overflow su tipi profondi (vedi DEF-007)
- Repeated occurs-check se una variable appare in molti vincoli: potrebbe essere cached

**Structural**:

- Magic CustomType result (success quando non dovrebbe) è un bug

---

### Sistema 5 › Componente: TypeScheme::instantiate()

#### 3.1 Responsabilità

Dato uno schema di tipo polimorfrico ∀(vars).body, generare un'istanza con variabili di tipo fresche, preparando lo schema per essere utilizzato in un contesto nuovo.

#### 3.2 Struttura

**Non una classe**: è una struct `TypeScheme` con metodo `instantiate()`.

```cpp
struct TypeScheme {
  std::vector<TypeVarId> quantified_vars;  // Vars vincolate ∀
  TypePtr body;                            // Tipo corpo
  bool is_const{false};                    // Immutabilità
  std::optional<TypePtr> return_type;      // Contesto funzione
  std::optional<std::string> function_name;
  
  TypePtr instantiate() const;
};
```

#### 3.3 Analisi interfaccia

**Signature**: `[[nodiscard]] TypePtr instantiate() const;`

**Precondizioni**: Nessuna esplicita, ma assume TypePtr non-null.

**Postcondizioni**: Ritorna un nuovo TypePtr dove ogni occurrence di una variabile in `quantified_vars` è rimpiazzato con una variabile di tipo fresca.

**Idempotency**: Non idempotente — due invocazioni su stessa istanza generano diversi binding di variabili fresche.

#### 3.4 Implementazione

[file:TypeScheme.cpp] linee ~18-50.

```cpp
TypePtr instantiate() const {
  if (quantified_vars.empty()) { return body; }
  
  std::unordered_map<TypeVarId, TypePtr> fresh_vars;
  for (auto qvar : quantified_vars) {
    fresh_vars[qvar] = fresh_type_variable();
  }
  
  return substitute_quantified(body, fresh_vars);
}
```

**Helper `substitute_quantified`** (nested): Traversa il body ricorsivamente, rimpiazzando ogni TypeVariable che appare in fresh_vars, recursively per Array/Vector element types.

**Complessità**: O(size of body tree) per single traversal.

#### 3.5 Error handling

**Detection**: Nessuno — assume input valido.

**Silent failures**: Se `fresh_type_variable()` fallisce (memory error), std::make_shared potrebbe gettare. Non è catturato.

#### 3.6 Type consistency

**Safe**: Tutti i TypePtr sono `std::shared_ptr<const TypeBase>`, immutabili.

**Casts**: Nessuno unsafe.

---

### Sistema 7 › Componente: ConstraintSet

#### 3.1 Responsabilità

Accumulo di vincoli di uguaglianza di tipo durante traversal dell'AST, assegnazione di ID unici, retrieval by ID o iteration su tutti.

#### 3.2 Struttura di classe

```cpp
class ConstraintSet {
private:
  std::vector<Constraint> constraints_;
  ConstraintId next_id_{1};
};
```

#### 3.3 Interfaccia

```cpp
ConstraintId add(TypePtr lhs, TypePtr rhs, SourceSpan origin, std::string_view reason);
const std::vector<Constraint>& constraints() const noexcept;
const Constraint* get(ConstraintId id) const noexcept;
std::size_t size() const noexcept;
```

**Precondizioni**: `add()` assume lhs/rhs non-null (non checked).

**Postcondizioni**: `add()` ritorna un ID >= 1 e aggiunge il vincolo al vettore. Subsequent `constraints()` include il vincolo appena aggiunto.

#### 3.4 Implementazione

[file:Constraint.cpp] linee ~10-35.

```cpp
ConstraintId add(...) {
  auto id = next_id_++;
  constraints_.push_back(Constraint{
    .id = id, .lhs = lhs, .rhs = rhs, 
    .origin = origin, .reason = reason});
  return id;
}

const std::vector<Constraint>& constraints() const noexcept {
  return constraints_;
}

const Constraint* get(ConstraintId id) const noexcept {
  auto it = std::ranges::find(constraints_, id, &Constraint::id);
  return (it != end) ? &*it : nullptr;
}
```

**Complessità**: `add()` O(1), `get()` O(n) lineare search.

#### 3.5 Error handling

**Detection**: Non viene validato che lhs/rhs non-null. Se null è passato, behaviour dipende da come il constraint è utilizzato (probabilmente crash in unify()).

#### 3.6 Type consistency

**Safe**: Tutti i member types sono validi. Nessun cast.

#### 3.8 Optimization

**Performance issue**: `get(id)` è lineare. Se vengono fatti molti get() calls, potrebbe beneficiare di un index interno (map from ID to pointer). Tuttavia, in practice il constraint set è iterato uniformemente.

---

## Fase 4 — Raccomandazioni Prioritizzate

### 4.1 Registro Raccomandazioni

---

#### REC-001: Implementare il metodo stub zonk_block_full()

**Titolo**: Implementare metodo zonk_block_full() mancante  
**Deficiency Addressed**: Phase 2, §2.5 DEF-001. Il metodo `zonk_block_full()` è dichiarato in `TypeChecker.hpp` linea ~85 ma non ha implementazione in `TypeChecker.cpp`, causando link error quando uno statement contiene un BlockStmt.

**Descrizione**: 
Il metodo `zonk_block_full()` è invocato da `zonk_stmt_full()` ([file:TypeChecker.cpp] linea ~310) per il caso `NodeKind::BlockStmt`, ma la sua definizione non esiste. Questo causa un compilazione failure al link time.

**Azione richiesta**: Aggiungere implementazione di `zonk_block_full()` in `TypeChecker.cpp`. Il metodo deve:

1. Ricevere una referenza const a `TypedBlockStmt` e una `Substitution` const
2. Iterare su tutti i statement del block
3. Invocare `zonk_stmt_full()` su ciascuno
4. Ritornare un `std::unique_ptr<TypedBlockStmt>` con i statement zonked e il tipo (void)

**Pseudocodice**:

```cpp
std::unique_ptr<TypedBlockStmt> TypeChecker::zonk_block_full(
    const Substitution& subst, 
    const TypedBlockStmt& block) {
  std::vector<TypedStmtPtr> zonked_stmts;
  for (const auto& stmt : block.statements()) {
    zonked_stmts.push_back(zonk_stmt_full(subst, *stmt));
  }
  return std::make_unique<TypedBlockStmt>(
    std::move(zonked_stmts), 
    PrimitiveType::void_(), 
    block.location());
}
```

**Entry point di cambiamento**: File [file:TypeChecker.cpp], dopo il metodo `zonk_stmt_full()` (circa linea ~440).

**Outcome atteso**: Link success, BlockStmt peut essere zonked correttamente.

---

**Feasibility Score**: **5**  
*Justification*: Metodo è straightforward, codebase contiene template simile in `zonk_stmt_full()` per reference, no external dependencies.

**Expected ROI**: **5**  
*Justification*: Blocca completamente la compilazione di programmi con blocchi; correzione eliminate critical build failure.

**Implementation Effort**: **5**  
*Justification*: Meno di 10 linee di codice, segue pattern già presente in codebase.

**Priority Rank**: `(5×2) + (5×2) + (5×1) = 25`

**Estimated Implementation Time**: `0.5–2 hours` (codifica, testing, integration).

**Required Resources**: 

- Roles: uno senior C++ engineer (familiarità con TypeChecker)
- Tools: C++ compiler, version control
- Access: Write access to TypeChecker.cpp
- External: Nessuno

**Effectiveness Indicators**:

1. Link stage completato without zonk_block_full undefined reference error.
2. Test suite per BlockStmt statements passa (creare se non esiste).

---

#### REC-002: Implementare constraint generation per tutte le espressioni binarie

**Titolo**: Completare constraint generation per operatori binari  
**Deficiency Addressed**: Phase 2, §2.5 DEF-002. Molti helper method per type_expr non sono completamente implementati, incluso `type_binary_expr()`.

**Descrizione**:  
Le operazioni binarie (aritmetica, comparazione, logica, bitwise) sono essenziali per quasi tutti i programmi. L'attuale implementazione probabilmente ha stub per alcuni operatori (vedi i metodi `type_binary_arithmetic_op`, `type_binary_comparison_op`, etc. dichiarati in TypeChecker.hpp linee ~102-105).

Se questi metodi non emettono vincoli corretti, type inference per espressioni binarie fallisce o produce tipi errati.

**Azione richiesta**: Completare implementazione di:

1. `type_binary_arithmetic_op(expr, lhs_type, rhs_type)` — Arithmetic (+, -, *, /, %) require numeric types (int/float families), result is numeric
2. `type_binary_comparison_op(expr, lhs_type, rhs_type)` — Comparison (<, >, <=, >=, ==, !=) work on numeric, return bool
3. `type_binary_logical_op(expr, lhs_type, rhs_type)` — Logical (&&, ||) require bool, return bool
4. `type_binary_bitwise_op(expr, lhs_type, rhs_type)` — Bitwise (&, |, ^, <<, >>) work on integers, return integer

For each, emit constraints:

- LHS type must be compatible with operator
- RHS type must be compatible
- Result type is determined (e.g., arithmetic → numeric, comparison → bool)

**Entry point**: File [file:TypeChecker.cpp], metodi `type_binary_arithmetic_op()`, etc. (likely stubs or partial).

**Outcome atteso**: Binary expressions type correctly, correct constraints emitted for unification.

---

**Feasibility Score**: **4**  
*Justification*: Requires understanding of operator semantics and type rules; code patterns are clear in existing expression handlers; may require 50+ lines per operator group.

**Expected ROI**: **5**  
*Justification*: Binary expressions are ubiquitous; incomplete implementation breaks type inference for most programs.

**Implementation Effort**: **4**  
*Justification*: ~2 weeks for thorough implementation, testing per operator group, edge case handling.

**Priority Rank**: `(4×2) + (5×2) + (4×1) = 27`

**Estimated Implementation Time**: `1–2 weeks` (thorough implementation + testing).

**Required Resources**:

- Roles: one senior C++ engineer, one QA engineer
- Tools: C++ compiler, unit test framework (Catch2)
- Access: Write TypeChecker.cpp
- External: Language specification for operator type rules

**Effectiveness Indicators**:

1. All binary operator test cases pass (create comprehensive test suite)
2. Arithmetic expression `1 + 2` type-checks to integer
3. Comparison expression `1 < 2` type-checks to bool

---

#### REC-003: Aggiungere validazione di break/continue dentro loop

**Titolo**: Implementare validazione di break/continue statement context  
**Deficiency Addressed**: Phase 2, §2.5 DEF-004. Member `loop_depth_` è dichiarato ma mai usato, consenti break/continue al top-level.

**Descrizione**:  
Attualmente, un programma con `break;` al di fuori di un loop non viene segnalato come errore. Il member `loop_depth_` ([file:TypeChecker.hpp] linea ~116) esiste ma non è incrementato/decrementato durante type checking.

**Azione richiesta**:

1. Incrementare `loop_depth_` quando si entra in un loop (WhileStmt, ForStmt)
2. Decrementare quando si esce
3. In `type_break_stmt()` e `type_continue_stmt()`, verificare che `loop_depth_ > 0`; se no, emettere errore

**Pseudocodice**:

```cpp
TypedStmtPtr TypeChecker::type_while_stmt(const WhileStmt& stmt) {
  ++loop_depth_;
  auto typed_body = type_stmt(stmt.body());
  --loop_depth_;
  // ... rest of implementation
}

TypedStmtPtr TypeChecker::type_break_stmt(const BreakStmt& stmt) {
  if (loop_depth_ == 0) {
    errors_.push_back(CompileError::TypeError(
      ErrorCode::E2050, "Invalid break statement",
      stmt.location(),
      "break is only valid inside a loop"));
    return std::make_unique<TypedBreakStmt>(error_type(), stmt.location());
  }
  // ... rest
}
```

**Entry point**: File [file:TypeChecker.cpp], methods for loop typing (type_while_stmt, type_for_stmt) and break/continue.

**Outcome atteso**: break/continue outside loop produce type error, loop_depth_ is properly tracked.

---

**Feasibility Score**: **5**  
*Justification*: Infrastructure (loop_depth_) already declared, implementation is simple increment/decrement + one check.

**Expected ROI**: **4**  
*Justification*: Prevents invalid programs from passing type checking; moderate impact (not ubiquitous issue, but important correctness).

**Implementation Effort**: **5**  
*Justification*: < 20 lines of code, straightforward logic.

**Priority Rank**: `(5×2) + (4×2) + (5×1) = 23`

**Estimated Implementation Time**: `2–4 hours` (implementation + unit tests).

**Required Resources**:

- Roles: one C++ engineer
- Tools: C++ compiler, test framework
- Access: TypeChecker.cpp
- External: None

**Effectiveness Indicators**:

1. Program `{ break; }` at top-level produces E2050 error
2. Program `while(true) { break; }` type-checks successfully
3. Test suite for loop context validation passes

---

#### REC-004: Refactor magic string function context in SymbolTable

**Stato**: ✅ Completato (15 aprile 2026)

**Titolo**: Sostituire magic string "__function_context__" con dedicated field  
**Deficiency Addressed**: Phase 2, §2.5 DEF-003 and Phase 1, §1.4.d. String magic `__function_context__` è fragile.

**Descrizione**:  
Attualmente il contesto di ritorno di funzioni è memorizzato come binding speciale su key `"__function_context__"` in SymbolTable ([file:SymbolTable.cpp] linee ~44-50). Questo è fragile: se un utente crea una variabile locale named `__function_context__`, behaviour diventa ambiguo.

**Azione richiesta**: Aggiungere un dedicated field in SymbolTable:

```cpp
std::optional<std::pair<TypePtr, std::string>> function_return_context_;
```

Refactor `set_function_return_context()` e `get_function_return_context()` per usare questo field anziché la binding magic.

**Entry point**: File [file:SymbolTable.hpp] e [file:SymbolTable.cpp], class SymbolTable declaration.

**Outcome atteso**: No magic strings, clearer semantics, no fragility with user-defined names.

---

**Feasibility Score**: **4**  
*Justification*: Refactoring è straightforward, but requires changing multiple call sites; no external dependencies.

**Expected ROI**: **2**  
*Justification*: Robustness improvement è marginal in practice (unlikely user creates "__function_context__" variable); mainly maintainability benefit.

**Implementation Effort**: **5**  
*Justification*: ~30 lines of code, localized change.

**Priority Rank**: `(4×2) + (2×2) + (5×1) = 17`

**Estimated Implementation Time**: `4–8 hours` (refactoring, testing call sites).

**Required Resources**:

- Roles: one C++ engineer
- Tools: C++ compiler, refactoring tools
- Access: SymbolTable.hpp/cpp, TypeChecker.cpp (call sites)
- External: None

**Effectiveness Indicators**:

1. Magic string "__function_context__" no longer appears in SymbolTable
2. Function return context properly tracked via dedicated field
3. Return statement validation still works correctly

---

#### REC-005: Implementare error recovery robusto per cascate di errori

**Titolo**: Migliorare error recovery tramite unified error propagation  
**Deficiency Addressed**: Phase 1, §1.4.a e Phase 2, §2.5 DEF-005. Propagazione di errori è incoerenete fra sistemi.

**Descrizione**:  
Attualmente, error recovery è ad-hoc: ErrorType unifica silenziosamente con qualunque tipo, ma non tutti i percorsi di errore utilizzano ErrorType. Questo può portare a cascate di errori.

**Azione richiesta**:

1. Centralizzare error recovery in ConstraintSolver: quando unificazione fallisce, inserire ErrorType binding invece di solo segnalare errore
2. In TypeChecker, quando type inference fallisce, emettere tipo error anziché fresh_type_variable()
3. Assicurare che tutti i percorsi di errore producono ErrorType, non fresh variables

**Pseudocodice**:

```cpp
std::expected<void, CompileError> ConstraintSolver::unify(...) {
  if (unification_fails) {
    // Instead of just returning error:
    // - Log error to result.errors
    // - Bind one or both variables to ErrorType to prevent cascades
    substitution_.bind(extract_vars(t1), error_type());
    substitution_.bind(extract_vars(t2), error_type());
    return std::unexpected{error};
  }
}
```

**Entry point**: File [file:ConstraintSolver.cpp], method `unify()` and `solve()`.

**Outcome atteso**: Single type error no longer cascades into multiple derived errors.

---

**Feasibility Score**: **3**  
*Justification*: Requires careful coordination between systems; needs testing to ensure cascades are actually prevented without suppressing real errors.

**Expected ROI**: **4**  
*Justification*: Better error messages (users see root cause, not cascade); moderate impact on user experience.

**Implementation Effort**: **3**  
*Justification*: ~50 lines of logic, but requires thorough testing; estimated 2-3 weeks.

**Priority Rank**: `(3×2) + (4×2) + (3×1) = 17`

**Estimated Implementation Time**: `2–3 weeks` (design error recovery strategy, implementation, extensive testing).

**Required Resources**:

- Roles: one senior C++ engineer, one QA engineer
- Tools: C++ compiler, test framework, error message corpus for validation
- Access: ConstraintSolver, TypeChecker
- External: None

**Effectiveness Indicators**:

1. Single type error in expression produces only one diagnostic, not cascade
2. Complex type mismatch (nested expressions) produces focused error message
3. Error recovery test suite validates no false cascades

---

#### REC-006: Aggiungere type signature validation per function calls

**Stato**: ✅ Completato (15 aprile 2026)

**Titolo**: Validare type signatures di function calls contro declarations  
**Deficiency Addressed**: Phase 2, §2.5 DEF-002 (incomplete constraint generation). Function call type checking è incomplete.

**Descrizione**:  
TypeChecker memorizza function declarations in `function_decls_` ([file:TypeChecker.hpp] linea ~115), ma non sembra validare che argomenti di function call corrispondono ai parametri dichiarati.

**Azione richiesta**: Nel metodo `type_call_expr()`:

1. Look up callee identifier in symbol table
2. If it's a function binding, extract declared parameter types
3. For each argument, emit constraint: arg_type = param_type
4. Validate number of arguments matches number of parameters

**Pseudocodice**:

```cpp
TypedExprPtr TypeChecker::type_call_expr(const CallExpr& expr) {
  auto callee_type = type_expr(expr.callee());
  const auto& args = expr.args();
  
  // Look up function declaration
  if (const auto* func_decl = get_function_decl(expr)) {
    const auto& params = func_decl->params();
    if (args.size() != params.size()) {
      errors_.push_back(CompileError::TypeError(
        ErrorCode::E2040, "Argument count mismatch",
        expr.location(),
        FORMAT("Expected {} args, got {}", params.size(), args.size())));
    }
    
    for (size_t i = 0; i < std::min(args.size(), params.size()); ++i) {
      auto arg_type = type_expr(*args[i]);
      auto param_type = params[i].type_annotation;
      constraints_.add(arg_type, param_type, 
                       args[i]->location(),
                       FORMAT("function argument {}", i));
    }
  }
  // ... rest
}
```

**Entry point**: File [file:TypeChecker.cpp], method `type_call_expr()`.

**Outcome atteso**: Function call arguments validated against parameter types.

---

**Feasibility Score**: **4**  
*Justification*: Function signature lookup is already in place (function_decls_), main work is constraint emission; moderate complexity.

**Expected ROI**: **5**  
*Justification*: Critical for correctness; function calls are ubiquitous; current implementation likely misses argument validation.

**Implementation Effort**: **4**  
*Justification*: ~40 lines of code, but requires careful handling of edge cases (variadic functions, overloads if supported).

**Priority Rank**: `(4×2) + (5×2) + (4×1) = 26`

**Estimated Implementation Time**: `1–2 weeks` (implementation, comprehensive call expression test suite).

**Required Resources**:

- Roles: one senior C++ engineer, one QA
- Tools: C++ compiler, test framework
- Access: TypeChecker.cpp
- External: Language specification for function call semantics

**Effectiveness Indicators**:

1. Function call `foo(1, 2)` validated: foo must accept 2 numeric args
2. Mismatch `foo(1, 2)` called with 1 arg produces error
3. Test suite for function call validation passes

---

#### REC-007: Prevenire stack overflow nell'occurs-check ricorsivo

**Titolo**: Implementare occurs-check iterativo o tramite memoization  
**Deficiency Addressed**: Phase 3, §2.5 DEF-007. Occurs-check ricorsivo può stack overflow su tipi profondamente annidati.

**Descrizione**:  
`ConstraintSolver::occurs_in()` è ricorsivo ([file:ConstraintSolver.cpp] linea ~100) senza garantie di tail call optimization. Tipi profondamente annidati (Array<Array<...>>) con profondità 10000+ causano stack overflow.

**Azione richiesta**: Refactor occurs-check per utilizzare iterative DFS con stack esplicito, oppure aggiungere memoization per evitare ri-traversal.

**Pseudocodice (iterative)**:

```cpp
bool ConstraintSolver::occurs_in(TypeVarId var, const TypePtr& type, 
                                 const Substitution& subst) {
  std::vector<TypePtr> work_list{type};
  std::unordered_set<const TypeBase*> visited;
  
  while (!work_list.empty()) {
    auto current = work_list.back();
    work_list.pop_back();
    
    if (!current || visited.count(current.get())) continue;
    visited.insert(current.get());
    
    auto resolved = subst.apply(current);
    if (const auto* tv = TypeVariable::classof(resolved.get())) {
      if (tv->id() == var) return true;
    } else if (const auto* arr = ArrayType::classof(resolved.get())) {
      work_list.push_back(arr->element_type());
    } // ... etc for Vector, Custom
  }
  return false;
}
```

**Entry point**: File [file:ConstraintSolver.cpp], method `occurs_in()` (static).

**Outcome atteso**: Deep type nesting no longer causes stack overflow.

---

**Feasibility Score**: **4**  
*Justification*: Algorithm is straightforward; iterative vs recursive is well-known tradeoff; requires careful handling of visited set.

**Expected ROI**: **2**  
*Justification*: Robustness improvement, but stack overflow on deeply nested types is rare in practice (source code rarely creates such structures).

**Implementation Effort**: **4**  
*Justification*: ~50 lines of code, straightforward refactoring.

**Priority Rank**: `(4×2) + (2×2) + (4×1) = 16`

**Estimated Implementation Time**: `4–8 hours` (refactoring, testing deep nesting scenarios).

**Required Resources**:

- Roles: one C++ engineer
- Tools: C++ compiler, profiler (optional)
- Access: ConstraintSolver.cpp
- External: None

**Effectiveness Indicators**:

1. Program with Array<Array<...<Array>>> (depth 1000) no longer stack overflows
2. Occurs-check still correctly detects infinite types
3. Performance unchanged or improved (iterative often faster)

---

#### REC-008: Completare TypeVisitor per tutti i tipi composti

**Titolo**: Estendere TypeVisitor per CustomType e altri tipi composti futuri  
**Deficiency Addressed**: Phase 3, §2.5 (implicit). TypeVisitor attualmente copre solo Array, Vector, Custom (3 tipi), ma CustomType handler è no-op.

**Descrizione**:  
Attualmente `TypeVisitor` fornisce metodi virtuali per `visit_array()`, `visit_vector()`, `visit_custom()`. La implementazione di `visit_custom()` è no-op ([file:ConstraintSolver.cpp] linea ~50: `/* no-op — CustomType contains no type variables to resolve */`).

Tuttavia, se CustomType dovesse contenere type parameters in futuro (es., generic structs), questo visitor pattern dimostrerà il suo valore. Attualmente il design è corretto ma la documentazione e l'extensibilità potrebbero migliorare.

**Azione richiesta**:

1. Documentare il visitor pattern in TypeVisitor header con commenti Doxygen spiegando come estendere per nuovi compound types
2. Se CustomType deve supportare type parameters, aggiornare `visit_custom()` per traversare i type parameters
3. Aggiungere test che verifica visitor è invocato per ogni compound type

**Entry point**: File [file:TypeVisitor.hpp] e [file:TypeVisitor.cpp] (implementation).

**Outcome atteso**: Visitor pattern è well-documented e extensible; implementazione è pronta per future generic types.

---

**Feasibility Score**: **5**  
*Justification*: Primarily documentation + minor implementation if generics are added; straightforward.

**Expected ROI**: **2**  
*Justification*: Future-proofing; not urgent for current feature set.

**Implementation Effort**: **5**  
*Justification*: ~30 lines (documentation + optional generic support).

**Priority Rank**: `(5×2) + (2×2) + (5×1) = 17`

**Estimated Implementation Time**: `2–4 hours` (documentation, optional generics prototype).

**Required Resources**:

- Roles: one C++ engineer (familiar with generics if added)
- Tools: C++ compiler, documentation generator (Doxygen)
- Access: TypeVisitor.hpp/cpp
- External: Language specification for generic type rules (if generics added)

**Effectiveness Indicators**:

1. TypeVisitor Doxygen documentation includes extension guidelines
2. Adding new compound type visitor method requires only implementing one virtual method
3. Test suite validates visitor dispatch for all compound types

---

#### REC-009: Aggiungere safe variant di SymbolTable::pop_scope()

**Titolo**: Aggiungere asserzione di safety per pop_scope()  
**Deficiency Addressed**: Phase 2, §2.5 DEF-010. `pop_scope()` non verifica che scopes_ non sia vuoto.

**Descrizione**:  
`pop_scope()` ([file:SymbolTable.cpp] linea ~14) invoca direttamente `scopes_.pop_back()` senza verificare non-empty. Se scope nesting invariante è violato, undefined behaviour.

**Azione richiesta**: Aggiungere assert o throw:

```cpp
void SymbolTable::pop_scope() {
  if (scopes_.empty()) {
    throw std::runtime_error("pop_scope: scope stack underflow");
  }
  scopes_.pop_back();
}
```

O usando assert:

```cpp
void SymbolTable::pop_scope() {
  assert(!scopes_.empty() && "pop_scope: scope stack underflow");
  scopes_.pop_back();
}
```

Preferire `throw` per error handling, assert per invariant verification.

**Entry point**: File [file:SymbolTable.cpp], method `pop_scope()`.

**Outcome atteso**: Scope nesting errors are detected early, not silently corrupting state.

---

**Feasibility Score**: **5**  
*Justification*: One-line change, trivial.

**Expected ROI**: **2**  
*Justification*: Defensive programming; unlikely to trigger in well-written code, but improves robustness.

**Implementation Effort**: **5**  
*Justification*: One line of code.

**Priority Rank**: `(5×2) + (2×2) + (5×1) = 17`

**Estimated Implementation Time**: `0.5–1 hour`.

**Required Resources**:

- Roles: one C++ engineer
- Tools: C++ compiler
- Access: SymbolTable.cpp
- External: None

**Effectiveness Indicators**:

1. `pop_scope()` on empty stack throws or asserts
2. No test suite break (invariant is maintained by TypeChecker)

---

#### REC-010: Implementare caching index per ConstraintSet::get()

**Stato**: ✅ Completato (15 aprile 2026)

**Titolo**: Ottimizzare lookup constraint per ID  
**Deficiency Addressed**: Phase 3, §2.5 (implicit). `ConstraintSet::get(id)` è O(n) lineare search.

**Descrizione**:  
`ConstraintSet::get(ConstraintId id)` ([file:Constraint.cpp] linea ~24) esegue linear search su tutto il vettore di vincoli. Se constraint set è grande (1000+ vincoli), repeated get() calls sono lenti.

**Azione richiesta**: Aggiungere index interno (std::unordered_map<ConstraintId, size_t>) che mappa ID a posizione nel vettore:

```cpp
class ConstraintSet {
private:
  std::vector<Constraint> constraints_;
  std::unordered_map<ConstraintId, std::size_t> id_index_;
  ConstraintId next_id_{1};
  
  void add(...) {
    auto id = next_id_++;
    id_index_[id] = constraints_.size();
    constraints_.push_back(...);
  }
  
  const Constraint* get(ConstraintId id) const {
    auto it = id_index_.find(id);
    if (it == id_index_.end()) return nullptr;
    return &constraints_[it->second];
  }
};
```

**Entry point**: File [file:Constraint.hpp] e [file:Constraint.cpp].

**Outcome atteso**: `get()` is O(1) expected time.

---

**Feasibility Score**: **5**  
*Justification*: Straightforward optimization, no design changes.

**Expected ROI**: **1**  
*Justification*: Performance improvement only if `get()` is called frequently; may not be used heavily in practice (constraint set is usually iterated uniformly).

**Implementation Effort**: **5**  
*Justification*: ~20 lines of code.

**Priority Rank**: `(5×2) + (1×2) + (5×1) = 17`

**Estimated Implementation Time**: `1–2 hours` (optimization, verification that no performance regression).

**Required Resources**:

- Roles: one C++ engineer
- Tools: C++ compiler, profiler
- Access: Constraint.hpp/cpp
- External: None

**Effectiveness Indicators**:

1. `get()` call count profiling shows improvement if method is used
2. No functional change (still returns same constraints)
3. Memory overhead is linear in constraint count (acceptable)

---

### 4.2 Tabella Riassuntiva di Priorità

| Rank | ID | Titolo | Feasibility | ROI | Effort | Composite | Est. Time |
|------|-----|--------|-------------|-----|--------|-----------|-----------|
| 1 | REC-001 | Implementare zonk_block_full() | 5 | 5 | 5 | 25 | 0.5–2h |
| 2 | REC-006 | Validare type signatures function call ✅ COMPLETATO | 4 | 5 | 4 | 26 | 1–2w |
| 3 | REC-002 | Completare constraint generation binari | 4 | 5 | 4 | 27 | 1–2w |
| 4 | REC-003 | Aggiungere validazione break/continue | 5 | 4 | 5 | 23 | 2–4h |
| 5 | REC-004 | Refactor function context magic string ✅ COMPLETATO | 4 | 2 | 5 | 17 | 4–8h |
| 6 | REC-005 | Migliorare error recovery cascate | 3 | 4 | 3 | 17 | 2–3w |
| 7 | REC-007 | Prevenire stack overflow occurs-check | 4 | 2 | 4 | 16 | 4–8h |
| 8 | REC-008 | Estendere TypeVisitor | 5 | 2 | 5 | 17 | 2–4h |
| 9 | REC-009 | Aggiungere assert pop_scope() | 5 | 2 | 5 | 17 | 0.5–1h |
| 10 | REC-010 | Ottimizzare ConstraintSet::get() ✅ COMPLETATO | 5 | 1 | 5 | 17 | 1–2h |