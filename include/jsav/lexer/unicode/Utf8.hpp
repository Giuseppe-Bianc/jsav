/*
 * Created by gbian on 01/03/2026.
 * Copyright (c) 2026 All rights reserved.
 */

#pragma once

#include <cstddef>
#include <cstdint>
#include <string_view>

// NOLINTBEGIN(*-magic-numbers, *-avoid-magic-numbers)

namespace jsv::unicode {

    // =========================================================================
    // Types (per contracts/unicode-api.md)
    // =========================================================================

    /// Classification of a UTF-8 decode attempt.
    enum class Utf8Status : std::uint8_t {
        Ok,                    ///< Valid, well-formed sequence
        Overlong,              ///< Overlong encoding (e.g., 0xC0 0x80 for U+0000)
        Surrogate,             ///< Surrogate code point (U+D800–U+DFFF)
        OutOfRange,            ///< Code point > U+10FFFF
        OrphanedContinuation,  ///< Continuation byte (0x80–0xBF) without lead byte
        TruncatedSequence,     ///< Lead byte with missing continuation bytes
        InvalidLeadByte,       ///< Byte 0xF5–0xFF (never valid in UTF-8)
    };

    /// Result of decoding one UTF-8 sequence.
    struct Utf8DecodeResult {
        char32_t codepoint;        ///< Decoded code point; U+FFFD on any error
        std::uint8_t byte_length;  ///< Bytes consumed (1–4), always ≥ 1
        Utf8Status status;         ///< Ok or error classification
    };

    // =========================================================================
    // Internal sub-decoders (implementation detail — declared here for inlining)
    // =========================================================================
    namespace detail {

        /// Decode a 2-byte UTF-8 sequence starting at input[offset].
        /// Precondition: offset + 1 < input.size(), input[offset] is 0xC2–0xDF.
        [[nodiscard]] constexpr Utf8DecodeResult decode_2byte(std::string_view input, std::size_t offset) noexcept;

        /// Decode a 3-byte UTF-8 sequence starting at input[offset].
        /// Precondition: offset + 2 < input.size(), input[offset] is 0xE0–0xEF.
        [[nodiscard]] constexpr Utf8DecodeResult decode_3byte(std::string_view input, std::size_t offset) noexcept;

        /// Decode a 4-byte UTF-8 sequence starting at input[offset].
        /// Precondition: offset + 3 < input.size(), input[offset] is 0xF0–0xF4.
        [[nodiscard]] constexpr Utf8DecodeResult decode_4byte(std::string_view input, std::size_t offset) noexcept;

        /// Handle truncated 4-byte sequence (maximal-subpart sub-logic).
        /// Precondition: offset + 3 >= input.size()
        [[nodiscard]] constexpr Utf8DecodeResult decode_4byte_truncated(std::string_view input, std::size_t offset,
                                                                        std::uint8_t b0) noexcept;

    }  // namespace detail

    // =========================================================================
    // Public API (bodies implemented below — Phase 2)
    // =========================================================================

    /// Decode one UTF-8 sequence starting at input[offset].
    ///
    /// Returns decoded codepoint, bytes consumed, and validation status.
    /// On error, codepoint is U+FFFD and byte_length is the maximal subpart length.
    ///
    /// Precondition: offset < input.size()
    [[nodiscard]] constexpr Utf8DecodeResult decode_utf8(std::string_view input, std::size_t offset) noexcept;

    /// Validate that input[offset..] starts with a well-formed UTF-8 sequence.
    /// Equivalent to: decode_utf8(input, offset).status == Utf8Status::Ok
    [[nodiscard]] constexpr bool is_valid_utf8_at(std::string_view input, std::size_t offset) noexcept;

    // =========================================================================
    // Inline implementations (constexpr functions must be defined in headers)
    // =========================================================================

    namespace detail {

