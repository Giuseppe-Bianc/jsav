# Research Report: AST Parser Implementation

**Date**: 2026-03-21
**Branch**: `006-ast-parser-implementation`
**Status**: Complete — All NEEDS CLARIFICATION items resolved

---

## Technical Decisions

### Decision 1: Expression Parsing Algorithm — Pratt Parsing

**Decision**: Use Pratt Parsing algorithm for all expression types with explicit binding power table.

**Rationale**:
- Pratt parsing handles operator precedence and associativity through binding power pairs rather than function call stack depth
- Superior to traditional recursive descent for expressions with 12+ precedence levels (reduces code complexity, easier to extend)
- Each operator has (left_bp, right_bp) pair that encodes both precedence and associativity
- Left-associative operators: right_bp = left_bp + 1 (ensures `a - b - c` groups as `(a - b) - c`)
- Right-associative operators: right_bp = left_bp (ensures `a = b = c` groups as `a = (b = c)`)
- Prefix operators registered only in prefix_parse_table_, not infix table
- Postfix operators have right_bp = INT_MAX to consume left-hand side immediately

**Alternatives Considered**:
- **Recursive Descent for Expressions**: Rejected because it requires one function per precedence level (12 functions), making code harder to maintain and extend. Adding new operators requires modifying multiple functions.
- **Operator Precedence Parsing (classic)**: Rejected because Pratt parsing is a refinement that handles both prefix and postfix operators more elegantly through separate dispatch tables.
- **Table-Driven Parser Generators (yacc/bison)**: Rejected because they add external dependencies, increase build complexity, and reduce debugging visibility. Hand-written parser provides better error messages and recovery control.

---

### Decision 2: Statement Parsing Algorithm — Recursive Descent

**Decision**: Use Recursive Descent parsing for all statement and declaration types.

**Rationale**:
- Statement grammar is naturally hierarchical (blocks contain statements, functions contain blocks)
- Does not require precedence handling — statement keywords are unambiguous
- One function per statement type provides clear separation of concerns
- Easier debugging — each function corresponds to a specific grammar rule
- Context validation (break/continue in loops, return in functions) naturally integrated into parsing functions

**Alternatives Considered**:
- **Pratt Parsing for Statements**: Rejected because statements don't have operator precedence. Pratt parsing adds unnecessary complexity for keyword-driven dispatch.
- **Table-Driven LL(1) Parser**: Rejected because statement grammar has many keywords, making parse table large and hard to maintain. Recursive descent provides better error messages.
- **Parser Combinators**: Rejected because they add abstraction overhead and dependency complexity. Hand-written recursive descent is more transparent for compiler development.

---

### Decision 3: Error Recovery Strategy — Panic Mode

**Decision**: Implement panic-mode error recovery with synchronization points at `;`, `}`, or statement keywords.

**Rationale**:
- When error detected: (1) report error with source location, (2) set panic_mode_ = true, (3) synchronize() to advance to next boundary, (4) reset panic_mode_, (5) continue parsing
- Collects multiple errors per parse (improves developer productivity vs. stopping at first error)
- No artificial error limit — parser continues until memory exhaustion
- Synchronization points chosen to align with natural statement boundaries in the language grammar
- Simple to implement and debug compared to phrase-level or error-production recovery

**Alternatives Considered**:
- **Phrase-Level Recovery**: Rejected because it requires error productions for every grammar rule, increasing grammar complexity and maintenance burden.
- **Error Productions**: Rejected because they bloat the grammar with error-handling cases, making the core parsing logic harder to follow.
- **Stop-on-First-Error**: Rejected because it forces developers to fix errors one at a time, severely impacting iteration speed.
- **Correction-Based Recovery**: Rejected because automatic error correction (inserting missing tokens, deleting unexpected tokens) can mask real errors and produce confusing downstream errors.

---

### Decision 4: Memory Ownership Model — std::unique_ptr Exclusivity

**Decision**: AST nodes use std::unique_ptr<Node> exclusively. Parent nodes own child nodes. No raw new/delete.

**Rationale**:
- Zero overhead compared to raw pointers (no reference counting)
- Automatic deletion on scope exit (exception-safe, no leaks)
- Move-only semantics prevent accidental copies and clarify ownership transfer
- Clear ownership: parent owns children, no shared ownership ambiguity
- Integrates with existing project memory management patterns (Constitution Principle III)

**Alternatives Considered**:
- **std::shared_ptr**: Rejected because AST has clear parent-child ownership. Reference counting adds unnecessary overhead and obscures ownership semantics.
- **Raw Pointers with Manual delete**: Rejected because it's error-prone (memory leaks on exceptions, double-free bugs). Violates Constitution Principle III (Ownership Semantics Explicit).
- **Arena Allocation**: Rejected because it adds complexity and doesn't integrate well with existing codebase. AST lifetime is typically short-lived (parse → analyze → discard), so arena benefits are minimal.
- **std::unique_ptr with custom deleter**: Rejected because default deleter is sufficient. Custom deleters add complexity without benefit.

---

### Decision 5: Context Management — RAII Stack

**Decision**: Implement context stack with RAII ContextGuard objects for automatic scope management.

