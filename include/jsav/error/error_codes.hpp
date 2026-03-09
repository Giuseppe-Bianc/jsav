// Sistema di Codici di Errore per il Compilatore jsavrs
//
// Questo modulo fornisce un sistema completo e standardizzato di gestione dei codici di errore
// per il compilatore jsavrs. Ogni errore ha un identificatore unico (es. E0001) che
// abilita riferimenti rapidi, ricerca nella documentazione e integrazione con IDE.
//
// Intervalli Codici di Errore
//
// | Intervallo | Fase | Descrizione |
// |------------|------|-------------|
// | E0001-E0999 | Analisi Lessicale | Riconoscimento token, letterali, commenti |
// | E1001-E1999 | Parsing | Struttura sintattica, violazioni grammaticali |
// | E2001-E2999 | Analisi Semantica | Tipi, scope, dichiarazioni |
// | E3001-E3999 | Generazione IR | CFG, SSA, flusso di controllo |
// | E4001-E4999 | Generazione Codice | Assembly, ABI, registri |
// | E5001-E5999 | I/O & Sistema | Operazioni su file, CLI |

#pragma once

#include "../headers.hpp"

namespace jsv {

// ---------------------------------------------------------------------------
// Severity
// ---------------------------------------------------------------------------

enum class Severity : uint8_t {
    Note = 0,
    Warning = 1,
    Error = 2,
    Fatal = 3,
};
std::string to_string(Severity severity);

// ---------------------------------------------------------------------------
// CompilerPhase
// ---------------------------------------------------------------------------

enum class CompilerPhase : uint8_t {
    Lexer = 0,
    /*Parser = 1,
    Semantic = 2,
    IrGeneration = 3,
    CodeGeneration = 4,
    System = 5,*/
};
std::string to_string(CompilerPhase phase);

// ---------------------------------------------------------------------------
// ErrorCode
// ---------------------------------------------------------------------------

enum class ErrorCode {
    E0001,
    E0002,
    E0003,
    E0004,
    E0005,
    E0006,
    E0007,
    E0008,
    E0009,
    E0010,
    E1001,
    E1002,
    E1003,
    E1004,
    E1005,
    E1006,
    E1007,
    E1008,
    E1009,
    E1010,
    E1011,
    E1012,
    E1013,
    E1014,
    E1015,

    E2001,
    E2002,
    E2003,
    E2004,
    E2005,
    E2006,
    E2007,
    E2008,
    E2009,
    E2010,
    E2011,
    E2012,
    E2013,
    E2014,
    E2015,
    E2016,
    E2017,
    E2018,
    E2019,
    E2020,
    E2021,
    E2022,
    E2023,
    E2024,
    E2025,
    E2026,
    E2027,
    E2028,
    E2029,
    E2030,
    E2031,
    E2032,

    E3001,
    E3002,
    E3003,
    E3004,
    E3005,
    E3006,
    E3007,
    E3008,

    E4001,
    E4002,
    E4003,
    E4004,
    E4005,

    E5001,
    E5002,
    E5003,
    E5004,
    E5005,
};

    // ---------------------------------------------------------------------------
    // ErrorInfo
    // ---------------------------------------------------------------------------

    struct ErrorInfo {
        const char *code;              // Codice di errore (es. "E0001")
        uint16_t numeric_code;         // Codice numerico (es. 1)
        Severity severity;             // Gravità dell'errore
        CompilerPhase phase;           // Fase del compilatore in cui si verifica l'errore
        const char *message;           // Messaggio breve per l'utente
        const char *explanation;       // Spiegazione dettagliata dell'errore
        std::vector<const char *> suggestions;  // Suggerimenti per risolvere l'errore
    };

    [[nodiscard]] const ErrorInfo &get_error_info(ErrorCode error_code);

std::string code(ErrorCode error_code);
uint16_t numeric_code(ErrorCode error_code);
Severity severity(ErrorCode error_code);
CompilerPhase phase(ErrorCode error_code);
std::string message(ErrorCode error_code);
const char *explanation(ErrorCode error_code);
std::vector<const char *> suggestions(ErrorCode error_code);
std::string to_string(ErrorCode error_code);

}

// -------------------------------------------------------------------------
// std::formatter  (C++23 <format>)
// -------------------------------------------------------------------------
namespace std {
    template <> struct formatter<jsv::Severity> : formatter<string> {
        template <typename FormatContext> auto format(jsv::Severity severity, FormatContext &ctx) const {
            return formatter<string>::format(to_string(severity), ctx);
        }
    };

    template <> struct formatter<jsv::CompilerPhase> : formatter<string> {
        template <typename FormatContext> auto format(jsv::CompilerPhase phase, FormatContext &ctx) const {
            return formatter<string>::format(to_string(phase), ctx);
        }
    };

    template <> struct formatter<jsv::ErrorCode> : formatter<string> {
        template <typename FormatContext> auto format(jsv::ErrorCode error_code, FormatContext &ctx) const {
            return formatter<string>::format(to_string(error_code), ctx);
        }
    };
}  // namespace std

// -------------------------------------------------------------------------
// fmt::formatter  (fmtlib)
// -------------------------------------------------------------------------
template <> struct fmt::formatter<jsv::Severity> : fmt::formatter<std::string> {
    template <typename FormatContext> auto format(jsv::Severity severity, FormatContext &ctx) const {
        return fmt::formatter<std::string>::format(to_string(severity), ctx);
    }
};

template <> struct fmt::formatter<jsv::CompilerPhase> : fmt::formatter<std::string> {
    template <typename FormatContext> auto format(jsv::CompilerPhase phase, FormatContext &ctx) const {
        return fmt::formatter<std::string>::format(to_string(phase), ctx);
    }
};

template <> struct fmt::formatter<jsv::ErrorCode> : fmt::formatter<std::string> {
    template <typename FormatContext> auto format(jsv::ErrorCode error_code, FormatContext &ctx) const {
        return fmt::formatter<std::string>::format(to_string(error_code), ctx);
    }
};
