#!/bin/bash
# F7 test: search for inj-mult that breaks s² zero-variance at L=32
cd /mnt/c/Users/cpaci/Desktop/ftd

for inj in 2.0 3.0 4.0 5.0 6.0 7.0 8.0; do
    echo "=== L=32 inj=$inj ==="
    ./engine/build_wsl/campaign_operator_mixing --L=32 --inj-mult=$inj --samples=10 --seeds=2 2>&1 \
        | grep -E "collected|with non-zero state|stateSq.*Var|drop stateSq|cond\(S\) =" | head -6
done
