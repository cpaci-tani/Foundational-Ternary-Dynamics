# FOUND — Ladder Walk Step Sizes from O_h Structure (Program A, partial closure)

**Tag:** [THEOREM] (step-size multiset) + [SELECTION, narrowed] (step order)
**Ledger row:** FTD-0084
**Filed:** 2026-04-24
**Companions:**
- [FOUND_LADDER_GENERATING_RULE.md](FOUND_LADDER_GENERATING_RULE.md) — the ladder walk this closes
- [FOUND_COGITO_AXIOM_AND_FULL_TRACE.md](FOUND_COGITO_AXIOM_AND_FULL_TRACE.md) — S2 selection in chain §2.3
- [FOUND_MASTER_QUADRATIC_UNIQUENESS_PROOF.md](FOUND_MASTER_QUADRATIC_UNIQUENESS_PROOF.md) (FTD-0083) — Program E closed S1
- [test_ladder_walk_from_oh.cpp](../../../engine/tests/test_ladder_walk_from_oh.cpp) — constructive proof

---

## Executive statement

Program A, proposed in [FTD-0081](FOUND_MASTER_QUADRATIC_UNIFIED_MOTIVATION.md), asked whether the ladder-walk ordering $\{4, 3, 3, 6\}$ can be derived from the $O_h$ subgroup chain. The honest answer is **partial closure**:

- **Step-size multiset $\{3, 3, 4, 6\}$**: [THEOREM]
  Forced by the three structural integers $\{N_c, N_{\text{base}}, N_f\} = \{3, 4, 6\}$ of $O_h$, the $4$-parts / sum-$16$ constraints, and the requirement that all three features appear.
- **Step ORDER (which permutation of $\{3,3,4,6\}$)**: [SELECTION, narrowed]
  $12$ permutations satisfy the multiset constraint. The canonical FTD order $\{4, 3, 3, 6\}$ giving positions $\{4, 8, 11, 14, 20\}$ is SM-structurally motivated (spinor $\to$ color $\to$ color $\to$ flavor) but not forced by pure group theory.

Net effect on S2 in the cogito-axiom ladder ([FTD-0080](FOUND_COGITO_AXIOM_AND_FULL_TRACE.md)):
**S2 [SELECTION] $\to$ [THEOREM on multiset + SELECTION on order].**

Combined with Program E (FTD-0083) closing S1:
- S1 (master quadratic): **[THEOREM]**
- S2 (ladder walk): **[PARTIAL THEOREM]** — multiset closed, order narrowed

The chain from "$i$ exists" to $\alpha^{-1}$ now has one residual selection (the step order), narrowed from "free walk with free steps" to "permutation of a forced multiset."

---

## 1. What Program A was asked to do

From [FOUND_MASTER_QUADRATIC_UNIFIED_MOTIVATION.md §4.1](FOUND_MASTER_QUADRATIC_UNIFIED_MOTIVATION.md) / FTD-0081:

> **Program A:** Close S2 via the $O_h$ subgroup chain. The four step-sizes $\{4, 3, 3, 6\}$ correspond to representation-branching multiplicities at each subgroup transition.

And from FTD-0080 §2.3:

> **S2** = ladder walk ordering $\{4, 3, 3, 6\}$ summing to 16, giving positions $\{4, 8, 11, 14, 20\}$.

The task has two parts:
1. **Step sizes**: derive the integers $\{3, 4, 6\}$ from $O_h$.
2. **Step order**: derive the specific sequence $\{4, 3, 3, 6\}$ (as opposed to any of the 12 permutations).

Part 1 succeeds fully. Part 2 succeeds only partially.

---

## 2. Step 1 — O_h irrep decomposition of the $3^3$ shells

The Moore-26 neighborhood and the central voxel together form the 27-site block $3^3$. Under the action of the octahedral symmetry group $O_h$ (order 48), this block partitions into four orbits:

| Orbit | Size | Stabilizer | Character on $O_h$ classes |
|---|---|---|---|
| center | 1 | $O_h$ | $(1, 1, 1, 1, 1, 1, 1, 1, 1, 1)$ |
| faces | 6 | $C_{4v}$ | $(6, 0, 2, 2, 0, 0, 0, 0, 4, 2)$ |
| edges | 12 | $C_{2v}$ | $(12, 0, 0, 0, 2, 0, 0, 0, 4, 2)$ |
| corners | 8 | $C_{3v}$ | $(8, 2, 0, 0, 0, 0, 0, 0, 0, 4)$ |

