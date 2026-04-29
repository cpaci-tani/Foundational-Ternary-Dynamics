#!/bin/bash
# Phase 3: temperature sweep at A=50 to test regime-4 wake-up scaling
set -e
cd /mnt/c/Users/cpaci/Desktop/ftd
mkdir -p engine/results/regime4_phase3
rm -rf engine/results/regime4_phase3/T*
for T in 0.005 0.010 0.020 0.040; do
    echo "=== Phase 3 T=${T} A=50 L=32 10 seeds stride=1 ==="
    ./engine/build_wsl/campaign_amplitude_time_series \
        --L=32 --A=50.0 --T=${T} --seeds=10 --stride=1 --burn=200 --samples=500 \
        --output-dir=engine/results/regime4_phase3/T${T}/ \
        > engine/results/regime4_phase3/T${T}.log 2>&1
    echo "T=${T} done"
done
echo "ALL PHASE 3 COMPLETE"
