#!/bin/bash
set -e
cd /mnt/c/Users/cpaci/Desktop/ftd/engine/build_wsl
RESULTS=/mnt/c/Users/cpaci/Desktop/ftd/engine/results/gaussian_expansion_2026-04-26
mkdir -p "$RESULTS"

TESTS=(
  test_native_blocking_map
  test_native_flow
  test_native_current_flow
  test_native_response_flow
  test_native_engine_history_flow
  test_native_engine_transport_flow
  test_native_dual_half_shell
  test_native_continuity
  test_native_dual_cell_gauss
  test_native_source_response
  test_native_projection_convergence
  test_native_source_core_fork
  test_native_moore_layer_coupling
  test_native_moore_temporal_layers
  test_native_moore_shell_gauss
  test_native_reaction_ledger
  test_native_manifestation_ledger
  test_native_conserved_parent
  test_eft_blocking
  test_mixed_history_flow
)

for T in "${TESTS[@]}"; do
  echo "=== $T ==="
  START=$SECONDS
  if ./"$T" > "$RESULTS/$T.log" 2>&1; then
    DUR=$((SECONDS-START))
    echo "PASS  $T  (${DUR}s)"
  else
    DUR=$((SECONDS-START))
    echo "FAIL  $T  (${DUR}s)"
    tail -8 "$RESULTS/$T.log"
  fi
done
echo "--- DONE ---"
