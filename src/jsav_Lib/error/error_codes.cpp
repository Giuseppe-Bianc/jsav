/**
 * @file error_codes.cpp
 * @brief Implementation of the error code system for the jsav compiler
 * @details This module provides a comprehensive and standardized error management
 *          system for the jsav compiler. Each error has a unique identifier (e.g., E0001)
 *          enabling rapid reference, documentation lookup, and IDE integration.
 *
 * @section error_ranges Error Code Ranges
 *
 * | Range     | Phase          | Description                    |
 * |-----------|----------------|--------------------------------|
 * | E0001-E0999 | Lexer        | Token recognition, literals, comments |
 * | E1001-E1999 | Parser       | Syntactic structure, grammar violations |
 * | E2001-E2999 | Semantic     | Types, scope, declarations    |
 * | E3001-E3999 | IR Generation| CFG, SSA, control flow        |
 * | E4001-E4999 | Code Generation| Assembly, ABI, registers    |
 * | E5001-E5999 | System       | File operations, CLI          |
 *
 * @section error_structure Error Structure
 *
 * - ErrorCode: Unique error identifier (e.g., E0001)
 * - ErrorInfo: Complete error metadata structure
 * - Severity: Error severity level (Note, Warning, Error, Fatal)
 * - CompilerPhase: Compiler phase where error occurs
 *
 * @author gbian
 * @copyright Copyright (c) 2026 All rights reserved.
 */

/*
 * Created by gbian on 09/03/2026.
 * Copyright (c) 2026 All rights reserved.
 */
// NOLINTBEGIN(*-include-cleaner,*-avoid-magic-numbers,*-magic-numbers)
#include "jsav/error/error_codes.hpp"

/**
 * @namespace jsv
 * @brief The main namespace for the jsav compiler project
 * @details This namespace contains all compiler components including
 *          lexer, parser, semantic analyzer, IR generator, code generator,
 *          and error handling subsystems.
 */
namespace jsv {
    /**
     * @brief Converts a Severity enum to its string representation
     * @param severity The severity enum value to convert
     * @return The string representation ("nota", "avviso", "errore", or "fatale")
     */
    std::string to_string(Severity severity) {
        switch(severity) {
        case Severity::Note:
            return "nota";
        case Severity::Warning:
            return "avviso";
        case Severity::Error:
            return "errore";
        case Severity::Fatal:
            return "fatale";
        default:
            return "sconosciuto";
        }
    }
    /**
     * @brief Converts a CompilerPhase enum to its string representation
     * @param phase The compiler phase enum value to convert
     * @return The string representation ("lexer")
     */
    std::string to_string(CompilerPhase phase) {
        switch(phase) {
        case CompilerPhase::Lexer:
            return "lexer";
        case CompilerPhase::Parser:
            return "parser";
            /*case CompilerPhase::Semantic:       return "semantico";
            case CompilerPhase::IrGeneration:   return "gen-ir";
            case CompilerPhase::CodeGeneration: return "codegen";
            case CompilerPhase::System:         return "sistema";*/
        default:
            return "sconosciuto";
        }
    }

