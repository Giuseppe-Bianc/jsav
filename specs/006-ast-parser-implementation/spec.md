# Feature Specification: AST Parser Implementation

**Feature Branch**: `006-ast-parser-implementation`
**Created**: 2026-03-20
**Status**: Draft
**Input**: User description: "Build a parser that transforms a token stream (std::vector<Token>) from the existing Lexer into a typed Abstract Syntax Tree (AST) while collecting syntax errors in a std::vector<CompileError>. The parser must implement a hybrid parsing strategy: Pratt Parsing for all expression types
(using binding power pairs to handle operator precedence and associativity) combined with Recursive Descent Parsing for all statement and declaration types. The parser produces two outputs: (1) NodePtr pointing to the Program node (AST root), and (2) std::vector<CompileError> containing all syntax
errors encountered during parsing.

Supported AST Nodes: The parser must construct all 16 expression node types defined in Expressions.hpp: IntegerLiteral, FloatLiteral, StringLiteral, BoolLiteral, NullLiteral, Identifier, UnaryExpr, BinaryExpr, TernaryExpr, CallExpr, IndexExpr, MemberExpr, AssignExpr, CastExpr, ArrayLiteral, GroupingExpr. The parser must construct all 11 statement node types defined in Statements.hpp: ExprStmt, VarDecl, FuncDecl, ReturnStmt, IfStmt, WhileStmt, ForStmt, BlockStmt, BreakStmt, ContinueStmt, PrintStmt.

Module Architecture:

Parser Module (Main Orchestrator): Maintains parser state including tokens_(non-owning reference to token vector), current_ (current token index), errors_(vector of CompileError), panic_mode_(boolean error recovery flag), context_stack_(stack of Context records for tracking nesting: Global, Function, Loop, Block). Provides token navigation methods: advance() (consume current token, move to next), peek(offset=0) (inspect token at offset without consuming), check(TokenKind) (test if current token matches kind), match(TokenKind) (consume if matches, return true/false), expect(TokenKind, error_message) (consume or report error), is_at_end() (check if at EOF). Provides error handling: report_error(error_code, message, span, help) (create CompileError and add to errors_), synchronize() (panic-mode recovery by advancing to next
synchronization point: ;, }, or statement keywords var, fun, if, while, for, return, break, continue, print). Provides context management: push_context(Context) (enter new scope: Function, Loop, or Block), pop_context() (exit current scope), is_in_loop_context() (check if inside loop for break/continue validation), is_in_function_context() (check if inside function for return validation). Provides main entry point: parse() (return std::pair<NodePtr, std::vector<CompileError>>).

