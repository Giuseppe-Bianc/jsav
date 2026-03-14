# Data Model: Unicode-Aware Error Reporter

**Date**: 2026-03-14
**Status**: Draft
**Traceability**: All entities traced to spec.md requirements

---

## Overview

### Purpose

This document defines the **data model** for the Unicode-Aware Error Reporter feature. The data model consists of:
- **Configuration structures** (`ErrorDisplayConfig`)
- **Function interfaces** (`UnicodeColumn` module)
- **Integration points** (modifications to existing classes)

### Scope

**Included**:
- New `UnicodeColumn` module (public API and implementation)
- `ErrorReporter` extensions (configuration, constructor)
- `LineTracker` extension (source accessor)
- Validation rules and constraints

**Excluded**:
- Persistent data storage (not applicable — in-memory processing only)
- State machines (not applicable — stateless utility functions)
- Database schemas (not applicable — no database)

### Design Principles

| Principle | Application |
|-----------|-------------|
| **Separation of Concerns** | Display logic isolated in `UnicodeColumn`, not mixed with lexical analysis |
| **Zero Heap Allocation** | All functions operate on `std::string_view` (no copies, no allocations) |
| **Const-Correctness** | All parameters are `const` (read-only via `std::string_view`) |
| **Error Propagation** | `std::expected` for recoverable errors, `LERROR` for diagnostics |
| **Backward Compatibility** | Existing APIs preserved, new functionality additive |

---

## Entity Definitions

### Entity 1: ErrorDisplayConfig

**Entity ID**: ENT-001
**Kind**: Struct (configuration data)
**Visibility**: Public (`include/jsav/error/UnicodeColumn.hpp`)

#### Source Requirements

- FR-018: Tab stops configurable (default 8 columns)
- NFR-003: ANSI color optional with fallback

#### Purpose

Configuration for tab expansion and ANSI color output in error markers. Passed to `ErrorReporter` constructor for per-instance configuration.

#### Definition

```cpp
namespace jsv {

/// @brief Configuration for tab expansion and ANSI color output in error markers.
///
/// This struct controls the visual appearance of error markers (carets) in
/// compiler error messages. It allows customization of tab stop width and
/// enables/disables ANSI color escape codes.
///
/// @threadsafe Yes (immutable after construction)
/// @since 0.5.0 (feature 005-fix-unicode-error-reporting)
struct ErrorDisplayConfig {
    /// @brief Tab stop width in columns (default: 8).
    ///
    /// Controls how tab characters (U+0009) are expanded when calculating
    /// visual column positions. The default value of 8 matches GCC, Clang,
    /// and MSVC behavior.
    ///
    /// @note Must be > 0. Division by zero occurs if tab_stop_width == 0.
    /// @see FR-018, FR-010
    std::size_t tab_stop_width = 8;

    /// @brief Enable ANSI color escape codes for error markers (default: false).
    ///
    /// When true, error markers (carets) are output with ANSI color escape
    /// codes (red color). When false, plain '^' characters are used.
    ///
    /// @note Auto-detected by detect_ansi_color() based on environment variables.
    /// @note Respects NO_COLOR environment variable (overrides to false).
    /// @see NFR-003, detect_ansi_color()
    bool ansi_color = false;
};

} // namespace jsv
```

#### Fields

| Field | Type | Default | Constraints | Description |
|-------|------|---------|-------------|-------------|
| `tab_stop_width` | `std::size_t` | 8 | Must be > 0 | Number of columns per tab stop. Controls tab expansion in visual column calculation. |
| `ansi_color` | `bool` | `false` | None | Whether to use ANSI color escape codes. `true` = red carets, `false` = plain `^`. |

#### Validation Rules

| Rule | Field | Condition | Enforcement | Error Behavior |
|------|-------|-----------|-------------|----------------|
| V-001 | `tab_stop_width` | `tab_stop_width > 0` | Caller responsibility (documented precondition) | Undefined behavior (division by zero in tab expansion formula) |
| V-002 | `ansi_color` | None (boolean flag) | N/A | N/A |

**Note**: Validation rule V-001 is a **hard precondition**. Callers must ensure `tab_stop_width > 0`. No runtime check performed (performance consideration).

#### Relationships

| Relationship | Target | Direction | Description |
|--------------|--------|-----------|-------------|
| **Used by** | `ErrorReporter` | Consumer | `ErrorReporter` stores `ErrorDisplayConfig` as member variable `config_` |
| **Produced by** | `make_display_config()` | Producer | Free function creates default config with environment-detected settings |
| **Configured by** | User/Caller | Input | Passed to `ErrorReporter` constructor for customization |

#### Usage Examples

**Default Configuration**:
```cpp
jsv::ErrorDisplayConfig config;  // tab_stop_width=8, ansi_color=false
jsv::ErrorReporter reporter(line_tracker, config);
```

