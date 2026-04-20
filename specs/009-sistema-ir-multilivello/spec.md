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

## Clarifications

### Session 2026-04-19

- Q: Quale strategia SSA/PHI deve essere canonica nel MIR? -> A: PHI basati su reaching definitions; dominance-frontier solo supporto analitico non vincolante.
- Q: Come va definita l'equivalenza semantica tra livelli IR? -> A: Stessi valori finali e stessi effetti osservabili su memoria, con ordine relativo delle dipendenze preservato.
- Q: Qual è la politica di commit/failure dei pass? -> A: Ogni pass è atomico; su errore avviene rollback completo allo stato IR valido precedente.
- Q: Quale regola di equivalenza deve usare il sistema per i tipi definiti dall'utente? -> A: Equivalenza nominale (stessa identità di tipo dichiarato).
- Q: Quale politica di ordinamento deve valere per output di analisi, errori e report? -> A: Ordinamento totale deterministico con chiave canonica stabile.
- Q: Quale struttura deve avere la chiave canonica stabile di ordinamento? -> A: Chiave gerarchica stabile: modulo/funzione/blocco/indice-istruzione/indice-operando.
- Q: Quale strategia di tracciabilita tra livelli deve essere canonica? -> A: ID immutabile globale per ogni entita IR con relazioni di derivazione esplicite tra HIR, MIR e LIR.
- Q: Quale obiettivo di scala deve essere assunto per la feature? -> A: Scala media: fino a 100k istruzioni per funzione e 2M per modulo.
- Q: Quando un predecessore diventa non raggiungibile, quale politica PHI deve essere canonica? -> A: Rimozione immediata di arco e operando PHI corrispondente durante aggiornamento CFG (normalizzazione eager).
- Q: Per gli ID immutabili globali, quale politica di generazione deve essere canonica? -> A: ID deterministici derivati da percorso canonico strutturale, stabili a parità di input e pipeline.
- Q: Quando cambia la definizione di un Tipo utente con Valori già tipizzati, quale politica deve valere? -> A: Versionamento nominale: nuova definizione crea nuova identità tipo; i Valori esistenti restano sulla versione precedente.

### Session 2026-04-20

- Q: Quale politica di reporting errori deve applicare la validazione di un pass fallito? -> A: Batch per pass: raccoglie tutti gli errori della rappresentazione corrente, poi fallisce il pass in blocco.
- Q: Quando una trasformazione elimina un blocco che definisce valori usati in più punti (inclusi PHI), quale politica canonica deve valere? -> A: Rewrite-safe: riscrivere tutti gli usi verso definizioni equivalenti dominate, aggiornare PHI/CFG, poi eliminare il blocco solo se validazione passa.
- Q: Quando due accessi memoria sono in relazione may-alias e una trasformazione propone un riordino, quale politica canonica deve valere? -> A: Strict no-reorder: vietato riordinare accessi may-alias, salvo prova formale di indipendenza.
- Q: Quale politica canonica deve valere per la minimalità PHI quando la convergenza include percorsi non raggiungibili? -> A: Pruning eager: rimozione immediata dei contributi non raggiungibili e ricomputo locale della minimalità PHI a ogni aggiornamento CFG.
- Q: Quale modello di concorrenza deve essere canonico per esecuzione di pass e analisi? -> A: Esecuzione single-thread deterministica per pass (nessun parallelismo intra-pass).
- Q: Quale evidenza deve essere canonica per accettare la prova formale di indipendenza che consente eccezione a strict no-reorder? -> A: Doppio criterio: certificato verificabile oppure dimostrazione interna riproducibile con alias/liveness/dependence analysis e log completo.
- Q: Quale modello di mutazione IR deve essere canonico durante un pass? -> A: Working copy transazionale per funzione o pass, con commit atomico solo dopo validazione conclusa.

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