    /**
     * @brief Returns the error code string for the given error code enum
     * @param error_code The error code enum value
     * @return The error code string (e.g., "E0001")
     * @details This function maps all error code enum values to their
     *          string representation following the format "E" followed by
     *          four digits (E0001 through E5005).
     */
    std::string_view code(ErrorCode error_code) noexcept {
        switch(error_code) {
        case ErrorCode::E0001:
            return "E0001";
        case ErrorCode::E0002:
            return "E0002";
        case ErrorCode::E0003:
            return "E0003";
        case ErrorCode::E0004:
            return "E0004";
        case ErrorCode::E0005:
            return "E0005";
        case ErrorCode::E0006:
            return "E0006";
        case ErrorCode::E0007:
            return "E0007";
        case ErrorCode::E0008:
            return "E0008";
        case ErrorCode::E0009:
            return "E0009";
        case ErrorCode::E0010:
            return "E0010";
        case ErrorCode::E1001:
            return "E1001";
        case ErrorCode::E1002:
            return "E1002";
        case ErrorCode::E1003:
            return "E1003";
        case ErrorCode::E1004:
            return "E1004";
        case ErrorCode::E1005:
            return "E1005";
        case ErrorCode::E1006:
            return "E1006";
        case ErrorCode::E1007:
            return "E1007";
        case ErrorCode::E1008:
            return "E1008";
        case ErrorCode::E1009:
            return "E1009";
        case ErrorCode::E1010:
            return "E1010";
        case ErrorCode::E1011:
            return "E1011";
        case ErrorCode::E1012:
            return "E1012";
        case ErrorCode::E1013:
            return "E1013";
        case ErrorCode::E1014:
            return "E1014";
        case ErrorCode::E1015:
            return "E1015";
        case ErrorCode::E2001:
            return "E2001";
        case ErrorCode::E2002:
            return "E2002";
        case ErrorCode::E2003:
            return "E2003";
        case ErrorCode::E2004:
            return "E2004";
        case ErrorCode::E2005:
            return "E2005";
        case ErrorCode::E2006:
            return "E2006";
        case ErrorCode::E2007:
            return "E2007";
        case ErrorCode::E2008:
            return "E2008";
        case ErrorCode::E2009:
            return "E2009";
        case ErrorCode::E2010:
            return "E2010";
        case ErrorCode::E2011:
            return "E2011";
        case ErrorCode::E2012:
            return "E2012";
        case ErrorCode::E2013:
            return "E2013";
        case ErrorCode::E2014:
            return "E2014";
        case ErrorCode::E2015:
            return "E2015";
        case ErrorCode::E2016:
            return "E2016";
        case ErrorCode::E2017:
            return "E2017";
        case ErrorCode::E2018:
            return "E2018";
        case ErrorCode::E2019:
            return "E2019";
        case ErrorCode::E2020:
            return "E2020";
        case ErrorCode::E2021:
            return "E2021";
        case ErrorCode::E2022:
            return "E2022";
        case ErrorCode::E2023:
            return "E2023";
        case ErrorCode::E2024:
            return "E2024";
        case ErrorCode::E2025:
            return "E2025";
        case ErrorCode::E2026:
            return "E2026";
        case ErrorCode::E2027:
            return "E2027";
        case ErrorCode::E2028:
            return "E2028";
        case ErrorCode::E2029:
            return "E2029";
        case ErrorCode::E2030:
            return "E2030";
        case ErrorCode::E2031:
            return "E2031";
        case ErrorCode::E2032:
            return "E2032";
        case ErrorCode::E3001:
            return "E3001";
        case ErrorCode::E3002:
            return "E3002";
        case ErrorCode::E3003:
            return "E3003";
        case ErrorCode::E3004:
            return "E3004";
        case ErrorCode::E3005:
            return "E3005";
        case ErrorCode::E3006:
            return "E3006";
        case ErrorCode::E3007:
            return "E3007";
        case ErrorCode::E3008:
            return "E3008";
        case ErrorCode::E4001:
            return "E4001";
        case ErrorCode::E4002:
            return "E4002";
        case ErrorCode::E4003:
            return "E4003";
        case ErrorCode::E4004:
            return "E4004";
        case ErrorCode::E4005:
            return "E4005";
        case ErrorCode::E5001:
            return "E5001";
        case ErrorCode::E5002:
            return "E5002";
        case ErrorCode::E5003:
            return "E5003";
        case ErrorCode::E5004:
            return "E5004";
        case ErrorCode::E5005:
            return "E5005";
        default:
            return "SCONOSCIUTO";
        }
    }

