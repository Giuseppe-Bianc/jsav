<!--
SYNC IMPACT REPORT
==================
Version change: 1.5.0 → 1.6.0 (MINOR)

Modified Principles:
- III. C++ Core Guidelines Compliance: Added explicit RAII enforcement mandate,
  resource management checklist (error handling, boundary conditions, resource
  lifecycle, concurrency), and defined responsibility roles (code owner, reviewer,
  integration manager) with measurable compliance criteria

Added Principles:
- VIII. STL Algorithm Exclusivity: Requires mandatory preference for STL algorithms
  over custom implementations when semantically equivalent alternatives exist; all
  STL algorithms used MUST be fully supported by Visual Studio 2026; derogations
  require documented technical justification and technical review approval

Removed Principles:
- None

Templates Status:
- plan-template.md: ✓ No changes needed (Constitution Check section is generic;
  references constitution file dynamically)
- spec-template.md: ✓ No changes needed (technology-agnostic by design; STL
  preference is implementation detail)
- tasks-template.md: ✓ No changes needed (TDD workflow aligns with Principle IV;
  STL preference affects implementation tasks, not structure)

Follow-up TODOs: None

Version Bump Justification:
- MINOR: New Principle VIII (STL Algorithm Exclusivity) added — materially expands
  governance scope with enforceable rules for algorithm selection. Principle III
  expanded with responsibility assignments and resource management checklists.
-->

# jsav Constitution

## Core Principles

### I. Platform Independence

The jsav compiler MUST be implemented as an OS-independent system. All code MUST be portable across
different platforms without depending on specific OS functionalities. The implementation MUST leverage
C++23 standard library features exclusively. Platform-specific APIs MUST be avoided; if unavoidable
due to OS integration requirements, they MUST be isolated behind portable abstraction layers with
clear documentation justifying their necessity. The compiler MUST accept
`.vn` extension files as input and generate optimized executables compatible with the target
operating system.

**Rationale**: Ensures the compiler can be built and run on Windows, Linux, and macOS without
modification, maximizing accessibility and maintainability.

### II. Visual Studio 2026 Compatibility

All code MUST be fully compatible with Visual Studio 2026 and later versions. C++23 features used
MUST be verified as fully supported by MSVC. Any feature with incomplete MSVC support MUST be
avoided or conditionally compiled with appropriate fallbacks. This constraint applies to the
development and compilation environment only; it does not compromise the portability objective
of the compiled software with respect to target operating systems.

**Rationale**: Guarantees a consistent development experience for Windows developers and ensures
the primary IDE target can build the entire codebase without issues.

### III. C++ Core Guidelines Compliance

The codebase MUST rigorously adhere to established C++ community guidelines and style conventions,
including ISO C++ standards and C++ Core Guidelines. Compliance MUST be enforced through measurable
criteria and explicit patterns.

#### Mandatory Patterns

**Pattern: Ownership Semantics Explicit**

- Every resource MUST have a single, clearly identified owner
- Use `std::unique_ptr` for exclusive ownership, `std::shared_ptr` only when sharing is strictly necessary
- Raw pointers MUST be used exclusively as non-owning references of short duration
- Use `std::span` or references to pass non-owning views over data sequences
- Explicit `delete` MUST be confined to custom deleters only; never in application code
- AddressSanitizer MUST report zero leaks in test suite execution

**Pattern: Pervasive Const Correctness**

- All local variables MUST be declared `const` by default; remove qualifier only if mutation is required
- All methods that do not modify object state MUST be declared `const`
- Prefer `std::string_view` over `const std::string&` for read-only string parameters
- Parameters passed by reference MUST be `const&` unless mutation is required; justify any absence explicitly
- Static analysis MUST be configured to flag non-const variables that are never modified

**Pattern: Conscious Move Semantics**

- Classes managing resources MUST implement move constructor and move assignment operator per Rule of Five
- Return local objects by value, relying on copy elision and move semantics
- Use `std::move` explicitly at call sites when transferring ownership is intended
- Mark move constructors and operators as `noexcept` when possible to enable container optimizations
- Profilers MUST verify no unexpected copies of large objects occur

**Pattern: Structured Error Handling**

