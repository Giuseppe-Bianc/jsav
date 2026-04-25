/*
 * Created by gbian on 09/03/2026.
 * Copyright (c) 2026 All rights reserved.
 */

#pragma once

#include "../headers.hpp"
#include "../location/SourceSpan.hpp"
#include "error_codes.hpp"

namespace jsv {

    class CompileError {
    public:
        enum class Kind {
            LexerError,
            SyntaxError,
            TypeError,
            IrGeneratorError,
            /*AsmGeneratorError,
            IoError,*/
        };

        // --- Factory methods ---

        static CompileError LexerError(std::optional<ErrorCode> code, std::string_view message, const SourceSpan &span,
                                       std::optional<std::string> help);

        static CompileError SyntaxError(std::optional<ErrorCode> code, std::string_view message, const SourceSpan &span,
                                        std::optional<std::string> help);

        static CompileError TypeError(std::optional<ErrorCode> code, std::string_view message, const SourceSpan &span,
                                      std::optional<std::string> help);

        static CompileError IrGeneratorError(std::optional<ErrorCode> code, std::string_view message, const SourceSpan &span,
                                       std::optional<std::string> help);

        /*static CompileError AsmGeneratorError(
            std::optional<ErrorCode> code,
            std::string_view message
        );*/

        // --- Accessors ---

        std::string what() const;
        const std::optional<ErrorCode> &error_code() const;
        std::string_view message() const;
        const SourceSpan &span() const;
        std::optional<const std::string *> help() const;
        Kind kind() const;

        // --- Mutators ---

        void set_message(std::string_view new_message);
        void set_span(SourceSpan new_span);
        void set_help(std::optional<std::string> new_help);

    private:
        CompileError(Kind kind, std::optional<ErrorCode> code, std::string_view message, const SourceSpan &span,
                     std::optional<std::string> help)
                    : span_(span), message_(message), help_(vnd_move(help)), code_(vnd_move(code)), kind_(kind) {}

        SourceSpan span_;
        std::string message_;
        std::optional<std::string> help_;
        std::optional<ErrorCode> code_;
        Kind kind_;
    };
}  // namespace jsv
