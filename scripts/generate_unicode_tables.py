#!/usr/bin/env python3
"""
generate_unicode_tables.py — Offline generator for jsav lexer Unicode lookup tables.

Downloads UnicodeData.txt for Unicode 16.0.0, parses General Categories,
merges adjacent ranges for id_start / id_continue / whitespace classifiers,
and outputs a formatted constexpr C++ header.

SC-005 conformance: After generating tables, re-parses the generated C++ ranges and
verifies 100% round-trip coverage against UnicodeData.txt for categories L, M, N, Zs,
Zl, Zp.  Exits non-zero on any mismatch.

Usage:
    python scripts/generate_unicode_tables.py

Output:
    include/jsav/lexer/unicode/UnicodeData.hpp
"""

from bisect import bisect_right
from itertools import batched
import shutil
import subprocess
import sys
import time
import urllib.request
import gzip
import io
import zlib
from pathlib import Path
from datetime import date

# -----------------------------------------------------------------------------
# Configuration
# -----------------------------------------------------------------------------
UNICODE_VERSION = "16.0.0"
UNICODE_DATA_URL = (
    f"https://www.unicode.org/Public/{UNICODE_VERSION}/ucd/UnicodeData.txt"
)
OUTPUT_PATH = (
        Path(__file__).parent.parent
        / "include"
        / "jsav"
        / "lexer"
        / "unicode"
        / "UnicodeData.hpp"
)

UNITS = ("B", "KB", "MB", "GB", "TB")
_UNIT_MAX = len(UNITS) - 1

# General Category sets for each classifier
# id_start : Letter (Lu Ll Lt Lm Lo Nl) — categories starting with L, plus Nl
ID_START_CATEGORIES = frozenset({"Lu", "Ll", "Lt", "Lm", "Lo", "Nl"})
# id_continue : id_start + Mark (Mn Mc Me) + Number (Nd Nl No) + connector punct (Pc)
ID_CONTINUE_CATEGORIES = frozenset({
    "Lu", "Ll", "Lt", "Lm", "Lo",  # Letter
    "Mn", "Mc", "Me",  # Mark
    "Nd", "Nl", "No",  # Number
    "Pc",  # Connector punctuation (underscore etc.)
})
# whitespace : General Category Zs, Zl, Zp
WHITESPACE_CATEGORIES = frozenset({"Zs", "Zl", "Zp", "Cc"})

# "Letter" for is_letter() — all L categories
LETTER_CATEGORIES = frozenset({"Lu", "Ll", "Lt", "Lm", "Lo"})

def format_size(bytes: int) -> tuple[float, str]:
    size = float(bytes)
    unit = 0

    while size >= 1024.0 and unit < _UNIT_MAX:
        size /= 1024.0
        unit += 1

    return size, UNITS[unit]

# -----------------------------------------------------------------------------
# Timing Utilities
# -----------------------------------------------------------------------------

def format_duration(duration_ns: float) -> str:
    """
    Format a duration in nanoseconds to a human-readable string.

    Mimics the C++ ValueLabel::toString() format from
    include/jsavCore/timer/Times.hpp which produces composite time strings like:
    - "1s,234ms,567us,890ns" for seconds
    - "1ms,234us,567ns" for milliseconds
    - "1234us,567ns" for microseconds
    - "500ns" for nanoseconds

    Args:
        duration_ns: Duration in nanoseconds.

    Returns:
        Formatted string with composite time units matching C++ Timer output.

    Examples:
        >>> format_duration(500)
        '500ns'
        >>> format_duration(15567)
        '15us,567ns'
        >>> format_duration(2567890)
        '2ms,567us,890ns'
        >>> format_duration(1234567890)
        '1s,234ms,567us,890ns'
    """
    ns = int(duration_ns)  # cast once

    if ns < 1_000:
        # Pure nanoseconds
        return str(ns) + "ns"
    if ns < 1_000_000:
        # Microseconds + nanoseconds
        micros, nanos = divmod(ns, 1_000)
        if nanos:
            return str(micros) + "us," + str(nanos) + "ns"
        return str(micros) + "us"
    if ns < 1_000_000_000:
        # Milliseconds + microseconds + nanoseconds
        millis, rem = divmod(ns, 1_000_000)
        micros, nanos = divmod(rem, 1_000)
        s = str(millis) + "ms"
        if micros:
            s += "," + str(micros) + "us"
        if nanos:
            s += "," + str(nanos) + "ns"
        return s

    # Seconds + milliseconds + microseconds + nanoseconds
    secs, rem = divmod(ns, 1_000_000_000)
    millis, rem = divmod(rem, 1_000_000)
    micros, nanos = divmod(rem, 1_000)
    s = str(secs) + "s"
    if millis:
        s += "," + str(millis) + "ms"
    if micros:
        s += "," + str(micros) + "us"
    if nanos:
        s += "," + str(nanos) + "ns"
    return s


