//
// Created by gbian on 01/06/2024.
//
// NOLINTBEGIN(*-include-cleaner,*-macro-usage)

#pragma once

#include <utility>

/**
 * @brief Safely casts a value of type U to type T, potentially narrowing the value.
 *
 * @tparam T The target type of the cast.
 * @tparam U The source type of the cast.
 * @param[in] nci The value to be casted.
 * @return The result of the cast as type T.
 *
 * @details This function provides a convenient way to perform type conversions,
 *          especially in cases where narrowing conversions may occur, such as when
 *          converting from a larger integral type to a smaller integral type.
 *
 * @note This function performs a static_cast, which means it's a compile-time cast
 *       and might not perform runtime checks for validity. Ensure that the cast is
 *       safe in all contexts where it's used.
 *
 * @note This function is marked constexpr, allowing it to be evaluated at compile
 *       time when possible. This can potentially improve performance and enable
 *       optimizations.
 *
 * @note This function is noexcept, indicating that it won't throw exceptions under
 *       normal circumstances. However, be cautious when using it with types that
 *       might throw exceptions during copy construction or assignment.
 *
 * @warning This function does not perform runtime checks for validity of the cast.
 *          It's the responsibility of the programmer to ensure that the cast is safe.
 *
 * @par Example:
 * @code{.cpp}
 * int main() {
 *     constexpr int i = narrow_cast<int>(3.14);  // i will be 3
 *     // Safe narrowing conversion
 *     constexpr short s = narrow_cast<short>(50000);  // s will be -15536 due to overflow
 *     return 0;
 * }
 * @endcode
 *
 * @see static_cast
 * @see std::forward
 */
template <class T, class U> constexpr T narrow_cast(U &&nci) noexcept { return static_cast<T>(std::forward<U>(nci)); }

/**
 * @defgroup NarrowTypeCastingMacros Narrow Type Casting Macros
 * @{
 */

/**
 * @brief Macro to cast a value to bool using narrow_cast.
 *
 * @param x The value to cast.
 * @return The casted value.
 *
 * @par Example:
 * @code{.cpp}
 * bool b = NC_BOOL(1);  // true
 * @endcode
 *
 * @see narrow_cast
 */
#define NC_BOOL(x) narrow_cast<bool>(x)

/**
 * @brief Macro to cast a value to std::byte using narrow_cast.
 *
 * @param x The value to cast.
 * @return The casted value.
 *
 * @par Example:
 * @code{.cpp}
 * std::byte b = NC_B(255);  // std::byte{0xFF}
 * @endcode
 *
 * @see narrow_cast
 */
#define NC_B(x) narrow_cast<std::byte>(x)

/**
 * @brief Macro to cast a value to std::intptr_t using narrow_cast.
 *
 * @param x The value to be casted.
 * @return The value casted to std::intptr_t.
 *
 * @par Example:
 * @code{.cpp}
 * std::intptr_t ptr = NC_IPTR(somePointer);
 * @endcode
 *
 * @see narrow_cast
 */
#define NC_IPTR(x) narrow_cast<std::intptr_t>(x)

/**
 * @brief Macro to cast a value to std::uintptr_t using narrow_cast.
 *
 * @param x The value to be casted.
 * @return The value casted to std::uintptr_t.
 *
 * @par Example:
 * @code{.cpp}
 * std::uintptr_t ptr = NC_UIPTR(somePointer);
 * @endcode
 *
 * @see narrow_cast
 */
#define NC_UIPTR(x) narrow_cast<std::uintptr_t>(x)

/**
 * @brief Macro to cast a value to std::int8_t using narrow_cast.
 *
 * @param x The value to be casted.
 * @return The value casted to std::int8_t.
 *
 * @par Example:
 * @code{.cpp}
 * std::int8_t value = NC_I8T(42);
 * @endcode
 *
 * @see narrow_cast
 */
#define NC_I8T(x) narrow_cast<std::int8_t>(x)

/**
 * @brief Macro to cast a value to std::int16_t using narrow_cast.
 *
 * @param x The value to be casted.
 * @return The value casted to std::int16_t.
 *
 * @par Example:
 * @code{.cpp}
 * std::int16_t value = NC_I16T(42);
 * @endcode
 *
 * @see narrow_cast
 */
