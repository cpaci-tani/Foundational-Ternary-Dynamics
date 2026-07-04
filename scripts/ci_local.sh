#!/usr/bin/env bash
# =============================================================================
# ci_local.sh — WSL2-side local engine gate (revision 1.1).
#
# Complements scripts/ci_local.ps1 (the Windows-side gate): this one covers
# what only the WSL2 build can — the gcc golden reproduction and the GPU
# label set (CUDA campaigns are WSL2-only per CLAUDE.md).
#
# Usage (inside WSL2 Ubuntu-22.04, from repo root):
#   scripts/ci_local.sh          # build + gcc golden + merge_gate labels
#   scripts/ci_local.sh --gpu    # additionally run ctest -L gpu (RTX 5090)
# =============================================================================
set -euo pipefail

REPO="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
BUILD="$REPO/engine/build_wsl"
RUN_GPU=0
[[ "${1:-}" == "--gpu" ]] && RUN_GPU=1

step() { echo; echo "=== [ci_local.sh] $1 ==="; }

step "configure + build (WSL2 gcc)"
cmake -S "$REPO/engine" -B "$BUILD" >/dev/null
cmake --build "$BUILD" -j 24

step "gcc golden reproduction (per-platform hash policy)"
"$BUILD/test_render_bridge_golden"

# SERIAL ctest on WSL2 is deliberate, not an oversight: every test binary in
# the CUDA build creates a GPU context at RenderBridge construction, and
# concurrent context creation on the WSL2 CUDA stack stalls pathologically
# (measured 2026-07-02: the 0.37s determinism test exceeds its 60s timeout
# under -j 24 AND -j 2; -j 1 runs the whole 6-test bundle in 1.6s).
step "merge_gate ctest bundle (serial — WSL2 CUDA context contention)"
ctest --test-dir "$BUILD" -L merge_gate -j 1 --output-on-failure

if [[ $RUN_GPU -eq 1 ]]; then
    step "GPU label set (RTX 5090, serialized — dedicated device)"
    ctest --test-dir "$BUILD" -L gpu -j 1 --output-on-failure
fi

echo
echo "[ci_local.sh] ALL GREEN"
