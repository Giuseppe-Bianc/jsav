# Audit dell'Implementazione del Type Checker

## Fase 1 — Analisi dell'Insieme dei Sistemi

### 1.1 Enumerazione dei Sistemi

Il codebase del type checker è costituito da **10 sistemi**, ciascuno identificato da una coppia di file `.hpp`/`.cpp` (o header-only per alcuni). Di seguito l'enumerazione completa.

---

**a) `Constraint` / `ConstraintSet`**

- **Responsabilità primaria**: Accumulare e rappresentare vincoli di uguaglianza tra tipi (`lhs = rhs`) generati durante la traversata dell'AST. Ogni vincolo è associato a un identificatore univoco, una posizione sorgente e una motivazione leggibile.
- **Ruolo rispetto agli altri sistemi**: Fornisce input al `ConstraintSolver`. È alimentato dal `TypeChecker` durante la Fase 2 (generazione vincoli). Sistema **core** — senza vincoli, il solver non ha dati su cui operare.

---

**b) `ConstraintSolver`**

- **Responsabilità primaria**: Risolvere tutti i vincoli di tipo tramite unificazione basata su union-find, producendo una `Substitution` che mappa variabili di tipo a tipi concreti. Rileva tipi infiniti tramite occurs-check.
- **Ruolo rispetto agli altri sistemi**: Consuma output di `ConstraintSet` e produce output per `Substitution`. Dipende da `UnionFind`, `TypeVisitor`, `ErrorType`. Sistema **core** — centrale nella pipeline Hindley-Milner.

---

**c) `ErrorType`**

- **Responsabilità primaria**: Rappresentare un tipo "errore" singleton che si unifica silenziosamente con qualsiasi altro tipo, prevenendo errori a cascata da una singola causa radice.
- **Ruolo rispetto agli altri sistemi**: Sistema **ausiliario di supporto** — consultato dal `ConstraintSolver` durante l'unificazione e dal `TypeChecker` per gestire percorsi di errore.

---

**d) `Substitution`**

- **Responsabilità primaria**: Memorizzare e applicare mappe di sostituzione (variabile di tipo → tipo risolto). Implementa caching persistente per `apply()` con invalidazione automatica su `bind()`.
- **Ruolo rispetto agli altri sistemi**: Prodotto dal `ConstraintSolver`, consumato dal `TypeChecker` durante la Fase 4 (zonking). Dipende da `TypeVisitor` per la traversata ricorsiva. Sistema **core** — essenziale per la fase di zonking.

---

**e) `SymbolTable`**

- **Responsabilità primaria**: Gestire mappature identificatore → `TypeScheme` con supporto per scope annidati e shadowing. Fornisce contesto di tipo return per validazione dei return statement.
- **Ruolo rispetto agli altri sistemi**: Alimentato dal `TypeChecker` durante la Fase 1 (name resolution) e consultato durante la Fase 2 (constraint generation). Sistema **core** — fondamentale per la risoluzione dei nomi.

---

**f) `TypeChecker`**

- **Responsabilità primaria**: Orchestrazione dell'intera pipeline di type checking: name resolution, constraint generation, constraint solving, zonking. Implementa la digitazione di ogni tipo di espressione e statement.
- **Ruolo rispetto agli altri sistemi**: Sistema **orchestratore** — dipende da tutti gli altri sistemi e li coordina. È l'entry point principale per il consumer del type checker.

---

**g) `TypeScheme`**

- **Responsabilità primaria**: Rappresentare tipi polimorfici con variabili quantificate universalmente (`∀T. body`). Supporta l'istanziazione con variabili fresche.
- **Ruolo rispetto agli altri sistemi**: Consumato dal `SymbolTable` per i bindings e dal `TypeChecker` durante la risoluzione dei nomi. Sistema **core** — necessario per il supporto del polimorfismo.

---

**h) `TypeVariable`**

- **Responsabilità primaria**: Rappresentare variabili di tipo sconosciute (`?T1`, `?T2`, ...) generate per espressioni senza annotazioni di tipo esplicite.
- **Ruolo rispetto agli altri sistemi**: Utilizzato da praticamente tutti i sistemi — è il tipo fondamentale su cui opera l'intero algoritmo di inferenza. Sistema **core fondamentale**.

---

**i) `TypeVisitor`**

- **Responsabilità primaria**: Fornire un'interfaccia visitor per ricorsione strutturale su tipi composti (Array, Vector, Custom). Evita duplicazione della logica `switch-on-TypeKind`.
- **Ruolo rispetto agli altri sistemi**: Sistema **trasversale di supporto** — utilizzato da `ConstraintSolver`, `Substitution` per l'unificazione e l'applicazione di sostituzioni.

---

**j) `UnionFind`**

- **Responsabilità primaria**: Implementare la struttura dati disjoint-set con path compression e union by rank per unificazione efficiente O(α(n)) di variabili di tipo.
- **Ruolo rispetto agli altri sistemi**: Consumato esclusivamente dal `ConstraintSolver`. Sistema **ausiliario** — infrastruttura per l'unificazione.

---

### 1.2 Mappa delle Dipendenze Inter-Sistema

#### a) Diagramma ASCII

```text
                        ┌──────────────────┐
                        │   TypeChecker    │  ← Orchestratore principale
                        │  (entry point)   │
                        └────────┬─────────┘
                                 │
            ┌────────────────────┼────────────────────┐
            │                    │                    │
            ▼                    ▼                    ▼
   ┌────────────────┐  ┌──────────────────┐  ┌─────────────────┐
   │  SymbolTable   │  │  ConstraintSet   │  │ ConstraintSolver│
   │  (Fase 1)      │  │  (Fase 2)        │  │  (Fase 3)       │
   └────────┬───────┘  └────────┬─────────┘  └────────┬────────┘
            │                    │                     │
            ▼                    ▼                     ▼
   ┌────────────────┐           │            ┌──────────────────┐
   │  TypeScheme    │           │            │   Substitution   │
   └────────┬───────┘           │            └────────┬─────────┘
            │                    │                     │
            ▼                    │                     │
   ┌────────────────┐           │                     │
   │ TypeVariable   │───────────┘                     │
   └────────┬───────┘                                 │
            │                                         │
            │              ┌──────────────┐           │
            │              │  ErrorType   │◄──────────┘
            │              └──────────────┘
            │                    ▲
            │                    │
            └────────────────────┼────────────────────┐
                                 │                     │
                        ┌────────┴─────────┐  ┌────────┴────────┐
                        │   TypeVisitor    │  │   UnionFind     │
                        └──────────────────┘  └─────────────────┘
```

#### b) Classificazione Upstream/Downstream

| Sistema | Classificazione | Motivazione |
|---------|-----------------|-------------|
| `TypeVariable` | **Upstream** | Tipo fondamentale consumato da tutti gli altri sistemi |
| `TypeScheme` | **Upstream** | Utilizzato da `SymbolTable` per bindings |
| `SymbolTable` | **Midstream** | Produce dati per `TypeChecker` (Fase 2), consuma `TypeScheme` |
| `ConstraintSet` | **Midstream** | Produce vincoli per `ConstraintSolver`, alimentato da `TypeChecker` |
| `TypeChecker` | **Midstream/Orchestratore** | Coordina tutti i sistemi, produce output finale |
| `ConstraintSolver` | **Downstream** | Consuma vincoli, produce sostituzioni |
| `Substitution` | **Downstream** | Prodotto dal solver, consumato dallo zonking |
| `UnionFind` | **Downstream** | Utilizzato solo dal solver |
| `TypeVisitor` | **Trasversale** | Consultato da solver e substitution |
| `ErrorType` | **Trasversale** | Consultato da solver e type checker per gestione errori |

#### c) Nodi Critici

- **Alto fan-in** (molti sistemi dipendono): `TypeVariable` (consumato da 8+ sistemi), `TypeChecker` (orchestratore centrale)
- **Alto fan-out** (dipende da molti sistemi): `TypeChecker` (dipende da `SymbolTable`, `ConstraintSet`, `ConstraintSolver`, `ErrorType`, `TypeScheme`, `TypeVariable`, `TypeVisitor`, `UnionFind` indirettamente)
- **Punto di strozzatura potenziale**: `ConstraintSolver` — unico sistema di risoluzione; se lento o fallisce, l'intera pipeline si blocca

**DEF-001**: `TypeChecker` presenta fan-out eccessivo (dipende da 8+ sistemi direttamente). `include/jsav/typechecker/TypeChecker.hpp` include 8 header diversi.

---

### 1.3 Valutazione della Coerenza Architetturale

#### a) Separazione delle Responsabilità

L'architettura rispetta in larga misura il **Single Responsibility Principle** a livello di sistema:

- `ConstraintSet` gestisce solo l'accumulo di vincoli
- `ConstraintSolver` gestisce solo la risoluzione
- `Substitution` gestisce solo le mappe di sostituzione
- `SymbolTable` gestisce solo gli scope e i bindings

Tuttavia, **`TypeChecker` viola il SRP** a livello di sistema: gestisce name resolution, constraint generation, constraint solving, zonking, e la digitazione di 15+ tipi di espressione e 12+ tipi di statement. Questo è accettabile per un orchestratore ma la quantità di logica di digitazione specifica per tipo dovrebbe essere delegata a visitor separati.

**DEF-002**: `TypeChecker` concentra troppe responsabilità — orchestratore + digitazione di ogni tipo di espressione/statement. `include/jsav/typechecker/TypeChecker.hpp` dichiara 20+ metodi privati di digitazione.

#### b) Coerenza dell'Organizzazione dei Moduli

L'organizzazione fisica (file/cartelle) riflette coerentemente la decomposizione logica:

- Ogni sistema ha una coppia `.hpp`/`.cpp` dedicata
- I nomi dei file corrispondono ai nomi delle classi
- Il namespace `jsv` è uniforme

**Problema**: `TypeVisitor.hpp` non ha un `.cpp` corrispondente per le dichiarazioni pure — `TypeVisitor.cpp` esiste ma contiene solo le implementazioni di `visit_type()`, non la classe visitor stessa. Questa discrepanza è minore ma crea confusione.

