include(GenerateExportHeader)

add_library(jsav_lib jsav.cpp
        lexer/SourceLocation.cpp
        ../../include/jsav/lexer/SourceLocation.hpp
        lexer/SourceSpan.cpp
        ../../include/jsav/lexer/SourceSpan.hpp
        #[[        lexer/Token.cpp
                lexer/Tokenizer.cpp
                lexer/TokenizerUtility.cpp
                parser/Parser.cpp
                parser/ASTNode.cpp
                parser/BinaryExpressionNode.cpp
                parser/UnaryExpressionNode.cpp
                parser/NumberNode.cpp
                parser/VariableNode.cpp
                parser/TypeNode.cpp
                parser/IndexNode.cpp
                parser/ArrayNode.cpp
                parser/NullptrNode.cpp
                parser/Statement.cpp
                transpiler/Transpiler.cpp
                transpiler/ProjectBuilder.cpp
                parser/AST.cpp
                lexer/ErrorHandler.cpp]]
)

add_library(jsav::jsav_lib ALIAS jsav_lib)

# Retrieve the target name
get_target_property(target_name jsav_lib NAME)
include("${CMAKE_SOURCE_DIR}/cmake/Simd.cmake")
set_simd_instructions(${target_name})


target_link_libraries(jsav_lib
        PRIVATE
        jsav_options
        jsav_warnings
        PUBLIC
        jsav::jsav_core_lib
        #nlohmann_json::nlohmann_json
)

target_include_directories(jsav_lib
        PUBLIC
        $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
        $<BUILD_INTERFACE:${PROJECT_BINARY_DIR}/include>
        $<BUILD_INTERFACE:${CMAKE_BINARY_DIR}/configured_files/include>
)
if (jsav_BUILD_FUZZ_TESTS)
    target_link_libraries(jsav_lib PRIVATE -fsanitize=fuzzer-no-link)
    target_compile_options(jsav_lib PRIVATE -fsanitize=fuzzer-no-link)
endif ()

target_compile_features(jsav_lib PUBLIC cxx_std_${CMAKE_CXX_STANDARD})


set_target_properties(
        jsav_lib
        PROPERTIES VERSION ${PROJECT_VERSION}
        CXX_VISIBILITY_PRESET hidden
        VISIBILITY_INLINES_HIDDEN YES)

generate_export_header(jsav_lib EXPORT_FILE_NAME ${PROJECT_BINARY_DIR}/include/jsav/jsav_export.hpp)

if (NOT BUILD_SHARED_LIBS)
    target_compile_definitions(jsav_lib PUBLIC JSAV_LIB_STATIC_DEFINE)
endif ()
