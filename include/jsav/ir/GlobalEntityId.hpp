/*
 * Created by gbian on 26/04/2026.
 * Copyright (c) 2026 All rights reserved.
 */
// NOLINTBEGIN(*-include-cleaner)
#pragma once

#include "../ir/IrCommon.hpp"

namespace jsv {

    /// Immutable global entity ID derived from canonical structural path
    /// Format: module/function/block/index_type
    class GlobalEntityId {
    public:
        /// Create ID from canonical structural path components
        /// Deterministic hash derived from canonical string representation
        static GlobalEntityId from_canonical_path(
            std::string_view module_name,
            std::string_view function_name = "",
            std::uint64_t block_index = 0,
            std::uint64_t instruction_index = 0,
            std::string_view entity_type = "module"
        ) noexcept;

        /// Create ID from pre-computed hash (for deserialization)
        explicit GlobalEntityId(std::array<std::uint8_t, 32> hash_bytes) noexcept;

        /// Get the canonical string representation
        std::string canonical_path() const;

        /// Get the deterministic hash bytes (SHA-256)
        const std::array<std::uint8_t, 32>& hash_bytes() const noexcept {
            return hash_;
        }

        /// Immutable - no assignment or modification
        GlobalEntityId(const GlobalEntityId&) = default;
        GlobalEntityId& operator=(const GlobalEntityId&) = default;
        GlobalEntityId(GlobalEntityId&&) noexcept = default;
        GlobalEntityId& operator=(GlobalEntityId&&) noexcept = default;

        /// Equality based on hash
        bool operator==(const GlobalEntityId& other) const noexcept;
        bool operator!=(const GlobalEntityId& other) const noexcept;

        /// Ordering for deterministic output (based on hash)
        bool operator<(const GlobalEntityId& other) const noexcept;
        bool operator<=(const GlobalEntityId& other) const noexcept;
        bool operator>(const GlobalEntityId& other) const noexcept;
        bool operator>=(const GlobalEntityId& other) const noexcept;

        /// Hash value for use in maps/sets
        std::uint64_t hash_value() const noexcept;

    private:
        std::array<std::uint8_t, 32> hash_;
        std::string canonical_;

        GlobalEntityId(std::array<std::uint8_t, 32> hash, std::string canonical) noexcept;
    };

}  // namespace jsv

// NOLINTEND(*-include-cleaner)