# -----------------------------------------------------------------------------
# Parse UnicodeData.txt
# -----------------------------------------------------------------------------

def download_unicode_data() -> str:
    """Download UnicodeData.txt and return its contents as a UTF-8 string.

    Optimizations vs. original:
    - gzip/zlib imports hoisted to module level (avoids repeated sys.modules lookup).
    - Content-Encoding is checked before reading the body so decompression is
      performed as a streaming pass over the wire bytes, eliminating the
      double-buffer peak that existed when decompress() was called on an already-
      fully-read bytes object.
    - A timeout guards against indefinite blocking.
    """
    print(f"Downloading UnicodeData.txt (Unicode {UNICODE_VERSION})...")

    request = urllib.request.Request(UNICODE_DATA_URL)
    # Signal that we can handle compressed responses to reduce transfer size.
    request.add_header("Accept-Encoding", "gzip, deflate")

    with urllib.request.urlopen(request, timeout=30) as response:
        content_encoding = response.headers.get("Content-Encoding", "").lower()

        if content_encoding == "gzip":
            # Wrap the raw socket stream in GzipFile so decompression happens
            # incrementally — we never hold both the compressed and decompressed
            # bytes in memory at the same time.
            with gzip.GzipFile(fileobj=response) as gz_stream:
                raw_data: bytes = gz_stream.read()

        elif content_encoding == "deflate":
            # zlib's decompressobj lets us feed chunks from the wire directly,
            # again avoiding a second full-size allocation.
            decompressor = zlib.decompressobj(wbits=-zlib.MAX_WBITS)
            chunks: list[bytes] = []
            while chunk := response.read(65536):  # 64 KiB chunks
                chunks.append(decompressor.decompress(chunk))
            chunks.append(decompressor.flush())
            # join() performs a single allocation sized exactly to the output.
            raw_data = b"".join(chunks)

        else:
            # No compression — read directly.
            raw_data = response.read()

    # Report the byte count before decoding so it reflects actual download size.
    raw_len = len(raw_data)

    # Decode to str; raw_data goes out of scope immediately after and is
    # eligible for GC — no explicit del needed since there are no other refs.
    data = raw_data.decode("utf-8")

    size, unit = format_size(raw_len)
    print(f"  Downloaded {size:.2f} {unit} ({raw_len:,} bytes).")
    return data


def parse_codepoints(data: str) -> dict[int, str]:
    """Return {codepoint: general_category} mapping.

    Handles <..., First> / <..., Last> range markers for CJK blocks.

    Optimisations vs. original:
      - io.StringIO iterator: avoids materialising a full list of lines.
      - split(";", 2):        only splits the three fields we actually use.
      - dict.fromkeys+update: expands codepoint ranges in C, not a Python loop.
    """
    codepoints: dict[int, str] = {}
    pending_range_start: int | None = None
    pending_range_cat: str | None = None

    # Opt 1: iterate lazily — no upfront list allocation for all ~34 k lines.
    for line in io.StringIO(data):
        line = line.rstrip("\n\r")
        if not line:
            continue

        # Opt 2: maxsplit=2 — stop after the three fields we need;
        #         avoids allocating the 11 unused trailing fields per line.
        fields = line.split(";", 2)
        if len(fields) < 3:
            continue

        cp = int(fields[0], 16)
        name = fields[1]
        # fields[2] may contain trailing semicolons; we only need the category
        # token which is always the first thing before any further delimiter.
        cat = fields[2].split(";", 1)[0]   # isolate category from remainder

        if name.endswith(", First>"):
            pending_range_start = cp
            pending_range_cat = cat

        elif name.endswith(", Last>"):
            if pending_range_start is not None:
                # Opt 3: dict.fromkeys runs entirely in C — no Python bytecode
                #         dispatch per codepoint.  For the CJK Unified Ideographs
                #         block (~20 902 entries) this is the dominant win.
                codepoints.update(
                    dict.fromkeys(range(pending_range_start, cp + 1), pending_range_cat)
                )
            pending_range_start = None
            pending_range_cat = None

        else:
            codepoints[cp] = cat
            # Opt 4: only clear pending state when it is actually set,
            #         avoiding a redundant store on the hot (non-range) path.
            if pending_range_start is not None:
                pending_range_start = None

    return codepoints


