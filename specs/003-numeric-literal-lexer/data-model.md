# Data Model: Complete Numeric Literal Recognition

**Feature**: 003-numeric-literal-lexer
**Date**: 2026-03-03

## Entities

### Numeric Token (existing — behavior modification)

The `Token` entity with `TokenKind::Numeric` already exists in the project. This feature modifies
**what is captured** in the `text` field, not the token structure.

| Field | Type | Description |
|-------|------|-------------|
| `kind` | `TokenKind` | Always `TokenKind::Numeric` for decimal literals |
| `text` | `std::string_view` | View into original source, concatenation of G1+G2+G3 |
| `span` | `SourceSpan` | Position: file, start location (line, col, offset), end location |

**Invariants**:

- `text` contains exactly the bytes consumed from source, without normalization
- `text.size() >= 1` (at least one digit or initial dot)
- The token never crosses line boundaries

### Internal structure of numeric literal (non-persistent entity)

The numeric literal consists of three sequential groups recognized during scanning:

```text
┌─────────────────────────────────────────────────────────────┐
│  G1 (mandatory)     │  G2 (optional)   │  G3 (optional)   │
│                      │                   │                   │
│  Branch A: \d+\.?\d* │  [eE][+-]?\d+     │  [dDfF]           │
│  Branch B: \.\d+     │                   │  [uU](8|16|32)?   │
│                      │                   │  [iI](8|16|32)    │
└─────────────────────────────────────────────────────────────┘
```

### G1 — Numeric part (mandatory)

| Branch | Pattern | Entry condition | Examples |
|--------|---------|-----------------|----------|
| A | `\d+\.?\d*` | first byte is digit `[0-9]` | `42`, `3.`, `3.14`, `007` |
| B | `\.\d+` | first byte is `'.'`, `peek_byte(1)` is digit | `.5`, `.14`, `.0` |

**Validation rules**:

- Branch A: at least one initial digit (guaranteed by check in `next_token()`)
- Branch A trailing dot: `'.'` consumed even without fractional digits (`3.` → `Numeric("3.")`)
- Branch B: at least one digit after the dot (guaranteed by guard in `next_token()`)

### G2 — Exponent (optional)

| Component | Mandatory | Pattern |
|-----------|-----------|---------|
| Marker | Mandatory | `e` \| `E` |
| Sign | Optional | `+` \| `-` |
| Digits | **Mandatory** | `\d+` |

**Validation rules (non-destructive)**:

- If digits are missing after the marker → **rollback**: consume nothing, token ends before `e`/`E`
- If sign present but digits missing → **rollback**: consume neither `e`/`E` nor the sign
- Save/restore of `m_pos` and `m_column` for rollback

### G3 — Type suffix (optional)

| Suffix | Pattern | Semantic type | Forms compounds? |
|--------|---------|---------------|-----------------|
| `d` / `D` | single char | double (64-bit) | No |
| `f` / `F` | single char | float (32-bit) | **Never** (FR-016) |
| `u` / `U` | char + optional width | unsigned | Yes: `u8`, `u16`, `u32` |
| `i` / `I` | char + mandatory width | signed integer | Yes: `i8`, `i16`, `i32` |

**Valid widths**: exclusively `8`, `16`, `32`

**Matching order**: `32` → `16` → `8` (to avoid partial matches of `16` as `1`+`6`)

**Special rules**:

- `f`/`F` never forms compounds: `5f32` → `Numeric("5f")` + `Numeric("32")`
- `u`/`U` alone (without digits): **NOT consumed** (`42u` → `Numeric("42")` + `Identifier("u")`)
- `i`/`I` alone (without digits): **NOT consumed** (`1i` → `Numeric("1")` + `Identifier("i")`)
- `u`/`U` + digits, `i`/`I` + digits: **maximal munch** — consume all even if width is invalid (`1u64` → `Numeric("1u64")`, `1i64` → `Numeric("1i64")`)

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
