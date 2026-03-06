# Quickstart Guide: ASCII Keyword Validation Feature

**Feature**: 004-ascii-keyword-validation  
**Date**: 2026-03-06  
**Audience**: Compiler developers, lexer maintainers  
**Prerequisites**: C++23 knowledge, familiarity with jsav lexer architecture

---

## Overview

This feature adds ASCII-only validation to keyword matching in the jsav compiler lexer. It prevents non-ASCII homoglyph sequences (e.g., `fôr`, `clàss`, `іf`) from being recognized as keywords, while preserving full Unicode identifier support.

**Key Benefits**:

- ✅ Prevents homoglyph-based confusion attacks
- ✅ Zero new dependencies
- ✅ <5% performance impact
- ✅ 100% backward compatible for ASCII keywords
- ✅ Preserves Unicode identifier support

---

## What Changed

### Before (Vulnerable)

```cpp
// Source: fôr (with non-ASCII 'ô')
// Lexer behavior: Recognized as 'for' keyword ❌
for (int i = 0; i < 10; ++i) {  // Standard ASCII 'for'
    // ...
}

fôr (int i = 0; i < 10; ++i) {  // Non-ASCII 'fôr' — looked like 'for' but wasn't
    // This was incorrectly tokenized as 'for' keyword
    // Potential security/confusion issue
}
```

### After (Protected)

```cpp
// Source: fôr (with non-ASCII 'ô')
// Lexer behavior: Recognized as Identifier token ✅
for (int i = 0; i < 10; ++i) {  // Standard ASCII 'for' → Keyword token
    // ...
}

fôr (int i = 0; i < 10; ++i) {  // Non-ASCII 'fôr' → Identifier token
    // Now correctly tokenized as identifier
    // Parser will reject it (expected behavior)
}
```

---

## Implementation Details

### New Function

```cpp
/// @brief Validates that an identifier candidate contains only ASCII characters
///        in the range U+0021–U+007E (printable ASCII excluding space).
///
/// @param identifier The character sequence collected by the scanner.
/// @return true if all characters are in valid ASCII range; false otherwise.
///
/// @note Function is pure, stateless, and constexpr-evaluable.
/// @note Empty sequence triggers assert in debug builds (scanner invariant).
///
/// @code
/// // Usage in resolve_identifier_or_keyword
/// if (is_ascii_keyword_candidate(identifier) && 
///     keyword_table.contains(identifier)) {
///     return Token(keyword_table[identifier]);
/// }
/// @endcode
[[nodiscard]] constexpr bool 
is_ascii_keyword_candidate(std::string_view identifier) noexcept;
```

### Valid ASCII Range

| Range | Status | Examples |
|-------|--------|----------|
| U+0021–U+007E | ✅ Valid | `!` through `~` (printable ASCII) |
| U+005F (`_`) | ✅ Valid | Underscore (explicitly allowed) |
| U+0020 (space) | ❌ Invalid | Token boundary |
| U+0000–U+001F | ❌ Invalid | Control characters |
| U+007F (DEL) | ❌ Invalid | Control character |
| U+0080+ | ❌ Invalid | All non-ASCII (UTF-8 multi-byte) |

---

## How It Works

### Tokenization Flow

```text
1. Scanner collects identifier sequence
         ↓
2. is_ascii_keyword_candidate(identifier)
         ↓
    ┌────┴────┐
    │         │
  false      true
    │         │
    ↓         ↓
3a. Skip    3b. Check keyword table
    keyword     │
    lookup      │
    │           ├─────┬─────┐
    │           │     │     │
    ↓           ↓     ↓     ↓
4a. Return  4b.   4c.   4d. Return
    Identifier  Match  No Match
                Keyword  Identifier
```

### Example Scenarios

| Input | Step 2 Result | Step 3b | Final Token |
|-------|---------------|---------|-------------|
| `if` | `true` | Match | `TokenType::If` |
| `iffy` | `true` | No Match | `TokenType::Identifier` |
| `fôr` | `false` | Skipped | `TokenType::Identifier` |
| `іf` | `false` | Skipped | `TokenType::Identifier` |
| `variável` | `false` | Skipped | `TokenType::Identifier` |
| `If` | `true` | No Match (case) | `TokenType::Identifier` |

---

## Testing

### Run Tests