**Custom Tab Width**:
```cpp
jsv::ErrorDisplayConfig config;
config.tab_stop_width = 4;  // 4-column tabs (narrow terminals)
config.ansi_color = true;   // Enable ANSI color
jsv::ErrorReporter reporter(line_tracker, config);
```

**Environment-Aware Configuration**:
```cpp
auto config = jsv::make_display_config();  // Auto-detects ansi_color
jsv::ErrorReporter reporter(line_tracker, config);
```

---

### Entity 2: UnicodeColumn Module Functions

**Entity ID**: ENT-002
**Kind**: Module (collection of free functions)
**Visibility**: Public (`include/jsav/error/UnicodeColumn.hpp`)

#### Source Requirements

- FR-003 through FR-006: Code point-based column calculation
- FR-018: Tab expansion
- FR-019 through FR-020: BOM/null handling
- FR-025 through FR-028: UTF-8 validation
- NFR-003: ANSI color detection

#### Purpose

Provides UTF-8-aware visual column calculation and error marker extent computation. All functions are stateless, pure (no side effects), and operate on `std::string_view` inputs.

#### Module Structure

```text
UnicodeColumn Module
├── detect_ansi_color()      → bool
├── make_display_config()    → ErrorDisplayConfig
├── visual_column()          → std::expected<std::size_t, std::string>
└── marker_extents()         → std::expected<std::pair<std::size_t, std::size_t>, std::string>
```

---

#### Function 2.1: detect_ansi_color

**Function ID**: FUNC-002.01
**Visibility**: Public
**Linkage**: Free function (namespace scope)

##### Signature

```cpp
[[nodiscard]] bool detect_ansi_color() noexcept;
```

##### Source Requirements

- NFR-003: Optional color with fallback
- User clarification: "Environment variable detection (`COLORTERM`, `TERM`) - respecting `NO_COLOR`"

##### Purpose

Detect ANSI color support from environment variables. Returns `true` if the terminal supports ANSI color escape codes, `false` otherwise.

##### Algorithm

```text
1. Check NO_COLOR environment variable:
   - If set and non-empty → return false (user explicitly disabled color)

2. Check COLORTERM environment variable:
   - If value is "truecolor" or "24bit" → return true

3. Check TERM environment variable:
   - If value contains "color", "xterm", "screen", or "tmux" → return true

4. Default → return false
```

**Priority Order**:

```text
NO_COLOR (highest priority, overrides all)
  ↓
COLORTERM
  ↓
TERM
  ↓
Default: false (lowest priority)
```

##### Return Value

| Value | Meaning |
|-------|---------|
| `true` | Terminal supports ANSI color escape codes. Safe to output colored markers. |
| `false` | Terminal does not support color, or user disabled via `NO_COLOR`. Use plain `^` characters. |

##### Side Effects

**None**. Pure function (reads environment variables via `std::getenv`, no mutation).

##### Thread Safety

**Thread-safe**. `std::getenv` is thread-safe on all supported platforms (Windows, Linux, macOS).

##### Platform Behavior

| Platform | Environment Variables | Notes |
|----------|----------------------|-------|
| **Windows** | `NO_COLOR`, `COLORTERM`, `TERM` | Works in Windows Terminal, VS Code terminal, Git Bash. May return `false` in legacy `cmd.exe`. |
| **Linux** | `NO_COLOR`, `COLORTERM`, `TERM` | Standard terminal emulators (gnome-terminal, konsole, xterm) set `TERM` appropriately. |
| **macOS** | `NO_COLOR`, `COLORTERM`, `TERM` | Terminal.app and iTerm2 set `TERM` appropriately. |

##### Examples

**Environment → Return Value**:

| NO_COLOR | COLORTERM | TERM | Return |
|----------|-----------|------|--------|
| (unset) | (unset) | `xterm-256color` | `true` |
| (unset) | `truecolor` | (any) | `true` |
| `1` | (any) | (any) | `false` |
| (unset) | (unset) | `dumb` | `false` |
| (unset) | (unset) | `screen` | `true` |

##### Implementation Notes

```cpp
#include <cstdlib>  // std::getenv
#include <cstring>  // std::strcmp, std::strstr

[[nodiscard]] bool detect_ansi_color() noexcept {
    // 1. Check NO_COLOR (overrides all)
    if (const char* no_color = std::getenv("NO_COLOR");
        no_color != nullptr && no_color[0] != '\0') {
        return false;
    }

    // 2. Check COLORTERM (truecolor/24bit)
    if (const char* colorterm = std::getenv("COLORTERM");
        colorterm != nullptr) {
        if (std::strcmp(colorterm, "truecolor") == 0 ||
            std::strcmp(colorterm, "24bit") == 0) {
            return true;
        }
    }

    // 3. Check TERM (contains "color", "xterm", "screen", "tmux")
    if (const char* term = std::getenv("TERM");
        term != nullptr) {
        if (std::strstr(term, "color") != nullptr ||
            std::strstr(term, "xterm") != nullptr ||
            std::strstr(term, "screen") != nullptr ||
            std::strstr(term, "tmux") != nullptr) {
            return true;
        }
    }

    // 4. Default: no color
    return false;
}
```

