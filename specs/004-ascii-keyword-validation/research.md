# Research Report: ASCII-Only Keyword Validation

**Feature**: 004-ascii-keyword-validation  
**Date**: 2026-03-06  
**Status**: Complete

---

## Research Task 1: Unicode Codepoint Iteration in C++23

**Question**: How to correctly iterate over Unicode codepoints (not bytes) in C++23 for UTF-8 encoded source files?

### Decision

Use `std::string_view` with manual UTF-8 decoding, or leverage existing UTF-8 utilities in the codebase. The predicate will iterate using `char8_t` pointers and decode UTF-8 sequences to extract full codepoints.

### Rationale

- C++23 provides `char8_t` type for UTF-8 character representation
- `std::string` stores UTF-8 on most platforms but iteration via `char` gives bytes, not codepoints
- Manual UTF-8 decoding is straightforward for scalar range checking (U+0021–U+007E)
- No external Unicode library needed for simple scalar comparison

### Alternatives Considered

1. **ICU Library**: Full Unicode support but heavy dependency (~10MB), overkill for scalar range check
2. **unicode-segmentation (Rust-style)**: Not available in C++ stdlib; would require third-party
3. **`std::u8string` + `std::u8string_view`**: Requires C++20/23 support; MSVC support still maturing
4. **Manual UTF-8 decoding** (CHOSEN): Minimal code (~15 lines), zero dependencies, full control

### Implementation Pattern

```cpp
[[nodiscard]] constexpr bool is_ascii_keyword_candidate(std::string_view identifier) noexcept {
    const auto* data = reinterpret_cast<const unsigned char*>(identifier.data());
    const auto size = identifier.size();
    std::size_t pos = 0;
    
    while (pos < size) {
        const auto byte = data[pos];
        
        // Check if byte is in valid ASCII range U+0021–U+007E
        if (byte < 0x21 || byte > 0x7E) {
            return false;
        }
        
        // ASCII bytes are single-byte in UTF-8
        ++pos;
    }
    
    return true;
}
```

**Key Insight**: For the specific range U+0021–U+007E, UTF-8 encoding is identical to ASCII — all characters in this range are single-byte. Multi-byte UTF-8 sequences always have the high bit set (≥0x80), so checking `byte < 0x21 || byte > 0x7E` automatically rejects all non-ASCII codepoints without explicit UTF-8 decoding.

---

## Research Task 2: ASCII Validation Range Boundaries

**Question**: What is the authoritative ASCII range for keyword validation? Spec contains conflicting definitions (FR-003 variants).

### Decision
**Authoritative Range**: U+0021–U+007E (printable ASCII excluding space U+0020)  
**Underscore**: U+005F explicitly allowed (already within range)  
**Excluded**:

- U+0000–U+0020 (control characters + space)
- U+007F (DEL)
- U+0080+ (all non-ASCII, including extended ASCII, Unicode)

### Rationale

From spec clarifications (Session 2026-03-05):

- "ASCII stampabile: intervallo U+0020–U+007E e underscore (U+005F)"
- "Selected validation range option → Option C — use U+0021–U+007E (printable ASCII excluding space U+0020)"

Space (U+0020) excluded because:

- Keywords cannot contain spaces (lexical boundary)
- Space is whitespace token, not part of identifier/keyword

### Alternatives Considered

1. **U+0020–U+007E** (includes space): Rejected — space breaks token boundaries
2. **U+0020–U+007F** (includes DEL): Rejected — DEL is control character
3. **U+0021–U+007E** (CHOSEN): Matches spec clarification, excludes space and DEL

### Boundary Test Cases

| Codepoint | Character | Expected | Rationale |
|-----------|-----------|----------|-----------|
| U+0020 | space | ❌ false | Token boundary, not part of identifier |
| U+0021 | `!` | ✅ true | Lower boundary (inclusive) |
| U+005F | `_` | ✅ true | Valid identifier character, within range |
| U+007E | `~` | ✅ true | Upper boundary (inclusive) |
| U+007F | DEL | ❌ false | Control character, above range |
| U+0000 | NUL | ❌ false | Control character |
| U+00F4 | ô | ❌ false | Non-ASCII (multi-byte UTF-8) |
| U+0456 | і (Cyrillic) | ❌ false | Non-ASCII (multi-byte UTF-8) |

---

## Research Task 3: Empty Sequence Handling

**Question**: What happens when scanner encounters empty character sequence? Should predicate handle it?

### Decision

**Behavior**: Return `true` (vacuously true — all zero characters are ASCII)  
**Documentation**: Add assertion/comment that scanner invariant guarantees non-empty sequence  
**Assertion**: `assert(!identifier.empty())` in debug builds

### Rationale

- Spec clarification: "Empty sequence edge case — verify the scanner invariant that an empty sequence is never passed to the predicate"
- Scanner logic collects at least one character before calling `resolve_identifier_or_keyword`
- Empty sequence cannot occur in normal operation
- Returning `true` is safe: empty string won't match any keyword in table anyway

