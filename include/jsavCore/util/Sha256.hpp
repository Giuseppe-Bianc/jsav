#pragma once

#include "../headersCore.hpp"

/**
 * @namespace jsv::crypto
 * @brief Cryptographic utilities namespace.
 */
namespace jsv::crypto {

    // =============================================================================
    // SHA256 – incremental and one-shot SHA-256 hasher
    // =============================================================================
    class SHA256 {
    public:
        // ─────────────────────────────────────────────────────────────────────────
        // Public type aliases and constants
        // ─────────────────────────────────────────────────────────────────────────

        /// Raw 256-bit digest.
        /// std::byte is used (not uint8_t) because the digest bytes are
        /// not integers: arithmetic on them is meaningless and should be prevented.
        using Digest = std::array<std::byte, 32>;

        /// SHA-256 message-block size in bytes (FIPS 180-4 §3.1 – 512 bits).
        static constexpr std::size_t BLOCK_SIZE = 64u;

        /// SHA-256 digest size in bytes (256 bits).
        static constexpr std::size_t DIGEST_SIZE = 32u;

        // ─────────────────────────────────────────────────────────────────────────
        // Construction / reset
        // ─────────────────────────────────────────────────────────────────────────

        /// Construct and immediately initialise with H^(0) (FIPS 180-4 §5.3.3).
        /// RAII: the object is ready to accept data without any extra "init" call.
        SHA256() noexcept;

        /// Reset to the initial state.  Allows reuse without re-allocation.
        /// Called automatically by finalise() and finalise_hex().
        void reset() noexcept;

        // Value semantics: copying a context produces an independent hasher at
        // the same point in the byte stream, which is the natural expectation.
        SHA256(const SHA256 &) noexcept = default;
        SHA256 &operator=(const SHA256 &) noexcept = default;
        SHA256(SHA256 &&) noexcept = default;
        SHA256 &operator=(SHA256 &&) noexcept = default;
        ~SHA256() = default;

        // ─────────────────────────────────────────────────────────────────────────
        // Streaming interface
        // ─────────────────────────────────────────────────────────────────────────

        /// Feed a raw byte span into the hash.
        ///
        /// std::span<const std::byte> is the canonical parameter type for
        /// "read-only, untyped byte sequence".  It documents clearly that:
        ///   – the caller retains ownership (no copy, no allocation),
        ///   – the data is treated as raw bytes, not text or numbers.
        ///
        /// May be called any number of times before finalise().
        void update(std::span<const std::byte> data) noexcept;

        /// Convenience overload for string literals and std::string_view.
        /// Reinterprets each char as a raw byte via std::as_bytes – valid for
        /// UTF-8 input where every code unit is hashed as a byte value.
        void update(std::string_view text) noexcept;

        /// Finalise the hash:
        ///   1. Apply padding           (FIPS 180-4 §5.1.1)
        ///   2. Compress final block(s) (FIPS 180-4 §6.2.2)
        ///   3. Return the 32-byte digest.
        ///
        /// The context is reset after this call so it can be reused immediately.
        [[nodiscard]] Digest finalise() noexcept;

        /// Like finalise(), but returns a lowercase 64-character hex string.
        [[nodiscard]] std::string finalise_hex() noexcept;

        // ─────────────────────────────────────────────────────────────────────────
        // One-shot static helpers
        // ─────────────────────────────────────────────────────────────────────────

        /// Hash a string view → raw 32-byte digest.
        [[nodiscard]] static Digest digest(std::string_view text) noexcept;
        /// Hash a string view → lowercase 64-char hex string.
        [[nodiscard]] static std::string hash(std::string_view text) noexcept;

        /// Hash raw bytes → raw 32-byte digest.
        [[nodiscard]] static Digest digest(std::span<const std::byte> data) noexcept;
        /// Hash raw bytes → lowercase 64-char hex string.
        [[nodiscard]] static std::string hash(std::span<const std::byte> data) noexcept;

    private:
        // =========================================================================
        // Internal state
        //
        // No raw owning pointers: std::array manages all storage automatically.
        // The default destructor is therefore correct and sufficient (Rule of Zero).
        // =========================================================================

        /// Intermediate hash value H^(i) – eight 32-bit words (FIPS 180-4 §6.2.2).
        std::array<uint32_t, 8> state_{};

