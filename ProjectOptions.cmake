include(cmake/SystemLink.cmake)
include(cmake/LibFuzzer.cmake)
include(CMakeDependentOption)
include(CheckCXXCompilerFlag)


include(CheckCXXSourceCompiles)


macro(jsav_supports_sanitizers)
  # Emscripten doesn't support sanitizers
  if(EMSCRIPTEN)
    set(SUPPORTS_UBSAN OFF)
    set(SUPPORTS_ASAN OFF)
  elseif((CMAKE_CXX_COMPILER_ID MATCHES ".*Clang.*" OR CMAKE_CXX_COMPILER_ID MATCHES ".*GNU.*") AND NOT WIN32)

    message(STATUS "Sanity checking UndefinedBehaviorSanitizer, it should be supported on this platform")
    set(TEST_PROGRAM "int main() { return 0; }")

    # Check if UndefinedBehaviorSanitizer works at link time
    set(CMAKE_REQUIRED_FLAGS "-fsanitize=undefined")
    set(CMAKE_REQUIRED_LINK_OPTIONS "-fsanitize=undefined")
    check_cxx_source_compiles("${TEST_PROGRAM}" HAS_UBSAN_LINK_SUPPORT)

    if(HAS_UBSAN_LINK_SUPPORT)
      message(STATUS "UndefinedBehaviorSanitizer is supported at both compile and link time.")
      set(SUPPORTS_UBSAN ON)
    else()
      message(WARNING "UndefinedBehaviorSanitizer is NOT supported at link time.")
      set(SUPPORTS_UBSAN OFF)
    endif()
  else()
    set(SUPPORTS_UBSAN OFF)
  endif()

  if((CMAKE_CXX_COMPILER_ID MATCHES ".*Clang.*" OR CMAKE_CXX_COMPILER_ID MATCHES ".*GNU.*") AND WIN32)
    set(SUPPORTS_ASAN OFF)
  else()
    if (NOT WIN32)
      message(STATUS "Sanity checking AddressSanitizer, it should be supported on this platform")
      set(TEST_PROGRAM "int main() { return 0; }")

      # Check if AddressSanitizer works at link time
      set(CMAKE_REQUIRED_FLAGS "-fsanitize=address")
      set(CMAKE_REQUIRED_LINK_OPTIONS "-fsanitize=address")
      check_cxx_source_compiles("${TEST_PROGRAM}" HAS_ASAN_LINK_SUPPORT)

      if(HAS_ASAN_LINK_SUPPORT)
        message(STATUS "AddressSanitizer is supported at both compile and link time.")
        set(SUPPORTS_ASAN ON)
      else()
        message(WARNING "AddressSanitizer is NOT supported at link time.")
        set(SUPPORTS_ASAN OFF)
      endif()
    else()
      set(SUPPORTS_ASAN ON)
    endif()
  endif()
endmacro()

