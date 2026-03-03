# Feature Specification: Riconoscimento Completo dei Literal Numerici nel Lexer

**Feature Branch**: `003-numeric-literal-lexer`  
**Created**: 2026-03-03  
**Status**: Draft  
**Input**: User description: "Aggiornare il componente di analisi lessicale (lexer) del sistema per il
riconoscimento dei literal numerici utilizzando il pattern completo
(\d+\.?\d*|\.\d+)([eE][+-]?\d+)?([uUfFdD]|[iIuU](?:8|16|32))?, composto da tre gruppi
ordinati e distinti: il gruppo G1 (obbligatorio) definisce la parte numerica; il gruppo G2
(opzionale) definisce l'esponente in notazione scientifica; il gruppo G3 (opzionale) definisce
il suffisso di tipo. I tre gruppi devono comparire esclusivamente nell'ordine G1 → G2 → G3,
senza spazi o caratteri interposti tra un gruppo e il successivo. Il gruppo G1 ha due alternative mutuamente esclusive valutate nell'ordine indicato: (A) \d+\.?\d*, che riconosce numeri con parte intera esplicita composta da una o più cifre decimali, opzionalmente seguita da un punto decimale e da zero o più cifre frazionarie — sono valide quindi le forme '42', '3.', '3.14' e '0.5', inclusa la forma con punto finale senza
cifre successive come '3.' che non deve essere rifiutata né mutilata; (B) \.\d+, che riconosce
numeri con sola parte frazionaria, composti da un punto decimale obbligatorio seguito da una o
più cifre — sono valide le forme '.5', '.14', '.0'. L'alternativa B si attiva solo quando il
primo carattere è '.' seguito immediatamente da almeno una cifra; un punto isolato '.' o
seguito da un carattere non-cifra non costituisce un literal numerico e non deve essere
consumato dal lexer come tale. Il gruppo G2, se presente, è composto da: un carattere [eE] (indifferentemente maiuscolo o minuscolo), un segno opzionale [+-] (se assente si assume positivo), e una o più cifre decimali obbligatorie \d+. Il gruppo G2 deve essere riconosciuto solo se la sequenza [eE][+-]?\d+ è
immediatamente contigua alla fine di G1. Se dopo [eE] non compaiono cifre (con o senza segno
intermedio), il carattere 'e'/'E' non deve essere incluso nel token numerico e deve essere
restituito al flusso come token separato; analogamente, se [eE] è presente ma il segno [+-]
non è seguito da cifre, né il segno né il marcatore di esponente devono essere consumati. Il gruppo G3, se presente, deve essere riconosciuto secondo la regola del match più lungo
(greedy/maximal munch): i suffissi composti [iIuU](?:8|16|32) hanno priorità sui suffissi
singolo-carattere [uU] quando il carattere successivo forma una larghezza valida. I suffissi
singolo-carattere ammessi sono: u/U per unsigned integer, f/F per float a 32 bit, d/D per
double a 64 bit. I suffissi composti ammessi sono: i8, i16, i32, I8, I16, I32 per integer
signed rispettivamente a 8, 16 e 32 bit; u8, u16, u32, U8, U16, U32 per unsigned integer
rispettivamente a 8, 16 e 32 bit. Le larghezze valide per i suffissi composti sono
esclusivamente 8, 16 e 32: sequenze come i64, u64, i128, u128 non costituiscono suffissi
validi e la lettera prefisso non deve essere consumata nel token. Il carattere 'i' o 'I' da
solo (senza larghezza valida) non è un suffisso valido. Il carattere 'f' o 'F' non forma mai
un suffisso composto con cifre (es. 'f32' deve essere letto come suffisso 'f' seguito da token
separato '32'). Il lexer deve riconoscere la larghezza tentando prima '32', poi '16', poi '8',
per evitare che '16' venga letto come '1' seguito da '6'. Il lexer deve applicare la regola del massimo consumo (maximal munch): a parità di posizione iniziale, deve essere prodotto il token numerico più lungo possibile. Il token numerico termina al primo carattere che non può essere incluso nel pattern nella posizione corrente, inclusi: spazi bianchi, operatori, delimitatori, fine file, e qualsiasi carattere alfabetico che non formi un suffisso valido nella posizione attuale. I caratteri '+' e '-' sono parte del token numerico esclusivamente quando compaiono immediatamente dopo [eE] all'interno del gruppo G2;
in tutti gli altri contesti, incluso prima del literal, sono token separati (es. in '-42' il
'-' è operatore unario e il token numerico è '42'). Il token generato deve essere di tipo TokenKind::Numeric e il campo text deve contenere l'intera sequenza di caratteri consumati, esattamente come appaiono nel sorgente, senza alcuna normalizzazione: gli zeri iniziali non devono essere rimossi ('007' resta '007'), il punto finale non deve essere eliminato ('3.' resta '3.'), il punto iniziale non deve essere espanso
('.5' resta '.5'), il case dei suffissi non deve essere alterato ('1.0F' resta '1.0F'), e il
valore numerico non deve essere calcolato né interpretato. Il token deve inoltre portare le
informazioni di posizione nel sorgente: indice di inizio, indice di fine (inclusivo), numero di
riga (1-based) e numero di colonna (1-based) del primo carattere. Il riconoscimento deve avvenire in un unico passaggio sul flusso di input (single-pass, O(n)
rispetto alla lunghezza del literal) senza backtracking non lineare e senza uso di librerie di
regex a runtime. Un literal numerico non può estendersi su più righe. I caratteri non-ASCII
sono sempre non-cifra e terminano qualsiasi literal numerico in corso.I casi già supportati devono continuare a funzionare senza regressioni: interi semplici (0, 1,
42, 007), decimali con parte intera (1.0, 3.14, 0.5), decimali con punto finale (1., 42.),
decimali con sola parte frazionaria (.5, .14). I seguenti casi di confine devono produrre i
token indicati: '1e' → token '1' + token separato 'e'; '1e+' → token '1 + token separati
'e', '+'; '1u64' → token Numeric '1u64' (maximal munch); '1i' → token '1' + token separato 'i';
'42 u8' → token '42' + token separato 'u8' (lo spazio impedisce l'attacco del suffisso);
'42u' → token '42' + token separato 'u' ('u' da solo non è suffisso valido); "." → non
è un token numerico; '-42' → token "-" + token '42'."

## User Scenarios & Testing *(mandatory)*

### User Story 1 — Riconoscimento numeri interi e decimali di base (Priority: P1)

Il lexer deve riconoscere correttamente i literal numerici nelle forme fondamentali: numeri interi semplici (es. `0`, `1`, `42`, `007`), numeri decimali con parte intera e frazionaria (es. `1.0`, `3.14`, `0.5`), numeri decimali con solo punto finale (es. `1.`, `42.`), e numeri con sola parte frazionaria preceduta da punto (es. `.5`, `.14`, `.0`). Il testo del token deve essere preservato esattamente come appare nel sorgente, senza normalizzazione.

**Why this priority**: Questa è la funzionalità fondamentale (gruppo G1) su cui si costruiscono tutti gli altri scenari. Senza un riconoscimento corretto dei numeri di base, notazione scientifica e suffissi non possono essere aggiunti. Include anche la non-regressione dei casi già supportati.

**Independent Test**: Può essere testato fornendo al lexer stringhe contenenti ciascuna forma numerica di base e verificando che il token prodotto contenga il testo esatto e le coordinate di posizione corrette.

**Acceptance Scenarios**:

1. **Given** l'input `42`, **When** il lexer analizza il flusso, **Then** produce un token di tipo Numeric con testo `42`
2. **Given** l'input `3.14`, **When** il lexer analizza il flusso, **Then** produce un token di tipo Numeric con testo `3.14`
3. **Given** l'input `3.`, **When** il lexer analizza il flusso, **Then** produce un token di tipo Numeric con testo `3.` (punto finale preservato)
4. **Given** l'input `.5`, **When** il lexer analizza il flusso, **Then** produce un token di tipo Numeric con testo `.5` (punto iniziale preservato)
5. **Given** l'input `007`, **When** il lexer analizza il flusso, **Then** produce un token di tipo Numeric con testo `007` (zeri iniziali preservati)
6. **Given** l'input `.`, **When** il lexer analizza il flusso, **Then** il punto NON viene riconosciuto come token numerico
7. **Given** l'input `.abc`, **When** il lexer analizza il flusso, **Then** il punto NON viene riconosciuto come token numerico (nessuna cifra dopo il punto)

---

### User Story 2 — Riconoscimento notazione scientifica (Priority: P2)

Il lexer deve riconoscere il gruppo esponente (G2) immediatamente dopo la parte numerica (G1). Il gruppo è composto da un marcatore `e`/`E`, un segno opzionale `+`/`-`, e una o più cifre obbligatorie. Se dopo il marcatore le cifre sono assenti, il marcatore e l'eventuale segno non devono essere consumati nel token numerico.

**Why this priority**: La notazione scientifica è il secondo livello di complessità, dipende dal corretto funzionamento di G1, ed è ampiamente usata per literal float in contesti scientifici e ingegneristici.

**Independent Test**: Può essere testato fornendo stringhe con notazione scientifica valida e non valida e verificando che i token prodotti siano corretti.

**Acceptance Scenarios**:

1. **Given** l'input `1e10`, **When** il lexer analizza, **Then** produce un singolo token Numeric con testo `1e10`
2. **Given** l'input `3.14E+2`, **When** il lexer analizza, **Then** produce un singolo token Numeric con testo `3.14E+2`
3. **Given** l'input `2.5e-3`, **When** il lexer analizza, **Then** produce un singolo token Numeric con testo `2.5e-3`
4. **Given** l'input `.5E10`, **When** il lexer analizza, **Then** produce un singolo token Numeric con testo `.5E10`
5. **Given** l'input `1e`, **When** il lexer analizza, **Then** produce token Numeric `1` seguito da un token separato `e` (nessuna cifra dopo l'esponente)
6. **Given** l'input `1e+`, **When** il lexer analizza, **Then** produce token Numeric `1` seguito da token separati `e` e `+` (segno senza cifre)
7. **Given** l'input `1E-`, **When** il lexer analizza, **Then** produce token Numeric `1` seguito da token separati `E` e `-`

