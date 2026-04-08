# Quickstart: Hindley-Milner Type Checker

**Feature**: 008-type-checker-constraint-solver
**Date**: 2026-04-02

## Overview

This guide demonstrates how to use the jsav type checker to transform a Raw AST into a fully typed AST with comprehensive error reporting.

---

## Basic Usage

### Type Checking a Program

```cpp
#include "jsav/typechecker/TypeChecker.hpp"
#include "jsav/ast/Program.hpp"

// Assume you have a parsed Raw AST
const jsv::Program& raw_ast = /* from parser */;

// Create type checker and run
jsv::TypeChecker checker;
auto result = checker.check(raw_ast);
auto& [typed_ast, errors] = result;

// Check for errors
if (errors.empty()) {
    // Success! Access the fully typed AST
    for (const auto& stmt : typed_ast.statements()) {
        LINFO("Statement type: {}", stmt->node_type()->to_string());
    }
} else {
    // Handle errors
    for (const auto& error : errors) {
        LERROR("[{}] {} at {}:{}",
               error.code(),
               error.message(),
               error.span().start().line(),
               error.span().start().column());
    }
}
```

---

## Working with Types

### Inspecting Expression Types

```cpp
// Get type of any typed expression
void inspect_expr(const jsv::TypedExpr& expr) {
    const jsv::TypePtr& type = expr.node_type();
    
    if (type->is_integer()) {
        LINFO("Integer type: {}", jsv::type_kind_name(type->kind()));
    } else if (type->is_floating_point()) {
        LINFO("Float type: {}", jsv::type_kind_name(type->kind()));
    } else if (type->kind() == jsv::TypeKind::Bool) {
        LINFO("Boolean type");
    }
}
```

### Checking Type Equality

```cpp
// Types can be compared structurally
bool same_type(const jsv::TypePtr& t1, const jsv::TypePtr& t2) {
    if (t1->kind() != t2->kind()) return false;
    
    // For primitive types, kind equality is sufficient
    if (t1->is_primitive()) return true;
    
    // For compound types, check element types
    if (t1->kind() == jsv::TypeKind::Array) {
        const auto* arr1 = jsv::type_cast<jsv::ArrayType>(t1.get());
        const auto* arr2 = jsv::type_cast<jsv::ArrayType>(t2.get());
        return same_type(arr1->element_type(), arr2->element_type());
    }
    
    return false;
}
```

---

## Error Handling

### Processing Type Errors

```cpp
void process_errors(const std::vector<jsv::CompileError>& errors) {
    for (const auto& error : errors) {
        // Error code categories
        switch (error.code()) {
        case jsv::ErrorCode::E2034:  // Type mismatch
            LERROR("Type mismatch: expected {}, found {}",
                   error.expected_type(),
                   error.actual_type());
            break;
            
        case jsv::ErrorCode::E2035:  // Recursive type
            LERROR("Recursive type detected - infinite type");
            break;
            
        case jsv::ErrorCode::E2036:  // Unresolved type variable
            LERROR("Could not infer type for expression");
            break;
            
        default:
            LERROR("{}", error.message());
        }
        
        // Source location
        const auto& span = error.span();
        LERROR("  at line {}, column {}", 
               span.start().line(), 
               span.start().column());
        
        // Fix suggestion (if available)
        if (error.has_suggestion()) {
            LINFO("  suggestion: {}", error.suggestion());
        }
    }
}
```

---

## Common Scenarios

### Scenario 1: Well-Typed Arithmetic

```cpp
// Input: var x: i32 = 1 + 2;
// After type checking:
//   - BinaryExpr(+) has type i32
//   - IntegerLiteral(1) has type i32
//   - IntegerLiteral(2) has type i32
//   - VarDecl(x) has type i32

TEST_CASE("TypeChecker_Arithmetic_InfersCorrectTypes", "[typechecker]") {
    auto program = make_program({
        make_var_decl("x", "i32", make_binary(
            BinaryOp::Add,
            make_int_literal(1),
            make_int_literal(2)
        ))
    });
    
    jsv::TypeChecker checker;
    auto result = checker.check(*program);
    auto& [typed_ast, errors] = result;

    REQUIRE(errors.empty());
    REQUIRE(typed_ast.statements().size() == 1);

    const auto& var_decl = node_cast<TypedVarDecl>(
        typed_ast.statements()[0].get());
    REQUIRE(var_decl.node_type()->kind() == TypeKind::I32);
}
```

### Scenario 2: Type Mismatch Error

