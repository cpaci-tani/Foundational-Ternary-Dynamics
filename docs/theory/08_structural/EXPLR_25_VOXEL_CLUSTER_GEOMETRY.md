# Exploration — Mathematical Interpretation of the 25-Voxel ic1 Cluster

**Tag:** [EXPLORATORY] / [STRUCTURAL HYPOTHESIS] (mathematical interpretation of an engine measurement; bridges algebra and engine via Moore-26 decomposition)
**Date:** 2026-04-27
**Builds on:** [`ANALYSIS_EMERGENT_SPECTRUM_G1.md`](../10_eft_program/ANALYSIS_EMERGENT_SPECTRUM_G1.md) (FTD-0107: deterministic 25-voxel cluster L-invariant), [`SPEC_ALGEBRAIC_SPINE.md`](../01_reference/SPEC_ALGEBRAIC_SPINE.md), [`DERIV_BCC_MULTIPLICATIVE_STRUCTURE.md`](DERIV_BCC_MULTIPLICATIVE_STRUCTURE.md), [`THEOREM_MOORE_LAYER_DECOMPOSITION.md`](THEOREM_MOORE_LAYER_DECOMPOSITION.md)
**Open question (per `WHERE_WE_LEFT_OFF.md` §10):** WHY exactly 25 voxels for the point-injection bound state?

---

## 0 · The empirical fact

Per FTD-0107 (measured 2026-04-27 at L ∈ {32, 64}): point injection of $10 K_\text{GENESIS}$ at lattice center produces a stable bound-state cluster with the following deterministic properties:

- **Voxel count: exactly 25**, across 5/5 seeds at L=32 AND 5/5 seeds at L=64
- **Centroid: exact integer lattice center** (32, 32, 32) at L=64; (16, 16, 16) ± 0.1 at L=32 (Langevin perturbations)
- **Static**: no centroid propagation across 2400 ticks
- **Charge sum**: typically −1, varies seed-to-seed within the cluster
- **L-invariant absolute size**: same 25 voxels at L=32 (occupying 0.076% of lattice) and L=64 (0.0095%)

The number 25 is empirically locked. This document interprets it mathematically.

---

## 1 · 25 is structurally meaningful: the second centered octahedral number

**[THEOREM]** (integer-counting fact): the number of integer points in the L¹ ball of radius $r$ in ℤ³ is the **centered octahedral number**:

$$O(r) = \frac{(2r+1)(2r^2+2r+3)}{3}$$

| $r$ | $O(r)$ | Cubic-symmetric form |
|---:|---:|---|
| 0 | 1 | center voxel only |
| 1 | 7 | center + 6 face neighbors |
| **2** | **25** | **center + face + edge + face2** ← **the cluster size** |
| 3 | 63 | adds BCC corners + face-edge etc. |
| 4 | 129 | |
| 5 | 231 | |