macro(jsav_setup_options)
  option(jsav_ENABLE_HARDENING "Enable hardening" ON)
  option(jsav_ENABLE_COVERAGE "Enable coverage reporting" OFF)
  cmake_dependent_option(
    jsav_ENABLE_GLOBAL_HARDENING
    "Attempt to push hardening options to built dependencies"
    ON
    jsav_ENABLE_HARDENING
    OFF)

  jsav_supports_sanitizers()

  if(NOT PROJECT_IS_TOP_LEVEL OR jsav_PACKAGING_MAINTAINER_MODE)
    option(jsav_ENABLE_IPO "Enable IPO/LTO" OFF)
    option(jsav_WARNINGS_AS_ERRORS "Treat Warnings As Errors" OFF)
    option(jsav_ENABLE_SANITIZER_ADDRESS "Enable address sanitizer" OFF)
    option(jsav_ENABLE_SANITIZER_LEAK "Enable leak sanitizer" OFF)
    option(jsav_ENABLE_SANITIZER_UNDEFINED "Enable undefined sanitizer" OFF)
    option(jsav_ENABLE_SANITIZER_THREAD "Enable thread sanitizer" OFF)
    option(jsav_ENABLE_SANITIZER_MEMORY "Enable memory sanitizer" OFF)
    option(jsav_ENABLE_UNITY_BUILD "Enable unity builds" OFF)
    option(jsav_ENABLE_CLANG_TIDY "Enable clang-tidy" OFF)
    option(jsav_ENABLE_CPPCHECK "Enable cpp-check analysis" OFF)
    option(jsav_ENABLE_PCH "Enable precompiled headers" OFF)
    option(jsav_ENABLE_CACHE "Enable ccache" OFF)
  else()
    option(jsav_ENABLE_IPO "Enable IPO/LTO" ON)
    option(jsav_WARNINGS_AS_ERRORS "Treat Warnings As Errors" ON)
    option(jsav_ENABLE_SANITIZER_ADDRESS "Enable address sanitizer" ${SUPPORTS_ASAN})
    option(jsav_ENABLE_SANITIZER_LEAK "Enable leak sanitizer" OFF)
    option(jsav_ENABLE_SANITIZER_UNDEFINED "Enable undefined sanitizer" ${SUPPORTS_UBSAN})
    option(jsav_ENABLE_SANITIZER_THREAD "Enable thread sanitizer" OFF)
    option(jsav_ENABLE_SANITIZER_MEMORY "Enable memory sanitizer" OFF)
    option(jsav_ENABLE_UNITY_BUILD "Enable unity builds" OFF)
    option(jsav_ENABLE_CLANG_TIDY "Enable clang-tidy" ON)
    option(jsav_ENABLE_CPPCHECK "Enable cpp-check analysis" ON)
    option(jsav_ENABLE_PCH "Enable precompiled headers" OFF)
    option(jsav_ENABLE_CACHE "Enable ccache" ON)
  endif()

  if(NOT PROJECT_IS_TOP_LEVEL)
    mark_as_advanced(
      jsav_ENABLE_IPO
      jsav_WARNINGS_AS_ERRORS
      jsav_ENABLE_SANITIZER_ADDRESS
      jsav_ENABLE_SANITIZER_LEAK
      jsav_ENABLE_SANITIZER_UNDEFINED
      jsav_ENABLE_SANITIZER_THREAD
      jsav_ENABLE_SANITIZER_MEMORY
      jsav_ENABLE_UNITY_BUILD
      jsav_ENABLE_CLANG_TIDY
      jsav_ENABLE_CPPCHECK
      jsav_ENABLE_COVERAGE
      jsav_ENABLE_PCH
      jsav_ENABLE_CACHE)
  endif()

  jsav_check_libfuzzer_support(LIBFUZZER_SUPPORTED)
  if(LIBFUZZER_SUPPORTED AND (jsav_ENABLE_SANITIZER_ADDRESS OR jsav_ENABLE_SANITIZER_THREAD OR jsav_ENABLE_SANITIZER_UNDEFINED))
    set(DEFAULT_FUZZER ON)
  else()
    set(DEFAULT_FUZZER OFF)
  endif()

  option(jsav_BUILD_FUZZ_TESTS "Enable fuzz testing executable" ${DEFAULT_FUZZER})

endmacro()

macro(jsav_global_options)
  include(cmake/Simd.cmake)
  check_all_simd_features()
  print_simd_support()
  if(jsav_ENABLE_IPO)
    include(cmake/InterproceduralOptimization.cmake)
    jsav_enable_ipo()
  endif()

  jsav_supports_sanitizers()

  if(jsav_ENABLE_HARDENING AND jsav_ENABLE_GLOBAL_HARDENING)
    include(cmake/Hardening.cmake)
    if(NOT SUPPORTS_UBSAN 
       OR jsav_ENABLE_SANITIZER_UNDEFINED
       OR jsav_ENABLE_SANITIZER_ADDRESS
       OR jsav_ENABLE_SANITIZER_THREAD
       OR jsav_ENABLE_SANITIZER_LEAK)
      set(ENABLE_UBSAN_MINIMAL_RUNTIME FALSE)
    else()
      set(ENABLE_UBSAN_MINIMAL_RUNTIME TRUE)
    endif()
    message("${jsav_ENABLE_HARDENING} ${ENABLE_UBSAN_MINIMAL_RUNTIME} ${jsav_ENABLE_SANITIZER_UNDEFINED}")
    jsav_enable_hardening(jsav_options ON ${ENABLE_UBSAN_MINIMAL_RUNTIME})
  endif()
