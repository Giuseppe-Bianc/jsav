/*
 * Created by gbian on 12/03/2026.
 * Copyright (c) 2026 All rights reserved.
 */
// NOLINTBEGIN(*-include-cleaner)
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
        lines_.clear();
        // Reserve a rough estimate to avoid repeated reallocations.
        // Average line length heuristic: 40 chars.
        if(!source_.empty()) { lines_.reserve(source_.size() / 40 + 1); }

        std::size_t pos = 0;
        while(pos <= source_.size()) {
            // Find the next newline (or end-of-string).
            const std::size_t newline = source_.find('\n', pos);
            const std::size_t end = (newline == std::string_view::npos) ? source_.size() : newline;

            // Strip a trailing '\r' so Windows CRLF files work transparently.
            std::size_t line_end = end;
            if(line_end > pos && source_[line_end - 1] == '\r') { --line_end; }

            // Push a view directly into the caller-owned buffer — zero copy.
            lines_.push_back(source_.substr(pos, line_end - pos));

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
// NOLINTEND(*-include-cleaner)