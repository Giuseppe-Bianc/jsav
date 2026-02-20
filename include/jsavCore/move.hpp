// NOLINTBEGIN(*-missing-std-forward, *-qualified-auto, *-implicit-bool-conversion, *-identifier-length)
#pragma once

#include "disableWarn.hpp"

DISABLE_WARNINGS_PUSH(4521 26429 26481)

namespace vnd {
    /////////////////////////////
    // remove_rvalue_reference

    /**
     * @brief Namespace containing utilities related to removing rvalue references.
     *
     * @details This namespace provides template metaprogramming utilities for
     *          manipulating rvalue references, particularly for use in move
     *          semantics operations.
     */
    namespace no_adl {
        /**
         * @brief Template struct for removing rvalue reference from a type.
         *
         * @tparam T Type from which rvalue reference needs to be removed.
         *
         * @details This primary template handles the case where T is not an
         *          rvalue reference, simply returning T unchanged.
         */
        template <typename T> struct remove_rvalue_reference {
            using type = T;  ///< Resulting type after removing rvalue reference.
        };

        /**
         * @brief Template specialization for removing rvalue reference from T&&.
         *
         * @tparam T Type with rvalue reference.
         *
         * @details This specialization handles rvalue references, removing the &&
         *          to yield the underlying type T.
         */
        template <typename T> struct remove_rvalue_reference<T &&> {
            using type = T;  ///< Resulting type after removing rvalue reference.
        };
    }  // namespace no_adl

    using no_adl::remove_rvalue_reference;  ///< Alias for easier access to remove_rvalue_reference.

    /**
     * @brief Alias template for removing rvalue reference from a type.
     *
     * @tparam T Type from which rvalue reference needs to be removed.
     *
     * @details This alias template provides a convenient shorthand for
     *          remove_rvalue_reference<T>::type, following C++14 convention
     *          for type trait aliases.
     *
     * @par Example:
     * @code{.cpp}
     * using T1 = remove_rvalue_reference_t<int&&>;  // int
     * using T2 = remove_rvalue_reference_t<int&>;   // int&
     * using T3 = remove_rvalue_reference_t<int>;    // int
     * @endcode
     */
    template <typename T> using remove_rvalue_reference_t = typename vnd::remove_rvalue_reference<T>::type;

    /**
     * @brief Checks if the provided expression is a valid identifier expression.
     *
     * @param[in] strExpr The expression to be checked (as a C-string).
     * @return true if the expression is a valid identifier expression, false otherwise.
     *
     * @details An identifier expression is considered valid if it consists of:
     *          - Alphanumeric characters (A-Z, a-z, 0-9)
     *          - Underscores (_)
     *          - Colons (:) for namespace qualifiers
     *
     * This function is used internally by move semantics macros to determine
     * if additional compile-time checks should be applied.
     *
     * @note The function is marked constexpr for compile-time evaluation.
     * @note The function is marked noexcept as it performs only character comparisons.
     */
    constexpr bool is_id_expression(char const *const strExpr) noexcept {
        for(auto it = strExpr; *it; ++it) {
            if(!('A' <= *it && *it <= 'Z') && !('a' <= *it && *it <= 'z') && !('0' <= *it && *it <= '9') && *it != '_' && *it != ':') {
                return false;
            }
        }
        return true;
    }
}  // namespace vnd

/**
 * @brief Macro to check if an expression is a valid identifier.
 *
 * @param ... The expression to check.
 * @return true if the expression is a valid identifier, false otherwise.
 *
 * @details This macro stringifies its argument and passes it to
 *          vnd::is_id_expression() for validation.
 *
 * @see vnd::is_id_expression
 */
#define vnd_is_id_expression(...) vnd::is_id_expression(#__VA_ARGS__)

/////////////////////////////////////////////
// safer variants of std::move