---

#### Function 2.2: make_display_config

**Function ID**: FUNC-002.02
**Visibility**: Public
**Linkage**: Free function (namespace scope)

##### Signature

```cpp
[[nodiscard]] ErrorDisplayConfig make_display_config() noexcept;
```

##### Source Requirements

- NFR-003: Default configuration from environment

##### Purpose

Load default `ErrorDisplayConfig` from environment. Calls `detect_ansi_color()` to populate `ansi_color` field.

##### Algorithm

```
1. Create ErrorDisplayConfig config with default values
   - config.tab_stop_width = 8 (default)
   - config.ansi_color = false (default)

2. Set config.ansi_color = detect_ansi_color()

3. Return config
```

##### Return Value

| Field | Value |
|-------|-------|
| `tab_stop_width` | 8 (default) |
| `ansi_color` | Result of `detect_ansi_color()` |

##### Side Effects

**None**. Pure function (calls pure function `detect_ansi_color()`).

##### Thread Safety

**Thread-safe**. All called functions are thread-safe.

##### Usage Example

```cpp
auto config = jsv::make_display_config();
// config.tab_stop_width = 8
// config.ansi_color = detect_ansi_color() (environment-detected)

jsv::ErrorReporter reporter(line_tracker, config);
```

---

#### Function 2.3: visual_column

**Function ID**: FUNC-002.03
**Visibility**: Public
**Linkage**: Free function (namespace scope)

##### Signature

```cpp
[[nodiscard]] std::expected<std::size_t, std::string>
visual_column(std::string_view line,
              std::size_t     byte_offset,
              std::size_t     tab_stop_width = 8) noexcept;
```

##### Source Requirements

- FR-003: Column positions calculated by counting Unicode code points
- FR-004: Each code point = 1 column unit
- FR-018: Tab expansion formula
- FR-019: BOM handling
- FR-020: Null byte rejection
- FR-025, FR-026: Overlong/surrogate rejection
- FR-027: Line length limit

##### Purpose

Return the **1-based visual column** of `byte_offset` inside `line`. The visual column counts Unicode code points (not bytes) from the start of the line, with special handling for tab characters.

##### Parameters

| Parameter | Type | Direction | Description | Constraints |
|-----------|------|-----------|-------------|-------------|
| `line` | `std::string_view` | Input | Source line content (UTF-8 encoded) | Must be valid UTF-8 (or function returns error) |
| `byte_offset` | `std::size_t` | Input | Byte offset within line (0-based) | If >= `line.size()`, clamps to `line.size() + 1` |
| `tab_stop_width` | `std::size_t` | Input | Tab stop width in columns (default 8) | **Must be > 0** (precondition) |

##### Return Value

**Success Case**:
```cpp
std::expected<std::size_t, std::string> result = visual_column(...);
if (result.has_value()) {
    std::size_t column = result.value();  // 1-based visual column
}
```

| Return Value | Meaning |
|--------------|---------|
| `std::size_t` (value) | 1-based visual column (counting code points, with tab expansion) |

**Error Case**:
```cpp
std::expected<std::size_t, std::string> result = visual_column(...);
if (!result.has_value()) {
    std::string error = result.error();  // Error message
}
```

| Error Message | Trigger |
|---------------|---------|
| `"Invalid UTF-8 sequence at byte offset {offset}"` | Invalid UTF-8 byte sequence detected |
| `"Null byte (U+0000) at byte offset {offset}"` | Null byte (U+0000) detected |
| `"Source line exceeds 10,000 code points"` | Line length limit exceeded (FR-027) |

##### Algorithm

**Pseudocode**:
```
col = 1
pos = 0

// FR-019: Skip BOM at file start
if line starts with BOM bytes (0xEF 0xBB 0xBF):
    pos = 3

// Validate BOM position (only at file start)
if pos == 3 and line.data() != source_start:
    LERROR("BOM detected at non-file-start position")

// Walk UTF-8 code points
code_point_count = 0
while pos < byte_offset and pos < line.size():
    res = decode_utf8(line, pos)

    // FR-025, FR-026: Invalid UTF-8
    if res is error:
        LERROR("Invalid UTF-8 at byte offset {}", pos)
        return std::unexpected(FORMAT("Invalid UTF-8 sequence at byte offset {}", pos))

    // FR-020: Null byte
    if res.codepoint == U'\0':
        LERROR("Null byte at byte offset {}", pos)
        return std::unexpected(FORMAT("Null byte (U+0000) at byte offset {}", pos))

    // FR-027: Line length limit
    ++code_point_count
    if code_point_count > 10000:
        LERROR("Source line exceeds 10,000 code points")
        return std::unexpected("Source line exceeds 10,000 code points")

    // FR-018: Tab expansion
    if res.codepoint == U'\t':
        col = ((col - 1) / tab_stop_width + 1) * tab_stop_width + 1
    else:
        col += 1

    pos += res.byte_length

// FR-013: Clamp to line_end + 1 if byte_offset past end
if byte_offset >= line.size():
    return col  // Return column at end of line

return col
```

