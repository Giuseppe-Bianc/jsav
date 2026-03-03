# Research: Riconoscimento Completo dei Literal Numerici nel Lexer

**Feature**: 003-numeric-literal-lexer
**Date**: 2026-03-03

## R1 — Trailing-dot behavior change: impatto sulla codebase

### Decision

Cambiare il comportamento di `scan_numeric_literal()` per includere il punto finale nel token
numerico: `123.` → `Numeric("123.")` anziché `Numeric("123") + Dot(".")`.

### Rationale

La spec FR-003 richiede esplicitamente che `3.` produca un token `Numeric` con testo `3.`.
Il linguaggio jsav non ha method-call syntax su literal numerici (non è Rust/Kotlin/Swift),
quindi la motivazione originale del split ("123.toString()") non si applica.
L'utente ha confermato esplicitamente il cambio nel task description.

### Alternatives considered

- **Mantenere lo split** `Numeric + Dot`: rifiutata perché contraddice FR-003 e la spec
  approvata. Il lexer jsav non necessita di supporto per method-call su literal.
- **Rendere configurabile**: rifiutata — complessità non giustificata, YAGNI.

### Impact assessment

- **Test esistenti**: Un solo test (`Lexer_AsciiOnlySource_TokenizeCorrectly` in
  test/tests.cpp L2771) verifica indirettamente i numeri (`"42"` come Numeric), ma nessun
  test verifica specificamente il trailing-dot split. Il test per `Dot` come operatore
  (`Lexer_AsciiOperators_UnchangedAfterUtf8` L3558) testa `"."` isolato, non `"42."`.
  **Nessun test esistente si rompe** con questo cambio.
- **Commento nel header**: Il doccomment in `Lexer.hpp` (riga 38-40) descrive il vecchio
  trailing-dot rule e va aggiornato.
- **Commento nell'implementazione**: Il commento in `Lexer.cpp` (righe 245-248) spiega il
  vecchio comportamento e va rimosso/sostituito.

---

## R2 — Leading-dot entry point in next_token()

### Decision

Aggiungere un branch in `next_token()` prima di `scan_operator_or_punctuation()`:
se `peek_byte() == '.'` e `std::isdigit(peek_byte(1))`, invocare `scan_numeric_literal(start)`.

### Rationale

Il flusso attuale in `next_token()` invia qualsiasi `'.'` direttamente a
`scan_operator_or_punctuation()` che lo emette come `Dot`. Per supportare `.5` come literal
numerico (FR-004, spec G1-B), il check deve avvenire prima del fallthrough all'operator scanner.

### Alternatives considered

- **Modificare `scan_operator_or_punctuation()`** per fare il lookahead internamente:
  rifiutata — viola separation of concerns, la logica numerica deve stare nel numeric scanner.
- **Aggiungere un pre-scanner generico**: rifiutata — overengineering per un singolo check.

### Implementation detail

Il punto isolato `'.'` e `'.abc'` continuano a raggiungere `scan_operator_or_punctuation()` perché
il guard `std::isdigit(peek_byte(1))` fallisce. Nessuna regressione per gli operatori Dot.

---

## R3 — Decomposizione per conformità Lizard (CCN ≤ 15)

### Decision

Decomporre `scan_numeric_literal()` in 4 metodi:

1. `scan_numeric_literal()` — orchestratore (G1 + chiamate a helper)
2. `try_scan_exponent()` — G2: lookahead non-distruttivo per `[eE][+-]?\d+`
3. `try_scan_type_suffix()` — G3: riconoscimento suffissi con maximal munch
4. `match_width_suffix()` — utility: matching larghezze `32`/`16`/`8`

### Rationale

Il pattern G1→G2→G3 completo genera almeno 20 branch (2 rami G1, 3 check esponente,
~8 check suffisso, 3 larghezze, edge cases). Senza decomposizione, la CCN stimata è ~25-30,
ben sopra il limite 15. Con la decomposizione:

- `scan_numeric_literal()`: CCN ~8 (digit loop, dot check, entry from dot, calls)
- `try_scan_exponent()`: CCN ~6 (e/E check, sign, digit loop, rollback)
- `try_scan_type_suffix()`: CCN ~8 (d/D, f/F, u/U, i/I branches + width check)
- `match_width_suffix()`: CCN ~4 (32/16/8 comparisons)

### Alternatives considered

