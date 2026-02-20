//
// Created by gbian on 01/06/2024.
//

#pragma once

/**
 * @namespace TypeSizes
 * @brief Namespace containing sizes of various types.
 *
 * @details This namespace provides constants representing the sizes of different
 *          types in bytes. It includes fundamental types, pointer types, character
 *          types, floating-point types, and string types.
 *
 * @par Example:
 * @code{.cpp}
 * constexpr std::size_t intSize = TypeSizes::sizeOfInt;  // Typically 4
 * constexpr std::size_t ptrSize = TypeSizes::sizeOfIntPtr;  // 4 on 32-bit, 8 on 64-bit
 * @endcode
 */
namespace TypeSizes {
    /// Size of a boolean type (typically 1 byte).
    static inline constexpr std::size_t sizeOfBool = sizeof(bool);

    /// Size of a byte type (always 1 byte by definition).
    static inline constexpr std::size_t sizeOfByte = sizeof(std::byte);

    /// Size of a pointer to an integer type (platform-dependent).
    static inline constexpr std::size_t sizeOfIntPtr = sizeof(std::intptr_t);

    /// Size of a pointer to an unsigned integer type (platform-dependent).
    static inline constexpr std::size_t sizeOfUintPtr = sizeof(std::uintptr_t);

    /// Size of a signed 8-bit integer type (always 1 byte).
    static inline constexpr std::size_t sizeOfInt8T = sizeof(std::int8_t);

    /// Size of a signed 16-bit integer type (always 2 bytes).
    static inline constexpr std::size_t sizeOfInt16T = sizeof(std::int16_t);

    /// Size of a signed 32-bit integer type (always 4 bytes).
    static inline constexpr std::size_t sizeOfInt32T = sizeof(std::int32_t);

    /// Size of a signed 64-bit integer type (always 8 bytes).
    static inline constexpr std::size_t sizeOfInt64T = sizeof(std::int64_t);

    /// Size of an unsigned 8-bit integer type (always 1 byte).
    static inline constexpr std::size_t sizeOfUint8T = sizeof(std::uint8_t);

    /// Size of an unsigned 16-bit integer type (always 2 bytes).
    static inline constexpr std::size_t sizeOfUint16T = sizeof(std::uint16_t);

    /// Size of an unsigned 32-bit integer type (always 4 bytes).
    static inline constexpr std::size_t sizeOfUint32T = sizeof(std::uint32_t);

    /// Size of an unsigned 64-bit integer type (always 8 bytes).
    static inline constexpr std::size_t sizeOfUint64T = sizeof(std::uint64_t);

    /// Size of a pointer difference type (platform-dependent).
    static inline constexpr std::size_t sizeOfPtrdiffT = sizeof(std::ptrdiff_t);

    /// Size of a structure holding the result of dividing two integers.
    static inline constexpr std::size_t sizeOfDivT = sizeof(std::div_t);

    /// Size of a structure holding the result of dividing two long integers.
    static inline constexpr std::size_t sizeOfLdivT = sizeof(std::ldiv_t);

    /// Size of a character type (always 1 byte by definition).
    static inline constexpr std::size_t sizeOfChar = sizeof(char);

    /// Size of a 16-bit character type (always 2 bytes).
    static inline constexpr std::size_t sizeOfChar16T = sizeof(char16_t);

    /// Size of a 32-bit character type (always 4 bytes).
    static inline constexpr std::size_t sizeOfChar32T = sizeof(char32_t);

    /// Size of an 8-bit character type (always 1 byte).
    static inline constexpr std::size_t sizeOfChar8T = sizeof(char8_t);

    /// Size of a wide character type (platform-dependent, typically 2 or 4 bytes).
    static inline constexpr std::size_t sizeOfWChar = sizeof(wchar_t);

    /// Size of a double-precision floating-point type (typically 8 bytes).
    static inline constexpr std::size_t sizeOfDouble = sizeof(double);