#define NC_I16T(x) narrow_cast<std::int16_t>(x)

/**
 * @brief Macro to cast a value to std::int32_t using narrow_cast.
 *
 * @param x The value to be casted.
 * @return The value casted to std::int32_t.
 *
 * @par Example:
 * @code{.cpp}
 * std::int32_t value = NC_I32T(42);
 * @endcode
 *
 * @see narrow_cast
 */
#define NC_I32T(x) narrow_cast<std::int32_t>(x)

/**
 * @brief Macro to cast a value to std::int64_t using narrow_cast.
 *
 * @param x The value to be casted.
 * @return The value casted to std::int64_t.
 *
 * @par Example:
 * @code{.cpp}
 * std::int64_t value = NC_I64T(42);
 * @endcode
 *
 * @see narrow_cast
 */
#define NC_I64T(x) narrow_cast<std::int64_t>(x)

/**
 * @brief Macro to cast a value to std::uint8_t using narrow_cast.
 *
 * @param x The value to be casted.
 * @return The value casted to std::uint8_t.
 *
 * @par Example:
 * @code{.cpp}
 * std::uint8_t value = NC_UI8T(42);
 * @endcode
 *
 * @see narrow_cast
 */
#define NC_UI8T(x) narrow_cast<std::uint8_t>(x)

/**
 * @brief Macro to cast a value to std::uint16_t using narrow_cast.
 *
 * @param x The value to be casted.
 * @return The value casted to std::uint16_t.
 *
 * @par Example:
 * @code{.cpp}
 * std::uint16_t value = NC_UI16T(42);
 * @endcode
 *
 * @see narrow_cast
 */
#define NC_UI16T(x) narrow_cast<std::uint16_t>(x)

/**
 * @brief Macro to cast a value to std::uint32_t using narrow_cast.
 *
 * @param x The value to be casted.
 * @return The value casted to std::uint32_t.
 *
 * @par Example:
 * @code{.cpp}
 * std::uint32_t value = NC_UI32T(42);
 * @endcode
 *
 * @see narrow_cast
 */
#define NC_UI32T(x) narrow_cast<std::uint32_t>(x)

/**
 * @brief Macro to cast a value to std::uint64_t using narrow_cast.
 *
 * @param x The value to be casted.
 * @return The value casted to std::uint64_t.
 *
 * @par Example:
 * @code{.cpp}
 * std::uint64_t value = NC_UI64T(42);
 * @endcode
 *
 * @see narrow_cast
 */
#define NC_UI64T(x) narrow_cast<std::uint64_t>(x)

/**
 * @brief Macro to cast a value to std::ptrdiff_t using narrow_cast.
 *
 * @param x The value to be casted.
 * @return The value casted to std::ptrdiff_t.
 *
 * @par Example:
 * @code{.cpp}
 * std::ptrdiff_t diff = NC_PTRDIFT(10);
 * @endcode
 *
 * @see narrow_cast
 */
#define NC_PTRDIFT(x) narrow_cast<std::ptrdiff_t>(x)

/**
 * @brief Macro to cast a value to std::div_t using narrow_cast.
 *
 * @param x The value to be casted.
 * @return The value casted to std::div_t.
 *
 * @par Example:
 * @code{.cpp}
 * std::div_t result = NC_DIVT(10);
 * @endcode
 *
 * @see narrow_cast
 */
#define NC_DIVT(x) narrow_cast<std::div_t>(x)

/**
 * @brief Macro to cast a value to std::ldiv_t using narrow_cast.
 *
 * @param x The value to be casted.
 * @return The value casted to std::ldiv_t.
 *
 * @par Example:
 * @code{.cpp}
 * std::ldiv_t result = NC_LDIVT(10);
 * @endcode
 *
 * @see narrow_cast
 */
#define NC_LDIVT(x) narrow_cast<std::ldiv_t>(x)

/**
 * @brief Macro to cast a value to char using narrow_cast.
 *
 * @param x The value to be casted.
 * @return The value casted to char.
 *
 * @par Example:
 * @code{.cpp}
 * char c = NC_C(65);  // 'A'
 * @endcode
 *
 * @see narrow_cast
 */
