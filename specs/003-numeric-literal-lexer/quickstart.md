# Quickstart: Numeric Literal Lexer Feature

**Feature**: 003-numeric-literal-lexer
**Branch**: `003-numeric-literal-lexer`

## Prerequisiti

- MSVC 2026 (Windows) oppure GCC/Clang con supporto C++23
- CMake 4.2+, Ninja
- clang-format installato

## Setup ambiente

```powershell
# Clonare e posizionarsi sul branch
git checkout 003-numeric-literal-lexer

# Configurare build (dalla root del progetto)
cmake -S . -B ./build -Djsav_ENABLE_CLANG_TIDY:BOOL=OFF -Djsav_ENABLE_IPO:BOOL=OFF -Djsav_ENABLE_CPPCHECK:BOOL=OFF -DFMT_PEDANTIC:BOOL=ON -Djsav_ENABLE_SANITIZER_ADDRESS:BOOL=OFF
```

## File coinvolti

| File | Tipo modifica |
|------|--------------|
| `src/jsav_Lib/lexer/Lexer.cpp` | Rewrite `scan_numeric_literal()` + modifica `next_token()` |
| `include/jsav/lexer/Lexer.hpp` | Aggiunta dichiarazioni helper privati |
| `test/tests.cpp` | Nuovi TEST_CASE per tutti gli scenari |
| `test/constexpr_tests.cpp` | Eventuali test STATIC_REQUIRE per helper constexpr |

## Workflow di sviluppo (TDD)

### 1. Scrivere test failing

Aggiungere i test in `test/tests.cpp` seguendo la struttura:

```cpp
TEST_CASE("Lexer_NumericBaseFormats_TokenizeCorrectly", "[lexer][numeric][phase7]") {
    SECTION("integer 42 produces single Numeric token") {
        jsv::Lexer lex{"42", "test.jsav"};
        const auto tokens = lex.tokenize();
        REQUIRE(tokens.size() == 2);  // Numeric + Eof
        REQUIRE(tokens[0].getKind() == jsv::TokenKind::Numeric);
        REQUIRE(tokens[0].getText() == "42");
    }
}
```

### 2. Build e verifica test red

```powershell
cd build
ninja tests relaxed_constexpr_tests
ctest -R "unittests|relaxed_constexpr" --output-on-failure
```

### 3. Implementare il codice minimo

Modificare `Lexer.cpp` per far passare i test.

### 4. Formattare

```powershell
clang-format -i src/jsav_Lib/lexer/Lexer.cpp include/jsav/lexer/Lexer.hpp test/tests.cpp test/constexpr_tests.cpp
```

### 5. Verificare regressione

```powershell
cd build
ninja tests relaxed_constexpr_tests
ctest -R "unittests|relaxed_constexpr" --output-on-failure
```

### 6. Verificare complessità

```powershell
cmake --build build --target lizard
```

## Priorità task

| Priorità | Task | Descrizione |
|----------|------|-------------|
| P0 | Task 1 | Modifica `next_token()` per leading-dot |
| P0 | Task 2 | Rewrite `scan_numeric_literal()` |
| P0 | Task 3 | Dichiarazioni helper in header |
| P1 | Task 4 | Test runtime (5 user stories) |
| P1 | Task 5 | Test constexpr |
| P2 | Task 6 | Suite di regressione completa |

## Verifica rapida

Dopo l'implementazione completa, un quick smoke test:

```cpp
jsv::Lexer lex{"3.14e+2f .5 1e 42u8 1i", "test.jsav"};
auto tokens = lex.tokenize();
// Expected: Numeric("3.14e+2f"), Numeric(".5"), Numeric("1"), Identifier("e"),
//           Numeric("42u8"), Numeric("1"), Identifier("i"), Eof
```
