/**
 * @file BitCast.hpp
 * @brief Bit casting macros using std::bit_cast for type-safe reinterpretation.
 *
 * @details This header provides macros for performing type-safe bit casts using
 *          std::bit_cast (C++20). Unlike reinterpret_cast, std::bit_cast creates
 *          a new object with the same bit representation, avoiding undefined behavior.
 *
 * @note std::bit_cast requires that the source and destination types are trivially
 *       copyable and have the same size.
 *
 * @copyright Created by gbian on 01/06/2024.
 * @copyright Copyright (c) 2024 All rights reserved.
 */

// NOLINTBEGIN(*-include-cleaner, *-macro-usage)
#pragma once
#include <bit>

/**
 * @defgroup BitTypeCastingMacros Bit Type Casting Macros
 * @{
 */

/**
 * @brief Macro to cast a value to bool using std::bit_cast.
 *
 * @param x The value to be casted.
 * @return The value casted to bool by reinterpreting the bit representation.
 *
 * @note Requires that the source type is trivially copyable and has the same size as bool.
 * @see std::bit_cast
 *
 * @par Example:
 * @code{.cpp}
 * std::byte b = std::byte{0x01};
 * bool result = BC_BOOL(b);  // true
 * @endcode
 */
#define BC_BOOL(x) std::bit_cast<bool>(x)

/**
 * @brief Macro to cast a value to std::byte using std::bit_cast.
 *
 * @param x The value to be casted.
 * @return The value casted to std::byte.
 *
 * @see std::bit_cast
 *
 * @par Example:
 * @code{.cpp}
 * bool b = true;
 * std::byte result = BC_B(b);  // std::byte{0x01}
 * @endcode
 */
#define BC_B(x) std::bit_cast<std::byte>(x)

/**
 * @brief Macro to cast a value to std::intptr_t using std::bit_cast.
 *
 * @param x The value to be casted.
 * @return The value casted to std::intptr_t.
 *
 * @see std::bit_cast
 *
 * @par Example:
 * @code{.cpp}
 * std::uintptr_t uiptr = 0x12345678;
 * std::intptr_t iptr = BC_IPTR(uiptr);
 * @endcode
 */
#define BC_IPTR(x) std::bit_cast<std::intptr_t>(x)

/**
 * @brief Macro to cast a value to std::uintptr_t using std::bit_cast.
 *
 * @param x The value to be casted.
 * @return The value casted to std::uintptr_t.
 *
 * @see std::bit_cast
 *
 * @par Example:
 * @code{.cpp}
 * std::intptr_t iptr = -1;
 * std::uintptr_t uiptr = BC_UIPTR(iptr);
 * @endcode
 */
#define BC_UIPTR(x) std::bit_cast<std::uintptr_t>(x)

/**
 * @brief Macro to cast a value to std::int8_t using std::bit_cast.
 *
 * @param x The value to be casted.
 * @return The value casted to std::int8_t.
 *
 * @see std::bit_cast
 *
 * @par Example:
 * @code{.cpp}
 * std::uint8_t u8 = 0xFF;
 * std::int8_t i8 = BC_I8T(u8);  // -1
 * @endcode
 */
#define BC_I8T(x) std::bit_cast<std::int8_t>(x)

/**
 * @brief Macro to cast a value to std::int16_t using std::bit_cast.
 *
 * @param x The value to be casted.
 * @return The value casted to std::int16_t.
 *
 * @see std::bit_cast
 *
 * @par Example:
 * @code{.cpp}
 * std::uint16_t u16 = 0xFFFF;
 * std::int16_t i16 = BC_I16T(u16);  // -1
 * @endcode
 */
#define BC_I16T(x) std::bit_cast<std::int16_t>(x)

/**
 * @brief Macro to cast a value to std::int32_t using std::bit_cast.
 *
 * @param x The value to be casted.
 * @return The value casted to std::int32_t.
 *
 * @see std::bit_cast
 *
 * @par Example:
 * @code{.cpp}
 * std::uint32_t u32 = 0xFFFFFFFF;
 * std::int32_t i32 = BC_I32T(u32);  // -1
 * @endcode
 */
#define BC_I32T(x) std::bit_cast<std::int32_t>(x)

