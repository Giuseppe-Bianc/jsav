# Feature Specification: Riconoscimento Completo dei Literal Numerici nel Lexer

**Feature Branch**: `003-numeric-literal-lexer`  
**Created**: 2026-03-03  
**Status**: Draft  
**Input**: User description: "Update the lexical analysis component (lexer) of the system for the recognition of numeric literals using the complete pattern `(\d+\.?\d*|\.\d+)([eE][+-]?\d+)?([uUfFdD]|[iIuU](?:8|16|32))?`, composed of three ordered and distinct groups: group G1 (mandatory) defines the numeric part; group G2 (optional) defines the exponent in scientific notation; group G3 (optional) defines the type suffix. The three groups must appear exclusively in the order G1 → G2 → G3, with no spaces or intervening characters between one group and the next. Group G1 has two mutually exclusive alternatives evaluated in the order shown: (A) `\d+\.?\d*`, which recognizes numbers with an explicit integer part composed of one or more decimal digits, optionally followed by a decimal point and zero or more fractional digits — the forms `42`, `3.`, `3.14`, and `0.5` are therefore valid, including the trailing-point form without subsequent digits such as `3.`, which must neither be rejected nor truncated; (B) `\.\d+`, which recognizes numbers with a fractional part only, composed of a mandatory decimal point followed by one or more digits — the forms `.5`, `.14`, `.0` are valid. Alternative B is activated only when the first character is `.` immediately followed by at least one digit; an isolated `.` or one followed by a non-digit character does not constitute a numeric literal and must not be consumed by the lexer as such. Group G2, when present, is composed of: a character `[eE]` (uppercase or lowercase, interchangeably), an optional sign `[+-]` (assumed positive if absent), and one or more mandatory decimal digits `\d+`. Group G2 must be recognized only if the sequence `[eE][+-]?\d+` is immediately contiguous with the end of G1. If no digits appear after `[eE]` (with or without an intervening sign), the character `e`/`E` must not be included in the numeric token and must be returned to the stream as a separate token; likewise, if `[eE]` is present but the sign `[+-]` is not followed by digits, neither the sign nor the exponent marker must be consumed. Group G3, when present, must be recognized according to the longest-match rule (greedy/maximal munch): compound suffixes `[iIuU](?:8|16|32)` take priority over single-character suffixes `[uU]` when the subsequent character forms a valid width. The permitted single-character suffixes are: `u`/`U` for unsigned integer, `f`/`F` for 32-bit float, `d`/`D` for 64-bit double. The permitted compound suffixes are: `i8`, `i16`, `i32`, `I8`, `I16`, `I32` for signed integers of 8, 16, and 32 bits respectively; `u8`, `u16`, `u32`, `U8`, `U16`, `U32` for unsigned integers of 8, 16, and 32 bits respectively. Valid widths for compound suffixes are exclusively 8, 16, and 32: sequences such as `i64`, `u64`, `i128`, `u128` do not constitute valid suffixes, and the prefix letter must not be consumed into the token. The character `i` or `I` alone (without a valid width) is not a valid suffix. The character `f` or `F` never forms a compound suffix with digits (e.g., `f32` must be read as suffix `f` followed by a separate token `32`). The lexer must recognize the width by attempting `32` first, then `16`, then `8`, to prevent `16` from being read as `1` followed by `6`. The lexer must apply the maximal munch rule: given the same starting position, the longest possible numeric token must be produced. The numeric token ends at the first character that cannot be included in the pattern at the current position, including: whitespace, operators, delimiters, end of file, and any alphabetic character that does not form a valid suffix at the current position. The characters `+` and `-` are part of the numeric token exclusively when they appear immediately after `[eE]` within group G2; in all other contexts, including before the literal, they are separate tokens (e.g., in `-42` the `-` is a unary operator and the numeric token is `42`). The generated token must be of type `TokenKind::Numeric` and the `text` field must contain the entire sequence of consumed characters exactly as they appear in the source, without any normalization: leading zeros must not be removed (`007` remains `007`), the trailing point must not be dropped (`3.` remains `3.`), the leading point must not be expanded (`.5` remains `.5`), the case of suffixes must not be altered (`1.0F` remains `1.0F`), and the numeric value must not be computed or interpreted. The token must also carry source position information: start index, end index (inclusive), line number (1-based), and column number (1-based) of the first character. Recognition must occur in a single pass over the input stream (single-pass, O(n) with respect to the length of the literal) without non-linear backtracking and without the use of regex libraries at runtime. A numeric literal cannot span multiple lines. Non-ASCII characters are always treated as non-digits and terminate any numeric literal in progress. Previously supported cases must continue to work without regressions: simple integers (`0`, `1`, `42`, `007`), decimals with an integer part (`1.0`, `3.14`, `0.5`), decimals with a trailing point (`1.`, `42.`), decimals with a fractional part only (`.5`, `.14`). The following edge cases must produce the tokens indicated: `1e` → token `1` + separate token `e`; `1e+` → token `1` + separate tokens `e`, `+`; `1u64` → `Numeric` token `1u64` (maximal munch) [Translator's Note: the source text lists `1u64` as a maximal munch example despite explicitly stating elsewhere that `u64` is not a valid suffix; the translation reproduces the source as written]; `1i` → token `1` + separate token `i`; `42 u8` → token `42` + separate token `u8` (the space prevents the suffix from attaching); `42u` → token `42` + separate token `u` (`u` alone is not a valid suffix); `.` → is not a numeric token; `-42` → token `-` + token `42`."

