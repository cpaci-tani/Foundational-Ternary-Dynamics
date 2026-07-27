# Theorem Boundary — Ten-Source Removal-Time Orbit Coherence

**FTD ID:** FTD-0593  
**Status:** `[THEOREM — INHERITED UNIFORM ORBIT INEQUALITY]` +
`[NUMERICAL FACT — EXHAUSTIVE FOUR-VOLUME PARTITION MAXIMA]` +
`[INCONCLUSIVE — TEN-SOURCE THRESHOLD]` + `[OPEN — N >= 10]`  
**Date:** 2026-07-26  
**Verdict:** `TEN_SOURCE_ORBIT_BOUND_INCONCLUSIVE`

## 1. Scope

Adopt the frozen FTD-0590 sector and theorem with ten distinct, stationary,
initially present ternary sources at arbitrary sites and with arbitrary signs.
Each source may be removed once at an arbitrary integer tick. The claim is
uniform before a hypothetical first descendant event on
`L={9,17,33,65}`.

No geometry, polarity, removal schedule, observation tick, or observation
site is selected or searched.

## 2. Registered inequality

FTD-0590 proves

\[
 |J(x,n)|\le H_L^{\rm orb}(N,r)
 =C_L\sqrt{N-r}
 +Q_L\sqrt{r+\mu_Lr(r-1)}.
\]

FTD-0593 evaluates only

\[
 H_L^{(10)}=\max_{0\le r\le10}
 \left[C_L\sqrt{10-r}
 +Q_L\sqrt{r+\mu_Lr(r-1)}\right].
\]

## 3. Exhaustive evaluation

All eleven integer partitions were evaluated on each registered quotient.

| `L` | maximizing `r` | `H_L^(10)` | `K_GENESIS-H_L^(10)` |
|---:|---:|---:|---:|
| 9 | 9 | 1.5663934397666830 | -0.050007380614705221 |
| 17 | 9 | 1.5933956191964316 | -0.077009560044453806 |
| 33 | 9 | 1.6062513990021401 | -0.089865339850162318 |
| 65 | 9 | 1.6127738812210539 | -0.096387822069076146 |

The upper bound exceeds `K_GENESIS=1.5163860591519780` on every registered
quotient. Therefore the FTD-0590 orbit inequality does not close `N=10`.

## 4. Logical consequence

The result is not a counterexample to the inequality and not evidence that a
ten-source history attains genesis. The inequality enlarged each source's
normalized pulse factor independently on every cubic orbit and then used a
sector triangle inequality. A super-threshold upper bound establishes only
that this relaxation lacks the capacity to decide the ten-source question.

The first-event theorem remains closed through `N<=9` by FTD-0592. `N=10`
remains open.

## 5. Integrity and next admissible refinement

- preregistration SHA-256:
  `10EBAFCC24B0589B975BD14E3CD4FD4508942830EA7A4FB541378655F25DC348`;
- all 44 registered `(L,r)` partitions evaluated;
- independent Python reconstruction: 130/130 PASS;
- no geometry or schedule search;
- no production/default/toggle/scenario change.

The preregistered next branch is an exact grouping of all cubic orbits that
share the same stencil eigenvalue `M`. It requires a new locked protocol. No
tolerance or coefficient may be altered inside FTD-0593.