namespace vnd::move_detail {
    /**
     * @brief Moves the given object into a new context, allowing efficient transfer of its resources.
     *
     * @tparam Decltype The type of the destination object. Use `auto` to deduce the type.
     * @tparam bIsIdExpression A boolean indicating whether the provided object is an identifier expression.
     * @param[in] obj The object to be moved.
     * @return decltype(auto) Returns an rvalue reference to the moved object.
     *
     * @details This function is similar to std::move, but includes additional compile-time
     *          checks and assertions via static_assert to prevent common move-related errors:
     *          - Prevents moving from const objects
     *          - Prevents unnecessary moves from rvalues
     *          - Warns about moving from lvalue references
     *          - Enforces correct usage patterns for member access
     *
     * @pre The object must not be const.
     * @pre The object must not already be an rvalue.
     * @pre If Decltype is an lvalue reference, bIsIdExpression must be true.
     *
     * @throws std::logic_error If the object is const ("Cannot move out of const.").
     * @throws std::logic_error If the object is already an rvalue ("Unnecessary move; already an rvalue.").
     * @throws std::logic_error If attempting to move from an lvalue reference.
     * @throws std::logic_error If bIsIdExpression is false with guidance on correct usage.
     *
     * @note This function should be used to perform move operations in contexts where
     *       strict conditions need to be enforced.
     *
     * @par Example:
     * @code{.cpp}
     * MyClass obj;
     * auto ptr = std::make_unique<MyClass>(vnd::move_detail::move<decltype(obj), true>(obj));
     * @endcode
     *
     * @see vnd_move_always
     * @see std::move
     */
    template <typename Decltype, bool bIsIdExpression> constexpr decltype(auto) move(auto &&obj) noexcept {
        static_assert(!std::is_const<std::remove_reference_t<decltype(obj)>>::value, "Cannot move out of const.");
        static_assert(!std::is_rvalue_reference<decltype(obj)>::value, "Unnecessary move; already an rvalue.");

        static_assert(!std::is_lvalue_reference<Decltype>::value,
                      "Are you sure you want to move out of an lvalue reference? Then use vnd_move_always.");
        static_assert(bIsIdExpression, "Instead of `vnd_move(obj.member)`, use `vnd_move(obj).member`. Instead of `vnd_move(ptr->member)`, "
                                       "use `vnd_move_always(ptr->member)`.");

        return static_cast<std::remove_reference_t<Decltype> &&>(obj);
    }

    namespace no_adl {

        /**
         * @brief A utility for conditionally moving or forwarding an object.
         *
         * @tparam Decltype The type of the object to be moved or forwarded.
         * @tparam bIsIdExpression A boolean indicating whether the object is an id expression.
         *
         * @details This template struct provides type manipulation for safe move operations,
         *          ensuring correct handling of lvalue references vs. other expressions.
         */
        template <typename Decltype, bool bIsIdExpression> struct move_if_owned final {
            /**
             * @brief Static assertion to ensure correct usage.
             *
             * @details Ensures that the macro is used correctly, providing guidance
             *          for member access patterns.
             */
            static_assert(std::is_reference<Decltype>::value || bIsIdExpression,
                          "Instead of `vnd_move_if_owned(obj.member)`, use `vnd_move_if_owned(obj).member`.");

            /**
             * @brief Alias for the type after moving.
             *
             * @details Defines the resulting type as an rvalue reference to Decltype.
             */
            using type = Decltype &&;
        };

        /**
         * @brief A utility for conditionally forwarding a constant object.
         *
         * @tparam Decltype The type of the object to be forwarded.
         * @tparam bIsIdExpression A boolean indicating whether the object is an id expression.
         *
         * @details This template struct provides type manipulation for const-preserving
         *          forwarding operations, handling lvalue references, rvalue references,
         *          and non-reference types appropriately.
         */
        template <typename Decltype, bool bIsIdExpression> struct const_forward final {
            static_assert(std::is_reference<Decltype>::value || bIsIdExpression,
                          "Instead of `vnd_const_forward(obj.member)`, use `vnd_const_forward(obj).member`.");

            /**
             * @brief Alias for the type after forwarding.
             *
             * @details Defines the resulting type based on the input:
             *          - Lvalue references remain lvalue references
             *          - Other types become const rvalue references
             */
            using type =
                std::conditional_t<std::is_lvalue_reference<Decltype>::value, Decltype, vnd::remove_rvalue_reference_t<Decltype> const &&>;
        };
    }  // namespace no_adl

}  // namespace vnd::move_detail

// T -> T&& (e.g. local variable)
// T& -> error (e.g. lvalue reference argument)
// T&& -> T&& (e.g. rvalue reference argument)
/**
 * @brief Macro for safe move operations with compile-time checks.
 *
 * @param ... The object to move.
 * @return An rvalue reference to the moved object.
 *
 * @details This macro wraps vnd::move_detail::move() with automatic type deduction
 *          and identifier expression checking. It enforces:
 *          - Cannot move from const objects
 *          - Cannot move from rvalues (unnecessary)
 *          - Cannot move from lvalue references (use vnd_move_always instead)
 *          - Correct usage for member access (vnd_move(obj).member not vnd_move(obj.member))
 *
 * @par Example:
 * @code{.cpp}
 * MyClass obj;
 * auto ptr = std::make_unique<MyClass>(vnd_move(obj));
 * @endcode
 *
 * @see vnd_move_always
 * @see vnd_move_if_owned
 */
#define vnd_move(...)                                                                                                                      \
    (static_cast<decltype(vnd::move_detail::move<decltype(__VA_ARGS__), vnd_is_id_expression(__VA_ARGS__)>(__VA_ARGS__))>(__VA_ARGS__))

