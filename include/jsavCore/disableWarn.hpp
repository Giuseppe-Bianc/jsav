#pragma once
// NOLINTBEGIN(*-macro-usage)

/**
 * @brief Internal helper macro that stringifies a pragma argument for use with _Pragma().
 *
 * @details This macro converts its argument into a string literal, enabling the
 *          use of _Pragma() with dynamically constructed pragma directives.
 *          It is an implementation detail used by other warning control macros.
 *
 * @param x The pragma to convert into a string literal.
 *
 * @return The string representation of the provided pragma.
 *
 * @note This is an internal helper macro; it should not be used directly
 *       in user code.
 */
#define JSAV_DO_PRAGMA(x) _Pragma(#x)

#ifdef _MSC_VER
/**
 * @brief Temporarily disables specified MSVC compiler warnings.
 *
 * @details Saves the current warning state and disables the listed warnings
 *          until the corresponding call to DISABLE_WARNINGS_POP().
 *          Use to suppress known warnings in specific code regions.
 *
 * @param[in] ... List of MSVC warning codes to disable (e.g., 4267, 4996).
 *
 * @note Available only when compiling with MSVC (_MSC_VER defined).
 * @see DISABLE_WARNINGS_POP() to restore the previous state.
 *
 * @par Example:
 * @code{.cpp}
 * DISABLE_WARNINGS_PUSH(4267 4996)
 * // Code that generates warnings 4267 and 4996
 * DISABLE_WARNINGS_POP()
 * @endcode
 */
#define DISABLE_WARNINGS_PUSH(...) __pragma(warning(push)) __pragma(warning(disable : __VA_ARGS__))

/**
 * @brief Restores the previously saved MSVC warning state.
 *
 * @details Must be called after DISABLE_WARNINGS_PUSH() to end the
 *          code region with disabled warnings and restore the
 *          previous configuration.
 *
 * @note Available only when compiling with MSVC (_MSC_VER defined).
 * @see DISABLE_WARNINGS_PUSH() to begin a region with disabled warnings.
 */
#define DISABLE_WARNINGS_POP() __pragma(warning(pop))

/**
 * @brief Empty macro for cross-compiler compatibility on MSVC.
 *
 * @details On MSVC this macro has no effect since Clang-specific
 *          warnings are not applicable.
 *
 * @param[in] warning Clang warning identifier (ignored on MSVC).
 *
 * @note Available only when compiling with MSVC; has no effect.
 */
#define DISABLE_CLANG_WARNINGS_PUSH(warning)

/**
 * @brief Empty macro for cross-compiler compatibility on MSVC.
 *
 * @details On MSVC this macro has no effect since Clang-specific
 *          warnings are not applicable.
 *
 * @note Available only when compiling with MSVC; has no effect.
 */
#define DISABLE_CLANG_WARNINGS_POP()

/**
 * @brief Empty macro for cross-compiler compatibility on MSVC.
 *
 * @details On MSVC this macro has no effect since GCC-specific
 *          warnings are not applicable.
 *
 * @param[in] warning GCC warning identifier (ignored on MSVC).
 *
 * @note Available only when compiling with MSVC; has no effect.
 */
#define DISABLE_GCC_WARNINGS_PUSH(warning)

/**
 * @brief Empty macro for cross-compiler compatibility on MSVC.
 *
 * @details On MSVC this macro has no effect since GCC-specific
 *          warnings are not applicable.
 *
 * @note Available only when compiling with MSVC; has no effect.
 */
#define DISABLE_GCC_WARNINGS_POP()

#elif defined(__clang__)
/**
 * @brief Empty macro for cross-compiler compatibility on Clang.
 *
 * @details On Clang this macro has no effect since MSVC-specific
 *          warnings are not applicable.
 *
 * @param[in] ... MSVC warning codes (ignored on Clang).
 *
 * @note Available only when compiling with Clang; has no effect.
 */
#define DISABLE_WARNINGS_PUSH(...)

/**
 * @brief Empty macro for cross-compiler compatibility on Clang.
 *
 * @details On Clang this macro has no effect since MSVC-specific
 *          warnings are not applicable.
 *
 * @note Available only when compiling with Clang; has no effect.
 */
#define DISABLE_WARNINGS_POP()

/**
 * @brief Temporarily disables a specific Clang compiler warning.
 *
 * @details Saves the current Clang diagnostic state and disables the
 *          specified warning until the corresponding call to DISABLE_CLANG_WARNINGS_POP().
 *          The warning format must include quotes (e.g., "-Wunused-variable").
 *
 * @param[in] warning String literal identifying the Clang warning to disable.
 *
 * @note Available only when compiling with Clang (__clang__ defined).
 * @see DISABLE_CLANG_WARNINGS_POP() to restore the previous state.
 *
 * @par Example:
 * @code{.cpp}
 * DISABLE_CLANG_WARNINGS_PUSH("-Wunused-variable")
 * // Code that generates unused variable warning
 * DISABLE_CLANG_WARNINGS_POP()
 * @endcode
 */
#define DISABLE_CLANG_WARNINGS_PUSH(warning) _Pragma("clang diagnostic push") JSAV_DO_PRAGMA(clang diagnostic ignored warning)

/**
 * @brief Restores the previously saved Clang diagnostic state.
 *
 * @details Must be called after DISABLE_CLANG_WARNINGS_PUSH() to end the
 *          code region with disabled warnings and restore the
 *          previous configuration.
 *
 * @note Available only when compiling with Clang (__clang__ defined).
 * @see DISABLE_CLANG_WARNINGS_PUSH() to begin a region with disabled warnings.
 */
#define DISABLE_CLANG_WARNINGS_POP() _Pragma("clang diagnostic pop")