- Recoverable errors MUST be signaled via `std::expected<T, ErrorType>` (C++23) or equivalent
- Exceptions MUST be reserved for non-local, unrecoverable exceptional conditions
- Error codes MUST NOT be ignored; all return values indicating errors MUST be handled
- Error messages MUST include sufficient context for diagnosis (what failed, why, context like file/index)
- Static analysis MUST be configured to fail CI if error return values are ignored

**Pattern: RAII Resource Management**

- All resources (memory, file handles, locks, network connections) MUST be managed via RAII wrappers
- Constructors MUST acquire resources; destructors MUST release them
- Resource acquisition MUST NOT fail after partial acquisition (strong exception guarantee)
- Smart pointers MUST be used for all heap-allocated objects; raw `new`/`delete` MUST NOT appear in application code
- `std::make_unique` and `std::make_shared` MUST be preferred over direct constructor calls for smart pointers

**Pattern: Undefined Behavior Elimination**

- Code MUST NOT invoke undefined behavior under any circumstances including: null dereference, signed
  integer overflow, out-of-bounds access, use-after-free, data races
- All pointer dereferences MUST be preceded by null checks unless invariants guarantee non-null
- Array and container access MUST use bounds-checked methods (`at()`) unless performance profiling
  demonstrates `operator[]` is necessary and safety is proven by other means
- Sanitizers (ASan, UBSan, TSan) MUST pass without violations in CI

#### Resource Management Checklist

Every module handling resources MUST explicitly document and verify:

1. **Error Handling**: All error paths MUST release acquired resources (RAII guarantees this)
2. **Boundary Conditions**: Empty inputs, maximum inputs, malformed data MUST be tested
3. **Resource Lifecycle**: Every acquisition MUST have a corresponding release (verified by ASan zero-leaks)
4. **Concurrency Safety**: Shared mutable state MUST be protected by mutexes or atomic operations;
   data races MUST be eliminated (verified by ThreadSanitizer where applicable)

#### Prohibited Anti-Patterns

**Anti-Pattern: Raw Pointer Ownership**

- NEVER use raw pointers (`T*`) to represent ownership of dynamically allocated resources
- NEVER delegate manual `delete` responsibility to programmers
- Rationale: Manual memory management is fragile; every execution path (including exception branches)
  must guarantee resource release

**Anti-Pattern: Const-Avoidance**

- NEVER omit `const` qualifier for variables, parameters, or methods when mutation is not required
- NEVER justify absence of `const` with "for simplicity"
- Rationale: Absence of `const` hides intent, prevents compiler optimizations, and signals ambiguity

**Anti-Pattern: Exception Swallowing**

- NEVER catch exceptions with empty `catch` blocks or log-only handlers that don't propagate or handle
- NEVER catch generic exceptions (`catch(...)`) except at outermost level for logging and controlled termination
- Rationale: Swallowing exceptions masks failures, allowing program to continue in inconsistent state

**Anti-Pattern: Undefined Behavior Tolerance**

- NEVER accept code that may invoke undefined behavior (null dereference, signed integer overflow,
  out-of-bounds access)
- NEVER justify UB with "it works in practice" or "compiler doesn't complain"
- Rationale: UB authorizes compiler to any transformation; different configurations may expose bugs

#### Enforcement Mechanisms

- Code reviews MUST verify compliance with all patterns and absence of anti-patterns
- Static analysis tools (clang-tidy, cppcheck) MUST report zero warnings
- Sanitizers (AddressSanitizer, UndefinedBehaviorSanitizer) MUST pass without violations
- Lizard complexity analysis MUST show all functions within thresholds:
    - Function length: ≤100 lines
    - Cyclomatic complexity: ≤15
    - Parameters: ≤6 per function

#### Responsibility Assignments

- **Code Owner**: Each module MUST have a designated code owner responsible for architectural
  decisions, API stability, and compliance with Principle III patterns
- **Reviewer**: Every pull request MUST be reviewed by at least one person other than the author;
  the reviewer MUST verify Principle III compliance explicitly before approval
- **Integration Manager**: Merges to `main` MUST be approved by an integration manager who verifies
  all quality gates pass (build, tests, static analysis, sanitizers, complexity, coverage, formatting)

**Rationale**: Ensures code consistency, readability, maintainability, and adherence to industry
best practices for long-term sustainability. Measurable criteria enable automated compliance validation.