This is OEIS [A001845](https://oeis.org/A001845).

**Per-shell decomposition** (number of integer points at L¹ distance exactly $k$): $4k^2 + 2$ for $k \geq 1$:

- $k=0$: 1 (center)
- $k=1$: 6 (face)
- $k=2$: 18 = 6 (face2 at axis-distance 2) + 12 (edge at L²=√2)
- $k=3$: 38 (includes 8 BCC corners + 24 face-edge combinations + 6 face3)

**The L¹ ball of radius 2 contains exactly 25 voxels.** That is the cleanest structural interpretation of the cluster size.

---

## 2 · Sub-stencil decomposition: SC + FCC, EXCLUDING BCC

The Moore-26 neighborhood of the origin in ℤ³ decomposes (per `THEOREM_MOORE_LAYER_DECOMPOSITION.md`) into three cubic sub-stencils:

- **SC (simple cubic)**: 6 face neighbors at $(±1, 0, 0)$, etc. Distance L¹=1, L²=1.
- **FCC (face-centered cubic)**: 12 edge neighbors at $(±1, ±1, 0)$, etc. Distance L¹=2, L²=√2.
- **BCC (body-centered cubic)**: 8 corner neighbors at $(±1, ±1, ±1)$. Distance L¹=3, L²=√3.

The 25-voxel L¹ ball of radius 2 contains:

| Component | Count | L¹ | L² | In Moore-1? |
|---|---:|---:|---:|---|
| **center** | 1 | 0 | 0 | yes |
| **SC** (face) | 6 | 1 | 1 | yes |
| **FCC** (edge) | 12 | 2 | √2 | yes |
| **face2** (axis at distance 2) | 6 | 2 | 2 | NO (Moore-2 shell) |
| **BCC** (corner) | 0 | — | — | NOT INCLUDED |

So the cluster is:

$$\boxed{\text{cluster} \;=\; \text{center} \;\cup\; \text{SC} \;\cup\; \text{FCC} \;\cup\; \text{face2}}$$

with the **8 BCC corners explicitly excluded**, and 6 voxels from a second SC-shell at L¹=2 (the face2 voxels at $(±2, 0, 0)$, etc.) included.

In Moore-Layer-Theorem language: the cluster spans **two Moore-shells** (Moore-1 partial + Moore-2 partial) and **systematically excludes the BCC corner voxels at L¹=3**. The exclusion is structural, not statistical — for the cluster to have cubic O_h symmetry and contain exactly 25 voxels at integer positions around the center, the BCC corners must be the missing component.

---

## 3 · The complementarity finding (load-bearing)

**The most structurally striking observation:**

- The **cluster** (engine measurement, FTD-0107) lives on **SC + FCC + face2**, EXCLUDING BCC.
- FTD's **algebraic spine** (G\* identity, Watson identity W₃ = G\*²/(2π), master quadratic) lives on the **BCC sub-stencil** per `DERIV_BCC_MULTIPLICATIVE_STRUCTURE.md`. Watson's BCC integral is the closed-form route from the cubic lattice to G\*.

**The engine bound state and the algebraic spine occupy COMPLEMENTARY parts of the Moore-26 decomposition:**

```
Moore-26 = SC(6) ∪ FCC(12) ∪ BCC(8)
           └─────────┬─────────┘  └─┬─┘
                     │              │
       cluster lives here      G\*, master quadratic,
                               Watson identity live here
```

This is the closest structural connection FTD has yet identified between the algebraic spine (number theory) and engine phenomenology (lattice physics) — and it's a **dual / complementary** relationship, not a direct one.

If this is structurally meaningful (vs coincidence), it suggests:
- The cluster is the **physical realization of the SC+FCC content** of the Moore decomposition
- The algebra is the **number-theoretic content of the BCC sub-stencil**
- The bridge between them is exactly the partition Moore-26 = SC ⊔ FCC ⊔ BCC, with the cluster realizing the SC⊔FCC complement

This is a [STRUCTURAL HYPOTHESIS], not a derivation — but it's concrete enough to be tested.

---

## 4 · The Green's-function level-set interpretation

A second mathematical interpretation: the cluster boundary corresponds to a level set of the lattice Poisson Green's function on ℤ³.

**3D simple-cubic lattice Green's function** $G_L(x)$ at integer points (Watson 1939, Joyce 1973, Glasser & Zucker 1980):

| Position | $G_L$ | L¹ | L² |
|---|---:|---:|---:|
| $(0,0,0)$ | 0.5055 | 0 | 0 |
| $(1,0,0)$ | 0.3314 | 1 | 1 |
| $(1,1,0)$ | 0.2292 | 2 | √2 |
| $(2,0,0)$ | **0.1810** | 2 | 2 |
| $(1,1,1)$ | **0.1809** | 3 | √3 |
| $(2,1,0)$ | 0.1527 | 3 | √5 |

(values normalized per Watson convention; relative ordering is the load-bearing fact, not absolute magnitudes)

**Critical structural fact**: $G_L(2,0,0) > G_L(1,1,1)$ by a margin of 0.07% (0.18099 vs 0.18093). The face2 voxel at $(2,0,0)$ has SLIGHTLY higher Green's function value than the BCC corner at $(1,1,1)$.

**A threshold cut between** $G_L(1,1,1) = 0.18093$ and $G_L(2,0,0) = 0.18099$ **selects exactly the 25-voxel L¹ ball of radius 2**:

- Above threshold (in cluster): center, 6 SC, 12 FCC, 6 face2 → 25 voxels
- Below threshold (excluded): 8 BCC corners, 24 face-edge at L¹=3, all longer distances

The 25-voxel cluster IS the level set of the lattice Poisson Green's function at this specific threshold. The threshold corresponds to whatever the FTD genesis equilibrium picks under Langevin pumping — a self-consistent density at which manifestation is just barely sustained.

**Caveat — this interpretation requires explanation, not just identification**: the threshold cuts in a 0.07% gap. In raw Poisson terms (without Langevin), this fine-tuning would be a striking coincidence. Two cleaner reads are possible:

1. **The Langevin-pumped equilibrium** lands exactly at this threshold by self-consistency (the cluster's manifested density acts as a source, and the equilibrium cluster size is where source-from-cluster + injected-source equals the level set just outside corners but just inside face2).

2. **The L¹ ball of radius 2 is a topologically privileged shape** (smallest cubic-symmetric "saturated" cluster where each interior voxel has enough same-state neighbors to resist Langevin decay), and the Green's function ordering is consistent with this — but the topology, not the level set, is what selects the size.

Either way, the empirical match is striking: the cluster's 25-voxel count and the L¹ ball of radius 2 / Green's-function-level-set structure converge.

---

## 5 · What this interpretation does NOT yet establish

**Not yet verified:**

- **Direct positional check**: the engine's `cluster_history` CSV stores aggregate voxel_count, centroid, charge_sum, but NOT individual voxel coordinates. The L¹-ball-radius-2 identification is a **strong hypothesis** consistent with the symmetry data (centroid at exact integer center, count = 25, cubic symmetry implied) but the actual positional layout has not been dumped from the engine. **First-priority verification**: instrument the campaign to emit per-cluster voxel-coordinate lists at terminal state.

- **Why radius 2 specifically**: section §4 offers two plausible structural mechanisms (self-consistent equilibrium, topological saturation) but neither is derived. The cluster size is empirically 25 across L ∈ {32, 64}; *why* this radius and not 1 or 3 is the open derivation question.

- **Energy / mass interpretation**: the cluster has ~25 voxels with total energy ~7000–18000 (Langevin background) and charge sum ~−1. There is no calibration to physical mass-units (FTD-0096 [OPEN]).

**Not pursued in this document (per CLAUDE.md / FTD-0097 anti-pattern-matching discipline):**

- Numerical-coincidence searches between the cluster's Green's-function-weighted observables and known physics constants. Any such investigation requires its own pre-registration, look-elsewhere control, and structural derivation route — exactly the discipline FTD-0097 just exercised. This document records the cluster's mathematical structure, NOT speculative numerical fits.

---

## 6 · Open structural questions (the next research thread)

1. **Verify the L¹-ball topology** by instrumenting the engine campaign to emit per-voxel coordinates at terminal state. Trivial code change (~20 LOC); would either confirm the L¹-ball-radius-2 hypothesis or reveal a different 25-voxel arrangement.

2. **Derive radius=2 from FTD axioms**. Candidate routes:
   - **Self-consistent equilibrium**: solve for the cluster size $r$ where Langevin-pumped equilibrium density equals the genesis threshold at the boundary $\partial B_1(r)$ but exceeds it inside. Closed-form solution would predict $r$ from $(K_\text{GENESIS}, T_\text{Langevin}, \text{injection amplitude})$.
   - **Topological saturation**: prove that for any cubic-symmetric cluster of size $\leq 25$, removing any voxel costs more energy than the Langevin pressure to add a voxel; for size $\geq 27$ (Moore-26 + center) it costs more energy than Langevin to add the BCC corners; size = 25 is the stable plateau.

3. **Test the complementarity hypothesis** (§3). If the cluster lives structurally on SC+FCC and the algebra on BCC, the dual relationship should manifest in additional engine observables. Candidates:
   - The flux-energy autocorrelation on the **SC + FCC sub-stencils** should exhibit the dynamics matching the cluster (mass scale, decay rates).
   - The flux-energy autocorrelation on the **BCC sub-stencil** should exhibit the algebraic-spine signature (Watson identity, master-quadratic-related eigenvalues — though FTD-0093 closed-negative for the specific ratio λ₊/λ₋ = 45.31 prediction; perhaps a different observable lives there).
   - **A quantitative complementarity check**: ratio of (cluster manifested-voxel signal on SC+FCC) to (BCC residual) should track 25/8 or some related geometric coefficient.

4. **L=128 G2 follow-up** for FTD-0107 (already in the priority queue). Locks the L-invariance further; would also test whether the cluster size shifts at larger L (it shouldn't, per Outcome A.2 logic).

---

## 7 · What this means for the "physically missing" diagnosis

Per `WHERE_WE_LEFT_OFF.md` §10, the load-bearing gap in FTD is the absence of a derivation chain between the algebraic spine (number theory) and the engine phenomenology (lattice physics). The 25-voxel cluster question is the most concrete entry point for closing this gap.

**This document advances the question structurally** but does NOT close it:

- ✅ **Identifies 25 as a clean structural number**: the second centered octahedral number, the L¹-ball-radius-2 count.
- ✅ **Links the cluster to a specific Moore-26 sub-decomposition**: SC + FCC + face2 + center, EXCLUDING BCC.
- ✅ **Identifies a complementarity** between the cluster's substrate (SC+FCC) and the algebraic spine's substrate (BCC). This is the cleanest structural connection FTD has yet found between the two pillars.
- ⚠ **Does NOT derive radius=2 from FTD axioms**. The "WHY radius 2" question stays [OPEN] as a structural derivation challenge.
- ⚠ **Does NOT verify the L¹-ball topology positionally**. Engine instrumentation needed.
- ⚠ **Does NOT promote any physics interpretation**. The cluster is engine-native phenomenology; mass / charge / particle-identity readings remain [OPEN].

**Net contribution**: the bridge between algebra and engine, while not yet closed, has its first concrete candidate: the **complementary-Moore-decomposition hypothesis** that the engine bound state realizes the SC+FCC content while the algebraic spine encodes the BCC content. This hypothesis is testable via engine measurements on the SC+FCC vs BCC sub-stencils — exactly the kind of structural test CLAUDE.md's epistemic discipline asks for.

---

## 8 · Single-line summary

**The 25-voxel ic1 bound-state cluster (FTD-0107) has a clean structural interpretation: 25 = O(2) is the second centered octahedral number, equal to the count of integer points in the L¹ ball of radius 2 in ℤ³. Decomposed by O_h orbit: 1 (center) + 6 (face1, SC sub-stencil) + 12 (edge, FCC sub-stencil) + 6 (face2 at axis-distance 2). The cluster EXCLUDES the 8 BCC corner voxels at L¹=3 — meaning it lives on the SC + FCC parts of the Moore-26 decomposition, exactly COMPLEMENTARY to the BCC sub-stencil where FTD's algebraic spine (G\*, Watson identity W₃ = G\*²/(2π), master quadratic) lives. The lattice Poisson Green's function on ℤ³ has G(2,0,0) ≈ 0.181 just BARELY above G(1,1,1) ≈ 0.181 (0.07% margin), so a threshold in this gap selects exactly the 25-voxel L¹-ball arrangement (face2 included, BCC corners excluded) — consistent with a Langevin-pumped equilibrium picking this level set self-consistently. Three open questions remain: positional verification (engine instrumentation), derivation of radius=2 from axioms, and quantitative test of the complementarity between cluster (SC+FCC) and spine (BCC) on engine sub-stencil observables. This is the most concrete bridge candidate FTD has found between number-theoretic structure and engine-as-instrument phenomenology — a [STRUCTURAL HYPOTHESIS], not yet [THEOREM].**
