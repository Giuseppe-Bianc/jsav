/*
 * Created by gbian on 20/02/2026.
 * Copyright (c) 2026 All rights reserved.
 */
// NOLINTBEGIN(*-include-cleaner, *-identifier-length)
#include "jsav/lexer/SourceSpan.hpp"

namespace jsv {

    // -------------------------------------------------------------------------
    // Constructors
    // -------------------------------------------------------------------------

    SourceSpan::SourceSpan() : file_path(empty_path()) {}

    // NOLINTBEGIN(*-easily-swappable-parameters)
    SourceSpan::SourceSpan(std::shared_ptr<const std::string> p_file_path, const SourceLocation &p_start,
                           const SourceLocation &p_end) noexcept
      : file_path{vnd_move(p_file_path)}, start{p_start}, end{p_end} {}
    // NOLINTEND(*-easily-swappable-parameters)

    // -------------------------------------------------------------------------
    // Mutation
    // -------------------------------------------------------------------------

    void SourceSpan::merge(const SourceSpan &other) noexcept {
        if(*file_path == *other.file_path) {
            if(other.start < start) { start = other.start; }
            if(other.end > end) { end = other.end; }
        }
    }

    // -------------------------------------------------------------------------
    // Immutable merge
    // -------------------------------------------------------------------------

    std::optional<SourceSpan> SourceSpan::merged(const SourceSpan &other) const {
        if(*file_path != *other.file_path) { return std::nullopt; }
        return SourceSpan{file_path, (start < other.start) ? start : other.start, (end > other.end) ? end : other.end};
    }

    // -------------------------------------------------------------------------
    // Ordering
    // -------------------------------------------------------------------------

    std::strong_ordering SourceSpan::operator<=>(const SourceSpan &other) const noexcept {
        if(auto cmp = *file_path <=> *other.file_path; cmp != 0) { return cmp; }
        if(auto cmp = start <=> other.start; cmp != 0) { return cmp; }
        return end <=> other.end;
    }

    bool SourceSpan::operator==(const SourceSpan &other) const noexcept { return (*this <=> other) == std::strong_ordering::equal; }

    // -------------------------------------------------------------------------
    // Formatting — single source of truth used by all formatters
    // -------------------------------------------------------------------------

    std::string SourceSpan::to_string() const {
        const auto truncated = truncate_path(std::filesystem::path{*file_path}, 2);
        return fmt::format("{}:line {}:column {} - line {}:column {}", truncated, start.line, start.column, end.line, end.column);
    }

    std::ostream &operator<<(std::ostream &os, const SourceSpan &span) { return os << span.to_string(); }

    auto SourceSpan::empty_path() -> std::shared_ptr<const std::string> {
        static const auto empty = MAKE_SHARED(const std::string, "");
        return empty;
    }

    // -------------------------------------------------------------------------
    // truncate_path
    // -------------------------------------------------------------------------

    std::string truncate_path(const fs::path &path, std::size_t depth) {
        std::vector<fs::path> components;
        std::ranges::copy_if(path, std::back_inserter(components), [](const auto &part) { return !part.empty(); });

        const std::size_t len = components.size();
        fs::path result;

        if(len <= depth) {
            result = std::ranges::fold_left(components, fs::path{}, std::divides{});
        } else {
            result = "..";
            for(std::size_t i = len - depth; i < len; ++i) { result /= components[i]; }
        }

        return result.string();
    }

}  // namespace jsv

// -------------------------------------------------------------------------
// std::hash
// -------------------------------------------------------------------------

namespace std {

    std::size_t hash<jsv::SourceSpan>::operator()(const jsv::SourceSpan &s) const noexcept {
        std::size_t seed = 0;
        hash_combine(seed, std::hash<std::string>{}(*s.file_path));
        hash_combine(seed, std::hash<jsv::SourceLocation>{}(s.start));
        hash_combine(seed, std::hash<jsv::SourceLocation>{}(s.end));
        return seed;
    }

}  // namespace std
// NOLINTEND(*-include-cleaner, *-identifier-length)