**Rationale**:
- Context stack tracks nesting: Global → Function → Loop → Block
- ContextGuard created on entry to scoped constructs (function body, loop body, block)
- Destructor automatically pops context on scope exit, including error paths
- Enables validation: break/continue require is_in_loop_context(), return requires is_in_function_context()
- Exception-safe: RAII ensures cleanup even if parsing throws (though parser avoids exceptions)

**Alternatives Considered**:
- **Manual push/pop_context() calls**: Rejected because error paths may skip pop calls, causing context corruption. RAII guarantees cleanup.
- **Thread-Local Storage**: Rejected because parser is single-threaded per compilation unit. TLS adds unnecessary complexity.
- **Explicit Context Parameter**: Rejected because it would require threading context through every parsing function, increasing boilerplate and error surface.
- **Scope Enumeration**: Rejected because it requires explicit scope tracking in every function. Stack-based approach is simpler and more efficient.

---

### Decision 6: Binding Power Table Design — 12 Levels with Explicit Associativity

**Decision**: Implement 12-level binding power table with explicit left_bp and right_bp values encoding associativity.

**Rationale**:
- Level 10 (postfix): (), [], . — left_bp=10, right_bp=INT_MAX (postfix operators consume LHS immediately)
- Level 9 (postfix): ++, -- — left_bp=9, right_bp=INT_MAX
- Level 8 (prefix): -, !, ~, ++, -- — prefix-only, right_bp=8
- Level 7-0 (binary left-assoc): left_bp=N, right_bp=N+1 (ensures left grouping)
- Level -1 (ternary): ? : — left_bp=-1, right_bp=0 (right-assoc)
- Level -2 (binary right-assoc): =, +=, etc. — left_bp=-2, right_bp=-1 (ensures right grouping)
- Table values are static constexpr — no runtime calculation overhead
- Associativity encoded directly in binding power pairs (no separate associativity enum)

**Alternatives Considered**:
- **Separate Precedence and Associativity Enums**: Rejected because it requires runtime logic to compute effective binding power. Direct encoding is simpler and faster.
- **Runtime Binding Power Calculation**: Rejected because it adds overhead and complexity. Compile-time constants are zero-cost.
- **Fewer Precedence Levels (e.g., 5-6)**: Rejected because C-style languages have 10+ distinct precedence levels. Collapsing levels produces incorrect parsing (e.g., treating + and * at same level).
- **More Precedence Levels (15+)**: Rejected because language grammar doesn't require finer granularity. Additional levels would be unused complexity.

---

### Decision 7: Error Code Mapping — Comprehensive Coverage

**Decision**: Map all syntax errors to specific ErrorCode values from existing error_codes.hpp.

**Rationale**:
- Expression errors: E0101-E0110 (unbalanced delimiters, missing operands, unexpected tokens, trailing commas)
- Declaration errors: E0201-E0206 (missing initializers, invalid names, duplicate parameters)
- Statement errors: E0301-E0308 (missing conditions, invalid placements, unreachable code)
- Structural errors: E0401-E0402 (unexpected tokens, EOF)
- Each error includes source location (file, line, column, span) for IDE integration
- Error messages include help text suggesting fixes

**Alternatives Considered**:
- **String-Based Error Messages**: Rejected because they're not machine-parseable. Error codes enable IDE integration, filtering, and localization.
- **Exception-Based Error Reporting**: Rejected because exceptions are for unrecoverable errors. Syntax errors are expected and should be collected, not thrown.
- **Single "Syntax Error" Code**: Rejected because it provides no diagnostic value. Specific codes help users understand and fix errors.
- **Line/Column Only (No Error Codes)**: Rejected because it prevents error categorization and filtering. Codes enable structured error handling.

---

### Decision 8: Token Stream Ownership — Non-Owning Reference

**Decision**: Parser holds non-owning std::string_view reference to token vector from Lexer.

**Rationale**:
- Lexer output must outlive Parser (clear lifetime contract)
- Avoids unnecessary token copying (memory efficient)
- std::string_view provides bounds-checked access (safer than raw pointer)
- Consistent with existing project patterns (Constitution Principle III)

**Alternatives Considered**:
- **Parser Owns Token Vector**: Rejected because Lexer should own its output. Transfer of ownership adds complexity and potential for double-free.
- **std::shared_ptr<Token Vector>**: Rejected because ownership is not shared — Lexer creates, Parser consumes, AST doesn't need tokens. Reference counting is unnecessary overhead.
- **Raw Pointer**: Rejected because it lacks bounds checking and doesn't express ownership intent. std::string_view is safer and clearer.
- **Token Iterator Pair**: Rejected because it requires maintaining two iterators. Single vector reference with index is simpler.

---

### Decision 9: AST Node Design — 27 Node Types

**Decision**: Implement 27 AST node types (16 expressions + 11 statements) using variant pattern with std::unique_ptr.

