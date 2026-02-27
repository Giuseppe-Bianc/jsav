# Feature Specification: CMake UTF-8 Support

**Feature Branch**: `001-cmake-utf8-support`
**Created**: 2026-02-27
**Status**: Draft
**Input**: User description: "il tuo compito è abilitare il mio progetto cmake per il supporto di UTF-8 in input e output"

## User Scenarios & Testing

### User Story 1 - Build Project with UTF-8 Source Files (Priority: P1)

As a developer working with internationalized code, I want to compile source files containing UTF-8 encoded characters (such as comments and string literals) so that my project builds successfully regardless of the character encoding in my source files.

**Why this priority**: This is the core functionality - without proper UTF-8 handling during compilation, developers cannot work with internationalized source code, making this the foundational requirement for all other UTF-8 features.

**Independent Test**: Can be fully tested by attempting to build a project containing UTF-8 characters in source files and verifying successful compilation without encoding errors.

**Acceptance Scenarios**:

1. **Given** a source file containing UTF-8 characters in string literals and comments, **When** the project is built using CMake, **Then** the build completes successfully without encoding-related errors or warnings.
2. **Given** source files with UTF-8 characters saved in different editors, **When** the project is built, **Then** all UTF-8 characters are preserved correctly in the compiled output.
3. **Given** a clean build environment on Windows with different system locales, **When** the project is built, **Then** UTF-8 handling works consistently regardless of system locale settings.

---

### User Story 2 - Display UTF-8 in Build Output and Logs (Priority: P2)

As a developer, I want to see UTF-8 characters correctly displayed in compiler output, build logs, and runtime console output so that I can read error messages, debug information, and application output containing international characters.

**Why this priority**: After successful compilation, developers need to be able to read and understand build output and application logs that may contain UTF-8 characters. This is essential for debugging and internationalization verification.

**Independent Test**: Can be tested by building a project that outputs UTF-8 strings and verifying that the console/build output displays characters correctly without mojibake or replacement characters.

**Acceptance Scenarios**:

1. **Given** a build that processes files with UTF-8 paths or names, **When** CMake outputs build messages, **Then** UTF-8 characters appear correctly in the console and build logs.
2. **Given** an application that prints UTF-8 strings to console, **When** the application runs after being built with the CMake configuration, **Then** the output displays correctly in the terminal on Windows, Linux, and macOS.
3. **Given** compiler errors referencing UTF-8 content, **When** errors are displayed, **Then** the error messages preserve UTF-8 characters correctly.

---

### User Story 3 - Cross-Platform UTF-8 Consistency (Priority: P3)

As a developer working across multiple operating systems, I want UTF-8 handling to work consistently on Windows, Linux, and macOS so that my build process and application behavior are predictable regardless of the development platform.

**Why this priority**: Cross-platform consistency is important for team collaboration and CI/CD, but it builds upon the core UTF-8 compilation and output features. Teams can work on a single platform without this, but multi-platform teams need this for consistency.

**Independent Test**: Can be tested by building the same project on Windows, Linux, and macOS and verifying that UTF-8 handling produces equivalent results on all platforms.

**Acceptance Scenarios**:

1. **Given** the same source files with UTF-8 content, **When** built on Windows using MSVC, GCC, or Clang, **Then** UTF-8 characters are handled consistently across all compilers.
2. **Given** a project built on Windows, **When** the same project is built on Linux or macOS, **Then** the UTF-8 behavior and output are equivalent.
3. **Given** CI/CD pipelines running on different platforms, **When** builds execute, **Then** UTF-8 handling does not cause platform-specific build failures.

---

### Edge Cases

- What happens when source files are saved with different encodings (UTF-8 with BOM, UTF-8 without BOM, UTF-16, legacy code pages)?
- How does the system handle mixed encoding scenarios where some files are UTF-8 and others use legacy encodings?
- What happens when console/terminal doesn't support UTF-8 output (older Windows cmd.exe without proper code page)?
- How does the build system handle UTF-8 characters in file paths, especially on Windows with legacy path limitations?
- What happens when environment variables contain UTF-8 characters that affect the build process?

**Note**: Per FR-008, legacy encodings and mixed encoding scenarios are explicitly out of scope. The system requires all source files to be UTF-8 encoded. When non-UTF-8 files are encountered, the build will fail with compiler errors; developers are responsible for identifying and converting problematic files using external tools as needed.

## Requirements

### Functional Requirements

- **FR-001**: CMake configuration MUST set compiler flags to treat source files as UTF-8 encoded: `/utf-8` for MSVC, `-finput-charset=UTF-8 -fexec-charset=UTF-8` for GCC and Clang.
- **FR-002**: Compiler output (stdout/stderr) MUST be configured to use UTF-8 encoding on Windows to preserve international characters in build messages.
- **FR-003**: CMake configuration SHALL provide header files, utilities, and documentation to enable UTF-8 console output for compiled executables. On Windows, this includes providing example code for `SetConsoleOutputCP(CP_UTF8)` initialization. Applications MUST invoke the initialization code unconditionally in their main() or entry point functions (no pre-detection of terminal compatibility). If initialization fails, the application MUST fail fast with a clear diagnostic error message indicating UTF-8 console initialization failure and terminal compatibility requirements.
- **FR-004**: CMake configuration MUST handle UTF-8 characters in file paths and directory names correctly during build configuration and execution.
- **FR-005**: Build system MUST preserve UTF-8 byte sequences in string literals without modification or reinterpretation.
- **FR-006**: CMake presets and configuration files MUST be saved and read as UTF-8 to support international characters in configuration values.
- **FR-007**: System MUST accept UTF-8 source files with or without BOM; BOM MUST be stripped during preprocessing if present to ensure consistent output.
- **FR-008**: System MUST require all source files to be UTF-8 encoded; legacy encodings (Windows-1252, ISO-8859-1, etc.) are not supported and developers are responsible for converting files to UTF-8 before inclusion.
- **FR-009**: Build system MUST log UTF-8 initialization status and encoding validation results during both build time and application runtime to aid troubleshooting and verification.