---

### User Story 3 — Riconoscimento suffissi di tipo (Priority: P3)

Il lexer deve riconoscere i suffissi di tipo (G3) immediatamente dopo G1 o G2. I suffissi singolo-carattere ammessi sono `u`/`U` (unsigned), `f`/`F` (float 32-bit), `d`/`D` (double 64-bit). I suffissi composti ammessi sono `i8`/`i16`/`i32` e `u8`/`u16`/`u32` (case insensitive). La regola maximal munch impone che i suffissi composti abbiano priorità. Il carattere `i`/`I` da solo non è un suffisso valido. `f`/`F` non forma mai suffissi composti con cifre.

**Why this priority**: I suffissi di tipo completano il pattern numerico e permettono di tipizzare i literal, ma sono meno frequenti della notazione scientifica pura.

**Independent Test**: Può essere testato fornendo cifre seguite da tutti i suffissi ammessi e verificando la corretta tokenizzazione.

**Acceptance Scenarios**:

1. **Given** l'input `42u`, **When** il lexer analizza, **Then** produce token `1` + token `u` (`u` da solo non è suffisso valido)
2. **Given** l'input `1.0F`, **When** il lexer analizza, **Then** produce token Numeric con testo `1.0F` (`F` è suffisso singolo valido)
3. **Given** l'input `10d`, **When** il lexer analizza, **Then** produce token Numeric con testo `10d` (`d` è suffisso singolo valido)
4. **Given** l'input `255u8`, **When** il lexer analizza, **Then** produce token Numeric con testo `255u8` (suffisso composto valido)
5. **Given** l'input `1000i32`, **When** il lexer analizza, **Then** produce token Numeric con testo `1000i32` (suffisso composto valido)
6. **Given** l'input `50i16`, **When** il lexer analizza, **Then** produce token Numeric con testo `50i16` (suffisso composto valido)
7. **Given** l'input `1i`, **When** il lexer analizza, **Then** produce token `1` + token `i` (`i` da solo non è suffisso valido)
8. **Given** l'input `1u64`, **When** il lexer analizza, **Then** produce token Numeric con testo `1u64` (maximal munch: `u` + cifre consuma tutto)
9. **Given** l'input `5f32`, **When** il lexer analizza, **Then** produce token `5f` + token `32` (`f` non forma mai suffissi composti)
10. **Given** l'input `42U`, **When** il lexer analizza, **Then** produce token `42` + token `U` (`U` da solo non è suffisso valido)
11. **Given** l'input `100I`, **When** il lexer analizza, **Then** produce token `100` + token `I` (`I` da solo non è suffisso valido)