- Quando un predecessore diventa non raggiungibile dopo una trasformazione, il sistema rimuove immediatamente l'arco e l'operando PHI corrispondente durante l'aggiornamento CFG (normalizzazione eager), mantenendo la validita SSA/PHI senza mismatch temporanei.
- Se una trasformazione elimina un blocco con definizioni usate in più punti (inclusi PHI), il sistema applica politica rewrite-safe: riscrive tutti gli usi verso definizioni equivalenti dominate, aggiorna PHI/CFG e consente l'eliminazione solo dopo validazione positiva.
- Quando la definizione di un Tipo utente cambia, il sistema applica versionamento nominale: crea una nuova identità di tipo e mantiene i Valori già tipizzati legati alla versione precedente.
- Quando la convergenza di controllo include percorsi non raggiungibili, il sistema applica pruning eager: rimuove immediatamente i contributi non raggiungibili e ricomputa localmente la minimalità dei PHI a ogni aggiornamento CFG.
- Se due accessi memoria sono in relazione may-alias, il sistema applica strict no-reorder: vieta il riordino, salvo disponibilità di una prova formale di indipendenza che dimostri assenza di dipendenza osservabile (vedi FR-028, FR-030 per criteri di evidenza).

## Requirements *(mandatory)*

### Functional Requirements

- **FR-001**: Il sistema MUST rappresentare programmi come Moduli contenenti Funzioni, Tipi e metadati con regole globali di visibilità e validazione.
- **FR-002**: Il sistema MUST supportare tre livelli IR distinti (HIR, MIR, LIR) con regole di validità specifiche per livello.
- **FR-003**: Il sistema MUST consentire la costruzione di IR a qualunque livello, purché ogni rappresentazione soddisfi le regole di validazione del livello scelto.
- **FR-004**: Il sistema MUST mantenere un grafo di controllo per ogni Funzione con entry unico, archi validi e terminatori di controllo in ogni blocco.
- **FR-005**: Il sistema MUST modellare Valori immutabili con singolo punto di definizione, Tipo esplicito e tracciabilità completa definizione-uso.
- **FR-006**: Il sistema MUST mantenere forma SSA completa in MIR, inclusi inserimento, aggiornamento e rimozione minima dei nodi PHI, usando reaching definitions come criterio canonico di placement.
- **FR-007**: Il sistema MUST inserire nodi PHI solo nei punti di convergenza in cui reaching definitions indica definizioni distinte provenienti da predecessori diversi; dominance-frontier può essere usata solo come supporto analitico non vincolante.
- **FR-008**: Il sistema MUST garantire che ogni nodo PHI includa esattamente un operando per predecessore del blocco target.
- **FR-009**: Il sistema MUST aggiornare automaticamente la struttura SSA e i PHI dopo qualunque modifica del grafo di controllo, preservando il criterio canonico basato su reaching definitions; se un predecessore diventa non raggiungibile, MUST rimuovere immediatamente arco e operando PHI corrispondente (normalizzazione eager).
- **FR-010**: Il sistema MUST supportare un sistema di tipi con tipi primitivi, composti e definiti dall’utente, includendo regole di struttura, equivalenza e compatibilità operazionale.
- **FR-011**: Il sistema MUST rifiutare operazioni con Tipi incompatibili e interrompere il pass corrente con errori espliciti e localizzati.
- **FR-012**: Il sistema MUST preservare i Tipi durante le trasformazioni oppure dichiarare conversioni valide e verificabili.
- **FR-013**: Il sistema MUST supportare analisi di dominanza, reaching definitions, liveness e dipendenze tra istruzioni in modalità forward e backward.
- **FR-014**: Il sistema MUST garantire risultati di analisi deterministici a parità di input, ordine dei pass e configurazione.
- **FR-015**: Il sistema MUST modellare accessi memoria tipizzati distinguendo lettura/scrittura e tracciando dipendenze e possibili alias.
- **FR-016**: Il sistema MUST impedire trasformazioni che alterano l’ordine osservabile degli accessi memoria quando esistono dipendenze.
- **FR-017**: Il sistema MUST consentire pipeline esplicite di pass (analisi, trasformazione, ottimizzazione, lowering) con precondizioni, invarianti preservati ed effetti dichiarati.
- **FR-018**: Il sistema MUST eseguire verifiche automatiche dopo ogni pass e produrre output validato oppure interrompere la pipeline con errori.
- **FR-019**: Il sistema MUST mantenere tracciabilità tra rappresentazioni consecutive (HIR→MIR→LIR) mediante ID immutabili globali per entità IR (Modulo/Funzione/Blocco/Istruzione/Valore) e relazioni di derivazione esplicite, per supportare audit delle trasformazioni; la generazione ID MUST essere deterministica e derivata da un percorso canonico strutturale dell'entità (modulo/funzione/blocco/indice e tipo entità), stabile a parità di input, ordine pass e configurazione.
- **FR-020**: Il sistema MUST impedire la persistenza di stati intermedi non validi.
- **FR-021**: Il sistema MUST considerare semanticamente equivalenti due rappresentazioni solo se preservano sia i valori finali osservabili sia gli effetti osservabili su memoria, inclusa la preservazione dell'ordine relativo tra accessi con dipendenze (come definite in FR-016).
- **FR-022**: Il sistema MUST eseguire ogni pass su una working copy transazionale della rappresentazione interessata (funzione o pass): il commit sullo stato IR osservabile avviene solo dopo validazione conclusa; in caso di errore di validazione o trasformazione deve essere mantenuto integralmente lo stato pre-pass osservabile, senza commit parziali.
- **FR-023**: Il sistema MUST applicare equivalenza nominale ai tipi definiti dall'utente: due tipi utente sono equivalenti solo se condividono la stessa identità dichiarativa nel modulo o nello spazio di visibilità definito; ogni modifica di definizione/shape/regole MUST generare una nuova identità nominale (versione), senza ritipizzare implicitamente i Valori già esistenti.
 Le versioni MUST essere identificate in modo univoco e deterministico (ad esempio tramite contatore monotono per tipo o hash della definizione); ogni Valore MUST mantenere un riferimento esplicito alla versione del Tipo a cui appartiene.

