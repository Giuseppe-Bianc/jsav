# Implementation Plan: AST Parser Implementation

**Branch**: `006-ast-parser-implementation` | **Date**: 2026-03-21 | **Spec**: [spec.md](./spec.md)
**Input**: Feature specification from `/specs/006-ast-parser-implementation/spec.md`

**Note**: This template is filled in by the `/speckit.plan` command. See `.specify/templates/plan-template.md` for the execution workflow.

## Summary

Implement a hybrid parser for the jsav compiler that transforms token streams from the existing Lexer into typed Abstract Syntax Trees (AST) while collecting syntax errors. The parser uses Pratt Parsing for expressions (binding power pairs for operator precedence/associativity) combined with Recursive Descent for statements. Architecture: three-module design (Parser orchestrator, ExpressionParser, StatementParser) with panic-mode error recovery, RAII context management, and std::unique_ptr-based AST memory ownership. Output: std::pair<NodePtr, std::vector<CompileError>>.

## Technical Context

**Language/Version**: C++23 (project standard, GCC 14+/Clang 18+/MSVC 2022+)
**Primary Dependencies**: Existing project dependencies only (spdlog 1.17.0, fmt 12.1.0, Catch2 3.13.0). No new dependencies — uses C++23 standard library containers (std::vector, std::unique_ptr, std::optional, std::stack, std::unordered_map) and existing project logging/formatting.
**Storage**: N/A (in-memory AST construction, no serialization)
**Testing**: Three-target Catch2 approach (constexpr_tests, relaxed_constexpr_tests, tests) with ≥80% line coverage, ≥70% branch coverage. Sanitizers: AddressSanitizer + UndefinedBehaviorSanitizer (zero violations required).
**Target Platform**: Cross-platform (Windows, Linux, macOS) — native compiler executable
**Project Type**: Compiler (parser phase transforming tokens to AST)
**Performance Goals**: Latency targets per NFR-003: p95 parsing latency <500ms for 10K LOC, <100ms for 1K LOC. Memory-efficient AST construction via std::unique_ptr. Single-pass parsing where possible.
**Constraints**: Unidirectional dependency flow (Lexer → Parser → AST), no circular dependencies with jsav_Core_lib, zero compiler warnings, clang-tidy/cppcheck zero issues, lizard thresholds (CCN ≤15, length ≤100, params ≤6)
**Scale/Scope**: 27 AST node types (16 expressions + 11 statements), 12-level binding power table, panic-mode error recovery with explicit resource limits (recursion depth ≤1000, memory usage ≤512MB). Performance optimization based on empirical profiling data only (Constitution Principle VII).

## Constitution Check

*GATE: Must pass before Phase 0 research. Re-check after Phase 1 design.*

### Principle I: Platform Independence

**Status**: ✅ Compliant

**Assessment**:
- Implementation uses C++23 standard library exclusively (std::vector, std::unique_ptr, std::optional, std::stack, std::unordered_map, std::string_view)
- No platform-specific APIs — all code is portable across Windows, Linux, macOS
- Parser operates on in-memory token streams; no file I/O or OS integration in parser module
- AST is purely in-memory with no serialization; no .vn file handling in parser phase

### Principle II: Visual Studio 2026 Compatibility

**Status**: ✅ Compliant

**Assessment**:
- C++23 features used are verified as supported by MSVC 2022+ (std::format, std::unique_ptr, std::optional, concepts where applicable)
- No experimental compiler features — only standardized C++23
- Project already configured for MSVC via CMakePresets.json (windows-msvc-debug-developer-mode)

### Principle III: C++ Core Guidelines Compliance

**Status**: ✅ Compliant

**Assessment**:
- **Ownership Semantics**: AST nodes use std::unique_ptr exclusively; parent nodes own child nodes. No raw new/delete. Token stream held as non-owning std::string_view reference.
- **Const Correctness**: Parser methods that don't modify state will be declared const. Parameters passed by const reference where appropriate.
- **Move Semantics**: AST nodes will implement move operations as needed. Return by value with copy elision.
- **Error Handling**: Syntax errors collected in std::vector<CompileError>, returned alongside AST via std::pair<NodePtr, std::vector<CompileError>>. Alternative considered: std::expected<T, E> (C++23) — rejected because parser always produces AST even when syntax errors present; std::expected semantics (either value OR error) inappropriate for parser that returns both partial AST and error list. No exceptions for expected failures.
- **Enforcement**: clang-tidy/cppcheck zero warnings, sanitizers zero violations, lizard thresholds enforced.

### Principle IV: Test-Driven Development (Red-Green)

**Status**: ✅ Compliant

**Assessment**:
- Three-target test strategy: constexpr_tests (binding power constants), relaxed_constexpr_tests (debugging), tests (runtime unit/integration tests)
- Test naming convention: Parser_Module_Scenario format (e.g., Parser_EmptyInput_ReturnsEmptyProgram)
- Coverage targets: ≥80% line, ≥70% branch via gcovr
- TDD workflow: Red (failing test) → Green (minimal implementation) → Refactor (under test protection)

### Principle V: Dependency Management

**Status**: ✅ Compliant

**Assessment**:
- No new dependencies added — parser uses only existing project dependencies (spdlog, fmt, Catch2)
- All dependencies managed via CPM.cmake with version locking
- No dependency header exposure in public headers (include/jsav/parser/)
- Dependencies linked with PRIVATE visibility