### IV. Test-Driven Development (Red-Green)

All code MUST be developed using the Red-Green TDD methodology:

1. **Red Phase**: Write a failing test first that defines the desired behavior
2. **Green Phase**: Implement the minimum code necessary to make the test pass
3. **Refactor Phase**: Only after tests pass, refactor while maintaining full test coverage

#### Mandatory Patterns

**Pattern: Test as Executable Specification**

- Test names MUST describe behavior in domain language, not implementation details
- Format: `[Unit]_[Scenario]_[ExpectedResult]` (e.g., `Lexer_EmptyInput_ReturnsEOF`)
- Each test MUST verify a single behavior or scenario
- Tests MUST follow Arrange-Act-Assert pattern with visual separation
- Tests MUST be organized by functionality or module, not by implementation class
- Each test MUST be understandable by a new team member reading only the test

**Pattern: Intentional Minimal Implementation**

- During Green phase, implement ONLY the code strictly necessary to pass the current test
- Resist adding parameters, cases, or abstractions not required by the current test
- Implementation may be "ugly" or hardcoded; it will be improved in refactoring
- Speed of cycle is prioritized over temporary elegance
- Time between Red and Green MUST NOT exceed 10-15 minutes; if so, test is too ambitious

**Pattern: Refactoring Under Test Protection**

- Refactoring MUST occur ONLY when entire test suite passes (Green state)
- Apply one refactoring transformation at a time
- Run tests after each transformation
- If tests fail, immediately revert the last change
- Conclude refactoring session with separate commit documenting transformations applied

**Pattern: Strategic Edge Case Coverage**

- Identify and systematically test boundary conditions common in compiler domain
- Explicitly test: empty inputs, maximum inputs, special characters, invalid sequences, overflow conditions
- Each edge case MUST be a separate test with descriptive name
- For lexer: test empty string, single character, token at buffer boundaries, non-ASCII characters,
  malformed escape sequences
- For parser: test empty expressions, maximum nesting, missing operators, unbalanced parentheses
- For semantic analysis: test undeclared identifiers, incompatible types, scope overflow

#### Prohibited Anti-Patterns

**Anti-Pattern: Test-After Development**

- NEVER write implementation first and add tests afterward
- NEVER write tests to "cover" already-written code
- Rationale: Tests written after implementation test the code as-written, not as-it-should-behavior;
  tests become fragile and coupled to implementation details

**Anti-Pattern: Premature Gold Plating**

- NEVER implement more than necessary during Green phase
- NEVER add optimizations, handle untested cases, or create anticipated abstractions
- NEVER justify with "we'll need it later" or "it's more elegant"
- Rationale: Anticipatory implementation violates YAGNI; introduces unverified complexity

**Anti-Pattern: Refactoring Without Net**

- NEVER modify code structure while tests are failing
- NEVER skip running tests after each refactoring change
- NEVER confide in ability to "keep track" of changes mentally
- Rationale: Refactoring is safe only when tests provide immediate feedback

**Anti-Pattern: Fragile Tests**

- NEVER verify implementation details instead of observable behaviors
- NEVER test: internal call order, intermediate data structures, exact log messages
- NEVER test "how"; test "what"
- Rationale: Fragile tests create disincentive to refactoring; every structural change requires
  test rewrites

#### Test Pyramid Structure

The test pyramid MUST be followed with three distinct test tiers:

- **Compile-time tests** (`constexpr_tests.cpp`): For compile-time verification using `STATIC_REQUIRE`.
  If it compiles, tests pass.
- **Runtime unit tests** (`tests.cpp`): For runtime functionality using Catch2 framework with `REQUIRE`
  assertions
- **Integration tests**: For component interactions, system-level behavior, and I/O operations

No code MUST be committed without corresponding tests. Tests MUST be written before implementation code.
The workflow for adding tests MUST be:

1. Add tests to `relaxed_constexpr_tests` first (runtime version)
2. Debug and fix issues
3. Verify they compile in `constexpr_tests` (compile-time version)
4. For non-constexpr functionality, add to `tests.cpp`

**Rationale**: Guarantees code reliability, enables safe refactoring, provides living documentation,
and ensures incremental quality control throughout development. Three-tier structure maximizes
compile-time verification while supporting runtime testing.

### V. Dependency Management

