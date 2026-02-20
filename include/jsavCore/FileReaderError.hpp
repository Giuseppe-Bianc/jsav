//
// Created by gbian on 16/01/2024.
//
// NOLINTBEGIN(*-easily-swappable-parameters, *-include-cleaner)
#pragma once

#include "headersCore.hpp"

/**
 * @class FileReadError
 * @brief Exception class for file reading errors.
 *
 * @details This class inherits from std::runtime_error and is used to signal
 *          errors that occur during file reading operations. It provides a
 *          type-safe way to distinguish file reading errors from other runtime errors.
 *
 * @throws std::runtime_error Base class for all file reading exceptions.
 *
 * @par Example:
 * @code{.cpp}
 * try {
 *     auto content = readFromFile("nonexistent.txt");
 * } catch(const FileReadError& e) {
 *     LERROR("File read error: {}", e.what());
 * }
 * @endcode
 */
class FileReadError final : public std::runtime_error {
public:
    /**
     * @brief Constructs a FileReadError with a custom error message.
     *
     * @param[in] message The error message describing the file reading failure.
     *
     * @par Example:
     * @code{.cpp}
     * throw FileReadError("Failed to open file: config.txt");
     * @endcode
     */
    explicit FileReadError(const std::string &message) : std::runtime_error(message) {}
};

/**
 * @def FILEREADEREERRORF(...)
 * @brief Macro for creating FileReadError exceptions with formatted messages.
 *
 * @param ... The format string and arguments for the error message.
 * @return A FileReadError exception with the formatted message.
 *
 * @details This macro wraps the FORMAT() macro to create FileReadError exceptions
 *          with formatted error messages. It provides a convenient way to create
 *          descriptive error messages for file reading failures.
 *
 * @pre The FORMAT() macro must be available (requires format.hpp).
 *
 * @par Example:
 * @code{.cpp}
 * if(!file.is_open()) {
 *     throw FILEREADEREERRORF("Unable to open file: {}", filename);
 * }
 * @endcode
 *
 * @see FileReadError
 * @see FORMAT
 */
#define FILEREADEREERRORF(...) FileReadError(FORMAT(__VA_ARGS__))
// NOLINTEND(*-easily-swappable-parameters, *-include-cleaner)
