# Data Model: ASCII Keyword Validation Predicate

**Feature**: 004-ascii-keyword-validation  
**Date**: 2026-03-06  
**Status**: Draft

---

## Overview

This document defines the data structures, entities, and validation rules for the ASCII-only keyword validation feature. The feature introduces a single pure function (predicate) with no state, no side effects, and minimal data flow.

---

## Entity Definitions

### 1. `is_ascii_keyword_candidate` Predicate Function

**Type**: Pure function (stateless predicate)  
**Purpose**: Validate that a character sequence contains only ASCII characters in the range U+0021–U+007E

#### Signature

```cpp
[[nodiscard]] constexpr bool 
is_ascii_keyword_candidate(std::string_view identifier) noexcept;
```

#### Parameters

| Parameter | Type | Description | Constraints |
|-----------|------|-------------|-------------|
| `identifier` | `std::string_view` | Character sequence collected by scanner | Non-empty (scanner invariant), UTF-8 encoded |

#### Return Value

| Value | Type | Meaning |
|-------|------|---------|
| `true` | `bool` | All characters in sequence are within U+0021–U+007E; proceed to keyword table lookup |
| `false` | `bool` | At least one character is outside valid ASCII range; skip keyword lookup, emit as Identifier |

#### Validation Rules

| Rule ID | Condition | Action |
|---------|-----------|--------|
| VR-001 | Any byte < 0x21 | Return `false` (control characters, space) |
| VR-002 | Any byte > 0x7E | Return `false` (DEL, non-ASCII UTF-8) |
| VR-003 | All bytes in [0x21, 0x7E] | Return `true` |
| VR-004 | Empty sequence | Assert (debug), return `true` (release) |

#### State Transitions

**None** — Function is pure and stateless. No internal state, no side effects.

#### Performance Characteristics

| Metric | Value | Rationale |
|--------|-------|-----------|
| Time Complexity | O(n) | Single pass over input bytes |
| Space Complexity | O(1) | No heap allocation, stack-only |
| Branch Prediction | High accuracy | ASCII-only input (common case) is highly predictable |

---

### 2. Integration Point: `resolve_identifier_or_keyword`

**Location**: `src/jsav_Lib/lexer/Lexer.cpp` (or equivalent lexer implementation file)  
**Modification**: Insert ASCII validation call before keyword table lookup

#### Before (Existing Logic)

```cpp
Token resolve_identifier_or_keyword(std::string_view identifier) {
    if (keyword_table.contains(identifier)) {
        return Token(keyword_table[identifier]);
    }
    return Token(TokenType::Identifier, identifier);
}
```

#### After (Modified Logic)

```cpp
Token resolve_identifier_or_keyword(std::string_view identifier) {
    // NEW: ASCII validation gate
    if (is_ascii_keyword_candidate(identifier) && keyword_table.contains(identifier)) {
        return Token(keyword_table[identifier]);
    }
    return Token(TokenType::Identifier, identifier);
}
```

#### Data Flow

```
Scanner collects identifier sequence
           ↓
is_ascii_keyword_candidate(identifier)
           ↓
    ┌──────┴──────┐
    │             │
  false          true
    │             │
    ↓             ↓
Skip keyword  Check keyword
lookup        table
    │             │
    └──────┬──────┘
           ↓
    Return Identifier token
    (or Keyword token if matched)
```

---

## Validation Rules (Detailed)

### VR-001: Lower Boundary Check

**Condition**: `byte < 0x21`  
**Examples**:

- U+0020 (space) → `false`
- U+001F (unit separator) → `false`
- U+0000 (NUL) → `false`

**Rationale**: Space is token boundary; control characters are invalid in keywords.

### VR-002: Upper Boundary Check

**Condition**: `byte > 0x7E`  
**Examples**:

- U+007F (DEL) → `false`
- U+0080 (start of UTF-8 continuation) → `false`
- U+00F4 (ô first byte) → `false`
- U+0456 (Cyrillic і, multi-byte) → `false`

**Rationale**: DEL is control character; bytes ≥0x80 indicate multi-byte UTF-8 (non-ASCII).