### Alternatives Considered

1. **Return `false`**: Would treat empty as identifier — but empty identifier is invalid
2. **Throw exception**: Overkill for impossible condition
3. **Assert + return `true`** (CHOSEN): Documents invariant, safe fallback, zero runtime cost in release

### Implementation

```cpp
[[nodiscard]] constexpr bool is_ascii_keyword_candidate(std::string_view identifier) noexcept {
    assert(!identifier.empty() && "Scanner invariant: identifier sequence must not be empty");
    
    // ... validation logic ...
}
```

---

## Research Task 4: Performance Impact Analysis

**Question**: What is the performance impact of adding O(n) scan per identifier/keyword?

### Decision

**Expected Impact**: <2% throughput regression (well within 5% gate)  
**Mitigation**: Predicate is branchless for ASCII-only input (common case), highly predictable

### Rationale

- Predicate adds one additional pass over identifier characters
- For ASCII identifiers (99%+ of real code): branch predictor handles perfectly (always taken)
- For non-ASCII identifiers: early exit on first non-ASCII byte (fast fail)
- Modern CPUs: branch prediction + SIMD potential for future optimization

### Benchmarking Strategy

1. **Baseline**: Measure lexer throughput on representative corpus before change
2. **Post-patch**: Measure same corpus after change
3. **CI Gate**: Fail if regression >5%
4. **Recording**: Attach before/after table to PR

### Alternatives Considered

1. **Cache validation result**: Not worth complexity — identifiers typically short (<20 chars)
2. **Integrate into scanner loop**: Would complicate scanner logic; separate function is cleaner
3. **Separate pass** (CHOSEN): Clean separation, minimal complexity, acceptable performance

---

## Research Task 5: Best Practices for Property-Based Testing

**Question**: Should we add property-based testing (e.g., Catch2 generators, hypothesis) for boundary fuzzing?

### Decision

**Use Catch2 Generators** (already available in project via Catch2)  
**Do NOT add**: proptest (Rust), hypothesis (Python) — wrong language, unnecessary dependency

### Rationale

- Catch2 v3.13.0 includes generator support for property-based testing
- Zero new dependencies (aligns with Constitution Principle V)
- Sufficient for boundary value generation and random codepoint testing
- Dev-only feature; no impact on production binary

### Implementation Pattern

```cpp
TEST_CASE("ASCII validation boundary fuzzing", "[lexer][ascii_validation][property]") {
    // Generate random codepoints and verify predicate behavior
    GENERATE(take(1000, random<std::uint32_t>(0x0000, 0x10FFFF)));
    
    const auto codepoint = static_cast<char32_t>(value);
    const auto expected = (codepoint >= 0x21 && codepoint <= 0x7E);
    const auto input = to_utf8_string(codepoint);
    
    REQUIRE(is_ascii_keyword_candidate(input) == expected);
}
```

---

## Research Task 6: Identifier Validity Rules Preservation

**Question**: Does ASCII validation interfere with existing identifier validity rules (start/continuation characters)?

### Decision

**No interference**: ASCII validation runs AFTER identifier collection, BEFORE keyword lookup  
**Preservation**: All existing identifier rules (Unicode start/continuation) remain unchanged

### Rationale

From spec:

- FR-007: "The lexer MUST preserve all existing identifier validity rules"
- FR-001: "The lexer MUST continue to read and return identifiers exactly as before"

Flow:

1. Scanner collects identifier sequence (Unicode allowed) → unchanged
2. Call `is_ascii_keyword_candidate` → new step
3. If `false`: emit as Identifier token → unchanged for non-ASCII
4. If `true`: proceed to keyword table lookup → unchanged for ASCII

### Alternatives Considered

1. **Validate during scanning**: Would complicate scanner; reject early
2. **Post-collection validation** (CHOSEN): Clean separation, minimal change to existing logic
3. **Keyword table with Unicode entries**: Overly complex; keywords are ASCII-only by design

---

## Consolidated Findings Summary

| Unknown | Decision | Impact |
|---------|----------|--------|
| Unicode iteration | Byte-level check sufficient for U+0021–U+007E | Simple implementation, no UTF-8 decoding needed |
| ASCII range | U+0021–U+007E (exclude space, DEL) | Matches spec clarification |
| Empty sequence | Assert + return `true` | Documents invariant, safe fallback |
| Performance | Expected <2% regression | Well within 5% CI gate |
| Property testing | Use Catch2 generators | Zero new dependencies |
| Identifier rules | No interference (validation before keyword lookup only) | Full backward compatibility |

---

## References

- **Spec**: `specs/004-ascii-keyword-validation/spec.md` (FR-003, clarifications)
- **C++23 Standard**: `char8_t`, UTF-8 encoding rules
- **Catch2 Documentation**: Generator framework for property-based testing
- **UTF-8 Encoding**: RFC 3629 — single-byte range U+0000–U+007F

---

**Status**: ✅ All NEEDS CLARIFICATION items resolved. Proceed to Phase 1.