# -----------------------------------------------------------------------------
# Build sorted ranges
# -----------------------------------------------------------------------------

def build_ranges(
        codepoints: dict[int, str], categories: frozenset[str]
) -> list[tuple[int, int]]:
    """Build sorted, merged list of (first, last) inclusive ranges for the given categories."""
    category_set: frozenset[str] = (
        categories if isinstance(categories, frozenset) else frozenset(categories)
    )

    # --- Bottleneck 2 fix: bail out before any allocation on empty input.
    if not codepoints:
        return []

    # Collect and sort matching code points
    matching: list[int] = sorted(
        cp for cp, cat in codepoints.items() if cat in category_set
    )
    if not matching:
        return []

    ranges: list[tuple[int, int]] = []
    start = end = matching[0]

    for cp in matching[1:]:
        if cp == end + 1:
            end = cp
        else:
            ranges.append((start, end))
            start = end = cp  # --- Bottleneck 4 fix: collapsed tuple unpack
    ranges.append((start, end))
    return ranges


# -----------------------------------------------------------------------------
# SC-005 Conformance Validation
# -----------------------------------------------------------------------------

def validate_round_trip(
        codepoints: dict[int, str],
        generated_ranges: dict[str, list[tuple[int, int]]],
) -> None:
    """
    Re-parse the generated C++ ranges and verify 100% round-trip coverage
    against UnicodeData.txt for categories L, M, N, Zs, Zl, Zp.

    - Every code point in UnicodeData.txt with a matching General Category must
      be contained in exactly one generated range.
    - No generated range contains code points outside the category.

    Exits non-zero on any mismatch.
    """
    print("Running SC-005 conformance validation...")

    def in_ranges(cp: int, ranges: list[tuple[int, int]]) -> bool:
        i = bisect_right(ranges, cp, key=lambda r: r[0])
        return i > 0 and ranges[i - 1][1] >= cp

    # Build per-classifier expected sets from codepoints
    classifier_info = [
        ("is_letter (L)", LETTER_CATEGORIES, generated_ranges["letter"]),
        ("id_start (L+Nl)", ID_START_CATEGORIES, generated_ranges["id_start"]),
        ("id_continue (L+M+N+Pc)", ID_CONTINUE_CATEGORIES, generated_ranges["id_continue"]),
        ("whitespace (Zs+Zl+Zp+Cc)", WHITESPACE_CATEGORIES, generated_ranges["whitespace"]),
    ]

    errors = 0
    for name, cats, ranges in classifier_info:
        expected = {cp for cp, cat in codepoints.items() if cat in cats}
        # Check all expected are in ranges
        for cp in expected:
            if not in_ranges(cp, ranges):
                print(
                    f"  SC-005 FAIL [{name}]: U+{cp:04X} ({codepoints[cp]}) "
                    f"not in generated ranges"
                )
                errors += 1
                if errors > 10:
                    print("  (Too many errors, stopping...)")
                    sys.exit(1)
        # Check no extra codepoints in ranges
        for first, last in ranges:
            for cp in range(first, last + 1):
                cat = codepoints.get(cp)
                if cat not in cats:
                    print(
                        f"  SC-005 FAIL [{name}]: U+{cp:04X} (cat={cat}) in range "
                        f"U+{first:04X}-U+{last:04X} but not in expected categories"
                    )
                    errors += 1
                    if errors > 10:
                        print("  (Too many errors, stopping...)")
                        sys.exit(1)
        if errors == 0:
            print(f"  SC-005 PASS [{name}]: {len(ranges)} ranges, {len(expected)} code points OK")

    if errors > 0:
        print(f"\nSC-005 FAILED with {errors} errors. See above.")
        sys.exit(1)
    print("SC-005 conformance validation PASSED.\n")


