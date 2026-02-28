# Implementation Plan: CMake UTF-8 Support

**Branch**: `001-cmake-utf8-support` | **Date**: 2026-02-27 | **Spec**: [spec.md](./spec.md)
**Input**: Feature specification from `/specs/001-cmake-utf8-support/spec.md`

**Note**: This template is filled in by the `/speckit.plan` command. See `.specify/templates/plan-template.md` for the execution workflow.

## Summary

Add UTF-8 support to the jsav C++23 compiler project by introducing three CMake modules (`cmake/Utf8Compiler.cmake`, `cmake/Utf8Console.cmake`, `cmake/Utf8BomStrip.cmake`), one configured header (`configured_files/include/jsav/utf8_console.hpp.in`), and a platform-abstraction library for console initialization (`src/jsav_Lib/utf8_console_win.cpp` and `src/jsav_Lib/utf8_console_unix.cpp`). The implementation uses compiler built-ins (`/utf-8` for MSVC, `-finput-charset=UTF-8 -fexec-charset=UTF-8` for GCC/Clang), Win32 `SetConsoleOutputCP(CP_UTF8)` API (encapsulated behind portable interface), and CMake's native file/string commands for BOM handling. Integration occurs through `jsav_ENABLE_UTF8` option (default ON) wired into existing `ProjectOptions.cmake` pipeline.

**Platform Abstraction**: To comply with Constitution Principle I (Platform Independence) and Principle III (C++ Core Guidelines), Windows-specific Win32 API calls are encapsulated in a platform-abstraction layer:
- `include/jsav/utf8/console.hpp` - Portable interface declaring `jsav::utf8::init_console()`. On Linux/macOS, this function is a no-op (returns immediately) to maintain API consistency across platforms; applications call the same function regardless of target OS.
- `src/jsav_Lib/utf8_console_win.cpp` - Windows implementation with `SetConsoleOutputCP(CP_UTF8)`
- `src/jsav_Lib/utf8_console_unix.cpp` - Linux/macOS implementation (no-op, returns immediately)
- CMake module `cmake/Utf8Console.cmake` selects appropriate implementation based on `CMAKE_SYSTEM_NAME`

**Note on configured_files**: The `configured_files/include/jsav/utf8_console.hpp.in` template is used for legacy compatibility during the transition period. New code should use `include/jsav/utf8/console.hpp` directly. The configured file will be deprecated in a future release.

## Technical Context

**Language/Version**: C++23 (CMake 3.29+, GCC 13+, Clang 18+, MSVC 2022+)
**Primary Dependencies**: fmt, spdlog, Catch2, CLI11 (all via CPM.cmake)
**Storage**: N/A (compiler project, no persistent storage)
**Testing**: Catch2 framework with constexpr_tests.cpp, tests.cpp, and relaxed_constexpr_tests
**Target Platform**: Cross-platform (Windows, Linux, macOS)
**Project Type**: Compiler/CLI tool
**Performance Goals**: Standard compiler performance; UTF-8 flag overhead negligible (<1% build time impact measured as wall-clock time for clean rebuild of jsav target with and without UTF-8 flags, averaged over 5 runs)
**Constraints**: UTF-8 support adds negligible build time overhead (<1% measured against identical build without UTF-8 flags); fail-fast on UTF-8 console initialization failure; no additional third-party dependencies; platform-specific code encapsulated behind portable interfaces
**Scale/Scope**: Single feature branch affecting build system (CMake), runtime initialization (header), and test suite

## Constitution Check

*GATE: Must pass before Phase 0 research. Re-check after Phase 1 design.*

### Initial Gate Evaluation

| Constitution Principle       | Status    | Justification                                                                                                                                                                                                                           |
|------------------------------|-----------|-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------|
| **I. Platform Independence** | ✅ PASS   | UTF-8 handling uses cross-platform compiler flags (`-finput-charset=UTF-8 -fexec-charset=UTF-8` for GCC/Clang, `/utf-8` for MSVC). Windows-specific `SetConsoleOutputCP` is encapsulated behind portable interface (`include/jsav/utf8/console.hpp`) with platform-specific implementations (`utf8_console_win.cpp`, `utf8_console_unix.cpp`). Linux/macOS require no runtime initialization. |
| **II. Visual Studio 2026 Compatibility** | ✅ PASS | MSVC 2022+ with `/utf-8` flag is fully supported. All CMake modules use standard CMake 3.29+ commands. No deprecated MSVC features used. Platform abstraction uses only standard C++23 and approved Win32 API.                                                                                               |
| **III. C++ Core Guidelines Compliance** | ✅ PASS | No raw `new`/`delete`; uses CMake functions and configured files. Platform abstraction encapsulates Win32 API behind portable interface (complies with "avoiding platform-specific APIs unless absolutely necessary and properly abstracted"). Function complexity trivial (<10 lines). Zero static analysis violations expected.        |
| **IV. Test-Driven Development** | ✅ PASS | All functionality covered by Catch2 tests in existing `tests.cpp` with `[utf8]` tag. Tests cover: string literal preservation, BOM absence, console initializer behavior, locale independence, FR-009 diagnostic logging.             |
| **V. Dependency Management** | ✅ PASS   | No new third-party dependencies. Uses compiler built-ins, Win32 API (already available), CMake native commands. fmt 12.1.0 already pinned for diagnostics.                                                                             |
| **VI. Documentation Standards** | ✅ PASS | This spec and plan follow markdownlint rules. Quickstart.md will document integration steps. All Markdown validated per `.vscode/settings.json`.                                                                                        |

