#!/bin/bash
# T_langevin sweep v2: post-graceful-degradation. Now we expect
# reduced-rank inversion to SUCCEED at T >= 0.10 even with evapFlux
# zero-variance, and we push higher T to see if evaporation activates.
set -e
cd /mnt/c/Users/cpaci/Desktop/ftd

mkdir -p engine/results/s_eff_nonlinear_2026-04-29/Tsweep_v2

# Push higher T values to find evaporation activation
for T in 0.100 0.150 0.300 0.500 1.000; do
    echo "=== T=${T} pair-rich L=16 (v2) ==="
    ./engine/build_wsl/campaign_s_eff_nonlinear \
        --scenario=pair-rich --L=16 --N-seeds=3 --N-samples=50 --N-burn=100 \
        --T-langevin=${T} \
        --output-dir=engine/results/s_eff_nonlinear_2026-04-29/Tsweep_v2/T${T}_pair-rich \
        > engine/results/s_eff_nonlinear_2026-04-29/Tsweep_v2/T${T}.log 2>&1 || true
    echo "  done"
done

echo "T SWEEP V2 COMPLETE"