        /// Partial-block buffer.
        /// std::byte signals "untyped byte storage" rather than character or
        /// numeric data.  No arithmetic on these bytes is intended.
        std::array<std::byte, BLOCK_SIZE> buffer_{};

        /// Number of bytes currently held in buffer_
        /// (class invariant: 0 ≤ buf_len_ < BLOCK_SIZE).
        std::size_t buf_len_{0};

        /// Total message length in *bits*, accumulated across all update() calls.
        /// Serialised as a big-endian 64-bit integer into the padding trailer
        /// (FIPS 180-4 §5.1.1).  Supports messages up to 2^64 bits.
        uint64_t bit_count_{0};

        // =========================================================================
        // Core compression function  (FIPS 180-4 §6.2.2)
        // =========================================================================

        /// Compress one 512-bit block.
        ///
        /// The fixed-extent span<…, BLOCK_SIZE> enforces the required block size
        /// in the type system: passing a buffer of the wrong size is a compile
        /// error, not a run-time bounds violation.
        void compress(std::span<const std::byte, BLOCK_SIZE> block) noexcept;

        // =========================================================================
        // FIPS 180-4 §4.1.2 – SHA-256 logical functions  (word size w = 32)
        //
        // Naming follows the standard exactly so every function can be located
        // in the PDF without translation.  All are constexpr, [[nodiscard]], and
        // noexcept; the compiler will inline and constant-fold them at -O1+.
        // =========================================================================

        // Ch(x,y,z)  = (x ∧ y) ⊕ (¬x ∧ z)
        [[nodiscard]] static constexpr uint32_t Ch(uint32_t x, uint32_t y, uint32_t z) noexcept { return (x & y) ^ (~x & z); }

        // Maj(x,y,z) = (x ∧ y) ⊕ (x ∧ z) ⊕ (y ∧ z)
        [[nodiscard]] static constexpr uint32_t Maj(uint32_t x, uint32_t y, uint32_t z) noexcept { return (x & y) ^ (x & z) ^ (y & z); }

        // ROTR^n(x)  = (x >> n) | (x << (32-n))     §3.2
        [[nodiscard]] static constexpr uint32_t rotr(uint32_t x, uint32_t n) noexcept { return (x >> n) | (x << (32u - n)); }

        // SHR^n(x)   = x >> n   (logical right shift) §3.2
        [[nodiscard]] static constexpr uint32_t shr(uint32_t x, uint32_t n) noexcept { return x >> n; }

        // Σ₀(x) = ROTR²(x)  ⊕ ROTR¹³(x) ⊕ ROTR²²(x)   §4.1.2
        [[nodiscard]] static constexpr uint32_t Sigma0(uint32_t x) noexcept { return rotr(x, 2) ^ rotr(x, 13) ^ rotr(x, 22); }

        // Σ₁(x) = ROTR⁶(x)  ⊕ ROTR¹¹(x) ⊕ ROTR²⁵(x)   §4.1.2
        [[nodiscard]] static constexpr uint32_t Sigma1(uint32_t x) noexcept { return rotr(x, 6) ^ rotr(x, 11) ^ rotr(x, 25); }

        // σ₀(x) = ROTR⁷(x)  ⊕ ROTR¹⁸(x) ⊕ SHR³(x)      §4.1.2
        [[nodiscard]] static constexpr uint32_t sigma0(uint32_t x) noexcept { return rotr(x, 7) ^ rotr(x, 18) ^ shr(x, 3); }

        // σ₁(x) = ROTR¹⁷(x) ⊕ ROTR¹⁹(x) ⊕ SHR¹⁰(x)     §4.1.2
        [[nodiscard]] static constexpr uint32_t sigma1(uint32_t x) noexcept { return rotr(x, 17) ^ rotr(x, 19) ^ shr(x, 10); }

