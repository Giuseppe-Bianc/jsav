add_executable(jsav main.cpp
        Costanti.hpp)


# Retrieve the target name
get_target_property(target_name jsav NAME)
include("${CMAKE_SOURCE_DIR}/cmake/Simd.cmake")
set_simd_instructions(${target_name})

target_link_libraries(
        jsav
        PRIVATE jsav::jsav_options
        jsav::jsav_warnings)

target_link_system_libraries(
        jsav
        PRIVATE
        CLI11::CLI11
        jsav::jsav_lib
)

target_include_directories(jsav PRIVATE "${CMAKE_BINARY_DIR}/configured_files/include")