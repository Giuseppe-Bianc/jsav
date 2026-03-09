/// @file error_codes.hpp
/// @brief Comprehensive error code system for the jsav compiler
///
/// This module provides a complete and standardized error code management system
/// for the jsav compiler. Each error has a unique identifier (e.g., E0001) that
/// enables quick references, documentation lookup, and IDE integration.
///
/// ## Error Code Ranges
///
/// | Range      | Phase              | Description                                      |
/// |------------|--------------------|--------------------------------------------------|
/// | E0001-E0999  | Lexical Analysis   | Token recognition, literals, comments            |
/// | E1001-E1999  | Parsing            | Syntactic structure, grammar violations          |
/// | E2001-E2999  | Semantic Analysis  | Type checking, scope, declarations               |
/// | E3001-E3999  | IR Generation      | Control flow graph, SSA, control flow            |
/// | E4001-E4999  | Code Generation    | Assembly output, ABI, registers                  |
/// | E5001-E5999  | I/O & System       | File operations, CLI                             |

#pragma once

#include "../headers.hpp"

namespace jsv {

    // ---------------------------------------------------------------------------
    /// @enum Severity
    /// @brief Defines the severity levels for compiler errors
    ///
    /// Error severity determines how the diagnostic is presented and whether
    /// compilation continues.
    // ---------------------------------------------------------------------------
    enum class Severity : uint8_t {
        Note = 0,     ///< Note-level diagnostic (lowest severity, informational only)
        Warning = 1,  ///< Warning-level diagnostic (does not stop compilation)
        Error = 2,    ///< Error-level diagnostic (compilation continues with error recovery)
        Fatal = 3,    ///< Fatal error (stops compilation immediately)
    };
    std::string to_string(Severity severity);

    // ---------------------------------------------------------------------------
    /// @enum CompilerPhase
    /// @brief Defines the compiler phases where errors can occur
    ///
    /// Each error is associated with a specific compiler phase to help identify
    /// where in the compilation pipeline the error originated.
    // ---------------------------------------------------------------------------
    enum class CompilerPhase : uint8_t {
        Lexer = 0,  ///< Lexical analysis phase (tokenization)
        /*Parser = 1,
        Semantic = 2,
        IrGeneration = 3,
        CodeGeneration = 4,
        System = 5,*/
    };
    std::string to_string(CompilerPhase phase);

    // ---------------------------------------------------------------------------
    /// \enum ErrorCode error_codes.hpp jsav/error/error_codes.hpp
    /// \brief Defines all error codes for the jsav compiler
    /// \details Error codes follow a systematic naming convention:
    ///          - E0001-E0999: Lexer errors (lexical analysis)
    ///          - E1001-E1999: Parser errors (syntax analysis)
    ///          - E2001-E2999: Semantic analysis errors (type checking, scope)
    ///          - E3001-E3999: IR generation errors (intermediate representation)
    ///          - E4001-E4999: Code generation errors (assembly output)
    ///          - E5001-E5999: System/I/O errors (file operations, CLI)
    ///
    /// Each error code is unique and can be used to reference documentation,
    /// search error databases, and integrate with IDE diagnostic tools.
    // ---------------------------------------------------------------------------
    enum class ErrorCode {
        // -------------------------------------------------------------------------
        // Lexer Errors (E0001-E0999)
        // -------------------------------------------------------------------------
        E0001,  ///< Token not valid or unrecognized
        E0002,  ///< Malformed binary numeric literal (e.g., `0b2`, `0b102`)
        E0003,  ///< Malformed octal numeric literal (e.g., `0o8`, `0o9`)
        E0004,  ///< Malformed hexadecimal numeric literal (e.g., `0xG`, `0xZZ`)
        E0005,  ///< Unterminated string literal (missing closing `"`)
        E0006,  ///< Unterminated character literal (missing closing `'`)
        E0007,  ///< Invalid escape sequence in string or character literal
        E0008,  ///< Unterminated multi-line comment (missing `*/`)
        E0009,  ///< Invalid numeric suffix on literal
        E0010,  ///< Numeric literal overflow (value exceeds type limits)