#define NC_C(x) narrow_cast<char>(x)

/**
 * @brief Macro to cast a value to char16_t using narrow_cast.
 *
 * @param x The value to be casted.
 * @return The value casted to char16_t.
 *
 * @par Example:
 * @code{.cpp}
 * char16_t c = NC_C16(65);  // u'A'
 * @endcode
 *
 * @see narrow_cast
 */
#define NC_C16(x) narrow_cast<char16_t>(x)

/**
 * @brief Macro to cast a value to char32_t using narrow_cast.
 *
 * @param x The value to be casted.
 * @return The value casted to char32_t.
 *
 * @par Example:
 * @code{.cpp}
 * char32_t c = NC_C32(65);  // U'A'
 * @endcode
 *
 * @see narrow_cast
 */
#define NC_C32(x) narrow_cast<char32_t>(x)

/**
 * @brief Macro to cast a value to char8_t using narrow_cast.
 *
 * @param x The value to be casted.
 * @return The value casted to char8_t.
 *
 * @par Example:
 * @code{.cpp}
 * char8_t c = NC_C8(65);  // u8'A'
 * @endcode
 *
 * @see narrow_cast
 */
#define NC_C8(x) narrow_cast<char8_t>(x)

/**
 * @brief Macro to cast a value to double using narrow_cast.
 *
 * @param x The value to be casted.
 * @return The value casted to double.
 *
 * @par Example:
 * @code{.cpp}
 * double d = NC_D(3.14f);  // 3.14
 * @endcode
 *
 * @see narrow_cast
 */
#define NC_D(x) narrow_cast<double>(x)

/**
 * @brief Macro to cast a value to float using narrow_cast.
 *
 * @param x The value to be casted.
 * @return The value casted to float.
 *
 * @par Example:
 * @code{.cpp}
 * float f = NC_F(3.14);  // 3.14f
 * @endcode
 *
 * @see narrow_cast
 */
#define NC_F(x) narrow_cast<float>(x)

/**
 * @brief Macro to cast a value to int using narrow_cast.
 *
 * @param x The value to be casted.
 * @return The value casted to int.
 *
 * @par Example:
 * @code{.cpp}
 * int i = NC_I(3.14f);  // 3
 * @endcode
 *
 * @see narrow_cast
 */
#define NC_I(x) narrow_cast<int>(x)

/**
 * @brief Macro to cast a value to long using narrow_cast.
 *
 * @param x The value to be casted.
 * @return The value casted to long.
 *
 * @par Example:
 * @code{.cpp}
 * long l = NC_L(42);  // 42L
 * @endcode
 *
 * @see narrow_cast
 */
#define NC_L(x) narrow_cast<long>(x)

/**
 * @brief Macro to cast a value to long double using narrow_cast.
 *
 * @param x The value to be casted.
 * @return The value casted to long double.
 *
 * @par Example:
 * @code{.cpp}
 * long double ld = NC_LD(3.14f);  // 3.14L
 * @endcode
 *
 * @see narrow_cast
 */
#define NC_LD(x) narrow_cast<long double>(x)

/**
 * @brief Macro to cast a value to long int using narrow_cast.
 *
 * @param x The value to be casted.
 * @return The value casted to long int.
 *
 * @par Example:
 * @code{.cpp}
 * long int li = NC_LI(42);  // 42L
 * @endcode
 *
 * @see narrow_cast
 */
#define NC_LI(x) narrow_cast<long int>(x)

/**
 * @brief Macro to cast a value to long long using narrow_cast.
 *
 * @param x The value to be casted.
 * @return The value casted to long long.
 *
 * @par Example:
 * @code{.cpp}
 * long long ll = NC_LL(42);  // 42LL
 * @endcode
 *
 * @see narrow_cast
 */
#define NC_LL(x) narrow_cast<long long>(x)

/**
 * @brief Macro to cast a value to long long int using narrow_cast.
 *
 * @param x The value to be casted.
 * @return The value casted to long long int.
 *
 * @par Example:
 * @code{.cpp}
 * long long int lli = NC_LLI(42);  // 42LL
 * @endcode
 *
 * @see narrow_cast
 */
#define NC_LLI(x) narrow_cast<long long int>(x)

