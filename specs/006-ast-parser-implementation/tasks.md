# Tasks: AST Parser Implementation

**Input**: Design documents from `/specs/006-ast-parser-implementation/`
**Prerequisites**: plan.md (required), spec.md (required for user stories), research.md, data-model.md, quickstart.md

**Tests**: Tests are INCLUDED per spec.md testing requirements. Three-target Catch2 approach (constexpr_tests, relaxed_constexpr_tests, tests) with ≥80% line coverage, ≥70% branch coverage.

**Organization**: Tasks are grouped by user story to enable independent implementation and testing of each story.

## Format: `[ID] [P?] [Story] Description`

- **[P]**: Can run in parallel (different files, no dependencies)
- **[Story]**: Which user story this task belongs to (e.g., US1, US2, US3, US4)
- Include exact file paths in descriptions

## Path Conventions

- **Headers**: `include/jsav/parser/` (public API, installed)
- **Implementation**: `src/jsav_Lib/parser/` (linked into jsav_lib static library)
- **Tests**: `test/` (integrated into existing test targets)

---

## Phase 1: Setup (Shared Infrastructure)

**Purpose**: Project initialization and parser module structure

- [ ] T001 Create parser module directory structure: `include/jsav/parser/` and `src/jsav_Lib/parser/`
- [ ] T002 Add parser module headers to `include/jsav/headers.hpp` master include
- [ ] T003 [P] Update CMakeLists.txt to include parser source files in jsav_lib target
- [ ] T004 [P] Configure parser module installation rules in root CMakeLists.txt

---

## Phase 2: Foundational (Blocking Prerequisites)

**Purpose**: Core infrastructure that MUST be complete before ANY user story can be implemented

**⚠️ CRITICAL**: No user story work can begin until this phase is complete

### Base Types and AST Infrastructure

- [ ] T005 [P] Create `include/jsav/parser/NodeKind.hpp` with NodeKind enum (27 node types + Program)
- [ ] T006 [P] Create `include/jsav/parser/SourceLocation.hpp` struct with file, line, column, length fields
- [ ] T007 [P] Create `include/jsav/parser/Node.hpp` abstract base class with kind(), location(), virtual destructor
- [ ] T008 [P] Create `include/jsav/parser/Expr.hpp` base class for expressions (inherits Node)
- [ ] T009 [P] Create `include/jsav/parser/Stmt.hpp` base class for statements (inherits Node)
- [ ] T010 [P] Create `include/jsav/parser/Type.hpp` with Type enum/class for type annotations (i32, f64, string, etc.)
- [ ] T011 [P] Create `include/jsav/parser/CompileError.hpp` with error code, message, SourceLocation, help text
- [ ] T012 [P] Create `include/jsav/parser/AST.hpp` master include for all AST node types

**Checkpoint**: Foundation ready - user story implementation can now begin in parallel

---

## Phase 3: User Story 1 - Parse Complete Program into AST (Priority: P1) 🎯 MVP

**Goal**: Transform a token stream from the Lexer into a typed Abstract Syntax Tree with a Program node as the root, containing all top-level declarations in correct order.

**Independent Test**: Given a valid token stream representing a complete program with declarations and statements, the parser produces a well-formed AST with a Program node as root, and the AST structure correctly reflects the source code's syntactic relationships. Can be tested with simple programs containing only variable declarations and print statements.

### Tests for User Story 1 ⚠️

> **NOTE: Write these tests FIRST, ensure they FAIL before implementation**

- [ ] T013 [P] [US1] Add constexpr test in `test/constexpr_tests.cpp`: Verify Program node construction compiles with STATIC_REQUIRE
- [ ] T014 [P] [US1] Add relaxed_constexpr test in `test/constexpr_tests.cpp`: Verify Program node with empty declarations list
- [ ] T015 [P] [US1] Add runtime test in `test/tests.cpp`: `Parser_EmptyInput_ReturnsEmptyProgram` - empty token stream produces valid empty Program node
- [ ] T016 [P] [US1] Add runtime test in `test/tests.cpp`: `Parser_SingleDeclaration_ReturnsProgramWithOneDeclaration` - single var declaration parsed correctly
- [ ] T017 [US1] Add runtime test in `test/tests.cpp`: `Parser_MultipleDeclarations_PreservesOrder` - multiple declarations appear in correct order in Program node
- [ ] T018 [US1] Add runtime test in `test/tests.cpp`: `Parser_NestedBlocks_CorrectHierarchy` - function containing block containing statement has correct parent-child relationships

### Implementation for User Story 1

#### Program Node and Expression Statement

- [ ] T019 [P] [US1] Create `include/jsav/parser/Program.hpp` with Program class containing `std::vector<StmtPtr> declarations_`
- [ ] T020 [P] [US1] Create `include/jsav/parser/ExprStmt.hpp` with ExprStmt class containing `ExprPtr expression_`
- [ ] T021 [P] [US1] Implement Program constructor and accessors in `src/jsav_Lib/parser/Program.cpp`
- [ ] T022 [P] [US1] Implement ExprStmt constructor and accessors in `src/jsav_Lib/parser/ExprStmt.cpp`

#### Parser Orchestrator - Core Infrastructure

- [ ] T023 [P] [US1] Create `include/jsav/parser/Parser.hpp` with Parser class declaration:
  - Members: `std::string_view source_`, `const std::vector<Token>* tokens_`, `std::size_t current_`, `std::vector<CompileError> errors_`, `bool panic_mode_`
  - Token navigation: `advance()`, `peek(offset)`, `check(TokenKind)`, `match(TokenKind)`, `expect(TokenKind, message)`
  - Error handling: `report_error(ErrorCode, message, SourceLocation, help)`
  - Main entry: `parse() -> std::pair<NodePtr, std::vector<CompileError>>`
- [ ] T024 [P] [US1] Create `include/jsav/parser/ParserContext.hpp` with Context enum (Global, Function, Loop, Block) and ContextGuard RAII class
- [ ] T025 [US1] Implement Parser token navigation methods in `src/jsav_Lib/parser/Parser.cpp`
- [ ] T026 [US1] Implement Parser error handling methods in `src/jsav_Lib/parser/Parser.cpp`
- [ ] T027 [US1] Implement Parser context management (push_context, pop_context, is_in_loop_context, is_in_function_context) in `src/jsav_Lib/parser/Parser.cpp`
- [ ] T028 [US1] Implement Parser::parse() main entry point that returns empty Program for empty input
- [ ] T029 [US1] Implement Parser::parse_statement() dispatch function with stub for expression_statement

