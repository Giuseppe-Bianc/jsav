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

cache_is_stale() {
    [ ! -f "$CACHE_FILE" ] || \
    # stat -f %m is macOS, stat -c %Y is Linux/Git Bash
    [ $(($(date +%s) - $(stat -f %m "$CACHE_FILE" 2>/dev/null || stat -c %Y "$CACHE_FILE" 2>/dev/null || echo 0))) -gt $CACHE_MAX_AGE ]
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
    if git rev-parse --git-dir > /dev/null 2>&1; then
        BRANCH=$(git branch --show-current 2>/dev/null)
        STAGED=$(git diff --cached --numstat 2>/dev/null | wc -l | tr -d ' ')
        MODIFIED=$(git diff --numstat 2>/dev/null | wc -l | tr -d ' ')
        echo "$BRANCH|$STAGED|$MODIFIED" > "$CACHE_FILE"
    else
        echo "||" > "$CACHE_FILE"
    fi
fi

IFS='|' read -r BRANCH STAGED MODIFIED < "$CACHE_FILE"

BRANCH_CACHED=""
if [ -n "$BRANCH" ]; then
    BRANCH_CACHED="| 🌿 $BRANCH +$STAGED ~$MODIFIED"
else
    BRANCH_CACHED=""
fi

COST_PER_HOUR=$(awk "BEGIN {printf \"%.2f\", ($DURATION_MS > 0) ? ($COST / $DURATION_MS * 3600000) : 0}")
COST_FMT=$(printf '$%.2f ($%.2f/h)' "$COST" "$COST_PER_HOUR")
echo -e "📁 ${DIR##*/}$BRANCH_CACHED ${BAR_COLOR}${BAR}${RESET} ${PCT}% | ${YELLOW}${COST_FMT}${RESET} | ⏱️ ${MINS}m ${SECS}s"