    /**
     * @brief Returns the numeric part of the error code
     * @param error_code The error code enum value
     * @return The numeric code (e.g., 1 for E0001, 1001 for E1001)
     * @details Extracts the numeric portion by removing the "E" prefix.
     *          For E0001-E0010 returns 1-10, for E1001-E1015 returns 1001-1015,
     *          and so on for all error code ranges.
     */
    uint16_t numeric_code(ErrorCode error_code) noexcept {
        switch(error_code) {
        case ErrorCode::E0001:
            return 1;
        case ErrorCode::E0002:
            return 2;
        case ErrorCode::E0003:
            return 3;
        case ErrorCode::E0004:
            return 4;
        case ErrorCode::E0005:
            return 5;
        case ErrorCode::E0006:
            return 6;
        case ErrorCode::E0007:
            return 7;
        case ErrorCode::E0008:
            return 8;
        case ErrorCode::E0009:
            return 9;
        case ErrorCode::E0010:
            return 10;
        case ErrorCode::E1001:
            return 1001;
        case ErrorCode::E1002:
            return 1002;
        case ErrorCode::E1003:
            return 1003;
        case ErrorCode::E1004:
            return 1004;
        case ErrorCode::E1005:
            return 1005;
        case ErrorCode::E1006:
            return 1006;
        case ErrorCode::E1007:
            return 1007;
        case ErrorCode::E1008:
            return 1008;
        case ErrorCode::E1009:
            return 1009;
        case ErrorCode::E1010:
            return 1010;
        case ErrorCode::E1011:
            return 1011;
        case ErrorCode::E1012:
            return 1012;
        case ErrorCode::E1013:
            return 1013;
        case ErrorCode::E1014:
            return 1014;
        case ErrorCode::E1015:
            return 1015;
        case ErrorCode::E2001:
            return 2001;
        case ErrorCode::E2002:
            return 2002;
        case ErrorCode::E2003:
            return 2003;
        case ErrorCode::E2004:
            return 2004;
        case ErrorCode::E2005:
            return 2005;
        case ErrorCode::E2006:
            return 2006;
        case ErrorCode::E2007:
            return 2007;
        case ErrorCode::E2008:
            return 2008;
        case ErrorCode::E2009:
            return 2009;
        case ErrorCode::E2010:
            return 2010;
        case ErrorCode::E2011:
            return 2011;
        case ErrorCode::E2012:
            return 2012;
        case ErrorCode::E2013:
            return 2013;
        case ErrorCode::E2014:
            return 2014;
        case ErrorCode::E2015:
            return 2015;
        case ErrorCode::E2016:
            return 2016;
        case ErrorCode::E2017:
            return 2017;
        case ErrorCode::E2018:
            return 2018;
        case ErrorCode::E2019:
            return 2019;
        case ErrorCode::E2020:
            return 2020;
        case ErrorCode::E2021:
            return 2021;
        case ErrorCode::E2022:
            return 2022;
        case ErrorCode::E2023:
            return 2023;
        case ErrorCode::E2024:
            return 2024;
        case ErrorCode::E2025:
            return 2025;
        case ErrorCode::E2026:
            return 2026;
        case ErrorCode::E2027:
            return 2027;
        case ErrorCode::E2028:
            return 2028;
        case ErrorCode::E2029:
            return 2029;
        case ErrorCode::E2030:
            return 2030;
        case ErrorCode::E2031:
            return 2031;
        case ErrorCode::E2032:
            return 2032;
        case ErrorCode::E3001:
            return 3001;
        case ErrorCode::E3002:
            return 3002;
        case ErrorCode::E3003:
            return 3003;
        case ErrorCode::E3004:
            return 3004;
        case ErrorCode::E3005:
            return 3005;
        case ErrorCode::E3006:
            return 3006;
        case ErrorCode::E3007:
            return 3007;
        case ErrorCode::E3008:
            return 3008;
        case ErrorCode::E4001:
            return 4001;
        case ErrorCode::E4002:
            return 4002;
        case ErrorCode::E4003:
            return 4003;
        case ErrorCode::E4004:
            return 4004;
        case ErrorCode::E4005:
            return 4005;
        case ErrorCode::E5001:
            return 5001;
        case ErrorCode::E5002:
            return 5002;
        case ErrorCode::E5003:
            return 5003;
        case ErrorCode::E5004:
            return 5004;
        case ErrorCode::E5005:
            return 5005;
        default:
            return 0;
        }
    }
    /**
     * @brief Returns the severity level for the given error code
     * @param error_code The error code enum value
     * @return The severity (Warning for E1013, Error for most errors)
     * @note E1013 (missing semicolon) is treated as a warning
     * @details Currently only E1013 returns Warning severity; all other
     *          error codes return Error severity.
     */
    Severity severity(ErrorCode error_code) {
        switch(error_code) {
        case ErrorCode::E1013:
            return Severity::Warning;
        default:
            return Severity::Error;
        }
    }

    /**
     * @brief Returns the compiler phase for the given error code
     * @param error_code The error code enum value
     * @return The compiler phase (Lexer, Parser, Semantic, etc.)
     * @details Determines the phase by parsing the numeric code range:
     *          - 1-999: Lexer
     *          - 1001-1999: Parser
     *          - 2001-2999: Semantic
     *          - 3001-3999: IR Generation
     *          - 4001-4999: Code Generation
     *          - 5001-5999: System
     */
    CompilerPhase phase(ErrorCode error_code) {
        const uint16_t num = numeric_code(error_code);
        if(num >= 1 && num <= 999) { return CompilerPhase::Lexer; }
        if (num >= 1001 && num <= 1999) { return CompilerPhase::Parser; }
        /*if (num >= 2001 && num <= 2999) return CompilerPhase::Semantic;
        if (num >= 3001 && num <= 3999) return CompilerPhase::IrGeneration;
        if (num >= 4001 && num <= 4999) return CompilerPhase::CodeGeneration;*/
        return CompilerPhase::Lexer;  // fallback (System non ancora abilitato)
    }