        // -------------------------------------------------------------------------
        // Parser Errors (E1001-E1999)
        // -------------------------------------------------------------------------
        E1001,  ///< Maximum recursion depth exceeded (parser stack overflow)
        E1002,  ///< Invalid type specifier in declaration
        E1003,  ///< Invalid assignment target (cannot assign to rvalue)
        E1004,  ///< Unexpected token in current parsing context
        E1005,  ///< Invalid binary operator for operand types
        E1006,  ///< Expression expected but not found
        E1007,  ///< Statement expected but not found
        E1008,  ///< Identifier expected (e.g., in declaration)
        E1009,  ///< Type annotation expected (e.g., in parameter list)
        E1010,  ///< Mismatched parenthesis (missing `(` or `)`)
        E1011,  ///< Mismatched brace (missing `{` or `}`)
        E1012,  ///< Mismatched bracket (missing `[` or `]`)
        E1013,  ///< Missing semicolon at end of statement (warning)
        E1014,  ///< Invalid function signature (malformed declaration)
        E1015,  ///< Invalid parameter list (malformed parameters)

        // -------------------------------------------------------------------------
        // Semantic Analysis Errors (E2001-E2999)
        // -------------------------------------------------------------------------
        E2001,  ///< Mismatched number of initializers in array/struct
        E2002,  ///< Type mismatch in assignment (incompatible types)
        E2003,  ///< Missing return statement in some code paths
        E2004,  ///< Condition expression must have boolean type
        E2005,  ///< Return statement outside function body
        E2006,  ///< Cannot return value from void function
        E2007,  ///< Return type mismatch (declared vs actual)
        E2008,  ///< Missing return value in non-void function
        E2009,  ///< Break statement outside loop body
        E2010,  ///< Continue statement outside loop body
        E2011,  ///< Bitwise operator requires integer operands
        E2012,  ///< Logical operator requires boolean operands
        E2013,  ///< Arithmetic operator requires numeric operands
        E2014,  ///< Incompatible types in comparison operation
        E2015,  ///< Type mismatch in binary operation
        E2016,  ///< Arithmetic operation not supported for types
        E2017,  ///< Logical operation requires boolean type
        E2018,  ///< Negation requires numeric type
        E2019,  ///< Logical NOT requires boolean type
        E2020,  ///< Empty array literal (must have at least one element)
        E2021,  ///< Mixed types in array literal (all must match)
        E2022,  ///< Function cannot be used as variable
        E2023,  ///< Variable not defined in current scope
        E2024,  ///< Cannot assign to immutable variable (const/readonly)
        E2025,  ///< Variable not defined in assignment target
        E2026,  ///< Callable must be a function type
        E2027,  ///< Function not defined (undefined reference)
        E2028,  ///< Wrong number of arguments in function call
        E2029,  ///< Argument type mismatch in function call
        E2030,  ///< Array index must be integer type
        E2031,  ///< Cannot index non-array type
        E2032,  ///< Duplicate declaration of same identifier

        // -------------------------------------------------------------------------
        // IR Generation Errors (E3001-E3999)
        // -------------------------------------------------------------------------
        E3001,  ///< Break outside loop in IR generation
        E3002,  ///< Continue outside loop in IR generation
        E3003,  ///< Invalid IR instruction generated
        E3004,  ///< Variable not defined in IR context
        E3005,  ///< Invalid basic block structure
        E3006,  ///< Invalid block terminator in CFG
        E3007,  ///< SSA transformation error (conversion failed)
        E3008,  ///< CFG construction error (control flow invalid)

        // -------------------------------------------------------------------------
        // Code Generation Errors (E4001-E4999)
        // -------------------------------------------------------------------------
        E4001,  ///< Invalid assembly instruction for target
        E4002,  ///< Register allocation failed (no available registers)
        E4003,  ///< Stack frame overflow (exceeds maximum size)
        E4004,  ///< Unsupported target platform for compilation
        E4005,  ///< ABI violation (calling convention mismatch)

        // -------------------------------------------------------------------------
        // System/I/O Errors (E5001-E5999)
        // -------------------------------------------------------------------------
        E5001,  ///< File not found (source or output file missing)
        E5002,  ///< Permission denied (insufficient access rights)
        E5003,  ///< Invalid file extension (unrecognized format)
        E5004,  ///< Write error (failed to write to file)
        E5005   ///< Read error (failed to read from file)
    };

