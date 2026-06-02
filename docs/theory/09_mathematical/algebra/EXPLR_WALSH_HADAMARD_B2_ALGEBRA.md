# Exploration: Walsh–Hadamard Grading of the b=2 Block vs. Cl(3,0)

**Date:** 2026-04-24
**Status:** [CONJECTURE] with [THEOREM] no-go for the spontaneous version
**Supersedes:** fermion-emergence claim in "Generated Document April 24, 2026 – 12:47 AM.pdf" (§1, labelled there as THEOREM 1)
**Depends on:** [THEOREM_MOORE_LAYER_DECOMPOSITION.md](../08_structural/THEOREM_MOORE_LAYER_DECOMPOSITION.md), [FOUND_AXIOM_ZERO.md](../02_foundations/FOUND_AXIOM_ZERO.md)
**Ledger row:** FTD-0061

---

## 1. The claim being audited

A 2×2×2 block contains $2^3 = 8$ ternary voxels $s(x,y,z) \in \{-1,0,+1\}$. The discrete Walsh–Hadamard transform decomposes a function on this block into 8 basis functions indexed by $v = (v_1,v_2,v_3) \in (\mathbb{Z}/2)^3$:

$$ \chi_v(x) = (-1)^{v \cdot x}, \quad x \in (\mathbb{Z}/2)^3. $$

Grouped by the Hamming weight $|v|$ the 8 basis functions split as

$$ 8 = \underbrace{1}_{|v|=0 \text{ (scalar)}} + \underbrace{3}_{|v|=1 \text{ (1-form)}} + \underbrace{3}_{|v|=2 \text{ (2-form)}} + \underbrace{1}_{|v|=3 \text{ (3-form)}}. $$

The original claim is that this 1+3+3+1 split **is** the Clifford algebra $\mathrm{Cl}(3,0)$ and therefore that "fermions are topological defects of the b=2 block" and "flux transitions on these sub-states are the Dirac $\gamma$-matrices".

## 2. What the b=2 block actually carries: a no-go result

The pointwise product of Walsh–Hadamard characters is

$$ \chi_u \cdot \chi_v = \chi_{u \oplus v}, $$

where $\oplus$ is bitwise XOR (addition in $(\mathbb{Z}/2)^3$). Under this product, the span of the 8 characters is the **group algebra**

$$ \mathbb{R}[(\mathbb{Z}/2)^3] \cong \mathbb{R}^8 \text{ (as rings)}. $$

This is **abelian**. $\chi_u \chi_v = \chi_v \chi_u$ for all $u, v$.

Compare $\mathrm{Cl}(3,0)$. As a vector space it is also 8-dimensional with basis $\{1, e_1, e_2, e_3, e_1 e_2, e_1 e_3, e_2 e_3, e_1 e_2 e_3\}$. As a ring,

$$ \mathrm{Cl}(3,0) \cong M_2(\mathbb{R}) \oplus M_2(\mathbb{R}). $$

This is **non-abelian**: $e_i e_j = -e_j e_i$ for $i \ne j$, $e_i^2 = +1$.

**[THEOREM] (no-go for spontaneous Clifford structure on the b=2 block).**

The 8-dimensional Walsh–Hadamard algebra $\mathbb{R}[(\mathbb{Z}/2)^3]$ is not ring-isomorphic to $\mathrm{Cl}(3,0)$. In particular, no permutation of the 8 Walsh–Hadamard characters and no re-identification of the pointwise product can realise the Clifford anticommutation relation $\{e_i, e_j\} = 2\delta_{ij}$.

*Proof.* $\mathbb{R}[(\mathbb{Z}/2)^3] \cong \mathbb{R}^8$ is commutative. $\mathrm{Cl}(3,0)$ contains the 2×2 real matrix ring as a direct summand and is therefore not commutative. Non-isomorphic rings. $\square$

So the sentence "the 1+3+3+1 structure of the b=2 block natively instantiates the Clifford algebra Cl(3,0)" is literally false **if the algebra is the one that comes for free with the block**. Any Clifford structure has to be introduced by **additional dynamics**, not by the block decomposition alone.

## 3. What does match

Two things coincide between the b=2 block and $\mathrm{Cl}(3,0)$ and are worth recording honestly:

1. **Dimension.** Both are 8-dimensional real vector spaces.
2. **Grading count.** The Hamming-weight grading of $(\mathbb{Z}/2)^3$ yields 1+3+3+1, which coincides with the grade decomposition of $\mathrm{Cl}(3,0)$ by generator count.

These are genuine, non-trivial matches. They are also insufficient. Every 8-dimensional real vector space is abstractly 8-dimensional; the grading-count match reflects the binomial identity $\binom{3}{0} + \binom{3}{1} + \binom{3}{2} + \binom{3}{3} = 8$, which is shared by any rank-3 boolean structure. The match is suggestive of a search; it is not itself a derivation.

## 4. Candidate ways to promote the conjecture to a theorem

For Clifford structure to genuinely emerge from FTD dynamics on a b=2 block, at least the following must be supplied:

**G1. Three generators named explicitly.** Identify three distinct operators $\hat e_1, \hat e_2, \hat e_3$ acting on the 8-dim Walsh–Hadamard space from native FTD dynamics (not from an imposed choice).

**G2. A non-abelian product.** Derive a product $\star$ on the 8-dim space from the engine's update rule such that $\hat e_i \star \hat e_j \ne \hat e_j \star \hat e_i$ for $i \ne j$. Candidates: composition of flux-rotation operators across the three coordinate planes, or the action of Moore-stencil phase shifts on Walsh modes.