    /**
     * @brief Returns a brief error message for the given error code
     * @param error_code The error code to get the message for
     * @return A string containing the error message
     * @note Messages are in Italian for end-user display
     */
    std::string_view message(ErrorCode error_code) noexcept {
        switch(error_code) {
        case ErrorCode::E0001:
            return "token non valido o non riconosciuto";
        case ErrorCode::E0002:
            return "letterale numerico binario malformato";
        case ErrorCode::E0003:
            return "letterale numerico ottale malformato";
        case ErrorCode::E0004:
            return "letterale numerico esadecimale malformato";
        case ErrorCode::E0005:
            return "letterale stringa non terminato";
        case ErrorCode::E0006:
            return "letterale carattere non terminato";
        case ErrorCode::E0007:
            return "sequenza di escape non valida";
        case ErrorCode::E0008:
            return "commento multi-linea non terminato";
        case ErrorCode::E0009:
            return "suffisso numerico non valido";
        case ErrorCode::E0010:
            return "overflow letterale numerico";
        case ErrorCode::E1001:
            return "profondità massima di ricorsione superata";
        case ErrorCode::E1002:
            return "specifica di tipo non valida";
        case ErrorCode::E1003:
            return "target di assegnazione non valido";
        case ErrorCode::E1004:
            return "token inaspettato";
        case ErrorCode::E1005:
            return "operatore binario non valido";
        case ErrorCode::E1006:
            return "espressione attesa";
        case ErrorCode::E1007:
            return "statement atteso";
        case ErrorCode::E1008:
            return "identificatore atteso";
        case ErrorCode::E1009:
            return "annotazione di tipo attesa";
        case ErrorCode::E1010:
            return "parentesi tonda non corrispondente";
        case ErrorCode::E1011:
            return "parentesi graffa non corrispondente";
        case ErrorCode::E1012:
            return "parentesi quadra non corrispondente";
        case ErrorCode::E1013:
            return "punto e virgola mancante";
        case ErrorCode::E1014:
            return "firma di funzione non valida";
        case ErrorCode::E1015:
            return "lista di parametri non valida";
        case ErrorCode::E2001:
            return "numero di inizializzatori non corrispondente";
        case ErrorCode::E2002:
            return "tipo non corrispondente nell'assegnazione";
        case ErrorCode::E2003:
            return "return mancante in alcuni percorsi del codice";
        case ErrorCode::E2004:
            return "la condizione deve essere booleana";
        case ErrorCode::E2005:
            return "return fuori dalla funzione";
        case ErrorCode::E2006:
            return "impossibile restituire valore da funzione void";
        case ErrorCode::E2007:
            return "tipo di return non corrispondente";
        case ErrorCode::E2008:
            return "valore di return mancante";
        case ErrorCode::E2009:
            return "break fuori dal ciclo";
        case ErrorCode::E2010:
            return "continue fuori dal ciclo";
        case ErrorCode::E2011:
            return "operatore bitwise richiede operandi interi";
        case ErrorCode::E2012:
            return "operatore logico richiede operandi booleani";
        case ErrorCode::E2013:
            return "operatore aritmetico richiede operandi numerici";
        case ErrorCode::E2014:
            return "tipi incompatibili nel confronto";
        case ErrorCode::E2015:
            return "tipo non corrispondente in operazione binaria";
        case ErrorCode::E2016:
            return "operazione aritmetica non supportata";
        case ErrorCode::E2017:
            return "operazione logica richiede booleano";
        case ErrorCode::E2018:
            return "negazione richiede tipo numerico";
        case ErrorCode::E2019:
            return "NOT logico richiede tipo booleano";
        case ErrorCode::E2020:
            return "letterale array vuoto";
        case ErrorCode::E2021:
            return "tipi misti in letterale array";
        case ErrorCode::E2022:
            return "funzione non può essere usata come variabile";
        case ErrorCode::E2023:
            return "variabile non definita";
        case ErrorCode::E2024:
            return "impossibile assegnare a variabile immutabile";
        case ErrorCode::E2025:
            return "variabile non definita nell'assegnazione";
        case ErrorCode::E2026:
            return "il chiamato deve essere una funzione";
        case ErrorCode::E2027:
            return "funzione non definita";
        case ErrorCode::E2028:
            return "numero errato di argomenti";
        case ErrorCode::E2029:
            return "tipo di argomento non corrispondente";
        case ErrorCode::E2030:
            return "l'indice dell'array deve essere intero";
        case ErrorCode::E2031:
            return "impossibile indicizzare tipo non-array";
        case ErrorCode::E2032:
            return "dichiarazione duplicata";
        case ErrorCode::E3001:
            return "break fuori dal ciclo in IR";
        case ErrorCode::E3002:
            return "continue fuori dal ciclo in IR";
        case ErrorCode::E3003:
            return "istruzione IR non valida";
        case ErrorCode::E3004:
            return "variabile non definita in IR";
        case ErrorCode::E3005:
            return "blocco base non valido";
        case ErrorCode::E3006:
            return "terminatore di blocco non valido";
        case ErrorCode::E3007:
            return "errore di trasformazione SSA";
        case ErrorCode::E3008:
            return "errore di costruzione CFG";
        case ErrorCode::E4001:
            return "istruzione assembly non valida";
        case ErrorCode::E4002:
            return "allocazione registro fallita";
        case ErrorCode::E4003:
            return "overflow stack frame";
        case ErrorCode::E4004:
            return "piattaforma target non supportata";
        case ErrorCode::E4005:
            return "violazione ABI";
        case ErrorCode::E5001:
            return "file non trovato";
        case ErrorCode::E5002:
            return "permesso negato";
        case ErrorCode::E5003:
            return "estensione file non valida";
        case ErrorCode::E5004:
            return "errore di scrittura";
        case ErrorCode::E5005:
            return "errore di lettura";
        default:
            return "errore sconosciuto";
        }
    }

