#!/usr/bin/env bash
# ===========================================================================
#  FTD Engine Test Runner
#  Usage:  ./engine/run_tests.sh [OPTIONS]
#
#  Options:
#    --build       Build the engine before testing (default)
#    --no-build    Skip the build step
#    --filter RE   Only run tests matching regex RE (passed to ctest -R)
#    --label LBL   Only run tests with label LBL (passed to ctest -L)
#                  Labels: unit, campaign, foundation, lagrangian, scale1, scale2
#    --jobs N      Run N tests in parallel (passed to ctest -j)
#    --verbose     Show full output for every test, not just failures
#    --cuda        Use engine/build_cuda instead of engine/build
#    --debug       Use Debug configuration instead of Release
#    --eta         Show estimated run time from previous test data
#    -h, --help    Show this help
# ===========================================================================

set -uo pipefail

# ── ANSI color codes ─────────────────────────────────────────────────────
RED=$'\033[0;31m'
GREEN=$'\033[0;32m'
YELLOW=$'\033[1;33m'
CYAN=$'\033[0;36m'  # reserved for future use
BOLD=$'\033[1m'
DIM=$'\033[2m'
RESET=$'\033[0m'
CLR=$'\033[K'

# ── Resolve paths ────────────────────────────────────────────────────────
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
ENGINE_DIR="$SCRIPT_DIR"

# ── Defaults ─────────────────────────────────────────────────────────────
DO_BUILD=true
BUILD_DIR="$ENGINE_DIR/build"
FILTER=""
LABEL=""
JOBS=""
VERBOSE=""
CONFIG="Release"
SHOW_ETA=false

# ── Parse arguments ──────────────────────────────────────────────────────
while [[ $# -gt 0 ]]; do
    case "$1" in
        --build)      DO_BUILD=true;  shift ;;
        --no-build)   DO_BUILD=false; shift ;;
        --filter)     FILTER="$2";    shift 2 ;;
        --label|-l)   LABEL="$2";     shift 2 ;;
        --jobs|-j)    JOBS="$2";      shift 2 ;;
        --eta)        SHOW_ETA=true;  shift ;;
        --verbose|-v) VERBOSE="--verbose"; shift ;;
        --cuda)       BUILD_DIR="$ENGINE_DIR/build_cuda"; shift ;;
        --debug)      CONFIG="Debug"; shift ;;
        -h|--help)
            sed -n '2,14p' "$0"
            exit 0
            ;;
        *)
            echo "${RED}Unknown option: $1${RESET}" >&2
            exit 1
            ;;
    esac
done

# ── Verify build directory exists ────────────────────────────────────────
if [[ ! -d "$BUILD_DIR" ]]; then
    echo "${RED}Build directory not found: $BUILD_DIR${RESET}"
    echo "${DIM}Run:  cmake -S engine -B engine/build${RESET}"
    exit 1
fi

# ── Build phase ──────────────────────────────────────────────────────────
if $DO_BUILD; then
    echo ""
    echo "${BOLD}========================================${RESET}"
    echo "${BOLD}  Building engine ($CONFIG)${RESET}"
    echo "${BOLD}========================================${RESET}"
    echo ""

    BUILD_START=$SECONDS
    BUILD_LOG=$(mktemp)
    BUILD_RC=0
    cmake --build "$BUILD_DIR" --config "$CONFIG" > "$BUILD_LOG" 2>&1 || BUILD_RC=$?
    BUILD_ELAPSED=$((SECONDS - BUILD_START))

    if [[ $BUILD_RC -eq 0 ]]; then
        OBJ_COUNT=$(grep -cE '(Building|Compiling)' "$BUILD_LOG" 2>/dev/null || true)
        OBJ_COUNT=${OBJ_COUNT:-0}
        OBJ_COUNT=$(echo "$OBJ_COUNT" | tr -d '[:space:]')
        if [[ "$OBJ_COUNT" -gt 0 ]] 2>/dev/null; then
            echo "  ${DIM}Compiled $OBJ_COUNT source files${RESET}"
        else
            echo "  ${DIM}All targets up to date${RESET}"
        fi
        echo "  ${GREEN}Build succeeded${RESET} ${DIM}(${BUILD_ELAPSED}s)${RESET}"
    else
        echo "  ${RED}Build FAILED${RESET} ${DIM}(${BUILD_ELAPSED}s)${RESET}"
        echo ""
        grep -iE '(error[: ]|fatal)' "$BUILD_LOG" | head -30 | while IFS= read -r eline; do
            echo "  ${RED}$eline${RESET}"
        done
        echo ""
        echo "${DIM}Full log: $BUILD_LOG${RESET}"
        exit 1
    fi
    rm -f "$BUILD_LOG"
    echo ""
fi

# ── Count total tests ────────────────────────────────────────────────────
CTEST_ARGS=(-C "$CONFIG" --output-on-failure --progress)
[[ -n "$FILTER" ]]  && CTEST_ARGS+=(-R "$FILTER")
[[ -n "$LABEL" ]]   && CTEST_ARGS+=(-L "$LABEL")
[[ -n "$JOBS" ]]    && CTEST_ARGS+=(-j "$JOBS")
[[ -n "$VERBOSE" ]] && CTEST_ARGS+=("$VERBOSE")

