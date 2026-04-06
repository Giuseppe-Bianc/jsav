/*
 * Created by gbian on 2 aprile 2026.
 * Copyright (c) 2026 All rights reserved.
 */
#include "jsav/typechecker/TypeChecker.hpp"
#include <utility>
#include "jsav/ast/Expressions.hpp"
#include "jsav/ast/NodeKind.hpp"
#include "jsav/ast/Statements.hpp"
#include "jsavCore/disableWarn.hpp"
#include "jsavCore/format.hpp"
#include "jsavCore/Log.hpp"
#include "jsav/typechecker/ErrorType.hpp"
#include "jsav/typechecker/TypeVariable.hpp"

DISABLE_WARNINGS_PUSH(4834 4100 26461)

namespace jsv {

// ============================================================
// Helper: parse type annotation string into TypePtr
// ============================================================
static TypePtr parse_type_annotation(std::string_view annot) {
    if(annot == "i8") return PrimitiveType::i8();
    if(annot == "i16") return PrimitiveType::i16();
    if(annot == "i32") return PrimitiveType::i32();
    if(annot == "i64") return PrimitiveType::i64();
    if(annot == "u8") return PrimitiveType::u8();
    if(annot == "u16") return PrimitiveType::u16();
    if(annot == "u32") return PrimitiveType::u32();
    if(annot == "u64") return PrimitiveType::u64();
    if(annot == "f32") return PrimitiveType::f32();
    if(annot == "f64") return PrimitiveType::f64();
    if(annot == "bool") return PrimitiveType::bool_();
    if(annot == "string") return PrimitiveType::string();
    if(annot == "char") return PrimitiveType::char_();
    if(annot == "void") return PrimitiveType::void_();
    if(annot == "nullptr") return PrimitiveType::nullptr_();
    // Unknown annotation — return nullptr so caller falls back to inference
    return nullptr;
}

// ============================================================
// Helper: recursively apply substitution to a TypePtr
// ============================================================
static TypePtr zonk_type(const Substitution& subst, const TypePtr& type) {
    if(!type) return type;

    if(const auto* tvar = dynamic_cast<const TypeVariable*>(type.get())) {
        if(auto resolved = subst.lookup(tvar->id())) {
            // Recursively zonk the resolved type (may itself be a type variable)
            return zonk_type(subst, *resolved);
        }
        return type;  // Unresolved type variable — leave as-is
    }

    if(const auto* arr = dynamic_cast<const ArrayType*>(type.get())) {
        auto elem = zonk_type(subst, arr->element_type());
        if(elem == arr->element_type()) return type;
        // Preserve original size expression
        return std::make_shared<ArrayType>(std::move(elem), arr->size_expr());
    }

    if(const auto* vec = dynamic_cast<const VectorType*>(type.get())) {
        auto elem = zonk_type(subst, vec->element_type());
        if(elem == vec->element_type()) return type;
        return std::make_shared<VectorType>(std::move(elem));
    }

    // CustomType and PrimitiveType: no substitution needed
    return type;
}

// ============================================================
// Main check() entry point
// ============================================================
TypeCheckResult TypeChecker::check(const Program& program) {
    symbols_ = SymbolTable{};
    constraints_ = ConstraintSet{};
    errors_ = std::vector<CompileError>{};
    typed_stmts_.clear();

    // Phase 1: Name resolution
    resolve_names(program);

    // Phase 2: Constraint generation
    generate_constraints(program);

    // Phase 3: Constraint solving
    auto solver_result = solve_constraints();
    if(!solver_result.errors.empty()) {
        errors_.insert(errors_.end(), solver_result.errors.begin(), solver_result.errors.end());
    }

    // Phase 4: Zonking — apply substitution to produce typed AST
    LINFO("Type checking complete: {} errors", errors_.size());
    return TypeCheckResult{zonk(solver_result.substitution), std::move(errors_)};
}

// ============================================================
// Phase 1: Name Resolution
// ============================================================
void TypeChecker::resolve_names(const Program& program) {
    symbols_.push_scope();  // Global scope

    for(const auto& stmt : program.statements()) {
        resolve_names_stmt(*stmt);
    }
}

void TypeChecker::resolve_names_stmt(const Stmt& stmt) {
    switch(stmt.kind()) {
    case NodeKind::FuncDecl: {
        const auto* fd = static_cast<const FuncDecl*>(&stmt);
        // Register function with a type variable (will be refined during constraint generation)
        auto func_type = fresh_type_variable();
        symbols_.define(fd->name(), TypeScheme::mono(func_type));
        LTRACE("Resolved function declaration: {}", fd->name());

        // Resolve names in function body
        symbols_.push_scope();
        for(const auto& param : fd->params()) {
            auto param_type = param.type_annotation ? param.type_annotation : fresh_type_variable();
            symbols_.define(param.name, TypeScheme::mono(param_type));
        }
        for(const auto& s : fd->body().statements()) {
            resolve_names_stmt(*s);
        }
        symbols_.pop_scope();
        break;
    }
    case NodeKind::MainStmt: {
        // Register 'main' implicitly
        symbols_.define("main", TypeScheme::mono(PrimitiveType::void_()));
        break;
    }
    case NodeKind::VarDecl: {
        const auto* vd = static_cast<const VarDecl*>(&stmt);
        // Handle multi-variable declarations
        const auto& names = vd->names();
        for(const auto& name : names) {
            auto var_type = fresh_type_variable();
            symbols_.define(name, TypeScheme::mono(var_type));
            LTRACE("Resolved variable declaration: {}", name);
        }
        break;
    }
    case NodeKind::BlockStmt: {
        const auto* bs = static_cast<const BlockStmt*>(&stmt);
        symbols_.push_scope();
        for(const auto& s : bs->statements()) {
            resolve_names_stmt(*s);
        }
        symbols_.pop_scope();
        break;
    }
    default:
        break;
    }
}

// ============================================================
// Phase 2: Constraint Generation
// ============================================================
void TypeChecker::generate_constraints(const Program& program) {
    typed_stmts_.clear();
    current_function_return_type_.reset();  // Ensure we start at top-level (no enclosing function)
    for(const auto& stmt : program.statements()) {
        typed_stmts_.push_back(type_stmt(*stmt));
    }
}

// ============================================================
// Phase 3: Constraint Solving
// ============================================================
SolverResult TypeChecker::solve_constraints() {
    ConstraintSolver solver;
    return solver.solve(constraints_);
}

// ============================================================
// Phase 4: Zonking
// ============================================================
TypedProgram TypeChecker::zonk(const Substitution& subst) {
    std::vector<TypedStmtPtr> zonked_stmts;
    zonked_stmts.reserve(typed_stmts_.size());

    for(auto& stmt : typed_stmts_) {
        if(!stmt) continue;
        auto zonked = zonk_stmt_full(subst, *stmt);
        if(zonked) {
            zonked_stmts.push_back(std::move(zonked));
        } else {
            // Fallback: keep original if zonk didn't produce a new node
            zonked_stmts.push_back(std::move(stmt));
        }
    }
    typed_stmts_.clear();

    auto program_type = PrimitiveType::void_();
    return TypedProgram{std::move(zonked_stmts), std::move(program_type)};
}

TypedStmtPtr TypeChecker::zonk_stmt_full(const Substitution& subst, const TypedStmt& stmt) {
    switch(stmt.kind()) {
    case NodeKind::ExprStmt: {
        const auto* es = static_cast<const TypedExprStmt*>(&stmt);
        auto zonked_expr = zonk_expr_full(subst, es->expression());
        return std::make_unique<TypedExprStmt>(std::move(zonked_expr), PrimitiveType::void_(), stmt.location());
    }
    case NodeKind::VarDecl: {
        const auto* vd = static_cast<const TypedVarDecl*>(&stmt);
        auto resolved_type = zonk_type(subst, vd->node_type());
        TypedExprPtr zonked_init;
        if(vd->has_initializer()) {
            zonked_init = zonk_expr_full(subst, vd->initializer());
        }
        return std::make_unique<TypedVarDecl>(vd->name(), std::move(resolved_type), std::move(zonked_init), vd->is_const(), stmt.location());
    }
    case NodeKind::FuncDecl: {
        const auto* fd = static_cast<const TypedFuncDecl*>(&stmt);
        std::vector<TypedFuncParam> zonked_params;
        for(const auto& param : fd->params()) {
            auto resolved_param_type = zonk_type(subst, param.type_annotation);
            zonked_params.push_back(TypedFuncParam{param.name, std::move(resolved_param_type), param.loc});
        }

        auto zonked_body = zonk_block_full(subst, fd->body());
        auto resolved_func_type = zonk_type(subst, fd->node_type());
        std::optional<TypePtr> zonked_ret;
        if(fd->return_type()) {
            zonked_ret = zonk_type(subst, *fd->return_type());
        }

        return std::make_unique<TypedFuncDecl>(fd->name(), std::move(zonked_params), std::move(zonked_ret),
                                                std::move(zonked_body), std::move(resolved_func_type), stmt.location());
    }
    case NodeKind::ReturnStmt: {
        const auto* rs = static_cast<const TypedReturnStmt*>(&stmt);
        TypedExprPtr zonked_value;
        auto resolved_ret_type = zonk_type(subst, rs->node_type());
        if(rs->has_value()) {
            zonked_value = zonk_expr_full(subst, rs->value());
        }
        return std::make_unique<TypedReturnStmt>(std::move(zonked_value), std::move(resolved_ret_type), stmt.location());
    }
    case NodeKind::IfStmt: {
        const auto* is = static_cast<const TypedIfStmt*>(&stmt);
        auto zonked_cond = zonk_expr_full(subst, is->condition());
        auto zonked_then = zonk_stmt_full(subst, is->then_branch());
        TypedStmtPtr zonked_else;
        if(is->has_else()) {
            zonked_else = zonk_stmt_full(subst, is->else_branch());
        }
        return std::make_unique<TypedIfStmt>(std::move(zonked_cond), std::move(zonked_then), std::move(zonked_else),
                                              PrimitiveType::void_(), stmt.location());
    }
    case NodeKind::WhileStmt: {
        const auto* ws = static_cast<const TypedWhileStmt*>(&stmt);
        auto zonked_cond = zonk_expr_full(subst, ws->condition());
        auto zonked_body = zonk_stmt_full(subst, ws->body());
        return std::make_unique<TypedWhileStmt>(std::move(zonked_cond), std::move(zonked_body), PrimitiveType::void_(), stmt.location());
    }
    case NodeKind::ForStmt: {
        const auto* fs = static_cast<const TypedForStmt*>(&stmt);
        TypedStmtPtr zonked_init;
        if(fs->has_init()) { zonked_init = zonk_stmt_full(subst, fs->init()); }
        TypedExprPtr zonked_cond;
        if(fs->has_condition()) { zonked_cond = zonk_expr_full(subst, fs->condition()); }
        TypedExprPtr zonked_incr;
        if(fs->has_increment()) { zonked_incr = zonk_expr_full(subst, fs->increment()); }
        auto zonked_body = zonk_stmt_full(subst, fs->body());
        return std::make_unique<TypedForStmt>(std::move(zonked_init), std::move(zonked_cond), std::move(zonked_incr),
                                               std::move(zonked_body), PrimitiveType::void_(), stmt.location());
    }
    case NodeKind::BlockStmt: {
        return zonk_block_full(subst, static_cast<const TypedBlockStmt&>(stmt));
    }
    case NodeKind::BreakStmt: {
        return std::make_unique<TypedBreakStmt>(zonk_type(subst, stmt.node_type()), stmt.location());
    }
    case NodeKind::ContinueStmt: {
        return std::make_unique<TypedContinueStmt>(zonk_type(subst, stmt.node_type()), stmt.location());
    }
    case NodeKind::MainStmt: {
        const auto* ms = static_cast<const TypedMainStmt*>(&stmt);
        // MainStmt body is a TypedStmtPtr — try to cast to BlockStmt and zonk
        std::vector<TypedStmtPtr> zonked_body_stmts;
        if(const auto* body_block = dynamic_cast<const TypedBlockStmt*>(&ms->body())) {
            for(const auto& s : body_block->statements()) {
                if(s) {
                    auto zonked = zonk_stmt_full(subst, *s);
                    if(zonked) zonked_body_stmts.push_back(std::move(zonked));
                }
            }
        }
        auto zonked_body = std::make_unique<TypedBlockStmt>(std::move(zonked_body_stmts), PrimitiveType::void_(), stmt.location());
        return std::make_unique<TypedMainStmt>(std::move(zonked_body), PrimitiveType::void_(), stmt.location());
    }
    default:
        return nullptr;
    }
}

TypedExprPtr TypeChecker::zonk_expr_full(const Substitution& subst, const TypedExpr& expr) {
    auto resolved_type = zonk_type(subst, expr.node_type());

    switch(expr.kind()) {
    case NodeKind::IntegerLiteral: {
        const auto* lit = static_cast<const TypedIntegerLiteral*>(&expr);
        return std::make_unique<TypedIntegerLiteral>(lit->value(), std::move(resolved_type), expr.location(), lit->type_suffix());
    }
    case NodeKind::FloatLiteral: {
        const auto* lit = static_cast<const TypedFloatLiteral*>(&expr);
        return std::make_unique<TypedFloatLiteral>(lit->value(), std::move(resolved_type), expr.location());
    }
    case NodeKind::StringLiteral: {
        const auto* lit = static_cast<const TypedStringLiteral*>(&expr);
        return std::make_unique<TypedStringLiteral>(lit->value(), std::move(resolved_type), expr.location());
    }
    case NodeKind::CharLiteral: {
        const auto* lit = static_cast<const TypedCharLiteral*>(&expr);
        return std::make_unique<TypedCharLiteral>(lit->value(), std::move(resolved_type), expr.location());
    }
    case NodeKind::BoolLiteral: {
        const auto* lit = static_cast<const TypedBoolLiteral*>(&expr);
        return std::make_unique<TypedBoolLiteral>(lit->value(), std::move(resolved_type), expr.location());
    }
    case NodeKind::NullLiteral: {
        return std::make_unique<TypedNullLiteral>(std::move(resolved_type), expr.location());
    }
    case NodeKind::Identifier: {
        const auto* id = static_cast<const TypedIdentifier*>(&expr);
        return std::make_unique<TypedIdentifier>(id->name(), std::move(resolved_type), expr.location());
    }
    case NodeKind::UnaryExpr: {
        const auto* un = static_cast<const TypedUnaryExpr*>(&expr);
        auto zonked_operand = zonk_expr_full(subst, un->operand());
        return std::make_unique<TypedUnaryExpr>(un->op(), std::move(zonked_operand), std::move(resolved_type), expr.location());
    }
    case NodeKind::BinaryExpr: {
        const auto* bin = static_cast<const TypedBinaryExpr*>(&expr);
        auto zonked_lhs = zonk_expr_full(subst, bin->lhs());
        auto zonked_rhs = zonk_expr_full(subst, bin->rhs());
        return std::make_unique<TypedBinaryExpr>(bin->op(), std::move(zonked_lhs), std::move(zonked_rhs), std::move(resolved_type), expr.location());
    }
    case NodeKind::TernaryExpr: {
        const auto* ter = static_cast<const TypedTernaryExpr*>(&expr);
        auto zonked_cond = zonk_expr_full(subst, ter->condition());
        auto zonked_then = zonk_expr_full(subst, ter->then_expr());
        auto zonked_else = zonk_expr_full(subst, ter->else_expr());
        return std::make_unique<TypedTernaryExpr>(std::move(zonked_cond), std::move(zonked_then), std::move(zonked_else), std::move(resolved_type), expr.location());
    }
    case NodeKind::CallExpr: {
        const auto* call = static_cast<const TypedCallExpr*>(&expr);
        auto zonked_callee = zonk_expr_full(subst, call->callee());
        std::vector<TypedExprPtr> zonked_args;
        for(const auto& arg : call->args()) {
            zonked_args.push_back(arg ? zonk_expr_full(subst, *arg) : nullptr);
        }
        return std::make_unique<TypedCallExpr>(std::move(zonked_callee), std::move(zonked_args), std::move(resolved_type), expr.location());
    }
    case NodeKind::IndexExpr: {
        const auto* idx = static_cast<const TypedIndexExpr*>(&expr);
        auto zonked_object = zonk_expr_full(subst, idx->object());
        auto zonked_index = zonk_expr_full(subst, idx->index());
        return std::make_unique<TypedIndexExpr>(std::move(zonked_object), std::move(zonked_index), std::move(resolved_type), expr.location());
    }
    case NodeKind::MemberExpr: {
        const auto* mem = static_cast<const TypedMemberExpr*>(&expr);
        auto zonked_object = zonk_expr_full(subst, mem->object());
        return std::make_unique<TypedMemberExpr>(std::move(zonked_object), mem->member(), std::move(resolved_type), expr.location());
    }
    case NodeKind::AssignExpr: {
        const auto* assign = static_cast<const TypedAssignExpr*>(&expr);
        auto zonked_target = zonk_expr_full(subst, assign->target());
        auto zonked_value = zonk_expr_full(subst, assign->value());
        return std::make_unique<TypedAssignExpr>(std::move(zonked_target), std::move(zonked_value), std::move(resolved_type), expr.location());
    }
    case NodeKind::CastExpr: {
        const auto* cast = static_cast<const TypedCastExpr*>(&expr);
        auto zonked_operand = zonk_expr_full(subst, cast->operand());
        return std::make_unique<TypedCastExpr>(cast->target_type(), std::move(zonked_operand), std::move(resolved_type), expr.location());
    }
    case NodeKind::ArrayLiteral: {
        const auto* arr = static_cast<const TypedArrayLiteral*>(&expr);
        std::vector<TypedExprPtr> zonked_elements;
        for(const auto& elem : arr->elements()) {
            zonked_elements.push_back(elem ? zonk_expr_full(subst, *elem) : nullptr);
        }
        return std::make_unique<TypedArrayLiteral>(std::move(zonked_elements), std::move(resolved_type), expr.location());
    }
    case NodeKind::GroupingExpr: {
        const auto* grp = static_cast<const TypedGroupingExpr*>(&expr);
        auto zonked_inner = zonk_expr_full(subst, grp->expression());
        return std::make_unique<TypedGroupingExpr>(std::move(zonked_inner), std::move(resolved_type), expr.location());
    }
    default:
        return nullptr;
    }
}

std::unique_ptr<TypedBlockStmt> TypeChecker::zonk_block_full(const Substitution& subst, const TypedBlockStmt& block) {
    std::vector<TypedStmtPtr> zonked_stmts;
    for(const auto& s : block.statements()) {
        if(s) {
            auto zonked = zonk_stmt_full(subst, *s);
            if(zonked) {
                zonked_stmts.push_back(std::move(zonked));
            } else {
                // Can't move from const — skip (original kept by callee)
            }
        }
    }
    auto resolved_type = zonk_type(subst, block.node_type());
    return std::make_unique<TypedBlockStmt>(std::move(zonked_stmts), std::move(resolved_type), block.location());
}

// ============================================================
// type_expr — Constraint generation for expressions
// ============================================================
TypedExprPtr TypeChecker::type_expr(const Expr& expr) {
    switch(expr.kind()) {
    case NodeKind::IntegerLiteral: {
        const auto* lit = static_cast<const IntegerLiteral*>(&expr);
        TypePtr type;
        if(const auto& suffix = lit->type_suffix(); suffix.has_value()) {
            // Use the explicit type suffix to determine the literal's type
            type = parse_type_annotation(*suffix);
            if(!type) {
                // Unknown suffix — fall back to i32
                type = PrimitiveType::i32();
            }
        } else {
            // No suffix — default to i64
            type = PrimitiveType::i64();
        }
        constraints_.add(type, type, lit->location(), FORMAT("integer literal type: {}", type->to_string()));
        return std::make_unique<TypedIntegerLiteral>(lit->value(), std::move(type), lit->location(), lit->type_suffix());
    }
    case NodeKind::FloatLiteral: {
        const auto* lit = static_cast<const FloatLiteral*>(&expr);
        // Float literals with suffix: f32, f64. No suffix defaults to f64.
        // The FloatLiteral doesn't have a type_suffix field, so we default to f64.
        // If the parser ever adds suffix support, check it here.
        auto type = PrimitiveType::f64();
        constraints_.add(type, type, lit->location(), "float literal defaults to f64");
        return std::make_unique<TypedFloatLiteral>(lit->value(), std::move(type), lit->location());
    }
    case NodeKind::StringLiteral: {
        const auto* lit = static_cast<const StringLiteral*>(&expr);
        auto type = PrimitiveType::string();
        constraints_.add(type, type, lit->location(), "string literal type");
        return std::make_unique<TypedStringLiteral>(lit->value(), std::move(type), lit->location());
    }
    case NodeKind::CharLiteral: {
        const auto* lit = static_cast<const CharLiteral*>(&expr);
        auto type = PrimitiveType::char_();
        constraints_.add(type, type, lit->location(), "char literal type");
        return std::make_unique<TypedCharLiteral>(lit->value(), std::move(type), lit->location());
    }
    case NodeKind::BoolLiteral: {
        const auto* lit = static_cast<const BoolLiteral*>(&expr);
        auto type = PrimitiveType::bool_();
        constraints_.add(type, type, lit->location(), "bool literal type");
        return std::make_unique<TypedBoolLiteral>(lit->value(), std::move(type), lit->location());
    }
    case NodeKind::NullLiteral: {
        const auto* lit = static_cast<const NullLiteral*>(&expr);
        auto type = PrimitiveType::nullptr_();
        constraints_.add(type, type, lit->location(), "null literal type");
        return std::make_unique<TypedNullLiteral>(std::move(type), lit->location());
    }
    case NodeKind::Identifier: {
        const auto* id = static_cast<const Identifier*>(&expr);
        auto sym = symbols_.lookup(id->name());
        if(!sym) {
            message_storage_.push_back(
                FORMAT("Undeclared identifier: {}", id->name()));
            errors_.push_back(CompileError::TypeError(
                ErrorCode::E2033,
                message_storage_.back(),
                id->location(),
                FORMAT("Identifier '{}' was not declared in this scope", id->name())));
            return std::make_unique<TypedIdentifier>(id->name(), error_type(), id->location());
        }
        // Instantiate the type scheme to get fresh type variables for polymorphic values
        return std::make_unique<TypedIdentifier>(id->name(), sym->instantiate(), id->location());
    }
    case NodeKind::BinaryExpr: {
        const auto* bin = static_cast<const BinaryExpr*>(&expr);
        auto lhs_typed = type_expr(bin->lhs());
        auto rhs_typed = type_expr(bin->rhs());

        const auto& lhs_type = lhs_typed->node_type();
        const auto& rhs_type = rhs_typed->node_type();

        TypePtr result_type;
        switch(bin->op()) {
        case BinaryOp::Add:
        case BinaryOp::Sub:
        case BinaryOp::Mul:
        case BinaryOp::Div:
        case BinaryOp::Mod:
            constraints_.add(lhs_type, rhs_type, bin->location(), "binary arithmetic: operands must match");
            result_type = lhs_type;
            break;
        case BinaryOp::Eq:
        case BinaryOp::Neq:
        case BinaryOp::Lt:
        case BinaryOp::Gt:
        case BinaryOp::Le:
        case BinaryOp::Ge:
            constraints_.add(lhs_type, rhs_type, bin->location(), "comparison: operands must match");
            result_type = PrimitiveType::bool_();
            break;
        case BinaryOp::And:
        case BinaryOp::Or:
            constraints_.add(lhs_type, PrimitiveType::bool_(), bin->location(), "logical op: lhs must be bool");
            constraints_.add(rhs_type, PrimitiveType::bool_(), bin->location(), "logical op: rhs must be bool");
            result_type = PrimitiveType::bool_();
            break;
        default:
            result_type = fresh_type_variable();
            constraints_.add(result_type, lhs_type, bin->location(), "binary expression default");
            break;
        }

        return std::make_unique<TypedBinaryExpr>(bin->op(), std::move(lhs_typed), std::move(rhs_typed), std::move(result_type), bin->location());
    }
    case NodeKind::UnaryExpr: {
        const auto* un = static_cast<const UnaryExpr*>(&expr);
        auto operand_typed = type_expr(un->operand());
        const auto& operand_type = operand_typed->node_type();

        TypePtr result_type;
        switch(un->op()) {
        case UnaryOp::Negate:
            result_type = operand_type;
            break;
        case UnaryOp::Not:
            constraints_.add(operand_type, PrimitiveType::bool_(), un->location(), "not: operand must be bool");
            result_type = PrimitiveType::bool_();
            break;
        case UnaryOp::PreInc:
        case UnaryOp::PostInc:
        case UnaryOp::PreDec:
        case UnaryOp::PostDec:
            constraints_.add(operand_type, PrimitiveType::i32(), un->location(), "inc/dec: operand must be integer");
            result_type = operand_type;
            break;
        default:
            result_type = fresh_type_variable();
            constraints_.add(result_type, operand_type, un->location(), "unary expression default");
            break;
        }

        return std::make_unique<TypedUnaryExpr>(un->op(), std::move(operand_typed), std::move(result_type), un->location());
    }
    case NodeKind::CallExpr: {
        const auto* call = static_cast<const CallExpr*>(&expr);

        // Type the callee
        auto callee_typed = type_expr(call->callee());
        auto callee_type = callee_typed->node_type();

        // Type all arguments
        std::vector<TypedExprPtr> typed_args;
        typed_args.reserve(call->args().size());
        std::vector<TypePtr> arg_types;
        arg_types.reserve(call->args().size());
        for(const auto& arg : call->args()) {
            auto typed_arg = type_expr(*arg);
            arg_types.push_back(typed_arg->node_type());
            typed_args.push_back(std::move(typed_arg));
        }

        // Create fresh type variables for argument types and return type
        auto result_type = fresh_type_variable();

        // Build expected function type: (arg1, arg2, ...) -> result
        // For now, generate pairwise constraints between callee type and each arg
        // A proper function type would be: Fn([arg_types...], result_type)
        // Since we don't have a function type yet, constrain callee against a type variable
        // and constrain each argument against expected parameter types

        // Look up the function in the symbol table to get its signature
        // For now, we generate a constraint that the callee must be callable
        // This is a simplification — a real implementation would use function types
        if(const auto* ident = dynamic_cast<const Identifier*>(&call->callee())) {
            if(auto sym = symbols_.lookup(ident->name())) {
                // If callee has a known type, constrain arguments
                // For polymorphic functions, instantiate fresh type variables
                // This is a placeholder — full function type support needs Fn types
            }
        }

        // Generate constraint: callee type must be compatible with (arg_types) -> result_type
        // This requires a proper function type representation
        // For now, we just ensure the callee is typed and args are typed
        constraints_.add(callee_type, callee_type, call->location(), "call expression callee");

        return std::make_unique<TypedCallExpr>(std::move(callee_typed), std::move(typed_args), std::move(result_type), call->location());
    }
    case NodeKind::ArrayLiteral: {
        const auto* arr = static_cast<const ArrayLiteral*>(&expr);
        auto elem_type = fresh_type_variable();
        std::vector<TypedExprPtr> typed_elements;
        typed_elements.reserve(arr->elements().size());

        for(const auto& elem : arr->elements()) {
            auto typed_elem = type_expr(*elem);
            constraints_.add(typed_elem->node_type(), elem_type, elem->location(), "array element type consistency");
            typed_elements.push_back(std::move(typed_elem));
        }

        auto size_expr = std::make_unique<IntegerLiteral>(static_cast<std::int64_t>(arr->elements().size()));
        auto array_type = std::make_shared<ArrayType>(elem_type, std::move(size_expr));
        constraints_.add(array_type, array_type, arr->location(), "array literal type");
        return std::make_unique<TypedArrayLiteral>(std::move(typed_elements), std::move(array_type), arr->location());
    }
    case NodeKind::GroupingExpr: {
        const auto* grp = static_cast<const GroupingExpr*>(&expr);
        auto inner = type_expr(grp->expression());
        return std::make_unique<TypedGroupingExpr>(std::move(inner), inner->node_type(), grp->location());
    }
    case NodeKind::AssignExpr: {
        const auto* assign = static_cast<const AssignExpr*>(&expr);
        auto target_typed = type_expr(assign->target());
        auto value_typed = type_expr(assign->value());

        constraints_.add(target_typed->node_type(), value_typed->node_type(), assign->location(),
                         "assignment: LHS type must match RHS type");

        return std::make_unique<TypedAssignExpr>(std::move(target_typed), std::move(value_typed),
                                                  target_typed->node_type(), assign->location());
    }
    case NodeKind::TernaryExpr: {
        const auto* ter = static_cast<const TernaryExpr*>(&expr);
        auto cond_typed = type_expr(ter->condition());
        constraints_.add(cond_typed->node_type(), PrimitiveType::bool_(), ter->location(), "ternary condition must be bool");

        auto then_typed = type_expr(ter->then_expr());
        auto else_typed = type_expr(ter->else_expr());

        // Both branches must have the same type
        constraints_.add(then_typed->node_type(), else_typed->node_type(), ter->location(), "ternary branches must match type");

        return std::make_unique<TypedTernaryExpr>(std::move(cond_typed), std::move(then_typed), std::move(else_typed),
                                                   then_typed->node_type(), ter->location());
    }
    case NodeKind::IndexExpr: {
        const auto* idx = static_cast<const IndexExpr*>(&expr);
        auto obj_typed = type_expr(idx->object());
        auto index_typed = type_expr(idx->index());

        // Index must be integer
        constraints_.add(index_typed->node_type(), PrimitiveType::i32(), idx->location(), "index must be integer");

        // Result type is a fresh variable (element type of array)
        auto result_type = fresh_type_variable();

        return std::make_unique<TypedIndexExpr>(std::move(obj_typed), std::move(index_typed), std::move(result_type), idx->location());
    }
    case NodeKind::MemberExpr: {
        const auto* mem = static_cast<const MemberExpr*>(&expr);
        auto obj_typed = type_expr(mem->object());
        auto result_type = fresh_type_variable();

        return std::make_unique<TypedMemberExpr>(std::move(obj_typed), mem->member(), std::move(result_type), mem->location());
    }
    case NodeKind::CastExpr: {
        const auto* cast = static_cast<const CastExpr*>(&expr);
        auto operand_typed = type_expr(cast->operand());

        // Parse the target type annotation
        auto target_type = parse_type_annotation(cast->target_type());
        if(!target_type) {
            // Unknown target type — use a fresh type variable
            target_type = fresh_type_variable();
        }

        return std::make_unique<TypedCastExpr>(cast->target_type(), std::move(operand_typed), std::move(target_type), cast->location());
    }
    default:
        errors_.push_back(CompileError::TypeError(
            ErrorCode::E2033,
            "Constraint generation failed for expression",
            expr.location(),
            FORMAT("Expression kind {} is not yet supported", static_cast<int>(expr.kind()))));
        return std::make_unique<TypedIdentifier>(std::string{"unknown"}, error_type(), expr.location());
    }
}

// ============================================================
// type_stmt — Constraint generation for statements
// ============================================================
TypedStmtPtr TypeChecker::type_stmt(const Stmt& stmt) {
    switch(stmt.kind()) {
    case NodeKind::ExprStmt: {
        const auto* es = static_cast<const ExprStmt*>(&stmt);
        auto typed_expr = type_expr(es->expression());
        return std::make_unique<TypedExprStmt>(std::move(typed_expr), PrimitiveType::void_(), es->location());
    }
    case NodeKind::VarDecl: {
        const auto* vd = static_cast<const VarDecl*>(&stmt);
        const auto& names = vd->names();
        const auto& initializers = vd->initializers();

        TypePtr var_type;

        if(vd->type_annotation()) {
            var_type = parse_type_annotation(*vd->type_annotation());
        }

        if(names.size() == 1) {
            // Single variable declaration
            if(!initializers.empty() && initializers[0]) {
                auto typed_init = type_expr(*initializers[0]);
                if(var_type) {
                    constraints_.add(var_type, typed_init->node_type(), vd->location(),
                                     FORMAT("variable '{}' type annotation vs initializer", names[0]));
                } else {
                    var_type = typed_init->node_type();
                }
                symbols_.define(names[0], TypeScheme::mono(var_type));
                return std::make_unique<TypedVarDecl>(names[0], std::move(var_type), std::move(typed_init), vd->is_const(), vd->location());
            } else {
                // No initializer — use annotation or fresh type variable
                if(!var_type) { var_type = fresh_type_variable(); }
                symbols_.define(names[0], TypeScheme::mono(var_type));
                return std::make_unique<TypedVarDecl>(names[0], std::move(var_type), nullptr, vd->is_const(), vd->location());
            }
        } else {
            // Multi-variable declaration — simplify: create one TypedVarDecl with first name
            // (Full multi-var support needs a different TypedVarDecl structure)
            TypePtr multi_type = var_type ? var_type : fresh_type_variable();
            for(std::size_t i = 0; i < names.size(); ++i) {
                TypePtr elem_type = multi_type;
                if(i < initializers.size() && initializers[i]) {
                    auto typed_init = type_expr(*initializers[i]);
                    if(elem_type) {
                        constraints_.add(elem_type, typed_init->node_type(), vd->location(),
                                         FORMAT("variable '{}' type annotation vs initializer", names[i]));
                    } else {
                        elem_type = typed_init->node_type();
                    }
                } else if(!elem_type) {
                    elem_type = fresh_type_variable();
                }
                symbols_.define(names[i], TypeScheme::mono(elem_type));
            }
            // Return a simplified single var decl for the first variable
            TypedExprPtr first_init;
            if(!initializers.empty() && initializers[0]) {
                first_init = type_expr(*initializers[0]);
            }
            return std::make_unique<TypedVarDecl>(names[0], std::move(multi_type), std::move(first_init), vd->is_const(), vd->location());
        }
    }
    case NodeKind::FuncDecl: {
        const auto* fd = static_cast<const FuncDecl*>(&stmt);
        current_function_return_type_ = fd->return_type() ? *fd->return_type() : PrimitiveType::void_();
        current_function_name_ = fd->name();

        symbols_.push_scope();  // Function scope

        std::vector<TypedFuncParam> typed_params;
        typed_params.reserve(fd->params().size());
        for(const auto& param : fd->params()) {
            TypePtr param_type;
            if(param.type_annotation) {
                param_type = param.type_annotation;
            } else {
                param_type = fresh_type_variable();
            }
            symbols_.define(param.name, TypeScheme::mono(param_type));
            typed_params.push_back(TypedFuncParam{param.name, std::move(param_type), param.loc});
        }

        const auto& body = fd->body();
        std::vector<TypedStmtPtr> typed_body_stmts;
        for(const auto& s : body.statements()) {
            typed_body_stmts.push_back(type_stmt(*s));
        }

        symbols_.pop_scope();
        current_function_return_type_.reset();
        current_function_name_.reset();

        auto func_type = fd->return_type() ? *fd->return_type() : PrimitiveType::void_();
        auto typed_body_stmt = std::make_unique<TypedBlockStmt>(std::move(typed_body_stmts), func_type, fd->location());
        return std::make_unique<TypedFuncDecl>(fd->name(), std::move(typed_params), fd->return_type(),
                                                std::move(typed_body_stmt), std::move(func_type), fd->location());
    }
    case NodeKind::ReturnStmt: {
        const auto* rs = static_cast<const ReturnStmt*>(&stmt);
        TypedExprPtr typed_value;
        TypePtr return_type = PrimitiveType::void_();

        if(!current_function_return_type_) {
            errors_.push_back(CompileError::TypeError(
                ErrorCode::E2005,
                "Return statement must be inside function body",
                rs->location(),
                "Return statements are only valid inside functions."));
            return std::make_unique<TypedReturnStmt>(nullptr, PrimitiveType::void_(), rs->location());
        }

        if(rs->has_value()) {
            typed_value = type_expr(rs->value());
            return_type = typed_value->node_type();

            // Check: cannot return a value from a void function
            if(*current_function_return_type_ && (*current_function_return_type_)->kind() == TypeKind::Void) {
                errors_.push_back(CompileError::TypeError(
                    ErrorCode::E2006,
                    "Cannot return a value from void function",
                    rs->location(),
                    "Remove the return value or change the function's return type."));
            } else if(current_function_return_type_) {
                // Early check: concrete type mismatch for return statements → E2007
                // Both types are resolved (not type variables) — compare directly
                bool mismatch_reported = false;
                if(return_type->kind() != TypeKind::TypeVar &&
                   (*current_function_return_type_)->kind() != TypeKind::TypeVar) {
                    if(return_type->to_string() != (*current_function_return_type_)->to_string()) {
                        message_storage_.push_back(
                            FORMAT("Return type mismatch, expected {} found {}",
                                   (*current_function_return_type_)->to_string(),
                                   return_type->to_string()));
                        errors_.push_back(CompileError::TypeError(
                            ErrorCode::E2007,
                            message_storage_.back(),
                            rs->location(),
                            "Change the return value type or update the function's return type."));
                        mismatch_reported = true;
                    }
                }
                // Only add constraint if no concrete mismatch was already reported
                if(!mismatch_reported) {
                    constraints_.add(return_type, *current_function_return_type_, rs->location(),
                                     "return type must match function declaration");
                }
            }
        } else {
            // Void return — function expects a value but none provided
            if(*current_function_return_type_ && (*current_function_return_type_)->kind() != TypeKind::Void) {
                message_storage_.push_back(
                    FORMAT("Return type mismatch, expected {} found void",
                           (*current_function_return_type_)->to_string()));
                errors_.push_back(CompileError::TypeError(
                    ErrorCode::E2008,
                    message_storage_.back(),
                    rs->location(),
                    "Return statement has no value but function expects a return type."));
            }
        }

        return std::make_unique<TypedReturnStmt>(std::move(typed_value), std::move(return_type), rs->location());
    }
    case NodeKind::IfStmt: {
        const auto* is = static_cast<const IfStmt*>(&stmt);
        auto typed_cond = type_expr(is->condition());
        constraints_.add(typed_cond->node_type(), PrimitiveType::bool_(), is->location(), "if condition must be bool");

        auto typed_then = type_stmt(is->then_branch());
        TypedStmtPtr typed_else;
        if(is->has_else()) { typed_else = type_stmt(is->else_branch()); }

        return std::make_unique<TypedIfStmt>(std::move(typed_cond), std::move(typed_then), std::move(typed_else),
                                              PrimitiveType::void_(), is->location());
    }
    case NodeKind::WhileStmt: {
        const auto* ws = static_cast<const WhileStmt*>(&stmt);
        auto typed_cond = type_expr(ws->condition());
        constraints_.add(typed_cond->node_type(), PrimitiveType::bool_(), ws->location(), "while condition must be bool");

        ++loop_depth_;
        auto typed_body = type_stmt(ws->body());
        --loop_depth_;
        return std::make_unique<TypedWhileStmt>(std::move(typed_cond), std::move(typed_body), PrimitiveType::void_(), ws->location());
    }
    case NodeKind::ForStmt: {
        const auto* fs = static_cast<const ForStmt*>(&stmt);

        symbols_.push_scope();  // For-loop scope

        TypedStmtPtr typed_init;
        if(fs->has_init()) { typed_init = type_stmt(fs->init()); }

        TypedExprPtr typed_cond;
        if(fs->has_condition()) {
            typed_cond = type_expr(fs->condition());
            constraints_.add(typed_cond->node_type(), PrimitiveType::bool_(), fs->location(), "for condition must be bool");
        }

        TypedExprPtr typed_incr;
        if(fs->has_increment()) { typed_incr = type_expr(fs->increment()); }

        ++loop_depth_;
        auto typed_body = type_stmt(fs->body());
        --loop_depth_;

        symbols_.pop_scope();

        return std::make_unique<TypedForStmt>(std::move(typed_init), std::move(typed_cond), std::move(typed_incr),
                                               std::move(typed_body), PrimitiveType::void_(), fs->location());
    }
    case NodeKind::BlockStmt: {
        const auto* bs = static_cast<const BlockStmt*>(&stmt);
        symbols_.push_scope();
        std::vector<TypedStmtPtr> typed_stmts;
        for(const auto& s : bs->statements()) { typed_stmts.push_back(type_stmt(*s)); }
        symbols_.pop_scope();

        return std::make_unique<TypedBlockStmt>(std::move(typed_stmts), PrimitiveType::void_(), bs->location());
    }
    case NodeKind::BreakStmt: {
        if(loop_depth_ <= 0) {
            errors_.push_back(CompileError::TypeError(
                ErrorCode::E2009,
                "Break statement outside loop",
                stmt.location(),
                "Break is only valid inside for or while loops."));
        }
        return std::make_unique<TypedBreakStmt>(PrimitiveType::void_(), stmt.location());
    }
    case NodeKind::ContinueStmt: {
        if(loop_depth_ <= 0) {
            errors_.push_back(CompileError::TypeError(
                ErrorCode::E2010,
                "Continue statement outside loop",
                stmt.location(),
                "Continue is only valid inside for or while loops."));
        }
        return std::make_unique<TypedContinueStmt>(PrimitiveType::void_(), stmt.location());
    }
    case NodeKind::MainStmt: {
        const auto* ms = static_cast<const MainStmt*>(&stmt);
        // MainStmt body is a StmtPtr, typically a BlockStmt
        std::vector<TypedStmtPtr> typed_body_stmts;
        // Main is implicitly a void function — allow return statements inside it
        current_function_return_type_ = PrimitiveType::void_();
        current_function_name_ = "main";
        if(const auto* body_block = dynamic_cast<const BlockStmt*>(&ms->body())) {
            for(const auto& s : body_block->statements()) {
                typed_body_stmts.push_back(type_stmt(*s));
            }
        } else {
            typed_body_stmts.push_back(type_stmt(ms->body()));
        }
        current_function_return_type_.reset();
        current_function_name_.reset();
        auto typed_body = std::make_unique<TypedBlockStmt>(std::move(typed_body_stmts), PrimitiveType::void_(), ms->location());
        return std::make_unique<TypedMainStmt>(std::move(typed_body), PrimitiveType::void_(), ms->location());
    }
    default:
        errors_.push_back(CompileError::TypeError(
            ErrorCode::E2033,
            "Constraint generation failed for statement",
            stmt.location(),
            FORMAT("Statement kind {} is not yet supported", static_cast<int>(stmt.kind()))));
        return std::make_unique<TypedExprStmt>(
            std::make_unique<TypedIdentifier>("unknown", error_type(), stmt.location()),
            PrimitiveType::void_(), stmt.location());
    }
}

}  // namespace jsv

DISABLE_WARNINGS_POP()
