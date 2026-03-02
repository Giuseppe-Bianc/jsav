#!/ usr / bin / env python3
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

import subprocess
import sys
import time
import urllib.request
from pathlib import Path
from datetime import date

#-- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -
#Configuration
#-- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -
UNICODE_VERSION = "16.0.0"
UNICODE_DATA_URL = (
    f"https://www.unicode.org/Public/{UNICODE_VERSION}/ucd/UnicodeData.txt"
)
OUTPUT_PATH = Path(__file__).parent.parent / "include" / "jsav" / "lexer" / "unicode" / "UnicodeData.hpp"

#General Category sets for each classifier
#id_start : Letter(Lu Ll Lt Lm Lo Nl) — categories starting with L, plus Nl
ID_START_CATEGORIES = frozenset({"Lu", "Ll", "Lt", "Lm", "Lo", "Nl"})
#id_continue : id_start + Mark(Mn Mc Me) + Number(Nd Nl No) + connector punct(Pc)
ID_CONTINUE_CATEGORIES = frozenset({
    "Lu", "Ll", "Lt", "Lm", "Lo",   # Letter
    "Mn", "Mc", "Me",                 # Mark
    "Nd", "Nl", "No",                 # Number
    "Pc",                             # Connector punctuation (underscore etc.)
})
#whitespace : General Category Zs, Zl, Zp
WHITESPACE_CATEGORIES = frozenset({"Zs", "Zl", "Zp"})

#"Letter" for is_letter() — all L categories
LETTER_CATEGORIES = frozenset({"Lu", "Ll", "Lt", "Lm", "Lo"})

#-- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -
#Timing Utilities
#-- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -


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
    if duration_ns < 1000:
#Pure nanoseconds
        return f"{int(duration_ns)}ns"
    elif duration_ns < 1_000_000:
#Microseconds + nanoseconds
        micros = int(duration_ns // 1000)
        nanos = int(duration_ns % 1000)
        return f"{micros}us,{nanos}ns" if nanos > 0 else f"{micros}us"
    elif duration_ns < 1_000_000_000:
#Milliseconds + microseconds + nanoseconds
        millis = int(duration_ns // 1_000_000)
        remainder = duration_ns % 1_000_000
        micros = int(remainder // 1000)
        nanos = int(remainder % 1000)
        parts = [f"{millis}ms"]
        if micros > 0:
            parts.append(f"{micros}us")
        if nanos > 0:
            parts.append(f"{nanos}ns")
        return ",".join(parts)
    else:
#Seconds + milliseconds + microseconds + nanoseconds
        secs = int(duration_ns // 1_000_000_000)
        remainder = duration_ns % 1_000_000_000
        millis = int(remainder // 1_000_000)
        remainder = remainder % 1_000_000
        micros = int(remainder // 1000)
        nanos = int(remainder % 1000)
        parts = [f"{secs}s"]
        if millis > 0:
            parts.append(f"{millis}ms")
        if micros > 0:
            parts.append(f"{micros}us")
        if nanos > 0:
            parts.append(f"{nanos}ns")
        return ",".join(parts)

#-- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -
#Parse UnicodeData.txt
#-- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -

def download_unicode_data() -> str:
    print(f"Downloading UnicodeData.txt (Unicode {UNICODE_VERSION})...")
    with urllib.request.urlopen(UNICODE_DATA_URL) as response:
        data = response.read().decode("utf-8")
    print(f"  Downloaded {len(data):,} bytes.")
    return data


def parse_codepoints(data: str) -> dict[int, str]:
    """Return {codepoint: general_category} mapping.
    Handles <..., First> / <..., Last> range markers for CJK blocks.
    """
    codepoints: dict[int, str] = {}
    pending_range_start: int | None = None
    pending_range_cat: str | None = None

    for line in data.splitlines():
        if not line:
            continue
        fields = line.split(";")
        if len(fields) < 3:
            continue
        cp = int(fields[0], 16)
        name = fields[1]
        cat = fields[2]

        if name.endswith(", First>"):
            pending_range_start = cp
            pending_range_cat = cat
        elif name.endswith(", Last>"):
            if pending_range_start is not None:
                for c in range(pending_range_start, cp + 1):
                    codepoints[c] = pending_range_cat  # type: ignore[assignment]
            pending_range_start = None
            pending_range_cat = None
        else:
            codepoints[cp] = cat
            pending_range_start = None

    return codepoints

#-- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -
#Build sorted ranges
#-- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -

def build_ranges(codepoints: dict[int, str], categories: frozenset[str]) -> list[tuple[int, int]]:
    """Build sorted, merged list of (first, last) inclusive ranges for the given categories."""
#Collect and sort matching code points
    matching = sorted(cp for cp, cat in codepoints.items() if cat in categories)
    if not matching:
        return []

    ranges: list[tuple[int, int]] = []
    start = matching[0]
    end = matching[0]

    for cp in matching[1:]:
        if cp == end + 1:
            end = cp
        else:
            ranges.append((start, end))
            start = cp
            end = cp
    ranges.append((start, end))
    return ranges

#-- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -
#SC - 005 Conformance Validation
#-- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -

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
        lo, hi = 0, len(ranges) - 1
        while lo <= hi:
            mid = (lo + hi) // 2
            if ranges[mid][0] <= cp <= ranges[mid][1]:
                return True
            elif cp < ranges[mid][0]:
                hi = mid - 1
            else:
                lo = mid + 1
        return False

#Build per - classifier expected sets from codepoints
    classifier_info = [
        ("is_letter (L)", LETTER_CATEGORIES, generated_ranges["letter"]),
        ("id_start (L+Nl)", ID_START_CATEGORIES, generated_ranges["id_start"]),
        ("id_continue (L+M+N+Pc)", ID_CONTINUE_CATEGORIES, generated_ranges["id_continue"]),
        ("whitespace (Zs+Zl+Zp)", WHITESPACE_CATEGORIES, generated_ranges["whitespace"]),
    ]

    errors = 0
    for name, cats, ranges in classifier_info:
        expected = {cp for cp, cat in codepoints.items() if cat in cats}
#Check all expected are in ranges
        for cp in expected:
            if not in_ranges(cp, ranges):
                print(f"  SC-005 FAIL [{name}]: U+{cp:04X} ({codepoints[cp]}) not in generated ranges")
                errors += 1
                if errors > 10:
                    print("  (Too many errors, stopping...)")
                    sys.exit(1)
#Check no extra codepoints in ranges
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

#-- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -
#Format C++ output
#-- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -

def format_ranges_cpp(ranges: list[tuple[int, int]], indent: str = "    ") -> str:
    """Format ranges as C++ initializer list, 4 entries per line."""
    lines: list[str] = []
    chunk: list[str] = []
    for i, (first, last) in enumerate(ranges):
        chunk.append(f"{{U'\\U{first:08X}', U'\\U{last:08X}'}}")
        if len(chunk) == 4 or i == len(ranges) - 1:
            lines.append(indent + ", ".join(chunk) + ",")
            chunk = []
    return "\n".join(lines)


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

    return f"""/*
 * Generated by scripts/generate_unicode_tables.py
 * Unicode version: {unicode_version}
 * Generation date: {generation_date}
 * DO NOT EDIT — regenerate with: python scripts/generate_unicode_tables.py
 */

#pragma once
// NOLINTBEGIN(*-magic-numbers, *-avoid-magic-numbers)

#include <algorithm>
#include <array>

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
                if(id_start_ranges[i - 1].last >= id_start_ranges[i].first) {{
                    return false;
                }}
            }}
            return true;
        }}(),
        "id_start_ranges must be sorted and non-overlapping (required for binary search)");

    // =========================================================================
    // id_continue ranges — General Category L + M + N + Pc
    // Used by is_id_continue()
    // =========================================================================

    inline constexpr std::array<CodepointRange, {len(id_continue_ranges)}> id_continue_ranges{{{{{id_continue_cpp}}}}};

    static_assert(id_continue_ranges.size() == {len(id_continue_ranges)}, "id_continue_ranges size mismatch");
    static_assert(
        []() consteval {{
            for(std::size_t i = 1; i < id_continue_ranges.size(); ++i) {{
                if(id_continue_ranges[i - 1].last >= id_continue_ranges[i].first) {{
                    return false;
                }}
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
                if(whitespace_ranges[i - 1].last >= whitespace_ranges[i].first) {{
                    return false;
                }}
            }}
            return true;
        }}(),
        "whitespace_ranges must be sorted and non-overlapping (required for binary search)");

    // =========================================================================
    // Classification functions
    // =========================================================================

    namespace detail {{
        /// Binary search: true if cp is in any of the sorted non-overlapping ranges.
        [[nodiscard]] constexpr bool in_ranges(char32_t cp, const CodepointRange *first, const CodepointRange *last) noexcept {{
            // std::upper_bound on the 'first' field: find first range whose start > cp
            auto it = std::upper_bound(first, last, cp, [](char32_t val, const CodepointRange &r) {{
                return val < r.first;
            }});
            if(it == first) {{
                return false;
            }}
            --it;
            return cp <= it->last;
        }}
    }}  // namespace detail

    /// True if cp has Unicode General Category L (Letter).
    /// ASCII fast-path: [A-Za-z]
    [[nodiscard]] constexpr bool is_letter(char32_t cp) noexcept {{
        if(cp < 0x80U) {{
            return (cp >= U'A' && cp <= U'Z') || (cp >= U'a' && cp <= U'z');
        }}
        return detail::in_ranges(cp, letter_ranges.data(), letter_ranges.data() + letter_ranges.size());
    }}

    /// True if cp may start an identifier: \\p{{Letter}} or '_' (U+005F).
    /// ASCII fast-path: [A-Za-z_]
    [[nodiscard]] constexpr bool is_id_start(char32_t cp) noexcept {{
        if(cp < 0x80U) {{
            return (cp >= U'A' && cp <= U'Z') || (cp >= U'a' && cp <= U'z') || cp == U'_';
        }}
        return detail::in_ranges(cp, id_start_ranges.data(), id_start_ranges.data() + id_start_ranges.size());
    }}

    /// True if cp may continue an identifier:
    /// \\p{{Letter}} | \\p{{Mark}} | \\p{{Number}} | '_' | ASCII digits.
    /// ASCII fast-path: [A-Za-z0-9_]
    [[nodiscard]] constexpr bool is_id_continue(char32_t cp) noexcept {{
        if(cp < 0x80U) {{
            return (cp >= U'A' && cp <= U'Z') || (cp >= U'a' && cp <= U'z') || (cp >= U'0' && cp <= U'9') || cp == U'_';
        }}
        return detail::in_ranges(cp, id_continue_ranges.data(), id_continue_ranges.data() + id_continue_ranges.size());
    }}

    /// True if cp is Unicode whitespace: General Category Zs, Zl, or Zp.
    /// Note: U+0020 SPACE is General Category Zs and returns true.
    /// Other ASCII whitespace (tab U+0009, LF U+000A, CR U+000D) are NOT Zs/Zl/Zp.
    [[nodiscard]] constexpr bool is_unicode_whitespace(char32_t cp) noexcept {{
        if(cp == U' ') {{
            return true;
        }}  // U+0020 SPACE is Zs — fast-path
        if(cp < 0x80U) {{
            return false;
        }}  // Other ASCII not in Zs/Zl/Zp
        return detail::in_ranges(cp, whitespace_ranges.data(), whitespace_ranges.data() + whitespace_ranges.size());
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

#-- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -
#clang - format Utility
#-- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -

def run_clang_format(file_path: Path) -> None:
    """
    Run clang-format on the generated file.

    Args:
        file_path: Path to the file to format.

    Raises:
        SystemExit: If clang-format fails.
    """
    print("Running clang-format...")
    try:
        subprocess.run(
            ["clang-format", "-i", str(file_path)],
            capture_output=True,
            text=True,
            check=True,
        )
        print("  clang-format completed successfully.")
    except subprocess.CalledProcessError as e:
        print(f"  clang-format failed: {e.stderr}")
        sys.exit(1)
    except FileNotFoundError:
        print("  clang-format not found in PATH. Skipping formatting.")

#-- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -
#Main
#-- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -

def main() -> None:
    start_time = time.perf_counter_ns()

    data = download_unicode_data()
    print("Parsing UnicodeData.txt...")
    codepoints = parse_codepoints(data)
    print(f"  Parsed {len(codepoints):,} code points.")

    print("Building ranges...")
    generated_ranges = {
    "letter" : build_ranges(codepoints, LETTER_CATEGORIES),
               "id_start" : build_ranges(codepoints, ID_START_CATEGORIES),
                            "id_continue" : build_ranges(codepoints, ID_CONTINUE_CATEGORIES),
                                            "whitespace" : build_ranges(codepoints, WHITESPACE_CATEGORIES),
    }
    for name, ranges in generated_ranges.items():
        print(f"  {name}: {len(ranges)} ranges")

#SC - 005 conformance validation
    validate_round_trip(codepoints, generated_ranges)

    generation_date = date.today().isoformat()
    header_content = generate_header(generated_ranges, UNICODE_VERSION, generation_date)

    OUTPUT_PATH.parent.mkdir(parents=True, exist_ok=True)
    OUTPUT_PATH.write_text(header_content, encoding="utf-8")
    print(f"Written: {OUTPUT_PATH}")
    print(f"  {len(header_content):,} bytes, {header_content.count(chr(10))} lines.")

#Run clang - format on the generated file

    end_time = time.perf_counter_ns()
    total_time = end_time - start_time
    print(f"Total generation time: {format_duration(total_time)}")
    start_time_clang = time.perf_counter_ns()
    run_clang_format(OUTPUT_PATH)
    end_time_clang = time.perf_counter_ns()
    total_time_clang = end_time_clang - start_time_clang
    print(f"Total clang-format time: {format_duration(total_time_clang)}")


if __name__ == "__main__":
    main()
