# Audit di Implementazione del Type Checker

## Fase 1 — Analisi dell'Insieme dei Sistemi

### 1.1 Enumerazione dei Sistemi

#### Sistema 1: **TypeChecker** — Orchestrazione dell'Inferenza di Tipo

**a) Nome del sistema**: `TypeChecker` (`include/jsav/typechecker/TypeChecker.hpp`)

**b) Responsabilità primaria**: Implementa una pipeline di inferenza di tipo vincolata in stile Hindley-Milner
articolata in 4 fasi:

- **Fase 1 — Risoluzione dei nomi**: Attraversa l'AST non tipizzato e popola la `SymbolTable` mappando identificatori a
  `TypeScheme`.
- **Fase 2 — Generazione dei vincoli**: Attraversa l'AST tipizzato parzialmente, genera vincoli di uguaglianza tra
  tipi (`ConstraintSet`) e produce nodi AST tipizzati intermedi.
- **Fase 3 — Risoluzione dei vincoli**: Delega al `ConstraintSolver` la risoluzione tramite unificazione union-find,
  producendo una `Substitution`.
- **Fase 4 — Zonking**: Applica la sostituzione all'AST tipizzato intermedio per produrre il `TypedProgram`
  completamente risolto.

**c) Ruolo rispetto agli altri sistemi**: Sistema **core** che consuma l'output del Parser (`Program`) e produce
`TypedProgram` per le fasi downstream (IR Generation, Code Generation). Coordina `SymbolTable`, `ConstraintSet`,
`ConstraintSolver` e `Substitution`. Tutti gli altri sistemi del type checker dipendono da esso o sono da esso
orchestrati.

---

#### Sistema 2: **ConstraintSet** — Accumulo dei Vincoli di Tipo

**a) Nome del sistema**: `ConstraintSet` (`include/jsav/typechecker/Constraint.hpp`)

**b) Responsabilità primaria**: Accumula vincoli di uguaglianza tra tipi (`lhs = rhs`) generati durante
l'attraversamento dell'AST. Assegna ID univoci a ogni vincolo e conserva metadati diagnostici (`SourceSpan`, `reason`).

**c) Ruolo**: Sistema **ausiliario** che funge da interfaccia tra il `TypeChecker` (produttore di vincoli) e il
`ConstraintSolver` (consumatore). È un semplice accumulatore senza logica computazionale propria.

---

#### Sistema 3: **ConstraintSolver** — Risoluzione dei Vincoli tramite Unificazione

**a) Nome del sistema**: `ConstraintSolver` (`include/jsav/typechecker/ConstraintSolver.hpp`)

**b) Responsabilità primaria**: Risolve un insieme di vincoli di tipo producendo una `Substitution` che mappa variabili
di tipo a tipi risolti. Implementa un algoritmo di unificazione strutturale con occurs-check per il rilevamento di tipi
infiniti.

**c) Ruolo**: Sistema **core** consumato dal `TypeChecker` nella Fase 3. Produce la `Substitution` che il `TypeChecker`
applica durante lo zonking. Dipende da `UnionFind`, `Substitution`, `TypeVisitor`.

---

#### Sistema 4: **SymbolTable** — Gestione dell'Ambito Lessicale

**a) Nome del sistema**: `SymbolTable` (`include/jsav/typechecker/SymbolTable.hpp`)

**b) Responsabilità primaria**: Gestisce mappature identificatore→`TypeScheme` con supporto per ambiti lessicali
nidificati, shadowing e tracciamento del contesto di ritorno delle funzioni.

**c) Ruolo**: Sistema **ausiliario di supporto trasversale** consultato dal `TypeChecker` durante la risoluzione dei
nomi (Fase 1) e la generazione dei vincoli (Fase 2). Fornisce servizi di lookup simbolico a tutti i sistemi che
necessitano di risolvere identificatori.

---

#### Sistema 5: **TypeScheme** — Schema di Tipo Polimorfico

**a) Nome del sistema**: `TypeScheme` (`include/jsav/typechecker/TypeScheme.hpp`)

**b) Responsabilità primaria**: Rappresenta tipi polimorfici nella forma `∀(vars). body` per l'inferenza Hindley-Milner.
Fornisce l'operazione di `instantiate()` per generare tipi freschi da schemi quantificati.

**c) Ruolo**: Sistema **rappresentazionale** consumato da `SymbolTable` (come valore delle mappature) e dal
`TypeChecker` (durante il lookup degli identificatori).

---

#### Sistema 6: **TypeVariable** — Variabili di Tipo

**a) Nome del sistema**: `TypeVariable` (`include/jsav/typechecker/TypeVariable.hpp`)

**b) Responsabilità primaria**: Rappresenta tipi sconosciuti (`?T1`, `?T2`, ...) che saranno risolti durante la
risoluzione dei vincoli. Fornisce una fabbrica `fresh_type_variable()` con contatore thread-local.

**c) Ruolo**: Sistema **rappresentazionale** fondamentale per l'inferenza. Usato da `TypeChecker`, `ConstraintSolver`,
`Substitution`, `TypeScheme`.

---

#### Sistema 7: **Substitution** — Mapping di Sostituzione

**a) Nome del sistema**: `Substitution` (`include/jsav/typechecker/Substitution.hpp`)

**b) Responsabilità primaria**: Mappa variabili di tipo a tipi risolti. Fornisce `apply()` per applicare la sostituzione
ricorsivamente a un `TypePtr`, con cache persistente per ottimizzare le applicazioni ripetute.

**c) Ruolo**: Sistema **di trasformazione** prodotto dal `ConstraintSolver` e consumato dal `TypeChecker` durante lo
zonking (Fase 4).

---

#### Sistema 8: **UnionFind** — Struttura Disjoint-Set

**a) Nome del sistema**: `UnionFind` (`include/jsav/typechecker/UnionFind.hpp`)

**b) Responsabilità primaria**: Implementa disjoint-set con compressione del cammino e unione per rango per operazioni
di unificazione in tempo O(α(n)).

**c) Ruolo**: Sistema **ausiliario** usato internamente dal `ConstraintSolver` per tracciare le classi di equivalenza
delle variabili di tipo.

---

#### Sistema 9: **ErrorType** — Tipo di Errore Singleton

**a) Nome del sistema**: `ErrorType` (`include/jsav/typechecker/ErrorType.hpp`)

**b) Responsabilità primaria**: Singleton che si unifica silenziosamente con qualsiasi tipo, prevenendo errori a cascata
dopo un errore di tipo rilevato.

**c) Ruolo**: Sistema **di supporto trasversale** usato dal `TypeChecker` per il recupero dagli errori e dal
`ConstraintSolver` per gestire l'unificazione con tipi errati.

---

#### Sistema 10: **TypeVisitor** — Visitatore per Tipi Composti

**a) Nome del sistema**: `TypeVisitor` (`include/jsav/typechecker/TypeVisitor.hpp`)

**b) Responsabilità primaria**: Fornisce dispatch strutturale per ricorsione su tipi composti (`ArrayType`,
`VectorType`) senza duplicare la logica switch-on-TypeKind.

**c) Ruolo**: Sistema **di supporto** usato da `Substitution`, `ConstraintSolver` e `TypeScheme` per attraversare tipi
composti.

---

#### Sistema 11: **Type** — Rappresentazione dei Tipi

**a) Nome del sistema**: `TypeBase` e derivati (`include/jsav/ast/Type.hpp`)

**b) Responsabilità primaria**: Definisce il sistema dei tipi completo con discriminante `TypeKind` (20 varianti), tipi
primitivi singleton, tipi custom, array e vector.

**c) Ruolo**: Sistema **fondamentale di rappresentazione** su cui dipende l'intero type checker. Consumato da tutti gli
altri sistemi.

---

#### Sistema 12: **CompileError** — Infrastruttura Diagnostica

**a) Nome del sistema**: `CompileError` (`include/jsav/error/CompileError.hpp`)

**b) Responsabilità primaria**: Rappresentazione unificata degli errori di compilazione con span di origine, codici
errore e testo di aiuto.

**c) Ruolo**: Sistema **trasversale** consumato dal `TypeChecker`, `ConstraintSolver`, `Parser`, `Lexer`.

---

#### Sistema 13: **AST Non Tipizzato e Tipizzato** — Rappresentazione del Programma

**a) Nome del sistema**: `Node`, `Expr`, `Stmt`, `TypedNode`, `TypedExpr`, `TypedStmt` e derivati (`include/jsav/ast/`)

**b) Responsabilità primaria**: Definisce le strutture dati dell'AST grezzo (output del parser) e dell'AST tipizzato (
output del type checker).

**c) Ruolo**: Sistema **rappresentazionale** — input e output del type checker.

---

### 1.2 Mappa delle Dipendenze Inter-Sistema

#### a) Diagramma ASCII