**Rationale**:
- Expression nodes: IntegerLiteral, FloatLiteral, StringLiteral, BoolLiteral, NullLiteral, Identifier, UnaryExpr, BinaryExpr, TernaryExpr, CallExpr, IndexExpr, MemberExpr, AssignExpr, CastExpr, ArrayLiteral, GroupingExpr
- Statement nodes: ExprStmt, VarDecl, FuncDecl, ReturnStmt, IfStmt, WhileStmt, ForStmt, BlockStmt, BreakStmt, ContinueStmt, PrintStmt
- Each node type has specific fields matching its semantic requirements
- Nodes use std::unique_ptr for child relationships
- Node base class provides common interface (source location, node kind)

**Alternatives Considered**:
- **Single Generic Node with Variant**: Rejected because it loses type safety and requires runtime type checking. Specific node types provide compile-time guarantees.
- **Fewer Node Types (e.g., generic Literal)**: Rejected because it loses semantic information. Distinguishing IntegerLiteral from FloatLiteral enables type checking.
- **More Node Types (e.g., separate AddExpr, SubExpr)**: Rejected because it explodes node count without benefit. BinaryExpr with operator field is simpler and more maintainable.
- **Polymorphic Class Hierarchy**: Rejected because it requires virtual functions and dynamic dispatch. Variant pattern with std::visit is more explicit and potentially faster.

---

### Decision 10: Testing Strategy — Three-Target Catch2

**Decision**: Use three-target Catch2 testing approach with specific coverage targets.

**Rationale**:
- constexpr_tests: Compile-time verification of binding power constants using STATIC_REQUIRE
- relaxed_constexpr_tests: Runtime version for debugging constexpr test failures
- tests: Runtime unit tests for all 27 node types, precedence, associativity, error cases, context validation
- Coverage targets: ≥80% line, ≥70% branch (gcovr)
- Sanitizers: AddressSanitizer + UndefinedBehaviorSanitizer (zero violations required)

**Alternatives Considered**:
- **Single Test Target**: Rejected because constexpr code requires compile-time verification. Runtime tests alone don't verify constexpr correctness.
- **Google Test**: Rejected because Catch2 has better constexpr support (STATIC_REQUIRE), simpler syntax, and is already used in project.
- **Custom Test Framework**: Rejected because it adds maintenance burden. Catch2 is mature, well-documented, and project already integrates it.
- **No Coverage Targets**: Rejected because coverage metrics provide objective quality gates. Without targets, coverage may drift below acceptable levels.

---

## Integration Points

### Lexer → Parser Integration

- Parser receives std::vector<Token> from Lexer (non-owning reference)
- Token structure must include: kind, lexeme, source location (file, line, column, span)
- Lexer must handle Unicode (UTF-8) and produce accurate source locations
- Parser assumes Lexer has already validated token structure (no unknown token kinds except explicit error tokens)

### Parser → AST Integration

- Parser constructs AST nodes via std::make_unique<NodeType>(...)
- AST must support all 27 node types with correct parent-child relationships
- Program node is root; contains list of top-level declarations
- Source locations propagated from tokens to AST nodes for error reporting

### Parser → Error Handler Integration

- Parser reports errors via report_error(error_code, message, span, help)
- CompileError structure must include: error code, message, source location, optional help text
- Error vector returned alongside AST (std::pair<NodePtr, std::vector<CompileError>>)
- Caller decides whether to proceed based on error count/severity

---

## Performance Considerations

### Time Complexity

- **Expression Parsing**: O(n) for n tokens (single Pratt loop pass)
- **Statement Parsing**: O(n) for n tokens (single recursive descent pass)
- **Error Recovery**: O(n) worst case (synchronize() scans each token at most once)
- **Overall**: O(n) linear time complexity for complete parse

### Space Complexity

- **AST**: O(n) for n tokens (each token may produce one AST node)
- **Context Stack**: O(d) for maximum nesting depth d (typically d << n)
- **Error Vector**: O(e) for e errors (no artificial limit)
- **Token Stream**: O(n) (owned by Lexer, referenced by Parser)

### Memory Ownership

- Parent nodes own child nodes via std::unique_ptr
- Destruction is automatic and recursive (parent destruction destroys children)
- No circular references (tree structure, not graph)
- No manual memory management (no new/delete in application code)

---

## Extension Mechanism

### Adding New Expression Operators

1. Add TokenKind to lexer (e.g., TokenKind::Mod for `%`)
2. Register prefix parser in prefix_parse_table_ (if prefix operator)
3. Register infix parser with binding power pair in infix_parse_table_ (if infix operator)
4. Implement parse function in ExpressionParser (e.g., parse_modulus())
5. Add corresponding AST node type if needed (e.g., ModExpr)
6. Add tests for precedence, associativity, error cases

### Adding New Statement Types

1. Add NodeKind to NodeKind.hpp (e.g., NodeKind::SwitchStmt)
2. Implement node class in Statements.hpp (e.g., SwitchStmt with cases, default, body)
3. Implement parse_switch_statement() in StatementParser
4. Add dispatch case in parse_statement() (switch on TokenKind::Switch)
5. Add context validation if needed (e.g., break allowed in switch)
6. Add tests for syntax, nesting, error cases

---

## Unresolved Questions

**None** — All NEEDS CLARIFICATION items from Technical Context have been resolved through this research phase.