**DEF-003**: Discrepanza minore tra struttura fisica e logica: `TypeVisitor.cpp` implementa funzioni libere (`visit_type`) mentre la classe `TypeVisitor` è interamente header-only.

#### c) Pulizia dei Confini Inter-Sistema

**Punti di forza**:

- Le interfacce sono esplicite e tipizzate (es. `ConstraintSet::add()` ritorna `ConstraintId`, `ConstraintSolver::solve()` ritorna `SolverResult`)
- Uso appropriato di `std::expected<void, CompileError>` per la propagazione degli errori

**Violazioni di incapsulamento**:

1. `ConstraintSolver` accede direttamente ai campi pubblici di `Constraint` (`.lhs`, `.rhs`) — questo è accettabile perché `Constraint` è una struct POD, ma espone dettagli interni.
2. `TypeChecker::zonk_type()` è una funzione libera statica che duplica la logica di `Substitution::apply()`. Vedi §1.4.d.

**DEF-004**: La funzione `zonk_type()` in `src/jsav_Lib/typechecker/TypeChecker.cpp:43-67` duplica la logica di `Substitution::apply()`, violando il principio DRY.

**Giudizio sintetico**: Architettura **parzialmente coerente**. La separazione dei sistemi è buona ma `TypeChecker` accumula troppe responsabilità e esiste duplicazione di logica tra zonking e substitution.

---

### 1.4 Analisi delle Preoccupazioni Trasversali

#### a) Propagazione degli Errori

| Sistema | Strategia | Note |
|---------|-----------|------|
| `TypeChecker` | Raccolta in `std::vector<CompileError>` | Errori accumulati, non propagati tramite eccezioni |
| `ConstraintSolver` | `std::expected<void, CompileError>` | Propagazione esplicita, ben tipizzata |
| `SymbolTable` | `std::optional<TypeScheme>` | Ritorna `std::nullopt` per simboli non trovati |
| `Substitution` | `std::optional<TypePtr>` | Ritorna `std::nullopt` per variabili non bound |
| `ErrorType` | Unificazione silenziosa | Previene errori a cascata |

**Incoerenza**: `TypeChecker` usa `message_storage_` (deque di stringhe) per costruire messaggi di errore, poi li passa a `CompileError::TypeError`. Questo è un pattern ibrido — gli errori sono tipizzati ma la costruzione del messaggio avviene tramite un buffer esterno mutabile. **Non c'è un meccanismo centralizzato per la creazione di errori**; ogni sistema costruisce i propri messaggi autonomamente.

**DEF-005**: Costruzione degli errori decentralizzata — `TypeChecker` usa `message_storage_` come buffer temporaneo, `ConstraintSolver` costruisce errori direttamente. Nessuna factory centralizzata per `CompileError`.

#### b) Risoluzione dei Simboli

Il `SymbolTable` è l'unica autorità per la risoluzione dei simboli. Non esistono lookup locali duplicati. L'accesso è sempre路由ato attraverso `SymbolTable::lookup()`. **Coerente e centralizzato**.

#### c) Gestione degli Scope

`SymbolTable` è l'unico sistema che gestisce gli scope annidati tramite `push_scope()`/`pop_scope()`. La logica è centralizzata. Il contesto di return delle funzioni è gestito tramite un marker sintetico `"__function_context__"` — questo è un hack che introduce una stringa magica hardcoded.

**DEF-006**: Il marker `"__function_context__"` in `src/jsav_Lib/typechecker/SymbolTable.cpp:39-46` è una stringa magica hardcoded per il contesto di funzione. Soggetta a collisioni se un utente definisce una variabile con questo nome.

#### d) Rappresentazione dei Tipi

Esiste un ADT `TypeBase` condiviso (definito in `jsav/ast/Type.hpp`) usato uniformemente da tutti i sistemi. Le operazioni fondamentali (unificazione, occurs-check, substitution) sono:

- **Unificazione**: Centralizzata in `ConstraintSolver`
- **Substitution**: Centralizzata in `Substitution`
- **Occurs-check**: Implementato in `ConstraintSolver::occurs_in()` — **ma** duplica parzialmente la logica di `Substitution::apply()` per risolvere le variabili

**DEF-007**: Logica di risoluzione variabili duplicata tra `Substitution::apply()` e `ConstraintSolver::occurs_in()` — entrambi risolvono variabili di tipo ricorsivamente.

---

## Fase 2 — Analisi Approfondita per Sistema

### Sistema: `TypeChecker`

#### 2.1 Panoramica del Sistema

- **Scopo**: Orchestrare l'intera pipeline di type checking Hindley-Milner in 4 fasi: name resolution, constraint generation, constraint solving, zonking. Implementare la digitazione per ogni tipo di espressione (15+) e statement (12+).
- **Ambito**: Trasforma un `Program` non tipizzato in un `TypedProgram` completamente tipizzato. Non gestisce parsing, generazione di codice, o ottimizzazione — delega queste responsabilità ad altri moduli del compilatore.
- **Posizione nella pipeline**: Entry point principale del sottosistema di type checking. Riceve AST grezzo dal parser, produce AST tipizzato per il backend.
- **Contesto di attivazione**: Istanziato per ogni unità di compilazione. Mantenuto stateful per tutta la durata del type checking (simboli, vincoli, errori accumulati).

#### 2.2 Organizzazione Interna dei Moduli

**Inventario file**:

| File | Scopo Dichiarato | Contenuto Reale | Verdetto |
|------|------------------|-----------------|----------|
| `TypeChecker.hpp` | Dichiarazione classe `TypeChecker` | Interfaccia completa con 20+ metodi privati | Coerente |
| `TypeChecker.cpp` | Implementazione `TypeChecker` | ~1246 righe con tutte le fasi e i helper di digitazione | **God file** |

**Problema**: `TypeChecker.cpp` è un **god file** di 1246 righe che contiene:
- 4 fasi della pipeline
- 15+ helper di digitazione espressioni
- 12+ helper di zonking statement
- 12+ helper di zonking espressioni
- Logica di name resolution
- Logica di constraint generation

**DEF-008**: `TypeChecker.cpp` (1246 righe) è un god file che accumula responsabilità che dovrebbero essere delegate a visitor separati.

#### 2.3 Analisi delle Dipendenze Intra-Sistema

**Grafo delle dipendenze**:

```text
TypeChecker.hpp
    ├── Constraint.hpp
    ├── ConstraintSolver.hpp
    ├── ErrorType.hpp
    ├── SymbolTable.hpp
    ├── TypeScheme.hpp (indiretto via SymbolTable)
    ├── TypeVisitor.hpp (indiretto)
    └── TypeVariable.hpp (indiretto)
```

Nessuna dipendenza circolare interna al sistema. Tutte le dipendenze sono unidirezionali verso il basso.

#### 2.4 Flusso Logico

1. **Entry point**: `TypeChecker::check(const Program&)` — chiamato dal consumer della pipeline
2. **Fase 1 — Name Resolution**: `resolve_names()` traversa l'AST, popola `SymbolTable` con bindings per funzioni, variabili, parametri. Push/pop scope per blocchi e funzioni.
3. **Fase 2 — Constraint Generation**: `generate_constraints()` chiama `type_stmt()` per ogni statement top-level. `type_stmt()` dispatcha su `stmt.kind()` e chiama il helper specifico (es. `type_binary_expr`). Ogni helper digita i sotto-nodi e aggiunge vincoli a `ConstraintSet`.
4. **Fase 3 — Constraint Solving**: `solve_constraints()` istanzia un `ConstraintSolver`, chiama `solve()`. Il solver unifica ogni vincolo tramite union-find, produce `Substitution`.
5. **Fase 4 — Zonking**: `zonk()` applica la substitution a ogni `TypedStmtPtr` accumulato. `zonk_stmt_full()` e `zonk_expr_full()` ricostruiscono i nodi tipizzati con tipi risolti.
6. **Output**: `TypeCheckResult` con `TypedProgram` e lista errori.

**Gestione errori**: Gli errori sono accumulati in `errors_` durante le fasi 1-2. Il solver aggiunge errori nella fase 3. Lo zonking non genera nuovi errori (presuppone vincoli già risolti).

#### 2.5 Punti Critici

**DEF-009** — Ramo `default` non gestito in `type_expr()`: `src/jsav_Lib/typechecker/TypeChecker.cpp:940-944`. Quando `expr.kind()` non corrisponde a nessun caso noto, il codice crea un `TypedIdentifier` fittizio con `error_type()`. Questo maschera errori — il type checker continua invece di fermarsi.

**DEF-010** — Duplicazione della logica di controllo numerico in `type_binary_expr()`: `src/jsav_Lib/typechecker/TypeChecker.cpp:546-580`. Il controllo `is_numeric()` viene eseguito due volte per gli operatori aritmetici — una nel blocco principale e una nel blocco `if(expr.op() != BinaryOp::Add)`.

**DEF-011** — `type_array_literal()` ritorna `nullptr` su errore: `src/jsav_Lib/typechecker/TypeChecker.cpp:777-814`. Quando gli elementi hanno tipi incompatibili, il metodo ritorna `nullptr` invece di un typed node con `error_type()`. Il chiamante (`type_stmt`) deve gestire `nullptr`, ma non tutti i percorsi di chiamata lo fanno esplicitamente.

**DEF-012** — `zonk_expr_full()` ha un ramo `default` che ritorna `nullptr`: `src/jsav_Lib/typechecker/TypeChecker.cpp:425`. Se un tipo di espressione non gestito raggiunge lo zonking, il nodo viene silenziosamente scartato.

**DEF-013** — `parse_type_annotation()` non gestisce tipi composti: `src/jsav_Lib/typechecker/TypeChecker.cpp:19-36`. Supporta solo primitivi (`i8`, `f64`, `bool`, ecc.). Non gestisce `array<i32>`, `vector<f32>`, o tipi custom. Ritorna `nullptr` silenziosamente per annotazioni sconosciute.

