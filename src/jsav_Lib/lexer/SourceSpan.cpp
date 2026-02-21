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

    SourceSpan::SourceSpan() noexcept : file_path{std::make_shared<const std::string>("")} {}

    // NOLINTBEGIN(*-easily-swappable-parameters)
    // Start/end parameter ordering is a well-established convention; names are semantically distinct
    SourceSpan::SourceSpan(std::shared_ptr<const std::string> p_file_path, const SourceLocation &p_start, const SourceLocation &p_end) noexcept
      : file_path{std::move(p_file_path)}, start{p_start}, end{p_end} {}
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

    // -------------------------------------------------------------------------
    // truncate_path
    // -------------------------------------------------------------------------

    std::string truncate_path(const std::filesystem::path &path, std::size_t depth) {
        std::vector<std::filesystem::path> components;
        for(const auto &part : path) {
            if(!part.empty()) { components.push_back(part); }
        }

        const std::size_t len = components.size();
        std::filesystem::path result;

        if(len <= depth) {
            for(const auto &c : components) { result /= c; }
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