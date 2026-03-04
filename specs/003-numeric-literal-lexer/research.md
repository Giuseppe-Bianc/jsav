# Research: Complete Numeric Literal Recognition in the Lexer

**Feature**: 003-numeric-literal-lexer
**Date**: 2026-03-03

## R1 — Trailing-dot behavior change: impact on codebase

### Decision

Change the behavior of `scan_numeric_literal()` to include the trailing dot in the numeric token:
`123.` → `Numeric("123.")` instead of `Numeric("123") + Dot(".")`.

### Rationale

The spec FR-003 explicitly requires that `3.` produces a `Numeric` token with text `3.`.
The jsav language does not have method-call syntax on numeric literals (unlike Rust/Kotlin/Swift),
so the original split motivation ("123.toString()") does not apply.
The user explicitly confirmed the change in the task description.

### Alternatives considered

- **Maintain the split** `Numeric + Dot`: rejected because it contradicts FR-003 and the approved
  spec. The jsav lexer does not need method-call support on literals.
- **Make it configurable**: rejected — complexity not justified, YAGNI.

### Impact assessment

- **Existing tests**: One test (`Lexer_AsciiOnlySource_TokenizeCorrectly` in test/tests.cpp L2771)
  indirectly verifies numbers (`"42"` as Numeric), but no test specifically verifies the trailing-dot
  split. The test for `Dot` as operator (`Lexer_AsciiOperators_UnchangedAfterUtf8` L3558) tests
  isolated `"."`, not `"42."`. **No existing tests break** with this change.
- **Header comment**: The doccomment in `Lexer.hpp` (line 38-40) describes the old trailing-dot
  rule and needs updating.
- **Implementation comment**: The comment in `Lexer.cpp` (lines 245-248) explains the old behavior
  and needs removal/replacement.

---

## R2 — Leading-dot entry point in next_token()

### Decision

Add a branch in `next_token()` before `scan_operator_or_punctuation()`: if
`peek_byte() == '.'` and `std::isdigit(peek_byte(1))`, invoke `scan_numeric_literal(start)`.

### Rationale

The current flow in `next_token()` sends any `'.'` directly to `scan_operator_or_punctuation()`
which emits it as `Dot`. To support `.5` as a numeric literal (FR-004, spec G1-B), the check must
occur before the fallthrough to the operator scanner.

### Alternatives considered

- **Modify `scan_operator_or_punctuation()`** to do the lookahead internally: rejected — violates
  separation of concerns, numeric logic must stay in the numeric scanner.
- **Add a generic pre-scanner**: rejected — overengineering for a single check.

### Implementation detail

The isolated dot `'.'` and `'.abc'` continue to reach `scan_operator_or_punctuation()` because
the guard `std::isdigit(peek_byte(1))` fails. No regression for Dot operators.

---

## R3 — Decomposition for Lizard compliance (CCN ≤ 15)

### Decision

Decompose `scan_numeric_literal()` into 4 methods:

1. `scan_numeric_literal()` — orchestrator (G1 + helper calls)
2. `try_scan_exponent()` — G2: non-destructive lookahead for `[eE][+-]?\d+`
3. `try_scan_type_suffix()` — G3: suffix recognition with maximal munch
4. `match_width_suffix()` — utility: matching widths `32`/`16`/`8`

### Rationale

The complete G1→G2→G3 pattern generates at least 20 branches (2 G1 branches, 3 exponent checks,
~8 suffix checks, 3 widths, edge cases). Without decomposition, estimated CCN is ~25-30, well
above the 15 limit. With decomposition:

- `scan_numeric_literal()`: CCN ~8 (digit loop, dot check, entry from dot, calls)
- `try_scan_exponent()`: CCN ~6 (e/E check, sign, digit loop, rollback)
- `try_scan_type_suffix()`: CCN ~8 (d/D, f/F, u/U, i/I branches + width check)
- `match_width_suffix()`: CCN ~4 (32/16/8 comparisons)

### Alternatives considered