The project MUST use the following approved dependencies:

- **fmtlib::fmt**: For formatting operations (when std::format unavailable)
- **spdlog::spdlog**: For structured logging
- **CLI11::CLI11**: For command-line interface parsing
- **Catch2::Catch2WithMain**: For testing framework (test-only dependency)

All dependencies MUST be managed via CPM.cmake (CMake Package Manager). No additional dependencies
MAY be added without explicit justification and approval.

#### Mandatory Patterns

**Pattern: Explicit Dependency Versioning**

- Every dependency MUST specify exact versions (e.g., `fmt/10.1.1`) not open ranges
- Lock files (e.g., `conan.lock`, vcpkg baseline) MUST be committed to repository
- Document explicit procedure for dependency updates including full test suite execution
- Verify dependency license compatibility with project requirements
- Document minimum supported versions of each dependency in README

**Pattern: Dependency Header Isolation**

- Dependency headers MUST be included ONLY in implementation files (`.cpp`) or dedicated internal headers
- Project public headers MUST NEVER expose dependency types directly
- Use forward declarations where possible to avoid inclusions in headers
- Configure CMake targets with `PRIVATE` (not `PUBLIC`) for dependencies that must not propagate
- Test external consumer compilation to verify no dependency exposure

#### Prohibited Anti-Patterns

**Anti-Pattern: Floating Dependencies**

- NEVER configure dependencies without specifying versions
- NEVER use "latest" or overly broad ranges
- NEVER rely implicitly on version available at build time
- Rationale: Without locked versions, builds at different times produce different results; silent
  updates introduce incompatibilities

**Anti-Pattern: Header Leak**

- NEVER include dependency headers in project public headers
- NEVER expose dependency types and interfaces to consumers
- Rationale: Exposing dependency headers creates transitive coupling; consumers become dependent
  on external libraries

**Rationale**: Maintains a minimal, well-curated dependency surface, reducing build complexity,
security risks, and maintenance burden.

### VI. Documentation Standards & markdownlint Compliance

All project documentation MUST adhere to the following standards:

- **markdownlint Compliance**: All Markdown documents MUST conform to the configuration specified
  in `.vscode/settings.json` under `markdownlint.config`. No violations MAY be committed.
- **Precision & Rigor**: Documentation MUST describe systematically every relevant element, paying
  attention to both major components and minor details, including their interconnections.
- **Completeness**: Documentation MUST be coherent, complete, and clearly understandable, ensuring
  accurate and structured representation of information.
- **Consistency**: Terminology, formatting, and structure MUST remain consistent across all documents.
- **Logical Coherence**: Content MUST progress in a logical order; related concepts MUST be grouped
  and cross-referenced.
- **Terminological Consistency**: The same term MUST always refer to the same concept; synonyms MUST
  be explicitly identified with a single canonical term chosen.
- **Completeness**: All relevant elements, essential data, and any conditions or limitations MUST be
  explicitly reported, avoiding omissions that could compromise correct interpretation.
- **Comprehensibility**: Formulation MUST be clear, precise, and appropriate for the target audience,
  avoiding obscure, redundant, or unnecessarily technical formulations unless strictly necessary.
- **Hierarchical Structure**: Information MUST be organized according to a logical and hierarchical
  structure, articulated in coherent sections or paragraphs, presented to facilitate consultation,
  traceability, and content verification.

#### Mandatory Patterns

**Pattern: Continuous Formatting Validation**

- Editor MUST be configured to show markdownlint violations in real-time
- Pre-commit hook MUST execute markdownlint and block commits with errors
- CI MUST include Markdown validation step that fails on violations
- Use `markdownlint --fix` for automatic correction where possible
- Document manual validation procedure in README

**Pattern: Project Terminology Glossary**

- Maintain `GLOSSARY.md` in documentation root defining key terms with precise definitions
- All documents MUST reference glossary for technical terms
- Synonyms MUST be explicitly identified; one canonical term chosen
- Glossary MUST be versioned alongside code
- During documentation code review, verify consistent terminology usage

**Pattern: Navigable Hierarchical Structure**

- Every document MUST have single level-1 heading (title)
- Heading levels MUST descend progressively without skipping
- Section titles MUST be descriptive and self-explanatory
- Heading structure MUST form navigable content index
- Limit depth to 4 levels; if more needed, consider splitting into separate documents
- Generate and include automatic table of contents for long documents

