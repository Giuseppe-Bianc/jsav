// NOLINTBEGIN(*-include-cleaner, *-avoid-magic-numbers)
#include "jsavCore/util/Sha256.hpp"
#include "jsavCore/format.hpp"

namespace jsv::crypto {

    SHA256::SHA256() noexcept { reset(); }

    void SHA256::reset() noexcept {
        // Load the eight initial hash words H^(0)  (FIPS 180-4 §5.3.3).
        state_ = H0;
        buf_len_ = 0u;
        bit_count_ = 0u;
        // Zero the buffer using std::ranges::fill with the typed zero value.
        // std::byte{0} is an explicit, arithmetic-free zero byte.
        std::ranges::fill(buffer_, std::byte{0});
    }

    // =============================================================================
    // update() – streaming ingestion
    // =============================================================================

    void SHA256::update(std::span<const std::byte> data) noexcept {
        // Accumulate the bit count for the length field in the padding trailer
        // (FIPS 180-4 §5.1.1).  data.size() is in bytes; shift left 3 for bits.
        bit_count_ += static_cast<uint64_t>(data.size()) << 3;

        std::size_t offset = 0u;

        // ── Step 1: If there is a partial block in the buffer from a previous
        //    call, try to complete it. ───────────────────────────────────────────
        if(buf_len_ > 0u) {
            const std::size_t space = BLOCK_SIZE - buf_len_;
            const std::size_t take = std::min(data.size(), space);

            // std::ranges::copy – no casts needed because both sides are
            // std::byte; intent is clear: raw bytes being moved into a buffer.
            std::ranges::copy(data.first(take), buffer_.begin() + static_cast<std::ptrdiff_t>(buf_len_));
            buf_len_ += take;
            offset += take;

            if(buf_len_ == BLOCK_SIZE) {
                compress(buffer_);  // fixed-extent span: BLOCK_SIZE enforced by type
                buf_len_ = 0u;
            }
        }

        // ── Step 2: Process all complete blocks directly from the input span ────
        // Construct a fixed-extent span<const std::byte, BLOCK_SIZE> from a
        // pointer + compile-time extent.  No copy into buffer_ is needed.
        while(data.size() - offset >= BLOCK_SIZE) {
            compress(std::span<const std::byte, BLOCK_SIZE>{data.data() + offset, BLOCK_SIZE});
            offset += BLOCK_SIZE;
        }

        // ── Step 3: Buffer any remaining tail bytes ──────────────────────────────
        const std::size_t tail = data.size() - offset;
        if(tail > 0u) {
            std::ranges::copy(data.subspan(offset, tail), buffer_.begin() + static_cast<std::ptrdiff_t>(buf_len_));
            buf_len_ += tail;
        }
    }

    void SHA256::update(std::string_view text) noexcept {
        // std::as_bytes reinterprets the char storage as const std::byte storage
        // without a cast.  This is the canonical, defined-behaviour conversion
        // from "text being hashed" to "raw byte sequence to hash".
        update(std::as_bytes(std::span{text}));
    }

    // =============================================================================
    // finalise() – padding, final compression, digest extraction
    //
    // FIPS 180-4 §5.1.1 (Padding for SHA-224 and SHA-256):
    //
    //   Let ℓ = message length in bits.  Append:
    //     1. The bit '1'                          (byte 0x80)
    //     2. k zero bits, where k is the smallest non-negative integer satisfying
    //        ℓ + 1 + k ≡ 448 (mod 512)           (zero-fill to byte offset 56)
    //     3. The 64-bit big-endian representation of ℓ  (bytes 56–63)
    //
    //   If the '1' bit and existing data leave fewer than 8 bytes at the end of
    //   the current block, an extra block of padding is needed.
    // =============================================================================

    SHA256::Digest SHA256::finalise() noexcept {
        // ── §5.1.1: Append the '1' bit (represented as byte 0x80) ───────────────
        buffer_[buf_len_++] = std::byte{0x80};

        // ── §5.1.1: Zero-pad to byte position 56 ─────────────────────────────────
        if(buf_len_ > 56u) {
            // The 0x80 byte has pushed us past position 56: we need one extra
            // block.  Zero-fill the rest of the current block, compress it, then
            // zero-fill the new block up to byte 56.
            std::ranges::fill(std::span{buffer_}.subspan(buf_len_, BLOCK_SIZE - buf_len_), std::byte{0});
            compress(buffer_);
            std::ranges::fill(std::span{buffer_}.first(56u), std::byte{0});
        } else {
            // Enough room in the current block: zero-fill from buf_len_ to 56.
            std::ranges::fill(std::span{buffer_}.subspan(buf_len_, 56u - buf_len_), std::byte{0});
        }

        // ── §5.1.1: Append the 64-bit big-endian message length (bits 56–63) ────
        store_be64(buffer_.data() + 56u, bit_count_);

        // ── §6.2.2: Compress the final padded block ──────────────────────────────
        compress(buffer_);

        // ── §6.2: Assemble the digest H₀^(N) ∥ H₁^(N) ∥ … ∥ H₇^(N) ─────────
        // Each 32-bit word is serialised in big-endian byte order (§3.1).
        Digest out{};
        for(std::size_t i = 0u; i < 8u; ++i) { store_be32(out.data() + i * 4u, state_[i]); }

        // Reset so the object is immediately reusable (RAII invariant maintained).
        reset();

        return out;
    }

