# Feature Specification: Sistema IR Multi-Livello Verificabile

**Feature Branch**: `009-sistema-ir-multilivello`  
**Created**: 19 aprile 2026  
**Status**: Draft  
**Input**: User description: "Progettare un sistema di rappresentazione intermedia multi-livello che consente di modellare, validare, analizzare e trasformare programmi lungo una pipeline di compilazione con progressiva riduzione dell’astrazione, garantendo conservazione semantica, precisione nel flusso dei dati e verificabilità formale delle trasformazioni.

Obiettivo e successo:
Gli utenti possono rappresentare programmi come strutture formali basate su grafo di controllo e flusso dei dati, e trasformarli attraverso HIR, MIR e LIR senza perdita di significato osservabile. Il sistema garantisce che ogni trasformazione sia semanticamente equivalente alla precedente rispetto agli effetti su valori e memoria. Il successo è determinato da: assenza di errori strutturali dopo ogni pass, forma SSA valida e minimale, corretta gestione dei nodi PHI, coerenza del sistema di tipi anche con tipi definiti dall’utente, e risultati di analisi deterministici. Ogni fase deve produrre una rappresentazione validata oppure essere interrotta con errori espliciti.

Entità principali:

- Modulo:
  Contiene tutte le definizioni globali. Include Funzioni, Tipi definiti dall’utente e metadati. Definisce il contesto di visibilità e le regole globali di validazione. Mantiene una tabella dei Tipi e delle firme delle Funzioni.
- Funzione:
  Unità primaria di trasformazione. Definita da una firma con parametri e risultati tipizzati. Contiene un grafo di controllo con un unico punto di ingresso. È il dominio su cui si applica la forma SSA.
- Blocco di base:
  Nodo del grafo di controllo. Contiene una sequenza ordinata di Istruzioni senza interruzioni interne. Ha predecessori e successori espliciti. Termina con una Istruzione di controllo.
- Grafo di controllo:
  Struttura diretta che collega Blocchi tramite archi di esecuzione. Definisce l’ordine possibile delle esecuzioni. Include un nodo entry e zero o più nodi di uscita.
- Istruzione:
  Operazione atomica. Può rappresentare calcolo, controllo o accesso alla memoria. Consuma Valori e produce nuovi Valori.
- Valore:
  Entità immutabile associata a una singola definizione. Ogni Valore ha un Tipo e un punto di definizione univoco.
- Tipo:
  Definisce dominio, struttura e regole di utilizzo dei Valori. Include tipi primitivi, composti e definiti dall’utente.

Relazioni:

- Un Modulo contiene Funzioni e Tipi.
- Una Funzione contiene Blocchi collegati nel grafo di controllo.
- Ogni Blocco contiene Istruzioni e termina con una Istruzione di controllo.
- Le Istruzioni definiscono Valori che possono essere usati da altre Istruzioni.
- Il grafo di controllo determina quali definizioni raggiungono ogni uso.
- I nodi PHI combinano Valori provenienti da predecessori distinti.

Livelli di astrazione:

HIR:

- Gli utenti possono esprimere il programma in forma semantica ad alto livello.
- Le operazioni mantengono significato logico diretto e possono rappresentare costrutti complessi.
- Le variabili possono essere concettualmente riassegnate prima della conversione completa in SSA.
- Le analisi includono verifica semantica, coerenza dei tipi e struttura del controllo.
- Non sono presenti vincoli legati a risorse o architettura.

MIR:

- Il programma è completamente convertito in forma SSA.
- Tutte le definizioni sono uniche e i nodi PHI sono esplicitamente presenti.
- Le operazioni sono granulari e rappresentano computazioni elementari.
- Le analisi includono dominanza, reaching definitions, dipendenze tra istruzioni.
- Le ottimizzazioni modificano il flusso dei dati mantenendo equivalenza semantica.

LIR:

- Le istruzioni sono vincolate al modello di esecuzione.
- Il controllo di flusso è espresso tramite salti espliciti.
- Le dipendenze temporali e di memoria sono completamente esplicite.
- Le operazioni sono ridotte a forme direttamente eseguibili.
- Non sono presenti astrazioni di alto livello.

Forma SSA e costruzione:

- Ogni Valore è definito una sola volta.
- Ogni uso è dominato dalla definizione.
- Il sistema mantiene una mappatura esplicita tra definizioni e usi.
- I nodi PHI sono inseriti nei punti di convergenza del controllo.

Costruzione PHI:

- Il sistema calcola le reaching definitions per ogni variabile.
- Per ogni Blocco con più predecessori, verifica se arrivano definizioni distinte.
- Inserisce un nodo PHI solo se necessario.
- Ogni nodo PHI contiene una associazione completa tra predecessori e Valori.
- Evita inserimenti ridondanti per mantenere una forma SSA minimale.
- Aggiorna automaticamente i PHI quando il grafo di controllo cambia.

Manutenzione SSA:

- Rinominazione delle variabili per mantenere unicità.
- Eliminazione di PHI non necessari.
- Propagazione dei Valori quando possibile.

Sistema di tipi:

- Ogni Valore ha un Tipo esplicito e immutabile.
- Le Istruzioni definiscono vincoli sui Tipi degli operandi e dei risultati.
- Il sistema supporta:
    - Tipi primitivi
    - Tipi composti
    - Tipi definiti dall’utente
- I Tipi definiti dall’utente devono specificare:
    - struttura
    - regole di equivalenza
    - compatibilità con operazioni
- Le operazioni devono essere valide rispetto ai Tipi.
- Le trasformazioni devono preservare i Tipi o dichiarare conversioni valide.
- Il sistema rifiuta operazioni con Tipi incompatibili.

Flusso dei dati e analisi:

- Il sistema costruisce un grafo delle dipendenze tra Istruzioni.
- Supporta analisi forward e backward.
- Reaching definitions:
    - determinano quali definizioni arrivano a ogni punto del programma
    - guidano inserimento PHI e ottimizzazioni
- Dominanza:
    - definisce validità delle definizioni rispetto agli usi
- Analisi di liveness:
    - identifica Valori vivi in ogni punto
- Le analisi sono consistenti con la forma SSA.

Gestione della memoria e aliasing:

- Le operazioni di memoria sono esplicite e tipizzate.
- Ogni accesso dichiara se è lettura o scrittura.
- Le dipendenze tra accessi sono tracciate.
- Il sistema modella possibili alias tra riferimenti.
- Le analisi possono determinare quando accessi sono indipendenti.
- Le trasformazioni non possono modificare l’ordine osservabile degli accessi in presenza di dipendenze.
- Il sistema garantisce coerenza del modello di memoria.

Pipeline di elaborazione:

- Gli utenti possono definire sequenze di pass.
- Ogni pass opera su una rappresentazione valida.
- Tipi di pass:
    - Analisi: non modifica la struttura
    - Trasformazione: modifica la rappresentazione
    - Ottimizzazione: riduce ridondanze
    - Lowering: riduce il livello di astrazione
- Ogni pass deve dichiarare:
    - precondizioni
    - invarianti preservati
    - effetti sulla struttura
- L’ordine dei pass è esplicito e controllato.
- Il sistema supporta esecuzione incrementale dei pass.

Verifica e validazione:

- Il sistema esegue verifiche dopo ogni pass.
- Verifica del grafo di controllo:
    - presenza di entry unico
    - archi validi
- Verifica SSA:
    - una sola definizione per Valore
    - dominanza rispettata
- Verifica PHI:
    - un operando per ogni predecessore
- Verifica dei Tipi:
    - compatibilità tra operandi e risultati
- Verifica delle dipendenze:
    - nessun uso senza definizione
- Verifica memoria:
    - rispetto delle dipendenze e aliasing