**Initial Gate Status**: ✅ PASS - All principles satisfied, no violations requiring justification.

---

### Post-Design Re-Evaluation (After Phase 1)

**Re-Evaluation Date**: 2026-02-27  
**Artifacts Reviewed**: research.md, data-model.md, contracts/, quickstart.md

| Constitution Principle       | Status            | Changes Since Initial Review                                                                                                                                                                                                                                                  |
|------------------------------|-------------------|-------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------|
| **I. Platform Independence** | ✅ PASS (Confirmed) | Design artifacts confirm: CMake modules use cross-platform commands; platform abstraction layer (`console.hpp`, `utf8_console_win.cpp`, `utf8_console_unix.cpp`) encapsulates Win32 API; Linux/macOS behavior is no-op.                                            |
| **II. Visual Studio 2026 Compatibility** | ✅ PASS (Confirmed) | Generated header template and platform abstraction use only standard C++23 and approved Win32 API; no deprecated features introduced.                                                                                                                                                                 |
| **III. C++ Core Guidelines Compliance** | ✅ PASS (Confirmed) | data-model.md confirms: `init_console()` is 15 lines (within ≤100 limit), cyclomatic complexity = 1 (within ≤15 limit), 0 parameters (within ≤6 limit). CMake functions are ≤40 lines. Platform abstraction complies with "avoiding platform-specific APIs unless properly abstracted".                   |
| **IV. Test-Driven Development** | ✅ PASS (Confirmed) | quickstart.md includes test integration steps; contracts specify `[utf8]` tag for selective test execution; test coverage includes all FR-001 through FR-009 requirements.                                                                                                     |
| **V. Dependency Management** | ✅ PASS (Confirmed) | contracts/ confirms: no new dependencies added; uses existing fmt 12.1.0, Win32 API (built-in), CMake native commands.                                                                                                                                                        |
| **VI. Documentation Standards** | ✅ PASS (Confirmed) | All Phase 1 artifacts (research.md, data-model.md, contracts/*.md, quickstart.md) follow markdownlint rules; terminology consistent; cross-references functional.                                                                                                              |

**Post-Design Gate Status**: ✅ PASS - No emergent violations detected during artifact generation. Design remains compliant with all constitution principles.

**Drift Analysis**: No drift detected between initial Technical Context and Phase 1 artifacts. All decisions in research.md align with constitution principles.

## Project Structure

### Documentation (this feature)

```text
specs/[###-feature]/
├── plan.md              # This file (/speckit.plan command output)
├── research.md          # Phase 0 output (/speckit.plan command)
├── data-model.md        # Phase 1 output (/speckit.plan command)
├── quickstart.md        # Phase 1 output (/speckit.plan command)
├── contracts/           # Phase 1 output (/speckit.plan command)
└── tasks.md             # Phase 2 output (/speckit.tasks command - NOT created by /speckit.plan)
```

### Source Code (repository root)

```text
cmake/
├── Utf8Compiler.cmake      # Compiler flag configuration
├── Utf8Console.cmake       # Console output setup (selects platform implementation)
└── Utf8BomStrip.cmake      # BOM detection/stripping

configured_files/
└── include/
    └── jsav/
        └── utf8_console.hpp.in  # Legacy template for backward compatibility (deprecated, use include/jsav/utf8/console.hpp)

include/jsav/utf8/
└── console.hpp             # Portable interface for console initialization (primary API). On Linux/macOS, init_console() is no-op for API consistency.

src/jsav_Lib/
├── utf8_console_win.cpp    # Windows implementation (SetConsoleOutputCP)
└── utf8_console_unix.cpp   # Linux/macOS implementation (no-op)

src/jsav/
└── main.cpp                # Main executable (calls jsav::utf8::init_console())

test/
└── tests.cpp               # Extended with [utf8] test cases
```

**Structure Decision**: Single project structure (Option 1). UTF-8 support implemented as CMake modules, configured header, and platform-abstraction library, integrated into existing build system. No new subdirectories required beyond existing project layout.

**Platform Abstraction Decision**: Win32 API calls encapsulated in `utf8_console_win.cpp` behind portable interface in `console.hpp` to comply with Constitution Principle I (Platform Independence) and Principle III (C++ Core Guidelines).

## Complexity Tracking

All functions within thresholds (CMake functions ≤50 lines, generated header ≤20 lines). No violations requiring justification.
