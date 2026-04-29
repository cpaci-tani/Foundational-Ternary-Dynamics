#!/bin/bash
# Phase 4: T sweep at multiple amplitudes (map T*(A) phase boundary)
# Phase 5: A=118 L=80 regime-3 thickening test
set -e
cd /mnt/c/Users/cpaci/Desktop/ftd
mkdir -p engine/results/regime4_phase4 engine/results/regime4_phase5

echo "=========================================="
echo "PHASE 4: Multi-amplitude T sweep at L=32"
echo "=========================================="

# A=20 with T=0.010, 0.020, 0.040 (already have T=0.005 from Phase 2B)
for T in 0.010 0.020 0.040; do
    echo "=== A=20 T=${T} ==="
    ./engine/build_wsl/campaign_amplitude_time_series \
        --L=32 --A=20.0 --T=${T} --seeds=10 --stride=1 --burn=200 --samples=500 \
        --output-dir=engine/results/regime4_phase4/A20_T${T}/ \
        > engine/results/regime4_phase4/A20_T${T}.log 2>&1
    echo "  done"
done

# A=30 with T=0.010, 0.020, 0.040 (already have T=0.005 from Phase 2C)
for T in 0.010 0.020 0.040; do
    echo "=== A=30 T=${T} ==="
    ./engine/build_wsl/campaign_amplitude_time_series \
        --L=32 --A=30.0 --T=${T} --seeds=10 --stride=1 --burn=200 --samples=500 \
        --output-dir=engine/results/regime4_phase4/A30_T${T}/ \
        > engine/results/regime4_phase4/A30_T${T}.log 2>&1
    echo "  done"
done

# A=80 with T=0.005, 0.010, 0.020, 0.040 (need L=64 for cluster ~1600 voxels)
for T in 0.005 0.010 0.020 0.040; do
    echo "=== A=80 T=${T} (L=64 to fit cluster) ==="
    ./engine/build_wsl/campaign_amplitude_time_series \
        --L=64 --A=80.0 --T=${T} --seeds=5 --stride=1 --burn=200 --samples=300 \
        --output-dir=engine/results/regime4_phase4/A80_T${T}_L64/ \
        > engine/results/regime4_phase4/A80_T${T}_L64.log 2>&1
    echo "  done"
done

echo "PHASE 4 COMPLETE"

echo "=========================================="
echo "PHASE 5: A=118 L=80 regime-3 thickening test"
echo "=========================================="

# Tau-equivalent amplitude at L=80 with stride=1 to test regime-3 σ_within
echo "=== A=117.93 L=80 T=0.005 (regime-3 large-N) ==="
./engine/build_wsl/campaign_amplitude_time_series \
    --L=80 --A=117.93 --T=0.005 --seeds=5 --stride=1 --burn=300 --samples=300 \
    --output-dir=engine/results/regime4_phase5/tau_T0.005/ \
    > engine/results/regime4_phase5/tau_T0.005.log 2>&1
echo "  done"

# Also at higher T
echo "=== A=117.93 L=80 T=0.020 ==="
./engine/build_wsl/campaign_amplitude_time_series \
    --L=80 --A=117.93 --T=0.020 --seeds=5 --stride=1 --burn=300 --samples=300 \
    --output-dir=engine/results/regime4_phase5/tau_T0.020/ \
    > engine/results/regime4_phase5/tau_T0.020.log 2>&1
echo "  done"

echo "ALL PHASES 4+5 COMPLETE"