endmacro()

macro(jsav_local_options)
  if(PROJECT_IS_TOP_LEVEL)
    include(cmake/StandardProjectSettings.cmake)
  endif()

  add_library(jsav_warnings INTERFACE)
  add_library(jsav_options INTERFACE)

  include(cmake/CompilerWarnings.cmake)
  jsav_set_project_warnings(
    jsav_warnings
    ${jsav_WARNINGS_AS_ERRORS}
    ""
    ""
    ""
    "")

  include(cmake/Linker.cmake)
  # Must configure each target with linker options, we're avoiding setting it globally for now

  if(NOT EMSCRIPTEN)
    include(cmake/Sanitizers.cmake)
    jsav_enable_sanitizers(
      jsav_options
      ${jsav_ENABLE_SANITIZER_ADDRESS}
      ${jsav_ENABLE_SANITIZER_LEAK}
      ${jsav_ENABLE_SANITIZER_UNDEFINED}
      ${jsav_ENABLE_SANITIZER_THREAD}
      ${jsav_ENABLE_SANITIZER_MEMORY})
  endif()

  set_target_properties(jsav_options PROPERTIES UNITY_BUILD ${jsav_ENABLE_UNITY_BUILD})

  if(jsav_ENABLE_PCH)
    target_precompile_headers(
      jsav_options
      INTERFACE
      <vector>
      <string>
      <utility>)
  endif()

  if(jsav_ENABLE_CACHE)
    include(cmake/Cache.cmake)
    jsav_enable_cache()
  endif()

  include(cmake/StaticAnalyzers.cmake)
  if(jsav_ENABLE_CLANG_TIDY)
    jsav_enable_clang_tidy(jsav_options ${jsav_WARNINGS_AS_ERRORS})
  endif()

  if(jsav_ENABLE_CPPCHECK)
    jsav_enable_cppcheck(${jsav_WARNINGS_AS_ERRORS} "" # override cppcheck options
    )
  endif()

  if(jsav_ENABLE_COVERAGE)
    include(cmake/Tests.cmake)
    jsav_enable_coverage(jsav_options)
  endif()

  if(jsav_WARNINGS_AS_ERRORS)
    check_cxx_compiler_flag("-Wl,--fatal-warnings" LINKER_FATAL_WARNINGS)
    if(LINKER_FATAL_WARNINGS)
      # This is not working consistently, so disabling for now
      # target_link_options(jsav_options INTERFACE -Wl,--fatal-warnings)
    endif()
  endif()

  if(jsav_ENABLE_HARDENING AND NOT jsav_ENABLE_GLOBAL_HARDENING)
    include(cmake/Hardening.cmake)
    if(NOT SUPPORTS_UBSAN 
       OR jsav_ENABLE_SANITIZER_UNDEFINED
       OR jsav_ENABLE_SANITIZER_ADDRESS
       OR jsav_ENABLE_SANITIZER_THREAD
       OR jsav_ENABLE_SANITIZER_LEAK)
      set(ENABLE_UBSAN_MINIMAL_RUNTIME FALSE)
    else()
      set(ENABLE_UBSAN_MINIMAL_RUNTIME TRUE)
    endif()
    jsav_enable_hardening(jsav_options OFF ${ENABLE_UBSAN_MINIMAL_RUNTIME})
  endif()

endmacro()
