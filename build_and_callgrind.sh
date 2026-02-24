#!/usr/bin/env bash
# -----------------------------------------------------------------------------
# build_and_callgrind.sh — Build the jsav project and run Callgrind profiling.
# Usage: ./build_and_callgrind.sh
# Author: (original author)
# Date:   2025-02-21
# Note:   Requires Bash >= 4.x and Valgrind with Callgrind tool installed.
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
readonly RUN_DIR="${JSAV_ROOT}/build/src/jsav/"

clear

cmake -S . -B ./build -Wno-dev -GNinja \
  -Djsav_ENABLE_SANITIZER_ADDRESS=OFF \
  -Djsav_WARNINGS_AS_ERRORS=ON \
  -Djsav_ENABLE_CLANG_TIDY:BOOL=ON \
  -Djsav_ENABLE_IPO:BOOL=OFF \
  -Djsav_PACKAGING_MAINTAINER_MODE=OFF || die "cmake configuration failed."

cmake --build ./build -j 3 || die "cmake build failed."

if [[ -d "${RUN_DIR}" ]]; then
  cd "${RUN_DIR}" || die "Failed to change directory to ${RUN_DIR}."
  echo "Current working directory: $(pwd)"

  #valgrind --time-stamp=yes --leak-check=full --track-origins=yes --vgdb=no --gen-suppressions=all --num-callers=20 --error-limit=no --freelist-vol=1000000 --expensive-definedness-checks=yes --undef-value-errors=yes --keep-stacktraces=alloc --read-inline-info=yes --partial-loads-ok=yes --trace-children=yes --demangle=yes --show-leak-kinds=all ./jsav
   valgrind --tool=callgrind --time-stamp=yes --cache-sim=yes --branch-sim=yes --instr-atstart=yes --trace-children=yes --vgdb=no ./jsav \
    || die "Valgrind/Callgrind run failed."

  echo "complete the valgrind work in $(pwd)"

  # Find the latest callgrind.out.<PID> file
  shopt -s nullglob
  callgrind_files=( callgrind.out.* )
  shopt -u nullglob

  if [[ ${#callgrind_files[@]} -eq 0 ]]; then
    die "No callgrind.out.<PID> file found!"
  fi

  # Determine the most recently modified file among the glob matches.
  CALLGRIND_FILE=""
  latest_time=0
  for f in "${callgrind_files[@]}"; do
    f_time=$(stat -c "%Y" "${f}" 2>/dev/null || stat -f "%m" "${f}" 2>/dev/null) \
      || die "stat failed on ${f}."
    if (( f_time > latest_time )); then
      latest_time="${f_time}"
      CALLGRIND_FILE="${f}"
    fi
  done

  echo "Found Callgrind output file: ${CALLGRIND_FILE}"
else
  echo "Directory ${RUN_DIR} does not exist."
fi