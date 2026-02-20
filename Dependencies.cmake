include(cmake/CPM.cmake)

function(AddSpdlogPackage WcharSupport WcharFilenames)
  cpmaddpackage(
          NAME spdlog
          VERSION 1.17.0
          GITHUB_REPOSITORY "gabime/spdlog"
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
    cpmaddpackage("gh:fmtlib/fmt#12.1.0")
  endif()

  if (NOT TARGET spdlog::spdlog)
        if (WIN32)
            AddSpdlogPackage(ON ON)
        else ()
            AddSpdlogPackage(OFF OFF)
        endif ()
    endif ()

  if(NOT TARGET Catch2::Catch2WithMain)
    cpmaddpackage("gh:catchorg/Catch2@3.13.0")
  endif()

  if(NOT TARGET CLI11::CLI11)
    cpmaddpackage("gh:CLIUtils/CLI11@2.6.1")
  endif()


endfunction()