- Gli errori sono riportati con localizzazione precisa.

Comportamenti attesi:

- Gli utenti possono costruire IR a qualsiasi livello.
- Gli utenti possono trasformare IR mantenendo validità.
- Gli utenti possono introdurre nuovi Tipi.
- Gli utenti possono eseguire analisi e ottenere informazioni sul programma.
- Il sistema mantiene tracciabilità tra rappresentazioni.
- Il sistema impedisce stati intermedi non validi.

Ambito incluso:

- IR multi-livello HIR MIR LIR
- Forma SSA completa con nodi PHI
- Costruzione precisa dei PHI basata su reaching definitions
- Sistema di tipi rigoroso ed estensibile
- Analisi di flusso dati e controllo
- Pipeline di trasformazioni e ottimizzazioni
- Verifica strutturale e semantica continua
- Gestione memoria e aliasing

Ambito escluso:

- Scelte di implementazione
- Architetture hardware specifiche
- Generazione diretta di codice macchina
- Interfacce utente"

## User Scenarios & Testing *(mandatory)*

### User Story 1 - Costruzione e Validazione IR (Priority: P1)

Come sviluppatore del compilatore, voglio costruire programmi in HIR, MIR o LIR e ottenere immediatamente una validazione strutturale e semantica, così da impedire stati intermedi non validi.

**Why this priority**: È il valore minimo indispensabile: senza validazione robusta, nessuna trasformazione successiva è affidabile.

**Independent Test**: Può essere testata costruendo un modulo con funzioni e blocchi validi/non validi a ogni livello; il sistema deve accettare solo rappresentazioni valide e interrompere i casi invalidi con errori espliciti.

**Acceptance Scenarios**:

1. **Given** un Modulo con Funzioni ben tipizzate e grafo di controllo valido, **When** viene eseguita la validazione del livello corrente, **Then** la rappresentazione è marcata come valida e pronta ai pass successivi.
2. **Given** una Funzione con blocchi senza terminatore o con archi incoerenti, **When** viene eseguita la validazione, **Then** la pipeline si interrompe con errori localizzati e descrittivi.
3. **Given** un uso di Valore senza definizione raggiungibile, **When** viene eseguita la validazione, **Then** il sistema segnala errore di dipendenza con localizzazione precisa.

---

### User Story 2 - Trasformazioni Semantiche tra HIR/MIR/LIR (Priority: P2)

Come sviluppatore del compilatore, voglio trasformare il programma da HIR a MIR e poi a LIR mantenendo il significato osservabile su valori e memoria, così da ottenere una pipeline di lowering affidabile.

**Why this priority**: Il cuore della feature è la riduzione progressiva dell’astrazione senza regressioni semantiche.

**Independent Test**: Può essere testata applicando sequenze di pass su programmi con controllo complesso e memoria; il risultato osservabile deve restare equivalente tra livelli consecutivi.

**Acceptance Scenarios**:

1. **Given** un programma valido in HIR, **When** viene applicato il pass di lowering a MIR, **Then** il programma risultante è valido e semanticamente equivalente rispetto a valori e memoria.
2. **Given** un programma valido in MIR, **When** viene applicato il pass di lowering a LIR, **Then** il controllo di flusso è espresso con salti espliciti e la semantica osservabile è preservata.
3. **Given** una trasformazione che violerebbe dipendenze di memoria, **When** il pass viene eseguito, **Then** il sistema rifiuta la trasformazione con errore esplicito.

---

### User Story 3 - Analisi Deterministiche e Tracciabilità (Priority: P3)

Come sviluppatore del compilatore, voglio eseguire analisi (dominanza, reaching definitions, liveness, dipendenze) con risultati deterministici e tracciabili tra livelli, così da supportare ottimizzazioni ripetibili e verificabili.

**Why this priority**: Migliora affidabilità e auditabilità dell’intera pipeline, ma si appoggia a validazione e trasformazioni già operative.