# -----------------------------------------------------------------------------
# Format C++ output
# -----------------------------------------------------------------------------

def format_ranges_cpp(ranges: list[tuple[int, int]], indent: str = "    ") -> str:
    """Format ranges as C++ initializer list, 4 entries per line."""
    def _fmt_entry(first: int, last: int) -> str:
        # Extracted to a named closure so the f-string is written once;
        # also makes the generator expression below easier to read.
        return f"{{U'\\U{first:08X}', U'\\U{last:08X}'}}"
    return "\n".join(
        # Each batch is at most 4 pairs; batched() handles the final short batch
        # automatically — no manual flush condition needed.
        indent + ", ".join(_fmt_entry(f, l) for f, l in batch) + ","
        for batch in batched(ranges, 4)
    )


def generate_header(
        generated_ranges: dict[str, list[tuple[int, int]]],
        unicode_version: str,
        generation_date: str,
) -> str:
    letter_ranges = generated_ranges["letter"]
    id_start_ranges = generated_ranges["id_start"]
    id_continue_ranges = generated_ranges["id_continue"]
    whitespace_ranges = generated_ranges["whitespace"]

    letter_cpp = format_ranges_cpp(letter_ranges)
    id_start_cpp = format_ranges_cpp(id_start_ranges)
    id_continue_cpp = format_ranges_cpp(id_continue_ranges)
    whitespace_cpp = format_ranges_cpp(whitespace_ranges)

    return f"""\
/*
 * Generated by scripts/generate_unicode_tables.py
 * Unicode version: {unicode_version}
 * Generation date: {generation_date}
 * DO NOT EDIT — regenerate with: python scripts/generate_unicode_tables.py
 */

#pragma once
// NOLINTBEGIN(*-magic-numbers, *-avoid-magic-numbers)

#include <algorithm>
#include <array>
#include <span>

namespace jsv::unicode {{
    /// Inclusive range of Unicode code points.
    struct CodepointRange {{
        char32_t first;  ///< Inclusive start
        char32_t last;   ///< Inclusive end
    }};

    // =========================================================================
    // Letter ranges — General Category L (Lu, Ll, Lt, Lm, Lo)
    // Used by is_letter()
    // =========================================================================

    static inline constexpr std::array<CodepointRange, {len(letter_ranges)}> letter_ranges{{{{{letter_cpp}}}}};

    static_assert(letter_ranges.size() == {len(letter_ranges)}, "letter_ranges size mismatch");

    // =========================================================================
    // id_start ranges — General Category L + Nl
    // Used by is_id_start()
    // =========================================================================

    static inline constexpr std::array<CodepointRange, {len(id_start_ranges)}> id_start_ranges{{{{{id_start_cpp}}}}};

    static_assert(id_start_ranges.size() == {len(id_start_ranges)}, "id_start_ranges size mismatch");
    static_assert(
        []() consteval {{
            for(std::size_t i = 1; i < id_start_ranges.size(); ++i) {{
                if(id_start_ranges[i - 1].last >= id_start_ranges[i].first) {{ return false; }}
            }}
            return true;
        }}(),
        "id_start_ranges must be sorted and non-overlapping (required for binary search)");

    // =========================================================================
    // id_continue ranges — General Category L + M + N + Pc
    // Used by is_id_continue()
    // =========================================================================

    static inline constexpr std::array<CodepointRange, {len(id_continue_ranges)}> id_continue_ranges{{{{{id_continue_cpp}}}}};

    static_assert(id_continue_ranges.size() == {len(id_continue_ranges)}, "id_continue_ranges size mismatch");
    static_assert(
        []() consteval {{
            for(std::size_t i = 1; i < id_continue_ranges.size(); ++i) {{
                if(id_continue_ranges[i - 1].last >= id_continue_ranges[i].first) {{ return false; }}
            }}
            return true;
        }}(),
        "id_continue_ranges must be sorted and non-overlapping (required for binary search)");

    // =========================================================================
    // whitespace ranges — General Category Zs + Zl + Zp
    // Used by is_unicode_whitespace()
    // =========================================================================

    static inline constexpr std::array<CodepointRange, {len(whitespace_ranges)}> whitespace_ranges{{{{{whitespace_cpp}}}}};

    static_assert(whitespace_ranges.size() == {len(whitespace_ranges)}, "whitespace_ranges size mismatch");
    static_assert(
        []() consteval {{
            for(std::size_t i = 1; i < whitespace_ranges.size(); ++i) {{
                if(whitespace_ranges[i - 1].last >= whitespace_ranges[i].first) {{ return false; }}
            }}
            return true;
        }}(),
        "whitespace_ranges must be sorted and non-overlapping (required for binary search)");

    // =========================================================================
    // Classification functions
    // =========================================================================

    namespace detail {{
        /// Binary search: true if cp is in any of the sorted non-overlapping ranges.
        ///
        /// Pre:  ranges is sorted by `first` and non-overlapping
        ///       (guaranteed by the static_assert blocks at each table definition).
        ///       The memory backing the span must outlive this call.
        /// Post: returns true iff cp belongs to at least one range in the span.
        /// Note: O(log N) via std::upper_bound.
        [[nodiscard]] constexpr bool in_ranges(
            char32_t cp, std::span<const CodepointRange> ranges) noexcept {{
            // Find the first range whose `first` is strictly greater than cp.
            auto it = std::upper_bound(
                ranges.begin(), ranges.end(), cp,
                [](char32_t val, const CodepointRange& r) noexcept {{
                    return val < r.first;
                }});
            if(it == ranges.begin()) {{ return false; }}
            --it;
            return cp <= it->last;
        }}
    }}  // namespace detail

    /// True if cp has Unicode General Category L (Letter).
    /// ASCII fast-path: [A-Za-z]
    [[nodiscard]] constexpr bool is_letter(char32_t cp) noexcept {{
        if(cp < 0x80U) {{ return (cp >= U'A' && cp <= U'Z') || (cp >= U'a' && cp <= U'z'); }}
        return detail::in_ranges(cp, letter_ranges);
    }}

    /// True if cp may start an identifier: \\p{{Letter}} or '_' (U+005F).
    /// ASCII fast-path: [A-Za-z_]
    [[nodiscard]] constexpr bool is_id_start(char32_t cp) noexcept {{
        if(cp < 0x80U) {{
            return (cp >= U'A' && cp <= U'Z') || (cp >= U'a' && cp <= U'z') || cp == U'_';
        }}
        return detail::in_ranges(cp, id_start_ranges);
    }}

    /// True if cp may continue an identifier:
    /// \\p{{Letter}} | \\p{{Mark}} | \\p{{Number}} | '_' | ASCII digits.
    /// ASCII fast-path: [A-Za-z0-9_]
    [[nodiscard]] constexpr bool is_id_continue(char32_t cp) noexcept {{
        if(cp < 0x80U) {{
            return (cp >= U'A' && cp <= U'Z') || (cp >= U'a' && cp <= U'z')
                || (cp >= U'0' && cp <= U'9') || cp == U'_';
        }}
        return detail::in_ranges(cp, id_continue_ranges);
    }}

    /// True if cp is Unicode whitespace: General Category Zs, Zl, or Zp.
    /// Note: U+0020 SPACE is General Category Zs and returns true.
    /// Other ASCII whitespace (tab U+0009, LF U+000A, CR U+000D) are NOT Zs/Zl/Zp.
    [[nodiscard]] constexpr bool is_unicode_whitespace(char32_t cp) noexcept {{
        if(cp == U' ') {{ return true; }}   // U+0020 SPACE is Zs — fast-path
        if(cp < 0x80U) {{ return false; }}  // Other ASCII not in Zs/Zl/Zp
        return detail::in_ranges(cp, whitespace_ranges);
    }}

    /// True if cp is a Unicode line terminator: NEL (U+0085), LINE SEPARATOR (U+2028),
    /// or PARAGRAPH SEPARATOR (U+2029). Used by skip_unicode_whitespace() to determine
    /// when to increment the line counter and reset the column counter.
    /// Note: LF (U+000A) is handled by the ASCII fast-path and is NOT included here.
    [[nodiscard]] constexpr bool is_unicode_line_terminator(char32_t cp) noexcept {{
        return cp == U'\\U00000085' || cp == U'\\U00002028' || cp == U'\\U00002029';
    }}
}}  // namespace jsv::unicode

// NOLINTEND(*-magic-numbers, *-avoid-magic-numbers)
"""