```text
                        ┌──────────────────┐
                        │     Parser       │  (upstream — produce Program)
                        └────────┬─────────┘
                                 │
                                 ▼
                        ┌──────────────────┐
                        │   TypeChecker    │  (midstream — orchestratore)
                        └──┬───┬───┬───┬───┘
                           │   │   │   │
              ┌────────────┘   │   │   └───────────────┐
              ▼                ▼   ▼                   ▼
     ┌────────────┐   ┌─────────────┐          ┌─────────────┐
     │ SymbolTable│   │ConstraintSet│          │Substitution │
     └────────────┘   └──────┬──────┘          └──────▲──────┘
                             │                        │
                             ▼                        │
                    ┌────────────────┐                │
                    │ConstraintSolver│────────────────┘
                    └───────┬────────┘
                            │
               ┌────────────┼────────────┐
               ▼            ▼            ▼
        ┌──────────┐ ┌───────────┐ ┌────────────┐
        │ UnionFind│ │TypeVisitor│ │ ErrorType  │
        └──────────┘ └───────────┘ └────────────┘
               ▲            ▲            ▲
               │            │            │
        ┌──────┴────────────┴────────────┴───────┐
        │          TypeVariable / TypeScheme     │
        └──────────────────┬─────────────────────┘
                           │
                           ▼
                    ┌───────────────┐
                    │   TypeBase    │  (fondamentale — tutti dipendono)
                    │ (ast/Type.hpp)│
                    └───────────────┘
                           ▲
                           │
              ┌────────────┴────────────┐
              ▼                         ▼
     ┌──────────────────┐        ┌─────────────────┐
     │ AST Non Tipizzato│        │ AST Tipizzato   │
     │ (Expr, Stmt)     │        │ (TypedExpr,     │
     └──────────────────┘        │  TypedStmt)     │
                                 └─────────────────┘
                                      │
                                      ▼
                               ┌───────────────┐
                               │ CompileError  │  (trasversale)
                               └───────────────┘
```

#### b) Classificazione Upstream/Downstream

| Sistema                       | Posizione    | Produttori                        | Consumatori                                  |
|-------------------------------|--------------|-----------------------------------|----------------------------------------------|
| `Type` (`TypeBase`)           | Fondamentale | —                                 | Tutti                                        |
| `AST Non Tipizzato`           | Upstream     | Parser                            | `TypeChecker`                                |
| `SymbolTable`                 | Midstream    | `TypeChecker` (Fase 1)            | `TypeChecker` (Fase 2)                       |
| `ConstraintSet`               | Midstream    | `TypeChecker` (Fase 2)            | `ConstraintSolver`                           |
| `TypeVariable` / `TypeScheme` | Fondamentale | —                                 | `TypeChecker`, `SymbolTable`, `Substitution` |
| `ConstraintSolver`            | Midstream    | `ConstraintSet`                   | `TypeChecker` (Fase 3)                       |
| `UnionFind`                   | Ausiliario   | —                                 | `ConstraintSolver`                           |
| `TypeVisitor`                 | Ausiliario   | —                                 | `Substitution`, `ConstraintSolver`           |
| `Substitution`                | Downstream   | `ConstraintSolver`                | `TypeChecker` (Fase 4)                       |
| `ErrorType`                   | Trasversale  | —                                 | `TypeChecker`, `ConstraintSolver`            |
| `AST Tipizzato`               | Downstream   | `TypeChecker`                     | Fasi successive (IR Gen)                     |
| `CompileError`                | Trasversale  | `TypeChecker`, `ConstraintSolver` | `ErrorReporter`                              |

#### c) Nodi Critici

- **Alto fan-in** (`TypeBase`, `TypeChecker`): `TypeBase` è il fondamento su cui poggiano tutti gli altri sistemi.
  `TypeChecker` è l'orchestratore centrale — un singolo punto di elaborazione per l'intera pipeline.
- **Alto fan-out** (`TypeChecker`): Dipende da `SymbolTable`, `ConstraintSet`, `ConstraintSolver`, `Substitution`,
  `ErrorType`, `TypeVariable`, e dall'intero AST. Questo accoppiamento elevato è intrinseco al ruolo di orchestrazione
  ma rappresenta un potenziale collo di bottiglia per l'estensibilità.

---

### 1.3 Valutazione della Coerenza Architetturale

#### a) Separazione delle Responsabilità

L'architettura rispetta sostanzialmente il **Single Responsibility Principle** a livello di sistema. Ogni sistema ha un
dominio di responsabilità ben delimitato:

- `SymbolTable` gestisce esclusivamente gli ambiti e il binding dei simboli.
- `ConstraintSet` accumula vincoli senza logica di risoluzione.
- `ConstraintSolver` risolve vincoli senza conoscere l'AST.
- `Substitution` applica mapping senza generare vincoli.

Tuttavia, il `TypeChecker` accumula responsabilità multiple: risoluzione dei nomi, generazione dei vincoli, controllo
del flusso di loop (break/continue), e zonking. **DEF-001**: `TypeChecker::type_stmt` gestisce sia la generazione dei
vincoli che la validazione semantica (controllo break/continue, contesto di ritorno), violando la separazione tra
inferenza e validazione [`TypeChecker.cpp:type_stmt`, righe 855–1213].

#### b) Coerenza dell'Organizzazione dei Moduli

La struttura fisica dei file riflette fedelmente la decomposizione logica:

- `include/jsav/typechecker/` contiene tutte le interfacce.
- `src/jsav_Lib/typechecker/` contiene tutte le implementazioni.
- I nomi seguono convenzioni uniformi (`*Type.hpp`, `*Solver.hpp`, `*Table.hpp`).

Non esistono "god file" — il file più grande è `TypeChecker.cpp` (1213 righe), giustificato dal suo ruolo di
orchestrazione.

#### c) Pulizia delle Interfacce Inter-Sistema

Le interfacce tra sistemi sono ben tipizzate e documentate:

- `TypeChecker::check()` accetta `const Program&` e restituisce `TypeCheckResult`.
- `ConstraintSolver::solve()` accetta `const ConstraintSet&` e restituisce `SolverResult`.
- `Substitution::apply()` è pura e ben documentata.

**Tuttavia**, esistono violazioni di incapsulamento: **DEF-002**: Il `TypeChecker` accede direttamente ai campi interni
dei nodi AST tramite `static_cast` e `dynamic_cast` invece di interfacce astratte [`TypeChecker.cpp:121`,
`TypeChecker.cpp:193`]. I vincoli sono creati con accesso diretto ai campi privati dei nodi.

**Giudizio sintetico**: Architettura **parzialmente coerente**. La separazione tra sistemi è ben progettata, ma il
`TypeChecker` accumula troppe responsabilità e l'accesso diretto ai dettagli interni dell'AST crea accoppiamento
fragile.

---

### 1.4 Analisi delle Preoccupazioni Trasversali

#### a) Propagazione degli Errori

**Strategia**: Raccolta accumulativa in `std::vector<CompileError> errors_` all'interno del `TypeChecker`. Gli errori
non vengono propagati tramite monadi `Result<T, E>` ma accumulati in un contenitore mutabile.

**Coerenza**: La strategia è coerente all'interno del type checker ma diverge dal `ConstraintSolver`, che usa
`std::expected<void, CompileError>` per la propagazione immediata. **DEF-003**: Incoerenza nella strategia di
propagazione errori — il `TypeChecker` accumula (`push_back`), il `ConstraintSolver` ritorna immediatamente (
`std::expected`). Se un errore di unificazione si verifica, il solver continua a processare vincoli successivi invece di
fallire rapidamente [`ConstraintSolver.cpp::solve`, righe 44-52].

**Struttura dei messaggi**: Uniforme grazie a `ErrorCode`, `SourceSpan`, e help text. I codici errore E2001-E2036
coprono 36 scenari semantici.

**Errori silenziosi**: **DEF-004**: Il nodo `default` in `zonk_expr_full` ritorna `nullptr` senza generare errore,
causando potenziali crash downstream [`TypeChecker.cpp:397-400`].

#### b) Risoluzione dei Simboli

La `SymbolTable` è l'unica autorità per la risoluzione dei simboli. Non esistono lookup locali duplicati. L'accesso è
sempre mediato tramite `lookup()` e `define()`. Coerente e centralizzato.

**DEF-005**: La `SymbolTable` usa `std::string_view` come chiave nelle mappe degli ambiti, il che richiede che le
stringhe sorgente vivano più a lungo della tabella. Se le chiavi provengono da stringhe temporanee, si verificano
dangling reference [`SymbolTable.hpp:99`].

#### c) Gestione degli Ambiti

La gestione degli ambiti è centralizzata nella `SymbolTable` con `push_scope()`/`pop_scope()`. Lo shadowing è gestito
correttamente tramite `insert_or_assign`.

**DEF-006**: Il contesto di ritorno delle funzioni è implementato inserendo un marker sintetico `"__function_context__"`
nella tabella dei simboli invece di usare una struttura dedicata. Questo inquina lo spazio dei nomi dei simboli e crea
un rischio di collisione se un utente definisce una variabile con quel nome [`SymbolTable.cpp:39-47`].

#### d) Rappresentazione dei Tipi

Esiste un'unica gerarchia `TypeBase` condivisa. Le operazioni fondamentali (unificazione, occurs-check, sostituzione)
sono centralizzate nel `ConstraintSolver` e nella `Substitution`.