##### Edge Cases

| Scenario | Input | Behavior | Return |
|----------|-------|----------|--------|
| **Empty line** | `line = ""`, `byte_offset = 0` | No code points to decode | `1` (column at start) |
| **BOM at file start** | `line = "\xEF\xBB\xBFabc"`, `byte_offset = 4` | BOM skipped | `2` (column after BOM and 'a') |
| **Byte offset past end** | `line = "abc"`, `byte_offset = 10` | Clamps to end | `4` (column after 'c') |
| **Tab at column 1** | `line = "\tabc"`, `byte_offset = 1` | Tab expands to next stop | `9` (with tab_stop_width=8) |
| **Tab at column 5** | `line = "abcd\t"`, `byte_offset = 5` | Tab expands to next stop | `9` (with tab_stop_width=8) |
| **Multi-byte code point** | `line = "你好"`, `byte_offset = 3` (after '好') | Counts as 1 column | `3` (not 6 bytes) |

##### Error Handling

**Logging**: All error paths call `LERROR()` before returning `std::unexpected`.

**Error Message Format**:
```cpp
FORMAT("Invalid UTF-8 sequence at byte offset {}", byte_offset)
FORMAT("Null byte (U+0000) at byte offset {}", byte_offset)
"Source line exceeds 10,000 code points"
```

##### Complexity

| Metric | Value |
|--------|-------|
| **Time** | O(byte_offset) — walks from line start to byte_offset |
| **Space** | O(1) — stack variables only, no heap allocation |

##### Thread Safety

**Thread-safe**. Pure function (no mutation, no shared state).

##### Usage Example

```cpp
std::string line = "let x = 你好;";
auto result = jsv::visual_column(line, 8);  // Byte offset of '你'

if (result.has_value()) {
    std::size_t col = result.value();  // col = 9 (8 code points + 1)
} else {
    LERROR("Encoding error: {}", result.error());
}
```

---

#### Function 2.4: marker_extents

**Function ID**: FUNC-002.04
**Visibility**: Public
**Linkage**: Free function (namespace scope)

##### Signature

```cpp
[[nodiscard]] std::expected<std::pair<std::size_t, std::size_t>, std::string>
marker_extents(std::string_view line,
               std::size_t     start_byte,
               std::size_t     end_byte,
               std::size_t     tab_stop_width = 8) noexcept;
```

##### Source Requirements

- FR-005: Leading spaces = code points before error
- FR-006: Caret count = code points in error span
- FR-010: Tabs contribute expanded width to caret count
- FR-012, FR-014: Empty line, error at col 1
- FR-013: Minimum 1 caret
- FR-018: Tab expansion

##### Purpose

Return `(leading_spaces, caret_count)` for the marker row in error messages. The `leading_spaces` value positions the first caret, and `caret_count` determines how many carets to display.

##### Parameters

| Parameter | Type | Direction | Description | Constraints |
|-----------|------|-----------|-------------|-------------|
| `line` | `std::string_view` | Input | Source line content (UTF-8 encoded) | Must be valid UTF-8 (or function returns error) |
| `start_byte` | `std::size_t` | Input | Start byte offset within line (inclusive) | If >= `line.size()`, clamps to `line.size()` |
| `end_byte` | `std::size_t` | Input | End byte offset within line (exclusive) | If >= `line.size()`, clamps to `line.size()` |
| `tab_stop_width` | `std::size_t` | Input | Tab stop width in columns (default 8) | **Must be > 0** (precondition) |

##### Return Value

**Success Case**:
```cpp
auto result = jsv::marker_extents(line, start, end);
if (result.has_value()) {
    auto [leading_spaces, caret_count] = result.value();
    // leading_spaces: number of spaces before first caret
    // caret_count: number of carets to display
}
```

| Return Value | Meaning |
|--------------|---------|
| `std::pair<std::size_t, std::size_t>` | `first` = leading spaces, `second` = caret count |

**Error Case**:
```cpp
auto result = jsv::marker_extents(line, start, end);
if (!result.has_value()) {
    std::string error = result.error();  // Error message
}
```

| Error Message | Trigger |
|---------------|---------|
| `"Invalid UTF-8 sequence at byte offset {offset}"` | Invalid UTF-8 in range [start_byte, end_byte) |
| `"Null byte (U+0000) at byte offset {offset}"` | Null byte in range [start_byte, end_byte) |
| `"Source line exceeds 10,000 code points"` | Line length limit exceeded |

##### Algorithm

