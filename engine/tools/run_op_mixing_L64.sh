#!/bin/bash
# Launch operator-mixing campaign at L=64 with --b4 RG semigroup test
cd /mnt/c/Users/cpaci/Desktop/ftd
mkdir -p engine/results/operator_mixing_2026-04-26
LOG=engine/results/operator_mixing_2026-04-26/L64_b4_log.txt
echo "[$(date)] L=64 --b4 starting" > "$LOG"
time ./engine/build_wsl/campaign_operator_mixing --L=64 --b4 >> "$LOG" 2>&1
echo "[$(date)] L=64 --b4 done, exit=$?" >> "$LOG"
