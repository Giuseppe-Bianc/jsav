<!--
SYNC IMPACT REPORT
==================
Version change: 1.1.1 → 1.2.0 (MINOR: Strengthened C++ Core Guidelines enforcement with explicit measurable criteria, clarified TDD test pyramid structure)

Modified Principles:
- III. C++ Core Guidelines Compliance: Enhanced with explicit measurable criteria (function length ≤100 lines, cyclomatic complexity ≤15, parameters ≤6, zero warnings from static analysis)
- IV. Test-Driven Development (Red-Green): Clarified test pyramid with three tiers (compile-time constexpr_tests, runtime unit tests, integration tests)

Added Principles:
- None

Removed Principles:
- None

Templates Status:
- plan-template.md: ✅ No updates required (Constitution Check section is generic)
- spec-template.md: ✅ No updates required (technology-agnostic by design)
- tasks-template.md: ✅ No updates required (TDD workflow aligns with Principle IV)
- agent-file-template.md: ✅ No updates required (auto-generated from plans)

Follow-up TODOs: None
-->

# jsav Constitution

## Core Principles

### I. Platform Independence

The jsav compiler MUST be implemented as an OS-independent system. All code MUST be portable across different platforms without depending on specific OS functionalities. The implementation MUST leverage C++23 standard library features exclusively, avoiding platform-specific APIs unless absolutely necessary and properly abstracted behind portable interfaces.

**Rationale**: Ensures the compiler can be built and run on Windows, Linux, and macOS without modification, maximizing accessibility and maintainability.

### II. Visual Studio 2026 Compatibility

All code MUST be fully compatible with Visual Studio 2026 and later versions. C++23 features used MUST be verified as fully supported by MSVC. Any feature with incomplete MSVC support MUST be avoided or conditionally compiled with appropriate fallbacks.

**Rationale**: Guarantees a consistent development experience for Windows developers and ensures the primary IDE target can build the entire codebase without issues.

### III. C++ Core Guidelines Compliance

The codebase MUST rigorously adhere to established C++ community guidelines and style conventions, including ISO C++ standards and C++ Core Guidelines. Compliance MUST be enforced through measurable criteria:

- **Modern C++ Practices**: Use `std::format`/`std::print` over iostreams, prefer `constexpr` where possible (leveraging C++23's expanded constexpr capabilities), use concepts for template constraints, apply `[[nodiscard]]` appropriately, use `enum class` over unscoped enums
- **Memory Management**: No raw `new`/`delete`; prefer stack allocation → `std::array`/`std::vector` → smart pointers; prefer `std::unique_ptr` over `std::shared_ptr`; follow Rule of 0 (avoid manual resource management)
- **Type Safety**: Define strong types over primitives (e.g., `Velocity{int}` not `int`), avoid C-style casts, use `const` liberally
- **Code Organization**: Prefer algorithms over raw loops, use ranged-for with `auto`, keep functions ≤100 lines, cyclomatic complexity ≤15, parameters ≤6 per function
- **Enforcement Mechanisms**: Code reviews MUST verify compliance; static analysis tools (clang-tidy, cppcheck) MUST report zero warnings; sanitizers (AddressSanitizer, UndefinedBehaviorSanitizer) MUST pass without violations; lizard complexity analysis MUST show all functions within thresholds

**Rationale**: Ensures code consistency, readability, maintainability, and adherence to industry best practices for long-term sustainability. Measurable criteria enable automated compliance validation.

### IV. Test-Driven Development (Red-Green)

All code MUST be developed using the Red-Green TDD methodology:

1. **Red Phase**: Write a failing test first that defines the desired behavior
2. **Green Phase**: Implement the minimum code necessary to make the test pass
3. **Refactor Phase**: Only after tests pass, refactor while maintaining full test coverage

The test pyramid MUST be followed with three distinct test tiers:

- **Compile-time tests** (`constexpr_tests.cpp`): For compile-time verification using `STATIC_REQUIRE`. If it compiles, tests pass.
- **Runtime unit tests** (`tests.cpp`): For runtime functionality using Catch2 framework with `REQUIRE` assertions
- **Integration tests**: For component interactions, system-level behavior, and I/O operations

No code MUST be committed without corresponding tests. Tests MUST be written before implementation code. The workflow for adding tests MUST be:

1. Add tests to `relaxed_constexpr_tests` first (runtime version)
2. Debug and fix issues
3. Verify they compile in `constexpr_tests` (compile-time version)
4. For non-constexpr functionality, add to `tests.cpp`

**Rationale**: Guarantees code reliability, enables safe refactoring, provides living documentation, and ensures incremental quality control throughout development. Three-tier structure maximizes compile-time verification while supporting runtime testing.

### V. Dependency Management

The project MUST use the following approved dependencies:

- **fmtlib::fmt**: For formatting operations (when std::format unavailable)
- **spdlog::spdlog**: For structured logging
- **CLI11::CLI11**: For command-line interface parsing
- **Catch2::Catch2WithMain**: For testing framework (test-only dependency)

All dependencies MUST be managed via CPM.cmake (CMake Package Manager). No additional dependencies MAY be added without explicit justification and approval.

**Rationale**: Maintains a minimal, well-curated dependency surface, reducing build complexity, security risks, and maintenance burden.

### VI. Documentation Standards & markdownlint Compliance

All project documentation MUST adhere to the following standards:

- **markdownlint Compliance**: All Markdown documents MUST conform to the configuration specified in `.vscode/settings.json` under `markdownlint.config`. No violations MAY be committed.
- **Precision & Rigor**: Documentation MUST describe systematically every relevant element, paying attention to both major components and minor details, including their interconnections.
- **Completeness**: Documentation MUST be coherent, complete, and clearly understandable, ensuring accurate and structured representation of information.
- **Consistency**: Terminology, formatting, and structure MUST remain consistent across all documents.

**Rationale**: High-quality documentation is essential for maintainability, onboarding, and long-term project sustainability. Automated linting ensures consistency; rigorous content ensures usefulness.

## Development Workflow

All development MUST follow this workflow:

1. **Branch Creation**: Create feature branch from `main` with naming convention `###-feature-name`
2. **TDD Cycle**: Follow Red-Green-Refactor for each increment
3. **Static Analysis**: All code MUST pass clang-tidy and cppcheck without warnings
4. **Code Coverage**: New code MUST maintain adequate test coverage (verified via gcovr)
5. **Code Formatting**: Run clang-format on all modified files before commit
6. **Sanitizer Verification**: Code MUST pass AddressSanitizer and UndefinedBehaviorSanitizer checks
7. **Complexity Verification**: Code MUST pass lizard complexity thresholds

## Quality Gates

Before any code MAY be merged to `main`, the following gates MUST pass:

- **Build**: Zero compile errors or warnings (warnings treated as errors)
- **Tests**: All test targets pass (constexpr, relaxed_constexpr, runtime)
- **Static Analysis**: clang-tidy and cppcheck report zero issues
- **Sanitizers**: No sanitizer violations detected
- **Complexity**: lizard analysis shows all functions within thresholds
- **Coverage**: New code maintains acceptable coverage levels
- **Formatting**: All code formatted per `.clang-format` configuration
- **Documentation**: All Markdown documents pass markdownlint validation per `.vscode/settings.json` configuration

## Governance

This constitution supersedes all other development practices and guidelines for the jsav project. Compliance is mandatory and verified through automated tooling in CI/CD pipelines.

**Amendment Procedure**:

1. Propose amendment with rationale
2. Update version according to semantic versioning
3. Document changes in Sync Impact Report
4. Update dependent templates if necessary
5. Obtain approval before merging

**Versioning Policy**:

- **MAJOR**: Backward-incompatible changes (principle removal, redefinition)
- **MINOR**: New principles, material expansions, new sections
- **PATCH**: Clarifications, wording improvements, typo fixes

**Compliance Review**: All pull requests MUST be reviewed for constitution compliance. CI/CD pipelines enforce automated compliance checks.

**Version**: 1.2.0 | **Ratified**: 2026-02-25 | **Last Amended**: 2026-02-26
