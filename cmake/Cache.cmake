# Enable cache if available
function(jsav_enable_cache)
  # Definisce le opzioni valide per il sistema di cache
  set(VALID_CACHE_OPTIONS "ccache" "sccache")

  # If the user has not already provided CACHE_OPTION, default to "ccache".
  if (NOT DEFINED CACHE{CACHE_OPTION})
    set(CACHE_OPTION "ccache" CACHE STRING "Compiler cache backend to use. Accepted values: 'ccache', 'sccache'.")
  endif ()

  # Populate the GUI / cmake-gui dropdown with the known-valid choices.
  set_property(CACHE CACHE_OPTION PROPERTY STRINGS "${VALID_CACHE_OPTIONS}")

  list(FIND VALID_CACHE_OPTIONS "${CACHE_OPTION}" CACHE_OPTION_INDEX)
  if (CACHE_OPTION_INDEX EQUAL -1)
    message(STATUS
            "Using custom compiler cache backend: '${CACHE_OPTION}'. Natively supported options are: ${CACHE_OPTION_VALUES}."
    )
  endif ()

  find_program(CACHE_BINARY "${CACHE_OPTION}" HINTS ENV PATH NO_CACHE)
  if (CACHE_BINARY)
    message(STATUS "Compiler cache '${CACHE_BINARY}' found. Setting C and C++ compiler launchers.")
        # FORCE is required because CMAKE_C/CXX_COMPILER_LAUNCHER may already be
        # cached from a previous configure run with a different backend.
    set(CMAKE_C_COMPILER_LAUNCHER "${CACHE_BINARY}" CACHE FILEPATH "C compiler cache launcher." FORCE)
    set(CMAKE_CXX_COMPILER_LAUNCHER "${CACHE_BINARY}" CACHE FILEPATH "C++ compiler cache launcher." FORCE)
  else ()
    message(WARNING "Compiler cache backend '${CACHE_OPTION}' was not found on PATH. Compiler caching will not be enabled.")
    endif ()

    # Hide CACHE_OPTION from the default cmake-gui / ccmake view to reduce
    # noise; advanced users can still toggle it with the 'advanced' filter.
  mark_as_advanced(CACHE_OPTION)
endfunction()
