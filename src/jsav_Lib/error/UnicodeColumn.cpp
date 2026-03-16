/*
 * Created by gbian on 16/03/2026.
 * Copyright (c) 2026 All rights reserved.
 */

// NOLINTBEGIN(*-include-cleaner)
#include "jsav/error/UnicodeColumn.hpp"
#include "jsav/lexer/unicode/Utf8.hpp"
#include "jsavCore/Log.hpp"

#include <cstdlib>
#include <cstring>

namespace jsv {

// -----------------------------------------------------------------------------
// detect_ansi_color
// -----------------------------------------------------------------------------
[[nodiscard]] bool detect_ansi_color() noexcept {
    // 1. Check NO_COLOR (overrides all) - per https://no-color.org/
    if(const char* no_color = std::getenv("NO_COLOR");
       no_color != nullptr && no_color[0] != '\0') {
        return false;
    }

    // 2. Check COLORTERM (truecolor/24bit)
    if(const char* colorterm = std::getenv("COLORTERM");
       colorterm != nullptr) {
        if(std::strcmp(colorterm, "truecolor") == 0 ||
           std::strcmp(colorterm, "24bit") == 0) {
            return true;
        }
    }

    // 3. Check TERM (contains "color", "xterm", "screen", "tmux")
    if(const char *term = std::getenv("TERM"); term != nullptr) {
        const std::string_view termView{term};
        if(termView.contains("color") || termView.contains("xterm") || termView.contains("screen") || termView.contains("tmux")) {
            return true;
        }
    }

    // 4. Default: no color (conservative fallback)
    return false;
}

// -----------------------------------------------------------------------------
// make_display_config
// -----------------------------------------------------------------------------
[[nodiscard]] ErrorDisplayConfig make_display_config() noexcept {
    ErrorDisplayConfig config;
    config.tab_stop_width = 8;  // Default matches GCC/Clang/MSVC
    config.ansi_color = detect_ansi_color();
    return config;
}

// -----------------------------------------------------------------------------
// visual_column
// -----------------------------------------------------------------------------
[[nodiscard]] std::expected<std::size_t, std::string>
visual_column(std::string_view line, std::size_t byte_offset,
              std::size_t tab_stop_width) noexcept {
    // Defensive: clamp byte_offset to line size
    if(byte_offset > line.size()) {
        byte_offset = line.size();
    }

    // FR-027: Line length limit (10,000 code points maximum)
    constexpr std::size_t k_max_code_points = 10000;

    std::size_t col = 1;  // 1-based visual column
    std::size_t pos = 0;  // Current byte position

    // FR-019: Skip BOM at file start (only at position 0 of the full source)
    // Note: This function receives a single line, so BOM detection is limited
    // to line 1. The caller (ErrorReporter) knows if this is line 1.
    if(line.size() >= 3 && pos == 0 && C_UC(line[0]) == 0xEF && C_UC(line[1]) == 0xBB && C_UC(line[2]) == 0xBF) {
        pos = 3;  // Skip BOM bytes
    }

    std::size_t code_point_count = 0;

    // Walk UTF-8 code points from start to byte_offset
    while(pos < byte_offset && pos < line.size()) {
        const auto res = jsv::unicode::decode_utf8(line, pos);

        // FR-025, FR-026, FR-020: Invalid UTF-8 detection
        if(res.status != jsv::unicode::Utf8Status::Ok) {
            // Classify error type for precise error message
            std::string error_msg;
            switch(res.status) {
            case jsv::unicode::Utf8Status::Overlong:
                error_msg = FORMAT("Overlong UTF-8 encoding at byte offset {}", pos);
                break;
            case jsv::unicode::Utf8Status::Surrogate:
                error_msg = FORMAT("UTF-16 surrogate half (U+D800–U+DFFF) not allowed in source files at byte offset {}", pos);
                break;
            case jsv::unicode::Utf8Status::OutOfRange:
            case jsv::unicode::Utf8Status::InvalidLeadByte:
            case jsv::unicode::Utf8Status::OrphanedContinuation:
            case jsv::unicode::Utf8Status::TruncatedSequence:
                error_msg = FORMAT("Invalid UTF-8 sequence at byte offset {}", pos);
                break;
            default:
                error_msg = FORMAT("Invalid UTF-8 sequence at byte offset {}", pos);
                break;
            }

            // FR-028: Log error at error level
            LERROR("{}", error_msg);
            return std::unexpected(error_msg);
        }

        // FR-020: Null byte (U+0000) rejection
        if(res.codepoint == U'\0') {
            const std::string error_msg = FORMAT("Null byte (U+0000) at byte offset {}", pos);
            LERROR("{}", error_msg);
            return std::unexpected(error_msg);
        }

        // FR-027: Line length limit check
        ++code_point_count;
        if(code_point_count > k_max_code_points) {
            const std::string error_msg = FORMAT("Line exceeds maximum length of {} code points (actual: {})",
                                                  k_max_code_points, code_point_count);
            LERROR("{}", error_msg);
            return std::unexpected(error_msg);
        }

        // FR-018: Tab expansion
        if(res.codepoint == U'\t') {
            // Formula: ((col - 1) / tab_stop_width + 1) * tab_stop_width + 1
            // Advances to next tab stop
            col = ((col - 1) / tab_stop_width + 1) * tab_stop_width + 1;
        } else {
            // All other code points: 1 column each (DEC-002: code points, not graphemes)
            col += 1;
        }

        pos += res.byte_length;
    }

    // FR-013: Clamp to line_end + 1 if byte_offset past end
    // (already handled by loop condition - returns col at end of line)

    return col;
}

// -----------------------------------------------------------------------------
// marker_extents
// -----------------------------------------------------------------------------
[[nodiscard]] std::expected<std::pair<std::size_t, std::size_t>, std::string>
marker_extents(std::string_view line, std::size_t start_byte, std::size_t end_byte,
               std::size_t tab_stop_width) noexcept {
    // Defensive: clamp byte offsets to line size
    if(start_byte > line.size()) {
        start_byte = line.size();
    }
    if(end_byte > line.size()) {
        end_byte = line.size();
    }

    // Calculate leading spaces (convert 1-based column to 0-based space count)
    auto leading_result = visual_column(line, start_byte, tab_stop_width);
    if(!leading_result.has_value()) {
        return std::unexpected(std::move(leading_result.error()));
    }
    std::size_t leading_spaces = leading_result.value() - 1;

    // FR-013: Minimum 1 caret guarantee
    if(start_byte >= end_byte) {
        return std::make_pair(leading_spaces, 1u);
    }

    // Calculate caret count by walking from start_byte to end_byte
    std::size_t caret_count = 0;
    std::size_t pos = start_byte;
    std::size_t code_point_count = 0;
    constexpr std::size_t k_max_code_points = 10000;

    while(pos < end_byte && pos < line.size()) {
        const auto res = jsv::unicode::decode_utf8(line, pos);

        // FR-025, FR-026, FR-020: Invalid UTF-8 detection (same as visual_column)
        if(res.status != jsv::unicode::Utf8Status::Ok) {
            std::string error_msg;
            switch(res.status) {
            case jsv::unicode::Utf8Status::Overlong:
                error_msg = FORMAT("Overlong UTF-8 encoding at byte offset {}", pos);
                break;
            case jsv::unicode::Utf8Status::Surrogate:
                error_msg = FORMAT("UTF-16 surrogate half (U+D800–U+DFFF) not allowed in source files at byte offset {}", pos);
                break;
            case jsv::unicode::Utf8Status::OutOfRange:
            case jsv::unicode::Utf8Status::InvalidLeadByte:
            case jsv::unicode::Utf8Status::OrphanedContinuation:
            case jsv::unicode::Utf8Status::TruncatedSequence:
                error_msg = FORMAT("Invalid UTF-8 sequence at byte offset {}", pos);
                break;
            default:
                error_msg = FORMAT("Invalid UTF-8 sequence at byte offset {}", pos);
                break;
            }

            LERROR("{}", error_msg);
            return std::unexpected(std::move(error_msg));
        }

        // FR-020: Null byte rejection
        if(res.codepoint == U'\0') {
            const std::string error_msg = FORMAT("Null byte (U+0000) at byte offset {}", pos);
            LERROR("{}", error_msg);
            return std::unexpected(error_msg);
        }

        // FR-027: Line length limit check
        ++code_point_count;
        if(code_point_count > k_max_code_points) {
            const std::string error_msg = FORMAT("Line exceeds maximum length of {} code points (actual: {})",
                                                  k_max_code_points, code_point_count);
            LERROR("{}", error_msg);
            return std::unexpected(error_msg);
        }

        // FR-010: Tabs contribute expanded width to caret count
        if(res.codepoint == U'\t') {
            // Expand tab to next tab stop (same formula as visual_column)
            const std::size_t next_tab = ((caret_count / tab_stop_width) + 1) * tab_stop_width;
            caret_count += (next_tab - caret_count);
        } else {
            // All other code points: 1 caret each
            caret_count += 1;
        }

        pos += res.byte_length;
    }

    // FR-013: Minimum 1 caret guarantee (already handled above for empty spans)
    if(caret_count == 0) {
        caret_count = 1;
    }

    return std::make_pair(leading_spaces, caret_count);
}

}  // namespace jsv

// NOLINTEND(*-include-cleaner)
