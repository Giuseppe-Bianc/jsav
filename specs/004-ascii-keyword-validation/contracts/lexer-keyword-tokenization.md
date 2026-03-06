# Interface Contract: Lexer Keyword Tokenization

**Feature**: 004-ascii-keyword-validation  
**Date**: 2026-03-06  
**Status**: Draft  
**Interface Type**: Lexer Tokenization Behavior (Internal API)

---

## Overview

This document defines the interface contract for lexer keyword tokenization after the ASCII validation feature is implemented. The contract specifies the expected behavior for consumers of the lexer (parser, semantic analyzer, test suite).

---

## Contract Scope

**What Changes**:

- Keyword matching now requires ASCII-only character sequences
- Non-ASCII sequences that resemble keywords are tokenized as Identifiers

**What Does NOT Change**:

- Function signatures
- Token variant types
- Public API headers
- Scanner logic
- Identifier validity rules
- Case-sensitivity rules

---

## Behavioral Contract

### Contract ID: LEX-KW-001 — ASCII Keyword Recognition

**Precondition**: Source code contains standard ASCII keywords (`if`, `for`, `class`, `while`, `return`, etc.)

**Postcondition**: Each keyword is tokenized as its corresponding `Keyword` token type

**Invariant**: Keywords are case-sensitive and must be lowercase (e.g., `if` is recognized, `If` and `IF` are not)

**Examples**:

| Input | Expected Token Type | Notes |
|-------|---------------------|-------|
| `if` | `TokenType::If` | Standard ASCII keyword |
| `for` | `TokenType::For` | Standard ASCII keyword |
| `class` | `TokenType::Class` | Standard ASCII keyword |
| `while` | `TokenType::While` | Standard ASCII keyword |
| `return` | `TokenType::Return` | Standard ASCII keyword |
| `If` | `TokenType::Identifier` | Case mismatch |
| `FOR` | `TokenType::Identifier` | Case mismatch |
| `iffy` | `TokenType::Identifier` | Substring, not exact match |

**Verification**: Existing test cases must pass without modification (SC-001).

---

### Contract ID: LEX-KW-002 — Non-ASCII Homoglyph Rejection

**Precondition**: Source code contains keyword-like sequences with non-ASCII characters

**Postcondition**: Sequence is tokenized as `Identifier` token, NOT as `Keyword` token

**Invariant**: Validation operates on full Unicode codepoints, not raw bytes

**Examples**:

| Input | Expected Token Type | Unicode Composition |
|-------|---------------------|---------------------|
| `fôr` | `TokenType::Identifier` | `f` + `ô` (U+00F4) + `r` |
| `clàss` | `TokenType::Identifier` | `c` + `l` + `à` (U+00E0) + `s` + `s` |
| `іf` | `TokenType::Identifier` | `і` (U+0456 Cyrillic) + `f` |
| `whilе` | `TokenType::Identifier` | `whil` + `е` (U+0435 Cyrillic) |
| `returп` | `TokenType::Identifier` | `retur` + `п` (U+043F Cyrillic) |

**Verification**: New test cases must verify 100% detection rate (SC-002).

---

### Contract ID: LEX-KW-003 — Unicode Identifier Preservation

**Precondition**: Source code contains identifiers with Unicode characters

**Postcondition**: Identifiers are tokenized as `Identifier` token (unchanged behavior)

**Invariant**: Unicode identifier support is preserved; only keyword matching is affected

**Examples**:

| Input | Expected Token Type | Notes |
|-------|---------------------|-------|
| `variável` | `TokenType::Identifier` | Latin extended |
| `名前` | `TokenType::Identifier` | Japanese Kanji |
| `αβγ` | `TokenType::Identifier` | Greek letters |
| `переменная` | `TokenType::Identifier` | Cyrillic text |
| `my_変数` | `TokenType::Identifier` | Mixed ASCII/non-ASCII |
| `value_α` | `TokenType::Identifier` | Mixed ASCII/non-ASCII |

**Verification**: Unicode identifier test cases must pass (SC-003).

---

### Contract ID: LEX-KW-004 — Mixed Sequence Handling

**Precondition**: Source code contains sequences mixing ASCII and non-ASCII characters in keyword-like patterns