```cpp
// Input: var x: i32 = "hello";
// Error: E2034 - cannot assign string to i32

TEST_CASE("TypeChecker_TypeMismatch_ReportsError", "[typechecker]") {
    auto program = make_program({
        make_var_decl("x", "i32", make_string_literal("hello"))
    });

    jsv::TypeChecker checker;
    auto result = checker.check(*program);
    auto& [typed_ast, errors] = result;

    REQUIRE(errors.size() == 1);
    REQUIRE(errors[0].code() == ErrorCode::E2034);
    REQUIRE(errors[0].message().contains("i32"));
    REQUIRE(errors[0].message().contains("string"));
}
```

### Scenario 3: Function Type Checking

```cpp
// Input: fn add(a: i32, b: i32) -> i32 { return a + b; }
// After type checking:
//   - Function has type (i32, i32) -> i32
//   - Return expression has type i32 (matches declaration)

TEST_CASE("TypeChecker_Function_InfersReturnType", "[typechecker]") {
    auto program = make_program({
        make_func_decl("add",
            {{"a", "i32"}, {"b", "i32"}},
            "i32",
            make_block({
                make_return(make_binary(
                    BinaryOp::Add,
                    make_identifier("a"),
                    make_identifier("b")
                ))
            })
        )
    });

    jsv::TypeChecker checker;
    auto result = checker.check(*program);
    auto& [typed_ast, errors] = result;

    REQUIRE(errors.empty());
}
```

### Scenario 4: Polymorphic Function Instantiation

```cpp
// Input:
//   fn id<T>(x: T) -> T { return x; }
//   var a: i32 = id(42);
//   var b: string = id("hello");
// Each call site gets fresh type variable instantiation

TEST_CASE("TypeChecker_Polymorphic_InstantiatesPerCallSite", "[typechecker]") {
    auto program = make_program({
        make_generic_func_decl("id", {"T"},
            {{"x", "T"}},
            "T",
            make_block({make_return(make_identifier("x"))})
        ),
        make_var_decl("a", "i32",
            make_call(make_identifier("id"), {make_int_literal(42)})),
        make_var_decl("b", "string",
            make_call(make_identifier("id"), {make_string_literal("hello")}))
    });

    jsv::TypeChecker checker;
    auto result = checker.check(*program);
    auto& [typed_ast, errors] = result;

    REQUIRE(errors.empty());
    // Both call sites resolved independently
}
```

---

## Traversing the Typed AST

### Using the TypedVisitor Pattern

```cpp
#include "jsav/ast/TypedVisitor.hpp"

class TypePrinter : public jsv::TypedVisitor {
public:
    void visit(const jsv::TypedIntegerLiteral& lit) override {
        LINFO("Integer {} : {}", lit.value(), 
              jsv::type_kind_name(lit.node_type()->kind()));
    }
    
    void visit(const jsv::TypedBinaryExpr& bin) override {
        LINFO("Binary {} : {}", jsv::binary_op_symbol(bin.op()),
              jsv::type_kind_name(bin.node_type()->kind()));
        bin.lhs().accept(*this);
        bin.rhs().accept(*this);
    }
    
    // ... other visit methods
};

// Usage
void print_types(const jsv::TypedProgram& program) {
    TypePrinter printer;
    for (const auto& stmt : program.statements()) {
        stmt->accept(printer);
    }
}
```

---

## Debug Logging

The type checker integrates with spdlog for diagnostics:

```cpp
// Set log level before type checking
spdlog::set_level(spdlog::level::trace);

// Now type checker will log:
// [TRACE] Generating constraint C1: ?T1 = i32 (from integer literal)
// [TRACE] Generating constraint C2: ?T2 = i32 (from integer literal)
// [TRACE] Generating constraint C3: ?T3 = ?T1 (from binary +)
// [TRACE] Unifying C1: ?T1 = i32 → binding ?T1 to i32
// [DEBUG] Solved 3 constraints with 1 error
// [INFO]  Type checking complete: 0 errors
```

---

## Performance Considerations

### Memory Usage

- Type checker creates TypedAST nodes incrementally
- Union-find uses O(n) memory where n = number of type variables
- Substitution map is sparse (only bindings stored)

### Scaling

- 10K AST nodes: < 1 second
- 100K constraints: < 50MB memory
- Performance degrades gracefully for larger inputs

---

## Next Steps

1. **Run tests**: `ninja tests && ctest -R typechecker`
2. **Check coverage**: Verify all NodeKind variants are handled
3. **Review errors**: Ensure all E2xxx codes are documented
4. **Integration**: Connect to code generator for downstream compilation