ExpressionParser Module (Pratt Parsing with Binding Powers): Implements Pratt Parsing algorithm using binding power pairs (left_bp, right_bp) for each
operator TokenKind. Maintains parser_(reference to Parser), prefix_parse_table_ (map from TokenKind to prefix parser function), infix_parse_table_(map
from TokenKind to infix parser function with left-hand side expression). Provides parse_expression(min_precedence) (main Pratt loop: parse left-hand
side, then while current operator's left binding power >= min_precedence, parse infix and continue). Provides `parse_prefix() (dispatch to prefix parser
based on current token: literals, identifiers, unary operators, grouping). Provides parse_infix(left, left_binding_power)` (parse binary operators,
calls, indexing, member access, assignment using right binding power to determine recursion depth).

Binding Power Table for Pratt Parsing (12 precedence levels with explicit associativity):

- Level 10 (highest, left-associative postfix): () calls, [] indexing, . member access — left_bp=10, no right_bp (postfix operators consume left-hand
   side immediately)
- Level 9 (left-associative postfix): ++, -- postfix — left_bp=9, no right_bp
- Level 8 (right-associative prefix): - negate, ! not, ~ bitnot, ++ preinc, -- predec — left_bp=N/A (prefix only), right_bp=8
- Level 7 (left-associative binary): * mul, / div, % mod — left_bp=7, right_bp=8 (left-associative: parse right-hand side with precedence+1)
- Level 6 (left-associative binary): + add, - sub — left_bp=6, right_bp=7
- Level 5 (left-associative binary): << shl, >> shr — left_bp=5, right_bp=6
- Level 4 (left-associative binary): < lt, > gt, <= le, >= ge — left_bp=4, right_bp=5
- Level 3 (left-associative binary): == eq, != neq — left_bp=3, right_bp=4
- Level 2 (left-associative binary): & bitand, ^ bitxor, | bitor — left_bp=2, right_bp=3
- Level 1 (left-associative binary): && and — left_bp=1, right_bp=2
- Level 0 (left-associative binary): || or — left_bp=0, right_bp=1
- Level -1 (right-associative ternary): ? : conditional — left_bp=-1, right_bp=0 (right-associative: parse right-hand side with same precedence)
- Level -2 (right-associative binary): =, +=, -=, *=, /=, %=, &=, ^=, |=, <<=, >>= — left_bp=-2, right_bp=-1 (right-associative: parse right-hand side
   with same precedence, enabling chained assignment a=b=c as a=(b=c))

Associativity Implementation: For left-associative binary operators, parse right-hand side with right_bp = left_bp + 1 so that a - b - c groups as (a -
b) - c. For right-associative operators (assignment, ternary, unary prefix), parse right-hand side with right_bp = left_bp (or left_bp + 1 for unary) so
that a = b = c groups as a = (b = c) and !flag binds tightly to flag.

StatementParser Module (Recursive Descent): Implements one parsing function per statement type. Provides parse_statement() (main dispatch: inspect current token and route to specific parser). Provides parse_var_declaration() (parse var/const keyword, identifier list, optional type annotation,
required initializer for const, semicolon). Provides parse_function_declaration() (parse fun keyword, identifier, parameter list with types, optional return type, block body). Provides parse_return_statement() (parse return keyword, optional expression, semicolon). Provides parse_if_statement() (parse
if keyword, condition in parentheses, then-branch statement, optional else branch). Provides parse_while_statement() (parse while keyword, condition in parentheses, body statement). Provides parse_for_statement() (parse for keyword, parentheses with optional init-statement, condition, increment, body statement). Provides parse_block() (parse { statement-list }). Provides parse_break_statement() (parse break keyword, semicolon). Provides parse_continue_statement() (parse continue keyword, semicolon). Provides parse_print_statement() (parse print keyword, expression, semicolon). Provides
parse_expression_statement() (parse expression, semicolon).

Error Detection and Reporting: The parser must detect and report the following syntax errors with appropriate ErrorCode values from the existing
error_codes.hpp:

- Expression Errors: E0101 (unbalanced parentheses: ( without matching )), E0102 (unbalanced brackets: [ without matching ]), E0103 (unbalanced braces:
   { without matching }), E0104 (missing operand for binary operator), E0105 (unexpected token in expression), E0106 (missing function call arguments: (
   without )), E0107 (missing index expression: [ without ]), E0108 (missing member name after .), E0109 (trailing comma in argument list), E0110
   (trailing comma in array literal)
- Declaration Errors: E0201 (missing initializer for const declaration), E0202 (invalid variable name), E0203 (missing type annotation), E0204
   (duplicate parameter name), E0205 (missing function body), E0206 (invalid return type)
- Statement Errors: E0301 (missing condition for if statement), E0302 (missing condition for while statement), E0303 (missing semicolon after
   statement), E0304 (break statement outside loop), E0305 (continue statement outside loop), E0306 (return statement outside function), E0307
   (unreachable code after return), E0308 (missing expression for print statement)
- Structural Errors: E0401 (unexpected token at top level), E0402 (unexpected end of file)

Error Recovery: Implement panic-mode error recovery. When an error is detected: (1) report the error with source location, (2) set panic_mode_ = true,
(3) call synchronize() to advance tokens until a synchronization point is found (;, }, or statement keywords), (4) reset panic_mode_ = false, (5)
continue parsing. No limit on error collection — parser continues until end of input.

Integration Points: ExpressionParser integrates with Parser by calling parser_.advance(), parser_.peek(), parser_.check(), parser_.match(), parser_.expect(), parser_.report_error(). StatementParser integrates with Parser similarly and integrates with ExpressionParser by calling expression_parser_.parse_expression(min_precedence) for parsing expressions within statements (initializers, conditions, arguments). Both
ExpressionParser and StatementParser construct AST nodes using std::make_unique<NodeType>(...) and return ExprPtr or StmtPtr to the caller.

Extensibility Design: New expression operators are added by: (1) adding TokenKind to lexer, (2) registering prefix parser in prefix_parse_table_for
prefix operators, (3) registering infix parser with binding power pair in infix_parse_table_ for infix operators, (4) implementing parse function in
ExpressionParser. New statement types are added by: (1) adding NodeKind to NodeKind.hpp, (2) implementing node class in Statements.hpp, (3) implementing
parse_new_statement() in StatementParser, (4) adding dispatch case in parse_statement(). Core parser infrastructure remains unchanged.

Testing Requirements: Each module must have comprehensive unit tests with ≥80% line coverage and ≥70% branch coverage. ExpressionParser tests must
verify: all 16 expression types parse correctly, operator precedence is respected (e.g., 1 + 2 *3 parses as 1 + (2* 3)), associativity is correct
(e.g., 1 - 2 - 3 parses as (1 - 2) - 3, a = b = c parses as a = (b = c)), error cases are detected (unbalanced delimiters, missing operands).
StatementParser tests must verify: all 11 statement types parse correctly, nested structures parse correctly (blocks within if, functions within blocks),
 error cases are detected (missing semicolons, invalid placements). Parser integration tests must verify: complete programs parse correctly, error
recovery works (multiple errors collected), AST positions match source locations.

Documentation Requirements: All public interfaces must have Doxygen documentation including: function purpose, parameter descriptions with types and
constraints, return value description, exceptions thrown (if any), usage example code. Module-level documentation must describe: module responsibility,
architectural role, integration points, extension mechanism. Error codes must have documentation describing: error condition, typical cause, suggested
fix."

## User Scenarios & Testing

### User Story 1 - Parse Complete Program into AST (Priority: P1)

**Description**: Compiler developers need to transform a complete source file (already tokenized by the Lexer) into a structured Abstract Syntax Tree that represents the program's syntactic structure. This is the foundational capability that enables all subsequent compiler phases.

**Why this priority**: This is the core MVP functionality. Without the ability to parse a complete program and produce an AST, no semantic analysis, optimization, or code generation can occur. This is the primary value proposition of the parser module.

**Independent Test**: Given a valid token stream representing a complete program, the parser produces a well-formed AST with a Program node as root, and the AST structure correctly reflects the source code's syntactic relationships.

**Acceptance Scenarios**:

1. **Given** a token stream representing a valid program with declarations and statements, **When** the parser processes the token stream, **Then** it produces an AST with a Program node containing all top-level declarations in correct order.
2. **Given** a token stream with nested block structures (functions containing blocks containing statements), **When** the parser processes the token stream, **Then** the AST correctly reflects the nesting hierarchy with proper parent-child relationships.
3. **Given** an empty token stream (only EOF), **When** the parser processes it, **Then** it produces a valid AST with an empty Program node.

---

### User Story 2 - Parse All Expression Types with Correct Precedence (Priority: P2)

**Description**: Compiler developers need the parser to correctly interpret all 16 expression types (literals, identifiers, unary/binary/ternary operators, function calls, indexing, member access, assignments, casts, arrays, groupings) while respecting operator precedence and associativity rules.

**Why this priority**: Expression parsing is fundamental to understanding program semantics. Incorrect precedence or associativity leads to fundamentally wrong interpretations of code (e.g., `1 + 2 * 3` must parse as `1 + (2 * 3)`, not `(1 + 2) * 3`). This can be demonstrated independently with expression-only inputs.

**Independent Test**: Given token streams representing expressions with mixed operators, the produced AST groups sub-expressions according to standard precedence rules, and associativity is correct for operators at the same precedence level.

**Acceptance Scenarios**:

1. **Given** an expression `1 + 2 * 3`, **When** parsed, **Then** the AST represents `1 + (2 * 3)` (multiplication binds tighter than addition).
2. **Given** an expression `1 - 2 - 3`, **When** parsed, **Then** the AST represents `(1 - 2) - 3` (left-associative subtraction).
3. **Given** an expression `a = b = c`, **When** parsed, **Then** the AST represents `a = (b = c)` (right-associative assignment).
4. **Given** an expression `!flag`, **When** parsed, **Then** the AST represents unary negation applied to `flag`.
5. **Given** an expression `func(arg1, arg2)`, **When** parsed, **Then** the AST represents a function call with two arguments.
6. **Given** an expression `array[index]`, **When** parsed, **Then** the AST represents an indexing operation.
7. **Given** an expression `object.member`, **When** parsed, **Then** the AST represents a member access operation.
8. **Given** a conditional expression `condition ? thenValue : elseValue`, **When** parsed, **Then** the AST represents a ternary conditional with three operands.

---

### User Story 3 - Parse All Statement and Declaration Types (Priority: P3)

**Description**: Compiler developers need the parser to correctly recognize and structure all 11 statement types (expression statements, variable declarations, function declarations, return statements, control flow: if/while/for, blocks, break/continue, print statements) with proper syntax validation.

**Why this priority**: Statements define program control flow and structure. While expressions compute values, statements define what the program *does*. This can be tested independently by parsing statement sequences without complex expressions.

**Independent Test**: Given token streams representing each statement type in isolation, the parser produces correctly structured AST nodes for each statement type with all required components.

**Acceptance Scenarios**:

1. **Given** a variable declaration `var x = 42`, **When** parsed, **Then** the AST contains a VarDecl node with identifier `x` and initializer expression.
2. **Given** a constant declaration `const PI = 3.14`, **When** parsed, **Then** the AST contains a VarDecl node marked as constant with required initializer.
3. **Given** a typed variable declaration `var x: i32 = 42`, **When** parsed, **Then** the AST contains a VarDecl node with type annotation resolved to `Type::I32`.
4. **Given** a function declaration `fun add(a: i32, b: i32) -> i32 { return a + b; }`, **When** parsed, **Then** the AST contains a FuncDecl node with parameters typed as `Type::I32` and return type `Type::I32`.
5. **Given** a function declaration with invalid type `fun foo(x: invalid_type) { }`, **When** parsed, **Then** the parser reports a syntax error for unknown type annotation.
6. **Given** a function declaration `fun add(a, b) { return a + b; }`, **When** parsed, **Then** the AST contains a FuncDecl node with untyped parameters (no type annotation).
7. **Given** an if statement `if (condition) { body; } else { alternative; }`, **When** parsed, **Then** the AST contains an IfStmt node with condition, then-branch, and else-branch.
8. **Given** a while loop `while (condition) { body; }`, **When** parsed, **Then** the AST contains a WhileStmt node with condition and body.
9. **Given** a for loop `for (init; condition; increment) { body; }`, **When** parsed, **Then** the AST contains a ForStmt node with all three components and body.
10. **Given** a return statement `return value;`, **When** parsed, **Then** the AST contains a ReturnStmt node with the return expression.
11. **Given** a block `{ statement1; statement2; }`, **When** parsed, **Then** the AST contains a BlockStmt node containing both statements.

---

### User Story 4 - Collect and Report Syntax Errors with Recovery (Priority: P4)

**Description**: Compiler developers need the parser to detect syntax errors (unbalanced delimiters, missing operands, invalid placements, etc.), report them with source locations and helpful messages, and continue parsing to collect additional errors rather than stopping at the first one.

**Why this priority**: Error recovery is essential for developer productivity. Stopping at the first error forces developers to fix errors one at a time. Collecting multiple errors per parse enables faster iteration. This can be tested independently by providing malformed inputs.

**Independent Test**: Given token streams with various syntax errors, the parser reports all errors (up to the maximum limit) with accurate source locations and appropriate error codes, and continues parsing after each error to find additional issues.

**Acceptance Scenarios**:

1. **Given** an expression with unbalanced parentheses `(1 + 2`, **When** parsed, **Then** the parser reports error E0101 (unbalanced parentheses) with the source location of the opening parenthesis.
2. **Given** a binary operator without right operand `5 +`, **When** parsed, **Then** the parser reports error E0104 (missing operand) at the operator location.
3. **Given** a const declaration without initializer `const x;`, **When** parsed, **Then** the parser reports error E0201 (missing initializer for const).
4. **Given** multiple syntax errors in sequence, **When** parsed, **Then** the parser reports all errors (up to 100) and does not abort after the first error.
5. **Given** a break statement outside any loop, **When** parsed, **Then** the parser reports error E0304 (break outside loop).
6. **Given** a function declaration without body `fun foo();`, **When** parsed, **Then** the parser reports error E0205 (missing function body).

---

### Edge Cases

- **What happens when the token stream is empty (only EOF)?** The parser produces a valid AST with an empty Program node and no errors.
- **How does the system handle pathological inputs with many syntax errors?** The parser collects all errors until memory exhaustion; no artificial limit is imposed.
- **What happens when parsing encounters an unexpected token at the top level?** The parser reports error E0401 (unexpected token at top level) and performs panic-mode recovery to continue parsing.
- **How does the system handle trailing commas in argument lists or array literals?** The parser reports error E0109 (trailing comma in argument list) or E0110 (trailing comma in array literal) and continues parsing.
- **What happens when a closing delimiter appears without matching opening delimiter?** The parser reports the appropriate unbalanced delimiter error (E0101, E0102, or E0103) and performs error recovery.
- **How does the system handle deeply nested expressions or statements?** The parser handles arbitrary nesting depth limited only by available stack space; no artificial nesting limits are imposed.

## Requirements

### Functional Requirements

- **FR-001**: System MUST transform a token stream from the Lexer into a typed Abstract Syntax Tree with a Program node as the root.
- **FR-002**: System MUST construct all 16 expression node types: IntegerLiteral, FloatLiteral, StringLiteral, BoolLiteral, NullLiteral, Identifier, UnaryExpr, BinaryExpr, TernaryExpr, CallExpr, IndexExpr, MemberExpr, AssignExpr, CastExpr, ArrayLiteral, GroupingExpr.
- **FR-003**: System MUST construct all 11 statement node types: ExprStmt, VarDecl, FuncDecl, ReturnStmt, IfStmt, WhileStmt, ForStmt, BlockStmt, BreakStmt, ContinueStmt, PrintStmt.
- **FR-004**: System MUST respect operator precedence rules so that expressions like `1 + 2 * 3` are grouped as `1 + (2 * 3)`.
- **FR-005**: System MUST respect left-associativity for binary operators (except assignment and ternary) so that `1 - 2 - 3` is grouped as `(1 - 2) - 3`.
- **FR-006**: System MUST respect right-associativity for assignment operators so that `a = b = c` is grouped as `a = (b = c)`.
- **FR-007**: System MUST collect syntax errors in a list and return them alongside the AST.
- **FR-008**: System MUST report errors with source location information (file, line, column, span).
- **FR-009**: System MUST perform panic-mode error recovery by synchronizing to the next statement boundary (`;`, `}`, or statement keywords) after detecting an error.
- **FR-010**: System MUST collect all syntax errors without a predefined limit, continuing parsing until end of input or memory exhaustion.
- **FR-011**: System MUST detect and report unbalanced delimiters (parentheses, brackets, braces) with appropriate error codes.
- **FR-012**: System MUST detect and report missing operands for binary operators.
- **FR-013**: System MUST detect and report missing initializer for const declarations.
- **FR-014**: System MUST detect and report break/continue statements outside of loop contexts.
- **FR-015**: System MUST detect and report return statements outside function contexts.
- **FR-016**: System MUST detect and report missing semicolons after statements that require them.
- **FR-017**: System MUST detect and report trailing commas in argument lists and array literals.
- **FR-018**: System MUST detect and report missing function body for function declarations.
- **FR-019**: System MUST detect and report unknown type annotations that are not in the builtin Type enumeration.
- **FR-020**: System MUST provide a main entry point that returns both the AST root and the list of collected errors.
- **FR-021**: System MUST maintain non-owning references to the input token stream (no token duplication).

### Non-Functional Requirements

- **NFR-001**: System MUST prioritize correctness over performance — optimization decisions must be based on empirical profiling data, not theoretical analysis.
- **NFR-002**: System MUST handle arbitrary nesting depth limited only by available stack space (no artificial limits).
- **NFR-003**: System MUST complete parsing of typical source files (1K-10K LOC) within interactive responsiveness thresholds (<100ms to <500ms) as a secondary concern after correctness.
- **NFR-004**: System MUST be single-threaded only — parser instances are NOT thread-safe; parallel compilation is achieved by running multiple parser instances on different files.

### Key Entities

- **Token Stream**: Ordered sequence of tokens produced by the Lexer, each with kind, value, and source location. The parser consumes this stream without modifying or owning it.
- **Abstract Syntax Tree (AST)**: Hierarchical tree structure representing the syntactic structure of the source program. Root is a Program node containing declaration children. Parent nodes own their children via `std::unique_ptr<Node>`; child-to-parent references use raw `Node*` (non-owning observers).
- **Program Node**: Root AST node containing all top-level declarations in the source file.
- **Expression Node**: AST node representing a value-producing construct (literals, operators, calls, etc.). All expression nodes derive from a common Expression base type. Owned by parent via `std::unique_ptr<Expression>`.
- **Statement Node**: AST node representing an action or declaration within the program. All statement nodes derive from a common Statement base type. Owned by parent via `std::unique_ptr<Statement>`.
- **Type**: Enumeration of builtin types (i8, i16, i32, i64, u8, u16, u32, u64, f32, f64, char, string, bool, void) used for type annotations on variable declarations and function parameters/return types.
- **Compile Error**: Structured error record containing error code, message, source location (span), and optional help text for recovery.
- **Error Code**: Unique identifier (e.g., E0101, E0201) for each type of syntax error, enabling precise error handling and documentation.

## Clarifications

### Session 2026-03-20

- Q: Should the parser parse type annotations as simple identifiers (strings) or as typed `Type` enum values? → A: Enum-based (Option B). Parser resolves type annotations to the existing `Type` enum (i8, i16, i32, i64, u8, u16, u32, u64, f32, f64, char, string, bool, void). Unknown types are rejected at parse time.
- Q: What ownership model should AST nodes use for parent-child relationships? → A: `std::unique_ptr<Node>` for parent-to-child ownership; raw `Node*` for child-to-parent references (non-owning observers).
- Q: How should the parser track context for validation (break/continue/return placement, duplicate parameters)? → A: Stack-based context tracking (`std::vector<Context>` where Context = {Function, Loop, Block}) — accurate nesting, detailed error locations.
- Q: What performance targets should the parser meet? → A: No explicit performance targets — correctness first, optimize based on profiling data later.
- Q: Should the parser support concurrent access from multiple threads? → A: Single-threaded only — parser instances are NOT thread-safe; parallel compilation achieved by running multiple parser instances on different files.
- Q: What memory limit should be enforced for error collection? → A: No hard limit — collect all errors until memory exhaustion (risk OOM on pathological inputs).

## Success Criteria

### Measurable Outcomes

- **SC-001**: Users can parse any syntactically valid program and obtain a well-formed AST where every source token is represented in the tree structure.
- **SC-002**: System correctly parses all 16 expression types as verified by unit tests covering each node type.
- **SC-003**: System correctly parses all 11 statement types as verified by unit tests covering each node type.
- **SC-004**: Operator precedence is 100% correct as verified by tests with mixed-operator expressions (e.g., `1 + 2 * 3`, `a && b || c`, `x << 1 + 2`).
- **SC-005**: Associativity is 100% correct as verified by tests with repeated operators (e.g., `1 - 2 - 3` for left-associative, `a = b = c` for right-associative).
- **SC-006**: System detects and reports all syntax errors listed in the error specification (E0101-E0110, E0201-E0206, E0301-E0308, E0401-E0402) with accurate source locations.
- **SC-007**: System collects all syntax errors from malformed input without artificial limits, enabling developers to see all issues in a single parse.
- **SC-008**: Error recovery allows parsing to continue after errors, collecting multiple errors from a single malformed input.
- **SC-009**: AST node source locations match the original token source locations, enabling accurate error reporting in downstream compiler phases.
- **SC-010**: Unit tests achieve ≥80% line coverage and ≥70% branch coverage for all parser modules.
- **SC-011**: All public interfaces have complete Doxygen documentation including purpose, parameters, return values, and usage examples.
- **SC-012**: New expression operators can be added by registering prefix/infix parsers without modifying core parser infrastructure.
- **SC-013**: New statement types can be added by implementing parse functions and updating the dispatch table without modifying core parser infrastructure.