- **FR-024**: Il sistema MUST emettere output di analisi, errori e report in ordinamento totale deterministico basato su una chiave canonica stabile gerarchica (modulo/funzione/blocco/indice-istruzione/indice-operando), a parità di input, ordine pass e configurazione; tale chiave MUST essere allineata alla politica canonica di generazione ID deterministici definita in FR-019.
- **FR-025**: Il sistema MUST supportare il caso d'uso target di scala media: fino a 100k istruzioni per Funzione e fino a 2M istruzioni complessive per Modulo mantenendo validazione, analisi e trasformazioni complete.
- **FR-026**: In caso di fallimento di validazione di un pass, il sistema MUST applicare reporting batch-per-pass: durante la fase di validazione post-trasformazione, raccogliere tutti gli errori rilevabili attraverso le verifiche strutturali, tipologiche, SSA, CFG e memoria applicabili sulla rappresentazione post-trasformazione, poi fallire il pass in blocco senza commit parziali (coerente con FR-022). Se un errore impedisce verifiche dipendenti, il sistema MUST annotare esplicitamente le verifiche omesse.
- **FR-027**: Se una trasformazione elimina un blocco con definizioni usate in più punti (inclusi nodi PHI), il sistema MUST applicare politica rewrite-safe: (1) verificare preliminarmente che per ogni valore V definito nel blocco candidato esista una definizione alternativa SSA-identica o semanticamente equivalente che domina tutti gli usi di V; (2) se la verifica fallisce, rifiutare la trasformazione senza modifiche; (3) altrimenti, riscrivere tutti gli usi verso le definizioni alternative, aggiornare coerentemente PHI/CFG, quindi (4) consentire l'eliminazione del blocco solo dopo validazione positiva post-trasformazione.
- **FR-028**: Se due accessi memoria sono in relazione may-alias, il sistema MUST applicare strict no-reorder: vietare il riordino degli accessi, salvo disponibilità di prova formale di indipendenza (come definita in FR-030) che dimostri preservazione degli effetti osservabili su memoria.
- **FR-029**: L'esecuzione canonica di pass e analisi MUST essere single-thread deterministica per pass (nessun parallelismo intra-pass), preservando ordinamento canonico e ripetibilità bit-identica dei risultati a parità di input, pipeline e configurazione.
- **FR-030**: Per derogare a strict no-reorder in presenza di may-alias (FR-028), il sistema MUST accettare solo prova formale di indipendenza con doppio criterio canonico: (a) certificato verificabile da checker dedicato conforme a formato standard (struttura dati contenente witness di indipendenza, regole di inferenza applicate, e fatto conclusivo), il cui checker MUST verificare soundness della derivazione; oppure (b) dimostrazione interna riproducibile basata su alias/liveness/dependence analysis con log deterministico completo contenente: fatti alias per gli accessi coinvolti, intervalli liveness dei valori, archi dependence rilevanti, passi di inferenza applicati, e conclusione di indipendenza, in formato machine-readable per verifica indipendente. In entrambi i casi, la prova MUST essere generata deterministicamente e riproducibile a parità di input, pipeline e configurazione (coerente con FR-014, FR-029).

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
- **SC-007**: Nel 100% dei casi validati come equivalenti tra livelli, devono risultare invariati sia i valori finali osservabili sia gli effetti osservabili su memoria con ordine relativo delle dipendenze preservato.
- **SC-008**: Nel 100% dei pass falliti, lo stato IR osservabile post-failure coincide con l’ultimo stato valido pre-pass (rollback completo, nessun commit parziale).
- **SC-009**: Nel 100% delle verifiche tipo-su-tipo per tipi definiti dall’utente, l’esito di equivalenza dipende unicamente dall’identità nominale dichiarata e non dalla sola struttura; in presenza di ridefinizione del tipo, il 100% dei Valori preesistenti resta associato alla versione nominale originaria.
- **SC-010**: Nel 100% delle esecuzioni ripetute con stesso input e stessa pipeline, l’ordine di analisi, errori e report coincide esattamente secondo la chiave canonica stabile gerarchica (modulo/funzione/blocco/indice-istruzione/indice-operando).
- **SC-011**: Nel 100% delle trasformazioni valide HIR→MIR→LIR sulla suite di regressione approvata, ogni entità IR tracciata conserva un ID immutabile globale deterministico (derivato da percorso canonico strutturale) e presenta almeno una relazione di derivazione esplicita verificabile verso l'entità sorgente immediata nel livello precedente; per supportare audit completo, il sistema SHOULD mantenere anche relazioni transitive verso l'entità originaria nel primo livello HIR quando l'entità è derivata da HIR.
- **SC-012**: Sulla suite di benchmark approvata di scala target (fino a 100k istruzioni per Funzione e 2M per Modulo), nel 100% dei casi il sistema completa validazione, analisi principali e pass previsti senza violare i vincoli funzionali definiti in FR-001..FR-030.
- **SC-013**: Nel 100% dei pass che falliscono validazione, il report include l'insieme completo degli errori rilevabili per quel pass sulla rappresentazione corrente e l'esecuzione termina con un unico esito di failure del pass (nessun commit parziale).
- **SC-014**: Nel 100% delle trasformazioni che eliminano blocchi con usi multipli (inclusi PHI), tutti gli usi risultano riscritti verso definizioni equivalenti dominate e la validazione post-pass conferma assenza di use-def dangling e coerenza SSA/CFG.
- **SC-015**: Nel 100% dei casi con accessi memoria may-alias, nessun pass effettua riordino senza prova formale di indipendenza; in presenza della prova, la validazione post-pass conferma preservazione degli effetti osservabili su memoria.
- **SC-016**: Nel 100% delle esecuzioni con stesso input, pipeline e configurazione, pass e analisi eseguiti in modalità canonica single-thread producono output e report bit-identici, senza dipendenze da interleaving concorrenti intra-pass.
- **SC-017**: Nel 100% dei casi in cui un pass applica eccezione a strict no-reorder su accessi may-alias, è presente evidenza verificabile conforme al doppio criterio canonico (certificato checker oppure dimostrazione interna riproducibile con log completo), e la validazione post-pass conferma preservazione degli effetti osservabili su memoria.

