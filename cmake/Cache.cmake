# Enable cache if available
function(jsav_enable_cache)
  # Definisce le opzioni valide per il sistema di cache
  set(VALID_CACHE_OPTIONS "ccache" "sccache")

  # Se non è già definita, imposta l'opzione di cache di default su "ccache"
  if (NOT DEFINED CACHE_OPTION)
    set(CACHE_OPTION "ccache" CACHE STRING "Compiler cache to be used (choices: 'ccache', 'sccache')")
  endif()

  # Imposta i valori ammissibili per CACHE_OPTION (visibili nella GUI o nei file di cache)
  set_property(CACHE CACHE_OPTION PROPERTY STRINGS ${VALID_CACHE_OPTIONS})

  # Verifica se il valore scelto è tra quelli ammessi
  list(FIND VALID_CACHE_OPTIONS "${CACHE_OPTION}" CACHE_OPTION_INDEX)
  if (CACHE_OPTION_INDEX EQUAL -1)
    message(STATUS
            "Using custom compiler cache system: '${CACHE_OPTION}'. Supported options are: ${CACHE_OPTION_VALUES}"
    )
  endif ()

  # Cerca il binario corrispondente al sistema di cache selezionato (ccache o sccache)
  find_program(CACHE_BINARY "${CACHE_OPTION}" HINTS ENV PATH NO_CACHE)
  if (CACHE_BINARY)
    message(STATUS "Compiler cache system '${CACHE_BINARY}' found. Enabling cache.")
    # Configura il launcher del compilatore per C e C++ per utilizzare il sistema di cache
    set(CMAKE_C_COMPILER_LAUNCHER "${CACHE_BINARY}" CACHE FILEPATH "C compiler cache" FORCE)
    set(CMAKE_CXX_COMPILER_LAUNCHER "${CACHE_BINARY}" CACHE FILEPATH "C++ compiler cache" FORCE)
  else()
    message(WARNING "Il sistema di cache selezionato '${CACHE_OPTION}' non è stato trovato nel PATH. La cache del compilatore non verrà abilitata.")
  endif()

  # Segna l'opzione CACHE_OPTION come avanzata per non ingombrare la visualizzazione principale della cache
  mark_as_advanced(CACHE_OPTION)
endfunction()