```bash
# Navigate to build directory
cd build

# Build tests
ninja tests

# Run all lexer tests
ctest -R "unittests.*Lexer" --output-on-failure

# Run specific ASCII validation tests
ctest -R "unittests.*ASCII" --output-on-failure
```

### Test Coverage

**Location**: `test/tests.cpp`

**Test Tiers**:

1. **Tier 1**: Predicate unit tests (boundary values)
2. **Tier 2**: Full tokenization integration tests
3. **Tier 3**: Performance benchmark

**Coverage Requirement**: 100% branch coverage on `is_ascii_keyword_candidate`

---

## Performance

### Expected Impact

| Metric | Baseline | Post-Patch | Change |
|--------|----------|------------|--------|
| Throughput (lines/sec) | 100% | ≥95% | ≤5% regression |
| Memory | Unchanged | Unchanged | 0% |
| Compile Time | Unchanged | Unchanged | 0% |

### Benchmark Command

```bash
# Run performance benchmark (Tier 3 tests)
ctest -R "unittests.*Benchmark" --output-on-failure

# Compare before/after results
# Attach benchmark delta table to PR
```

---

## Security Considerations

### Threat Mitigated

**Homoglyph Attack**: Using non-ASCII characters that visually resemble keywords to create confusing or malicious code.

**Example**:

```python
# Visual appearance:
if x > 0:
    # Do something

# Actual source (Cyrillic 'і'):
іf x > 0:  # Tokenized as Identifier, not If keyword
    # Parser error: expected keyword, got identifier
```

### Limitations

- Does NOT prevent all homoglyph attacks (only keyword matching)
- Identifiers can still contain Unicode (by design)
- Visual similarity detection is out of scope

---

## Troubleshooting

### Problem: Existing tests fail

**Solution**: Verify test expectations. ASCII keywords (`if`, `for`, `class`) should still tokenize as keywords. If tests fail, check:

- Test input encoding (must be UTF-8)
- Test case sensitivity (keywords are case-sensitive)
- Test substring handling (`iffy` is identifier, not keyword)

### Problem: Performance regression >5%

**Solution**: Profile lexer to identify bottleneck. Check:

- Compiler optimization level (must be Release or RelWithDebInfo)
- Branch prediction (ASCII-only input should be highly predictable)
- Unnecessary copies (ensure `std::string_view` is passed by reference)

### Problem: Non-ASCII identifiers broken

**Solution**: This indicates a bug. Non-ASCII identifiers should still work. Verify:

- ASCII validation runs BEFORE keyword lookup only
- Non-ASCII sequences skip keyword table (return `false`)
- Identifier collection logic is unchanged

---

## Migration Guide

### For Compiler Developers

**No action required** — Feature is backward compatible.

- Existing code continues to work unchanged
- No API changes
- No signature changes
- No new dependencies

### For Language Users

**No action required** — Feature is transparent.

- Standard ASCII keywords work as before
- Unicode identifiers work as before
- Non-ASCII keyword lookalikes now correctly fail (improved error messages)

---

## Related Documentation

- **Specification**: `specs/004-ascii-keyword-validation/spec.md`
- **Research**: `specs/004-ascii-keyword-validation/research.md`
- **Data Model**: `specs/004-ascii-keyword-validation/data-model.md`
- **Contracts**: `specs/004-ascii-keyword-validation/contracts/lexer-keyword-tokenization.md`
- **Implementation Plan**: `specs/004-ascii-keyword-validation/plan.md`

---

## FAQ

**Q: Why U+0021–U+007E instead of U+0020–U+007E?**  
A: Space (U+0020) is a token boundary, not part of identifiers. Excluding it prevents edge cases.

**Q: Does this break Unicode identifiers?**  
A: No. Unicode identifiers are still valid. Only keyword matching is restricted to ASCII.

**Q: What about keywords with underscores (e.g., `__declspec`)?**  
A: Underscore (U+005F) is within U+0021–U+007E range. No change in behavior.

**Q: Can I disable this feature?**  
A: No. Feature is always-on for security. No configuration needed.

**Q: Will this affect compilation speed?**  
A: Negligible impact (<5% regression target). Predicate is O(n) and highly optimized.

---

**Last Updated**: 2026-03-06  
**Maintainer**: jsav compiler team
