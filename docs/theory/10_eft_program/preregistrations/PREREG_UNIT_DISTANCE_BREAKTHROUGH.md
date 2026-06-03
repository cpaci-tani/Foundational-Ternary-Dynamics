# PREREG_UNIT_DISTANCE_BREAKTHROUGH

**Tag:** [PRE-REGISTRATION]
**Date:** 2026-06-02
**Subject:** Computational Search for High-Density Unit Distance Graphs

## 1. Hypothesis
The FTD discrete engine, when configured to process algebraic coordinates, can execute a parallelized search that constructs unit distance graphs with $u(n) > C \cdot n$ for specific values of $n$, pushing towards or verifying the super-linear bounds established by recent AI mathematical breakthroughs.

## 2. Search Parameters
- **Dimensions:** 2D Euclidean plane embedded via algebraic extensions $\mathbb{Z}[\sqrt{2}, \sqrt{3}]$.
- **Search Space:** $N$ candidate points arranged in a dense grid or cyclotomic structure.
- **Optimization Metric:** Maximize the number of edges (unit distances) for a subgraph of size $n$.

## 3. Methodology
- **Generation:** Construct a base set of 10,000 points where many points share exact unit distances. This relies on selecting points $(a+b\sqrt{2}, c+d\sqrt{2})$ such that $(a-a')^2 + ... = 1$.
- **C++ Solver:** The C++ engine (`test_erdos_unit_distance.cpp`) will read this generated graph and attempt to extract sub-graphs of size $n$ that maximize the edge count $u(n)$.
- **Algorithm:** Randomized Greedy + Simulated Annealing over the vertex set.

## 4. Expected Outcomes
- **Outcome A (FOUND):** The solver successfully constructs sets where $u(n)$ is significantly super-linear (e.g., explicitly exceeding $n \log n$ heuristics for small $n$).
- **Outcome B (UNDERDETERMINED):** The solver gets stuck in local maxima typical of hypercube/grid subsets, reproducing known standard lower bounds without breaking into novel density regimes.

## 5. Verification
Results are automatically verified by computing Euclidean distance in double precision and confirming $|d - 1.0| < 10^{-12}$.