---

### User Story 4 — Pattern completo G1-G2-G3 combinato (Priority: P4)

Il lexer deve riconoscere la combinazione dei tre gruppi nell'ordine esclusivo G1 → G2 → G3, contigui e senza separatori. Espressioni che combinano notazione scientifica e suffisso di tipo devono produrre un singolo token numerico.

**Why this priority**: Questo scenario copre la combinazione completa del pattern ed è il meno frequente in input reali, ma necessario per la correttezza complessiva.

**Independent Test**: Può essere testato con stringhe che combinano tutti e tre i gruppi e verificando il token risultante.

**Acceptance Scenarios**:

1. **Given** l'input `1.5e10f`, **When** il lexer analizza, **Then** produce un singolo token Numeric con testo `1.5e10f`
2. **Given** l'input `2.0E-3d`, **When** il lexer analizza, **Then** produce un singolo token Numeric con testo `2.0E-3d`
3. **Given** l'input `1e2u16`, **When** il lexer analizza, **Then** produce un singolo token Numeric con testo `1e2u16`
4. **Given** l'input `.5e1i32`, **When** il lexer analizza, **Then** produce un singolo token Numeric con testo `.5e1i32`

---

### User Story 5 — Regola maximal munch e confini di token (Priority: P5)

Il lexer deve applicare la regola del massimo consumo: deve produrre il token numerico più lungo possibile. I caratteri `+` e `-` sono parte del token solo all'interno del gruppo G2. Spazi, operatori, delimitatori, fine file e caratteri non-ASCII terminano il literal.