**G3. The anticommutator.** Compute $\{\hat e_i, \hat e_j\} := \hat e_i \star \hat e_j + \hat e_j \star \hat e_i$ and show it equals $2\delta_{ij} \mathbb{1}$ on this 8-dim space — without choosing coefficients to make it so.

**G4. Signature and the step to Dirac.** The Dirac algebra is $\mathrm{Cl}(1,3)$ (real 16-dim) or $\mathrm{Cl}(3,1)$. A Euclidean $\mathrm{Cl}(3,0)$ construction is not yet the Dirac algebra. Gate G4 is: show that Lorentz signature emerges in a continuum limit of the engine — not assumed.

**G5. Fermion statistics.** Clifford structure alone does not yet give fermion number or spin-statistics. Show that topological defects in the Clifford-graded field have half-integer angular-momentum commutators in the engine's rotational structure.

Until all five gates are passed, "fermions emerge from the b=2 block" is a direction, not a result.

## 5. Relation to existing FTD work

- [THEOREM_MOORE_LAYER_DECOMPOSITION.md](../08_structural/THEOREM_MOORE_LAYER_DECOMPOSITION.md) already supplies a 6+12+8 = 26 Moore decomposition into SC + FCC + BCC. That is about directional sublattices in a different sense (not about algebraic products). The b=2 / Walsh decomposition here is cell-internal; the Moore decomposition is inter-site. They are independent structures and should not be conflated.
- [SPEC_FTD_NATIVE_BLOCKING_MAP.md](../10_eft_program/SPEC_FTD_NATIVE_BLOCKING_MAP.md) defines the b=2 Wilsonian blocking map on source / flux / current. That map sums fine variables into coarse variables and does **not** introduce a Clifford product. Whether blocking dynamics on iterated b=2 steps generates such a product at criticality is the live question; it is not settled by the block decomposition itself.
- The existing matter-sector documents (`DERIV_PROJECTED_DIRAC_OPERATOR_AND_CHARGE_NORMALIZATION.md`, `DERIV_PROJECTED_EFT_MATTER_COUPLING.md`) already carry [SELECTION] tags for Dirac matter. They explicitly note that the fermion content is not forced by FTD axioms. That state is unchanged by this exploration.

## 6. Epistemic tag

The substantive claim of §1 of the PDF is therefore split:

| Piece | Tag | Justification |
|---|---|---|
| 8-dim b=2 block admits a 1+3+3+1 Hamming-weight grading | [THEOREM] | Character theory of $(\mathbb{Z}/2)^3$. Immediate. |
| This grading = $\mathrm{Cl}(3,0)$ vector-space grading | [OBSERVATION] | Dimension and grade counts match. |
| The b=2 block **natively carries Cl(3,0) as an algebra** | [CLOSED NEGATIVE] | §2 no-go. Pointwise WH product is abelian; Cl(3,0) is not. |
| FTD dynamics on a b=2 block **dynamically generate** a Clifford product | [CONJECTURE] | Open; would require passing G1–G5 above. |
| Fermions are topological defects of the b=2 block | [CONJECTURE] | Contingent on the Clifford-product conjecture plus G5. |
| The flux-transition rules equal Dirac $\gamma$-matrices | [CONJECTURE] / likely [OPEN NEGATIVE] at the signature level | Cl(3,0) is Euclidean; Dirac algebra is Lorentzian 16-dim. |

## 6.5 Measurement outcome (2026-04-24)

Gate G1–G3 for the dynamical-emergence conjecture were carried out on the GPU via `engine/tests/test_wh_clifford_anticommutator.cpp` (ctest `wh_clifford_anticommutator`, labels gpu native eft). Protocol: L=8 lattice, 2³ corner block, inject flux ∝ A·χ_f along the matching axis (A=10 for deterministic genesis), run 1 tick, inject mode g, run 1 tick, WH-decompose the state field on the block.

Measured anticommutator on the three weight-1 modes:

$$ \{e_i, e_j\} = 2 \cdot \mathbb{1} \quad \text{for all } i, j \in \{1,2,3\}. $$

Diagonal agrees with Clifford ($e_i^2 = \mathbb{1}$). Off-diagonal is maximally wrong: Clifford requires 0, measurement gives $2 \cdot \mathbb{1}$.

**Cause.** Every weight-1 mode injection produces the same uniform state after tick 1 via the genesis rule (sign of divergence → polarity). The mode index is lost before the second tick can expose any ordering asymmetry.

**Verdict.** Clifford emergence on the b=2 block via genesis + movement is **falsified**. See [DERIV_WH_ALGEBRA_VS_CLIFFORD_NOGO.md §8](DERIV_WH_ALGEBRA_VS_CLIFFORD_NOGO.md#8-measurement-executed-2026-04-24) for the measurement table and [LEDGER.md:FTD-0061](../07_assessment/core_ledgers/LEDGER.md) for the closure record.

The fermion-emergence direction is not yet fully closed — four alternative routes (pair production, weak transmutation, Moore-26 stencil, velocity-driven movement) remain measurable with the same harness. Each is a future falsifier.

## 7. What this changes in the project

Nothing in the canonical derivation chain is promoted or demoted. The existing documents remain:

- $x_+ = 1/\alpha$: [STRONGLY MOTIVATED CONJECTURE]
- Master quadratic roots: [THEOREM] (arithmetic)
- Dirac matter: [SELECTION]
- Projected U(1): [THEOREM] under specified gauge choice

This file adds one more [CONJECTURE] row (FTD-0061) to the ledger and one [CLOSED NEGATIVE] sub-claim (the spontaneous version). It does not close the "fermions from FTD" program — that remains open.

---

*Filed 2026-04-24 in response to a PDF draft that labelled the above as THEOREM 1. The reframe preserves the idea as a direction of work while keeping the tag honest.*
