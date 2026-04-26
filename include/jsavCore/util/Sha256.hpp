#pragma once

#include "../headersCore.hpp"

/**
 * @namespace jsv::crypto
 * @brief Cryptographic utilities namespace.
 */
namespace jsv::crypto {

    /**
     * @brief Implementation of the SHA-256 secure hash algorithm.
     *
     * Following FIPS 180-4: Secure Hash Standard (SHS).
     * https://nvlpubs.nist.gov/nistpubs/FIPS/NIST.FIPS.180-4.pdf
     */
    class Sha256 {
    public:
        /// Size of the SHA-256 digest in bytes (256 bits).
        static constexpr std::size_t DigestSize = 32;
        /// Size of the SHA-256 processing block in bytes (512 bits).
        static constexpr std::size_t BlockSize = 64;

        /// Type representing the final 256-bit digest.
        using Digest = std::array<std::byte, DigestSize>;

        /**
         * @brief Default constructor initializing the SHA-256 state.
         */
        Sha256() noexcept;

        /**
         * @brief Updates the hash state with new data.
         * @param data A span of bytes to process.
         */
        void update(std::span<const std::byte> data) noexcept;

        /**
         * @brief Finalizes the hashing process and returns the digest.
         * @return The 32-byte SHA-256 hash.
         * @note This method resets the internal state after completion.
         */
        [[nodiscard]] Digest finalize() noexcept;

        /**
         * @brief One-shot static method to compute SHA-256 of a byte span.
         * @param data Data to hash.
         * @return The computed hash.
         */
        [[nodiscard]] static Digest hash(std::span<const std::byte> data) noexcept;

        /**
         * @brief Helper to convert a digest to its hexadecimal string representation.
         * @param digest The digest to convert.
         * @return A hexadecimal string of 64 characters.
         */
        [[nodiscard]] static std::string toHexString(const Digest& digest);

    private:
        /**
         * @brief Processes a single 512-bit block.
         * @param block A span of 64 bytes of data.
         */
        void transform(std::span<const std::byte, BlockSize> block) noexcept;

        std::array<std::uint32_t, 8> state_;      ///< Intermediate hash state (H0..H7).
        std::array<std::byte, BlockSize> buffer_; ///< Buffer for data not yet processed.
        std::uint64_t count_;                    ///< Total number of bits processed.
        std::size_t bufferIdx_;                  ///< Current index in the buffer.
    };

}  // namespace jsv::crypto
