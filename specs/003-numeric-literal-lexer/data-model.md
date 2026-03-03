# Data Model: Riconoscimento Completo dei Literal Numerici

**Feature**: 003-numeric-literal-lexer
**Date**: 2026-03-03

## Entities

### Token Numerico (existing — behavior modification)

L'entità `Token` con `TokenKind::Numeric` esiste già nel progetto. Questa feature modifica
**cosa viene catturato** nel campo `text`, non la struttura del token.

| Campo | Tipo | Descrizione |
|-------|------|-------------|
| `kind` | `TokenKind` | Sempre `TokenKind::Numeric` per i literal decimali |
| `text` | `std::string_view` | Vista nel sorgente originale, concatenazione G1+G2+G3 |
| `span` | `SourceSpan` | Posizione: file, start location (line, col, offset), end location |

**Invarianti**:

- `text` contiene esattamente i byte consumati dal sorgente, senza normalizzazione
- `text.size() >= 1` (almeno una cifra o un punto iniziale)
- Il token non attraversa mai confini di riga

### Struttura interna del literal numerico (non entità persistente)

Il literal numerico è composto da tre gruppi sequenziali riconosciuti durante lo scan:

```text
┌─────────────────────────────────────────────────────────────┐
│  G1 (obbligatorio)  │  G2 (opzionale)  │  G3 (opzionale)  │
│                      │                   │                   │
│  Ramo A: \d+\.?\d*   │  [eE][+-]?\d+     │  [dDfF]           │
│  Ramo B: \.\d+       │                   │  [uU](8|16|32)?   │
│                      │                   │  [iI](8|16|32)    │
└─────────────────────────────────────────────────────────────┘
```

### G1 — Parte numerica (obbligatoria)

| Ramo | Pattern | Entry condition | Esempi |
|------|---------|-----------------|--------|
| A | `\d+\.?\d*` | primo byte è digit `[0-9]` | `42`, `3.`, `3.14`, `007` |
| B | `\.\d+` | primo byte è `'.'`, `peek_byte(1)` è digit | `.5`, `.14`, `.0` |

**Regole di validazione**:

- Ramo A: almeno una cifra iniziale (garantito dal check in `next_token()`)
- Ramo A trailing dot: `'.'` consumato anche senza cifre frazionarie (`3.` → `Numeric("3.")`)
- Ramo B: almeno una cifra dopo il punto (garantito dal guard in `next_token()`)

### G2 — Esponente (opzionale)

| Componente | Obbligatorietà | Pattern |
|------------|----------------|---------|
| Marcatore | Obbligatorio | `e` \| `E` |
| Segno | Opzionale | `+` \| `-` |
| Cifre | **Obbligatorio** | `\d+` |

**Regole di validazione (non-distruttivo)**:

- Se dopo il marcatore mancano le cifre → **rollback**: non consumare nulla, il token finisce prima di `e`/`E`
- Se segno presente ma cifre mancanti → **rollback**: non consumare né `e`/`E` né il segno
- Save/restore di `m_pos` e `m_column` per il rollback

### G3 — Suffisso di tipo (opzionale)

| Suffisso | Pattern | Tipo semantico | Forma composti? |
|----------|---------|----------------|-----------------|
| `d` / `D` | singolo char | double (64-bit) | No |
| `f` / `F` | singolo char | float (32-bit) | **Mai** (FR-016) |
| `u` / `U` | char + opzionale width | unsigned | Sì: `u8`, `u16`, `u32` |
| `i` / `I` | char + obbligatoria width | signed integer | Sì: `i8`, `i16`, `i32` |

**Larghezze valide**: esclusivamente `8`, `16`, `32`

**Ordine matching**: `32` → `16` → `8` (per evitare match parziali di `16` come `1`+`6`)

**Regole speciali**:

- `f`/`F` non forma mai composti: `5f32` → `Numeric("5f")` + `Numeric("32")`
- `u`/`U` senza larghezza valida: consumato come bare unsigned (`42u`)
- `i`/`I` senza larghezza valida: **non consumato** (`1i` → `Numeric("1")` + `Identifier("i")`)
- `u`/`U` con larghezza non valida (64, 128): `u` consumato bare, cifre restano (`1u64` → `Numeric("1u")` + `Numeric("64")`)

## State Transitions

```text
                    ┌──────────┐
                    │  Entry   │
                    │next_token│
                    └────┬─────┘
                         │
               ┌─────────┴─────────┐
               │ first byte?       │
               ├──────┬────────────┤
            digit     '.' + digit   other
               │         │           │
               ▼         ▼           ▼
         ┌──────────────────┐   operator/
         │  scan_numeric_   │   punct
         │  literal(start)  │
         └────────┬─────────┘
                  │
         ┌────────┴────────┐
         │ G1: consume     │
         │ integer + opt.  │
         │ fractional part │
         └────────┬────────┘
                  │
         ┌────────┴────────┐
         │ try_scan_       │──── rollback if invalid
         │ exponent()      │
         └────────┬────────┘
                  │
         ┌────────┴────────┐
         │ try_scan_type_  │──── returns without consuming
         │ suffix()        │     if no valid suffix
         └────────┬────────┘
                  │
                  ▼
         ┌──────────────┐
         │  make_token   │
         │  (Numeric,    │
         │   text, start)│
         └──────────────┘
```

## Relationships

```text
Lexer::next_token()
  ├── [digit entry]  → Lexer::scan_numeric_literal()
  │                       ├── Lexer::try_scan_exponent()
  │                       └── Lexer::try_scan_type_suffix()
  │                               └── Lexer::match_width_suffix()
  ├── [dot+digit entry] → Lexer::scan_numeric_literal()  (same path)
  └── [dot alone]       → Lexer::scan_operator_or_punctuation() → Dot
```

## Method Signatures (new private helpers)

```cpp
/// Attempt to consume an exponent group [eE][+-]?\d+.
/// Uses save/restore: if the exponent is incomplete, restores position
/// and returns without consuming anything.
void try_scan_exponent();

/// Attempt to consume a type suffix (d/D, f/F, u/U[width], i/I<width>).
/// Returns without consuming if no valid suffix is found at current position.
void try_scan_type_suffix();

/// Check if the next 1-2 bytes form a valid width suffix (32, 16, or 8).
/// If matched, advance past the width bytes and return true.
/// Otherwise return false without advancing.
[[nodiscard]] bool match_width_suffix();
```