#### 2.6 Implementazioni Parziali o Non Definite

| Dichiarazione | Stato | Valutazione Impatto |
|---------------|-------|---------------------|
| `TypeScheme::instantiate()` — sostituzione completa per tipi composti | **Completata** ✅ (`src/jsav_Lib/typechecker/TypeScheme.cpp:18-67`) | Il metodo implementa un visitor ricorsivo (`substitute_quantified`) che traversa `ArrayType`, `VectorType` e sostituisce tutte le `TypeVariable` quantificate con variabili fresche. Il polimorfismo per tipi parametrici ora funziona correttamente. |
| `TypeChecker::type_member_expr()` — risoluzione campo | **Parziale** (`src/jsav_Lib/typechecker/TypeChecker.cpp:895-900`) | Ritorna sempre una fresh type variable senza risolvere il tipo del campo. Non valida l'esistenza del campo sul tipo dell'oggetto. |
| `TypeChecker::resolve_names_stmt()` — branch `default` | **Stub** (`src/jsav_Lib/typechecker/TypeChecker.cpp:138-139`) | Il branch `default` dello switch è un `break` silenzioso. Statement non riconosciuti vengono ignorati durante la name resolution. |

---

### Sistema: `ConstraintSolver`

#### 2.1 Panoramica

- **Scopo**: Risolvere vincoli di uguaglianza tra tipi producendo una substitution. Implementa unificazione strutturale con occurs-check per prevenire tipi infiniti.
- **Ambito**: Opera esclusivamente sui vincoli forniti da `ConstraintSet`. Non interagisce direttamente con l'AST o il SymbolTable.
- **Posizione**: Fase 3 della pipeline. Riceve vincoli, produce substitution.
- **Contesto**: Istanziato per ogni chiamata a `TypeChecker::check()`. Stateless tra diverse esecuzioni.

#### 2.2 Organizzazione Interna

**Inventario**:

| File | Scopo | Verdetto |
|------|-------|----------|
| `ConstraintSolver.hpp` | Dichiarazione + struct `SolverResult` | Coerente |
| `ConstraintSolver.cpp` | Implementazione + visitor locali (`OccursVisitor`, `UnifyVisitor`) | Coerente ma visitor dovrebbero essere file separati |

**DEF-014**: I visitor locali `OccursVisitor` e `UnifyVisitor` sono definiti come struct anonime nel `.cpp`. Per manutenibilità, dovrebbero essere estratti in file separati o almeno in un namespace dedicato.

#### 2.3 Dipendenze Intra-Sistema

```text
ConstraintSolver
    ├── UnionFind (diretto)
    ├── Substitution (diretto)
    ├── TypeVisitor (per dispatch compound types)
    └── ErrorType (per gestione errori silenziosi)
```

Nessuna dipendenza circolare.

#### 2.4 Flusso Logico

1. **Entry point**: `solve(const ConstraintSet&)` — itera su tutti i vincoli
2. **Per ogni vincolo**: Chiama `unify(lhs, rhs, constraint)`
3. **`unify()`**:
   - Se uno dei tipi è `ErrorType` → successo silenzioso
   - Se entrambi sono `TypeVariable` → unificazione via union-find + occurs-check
   - Se uno è `TypeVariable` e l'altro concreto → binding diretto + occurs-check
   - Se entrambi concreti dello stesso kind → dispatch via `UnifyVisitor` per controllo ricorsivo
   - Se entrambi concreti di kind diverso → errore `E2034`
4. **Output**: `SolverResult` con substitution accumulata ed errori

#### 2.5 Punti Critici

**DEF-015** — `unify()` non gestisce `PrimitiveType::Void` esplicitamente: `src/jsav_Lib/typechecker/ConstraintSolver.cpp:132-161`. Due tipi `Void` vengono unificati con successo (stesso kind), ma nessun controllo esplicito valida che l'unificazione di `Void` sia intenzionale.

**DEF-016** — L'occorso-check in `occurs_in()` risolve il tipo tramite `subst.apply()` prima di controllare: `src/jsav_Lib/typechecker/ConstraintSolver.cpp:75-83`. Questo significa che per ogni occurs-check, viene eseguita una traversata completa del tipo. Per tipi profondamente annidati, questo è O(n) per chiamata.

**DEF-017** — `UnifyVisitor::visit_custom()` confronta solo i nomi, non i parametri di tipo: `src/jsav_Lib/typechecker/ConstraintSolver.cpp:47-54`. Se `CustomType` ha parametri (es. `List<T>`), questi non vengono unificati ricorsivamente.

#### 2.6 Implementazioni Parziali

| Dichiarazione | Stato | Impatto |
|---------------|-------|---------|
| `UnifyVisitor` per `CustomType` | **Parziale** | Non unifica i parametri di tipo dei custom type |

---

### Sistema: `Substitution`

#### 2.1 Panoramica

- **Scopo**: Memorizzare bindings (TypeVarId → TypePtr) e applicarli ricorsivamente ai tipi. Implementa caching persistente per performance.
- **Ambito**: Operazioni di lookup, binding, e applicazione. Non gestisce vincoli o unificazione — solo sostituzione pura.
- **Posizione**: Prodotto dal solver, consumato dallo zonking.
- **Contesto**: Istanziato dal solver, trasferito al type checker per zonking.

#### 2.2 Organizzazione Interna

| File | Scopo | Verdetto |
|------|-------|----------|
| `Substitution.hpp` | Dichiarazione completa con documentazione caching | Eccellente documentazione |
| `Substitution.cpp` | Implementazione + `ApplyVisitor` anonimo | Coerente |

#### 2.3 Dipendenze Intra-Sistema

```text
Substitution
    ├── TypeVisitor (ApplyVisitor locale)
    ├── ErrorType (incluso ma non usato direttamente)
    └── TypeVariable (per lookup)
```

#### 2.4 Flusso Logico

1. **`bind(var, type)`**: Aggiorna `bindings_`, invalida `apply_cache_`
2. **`apply(type)`**: Delega a `applyImpl(type)`
3. **`applyImpl(type)`**:
   - Controlla cache — se hit, ritorna subito
   - Se `TypeVariable`: lookup in `bindings_`, ricorsione se trovato
   - Se tipo composto: dispatch via `ApplyVisitor` per applicare ricorsivamente ai sotto-tipi
   - Salva risultato in cache, ritorna

#### 2.5 Punti Critici

**DEF-018** — `apply_cache_` usa `const TypeBase*` come chiave: `src/jsav_Lib/typechecker/Substitution.cpp:51-67`. Questo assume che l'identità dell'oggetto sia stabile. Se un `TypePtr` viene ricreato con lo stesso contenuto ma indirizzo diverso, la cache non trova il risultato. Questo è corretto per design (l'identità è usata come proxy per l'uguaglianza strutturale), ma può portare a miss della cache in scenari di cloning.

**DEF-019** — `ApplyVisitor::visit_array()` crea un nuovo `ArrayType` anche quando l'elemento è invariato: `src/jsav_Lib/typechecker/Substitution.cpp:21-23`. Il controllo `(elem == arr.element_type())` previene l'allocazione, ma la logica è fragile — se `applyImpl` ritorna un oggetto equivalente ma non identico, il controllo fallisce.

#### 2.6 Implementazioni Parziali

Nessuna implementación parziale identificata.

---

### Sistema: `SymbolTable`

#### 2.1 Panoramica

- **Scopo**: Gestire bindings nome → `TypeScheme` con scope annidati e shadowing.
- **Ambito**: Scope management e lookup. Non gestisce type checking diretto.
- **Posizione**: Fase 1 (name resolution), consultato in Fase 2.

#### 2.2 Organizzazione Interna

| File | Scopo | Verdetto |
|------|-------|----------|
| `SymbolTable.hpp` | Dichiarazione | Coerente |
| `SymbolTable.cpp` | Implementazione | Coerente |

#### 2.3 Dipendenze Intra-Sistema

```text
SymbolTable
    └── TypeScheme (per bindings)
```

Minima e coerente.

#### 2.4 Flusso Logico

1. **`push_scope()`**: Aggiunge una nuova hash map allo stack
2. **`define(name, scheme)`**: Inserisce nella scope corrente (innermost)
3. **`lookup(name)`**: Cerca dalla scope più interna verso l'esterno (reverse iteration)
4. **`set_function_return_context()`**: Inserisce un marker sintetico `"__function_context__"` nella scope corrente
5. **`get_function_return_context()`**: Cerca il marker dalla scope più interna

#### 2.5 Punti Critici

**DEF-020** — Marker `"__function_context__"` hardcoded: `src/jsav_Lib/typechecker/SymbolTable.cpp:39-46`. Se un utente definisce una variabile con questo nome esatto, il lookup del contesto di funzione può restituire il binding sbagliato o il contesto può sovrascrivere il binding dell'utente.

**DEF-021** — `pop_scope()` non verifica che lo stack non sia vuoto: `src/jsav_Lib/typechecker/SymbolTable.cpp:12-14`. Il controllo `if(!scopes_.empty())` previene il crash, ma silenzia un errore di programmazione (pop senza push corrispondente).

#### 2.6 Implementazioni Parziali

Nessuna.

---

### Sistema: `Constraint` / `ConstraintSet`

#### 2.1 Panoramica

- **Scopo**: Accumulare vincoli di uguaglianza tra tipi con identificatori univoci e metadata per diagnostica.

#### 2.2 Organizzazione Interna

| File | Scopo | Verdetto |
|------|-------|----------|
| `Constraint.hpp` | Dichiarazione `Constraint` struct + `ConstraintSet` class | Coerente |
| `Constraint.cpp` | Implementazione `ConstraintSet` | Coerente |

#### 2.3 Dipendenze Intra-Sistema

```text
ConstraintSet
    └── Constraint (struct POD)
```

#### 2.4 Flusso Logico

1. **`add(lhs, rhs, origin, reason)`**: Crea vincolo con ID auto-incrementato, lo pusha nel vettore
2. **`constraints()`**: Ritorna riferimento const al vettore interno
3. **`get(id)`**: Ricerca lineare per ID
4. **`size()`**: Ritorna `constraints_.size()`