### Principle VI: Documentation Standards

**Status**: ✅ Compliant

**Assessment**:
- All public interfaces will have Doxygen documentation (function purpose, parameters, return values, examples)
- Module-level documentation describes responsibility, architectural role, integration points
- Error codes documented with condition, typical cause, suggested fix
- This plan.md and research.md follow markdown structure standards

### Principle VII: Algorithmic Design Excellence

**Status**: ✅ Compliant

**Assessment**:
- **Pratt Parsing**: Selected for expression parsing due to optimal handling of operator precedence/associativity. Time complexity: O(n) for n tokens. Superior to recursive descent for expressions with many precedence levels.
- **Recursive Descent**: Selected for statement parsing due to natural hierarchical grammar. Clear separation of concerns, one function per statement type.
- **Panic-Mode Recovery**: Linear-time error recovery algorithm. Synchronizes to next statement boundary (;, }, or statement keyword).
- **RAII Context Management**: Stack-based context tracking with automatic cleanup on scope exit (including error paths).
- Formal analysis documented in research.md with complexity characteristics and trade-offs.

---

## Post-Design Constitution Re-Evaluation

*Completed after Phase 1 design artifacts generated*

### Re-Evaluation Summary

All constitution principles remain compliant after design completion. Design artifacts (data-model.md, quickstart.md) align with initial constitution check.

### Principle I: Platform Independence — ✅ Still Compliant

**Design Verification**:
- data-model.md uses only C++23 standard library types (std::unique_ptr, std::optional, std::vector, std::string, std::string_view)
- No platform-specific APIs in AST node definitions
- No serialization or file I/O in data model
- quickstart.md examples are cross-platform (standard C++ only)

### Principle II: Visual Studio 2026 Compatibility — ✅ Still Compliant

**Design Verification**:
- All AST node classes use standard C++ patterns (virtual functions, smart pointers, move semantics)
- No experimental C++23 features — only widely-supported features (std::optional, std::unique_ptr, std::string_view)
- MSVC 2022+ fully supports all patterns used

### Principle III: C++ Core Guidelines Compliance — ✅ Still Compliant

**Design Verification**:
- data-model.md enforces ownership via std::unique_ptr exclusively
- All node classes are non-copyable, movable (disable copying, enable moving)
- Const correctness: accessor methods declared const, parameters passed by const reference
- Error handling: std::vector<CompileError> returned, no exceptions for syntax errors
- RAII: ContextGuard pattern ensures automatic cleanup

### Principle IV: Test-Driven Development — ✅ Still Compliant

**Design Verification**:
- quickstart.md includes test examples for all three targets (constexpr, relaxed_constexpr, runtime)
- Test naming follows Parser_Module_Scenario convention
- Coverage targets documented (≥80% line, ≥70% branch)

### Principle V: Dependency Management — ✅ Still Compliant

**Design Verification**:
- No new dependencies in data-model.md or quickstart.md
- Uses only existing project dependencies (spdlog for logging in examples, Catch2 for tests)
- No dependency types exposed in AST node headers

### Principle VI: Documentation Standards — ✅ Still Compliant

**Design Verification**:
- data-model.md has complete Doxygen-style documentation for all 27 node types
- quickstart.md includes usage examples, troubleshooting, extension guides
- All markdown files follow heading hierarchy (H1 → H2 → H3)
- Terminology consistent across plan.md, research.md, data-model.md, quickstart.md

### Principle VII: Algorithmic Design Excellence — ✅ Still Compliant

**Design Verification**:
- research.md documents algorithmic choices with formal analysis (O(n) complexity for all phases)
- Pratt parsing justified with precedence/associativity analysis
- Recursive descent justified with hierarchical grammar analysis
- Panic-mode recovery analyzed with worst-case linear time

---

## New Violations Introduced

**None** — Post-design re-evaluation confirms no new violations introduced by design artifacts.

## Project Structure

### Documentation (this feature)

```text
specs/006-ast-parser-implementation/
├── plan.md              # This file
├── research.md          # Phase 0 output
├── data-model.md        # Phase 1 output
├── quickstart.md        # Phase 1 output
├── contracts/           # Phase 1 output (N/A - internal compiler module)
└── tasks.md             # Phase 2 output (NOT created by /speckit.plan)
```

### Source Code (repository root)

```text
include/jsav/parser/
├── Parser.hpp              # Main orchestrator header
├── ExpressionParser.hpp    # Pratt parsing header
└── StatementParser.hpp     # Recursive descent header

src/jsav_Lib/parser/
├── Parser.cpp              # Main orchestrator implementation
├── ExpressionParser.cpp    # Pratt parsing implementation
└── StatementParser.cpp     # Recursive descent implementation

test/
├── constexpr_tests.cpp     # Compile-time binding power verification
└── tests.cpp               # Runtime unit/integration tests
```

**Structure Decision**: Standard project structure following existing jsav architecture. Headers in include/jsav/parser/ (public API, installed), implementation in src/jsav_Lib/parser/ (linked into jsav_lib static library). Tests integrated into existing test targets.

## Complexity Tracking

> **Fill ONLY if Constitution Check has violations that must be justified**

No violations identified. All principles are compliant.

| Violation | Why Needed | Simpler Alternative Rejected Because |
|-----------|------------|-------------------------------------|
| N/A | N/A | N/A |