#### Integration and Testing

- [ ] T030 [US1] Update `test/tests.cpp` includes to import parser headers
- [ ] T031 [US1] Add test helper function to create token streams from source strings in `test/testsConstanst.hpp`
- [ ] T032 [US1] Run all User Story 1 tests and verify they pass
- [ ] T033 [US1] Verify gcovr coverage for User Story 1 files (target ≥80% line, ≥70% branch)

**Checkpoint**: At this point, User Story 1 should be fully functional and testable independently - can parse empty programs and programs with simple expression statements

---

## Phase 4: User Story 2 - Parse All Expression Types with Correct Precedence (Priority: P2)

**Goal**: Correctly interpret all 16 expression types (literals, identifiers, unary/binary/ternary operators, function calls, indexing, member access, assignments, casts, arrays, groupings) while respecting operator precedence and associativity rules.

**Independent Test**: Given token streams representing expressions with mixed operators, the produced AST groups sub-expressions according to standard precedence rules (e.g., `1 + 2 * 3` parses as `1 + (2 * 3)`), and associativity is correct for operators at the same precedence level (e.g., `1 - 2 - 3` parses as `(1 - 2) - 3`, `a = b = c` parses as `a = (b = c)`).

### Tests for User Story 2 ⚠️

> **NOTE: Write these tests FIRST, ensure they FAIL before implementation**

#### Constexpr Tests (Binding Power Verification)

- [ ] T034 [P] [US2] Add constexpr test in `test/constexpr_tests.cpp`: `BindingPower_MultiplicationHigherThanAddition` - STATIC_REQUIRE mul BP > add BP
- [ ] T035 [P] [US2] Add constexpr test in `test/constexpr_tests.cpp`: `BindingPower_LeftAssociativeBinary` - STATIC_REQUIRE right_bp = left_bp + 1 for levels 7-0
- [ ] T036 [P] [US2] Add constexpr test in `test/constexpr_tests.cpp`: `BindingPower_RightAssociativeAssignment` - STATIC_REQUIRE assignment right_bp = -1, left_bp = -2
- [ ] T037 [P] [US2] Add constexpr test in `test/constexpr_tests.cpp`: `BindingPower_PostfixOperators` - STATIC_REQUIRE postfix BP has right_bp = INT_MAX

#### Runtime Tests - Literals

