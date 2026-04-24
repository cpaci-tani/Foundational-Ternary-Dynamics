#!/bin/bash
# Tier-2 Plan B sweep on WSL2/GPU.
# L in {32, 64}, densities {0, 1e-3, 1e-2}, M=16.
# Run from Windows as:
#   wsl.exe -d Ubuntu-22.04 -- bash /mnt/c/Users/cpaci/Desktop/ftd/scripts/exploration/run_tier2_wsl.sh

set -u
cd /mnt/c/Users/cpaci/Desktop/ftd

OUT=scripts/exploration/outputs/manifestation_flow_tier2.json
LOG=scripts/exploration/outputs/manifestation_flow_tier2.log
EXE=engine/build_wsl/benchmark_manifestation_flow_cpu

echo "[" > "$OUT"
first=1
total=$((2 * 3 * 16))
i=0

for L in 32 64; do
  for n in 0 0.001 0.01; do
    for seed_idx in $(seq 0 15); do
      i=$((i+1))
      n_int=$(python3 -c "print(int($n * 1000000))")
      seed=$((L * 1000000 + n_int * 1000 + seed_idx))
      printf "[%d/%d] L=%d n=%s seed_idx=%d ...\n" "$i" "$total" "$L" "$n" "$seed_idx" | tee -a "$LOG"
      row=$("$EXE" --L=$L --density=$n --seed=$seed --settle=200 --gpu 2>&1 | grep -E '^\[\{' | head -1 | sed 's/^\[//;s/\]$//')
      if [ -n "$row" ]; then
        if [ $first -eq 0 ]; then echo "," >> "$OUT"; fi
        echo -n "$row" >> "$OUT"
        first=0
        ratio=$(echo "$row" | python3 -c "import json,sys; d=json.load(sys.stdin); print(d.get('flux_energy_ratio','n/a'), d.get('wall_seconds','n/a'))")
        printf "    -> flux_energy_ratio=%s\n" "$ratio" | tee -a "$LOG"
      else
        echo "    -> FAILED" | tee -a "$LOG"
      fi
    done
  done
done

echo "]" >> "$OUT"
echo "wrote $OUT" | tee -a "$LOG"
