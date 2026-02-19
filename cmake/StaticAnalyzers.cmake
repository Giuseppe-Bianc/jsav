macro(jsav_enable_cppcheck WARNINGS_AS_ERRORS CPPCHECK_OPTIONS)
  find_program(CPPCHECK cppcheck)
  if(CPPCHECK)

        if(CMAKE_GENERATOR MATCHES ".*Visual Studio.*")
            set(CPPCHECK_TEMPLATE "vs")
            message(STATUS "Using Visual Studio output template")
        else()
            set(CPPCHECK_TEMPLATE "gcc")
            message(STATUS "Using GCC-style output template")
        endif()

        if("${CPPCHECK_OPTIONS}" STREQUAL "")
            # Enable all warnings that are actionable by the user of this toolset
            # style should enable the other 3, but we'll be explicit just in case
            set(SUPPRESS_DIR "*:${CMAKE_CURRENT_BINARY_DIR}/_deps/*.h")
            message(STATUS "  Suppressing warnings from: ${SUPPRESS_DIR}")
            message(STATUS "  Enabling checks: style, performance, warning, portability")
            message(STATUS "  Active suppressions: cppcheckError, internalAstError, unmatchedSuppression, passedByValue, syntaxError, preprocessorErrorDirective")
            set(CMAKE_CXX_CPPCHECK
                    ${CPPCHECK}
                    --template=${CPPCHECK_TEMPLATE}
                    --enable=style,performance,warning,portability
                    --inline-suppr
                    # We cannot act on a bug/missing feature of cppcheck
                    --suppress=cppcheckError
                    --suppress=internalAstError
                    # if a file does not have an internalAstError, we get an unmatchedSuppression error
                    --suppress=unmatchedSuppression
                    # noisy and incorrect sometimes
                    --suppress=passedByValue
                    # ignores code that cppcheck thinks is invalid C++
                    --suppress=syntaxError
                    --suppress=preprocessorErrorDirective
                    # ignores static_assert type failures
          --suppress=knownConditionTrueFalse
          --inconclusive
                    --suppress=${SUPPRESS_DIR}
            )
        else()
            message(STATUS "Using custom cppcheck options: ${CPPCHECK_OPTIONS}")
            # if the user provides a CPPCHECK_OPTIONS with a template specified, it will override this template
            set(CMAKE_CXX_CPPCHECK ${CPPCHECK} --template=${CPPCHECK_TEMPLATE} ${CPPCHECK_OPTIONS})
        endif()

    if(NOT
       "${CMAKE_CXX_STANDARD}"
       STREQUAL
       "")
      set(CMAKE_CXX_CPPCHECK ${CMAKE_CXX_CPPCHECK} --std=c++${CMAKE_CXX_STANDARD})
    endif()
    if(${WARNINGS_AS_ERRORS})
      list(APPEND CMAKE_CXX_CPPCHECK --error-exitcode=2)
    endif()
  else()
    message(WARNING "cppcheck requested but executable not found")
  endif()
endmacro()

macro(jsav_enable_clang_tidy target WARNINGS_AS_ERRORS)

    find_program(CLANGTIDY clang-tidy)
    if (CLANGTIDY)
        if (NOT CMAKE_CXX_COMPILER_ID MATCHES ".*Clang")
            get_target_property(TARGET_PCH ${target} INTERFACE_PRECOMPILE_HEADERS)
            if ("${TARGET_PCH}" STREQUAL "TARGET_PCH-NOTFOUND")
                get_target_property(TARGET_PCH ${target} PRECOMPILE_HEADERS)
            endif ()

            if (NOT ("${TARGET_PCH}" STREQUAL "TARGET_PCH-NOTFOUND"))
                message(SEND_ERROR "CLANG-TIDY|PCH conflict: "
                        "Cannot use clang-tidy with non-Clang compiler and precompiled headers. "
                        "GCC's PCH files are incompatible with clang-tidy.")
            else ()
                message(STATUS "No precompiled headers found - safe to proceed with clang-tidy")
            endif ()
        else ()
            message(STATUS "Clang compiler detected - no PCH compatibility issues expected")
        endif ()

        # Construct clang-tidy command line
        set(CLANG_TIDY_OPTIONS
                ${CLANGTIDY}
                -extra-arg=-Wno-unknown-warning-option
                -extra-arg=-Wno-ignored-optimization-argument
                -extra-arg=-Wno-unused-command-line-argument
                -p ${CMAKE_BINARY_DIR})

        # set standard
        if (NOT "${CMAKE_CXX_STANDARD}" STREQUAL "")
            if ("${CLANG_TIDY_OPTIONS_DRIVER_MODE}" STREQUAL "cl")
                list(APPEND CLANG_TIDY_OPTIONS -extra-arg=/std:c++${CMAKE_CXX_STANDARD})
                message(STATUS "Configured C++${CMAKE_CXX_STANDARD} standard for MSVC compatibility")
            else ()
                list(APPEND CLANG_TIDY_OPTIONS -extra-arg=-std=c++${CMAKE_CXX_STANDARD})
                message(STATUS "Configured C++${CMAKE_CXX_STANDARD} standard")
            endif ()
        endif ()

        # set warnings as errors
        if (${WARNINGS_AS_ERRORS})
            list(APPEND CLANG_TIDY_OPTIONS -warnings-as-errors=*)
            message(STATUS "Treating all clang-tidy warnings as errors")
        endif ()

    message("Also setting clang-tidy globally")
    set(CMAKE_CXX_CLANG_TIDY ${CLANG_TIDY_OPTIONS})
  else()
    message(WARNING "clang-tidy requested but executable not found")
  endif()
endmacro()

macro(jsav_enable_include_what_you_use)
  find_program(INCLUDE_WHAT_YOU_USE include-what-you-use)
  if(INCLUDE_WHAT_YOU_USE)
    set(CMAKE_CXX_INCLUDE_WHAT_YOU_USE ${INCLUDE_WHAT_YOU_USE})
  else()
    message(WARNING "include-what-you-use requested but executable not found")
  endif()
endmacro()