## Assumptions

- Gli utenti sono sviluppatori del compilatore e operano su programmi già lessicalmente e sintatticamente validi.
- La validazione richiesta riguarda coerenza strutturale, tipologica, di flusso e di memoria delle rappresentazioni IR.
- La nozione di equivalenza semantica richiede uguaglianza dei valori finali osservabili e conservazione degli effetti osservabili su memoria, inclusa la preservazione dell’ordine relativo imposto dalle dipendenze.
- Le pipeline di pass sono dichiarate esplicitamente e valutate in ordine deterministico.
- La scala obiettivo della feature è media: fino a 100k istruzioni per Funzione e 2M istruzioni per Modulo.
- I tipi definiti dall’utente forniscono metadati sufficienti per equivalenza e compatibilità operazionale.
- Per i tipi definiti dall’utente, l’equivalenza canonica adottata è nominale.
- Le evoluzioni dei tipi definiti dall'utente seguono versionamento nominale e non mutano retroattivamente la tipizzazione dei Valori già emessi.
- Le esigenze di interfaccia grafica e output machine code non fanno parte di questa feature.

# 🔧 ESTENSIONE TECNICA: MODELLO SSA E COSTRUZIONE PHI

Questa sezione integra formalmente i risultati della letteratura su SSA, dominance frontier e reaching definitions nel livello MIR.

