/*
 * Created by Codex on 26/04/2026.
 */

#pragma once

#include "../headersCore.hpp"

namespace jsv::crypto {

    struct Sha256Digest {
        std::array<std::byte, 32> bytes{};

        [[nodiscard]] std::string hex() const;
        [[nodiscard]] bool operator==(const Sha256Digest &other) const noexcept = default;
    };

    [[nodiscard]] Sha256Digest sha256(std::span<const std::byte> bytes);
    [[nodiscard]] Sha256Digest sha256(std::string_view text);
    [[nodiscard]] std::string sha256_hex(std::span<const std::byte> bytes);
    [[nodiscard]] std::string sha256_hex(std::string_view text);

}  // namespace jsv::crypto