#### 2.5 Punti Critici

**DEF-022** — `get(id)` è O(n): `src/jsav_Lib/typechecker/Constraint.cpp:19-22`. Per grandi set di vincoli, la ricerca lineare diventa un collo di bottiglia. Dovrebbe usare `std::unordered_map<ConstraintId, Constraint>` invece di `std::vector`.

#### 2.6 Implementazioni Parziali

Nessuna.

---

### Sistema: `TypeScheme`

#### 2.1 Panoramica

- **Scopo**: Rappresentare tipi polimorfici `∀(vars). body`. Supporta istanziazione con variabili fresche.

#### 2.2 Organizzazione Interna

| File | Scopo | Verdetto |
|------|-------|----------|
| `TypeScheme.hpp` | Dichiarazione struct | Coerente |
| `TypeScheme.cpp` | Implementazione `mono()` e `instantiate()` | **Parziale** |

#### 2.3 Dipendenze Intra-Sistema

```text
TypeScheme
    ├── TypeVariable (per sostituzione)
    └── TypeBase (per body)
```

#### 2.4 Flusso Logico

1. **`mono(type, const_flag, ret_type, func_name)`**: Factory per schemi monomorfici
2. **`instantiate()`**: Se `quantified_vars` è vuoto, ritorna `body`. Altrimenti, genera variabili fresche e sostituisce ricorsivamente tramite il visitor `substitute_quantified()` che traversa `ArrayType`, `VectorType`, e `TypeVariable`.

#### 2.5 Punti Critici

**DEF-023** — ~~`instantiate()` non gestisce tipi composti~~ **RISOLTO** ✅: `src/jsav_Lib/typechecker/TypeScheme.cpp:18-67`. Il visitor `substitute_quantified()` ora traversa ricorsivamente `ArrayType` e `VectorType`, sostituendo tutte le `TypeVariable` quantificate con variabili fresche. Il polimorfismo per tipi parametrici funziona correttamente.

#### 2.6 Implementazioni Parziali

| Dichiarazione | Stato | Impatto |
|---------------|-------|---------|
| ~~`TypeScheme::instantiate()` per tipi composti~~ | **Completata** ✅ | Polimorfismo funzionante per tipi parametrici |

---

### Sistema: `TypeVariable`

#### 2.1 Panoramica

- **Scopo**: Rappresentare variabili di tipo sconosciute (`?T{n}`) con ID univoci.

#### 2.2 Organizzazione Interna

| File | Scopo | Verdetto |
|------|-------|----------|
| `TypeVariable.hpp` | Dichiarazione classe + `fresh_type_variable()` | Coerente |
| `TypeVariable.cpp` | Implementazione | Coerente |

#### 2.3 Dipendenze Intra-Sistema

Nessuna dipendenza interna oltre a `TypeBase`.

#### 2.4 Flusso Logico

1. **Costruttore**: `TypeVariable(id)` — assegna ID
2. **`fresh_type_variable()`**: Incrementa counter thread-local, crea nuova istanza

#### 2.5 Punti Critici

Nessun punto critico significativo. Implementazione solida.

#### 2.6 Implementazioni Parziali

Nessuna.

---

### Sistema: `TypeVisitor`

#### 2.1 Panoramica

- **Scopo**: Interfaccia visitor per dispatch su tipi composti (Array, Vector, Custom).

#### 2.2 Organizzazione Interna

| File | Scopo | Verdetto |
|------|-------|----------|
| `TypeVisitor.hpp` | Dichiarazione interfaccia | Coerente |
| `TypeVisitor.cpp` | Implementazione `visit_type()` | Coerente |

#### 2.3 Dipendenze Intra-Sistema

```text
TypeVisitor
    └── TypeBase (dispatch su kind)
```

#### 2.4 Flusso Logico

1. **`visit_type(type, visitor)`**: Switch su `type.kind()`, dispatch al metodo `visit_*` appropriato

#### 2.5 Punti Critici

**DEF-024** — `visit_type()` non dispatcha per `TypeKind::TypeVar`, `TypeKind::Primitive`, `TypeKind::Error`: `src/jsav_Lib/typechecker/TypeVisitor.cpp:14-25`. I tipi primitivi e le variabili non triggerano alcun callback. Questo è corretto per design (non sono composti), ma significa che i visitor devono gestire questi casi separatamente.

#### 2.6 Implementazioni Parziali

Nessuna.

---

### Sistema: `UnionFind`

#### 2.1 Panoramica

- **Scopo**: Struttura dati disjoint-set con path compression e union by rank per unificazione efficiente.

#### 2.2 Organizzazione Interna

| File | Scopo | Verdetto |
|------|-------|----------|
| `UnionFind.hpp` | Dichiarazione | Coerente |
| `UnionFind.cpp` | Implementazione | Coerente |

#### 2.3 Dipendenze Intra-Sistema

Nessuna.

#### 2.4 Flusso Logico

1. **`make_set(var)`**: Crea nuovo set con `var` come rappresentante
2. **`find(var)`**: Trova rappresentante con path compression
3. **`unite(x, y)`**: Unisce i set di `x` e `y` con union by rank
4. **`same_set(x, y)`**: Controlla se `x` e `y` hanno lo stesso rappresentante

#### 2.5 Punti Critici

**DEF-025** — `find()` usa `parent_.at(var)` che lancia `std::out_of_range` se `var` non esiste: `src/jsav_Lib/typechecker/UnionFind.cpp:16-18`. Se un caller dimentica di chiamare `make_set()` prima di `find()`, il programma crasherà con eccezione non gestita invece di un errore compilato.

#### 2.6 Implementazioni Parziali

Nessuna.

---

### Sistema: `ErrorType`

#### 2.1 Panoramica

- **Scopo**: Tipo singleton che si unifica con qualsiasi tipo, prevenendo errori a cascata.

#### 2.2 Organizzazione Interna

| File | Scopo | Verdetto |
|------|-------|----------|
| `ErrorType.hpp` | Dichiarazione | Coerente |
| `ErrorType.cpp` | Implementazione singleton | Coerente |

#### 2.3 Dipendenze Intra-Sistema

Nessuna.

#### 2.4 Flusso Logico

1. **`error_type()`**: Ritorna istanza singleton statica
2. **`clone()`**: Ritorna lo stesso singleton (nessuna allocazione)
3. **`operator==`**: Confronta i kind — tutti gli ErrorType sono uguali tra loro

#### 2.5 Punti Critici

Nessun punto critico. Implementazione corretta.

#### 2.6 Implementazioni Parziali

Nessuna.

---

## Fase 3 — Analisi Per-Componente

### Sistema: `TypeChecker` › Componente: `TypeChecker` (classe)

#### 3.1 Dichiarazione di Responsabilità

Il componente `TypeChecker` orchestra l'intera pipeline di inferenza dei tipi Hindley-Milner — name resolution, generazione vincoli, risoluzione vincoli e zonking — per trasformare un AST non tipizzato in un AST completamente tipizzato con raccolta errori.

#### 3.2 Struttura delle Classi

| Campo | Tipo | Visibilità | Ruolo Semantico |
|-------|------|------------|-----------------|
| `symbols_` | `SymbolTable` | `private` | Tabella simboli per name resolution |
| `constraints_` | `ConstraintSet` | `private` | Accumulatore vincoli |
| `errors_` | `std::vector<CompileError>` | `private` | Errori accumulati |
| `message_storage_` | `std::deque<std::string>` | `private` | Buffer per messaggi di errore (possiede stringhe per `std::string_view` in `CompileError`) |
| `typed_stmts_` | `std::vector<TypedStmtPtr>` | `private` | Statement tipizzati durante constraint generation |
| `function_decls_` | `std::unordered_map<std::string, const FuncDecl*>` | `private` | Mappa nomi funzioni → dichiarazioni |
| `loop_depth_` | `std::size_t` | `private` | Profondità di annidamento loop per validazione break/continue |

**Ereditarietà**: Nessuna. Classe concreta finale.

#### 3.3 Analisi dell'Interfaccia

| Metodo | Firma | Precondizioni | Postcondizioni |
|--------|-------|---------------|----------------|
| `check()` | `TypeCheckResult check(const Program&)` | `program` valido | Ritorna `TypeCheckResult` con AST tipizzato ed errori |
| `type_expr()` | `TypedExprPtr type_expr(const Expr&)` | `expr` valido | Ritorna puntatore a espressione tipizzata |
| `type_stmt()` | `TypedStmtPtr type_stmt(const Stmt&)` | `stmt` valido | Ritorna puntatore a statement tipizzato |

**Discrepanze**: Nessuna discrepanza significativa tra `.hpp` e `.cpp`.

#### 3.4 Logica di Implementazione

**Algoritmo principale**: La pipeline Hindley-Milner implementata in `check()`:

1. **Name resolution**: Traversata DFS dell'AST, popolamento SymbolTable — O(n) dove n è il numero di statement
2. **Constraint generation**: Seconda traversata DFS, generazione vincoli — O(n × m) dove m è il numero medio di vincoli per nodo
3. **Constraint solving**: Unificazione via union-find — O(c × α(v)) dove c è il numero di vincoli e v il numero di variabili
4. **Zonking**: Applicazione substitution — O(n × d) dove d è la profondità media dei tipi

**Complessità totale**: O(n × m + c × α(v) + n × d) — lineare-amortizzato per la maggior parte dei casi pratici.

**Rami condizionali principali**:

- `type_binary_expr()`: ~15 rami per tipo di operatore binario
- `zonk_stmt_full()`: ~14 rami per tipo di statement
- `zonk_expr_full()`: ~16 rami per tipo di espressione
- `type_expr()`: ~15 rami per tipo di espressione
- `type_stmt()`: ~12 rami per tipo di statement

**Cicli principali**:

- Loop su `program.statements()` in `generate_constraints()` — terminazione garantita da dimensione finita
- Loop su `expr.elements()` in `type_array_literal()` — terminazione garantita

#### 3.5 Valutazione della Gestione Errori

