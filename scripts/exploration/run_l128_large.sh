#!/bin/bash
# L=128 LARGE production: 10 seeds * 2000 samples = 20,000 snapshot pairs
# at T_langevin=0.100, pair-rich, with b=4 measurement.
#
# Expected wall: ~20 hours on RTX 5090 via WSL2.
# Per-snapshot time at L=128 ~= 3.6 sec (8x L=64's 0.45 sec/snap).
# Per-seed: 2000 * 3.6 = 7200 sec = 2 hours.
# Total: 10 seeds * 2 hours + matrix inversion + bootstrap = ~20.5 hours.
#
# Output: engine/results/s_eff_nonlinear_2026-04-29/L128_prod_T0.100_LARGE/
#   M_ab.csv, M_ab_b4.csv, M_ab_stderr.csv, eigenvalues.csv,
#   per_snapshot_moments.csv, rg_semigroup.txt, meta.json, run.log
#
# This is the v1.3 closure run for the FTD-0112 cross-L characterization.
# Resolves the continuum-limit (L → ∞) question: does the L-dependent
# flow stabilize at the L=64 trend or continue drifting?
set -e
cd /mnt/c/Users/cpaci/Desktop/ftd

OUT=engine/results/s_eff_nonlinear_2026-04-29/L128_prod_T0.100_LARGE
echo "=== L=128 LARGE production ==="
echo "Started: $(date)"
echo "Estimated wall: ~20 hours"
time ./engine/build_wsl/campaign_s_eff_nonlinear \
    --scenario=pair-rich --L=128 --N-seeds=10 --N-samples=2000 --N-burn=200 \
    --T-langevin=0.100 --b4 \
    --output-dir=${OUT} 2>&1 | tee ${OUT}_run.log
echo "Finished: $(date)"