        [[nodiscard]] constexpr Utf8DecodeResult decode_2byte(std::string_view input, std::size_t offset) noexcept {
            // Lead byte already validated as 0xC2–0xDF by caller
            const auto b0 = static_cast<std::uint8_t>(input[offset]);
            // Need exactly 1 continuation byte
            if(offset + 1 >= input.size()) { return {U'\uFFFD', 1, Utf8Status::TruncatedSequence}; }
            const auto b1 = static_cast<std::uint8_t>(input[offset + 1]);
            if((b1 & 0xC0U) != 0x80U) {
                // Invalid continuation byte — consume only the lead byte
                return {U'\uFFFD', 1, Utf8Status::TruncatedSequence};
            }
            const char32_t cp = ((b0 & 0x1FU) << 6U) | (b1 & 0x3FU);
            // Overlong check: valid 2-byte range is U+0080–U+07FF.
            // b0 is 0xC2–0xDF which guarantees cp >= U+0080, no extra check needed.
            return {cp, 2, Utf8Status::Ok};
        }

        [[nodiscard]] constexpr Utf8DecodeResult decode_3byte(std::string_view input, std::size_t offset) noexcept {
            const auto b0 = static_cast<std::uint8_t>(input[offset]);
            // Need exactly 2 continuation bytes
            if(offset + 2 >= input.size()) {
                // Maximal subpart: if b1 exists and is a valid continuation, consume 2
                if(offset + 1 < input.size()) {
                    const auto b1 = static_cast<std::uint8_t>(input[offset + 1]);
                    if((b1 & 0xC0U) == 0x80U) {
                        // b0 = E0: b1 must also be A0–BF for a valid prefix
                        if(b0 == 0xE0U && b1 < 0xA0U) { return {U'\uFFFD', 1, Utf8Status::Overlong}; }
                        // b0 = ED: b1 must be 80–9F (surrogates are A0–BF)
                        if(b0 == 0xEDU && b1 >= 0xA0U) { return {U'\uFFFD', 1, Utf8Status::Surrogate}; }
                        return {U'\uFFFD', 2, Utf8Status::TruncatedSequence};
                    }
                }
                return {U'\uFFFD', 1, Utf8Status::TruncatedSequence};
            }
            const auto b1 = static_cast<std::uint8_t>(input[offset + 1]);
            const auto b2 = static_cast<std::uint8_t>(input[offset + 2]);
            if((b1 & 0xC0U) != 0x80U) { return {U'\uFFFD', 1, Utf8Status::TruncatedSequence}; }
            // Overlong 3-byte: E0 with b1 < A0
            if(b0 == 0xE0U && b1 < 0xA0U) { return {U'\uFFFD', 1, Utf8Status::Overlong}; }
            // Surrogate: ED with b1 in A0–BF
            if(b0 == 0xEDU && b1 >= 0xA0U) { return {U'\uFFFD', 1, Utf8Status::Surrogate}; }
            if((b2 & 0xC0U) != 0x80U) { return {U'\uFFFD', 2, Utf8Status::TruncatedSequence}; }
            const char32_t cp = ((b0 & 0x0FU) << 12U) | ((b1 & 0x3FU) << 6U) | (b2 & 0x3FU);
            return {cp, 3, Utf8Status::Ok};
        }

        [[nodiscard]] constexpr Utf8DecodeResult decode_4byte_truncated(std::string_view input, std::size_t offset,
                                                                        std::uint8_t b0) noexcept {
            if(offset + 1 < input.size()) {
                const auto b1 = static_cast<std::uint8_t>(input[offset + 1]);
                if((b1 & 0xC0U) == 0x80U) {
                    if(b0 == 0xF0U && b1 < 0x90U) { return {U'\uFFFD', 1, Utf8Status::Overlong}; }
                    if(b0 == 0xF4U && b1 >= 0x90U) { return {U'\uFFFD', 1, Utf8Status::OutOfRange}; }
                    if(offset + 2 < input.size()) {
                        const auto b2 = static_cast<std::uint8_t>(input[offset + 2]);
                        if((b2 & 0xC0U) == 0x80U) { return {U'\uFFFD', 3, Utf8Status::TruncatedSequence}; }
                    }
                    return {U'\uFFFD', 2, Utf8Status::TruncatedSequence};
                }
            }
            return {U'\uFFFD', 1, Utf8Status::TruncatedSequence};
        }

