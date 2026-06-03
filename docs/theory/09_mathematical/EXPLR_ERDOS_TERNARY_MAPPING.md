# EXPLR_ERDOS_TERNARY_MAPPING

**Tag:** [EXPLORATION] / [OPEN]
**Date:** 2026-06-02
**Subject:** Mapping the Erdős Cap Set Problem into FTD's Ternary Ontology

## 1. The Erdős Cap Set Problem
The cap set problem, strongly associated with Paul Erdős and combinatorial geometry, asks for the maximum size of a subset $A \subset \mathbb{Z}_3^n$ (or $\mathbb{F}_3^n$) such that no three elements form an arithmetic progression. In $\mathbb{Z}_3^n$, three elements $x, y, z$ form an arithmetic progression if and only if $x + y + z = 0 \pmod 3$.

## 2. FTD Ontological Mapping
FTD operates fundamentally on a ternary substrate where each voxel $s_i \in \{-1, 0, 1\}$. Note that under modulo 3 arithmetic, $\{-1, 0, 1\}$ is isomorphic to $\{2, 0, 1\} = \mathbb{Z}_3$. 
Thus, any $n$-voxel block in FTD corresponds to a vector in $\mathbb{Z}_3^n$.

### 2.1 The Cap Set as a Kinematic Constraint
In FTD, the manifestation field determines allowed spatial configurations. A "Cap Set Constraint" imposes a zero-mode rejection criteria: no three spatial configurations within a dynamically selected active set $A$ can superpose to the void state $0 \pmod 3$.
This is essentially a non-linear geometric scattering constraint:
$x + y + z \neq 0$ for all distinct $x, y, z \in A$.

### 2.2 FTD "Ammunition"
We can use FTD's engine to heuristically search for Cap Sets. By representing potential cap sets as bitmasks or ternary matrices and evolving them through a simulated annealing / evolutionary algorithm parameterized by FTD's `voxel` interactions, we aim to discover novel lower bounds for $n \ge 6$.

## 3. Scope of the Exploration
This mapping opens the door to computational sweeps utilizing GPU-accelerated tensor arithmetic over the Moore neighborhood. The goal is to construct a Python-based search loop `erdos_capset_solver.py` that interfaces with the FTD combinatorial framework to continuously evolve candidate sets toward the theoretical bounds (e.g., Ellenberg-Gijswijt upper bounds).
