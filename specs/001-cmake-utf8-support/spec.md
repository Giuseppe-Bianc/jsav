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

## Requirements

### Functional Requirements

- **FR-001**: CMake configuration MUST set compiler flags to treat source files as UTF-8 encoded: `/utf-8` for MSVC, `-finput-charset=UTF-8 -fexec-charset=UTF-8` for GCC and Clang.
- **FR-002**: Compiler output (stdout/stderr) MUST be configured to use UTF-8 encoding on Windows to preserve international characters in build messages.
- **FR-003**: Runtime console output from compiled executables MUST support UTF-8 character display on Windows (via `SetConsoleOutputCP(CP_UTF8)`), Linux, and macOS.
- **FR-004**: CMake configuration MUST handle UTF-8 characters in file paths and directory names correctly during build configuration and execution.
- **FR-005**: Build system MUST preserve UTF-8 byte sequences in string literals without modification or reinterpretation.
- **FR-006**: CMake presets and configuration files MUST be saved and read as UTF-8 to support international characters in configuration values.
- **FR-007**: System MUST accept UTF-8 source files with or without BOM; BOM MUST be stripped during preprocessing if present to ensure consistent output.
- **FR-008**: System MUST require all source files to be UTF-8 encoded; legacy encodings (Windows-1252, ISO-8859-1, etc.) are not supported and developers are responsible for converting files to UTF-8 before inclusion.

### Key Entities

- **Source Files**: C++ source and header files that may contain UTF-8 encoded characters in comments and string literals.
- **Build Output**: Text output generated during compilation including compiler messages, warnings, errors, and custom build steps.
- **Console Environment**: Terminal or command prompt where build commands are executed and application output is displayed.
- **CMake Configuration**: CMakeLists.txt files, presets, and cache variables that control build behavior.

## Success Criteria

### Measurable Outcomes

- **SC-001**: Developers can successfully build the project containing UTF-8 characters in source files on Windows, Linux, and macOS with zero encoding-related build errors.
- **SC-002**: UTF-8 characters in compiler output and build logs display correctly (no mojibake, replacement characters, or garbled text) in 100% of test cases across supported platforms.
- **SC-003**: Application runtime output containing UTF-8 strings displays correctly in standard terminals on all supported operating systems without requiring manual code page configuration by users.
- **SC-004**: Build process handles source files with UTF-8 characters in file paths up to 260 characters on Windows and standard path lengths on Unix-like systems without errors.
- **SC-005**: New developers can clone the repository and build successfully on their first attempt regardless of their system's default locale or language settings.
- **SC-006**: CI/CD builds on all supported platforms complete successfully with UTF-8 content in source files, logs, and test output.

## Clarifications

### Session 2026-02-27

- Q: Which compiler flags should be used to treat source files as UTF-8? → A: Use both input and execution charset flags (`/utf-8` MSVC, `-finput-charset=UTF-8 -fexec-charset=UTF-8` GCC/Clang).
- Q: Which method should be used for UTF-8 console output on Windows? → A: Use `SetConsoleOutputCP(CP_UTF8)` in application initialization. Single call affects all console output.
- Q: Should UTF-8 identifiers (non-ASCII characters in variable/function names) be supported? → A: Explicitly exclude UTF-8 identifiers from scope. Only comments and string literals must support UTF-8.
- Q: How should UTF-8 BOM (Byte Order Mark) be handled? → A: Accept both forms, strip BOM during preprocessing if present. Ensures consistent output.

## Assumptions

- All source files in the project are expected to be saved in UTF-8 encoding (with or without BOM).
- Developers are using modern terminals that support UTF-8 (Windows Terminal, modern versions of cmd.exe with appropriate code page, or Unix terminals).
- The project targets C++23 and uses compilers that support UTF-8 source file handling (MSVC 2019+, GCC 10+, Clang 10+).
- Legacy encoding support (automatic detection/conversion) is out of scope for this feature - the focus is on proper UTF-8 handling.
- UTF-8 support in file paths assumes the underlying OS and filesystem support UTF-8 (NTFS on Windows, most modern filesystems on Linux/macOS).
- UTF-8 identifiers (non-ASCII characters in variable/function names) are explicitly out of scope; only comments and string literals require UTF-8 support.
