include("${CMAKE_CURRENT_LIST_DIR}/Utilities.cmake")

macro(jsav_detect_architecture)
  # detect the architecture
  string(TOLOWER "${CMAKE_SYSTEM_PROCESSOR}" CMAKE_SYSTEM_PROCESSOR_LOWER)

  if (CMAKE_SYSTEM_PROCESSOR_LOWER MATCHES "^(x86|i[3456]86)$")
    set(VCVARSALL_ARCH "x86")
  elseif (CMAKE_SYSTEM_PROCESSOR_LOWER MATCHES "^(x64|x86_64|amd64)$")
    set(VCVARSALL_ARCH "x64")
  elseif (CMAKE_SYSTEM_PROCESSOR_LOWER STREQUAL "arm")
    set(VCVARSALL_ARCH "arm")
  elseif (CMAKE_SYSTEM_PROCESSOR_LOWER MATCHES "^(arm64|aarch64)$")
    set(VCVARSALL_ARCH "arm64")
  else ()
    set(VCVARSALL_ARCH "${CMAKE_SYSTEM_PROCESSOR}")
    message(WARNING "Unknown architecture detected: ${CMAKE_SYSTEM_PROCESSOR_LOWER}. Using system processor architecture: ${VCVARSALL_ARCH}.")
  endif ()

  # Determine pointer size (4 for 32-bit, 8 for 64-bit)
  if (CMAKE_SIZEOF_VOID_P EQUAL 8)
    set(ARCHITECTURE_BITNESS "64-bit")
  elseif (CMAKE_SIZEOF_VOID_P EQUAL 4)
    set(ARCHITECTURE_BITNESS "32-bit")
  else ()
    set(ARCHITECTURE_BITNESS "Unknown bitness")
  endif ()

  # Output detailed architecture information
  message(STATUS "Detected system processor: ${CMAKE_SYSTEM_PROCESSOR}")
  message(STATUS "Normalized processor: ${CMAKE_SYSTEM_PROCESSOR_LOWER}")
  message(STATUS "Architecture bitness: ${ARCHITECTURE_BITNESS}")
  message(STATUS "VCVARSALL_ARCH set to: ${VCVARSALL_ARCH}")
endmacro()

# Run vcvarsall.bat and set CMake environment variables
function(jsav_run_vcvarsall)
  # if MSVC but VSCMD_VER is not set, which means vcvarsall has not run
  if(MSVC AND "$ENV{VSCMD_VER}" STREQUAL "")

    # find vcvarsall.bat
    get_filename_component(MSVC_DIR ${CMAKE_CXX_COMPILER} DIRECTORY)
    find_file(
      VCVARSALL_FILE
      NAMES vcvarsall.bat
      PATHS "${MSVC_DIR}"
            "${MSVC_DIR}/.."
            "${MSVC_DIR}/../.."
            "${MSVC_DIR}/../../../../../../../.."
            "${MSVC_DIR}/../../../../../../.."
            PATH_SUFFIXES "VC/Auxiliary/Build" "Common7/Tools" "Tools")

    if(EXISTS ${VCVARSALL_FILE})
      # detect the architecture
      jsav_detect_architecture()

      # Run vcvarsall.bat and print informative messages
      message(STATUS "Searching for vcvarsall.bat in the Visual Studio directory...")
      message(STATUS "Found vcvarsall.bat at: ${VCVARSALL_FILE}")
      # Run vcvarsall.bat to set up the MSVC environment
      message(STATUS "Running `${VCVARSALL_FILE} ${VCVARSALL_ARCH}` to set up the MSVC environment")
      execute_process(
              COMMAND cmd /c "${VCVARSALL_FILE}" "${VCVARSALL_ARCH}" "&&" "call" "echo" "VCVARSALL_ENV_START" "&" "set"
              OUTPUT_VARIABLE VCVARSALL_OUTPUT
              OUTPUT_STRIP_TRAILING_WHITESPACE)

      # Parse the output and set the environment variables
      find_substring_by_prefix(VCVARSALL_ENV "VCVARSALL_ENV_START" "${VCVARSALL_OUTPUT}")
      if (NOT VCVARSALL_ENV)
        message(WARNING "No environment variables found in the vcvarsall.bat output. Check for errors.")
      else ()
        set_env_from_string("${VCVARSALL_ENV}")
        message(STATUS "MSVC environment is set up successfully.")
      endif ()
    else ()
      message(WARNING "Could not find `vcvarsall.bat` for automatic MSVC environment preparation. Please manually open the MSVC command prompt and rebuild the project.")
    endif ()
  endif ()
endfunction()
