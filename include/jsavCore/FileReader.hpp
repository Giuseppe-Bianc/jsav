//
// Created by gbian on 15/05/2024.
//
// NOLINTBEGIN(*-include-cleaner,  hicpp-signed-bitwise)
#pragma once

#include "FileReaderError.hpp"
#include "headersCore.hpp"
#include "timer/Timer.hpp"

namespace vnd {

    /**
     * @brief Opens a file for binary reading with proper error handling.
     *
     * @param[in] filePath The path to the file to open.
     * @return An std::ifstream opened for binary reading.
     *
     * @details This function opens the specified file in binary mode and sets up
     *          exception handling for stream errors. If the file cannot be opened,
     *          a FileReadError exception is thrown with a descriptive message.
     *
     * @throws FileReadError If the file cannot be opened (e.g., file doesn't exist,
     *         permission denied, path is a directory).
     *
     * @pre filePath must point to a valid, accessible file.
     * @post The returned ifstream is opened and ready for reading.
     * @post The stream's exception mask is set to failbit | badbit.
     *
     * @note The file is opened in binary mode (std::ios::binary).
     * @note The function uses RAII; the file will be automatically closed when
     *       the returned ifstream goes out of scope.
     *
     * @par Example:
     * @code{.cpp}
     * try {
     *     auto fileStream = openFile("data.bin");
     *     // Read from fileStream...
     * } catch(const FileReadError& e) {
     *     LERROR("Failed to open file: {}", e.what());
     * }
     * @endcode
     */
    inline auto openFile(const fs::path &filePath) -> std::ifstream {
        std::ifstream fileStream(filePath, std::ios::in | std::ios::binary);
        if(!fileStream.is_open()) { throw FILEREADEREERRORF("Unable to open file: {}", filePath.string()); }

        fileStream.exceptions(std::ios::failbit | std::ios::badbit);
        return fileStream;
    }

    /**
     * @brief Reads the entire contents of a file into a string.
     *
     * @param[in] filename The path to the file to read (as a string_view).
     * @return A std::string containing the entire file contents.
     *
     * @details This function reads the entire contents of the specified file into
     *          a std::string. It performs the following operations:
     *          1. Validates that the path exists and is a regular file
     *          2. Acquires a mutex lock for thread safety
     *          3. Optionally measures read performance with a timer (INDEPT mode)
     *          4. Pre-allocates the string buffer based on file size
     *          5. Reads the file contents in binary mode
     *          6. Handles various error conditions with descriptive exceptions
     *
     * @throws FileReadError If:
     *         - The file does not exist
     *         - The path is not a regular file (e.g., directory, symlink)
     *         - The file cannot be opened
     *         - The file size cannot be determined
     *         - An I/O error occurs during reading
     *
     * @pre The file must exist and be accessible.
     * @pre The path must point to a regular file (not a directory or special file).
     * @post The returned string contains the complete file contents.
     *
     * @note This function is thread-safe; concurrent calls are serialized via a mutex.
     * @note In INDEPT mode, an AutoTimer measures and logs the read performance.
     * @note The file is read in binary mode; no character translation is performed.
     * @note If the file is truncated during reading, only the successfully read
     *       bytes are returned (no exception is thrown).
     *
     * @par Example:
     * @code{.cpp}
     * try {
     *     std::string content = readFromFile("config.txt");
     *     LINFO("File content: {}", content);
     * } catch(const FileReadError& e) {
     *     LERROR("Read failed: {}", e.what());
     * }
     * @endcode
     *
     * @see openFile
     * @see AutoTimer
     */
    inline auto readFromFile(const std::string_view filename) -> std::string {
        static std::mutex fileReadMutex;
        const std::scoped_lock lock(fileReadMutex);  // Ensure thread safety
        const auto filePath = fs::path(filename);
        if(!fs::exists(filePath)) { throw FILEREADEREERRORF("File not found: {}", filePath); }
        if(!fs::is_regular_file(filePath)) { throw FILEREADEREERRORF("Path is not a regular file: {}", filePath); }

#ifdef INDEPT
        // Optional timer for performance measurement.
        const AutoTimer timer(FORMAT("Reading file {}", filename));
#endif

        // Ensure the file is properly opened and manage it with RAII.
        auto fileStream = openFile(filePath);

        try {
            // Get file size for pre-allocation
            fileStream.seekg(0, std::ios::end);
            const auto fileSize = fileStream.tellg();
            fileStream.seekg(0, std::ios::beg);

            if(fileSize < 0) { throw FILEREADEREERRORF("Unable to determine file size: {}", filePath); }

            std::string content(static_cast<std::size_t>(fileSize), '\0');
            fileStream.read(content.data(), fileSize);

            // Handle case where fewer bytes were read (e.g., file truncated during read)
            const auto bytesRead = fileStream.gcount();
            content.resize(static_cast<std::size_t>(bytesRead));

            return content;
        } catch(const std::ios_base::failure &e) {
            throw FILEREADEREERRORF("Unable to read file: {}. Reason: {}", filePath, e.what());
        } catch(const std::exception &e) {
            throw FILEREADEREERRORF("An error occurred while reading the file: {}. Reason: {}", filePath, e.what());
        }
    }
}  // namespace vnd

// NOLINTEND(*-include-cleaner,  hicpp-signed-bitwise)
