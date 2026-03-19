//
// Created by gbian on 15/05/2024.
//
// clang-format off
// NOLINTBEGIN(*-include-cleaner, hicpp-signed-bitwise, *-diagnostic-double-promotion, *-pro-bounds-constant-array-index, *-identifier-length)
// clang-format on
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
     * @throws FileReadError If the file cannot be opened.
     *
     * @pre  filePath must point to a valid, accessible file.
     * @post The returned ifstream is open, positioned at byte 0, with
     *       exception mask set to failbit | badbit.
     */
    [[nodiscard]] inline auto openFile(const fs::path &filePath) -> std::ifstream {
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
     * @throws FileReadError If the file does not exist, is not a regular file,
     *         cannot be opened, or an I/O error occurs during reading.
     *
     * @note Thread-safe: concurrent calls are serialized via a function-local mutex.
     * @note In INDEPT mode, an AutoTimer measures and logs the read performance.
     * @note If the file is truncated between stat and read, only the bytes actually
     *       read are returned.
     *
     * @see openFile
     */
    [[nodiscard]] inline auto readFromFile(const std::string_view filename) -> std::string {
        static std::mutex fileReadMutex;
        const std::scoped_lock lock(fileReadMutex);  // Ensure thread safety
        const fs::path filePath(filename);
        const auto status = fs::status(filePath);
        if(!fs::exists(status)) { throw FILEREADEREERRORF("File not found: {}", filePath.string()); }
        if(!fs::is_regular_file(status)) { throw FILEREADEREERRORF("Path is not a regular file: {}", filePath.string()); }
        const auto rawFileSize = fs::file_size(filePath);
        const auto fileSize = C_ST(rawFileSize);

#ifdef INDEPT
        // Optional timer for performance measurement.
        const AutoTimer timer(FORMAT("Reading file {}", filename));
#endif

        // Ensure the file is properly opened and manage it with RAII.
        auto fileStream = openFile(filePath);

        try {
            // PERF: string pre-allocated to exact file size — no reallocation during read.
            std::string content(fileSize, '\0');

            // CORRECTNESS: explicit cast to std::streamsize avoids implicit narrowing
            // from std::size_t (unsigned 64-bit) on platforms where std::streamsize
            // is signed 64-bit. Values up to 2^63-1 are safe; larger files would
            // require chunked reading (out of scope for this utility).
            fileStream.read(content.data(), static_cast<std::streamsize>(fileSize));

            // Defensive resize: handles file truncation between stat and read.
            const auto bytesRead = C_ST(fileStream.gcount());
            if(bytesRead < fileSize) { content.resize(bytesRead); }

            return content;
        } catch(const std::ios_base::failure &e) {
            throw FILEREADEREERRORF("Unable to read file: {}. Reason: {}", filePath.string(), e.what());
        }
    }
}  // namespace vnd

// ─── Support structures ───────────────────────────────────────────────────

struct SizeSystem {
    std::string_view name;
    long double base;
    std::array<std::string_view, 6> prefixes;
};

struct FormattedSize {
    long double value;
    std::string_view suffix;
};

struct FileSizeInfo {
    uintmax_t bytes;

    [[nodiscard]] constexpr FormattedSize format(const SizeSystem &sys) const noexcept {
        auto v = C_LD(bytes);
        std::size_t i = 0;

        while(i < 5 && v >= sys.base) {
            v /= sys.base;
            ++i;
        }

        return {.value = v, .suffix = sys.prefixes[i]};
    }
};

struct FormattedSizePair {
    FormattedSize si;
    FormattedSize iec;
};

struct FileSizeReport {
    FileSizeInfo info;
    const SizeSystem &si_sys;
    const SizeSystem &iec_sys;

    [[nodiscard]] constexpr FormattedSizePair make_pair() const noexcept {
        return {
            .si = info.format(si_sys),
            .iec = info.format(iec_sys),
        };
    }
};

// ─── std::formatter ──────────────────────────────────────────────────────────

template <> struct std::formatter<FormattedSize> : std::formatter<std::string_view> {
    template <typename FormatContext> auto format(const FormattedSize &fs, FormatContext &ctx) const {
        return std::format_to(ctx.out(), "{:.2Lf} {}", fs.value, fs.suffix);
    }
};

template <> struct std::formatter<FormattedSizePair> : std::formatter<std::string_view> {
    template <typename FormatContext> auto format(const FormattedSizePair &p, FormatContext &ctx) const {
        return std::format_to(ctx.out(), "{:<20} {:<20}", std::format("{}", p.si), std::format("{}", p.iec));
    }
};

template <> struct std::formatter<FileSizeReport> : std::formatter<std::string_view> {
    template <typename FormatContext> auto format(const FileSizeReport &r, FormatContext &ctx) const {
        auto out = ctx.out();
        const auto pair = r.make_pair();
        out = std::format_to(out, "Bytes : {}\n", r.info.bytes);
        out = std::format_to(out, "{:-<41}\n", "");
        out = std::format_to(out, "{:<20} {:<20}\n", "SI", "IEC");
        out = std::format_to(out, "{:-<41}\n", "");
        out = std::format_to(out, "{}", pair);
        return out;
    }
};

// ─── fmt::formatter ──────────────────────────────────────────────────────────

template <> struct fmt::formatter<FormattedSize> : fmt::formatter<std::string_view> {
    template <typename FormatContext> auto format(const FormattedSize &fs, FormatContext &ctx) const {
        return fmt::format_to(ctx.out(), "{:.2Lf} {}", fs.value, fs.suffix);
    }
};

template <> struct fmt::formatter<FormattedSizePair> : fmt::formatter<std::string_view> {
    template <typename FormatContext> auto format(const FormattedSizePair &p, FormatContext &ctx) const {
        return fmt::format_to(ctx.out(), "{:<20} {:<20}", fmt::format("{}", p.si), fmt::format("{}", p.iec));
    }
};

template <> struct fmt::formatter<FileSizeReport> : fmt::formatter<std::string_view> {
    template <typename FormatContext> auto format(const FileSizeReport &r, FormatContext &ctx) const {
        auto out = ctx.out();
        const auto pair = r.make_pair();
        out = fmt::format_to(out, "Bytes : {}\n", r.info.bytes);
        out = fmt::format_to(out, "{:-<41}\n", "");
        out = fmt::format_to(out, "{:<20} {:<20}\n", "SI", "IEC");
        out = fmt::format_to(out, "{:-<41}\n", "");
        out = fmt::format_to(out, "{}", pair);
        return out;
    }
};

// ─── Unit systems ────────────────────────────────────────────────────────

constexpr SizeSystem kSI = {.name = "SI", .base = 1000.0L, .prefixes = {"B", "KB", "MB", "GB", "TB", "PB"}};

constexpr SizeSystem kIEC = {.name = "IEC", .base = 1024.0L, .prefixes = {"B", "KiB", "MiB", "GiB", "TiB", "PiB"}};

// clang-format off
// NOLINTEND(*-include-cleaner, hicpp-signed-bitwise, *-diagnostic-double-promotion, *-pro-bounds-constant-array-index, *-identifier-length)
// clang-format on