**DEF-007**: Il `TypeVisitor` gestisce solo `ArrayType` e `VectorType` ma non `CustomType`. Se il linguaggio evolve per
supportare tipi custom composti, il visitatore dovrà essere esteso senza un meccanismo di estensibilità predefinito [
`TypeVisitor.hpp:53-63`].

**DEF-008**: La funzione `TypeScheme::instantiate()` è incompleta — non sostituisce le variabili quantificate
all'interno di tipi composti, gestendo solo il caso in cui il body è una `TypeVariable` diretta [
`TypeScheme.cpp:20-35`].

---

## Fase 2 — Analisi Per-Sistema

### Sistema: TypeChecker

#### 2.1 Panoramica del Sistema

**Scopo**: Il `TypeChecker` trasforma un AST non tipizzato (`Program`) in un AST completamente tipizzato (
`TypedProgram`) tramite una pipeline a 4 fasi: risoluzione dei nomi, generazione dei vincoli, risoluzione dei vincoli e
zonking. Implementa un algoritmo di inferenza vincolata in stile Hindley-Milner con occurs-check e recupero dagli errori
tramite `ErrorType`.

**Ambito**: Copre l'intera pipeline di type checking. Non gestisce la generazione di IR né la generazione di codice —
questi sono delegati a sistemi downstream. Non esegue il parsing — riceve un AST già costruito.

**Posizione nella pipeline**: Si colloca tra il Parser (upstream) e il generatore IR (downstream). Riceve `Program` e
produce `TypedProgram`.

**Contesto di attivazione**: Istanziato una volta per unità di compilazione. Il metodo `check()` è l'entry point
principale, invocato dal driver di compilazione dopo il parsing. Lo stato interno (`SymbolTable`, `ConstraintSet`,
`errors_`) viene resettato ad ogni chiamata a `check()`.

---

#### 2.2 Organizzazione Interna dei Moduli

**Inventario file**:

- `include/jsav/typechecker/TypeChecker.hpp` — Dichiarazione della classe, struct `TypeCheckResult`, 17 metodi privati
  di typing, 3 metodi privati di zonking.
- `src/jsav_Lib/typechecker/TypeChecker.cpp` — Implementazione completa (1213 righe).

**Confini dei moduli**: Il file unico è coerente con il ruolo di orchestrazione. Tuttavia, le 17 funzioni `type_*` per
le espressioni potrebbero essere estratte in un componente `ExpressionTyper` dedicato.

**Organizzazione degli header**: L'header espone tutto il necessario e nulla di più. I metodi `type_expr()` e
`type_stmt()` sono pubblici per testing, il che è appropriato.

**Verdetto**: Logica e mantenibile, ma la dimensione (1213 righe) supera la soglia consigliata di 1000 righe.

---

#### 2.3 Analisi delle Dipendenze Intra-Sistema

Il `TypeChecker` dipende da:

- `SymbolTable` → utilizzato per lookup e definizione dei simboli.
- `ConstraintSet` → accumulatore dei vincoli.
- `ConstraintSolver` → risoluzione.
- `Substitution` → applicata durante lo zonking.
- `ErrorType` → recupero errori.
- `TypeVariable` → generazione variabili fresche.

Non esistono dipendenze circolari interne al sistema. L'accoppiamento è diretto e non mediato da interfacce astratte —
modificare l'API di uno qualsiasi di questi sistemi richiederebbe modifiche al `TypeChecker`.

---

#### 2.4 Flusso Logico

**Entry point**: `TypeChecker::check(const Program&)` [`TypeChecker.cpp:76-91`].

**Elaborazione input**:

1. Resetta lo stato interno (`symbols_`, `constraints_`, `errors_`).
2. **Fase 1**: Chiama `resolve_names(program)` che attraversa tutti gli statement, push/pop scope, e definisce simboli
   per variabili e funzioni.
3. **Fase 2**: Chiama `generate_constraints(program)` che trasforma ogni statement tramite `type_stmt()`, producendo
   `typed_stmts_` e accumulando vincoli.
4. **Fase 3**: Chiama `solve_constraints()` che istanzia un `ConstraintSolver` e risolve tutti i vincoli.
5. **Fase 4**: Chiama `zonk(subst)` che applica la sostituzione a tutti gli statement tipizzati.

**Produzione output**: Restituisce `TypeCheckResult{TypedProgram, errors}`.

**Gestione errori**: Gli errori vengono accumulati in `errors_`. Il processamento continua anche dopo errori (error
recovery tramite `ErrorType`).

---

#### 2.5 Punti Critici

1. **DEF-001** (§1.3a): `type_stmt` gestisce sia vincoli che validazione semantica.
2. **DEF-009**: La funzione `parse_type_annotation` [`TypeChecker.cpp:17-35`] usa una catena if/else invece di una
   lookup table. Ogni nuovo tipo primitivo richiede una nuova riga.
3. **DEF-010**: `zonk_type` [`TypeChecker.cpp:40-72`] usa `dynamic_cast` invece del visitor pattern, violando la
   strategia del progetto di usare `TypeVisitor` per l'attraversamento dei tipi.
4. **DEF-011**: Il gestore `default` in `type_expr` [`TypeChecker.cpp:849-854`] restituisce un `TypedIdentifier` con
   `error_type()` invece di `nullptr`, creando incoerenza con il gestore `default` di `zonk_expr_full` che restituisce
   `nullptr`.

---

#### 2.6 Implementazioni Parziali o Non Definite

Nessun metodo dichiarato in `TypeChecker.hpp` manca di implementazione. Tutti i 17 metodi `type_*` e i 3 metodi `zonk_*`
sono implementati.

---

### Sistema: ConstraintSolver

#### 2.1 Panoramica

**Scopo**: Risolve vincoli di uguaglianza tra tipi producendo una sostituzione unificata. Implementa unificazione
strutturale con occurs-check.

**Ambito**: Risolve esclusivamente vincoli `lhs = rhs`. Non genera vincoli né applica sostituzioni all'AST.

**Posizione**: Fase 3 della pipeline. Consuma `ConstraintSet`, produce `SolverResult`.

**Contesto**: Istanziato internamente dal `TypeChecker::solve_constraints()`. Stateless tra le chiamate.

---

#### 2.2 Organizzazione Interna

- `include/jsav/typechecker/ConstraintSolver.hpp` — Dichiarazione.
- `src/jsav/Lib/typechecker/ConstraintSolver.cpp` — Implementazione.

Organizzazione logica e coerente.

---

#### 2.3 Dipendenze Intra-Sistema

Dipende da `UnionFind`, `Substitution`, `TypeVisitor`, `ErrorType`. Nessuna dipendenza circolare.

---

#### 2.4 Flusso Logico

**Entry point**: `solve(const ConstraintSet&)` [`ConstraintSolver.cpp:44-56`].

Per ogni vincolo, chiama `unify(lhs, rhs, constraint)`. Se l'unificazione fallisce, accumula l'errore in
`result.errors`. Alla fine, restituisce la sostituzione accumulata.

L'unificazione (`unify`) [`ConstraintSolver.cpp:63-126`]:

1. Se un operando è `ErrorType`, l'unificazione ha successo silenziosamente.
2. Se entrambi sono `TypeVariable`, verifica l'occurs-check, poi unisce tramite `UnionFind` e vincola nella
   `Substitution`.
3. Se uno è `TypeVariable` e l'altro concreto, bind la variabile al concreto.
4. Se entrambi concreti e stesso kind, usa `UnifyVisitor` per unificare ricorsivamente.
5. Se kind diversi, restituisce errore `E2034`.

---

#### 2.5 Punti Critici

1. **DEF-003** (§1.4a): Il solver continua dopo errori invece di fallire rapidamente.
2. **DEF-012**: L'`OccursVisitor` e l'`UnifyVisitor` sono struct locali definite nel file `.cpp` [
   `ConstraintSolver.cpp:12-39`]. Non sono riutilizzabili da altri moduli.
3. **DEF-013**: L'unificazione numerica permissiva [`ConstraintSolver.cpp:101-118`] accetta `i64` = `f64` come errore di
   mismatch ma suggerisce un cast invece di fallire immediatamente. Questo comportamento è discutibile: alcuni type
   checker trattano i mismatch numerici come errori fatali.

---

#### 2.6 Implementazioni Parziali

Nessuna. Tutte le funzioni dichiarate sono implementate.

---

### Sistema: SymbolTable

#### 2.1 Panoramica

**Scopo**: Gestisce binding identificatore→`TypeScheme` con scope nidificati.

**Ambito**: Esclusivamente gestione degli ambiti e lookup. Non esegue type checking.

**Posizione**: Consultato dal `TypeChecker` nelle Fasi 1 e 2.

**Contesto**: Reset all'inizio di ogni `check()`.

---

#### 2.2 Organizzazione Interna

- `include/jsav/typechecker/SymbolTable.hpp`
- `src/jsav_Lib/typechecker/SymbolTable.cpp`

Coerente e minimale.

---