/**
 * @brief Macro to cast a value to short using narrow_cast.
 *
 * @param x The value to be casted.
 * @return The value casted to short.
 *
 * @par Example:
 * @code{.cpp}
 * short s = NC_S(42);  // 42
 * @endcode
 *
 * @see narrow_cast
 */
#define NC_S(x) narrow_cast<short>(x)

/**
 * @brief Macro to cast a value to short int using narrow_cast.
 *
 * @param x The value to be casted.
 * @return The value casted to short int.
 *
 * @par Example:
 * @code{.cpp}
 * short si = NC_SI(42);  // 42
 * @endcode
 *
 * @see narrow_cast
 */
#define NC_SI(x) narrow_cast<short int>(x)

/**
 * @brief Macro to cast a value to unsigned char using narrow_cast.
 *
 * @param x The value to be casted.
 * @return The value casted to unsigned char.
 *
 * @par Example:
 * @code{.cpp}
 * unsigned char uc = NC_UC(65);  // 65U
 * @endcode
 *
 * @see narrow_cast
 */
#define NC_UC(x) narrow_cast<unsigned char>(x)

/**
 * @brief Macro to cast a value to unsigned int using narrow_cast.
 *
 * @param x The value to be casted.
 * @return The value casted to unsigned int.
 *
 * @par Example:
 * @code{.cpp}
 * unsigned int ui = NC_UI(42);  // 42U
 * @endcode
 *
 * @see narrow_cast
 */
#define NC_UI(x) narrow_cast<unsigned int>(x)

/**
 * @brief Macro to cast a value to unsigned long using narrow_cast.
 *
 * @param x The value to be casted.
 * @return The value casted to unsigned long.
 *
 * @par Example:
 * @code{.cpp}
 * unsigned long ul = NC_UL(42);  // 42UL
 * @endcode
 *
 * @see narrow_cast
 */
#define NC_UL(x) narrow_cast<unsigned long>(x)

/**
 * @brief Macro to cast a value to unsigned long int using narrow_cast.
 *
 * @param x The value to be casted.
 * @return The value casted to unsigned long int.
 *
 * @par Example:
 * @code{.cpp}
 * unsigned long int uli = NC_ULI(65);  // 65UL
 * @endcode
 *
 * @see narrow_cast
 */
#define NC_ULI(x) narrow_cast<unsigned long int>(x)

/**
 * @brief Macro to cast a value to unsigned long long using narrow_cast.
 *
 * @param x The value to be casted.
 * @return The value casted to unsigned long long.
 *
 * @par Example:
 * @code{.cpp}
 * unsigned long long ull = NC_ULL(42);  // 42ULL
 * @endcode
 *
 * @see narrow_cast
 */
#define NC_ULL(x) narrow_cast<unsigned long long>(x)

/**
 * @brief Macro to cast a value to unsigned long long int using narrow_cast.
 *
 * @param x The value to be casted.
 * @return The value casted to unsigned long long int.
 *
 * @par Example:
 * @code{.cpp}
 * unsigned long long int ulli = NC_ULLI(42);  // 42ULL
 * @endcode
 *
 * @see narrow_cast
 */
#define NC_ULLI(x) narrow_cast<unsigned long long int>(x)

/**
 * @brief Macro to cast a value to std::string using narrow_cast.
 *
 * @param x The value to be casted.
 * @return The value casted to std::string.
 *
 * @par Example:
 * @code{.cpp}
 * std::string str = NC_STR("Hello");
 * @endcode
 *
 * @see narrow_cast
 */
#define NC_STR(x) narrow_cast<std::string>(x)

/**
 * @brief Macro to cast a value to std::wstring using narrow_cast.
 *
 * @param x The value to be casted.
 * @return The value casted to std::wstring.
 *
 * @par Example:
 * @code{.cpp}
 * std::wstring wstr = NC_WSTR("Hello");
 * @endcode
 *
 * @see narrow_cast
 */
#define NC_WSTR(x) narrow_cast<std::wstring>(x)

/**
 * @brief Macro to cast a value to std::u8string using narrow_cast.
 *
 * @param x The value to be casted.
 * @return The value casted to std::u8string.
 *
 * @par Example:
 * @code{.cpp}
 * std::u8string u8str = NC_U8STR(u8"Hello");
 * @endcode
 *
 * @see narrow_cast
 */