**Pseudocode**:
```
// Calculate leading spaces
leading_result = visual_column(line, start_byte, tab_stop_width)
if leading_result is error:
    return std::unexpected(leading_result.error())

leading_spaces = leading_result.value() - 1  // Convert 1-based to 0-based

// Calculate caret count (sum of visual widths from start to end)
caret_count = 0
pos = start_byte
code_point_count = 0

while pos < end_byte and pos < line.size():
    res = decode_utf8(line, pos)

    if res is error:
        LERROR("Invalid UTF-8 at byte offset {}", pos)
        return std::unexpected(FORMAT("Invalid UTF-8 sequence at byte offset {}", pos))

    if res.codepoint == U'\0':
        LERROR("Null byte at byte offset {}", pos)
        return std::unexpected(FORMAT("Null byte (U+0000) at byte offset {}", pos))

    // FR-027: Line length limit
    ++code_point_count
    if code_point_count > 10000:
        LERROR("Source line exceeds 10,000 code points")
        return std::unexpected("Source line exceeds 10,000 code points")

    // FR-010: Tabs contribute expanded width to caret count
    if res.codepoint == U'\t':
        next_tab = ((caret_count / tab_stop_width) + 1) * tab_stop_width
        caret_count += (next_tab - caret_count)
    else:
        caret_count += 1

    pos += res.byte_length

// FR-013: Minimum 1 caret
if caret_count == 0:
    caret_count = 1

return std::make_pair(leading_spaces, caret_count)
```

##### Edge Cases

| Scenario | Input | Behavior | Return |
|----------|-------|----------|--------|
| **Empty line, error at col 1** | `line = ""`, `start_byte = 0`, `end_byte = 0` | caret_count = 1 (minimum) | `(0, 1)` |
| **Error at last code point** | `line = "abc"`, `start_byte = 2`, `end_byte = 3` | Single code point | `(2, 1)` |
| **start_byte >= end_byte** | `line = "abc"`, `start_byte = 3`, `end_byte = 2` | caret_count = 1 (minimum) | `(3, 1)` |
| **Tab in error span** | `line = "a\tb"`, `start_byte = 1`, `end_byte = 2` | Tab expands to next stop | `(1, 7)` (with tab_stop_width=8) |
| **Multi-byte error span** | `line = "你好"`, `start_byte = 0`, `end_byte = 6` | 2 code points | `(0, 2)` |

##### Error Handling

Same as `visual_column()` (invalid UTF-8, null byte, line length limit).

##### Complexity

| Metric | Value |
|--------|-------|
| **Time** | O(end_byte - start_byte) — walks from start_byte to end_byte |
| **Space** | O(1) — stack variables only, no heap allocation |

##### Thread Safety

**Thread-safe**. Pure function (no mutation, no shared state).

##### Usage Example

```cpp
std::string line = "let x = 你好;";
auto result = jsv::marker_extents(line, 8, 11);  // Byte span of '你好'

if (result.has_value()) {
    auto [leading, width] = result.value();
    // leading = 8 (spaces before '你')
    // width = 2 (carets for '你' and '好')
} else {
    LERROR("Encoding error: {}", result.error());
}
```

---

## Integration Points

### Integration 1: LineTracker Extension

**Integration ID**: INT-001
**Target Class**: `LineTracker`
**Target Header**: `include/jsav/core/LineTracker.hpp`
**Change Type**: Additive (no breaking changes)

#### Source Requirements

- Data flow specification in plan.md

#### Purpose

Provide access to full source buffer for byte offset calculation in `ErrorReporter::format_spanned_error`.

#### Change Specification

**Addition**:
```cpp
// In include/jsav/core/LineTracker.hpp, public section:

/// @brief Returns the full source buffer as a string_view.
///
/// This accessor provides read-only access to the complete source buffer
/// managed by LineTracker. It is used to calculate byte offsets for
/// error reporting.
///
/// @return std::string_view spanning the entire source buffer.
/// @threadsafe Yes (read-only access to immutable data).
/// @since 0.5.0 (feature 005-fix-unicode-error-reporting)
[[nodiscard]] std::string_view source() const noexcept { return source_; }
```

#### Impact Analysis

| Aspect | Impact |
|--------|--------|
| **ABI Compatibility** | No break (pure addition, no existing members changed) |
| **API Compatibility** | No break (existing API unchanged) |
| **Behavior Change** | None (read-only accessor) |
| **Performance** | None (inline one-liner) |
| **Test Impact** | None (internal utility, not directly tested) |

#### Usage in ErrorReporter

```cpp
// In src/jsav_Lib/error/ErrorReporter.cpp:

const std::size_t line_start_byte_offset =
    source_line.data() - line_tracker.source().data();

auto extents = jsv::marker_extents(
    source_line,
    span.start.absolute_pos - line_start_byte_offset,
    span.end.absolute_pos   - line_start_byte_offset,
    config_.tab_stop_width);
```

---

### Integration 2: ErrorReporter Extension

**Integration ID**: INT-002
**Target Class**: `ErrorReporter`
**Target Headers**: `include/jsav/error/ErrorReporter.hpp`, `src/jsav_Lib/error/ErrorReporter.cpp`
**Change Type**: Additive + Modification (backward-compatible)

#### Source Requirements

- FR-001: Error message format with visual marker
- FR-002: Visual marker row with leading spaces and carets
- SC-002: Backward compatibility for ASCII source