**Postcondition**: Sequence is tokenized as `Identifier` token

**Invariant**: Any non-ASCII character invalidates keyword matching

**Examples**:

| Input | Expected Token Type | Unicode Composition |
|-------|---------------------|---------------------|
| `iф` | `TokenType::Identifier` | ASCII `i` + Cyrillic `ф` (U+0444) |
| `if\u0000` | `TokenType::Identifier` | ASCII + NUL control character |
| `for\u0001` | `TokenType::Identifier` | ASCII + SOH control character |
| `wörld` | `TokenType::Identifier` | ASCII + non-ASCII |
| `clаss` | `TokenType::Identifier` | ASCII `cl` + Cyrillic `а` (U+0430) + ASCII `ss` |

**Verification**: Edge case test cases must cover mixed sequences.

---

### Contract ID: LEX-KW-005 — Token Boundary Preservation

**Precondition**: Source code contains non-ASCII keyword lookalikes followed by whitespace or punctuation

**Postcondition**: Tokenization produces correct token sequence (identifier + whitespace/operator)

**Invariant**: Token boundaries are preserved; no merging or splitting errors

**Examples**:

| Input | Expected Tokens | Notes |
|-------|-----------------|-------|
| `fôr ` | `Identifier("fôr")` + `Whitespace` | Trailing space |
| `fôr;` | `Identifier("fôr")` + `Semicolon` | Trailing punctuation |
| `fôr(` | `Identifier("fôr")` + `LParen` | Trailing operator |
| `clàss\n` | `Identifier("clàss")` + `Newline` | Trailing newline |

**Verification**: Boundary tokenization tests must pass.

---

## Performance Contract

### Contract ID: LEX-PERF-001 — Throughput Guarantee

**Precondition**: Lexer processes representative source corpus

**Postcondition**: Throughput regression ≤5% compared to baseline (pre-modification)

**Measurement**:

- Baseline: Measure before feature implementation
- Post-patch: Measure after feature implementation
- CI Gate: Fail build if regression >5%

**Rationale**: ASCII validation adds O(n) scan per identifier; must remain within acceptable performance budget.

---

## Error Handling Contract

### Contract ID: LEX-ERR-001 — No New Error Conditions

**Precondition**: ASCII validation predicate executes

**Postcondition**: No exceptions thrown, no error codes returned

**Invariant**: Function is `noexcept`; errors are impossible by design

**Behavior**:

- Invalid input (non-ASCII) → Return `false`, treat as identifier (graceful degradation)
- Empty input → Assert (debug), return `true` (release)
- No error propagation to caller

---

## Compatibility Contract

### Contract ID: LEX-COMPAT-001 — Backward Compatibility

**Precondition**: Existing code using lexer

**Postcondition**: All existing test cases pass without modification

**Invariant**: 100% backward compatibility for ASCII keywords (SC-001)

**Scope**:

- Public API: Unchanged
- Token types: Unchanged
- Function signatures: Unchanged
- Behavior for ASCII input: Unchanged

---

### Contract ID: LEX-COMPAT-002 — Forward Compatibility

**Precondition**: Future extensions to keyword table

**Postcondition**: New keywords automatically benefit from ASCII validation

**Invariant**: Validation is generic; no keyword-specific logic

**Extension Mechanism**:

- Add new keyword to keyword table → ASCII validation applies automatically
- No changes needed to validation logic
- Generic validation design ensures extensibility without code modifications

---

## Version History

| Version | Date | Change |
|---------|------|--------|
| 1.0 | 2026-03-06 | Initial contract definition |

---

## Compliance Verification

**Test Coverage**:

- Unit tests: `test/tests.cpp` — Tier 1 (boundary values)
- Integration tests: `test/tests.cpp` — Tier 2 (full tokenization)
- Performance tests: `test/tests.cpp` — Tier 3 (benchmark)

**CI Gates**:

- ✅ All existing tests pass (LEX-COMPAT-001)
- ✅ All new tests pass (LEX-KW-002, LEX-KW-003)
- ✅ Performance regression ≤5% (LEX-PERF-001)
- ✅ 100% branch coverage on modified code (SC-005)

---

**Next Steps**:

1. Generate quickstart guide
2. Update agent context
3. Proceed to Phase 2 (task generation)
