# Theorem Boundary — Ten-Source Pair-Distance Capacity

**FTD ID:** FTD-0595  
**Status:** `[THEOREM — TWO-CLASS PAIR-DISTANCE GRAM BOUND]` +
`[COMPUTER-ASSISTED THEOREM — AXIAL-EDGE CAPACITY THROUGH N=9]` +
`[NUMERICAL FACT — FOUR EXHAUSTIVE SHELL-KERNEL SCANS]` +
`[INCONCLUSIVE — N=10]` +
`[CLOSED NEGATIVE — TWO-CLASS PAIR-DISTANCE DECIDER]`  
**Date:** 2026-07-26  
**Verdict:** `TEN_SOURCE_PAIR_DISTANCE_BOUND_INCONCLUSIVE`

## 1. Shared-shell displacement kernel

Retain the exact FTD-0594 stencil-eigenvalue shells `S` and write

\[
 K_S(d)=\sum_{O\subset S}w_O\chi_O(d),\qquad
 \kappa_L(d)=\frac{1}{W_L}\sum_S|K_S(d)|.
\]

For the six axial nearest-neighbor displacements `A_L`, cubic covariance gives
one value `kappa_1`. Define `kappa_2` as the maximum over every other nonzero
displacement. Exhaustive displacement-orbit evaluation gives

| `L` | `kappa_1` | `kappa_2` | maximizing nonaxial `d` |
|---:|---:|---:|---:|
| 9 | 0.35925757426614324 | 0.35393581076466141 | `(0,0,4)` |
| 17 | 0.36250597734262191 | 0.36158615734779392 | `(8,8,8)` |
| 33 | 0.36267617904631827 | 0.35884800190208921 | `(16,16,16)` |
| 65 | 0.36273662797281120 | 0.36134496554309592 | `(32,32,32)` |

The largest six-direction covariance residual is `1.63e-14`, and the largest
direct Fourier/shell residual is `2.67e-15`, both below the registered
`5e-13` gate.

## 2. Exact axial-pair capacity

For `r` distinct sites, let `e_L(r)` be the maximum number of unordered pairs
at axial nearest-neighbor displacement. If a maximizing induced axial graph
were disconnected, translate one component along a shortest periodic path
toward another. Until first contact its internal edges are unchanged; at first
contact at least one cross-component edge is added. Hence a maximizer has a
connected representative and connected-set growth is exhaustive.

Canonicalization under translations and all 48 signed coordinate
permutations gives:

| `r` | 1 | 2 | 3 | 4 | 5 | 6 | 7 | 8 | 9 |
|---:|---:|---:|---:|---:|---:|---:|---:|---:|---:|
| canonical animals | 1 | 1 | 2 | 7 | 23 | 112 | 607 | 3,811 | 25,413 |
| `e_L(r)` | 0 | 1 | 2 | 4 | 5 | 7 | 9 | 12 | 13 |

The periodic `L=9` and nonwrapping `L=17` enumerations agree. For `L>=17`, a
connected set of at most nine sites has coordinate span at most eight, so the
`L=17` capacity is the infinite-cubic-lattice capacity and applies at
`L=33,65`.

## 3. Two-class Gram theorem

Among the `r choose 2` removed-source pairs, at most `e_L(r)` are axial.
Applying `kappa_1` to those pairs and `kappa_2` to the remainder yields

\[
 G_L(r)=r+2\left[e_L(r)\kappa_{1,L}
 +\left({r\choose2}-e_L(r)\right)\kappa_{2,L}\right].
\]

Therefore, for `0<=r<=9`,

\[
 \boxed{H_L^{\rm pair}(10,r)
 =C_L\sqrt{10-r}+Q_L\sqrt{G_L(r)}}.
\]

This is no weaker than FTD-0594 because every pair coefficient is bounded by
the FTD-0594 global maximum. The already-subcritical `r=10` FTD-0594 partition
is retained unchanged.

## 4. Evaluation and boundary

| `L` | maximizing `r` | pair-distance bound | `K_GENESIS-bound` |
|---:|---:|---:|---:|
| 9 | 9 | 1.5596603322901916 | -0.043274273138213859 |
| 17 | 9 | 1.5926213057370728 | -0.076235246585095062 |
| 33 | 9 | 1.6030014362387295 | -0.086615377086751710 |
| 65 | 9 | 1.6115888533818610 | -0.095202794229883203 |

All four registered upper bounds remain super-threshold. FTD-0595 therefore
does not close `N=10` and does not construct a genesis history. It proves only
that axial-pair capacity, with all other distances collapsed into one class,
is insufficient as a uniform decider.

## 5. Verification

- preregistration SHA-256:
  `3652D216C915389CD1838CA453C6B0A42F47D748771A9C5D3A1AF23BEEA5AB96`;
- 25,413 canonical size-nine animals independently generated in C++ and
  Python for both registered capacity quotients;
- every animal count, edge cap, exact shell kernel, Gram factor, and partition
  bound independently reconstructed: 258/258 PASS;
- no source history, polarity, schedule, observation time, threshold-selected
  shape, third distance class, toggle, scenario, or production change.
