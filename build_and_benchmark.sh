#!/usr/bin/env bash
# -----------------------------------------------------------------------------
# build_and_benchmark.sh — Build the jsav project and run performance benchmarks.
# Usage: ./build_and_benchmark.sh
# Author: (generated from build_and_callgrind.sh template)
# Date:   2026-03-08
# Note:   Requires Bash >= 4.x and CMake with Ninja generator.
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
readonly BENCHMARK_DIR="${JSAV_ROOT}/build/test/"

clear

echo "Configuring jsav project for benchmarks..."
cmake -S . -B ./build -Wno-dev -GNinja \
  -Djsav_ENABLE_SANITIZER_ADDRESS=OFF \
  -Djsav_ENABLE_SANITIZER_UNDEFINED=OFF \
  -Djsav_WARNINGS_AS_ERRORS=ON \
  -Djsav_ENABLE_CLANG_TIDY:BOOL=OFF \
  -Djsav_ENABLE_CPPCHECK:BOOL=OFF \
  -Djsav_ENABLE_IPO:BOOL=OFF \
  -Djsav_PACKAGING_MAINTAINER_MODE=OFF || die "cmake configuration failed."

echo "Building benchmarks target..."
cmake --build ./build --config Release --target benchmarks || die "cmake build failed."

if [[ -d "${BENCHMARK_DIR}" ]]; then
  cd "${BENCHMARK_DIR}" || die "Failed to change directory to ${BENCHMARK_DIR}."
  echo "Current working directory: $(pwd)"
  echo ""
  echo "Running benchmarks with 2500 samples and 3s warmup..."
  echo "============================================================"
  echo ""
  
  ./benchmarks [!benchmark] --benchmark-samples 2500 --benchmark-warmup-time 3 \
    || die "Benchmark run failed."

  echo ""
  echo "============================================================"
  echo "Benchmark run complete!"
  echo "Results are displayed above."
else
  die "Directory ${BENCHMARK_DIR} does not exist."
fi
