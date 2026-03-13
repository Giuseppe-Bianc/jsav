// NOLINTBEGIN(*-include-cleaner, *-avoid-magic-numbers, *-magic-numbers)
#include "jsav/location/LineTracker.hpp"

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
        const auto newline_count = C_ST(std::ranges::count(source_, '\n'));
        lines_.reserve(newline_count + 1);

        // Single forward scan: find each '\n', strip optional '\r', push view.
        for(std::size_t pos = 0; pos <= source_.size();) {
            const std::size_t newline = source_.find('\n', pos);
            const std::size_t end = (newline != std::string_view::npos) ? newline : source_.size();

            // Strip a trailing '\r' so Windows CRLF files work transparently.
            const std::size_t line_end = (end > pos && source_[end - 1] == '\r') ? end - 1 : end;

            lines_.emplace_back(source_.data() + pos, line_end - pos);

            if(newline == std::string_view::npos) { break; }
            pos = newline + 1;
        }
    }

    // -------------------------------------------------------------------------
    // Public interface
    // -------------------------------------------------------------------------

    std::string_view LineTracker::get_line(std::size_t line_number) const noexcept {
        // line_number is 1-based; guard against 0 and out-of-range.
        if(line_number == 0 || line_number > lines_.size()) { return {}; }
        return lines_[line_number - 1];
    }

    std::size_t LineTracker::line_count() const noexcept { return lines_.size(); }
    bool LineTracker::empty() const noexcept { return lines_.empty(); }

}  // namespace jsv
// NOLINTEND(*-include-cleaner, *-avoid-magic-numbers, *-magic-numbers)