#### 2.3 Dipendenze Intra-Sistema

Dipende solo da `TypeScheme`. Nessun ciclo.

---

#### 2.4 Flusso Logico

`push_scope()` aggiunge una mappa vuota. `pop_scope()` la rimuove. `define()` inserisce nello scope corrente. `lookup()`
cerca dall'interno verso l'esterno.

---

#### 2.5 Punti Critici

1. **DEF-005** (§1.4b): Chiavi `string_view` con potenziale dangling reference.
2. **DEF-006** (§1.4c): Marker `"__function_context__"` che inquina lo spazio dei simboli.
3. **DEF-014**: `pop_scope()` non verifica che lo stack non sia vuoto — silenziosamente no-op se vuoto, nascondendo bug
   di sbilanciamento [`SymbolTable.cpp:13-15`].

---

#### 2.6 Implementazioni Parziali

Nessuna.

---

### Sistema: Substitution

#### 2.1 Panoramica

**Scopo**: Mappa variabili di tipo a tipi risolti con caching persistente.

---

#### 2.2-2.6 Sintesi

Implementazione solida con cache ben progettata. **DEF-015**: La cache non è thread-safe, ma il commento nell'header lo
documenta esplicitamente [`Substitution.hpp:77-79`]. Nessun'altra criticità significativa.

---

### Sistema: UnionFind

#### 2.1-2.6 Sintesi

Implementazione standard di disjoint-set. Corretta e ben documentata. Nota di const-correctness gestita esplicitamente
nel commento [`UnionFind.hpp:24-32`]. Nessuna criticità.

---

### Sistema: TypeScheme

#### 2.1-2.6 Sintesi

**DEF-008** (§1.4d): `instantiate()` è incompleto per tipi composti. Questo impedisce il corretto supporto per il
polimorfismo parametrico su tipi come `Vec<T>` quando `T` è una variabile quantificata [`TypeScheme.cpp:20-35`].

---

### Sistema: TypeVariable

#### 2.1-2.6 Sintesi

Implementazione minimale e corretta. Il contatore thread-local garantisce unicità. Nessuna criticità.

---

### Sistema: ErrorType

#### 2.1-2.6 Sintesi

Singleton corretto. Previene errori a cascata come progettato. Nessuna criticità.

---

### Sistema: TypeVisitor

#### 2.1-2.6 Sintesi

**DEF-007** (§1.4d): Gestisce solo `Array` e `Vector`. `CustomType` non è visitato. Estensibilità limitata — ogni nuovo
tipo composto richiede modifica del visitor.

---

### Sistema: Type (TypeBase) › Componente: TypeBase

#### 2.1 Panoramica

**Scopo**: Definisce la gerarchia dei tipi del linguaggio con discriminante `TypeKind` (20 varianti), fornendo interfacce
virtuali pure per `to_string()` e `operator==`, nonché metodi constexpr per la classificazione (`is_integer()`,
`is_numeric()`, ecc.).

**Ambito**: Esclusivamente rappresentazione dei tipi. Non esegue inferenza, unificazione o sostituzione.

**Posizione**: Fondamento dell'intera pipeline — consumato da tutti i 12 sistemi successivi.

**Contesto**: Istanziato come singleton per i primitivi, allocato dinamicamente per i tipi composti e le variabili.

---

#### 2.2 Organizzazione Interna

- `include/jsav/ast/Type.hpp` — Dichiarazione completa di `TypeBase`, `PrimitiveType`, `CustomType`, `ArrayType`,
  `VectorType`, alias `TypePtr`, e formatter `std`/`fmt`.
- `src/jsav_Lib/ast/Type.cpp` — Implementazioni di `to_string()` e helper per confronto size expression.

Organizzazione coerente. Il file header è voluminoso (629 righe) ma giustificato dalla completezza della gerarchia.

---

#### 2.3 Dipendenze Intra-Sistema

