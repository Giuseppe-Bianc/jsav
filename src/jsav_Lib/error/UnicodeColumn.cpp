/*
 * Created by gbian on 14/03/2026.
 * Copyright (c) 2026 All rights reserved.
 */
// NOLINTBEGIN(*-include-cleaner, *-mt-unsafe, *-easily-swappable-parameters, *-diagnostic-deprecated-declarations)
#include "jsav/error/UnicodeColumn.hpp"

namespace jsv {
    constexpr std::size_t MAX_CODE_POINTS = 10000;
    static constexpr std::string_view utf8_bom{"\xEF\xBB\xBF"};
    static constexpr std::array<std::string_view, 4> known_color_terms{"color", "xterm", "screen", "tmux"};
    [[nodiscard]] static std::optional<std::string> safe_getenv(const char *name) noexcept {
#ifdef _MSC_VER
        char *buf = nullptr;
        std::size_t len = 0;
        if(_dupenv_s(&buf, &len, name) != 0 || buf == nullptr) { return std::nullopt; }
        std::string value(buf, len > 0 ? len - 1 : 0);  // len includes '\0'
        free(buf);                                      // NOLINT(*-no-malloc) — _dupenv_s contract requires free()
        return value;
#else
        const char *val = std::getenv(name);
        if(val == nullptr) { return std::nullopt; }
        return std::string(val);
#endif
    }
    // ---------------------------------------------------------------------------
    // detect_ansi_color()
    // ---------------------------------------------------------------------------
    [[nodiscard]] bool detect_ansi_color() noexcept {
        // 1. Check NO_COLOR (overrides all)
        // Per https://no-color.org/ - any non-empty value disables color
        if(const auto no_color = safe_getenv("NO_COLOR"); no_color.has_value()) {
            if(const std::string_view no_color_view{*no_color}; !no_color_view.empty()) { return false; }
        }

        // 2. Check COLORTERM (truecolor/24bit)
        if(const auto colorterm = safe_getenv("COLORTERM"); colorterm.has_value()) {
            if(const std::string_view colorterm_view{*colorterm}; colorterm_view == "truecolor" || colorterm_view == "24bit") {
                return true;
            }
        }

        if(const auto term = safe_getenv("TERM"); term.has_value()) {
            const std::string_view term_view{*term};
            // C++23: string_view::contains() replaces find() != npos — states intent directly
            if(std::ranges::any_of(known_color_terms,
                                   [&term_view](const std::string_view keyword) noexcept { return term_view.contains(keyword); })) {
                return true;
            }
        }

        // 3. Default: colors enabled
        return true;
    }

    // ---------------------------------------------------------------------------
    // make_display_config()
    // ---------------------------------------------------------------------------
    [[nodiscard]] ErrorDisplayConfig make_display_config() noexcept {
        ErrorDisplayConfig config;
        config.tab_stop_width = tab_stop_width_default;  // Default (matches GCC/Clang/MSVC)
        config.ansi_color = detect_ansi_color();
        return config;
    }