LIST_ARGS=(-C "$CONFIG" -N)
[[ -n "$FILTER" ]] && LIST_ARGS+=(-R "$FILTER")
[[ -n "$LABEL" ]]  && LIST_ARGS+=(-L "$LABEL")

TOTAL_TESTS=$(cd "$BUILD_DIR" && ctest "${LIST_ARGS[@]}" 2>/dev/null \
    | grep "^Total Tests:" | awk '{print $NF}')

if [[ -z "$TOTAL_TESTS" || "$TOTAL_TESTS" -eq 0 ]]; then
    echo "${RED}No tests found.${RESET}"
    exit 1
fi

LABEL_INFO=""
[[ -n "$LABEL" ]] && LABEL_INFO=" [label: $LABEL]"
[[ -n "$FILTER" ]] && LABEL_INFO=" [filter: $FILTER]"

echo "${BOLD}========================================${RESET}"
echo "${BOLD}  Running $TOTAL_TESTS tests ($CONFIG)$LABEL_INFO${RESET}"
echo "${BOLD}========================================${RESET}"

# Show ETA from CTestCostData.txt if available
COST_FILE="$BUILD_DIR/Testing/Temporary/CTestCostData.txt"
if [[ -f "$COST_FILE" ]]; then
    TOTAL_SEC=$(awk '{sum += $3} END {printf "%.0f", sum}' "$COST_FILE" 2>/dev/null || echo "0")
    if [[ "$TOTAL_SEC" -gt 0 ]]; then
        MINS=$((TOTAL_SEC / 60))
        SECS=$((TOTAL_SEC % 60))
        echo "  ${DIM}ETA (from previous run): ~${MINS}m ${SECS}s${RESET}"
    fi
fi
echo ""

# ── Run ctest with real-time streaming output ────────────────────────────
# Strategy: ctest writes to a temp file via tee. We also save the full
# output for post-run parsing.  The `while read` loop runs in the MAIN
# shell (via process substitution) so variables survive.

PASSED=0
FAILED=0
SKIPPED=0
FAIL_LIST_FILE=$(mktemp)
FULL_LOG=$(mktemp)
COLLECTING_FAIL=""
FAIL_BUF=""
TEST_START=$SECONDS

# Use process substitution so the while loop runs in the current shell
# and variable updates persist.
while IFS= read -r line; do

    # ── Result line: passed ───────────────────────────────────────────
    if [[ "$line" =~ ^[[:space:]]*[0-9]+/[0-9]+.*[[:space:]]Passed[[:space:]] ]]; then
        PASSED=$((PASSED + 1))
        COLLECTING_FAIL=""
        FAIL_BUF=""

        TEST_NAME=$(echo "$line" | sed -E 's|^[[:space:]]*[0-9]+/[0-9]+ Test[[:space:]]+#[0-9]+:[[:space:]]+||; s| \.+.*||')
        TEST_TIME=$(echo "$line" | grep -oE '[0-9]+\.[0-9]+ sec' || true)

        DONE=$((PASSED + FAILED + SKIPPED))
        printf "\r${CLR}  ${GREEN}PASS${RESET}  %-42s ${DIM}%s${RESET}" "$TEST_NAME" "$TEST_TIME"
        printf "  [${GREEN}%d${RESET}/%d" "$PASSED" "$TOTAL_TESTS"
        if [[ $FAILED -gt 0 ]]; then
            printf " ${RED}%dF${RESET}" "$FAILED"
        fi
        printf "]\n"

    # ── Result line: failed / timeout ─────────────────────────────────
    elif [[ "$line" =~ \*\*\*(Failed|Timeout) ]]; then
        FAILED=$((FAILED + 1))

        TEST_NAME=$(echo "$line" | sed -E 's|^[[:space:]]*[0-9]+/[0-9]+ Test[[:space:]]+#[0-9]+:[[:space:]]+||; s| \.+.*||')
        TEST_TIME=$(echo "$line" | grep -oE '[0-9]+\.[0-9]+ sec' || true)

        # Save to fail list file (so we can read after loop)
        echo "$TEST_NAME" >> "$FAIL_LIST_FILE"

        DONE=$((PASSED + FAILED + SKIPPED))
        printf "\r${CLR}  ${RED}FAIL${RESET}  ${BOLD}${RED}%-42s${RESET} ${DIM}%s${RESET}" "$TEST_NAME" "$TEST_TIME"
        printf "  [${GREEN}%d${RESET}/%d ${RED}%dF${RESET}]\n" "$PASSED" "$TOTAL_TESTS" "$FAILED"

        # Print collected failure output
        if [[ -n "$FAIL_BUF" ]]; then
            LINE_COUNT=0
            while IFS= read -r fline; do
                LINE_COUNT=$((LINE_COUNT + 1))
                if [[ $LINE_COUNT -le 25 ]]; then
                    echo "       ${DIM}| $fline${RESET}"
                fi
            done <<< "$FAIL_BUF"
            if [[ $LINE_COUNT -gt 25 ]]; then
                echo "       ${DIM}| ... ($((LINE_COUNT - 25)) more lines)${RESET}"
            fi
        fi

        COLLECTING_FAIL=""
        FAIL_BUF=""

    # ── Result line: skipped / not run ────────────────────────────────
    elif [[ "$line" =~ \*\*\*Not\ Run ]]; then
        SKIPPED=$((SKIPPED + 1))
        COLLECTING_FAIL=""
        FAIL_BUF=""

        TEST_NAME=$(echo "$line" | sed -E 's|^[[:space:]]*[0-9]+/[0-9]+ Test[[:space:]]+#[0-9]+:[[:space:]]+||; s| \.+.*||')
        DONE=$((PASSED + FAILED + SKIPPED))
        printf "\r${CLR}  ${YELLOW}SKIP${RESET}  %-42s  [${GREEN}%d${RESET}/%d]\n" "$TEST_NAME" "$PASSED" "$TOTAL_TESTS"

    # ── Start line: begin collecting potential failure output ──────────
    elif [[ "$line" =~ ^[[:space:]]+Start\ [0-9]+: ]]; then
        COLLECTING_FAIL="yes"
        FAIL_BUF=""

    # ── Between Start and result: buffer for potential failure ─────────
    elif [[ -n "$COLLECTING_FAIL" ]]; then
        if [[ -n "$FAIL_BUF" ]]; then
            FAIL_BUF="${FAIL_BUF}"$'\n'"$line"
        else
            FAIL_BUF="$line"
        fi
    fi

