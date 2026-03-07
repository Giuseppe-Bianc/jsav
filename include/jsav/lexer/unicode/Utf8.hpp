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

        constexpr std::uint8_t CONT_MASK = 0xC0;
        constexpr std::uint8_t CONT_TAG = 0x80;
        constexpr std::uint8_t ASCII_MAX = 0x7F;
        constexpr std::uint8_t CONTINUATION_MIN = 0x80;
        constexpr std::uint8_t CONTINUATION_MAX = 0xBF;
        constexpr std::uint8_t LEAD_2BYTE_MIN = 0xC2;
        constexpr std::uint8_t LEAD_2BYTE_MAX = 0xDF;
        constexpr std::uint8_t LEAD_2BYTE_OVERLONG_MAX = 0xC1;
        constexpr std::uint8_t LEAD_3BYTE_MIN = 0xE0;
        constexpr std::uint8_t LEAD_3BYTE_MAX = 0xEF;
        constexpr std::uint8_t LEAD_3BYTE_SURROGATE = 0xED;
        constexpr std::uint8_t LEAD_4BYTE_MIN = 0xF0;
        constexpr std::uint8_t LEAD_4BYTE_MAX = 0xF4;
        constexpr std::uint8_t LEAD_INVALID_MIN = 0xF5;
        constexpr std::uint8_t PAYLOAD_MASK = 0x3F;
        constexpr std::uint8_t LEAD_2BYTE_PAYLOAD_MASK = 0x1F;
        constexpr std::uint8_t LEAD_3BYTE_PAYLOAD_MASK = 0x0F;
        constexpr std::uint8_t LEAD_4BYTE_PAYLOAD_MASK = 0x07;
        constexpr std::uint8_t OVERLONG_3BYTE_THRESHOLD = 0xA0;
        constexpr std::uint8_t OVERLONG_4BYTE_THRESHOLD = 0x90;
        constexpr std::uint8_t SHIFT_2BYTE = 6U;
        constexpr std::uint8_t SHIFT_3BYTE = 12U;
        constexpr std::uint8_t SHIFT_4BYTE = 18U;

        constexpr char32_t REPLACEMENT_CHAR = U'\uFFFD';

        constexpr bool is_continuation(std::uint8_t b) noexcept { return (b & CONT_MASK) == CONT_TAG; }

        [[nodiscard]] constexpr Utf8DecodeResult decode_2byte(std::string_view input, std::size_t offset) noexcept {
            // Lead byte already validated as 0xC2–0xDF by caller
            const auto b0 = C_UI8T(input[offset]);
            // Need exactly 1 continuation byte
            if(offset + 1 >= input.size()) { return {REPLACEMENT_CHAR, 1, Utf8Status::TruncatedSequence}; }
            const auto b1 = static_cast<std::uint8_t>(input[offset + 1]);
            if(!is_continuation(b1)) { return {REPLACEMENT_CHAR, 1, Utf8Status::TruncatedSequence}; }
            const char32_t cp = C_C32(((b0 & LEAD_2BYTE_PAYLOAD_MASK) << SHIFT_2BYTE) | (b1 & PAYLOAD_MASK));
            // Overlong check: valid 2-byte range is U+0080–U+07FF.
            // b0 is 0xC2–0xDF which guarantees cp >= U+0080, no extra check needed.
            return {cp, 2, Utf8Status::Ok};
        }

        [[nodiscard]] constexpr Utf8DecodeResult decode_3byte(std::string_view input, std::size_t offset) noexcept {
            const auto b0 = C_UI8T(input[offset]);
            const auto offset1 = offset + 1;
            const auto offset2 = offset + 2;
            // Need exactly 2 continuation bytes
            if(offset2 >= input.size()) {
                // Maximal subpart: if b1 exists and is a valid continuation, consume 2
                if(offset1 < input.size()) {
                    if(const auto b1 = C_UI8T(input[offset1]); is_continuation(b1)) {
                        if(b0 == LEAD_3BYTE_MIN && b1 < OVERLONG_3BYTE_THRESHOLD) { return {REPLACEMENT_CHAR, 2, Utf8Status::Overlong}; }
                        if(b0 == LEAD_3BYTE_SURROGATE && b1 >= OVERLONG_3BYTE_THRESHOLD) {
                            return {REPLACEMENT_CHAR, 2, Utf8Status::Surrogate};
                        }
                        return {REPLACEMENT_CHAR, 2, Utf8Status::TruncatedSequence};
                    }
                }
                return {REPLACEMENT_CHAR, 1, Utf8Status::TruncatedSequence};
            }
            const auto b1 = C_UI8T(input[offset1]);
            const auto b2 = C_UI8T(input[offset2]);
            if(!is_continuation(b1)) { return {REPLACEMENT_CHAR, 1, Utf8Status::TruncatedSequence}; }
            // Overlong 3-byte: E0 with b1 < A0 — consume maximal subpart (all 3 bytes if valid)
            if(b0 == LEAD_3BYTE_MIN && b1 < OVERLONG_3BYTE_THRESHOLD) {
                if(is_continuation(b2)) { return {REPLACEMENT_CHAR, 3, Utf8Status::Overlong}; }
                return {REPLACEMENT_CHAR, 2, Utf8Status::Overlong};
            }
            // Surrogate: ED with b1 in A0–BF — consume maximal subpart (all 3 bytes if valid)
            if(b0 == LEAD_3BYTE_SURROGATE && b1 >= OVERLONG_3BYTE_THRESHOLD) {
                if(is_continuation(b2)) { return {REPLACEMENT_CHAR, 3, Utf8Status::Surrogate}; }
                return {REPLACEMENT_CHAR, 2, Utf8Status::Surrogate};
            }
            if(!is_continuation(b2)) { return {REPLACEMENT_CHAR, 2, Utf8Status::TruncatedSequence}; }
            const char32_t cp = C_C32(((b0 & LEAD_3BYTE_PAYLOAD_MASK) << SHIFT_3BYTE) | ((b1 & PAYLOAD_MASK) << SHIFT_2BYTE) |
                                      (b2 & PAYLOAD_MASK));

            return {cp, 3, Utf8Status::Ok};
        }

        [[nodiscard]] constexpr Utf8DecodeResult decode_4byte_truncated(std::string_view input, std::size_t offset,
                                                                        std::uint8_t b0) noexcept {
            const auto offset1 = offset + 1;
            const auto offset2 = offset + 2;
            if(offset1 < input.size()) {
                if(const auto b1 = C_UI8T(input[offset1]); is_continuation(b1)) {
                    // Overlong 4-byte: F0 with b1 < 90 — consume maximal subpart
                    if(b0 == LEAD_4BYTE_MIN && b1 < OVERLONG_4BYTE_THRESHOLD) {
                        if(offset2 < input.size() && is_continuation(C_UI8T(input[offset2]))) {
                            return {REPLACEMENT_CHAR, 3, Utf8Status::Overlong};
                        }
                        return {REPLACEMENT_CHAR, 2, Utf8Status::Overlong};
                    }
                    // Out-of-range: F4 with b1 >= 90 — consume maximal subpart
                    if(b0 == LEAD_4BYTE_MAX && b1 >= OVERLONG_4BYTE_THRESHOLD) {
                        if(offset2 < input.size() && is_continuation(C_UI8T(input[offset2]))) {
                            return {REPLACEMENT_CHAR, 3, Utf8Status::OutOfRange};
                        }
                        return {REPLACEMENT_CHAR, 2, Utf8Status::OutOfRange};
                    }
                    if(offset2 < input.size() && is_continuation(C_UI8T(input[offset2]))) {
                        return {REPLACEMENT_CHAR, 3, Utf8Status::TruncatedSequence};
                    }
                    return {REPLACEMENT_CHAR, 2, Utf8Status::TruncatedSequence};
                }
            }
            return {REPLACEMENT_CHAR, 1, Utf8Status::TruncatedSequence};
        }

        [[nodiscard]] constexpr Utf8DecodeResult decode_4byte(std::string_view input, std::size_t offset) noexcept {
            const auto b0 = C_UI8T(input[offset]);
            const auto offset3 = offset + 3;
            // Need exactly 3 continuation bytes
            if(offset3 >= input.size()) { return decode_4byte_truncated(input, offset, b0); }
            const auto b1 = C_UI8T(input[offset + 1]);
            const auto b2 = C_UI8T(input[offset + 2]);
            const auto b3 = C_UI8T(input[offset3]);
            if(!is_continuation(b1)) { return {REPLACEMENT_CHAR, 1, Utf8Status::TruncatedSequence}; }
            // Overlong 4-byte: F0 with b1 < 90 — consume all 4 bytes (maximal subpart)
            if(b0 == LEAD_4BYTE_MIN && b1 < OVERLONG_4BYTE_THRESHOLD) {
                if(!is_continuation(b2)) { return {REPLACEMENT_CHAR, 2, Utf8Status::Overlong}; }
                if(!is_continuation(b3)) { return {REPLACEMENT_CHAR, 3, Utf8Status::Overlong}; }
                return {REPLACEMENT_CHAR, 4, Utf8Status::Overlong};
            }
            // Out-of-range: F4 90+ (U+110000+) — consume all 4 bytes (maximal subpart)
            if(b0 == LEAD_4BYTE_MAX && b1 >= OVERLONG_4BYTE_THRESHOLD) {
                if(!is_continuation(b2)) { return {REPLACEMENT_CHAR, 2, Utf8Status::OutOfRange}; }
                if(!is_continuation(b3)) { return {REPLACEMENT_CHAR, 3, Utf8Status::OutOfRange}; }
                return {REPLACEMENT_CHAR, 4, Utf8Status::OutOfRange};
            }
            if(!is_continuation(b2)) { return {REPLACEMENT_CHAR, 2, Utf8Status::TruncatedSequence}; }
            if(!is_continuation(b3)) { return {REPLACEMENT_CHAR, 3, Utf8Status::TruncatedSequence}; }
            const char32_t cp = C_C32(((b0 & LEAD_4BYTE_PAYLOAD_MASK) << SHIFT_4BYTE) | ((b1 & PAYLOAD_MASK) << SHIFT_3BYTE) |
                                      ((b2 & PAYLOAD_MASK) << SHIFT_2BYTE) | (b3 & PAYLOAD_MASK));
            return {cp, 4, Utf8Status::Ok};
        }

    }  // namespace detail

    // ─────────────────────────────────────────────────────────────────────────
    // decode_utf8 — primary entry point (R-01: if-else cascade with ASCII fast-path)
    // ─────────────────────────────────────────────────────────────────────────

    [[nodiscard]] constexpr Utf8DecodeResult decode_utf8(std::string_view input, std::size_t offset) noexcept {
        // Defensive bounds check: prevent out-of-bounds access
        if(offset >= input.size()) { return {detail::REPLACEMENT_CHAR, 1, Utf8Status::OutOfRange}; }
        const auto first = C_UI8T(input[offset]);
        // ASCII fast-path (> 95% of real-world input)
        if(first <= detail::ASCII_MAX) [[likely]] { return {static_cast<char32_t>(first), 1, Utf8Status::Ok}; }

        // Orphaned continuation byte (0x80–0xBF)
        if(first <= detail::CONTINUATION_MAX) { return {detail::REPLACEMENT_CHAR, 1, Utf8Status::OrphanedContinuation}; }

        // 2-byte sequence: lead byte 0xC0–0xDF
        if(first < detail::LEAD_3BYTE_MIN) {
            // 0xC0–0xC1 are always overlong (would encode U+0000–U+007F)
            // Consume maximal subpart: both bytes if valid continuation exists
            if(first <= detail::LEAD_2BYTE_OVERLONG_MAX) {
                if(offset + 1 < input.size() && detail::is_continuation(C_UI8T(input[offset + 1]))) {
                    return {detail::REPLACEMENT_CHAR, 2, Utf8Status::Overlong};
                }
                return {detail::REPLACEMENT_CHAR, 1, Utf8Status::Overlong};
            }
            return detail::decode_2byte(input, offset);
        }

        // 3-byte sequence: lead byte 0xE0–0xEF
        if(first < detail::LEAD_4BYTE_MIN) { return detail::decode_3byte(input, offset); }

        // 4-byte sequence: lead byte 0xF0–0xF4
        if(first <= detail::LEAD_4BYTE_MAX) { return detail::decode_4byte(input, offset); }

        // 0xF5–0xFF: invalid lead byte
        return {detail::REPLACEMENT_CHAR, 1, Utf8Status::InvalidLeadByte};
    }

    [[nodiscard]] constexpr bool is_valid_utf8_at(std::string_view input, std::size_t offset) noexcept {
        // Defensive bounds check: prevent out-of-bounds access
        if(offset >= input.size()) { return false; }
        return decode_utf8(input, offset).status == Utf8Status::Ok;
    }

}  // namespace jsv::unicode

// NOLINTEND(*-magic-numbers, *-avoid-magic-numbers)