#### Purpose

Integrate `UnicodeColumn` module into error reporting pipeline. Add configuration support and modify `format_spanned_error` to use code point-based column calculation.

#### Change Specification

**1. Add Member Variable**:
```cpp
// In include/jsav/error/ErrorReporter.hpp, private section:

/// @brief Configuration for tab expansion and ANSI color output.
///
/// Controls the visual appearance of error markers. Initialized via
/// constructor parameter or default-constructed with make_display_config().
/// @see ErrorDisplayConfig
ErrorDisplayConfig config_;
```

**2. Add Two-Argument Constructor**:
```cpp
// In include/jsav/error/ErrorReporter.hpp, public section:

/// @brief Constructs an ErrorReporter with custom display configuration.
///
/// This constructor allows explicit configuration of tab stop width and
/// ANSI color output. For automatic configuration based on environment
/// variables, use the single-argument constructor.
///
/// @param line_tracker Reference to LineTracker for line retrieval.
/// @param config Display configuration (tab width, ANSI color).
/// @threadsafe Not threadsafe (shared mutable state if used across threads).
/// @since 0.5.0 (feature 005-fix-unicode-error-reporting)
ErrorReporter(const LineTracker &line_tracker,
              ErrorDisplayConfig config) noexcept
    : line_tracker_(line_tracker), config_(config) {}
```

**3. Modify Existing Constructor**:
```cpp
// In src/jsav_Lib/error/ErrorReporter.cpp:

ErrorReporter::ErrorReporter(const LineTracker &line_tracker) noexcept
    : line_tracker_(line_tracker), config_(make_display_config()) {}
```

**4. Modify format_spanned_error**:

**BEFORE** (byte-based, incorrect for Unicode):
```cpp
const std::size_t start_offset = (start_col > 0u) ? (start_col - 1u) : 0u;
const std::string underline = FORMAT("{:>{}}{}", "", start_offset, std::string(span_width, '^'));
FORMAT_TO(out, "     │ {}\n", underline);
```

**AFTER** (code point-based, FR-003 through FR-006):
```cpp
// Calculate byte offset of line start
const std::size_t line_start_byte_offset =
    source_line.data() - line_tracker_.source().data();

// Calculate marker extents (leading spaces, caret count)
auto extents = jsv::marker_extents(
    source_line,
    span.start.absolute_pos - line_start_byte_offset,  // Byte offset within line
    span.end.absolute_pos   - line_start_byte_offset,  // Byte offset within line
    config_.tab_stop_width);

if (!extents) {
    // FR-016 / FR-028: Encoding error already logged by marker_extents
    // Emit the error message at the current output position and continue.
    FORMAT_TO(out, "     │ {}\n",
              ansi::red_bold(FORMAT("(encoding error: {})", extents.error())));
} else {
    const auto [leading, width] = *extents;
    const std::string carets(width, '^');
    const std::string underline = FORMAT("{:>{}}{}", "", leading, carets);

    // Apply ANSI color if enabled (NFR-003)
    if (config_.ansi_color) {
        FORMAT_TO(out, "     │ {}\n", ansi::red(underline));
    } else {
        FORMAT_TO(out, "     │ {}\n", underline);
    }
}
```

#### Backward Compatibility

| Aspect | Status | Notes |
|--------|--------|-------|
| **Existing Constructor** | ✅ Preserved | Single-argument constructor still works |
| **Default Behavior** | ✅ Unchanged | Default-configured `ErrorDisplayConfig` used |
| **ASCII Output** | ✅ Identical | Byte-for-byte identical output (SC-002) |
| **API Surface** | ✅ Additive | New constructor added, existing unchanged |

---

## Validation Rules

### Rule 1: Tab Stop Width Validation

**Rule ID**: VAL-001
**Severity**: Critical (causes undefined behavior if violated)

#### Specification

| Aspect | Value |
|--------|-------|
| **Source** | FR-018 (tab expansion formula) |
| **Rule** | `tab_stop_width` must be > 0 |
| **Enforcement** | Caller responsibility (documented precondition) |
| **Error Behavior** | Undefined (division by zero in tab expansion formula) |

#### Rationale