    /**
     * @brief Returns a detailed explanation for the given error code
     * @param error_code The error code to get the explanation for
     * @return A C-string containing the detailed explanation
     * @note Explanations are in Italian and provide context-aware guidance
     * @details This function returns verbose explanations that help users
     *          understand why an error occurred and how to fix it.
     *          Each explanation covers the specific error pattern and provides
     *          concrete examples where applicable.
     */
    const char *explanation(ErrorCode error_code) {
        switch(error_code) {
        case ErrorCode::E0001:
            return "Il lexer ha incontrato una sequenza di caratteri che non corrisponde a nessun pattern di token valido.\n"
                   "Verificare la presenza di errori di battitura o caratteri non supportati. I token validi includono identificatori,\n"
                   "parole chiave, operatori e letterali.";
        case ErrorCode::E0002:
            return "I letterali binari devono avere almeno una cifra binaria (0 o 1) dopo `#b`.\n"
                   "Esempio: `#b1010` per decimale 10.";
        case ErrorCode::E0003:
            return "I letterali ottali devono avere almeno una cifra ottale (0-7) dopo `#o`.\n"
                   "Esempio: `#o755` per decimale 493.";
        case ErrorCode::E0004:
            return "I letterali esadecimali devono avere almeno una cifra esadecimale (0-9, a-f, A-F) dopo `#x`.\n"
                   "Esempio: `#xDEAD` per decimale 57005.";
        case ErrorCode::E0005:
            return "I letterali stringa devono essere chiusi con virgolette doppie corrispondenti prima della fine della riga.\n"
                   "Esempio: `\"ciao mondo\"` invece di `\"ciao mondo`.";
        case ErrorCode::E0006:
            return "I letterali carattere devono essere chiusi con apici singoli corrispondenti.\n"
                   "Esempio: `'x'` invece di `'x`.";
        case ErrorCode::E0007:
            return "La sequenza di escape non è riconosciuta. Le sequenze di escape valide includono:\n"
                   "\\n (nuova riga), \\r (ritorno carrello), \\t (tab), \\\\ (backslash),\n"
                   "\\' (apice singolo), \\\" (virgolette doppie), \\0 (null), \\u{XXXX} (unicode).";
        case ErrorCode::E0008:
            return "I commenti multi-linea aperti con `/*` devono essere chiusi con `*/`.\n"
                   "Verificare la presenza di marcatori di chiusura mancanti o annidamenti accidentali.";
        case ErrorCode::E0009:
            return "Il suffisso sul letterale numerico non è riconosciuto.\n"
                   "Suffissi validi: i8, i16, i32, i64, u8, u16, u32, u64, f32, f64.";
        case ErrorCode::E0010:
            return "Il valore numerico supera l'intervallo del tipo target.\n"
                   "Usare un tipo più grande o ridurre il valore.";
        case ErrorCode::E1001:
            return "Il parser ha superato il suo limite di ricorsione a causa di espressioni profondamente annidate.\n"
                   "Semplificare l'espressione o suddividerla in parti più piccole.";
        case ErrorCode::E1002:
            return "Atteso un tipo valido ma trovato qualcos'altro.\n"
                   "Tipi validi: i8, i16, i32, i64, u8, u16, u32, u64, f32, f64, char, string, bool,\n"
                   "o identificatori di tipo personalizzati.";
        case ErrorCode::E1003:
            return "Solo variabili ed elementi di array possono essere assegnati.\n"
                   "Esempi: `x = 5` o `arr[0] = 1`.";
        case ErrorCode::E2023:
            return "La variabile non è stata dichiarata nello scope corrente o in alcuno scope esterno.\n"
                   "Dichiarare la variabile con `var` o `const` prima di usarla.";
        case ErrorCode::E2024:
            return "Le variabili dichiarate con `const` non possono essere riassegnate.\n"
                   "Usare `var` per variabili mutabili o rimuovere la riassegnazione.";
        case ErrorCode::E2027:
            return "La funzione non è stata dichiarata.\n"
                   "Definire la funzione con `fun` prima di chiamarla.";
        case ErrorCode::E2028:
            return "Il numero di argomenti forniti non corrisponde al numero di parametri della funzione.\n"
                   "Verificare la definizione della funzione e fornire il numero corretto di argomenti.";
        default:
            return "Vedere il messaggio di errore per i dettagli.";
        }
    }