/**
 * @brief Macro to cast a value to std::int64_t using std::bit_cast.
 *
 * @param x The value to be casted.
 * @return The value casted to std::int64_t.
 *
 * @see std::bit_cast
 *
 * @par Example:
 * @code{.cpp}
 * std::uint64_t u64 = 0xFFFFFFFFFFFFFFFF;
 * std::int64_t i64 = BC_I64T(u64);  // -1
 * @endcode
 */
#define BC_I64T(x) std::bit_cast<std::int64_t>(x)

/**
 * @brief Macro to cast a value to std::uint8_t using std::bit_cast.
 *
 * @param x The value to be casted.
 * @return The value casted to std::uint8_t.
 *
 * @see std::bit_cast
 *
 * @par Example:
 * @code{.cpp}
 * std::int8_t i8 = -1;
 * std::uint8_t u8 = BC_UI8T(i8);  // 0xFF
 * @endcode
 */
#define BC_UI8T(x) std::bit_cast<std::uint8_t>(x)

/**
 * @brief Macro to cast a value to std::uint16_t using std::bit_cast.
 *
 * @param x The value to be casted.
 * @return The value casted to std::uint16_t.
 *
 * @see std::bit_cast
 *
 * @par Example:
 * @code{.cpp}
 * std::int16_t i16 = -1;
 * std::uint16_t u16 = BC_UI16T(i16);  // 0xFFFF
 * @endcode
 */
#define BC_UI16T(x) std::bit_cast<std::uint16_t>(x)

/**
 * @brief Macro to cast a value to std::uint32_t using std::bit_cast.
 *
 * @param x The value to be casted.
 * @return The value casted to std::uint32_t.
 *
 * @see std::bit_cast
 *
 * @par Example:
 * @code{.cpp}
 * std::int32_t i32 = -1;
 * std::uint32_t u32 = BC_UI32T(i32);  // 0xFFFFFFFF
 * @endcode
 */
#define BC_UI32T(x) std::bit_cast<std::uint32_t>(x)

/**
 * @brief Macro to cast a value to std::uint64_t using std::bit_cast.
 *
 * @param x The value to be casted.
 * @return The value casted to std::uint64_t.
 *
 * @see std::bit_cast
 *
 * @par Example:
 * @code{.cpp}
 * std::int64_t i64 = -1;
 * std::uint64_t u64 = BC_UI64T(i64);  // 0xFFFFFFFFFFFFFFFF
 * @endcode
 */
#define BC_UI64T(x) std::bit_cast<std::uint64_t>(x)

/**
 * @brief Macro to cast a value to std::ptrdiff_t using std::bit_cast.
 *
 * @param x The value to be casted.
 * @return The value casted to std::ptrdiff_t.
 *
 * @see std::bit_cast
 *
 * @par Example:
 * @code{.cpp}
 * std::size_t sz = 0x12345678;
 * std::ptrdiff_t diff = BC_PTRDIFT(sz);
 * @endcode
 */
#define BC_PTRDIFT(x) std::bit_cast<std::ptrdiff_t>(x)

/**
 * @brief Macro to cast a value to std::div_t using std::bit_cast.
 *
 * @param x The value to be casted.
 * @return The value casted to std::div_t.
 *
 * @see std::bit_cast
 *
 * @par Example:
 * @code{.cpp}
 * std::div_t result = BC_DIVT(someValue);
 * @endcode
 */
#define BC_DIVT(x) std::bit_cast<std::div_t>(x)

/**
 * @brief Macro to cast a value to std::ldiv_t using std::bit_cast.
 *
 * @param x The value to be casted.
 * @return The value casted to std::ldiv_t.
 *
 * @see std::bit_cast
 *
 * @par Example:
 * @code{.cpp}
 * std::ldiv_t result = BC_LDIVT(someValue);
 * @endcode
 */
#define BC_LDIVT(x) std::bit_cast<std::ldiv_t>(x)

/**
 * @brief Macro to cast a value to std::lldiv_t using std::bit_cast.
 *
 * @param x The value to be casted.
 * @return The value casted to std::lldiv_t.
 *
 * @see std::bit_cast
 *
 * @par Example:
 * @code{.cpp}
 * std::lldiv_t result = BC_LLDIVT(someValue);
 * @endcode
 */
#define BC_LLDIVT(x) std::bit_cast<std::lldiv_t>(x)

/** @} */  // end of BitTypeCastingMacros group
// NOLINTEND(*-include-cleaner, *-macro-usage)