#define NC_U8STR(x) narrow_cast<std::u8string>(x)

/**
 * @brief Macro to cast a value to std::u16string using narrow_cast.
 *
 * @param x The value to be casted.
 * @return The value casted to std::u16string.
 *
 * @par Example:
 * @code{.cpp}
 * std::u16string u16str = NC_U16STR(u"Hello");
 * @endcode
 *
 * @see narrow_cast
 */
#define NC_U16STR(x) narrow_cast<std::u16string>(x)

/**
 * @brief Macro to cast a value to std::u32string using narrow_cast.
 *
 * @param x The value to be casted.
 * @return The value casted to std::u32string.
 *
 * @par Example:
 * @code{.cpp}
 * std::u32string u32str = NC_U32STR(U"Hello");
 * @endcode
 *
 * @see narrow_cast
 */
#define NC_U32STR(x) narrow_cast<std::u32string>(x)

/**
 * @brief Macro to cast a value to std::string_view using narrow_cast.
 *
 * @param x The value to be casted.
 * @return The value casted to std::string_view.
 *
 * @par Example:
 * @code{.cpp}
 * std::string_view strView = NC_STRV("Hello, World!");
 * @endcode
 *
 * @see narrow_cast
 */
#define NC_STRV(x) narrow_cast<std::string_view>(x)

/**
 * @brief Macro to cast a value to std::wstring_view using narrow_cast.
 *
 * @param x The value to be casted.
 * @return The value casted to std::wstring_view.
 *
 * @par Example:
 * @code{.cpp}
 * std::wstring_view wstrView = NC_WSTRV(L"Hello, World!");
 * @endcode
 *
 * @see narrow_cast
 */
#define NC_WSTRV(x) narrow_cast<std::wstring_view>(x)

/**
 * @brief Macro to cast a value to std::u8string_view using narrow_cast.
 *
 * @param x The value to be casted.
 * @return The value casted to std::u8string_view.
 *
 * @par Example:
 * @code{.cpp}
 * std::u8string_view u8strView = NC_U8STRV(u8"Hello, World!");
 * @endcode
 *
 * @see narrow_cast
 */
#define NC_U8STRV(x) narrow_cast<std::u8string_view>(x)

/**
 * @brief Macro to cast a value to std::u16string_view using narrow_cast.
 *
 * @param x The value to be casted.
 * @return The value casted to std::u16string_view.
 *
 * @par Example:
 * @code{.cpp}
 * std::u16string_view u16strView = NC_U16STRV(u"Hello, World!");
 * @endcode
 *
 * @see narrow_cast
 */
#define NC_U16STRV(x) narrow_cast<std::u16string_view>(x)

/**
 * @brief Macro to cast a value to std::u32string_view using narrow_cast.
 *
 * @param x The value to be casted.
 * @return The value casted to std::u32string_view.
 *
 * @par Example:
 * @code{.cpp}
 * std::u32string_view u32strView = NC_U32STRV(U"Hello, World!");
 * @endcode
 *
 * @see narrow_cast
 */
#define NC_U32STRV(x) narrow_cast<std::u32string_view>(x)

/**
 * @brief Macro to cast a value to std::size_t using narrow_cast.
 *
 * @param x The value to be casted.
 * @return The value casted to std::size_t.
 *
 * @par Example:
 * @code{.cpp}
 * std::size_t st = NC_ST(42);  // 42UZ
 * @endcode
 *
 * @see narrow_cast
 */
#define NC_ST(x) narrow_cast<std::size_t>(x)

/**
 * @brief Macro to cast a value to const uint32_t* using narrow_cast.
 *
 * @param x The value to be casted.
 * @return The value casted to const uint32_t*.
 *
 * @par Example:
 * @code{.cpp}
 * const uint32_t* cpu32t = NC_CPCU32T(&value);
 * @endcode
 *
 * @see narrow_cast
 */
#define NC_CPCU32T(x) narrow_cast<const uint32_t *>(static_cast<const void *>(x))
/** @} */  // end of NarrowTypeCastingMacros group

// NOLINTEND(*-include-cleaner, *-macro-usage)