    // ---------------------------------------------------------------------------
    // visual_column()
    // ---------------------------------------------------------------------------
    [[nodiscard]] std::expected<std::size_t, std::string> visual_column(std::string_view line, std::size_t byte_offset,
                                                                        std::size_t tab_stop_width) {
        // Precondition: tab_stop_width > 0 (caller responsibility)
        // Note: Division by zero would occur in tab expansion formula if tab_stop_width == 0
        if(tab_stop_width == 0) [[unlikely]] { return std::unexpected(std::string("tab_stop_width must be greater than 0")); }

        std::size_t col = 1;  // 1-based visual column
        std::size_t pos = 0;  // Byte position in line

        // FR-019: Skip BOM at file start (only at position 0)
        if(line.size() >= 3 && line.starts_with(utf8_bom)) {
            pos = 3;  // Skip BOM bytes
        }

        // Walk UTF-8 code points from start to byte_offset
        std::size_t code_point_count = 0;

        while(pos < byte_offset && pos < line.size()) {
            // Decode UTF-8 sequence at current position
            const auto res = unicode::decode_utf8(line, pos);

            // FR-025, FR-026: Check for invalid UTF-8
            if(res.status != unicode::Utf8Status::Ok) [[unlikely]] {
                LERROR("Invalid UTF-8 sequence at byte offset {}", pos);
                return std::unexpected(FORMAT("Invalid UTF-8 sequence at byte offset {}", pos));
            }

            // FR-020: Check for null byte (U+0000)
            if(res.codepoint == U'\0') [[unlikely]] {
                LERROR("Null byte (U+0000) at byte offset {}", pos);
                return std::unexpected(FORMAT("Null byte (U+0000) at byte offset {}", pos));
            }

            // FR-027: Check line length limit
            ++code_point_count;
            if(code_point_count > MAX_CODE_POINTS) [[unlikely]] {
                LERROR("Source line exceeds 10,000 code points");
                return std::unexpected(std::string("Source line exceeds 10,000 code points"));
            }

            // FR-018: Tab expansion
            if(res.codepoint == U'\t') {
                // Formula: next_tab_stop = ((col - 1) / tab_stop_width + 1) * tab_stop_width + 1
                // Note: Integer division truncates toward zero (C++23 standard)
                col = ((col - 1) / tab_stop_width + 1) * tab_stop_width + 1;
            } else {
                // FR-003, FR-004: Each code point = 1 column unit
                col += 1;
            }

            pos += res.byte_length;
        }

        // FR-013: Clamp to line_end + 1 if byte_offset past end
        // (Already handled by returning current col)

        return col;
    }

    // ---------------------------------------------------------------------------
    // marker_extents()
    // ---------------------------------------------------------------------------
    [[nodiscard]] std::expected<std::pair<std::size_t, std::size_t>, std::string> marker_extents(std::string_view line,
                                                                                                 std::size_t start_byte,
                                                                                                 std::size_t end_byte,
                                                                                                 std::size_t tab_stop_width) {
        // Precondition: tab_stop_width > 0 (caller responsibility)

        // Calculate leading spaces (visual column at start_byte, converted to 0-based)
        auto leading_result = visual_column(line, start_byte, tab_stop_width);
        if(!leading_result.has_value()) [[unlikely]] { return std::unexpected(leading_result.error()); }

        // Convert 1-based column to 0-based leading spaces
        const std::size_t leading_spaces = leading_result.value() - 1;

        // Calculate caret count by walking from start_byte to end_byte
        std::size_t caret_count = 0;
        std::size_t pos = start_byte;
        std::size_t code_point_count = 0;

        while(pos < end_byte && pos < line.size()) {
            // Decode UTF-8 sequence at current position
            const auto res = unicode::decode_utf8(line, pos);

            // FR-025, FR-026: Check for invalid UTF-8
            if(res.status != unicode::Utf8Status::Ok) [[unlikely]] {
                LERROR("Invalid UTF-8 sequence at byte offset {}", pos);
                return std::unexpected(FORMAT("Invalid UTF-8 sequence at byte offset {}", pos));
            }

            // FR-020: Check for null byte (U+0000)
            if(res.codepoint == U'\0') [[unlikely]] {
                LERROR("Null byte (U+0000) at byte offset {}", pos);
                return std::unexpected(FORMAT("Null byte (U+0000) at byte offset {}", pos));
            }

            // FR-027: Check line length limit
            ++code_point_count;
            if(code_point_count > MAX_CODE_POINTS) [[unlikely]] {
                LERROR("Source line exceeds 10,000 code points");
                return std::unexpected("Source line exceeds 10,000 code points");
            }

            // FR-010: Tabs contribute expanded width to caret count
            if(res.codepoint == U'\t') {
                // Expand tab to next tab stop
                const std::size_t next_tab_stop = ((caret_count / tab_stop_width) + 1) * tab_stop_width;
                caret_count += (next_tab_stop - caret_count);
            } else {
                // FR-006: Each code point = 1 caret
                caret_count += 1;
            }

            pos += res.byte_length;
        }

        // FR-013: Minimum 1 caret guarantee
        if(caret_count == 0) { caret_count = 1; }

        return std::make_pair(leading_spaces, caret_count);
    }

}  // namespace jsv
// NOLINTEND(*-include-cleaner, *-mt-unsafe, *-easily-swappable-parameters, *-diagnostic-deprecated-declarations)