        [[nodiscard]] constexpr Utf8DecodeResult decode_4byte(std::string_view input, std::size_t offset) noexcept {
            const auto b0 = static_cast<std::uint8_t>(input[offset]);
            // Need exactly 3 continuation bytes
            if(offset + 3 >= input.size()) { return decode_4byte_truncated(input, offset, b0); }
            const auto b1 = static_cast<std::uint8_t>(input[offset + 1]);
            const auto b2 = static_cast<std::uint8_t>(input[offset + 2]);
            const auto b3 = static_cast<std::uint8_t>(input[offset + 3]);
            if((b1 & 0xC0U) != 0x80U) { return {U'\uFFFD', 1, Utf8Status::TruncatedSequence}; }
            // Overlong 4-byte: F0 with b1 < 90
            if(b0 == 0xF0U && b1 < 0x90U) { return {U'\uFFFD', 1, Utf8Status::Overlong}; }
            // Out-of-range: F4 90+ (U+110000+)
            if(b0 == 0xF4U && b1 >= 0x90U) { return {U'\uFFFD', 1, Utf8Status::OutOfRange}; }
            if((b2 & 0xC0U) != 0x80U) { return {U'\uFFFD', 2, Utf8Status::TruncatedSequence}; }
            if((b3 & 0xC0U) != 0x80U) { return {U'\uFFFD', 3, Utf8Status::TruncatedSequence}; }
            const char32_t cp = ((b0 & 0x07U) << 18U) | ((b1 & 0x3FU) << 12U) | ((b2 & 0x3FU) << 6U) | (b3 & 0x3FU);
            return {cp, 4, Utf8Status::Ok};
        }

    }  // namespace detail

    // ─────────────────────────────────────────────────────────────────────────
    // decode_utf8 — primary entry point (R-01: if-else cascade with ASCII fast-path)
    // ─────────────────────────────────────────────────────────────────────────

    [[nodiscard]] constexpr Utf8DecodeResult decode_utf8(std::string_view input, std::size_t offset) noexcept {
        // Defensive bounds check: prevent out-of-bounds access
        if(offset >= input.size()) { return {U'\uFFFD', 1, Utf8Status::OutOfRange}; }
        const auto first = static_cast<std::uint8_t>(input[offset]);

        // ASCII fast-path (> 95% of real-world input)
        if(first < 0x80U) [[likely]] { return {static_cast<char32_t>(first), 1, Utf8Status::Ok}; }

        // Orphaned continuation byte (0x80–0xBF)
        if(first < 0xC0U) { return {U'\uFFFD', 1, Utf8Status::OrphanedContinuation}; }

        // 2-byte sequence: lead byte 0xC0–0xDF
        if(first < 0xE0U) {
            // 0xC0–0xC1 are always overlong (would encode U+0000–U+007F)
            if(first < 0xC2U) { return {U'\uFFFD', 1, Utf8Status::Overlong}; }
            return detail::decode_2byte(input, offset);
        }

        // 3-byte sequence: lead byte 0xE0–0xEF
        if(first < 0xF0U) { return detail::decode_3byte(input, offset); }

        // 4-byte sequence: lead byte 0xF0–0xF4
        if(first <= 0xF4U) { return detail::decode_4byte(input, offset); }

        // 0xF5–0xFF: invalid lead byte
        return {U'\uFFFD', 1, Utf8Status::InvalidLeadByte};
    }

    [[nodiscard]] constexpr bool is_valid_utf8_at(std::string_view input, std::size_t offset) noexcept {
        // Defensive bounds check: prevent out-of-bounds access
        if(offset >= input.size()) { return false; }
        return decode_utf8(input, offset).status == Utf8Status::Ok;
    }

}  // namespace jsv::unicode

// NOLINTEND(*-magic-numbers, *-avoid-magic-numbers)