/**
 * @brief Empty macro for cross-compiler compatibility on Clang.
 *
 * @details On Clang this macro has no effect since GCC-specific
 *          warnings are not applicable.
 *
 * @param[in] warning GCC warning identifier (ignored on Clang).
 *
 * @note Available only when compiling with Clang; has no effect.
 */
#define DISABLE_GCC_WARNINGS_PUSH(warning)

/**
 * @brief Empty macro for cross-compiler compatibility on Clang.
 *
 * @details On Clang this macro has no effect since GCC-specific
 *          warnings are not applicable.
 *
 * @note Available only when compiling with Clang; has no effect.
 */
#define DISABLE_GCC_WARNINGS_POP()

#elif defined(__GNUC__)
/**
 * @brief Empty macro for cross-compiler compatibility on GCC.
 *
 * @details On GCC this macro has no effect since MSVC-specific
 *          warnings are not applicable.
 *
 * @param[in] ... MSVC warning codes (ignored on GCC).
 *
 * @note Available only when compiling with GCC (__GNUC__ defined); has no effect.
 */
#define DISABLE_WARNINGS_PUSH(...)

/**
 * @brief Empty macro for cross-compiler compatibility on GCC.
 *
 * @details On GCC this macro has no effect since MSVC-specific
 *          warnings are not applicable.
 *
 * @note Available only when compiling with GCC (__GNUC__ defined); has no effect.
 */
#define DISABLE_WARNINGS_POP()

/**
 * @brief Empty macro for cross-compiler compatibility on GCC.
 *
 * @details On GCC this macro has no effect since Clang-specific
 *          warnings are not applicable.
 *
 * @param[in] warning Clang warning identifier (ignored on GCC).
 *
 * @note Available only when compiling with GCC; has no effect.
 */
#define DISABLE_CLANG_WARNINGS_PUSH(warning)

/**
 * @brief Empty macro for cross-compiler compatibility on GCC.
 *
 * @details On GCC this macro has no effect since Clang-specific
 *          warnings are not applicable.
 *
 * @note Available only when compiling with GCC; has no effect.
 */
#define DISABLE_CLANG_WARNINGS_POP()

/**
 * @brief Temporarily disables a specific GCC compiler warning.
 *
 * @details Saves the current GCC diagnostic state and disables the
 *          specified warning until the corresponding call to DISABLE_GCC_WARNINGS_POP().
 *          The warning format must conform to GCC syntax (e.g., "-Wunused-variable").
 *
 * @param[in] warning String literal identifying the GCC warning to disable.
 *
 * @note Available only when compiling with GCC (__GNUC__ defined).
 * @see DISABLE_GCC_WARNINGS_POP() to restore the previous state.
 *
 * @par Example:
 * @code{.cpp}
 * DISABLE_GCC_WARNINGS_PUSH("-Wunused-variable")
 * // Code that generates unused variable warning
 * DISABLE_GCC_WARNINGS_POP()
 * @endcode
 */
#define DISABLE_GCC_WARNINGS_PUSH(warning) _Pragma("GCC diagnostic push") JSAV_DO_PRAGMA(GCC diagnostic ignored warning)

/**
 * @brief Restores the previously saved GCC diagnostic state.
 *
 * @details Must be called after DISABLE_GCC_WARNINGS_PUSH() to end the
 *          code region with disabled warnings and restore the
 *          previous configuration.
 *
 * @note Available only when compiling with GCC (__GNUC__ defined).
 * @see DISABLE_GCC_WARNINGS_PUSH() to begin a region with disabled warnings.
 */
#define DISABLE_GCC_WARNINGS_POP() _Pragma("GCC diagnostic pop")

#else
/**
 * @brief Empty macro for unsupported compilers.
 *
 * @details This fallback definition is used when the compiler
 *          is neither MSVC, Clang, nor GCC. It has no effect.
 *
 * @param[in] ... Ignored parameters.
 *
 * @note Used as a fallback for unsupported compilers.
 */
#define DISABLE_WARNINGS_PUSH(...)

/**
 * @brief Empty macro for unsupported compilers.
 *
 * @details This fallback definition is used when the compiler
 *          is neither MSVC, Clang, nor GCC. It has no effect.
 *
 * @note Used as a fallback for unsupported compilers.
 */
#define DISABLE_WARNINGS_POP()

/**
 * @brief Empty macro for unsupported compilers.
 *
 * @details This fallback definition is used when the compiler
 *          is neither MSVC, Clang, nor GCC. It has no effect.
 *
 * @param[in] warning Ignored parameter.
 *
 * @note Used as a fallback for unsupported compilers.
 */
#define DISABLE_CLANG_WARNINGS_PUSH(warning)

/**
 * @brief Empty macro for unsupported compilers.
 *
 * @details This fallback definition is used when the compiler
 *          is neither MSVC, Clang, nor GCC. It has no effect.
 *
 * @note Used as a fallback for unsupported compilers.
 */
#define DISABLE_CLANG_WARNINGS_POP()

/**
 * @brief Empty macro for unsupported compilers.
 *
 * @details This fallback definition is used when the compiler
 *          is neither MSVC, Clang, nor GCC. It has no effect.
 *
 * @param[in] warning Ignored parameter.
 *
 * @note Used as a fallback for unsupported compilers.
 */
#define DISABLE_GCC_WARNINGS_PUSH(warning)

/**
 * @brief Empty macro for unsupported compilers.
 *
 * @details This fallback definition is used when the compiler
 *          is neither MSVC, Clang, nor GCC. It has no effect.
 *
 * @note Used as a fallback for unsupported compilers.
 */
#define DISABLE_GCC_WARNINGS_POP()
#endif

// NOLINTEND(*-macro-usage)
