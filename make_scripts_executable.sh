#!/usr/bin/env bash
# -----------------------------------------------------------------------------
# make_scripts_executable.sh — Grants execute permission to all project
#                              build and utility scripts.
# Usage: ./make_scripts_executable.sh
# Author:
# Date:
# -----------------------------------------------------------------------------

set -euo pipefail

# -----------------------------------------------------------------------------
# Constants
# -----------------------------------------------------------------------------

readonly SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

readonly -a TARGET_SCRIPTS=(
  "build_and_callgrind.sh"
  "build_and_run.sh"
  "build_and_test_coverage.sh"
  "build_and_valgrind.sh"
  "cleanHtlmAndCss.sh"
)

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

# Grants owner execute permission to a single script file.
# Prints a status line for each file processed.
# Arguments:
#   $1 - Absolute path to the script file.
make_executable() {
  local script_path="${1}"

  if [[ ! -f "${script_path}" ]]; then
    echo "  [SKIP]  ${script_path} — file not found."
    return 0
  fi

  chmod u+x "${script_path}" \
    || die "chmod failed for: ${script_path}"

  echo "  [OK]    ${script_path}"
}

# -----------------------------------------------------------------------------
# Main logic
# -----------------------------------------------------------------------------

echo "Setting execute permission on project scripts in: ${SCRIPT_DIR}"
echo ""

for script in "${TARGET_SCRIPTS[@]}"; do
  make_executable "${SCRIPT_DIR}/${script}"
done

echo ""
echo "Done."