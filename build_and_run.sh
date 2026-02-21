#!/usr/bin/env bash
# -----------------------------------------------------------------------------
# build_and_run.sh — Build the jsav project and run the resulting binary.
# Usage: ./build_and_run.sh
# Author: (original author)
# Date:   2025-02-21
# -----------------------------------------------------------------------------

set -euo pipefail

# Prints an error message to stderr and exits.
# Arguments:
#   $1 - Error message string.
#   $2 - (Optional) Exit code. Defaults to 1.
die() {
  local message="${1:-"An unexpected error occurred."}"
  local exit_code="${2:-1}"
  echo "ERROR: ${message}" >&2
  exit "${exit_code}"
}

readonly JSAV_ROOT="${PWD}"
readonly RUN_DIR="${JSAV_ROOT}/build/src/jsav/"

clear

cmake -S . -B ./build -Wno-dev -GNinja -Djsav_WARNINGS_AS_ERRORS=ON -Djsav_ENABLE_CLANG_TIDY:BOOL=ON -Djsav_ENABLE_IPO:BOOL=OFF || die "cmake configuration failed."

cmake --build ./build -j 3 || die "cmake build failed."

if [[ -d "${RUN_DIR}" ]]; then
  cd "${RUN_DIR}" || die "Failed to change directory to ${RUN_DIR}."
  echo "Current working directory: $(pwd)"
  ./jsav || die "jsav execution failed."
  echo "complete."
else
  echo "Directory ${RUN_DIR} does not exist."
fi