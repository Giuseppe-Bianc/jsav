/*
 * Created by gbian on 25/04/2026.
 * Copyright (c) 2026 All rights reserved.
 */
// NOLINTBEGIN(*-include-cleaner)
#include "jsav/ir/GlobalEntityId.hpp"

namespace jsv {

    namespace {
        [[nodiscard]] std::uint64_t fnv1a64(const std::string_view text, const std::uint64_t seed) noexcept {
            constexpr std::uint64_t prime = 1099511628211ULL;
            std::uint64_t hash = seed;
            for(const char ch : text) {
                hash ^= static_cast<std::uint8_t>(ch);
                hash *= prime;
            }
            return hash;
        }
    }  // namespace

    GlobalEntityId GlobalEntityId::from_canonical_path(const std::string_view canonical_path) noexcept {
        constexpr std::uint64_t offset_a = 14695981039346656037ULL;
        constexpr std::uint64_t offset_b = 10995116282110000019ULL;

        const std::uint64_t hash_a = fnv1a64(canonical_path, offset_a);
        const std::uint64_t hash_b = fnv1a64(canonical_path, offset_b);

        std::array<std::uint8_t, 16> bytes{};
        for(std::size_t idx = 0; idx < 8; ++idx) {
            const auto shift = static_cast<unsigned int>(idx * 8);
            bytes[idx] = static_cast<std::uint8_t>((hash_a >> shift) & 0xFFU);
            bytes[idx + 8] = static_cast<std::uint8_t>((hash_b >> shift) & 0xFFU);
        }

        return GlobalEntityId(bytes);
    }

    GlobalEntityId GlobalEntityId::from_canonical_key(const CanonicalKey &canonical_key) noexcept {
        return from_canonical_path(canonical_key.to_string());
    }

    std::string GlobalEntityId::to_hex() const {
        std::string out;
        out.reserve(32);
        for(const auto byte : bytes_) { out += FORMAT("{:02x}", byte); }
        return out;
    }

}  // namespace jsv
// NOLINTEND(*-include-cleaner)
