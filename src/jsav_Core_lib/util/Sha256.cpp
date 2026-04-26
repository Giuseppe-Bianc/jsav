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

        [[nodiscard]] constexpr std::uint32_t rotr(std::uint32_t value, std::uint32_t shift) noexcept {
            return (value >> shift) | (value << (32U - shift));
        }

        // Load big-endian uint32_t from byte array
        // Optimized for x86-64: compile-time branch eliminated, single byteswap on LE
        [[nodiscard]] std::uint32_t load_be32(const std::uint8_t *p) noexcept {
            std::uint32_t word;
            std::memcpy(&word, p, sizeof(word));
            if constexpr(std::endian::native == std::endian::little) {
                return std::byteswap(word);  // C++23: single instruction on x86-64 (bswap)
            }
            return word;
        }

        // Store big-endian uint32_t to byte array
        // Optimized for x86-64: byteswap before store on LE, zero-cost on BE
        void store_be32(std::byte *p, std::uint32_t value) noexcept {
            if constexpr(std::endian::native == std::endian::little) { value = std::byteswap(value); }
            std::memcpy(p, &value, sizeof(value));
        }

        [[nodiscard]] std::vector<std::uint8_t> pad_message(std::span<const std::byte> bytes) {
            std::vector<std::uint8_t> padded;
            padded.reserve(bytes.size() + 72U);

            std::ranges::transform(bytes, std::back_inserter(padded), [](std::byte v) { return std::to_integer<std::uint8_t>(v); });

            padded.push_back(0x80U);
            while((padded.size() % 64U) != 56U) { padded.push_back(0U); }

            const std::uint64_t bit_length = static_cast<std::uint64_t>(bytes.size()) * 8U;
            for(int shift = 56; shift >= 0; shift -= 8) { padded.push_back(static_cast<std::uint8_t>((bit_length >> shift) & 0xFFU)); }
            return padded;
        }
    }  // namespace

    std::string Sha256Digest::hex() const {
        static constexpr std::array<char, 16> digits{'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'a', 'b', 'c', 'd', 'e', 'f'};
        std::string output;
        output.reserve(bytes.size() * 2U);
        for(std::byte byte : bytes) {
            const auto val = std::to_integer<std::uint8_t>(byte);
            output.push_back(digits[(val >> 4U) & 0x0FU]);
            output.push_back(digits[val & 0x0FU]);
        }
        return output;
    }

    Sha256Digest sha256(std::span<const std::byte> input) {
        std::array<std::uint32_t, 8> hash = initial_hash;
        const std::vector<std::uint8_t> padded = pad_message(input);

        for(std::size_t offset = 0; offset < padded.size(); offset += 64U) {
            std::array<std::uint32_t, 64> schedule{};

            // Load first 16 words with big-endian interpretation (optimized)
            for(std::size_t i = 0; i < 16U; ++i) { schedule[i] = load_be32(&padded[offset + i * 4U]); }

            // Extend message schedule
            for(std::size_t i = 16U; i < 64U; ++i) {
                const auto s0 = rotr(schedule[i - 15], 7U) ^ rotr(schedule[i - 15], 18U) ^ (schedule[i - 15] >> 3U);
                const auto s1 = rotr(schedule[i - 2], 17U) ^ rotr(schedule[i - 2], 19U) ^ (schedule[i - 2] >> 10U);
                schedule[i] = schedule[i - 16U] + s0 + schedule[i - 7U] + s1;
            }

            // Initialize working variables
            auto [a, b, c, d, e, f, g, h] = std::tie(hash[0], hash[1], hash[2], hash[3], hash[4], hash[5], hash[6], hash[7]);

            // Compression function
            for(std::size_t i = 0; i < 64U; ++i) {
                const std::uint32_t Sigma1 = rotr(e, 6U) ^ rotr(e, 11U) ^ rotr(e, 25U);
                const std::uint32_t Ch = (e & f) ^ (~e & g);
                const std::uint32_t temp1 = h + Sigma1 + Ch + round_constants[i] + schedule[i];
                const std::uint32_t Sigma0 = rotr(a, 2U) ^ rotr(a, 13U) ^ rotr(a, 22U);
                const std::uint32_t Maj = (a & b) ^ (a & c) ^ (b & c);
                const std::uint32_t temp2 = Sigma0 + Maj;

                h = g;
                g = f;
                f = e;
                e = d + temp1;
                d = c;
                c = b;
                b = a;
                a = temp1 + temp2;
            }

            // Update hash state
            hash[0] += a;
            hash[1] += b;
            hash[2] += c;
            hash[3] += d;
            hash[4] += e;
            hash[5] += f;
            hash[6] += g;
            hash[7] += h;
        }

        // Serialize final hash in big-endian byte order (optimized)
        Sha256Digest digest{};
        for(std::size_t i = 0; i < 8U; ++i) { store_be32(&digest.bytes[i * 4U], hash[i]); }
        return digest;
    }

    Sha256Digest sha256(std::string_view text) {
        auto *data = reinterpret_cast<const std::byte *>(text.data());
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