// T -> T&& (e.g. local variable)
// T& -> T& (e.g. lvalue reference argument)
// T&& -> T&& (e.g. rvalue reference argument)
/**
 * @brief Macro for conditional move operations based on ownership.
 *
 * @param ... The object to potentially move.
 * @return An rvalue reference if the object is owned, otherwise an lvalue reference.
 *
 * @details This macro provides safe conditional move semantics:
 *          - For local variables (T): moves (T -> T&&)
 *          - For lvalue reference arguments (T&): forwards as lvalue (T& -> T&)
 *          - For rvalue reference arguments (T&&): forwards as rvalue (T&& -> T&&)
 *
 * @par Example:
 * @code{.cpp}
 * template<typename T>
 * void wrapper(T&& value) {
 *     process(vnd_move_if_owned(value));
 * }
 * @endcode
 *
 * @see vnd_move
 * @see vnd_move_always
 */
#define vnd_move_if_owned(...)                                                                                                             \
    (static_cast<typename vnd::move_detail::no_adl::move_if_owned<decltype(__VA_ARGS__), vnd_is_id_expression(__VA_ARGS__)>::type>(        \
        __VA_ARGS__))

// MSVC sometimes forgets a const when decltyping.
/**
 * @brief Macro for conditional move with explicit type specification (MSVC workaround).
 *
 * @param Type The explicit type to use for the move operation.
 * @param ... The object to potentially move.
 * @return An rvalue reference to the moved object.
 *
 * @details This macro is a workaround for MSVC compiler behavior where decltype()
 *          may incorrectly drop const qualifiers. It allows explicit specification
 *          of the type for the move operation.
 *
 * @par Example:
 * @code{.cpp}
 * auto result = vnd_move_if_owned_msvc_workaround(const MyClass&, obj);
 * @endcode
 *
 * @see vnd_move_if_owned
 */
#define vnd_move_if_owned_msvc_workaround(Type, ...)                                                                                       \
    (static_cast<typename vnd::move_detail::no_adl::move_if_owned<Type, vnd_is_id_expression(__VA_ARGS__)>::type>(__VA_ARGS__))

/**
 * @brief Moves the input value, asserting that it is not const.
 *
 * @tparam T The type of the input value.
 * @param[in] t The input value to be moved.
 * @return A rvalue reference to the input value.
 *
 * @details This function is similar to std::move but includes a static assertion
 *          to ensure that the input value is not const. This is useful for
 *          enforcing move semantics in a context where constness should be
 *          checked at compile time.
 *
 * @note The function is marked as constexpr and noexcept.
 * @note Throws static_assert if the input value is const.
 *
 * @par Example:
 * @code{.cpp}
 * MyClass obj;
 * auto ptr = std::make_unique<MyClass>(vnd_move_always(obj));
 * @endcode
 */
template <typename T> [[nodiscard]] constexpr std::remove_reference_t<T> &&vnd_move_always(T &&t) noexcept {
    static_assert(!std::is_const<std::remove_reference_t<T>>::value, "Cannot move out of const.");
    return static_cast<std::remove_reference_t<T> &&>(t);
}

/**
 * @brief Moves the input value, even if it is const.
 *
 * @tparam T The type of the input value.
 * @param[in] t The input value to be moved.
 * @return A rvalue reference to the input value.
 *
 * @details This function performs a move operation on the input value without
 *          checking for constness. It allows const values to be moved, which
 *          is generally not recommended but can be useful in specific scenarios
 *          where such behavior is required.
 *
 * @note The function is marked as constexpr and noexcept.
 * @warning Moving from const objects typically results in a copy, not a true move.
 *
 * @par Example:
 * @code{.cpp}
 * const MyClass obj;
 * auto ptr = std::make_unique<MyClass>(vnd_move_always_even_const(obj));
 * @endcode
 */
template <typename T> [[nodiscard]] constexpr std::remove_reference_t<T> &&vnd_move_always_even_const(T &&t) noexcept {
    return static_cast<std::remove_reference_t<T> &&>(t);
}

/**
 * @brief Forwards the input value with const qualification preserved.
 *
 * @param ... The input value to be forwarded.
 * @return The forwarded value with appropriate const qualification.
 *
 * @details This macro provides a way to forward a value while preserving constness
 *          based on the type of the input expression. It supports forwarding of:
 *          - Const rvalue references
 *          - Lvalue references
 *          - Rvalue references
 *
 * @note This macro relies on `vnd::move_detail::no_adl::const_forward` and `vnd_is_id_expression`.
 *
 * @par Example:
 * @code{.cpp}
 * template<typename T>
 * void wrapper(T&& value) {
 *     process(vnd_const_forward(value));
 * }
 * @endcode
 */
#define vnd_const_forward(...)                                                                                                             \
    (static_cast<typename vnd::move_detail::no_adl::const_forward<decltype(__VA_ARGS__), vnd_is_id_expression(__VA_ARGS__)>::type>(        \
        __VA_ARGS__))
DISABLE_WARNINGS_POP()
// NOLINTEND(*-missing-std-forward, *-qualified-auto, *-implicit-bool-conversion, *-identifier-length)