- **Metodo monolitico con `NOLINTBEGIN`**: rifiutata — la constitution (III, enforcement)
  richiede conformità senza eccezioni per nuove funzioni.
- **State machine con tabella di transizione**: rifiutata — over-engineering per 4 gruppi
  lineari, peggior leggibilità, non giustificabile.

---

## R4 — Lookahead non-distruttivo per esponente (G2)

### Decision

Usare save/restore di `m_pos` e `m_column` per il lookahead dell'esponente.
Se la validazione fallisce (`e` senza cifre), ripristinare la posizione originale.

### Rationale

La spec FR-009/FR-010 richiede che `1e`, `1e+`, `1E-` NON consumino il marcatore.
L'approccio corrente in `Lexer.cpp` consuma `e`/`E` incondizionatamente (L254-258) — produce
token errati come `"1e"` o `"1e+"` anziché separare in `"1"` + `"e"` + `"+"`.

Il pattern save/restore è il più semplice:

```cpp
const auto saved_pos = m_pos;
const auto saved_col = m_column;
// tentativo di consumo...
if(/*invalid*/) { m_pos = saved_pos; m_column = saved_col; }
```

### Alternatives considered

- **Lookahead con peek_byte(n)**: possibile ma richiede contare offset manuali per
  `e` + opzionale `+/-` + almeno 1 digit, risultando in codice più fragile.
- **Advance e backtrack generico**: equivalente a save/restore, più verboso.

### Note

Il save/restore di `m_line` non è necessario: l'esponente non può attraversare righe (FR-028),
e i caratteri `e`, `+`, `-`, digits sono tutti single-byte ASCII che non avanzano la riga.

---

## R5 — Underscore separators

### Decision

Il supporto agli underscore come digit separator NON rientra nello scope di questa feature.
Il pattern di riferimento è esclusivamente `(\d+\.?\d*|\.\d+)([eE][+-]?\d+)?([uUfFdD]|[iIuU](?:8|16|32))?`.
Gli underscore nei literal hash-prefixed (`#b`, `#o`, `#x`) rimangono invariati poiché gestiti
da scanner separati.

---

## R6 — Suffix matching: ordine e maximal munch

### Decision

Ordine di matching dei suffissi (G3): `d`/`D` → `f`/`F` → `u`/`U` → `i`/`I`.
Per `u`/`U` e `i`/`I`: tentare prima le larghezze composte `32` → `16` → `8`
(confronto a 2 cifre prima di 1 cifra).

### Rationale

La spec FR-017 richiede esplicitamente l'ordine `32→16→8` per evitare match parziali.
`d`/`D` e `f`/`F` sono sempre single-char (FR-016: `f` non forma composti).
L'ordine `d→f→u→i` non è critico (i caratteri sono mutualmente esclusivi) ma migliora
la leggibilità del codice.

### Key edge cases verified

| Input | Expected tokens | Rule applied |
|-------|----------------|--------------|
| `5f32` | `5f` + `32` | FR-016: `f` never forms compounds |
| `1u64` | `1u` + `64` | FR-014: `64` not a valid width, `u` consumed as bare |
| `1i` | `1` + `i` | FR-015: `i` alone is not a suffix |
| `1i32` | `1i32` | FR-012: compound suffix consumed |
| `1i64` | `1` + `i64` | FR-015: `64` invalid → `i` not consumed → remains as separate tokens |
| `42u` | `42u` | FR-011: bare unsigned |
| `255u8` | `255u8` | FR-012: compound unsigned with width |

### Alternatives considered

- **Ordine inversione `8→16→32`**: rifiutata — `16` verrebbe letto come `1`+`6` con peek
  a singola cifra.
- **Regex-based suffix pattern**: rifiutata — FR-027 vieta regex a runtime.

---

## R7 — `classify_word` interaction con suffissi standalone

### Decision

Nessuna modifica a `classify_word()`. I token tipo `u8`, `i32`, `f32` quando appaiono
come parole standalone (dopo whitespace) continuano a essere classificati come
`TypeU8`, `TypeI32`, `TypeF32` rispettivamente.

### Rationale

Questo è corretto: in `42 u8` lo spazio impedisce l'attacco del suffisso al numero,
quindi `42` → `Numeric`, poi `u8` passa per `scan_identifier_or_keyword` → `classify_word`
→ `TypeU8`. Nessun conflitto con la logica dei suffissi numerici che opera solo su caratteri
contigui.

### Alternatives considered

- Nessuna — il design attuale è già corretto.
