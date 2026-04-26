// NOLINTBEGIN(*-include-cleaner, *-avoid-magic-numbers)
#include "jsavCore/util/Sha256.hpp"
#include "jsavCore/format.hpp"
#include <bit>

namespace jsv::crypto {

    namespace {

        // SHA-256 Constants (FIPS 180-4, 4.2.2)
        static constexpr std::array<std::uint32_t, 64> K = {
            0x428a2f98, 0x71374491, 0xb5c0fbcf, 0xe9b5dba5, 0x3956c25b, 0x59f111f1, 0x923f82a4, 0xab1c5ed5, 0xd807aa98, 0x12835b01,
            0x243185be, 0x550c7dc3, 0x72be5d74, 0x80deb1fe, 0x9bdc06a7, 0xc19bf174, 0xe49b69c1, 0xefbe4786, 0x0fc19dc6, 0x240ca1cc,
            0x2de92c6f, 0x4a7484aa, 0x5cb0a9dc, 0x76f988da, 0x983e5152, 0xa831c66d, 0xb00327c8, 0xbf597fc7, 0xc6e00bf3, 0xd5a79147,
            0x06ca6351, 0x14292967, 0x27b70a85, 0x2e1b2138, 0x4d2c6dfc, 0x53380d13, 0x650a7354, 0x766a0abb, 0x81c2c92e, 0x92722c85,
            0xa2bfe8a1, 0xa81a664b, 0xc24b8b70, 0xc76c51a3, 0xd192e819, 0xd6990624, 0xf40e3585, 0x106aa070, 0x19a4c116, 0x1e376c08,
            0x2748774c, 0x34b0bcb5, 0x391c0cb3, 0x4ed8aa4a, 0x5b9cca4f, 0x682e6ff3, 0x748f82ee, 0x78a5636f, 0x84c87814, 0x8cc70208,
            0x90befffa, 0xa4506ceb, 0xbef9a3f7, 0xc67178f2};

        static constexpr std::array<std::uint32_t, 8> H_INIT = {0x6a09e667, 0xbb67ae85, 0x3c6ef372, 0xa54ff53a,
                                                               0x510e527f, 0x9b05688c, 0x1f83d9ab, 0x5be0cd19};

        [[nodiscard]] constexpr std::uint32_t ch(std::uint32_t x, std::uint32_t y, std::uint32_t z) noexcept { return (x & y) ^ (~x & z); }
        [[nodiscard]] constexpr std::uint32_t maj(std::uint32_t x, std::uint32_t y, std::uint32_t z) noexcept {
            return (x & y) ^ (x & z) ^ (y & z);
        }
        [[nodiscard]] constexpr std::uint32_t BigSigma0(std::uint32_t x) noexcept {
            return std::rotr(x, 2) ^ std::rotr(x, 13) ^ std::rotr(x, 22);
        }
        [[nodiscard]] constexpr std::uint32_t BigSigma1(std::uint32_t x) noexcept {
            return std::rotr(x, 6) ^ std::rotr(x, 11) ^ std::rotr(x, 25);
        }
        [[nodiscard]] constexpr std::uint32_t smallSigma0(std::uint32_t x) noexcept {
            return std::rotr(x, 7) ^ std::rotr(x, 18) ^ (x >> 3);
        }
        [[nodiscard]] constexpr std::uint32_t smallSigma1(std::uint32_t x) noexcept {
            return std::rotr(x, 17) ^ std::rotr(x, 19) ^ (x >> 10);
        }

    }  // namespace

    Sha256::Sha256() noexcept : state_(H_INIT), count_(0), bufferIdx_(0) { buffer_.fill(std::byte{0}); }

    void Sha256::update(std::span<const std::byte> data) noexcept {
        for(const std::byte byte : data) {
            buffer_.at(bufferIdx_) = byte;
            bufferIdx_++;
            count_ += 8;
            if(bufferIdx_ == BlockSize) {
                transform(buffer_);
                bufferIdx_ = 0;
            }
        }
    }

    void Sha256::transform(std::span<const std::byte, BlockSize> block) noexcept {
        std::array<std::uint32_t, 64> w{};

        for(std::size_t t = 0; t < 16; ++t) {
            auto t_4 = t * 4;
            w.at(t) = (C_UI32T(block[t_4 + 0]) << 24) | (C_UI32T(block[t_4 + 1]) << 16) | (C_UI32T(block[t_4 + 2]) << 8) |
                      (C_UI32T(block[t_4 + 3]));
        }
        for(std::size_t t = 16; t < 64; ++t) {
            w.at(t) = smallSigma1(w.at(t - 2)) + w.at(t - 7) + smallSigma0(w.at(t - 15)) + w.at(t - 16);
        }

        std::uint32_t a = state_.at(0);
        std::uint32_t b = state_.at(1);
        std::uint32_t c = state_.at(2);
        std::uint32_t d = state_.at(3);
        std::uint32_t e = state_.at(4);
        std::uint32_t f = state_.at(5);
        std::uint32_t g = state_.at(6);
        std::uint32_t h = state_.at(7);

        for(std::size_t t = 0; t < 64; ++t) {
            const std::uint32_t t1 = h + BigSigma1(e) + ch(e, f, g) + K.at(t) + w.at(t);
            const std::uint32_t t2 = BigSigma0(a) + maj(a, b, c);
            h = g;
            g = f;
            f = e;
            e = d + t1;
            d = c;
            c = b;
            b = a;
            a = t1 + t2;
        }

        state_.at(0) += a;
        state_.at(1) += b;
        state_.at(2) += c;
        state_.at(3) += d;
        state_.at(4) += e;
        state_.at(5) += f;
        state_.at(6) += g;
        state_.at(7) += h;
    }

    Sha256::Digest Sha256::finalize() noexcept {
        const std::uint64_t bitCount = count_;

        // Append padding bit 1 (0x80)
        const std::array<std::byte, 1> pad = {std::byte{0x80}};
        update(pad);


        while(bufferIdx_ != 56) {
            const std::array<std::byte, 1> zero = {std::byte{0x00}};
            update(zero);
        }

        // Append original message length in bits as a 64-bit big-endian integer
        for(int i = 7; i >= 0; --i) {
            const std::array<std::byte, 1> lengthByte = {C_B((bitCount >> (C_UI64T(i) * 8)) & 0xFF)};
            update(lengthByte);
        }

        Digest digest{};
        for(std::size_t i = 0; i < 8; ++i) {
            auto i_4 = i * 4;
            auto current = state_.at(i);
            digest.at(i_4 + 0) = C_B((current >> 24) & 0xFF);
            digest.at(i_4 + 1) = C_B((current >> 16) & 0xFF);
            digest.at(i_4 + 2) = C_B((current >> 8) & 0xFF);
            digest.at(i_4 + 3) = C_B(current & 0xFF);
        }

        *this = Sha256();
        return digest;
    }

    Sha256::Digest Sha256::hash(std::span<const std::byte> data) noexcept {
        Sha256 hasher;
        hasher.update(data);
        return hasher.finalize();
    }

    std::string Sha256::toHexString(const Digest &digest) {
        std::string res;
        res.reserve(DigestSize * 2);
        for(const auto b : digest) { res += FORMAT("{:02x}", std::to_integer<std::uint8_t>(b)); }
        return res;
    }

}  // namespace jsv::crypto
// NOLINTEND(*-include-cleaner, *-avoid-magic-numbers)