### VR-003: Valid ASCII Range

**Condition**: `0x21 <= byte <= 0x7E` for all bytes  
**Examples**:

- `if` → `true`
- `class` → `true`
- `variable_name` → `true`
- `my_var!` → `true` (all chars in range, though `!` not valid identifier char — that's a separate check)

**Rationale**: Matches spec FR-003 definition.

### VR-004: Empty Sequence Handling

**Condition**: `identifier.empty()`  
**Action**:

- Debug: `assert(!identifier.empty())`
- Release: Return `true` (vacuously)

**Rationale**: Scanner invariant guarantees non-empty sequence; assertion documents assumption.

---

## Relationships

### Dependency Graph

```text
is_ascii_keyword_candidate (pure function)
         ↓
    (no dependencies — uses only C++23 stdlib)
    
resolve_identifier_or_keyword
         ↓
is_ascii_keyword_candidate
         ↓
    keyword_table (read-only)
```

### Call Hierarchy

```text
Lexer::tokenize()
    ↓
Lexer::scan_identifier()
    ↓
resolve_identifier_or_keyword()
    ↓
is_ascii_keyword_candidate()  ← NEW
```

---

## Constraints

### Compile-Time Constraints

- Function marked `constexpr` — must be evaluable at compile-time
- No dynamic allocation
- No exceptions
- No I/O operations

### Runtime Constraints

- Zero heap allocation (stack-only)
- No synchronization (stateless, thread-safe)
- No external library calls

### API Constraints

- No changes to public headers (`include/jsav/`)
- No changes to function signatures (except internal lexer function)
- No new token variants

---

## Test Data Requirements

### Tier 1: Boundary Values

| Input | Expected | Coverage Target |
|-------|----------|-----------------|
| `!` (U+0021) | `true` | Lower boundary |
| `~` (U+007E) | `true` | Upper boundary |
| ` ` (U+0020) | `false` | Excluded space |
- `` (U+007F) | `false` | DEL character |
| `_` (U+005F) | `true` | Underscore (explicitly valid) |
| NUL (U+0000) | `false` | Control character |
| `Ā` (U+0100) | `false` | Latin Extended |
| `і` (U+0456) | `false` | Cyrillic |
| `` (empty) | `true` | Vacuous truth |

### Tier 2: Integration Cases

| Input | Expected Token | Rationale |
|-------|----------------|-----------|
| `if` | Keyword | ASCII keyword |
| `iffy` | Identifier | Substring rule |
| `If` | Identifier | Case sensitivity |
| `fôr` | Identifier | Non-ASCII circumflex |
| `clàss` | Identifier | Non-ASCII grave accent |
| `іf` | Identifier | Cyrillic lookalike |
| `variável` | Identifier | Unicode identifier |
| `名前` | Identifier | Non-Latin script |
| `my_変数` | Identifier | Mixed ASCII/non-ASCII |
| `iф` | Identifier | Mixed sequence |
| `if\u0000` | Identifier | Control character in keyword-like sequence |

### Tier 3: Performance Benchmark

| Metric | Baseline | Target | Gate |
|--------|----------|--------|------|
| Throughput (lines/sec) | Measure pre-patch | ≥95% of baseline | ≤5% regression |

---

## Security Considerations

### Homoglyph Attack Prevention

**Threat**: Malicious actor uses non-ASCII characters that visually resemble keywords to create confusing or misleading code.

**Example**:

```python
# Looks like: if x > 0:
іf x > 0:  # Cyrillic 'і' instead of ASCII 'i'
    # Malicious code here
```

**Mitigation**: ASCII validation ensures `іf` is tokenized as Identifier, not `if` keyword, causing syntax error rather than silent behavior change.

### Limitations

- Does not prevent all homoglyph attacks (only keyword matching)
- Identifiers can still contain Unicode (by design)
- Visual similarity detection is out of scope

---

## Version History

| Version | Date | Change |
|---------|------|--------|
| 1.0 | 2026-03-06 | Initial design |

---

**Next Steps**:

1. Generate interface contracts (if applicable)
2. Generate quickstart guide
3. Update agent context with new technology