    /**
     * @brief Returns a list of actionable suggestions to resolve the given error
     * @param error_code The error code to get suggestions for
     * @return A vector of C-string suggestions that users can follow
     * @note Suggestions are in Italian and provide specific code examples
     * @details This function returns practical fixes for common errors.
     *          Each suggestion is a concrete action the user can take to resolve
     *          the issue. For errors without specific suggestions, an empty vector
     *          is returned.
     */
    std::span<const char *const> suggestions(ErrorCode error_code) noexcept {
        static constexpr std::array<const char *, 2> kE0002 = {"Aggiungere cifre binarie dopo #b: #b1010",
                                                               "Verificare cifre non valide (solo 0 e 1 ammessi)"};
        static constexpr std::array<const char *, 2> kE0003 = {"Aggiungere cifre ottali dopo #o: #o755",
                                                               "Verificare cifre non valide (solo 0-7 ammessi)"};
        static constexpr std::array<const char *, 2> kE0004 = {"Aggiungere cifre esadecimali dopo #x: #xDEAD",
                                                               "Verificare cifre non valide (solo 0-9, a-f, A-F ammessi)"};
        static constexpr std::array<const char *, 2> kE0005 = {R"(Aggiungere virgolette doppie di chiusura: "ciao")",
                                                               R"(Usare sequenza di escape per virgolette incorporate: "dire \"ciao\"")"};
        static constexpr std::array<const char *, 2> kE2009_10 = {"Spostare lo statement all'interno di un ciclo while o for",
                                                                  "Usare return per uscire da una funzione invece"};
        static constexpr std::array<const char *, 3> kE2023 = {"Dichiarare la variabile: var x: i32 = 0",
                                                               "Verificare errori di battitura nel nome della variabile",
                                                               "Assicurarsi che la variabile sia nello scope"};
        static constexpr std::array<const char *, 2> kE2024 = {"Usare 'var' invece di 'const' per variabili mutabili",
                                                               "Rimuovere la riassegnazione"};

        switch(error_code) {
        case ErrorCode::E0002:
            return kE0002;
        case ErrorCode::E0003:
            return kE0003;
        case ErrorCode::E0004:
            return kE0004;
        case ErrorCode::E0005:
            return kE0005;
        case ErrorCode::E2009:
        case ErrorCode::E2010:
            return kE2009_10;
        case ErrorCode::E2023:
            return kE2023;
        case ErrorCode::E2024:
            return kE2024;
        default:
            return {};  // default-constructed span: data=null, size=0
        }
    }

    /**
     * @brief Converts an error code to a formatted diagnostic string
     * @param error_code The error code enum value
     * @return A string in the format "CODE: message" (e.g., "E0001: token non valido o non riconosciuto")
     * @details This function combines the error code string with its brief message
     *          for use in diagnostic output.
     */
    std::string to_string(ErrorCode error_code) { return FORMAT("{}: {}", code(error_code), message(error_code)); }
}  // namespace jsv

// NOLINTEND(*-include-cleaner,*-avoid-magic-numbers,*-magic-numbers)
