#!/usr/bin/env bash
# -----------------------------------------------------------------------------
# build_and_test_coverage.sh — Build jsav with coverage, run tests, and
#                              generate a Cobertura/HTML coverage report.
# Usage: ./build_and_test_coverage.sh
# Author: (original author)
# Date:   2025-02-21
# Note:   Requires gcovr, gcov, and xdg-open (Linux desktop environment).
# -----------------------------------------------------------------------------

set -euo pipefail

# Prints an error message to stderr and exits.
# Arguments:
#   $1 - Error message string.
#   $2 - (Optional) Exit code. Defaults to 1.
die() {
  local message="${1:-"An unexpected error occurred."}"
  local exit_code="${2:-1}"
  if ! [[ "${exit_code}" =~ ^[0-9]+$ ]]; then
    echo "ERROR: Invalid exit code '${exit_code}', using 1." >&2
    exit_code=1
  fi
  echo "ERROR: ${message}" >&2
  exit "${exit_code}"
}

readonly JSAV_ROOT="${PWD}"
readonly COBERTURA_F="${JSAV_ROOT}/out/cobertura.xml"
readonly COBERTURA_HTML="${JSAV_ROOT}/out/coverage/index.html"
readonly COVERAGE_DIR="${JSAV_ROOT}/out/coverage"
readonly BUILD_DIR="${JSAV_ROOT}/build"

if [[ -d "${COVERAGE_DIR}" ]]; then
  cd "${COVERAGE_DIR}" || die "Failed to change directory to ${COVERAGE_DIR}."
  echo "Cleaning up .html and .css files in ${COVERAGE_DIR}"

  # Remove all .html files if they exist
  shopt -s nullglob
  html_files=( "${COVERAGE_DIR}"/*.html )
  shopt -u nullglob
  if [[ ${#html_files[@]} -gt 0 ]]; then
    rm -- "${html_files[@]}"
  else
    echo "No .html files found to delete."
  fi

  # Remove all .css files if they exist
  shopt -s nullglob
  css_files=( "${COVERAGE_DIR}"/*.css )
  shopt -u nullglob
  if [[ ${#css_files[@]} -gt 0 ]]; then
    rm -- "${css_files[@]}"
  else
    echo "No .css files found to delete."
  fi

  # Remove cobertura.xml file if it exists
  if [[ -f "${COBERTURA_F}" ]]; then
    rm -- "${COBERTURA_F}"
  else
    echo "No ${COBERTURA_F} file found to delete."
  fi

  echo "Cleanup complete."
else
  echo "Directory ${COVERAGE_DIR} does not exist."
fi

cd "${JSAV_ROOT}" || die "Failed to return to project root ${JSAV_ROOT}."
clear

cmake -S . -B ./build -Wno-dev -GNinja \
  -Djsav_WARNINGS_AS_ERRORS=ON \
  -Djsav_ENABLE_CLANG_TIDY:BOOL=ON \
  -Djsav_ENABLE_COVERAGE:BOOL=ON \
  -Djsav_ENABLE_CPPCHECK:BOOL=OFF \
  -Djsav_ENABLE_IPO:BOOL=OFF \
  -Djsav_PACKAGING_MAINTAINER_MODE=OFF || die "cmake configuration failed."

cmake --build ./build --target tests -j 3 || die "cmake build of 'tests' target failed."
cmake --build ./build --target constexpr_tests -j 3 || die "cmake build of 'constexpr_tests' target failed."
cmake --build ./build --target relaxed_constexpr_tests -j 3 || die "cmake build of 'relaxed_constexpr_tests' target failed."

if [[ -d "${BUILD_DIR}" ]]; then
  cd "${BUILD_DIR}" || die "Failed to change directory to ${BUILD_DIR}."
  echo "Current working directory: $(pwd)"

  ctest -C Debug || die "ctest run failed."

  read -rp "Press any key to run gcovr... " -n 1 -s
  clear
  echo "Current working directory: $(pwd)"

  gcovr -j 3  --root ../ --config ../gcovr.cfg --gcov-executable 'gcov' --rerun-failed --output-on-failure --exclude-unreachable-branches --exclude-noncode-lines || die "gcovr failed."

  xdg-open "${COBERTURA_HTML}" || die "Failed to open coverage report."
  echo "complete."
else
  echo "Directory ${BUILD_DIR} does not exist."
fi