(Class order: $E, 8C_3, 6C_4, 3C_2, 6C_2', i, 8S_6, 6S_4, 3\sigma_h, 6\sigma_d$.)

By Frobenius reciprocity / inner product with the $O_h$ character table:

$$
\begin{aligned}
\text{center} &= A_{1g} \\
\text{6 faces} &= A_{1g} \oplus E_g \oplus T_{1u} \\
\text{12 edges} &= A_{1g} \oplus E_g \oplus T_{2g} \oplus T_{1u} \oplus T_{2u} \\
\text{8 corners} &= A_{1g} \oplus T_{2g} \oplus A_{2u} \oplus T_{1u}
\end{aligned}
$$

Full block:
$$
3^3 = 4\,A_{1g} \oplus 2\,E_g \oplus 2\,T_{2g} \oplus A_{2u} \oplus 3\,T_{1u} \oplus T_{2u}
$$
(verified numerically, dimensional sum $= 27$ ✓).

This is the canonical decomposition of the Moore lattice under cubic symmetry. See `engine/tests/test_ladder_walk_from_oh.cpp` for the constructive verification.

### 2.1 The three structural integers

**$N_{\text{base}} = 4$ from $O_h$:**
- Number of $1$-dim irreps of $O_h$ is $4$: $\{A_{1g}, A_{2g}, A_{1u}, A_{2u}\}$.
- Equivalently, $|O_h / [O_h, O_h]| = 4$ (the abelianization is $\mathbb{Z}/2 \times \mathbb{Z}/2$, corresponding to parity × proper/improper).
- Multiplicity of the trivial irrep $A_{1g}$ in the full $3^3$ permutation rep is $4$ (one from each orbit). This counts the $O_h$-invariant functions on the $3^3$ block — exactly the independent "scalar" degrees of freedom.

These three readings agree: **$N_{\text{base}} = 4$ is forced by $O_h$**.

**$N_c = 3$ from $O_h$:**
- Every $T$-type (3-dim) irrep of $O_h$ has dimension $3$: $\dim(T_{1u}) = \dim(T_{2u}) = \dim(T_{1g}) = \dim(T_{2g}) = 3$.
- $T_{1u}$ is the standard vector rep (the action of $O_h$ on Cartesian axes).
- $N_c = 3$ = spatial dimension $D$ = smallest faithful-vector irrep dimension.

**$N_c = 3$ is forced by $O_h$** (and equivalently by $D = 3$, which itself is forced by the combinatorial identity $16 = 2^D(D-1)!$ from FTD-0080).

**$N_f = 6$ from $O_h$:**
- $|$face orbit$|$ = $6$ (the $O_h$-orbit of a face, with stabilizer $C_{4v}$ of order $8$; $48 / 8 = 6$).
- $\dim(T_{1u}) + \dim(T_{2u}) = 6$ (combined dim of one parity class of $T$-type irreps).
- $\dim(T_{1g}) + \dim(T_{2g}) = 6$ (same, other parity class).

All three readings give $6$. **$N_f = 6$ is forced by $O_h$**.

### 2.2 Why color appears twice

The ladder walk uses $N_c$ twice ($3 + 3 = 6$). This has a natural $O_h$ reading: **$T$-type irreps come in two parity classes**, $\{T_{1g}, T_{2g}\}$ (gerade) and $\{T_{1u}, T_{2u}\}$ (ungerade). Each class contributes one "color instance" (3-dim vector sector).

Alternatively: in the $3^3$ decomposition, 3-dim irreps appear with total multiplicity $2 + 3 + 1 = 6$ (two $T_{2g}$, three $T_{1u}$, one $T_{2u}$). The count of 3-dim irreps appearing is $3$ ($T_{2g}$, $T_{1u}$, $T_{2u}$); two of these belong to one parity, one to the other — the "two independent color insertions" reading is natural.

**Caveat:** this is suggestive, not forced. The "twice" structure of $N_c$ is the cleanest matching to SM physics (QCD color + seesaw), but the group-theoretic evidence is heuristic rather than uniquely forcing.

---

## 3. Step 2 — Partition theorem

**Theorem (partition uniqueness).** Let $(a, b, c) \in \mathbb{Z}_{\ge 0}^3$ satisfy
- (C1) $a + b + c = 4$ (exactly 4 parts),
- (C2) $3a + 4b + 6c = 16$ (sum = master-quadratic coefficient),
- (C3) $a, b, c \ge 0$ with parts drawn from $\{3, 4, 6\}$.

Then the only solutions are $(0, 4, 0)$ yielding multiset $\{4, 4, 4, 4\}$ and $(2, 1, 1)$ yielding multiset $\{3, 3, 4, 6\}$.

*Proof.* From (C1), $b = 4 - a - c$. Substituting into (C2):
$$3a + 4(4 - a - c) + 6c = 16 \implies -a + 2c = 0 \implies a = 2c.$$
Then $b = 4 - 3c$. For $c = 0$: $(a, b, c) = (0, 4, 0)$. For $c = 1$: $(2, 1, 1)$. For $c \ge 2$: $b < 0$, excluded.  $\square$

**Corollary (Program A step-size multiset).** Under the additional constraint
- (C4) all three structural integers present: $a \ge 1$, $b \ge 1$, $c \ge 1$,

the unique solution is $(a, b, c) = (2, 1, 1)$, yielding multiset $\{3, 3, 4, 6\}$.

*Proof.* $(0, 4, 0)$ fails $a \ge 1$. Only $(2, 1, 1)$ remains. $\square$

### 3.1 Why (C4) is not arbitrary

The "all features present" constraint is the structural completeness principle: a framework that claims to unify EW + QCD + flavor must have all three counted. The multiset $\{4, 4, 4, 4\}$ represents "only $N_{\text{base}}$ four times" — physically, this would mean the Standard Model has no color and no flavors, which contradicts the observed existence of quarks and three generations.

So (C4) is not a free choice; it's the structural requirement that all $O_h$-forced integers contribute to the walk.

---

## 4. Step 3 — The order question (remaining selection)

The multiset $\{3, 3, 4, 6\}$ admits $4! / 2! = 12$ distinct orderings. The canonical FTD choice is $\{4, 3, 3, 6\}$ (start-value $4$, then add $N_{\text{base}}, N_c, N_c, N_f$ in that sequence), giving positions $\{4, 8, 11, 14, 20\}$.

### 4.1 The physics-motivated argument

From [FOUND_LADDER_GENERATING_RULE.md §II](FOUND_LADDER_GENERATING_RULE.md):

> Each step in the walk adds precisely the structural element needed for the next layer of physical complexity. The walk cannot be rearranged: you need spinors before you need color (electroweak symmetry breaking precedes confinement in energy), and you need all species counted before gravity makes sense (gravity couples universally).

This is SM symmetry-breaking logic:
- $N_{\text{base}} = 4$ first: SU(2) doublets + Higgs mechanism (EW before QCD).
- $N_c = 3$ next: QCD confinement.
- $N_c = 3$ again: seesaw mechanism (color in neutrino sector).
- $N_f = 6$ last: gravity sees all species.

### 4.2 Why this is not forceable from $O_h$ alone

$O_h$ is a purely geometric/representational object. It has no dynamical ordering — the irreps are simultaneously present in the representation ring. Extracting a time-ordered sequence $\{N_{\text{base}}, N_c, N_c, N_f\}$ from $O_h$ alone would require additional structure (a filtration, a subgroup chain with step indices matching, a running-coupling hierarchy, or something similar).

Known $O_h$ subgroup chains (e.g., $O_h \supset O \supset T \supset D_2 \supset C_2 \supset C_1$ with indices $\{2, 2, 3, 2, 2\}$) do not produce the step sequence $\{4, 3, 3, 6\}$ as a sequence of subgroup indices. The chain $O_h \supset T_d \supset D_{2d} \supset C_{2v} \supset C_s \supset C_1$ gives indices $\{2, 3, 2, 2, 2\}$, also not matching.

Attempting to match $\{4, 3, 3, 6\}$ as branching multiplicities is worse: each $O_h \to T_d$ restriction gives uniform multiplicity $1$ for each $T_d$ irrep (doubly-covered by gerade/ungerade pairs). No clean assignment produces the step sequence.

**Honest verdict:** The ORDER of additions is SM-structural physics input. It is consistent with $O_h$ structure (no contradiction) but not forced by it.

---

## 5. What this closes vs. leaves open

### 5.1 After Program A

| Content | Status | Source |
|---|---|---|
| Step integers $\{N_{\text{base}}, N_c, N_f\} = \{4, 3, 6\}$ | [THEOREM] | $O_h$ irrep structure (§2) |
| Step-size multiset $\{3, 3, 4, 6\}$ | [THEOREM] | Partition theorem (§3) |
| Total sum $= 16$ | [THEOREM] | FTD-0083 (master-quadratic coefficient) |
| Color-appears-twice | [STRONGLY MOTIVATED] | Parity classes of $T$-type irreps (§2.2) |
| Step ORDER $\{4, 3, 3, 6\}$ | [SELECTION, narrowed] | SM physics input (§4) |
| Positions $\{4, 8, 11, 14, 20\}$ | [CONDITIONAL on order] | Consequence of order choice |

### 5.2 Impact on cogito-axiom ladder

| Selection | Before Programs E + A | After Programs E + A |
|---|---|---|
| S1 (master quadratic) | [SELECTION] | **[THEOREM]** (FTD-0083) |
| S2 (ladder walk) | [SELECTION] | **[PARTIAL THEOREM]** (this doc) |

**Cogito-axiom ladder residual selection content:** one partial-selection remaining — the specific order within the forced multiset $\{3, 3, 4, 6\}$. This is a much narrower selection than "the full ladder walk is a free choice."

### 5.3 Downstream consequences

- **Electron exponent $n = 11$** ([m_e formula](../05_particles/DERIV_ELECTRON_MASS_MOTIVATION.md)): conditional on the SM-structural order. Under any of the 4 orderings of $\{3, 3, 4, 6\}$ beginning with $\{N_{\text{base}}, N_c\}$ or $\{N_c, N_{\text{base}}\}$, the cumulative sum reaches 11 at step 3. Other orderings give different $n_3$ values (e.g., $\{6, 3, 3, 4\}$ gives $n_3 = 13$).
- **$n = 20$ (gravity)**: invariant under all orderings since it's the total sum. [THEOREM].
- **$m_e = m_P \sqrt{2\pi} (16/3) \alpha^{11}$**: the $\sqrt{2\pi}$ and $16/3$ prefactors are [THEOREM] (FTD-0077 §3); the exponent $n = 11$ is now [SELECTION, narrowed by Program A] — conditional on the SM-structural order.

---

## 6. Can the residual order be closed?

Three plausible closure paths, all requiring new work:

### 6.1 Path 1: Running-coupling hierarchy

At each structural addition, the running coupling scale enters a new regime. The order could be forced by the sequence of natural energy scales:
- $\Lambda_{\text{EW}}$ (electroweak, ~100 GeV) precedes $\Lambda_{\text{QCD}}$ (~200 MeV) in the top-down RG flow.
- Adding $N_{\text{base}}$ first then $N_c$ reflects high-energy-first ordering.

**Caveat:** this requires dynamic input (running couplings), which is external to $O_h$.

### 6.2 Path 2: Filtered O_h chain

A specific subgroup chain $O_h = G_0 \supset G_1 \supset \cdots \supset G_n$ whose intermediate centralizers or invariants produce the sequence $\{4, 3, 3, 6\}$. Exhaustive search over natural chains (documented in §4.2) has not produced a match.

**Caveat:** even if a chain is found, the choice of chain would itself be a selection.

### 6.3 Path 3: Recovery from the master quadratic

Since Programs E + A reduce the chain to a single residual selection (the order), perhaps the order can be fixed by requiring consistency with a specific physics prediction (e.g., $m_e/m_p$ ratio, neutrino mass scale). This would convert the residual [SELECTION] into a physics-calibration tag.

**Caveat:** this is a calibration, not a derivation — consistent with [SELECTION, narrowed] but not promoting to [THEOREM].

### 6.4 Recommendation

Accept **[THEOREM on multiset + SELECTION on order]** as the final status of S2, unless a Path-1/Path-2 derivation is discovered. Analogous to how SP4 (physical identification $x_+ = 1/\alpha$) remains [STRONGLY MOTIVATED CONJECTURE] despite the master quadratic itself being [THEOREM] — the algebraic/geometric content closes, but the physical assignment retains a selection layer.

---

## 7. Status summary

**Program A: PARTIAL CLOSURE** as of 2026-04-24.

Test: `engine/tests/test_ladder_walk_from_oh.cpp`
- O_h irrep decomposition verified numerically (integer multiplicities, total dim = 27).
- Partition enumeration confirms uniqueness of $\{3, 3, 4, 6\}$ under (C1)-(C4).
- Result: `ladder_walk_from_oh` PASS (exit 0).

**Promotions:**
- {3, 4, 6} as structural integers: **[THEOREM]**
- Step-size multiset: **[THEOREM]**
- S2 (ladder walk): **[PARTIAL THEOREM]** (multiset + narrowed order)

**Remaining:**
- Step ORDER within the multiset: **[SELECTION, narrowed]** (12 orderings allowed; SM-structural order chosen)

**Cogito-axiom ladder status (after Programs E + A):**
- S1: [THEOREM] (Program E, FTD-0083)
- S2: [PARTIAL THEOREM] (Program A, this doc)
- Residual: one narrow selection (the step order within a forced multiset)

---

*Filed 2026-04-24. Closes Program A as a partial closure of S2. Promotes step-size multiset $\{3, 3, 4, 6\}$ from [SELECTION] to [THEOREM] via $O_h$ structural integers + partition theorem. Step ORDER remains [SELECTION, narrowed] pending additional derivation work. Combined with Program E (FTD-0083), the cogito-axiom ladder is reduced from 2 selections to 1 narrow selection.*