    // ---------------------------------------------------------------------------
    /// @struct ErrorInfo
    /// @brief Contains complete information about a specific error
    ///
    /// This structure provides complete error metadata for diagnostics
    /// and IDE integration.
    // ---------------------------------------------------------------------------
    struct ErrorInfo {
        const char *code;                       ///< Error code string (e.g., "E0001")
        uint16_t numeric_code;                  ///< Numeric part of error code (e.g., 1)
        Severity severity;                      ///< Severity level of the error
        CompilerPhase phase;                    ///< Compiler phase where error occurred
        const char *message;                    ///< Brief user-facing error message
        const char *explanation;                ///< Detailed explanation of the error
        std::vector<const char *> suggestions;  ///< Actionable suggestions to fix the error
    };

    /// @brief Retrieves complete error information for the given error code
    /// @param error_code The error code to look up
    /// @return A const reference to the ErrorInfo structure
    /// @note The returned reference is valid for the lifetime of the program
    [[nodiscard]] const ErrorInfo &get_error_info(ErrorCode error_code);

    /// @brief Returns the error code string (e.g., "E0001")
    /// @param error_code The error code enum value
    /// @return The error code string
    std::string code(ErrorCode error_code);

    /// @brief Returns the numeric part of the error code
    /// @param error_code The error code enum value
    /// @return The numeric code (e.g., 1 for E0001)
    uint16_t numeric_code(ErrorCode error_code);

    /// @brief Returns the severity level for the given error code
    /// @param error_code The error code enum value
    /// @return The severity (Note, Warning, Error, or Fatal)
    Severity severity(ErrorCode error_code);

    /// @brief Returns the compiler phase where the error occurs
    /// @param error_code The error code enum value
    /// @return The compiler phase enum
    CompilerPhase phase(ErrorCode error_code);

    /// @brief Returns a brief error message for the given error code
    /// @param error_code The error code enum value
    /// @return The error message string
    std::string message(ErrorCode error_code);

    /// @brief Returns a detailed explanation for the given error code
    /// @param error_code The error code enum value
    /// @return A C-string with the detailed explanation
    const char *explanation(ErrorCode error_code);

    /// @brief Returns actionable suggestions to fix the given error
    /// @param error_code The error code enum value
    /// @return A vector of suggestion strings
    std::vector<const char *> suggestions(ErrorCode error_code);

    /// @brief Converts an error code to a formatted string
    /// @param error_code The error code enum value
    /// @return A string in the format "CODE: message"
    std::string to_string(ErrorCode error_code);

}  // namespace jsv

// -------------------------------------------------------------------------
// std::formatter  (C++23 <format>)
// -------------------------------------------------------------------------
namespace std {
    template <> struct formatter<jsv::Severity> : formatter<string> {
        template <typename FormatContext> auto format(jsv::Severity severity, FormatContext &ctx) const {
            return formatter<string>::format(to_string(severity), ctx);
        }
    };

    template <> struct formatter<jsv::CompilerPhase> : formatter<string> {
        template <typename FormatContext> auto format(jsv::CompilerPhase phase, FormatContext &ctx) const {
            return formatter<string>::format(to_string(phase), ctx);
        }
    };

    template <> struct formatter<jsv::ErrorCode> : formatter<string> {
        template <typename FormatContext> auto format(jsv::ErrorCode error_code, FormatContext &ctx) const {
            return formatter<string>::format(to_string(error_code), ctx);
        }
    };
}  // namespace std

// -------------------------------------------------------------------------
// fmt::formatter  (fmtlib)
// -------------------------------------------------------------------------
template <> struct fmt::formatter<jsv::Severity> : fmt::formatter<std::string> {
    template <typename FormatContext> auto format(jsv::Severity severity, FormatContext &ctx) const {
        return fmt::formatter<std::string>::format(to_string(severity), ctx);
    }
};

template <> struct fmt::formatter<jsv::CompilerPhase> : fmt::formatter<std::string> {
    template <typename FormatContext> auto format(jsv::CompilerPhase phase, FormatContext &ctx) const {
        return fmt::formatter<std::string>::format(to_string(phase), ctx);
    }
};

template <> struct fmt::formatter<jsv::ErrorCode> : fmt::formatter<std::string> {
    template <typename FormatContext> auto format(jsv::ErrorCode error_code, FormatContext &ctx) const {
        return fmt::formatter<std::string>::format(to_string(error_code), ctx);
    }
};