- [ ] T038 [P] [US2] Add runtime test in `test/tests.cpp`: `Parser_IntegerLiteral_ParsesCorrectly` - test various integer formats (decimal, hex, binary)
- [ ] T039 [P] [US2] Add runtime test in `test/tests.cpp`: `Parser_FloatLiteral_ParsesCorrectly` - test decimal and scientific notation
- [ ] T040 [P] [US2] Add runtime test in `test/tests.cpp`: `Parser_StringLiteral_ParsesWithEscapes` - test escape sequences (\n, \t, \\, \")
- [ ] T041 [P] [US2] Add runtime test in `test/tests.cpp`: `Parser_BoolLiteral_ParsesTrueAndFalse`
- [ ] T042 [P] [US2] Add runtime test in `test/tests.cpp`: `Parser_NullLiteral_ParsesCorrectly`

#### Runtime Tests - Operator Precedence

- [ ] T043 [P] [US2] Add runtime test in `test/tests.cpp`: `Parser_Precedence_MultiplicationBeforeAddition` - `1 + 2 * 3` groups as `1 + (2 * 3)`
- [ ] T044 [P] [US2] Add runtime test in `test/tests.cpp`: `Parser_Precedence_AdditionBeforeComparison` - `1 + 2 > 3 * 4` groups correctly
- [ ] T045 [P] [US2] Add runtime test in `test/tests.cpp`: `Parser_Associativity_LeftAssociativeSubtraction` - `1 - 2 - 3` groups as `(1 - 2) - 3`
- [ ] T046 [P] [US2] Add runtime test in `test/tests.cpp`: `Parser_Associativity_RightAssociativeAssignment` - `a = b = c` groups as `a = (b = c)`
- [ ] T047 [P] [US2] Add runtime test in `test/tests.cpp`: `Parser_Associativity_RightAssociativeTernary` - `a ? b : c ? d : e` groups correctly

#### Runtime Tests - Expression Types

- [ ] T048 [P] [US2] Add runtime test in `test/tests.cpp`: `Parser_UnaryExpr_ParsesNegation` - `-x`
- [ ] T049 [P] [US2] Add runtime test in `test/tests.cpp`: `Parser_UnaryExpr_ParsesLogicalNot` - `!flag`
- [ ] T050 [P] [US2] Add runtime test in `test/tests.cpp`: `Parser_BinaryExpr_ParsesAllOperators` - test +, -, *, /, %, &, |, ^, <<, >>, &&, ||
- [ ] T051 [P] [US2] Add runtime test in `test/tests.cpp`: `Parser_CallExpr_ParsesWithArguments` - `func(arg1, arg2)`
- [ ] T052 [P] [US2] Add runtime test in `test/tests.cpp`: `Parser_IndexExpr_ParsesArrayAccess` - `array[index]`
- [ ] T053 [P] [US2] Add runtime test in `test/tests.cpp`: `Parser_MemberExpr_ParsesMemberAccess` - `object.member`
- [ ] T054 [P] [US2] Add runtime test in `test/tests.cpp`: `Parser_TernaryExpr_ParsesConditional` - `cond ? thenVal : elseVal`
- [ ] T055 [P] [US2] Add runtime test in `test/tests.cpp`: `Parser_AssignExpr_ParsesAssignment` - `x = value`
- [ ] T056 [P] [US2] Add runtime test in `test/tests.cpp`: `Parser_CompoundAssignExpr_ParsesCompoundAssignment` - `x += value`, `x -= value`, etc.
- [ ] T057 [P] [US2] Add runtime test in `test/tests.cpp`: `Parser_CastExpr_ParsesTypeCast` - `i32(x)`, `float(value)`
- [ ] T058 [P] [US2] Add runtime test in `test/tests.cpp`: `Parser_ArrayLiteral_ParsesElements` - `[1, 2, 3]`
- [ ] T059 [P] [US2] Add runtime test in `test/tests.cpp`: `Parser_GroupingExpr_ParsesParentheses` - `(1 + 2) * 3`

### Implementation for User Story 2

#### Expression Node Classes (All 16 Types)

- [ ] T060 [P] [US2] Create `include/jsav/parser/IntegerLiteral.hpp` with IntegerLiteral class (std::int64_t value_)
- [ ] T061 [P] [US2] Create `include/jsav/parser/FloatLiteral.hpp` with FloatLiteral class (double value_)
- [ ] T062 [P] [US2] Create `include/jsav/parser/StringLiteral.hpp` with StringLiteral class (std::string value_)
- [ ] T063 [P] [US2] Create `include/jsav/parser/BoolLiteral.hpp` with BoolLiteral class (bool value_)
- [ ] T064 [P] [US2] Create `include/jsav/parser/NullLiteral.hpp` with NullLiteral class
- [ ] T065 [P] [US2] Create `include/jsav/parser/Identifier.hpp` with Identifier class (std::string name_)
- [ ] T066 [P] [US2] Create `include/jsav/parser/UnaryExpr.hpp` with UnaryExpr class (TokenKind op_, ExprPtr operand_)
- [ ] T067 [P] [US2] Create `include/jsav/parser/BinaryExpr.hpp` with BinaryExpr class (ExprPtr left_, TokenKind op_, ExprPtr right_)
- [ ] T068 [P] [US2] Create `include/jsav/parser/TernaryExpr.hpp` with TernaryExpr class (ExprPtr condition_, thenBranch_, elseBranch_)
- [ ] T069 [P] [US2] Create `include/jsav/parser/CallExpr.hpp` with CallExpr class (ExprPtr callee_, std::vector<ExprPtr> arguments_)
- [ ] T070 [P] [US2] Create `include/jsav/parser/IndexExpr.hpp` with IndexExpr class (ExprPtr array_, ExprPtr index_)
- [ ] T071 [P] [US2] Create `include/jsav/parser/MemberExpr.hpp` with MemberExpr class (ExprPtr object_, std::string memberName_)
- [ ] T072 [P] [US2] Create `include/jsav/parser/AssignExpr.hpp` with AssignExpr class (TokenKind op_, ExprPtr target_, ExprPtr value_)
- [ ] T073 [P] [US2] Create `include/jsav/parser/CastExpr.hpp` with CastExpr class (Type targetType_, ExprPtr expression_)
- [ ] T074 [P] [US2] Create `include/jsav/parser/ArrayLiteral.hpp` with ArrayLiteral class (std::vector<ExprPtr> elements_)
- [ ] T075 [P] [US2] Create `include/jsav/parser/GroupingExpr.hpp` with GroupingExpr class (ExprPtr expression_)

#### Expression Node Implementations

- [ ] T076 [P] [US2] Implement all 16 expression node constructors and accessors in `src/jsav_Lib/parser/expressions/` (one .cpp file per node type)
- [ ] T077 [US2] Add all expression node .cpp files to CMakeLists.txt in `src/jsav_Lib/parser/CMakeLists.txt`

#### ExpressionParser Module (Pratt Parsing)

- [ ] T078 [P] [US2] Create `include/jsav/parser/BindingPower.hpp` with BindingPower struct (left_bp, right_bp as int)
- [ ] T079 [P] [US2] Create `include/jsav/parser/ExpressionParser.hpp` with ExpressionParser class:
  - Members: `Parser& parser_`, `static constexpr std::array<BindingPower, 256> binding_powers_`
  - Prefix parse table: `std::unordered_map<TokenKind, prefix_parser_fn> prefix_parse_table_`
  - Infix parse table: `std::unordered_map<TokenKind, infix_parser_fn> infix_parse_table_`
  - Main method: `parse_expression(int min_precedence = -2)`
  - Prefix parsers: `parse_prefix()`, `parse_literal()`, `parse_identifier()`, `parse_unary()`
  - Infix parsers: `parse_infix(left, left_bp)`, `parse_binary(left, left_bp)`, `parse_call(left)`, `parse_index(left)`, `parse_member(left)`, `parse_assignment(left)`, `parse_ternary(left)`
- [ ] T080 [US2] Implement binding power table in `src/jsav_Lib/parser/ExpressionParser.cpp` with all 12 precedence levels per research.md Decision 6:
  - Level 10: (), [], . (postfix, left_bp=10, right_bp=INT_MAX)
  - Level 9: ++, -- postfix (left_bp=9, right_bp=INT_MAX)
  - Level 8: -, !, ~, ++, -- prefix (right_bp=8, prefix-only)
  - Level 7: *, /, % (left_bp=7, right_bp=8)
  - Level 6: +, - (left_bp=6, right_bp=7)
  - Level 5: <<, >> (left_bp=5, right_bp=6)
  - Level 4: <, >, <=, >= (left_bp=4, right_bp=5)
  - Level 3: ==, != (left_bp=3, right_bp=4)
  - Level 2: &, ^, | (left_bp=2, right_bp=3)
  - Level 1: && (left_bp=1, right_bp=2)
  - Level 0: || (left_bp=0, right_bp=1)
  - Level -1: ? : (left_bp=-1, right_bp=0)
  - Level -2: =, +=, -=, etc. (left_bp=-2, right_bp=-1)
- [ ] T081 [US2] Implement ExpressionParser constructor with prefix_parse_table_ and infix_parse_table_ registration
- [ ] T082 [US2] Implement `parse_expression(min_precedence)` main Pratt loop in `src/jsav_Lib/parser/ExpressionParser.cpp`
- [ ] T083 [US2] Implement all prefix parser functions (parse_literal, parse_identifier, parse_unary, parse_grouping)
- [ ] T084 [US2] Implement all infix parser functions (parse_binary, parse_call, parse_index, parse_member, parse_assignment, parse_ternary)
- [ ] T085 [US2] Implement postfix operator handling (parse_call, parse_index, parse_member, parse_postfix_incdec)

#### Parser Integration

- [ ] T086 [US2] Update `include/jsav/parser/Parser.hpp` to include ExpressionParser and add `ExpressionParser expression_parser_` member
- [ ] T087 [US2] Update Parser::parse_statement() to call `expression_parser_.parse_expression()` for expression statements
- [ ] T088 [US2] Implement semicolon consumption after expression statements

#### Testing and Coverage

- [ ] T089 [US2] Run all User Story 2 tests (constexpr_tests, relaxed_constexpr_tests, tests) and verify they pass
- [ ] T090 [US2] Verify gcovr coverage for User Story 2 files (target ≥80% line, ≥70% branch)
- [ ] T091 [US2] Run AddressSanitizer and UndefinedBehaviorSanitizer tests - verify zero violations

**Checkpoint**: At this point, User Story 2 should be fully functional and testable independently - can parse all 16 expression types with correct precedence and associativity

---

## Phase 5: User Story 3 - Parse All Statement and Declaration Types (Priority: P3)

**Goal**: Correctly recognize and structure all 11 statement types (expression statements, variable declarations, function declarations, return statements, control flow: if/while/for, blocks, break/continue, print statements) with proper syntax validation.

**Independent Test**: Given token streams representing each statement type in isolation, the parser produces correctly structured AST nodes for each statement type with all required components (identifiers, type annotations, initializers, conditions, bodies).

### Tests for User Story 3 ⚠️

> **NOTE: Write these tests FIRST, ensure they FAIL before implementation**

#### Variable Declarations

- [ ] T092 [P] [US3] Add runtime test in `test/tests.cpp`: `Parser_VarDeclaration_ParsesWithInitializer` - `var x = 42`
- [ ] T093 [P] [US3] Add runtime test in `test/tests.cpp`: `Parser_ConstDeclaration_RequiresInitializer` - `const PI = 3.14`
- [ ] T094 [P] [US3] Add runtime test in `test/tests.cpp`: `Parser_TypedDeclaration_ParsesTypeAnnotation` - `var x: i32 = 42`
- [ ] T095 [P] [US3] Add runtime test in `test/tests.cpp`: `Parser_MultipleVarDeclarations_ParsesCommaSeparated` - `var x = 1, y = 2`

#### Function Declarations

- [ ] T096 [P] [US3] Add runtime test in `test/tests.cpp`: `Parser_FuncDeclaration_ParsesParametersAndBody` - `fun add(a: i32, b: i32) -> i32 { return a + b; }`
- [ ] T097 [P] [US3] Add runtime test in `test/tests.cpp`: `Parser_FuncDeclaration_ParsesWithoutReturnType` - `fun print(x) { }`
- [ ] T098 [P] [US3] Add runtime test in `test/tests.cpp`: `Parser_FuncDeclaration_ParsesUntypedParameters` - `fun add(a, b) { return a + b; }`
- [ ] T099 [P] [US3] Add runtime test in `test/tests.cpp`: `Parser_FuncDeclaration_ReportsDuplicateParameterNames` - error E0204

#### Control Flow Statements

- [ ] T100 [P] [US3] Add runtime test in `test/tests.cpp`: `Parser_IfStatement_ParsesWithElseBranch` - `if (cond) { } else { }`
- [ ] T101 [P] [US3] Add runtime test in `test/tests.cpp`: `Parser_IfStatement_ParsesWithoutElseBranch` - `if (cond) { }`
- [ ] T102 [P] [US3] Add runtime test in `test/tests.cpp`: `Parser_WhileStatement_ParsesConditionAndBody` - `while (cond) { }`
- [ ] T103 [P] [US3] Add runtime test in `test/tests.cpp`: `Parser_ForStatement_ParsesAllComponents` - `for (init; cond; inc) { }`
- [ ] T104 [P] [US3] Add runtime test in `test/tests.cpp`: `Parser_ForStatement_ParsesWithEmptyComponents` - `for (; ; ) { }`

#### Return, Break, Continue, Print

- [ ] T105 [P] [US3] Add runtime test in `test/tests.cpp`: `Parser_ReturnStatement_ParsesWithValue` - `return value;`
- [ ] T106 [P] [US3] Add runtime test in `test/tests.cpp`: `Parser_ReturnStatement_ParsesWithoutValue` - `return;`
- [ ] T107 [P] [US3] Add runtime test in `test/tests.cpp`: `Parser_BreakStatement_ParsesInLoop` - `while (true) { break; }`
- [ ] T108 [P] [US3] Add runtime test in `test/tests.cpp`: `Parser_ContinueStatement_ParsesInLoop` - `while (true) { continue; }`
- [ ] T109 [P] [US3] Add runtime test in `test/tests.cpp`: `Parser_PrintStatement_ParsesExpression` - `print x`

#### Nested Structures

- [ ] T110 [P] [US3] Add runtime test in `test/tests.cpp`: `Parser_NestedBlocks_ParsesCorrectly` - blocks within if, functions within blocks
- [ ] T111 [P] [US3] Add runtime test in `test/tests.cpp`: `Parser_NestedControlFlow_ParsesCorrectly` - if within while within for

### Implementation for User Story 3

#### Statement Node Classes (All 11 Types)

- [ ] T112 [P] [US3] Create `include/jsav/parser/VarDecl.hpp` with VarDecl class (std::string name_, std::optional<Type> type_, ExprPtr initializer_, bool isConst_)
- [ ] T113 [P] [US3] Create `include/jsav/parser/Parameter.hpp` with Parameter struct (std::string name, std::optional<Type> type, SourceLocation location)
- [ ] T114 [P] [US3] Create `include/jsav/parser/FuncDecl.hpp` with FuncDecl class (std::string name_, std::vector<Parameter> params_, std::optional<Type> returnType_, BlockStmtPtr body_)
- [ ] T115 [P] [US3] Create `include/jsav/parser/ReturnStmt.hpp` with ReturnStmt class (ExprPtr value_)
- [ ] T116 [P] [US3] Create `include/jsav/parser/IfStmt.hpp` with IfStmt class (ExprPtr condition_, StmtPtr thenBranch_, StmtPtr elseBranch_)
- [ ] T117 [P] [US3] Create `include/jsav/parser/WhileStmt.hpp` with WhileStmt class (ExprPtr condition_, StmtPtr body_)
- [ ] T118 [P] [US3] Create `include/jsav/parser/ForStmt.hpp` with ForStmt class (StmtPtr init_, ExprPtr condition_, ExprPtr increment_, StmtPtr body_)
- [ ] T119 [P] [US3] Create `include/jsav/parser/BlockStmt.hpp` with BlockStmt class (std::vector<StmtPtr> statements_)
- [ ] T120 [P] [US3] Create `include/jsav/parser/BreakStmt.hpp` with BreakStmt class
- [ ] T121 [P] [US3] Create `include/jsav/parser/ContinueStmt.hpp` with ContinueStmt class
- [ ] T122 [P] [US3] Create `include/jsav/parser/PrintStmt.hpp` with PrintStmt class (ExprPtr expression_)

#### Statement Node Implementations

- [ ] T123 [P] [US3] Implement all 11 statement node constructors and accessors in `src/jsav_Lib/parser/statements/` (one .cpp file per node type)
- [ ] T124 [US3] Add all statement node .cpp files to CMakeLists.txt in `src/jsav_Lib/parser/CMakeLists.txt`

#### StatementParser Module (Recursive Descent)

- [ ] T125 [P] [US3] Create `include/jsav/parser/StatementParser.hpp` with StatementParser class:
  - Members: `Parser& parser_`, `ExpressionParser& expression_parser_`
  - Main dispatch: `parse_statement()`
  - Declaration parsers: `parse_var_declaration()`, `parse_function_declaration()`
  - Statement parsers: `parse_return_statement()`, `parse_if_statement()`, `parse_while_statement()`, `parse_for_statement()`, `parse_block()`, `parse_break_statement()`, `parse_continue_statement()`, `parse_print_statement()`, `parse_expression_statement()`
- [ ] T126 [US3] Implement StatementParser constructor in `src/jsav_Lib/parser/StatementParser.cpp`
- [ ] T127 [US3] Implement `parse_statement()` dispatch function with TokenKind-based routing
- [ ] T128 [US3] Implement `parse_var_declaration()` with support for var/const, type annotations, comma-separated declarations, required initializer for const
- [ ] T129 [US3] Implement `parse_function_declaration()` with parameter list parsing, type annotations, optional return type, block body
- [ ] T130 [US3] Implement `parse_return_statement()` with optional expression, semicolon
- [ ] T131 [US3] Implement `parse_if_statement()` with condition in parentheses, then-branch, optional else-branch
- [ ] T132 [US3] Implement `parse_while_statement()` with condition in parentheses, body statement
- [ ] T133 [US3] Implement `parse_for_statement()` with optional init-statement, condition, increment, body statement
- [ ] T134 [US3] Implement `parse_block()` with { statement-list }
- [ ] T135 [US3] Implement `parse_break_statement()` with semicolon
- [ ] T136 [US3] Implement `parse_continue_statement()` with semicolon
- [ ] T137 [US3] Implement `parse_print_statement()` with expression, semicolon
- [ ] T138 [US3] Implement `parse_expression_statement()` with expression, semicolon

#### Context Validation

- [ ] T139 [US3] Implement ContextGuard RAII usage in StatementParser functions:
  - Function body parsing: push_context(Context::Function), automatic pop via ContextGuard
  - Loop body parsing (while, for): push_context(Context::Loop), automatic pop via ContextGuard
  - Block parsing: push_context(Context::Block), automatic pop via ContextGuard
- [ ] T140 [US3] Implement validation in `parse_break_statement()`: check `parser_.is_in_loop_context()`, report E0304 if false
- [ ] T141 [US3] Implement validation in `parse_continue_statement()`: check `parser_.is_in_loop_context()`, report E0305 if false
- [ ] T142 [US3] Implement validation in `parse_return_statement()`: check `parser_.is_in_function_context()`, report E0306 if false

#### Parser Integration

- [ ] T143 [US3] Update `include/jsav/parser/Parser.hpp` to include StatementParser and add `StatementParser statement_parser_` member
- [ ] T144 [US3] Update Parser::parse() to call `statement_parser_.parse_statement()` in loop for top-level declarations
- [ ] T145 [US3] Implement top-level parsing loop that collects declarations until EOF

#### Testing and Coverage

- [ ] T146 [US3] Run all User Story 3 tests and verify they pass
- [ ] T147 [US3] Verify gcovr coverage for User Story 3 files (target ≥80% line, ≥70% branch)
- [ ] T148 [US3] Run AddressSanitizer and UndefinedBehaviorSanitizer tests - verify zero violations

**Checkpoint**: At this point, User Stories 1, 2, AND 3 should all work independently - can parse complete programs with all expression and statement types

---

## Phase 6: User Story 4 - Collect and Report Syntax Errors with Recovery (Priority: P4)

**Goal**: Detect syntax errors (unbalanced delimiters, missing operands, invalid placements, etc.), report them with source locations and helpful messages, and continue parsing to collect additional errors rather than stopping at the first one.

**Independent Test**: Given token streams with various syntax errors, the parser reports all errors (up to the maximum limit) with accurate source locations and appropriate error codes, and continues parsing after each error to find additional issues.

### Tests for User Story 4 ⚠️

> **NOTE: Write these tests FIRST, ensure they FAIL before implementation**

#### Expression Errors

- [ ] T149 [P] [US4] Add runtime test in `test/tests.cpp`: `Parser_E0101_UnbalancedParentheses` - `(1 + 2` reports E0101 at opening paren location
- [ ] T150 [P] [US4] Add runtime test in `test/tests.cpp`: `Parser_E0102_UnbalancedBrackets` - `[1 + 2` reports E0102
- [ ] T151 [P] [US4] Add runtime test in `test/tests.cpp`: `Parser_E0103_UnbalancedBraces` - `{1 + 2` reports E0103
- [ ] T152 [P] [US4] Add runtime test in `test/tests.cpp`: `Parser_E0104_MissingBinaryOperand` - `5 +` reports E0104 at operator location
- [ ] T153 [P] [US4] Add runtime test in `test/tests.cpp`: `Parser_E0105_UnexpectedTokenInExpression` - `1 @ 2` reports E0105 (assuming @ is unknown)
- [ ] T154 [P] [US4] Add runtime test in `test/tests.cpp`: `Parser_E0106_MissingFunctionCallArguments` - `func(` without `)` reports E0106
- [ ] T155 [P] [US4] Add runtime test in `test/tests.cpp`: `Parser_E0107_MissingIndexExpression` - `arr[` without `]` reports E0107
- [ ] T156 [P] [US4] Add runtime test in `test/tests.cpp`: `Parser_E0108_MissingMemberName` - `obj.` without identifier reports E0108
- [ ] T157 [P] [US4] Add runtime test in `test/tests.cpp`: `Parser_E0109_TrailingCommaInArgs` - `func(1, 2,)` reports E0109
- [ ] T158 [P] [US4] Add runtime test in `test/tests.cpp`: `Parser_E0110_TrailingCommaInArray` - `[1, 2,]` reports E0110

#### Declaration Errors

- [ ] T159 [P] [US4] Add runtime test in `test/tests.cpp`: `Parser_E0201_MissingConstInitializer` - `const x;` reports E0201
- [ ] T160 [P] [US4] Add runtime test in `test/tests.cpp`: `Parser_E0202_InvalidVariableName` - `var 123 = 5;` reports E0202
- [ ] T161 [P] [US4] Add runtime test in `test/tests.cpp`: `Parser_E0204_DuplicateParameterNames` - `fun f(x: i32, x: i32)` reports E0204
- [ ] T162 [P] [US4] Add runtime test in `test/tests.cpp`: `Parser_E0205_MissingFunctionBody` - `fun foo();` reports E0205

#### Statement Errors

- [ ] T163 [P] [US4] Add runtime test in `test/tests.cpp`: `Parser_E0301_MissingIfCondition` - `if { }` reports E0301
- [ ] T164 [P] [US4] Add runtime test in `test/tests.cpp`: `Parser_E0302_MissingWhileCondition` - `while { }` reports E0302
- [ ] T165 [P] [US4] Add runtime test in `test/tests.cpp`: `Parser_E0303_MissingSemicolon` - `var x = 5` without `;` reports E0303
- [ ] T166 [P] [US4] Add runtime test in `test/tests.cpp`: `Parser_E0304_BreakOutsideLoop` - `break;` at top level reports E0304
- [ ] T167 [P] [US4] Add runtime test in `test/tests.cpp`: `Parser_E0305_ContinueOutsideLoop` - `continue;` at top level reports E0305
- [ ] T168 [P] [US4] Add runtime test in `test/tests.cpp`: `Parser_E0306_ReturnOutsideFunction` - `return 5;` at top level reports E0306
- [ ] T169 [P] [US4] Add runtime test in `test/tests.cpp`: `Parser_E0308_MissingPrintExpression` - `print;` reports E0308

#### Structural Errors and Error Recovery

- [ ] T170 [P] [US4] Add runtime test in `test/tests.cpp`: `Parser_E0401_UnexpectedTokenAtTopLevel` - random token at top level reports E0401
- [ ] T171 [P] [US4] Add runtime test in `test/tests.cpp`: `Parser_E0402_UnexpectedEOF` - incomplete program reports E0402
- [ ] T172 [P] [US4] Add runtime test in `test/tests.cpp`: `Parser_ErrorRecovery_MultipleErrorsCollected` - multiple syntax errors in sequence all reported (up to 100)
- [ ] T173 [P] [US4] Add runtime test in `test/tests.cpp`: `Parser_ErrorRecovery_ContinuesAfterUnbalancedParen` - `(1 + 2; var y = 5;` reports E0101 but still parses `var y = 5`
- [ ] T174 [P] [US4] Add runtime test in `test/tests.cpp`: `Parser_ErrorRecovery_SynchronizesToStatementBoundary` - error in middle of statement recovers at next `;` or `}`

### Implementation for User Story 4

#### Error Code Definitions

- [ ] T175 [P] [US4] Create `include/jsav/parser/ErrorCodes.hpp` with ErrorCode enum containing all error codes from spec.md:
  - Expression errors: E0101-E0110
  - Declaration errors: E0201-E0206
  - Statement errors: E0301-E0308
  - Structural errors: E0401-E0402
- [ ] T176 [US4] Add error code to string conversion function `errorCodeToString(ErrorCode)` in `src/jsav_Lib/parser/ErrorCodes.cpp`

#### Panic-Mode Error Recovery

- [ ] T177 [P] [US4] Implement `synchronize()` method in `src/jsav_Lib/parser/Parser.cpp`:
  - Advance tokens until synchronization point found (`;`, `}`, or statement keywords: var, fun, if, while, for, return, break, continue, print)
  - Reset panic_mode_ after synchronization
- [ ] T178 [US4] Update all error detection points to call `synchronize()` after `report_error()`
- [ ] T179 [US4] Implement panic_mode_ flag logic: skip token processing while in panic mode until synchronization point

#### Expression Error Detection (ExpressionParser)

- [ ] T180 [US4] Implement E0101 detection in `parse_grouping()`: report error if `)` not found
- [ ] T181 [US4] Implement E0102 detection in `parse_index()`: report error if `]` not found
- [ ] T182 [US4] Implement E0103 detection in block parsing: report error if `}` not found
- [ ] T183 [US4] Implement E0104 detection in binary operator parsing: report error if right operand missing
- [ ] T184 [US4] Implement E0105 detection: report error for unknown/unexpected tokens in expression context
- [ ] T185 [US4] Implement E0106 detection: report error if function call `(` without matching `)`
- [ ] T186 [US4] Implement E0107 detection: report error if index `[` without matching `]`
- [ ] T187 [US4] Implement E0108 detection: report error if member access `.` without identifier
- [ ] T188 [US4] Implement E0109 detection: report error for trailing comma in argument list before `)`
- [ ] T189 [US4] Implement E0110 detection: report error for trailing comma in array literal before `]`

#### Declaration Error Detection (StatementParser)

- [ ] T190 [US4] Implement E0201 detection in `parse_var_declaration()`: check const has initializer, report if missing
- [ ] T191 [US4] Implement E0202 detection: validate identifier after var/const keyword
- [ ] T192 [US4] Implement E0203 detection: report error for missing type annotation where required
- [ ] T193 [US4] Implement E0204 detection in `parse_function_declaration()`: check for duplicate parameter names
- [ ] T194 [US4] Implement E0205 detection: check function has block body, report if missing
- [ ] T195 [US4] Implement E0206 detection: validate return type is known type

#### Statement Error Detection (StatementParser)

- [ ] T196 [US4] Implement E0301 detection in `parse_if_statement()`: check condition present
- [ ] T197 [US4] Implement E0302 detection in `parse_while_statement()`: check condition present
- [ ] T198 [US4] Implement E0303 detection: check semicolon after statements that require it
- [ ] T199 [US4] Implement E0304 detection in `parse_break_statement()`: already implemented in US3 (T140)
- [ ] T200 [US4] Implement E0305 detection in `parse_continue_statement()`: already implemented in US3 (T141)
- [ ] T201 [US4] Implement E0306 detection in `parse_return_statement()`: already implemented in US3 (T142)
- [ ] T202 [US4] Implement E0307 detection: check for statements after return in same block
- [ ] T203 [US4] Implement E0308 detection in `parse_print_statement()`: check expression present

#### Structural Error Detection (Parser)

- [ ] T204 [US4] Implement E0401 detection in top-level parsing loop: report error for unexpected tokens
- [ ] T205 [US4] Implement E0402 detection: report error if EOF reached in unexpected state (unclosed delimiters)

#### Error Message Formatting

- [ ] T206 [P] [US4] Implement error message formatting in `CompileError` class with:
  - Error code string representation
  - Message text
  - Source location (file:line:column)
  - Optional help text with suggested fix
- [ ] T207 [US4] Add helpful error messages for each error code with typical causes and fixes

#### Testing and Coverage

- [ ] T208 [US4] Run all User Story 4 tests and verify they pass
- [ ] T209 [US4] Verify gcovr coverage for User Story 4 files (target ≥80% line, ≥70% branch)
- [ ] T210 [US4] Run AddressSanitizer and UndefinedBehaviorSanitizer tests - verify zero violations
- [ ] T211 [US4] Test error recovery with pathological inputs (many errors in sequence)

**Checkpoint**: All user stories should now be independently functional - parser detects and reports all syntax errors with recovery

---

## Phase N: Polish & Cross-Cutting Concerns

**Purpose**: Improvements that affect multiple user stories

- [ ] T212 [P] Add Doxygen documentation to all public headers in `include/jsav/parser/`:
  - Module-level documentation for Parser, ExpressionParser, StatementParser
  - Function documentation with purpose, parameters, return values, examples
  - Error code documentation with condition, typical cause, suggested fix
- [ ] T213 [P] Add usage examples to `include/jsav/parser/Parser.hpp` showing basic parsing workflow
- [ ] T214 [P] Update quickstart.md with complete examples for all user stories
- [ ] T215 [P] Code cleanup and refactoring based on clang-tidy/cppcheck suggestions
- [ ] T216 [P] Performance optimization: profile parser with large inputs (10k+ lines) using RelWithDebInfo build
- [ ] T217 [P] Add additional unit tests for edge cases not covered in user story tests
- [ ] T218 [P] Verify all lizard thresholds met (CCN ≤15, length ≤100 lines, params ≤6)
- [ ] T219 [P] Run full test suite with all sanitizers enabled - verify zero violations
- [ ] T220 [P] Generate final gcovr coverage report - verify ≥80% line, ≥70% branch coverage
- [ ] T221 [P] Update AGENTS.md and QWEN.md with parser module documentation
- [ ] T222 [P] Create parser architecture diagram showing module relationships

---

## Dependencies & Execution Order

### Phase Dependencies

- **Setup (Phase 1)**: No dependencies - can start immediately
- **Foundational (Phase 2)**: Depends on Setup completion - BLOCKS all user stories
- **User Stories (Phase 3-6)**: All depend on Foundational phase completion
  - User stories can then proceed in parallel (if staffed)
  - Or sequentially in priority order (P1 → P2 → P3 → P4)
- **Polish (Final Phase)**: Depends on all desired user stories being complete

### User Story Dependencies

- **User Story 1 (P1)**: Can start after Foundational (Phase 2) - No dependencies on other stories
  - Implements: Program node, ExprStmt, Parser orchestrator core infrastructure
  - Independent test: Empty program and simple declaration parsing
- **User Story 2 (P2)**: Can start after Foundational (Phase 2) - Independent of US1 implementation details
  - Implements: All 16 expression nodes, ExpressionParser with Pratt parsing
  - Independent test: Expression precedence and associativity with isolated expression inputs
- **User Story 3 (P3)**: Can start after Foundational (Phase 2) - Independent of US2 implementation details
  - Implements: All 11 statement nodes, StatementParser with recursive descent
  - Independent test: Each statement type parsed correctly in isolation
- **User Story 4 (P4)**: Depends on US1, US2, US3 completion (error detection requires parsing infrastructure)
  - Implements: Error detection, panic-mode recovery, error reporting
  - Independent test: Malformed inputs produce correct error codes and recovery

### Within Each User Story

- Tests (if included) MUST be written and FAIL before implementation (TDD Red-Green-Refactor)
- Models before services (node headers before implementations)
- Services before endpoints (parser modules before integration)
- Core implementation before integration
- Story complete before moving to next priority

### Parallel Opportunities

- All Setup tasks marked [P] can run in parallel (T003, T004)
- All Foundational tasks marked [P] can run in parallel (T005-T012)
- Once Foundational phase completes:
  - Developer A: User Story 1 (T013-T033)
  - Developer B: User Story 2 (T034-T091) - can proceed in parallel with US1 after T012
  - Developer C: User Story 3 (T092-T148) - can proceed in parallel after T012
- All tests for a user story marked [P] can run in parallel
- Models within a story marked [P] can run in parallel (e.g., T060-T075 for all expression nodes)
- Different user stories can be worked on in parallel by different team members

---

## Parallel Example: User Story 2

```bash
# Launch all expression node header creations together (T060-T075):
Task: "Create IntegerLiteral.hpp"
Task: "Create FloatLiteral.hpp"
Task: "Create StringLiteral.hpp"
Task: "Create BoolLiteral.hpp"
Task: "Create NullLiteral.hpp"
Task: "Create Identifier.hpp"
Task: "Create UnaryExpr.hpp"
Task: "Create BinaryExpr.hpp"
Task: "Create TernaryExpr.hpp"
Task: "Create CallExpr.hpp"
Task: "Create IndexExpr.hpp"
Task: "Create MemberExpr.hpp"
Task: "Create AssignExpr.hpp"
Task: "Create CastExpr.hpp"
Task: "Create ArrayLiteral.hpp"
Task: "Create GroupingExpr.hpp"

# Launch all binding power constexpr tests together (T034-T037):
Task: "BindingPower_MultiplicationHigherThanAddition"
Task: "BindingPower_LeftAssociativeBinary"
Task: "BindingPower_RightAssociativeAssignment"
Task: "BindingPower_PostfixOperators"

# Launch all literal parsing tests together (T038-T042):
Task: "Parser_IntegerLiteral_ParsesCorrectly"
Task: "Parser_FloatLiteral_ParsesCorrectly"
Task: "Parser_StringLiteral_ParsesWithEscapes"
Task: "Parser_BoolLiteral_ParsesTrueAndFalse"
Task: "Parser_NullLiteral_ParsesCorrectly"
```

---

## Parallel Example: User Story 3

```bash
# Launch all statement node header creations together (T112-T122):
Task: "Create VarDecl.hpp"
Task: "Create Parameter.hpp"
Task: "Create FuncDecl.hpp"
Task: "Create ReturnStmt.hpp"
Task: "Create IfStmt.hpp"
Task: "Create WhileStmt.hpp"
Task: "Create ForStmt.hpp"
Task: "Create BlockStmt.hpp"
Task: "Create BreakStmt.hpp"
Task: "Create ContinueStmt.hpp"
Task: "Create PrintStmt.hpp"

# Launch all statement parser implementations together (T128-T138):
Task: "Implement parse_var_declaration()"
Task: "Implement parse_function_declaration()"
Task: "Implement parse_return_statement()"
Task: "Implement parse_if_statement()"
Task: "Implement parse_while_statement()"
Task: "Implement parse_for_statement()"
Task: "Implement parse_block()"
Task: "Implement parse_break_statement()"
Task: "Implement parse_continue_statement()"
Task: "Implement parse_print_statement()"
Task: "Implement parse_expression_statement()"
```

---

## Implementation Strategy

### MVP First (User Story 1 Only)

1. Complete Phase 1: Setup (T001-T004)
2. Complete Phase 2: Foundational (T005-T012) - CRITICAL - blocks all stories
3. Complete Phase 3: User Story 1 (T013-T033)
4. **STOP and VALIDATE**: Test User Story 1 independently with empty programs and simple declarations
5. Deploy/demo if ready - MVP demonstrates parser infrastructure working

### Incremental Delivery

1. Complete Setup + Foundational → Foundation ready (AST node infrastructure)
2. Add User Story 1 → Test independently (empty programs, expression statements) → Deploy/Demo (MVP!)
3. Add User Story 2 → Test independently (all expression types with correct precedence) → Deploy/Demo
4. Add User Story 3 → Test independently (all statement types) → Deploy/Demo
5. Add User Story 4 → Test independently (error detection and recovery) → Deploy/Demo
6. Each story adds value without breaking previous stories

### Parallel Team Strategy

With multiple developers:

1. Team completes Setup + Foundational together (T001-T012)
2. Once Foundational is done:
   - Developer A: User Story 1 (T013-T033) - Parser orchestrator, Program node
   - Developer B: User Story 2 (T034-T091) - ExpressionParser, all 16 expression nodes
   - Developer C: User Story 3 (T092-T148) - StatementParser, all 11 statement nodes
3. After US1, US2, US3 complete:
   - Team together: User Story 4 (T149-T211) - Error detection and recovery
4. Stories complete and integrate independently
5. Polish phase (T212-T222) distributed among team

---

## Notes

- **[P] tasks** = different files, no dependencies - can execute in parallel
- **[Story] label** maps task to specific user story for traceability (US1, US2, US3, US4)
- Each user story should be independently completable and testable
- Verify tests fail before implementing (TDD Red-Green-Refactor workflow)
- Commit after each task or logical group of small tasks
- Stop at any checkpoint to validate story independently
- Avoid: vague tasks, same file conflicts, cross-story dependencies that break independence
- **Test Naming Convention**: `Parser_Module_Scenario` format (e.g., `Parser_EmptyInput_ReturnsEmptyProgram`, `Parser_Precedence_MultiplicationBeforeAddition`)
- **Test Tags**: Use Catch2 tags for filtered execution: `[parser]`, `[expressions]`, `[statements]`, `[errors]`, `[US1]`, `[US2]`, `[US3]`, `[US4]`
- **Coverage Tracking**: Run gcovr after each user story completion to track coverage progress
- **Sanitizer Testing**: Run AddressSanitizer and UndefinedBehaviorSanitizer after each user story - zero violations required

---

## Task Summary

**Total Tasks**: 222 tasks

**Breakdown by Phase**:

- Phase 1 (Setup): 4 tasks
- Phase 2 (Foundational): 8 tasks
- Phase 3 (User Story 1): 21 tasks
- Phase 4 (User Story 2): 58 tasks
- Phase 5 (User Story 3): 57 tasks
- Phase 6 (User Story 4): 40 tasks
- Phase N (Polish): 11 tasks

**Breakdown by User Story**:

- US1 (Parse Complete Program): 21 tasks (T013-T033)
- US2 (Parse Expressions): 58 tasks (T034-T091)
- US3 (Parse Statements): 57 tasks (T092-T148)
- US4 (Error Detection): 40 tasks (T149-T211)

**Parallel Opportunities Identified**:

- Foundational phase: 8 tasks can run in parallel (T005-T012)
- User Story 2: 16 expression node headers can run in parallel (T060-T075)
- User Story 3: 11 statement node headers can run in parallel (T112-T122)
- User Story 3: 11 statement parser implementations can run in parallel (T128-T138)
- All user stories can proceed in parallel after Foundational phase (if team capacity allows)

**Independent Test Criteria**:

- US1: Empty program produces valid Program node; simple declarations parsed correctly
- US2: Expression `1 + 2 * 3` groups as `1 + (2 * 3)`; `1 - 2 - 3` groups as `(1 - 2) - 3`; `a = b = c` groups as `a = (b = c)`
- US3: Each statement type (var, fun, if, while, for, return, break, continue, print) parsed correctly in isolation
- US4: Syntax errors detected with correct error codes; parser recovers and continues to find additional errors

**Suggested MVP Scope**: User Story 1 only (T001-T033) - demonstrates parser infrastructure with ability to parse empty programs and simple expression statements

**Format Validation**: ✅ ALL tasks follow the checklist format:

- Checkbox: `- [ ]`
- Task ID: Sequential (T001, T002, T003...)
- [P] marker: Included only for parallelizable tasks
- [Story] label: Included for all user story phase tasks (US1, US2, US3, US4)
- Description: Clear action with exact file path