done < <(cd "$BUILD_DIR" && ctest "${CTEST_ARGS[@]}" 2>&1 | tee "$FULL_LOG")

TEST_ELAPSED=$((SECONDS - TEST_START))

# ── Extract ctest timing ─────────────────────────────────────────────────
CTEST_TIME=$(grep -oE 'Total Test time \(real\) = +[0-9.]+' "$FULL_LOG" \
    | grep -oE '[0-9]+\.[0-9]+' 2>/dev/null || echo "?")

# Read failed test names from file
declare -a FAILED_NAMES=()
if [[ -f "$FAIL_LIST_FILE" ]]; then
    while IFS= read -r fname; do
        [[ -n "$fname" ]] && FAILED_NAMES+=("$fname")
    done < "$FAIL_LIST_FILE"
fi

rm -f "$FULL_LOG" "$FAIL_LIST_FILE"

TOTAL=$((PASSED + FAILED + SKIPPED))

# ── Summary ──────────────────────────────────────────────────────────────
echo ""
echo "${BOLD}========================================${RESET}"
echo "${BOLD}  Test Summary${RESET}"
echo "${BOLD}========================================${RESET}"
echo ""

printf "  ${BOLD}%-14s${RESET}  %s\n" "Total:" "$TOTAL"

if [[ "$PASSED" -eq "$TOTAL" && "$TOTAL" -gt 0 ]]; then
    printf "  ${BOLD}%-14s${RESET}  ${GREEN}${BOLD}%s${RESET}\n" "Passed:" "$PASSED"
else
    printf "  ${BOLD}%-14s${RESET}  ${GREEN}%s${RESET}\n" "Passed:" "$PASSED"
fi

if [[ "$FAILED" -gt 0 ]]; then
    printf "  ${BOLD}%-14s${RESET}  ${RED}${BOLD}%s${RESET}\n" "Failed:" "$FAILED"
else
    printf "  ${BOLD}%-14s${RESET}  %s\n" "Failed:" "0"
fi

if [[ "$SKIPPED" -gt 0 ]]; then
    printf "  ${BOLD}%-14s${RESET}  ${YELLOW}%s${RESET}\n" "Skipped:" "$SKIPPED"
else
    printf "  ${BOLD}%-14s${RESET}  %s\n" "Skipped:" "0"
fi

echo ""
printf "  ${BOLD}%-14s${RESET}  %ss (wall: %ds)\n" "Time:" "$CTEST_TIME" "$TEST_ELAPSED"

# ── Failed test list ─────────────────────────────────────────────────────
if [[ ${#FAILED_NAMES[@]} -gt 0 ]]; then
    echo ""
    echo "  ${RED}${BOLD}Failed tests:${RESET}"
    for name in "${FAILED_NAMES[@]}"; do
        echo "    ${RED}x${RESET}  $name"
    done
fi

# ── Final banner ─────────────────────────────────────────────────────────
echo ""
if [[ "$FAILED" -eq 0 && "$TOTAL" -gt 0 ]]; then
    echo "  ${GREEN}${BOLD}ALL $PASSED TESTS PASSED${RESET}"
else
    echo "  ${RED}${BOLD}$FAILED OF $TOTAL TEST(S) FAILED${RESET}"
fi
echo ""

# Exit code: 0 if all passed, 1 if any failed
if [[ "$FAILED" -gt 0 ]]; then
    exit 1
fi
exit 0