        // =========================================================================
        // FIPS 180-4 §4.2.2 – SHA-256 round constants  K₀ … K₆₃
        //
        // First 32 bits of the fractional parts of the cube roots of the first
        // 64 primes.  constexpr → read-only data segment / constant-folded.
        // =========================================================================
        static constexpr std::array<uint32_t, 64> K{
            {0x428a2f98u, 0x71374491u, 0xb5c0fbcfu, 0xe9b5dba5u, 0x3956c25bu, 0x59f111f1u, 0x923f82a4u, 0xab1c5ed5u,
             0xd807aa98u, 0x12835b01u, 0x243185beu, 0x550c7dc3u, 0x72be5d74u, 0x80deb1feu, 0x9bdc06a7u, 0xc19bf174u,
             0xe49b69c1u, 0xefbe4786u, 0x0fc19dc6u, 0x240ca1ccu, 0x2de92c6fu, 0x4a7484aau, 0x5cb0a9dcu, 0x76f988dau,
             0x983e5152u, 0xa831c66du, 0xb00327c8u, 0xbf597fc7u, 0xc6e00bf3u, 0xd5a79147u, 0x06ca6351u, 0x14292967u,
             0x27b70a85u, 0x2e1b2138u, 0x4d2c6dfcu, 0x53380d13u, 0x650a7354u, 0x766a0abbu, 0x81c2c92eu, 0x92722c85u,
             0xa2bfe8a1u, 0xa81a664bu, 0xc24b8b70u, 0xc76c51a3u, 0xd192e819u, 0xd6990624u, 0xf40e3585u, 0x106aa070u,
             0x19a4c116u, 0x1e376c08u, 0x2748774cu, 0x34b0bcb5u, 0x391c0cb3u, 0x4ed8aa4au, 0x5b9cca4fu, 0x682e6ff3u,
             0x748f82eeu, 0x78a5636fu, 0x84c87814u, 0x8cc70208u, 0x90befffau, 0xa4506cebu, 0xbef9a3f7u, 0xc67178f2u}};

        // =========================================================================
        // FIPS 180-4 §5.3.3 – SHA-256 initial hash value  H^(0)
        //
        // First 32 bits of the fractional parts of the square roots of the first
        // 8 primes (2, 3, 5, 7, 11, 13, 17, 19).
        // =========================================================================
        static constexpr std::array<uint32_t, 8> H0{
            {0x6a09e667u, 0xbb67ae85u, 0x3c6ef372u, 0xa54ff53au, 0x510e527fu, 0x9b05688cu, 0x1f83d9abu, 0x5be0cd19u}};

        // =========================================================================
        // Byte-order helpers
        //
        // SHA-256 uses big-endian byte order (FIPS 180-4 §3.1, "big-endian
        // convention").  These helpers operate on std::byte pointers to reinforce
        // that buffers hold raw, untyped bytes.
        //
        // std::to_integer<T> is the sole gateway from std::byte to an arithmetic
        // type, making every byte→integer conversion visible and intentional.
        // =========================================================================

        /// Store a 32-bit word as 4 big-endian bytes.
        static constexpr void store_be32(std::byte *out, uint32_t v) noexcept {
            out[0] = static_cast<std::byte>(v >> 24);
            out[1] = static_cast<std::byte>(v >> 16);
            out[2] = static_cast<std::byte>(v >> 8);
            out[3] = static_cast<std::byte>(v);
        }

        /// Load a 32-bit word from 4 big-endian bytes.
        [[nodiscard]] static constexpr uint32_t load_be32(const std::byte *in) noexcept {
            return (std::to_integer<uint32_t>(in[0]) << 24) | (std::to_integer<uint32_t>(in[1]) << 16) |
                   (std::to_integer<uint32_t>(in[2]) << 8) | std::to_integer<uint32_t>(in[3]);
        }

        /// Store a 64-bit value as 8 big-endian bytes.
        /// Used for the message-length field in the padding trailer
        /// (FIPS 180-4 §5.1.1).
        static constexpr void store_be64(std::byte *out, uint64_t v) noexcept {
            out[0] = static_cast<std::byte>(v >> 56);
            out[1] = static_cast<std::byte>(v >> 48);
            out[2] = static_cast<std::byte>(v >> 40);
            out[3] = static_cast<std::byte>(v >> 32);
            out[4] = static_cast<std::byte>(v >> 24);
            out[5] = static_cast<std::byte>(v >> 16);
            out[6] = static_cast<std::byte>(v >> 8);
            out[7] = static_cast<std::byte>(v);
        }

        /// Convert a raw Digest to a lowercase 64-character hex string.
        [[nodiscard]] static std::string to_hex(const Digest &d) noexcept;
    };

}  // namespace jsv::crypto