**Independent Test**: Può essere testata rieseguendo le stesse analisi sullo stesso input e confrontando i risultati; devono essere identici e coerenti con la forma SSA.

**Acceptance Scenarios**:

1. **Given** una rappresentazione MIR valida in SSA, **When** vengono eseguite analisi forward e backward, **Then** i risultati sono deterministici e coerenti con dominanza e reaching definitions.
2. **Given** modifiche del grafo di controllo dovute a ottimizzazioni, **When** il sistema aggiorna SSA e PHI, **Then** le analisi successive restano consistenti e senza incongruenze.

---

### Edge Cases

- Cosa accade quando un blocco ha predecessori multipli con definizioni parziali della stessa variabile?
- Come viene gestita una trasformazione che elimina un blocco con PHI usati in più punti?
- Cosa accade quando un Tipo definito dall’utente cambia regole di equivalenza mentre esistono già Valori tipizzati?
- Come viene gestita la convergenza di controllo con percorsi non raggiungibili che influenzano la minimalità dei PHI?
- Cosa accade quando due accessi memoria possono aliasare e una trasformazione tenta il riordino?

## Requirements *(mandatory)*

### Functional Requirements

- **FR-001**: Il sistema MUST rappresentare programmi come Moduli contenenti Funzioni, Tipi e metadati con regole globali di visibilità e validazione.
- **FR-002**: Il sistema MUST supportare tre livelli IR distinti (HIR, MIR, LIR) con regole di validità specifiche per livello.
- **FR-003**: Il sistema MUST consentire la costruzione di IR a qualunque livello, purché ogni rappresentazione soddisfi le regole di validazione del livello scelto.
- **FR-004**: Il sistema MUST mantenere un grafo di controllo per ogni Funzione con entry unico, archi validi e terminatori di controllo in ogni blocco.
- **FR-005**: Il sistema MUST modellare Valori immutabili con singolo punto di definizione, Tipo esplicito e tracciabilità completa definizione-uso.
- **FR-006**: Il sistema MUST mantenere forma SSA completa in MIR, inclusi inserimento, aggiornamento e rimozione minima dei nodi PHI.
- **FR-007**: Il sistema MUST inserire nodi PHI solo nei punti di convergenza in cui reaching definitions indica definizioni distinte provenienti da predecessori diversi.
- **FR-008**: Il sistema MUST garantire che ogni nodo PHI includa esattamente un operando per predecessore del blocco target.
- **FR-009**: Il sistema MUST aggiornare automaticamente la struttura SSA e i PHI dopo qualunque modifica del grafo di controllo.
- **FR-010**: Il sistema MUST supportare un sistema di tipi con tipi primitivi, composti e definiti dall’utente, includendo regole di struttura, equivalenza e compatibilità operazionale.
- **FR-011**: Il sistema MUST rifiutare operazioni con Tipi incompatibili e interrompere il pass corrente con errori espliciti e localizzati.
- **FR-012**: Il sistema MUST preservare i Tipi durante le trasformazioni oppure dichiarare conversioni valide e verificabili.
- **FR-013**: Il sistema MUST supportare analisi di dominanza, reaching definitions, liveness e dipendenze tra istruzioni in modalità forward e backward.
- **FR-014**: Il sistema MUST garantire risultati di analisi deterministici a parità di input, ordine dei pass e configurazione.
- **FR-015**: Il sistema MUST modellare accessi memoria tipizzati distinguendo lettura/scrittura e tracciando dipendenze e possibili alias.
- **FR-016**: Il sistema MUST impedire trasformazioni che alterano l’ordine osservabile degli accessi memoria quando esistono dipendenze.
- **FR-017**: Il sistema MUST consentire pipeline esplicite di pass (analisi, trasformazione, ottimizzazione, lowering) con precondizioni, invarianti preservati ed effetti dichiarati.
- **FR-018**: Il sistema MUST eseguire verifiche automatiche dopo ogni pass e produrre output validato oppure interrompere la pipeline con errori.
- **FR-019**: Il sistema MUST mantenere tracciabilità tra rappresentazioni consecutive (HIR→MIR→LIR) per supportare audit delle trasformazioni.
- **FR-020**: Il sistema MUST impedire la persistenza di stati intermedi non validi.