- **Monolithic method with `NOLINTBEGIN`**: rejected — the constitution (III, enforcement)
  requires compliance without exceptions for new functions.
- **State machine with transition table**: rejected — over-engineering for 4 linear groups,
  worse readability, not justifiable.

---

## R4 — Non-destructive lookahead for exponent (G2)

### Decision

Use save/restore of `m_pos` and `m_column` for exponent lookahead. If validation fails
(`e` without digits), restore the original position.

### Rationale

The spec FR-009/FR-010 requires that `1e`, `1e+`, `1E-` do NOT consume the marker. The current
approach in `Lexer.cpp` consumes `e`/`E` unconditionally (L254-258) — produces erroneous tokens
like `"1e"` or `"1e+"` instead of separating into `"1"` + `"e"` + `"+"`.

The save/restore pattern is the simplest:

```cpp
const auto saved_pos = m_pos;
const auto saved_col = m_column;
// attempt consumption...
if(/*invalid*/) { m_pos = saved_pos; m_column = saved_col; }
```

### Alternatives considered

- **Lookahead with peek_byte(n)**: possible but requires manual offset counting for
  `e` + optional `+/-` + at least 1 digit, resulting in more fragile code.
- **Advance and generic backtrack**: equivalent to save/restore, more verbose.

### Notes

Saving/restoring `m_line` is not necessary: the exponent cannot cross lines (FR-028), and
characters `e`, `+`, `-`, digits are all single-byte ASCII that do not advance the line.

---

## R5 — Underscore separators

### Decision

Support for underscores as digit separators is NOT in scope for this feature. The reference
pattern is exclusively `(\d+\.?\d*|\.\d+)([eE][+-]?\d+)?([uUfFdD]|[iIuU](?:8|16|32))?`.
Underscores in hash-prefixed literals (`#b`, `#o`, `#x`) remain unchanged as they are handled
by separate scanners.

---

## R6 — Suffix matching: order and maximal munch

### Decision

Suffix matching order (G3): `d`/`D` → `f`/`F` → `u`/`U` → `i`/`I`. For `u`/`U` and `i`/`I`:
attempt compound widths first `32` → `16` → `8` (2-digit comparison before 1-digit).

### Rationale

The spec FR-017 explicitly requires the order `32→16→8` to avoid partial matches. `d`/`D` and
`f`/`F` are always single-char (FR-016: `f` does not form compounds). The order `d→f→u→i` is
not critical (characters are mutually exclusive) but improves code readability.

### Key edge cases verified

| Input | Expected tokens | Rule applied |
|-------|----------------|--------------|
| `5f32` | `5f` + `32` | FR-016: `f` never forms compounds |
| `1u64` | `Numeric("1u64")` | FR-011b: maximal munch — `u` + digits consumes all even if width invalid |
| `1i` | `1` + `i` | FR-015: `i` alone is not a suffix |
| `1i32` | `1i32` | FR-012: compound suffix consumed |
| `1i64` | `Numeric("1i64")` | FR-011b: maximal munch — `i` + digits consumes all even if width invalid |
| `42u` | `42` + `u` | FR-011/FR-015b: `u` alone is NOT a valid suffix |
| `255u8` | `255u8` | FR-012: compound unsigned with width |

### Alternatives considered

- **Reverse order `8→16→32`**: rejected — `16` would be read as `1`+`6` with single-digit peek.
- **Regex-based suffix pattern**: rejected — FR-027 forbids runtime regex.

---

## R7 — `classify_word` interaction with standalone suffixes

### Decision

No changes to `classify_word()`. Tokens like `u8`, `i32`, `f32` when appearing as standalone
words (after whitespace) continue to be classified as `TypeU8`, `TypeI32`, `TypeF32` respectively.

### Rationale

This is correct: in `42 u8` the space prevents the suffix from attaching to the number, so
`42` → `Numeric`, then `u8` goes through `scan_identifier_or_keyword` → `classify_word` →
`TypeU8`. No conflict with numeric suffix logic which operates only on contiguous characters.

### Alternatives considered

- None — the current design is already correct.
