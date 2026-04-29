#!/bin/bash
# Phase 7: fine-grained amplitude scan to localize the regime-4 activation
# threshold (or window) at T=0.020.
#
# Discriminates three competing readings of the Phase 4 U-shape:
#   (a) sharp threshold above some A* — only A=50 happens to be just past it
#   (b) active window A ∈ [A_low, A_high] — A=50 in window, A=80 past it
#   (c) genuine peak at A=50 — non-monotone with maximum here
#
# Runs A ∈ {40, 45, 55, 60, 65, 70} at T=0.020. Combined with existing
# A=30, A=50, A=80 data this gives an 8-point scan across the candidate
# threshold/window region.
set -e
cd /mnt/c/Users/cpaci/Desktop/ftd
mkdir -p engine/results/regime4_phase7

for A in 40 45 55 60 65 70; do
    echo "=== Phase 7 A=${A}.0 T=0.020 L=32 stride=1 ==="
    ./engine/build_wsl/campaign_amplitude_time_series \
        --L=32 --A=${A}.0 --T=0.020 --seeds=10 --stride=1 --burn=200 --samples=500 \
        --output-dir=engine/results/regime4_phase7/A${A}_T0.020/ \
        > engine/results/regime4_phase7/A${A}_T0.020.log 2>&1
    echo "  done"
done
echo "PHASE 7 COMPLETE"
