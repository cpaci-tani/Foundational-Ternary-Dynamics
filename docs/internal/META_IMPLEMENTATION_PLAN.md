# Implementation Plan: Extreme Scale & Fibonacci Analysis

## 1. Scale Upgrade
- **Grid**: 200x200x200 (8,000,000 voxels).
- **Center**: 100, 100, 100.

## 2. Structural Analysis Module
- **File**: `ternary_matrix/analysis/structure_metrics.py`
- **Method**: Connected Component Labeling (CCL).
- **Target**: Count the number of Matter particles in each connected group.
- **Output**: Histogram of Cluster Sizes.

## 3. The Search for Integers
We are looking for statistical anomalies (peaks) at specific integers:
- **3** (Triad)
- **4** (Tetrad)
- **7**
- **13** (Fibonacci F7)

## 4. Execution
1. Update `config.py`.
2. Update `run_visual_sim.py` to include the new Analysis step.
3. Run for 50 ticks.
4. Print the "Cluster Size Distribution" at the end.