    /// Size of a single-precision floating-point type (typically 4 bytes).
    static inline constexpr std::size_t sizeOfFloat = sizeof(float);

    /// Size of an integer type (platform-dependent, typically 4 bytes).
    static inline constexpr std::size_t sizeOfInt = sizeof(int);

    /// Size of a long integer type (platform-dependent, 4 or 8 bytes).
    static inline constexpr std::size_t sizeOfLong = sizeof(long);

    /// Size of a long double-precision floating-point type (typically 12 or 16 bytes).
    static inline constexpr std::size_t sizeOfLongDouble = sizeof(long double);

    /// Size of a long integer type (same as sizeOfLong).
    static inline constexpr std::size_t sizeOfLongInt = sizeof(long int);

    /// Size of a long long integer type (at least 8 bytes).
    static inline constexpr std::size_t sizeOfLongLong = sizeof(long long);

    /// Size of a long long integer type (same as sizeOfLongLong).
    static inline constexpr std::size_t sizeOfLongLongInt = sizeof(long long int);

    /// Size of a short integer type (typically 2 bytes).
    static inline constexpr std::size_t sizeOfShort = sizeof(short);

    /// Size of a short integer type (same as sizeOfShort).
    static inline constexpr std::size_t sizeOfShortInt = sizeof(short int);

    /// Size of an unsigned character type (always 1 byte).
    static inline constexpr std::size_t sizeOfUChar = sizeof(unsigned char);

    /// Size of an unsigned integer type (platform-dependent, typically 4 bytes).
    static inline constexpr std::size_t sizeOfUInt = sizeof(unsigned int);

    /// Size of an unsigned long integer type (platform-dependent).
    static inline constexpr std::size_t sizeOfULong = sizeof(unsigned long);

    /// Size of an unsigned long integer type (same as sizeOfULong).
    static inline constexpr std::size_t sizeOfULongInt = sizeof(unsigned long int);

    /// Size of an unsigned long long integer type (at least 8 bytes).
    static inline constexpr std::size_t sizeOfULongLong = sizeof(unsigned long long);

    /// Size of an unsigned long long integer type (same as sizeOfULongLong).
    static inline constexpr std::size_t sizeOfULongLongInt = sizeof(unsigned long long int);

    /// Size of a C++ standard string (implementation-dependent, typically 24-32 bytes).
    static inline constexpr std::size_t sizeOfString = sizeof(std::string);

    /// Size of a wide character string (implementation-dependent).
    static inline constexpr std::size_t sizeOfWString = sizeof(std::wstring);

    /// Size of a UTF-8 encoded string (implementation-dependent).
    static inline constexpr std::size_t sizeOfU8String = sizeof(std::u8string);

    /// Size of a UTF-16 encoded string (implementation-dependent).
    static inline constexpr std::size_t sizeOfU16String = sizeof(std::u16string);

    /// Size of a UTF-32 encoded string (implementation-dependent).
    static inline constexpr std::size_t sizeOfU32String = sizeof(std::u32string);

    /// Size of a C++ standard string view (typically 16 bytes: pointer + size).
    static inline constexpr std::size_t sizeOfStringView = sizeof(std::string_view);

    /// Size of a wide character string view (implementation-dependent).
    static inline constexpr std::size_t sizeOfWStringView = sizeof(std::wstring_view);

    /// Size of a UTF-8 encoded string view (implementation-dependent).
    static inline constexpr std::size_t sizeOfU8StringView = sizeof(std::u8string_view);

    /// Size of a UTF-16 encoded string view (implementation-dependent).
    static inline constexpr std::size_t sizeOfU16StringView = sizeof(std::u16string_view);

    /// Size of a UTF-32 encoded string view (implementation-dependent).
    static inline constexpr std::size_t sizeOfU32StringView = sizeof(std::u32string_view);
}  // namespace TypeSizes
