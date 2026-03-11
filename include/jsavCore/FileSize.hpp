/*
 * Created by gbian on 09/03/2026.
 * Copyright (c) 2026 All rights reserved.
 */

#pragma once
// NOLINTBEGIN(*-diagnostic-double-promotion, *-pro-bounds-constant-array-index, *-identifier-length)

#include "cast/casts.hpp"
#include "format.hpp"

/**
 * @namespace fs
 * @brief Namespace alias for std::filesystem.
 */
namespace fs = std::filesystem;  // NOLINT(*-unused-alias-decls)

// ─── Strutture di supporto ───────────────────────────────────────────────────

struct SizePrefix {
    const char *suffix;
    long double threshold;
};

struct SizeSystem {
    const char *name;
    long double base;
    std::array<SizePrefix, 6> prefixes;
};

struct FormattedSize {
    long double value;
    const char *suffix;
};

struct FileSizeInfo {
    uintmax_t bytes;

    [[nodiscard]] constexpr FormattedSize format(const SizeSystem &sys) const noexcept {
        auto v = C_LD(bytes);
        int i = 0;

        while(i < 5 && v >= sys.base) {
            v /= sys.base;
            ++i;
        }

        return {.value = v, .suffix = sys.prefixes[i].suffix};
    }
};

struct FormattedSizePair {
    FormattedSize si;
    FormattedSize iec;
};

struct FileSizeReport {
    fs::path path;  // ← fs::path non è constexpr
    FileSizeInfo info;
    const SizeSystem &si_sys;
    const SizeSystem &iec_sys;

    [[nodiscard]] constexpr FormattedSizePair make_pair() const noexcept {
        return {
            .si = info.format(si_sys),
            .iec = info.format(iec_sys),
        };
    }

    // factory: runtime — fs::file_size non è constexpr
    [[nodiscard]] static FileSizeReport from_path(const fs::path &p, const SizeSystem &si, const SizeSystem &iec) {
        std::error_code ec;
        const uintmax_t bytes = fs::file_size(p, ec);
        if(ec) { throw fs::filesystem_error("cannot read file size", p, ec); }
        return {
            .path = p,
            .info = {.bytes = bytes},
            .si_sys = si,
            .iec_sys = iec,
        };
    }
};

// ─── std::formatter ──────────────────────────────────────────────────────────

template <> struct std::formatter<FormattedSize> : std::formatter<std::string> {
    template <typename FormatContext> auto format(const FormattedSize &fs, FormatContext &ctx) const {
        return std::format_to(ctx.out(), "{:.2Lf} {}", fs.value, fs.suffix);
    }
};

template <> struct std::formatter<FormattedSizePair> : std::formatter<std::string> {
    template <typename FormatContext> auto format(const FormattedSizePair &p, FormatContext &ctx) const {
        return std::format_to(ctx.out(), "{:<20} {:<20}", std::format("{}", p.si), std::format("{}", p.iec));
    }
};

template <> struct std::formatter<FileSizeReport> : std::formatter<std::string> {
    template <typename FormatContext> auto format(const FileSizeReport &r, FormatContext &ctx) const {
        auto out = ctx.out();
        auto pair = r.make_pair();  // ← constexpr chiamata a runtime: ok
        out = std::format_to(out, "File  : {}\n", r.path.string());
        out = std::format_to(out, "Bytes : {}\n", r.info.bytes);
        out = std::format_to(out, "{:-<41}\n", "");
        out = std::format_to(out, "{:<20} {:<20}\n", "SI", "IEC");
        out = std::format_to(out, "{:-<41}\n", "");
        out = std::format_to(out, "{}", pair);
        return out;
    }
};

// ─── fmt::formatter ──────────────────────────────────────────────────────────

template <> struct fmt::formatter<FormattedSize> : fmt::formatter<std::string> {
    template <typename FormatContext> auto format(const FormattedSize &fs, FormatContext &ctx) const {
        return fmt::format_to(ctx.out(), "{:.2Lf} {}", fs.value, fs.suffix);
    }
};

template <> struct fmt::formatter<FormattedSizePair> : fmt::formatter<std::string> {
    template <typename FormatContext> auto format(const FormattedSizePair &p, FormatContext &ctx) const {
        return fmt::format_to(ctx.out(), "{:<20} {:<20}", fmt::format("{}", p.si), fmt::format("{}", p.iec));
    }
};

template <> struct fmt::formatter<FileSizeReport> : fmt::formatter<std::string> {
    template <typename FormatContext> auto format(const FileSizeReport &r, FormatContext &ctx) const {
        auto out = ctx.out();
        auto pair = r.make_pair();
        out = fmt::format_to(out, "File  : {}\n", r.path.string());
        out = fmt::format_to(out, "Bytes : {}\n", r.info.bytes);
        out = fmt::format_to(out, "{:-<41}\n", "");
        out = fmt::format_to(out, "{:<20} {:<20}\n", "SI", "IEC");
        out = fmt::format_to(out, "{:-<41}\n", "");
        out = fmt::format_to(out, "{}", pair);
        return out;
    }
};

// ─── Sistemi di unità ────────────────────────────────────────────────────────

static inline constexpr SizeSystem kSI = {.name = "SI",
                                          .base = 1000.0L,
                                          .prefixes = {{
                                              {"B", 1.0L},
                                              {"KB", 1e3L},
                                              {"MB", 1e6L},
                                              {"GB", 1e9L},
                                              {"TB", 1e12L},
                                              {"PB", 1e15L},
                                          }}};

static inline constexpr SizeSystem kIEC = {.name = "IEC",
                                           .base = 1024.0L,
                                           .prefixes = {{
                                               {"B", 1.0L},
                                               {"KiB", 1024.0L},
                                               {"MiB", 1048576.0L},
                                               {"GiB", 1073741824.0L},
                                               {"TiB", 1125899906842624.0L},
                                               {"PiB", 1152921504606846976.0L},
                                           }}};

// NOLINTEND(*-diagnostic-double-promotion, *-pro-bounds-constant-array-index, *-identifier-length)