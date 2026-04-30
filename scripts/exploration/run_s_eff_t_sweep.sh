#!/bin/bash
# T_langevin sweep over the pair-rich scenario at L=16 to find where
# reaction-sector operators activate.
#
# Output: engine/results/s_eff_nonlinear_2026-04-29/Tsweep/T<T>_<scenario>/
set -e
cd /mnt/c/Users/cpaci/Desktop/ftd

mkdir -p engine/results/s_eff_nonlinear_2026-04-29/Tsweep

for T in 0.005 0.020 0.050 0.100 0.200; do
    echo "=== T=${T} pair-rich L=16 ==="
    ./engine/build_wsl/campaign_s_eff_nonlinear \
        --scenario=pair-rich --L=16 --N-seeds=2 --N-samples=30 --N-burn=100 \
        --T-langevin=${T} \
        --output-dir=engine/results/s_eff_nonlinear_2026-04-29/Tsweep/T${T}_pair-rich \
        > engine/results/s_eff_nonlinear_2026-04-29/Tsweep/T${T}.log 2>&1 || true
    echo "  done"
done

echo "=== periodic injection variant: T=0.005 inject every 5 ticks ==="
./engine/build_wsl/campaign_s_eff_nonlinear \
    --scenario=pair-rich --L=16 --N-seeds=2 --N-samples=30 --N-burn=100 \
    --T-langevin=0.005 --inject-period=5 --inject-amp=1.5 \
    --output-dir=engine/results/s_eff_nonlinear_2026-04-29/Tsweep/T0.005_pair-rich_inject5 \
    > engine/results/s_eff_nonlinear_2026-04-29/Tsweep/inject5.log 2>&1 || true
echo "  done"

echo "=== genesis-rich periodic injection T=0.020 inject every 5 ticks ==="
./engine/build_wsl/campaign_s_eff_nonlinear \
    --scenario=genesis-rich --L=16 --N-seeds=2 --N-samples=30 --N-burn=100 \
    --T-langevin=0.020 --inject-period=5 --inject-amp=2.0 \
    --output-dir=engine/results/s_eff_nonlinear_2026-04-29/Tsweep/T0.020_genesis_inject5 \
    > engine/results/s_eff_nonlinear_2026-04-29/Tsweep/T0.020_genesis_inject5.log 2>&1 || true
echo "  done"

echo "T SWEEP COMPLETE"