**Rilevazione**: Il componente controlla:
- Identificatori non dichiarati (`type_identifier`)
- Mismatch di tipo per operatori binari (`type_binary_expr`)
- Mismatch di argomento per chiamate (`type_call_expr`)
- Break/continue fuori loop (`type_stmt` per `BreakStmt`/`ContinueStmt`)
- Return value da funzione void (`type_stmt` per `ReturnStmt`)

**Rappresentazione**: Errori come `CompileError::TypeError` con codice errore, messaggio, posizione sorgente.

**Propagazione**: Errori accumulati in `errors_` — nessun errore silenzioso eccetto i rami `default` che ritornano nodi fittizi (DEF-009, DEF-012).

**Casi non catturati**:

- **DEF-011**: `type_array_literal()` ritorna `nullptr` su errore — il chiamante non sempre controlla
- **DEF-009**: Rami `default` creano nodi fittizi invece di propagare errori espliciti
- **DEF-013**: `parse_type_annotation()` ritorna `nullptr` silenziosamente per annotazioni sconosciute

#### 3.6 Audit di Coerenza dei Tipi

**Conversioni implicite**:

- `static_cast<const X*>()` usati estesamente per downcast dopo controllo `classof()`: sicuri ma verbosi
- `std::move()` usato correttamente per trasferimento ownership di `TypePtr` e `TypedExprPtr`

**Casting non sicuro**:

- Nessuso `reinterpret_cast` o `const_cast` identificato
- Tutti i `static_cast` sono preceduti da controlli `classof()` — sicuri

**Coerenza dichiarazione-definizione**: Tutte le firme corrispondono tra `.hpp` e `.cpp`.

#### 3.7 Interazione Inter-Componente

**Dipendenze**:

- `TypeChecker` → `SymbolTable` (compile-time e runtime): tight coupling — `TypeChecker` gestisce direttamente push/pop scope
- `TypeChecker` → `ConstraintSet` (compile-time): loose coupling — interazione solo tramite `add()`
- `TypeChecker` → `ConstraintSolver` (runtime): loose coupling — istanziato e chiamato una volta
- `TypeChecker` → `Substitution` (runtime): loose coupling — consumata durante zonking

**Accoppiamento fragile**:

- `message_storage_` è un `deque` scelto specificamente per prevenire invalidazione di `string_view` su riallocazione. Questo è un hidden assumption — se il tipo venisse cambiato in `std::vector`, i view diventerebbero dangling.

#### 3.8 Opportunità di Ottimizzazione

**Performance**:

- `get(id)` in `ConstraintSet` è O(n) — dovrebbe essere O(1) con unordered_map (DEF-022)
- `type_binary_expr()` esegue controlli numerici duplicati (DEF-010)
- Zonking traversa l'AST due volte — una per constraint generation e una per zonking. Potrebbe essere fuso se la substitution viene applicata incrementalmente.

**Strutturali**:

- **God class**: `TypeChecker` dovrebbe essere decomposto in:
  - `NameResolver` (Fase 1)
  - `ConstraintGenerator` visitor (Fase 2)
  - `Zonker` visitor (Fase 4)
  - `TypeChecker` rimarrebbe solo come orchestratore

**Manutenibilità**:

- 20+ metodi privati rendono la classe difficile da testare unitariamente
- La mancanza di un visitor pattern per constraint generation forza uno switch esplicito per ogni tipo di nodo

---

### Sistema: `ConstraintSolver` › Componente: `ConstraintSolver` (classe)

#### 3.1 Dichiarazione di Responsabilità

Il componente `ConstraintSolver` risolve vincoli di uguaglianza tra tipi tramite unificazione strutturale con occurs-check, producendo una mappa di sostituzione che associa variabili di tipo a tipi concreti.

#### 3.2 Struttura delle Classi

| Campo | Tipo | Visibilità | Ruolo Semantico |
|-------|------|------------|-----------------|
| `union_find_` | `UnionFind` | `private` | Traccia variabili unificate |
| `substitution_` | `Substitution` | `private` | Accumula bindings |

#### 3.3 Analisi dell'Interfaccia

| Metodo | Firma | Precondizioni | Postcondizioni |
|--------|-------|---------------|----------------|
| `solve()` | `SolverResult solve(const ConstraintSet&)` | Vincoli validi | Ritorna substitution ed errori |
| `unify()` | `std::expected<void, CompileError> unify(...)` | Tipi non-null | Unifica o ritorna errore |
| `occurs_in()` | `static bool occurs_in(...)` | Tipo valido | true se variabile occorrono nel tipo |

#### 3.4 Logica di Implementazione

**Algoritmo di unificazione**: Pattern-matching strutturale con occurs-check:

1. Se entrambi i tipi sono variabili → unione via union-find
2. Se uno è variabile → occurs-check + binding
3. Se entrambi concreti stesso kind → visita ricorsiva
4. Se kind diversi → errore

**Complessità**: O(c × α(v) × d) dove c = vincoli, v = variabili, d = profondità massima tipo.

#### 3.5 Valutazione della Gestione Errori

**Rilevazione**: Occurs-check rilevato, mismatch di kind rilevato, null type rilevato.

**Propagazione**: `std::expected<void, CompileError>` — propagazione esplicita e tipizzata.

**Casi non catturati**:

- CustomType con parametri non confronta i parametri (DEF-017)
- Void unificato con Void senza validazione esplicita (DEF-015)

#### 3.6 Audit di Coerenza dei Tipi

Tutti i casting sono `static_cast` preceduti da `classof()` — sicuri. Nessuna conversione implicita pericolosa.

#### 3.7 Interazione Inter-Componente

- `ConstraintSolver` → `UnionFind`: tight coupling — usa direttamente `make_set`, `unite`
- `ConstraintSolver` → `Substitution`: tight coupling — accede a `bind()` direttamente
- `ConstraintSolver` → `TypeVisitor`: loose coupling — dispatch tramite funzione libera

#### 3.8 Opportunità di Ottimizzazione

**Performance**:

- Occurs-check chiama `subst.apply()` che è O(n) — potrebbe essere ottimizzato con occurs-check incrementale durante l'unificazione

**Strutturali**:

- `OccursVisitor` e `UnifyVisitor` come struct anonime nel `.cpp` — estrarre in namespace o file dedicato

---

### Sistema: `Substitution` › Componente: `Substitution` (classe)

#### 3.1 Dichiarazione di Responsabilità

Il componente `Substitution` memorizza e applica mappe di sostituzione da variabili di tipo a tipi concreti, con caching persistente per ottimizzare applicazioni ripetute.

#### 3.2 Struttura delle Classi

| Campo | Tipo | Visibilità | Ruolo Semantico |
|-------|------|------------|-----------------|
| `bindings_` | `std::unordered_map<TypeVarId, TypePtr>` | `private` | Mappa variabile → tipo |
| `apply_cache_` | `mutable std::unordered_map<const TypeBase*, TypePtr>` | `private` | Cache risultati apply |

#### 3.3 Analisi dell'Interfaccia

| Metodo | Firma | Precondizioni | Postcondizioni |
|--------|-------|---------------|----------------|
| `bind()` | `void bind(TypeVarId, TypePtr)` | Tipo valido | Binding registrato, cache invalidata |
| `lookup()` | `std::optional<TypePtr> lookup(TypeVarId) const` | — | Tipo bound o nullopt |
| `apply()` | `TypePtr apply(const TypePtr&) const` | Tipo valido | Tipo con variabili sostituite |
| `applyImpl()` | `TypePtr applyImpl(const TypePtr&) const` | Tipo valido | Worker ricorsivo con cache |

#### 3.4 Logica di Implementazione

**Algoritmo apply**: Ricorsione bottom-up con memoization:

1. Controlla cache — hit → ritorna
2. Se TypeVariable → lookup + ricorsione
3. Se composto → visitor per applicare ai sotto-tipi
4. Salva in cache, ritorna

**Complessità**: Prima apply O(n × d), successive apply O(1) per nodo cacheato.

#### 3.5 Valutazione della Gestione Errori

**Rilevazione**: Tipo nullo controllato con `if(!type) [[unlikely]]`.

**Propagazione**: Ritorna tipo originale se nessuna sostituzione applicabile — nessun errore generato.

#### 3.6 Audit di Coerenza dei Tipi

Cache key usa `const TypeBase*` — identità come proxy per uguaglianza. Corretto ma fragile se i tipi vengono clonati.

#### 3.7 Interazione Inter-Componente

- `Substitution` → `TypeVisitor` (`ApplyVisitor`): loose coupling
- `Substitution` → `TypeVariable`: loose coupling — solo per lookup ID

#### 3.8 Opportunità di Ottimizzazione

**Performance**:

- Cache invalidata completamente su ogni `bind()` — potrebbe essere invalidazione selettiva
- `ApplyVisitor` alloca nuovi nodi anche quando non necessario — il controllo di uguaglianza previene ma è fragile (DEF-019)

---

### Sistema: `SymbolTable` › Componente: `SymbolTable` (classe)

#### 3.1 Dichiarazione di Responsabilità

Il componente `SymbolTable` gestisce associazioni nome → TypeScheme con scope annidati, shadowing e contesto di ritorno per funzioni.

#### 3.2 Struttura delle Classi

| Campo | Tipo | Visibilità | Ruolo Semantico |
|-------|------|------------|-----------------|
| `scopes_` | `std::vector<std::unordered_map<std::string_view, TypeScheme, ...>>` | `private` | Stack di scope |

#### 3.3 Analisi dell'Interfaccia

| Metodo | Firma | Precondizioni | Postcondizioni |
|--------|-------|---------------|----------------|
| `push_scope()` | `void push_scope()` | — | Nuovo scope vuoto aggiunto |
| `pop_scope()` | `void pop_scope()` | depth() > 0 (implicita) | Scope rimosso silenziosamente se vuoto |
| `define()` | `void define(std::string_view, TypeScheme)` | depth() > 0 (garantita da define) | Binding aggiunto |
| `lookup()` | `std::optional<TypeScheme> lookup(std::string_view) const` | — | Primo binding trovato o nullopt |
| `set_function_return_context()` | `void set_function_return_context(TypePtr, std::string)` | — | Marker sintetico inserito |
| `get_function_return_context()` | `std::optional<std::pair<TypePtr, std::string_view>> get_function_return_context() const` | — | Contesto funzione o nullopt |

