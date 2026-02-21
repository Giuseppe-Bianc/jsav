#!/usr/bin/env bash
# -----------------------------------------------------------------------------
# build_and_valgrind.sh — Build the jsav project and run a full Valgrind
#                         memory analysis (Memcheck tool).
# Usage: ./build_and_valgrind.sh
# Author: (original author)
# Date:   2025-02-21
# Note:   Requires Valgrind installed and available in PATH.
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

cmake -S . -B ./build -Wno-dev -GNinja -Djsav_ENABLE_SANITIZER_ADDRESS=OFF -Djsav_WARNINGS_AS_ERRORS=ON -Djsav_ENABLE_CLANG_TIDY:BOOL=ON -Djsav_ENABLE_IPO:BOOL=OFF || die "cmake configuration failed."

cmake --build ./build -j 3 || die "cmake build failed."

if [[ -d "${RUN_DIR}" ]]; then
  cd "${RUN_DIR}" || die "Failed to change directory to ${RUN_DIR}."
  echo "Current working directory: $(pwd)"

  valgrind --time-stamp=yes --leak-check=full --track-origins=yes --vgdb=no --gen-suppressions=all --num-callers=20 --error-limit=no --freelist-vol=1000000 --expensive-definedness-checks=yes --undef-value-errors=yes --keep-stacktraces=alloc --read-inline-info=yes --partial-loads-ok=yes --trace-children=yes --demangle=yes --show-leak-kinds=all ./jsav || die "Valgrind run failed."

  #valgrind --tool=callgrind --time-stamp=yes --cache-sim=yes --branch-sim=yes --instr-atstart=yes --trace-children=yes --vgdb=no ./jsav
  echo "complete the valgrind work in $(pwd)"

  # Find the latest callgrind.out.<PID> file
  #CALLGRIND_FILE=$(ls -t callgrind.out.* 2>/dev/null | head -n 1)

  #if [ -z "$CALLGRIND_FILE" ]; then
  #    echo "No callgrind.out.<PID> file found!"
  #    exit 1
  #else
  #    echo "Found Callgrind output file: $CALLGRIND_FILE"
  #fi
else
  echo "Directory ${RUN_DIR} does not exist."
fi