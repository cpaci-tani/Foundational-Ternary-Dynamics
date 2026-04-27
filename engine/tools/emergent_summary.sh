#!/bin/bash
# Per-IC manifested-count + cluster-count summary
cd /mnt/c/Users/cpaci/Desktop/ftd
ROOT=engine/results/emergent_spectrum_2026-04-27
for ic in ic1_inject ic2_thermal ic3_collision ic4_paircreate ic5_baryogenesis; do
    echo "=== $ic ==="
    for f in "$ROOT/$ic"/per_snapshot_census_seed*.csv; do
        if [ -f "$f" ]; then
            seed=$(basename "$f" | sed 's/per_snapshot_census_seed//;s/.csv//')
            # Get terminal manifested count + cluster count
            tail -1 "$f" | awk -F, -v seed="$seed" '{print "  seed="seed" terminal manifested="$3" clusters="$6" max_voxels="$7" total_E="$2}'
        fi
    done
done
