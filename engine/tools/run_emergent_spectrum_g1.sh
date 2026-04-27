#!/bin/bash
# FTD-0107 G1 follow-up: L=64 emergent-spectrum rerun
cd /mnt/c/Users/cpaci/Desktop/ftd
mkdir -p engine/results/emergent_spectrum_2026-04-27_L64
LOG=engine/results/emergent_spectrum_2026-04-27_L64/production_log.txt
echo "[$(date)] FTD-0107 G1 L=64 starting (pre-reg tag: preregister-emergent-spectrum-g1)" > "$LOG"
time ./engine/build_wsl/campaign_emergent_spectrum --L=64 --seeds=5 --samples=50 --burn=200 --stride=50 --output-dir=engine/results/emergent_spectrum_2026-04-27_L64 >> "$LOG" 2>&1
echo "[$(date)] FTD-0107 G1 done, exit=$?" >> "$LOG"
