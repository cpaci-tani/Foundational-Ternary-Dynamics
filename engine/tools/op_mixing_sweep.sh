#!/bin/bash
# F2 sweep — try to break the s² zero-variance degeneracy
cd /mnt/c/Users/cpaci/Desktop/ftd

CONFIGS=(
    "--burn=10 --samples=20 --seeds=2"
    "--burn=20 --samples=20 --seeds=2"
    "--burn=50 --samples=20 --seeds=2"
    "--inj-mult=1.0 --samples=20 --seeds=2"
    "--inj-mult=2.0 --samples=20 --seeds=2"
    "--inj-mult=5.0 --samples=20 --seeds=2"
    "--inj-mult=10.0 --samples=20 --seeds=2"
    "--lT=0.001 --samples=20 --seeds=2"
    "--lT=0.05  --samples=20 --seeds=2"
    "--lT=0.5   --samples=20 --seeds=2"
    "--lT=2.0   --samples=20 --seeds=2"
    "--inj-mult=0.5 --samples=20 --seeds=2"
    "--inj-mult=0.1 --samples=20 --seeds=2"
    "--burn=5  --inj-mult=10.0 --samples=20 --seeds=2"
    "--burn=2  --inj-mult=10.0 --samples=20 --seeds=2"
    "--burn=0  --inj-mult=10.0 --samples=20 --seeds=2"
)

for cfg in "${CONFIGS[@]}"; do
    echo "=== $cfg ==="
    ./engine/build_wsl/campaign_operator_mixing $cfg 2>&1 | \
        grep -E "stateSq.*Var|drop stateSq|nonzero state|cond\(S\)|with non-zero state" | head -4
done
