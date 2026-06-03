# PREREG_ERDOS_BREAKTHROUGH_v1

**Tag:** [PRE-REGISTRATION]
**Date:** 2026-06-02
**Subject:** Computational Search for Maximal Cap Sets in $\mathbb{Z}_3^n$ using FTD Heuristics

## 1. Hypothesis
The FTD discrete manifestation framework, specifically modeled via simulated annealing or genetic algorithms over ternary states, can discover maximal or near-maximal Cap Sets (sets with no 3-term arithmetic progressions) in high-dimensional spaces ($n \ge 5$) more efficiently than random sampling.

## 2. Search Parameters
- **Dimensions ($n$):** Primary targets are $n=4, 5, 6$.
- **Alphabet:** $\{-1, 0, 1\}$ mapped modulo 3 to $\mathbb{Z}_3$.
- **Constraint:** For any three distinct vectors $v_1, v_2, v_3 \in A$, we require $v_1 + v_2 + v_3 \neq 0 \pmod 3$.
- **Optimization Metric:** Maximize $|A|$.

## 3. Methodology
We will implement an automated Python script `erdos_capset_solver.py` that utilizes a stochastic optimization algorithm (simulated annealing / hill climbing) to build and mutate candidate sets.
- **Initial State:** A randomly generated valid cap set of small size.
- **Mutations:** Adding valid vectors, swapping vectors, or deleting and rebuilding subsets.
- **Stopping Condition:** Stagnation after a defined number of generations ($10^4$ iterations per dimension run), or matching the known maximums for $n \le 5$.

## 4. Expected Outcomes
- **Outcome A (FOUND):** The solver successfully recovers known maximum cap sets for $n \le 5$ (e.g., $|A|=9$ for $n=2$, $|A|=20$ for $n=3$, $|A|=45$ for $n=4$, $|A|=112$ for $n=5$), demonstrating that the search algorithm is capable and functional.
- **Outcome B (UNDERDETERMINED):** The solver finds large cap sets but fails to reach the known maximums, indicating the heuristic requires deeper mathematical integration with FTD's Master Quadratic or Moore dynamics.
- **Outcome C (CLOSED NEGATIVE):** The solver is structurally incapable of efficiently searching the space, falling back to random performance.

## 5. Verification
Results will be strictly checked by an independent $O(|A|^3)$ verification function to ensure no false positives exist in the final reported sets.
