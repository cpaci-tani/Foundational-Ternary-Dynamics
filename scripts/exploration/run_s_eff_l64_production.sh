#!/bin/bash
# v1.1 L=64 production. Run after L=32 LARGE completes to compare cond(S)
# improvement and reaction-sector entry stderr at the larger lattice.
#
# Expected wall: ~15 min on RTX 5090 (8x volume scaling vs L=32 at same N).
# Expected result:
#   - cond(S) drops by ~10x (per FTD-0099 L=16->32 trend)
#   - Reaction-sector M_ab entries closer to convergence
#   - More entries pass Gate A < 30% stderr
set -e
cd /mnt/c/Users/cpaci/Desktop/ftd

OUT=engine/results/s_eff_nonlinear_2026-04-29/L64_prod_T0.100
echo "=== L=64 production at T_langevin=0.100, pair-rich, 10 seeds x 200 samples ==="
time ./engine/build_wsl/campaign_s_eff_nonlinear \
    --scenario=pair-rich --L=64 --N-seeds=10 --N-samples=200 --N-burn=200 \
    --T-langevin=0.100 --b4 \
    --output-dir=$OUT \
    > $OUT.log 2>&1 || true

echo "  done -- log: $OUT.log"
echo "  M_ab.csv -- $OUT/M_ab.csv"
echo "  rg_semigroup.txt -- $OUT/rg_semigroup.txt"
