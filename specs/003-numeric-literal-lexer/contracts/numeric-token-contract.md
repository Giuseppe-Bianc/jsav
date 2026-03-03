# Contracts: Numeric Literal Lexer Interface

**Feature**: 003-numeric-literal-lexer
**Date**: 2026-03-03

## Public Interface (invariato)

Il lexer jsav non espone nuove interfacce pubbliche con questa feature. L'interfaccia pubblica
esistente (`Lexer::tokenize()` e `Lexer::next_token()`) rimane invariata nella firma.

Il **contratto comportamentale** cambia per i token di tipo `TokenKind::Numeric`:

## Contratto: Token Numeric — Pattern G1→G2→G3

### Input

Qualsiasi sequenza di byte nel sorgente che inizia con:

- una cifra decimale `[0-9]`, oppure
- un punto `'.'` immediatamente seguito da una cifra decimale `[0-9]`

### Output

Un singolo `Token` con:

- `kind == TokenKind::Numeric`
- `text` = sottostringa del sorgente corrispondente alla maximal munch di G1(+G2)(+G3)
- `span.start` = posizione del primo byte consumato
- `span.end` = posizione dopo l'ultimo byte consumato

### Postcondizioni

1. **Testo preservato**: `text` contiene esattamente i byte del sorgente, senza normalizzazione
2. **Maximal munch**: nessun suffisso o esponente valido è rimasto non-consumato
3. **No overconsumption**: caratteri che non formano parte valida del pattern non sono consumati
4. **Linear time**: lo scan procede in una singola passata, O(n) rispetto alla lunghezza del literal
5. **Single line**: il token non attraversa confini di riga

### Contratto specifico per G2 (esponente)

| Condizione | `e`/`E` consumato? | Segno consumato? | Risultato |
|------------|---------------------|------------------|-----------|
| `eE` + cifre | Sì | N/A | Token include esponente |
| `eE` + segno + cifre | Sì | Sì | Token include esponente con segno |
| `eE` + nessuna cifra | **No** | **No** | Token termina prima di `e`/`E` |
| `eE` + segno + nessuna cifra | **No** | **No** | Token termina prima di `e`/`E` |

### Contratto specifico per G3 (suffisso)

| Input suffix | Consumato nel token? | Motivo |
|--------------|---------------------|--------|
| `d` / `D` | Sì (1 byte) | Suffisso double valido |
| `f` / `F` | Sì (1 byte) | Suffisso float valido — **mai** composto |
| `u` / `U` + width valida | Sì (1+width bytes) | Suffisso composto unsigned |
| `u` / `U` senza width | Sì (1 byte) | Suffisso bare unsigned |
| `i` / `I` + width valida | Sì (1+width bytes) | Suffisso composto signed |
| `i` / `I` senza width | **No** (0 bytes) | `i` da solo non è suffisso valido |

### Non-regressione

I seguenti comportamenti esistenti sono preservati:

- `42` → `Numeric("42")`
- `3.14` → `Numeric("3.14")`
- `#b1010` → `Binary("#b1010")` (scanner separato, invariato)
- `#xFF` → `Hexadecimal("#xFF")` (scanner separato, invariato)
- `"hello"` → `StringLiteral('"hello"')` (invariato)
- `.` isolato → `Dot(".")` (invariato)
- `.abc` → `Dot(".")` + `Identifier("abc")` (invariato)

### Cambio di comportamento (breaking change intenzionale)

| Input | Vecchio output | Nuovo output |
|-------|---------------|--------------|
| `3.` | `Numeric("3")` + `Dot(".")` | `Numeric("3.")` |
| `42.` | `Numeric("42")` + `Dot(".")` | `Numeric("42.")` |
| `.5` | `Dot(".")` + `Numeric("5")` | `Numeric(".5")` |
| `1e10` | `Numeric("1e10")` (già corretto*) | `Numeric("1e10")` |
| `1e` | `Numeric("1e")` (BUG) | `Numeric("1")` + `Identifier("e")` |
| `1e+` | `Numeric("1e+")` (BUG) | `Numeric("1")` + `Identifier("e")` + `Plus("+")` |
| `42u` | `Numeric("42u")` | `Numeric("42")` + `Identifier("u")` (`u` da solo non è suffisso valido) |
| `1i32` | `Numeric("1i32")` | `Numeric("1i32")` (invariato) |
| `42f` | non riconosciuto come suffix | `Numeric("42f")` (`f` è suffisso singolo valido) |
| `42d` | non riconosciuto come suffix | `Numeric("42d")` (`d` è suffisso singolo valido) |

*Il vecchio scanner consumava `e` incondizionatamente; per `1e10` il risultato era accidentalmente
corretto, ma per `1e` e `1e+` produceva token errati.
