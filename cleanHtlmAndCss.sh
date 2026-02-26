#!/usr/bin/env bash
# -----------------------------------------------------------------------------
# clean_coverage.sh — Removes generated coverage artefacts (HTML, CSS, and
#                     Cobertura XML) from the JSAV output directories.
# Usage: ./clean_coverage.sh
# Author:
# Date:
# -----------------------------------------------------------------------------

set -euo pipefail

readonly JSAV_ROOT="${PWD}"
readonly COVERAGE_DIR="${JSAV_ROOT}/out/coverage"
readonly COBERTURA_F="${JSAV_ROOT}/out/cobertura.xml"
readonly BUILD_DIR="${JSAV_ROOT}/build"   # Reserved for future use.

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


# Removes all files matching a glob pattern inside a directory.
# Prints a notice when no matching files are found.
# Arguments:
#   $1 - The glob pattern to match (e.g. "/some/dir/*.html").
#   $2 - A human-readable label for the file type (e.g. ".html").
remove_by_glob() {
  local pattern="${1}"
  local label="${2}"

  # nullglob prevents the literal glob string from being passed to `rm`
  # when no files match; it is scoped tightly to this function.
  shopt -s nullglob
  local -a matched=( ${pattern} )
  shopt -u nullglob

  if [[ ${#matched[@]} -gt 0 ]]; then
    rm -- "${matched[@]}"
  else
    echo "No ${label} files found to delete."
  fi
}

# -----------------------------------------------------------------------------
# Main logic
# -----------------------------------------------------------------------------

if [[ ! -d "${COVERAGE_DIR}" ]]; then
  echo "Directory ${COVERAGE_DIR} does not exist."
  exit 0
fi

echo "Cleaning up .html and .css files in ${COVERAGE_DIR}"

remove_by_glob "${COVERAGE_DIR}/*.html" ".html"
remove_by_glob "${COVERAGE_DIR}/*.css"  ".css"

# Remove the Cobertura XML report if present.
if [[ -f "${COBERTURA_F}" ]]; then
  rm -- "${COBERTURA_F}"
else
  echo "No ${COBERTURA_F} file found to delete."
fi

echo "Cleanup complete."