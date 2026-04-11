/*
 * Created by gbian on 11 aprile 2026.
 * Copyright (c) 2026 All rights reserved.
 */
#pragma once

// clang-format off
#include "jsav/ast/Type.hpp"
// clang-format on

namespace jsv {

    /**
     * @brief Visitor interface for structural recursion over compound types.
     *
     * Provides a unified dispatch mechanism for traversing compound types
     * (Array, Vector) without duplicating switch-on-TypeKind logic across
     * multiple subsystems (substitution, unification, occurs-check).
     *
     * Implementations override the visit_* methods to define behavior for
     * each compound type. The default implementations are no-ops, allowing
     * subclasses to handle only the variants they care about.
     *
     * ### Usage Pattern
     *
     * @code
     * class MyVisitor : public TypeVisitor {
     * public:
     *     void visit_array(const ArrayType& arr) override {
     *         // Handle array case
     *     }
     *     void visit_vector(const VectorType& vec) override {
     *         // Handle vector case
     *     }
     * };
     *
     * MyVisitor visitor;
     * visit_type(type, visitor);  // dispatches based on type->kind()
     * @endcode
     *
     * @note This visitor operates on raw TypeBase pointers/references. It does
     *       not take ownership. The caller must ensure the type outlives the visit.
     */
    class TypeVisitor {
    public:
        virtual ~TypeVisitor() = default;

        /**
         * @brief Called when visiting an ArrayType.
         * @param arr The array type being visited.
         */
        virtual void visit_array(const ArrayType &arr) = 0;

        /**
         * @brief Called when visiting a VectorType.
         * @param vec The vector type being visited.
         */
        virtual void visit_vector(const VectorType &vec) = 0;
    };

    /**
     * @brief Dispatch a type to a TypeVisitor based on its kind.
     *
     * Calls the appropriate visit_* method on @p visitor if @p type is a
     * compound type (Array or Vector). Does nothing for primitive, custom,
     * or special types.
     *
     * @param type The type to dispatch.
     * @param visitor The visitor to invoke.
     *
     * @code
     * struct ElementCollector : TypeVisitor {
     *     std::vector<TypePtr> elements;
     *     void visit_array(const ArrayType& arr) override { elements.push_back(arr.element_type()); }
     *     void visit_vector(const VectorType& vec) override { elements.push_back(vec.element_type()); }
     * };
     *
     * ElementCollector collector;
     * visit_type(my_type, collector);
     * @endcode
     */
    void visit_type(const TypeBase &type, TypeVisitor &visitor);

    /**
     * @brief Overload for shared_ptr<TypeBase> convenience.
     * @param type Shared pointer to type to dispatch.
     * @param visitor The visitor to invoke.
     */
    void visit_type(const TypePtr &type, TypeVisitor &visitor);

}  // namespace jsv