## User Scenarios & Testing *(mandatory)*

### User Story 1 — Recognizing basic integers and decimals (Priority: P1)

The lexer must correctly recognize numeric literals in the following fundamental forms: simple integers (e.g., `0`, `1`, `42`, `007`), decimal numbers with both an integer and a fractional part (e.g., `1.0`, `3.14`, `0.5`), decimal numbers with a trailing dot only (e.g., `1.`, `42.`), and numbers with a fractional part only, preceded by a dot (e.g., `.5`, `.14`, `.0`). The token text must be preserved exactly as it appears in the source, without normalization.

**Why this priority**: This is the foundational functionality (group G1) upon which all other scenarios are built. Without correct recognition of basic numeric forms, scientific notation and suffixes cannot be added. This also includes non-regression coverage of already supported cases.

**Independent Test**: It can be tested by supplying the lexer with strings containing each basic numeric form and verifying that the produced token contains the exact text and correct position coordinates.

**Acceptance Scenarios**:

1. **Given** the input `42`, **When** the lexer parses the stream, **Then** produces a Numeric token with text `42`
2. **Given** the input `3.14`, **When** the lexer parses the stream, **Then** produces a Numeric token with text `3.14`
3. **Given** the input `3.`, **When** the lexer parses the stream, **Then** produces a Numeric token with text `3.` (ending point preserved)
4. **Given** the input `.5`, **When** the lexer parses the stream, **Then** produces a Numeric token with text `.5` (start point preserved)
5. **Given** the input `007`, **When** the lexer parses the stream, **Then** produces a Numeric token with text `007` (leading zeros preserved)
6. **Given** the input `.`, **When** the lexer parses the stream, **Then** the period is NOT recognized as a numeric token
7. **Given** the input `.abc`, **When** the lexer parses the stream, **Then** the period is NOT recognized as a numeric token (no digits after the period)

---

### User Story 2 — Scientific Notation Recognition (Priority: P2)

The lexer must recognize the exponent group (G2) immediately following the numeric portion (G1). The group consists of an `e`/`E` marker, an optional `+`/`-` sign, and one or more mandatory digits. If the digits are absent after the marker, the marker and any sign must not be consumed in the numeric token.

**Why this priority**: Scientific notation is the second level of complexity, dependent on the correct functioning of G1, and is widely used for float literals in scientific and engineering contexts.

**Independent Test**: It can be tested by providing strings with valid and invalid scientific notation and verifying that the tokens produced are correct.

**Acceptance Scenarios**:

1. **Given** the input `1e10`, **When** the lexer parses, **Then** produces a single Numeric token with text `1e10`
2. **Given** the input `3.14E+2`, **When** the lexer parses, **Then** produces a single Numeric token with text `3.14E+2`
3. **Given** the input `2.5e-3`, **When** the lexer parses, **Then** produces a single Numeric token with text `2.5e-3`
4. **Given** the input `.5E10`, **When** the lexer parses, **Then** produces a single Numeric token with text `.5E10`
5. **Given** the input `1e`, **When** the lexer parses, **Then** produces Numeric token `1` followed by a separate token `e` (no digit after (the exponent)
6. **Given** the input `1e+`, **When** the lexer parses, **Then** produces the Numeric token `1` followed by separate tokens `e` and `+` (sign without digits)
7. **Given** the input `1E-`, **When** the lexer parses, **Then** produces the Numeric token `1` followed by separate tokens `E` and `-`

---

### User Story 3 — Type Suffix Recognition (Priority: P3)

The lexer must recognize type suffixes (G3) immediately following G1 or G2. Allowed single-character suffixes are `u`/`U` (unsigned), `f`/`F` (float 32-bit), and `d`/`D` (double 64-bit). Allowed compound suffixes are `i8`/`i16`/`i32` and `u8`/`u16`/`u32` (case insensitive). The maximal munch rule dictates that compound suffixes have priority. The character `i`/`I` alone is not a valid suffix. `f`/`F` never forms compound suffixes with digits.

**Why this priority**: Type suffixes complete the numeric pattern and allow literals to be typed, but they are less common than pure scientific notation.

**Independent Test**: Can be tested by providing digits followed by all allowed suffixes and verifying correct tokenization.

**Acceptance Scenarios**:

1. **Given** the input `42u`, **When** the lexer parses, **Then** produces token `1` + token `u` (`u` alone is not a valid suffix)
2. **Given** the input `1.0F`, **When** the lexer parses, **Then** produces Numeric token with text `1.0F` (`F` is a valid single suffix)
3. **Given** the input `10d`, **When** the lexer parses, **Then** produces Numeric token with text `10d` (`d` is a valid single suffix)
4. **Given** the input `255u8`, **When** the lexer parses, **Then** produces Numeric token with text `255u8` (valid compound suffix)
5. **Given** the input `1000i32`, **When** the lexer parses, **Then** produces Numeric token with text `1000i32` (valid compound suffix)
6. **Given** the input `50i16`, **When** the lexer parses, **Then** produces Numeric token with text `50i16` (valid compound suffix)
7. **Given** the input `1i`, **When** the lexer parses, **Then** produces token `1` + token `i` (`i` alone is not a valid suffix)
8. **Given** the input `1u64`, **When** the lexer parses, **Then** produces Numeric token with text `1u64` (maximal munch: `u` + digits consumes everything)
9. **Given** the input `5f32`, **When** the lexer parses, **Then** produces token `5f` + token `32` (`f` never forms suffixes) (compounds)
10. **Given** the input `42U`, **When** the lexer parses, **Then** produces token `42` + token `U` (`U` alone is not a valid suffix)
11. **Given** the input `100I`, **When** the lexer parses, **Then** produces token `100` + token `I` (`I` alone is not a valid suffix)

---

### User Story 4 — Complete G1-G2-G3 Combined Pattern (Priority: P4)

The lexer must recognize the combination of the three groups in the exclusive order G1 → G2 → G3, contiguous and without separators. Expressions combining scientific notation and a type suffix must produce a single numeric token.

**Why this priority**: This scenario covers the complete combination of the pattern and is the least frequent in real inputs, but necessary for overall correctness.

**Independent Test**: It can be tested with strings that combine all three groups and checking the resulting token.

**Acceptance Scenarios**:

1. **Given** the input `1.5e10f`, **When** the lexer parses, **Then** produces a single Numeric token with the text `1.5e10f`
2. **Given** the input `2.0E-3d`, **When** the lexer parses, **Then** produces a single Numeric token with the text `2.0E-3d`
3. **Given** the input `1e2u16`, **When** the lexer parses, **Then** produces a single Numeric token with the text `1e2u16`
4. **Given** the input `.5e1i32`, **When** the lexer parses, **Then** produces a single Numeric token with the text `.5e1i32`

---

### User Story 5 — Maximal Munch Rule and Token Boundaries (Priority: P5)

The lexer must apply the maximal munch rule: it must produce the longest possible numeric token. The characters `+` and `-` are part of the token only within the G2 group. Spaces, operators, delimiters, end-of-file characters, and non-ASCII characters terminate the literal.

**Why this priority**: Correct token boundaries are essential to avoid over- or under-consumption of characters, but it relies on the correct implementation of the previous groups.

**Independent Test**: Can be tested with inputs containing numbers adjacent to other tokens, verifying correct separation.

**Acceptance Scenarios**:

1. **Given** the input `-42`, **When** the lexer parses, **Then** produces the token `-` (operator) followed by the token Numeric `42`
2. **Given** the input `42 u8`, **When** the lexer parses, **Then** produces the token Numeric `42` followed by a separate token `u8` (the space interrupts the suffix start)
3. **Given** the input `3.14+2`, **When** the lexer parses, **Then** produces the token Numeric `3.14` followed by the token `+` and the token Numeric `2`
4. **Given** the input `1e2+3`, **When** the lexer parses, **Then** produces the token Numeric `1e2` followed by the token `+` and the token Numeric `3`

---

### Edge Cases

- A single period (`.`) must not be recognized as a numeric token.
- A period followed by a non-digit character (`.abc`) is not a numeric token.
- `1e` produces token `1` + token `e` (incomplete exponent)
- `1e+` produces token `1` + token `e` + token `+` (exponent without digits after the sign)
- `1E-` produces token `1` + token `E` + token `-`
- `1u` produces token `1` + token `u` (`u` alone is not a valid suffix)
- `1u64` produces Numeric token `1u64` (maximal munch: `u` followed by digits consumes all)
- `1i` produces token `1` + token `i` (`i` alone is not a valid suffix)
- `1i64` produces Numeric token `1i64` (maximal munch: `i` followed by (The digits consume everything.)
- `42 u8` produces token `42` + token `u8` (the space prevents the suffix from attaching to the number).
- `-42` produces tokens `-` + token `42` (`-` is not part of the numeric literal)
- `5f32` produces tokens `5f` + token `32` (`f` never forms compound suffixes with digits)
- `1d` produces Numeric token `1d` (`d` is a valid single suffix)
- `1f` produces Numeric token `1f` (`f` is a valid single suffix)
- Non-ASCII characters immediately terminate the numeric token.
- The numeric literal cannot span multiple lines.
- The priority for recognizing compound suffix widths is: `32`, then `16`, then `8`.
- `1e2i32` produces a single token `1e2i32` (G1 + G2 + G3 combined).

## Requirements *(mandatory)*

### Functional Requirements

#### Group G1 — Numeric Part (required)

- **FR-001**: The system MUST recognize integers composed of one or more decimal digits (e.g., `0`, `1`, `42`, `007`)
- **FR-002**: The system MUST recognize decimal numbers with both a whole number and a fractional part (e.g., `1.0`, `3.14`, `0.5`)
- **FR-003**: The system MUST recognize numbers with a final period without fractional digits (e.g., `3.`, `42.`) while preserving the period in the token text.
- **FR-004**: The system MUST recognize numbers with only a fractional part composed of a period followed by one or more digits (e.g., `.5`, `.14`, `.0`)
- **FR-005**: The system MUST NOT recognize a single period (`.`) as a numeric token.
- **FR-006**: The system MUST NOT recognize a period followed by a non-digit character (e.g., `.abc`) as a numeric token.
- **FR-007**: The two G1 alternatives must be evaluated in order: first the form with the integer part (A), then the form with only the period (B).

#### Group G2 — Scientific Notation (Optional)

- **FR-008**: The system MUST recognize an exponent group consisting of `e`/`E`, an optional sign `+`/`-`, and one or more mandatory digits, if immediately adjacent to G1.
- **FR-009**: If no digits (with or without an intervening sign) appear after `e`/`E`, the `e`/`E` marker MUST NOT be included in the numeric token and MUST be returned as a separate token.
- **FR-010**: If `e`/`E` is followed by `+`/`-` but with no subsequent digits, neither the sign nor the marker MUST be consumed in the numeric token.

#### Group G3 — Type Suffix (optional)

- **FR-011**: The system MUST recognize **valid** single-character suffixes: `f`/`F` (float 32-bit), `d`/`D` (double 64-bit). The characters `u`/`U` alone are **NOT** valid suffixes and MUST NOT be consumed as part of the numeric token.
- **FR-011b**: The system MUST apply the maximal munch rule for `u`/`U` and `i`/`I` followed by digits: if the letter is followed by one or more digits, the lexer MUST consume the entire sequence (letter + digits) as part of the numeric token, even if the digits do not constitute a valid width (e.g., `1u64` → `Numeric("1u64")`, `1i64` → `Numeric("1i64")`)
- **FR-012**: The system MUST recognize the compound suffixes: `i8`/`i16`/`i32`, `I8`/`I16`/`I32` (signed integer), `u8`/`u16`/`u32`, `U8`/`U16`/`U32` (unsigned integer)
- **FR-013**: The system MUST apply the longest match (maximal munch) rule for suffixes: compound suffixes take priority over single-character suffixes when the next character forms a valid width.
- **FR-014**: Valid widths for compound suffixes are only `8`, `16` and `32`; Sequences such as `i64`, `u64`, `i128`, `u128` are NOT valid suffixes (but are consumed for maximal munch if preceded by `u`/`U` or `i`/`I`).
- **FR-015**: The character `i`/`I` alone (without any following digits) is NOT a valid suffix and MUST NOT be consumed in the numeric token.
- **FR-015b**: The character `u`/`U` alone (without any following digits) is NOT a valid suffix and MUST NOT be consumed in the numeric token.
- **FR-016**: The character `f`/`F` NEVER forms a composite suffix with digits (e.g., `f32` must be read as the suffix `f` followed by a separate token `32`).
- **FR-017**: Width recognition MUST first attempt `32`, then `16`, then `16`. `8`, to avoid partial matches (e.g. `16` read as `1` + `6`)

#### Group Order and Contiguity

- **FR-018**: The three groups MUST appear exclusively in the order G1 → G2 → G3, without spaces or intervening characters.
- **FR-019**: Each group is optional except G1, which is mandatory.

#### Maximal Munch Rule and Token Boundaries

- **FR-020**: The system MUST apply the maximal munch rule: for the same starting position, the longest possible numeric token must be produced.
- **FR-021**: The characters `+` and `-` are part of the numeric token ONLY when they appear immediately after `e`/`E` in group G2; in all other contexts, they are separate tokens.
- **FR-022**: The numeric token ends at the first non-consumable character: whitespace, operators, delimiters, end-of-file, non-ASCII characters, or alphabetic characters that do not form a valid suffix at the current position.

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