#### Prohibited Anti-Patterns

**Anti-Pattern: Deferred Validation**

- NEVER postpone markdownlint compliance verification to later phase
- NEVER accumulate violations for "batch correction"
- Rationale: Accumulated violations are harder to correct; context is forgotten; time pressure
  leads to superficial fixes

**Anti-Pattern: Terminological Drift**

- NEVER use different terms for same concept across or within documents
- NEVER vary terminology for "stylistic variety"
- Rationale: Variation creates ambiguity; readers cannot determine if "module", "component", and
  "unit" refer to same or distinct concepts

**Anti-Pattern: Wall of Text**

- NEVER produce documents without hierarchical structure
- NEVER write long paragraphs without headings, lists, or visual organization
- Rationale: Technical readers scan for specific information; unstructured text forces integral
  reading

**Rationale**: High-quality documentation is essential for maintainability, onboarding, and
long-term project sustainability. Automated linting ensures consistency; rigorous content
ensures usefulness.

### VII. Algorithmic Design Excellence

When problem characteristics permit, the implementation MUST employ advanced algorithmic design
techniques to achieve efficient, scalable solutions with optimal computational complexity. The
choice of algorithmic paradigm MUST be justified through formal analysis.

#### Mandatory Algorithmic Paradigms

The following algorithmic design techniques MUST be considered and applied when appropriate:

**Dynamic Programming**

- MUST be used when problem exhibits overlapping subproblems and optimal substructure
- Implementation MUST use memoization or tabulation to avoid recomputation
- Time and space complexity trade-offs MUST be explicitly documented
- Example applications: parsing algorithms, optimization problems, sequence analysis

**Greedy Algorithms**

- MUST be used when problem satisfies greedy choice property
- Proof or justification of greedy choice validity MUST be documented
- Local optimality leading to global optimality MUST be demonstrated
- Example applications: scheduling, Huffman coding, minimum spanning trees

**Divide and Conquer**

- MUST be used when problem can be decomposed into independent subproblems
- Recurrence relations MUST be analyzed for time complexity
- Base cases and combination steps MUST be clearly identified
- Example applications: sorting, searching, matrix multiplication

**Backtracking**

- MUST be used for constraint satisfaction and combinatorial search problems
- Pruning strategies MUST be documented and justified
- Worst-case complexity MUST be analyzed
- Example applications: parsing ambiguous grammars, constraint solving

**Branch and Bound**

- MUST be used for discrete optimization problems when exact solutions required
- Bounding functions MUST be documented with complexity analysis
- Pruning criteria MUST be explicitly stated
- Example applications: combinatorial optimization, integer programming

#### Formal Analysis Requirements

For every algorithm employing advanced techniques, documentation MUST include:

**Problem Analysis**

- Formal problem statement with clear objectives and constraints
- Input data structure and size characteristics
- Required optimality level (exact vs. approximate solutions)

**Structural Property Identification**

- Presence of overlapping subproblems (for dynamic programming)
- Greedy choice property validation (for greedy algorithms)
- Subproblem independence (for divide and conquer)
- Constraint structure (for backtracking/branch and bound)

**Complexity Analysis**

- Time complexity using asymptotic notation (O, Θ, Ω)
- Space complexity including auxiliary data structures
- Trade-offs between different algorithmic approaches
- Expected vs. worst-case performance characteristics

**Strategy Comparison**

- Systematic evaluation of alternative paradigms
- Justification for selected approach over alternatives
- Performance bounds and scalability analysis

**Algorithm Construction**

- Detailed step-by-step construction process
- Data structures employed with justification
- Decision-making logic at each computational step

**Verification & Validation**

- Correctness proof or justification
- Rigorous complexity analysis
- Performance evaluation with representative test cases
- Benchmark results under realistic scenarios

#### Complexity Goals

Algorithmic solutions MUST target the following complexity characteristics:

- **Time Complexity**: Minimize asymptotic time complexity for problem class
- **Space Complexity**: Balance memory usage against time requirements
- **Scalability**: Handle large input sizes gracefully (analyze growth rates)
- **Resource Efficiency**: Optimize for target platform constraints (memory, CPU)

#### Documentation Requirements