### Key Entities *(include if feature involves data)*

- **Modulo**: Contenitore globale di Funzioni, Tipi definiti dall’utente, metadati, tabella tipi e firme funzione.
- **Funzione**: Unità primaria di trasformazione con firma tipizzata, entry unico e dominio SSA.
- **Blocco di base**: Nodo del grafo di controllo con sequenza ordinata di Istruzioni e terminatore di controllo obbligatorio.
- **Grafo di controllo**: Struttura diretta di esecuzione tra blocchi con vincoli di raggiungibilità e convergenza.
- **Istruzione**: Operazione atomica di calcolo, controllo o memoria che consuma e/o produce Valori.
- **Valore**: Entità immutabile con Tipo e definizione univoca.
- **Tipo**: Definizione formale di dominio e regole d’uso dei Valori, inclusa estensione con tipi definiti dall’utente.
- **Nodo PHI**: Combinatore SSA di definizioni provenienti da predecessori multipli.
- **Pass**: Trasformazione o analisi con contratto esplicito (precondizioni, invarianti, effetti).

### Confini di Ambito

**In scope**:

- IR multi-livello HIR/MIR/LIR.
- Forma SSA completa con gestione PHI minimale.
- Costruzione PHI basata su reaching definitions.
- Sistema di tipi rigoroso ed estensibile.
- Analisi di flusso dati/controllo deterministiche.
- Pipeline di pass con verifica continua.
- Modellazione esplicita memoria e aliasing.

**Out of scope**:

- Scelte di implementazione specifiche.
- Dipendenze da architetture hardware specifiche.
- Generazione diretta di codice macchina.
- Interfacce utente.

## Success Criteria *(mandatory)*

### Measurable Outcomes

- **SC-001**: Il 100% dei pass eseguiti su input valido produce una rappresentazione validata oppure un errore esplicito, senza stati intermedi persistenti non validi.
- **SC-002**: Nel 100% dei casi validi in MIR, ogni uso di Valore è dominato dalla sua definizione e ogni Valore ha una sola definizione.
- **SC-003**: Nel 100% dei blocchi con PHI, ogni predecessore contribuisce con esattamente un operando e non sono presenti PHI ridondanti osservabili dopo la fase di minimizzazione.
- **SC-004**: Nel 100% delle trasformazioni HIR→MIR e MIR→LIR su suite di regressione approvata, l’equivalenza semantica osservabile su valori e memoria è preservata.
- **SC-005**: Nel 100% delle esecuzioni ripetute con stesso input e stessa pipeline, i risultati delle analisi (dominanza, reaching definitions, liveness, dipendenze) sono identici.
- **SC-006**: Nel 100% dei casi di incompatibilità di tipo o violazione strutturale, la pipeline si interrompe nella fase corretta con errore localizzato e motivazione verificabile.

## Assumptions

- Gli utenti sono sviluppatori del compilatore e operano su programmi già lessicalmente e sintatticamente validi.
- La validazione richiesta riguarda coerenza strutturale, tipologica, di flusso e di memoria delle rappresentazioni IR.
- La nozione di equivalenza semantica è definita in termini di effetti osservabili su valori prodotti e stato memoria osservabile.
- Le pipeline di pass sono dichiarate esplicitamente e valutate in ordine deterministico.
- I tipi definiti dall’utente forniscono metadati sufficienti per equivalenza e compatibilità operazionale.
- Le esigenze di interfaccia grafica e output machine code non fanno parte di questa feature.