**Why not runtime check?**
- Performance consideration (check on every call)
- Caller can validate once at construction time
- Documented precondition (caller's responsibility)

**Why critical?**
- Division by zero in formula: `((col - 1) / tab_stop_width + 1) * tab_stop_width + 1`
- Undefined behavior (crash, incorrect output)

#### Mitigation

**Caller Responsibility**:
```cpp
jsv::ErrorDisplayConfig config;
config.tab_stop_width = 8;  // Caller ensures > 0
// config.tab_stop_width = 0;  // UNDEFINED BEHAVIOR - do not do this
```

**Optional Debug Check** (for development builds only):
```cpp
#ifdef DEBUG
assert(tab_stop_width > 0 && "tab_stop_width must be > 0");
#endif
```

---

### Rule 2: Line Length Limit

**Rule ID**: VAL-002
**Severity**: High (DoS prevention)

#### Specification

| Aspect | Value |
|--------|-------|
| **Source** | FR-027 (line length limit) |
| **Rule** | Lines must not exceed 10,000 code points |
| **Enforcement** | Runtime check in `visual_column()` and `marker_extents()` |
| **Error Behavior** | `LERROR` + `std::unexpected("Source line exceeds 10,000 code points")` |

#### Rationale

**Why 10,000?**
- Practical limit (typical source lines < 200 code points)
- DoS prevention (pathological inputs)
- Performance consideration (O(n) algorithm)

**Why runtime check?**
- Cannot validate at compile time (input-dependent)
- Security requirement (malicious input)

#### Implementation

```cpp
constexpr std::size_t MAX_CODE_POINTS = 10000;

std::size_t code_point_count = 0;
while (pos < byte_offset) {
    // ... decode code point ...

    ++code_point_count;
    if (code_point_count > MAX_CODE_POINTS) {
        LERROR("Source line exceeds 10,000 code points at line {}", line_number);
        return std::unexpected("Source line exceeds 10,000 code points");
    }
}
```

---

### Rule 3: UTF-8 Validity

**Rule ID**: VAL-003
**Severity**: Critical (security and correctness)

#### Specification

| Aspect | Value |
|--------|-------|
| **Source** | FR-020, FR-025, FR-026 (UTF-8 validation) |
| **Rule** | All input must be valid UTF-8 (no null bytes, no overlong sequences, no surrogate halves) |
| **Enforcement** | `decode_utf8` validation at each code point |
| **Error Behavior** | `LERROR` + `std::unexpected` with byte offset and line number |

#### Rationale

**Why strict validation?**
- Security (malformed UTF-8 attacks)
- Correctness (invalid input → incorrect output)
- Standards compliance (RFC 3629)

**Why reject null bytes?**
- C++ string terminator
- Security (truncation attacks)

#### Validation Coverage

| Sequence Type | Valid? | Action |
|---------------|--------|--------|
| Single-byte (0x00-0x7F) | ✅ (except 0x00) | Decode normally |
| Null byte (0x00) | ❌ | Reject with error |
| Multi-byte (0xC0-0xFD) | ✅ (if well-formed) | Decode and validate |
| Overlong encoding | ❌ | Reject with error |
| Surrogate half (U+D800–U+DFFF) | ❌ | Reject with error |
| Invalid code point (U+FFFE, U+FFFF) | ❌ | Reject with error |
| Truncated sequence | ❌ | Reject with error |

---

## Data Flow Summary

### End-to-End Flow

```
┌─────────────────────────────────────────────────────────────────────────┐
│ User provides source file (UTF-8 encoded)                               │
└─────────────────────────────────────────────────────────────────────────┘
                                    │
                                    ▼
┌─────────────────────────────────────────────────────────────────────────┐
│ Lexer produces SourceSpan                                               │
│   → start.absolute_pos  (byte offset, ground truth)                     │
│   → end.absolute_pos    (byte offset, ground truth)                     │
│   → start.line          (1-based line number)                           │
└─────────────────────────────────────────────────────────────────────────┘
                                    │
                                    ▼
┌─────────────────────────────────────────────────────────────────────────┐
│ ErrorReporter::format_spanned_error invoked                             │
│   → Input: SourceSpan (byte-based positions)                            │
└─────────────────────────────────────────────────────────────────────────┘
                                    │
                                    ▼
┌─────────────────────────────────────────────────────────────────────────┐
│ LineTracker::get_line(start_line) → source_line (string_view, O(1))     │
│ LineTracker::source() → full source buffer (for offset calc)            │
└─────────────────────────────────────────────────────────────────────────┘
                                    │
                                    ▼
┌─────────────────────────────────────────────────────────────────────────┐
│ Calculate line_start_byte_offset                                        │
│   line_start_byte_offset = source_line.data() - line_tracker.source().data() │
└─────────────────────────────────────────────────────────────────────────┘
                                    │
                                    ▼
┌─────────────────────────────────────────────────────────────────────────┐
│ UnicodeColumn::marker_extents(                                          │
│     source_line,                                                        │
│     start.absolute_pos - line_start_byte_offset,                        │
│     end.absolute_pos   - line_start_byte_offset,                        │
│     config_.tab_stop_width)                                             │
│                                                                         │
│   → Internally calls visual_column() for leading spaces                 │
│   → Walks UTF-8 code points for caret count                             │
│   → Handles tabs, BOM, null bytes, invalid sequences                    │
└─────────────────────────────────────────────────────────────────────────┘
                                    │
                                    ▼
┌─────────────────────────────────────────────────────────────────────────┐
│ Returns (leading_spaces, caret_count)                                   │
│   → leading_spaces: Number of spaces before first caret                 │
│   → caret_count: Number of carets to display                            │
└─────────────────────────────────────────────────────────────────────────┘
                                    │
                                    ▼
┌─────────────────────────────────────────────────────────────────────────┐
│ Format underline with leading spaces + carets                           │
│   → std::string underline = FORMAT("{:>{}}{}", "", leading, carets)     │
└─────────────────────────────────────────────────────────────────────────┘
                                    │
                                    ▼
┌─────────────────────────────────────────────────────────────────────────┐
│ Output error message with ANSI color (if enabled)                       │
│   → if (config_.ansi_color) ansi::red(underline) : underline            │
│   → FORMAT_TO(out, "     │ {}\n", underline)                            │
└─────────────────────────────────────────────────────────────────────────┘
```

### Component Interactions

```
┌──────────────────┐         ┌──────────────────┐         ┌──────────────────┐
│   ErrorReporter  │────────▶│  UnicodeColumn   │────────▶│   LineTracker    │
│                  │  Uses   │                  │  Uses    │                  │
│  - config_       │         │  - visual_column │         │  - source()      │
│  - format_spanned│         │  - marker_extents│         │  - get_line()    │
└──────────────────┘         └──────────────────┘         └──────────────────┘
         │                           │                           │
         │                           │                           │
         ▼                           ▼                           ▼
┌──────────────────┐         ┌──────────────────┐         ┌──────────────────┐
│  spdlog (LERROR) │         │  Utf8.hpp        │         │  Source buffer   │
│  - Diagnostics   │         │  - decode_utf8   │         │  - Immutable     │
└──────────────────┘         └──────────────────┘         └──────────────────┘
```

---

## Traceability Matrix

### Requirements Coverage

| Entity | Traces To | Requirement Text | Verification Method |
|--------|-----------|------------------|---------------------|
| `ErrorDisplayConfig` | FR-018, NFR-003 | Tab stops and ANSI color fallback | Code review, unit test |
| `detect_ansi_color()` | NFR-003 | Environment variable detection | Unit test |
| `make_display_config()` | NFR-003 | Default configuration loading | Unit test |
| `visual_column()` | FR-003, FR-004, FR-018, FR-019, FR-020 | Code point counting, tab expansion, BOM/null handling | Unit test (P1, P3) |
| `marker_extents()` | FR-005, FR-006, FR-010, FR-012, FR-013, FR-014 | Leading spaces, caret count, edge cases | Unit test (P1, P3) |
| `LineTracker::source()` | Data flow | Byte offset calculation | Code review |
| `ErrorReporter` extension | FR-001, FR-002 | Error message format with visual marker | Unit test, integration test |
| Line length limit | FR-027 | DoS prevention | Unit test (P3) |
| UTF-8 validation | FR-020, FR-025, FR-026 | Security, correctness | Unit test (P2) |

### Test Coverage

| Entity | Test Fixture | Test Cases |
|--------|--------------|------------|
| `visual_column()` | P1, P3 | ASCII, multi-byte, tab, BOM, null byte, line limit |
| `marker_extents()` | P1, P3 | Unicode alignment, edge cases, empty line |
| `detect_ansi_color()` | N/A (internal) | Tested indirectly via `ErrorReporter` output |
| `ErrorReporter` extension | P1, P2, P3 | Full integration tests |

---

## Glossary

| Term | Definition | Usage in Document |
|------|------------|-------------------|
| **Byte Offset** | Position in bytes from start of line (0-based) | `byte_offset`, `start_byte`, `end_byte` |
| **Code Point** | Unicode scalar value (U+0000 to U+10FFFF, excluding surrogates) | Column counting unit |
| **Visual Column** | 1-based position counting code points (not bytes) from line start | `visual_column()` return value |
| **BOM** | Byte Order Mark (U+FEFF, encoded as 0xEF 0xBB 0xBF in UTF-8) | Skipped at file start |
| **Overlong Encoding** | UTF-8 sequence using more bytes than necessary (invalid) | Rejected by validation |
| **Surrogate Half** | Code point in range U+D800–U+DFFF (invalid in UTF-8) | Rejected by validation |
| **ZWJ** | Zero Width Joiner (U+200D, used in emoji sequences) | Counted as separate code point |
| **Grapheme Cluster** | User-perceived character (not used in this feature) | Explicitly NOT used |
| **std::expected** | C++23 type for error propagation (monadic error handling) | Return type for fallible functions |
| **std::string_view** | Non-owning view over a string (read-only, no allocation) | Parameter type for all functions |

---

## References

1. **Feature Specification**: `specs/005-fix-unicode-error-reporting/spec.md`
2. **Research Document**: `specs/005-fix-unicode-error-reporting/research.md`
3. **Implementation Plan**: `specs/005-fix-unicode-error-reporting/plan.md`
4. **Quickstart Guide**: `specs/005-fix-unicode-error-reporting/quickstart.md`
5. **C++23 Standard**: ISO/IEC 14882:2023 (`std::expected`, `std::string_view`)
6. **UTF-8 Standard**: RFC 3629
7. **Constitution**: `.specify/memory/constitution.md`

---

**End of Data Model Document**
