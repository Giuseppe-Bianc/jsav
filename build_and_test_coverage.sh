#!/usr/bin/env bash
# -----------------------------------------------------------------------------
# build_and_test_coverage.sh — Build jsav with coverage, run tests, and
#                              generate a Cobertura/HTML coverage report.
# Usage: ./build_and_test_coverage.sh
# Author: (original author)
# Date:   2025-02-21
# Note:   Requires gcovr, gcov, and xdg-open (Linux desktop environment).
#         Requires cleanHtlmAndCss.sh to be in the same directory as this script.
# -----------------------------------------------------------------------------

set -euo pipefail

# -----------------------------------------------------------------------------
# Constants
# -----------------------------------------------------------------------------

# Resolve the directory this script lives in, regardless of the working
# directory from which it is invoked.
readonly SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
readonly CLEAN_SCRIPT="${SCRIPT_DIR}/cleanHtlmAndCss.sh"

readonly JSAV_ROOT="${PWD}"
readonly COBERTURA_HTML="${JSAV_ROOT}/out/coverage/index.html"
readonly BUILD_DIR="${JSAV_ROOT}/build"

# -----------------------------------------------------------------------------
# Functions
# -----------------------------------------------------------------------------

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

# Delegates all coverage artefact cleanup to the dedicated clean script.
# Exits with a descriptive error if the script cannot be found or executed.
run_clean() {
  [[ -f "${CLEAN_SCRIPT}" ]] \
    || die "cleanHtlmAndCss.sh not found at: ${CLEAN_SCRIPT}"
  [[ -x "${CLEAN_SCRIPT}" ]] \
    || die "cleanHtlmAndCss.sh is not executable: ${CLEAN_SCRIPT}"

  bash "${CLEAN_SCRIPT}" || die "cleanHtlmAndCss.sh failed."
}

# -----------------------------------------------------------------------------
# Main logic
# -----------------------------------------------------------------------------

run_clean
clear

cmake -S . -B ./build -Wno-dev -GNinja \
  -Djsav_WARNINGS_AS_ERRORS=ON \
  -Djsav_ENABLE_CLANG_TIDY:BOOL=ON \
  -Djsav_ENABLE_COVERAGE:BOOL=ON \
  -Djsav_ENABLE_CPPCHECK:BOOL=OFF \
  -Djsav_ENABLE_IPO:BOOL=OFF \
  -Djsav_PACKAGING_MAINTAINER_MODE=OFF || die "cmake configuration failed."

cmake --build ./build -j 3 || die "cmake build failed."

if [[ ! -d "${BUILD_DIR}" ]]; then
  die "Build directory does not exist after cmake build: ${BUILD_DIR}"
fi

cd "${BUILD_DIR}" || die "Failed to change directory to ${BUILD_DIR}."
echo "Current working directory: $(pwd)"

ctest -C Debug || die "ctest run failed."

read -rp "Press any key to run gcovr... " -n 1 -s
clear
echo "Current working directory: $(pwd)"

  gcovr -j 3  --root ../ --config ../gcovr.cfg --gcov-executable 'gcov' --exclude-unreachable-branches --exclude-noncode-lines || die "gcovr failed."

xdg-open "${COBERTURA_HTML}" || die "Failed to open coverage report."
echo "complete."