# -----------------------------------------------------------------------------
# clang-format Utility
# -----------------------------------------------------------------------------

def run_clang_format(file_path: Path) -> None:
    timeout = 30
    """
    Run clang-format on the generated file using the project's .clang-format config.

    Args:
        file_path: Path to the file to format.

    Raises:
        SystemExit: If clang-format fails.
    """
    print("Running clang-format...")
    # Find the project root (directory containing .clang-format)
    script_dir = Path(__file__).parent
    project_root = script_dir.parent
    clang_format_config = project_root / ".clang-format"

    # Resolve clang-format executable path explicitly
    clang_format_bin = shutil.which("clang-format")
    if clang_format_bin is None:
        print("  clang-format not found in PATH. Skipping formatting.")
        return

    try:
        subprocess.run(
            [
                clang_format_bin,
                "-i",
                f"--style=file:{clang_format_config}",
                str(file_path),
            ],
            # Only capture stderr: we never inspect stdout on the success path,
            # so routing it to DEVNULL avoids allocating a stdout pipe buffer
            # in Python heap memory entirely.
            stdout=subprocess.DEVNULL,
            stderr=subprocess.PIPE,
            text=True,
            check=True,
            # Prevent an unresponsive clang-format from blocking the process
            # indefinitely; raises subprocess.TimeoutExpired on breach.
            timeout=timeout,
        )
    except subprocess.TimeoutExpired as e:
        # Re-raise with richer context; caller decides whether to retry or abort.
        raise subprocess.TimeoutExpired(
            e.cmd, timeout, stderr=e.stderr
        ) from e
    except subprocess.CalledProcessError as e:
        # Raise instead of sys.exit so this function remains composable and
        # unit-testable without spawning a real subprocess in every test.
        raise RuntimeError(
            f"clang-format failed on {file_path!r}:\n{e.stderr}"
        ) from e

    print("  clang-format completed successfully.")


