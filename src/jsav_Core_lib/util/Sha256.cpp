// NOLINTBEGIN(*-include-cleaner, *-avoid-magic-numbers)
#include "jsavCore/util/Sha256.hpp"

namespace jsv::crypto {
    namespace {
        constexpr std::array<std::uint32_t, 8> initial_hash{
            0x6A09E667U,
            0xBB67AE85U,
            0x3C6EF372U,
            0xA54FF53AU,
            0x510E527FU,
            0x9B05688CU,
            0x1F83D9ABU,
            0x5BE0CD19U,
        };

        constexpr std::array<std::uint32_t, 64> round_constants{
            0x428A2F98U, 0x71374491U, 0xB5C0FBCFU, 0xE9B5DBA5U, 0x3956C25BU, 0x59F111F1U, 0x923F82A4U, 0xAB1C5ED5U,
            0xD807AA98U, 0x12835B01U, 0x243185BEU, 0x550C7DC3U, 0x72BE5D74U, 0x80DEB1FEU, 0x9BDC06A7U, 0xC19BF174U,
            0xE49B69C1U, 0xEFBE4786U, 0x0FC19DC6U, 0x240CA1CCU, 0x2DE92C6FU, 0x4A7484AAU, 0x5CB0A9DCU, 0x76F988DAU,
            0x983E5152U, 0xA831C66DU, 0xB00327C8U, 0xBF597FC7U, 0xC6E00BF3U, 0xD5A79147U, 0x06CA6351U, 0x14292967U,
            0x27B70A85U, 0x2E1B2138U, 0x4D2C6DFCU, 0x53380D13U, 0x650A7354U, 0x766A0ABBU, 0x81C2C92EU, 0x92722C85U,
            0xA2BFE8A1U, 0xA81A664BU, 0xC24B8B70U, 0xC76C51A3U, 0xD192E819U, 0xD6990624U, 0xF40E3585U, 0x106AA070U,
            0x19A4C116U, 0x1E376C08U, 0x2748774CU, 0x34B0BCB5U, 0x391C0CB3U, 0x4ED8AA4AU, 0x5B9CCA4FU, 0x682E6FF3U,
            0x748F82EEU, 0x78A5636FU, 0x84C87814U, 0x8CC70208U, 0x90BEFFFAU, 0xA4506CEBU, 0xBEF9A3F7U, 0xC67178F2U,
        };

        [[nodiscard]] constexpr std::uint32_t rotr(const std::uint32_t value, const std::uint32_t shift) noexcept {
            return (value >> shift) | (value << (32U - shift));
        }

        [[nodiscard]] std::vector<std::uint8_t> pad_message(std::span<const std::byte> bytes) {
            std::vector<std::uint8_t> padded;
            padded.reserve(bytes.size() + 72U);

            std::ranges::transform(bytes, std::back_inserter(padded),
                                   [](const std::byte value) { return std::to_integer<std::uint8_t>(value); });

            padded.push_back(0x80U);
            while((padded.size() % 64U) != 56U) {
                padded.push_back(0U);
            }

            const std::uint64_t bit_length = static_cast<std::uint64_t>(bytes.size()) * 8U;
            for(int shift = 56; shift >= 0; shift -= 8) {
                padded.push_back(static_cast<std::uint8_t>((bit_length >> shift) & 0xFFU));
            }

            return padded;
        }
    }  // namespace

    std::string Sha256Digest::hex() const {
        static constexpr std::array<char, 16> digits{'0', '1', '2', '3', '4', '5', '6', '7',
                                                     '8', '9', 'a', 'b', 'c', 'd', 'e', 'f'};

        std::string output;
        output.reserve(bytes.size() * 2U);
        for(const std::byte byte : bytes) {
            const auto value = std::to_integer<std::uint8_t>(byte);
            output.push_back(digits[(value >> 4U) & 0x0FU]);
            output.push_back(digits[value & 0x0FU]);
        }
        return output;
    }

    Sha256Digest sha256(std::span<const std::byte> bytes) {
        std::array<std::uint32_t, 8> hash = initial_hash;
        const std::vector<std::uint8_t> padded = pad_message(bytes);

        for(std::size_t offset = 0; offset < padded.size(); offset += 64U) {
            std::array<std::uint32_t, 64> schedule{};
            for(std::size_t index = 0; index < 16U; ++index) {
                const std::size_t base = offset + (index * 4U);
                schedule[index] = (C_UI32T(padded[base]) << 24U) |
                                  (C_UI32T(padded[base + 1U]) << 16U) |
                                  (C_UI32T(padded[base + 2U]) << 8U) |
                                  C_UI32T(padded[base + 3U]);
            }

            for(std::size_t index = 16U; index < schedule.size(); ++index) {
                const auto index_15 = index - 15U;
                const auto index_2 = index - 2U;
                const std::uint32_t s0 = rotr(schedule[index_15], 7U) ^ rotr(schedule[index_15], 18U) ^
                                         (schedule[index_15] >> 3U);
                const std::uint32_t s1 = rotr(schedule[index_2], 17U) ^ rotr(schedule[index_2], 19U) ^
                                         (schedule[index_2] >> 10U);
                schedule[index] = schedule[index - 16U] + s0 + schedule[index - 7U] + s1;
            }

            std::uint32_t a = hash[0];
            std::uint32_t b = hash[1];
            std::uint32_t c = hash[2];
            std::uint32_t d = hash[3];
            std::uint32_t e = hash[4];
            std::uint32_t f = hash[5];
            std::uint32_t g = hash[6];
            std::uint32_t h = hash[7];

            for(std::size_t index = 0; index < schedule.size(); ++index) {
                const std::uint32_t sigma1 = rotr(e, 6U) ^ rotr(e, 11U) ^ rotr(e, 25U);
                const std::uint32_t choice = (e & f) ^ (~e & g);
                const std::uint32_t temp1 = h + sigma1 + choice + round_constants[index] + schedule[index];
                const std::uint32_t sigma0 = rotr(a, 2U) ^ rotr(a, 13U) ^ rotr(a, 22U);
                const std::uint32_t majority = (a & b) ^ (a & c) ^ (b & c);
                const std::uint32_t temp2 = sigma0 + majority;

                h = g;
                g = f;
                f = e;
                e = d + temp1;
                d = c;
                c = b;
                b = a;
                a = temp1 + temp2;
            }

            hash[0] += a;
            hash[1] += b;
            hash[2] += c;
            hash[3] += d;
            hash[4] += e;
            hash[5] += f;
            hash[6] += g;
            hash[7] += h;
        }

        Sha256Digest digest{};
        for(std::size_t index = 0; index < hash.size(); ++index) {
            const std::uint32_t value = hash[index];
            const auto byte_index = index * 4U;
            digest.bytes[byte_index] = C_B((value >> 24U) & 0xFFU);
            digest.bytes[byte_index + 1U] = C_B((value >> 16U) & 0xFFU);
            digest.bytes[byte_index + 2U] = C_B((value >> 8U) & 0xFFU);
            digest.bytes[byte_index + 3U] = C_B(value & 0xFFU);
        }
        return digest;
    }

    Sha256Digest sha256(const std::string_view text) {
        const auto *data = reinterpret_cast<const std::byte *>(text.data());
        return sha256(std::span<const std::byte>{data, text.size()});
    }

    std::string sha256_hex(const std::span<const std::byte> bytes) {
        return sha256(bytes).hex();
    }

    std::string sha256_hex(const std::string_view text) {
        return sha256(text).hex();
    }

}  // namespace jsv::crypto
// NOLINTEND(*-include-cleaner, *-avoid-magic-numbers)