    std::string SHA256::finalise_hex() noexcept { return to_hex(finalise()); }

    // =============================================================================
    // compress() – SHA-256 round function  (FIPS 180-4 §6.2.2)
    //
    // Parameter: one 512-bit block as a fixed-extent span<const std::byte, 64>.
    // The fixed extent is a compile-time guarantee that the correct amount of
    // data is always passed; no run-time bounds check is needed.
    //
    // Effect: updates state_[0..7]  (the intermediate hash value H^(i)).
    // =============================================================================

    void SHA256::compress(std::span<const std::byte, BLOCK_SIZE> block) noexcept {
        // ── §6.2.2, Step 1: Prepare the message schedule  W₀ … W₆₃ ─────────────
        //
        //   t =  0..15:  Wt = M_t^(i)   (16 words loaded from the block)
        //   t = 16..63:  Wt = σ₁(W_{t-2}) + W_{t-7} + σ₀(W_{t-15}) + W_{t-16}
        //
        // load_be32 takes const std::byte*, converting the buffer bytes to a
        // uint32_t word via std::to_integer – the only arithmetic gateway.
        std::array<uint32_t, 64> W{};

        for(std::size_t t = 0u; t < 16u; ++t) { W[t] = load_be32(block.data() + t * 4u); }
        for(std::size_t t = 16u; t < 64u; ++t) { W[t] = sigma1(W[t - 2]) + W[t - 7] + sigma0(W[t - 15]) + W[t - 16]; }

        // ── §6.2.2, Step 2: Initialise the eight working variables ──────────────
        //
        //   a = H₀^(i-1),  b = H₁^(i-1),  …,  h = H₇^(i-1)
        uint32_t a = state_[0];
        uint32_t b = state_[1];
        uint32_t c = state_[2];
        uint32_t d = state_[3];
        uint32_t e = state_[4];
        uint32_t f = state_[5];
        uint32_t g = state_[6];
        uint32_t h = state_[7];

        // ── §6.2.2, Step 3: 64 compression rounds ────────────────────────────────
        //
        //   T₁ = h + Σ₁(e) + Ch(e,f,g) + Kₜ + Wₜ
        //   T₂ = Σ₀(a) + Maj(a,b,c)
        //   h←g, g←f, f←e, e←d+T₁, d←c, c←b, b←a, a←T₁+T₂
        //
        // All additions are modulo 2^32 (uint32_t wraps naturally).
        for(std::size_t t = 0u; t < 64u; ++t) {
            const uint32_t T1 = h + Sigma1(e) + Ch(e, f, g) + K[t] + W[t];
            const uint32_t T2 = Sigma0(a) + Maj(a, b, c);

            h = g;
            g = f;
            f = e;
            e = d + T1;
            d = c;
            c = b;
            b = a;
            a = T1 + T2;
        }

        // ── §6.2.2, Step 4: Compute the new intermediate hash value  H^(i) ──────
        //
        //   H₀^(i) = a + H₀^(i-1),  H₁^(i) = b + H₁^(i-1), …
        state_[0] += a;
        state_[1] += b;
        state_[2] += c;
        state_[3] += d;
        state_[4] += e;
        state_[5] += f;
        state_[6] += g;
        state_[7] += h;
    }

    // =============================================================================
    // One-shot static helpers – thin wrappers around the streaming interface
    // =============================================================================

    SHA256::Digest SHA256::digest(std::string_view text) noexcept {
        SHA256 ctx;
        ctx.update(text);
        return ctx.finalise();
    }

    std::string SHA256::hash(std::string_view text) noexcept { return to_hex(digest(text)); }

    SHA256::Digest SHA256::digest(std::span<const std::byte> data) noexcept {
        SHA256 ctx;
        ctx.update(data);
        return ctx.finalise();
    }

    std::string SHA256::hash(std::span<const std::byte> data) noexcept { return to_hex(digest(data)); }

    // =============================================================================
    // to_hex() – raw Digest → lowercase hex string
    //
    // std::to_integer<unsigned> is the explicit, type-safe conversion from
    // std::byte to an integer value.  std::format("{:02x}", …) formats it as
    // exactly 2 lowercase hex characters.
    // =============================================================================

    std::string SHA256::to_hex(const Digest &d) noexcept {
        std::string out;
        out.reserve(DIGEST_SIZE * 2u);  // 64 hex characters, no reallocations

        for(const std::byte b : d) { out += FORMAT("{:02x}", std::to_integer<unsigned>(b)); }

        return out;
    }
} // namespace jsv::crypto
// NOLINTEND(*-include-cleaner, *-avoid-magic-numbers)
