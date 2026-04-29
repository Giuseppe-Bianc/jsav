include(cmake/CPM.cmake)

function(AddSpdlogPackage WcharSupport WcharFilenames)
    cpmaddpackage(
            NAME spdlog
            VERSION 1.17.0
            GITHUB_REPOSITORY "gabime/spdlog"
            SYSTEM
            YES
            OPTIONS
            "SPDLOG_FMT_EXTERNAL ON"
            "SPDLOG_ENABLE_PCH ON"
            "SPDLOG_BUILD_PIC ON"
            "SPDLOG_WCHAR_SUPPORT ${WcharSupport}"
            "SPDLOG_WCHAR_FILENAMES ${WcharFilenames}"
            "SPDLOG_SANITIZE_ADDRESS OFF"
    )

endfunction()

# Done as a function so that updates to variables like
# CMAKE_CXX_FLAGS don't propagate out to other
# targets
function(jsav_setup_dependencies)

  # For each dependency, see if it's
  # already been provided to us by a parent project

  if(NOT TARGET fmtlib::fmtlib)
    cpmaddpackage(
      NAME
      fmt
      GITHUB_REPOSITORY
      "fmtlib/fmt"
      GIT_TAG
      "12.1.0"
      SYSTEM
      YES)
    target_compile_options(fmt PRIVATE $<$<COMPILE_LANG_AND_ID:CXX,MSVC>:/wd4834>)
  endif()

    if (NOT TARGET spdlog::spdlog)
        if (WIN32)
            AddSpdlogPackage(ON ON)
        else ()
            AddSpdlogPackage(OFF OFF)
        endif ()
    endif ()

  if(NOT TARGET Catch2::Catch2WithMain)
    cpmaddpackage(
      NAME
      Catch2
      VERSION
      3.14.0
      GITHUB_REPOSITORY
      "catchorg/Catch2"
      SYSTEM
      YES)
  endif()

  if(NOT TARGET CLI11::CLI11)
    cpmaddpackage(
      NAME
      CLI11
      VERSION
      2.6.1
      GITHUB_REPOSITORY
      "CLIUtils/CLI11"
      SYSTEM
      YES)
  endif()

endfunction()
