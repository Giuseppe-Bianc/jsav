// NOLINTBEGIN(*-include-cleaner, *-avoid-magic-numbers, *-magic-numbers)
#include "jsav/location/LineTracker.hpp"
#include "jsav/lexer/unicode/UnicodeData.hpp"
#include "jsav/lexer/unicode/Utf8.hpp"

namespace jsv {

    // -------------------------------------------------------------------------
    // Construction
    // -------------------------------------------------------------------------

    LineTracker::LineTracker(std::string_view source) : source_(source) { index_lines(); }

    // -------------------------------------------------------------------------
    // Private helpers
    // -------------------------------------------------------------------------

    void LineTracker::index_lines() {
        if(source_.empty()) { return; }
        lines_.clear();

        // '\n' count is a lower bound; Unicode terminators (NEL, LS, PS) may add more lines.
        lines_.reserve(C_ST(std::ranges::count(source_, '\n')) + 1);

        for(std::size_t pos = 0; pos <= source_.size();) {
            // Scan forward, decoding one codepoint at a time, until a line terminator is found.
            std::size_t scan = pos;
            jsv::unicode::Utf8DecodeResult term{};
            bool found = false;

            while(scan < source_.size()) {
                const auto res = jsv::unicode::decode_utf8(source_, scan);
                if(res.codepoint == U'\n' || jsv::unicode::is_unicode_line_terminator(res.codepoint)) {
                    term = res;
                    found = true;
                    break;
                }
                scan += res.byte_length;
            }

            if(!found) {
                // No terminator remains — push the final (possibly empty) line and stop.
                lines_.emplace_back(source_.substr(pos));
                break;
            }

            // Strip a trailing '\r' for CRLF; only applicable immediately before '\n'.
            const std::size_t line_end = (term.codepoint == U'\n' && scan > pos && source_[scan - 1] == '\r') ? scan - 1 : scan;

            lines_.emplace_back(source_.substr(pos, line_end - pos));
            pos = scan + term.byte_length;
        }
    }

    // -------------------------------------------------------------------------
    // Public interface
    // -------------------------------------------------------------------------

    std::string_view LineTracker::get_line(std::size_t line_number) const noexcept {
        if(line_number == 0 || line_number > lines_.size()) { return {}; }
        return lines_[line_number - 1];
    }

    std::size_t LineTracker::line_count() const noexcept { return lines_.size(); }
    bool LineTracker::empty() const noexcept { return lines_.empty(); }

}  // namespace jsv
// NOLINTEND(*-include-cleaner, *-avoid-magic-numbers, *-magic-numbers)