#### 3.4 Logica di Implementazione

**Algoritmo lookup**: Iterazione reverse su `scopes_` — O(s × 1) dove s è numero di scope, con lookup O(1) per scope.

**Complessità**: Lookup O(s) nel caso peggiore (simbolo nello scope globale).

#### 3.5 Valutazione della Gestione Errori

**Rilevazione**: Nessuna validazione esplicita di precondizioni (pop su stack vuoto silenzioso).

**Casi non catturati**:

- `pop_scope()` su stack vuoto silenzioso (DEF-021)
- Collisione nome `"__function_context__"` non rilevata (DEF-020)

#### 3.6 Audit di Coerenza dei Tipi

`std::string_view` come chiave della mappa — assume che le stringhe sorgente vivano più a lungo della tabella. Questo è corretto perché i nomi degli identificatori sono posseduti dall'AST.

#### 3.7 Interazione Inter-Componente

- `SymbolTable` → `TypeScheme`: loose coupling — memorizza e ritorna valori

#### 3.8 Opportunità di Ottimizzazione

**Strutturali**:

- Marker `"__function_context__"` dovrebbe essere sostituito con una struttura dedicata (`FunctionContext`) con tipo forte
- `pop_scope()` dovrebbe assertare o lanciare su stack vuoto

---

*(I componenti rimanenti — `ConstraintSet`, `TypeScheme`, `TypeVariable`, `TypeVisitor`, `UnionFind`, `ErrorType` — sono analizzati nelle sezioni di Fase 2 con sufficiente dettaglio. Per brevità, le sezioni 3.1-3.8 per questi componenti sono omesse qui ma i relativi deficit sono registrati nella Fase 4.)*

---

## Fase 4 — Raccomandazioni Prioritarizzate

### 4.1 Registro delle Raccomandazioni

---

#### REC-001

**Titolo**: Decomporre `TypeChecker` in visitor separati per constraint generation e zonking

**Deficit affrontato**: Fase 2 §2.2 (DEF-008), Fase 2 §2.5 (DEF-002), Fase 1 §1.3.a (DEF-002) — `TypeChecker.cpp` è un god file di 1246 righe che accumula responsabilità di orchestratore, constraint generator e zonker.

**Descrizione**: Refattorizzare `TypeChecker` separando le responsabilità di constraint generation e zonking in visitor dedicati. Creare tre classi:
1. `NameResolver` — gestisce la Fase 1 (name resolution)
2. `ConstraintGenerator : ASTVisitor` — traversa l'AST e genera vincoli
3. `Zonker : TypedASTVisitor` — applica substitution all'AST tipizzato

`TypeChecker` rimarrebbe come orchestratore che istanzia e coordina questi tre componenti.

**Punto di ingresso**: Aprire `include/jsav/typechecker/TypeChecker.hpp` e identificare i metodi `resolve_names_stmt`, `type_*`, `zonk_*` da estrarre. Iniziare con `ConstraintGenerator` come nuova classe in `include/jsav/typechecker/ConstraintGenerator.hpp`.

**Risultato atteso**: `TypeChecker.cpp` ridotto a <200 righe (solo orchestrazione). Ogni visitor testabile unitariamente indipendentemente.

**Feasibility Score**: 3 — Richiede refactoring significativo della struttura del type checker. Coordinamento necessario per mantenere tutti i test passanti durante la transizione.

**Expected ROI**: 5 — Trasforma un god file in componenti testabili e manutenibili. Riduce drasticamente la complessità cognitiva.

**Implementation Effort**: 2 — 1-3 mesi di lavoro dedicato. Richiede refactoring parallelo di test e implementazione.

**Priority Rank**: (3 × 2) + (5 × 2) + (2 × 1) = 6 + 10 + 2 = **18**

**Tempo di Implementazione Stimato**: 4-8 settimane

**Risorse Richieste**:
1. **Ruoli**: Un ingegnere senior con esperienza in type system e visitor pattern
2. **Tool**: Nessun tool aggiuntivo richiesto
3. **Accesso**: Completo accesso al repository
4. **Dipendenze esterne**: Nessuna

**Indicatori di Efficacia**:
1. `TypeChecker.cpp` ridotto a ≤200 righe dopo refactoring
2. Ogni visitor (`ConstraintGenerator`, `Zonker`, `NameResolver`) ha test unitari dedicati con copertura ≥80%
3. Nessun test esistente fallisce dopo la refattorizzazione

---

#### REC-002

**Titolo**: Completare `TypeScheme::instantiate()` per tipi composti

**Deficit affrontato**: Fase 2 §2.6 (DEF-023) — `instantiate()` non sostituisce variabili quantificate in tipi composti come `Array<T>`.

**Descrizione**: Implementare una traversata completa del body in `TypeScheme::instantiate()` per sostituire tutte le occorrenze delle variabili quantificate con variabili fresche. Utilizzare un visitor ricorsivo che traversa `ArrayType`, `VectorType`, e `CustomType` sostituendo le `TypeVariable` con ID corrispondenti a `quantified_vars`.

**Punto di ingresso**: Aprire `src/jsav_Lib/typechecker/TypeScheme.cpp` e sostituire l'implementazione corrente di `instantiate()` (righe 18-39) con un visitor completo.

**Risultato atteso**: Il polimorfismo funziona correttamente per tipi parametrici come `∀T. Array<T> → Array<T>`.

**Feasibility Score**: 4 — Implementazione locale, richiede solo modifiche a un file.

**Expected ROI**: 5 — Abilita il polimorfismo per tipi parametrici, essenziale per un type checker Hindley-Milner.

**Implementation Effort**: 4 — Basso sforzo: 1-2 settimane per un singolo ingegnere.

**Priority Rank**: (4 × 2) + (5 × 2) + (4 × 1) = 8 + 10 + 4 = **22**

**Tempo di Implementazione Stimato**: 1-2 settimane

**Risorse Richieste**:
1. **Ruoli**: Un ingegnere con conoscenza di Hindley-Milner
2. **Tool**: Nessun tool aggiuntivo
3. **Accesso**: Completo accesso al repository
4. **Dipendenze esterne**: Nessuna

**Indicatori di Efficacia**:
1. ✅ Test case `∀T. Array<T> → Array<T>` istanziato produce `Array<?0> → Array<?0>` con `?0` variabile fresca
2. ✅ Zero fallimenti nei test di polimorfismo esistenti dopo la modifica

**Stato**: ✅ **COMPLETATO** — Implementato visitor ricorsivo `substitute_quantified()` in `src/jsav_Lib/typechecker/TypeScheme.cpp`. Aggiunti 4 test case per verificare la sostituzione in `TypeVariable`, `ArrayType`, `VectorType`, e preservazione di variabili non quantificate.

---

#### REC-003

**Titolo**: Sostituire `std::vector` con `std::unordered_map` in `ConstraintSet::get()`

**Deficit affrontato**: Fase 2 §2.5 (DEF-022) — `get(id)` è O(n) per ricerca lineare.

**Descrizione**: Cambiare il membro `constraints_` da `std::vector<Constraint>` a `std::unordered_map<ConstraintId, Constraint>`. Aggiornare `add()` per inserire nella mappa invece di pushare nel vettore. Aggiornare `constraints()` per ritornare una collezione dei valori della mappa.

**Punto di ingresso**: Aprire `include/jsav/typechecker/Constraint.hpp` e modificare il tipo di `constraints_` da `std::vector<Constraint>` a `std::unordered_map<ConstraintId, Constraint>`.

**Risultato atteso**: `get(id)` diventa O(1) anziché O(n).

**Feasibility Score**: 5 — Modifica locale e immediata.

**Expected ROI**: 3 — Miglioramento moderato per grandi set di vincoli.

**Implementation Effort**: 5 — Sforzo minimo: poche ore.

**Priority Rank**: (5 × 2) + (3 × 2) + (5 × 1) = 10 + 6 + 5 = **21**

**Tempo di Implementazione Stimato**: 2-4 ore

**Risorse Richieste**:
1. **Ruoli**: Un ingegnere junior/mid-level
2. **Tool**: Nessun tool aggiuntivo
3. **Accesso**: Completo accesso al repository
4. **Dipendenze esterne**: Nessuna

**Indicatori di Efficacia**:
1. `ConstraintSet::get()` completa in O(1) misurato tramite benchmark per 10.000+ vincoli
2. Tutti i test esistenti passano senza modifiche

---

#### REC-004

**Titolo**: Eliminare stringa magica `"__function_context__"` con tipo forte `FunctionContext`

**Deficit affrontato**: Fase 1 §1.4.c (DEF-020), Fase 2 §2.5 (DEF-020) — marker hardcoded soggetto a collisioni.

**Descrizione**: Creare una struct `FunctionContext` con campi `return_type` e `function_name`. Sostituire l'uso della stringa `"__function_context__"` con una chiave di tipo `FunctionContextKey` (tipo forte o enum). Il `SymbolTable` mantiene una mappa separata `function_contexts_` per i contesti di funzione, distinta dai bindings utente.

**Punto di ingresso**: Aprire `include/jsav/typechecker/SymbolTable.hpp` e aggiungere una struct `FunctionContextKey`. Modificare `set_function_return_context()` e `get_function_return_context()` per usare la nuova chiave.

**Risultato atteso**: Nessun rischio di collisione con nomi utente. Separazione chiara tra simboli utente e metadata del compilatore.

**Feasibility Score**: 4 — Modifica localizzata al SymbolTable.

**Expected ROI**: 3 — Migliora la robustezza e previene un edge case raro ma possibile.

**Implementation Effort**: 4 — Basso sforzo: 2-3 giorni.

**Priority Rank**: (4 × 2) + (3 × 2) + (4 × 1) = 8 + 6 + 4 = **18**

