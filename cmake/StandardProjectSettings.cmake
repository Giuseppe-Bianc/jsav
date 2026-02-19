# Set a default build type if none was specified
# Check if neither a build type nor configuration types are set.
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  message(STATUS "No build type specified. Defaulting to 'RelWithDebInfo'.")
  set(CMAKE_BUILD_TYPE RelWithDebInfo CACHE STRING "Choose the type of build." FORCE)
  # Define the possible build types for cmake-gui or ccmake.
  set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Debug" "Release" "MinSizeRel" "RelWithDebInfo")
else()
  message(STATUS "User-specified build type: ${CMAKE_BUILD_TYPE}")
endif()

# Print general CMake configuration details.
message(STATUS "CMake Version: ${CMAKE_VERSION}")
message(STATUS "CMake Generator: ${CMAKE_GENERATOR}")

# Check and display configuration type information.
if(DEFINED CMAKE_CONFIGURATION_TYPES)
  string(REPLACE ";" ", " config_list "${CMAKE_CONFIGURATION_TYPES}")
  message(STATUS "Multi-configuration generator detected. Available configurations: ${config_list}")
else()
  message(STATUS "Single-configuration generator detected. Current build type: ${CMAKE_BUILD_TYPE}")
endif()

message(STATUS "Configured build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "Available build types (for single-configuration generators): Debug, Release, MinSizeRel, RelWithDebInfo")


# Generate compile_commands.json to make it easier to work with clang based tools
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Enhanced compiler diagnostics configuration
message(STATUS "Configuring enhanced compiler diagnostics...")

if(CMAKE_CXX_COMPILER_ID MATCHES ".*Clang")
    add_compile_options($<$<COMPILE_LANGUAGE:C>:-fcolor-diagnostics> $<$<COMPILE_LANGUAGE:CXX>:-fcolor-diagnostics>)
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    add_compile_options($<$<COMPILE_LANGUAGE:C>:-fdiagnostics-color=always>
                        $<$<COMPILE_LANGUAGE:CXX>:-fdiagnostics-color=always>)
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC" AND MSVC_VERSION GREATER 1900)
  add_compile_options(/diagnostics:column)
else()
  message(WARNING "Unsupported compiler detected: ${CMAKE_CXX_COMPILER_ID}\n"
          "  No enhanced diagnostics configured. Please consider:\n"
          "  - Requesting support for this compiler\n"
          "  - Checking documentation for compiler-specific diagnostic options\n"
          "  - Updating to a newer compiler version if possible")
endif()

message(STATUS "Completed compiler diagnostics configuration for ${CMAKE_CXX_COMPILER_ID}")


# run vcvarsall when msvc is used
include("${CMAKE_CURRENT_LIST_DIR}/VCEnvironment.cmake")
jsav_run_vcvarsall()
