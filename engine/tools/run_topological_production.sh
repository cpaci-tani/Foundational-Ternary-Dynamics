#!/bin/bash
# Campaign D production: 4 sub-experiments × 5 seeds at L=32
cd "$(dirname "$(dirname "$(dirname "$(realpath "$0")")")")"
mkdir -p engine/results/topological_observables_2026-04-27
LOG=engine/results/topological_observables_2026-04-27/production_log.txt
echo "[$(date)] Campaign D production starting" > "$LOG"
time ./engine/build_wsl/campaign_topological_observables --L=32 --seeds=5 --samples=40 --burn=200 --stride=50 >> "$LOG" 2>&1
echo "[$(date)] Campaign D production done, exit=$?" >> "$LOG"
