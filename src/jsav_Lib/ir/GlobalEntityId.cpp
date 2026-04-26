/*
 * Created by gbian on 25/04/2026.
 * Copyright (c) 2026 All rights reserved.
 */
// NOLINTBEGIN(*-include-cleaner)
#include "jsav/ir/GlobalEntityId.hpp"

namespace jsv {

    GlobalEntityId::GlobalEntityId(std::array<std::uint8_t, 32> hash_bytes) noexcept
        : hash_(hash_bytes) {
        // Reconstruct canonical path from hash (simplified)
        canonical_ = "id_from_hash";
    }

    GlobalEntityId GlobalEntityId::from_canonical_path(
        std::string_view module_name,
        std::string_view function_name,
        std::uint64_t block_index,
        std::uint64_t instruction_index,
        std::string_view entity_type) noexcept {

        // Build canonical string
        std::string canonical = fmt::format("{}:{}:{}:{}:{}",
            module_name,
            function_name.empty() ? "_" : function_name,
            block_index,
            instruction_index,
            entity_type);

        // Placeholder: Create hash (in production, use SHA-256)
        std::array<std::uint8_t, 32> hash{};
        std::fill(hash.begin(), hash.end(), 0);

        return GlobalEntityId(hash, canonical);
    }

    GlobalEntityId::GlobalEntityId(std::array<std::uint8_t, 32> hash, std::string canonical) noexcept
        : hash_(hash), canonical_(std::move(canonical)) {
    }

    std::string GlobalEntityId::canonical_path() const {
        return canonical_;
    }

    bool GlobalEntityId::operator==(const GlobalEntityId& other) const noexcept {
        return hash_ == other.hash_;
    }

    bool GlobalEntityId::operator!=(const GlobalEntityId& other) const noexcept {
        return !(*this == other);
    }

    bool GlobalEntityId::operator<(const GlobalEntityId& other) const noexcept {
        return hash_ < other.hash_;
    }

    bool GlobalEntityId::operator<=(const GlobalEntityId& other) const noexcept {
        return hash_ <= other.hash_;
    }

    bool GlobalEntityId::operator>(const GlobalEntityId& other) const noexcept {
        return hash_ > other.hash_;
    }

    bool GlobalEntityId::operator>=(const GlobalEntityId& other) const noexcept {
        return hash_ >= other.hash_;
    }

    std::uint64_t GlobalEntityId::hash_value() const noexcept {
        // Extract first 8 bytes as hash value
        std::uint64_t result = 0;
        for (int i = 0; i < 8 && i < 32; ++i) {
            result = (result << 8) | hash_[i];
        }
        return result;
    }

}  // namespace jsv
// NOLINTEND(*-include-cleaner)