---

## 1. Vincolo fondamentale SSA nel MIR

Il MIR è vincolato alla forma SSA:

\forall v,\ \exists!\ \text{def}(v) \land \text{def}(v) \text{ domina tutte le use}(v)

Questo implica un controllo strutturale completo del grafo di flusso e delle definizioni.

---

## 2. Costruzione classica basata su dominanza

La strategia standard usa dominance frontier.

DF(n)={m \mid n \text{ domina un predecessore di } m \land n \not\succ m}

Iterazione:

DF^{+}(S)=\mu X.; S \cup DF(X)

Uso:

- inserzione PHI nei join points
- dipendenza da dominanza globale

Limite:

- over-approximation
- PHI ridondanti possibili

---

## 3. Limite strutturale del modello DF

Il modello DF introduce approssimazione:

- join points stimati per struttura, non per reale reachability
- sensibilità a CFG non riducibili
- PHI non necessari in casi locali

---

## 4. Costruzione basata su reaching definitions

Alternativa dataflow-based.

RD(n)={d \mid d \text{ raggiunge } n}

Condizione PHI:

\text{PHI}(n,x) \iff |RD_{pred}(n,x)| > 1

Proprietà:

- PHI solo quando semanticamente necessari
- riduzione ridondanza
- dipendenza da dataflow reale

---

## 5. Costruzione incrementale SSA

Modello alternativo:

- PHI inseriti durante costruzione CFG
- rinomina simultanea
- nessuna fase DF globale necessaria

Implicazione:

- MIR può essere costruito direttamente in SSA valida
- aggiornamenti locali propagano modifiche SSA

---

## 6. Impatto architetturale sul sistema IR

Il MIR adotta una modalità SSA canonica:

1. Reaching definitions SSA (criterio normativo per inserimento PHI)

Dominance frontier è consentita come supporto di analisi e ottimizzazione, ma non definisce da sola la validità finale dei PHI.

Vincolo globale:

\forall \phi,\ \forall i,\ operand_i \in RD(pred_i)

---

## 7. Integrazione nei requisiti

FR-006 SSA completa richiede criterio canonico RD per il placement PHI.

FR-007 PHI placement dipende da reaching definitions; DF resta ausiliaria.

FR-009 aggiornamento SSA richiede ricalcolo locale coerente con RD.

---

## 8. Sintesi strutturale

Il sistema IR adotta una teoria SSA canonica nel MIR:

- RD-based: criterio normativo di correttezza e minimalità PHI
- DF-based: supporto opzionale per accelerare analisi o candidati di inserimento
- Incrementale: ammessa solo se produce output equivalente al criterio RD

Il MIR non delega la scelta canonica al singolo pass: la validità finale è sempre verificata rispetto a reaching definitions.