# -----------------------------------------------------------------------------
# Main
# -----------------------------------------------------------------------------

def main() -> None:
    start_time = time.perf_counter_ns()

    data = download_unicode_data()
    print("Parsing UnicodeData.txt...")
    codepoints = parse_codepoints(data)
    print(f"  Parsed {len(codepoints):,} code points.")

    print("Building ranges...")
    generated_ranges = {
        "letter": build_ranges(codepoints, LETTER_CATEGORIES),
        "id_start": build_ranges(codepoints, ID_START_CATEGORIES),
        "id_continue": build_ranges(codepoints, ID_CONTINUE_CATEGORIES),
        "whitespace": build_ranges(codepoints, WHITESPACE_CATEGORIES),
    }
    sys.stdout.write(
        "\n".join(f"  {name}: {len(ranges)} ranges" for name, ranges in generated_ranges.items())+ "\n"
    )

    # SC-005 conformance validation
    validate_round_trip(codepoints, generated_ranges)

    generation_date = date.today().isoformat()
    header_content = generate_header(generated_ranges, UNICODE_VERSION, generation_date)

    OUTPUT_PATH.parent.mkdir(parents=True, exist_ok=True)
    OUTPUT_PATH.write_text(header_content, encoding="utf-8")
    line_count = header_content.count("\n")
    size, unit = format_size(len(header_content))
    print(f"Written: {OUTPUT_PATH}\n{size:.2f} {unit} ({len(header_content):,} bytes), {line_count} lines.")


    end_time = time.perf_counter_ns()
    total_time = end_time - start_time
    start_time_clang = time.perf_counter_ns()
    run_clang_format(OUTPUT_PATH)
    end_time_clang = time.perf_counter_ns()
    total_time_clang = end_time_clang - start_time_clang
    print(f"Total generation time: {format_duration(total_time)}")
    print(f"Total clang-format time: {format_duration(total_time_clang)}")


if __name__ == "__main__":
    main()