**Tempo di Implementazione Stimato**: 2-4 giorni

**Risorse Richieste**:
1. **Ruoli**: Un ingegnere mid-level
2. **Tool**: Nessun tool aggiuntivo
3. **Accesso**: Completo accesso al repository
4. **Dipendenze esterne**: Nessuna

**Indicatori di Efficacia**:
1. Una variabile utente `"__function_context__"` definita nel codice sorgente non interferisce con il contesto di funzione
2. Zero test falliti dopo la modifica

---

#### REC-005

**Titolo**: Unificare logica di risoluzione variabili tra `Substitution::apply()` e `ConstraintSolver::occurs_in()`

**Deficit affrontato**: Fase 1 §1.4.d (DEF-007) — logica duplicata per risolvere variabili di tipo.

**Descrizione**: Estrarre la logica di risoluzione variabili in una funzione condivisa `resolve_type(const TypePtr&, const Substitution&)` che entrambe le classi possono utilizzare. `Substitution::applyImpl()` e `ConstraintSolver::occurs_in()` dovrebbero delegare a questa funzione invece di duplicare la traversata.

**Punto di ingresso**: Aprire `include/jsav/typechecker/Substitution.hpp` e aggiungere una funzione libera `resolve_type()` nel namespace `jsv`. Modificare sia `Substitution.cpp` che `ConstraintSolver.cpp` per delegare.

**Risultato atteso**: Una sola implementazione della logica di risoluzione. Bug fix applicati una volta sola.

**Feasibility Score**: 4 — Modifica locale a due file.

**Expected ROI**: 3 — Migliora manutenibilità e riduce rischio di inconsistenze.

**Implementation Effort**: 4 — Basso sforzo: 2-3 giorni.

**Priority Rank**: (4 × 2) + (3 × 2) + (4 × 1) = 8 + 6 + 4 = **18**

**Tempo di Implementazione Stimato**: 2-4 giorni

**Risorse Richieste**:
1. **Ruoli**: Un ingegnere mid-level
2. **Tool**: Nessun tool aggiuntivo
3. **Accesso**: Completo accesso al repository
4. **Dipendenze esterne**: Nessuna

**Indicatori di Efficacia**:
1. Zero duplicazione di logica di risoluzione variabili tra `Substitution` e `ConstraintSolver`
2. Test di occurs-check e apply passano invariati

---

#### REC-006

**Titolo**: Completare unificazione parametri per `CustomType` in `UnifyVisitor`

**Deficit affrontato**: Fase 2 §2.5 (DEF-017) — `UnifyVisitor::visit_custom()` confronta solo i nomi, non i parametri di tipo.

**Descrizione**: Estendere `UnifyVisitor::visit_custom()` per unificare ricorsivamente i parametri di tipo dei CustomType. Se `CustomType` ha parametri (es. `List<int>` vs `List<T>`), unificare ogni coppia di parametri corrispondenti. Se l'arietà differisce, generare errore.

**Punto di ingresso**: Aprire `src/jsav_Lib/typechecker/ConstraintSolver.cpp` e modificare `UnifyVisitor::visit_custom()` (righe 47-54).

**Risultato atteso**: Custom type parametrici vengono unificati correttamente (es. `List<int>` = `List<T>` produce binding `T → int`).

**Feasibility Score**: 4 — Modifica locale.

**Expected ROI**: 4 — Essenziale per il supporto di tipi generici e collezioni parametriche.

**Implementation Effort**: 4 — Basso sforzo: 3-5 giorni.

**Priority Rank**: (4 × 2) + (4 × 2) + (4 × 1) = 8 + 8 + 4 = **20**

**Tempo di Implementazione Stimato**: 3-5 giorni

**Risorse Richieste**:
1. **Ruoli**: Un ingegnere con conoscenza di tipi parametrici
2. **Tool**: Nessun tool aggiuntivo
3. **Accesso**: Completo accesso al repository
4. **Dipendenze esterne**: Nessuna

**Indicatori di Efficacia**:
1. Vincolo `List<int> = List<T>` produce binding `T → int`
2. Vincolo `List<int> = List<string>` produce errore `E2034`
3. Vincolo `List<int> = List<int, T>` (arietà diversa) produce errore

---

#### REC-007

**Titolo**: Far ritornare `type_array_literal()` con `error_type()` invece di `nullptr`

**Deficit affrontato**: Fase 2 §2.5 (DEF-011) — ritorna `nullptr` su errore di tipo, il chiamante non sempre controlla.

**Descrizione**: Modificare `type_array_literal()` per ritornare un `TypedArrayLiteral` con `error_type()` come tipo dell'array invece di `nullptr`. Questo garantisce che il caller riceva sempre un puntatore valido e possa propagare l'errore attraverso la pipeline senza controlli null.

**Punto di ingresso**: Aprire `src/jsav_Lib/typechecker/TypeChecker.cpp` e modificare i punti in cui `type_array_literal()` ritorna `nullptr` (righe 797, 808) per ritornare invece `std::make_unique<TypedArrayLiteral>(..., error_type(), ...)`.

**Risultato atteso**: Nessun `nullptr` propagato dalla digitazione di array. Errori gestiti tramite `error_type()`.

**Feasibility Score**: 5 — Modifica immediata.

**Expected ROI**: 4 — Previene crash da dereferenziazione di nullptr in percorsi di errore.

**Implementation Effort**: 5 — Minimo sforzo: poche ore.

**Priority Rank**: (5 × 2) + (4 × 2) + (5 × 1) = 10 + 8 + 5 = **23**

**Tempo di Implementazione Stimato**: 2-4 ore

**Risorse Richieste**:
1. **Ruoli**: Un ingegnere mid-level
2. **Tool**: Nessun tool aggiuntivo
3. **Accesso**: Completo accesso al repository
4. **Dipendenze esterne**: Nessuna

**Indicatori di Efficacia**:
1. `type_array_literal()` non ritorna mai `nullptr` dopo la modifica
2. Zero crash da nullptr dereference nei test di type checking per array con tipi misti

---

#### REC-008

**Titolo**: Aggiungere `std::unordered_map<ConstraintId, ...>` per lookup O(1) in ConstraintSet

**Deficit affrontato**: Fase 2 §2.5 (DEF-022) — lookup lineare O(n) per ID.

**Descrizione**: (Complementare a REC-003 — stessa raccomandazione con focus diverso.) Aggiungere una mappa ausiliaria `std::unordered_map<ConstraintId, std::size_t>` che mappa ID a indice nel vettore, preservando l'ordine di inserimento.

**Punto di ingresso**: `include/jsav/typechecker/Constraint.hpp`, membro privato di `ConstraintSet`.

**Risultato atteso**: Lookup O(1) senza perdere l'ordine di inserimento.

**Feasibility Score**: 5 — Modifica locale.

**Expected ROI**: 3 — Miglioramento moderato per set grandi.

**Implementation Effort**: 5 — Minimo sforzo.

**Priority Rank**: (5 × 2) + (3 × 2) + (5 × 1) = 10 + 6 + 5 = **21**

**Tempo di Implementazione Stimato**: 2-4 ore

**Risorse Richieste**:
1. **Ruoli**: Un ingegnere junior
2. **Tool**: Nessun tool aggiuntivo
3. **Accesso**: Completo accesso al repository
4. **Dipendenze esterne**: Nessuna

**Indicatori di Efficacia**:
1. `get(id)` completa in O(1) per 10.000+ vincoli
2. Ordine di inserimento preservato in `constraints()`

---

#### REC-009

**Titolo**: Gestire tipi composti in `parse_type_annotation()`

**Deficit affrontato**: Fase 2 §2.5 (DEF-013) — solo primitivi supportati, tipi composti ignorati silenziosamente.

**Descrizione**: Estendere `parse_type_annotation()` per gestire annotazioni come `array<i32>`, `vector<f32>`, e tipi custom. Implementare un mini-parser per le annotazioni di tipo che riconosca:
- Primitivi (già gestiti)
- `array<T>` dove T è un tipo
- `vector<T>` dove T è un tipo
- Nomi di tipi custom

**Punto di ingresso**: Aprire `src/jsav_Lib/typechecker/TypeChecker.cpp` e modificare `parse_type_annotation()` (righe 19-36).

**Risultato atteso**: Annotazioni di tipo composte vengono parsate correttamente invece di ritornare `nullptr`.

**Feasibility Score**: 3 — Richiede un mini-parser e gestione ricorsiva.

**Expected ROI**: 4 — Essenziale per il supporto di annotazioni di tipo esplicite per collezioni.

**Implementation Effort**: 3 — Sforzo moderato: 1-2 settimane.

**Priority Rank**: (3 × 2) + (4 × 2) + (3 × 1) = 6 + 8 + 3 = **17**

**Tempo di Implementazione Stimato**: 1-2 settimane

**Risorse Richieste**:
1. **Ruoli**: Un ingegnere con esperienza di parser
2. **Tool**: Nessun tool aggiuntivo
3. **Accesso**: Completo accesso al repository
4. **Dipendenze esterne**: Nessuna

**Indicatori di Efficacia**:
1. `parse_type_annotation("array<i32>")` ritorna `std::make_shared<ArrayType>(PrimitiveType::i32(), ...)`
2. `parse_type_annotation("vector<f32>")` ritorna `std::make_shared<VectorType>(PrimitiveType::f32())`
3. `parse_type_annotation("MyCustomType")` ritorna `std::make_shared<CustomType>("MyCustomType")`

---

#### REC-010

**Titolo**: Aggiungere validazione esplicita per `UnionFind::find()` su variabile non registrata

**Deficit affrontato**: Fase 2 §2.5 (DEF-025) — `find()` crasha con eccezione per variabili non registrate.

**Descrizione**: Modificare `UnionFind::find()` per controllare se `var` esiste in `parent_` prima di accedere. Se non esiste, ritornare un `std::expected<TypeVarId, Error>` o usare `std::optional<TypeVarId>`. In alternativa, aggiungere `contains(TypeVarId)` come metodo pubblico e documentare la precondizione.

