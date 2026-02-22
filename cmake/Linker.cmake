macro(jsav_configure_linker project_name)
    set(jsav_USER_LINKER_OPTION "DEFAULT" CACHE STRING "Linker to be used")
    set(jsav_USER_LINKER_OPTION_VALUES "DEFAULT" "SYSTEM" "LLD" "GOLD" "BFD" "MOLD" "SOLD" "APPLE_CLASSIC" "MSVC")
    set_property(CACHE jsav_USER_LINKER_OPTION PROPERTY STRINGS ${jsav_USER_LINKER_OPTION_VALUES})
    list(
            FIND
            jsav_USER_LINKER_OPTION_VALUES
            ${jsav_USER_LINKER_OPTION}
            jsav_USER_LINKER_OPTION_INDEX
    )

    if (${jsav_USER_LINKER_OPTION_INDEX} EQUAL -1)
        message(STATUS "Using custom linker: '${jsav_USER_LINKER_OPTION}', explicitly supported entries are ${jsav_USER_LINKER_OPTION_VALUES}")
    endif ()

    set_target_properties(${project_name} PROPERTIES LINKER_TYPE "${jsav_USER_LINKER_OPTION}")
endmacro()