`TypeBase` → nessuna dipendenza interna. `PrimitiveType` → singleton statici. `ArrayType` → dipende da `Expr` per la size
expression (accoppiamento cross-modulo con l'AST). `CustomType` → dipende da `std::string`.

L'accoppiamento `ArrayType` → `Expr` è una violazione di layer: il sistema dei tipi dipende dal sistema AST.

---

#### 2.4 Flusso Logico

Istanza costruita → `shared_ptr<const TypeBase>` gestisce il lifetime. Confronto tramite `operator==` virtuale.
Formattazione tramite `to_string()`. Nessun side effect.

---

#### 2.5 Punti Critici

1. **DEF-016**: `TypeBase` cancella copy/move ma non fornisce `clone()` virtuale [
   `Type.hpp:223-227`]. Questo impedisce la clonazione polimorfica quando necessario (es. deep copy di un sottoalbero di
   tipo durante la trasformazione dell'AST).
2. **DEF-020**: `ArrayType` dipende da `Expr` (`std::shared_ptr<const Expr> size_expr_`) per memorizzare l'espressione
   della dimensione. Questo crea una dipendenza circolare inversa: il modulo `ast/Type.hpp` include `Node.hpp` → `Expr`
   [
   `Type.hpp:5`]. Se l'AST viene refattorizzato, il sistema dei tipi si rompe.
3. **DEF-021**: La gerarchia non ha un metodo `hash()` virtuale, rendendo impossibile usare `TypePtr` come chiave in
   `unordered_map` senza un hasher custom esterno.

---

#### 2.6 Implementazioni Parziali

Nessuna. Tutte le funzioni dichiarate sono implementate.

---

### Sistema: Type (TypeBase) › Componente: PrimitiveType

#### 3.1 Dichiarazione di Responsabilità

Il componente **`PrimitiveType`** fornisce istanze singleton immutabili per ogni tipo primitivo del linguaggio (15
varianti), garantendo che confronti per identità e uguaglianza siano O(1).

#### 3.2 Struttura delle Classi

| Campo | Tipo | Visibilità | Ruolo Semantico |
|-------|------|------------|-----------------|
| `kind_` | `TypeKind` | `private` (ereditato) | Discriminante del tipo |
| Nessun altro campo | — | — | Istanze singleton senza stato |

**Special Member Functions**:
- Costruttore: privato con `PrivateTag`, accessibile solo alle factory statiche.
- Distruttore: ereditato virtuale da `TypeBase`.
- Copy/Move: cancellati da `TypeBase`.

**Singleton Pattern**: Ogni factory (`i8()`, `i16()`, ..., `void_()`) usa `static const auto instance =
make_shared<...>()`, garantendo un'unica istanza globale per tipo primitivo. Thread-safe per la costruzione statica
locale (guarantita da C++11).

#### 3.3 Analisi dell'Interfaccia

Tutte le 15 factory sono `[[nodiscard]] static`, noexcept implicitamente (nessuna allocazione dopo la prima).
`classof(const TypeBase *)` è `constexpr` — valutabile a compile-time.

**Contratto**: Due istanze dello stesso primitivo sono sempre lo stesso oggetto (identità = uguaglianza).

**Coerenza header/implementation**: `to_string()` dichiarato nell'header, implementato nel `.cpp` con chiamata a
`type_kind_name()` — coerente.

#### 3.4 Logica di Implementazione

Zero logica computazionale. Ogni factory restituisce un singleton pre-costruito. `operator==` confronta solo `kind()` —
O(1). `to_string()` delega a `type_kind_name()` — O(1).

Complessità: O(1) per ogni operazione. Spazio: O(1) — 15 istanze singleton.

#### 3.5 Valutazione della Gestione Errori

Nessun errore possibile — i singleton non falliscono. Design corretto.

#### 3.6 Audit di Coerenza dei Tipi

Tipi coerenti. `classof()` usa enumerazione esplicita di tutti i `TypeKind` primitivi — robusto rispetto all'aggiunta
di nuovi tipi non primitivi, ma fragile se si aggiunge un nuovo primitivo e ci si dimentica di aggiornare `classof()`.

#### 3.7 Interazioni Inter-Componente

Consumato da ogni sistema del type checker. Accoppiamento ubiquitario ma inevitabile per un sistema di tipi.

#### 3.8 Opportunità di Ottimizzazione

**Performance**: Ottimale — singleton, O(1) per tutto.

**Strutturale**: `classof()` potrebbe essere generato da una `constexpr` lookup table invece di un switch esplicito,
riducendo la manutenzione.

**Manutenibilità**: Naming snake_case (`i8()`, `bool_()`) coerente con `type_kind_name()`.

---

### Sistema: Type (TypeBase) › Componente: ArrayType

#### 3.1 Dichiarazione di Responsabilità

Il componente **`ArrayType`** rappresenta tipi array a dimensione fissa `[T; N]`, memorizzando il tipo elemento e
l'espressione compile-time della dimensione.

#### 3.2 Struttura delle Classi

| Campo | Tipo | Vis. | Ruolo |
|-------|------|------|-------|
| `element_type_` | `std::shared_ptr<const TypeBase>` | `private` | Tipo degli elementi |
| `size_expr_` | `std::shared_ptr<const Expr>` | `private` | Espressione della dimensione |

**DEF-020**: La dipendenza da `Expr` è cross-layer — il modulo dei tipi non dovrebbe dipendere dall'AST non tipizzato.

**Ownership**: `shared_ptr<const T>` — condivisione immutabile, nessuna proprietà esclusiva.

#### 3.3 Analisi dell'Interfaccia

Costruttore: `ArrayType(shared_ptr<TypeBase>, shared_ptr<Expr>)` — richiede entrambi non-null (asserzioni).
`element_type()` e `size_expr()`: accessor const noexcept.
`operator==`: confronta `element_type_` per uguaglianza strutturale e `size_expr_` tramite `sizes_equal()`.

**Discrepanza**: Il costruttore accetta `shared_ptr<const TypeBase>` ma l'header dichiara il parametro come
`std::shared_ptr<const TypeBase>` — coerente.

#### 3.4 Logica di Implementazione

`sizes_equal()` [`Type.cpp:57-62`] usa `node_dyn_cast<IntegerLiteral>` per estrarre il valore numerico. Se le
espressioni non sono `IntegerLiteral`, ricade sul confronto per identità (`&a == &b`).

Questo è corretto ma incompleto: due `IntegerLiteral` con lo stesso valore ma indirizzi diversi sono confrontati
correttamente, ma due espressioni complesse equivalenti (es. `2+2` e `4`) sono considerate diverse.

#### 3.5 Gestione Errori

Asserzioni su null pointer nel costruttore. Nessun errore runtime dopo costruzione.

#### 3.6 Coerenza Tipi

`static_cast<const ArrayType *>` dopo controllo `kind()` — corretto e coerente con il pattern del progetto.

#### 3.7 Interazioni Inter-Componente

Dipende da `Expr` (cross-layer). Consumato da `TypeChecker`, `ConstraintSolver`, `Substitution`.

#### 3.8 Opportunità di Ottimizzazione

**Strutturale**: **DEF-022**: Memorizzare `std::int64_t size_value` invece di `shared_ptr<const Expr>` eliminerebbe la
dipendenza cross-layer e semplificherebbe il confronto [
`Type.hpp:420-421`]. La size expression è necessaria solo durante il parsing — dopo, il valore intero è sufficiente.

---

### Sistema: Type (TypeBase) › Componente: VectorType

#### 3.1 Dichiarazione di Responsabilità

Il componente **`VectorType`** rappresenta tipi vettore dinamico `Vec<T>`, memorizzando esclusivamente il tipo
elemento.

#### 3.2 Struttura delle Classi

| Campo | Tipo | Vis. | Ruolo |
|-------|------|------|-------|
| `element_type_` | `std::shared_ptr<const TypeBase>` | `private` | Tipo degli elementi |

Minimale e corretto.

#### 3.3-3.8 Sintesi

Implementazione corretta e ben strutturata. Nessuna criticità. `operator==` delega al confronto strutturale
dell'elemento. `classof()` controlla `TypeKind::Vector`. Costruttore con assert non-null. Nessun cross-layer dependency.

---

### Sistema: Type (TypeBase) › Componente: CustomType

#### 3.1 Dichiarazione di Responsabilità

Il componente **`CustomType`** rappresenta tipi definiti dall'utente (struct, enum, typedef) tramite un nome simbolico.

#### 3.2 Struttura delle Classi

| Campo | Tipo | Vis. | Ruolo |
|-------|------|------|-------|
| `name_` | `std::shared_ptr<const std::string>` | `private` | Nome del tipo custom |

`shared_ptr<const string>` per condivisione immutabile. `name()` restituisce `string_view` — efficiente.

#### 3.3-3.8 Sintesi

Corretto. `operator==` confronta i nomi per uguaglianza strutturale. Nessuna criticità rilevante.

**DEF-007 correlato**: Non visitato da `TypeVisitor` — se i tipi custom dovessero avere membri, servirebbe un meccanismo
di introspezione attualmente assente.

---

### Sistema: CompileError

#### 2.1-2.6 Sintesi

**DEF-017**: Il campo `message_` è `std::string_view` ma il costruttore è privato e i factory method accettano
`std::string_view`. Se la stringa sorgente viene deallocata, il view diventa dangling [`CompileError.hpp:64-67`]. Il
`TypeChecker` mitiga questo usando `message_storage_` (un `deque`) per possedere le stringhe, ma questa garanzia non è
documentata nell'interfaccia di `CompileError`.

---

## Fase 3 — Analisi Per-Componente

Dato l'elevato numero di componenti (13 sistemi × ~5 classi ciascuno = ~65 componenti), questa sezione si concentra sui
componenti critici identificati nelle Fasi 1-2.

### Sistema: TypeChecker › Componente: TypeChecker

#### 3.1 Dichiarazione di Responsabilità

Il componente **`TypeChecker`** esegue l'inferenza completa dei tipi per un programma AST non tipizzato, producendo un
AST tipizzato con tutti i tipi risolti, attraversando dichiarazioni, espressioni e statement per generare e risolvere
vincoli di uguaglianza tra tipi.

#### 3.2 Struttura delle Classi

| Campo              | Tipo                                               | Visibilità | Ruolo Semantico                                            |
|--------------------|----------------------------------------------------|------------|------------------------------------------------------------|
| `symbols_`         | `SymbolTable`                                      | `private`  | Tabella dei simboli con scope lessicale                    |
| `constraints_`     | `ConstraintSet`                                    | `private`  | Accumulatore di vincoli                                    |
| `errors_`          | `std::vector<CompileError>`                        | `private`  | Errori raccolti                                            |
| `message_storage_` | `std::deque<std::string>`                          | `private`  | Proprietario delle stringhe per `string_view` negli errori |
| `typed_stmts_`     | `std::vector<TypedStmtPtr>`                        | `private`  | Statement tipizzati intermedi                              |
| `function_decls_`  | `std::unordered_map<std::string, const FuncDecl*>` | `private`  | Lookup firme di funzione                                   |
| `loop_depth_`      | `std::size_t`                                      | `private`  | Profondità di annidamento loop                             |

Nessuna ereditarietà. Classe concreta standalone.

#### 3.3 Analisi dell'Interfaccia

**`TypeCheckResult check(const Program&)`**:

- Precondizione: `program` valido e ben formato dal parser.
- Postcondizione: Restituisce AST tipizzato + errori.
- Contratto: Resetta lo stato, esegue 4 fasi, non lancia eccezioni.

**`TypedExprPtr type_expr(const Expr&)`**:

- Precondizione: Risoluzione nomi già eseguita.
- Postcondizione: Espressione tipizzata con tipo annotato.

**`TypedStmtPtr type_stmt(const Stmt&)`**:

- Precondizione: Risoluzione nomi già eseguita.
- Postcondizione: Statement tipizzato con vincoli generati.

#### 3.4 Logica di Implementazione

L'algoritmo di `type_binary_expr` [`TypeChecker.cpp:529-639`] è il più complesso (CCN stimato >20):

1. Tipizza ricorsivamente lhs e rhs.
2. Switch sull'operatore (19 casi).
3. Per `Add`: controlla string/char, altrimenti fallthrough su arithmetic.
4. Per arithmetic: vincola lhs=rhs, controlla numeric.
5. Per comparison: vincola lhs=rhs, risultato bool.
6. Per logical: controlla boolean.
7. Per bitwise: controlla integer.

Complessità temporale: O(n) dove n è il numero di nodi AST, poiché ogni nodo è visitato una volta. La risoluzione dei
vincoli è O(m × α(m)) dove m è il numero di vincoli.

#### 3.5 Valutazione della Gestione Errori

Gli errori sono rilevati esplicitamente con controlli di tipo prima dell'operazione. Rappresentati come
`CompileError::TypeError` con `ErrorCode`. Propagati tramite `push_back` su `errors_`. I messaggi sono strutturati con
suggerimenti.

**Casi non gestiti**: `type_member_expr` [`TypeChecker.cpp:816-822`] non valida l'esistenza del membro — assegna
semplicemente una variabile di tipo fresca senza vincoli, silenziosamente ignorando errori semantici.

#### 3.6 Audit di Coerenza dei Tipi

**DEF-018**: `parse_type_annotation` [`TypeChecker.cpp:17-35`] restituisce `nullptr` per annotazioni sconosciute, ma il
chiamante in `type_var_decl` non controlla sistematicamente per `nullptr` [`TypeChecker.cpp:969`].

**DEF-019**: `zonk_type` usa `dynamic_cast` [`TypeChecker.cpp:42,51,57`] invece di `static_cast` dopo controllo
`kind()`, incoerente con il pattern usato altrove nel progetto.

#### 3.7 Interazioni Inter-Componente

Il `TypeChecker` dipende direttamente da 7+ sistemi interni. L'accoppiamento è stretto ma necessario per il ruolo di
orchestrazione.

#### 3.8 Opportunità di Ottimizzazione

**Performance**: `type_binary_expr` controlla `is_numeric()` due volte per lo stesso operando (righe 563 e 575).
Unificazione dei controlli ridurrebbe duplicazione.

**Strutturale**: Estrarre le 17 funzioni `type_*` in una classe `ExpressionTyper` con Strategy pattern ridurrebbe la
complessità cognitiva del `TypeChecker`.

**Manutenibilità**: I nomi delle funzioni `type_*` seguono snake_case mentre il progetto usa camelCase. Incoerenza di
convenzione.

---

### Sistema: ConstraintSolver › Componente: ConstraintSolver

#### 3.1 Responsabilità

Il componente **`ConstraintSolver`** risolve vincoli di uguaglianza tra tipi producendo una sostituzione unificante che
mappa variabili di tipo a tipi concreti.

#### 3.2 Struttura

| Campo           | Tipo           | Vis.      | Ruolo                             |
|-----------------|----------------|-----------|-----------------------------------|
| `union_find_`   | `UnionFind`    | `private` | Traccia equivalenze tra variabili |
| `substitution_` | `Substitution` | `private` | Accumula binding                  |

#### 3.3 Interfaccia

**`SolverResult solve(const ConstraintSet&)`**: Processa tutti i vincoli, accumula errori, restituisce sostituzione.

**`unify(TypePtr, TypePtr, const Constraint&)`**: Unifica due tipi. Usa occurs-check per prevenire tipi infiniti.

#### 3.4 Logica

Algoritmo di unificazione strutturale standard:

1. ErrorType → successo silenzioso.
2. TypeVariable-TypeVariable → occurs-check + bind.
3. TypeVariable-Concrete → bind.
4. Concrete-Concrete → stessa kind? visita ricorsiva : errore.

Complessità: O(m × n) dove m = vincoli, n = profondità tipi composti.

#### 3.5 Gestione Errori

Usa `std::expected<void, CompileError>` per propagazione esplicita. Corretto e moderno.

#### 3.6 Coerenza Tipi

Nessun casting unsafe. Uso appropriato di `dynamic_cast` per type narrowing.

#### 3.7 Interazioni

Dipende da `UnionFind`, `Substitution`, `TypeVisitor`, `ErrorType`.

#### 3.8 Ottimizzazioni

**Performance**: L'occurs-check applica la sostituzione ad ogni visita [`ConstraintSolver.cpp:58`], potenzialmente O(n²)
per tipi profondamente nidificati. Lazy evaluation con memoizzazione ridurrebbe il costo.

---

### Sistema: SymbolTable › Componente: SymbolTable

#### 3.1 Responsabilità

Il componente **`SymbolTable`** gestisce mappature identificatore→`TypeScheme` con ambito lessicale nidificato.

#### 3.2 Struttura

| Campo     | Tipo                                                  | Vis.      | Ruolo                   |
|-----------|-------------------------------------------------------|-----------|-------------------------|
| `scopes_` | `std::vector<unordered_map<string_view, TypeScheme>>` | `private` | Stack di frame di scope |

#### 3.3 Interfaccia

Tutti i metodi sono documentati con Doxygen. Precondizioni chiare.

#### 3.4 Logica

Operazioni O(1) per define/lookup nello scope corrente. O(d) per lookup attraverso gli scope, dove d = profondità.

#### 3.5 Gestione Errori

`pop_scope()` silenziosamente no-op se vuoto — **DEF-014**.

#### 3.6 Coerenza Tipi

**DEF-005**: `string_view` come chiave richiede lifetime management esterno.

#### 3.7 Interazioni

Consumato da `TypeChecker`. Nessun accoppiamento eccessivo.

#### 3.8 Ottimizzazioni

**Strutturale**: Estrarre il contesto di funzione in una classe `FunctionContext` separata invece di inquinare la symbol
table con `"__function_context__"`.

---

### Sistema: TypeScheme › Componente: TypeScheme

#### 3.1 Responsabilità

Il componente **`TypeScheme`** rappresenta schemi di tipo polimorfico con quantificazione universale.

#### 3.2 Struttura

Struttura POD con campi pubblici. `quantified_vars`, `body`, `is_const`, `return_type`, `function_name`.

#### 3.3 Interfaccia

**`instantiate()`**: Sostituisce variabili quantificate con variabili fresche.

**`mono()`**: Factory per schemi monomorfici.

#### 3.4 Logica

**DEF-008**: `instantiate()` gestisce solo il caso in cui `body` è una `TypeVariable` diretta. Per `ArrayType` o
`VectorType` contenenti variabili quantificate, la sostituzione non avviene [`TypeScheme.cpp:28-35`].

#### 3.5-3.8

Vedi DEF-008. La complessità dell'algoritmo di sostituzione completa richiederebbe un visitor che attraversi
ricorsivamente il body.

---

## Fase 4 — Raccomandazioni Prioritarie

### 4.1 Registro delle Raccomandazioni

---

#### **REC-001**

**Titolo**: Completare l'implementazione di `TypeScheme::instantiate()` per tipi composti

**Deficienza affrontata**: Fase 2, §2.6 — `TypeScheme::instantiate()` non sostituisce le variabili quantificate
all'interno di tipi composti come `ArrayType` e `VectorType` (`DEF-008`).

**Descrizione**:
Change entry point: `include/jsav/typechecker/TypeScheme.hpp`, metodo `TypeScheme::instantiate()`.

Attualmente, se il body di uno schema è un `ArrayType` la cui variabile elemento è quantificata, `instantiate()`
restituisce il body inalterato. Questo impedisce il corretto polimorfismo per tipi come `Vec<T>`.

Implementare un visitatore `SubstitutionVisitor` che:

1. Attraversa ricorsivamente il body.
2. Per ogni `TypeVariable` il cui ID è in `quantified_vars`, genera una variabile fresca e la sostituisce.
3. Per `ArrayType` e `VectorType`, visita ricorsivamente l'element type.
4. Per `CustomType`, nessuna sostituzione necessaria.

L'approccio consigliato è un visitor locale nel file `TypeScheme.cpp` (pattern già usato in `ConstraintSolver.cpp`).
Alternative con `std::expected` sono state scartate perché l'instantiation non può fallire.

Risultato atteso: Il polimorfismo parametrico funziona correttamente per tutti i tipi composti.

**Punteggio di Fattibilità**: 4 — Richiede la scrittura di un visitor ricorsivo ma è confinato a un singolo file. La
struttura del visitor è già presente nel codice base come riferimento.

**ROI Atteso**: 5 — Impatto trasformativo: senza questa correzione, il polimorfismo è fondamentalmente rotto per tutti i
tipi composti, compromettendo la correttezza dell'intero sistema di tipi.

**Sforzo di Implementazione**: 3 — Richiede 2-4 settimane di sviluppo e testing, inclusa la scrittura di test per
`Vec<T>` e `[T; N]` con variabili quantificate.

**Punteggio di Priorità**: (4 × 2) + (5 × 2) + (3 × 1) = 8 + 10 + 3 = **21**

**Tempo Stimato**: 2–4 settimane

**Risorse Richieste**:

- Ruoli: Un ingegnere C++ senior con esperienza in sistemi di tipi.
- Strumenti: Catch2 per unit test, clang-tidy per analisi statica.
- Accessi: Disponibilità del repository.
- Dipendenze esterne: Nessuna.

**Indicatori di Efficacia**:

1. Zero test fallenti per l'instantiation di `Vec<T>` e `[T; N]` nella suite `typechecker` dopo l'implementazione.
2. Coverage del ramo `ArrayType`/`VectorType` in `instantiate()` ≥90% come riportato da gcovr.

---

#### **REC-002**

**Titolo**: Sostituire `dynamic_cast` con `static_cast` + controllo `kind()` in `zonk_type`

**Deficienza affrontata**: Fase 3, §3.6 — `zonk_type` usa `dynamic_cast` incoerentemente con il resto del codice base (
`DEF-019`).

**Descrizione**:
Change entry point: `src/jsav_Lib/typechecker/TypeChecker.cpp`, funzione `zonk_type` (righe 40-72).

Sostituire ogni `dynamic_cast<const TypeX *>(type.get())` con:

```cpp
if(type->kind() == TypeKind::TypeVar) {
    const auto *tv = static_cast<const TypeVariable *>(type.get());
    // ...
}
```

Questo approccio è: (a) più performante (evita RTTI), (b) coerente con il pattern usato in `Type.cpp` per
l'uguaglianza, (c) coerente con le direttive NOLINT che sopprimono warnings su `static_cast-downcast` perché il
controllo `kind()` garantisce sicurezza.

**Punteggio di Fattibilità**: 5 — Modifica puramente meccanica, nessun cambiamento semantico.

**ROI Atteso**: 3 — Miglioramento misurabile delle prestazioni nei percorsi caldi di zonking e coerenza stilistica.

**Sforzo di Implementazione**: 5 — Minimo sforzo: 2-4 ore per un singolo sviluppatore.

**Punteggio di Priorità**: (5 × 2) + (3 × 2) + (5 × 1) = 10 + 6 + 5 = **21**

**Tempo Stimato**: 2–4 ore

**Risorse Richieste**:

- Ruoli: Un ingegnere C++ mid-level.
- Strumenti: clang-tidy per verificare assenza di warning.
- Accessi: Nessuno speciale.
- Dipendenze esterne: Nessuna.

**Indicatori di Efficacia**:

1. Zero occorrenze di `dynamic_cast` in `zonk_type` come verificato da grep post-implementazione.
2. Zero nuovi warning da clang-tidy sul file modificato.

---

#### **REC-003**

**Titolo**: Estrarre il contesto di funzione da `SymbolTable` in `FunctionContext` dedicato

**Deficienza affrontata**: Fase 1, §1.4c — Il marker `"__function_context__"` inquina lo spazio dei simboli (`DEF-006`).

**Descrizione**:
Change entry point: `include/jsav/typechecker/SymbolTable.hpp`, metodi `set_function_return_context()` e
`get_function_return_context()`.

Creare una classe `FunctionContext` con stack LIFO indipendente:

```cpp
class FunctionContext {
    std::vector<std::pair<TypePtr, std::string>> stack_;
public:
    void push(TypePtr ret, std::string name);
    void pop();
    std::optional<std::pair<TypePtr, std::string_view>> current() const;
};
```

Sostituire l'uso di `"__function_context__"` nella `SymbolTable` con delega a `FunctionContext`. Questo elimina il
rischio di collisione e separa le responsabilità.

**Punteggio di Fattibilità**: 4 — Refactoring confinato, richiede modifiche a `SymbolTable` e `TypeChecker`.

**ROI Atteso**: 3 — Migliora la manutenibilità e previene bug sottili di collisione dei nomi.

**Sforzo di Implementazione**: 4 — 1-2 settimane, con testing dello stack annidato.

**Punteggio di Priorità**: (4 × 2) + (3 × 2) + (4 × 1) = 8 + 6 + 4 = **18**

**Tempo Stimato**: 1–2 settimane

**Risorse Richieste**:

- Ruoli: Un ingegnere C++ mid-level.
- Strumenti: Catch2.
- Accessi: Nessuno.
- Dipendenze: Nessuna.

**Indicatori di Efficacia**:

1. Assenza della stringa `"__function_context__"` nel codice dopo il refactoring.
2. Tutti i test esistenti passano senza modifiche.

---

#### **REC-004**

**Titolo**: Far fallire rapidamente il `ConstraintSolver` al primo errore di unificazione

**Deficienza affrontata**: Fase 1, §1.4a — Il solver continua dopo errori invece di short-circuit (`DEF-003`).

**Descrizione**:
Change entry point: `src/jsav_Lib/typechecker/ConstraintSolver.cpp`, metodo `solve()` (righe 44-56).

Modificare il loop per interrompersi al primo errore irreversibile (E2034 type mismatch, E2035 occurs check):

```cpp
for(const auto &constraint : constraints.constraints()) {
    auto unify_result = unify(constraint.lhs, constraint.rhs, constraint);
    if(!unify_result) {
        result.errors.push_back(unify_result.error());
        // Per errori irreversibili, interrompere
        if(unify_result.error().error_code() == ErrorCode::E2035) {
            break;  // Occurs check = errore fatale
        }
    }
}
```

**Punteggio di Fattibilità**: 5 — Modifica di 5-10 righe.

**ROI Atteso**: 3 — Riduce il tempo di compilazione su programmi errati e previene errori cascading.

**Sforzo di Implementazione**: 5 — Minimo: poche ore.

**Punteggio di Priorità**: (5 × 2) + (3 × 2) + (5 × 1) = 10 + 6 + 5 = **21**

**Tempo Stimato**: 2–4 ore

**Risorse Richieste**:

- Ruoli: Un ingegnere C++ mid-level.
- Strumenti: Catch2 per regression testing.
- Accessi: Nessuno.
- Dipendenze: Nessuna.

**Indicatori di Efficacia**:

1. Il solver interrompe l'esecuzione al primo occurs-check failure, verificabile con benchmark su programmi con tipi
   ricorsivi.
2. Zero regressioni nella suite di test esistente.

---

#### **REC-005**

**Titolo**: Aggiungere controllo `nullptr` su `var_type` in `type_stmt` per `VarDecl`

**Deficienza affrontata**: Fase 3, §3.6 — `parse_type_annotation` restituisce `nullptr` per annotazioni sconosciute ma
il chiamante non controlla sistematicamente (`DEF-018`).

**Descrizione**:
Change entry point: `src/jsav_Lib/typechecker/TypeChecker.cpp`, sezione `VarDecl` in `type_stmt()` (righe 969-971).

Aggiungere:

```cpp
if(var_type && !var_type->is_primitive()) {
    // annotazione sconosciuta — fallback a type variable
    var_type = fresh_type_variable();
}
```

**Punteggio di Fattibilità**: 5 — Modifica di 3 righe.

**ROI Atteso**: 4 — Previene crash a runtime su annotazioni di tipo non riconosciute, migliorando significativamente la
robustezza.

**Sforzo di Implementazione**: 5 — Minimo: 1-2 ore.

**Punteggio di Priorità**: (5 × 2) + (4 × 2) + (5 × 1) = 10 + 8 + 5 = **23**

**Tempo Stimato**: 1–2 ore

**Risorse Richieste**:

- Ruoli: Un ingegnere C++ mid-level.
- Strumenti: Catch2.
- Accessi: Nessuno.
- Dipendenze: Nessuna.

**Indicatori di Efficacia**:

1. Zero crash su annotazioni di tipo sconosciute, verificabile con test di edge case.
2. Coverage del ramo `nullptr` in `type_stmt(VarDecl)` = 100%.

---

#### **REC-006**

**Titolo**: Validare l'esistenza dei membri in `type_member_expr`

**Deficienza affrontata**: Fase 3, §3.5 — `type_member_expr` non valida i membri, assegnando silenziosamente una type
variable (`DEF-011` correlato).

**Descrizione**:
Change entry point: `src/jsav_Lib/typechecker/TypeChecker.cpp`, metodo `type_member_expr()` (righe 816-822).

Attualmente il metodo assegna `fresh_type_variable()` come risultato senza verificare se il membro esiste sul tipo
dell'oggetto. Per un linguaggio che supporta struct/class, questo è un buco semantico.

Implementare una lookup del membro sul tipo dell'oggetto. Se il tipo è `CustomType`, richiedere una tabella dei membri (
da aggiungere al sistema dei tipi). Se il tipo non ha membri, generare errore E2031 o un nuovo codice errore per membri
inesistenti.

**Punteggio di Fattibilità**: 3 — Richiede l'aggiunta di una tabella dei membri al sistema dei tipi, che attualmente non
esiste.

**ROI Atteso**: 4 — Previene errori silenziosi che potrebbero causare comportamenti indefiniti a runtime.

**Sforzo di Implementazione**: 2 — Richiede 1-3 mesi, inclusa la progettazione della tabella dei membri.

**Punteggio di Priorità**: (3 × 2) + (4 × 2) + (2 × 1) = 6 + 8 + 2 = **16**

**Tempo Stimato**: 2–4 settimane

**Risorse Richieste**:

- Ruoli: Un ingegnere C++ senior.
- Strumenti: Catch2.
- Accessi: Nessuno.
- Dipendenze: Richiede il sistema struct/class (se non ancora implementato).

**Indicatori di Efficacia**:

1. Errore compilazione per accesso a membro inesistente, verificabile con test mirati.
2. Zero type variable irrisolte nel `TypedMemberExpr` finale.

---

#### **REC-007**

**Titolo**: Convertire `string_view` in `std::string` per le chiavi di `SymbolTable`

**Deficienza affrontata**: Fase 1, §1.4b — Chiavi `string_view` con rischio di dangling reference (`DEF-005`).

**Descrizione**:
Change entry point: `include/jsav/typechecker/SymbolTable.hpp`, campo `scopes_` (riga 99).

Sostituire:

```cpp
std::unordered_map<std::string_view, TypeScheme, StringHash, std::equal_to<>>
```

con:

```cpp
std::unordered_map<std::string, TypeScheme>
```

Il costo di allocazione aggiuntivo è trascurabile rispetto ai benefici di sicurezza. Il lookup eterogeneo può essere
mantenuto con `transparent_key_equal` se necessario per le performance.

**Punteggio di Fattibilità**: 5 — Modifica meccanica su 3-4 punti nel codice.

**ROI Atteso**: 4 — Elimina una classe intera di bug di memoria (use-after-free).

**Sforzo di Implementazione**: 5 — Minimo: 2-4 ore.

**Punteggio di Priorità**: (5 × 2) + (4 × 2) + (5 × 1) = 10 + 8 + 5 = **23**

**Tempo Stimato**: 2–4 ore

**Risorse Richieste**:

- Ruoli: Un ingegnere C++ mid-level.
- Strumenti: AddressSanitizer per verifica.
- Accessi: Nessuno.
- Dipendenze: Nessuna.

**Indicatori di Efficacia**:

1. Zero violazioni AddressSanitizer nei test della `SymbolTable` dopo la modifica.
2. Degradazione delle prestazioni < 5% misurata con benchmark su programmi con >1000 dichiarazioni.

---

#### **REC-008**

**Titolo**: Aggiungere asserzione di bilanciamento degli scope in `pop_scope()`

**Deficienza affrontata**: Fase 2, §2.5 — `pop_scope()` silenziosamente no-op se vuoto (`DEF-014`).

**Descrizione**:
Change entry point: `src/jsav_Lib/typechecker/SymbolTable.cpp`, metodo `pop_scope()` (righe 13-15).

Sostituire il silent no-op con:

```cpp
void SymbolTable::pop_scope() {
    assert(!scopes_.empty() && "Scope stack underflow — mismatched push_scope/pop_scope");
    if(!scopes_.empty()) { scopes_.pop_back(); }
}
```

**Punteggio di Fattibilità**: 5 — 1 riga.

**ROI Atteso**: 2 — Migliora la diagnosticabilità dei bug di gestione scope.

**Sforzo di Implementazione**: 5 — Minimo: 15 minuti.

**Punteggio di Priorità**: (5 × 2) + (2 × 2) + (5 × 1) = 10 + 4 + 5 = **19**

**Tempo Stimato**: 15–30 minuti

**Risorse Richieste**:

- Ruoli: Qualsiasi ingegnere.
- Strumenti: Nessuno.
- Accessi: Nessuno.
- Dipendenze: Nessuna.

**Indicatori di Efficacia**:

1. Assertion failure invece di silent no-op su scope underflow, verificabile con test mirato.

---

#### **REC-009**

**Titolo**: Estrarre le 17 funzioni `type_*` in classe `ExpressionTyper` dedicata

**Deficienza affrontata**: Fase 1, §1.3a — `TypeChecker` accumula troppe responsabilità (`DEF-001`).

**Descrizione**:
Change entry point: `include/jsav/typechecker/TypeChecker.hpp`, metodi privati `type_integer_literal` attraverso
`type_cast_expr`.

Creare:

```cpp
class ExpressionTyper {
    SymbolTable &symbols_;
    ConstraintSet &constraints_;
    std::vector<CompileError> &errors_;
    std::deque<std::string> &message_storage_;
    // ...
public:
    TypedExprPtr type_expr(const Expr &);
    // 17 metodi privati
};
```

Il `TypeChecker` delega a `ExpressionTyper` e si concentra su orchestrazione e statement typing.

**Punteggio di Fattibilità**: 3 — Refactoring significativo che richiede attenzione ai riferimenti condivisi.

**ROI Atteso**: 3 — Migliora la manutenibilità e la testabilità riducendo la complessità cognitiva del TypeChecker.

**Sforzo di Implementazione**: 2 — 1-3 mesi di refactoring progressivo.

**Punteggio di Priorità**: (3 × 2) + (3 × 2) + (2 × 1) = 6 + 6 + 2 = **14**

**Tempo Stimato**: 3–6 settimane

**Risorse Richieste**:

- Ruoli: Un ingegnere C++ senior.
- Strumenti: Catch2, clang-tidy.
- Accessi: Nessuno.
- Dipendenze: Nessuna.

**Indicatori di Efficacia**:

1. CCN di `TypeChecker::type_stmt` ridotto a ≤15 come riportato da lizard.
2. Tutti i test esistenti passano senza modifiche.

---

#### ~~**REC-011**~~ — ✅ **RISOLTO**

**Titolo**: Aggiungere metodo virtuale `clone()` a `TypeBase` per clonazione polimorfica

**Deficienza affrontata**: Fase 3, §3.5 — `TypeBase` cancella copy/move ma non fornisce `clone()` virtuale (`DEF-016`).

**Stato**: ✅ Implementato e verificato. `clone()` è ora presente in tutte le 6 classi derivate:

| Classe | Implementazione | Strategia |
|--------|----------------|-----------|
| `PrimitiveType` | `shared_from_this()` | Restituisce singleton (zero allocazione), aggiunto `std::enable_shared_from_this` |
| `CustomType` | `std::make_shared<CustomType>(*name_)` | Deep copy del nome |
| `ArrayType` | `std::make_shared<ArrayType>(element_type_->clone(), size_expr_)` | Clone ricorsivo elemento, condivide size expr |
| `VectorType` | `std::make_shared<VectorType>(element_type_->clone())` | Clone ricorsivo elemento |
| `TypeVariable` | `std::make_shared<TypeVariable>(id_)` | Copia ID |
| `ErrorType` | `error_type()` | Restituisce singleton |

File modificati: `include/jsav/ast/Type.hpp`, `include/jsav/typechecker/TypeVariable.hpp`, `include/jsav/typechecker/ErrorType.hpp`, `src/jsav_Lib/typechecker/ErrorType.cpp`.

Build: OK. Test: 868/868 passati.

**Nota**: Questa raccomandazione è stata rimossa dalla tabella delle priorità §4.2 poiché completata.

---

#### **REC-012**

**Titolo**: Sostituire `shared_ptr<const Expr>` con `std::int64_t` in `ArrayType::size_expr_`

**Deficienza affrontata**: Fase 3, §3.8 — `ArrayType` memorizza l'espressione AST invece del valore numerico (`DEF-022`).

**Descrizione**:
Change entry point: `include/jsav/ast/Type.hpp`, classe `ArrayType` (righe 420-421), e `src/jsav_Lib/ast/Type.cpp`, metodo `sizes_equal()`.

Sostituire:

```cpp
std::shared_ptr<const Expr> size_expr_;
```

con:

```cpp
std::int64_t size_value_;
```

Il costruttore di `ArrayType` deve estrarre il valore da `Expr` al momento della costruzione:

```cpp
ArrayType(std::shared_ptr<const TypeBase> element_type, std::shared_ptr<const Expr> size_expr) {
    // Estrarre il valore da size_expr
    if(const auto *lit = node_dyn_cast<const IntegerLiteral>(size_expr.get())) {
        size_value_ = lit->value();
    } else {
        // Valore non risolvibile a compile-time — errore o placeholder
        size_value_ = -1;  // Sentinel per "non noto"
    }
}
```

Questo elimina la dipendenza cross-layer `Type → Expr` e semplifica `sizes_equal()` a un confronto intero O(1).

**Punteggio di Fattibilità**: 3 — Richiede modifiche al costruttore e a tutti i call site che creano `ArrayType`.

**ROI Atteso**: 3 — Elimina una dipendenza cross-layer e semplifica il confronto tra tipi array.

**Sforzo di Implementazione**: 3 — 2-4 settimane, inclusi aggiornamenti ai call site e test.

**Punteggio di Priorità**: (3 × 2) + (3 × 2) + (3 × 1) = 6 + 6 + 3 = **15**

**Tempo Stimato**: 2–4 settimane

**Risorse Richieste**:

- Ruoli: Un ingegnere C++ senior (per gestire il refactoring cross-layer).
- Strumenti: Catch2, clang-tidy.
- Accessi: Nessuno.
- Dipendenze: Nessuna.

**Indicatori di Efficacia**:

1. Zero riferimenti a `Expr` in `Type.hpp` dopo il refactoring, verificabile con grep.
2. Tutti i test esistenti passano senza modifiche.
3. `sizes_equal()` ridotta a `return a.size_value_ == b.size_value_` — O(1).

---

### 4.2 Tabella Riassuntiva delle Priorità

| Rank | ID      | Titolo                                                     | Fattibilità | ROI | Sforzo | Punteggio | Tempo Stimato |
|------|---------|------------------------------------------------------------|-------------|-----|--------|-----------|---------------|
| 1    | REC-005 | Aggiungere controllo `nullptr` su `var_type`               | 5           | 4   | 5      | 23        | 1–2 ore       |
| 2    | REC-007 | Convertire `string_view` in `std::string` per chiavi       | 5           | 4   | 5      | 23        | 2–4 ore       |
| 3    | REC-001 | Completare `TypeScheme::instantiate()` per composti        | 4           | 5   | 3      | 21        | 2–4 sett.     |
| 4    | REC-002 | Sostituire `dynamic_cast` con `static_cast` in `zonk_type` | 5           | 3   | 5      | 21        | 2–4 ore       |
| 5    | REC-004 | Far fallire rapidamente il `ConstraintSolver`              | 5           | 3   | 5      | 21        | 2–4 ore       |
| 6    | REC-008 | Aggiungere asserzione in `pop_scope()`                     | 5           | 2   | 5      | 19        | 15–30 min     |
| 7    | REC-010 | Lookup table per `parse_type_annotation`                   | 5           | 2   | 5      | 19        | 1–2 ore       |
| 8    | REC-003 | Estrarre `FunctionContext` da `SymbolTable`                | 4           | 3   | 4      | 18        | 1–2 sett.     |
| 9    | REC-006 | Validare membri in `type_member_expr`                      | 3           | 4   | 2      | 16        | 2–4 sett.     |
| 10   | REC-012 | Sostituire `Expr` con `int64_t` in `ArrayType`             | 3           | 3   | 3      | 15        | 2–4 sett.     |
| 11   | REC-009 | Estrarre `ExpressionTyper` dal `TypeChecker`               | 3           | 3   | 2      | 14        | 3–6 sett.     |