All algorithmic implementations MUST include:

- Inline comments explaining non-obvious algorithmic choices
- References to algorithmic paradigm and justification
- Complexity analysis in function documentation
- Edge case handling rationale

**Rationale**: Compiler implementation involves complex parsing, optimization, and code generation
problems where algorithmic choices directly impact performance, scalability, and correctness.
Formal analysis ensures informed decisions and maintainable implementations.

### VIII. STL Algorithm Exclusivity

When technically appropriate, STL algorithms MUST be preferred over custom implementations for
recurring operations including but not limited to: sorting, searching, transformation, filtering,
aggregation, counting, partitioning, reduction, and data collection manipulation. All STL algorithms
used MUST be fully supported by the compiler, standard library, and entire toolchain of Visual Studio 2026 and later versions.

#### Mandatory Rules

**Rule: STL Preference**

- An STL algorithm fulfilling the same functional requirement MUST be used in preference to custom
  implementations
- Priority MUST be given to functional correctness, followed by exposition clarity, architectural
  coherence, and overall codebase quality
- Practices fully conforming to modern C++ standards and shared development conventions MUST be favored

**Rule: Visual Studio 2026 Compatibility**

- All STL algorithms used MUST be fully supported by Visual Studio 2026 and later versions
- Any algorithm with incomplete or uncertain MSVC support MUST be avoided or conditionally compiled
  with documented fallbacks
- Extensions, proprietary constructs, non-standard constructs, or custom algorithms that may compromise
  full compatibility with Visual Studio 2026 or later versions MUST NOT be used

**Rule: Derogation Procedure**

- Derogations from the STL preference rule are permitted ONLY in the presence of specific technical
  requirements such as: measurable performance constraints, documented architectural necessities,
  interoperability requirements, or absence of a semantically equivalent standard solution
- Each derogation MUST be explicitly documented within the module's technical documentation or source
  code comments
- Each derogation MUST be submitted for technical review and approval before merge

#### Verification

- Code reviews MUST verify that STL algorithms are used where semantically applicable
- Static analysis SHOULD flag manual reimplementations of standard algorithms (e.g., custom `find`,
  custom `sort`, custom `transform`)
- Derogations MUST be counted and reported in code review summaries

**Rationale**: Systematic use of STL algorithms improves code readability, modularity, and overall
maintainability while reducing the probability of logical errors arising from repetitive or
non-uniform manual implementations. Compatibility with Visual Studio 2026 ensures technical
stability, operational predictability, maintenance simplicity, and future portability of the project
even under compiler updates or ISO C++ standard migrations.

## Development Workflow

All development MUST follow this workflow:

1. **Branch Creation**: Create feature branch from `main` with naming convention `###-feature-name`
2. **TDD Cycle**: Follow Red-Green-Refactor for each increment
3. **Static Analysis**: All code MUST pass clang-tidy and cppcheck without warnings
4. **Code Coverage**: New code MUST maintain adequate test coverage (verified via gcovr)
5. **Code Formatting**: Run clang-format on all modified files before commit
6. **Sanitizer Verification**: Code MUST pass AddressSanitizer and UndefinedBehaviorSanitizer checks
7. **Complexity Verification**: lizard analysis MUST show all functions within thresholds

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

This constitution supersedes all other development practices and guidelines for the jsav project.
Compliance is mandatory and verified through automated tooling in CI/CD pipelines.

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

**Compliance Review**: All pull requests MUST be reviewed for constitution compliance. CI/CD
pipelines enforce automated compliance checks. The following roles are defined:

- **Code Owner**: Accountable for architectural coherence and API stability of their assigned modules.
  MUST verify that changes comply with Principles III, V, and VIII before approval.
- **Reviewer**: Independent verifier of pull requests. MUST NOT be the author. MUST verify compliance
  with all applicable principles, with particular attention to Principle III patterns and anti-patterns,
  Principle IV TDD workflow, and Principle VIII STL usage.
- **Integration Manager**: Final gatekeeper for merges to `main`. MUST verify all quality gates pass
  (Build, Tests, Static Analysis, Sanitizers, Complexity, Coverage, Formatting, Documentation) before
  approving the merge.

**Version**: 1.6.0 | **Ratified**: 2026-02-25 | **Last Amended**: 2026-04-08
