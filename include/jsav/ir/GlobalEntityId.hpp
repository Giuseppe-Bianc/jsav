/*
 * Created by gbian on 26/04/2026.
 * Copyright (c) 2026 All rights reserved.
 */
// NOLINTBEGIN(*-include-cleaner)
#pragma once

#include "../ir/IrCommon.hpp"

namespace jsv {

    class GlobalEntityId {
    public:
        constexpr GlobalEntityId() noexcept = default;
        explicit constexpr GlobalEntityId(const std::array<std::uint8_t, 16> bytes) noexcept : bytes_(bytes) {}

        [[nodiscard]] static GlobalEntityId from_canonical_path(std::string_view canonical_path) noexcept;
        [[nodiscard]] static GlobalEntityId from_canonical_key(const CanonicalKey &canonical_key) noexcept;

        [[nodiscard]] constexpr const std::array<std::uint8_t, 16> &bytes() const noexcept { return bytes_; }

        [[nodiscard]] std::string to_hex() const;

        [[nodiscard]] static consteval std::uint64_t compile_time_hash(const std::string_view text) noexcept {
            constexpr std::uint64_t offset = 14695981039346656037ULL;
            constexpr std::uint64_t prime = 1099511628211ULL;

            std::uint64_t hash = offset;
            for(const char ch : text) {
                hash ^= static_cast<std::uint8_t>(ch);
                hash *= prime;
            }
            return hash;
        }

        [[nodiscard]] constexpr std::strong_ordering operator<=>(const GlobalEntityId &other) const noexcept = default;
        [[nodiscard]] constexpr bool operator==(const GlobalEntityId &other) const noexcept = default;

    private:
        std::array<std::uint8_t, 16> bytes_{};
    };

}  // namespace jsv

// NOLINTEND(*-include-cleaner)
