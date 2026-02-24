#!/bin/bash
input=$(cat)

MODEL=$(echo "$input" | jq -r '.model.display_name')
DIR=$(echo "$input" | jq -r '.workspace.current_dir')
COST=$(echo "$input" | jq -r '.cost.total_cost_usd // 0')
PCT=$(echo "$input" | jq -r '.context_window.used_percentage // 0' | cut -d. -f1)
DURATION_MS=$(echo "$input" | jq -r '.cost.total_duration_ms // 0')

CYAN='\033[36m'; GREEN='\033[32m'; YELLOW='\033[33m'; RED='\033[31m'; RESET='\033[0m'

# OS-independent cache directory
if [[ "$OSTYPE" == "msys"* ]] || [[ "$OSTYPE" == "win32" ]] || [[ "$OSTYPE" == "cygwin"* ]]; then
    CACHE_DIR="${TEMP:-${TMP:-/tmp}}"
else
    if [[ -d "/dev/shm" ]]; then
        CACHE_DIR="/dev/shm"
    else
        CACHE_DIR="/tmp"
    fi
fi

CACHE_FILE="$CACHE_DIR/statusline-git-cache"
CACHE_MAX_AGE=5  # seconds

# Returns the mtime (seconds since epoch) of a file in a cross-platform way.
# stat -f %m is BSD/macOS syntax; GNU stat (Linux, Windows Git Bash) uses -c %Y.
# On GNU stat, -f does NOT error — it prints filesystem info instead, so the
# two variants must be selected by $OSTYPE rather than chained with ||.
get_mtime() {
    if [[ "$OSTYPE" == "darwin"* ]]; then
        stat -f %m "$1" 2>/dev/null || echo 0
    else
        stat -c %Y "$1" 2>/dev/null || echo 0
    fi
}

# Returns a fingerprint that changes whenever staged, modified, OR untracked
# file counts change.
#
# .git/index mtime  — changes on every git add / git reset (staged changes)
# .git/HEAD content — changes on branch switch or commit
# untracked_count   — git ls-files --others covers new untracked files;
#                     these never touch .git/index so must be tracked separately
cache_get_git_sig() {
    local git_dir
    git_dir=$(git -C "$DIR" rev-parse --git-dir 2>/dev/null) || { echo ""; return; }
    # Resolve relative git-dir (e.g. ".git") against DIR so stat can find
    # .git/index even when the script's CWD differs from the repo root.
    case "$git_dir" in
        /*|?:*) : ;;              # already absolute: /home/…  or  C:\…
        *)      git_dir="$DIR/$git_dir" ;;
    esac
    local index_mtime
    index_mtime=$(get_mtime "$git_dir/index")
    local head_ref
    head_ref=$(cat "$git_dir/HEAD" 2>/dev/null || echo "")
    local untracked_count
    untracked_count=$(git -C "$DIR" ls-files --others --exclude-standard 2>/dev/null \
                      | wc -l | tr -d ' ')
    echo "${index_mtime}:${head_ref}:${untracked_count}"
}

# Cache file layout (3 lines):
#   Line 1 — DIR:<current_dir>
#   Line 2 — SIG:<git_fingerprint>
#   Line 3 — <branch>|<staged>|<modified>|<untracked>
#
# Staleness is declared when ANY of the following is true:
#   • the file does not exist
#   • the stored DIR does not match the current DIR
#   • the stored SIG does not match the current git fingerprint
#   • the file mtime exceeds CACHE_MAX_AGE (fallback for non-git dirs)
cache_is_stale() {
    [ ! -f "$CACHE_FILE" ] && return 0

    local stored_dir
    stored_dir=$(sed -n '1p' "$CACHE_FILE")
    [ "$stored_dir" != "DIR:$DIR" ] && return 0

    local stored_sig
    stored_sig=$(sed -n '2p' "$CACHE_FILE")
    local current_sig
    current_sig="SIG:$(cache_get_git_sig)"
    [ "$stored_sig" != "$current_sig" ] && return 0

    local mtime
    mtime=$(get_mtime "$CACHE_FILE")
    [ $(($(date +%s) - mtime)) -gt $CACHE_MAX_AGE ]
}

# Pick bar color based on context usage
if [ "$PCT" -ge 90 ]; then BAR_COLOR="$RED"
elif [ "$PCT" -ge 70 ]; then BAR_COLOR="$YELLOW"
else BAR_COLOR="$GREEN"; fi

FILLED=$((PCT / 10)); EMPTY=$((10 - FILLED))
BAR=""
for ((i=0; i<FILLED; i++)); do BAR+="█"; done
for ((i=0; i<EMPTY; i++)); do BAR+="░"; done

MINS=$((DURATION_MS / 60000)); SECS=$(((DURATION_MS % 60000) / 1000))

if cache_is_stale; then
    if git -C "$DIR" rev-parse --git-dir > /dev/null 2>&1; then
        BRANCH=$(git -C "$DIR" branch --show-current 2>/dev/null)
        STAGED=$(git -C "$DIR" diff --cached --numstat 2>/dev/null | wc -l | tr -d ' ')
        MODIFIED=$(git -C "$DIR" diff --numstat 2>/dev/null | wc -l | tr -d ' ')
        UNTRACKED=$(git -C "$DIR" ls-files --others --exclude-standard 2>/dev/null \
                    | wc -l | tr -d ' ')
        SIG=$(cache_get_git_sig)
        printf 'DIR:%s\nSIG:%s\n%s|%s|%s|%s\n' \
            "$DIR" "$SIG" "$BRANCH" "$STAGED" "$MODIFIED" "$UNTRACKED" > "$CACHE_FILE"
    else
        printf 'DIR:%s\nSIG:\n|||\n' "$DIR" > "$CACHE_FILE"
    fi
fi

IFS='|' read -r BRANCH STAGED MODIFIED UNTRACKED < <(sed -n '3p' "$CACHE_FILE")

BRANCH_CACHED=""
if [ -n "$BRANCH" ]; then
    BRANCH_CACHED="| 🌿 $BRANCH +$STAGED ~$MODIFIED ?$UNTRACKED"
else
    BRANCH_CACHED=""
fi

COST_PER_HOUR=$(awk "BEGIN {printf \"%.2f\", ($DURATION_MS > 0) ? ($COST / $DURATION_MS * 3600000) : 0}")
COST_FMT=$(printf '$%.2f ($%.2f/h)' "$COST" "$COST_PER_HOUR")
echo -e "📁 ${DIR##*/}$BRANCH_CACHED ${BAR_COLOR}${BAR}${RESET} ${PCT}% | ${YELLOW}${COST_FMT}${RESET} | ⏱️ ${MINS}m ${SECS}s"