**Why this priority**: La correttezza dei confini di token è essenziale per evitare consumo eccessivo o insufficiente di caratteri, ma si basa sulla corretta implementazione dei gruppi precedenti.

**Independent Test**: Può essere testato con input che contengono numeri adiacenti ad altri token e verificando la corretta separazione.

**Acceptance Scenarios**:

1. **Given** l'input `-42`, **When** il lexer analizza, **Then** produce token `-` (operatore) seguito da token Numeric `42`
2. **Given** l'input `42 u8`, **When** il lexer analizza, **Then** produce token Numeric `42` seguito da token separato `u8` (lo spazio interrompe l'attacco del suffisso)
3. **Given** l'input `3.14+2`, **When** il lexer analizza, **Then** produce token Numeric `3.14` seguito da token `+` e token Numeric `2`
4. **Given** l'input `1e2+3`, **When** il lexer analizza, **Then** produce token Numeric `1e2` seguito da token `+` e token Numeric `3`

---

### Edge Cases

- Un punto isolato (`.`) non deve essere riconosciuto come token numerico
- Un punto seguito da un carattere non-cifra (`.abc`) non è un token numerico
- `1e` produce token `1` + token `e` (esponente incompleto)
- `1e+` produce token `1` + token `e` + token `+` (esponente senza cifre dopo il segno)
- `1E-` produce token `1` + token `E` + token `-`
- `1u` produce token `1` + token `u` (`u` da solo non è suffisso valido)
- `1u64` produce token Numeric `1u64` (maximal munch: `u` seguito da cifre consuma tutto)
- `1i` produce token `1` + token `i` (`i` da solo non è suffisso valido)
- `1i64` produce token Numeric `1i64` (maximal munch: `i` seguito da cifre consuma tutto)
- `42 u8` produce token `42` + token `u8` (lo spazio impedisce l'attacco del suffisso al numero)
- `-42` produce token `-` + token `42` (`-` non fa parte del literal numerico)
- `5f32` produce token `5f` + token `32` (`f` non forma mai suffissi composti con cifre)
- `1d` produce token Numeric `1d` (`d` è suffisso singolo valido)
- `1f` produce token Numeric `1f` (`f` è suffisso singolo valido)
- Caratteri non-ASCII terminano immediatamente il token numerico
- Il literal numerico non può estendersi su più righe
- La priorità nel riconoscimento delle larghezze dei suffissi composti è: `32`, poi `16`, poi `8`
- `1e2i32` produce un singolo token `1e2i32` (G1 + G2 + G3 combinati)

## Requirements *(mandatory)*

### Functional Requirements

#### Gruppo G1 — Parte numerica (obbligatorio)

- **FR-001**: Il sistema DEVE riconoscere numeri interi composti da una o più cifre decimali (es. `0`, `1`, `42`, `007`)
- **FR-002**: Il sistema DEVE riconoscere numeri decimali con parte intera e parte frazionaria (es. `1.0`, `3.14`, `0.5`)
- **FR-003**: Il sistema DEVE riconoscere numeri con punto finale senza cifre frazionarie (es. `3.`, `42.`) preservando il punto nel testo del token
- **FR-004**: Il sistema DEVE riconoscere numeri con sola parte frazionaria composti da un punto seguito da una o più cifre (es. `.5`, `.14`, `.0`)
- **FR-005**: Il sistema NON DEVE riconoscere un punto isolato (`.`) come token numerico
- **FR-006**: Il sistema NON DEVE riconoscere un punto seguito da carattere non-cifra (es. `.abc`) come token numerico
- **FR-007**: Le due alternative di G1 devono essere valutate nell'ordine: prima la forma con parte intera (A), poi la forma con solo punto (B)

#### Gruppo G2 — Notazione scientifica (opzionale)

- **FR-008**: Il sistema DEVE riconoscere un gruppo esponente composto da `e`/`E`, un segno opzionale `+`/`-`, e una o più cifre obbligatorie, se immediatamente contiguo a G1
- **FR-009**: Se dopo `e`/`E` non compaiono cifre (con o senza segno intermedio), il marcatore `e`/`E` NON DEVE essere incluso nel token numerico e DEVE essere restituito come token separato
- **FR-010**: Se `e`/`E` è seguito da `+`/`-` ma senza cifre successive, né il segno né il marcatore DEVONO essere consumati nel token numerico

#### Gruppo G3 — Suffisso di tipo (opzionale)

- **FR-011**: Il sistema DEVE riconoscere i suffissi singolo-carattere **validi**: `f`/`F` (float 32-bit), `d`/`D` (double 64-bit). I caratteri `u`/`U` da soli **NON** sono suffissi validi e NON DEVONO essere consumati come parte del token numerico
- **FR-011b**: Il sistema DEVE applicare la regola maximal munch per `u`/`U` e `i`/`I` seguiti da cifre: se la lettera è seguita da una o più cifre, il lexer DEVE consumare l'intera sequenza (lettera + cifre) come parte del token numerico, anche se le cifre non costituiscono una larghezza valida (es. `1u64` → `Numeric("1u64")`, `1i64` → `Numeric("1i64")`)
- **FR-012**: Il sistema DEVE riconoscere i suffissi composti: `i8`/`i16`/`i32`, `I8`/`I16`/`I32` (signed integer), `u8`/`u16`/`u32`, `U8`/`U16`/`U32` (unsigned integer)
- **FR-013**: Il sistema DEVE applicare la regola del match più lungo (maximal munch) per i suffissi: i suffissi composti hanno priorità sui singolo-carattere quando il carattere successivo forma una larghezza valida
- **FR-014**: Le larghezze valide per i suffissi composti sono esclusivamente `8`, `16` e `32`; sequenze come `i64`, `u64`, `i128`, `u128` NON sono suffissi validi (ma vengono consumate per maximal munch se precedute da `u`/`U` o `i`/`I`)
- **FR-015**: Il carattere `i`/`I` da solo (senza cifre successive) NON è un suffisso valido e NON DEVE essere consumato nel token numerico
- **FR-015b**: Il carattere `u`/`U` da solo (senza cifre successive) NON è un suffisso valido e NON DEVE essere consumato nel token numerico
- **FR-016**: Il carattere `f`/`F` NON forma mai un suffisso composto con cifre (es. `f32` deve essere letto come suffisso `f` seguito da token separato `32`)
- **FR-017**: Il riconoscimento delle larghezze DEVE tentare prima `32`, poi `16`, poi `8`, per evitare match parziali (es. `16` letto come `1` + `6`)

#### Ordine e contiguità dei gruppi

- **FR-018**: I tre gruppi DEVONO comparire esclusivamente nell'ordine G1 → G2 → G3, senza spazi o caratteri interposti
- **FR-019**: Ogni gruppo è opzionale tranne G1, che è obbligatorio

#### Regola maximal munch e confini del token

- **FR-020**: Il sistema DEVE applicare la regola del massimo consumo: a parità di posizione iniziale, deve essere prodotto il token numerico più lungo possibile
- **FR-021**: I caratteri `+` e `-` sono parte del token numerico ESCLUSIVAMENTE quando compaiono immediatamente dopo `e`/`E` nel gruppo G2; in tutti gli altri contesti sono token separati
- **FR-022**: Il token numerico termina al primo carattere non consumabile: spazi bianchi, operatori, delimitatori, fine file, caratteri non-ASCII o caratteri alfabetici che non formano un suffisso valido nella posizione attuale

#### Formato del token prodotto

- **FR-023**: Il token generato DEVE essere di tipo Numeric
- **FR-024**: Il campo testo DEVE contenere l'intera sequenza di caratteri consumati esattamente come appaiono nel sorgente, senza alcuna normalizzazione (zeri iniziali preservati, punto finale/iniziale preservato, case dei suffissi preservato)
- **FR-025**: Il token DEVE portare le informazioni di posizione: indice di inizio, indice di fine (inclusivo), numero di riga (1-based) e numero di colonna (1-based) del primo carattere

#### Vincoli di performance e architettura

- **FR-026**: Il riconoscimento DEVE avvenire in un unico passaggio sul flusso (single-pass, complessità O(n) rispetto alla lunghezza del literal), senza backtracking non lineare
  - **Definizione**: Per "backtracking non lineare" si intende scansionare la stessa posizione di carattere più di due volte durante il riconoscimento di un singolo literal numerico
  - Il salvataggio e ripristino della posizione (`m_pos`, `m_column`) per tentativi di esponente o suffisso falliti conta come un singolo backtrack per ciascun tentativo
- **FR-027**: Il sistema NON DEVE utilizzare librerie di regex a runtime per il riconoscimento
- **FR-028**: Il literal numerico DEVE terminare al primo carattere newline (`\n`, `\r` o `\r\n`); il carattere newline NON DEVE essere consumato dal token `TokenKind::Numeric` e DEVE rimanere nel flusso per il token successivo, anche se il pattern G1→G2→G3 sarebbe altrimenti continuabile sulla riga successiva

#### Retrocompatibilità

- **FR-029**: Tutti i casi già supportati DEVONO continuare a funzionare senza regressioni: interi semplici, decimali con parte intera, decimali con punto finale, decimali con sola parte frazionaria

### Key Entities

- **Token Numerico**: Token di tipo Numeric prodotto dal lexer; contiene il testo letterale del numero riconosciuto, le coordinate di posizione nel sorgente (indice inizio, indice fine, riga, colonna), e rappresenta la concatenazione dei gruppi G1, G2 (se presente) e G3 (se presente)
- **Gruppo G1 (Parte numerica)**: Componente obbligatorio del literal numerico, con due alternative mutuamente esclusive: (A) parte intera con opzionale punto e cifre frazionarie, (B) punto seguito da cifre frazionarie
- **Gruppo G2 (Esponente)**: Componente opzionale per la notazione scientifica, composto da marcatore `e`/`E`, segno opzionale e cifre obbligatorie
- **Gruppo G3 (Suffisso di tipo)**: Componente opzionale che indica il tipo numerico; può essere singolo-carattere (`u`, `f`, `d`) o composto con larghezza (`i8`, `u16`, `i32`, ecc.)
- **Suffisso Singolo**: Uno tra `u`/`U`, `f`/`F`, `d`/`D`
- **Suffisso Composto**: Lettera prefisso (`i`/`I`/`u`/`U`) seguita da larghezza valida (`8`, `16`, `32`)

## Assumptions

- Il lexer opera in un contesto single-threaded durante l'analisi di un singolo flusso di input
- Il set di caratteri cifra è limitato a `0-9` (cifre ASCII); cifre Unicode non sono considerate
- L'ordine di precedenza delle alternative di G1 (prima A, poi B) è rilevante solo quando il primo carattere è una cifra — se il primo carattere è `.`, si attiva direttamente l'alternativa B
- I suffissi di tipo sono case-insensitive nel riconoscimento ma il testo originale viene preservato nel token
- Il suffisso `d`/`D` è ammesso solo come singolo carattere (non forma composti con cifre, analogamente a `f`/`F`)
- I caratteri `+` e `-` che precedono un literal numerico sono sempre token separati (operatori unari), anche quando l'intento semantico è quello di indicare un numero negativo

## Success Criteria *(mandatory)*

### Measurable Outcomes

- **SC-001**: Il 100% dei casi di test per numeri interi semplici (`0`, `1`, `42`, `007`) produce token con testo esatto e tipo Numeric
- **SC-002**: Il 100% dei casi di test per numeri decimali (con parte intera, con punto finale, con sola parte frazionaria) produce token corretti senza alterazione del testo sorgente
- **SC-003**: Il 100% dei casi di test per notazione scientifica valida (`1e10`, `3.14E+2`, `2.5e-3`) produce un singolo token Numeric contenente l'intera espressione
- **SC-004**: Il 100% dei casi di confine per notazione scientifica non valida (`1e`, `1e+`, `1E-`) produce token separati conformi alle regole di non-consumo
- **SC-005**: Il 100% dei casi di test per suffissi di tipo validi (singoli e composti) produce token con suffisso correttamente incluso
- **SC-006**: Il 100% dei casi di confine per suffissi non validi (`1i`, `1u64`, `5f32`) produce tokenizzazione conforme alle regole di maximal munch
- **SC-007**: Tutti i test preesistenti per il lexer continuano a passare senza regressioni
- **SC-008**: Il tempo di riconoscimento di un singolo literal numerico rimane proporzionale alla sua lunghezza (complessità lineare O(n)), verificabile per input fino a 1000 caratteri
