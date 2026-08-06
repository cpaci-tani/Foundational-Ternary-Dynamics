# PRE-REGISTRATION — Ten-source pair-distance capacity v1

**Date locked:** 2026-07-26  
**Identifier:** `FTD-0595`  
**Status:** `[PRE-REGISTRATION — LOCKED/PRE-EVALUATION]`  
**Parent:** `FTD-0594`  

## 1. Question

Does enforcing the simultaneous pair-distance consistency of distinct source
sites tighten the exact shared-`M` Gram bound enough to close the ten-source
first-event question?

FTD-0594 still assigns the worst nonzero displacement coherence independently
to every removed-source pair. This protocol distinguishes only the axial
nearest-neighbor displacement orbit from its complement and proves an exact
capacity bound on how many such pairs can coexist in one finite source set.

## 2. Frozen physical sector

Retain the FTD-0594 sector exactly:

- production 18-point wave stencil and native state-gradient source;
- zero initial `J`, wave velocity, and manifested velocity;
- one periodic odd quotient with `L in {9,17,33,65}`;
- ten distinct stationary ternary sources at arbitrary sites and signs;
- every source begins present and may be removed once at an arbitrary integer
  tick;
- no Gauss projection, damping, force, movement, transmutation, collision,
  clock, bath, toggle, scenario, or production modification.

The proof domain ends immediately before a hypothetical first descendant
genesis event.

## 3. Registered displacement kernel

Use the exact FTD-0594 shells and define

\[
 \kappa_L(d)=\frac1{W_L}\sum_S|K_S(d)|,
 \qquad d\ne0.
\]

Let

\[
 A_L=\{(\pm1,0,0),(0,\pm1,0),(0,0,\pm1)\},
\]

with coordinates modulo `L`, and register

\[
 \kappa_{1,L}=\kappa_L(1,0,0),
 \qquad
 \kappa_{2,L}=\max_{d\notin A_L,\ d\ne0}\kappa_L(d).
\]

Cubic covariance must make `kappa` identical on all six members of `A_L`.
The protocol remains valid if `kappa_1=kappa_2`, but requires
`kappa_1>=kappa_2-5e-13`. No additional distance class may be introduced after
evaluation.

## 4. Exact axial-edge capacity

For `r=1,...,9`, let `e_L(r)` be the maximum number of unordered pairs at an
axial nearest-neighbor displacement among `r` distinct sites of `Z_L^3`.

Compute `e_9(r)` and `e_17(r)` by exact connected-set growth:

1. start from the singleton `{(0,0,0)}`;
2. add one unused axial neighbor of any occupied site;
3. canonicalize under all periodic translations and all 48 signed coordinate
   permutations;
4. retain every distinct canonical set at each size through `r=9`;
5. count its induced axial edges exactly.

Record both the complete canonical-animal count and maximum edge count at
every size. A disconnected set cannot maximize axial edges: translate one
connected component along a shortest periodic path until first contact with
another component; internal edges are preserved and at least one edge is
gained. Thus connected growth is exhaustive for the maximum.

For `L>=17`, every connected set of at most nine vertices has a unique lift to
the infinite cubic lattice within coordinate span at most eight. Therefore use
`e_17(r)` also at `L=33,65`. The `L=9` enumeration remains separate so that a
possible length-nine wrap is not assumed away.

The C++ and Python implementations must independently reproduce every animal
count and edge maximum. This is an exact theorem enumeration, not a search for
a threshold-matching source configuration; no maximizing shape is promoted as
a physical history.

## 5. Pair-distance Gram bound

For `0<=r<=9`, every configuration has at most `e_L(r)` axial pairs and all
other pairs have coherence at most `kappa_2`. Hence

\[
\begin{aligned}
 G_L(r)={}&r+2\left[e_L(r)\kappa_{1,L}
 +\left({r\choose2}-e_L(r)\right)\kappa_{2,L}\right],\\
 H_L^{\rm pair}(10,r)={}&C_L\sqrt{10-r}+Q_L\sqrt{G_L(r)}.
\end{aligned}
\]

For `r=10`, retain the already-registered FTD-0594 shared-`M` bound unchanged;
that partition is subcritical on all four volumes before this protocol. The
reported ten-source bound is the maximum over these ten refined partitions
and the unchanged `r=10` partition.

## 6. Registered validity gates

- exact FTD-0594 cyclotomic shell partition and mode coverage;
- exhaustive nonzero displacement-orbit coverage;
- cubic covariance residual `<=5e-13` on `A_L`;
- direct shell-kernel residual `<=5e-13`;
- `kappa_1>=kappa_2-5e-13`;
- exact periodic-animal canonicalization and exhaustive growth through size 9;
- exact C++/Python equality for every animal count and edge maximum;
- independent C++/Python scalar agreement `<=5e-12` absolute;
- pair-distance bound no weaker than FTD-0594 by more than `5e-12`;
- finite bounds and unchanged
  `K_GENESIS=1.5163860591519780`.

All floating sums use compensated summation. Ties use the lexicographically
smallest canonical displacement and smallest `r` only for reporting.

## 7. Prohibited adaptation

No third distance class, source polarity, removal schedule, observation tick,
observation site, threshold-dependent shape selection, approximate key, or
post-evaluation edge cap is permitted in FTD-0595.

## 8. Required artifacts

- `engine/include/ftd/eft/ten_source_pair_distance_capacity.h`;
- `engine/src/eft/ten_source_pair_distance_capacity.cpp`;
- `engine/tests/test_ten_source_pair_distance_capacity.cpp`;
- `scripts/proofs/proof_ten_source_pair_distance_capacity.py`;
- `engine/results/ftd_0595/windows_msvc_cpu.json`;
- `engine/results/ftd_0595/windows_msvc_cpu.csv`;
- theorem, analysis, and adversarial audit records.

## 9. Outcome map

- If all gates pass and every registered partition is strictly subcritical,
  verdict:
  `ARBITRARY_REMOVAL_N_LE_10_CLOSED_BY_PAIR_DISTANCE_CAPACITY`.
- If all gates pass but any partition is not strictly subcritical, verdict:
  `TEN_SOURCE_PAIR_DISTANCE_BOUND_INCONCLUSIVE`.
- If any coverage, covariance, enumeration, or cross-language gate fails,
  verdict: `PROTOCOL_INVALID`.

An inconclusive upper bound is not a genesis witness. Closure is only a
first-event impossibility theorem in the frozen sector.

## 10. Failure consequence

No kernel class or combinatorial cap is changed after evaluation. An
inconclusive result closes this two-class pair-distance branch. Any full
distance-spectrum optimization or temporal phase-feasible-set analysis
requires a new identifier and a new lock.
