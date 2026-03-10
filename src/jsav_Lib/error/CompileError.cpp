/*
 * Created by gbian on 09/03/2026.
 * Copyright (c) 2026 All rights reserved.
 */
// NOLINTBEGIN(*-include-cleaner)
#include "jsav/error/CompileError.hpp"

namespace jsv {
    // ---------------------------------------------------------------------------
    // Factory methods
    // ---------------------------------------------------------------------------

    CompileError CompileError::LexerError(std::optional<ErrorCode> code, std::string_view message, const SourceSpan &span,
                                          std::optional<std::string> help) {
        return {Kind::LexerError, vnd_move(code), message, span, vnd_move(help)};
    }

    /*CompileError CompileError::SyntaxError(std::optional<jsv::ErrorCode> code, std::string_view message, const SourceSpan& span,
                                           std::optional<std::string> help) {
        return {Kind::SyntaxError, vnd_move(code), message, span, vnd_move(help)};
    }

    CompileError CompileError::TypeError(std::optional<jsv::ErrorCode> code, std::string_view message, const SourceSpan& span,
                                         std::optional<std::string> help) {
        return {Kind::TypeError, vnd_move(code), message, span, vnd_move(help)};
    }

    CompileError CompileError::IrGeneratorError(std::optional<jsv::ErrorCode> code, std::string_view message, const SourceSpan& span,
                                                std::optional<std::string> help) {
        return {Kind::IrGeneratorError, vnd_move(code), message, span, vnd_move(help)};
    }

    CompileError CompileError::AsmGeneratorError(std::optional<jsv::ErrorCode> code, std::string_view message) {
        return {Kind::AsmGeneratorError, vnd_move(code), message, SourceSpan{}, std::nullopt};
    }*/

    // ---------------------------------------------------------------------------
    // what
    // ---------------------------------------------------------------------------
    std::string CompileError::what() const {
        // LexerError is currently the only Kind; structured for future extension.
        switch(kind_) {
        case Kind::LexerError:
            {
                std::string result;
                if(code_.has_value()) {
                    result = FORMAT("[{}] Syntax error: {} at {}", jsv::code(code_.value()), message_, span_);
                } else {
                    result = FORMAT("Syntax error: {} at {}", message_, span_);
                }
                if(help_.has_value()) {
                    result += FORMAT("\nhelp: {}", help_.value());
                }
                return result;
            }
        }
        return {};
    }

    // ---------------------------------------------------------------------------
    // Accessors
    // ---------------------------------------------------------------------------

    const std::optional<ErrorCode> &CompileError::error_code() const {
        switch(kind_) {
        case Kind::LexerError:
            /*case Kind::SyntaxError:
            case Kind::TypeError:
            case Kind::IrGeneratorError:
            case Kind::AsmGeneratorError:*/
            return code_;
        }
        return code_;  // fallback (code_ sarà nullopt per i kind senza codice)
    }

    std::string_view CompileError::message() const {
        switch(kind_) {
        case Kind::LexerError:
            /*case Kind::SyntaxError:
            case Kind::TypeError:
            case Kind::IrGeneratorError:
            case Kind::AsmGeneratorError:*/
            return message_;
        }
        return std::string_view{};
    }

    const SourceSpan &CompileError::span() const {
        switch(kind_) {
        case Kind::LexerError:
        /*case Kind::SyntaxError:
        case Kind::TypeError:
        case Kind::IrGeneratorError:*/
        default:
            return span_;
        }
    }

    std::optional<const std::string *> CompileError::help() const {
        switch(kind_) {
        case Kind::LexerError:
            /*case Kind::SyntaxError:
            case Kind::TypeError:
            case Kind::IrGeneratorError:*/
            return help_ ? std::optional<const std::string *>(&(*help_)) : std::nullopt;
        default:
            return std::nullopt;
        }
    }

    CompileError::Kind CompileError::kind() const { return kind_; }

    // ---------------------------------------------------------------------------
    // Mutators
    // ---------------------------------------------------------------------------

    void CompileError::set_message(std::string_view new_message) {
        switch(kind_) {
        case Kind::LexerError:
            /*case Kind::SyntaxError:
            case Kind::TypeError:
            case Kind::IrGeneratorError:
            case Kind::AsmGeneratorError:*/
            message_ = new_message;
            break;
        }
    }

    void CompileError::set_span(SourceSpan new_span) {
        switch(kind_) {
        case Kind::LexerError:
            /*case Kind::SyntaxError:
            case Kind::TypeError:
            case Kind::IrGeneratorError:*/
            span_ = vnd_move(new_span);
            break;
        default:
            break;
        }
    }

    void CompileError::set_help(std::optional<std::string> new_help) {
        switch(kind_) {
        case Kind::LexerError:
            /*case Kind::SyntaxError:
            case Kind::TypeError:
            case Kind::IrGeneratorError:*/
            help_ = vnd_move(new_help);
            break;
        default:
            break;
        }
    }
}  // namespace jsv

// NOLINTEND(*-include-cleaner)