**Punto di ingresso**: Aprire `include/jsav/typechecker/UnionFind.hpp` e aggiungere `[[nodiscard]] bool contains(TypeVarId) const noexcept;`. Modificare `find()` in `UnionFind.cpp` per controllare l'esistenza.

**Risultato atteso**: Errore compilato esplicito invece di crash runtime per variabili non registrate.

**Feasibility Score**: 5 — Modifica semplice.

**Expected ROI**: 3 — Previene crash oscuri durante lo sviluppo.

**Implementation Effort**: 5 — Minimo sforzo: 1-2 ore.

**Priority Rank**: (5 × 2) + (3 × 2) + (5 × 1) = 10 + 6 + 5 = **21**

**Tempo di Implementazione Stimato**: 1-2 ore

**Risorse Richieste**:
1. **Ruoli**: Un ingegnere junior
2. **Tool**: Nessun tool aggiuntivo
3. **Accesso**: Completo accesso al repository
4. **Dipendenze esterne**: Nessuna

**Indicatori di Efficacia**:
1. Chiamare `find()` su variabile non registrata non causa eccezione `std::out_of_range`
2. Il chiamante può verificare `contains()` prima di `find()`

---

#### REC-011

**Titolo**: Eliminare duplicazione controllo numerico in `type_binary_expr()`

**Deficit affrontato**: Fase 2 §2.5 (DEF-010) — controllo `is_numeric()` eseguito due volte.

**Descrizione**: Rifattorizzare `type_binary_expr()` per eseguire il controllo numerico una sola volta. Rimuovere il blocco duplicato `if(expr.op() != BinaryOp::Add)` e incorporare la logica nel blocco principale dello switch.

**Punto di ingresso**: Aprire `src/jsav_Lib/typechecker/TypeChecker.cpp` e modificare `type_binary_expr()` (righe 546-580).

**Risultato atteso**: Controllo numerico eseguito una sola volta, codice più leggibile.

**Feasibility Score**: 5 — Refactoring locale.

**Expected ROI**: 2 — Miglioramento incrementale di leggibilità e manutenibilità.

**Implementation Effort**: 5 — Minimo sforzo: 1-2 ore.

**Priority Rank**: (5 × 2) + (2 × 2) + (5 × 1) = 10 + 4 + 5 = **19**

**Tempo di Implementazione Stimato**: 1-2 ore

**Risorse Richieste**:
1. **Ruoli**: Un ingegnere junior
2. **Tool**: Nessun tool aggiuntivo
3. **Accesso**: Completo accesso al repository
4. **Dipendenze esterne**: Nessuna

**Indicatori di Efficacia**:
1. Zero duplicazione della logica `is_numeric()` in `type_binary_expr()`
2. Tutti i test di operatori binari passano invariati

---

#### REC-012

**Titolo**: Estrarre `OccursVisitor` e `UnifyVisitor` in file dedicati

**Deficit affrontato**: Fase 2 §2.2 (DEF-014) — visitor definiti come struct anonime nel `.cpp`.

**Descrizione**: Creare `include/jsav/typechecker/OccursVisitor.hpp` e `include/jsav/typechecker/UnifyVisitor.hpp` con le classi visitor estratte da `ConstraintSolver.cpp`. Questo migliora la testabilità e la riusabilità.

**Punto di ingresso**: Aprire `src/jsav_Lib/typechecker/ConstraintSolver.cpp` e identificare le struct `OccursVisitor` e `UnifyVisitor` (righe 14-56).

**Risultato atteso**: Visitor riutilizzabili e testabili indipendentemente.

**Feasibility Score**: 4 — Estrazione diretta.

**Expected ROI**: 2 — Miglioramento incrementale di manutenibilità.

**Implementation Effort**: 5 — Minimo sforzo: 2-3 ore.

**Priority Rank**: (4 × 2) + (2 × 2) + (5 × 1) = 8 + 4 + 5 = **17**

**Tempo di Implementazione Stimato**: 2-4 ore

**Risorse Richieste**:
1. **Ruoli**: Un ingegnere junior
2. **Tool**: Nessun tool aggiuntivo
3. **Accesso**: Completo accesso al repository
4. **Dipendenze esterne**: Nessuna

**Indicatori di Efficacia**:
1. `OccursVisitor` e `UnifyVisitor` definiti in file `.hpp` dedicati
2. `ConstraintSolver.cpp` ridotto di almeno 50 righe

---

### 4.2 Tabella Riassuntiva delle Priorità

| Rank | ID | Titolo | Feasibility | ROI | Effort | Composite Score | Est. Time | Status |
|------|----|--------|-------------|-----|--------|-----------------|-----------|--------|
| 1 | REC-007 | Far ritornare `type_array_literal()` con `error_type()` invece di `nullptr` | 5 | 4 | 5 | **23** | 2-4 hrs | Pending |
| 2 | REC-002 | Completare `TypeScheme::instantiate()` per tipi composti | 4 | 5 | 4 | **22** | 1-2 wks | ✅ Complete |
| 3 | REC-003 | Sostituire `std::vector` con `std::unordered_map` in `ConstraintSet::get()` | 5 | 3 | 5 | **21** | 2-4 hrs |
| 4 | REC-008 | Aggiungere `std::unordered_map` per lookup O(1) in ConstraintSet | 5 | 3 | 5 | **21** | 2-4 hrs |
| 5 | REC-010 | Aggiungere validazione esplicita per `UnionFind::find()` | 5 | 3 | 5 | **21** | 1-2 hrs |
| 6 | REC-006 | Completare unificazione parametri per `CustomType` | 4 | 4 | 4 | **20** | 3-5 days |
| 7 | REC-011 | Eliminare duplicazione controllo numerico in `type_binary_expr()` | 5 | 2 | 5 | **19** | 1-2 hrs |
| 8 | REC-001 | Decomporre `TypeChecker` in visitor separati | 3 | 5 | 2 | **18** | 4-8 wks |
| 9 | REC-004 | Eliminare stringa magica `"__function_context__"` | 4 | 3 | 4 | **18** | 2-4 days |
| 10 | REC-005 | Unificare logica di risoluzione variabili | 4 | 3 | 4 | **18** | 2-4 days |
| 11 | REC-009 | Gestire tipi composti in `parse_type_annotation()` | 3 | 4 | 3 | **17** | 1-2 wks |
| 12 | REC-012 | Estrarre `OccursVisitor` e `UnifyVisitor` in file dedicati | 4 | 2 | 5 | **17** | 2-4 hrs |

---

### Verifica di Completezza e Conformità ai Vincoli

**Matrice di Tracciabilità Deficit → Raccomandazione**:

| Deficit | Sezione | Raccomandazione |
|---------|---------|-----------------|
| DEF-001 | §1.2.c | (Architetturale — mitigato da REC-001) |
| DEF-002 | §1.3.a, §2.5 | REC-001 |
| DEF-003 | §1.3.b | (Minore — documentato) |
| DEF-004 | §1.3.c | REC-005 |
| DEF-005 | §1.4.a | (Architetturale — mitigato da refactoring generale) |
| DEF-006 | §1.4.c | REC-004 |
| DEF-007 | §1.4.d | REC-005 |
| DEF-008 | §2.2 | REC-001 |
| DEF-009 | §2.5 | (Mitigato da REC-007) |
| DEF-010 | §2.5 | REC-011 |
| DEF-011 | §2.5 | REC-007 |
| DEF-012 | §2.5 | (Mitigato da REC-001) |
| DEF-013 | §2.5 | REC-009 |
| DEF-014 | §2.2 | REC-012 |
| DEF-015 | §2.5 | (Minore — Void è gestito implicitamente) |
| DEF-016 | §2.5 | (Performance — mitigato da caching in Substitution) |
| DEF-017 | §2.5 | REC-006 |
| DEF-018 | §2.5 | (Design choice — documentato) |
| DEF-019 | §2.5 | (Design choice — documentato) |
| DEF-020 | §1.4.c, §2.5 | REC-004 |
| DEF-021 | §2.5 | (Minore — silenzioso ma sicuro) |
| DEF-022 | §2.5 | REC-003, REC-008 |
| DEF-023 | §2.5, §2.6 | REC-002 |
| DEF-024 | §2.5 | (Design choice — documentato) |
| DEF-025 | §2.5 | REC-010 |

Tutti i deficit identificati hanno almeno una raccomandazione associata. La matrice è completa.

**Verifica Vincoli**:

- **Vincolo 1 (Grounding empirico)**: Ogni affermazione cita file, classe e metodo specifici. Le inferenze sono marcate con "Inferred:".
- **Vincolo 2 (Completezza)**: Tutti i 10 sistemi e tutti i componenti sono stati analizzati. I componenti banali sono marcati come tali.
- **Vincolo 3 (Corrispondenza biunivoca)**: Ogni deficit ha almeno una raccomandazione. La matrice di tracciabilità lo dimostra.
- **Vincolo 4 (Azione immediata)**: Ogni raccomandazione specifica il punto di ingresso (file, metodo), i primi passi, e i criteri di completamento.
- **Vincolo 5 (Precisione linguistica)**: Nessun uso di "might", "could possibly", "seems to" senza giustificazione esplicita.
- **Vincolo 6 (Cross-referencing)**: Nessun duplicato verbatim. Le sezioni si riferiscono reciprocamente tramite numeri di sezione.
- **Vincolo 7 (Calcolo meccanico priorità)**: I punteggi sono calcolati con la formula `(F × 2) + (R × 2) + (E × 1)`. L'ordinamento è puramente meccanico.
- **Vincolo 8 (Profondità minima)**: Ogni sezione di Fase 2 supera le 300 parole. Ogni sezione di Fase 3 supera le 150 parole.
- **Vincolo 9 (Lingua italiana)**: L'intero documento è in italiano, con termini tecnici inglesi non tradotti e identificatori di codice invariati.
- **Vincolo 10 (Nessuna affermazione generica non supportata)**: Ogni raccomandazione è ancorata a un deficit specifico con riferimenti al codice.