### Key Entities

- **Source Files**: C++ source and header files that may contain UTF-8 encoded characters in comments and string literals.
- **Build Output**: Text output generated during compilation including compiler messages, warnings, errors, and custom build steps.
- **Console Environment**: Terminal or command prompt where build commands are executed and application output is displayed.
- **CMake Configuration**: CMakeLists.txt files, presets, and cache variables that control build behavior.

## Success Criteria

### Measurable Outcomes

- **SC-001**: Developers can successfully build the project containing UTF-8 characters in source files on Windows, Linux, and macOS with zero encoding-related build errors.
- **SC-002**: UTF-8 characters in compiler output and build logs display correctly (no mojibake, replacement characters, or garbled text) in 100% of test cases across supported platforms.
- **SC-003a**: Application runtime output containing UTF-8 strings (including Latin-1 Supplement, CJK characters, emoji, and combining diacritics) displays correctly in standard terminals on all supported operating systems when applications have integrated UTF-8 initialization code per FR-003. "Displays correctly" means no mojibake, replacement characters (�), or garbled text in test output.
- **SC-003b**: On Windows, UTF-8 output displays correctly after developers integrate initialization code per FR-003; end users do not need to manually configure code pages (e.g., 'chcp 65001'). On Linux/macOS, UTF-8 locales are required (default on modern systems); no application-level initialization is required beyond standard iostream usage. Systems with non-UTF-8 locales are unsupported and developers must configure UTF-8 locales at the OS level.
- **SC-003c**: If UTF-8 initialization fails at runtime, applications MUST fail fast with a clear diagnostic error message (exit code and explanation). The error message MUST indicate that UTF-8 console initialization failed and provide guidance on terminal compatibility requirements.
- **SC-004**: Build process handles source files with UTF-8 characters in file paths without errors. On Windows with long path support enabled (Windows 10 1607+ with `LongPathsEnabled` registry key or manifest), paths up to 32,767 characters are supported. On systems without long path support, the traditional MAX_PATH limit of 260 characters applies. Unix-like systems follow standard filesystem path length limits (typically 4096 characters).
- **SC-005**: New developers can clone the repository and build successfully on their first attempt regardless of their system's default locale or language settings.
- **SC-006**: CI/CD builds on all supported platforms complete successfully with UTF-8 content in source files, logs, and test output. UTF-8 initialization status and encoding validation results MUST appear in build logs for troubleshooting.

## Clarifications

### Session 2026-02-27

- Q: Which compiler flags should be used to treat source files as UTF-8? → A: Use both input and execution charset flags (`/utf-8` MSVC, `-finput-charset=UTF-8 -fexec-charset=UTF-8` GCC/Clang).
- Q: Which method should be used for UTF-8 console output on Windows? → A: Use `SetConsoleOutputCP(CP_UTF8)` in application initialization. Single call affects all console output.
- Q: Should UTF-8 identifiers (non-ASCII characters in variable/function names) be supported? → A: Explicitly exclude UTF-8 identifiers from scope. Only comments and string literals must support UTF-8.
- Q: How should UTF-8 BOM (Byte Order Mark) be handled? → A: Accept both forms, strip BOM during preprocessing if present. Ensures consistent output.
- Q: When UTF-8 console initialization fails at runtime, what should the application do? → A: Fail fast with clear diagnostic error message.
- Q: Should the CMake configuration provide logging or diagnostic output when UTF-8 related issues occur during build or runtime? → A: Always log UTF-8 initialization status and encoding validation results.
- Q: When a non-UTF-8 file is encountered, should the build system provide additional tooling or guidance to help developers identify and convert the problematic file? → A: Rely on compiler error messages only (no additional CMake-level detection).
- Q: What should happen if a Linux/macOS system has a non-UTF-8 locale configured? → A: Assume UTF-8 locale; document as minimum environment requirement (no detection/fallback code).
- Q: Should the UTF-8 initialization code on Windows detect if the terminal is incompatible before attempting SetConsoleOutputCP, or attempt the call and handle failures reactively? → A: Attempt initialization unconditionally; fail fast with clear error if it fails (consistent with fail-fast strategy).

## Assumptions

- All source files in the project are expected to be saved in UTF-8 encoding (with or without BOM).
- Developers are using modern terminals that support UTF-8 (Windows Terminal, modern versions of cmd.exe with appropriate code page, or Unix terminals).
- Linux/macOS systems have UTF-8 locales configured (default on modern distributions); non-UTF-8 locales are unsupported.
- The project targets C++23 (C++ standard requirement).
- UTF-8 source file handling requires compilers that support charset specification flags: MSVC with /utf-8 flag (Visual Studio 2022+), GCC with -finput-charset and -fexec-charset (GCC 13+), or Clang with equivalent flags (Clang 18+). While older compilers may support UTF-8, the listed versions are recommended minimums for robust support.
- Legacy encoding support (automatic detection/conversion) is out of scope for this feature - the focus is on proper UTF-8 handling.
- UTF-8 support in file paths assumes the underlying OS and filesystem support UTF-8 (NTFS on Windows, most modern filesystems on Linux/macOS).
- UTF-8 identifiers (non-ASCII characters in variable/function names) are explicitly out of scope; only comments and string literals require UTF-8 support.
