# Exploration — Volumetric Properties + Structural Pontification on Octahedral Bound States

**Tag:** [EXPLORATORY] / **[POLYTOPE-DUALITY HYPOTHESIS REFUTED 2026-04-27]** — the cluster-on-SC+FCC vs algebra-on-BCC duality reading was directly tested by `engine/tests/test_emergent_ic1_topology.cpp` and the engine produced a topology that INCLUDES the BCC corners. See §10 (Corrigendum, end of file). The volumetric properties (§1) and the centered-octahedral-number facts (§4) remain valid as math; only the polytope-duality interpretation (§2-§3) is refuted.
**Date:** 2026-04-27 (with corrigendum same day)
**Builds on:** [`EXPLR_25_VOXEL_CLUSTER_GEOMETRY.md`](EXPLR_25_VOXEL_CLUSTER_GEOMETRY.md) (also has §11 corrigendum), [`THEOREM_MOORE_LAYER_DECOMPOSITION.md`](THEOREM_MOORE_LAYER_DECOMPOSITION.md), [`DERIV_BCC_MULTIPLICATIVE_STRUCTURE.md`](DERIV_BCC_MULTIPLICATIVE_STRUCTURE.md)
**Discipline reminder:** every numerical claim tagged per CLAUDE.md epistemic ladder; no pattern-matching promotion; the existing FTD-0097 look-elsewhere result rules out promotion based on monomial-level numerical fits at ε ≤ 10⁻⁴.

---

## 0 · Why this document exists

`EXPLR_25_VOXEL_CLUSTER_GEOMETRY.md` (companion, this morning) identified the cleanest mathematical interpretation of the FTD-0107 cluster: 25 = O(2) is the second centered octahedral number, and the cluster realizes the L¹-ball of radius 2 in ℤ³, which is the integer-point fillout of a regular octahedron. The cluster lives on SC + FCC + face2 sub-stencils, *complementary* to the BCC sub-stencil where FTD's algebraic spine lives.

This document does two things the morning doc didn't:

1. **Volumetric expansion**: surface area, interior structure, scaling, anisotropy, all of the size-3 thru size-6 octahedral sequence — what the cluster looks like as a 3D object and how that geometry behaves under L→ size scaling.

2. **Pontification on what this means structurally**: the cluster IS the dual polytope of the lattice cell; the SC+FCC vs BCC complementarity matches the Moore Layer Theorem's octahedron+cuboctahedron vs stella-octangula decomposition exactly; this is the cleanest structural bridge between FTD's algebraic spine (Watson identity on BCC) and engine phenomenology that the project has yet found.

---

## 1 · Volumetric properties of the L¹-ball-radius-r family

**[THEOREM]** (computed exhaustively from cubic-lattice geometry):

| $r$ | $O(r)$ | Internal pairs | Boundary faces | Interior voxels | Surface voxels | S/V ratio | Equiv. sphere $R$ | Sphere surface | Anisotropy |
|---:|---:|---:|---:|---:|---:|---:|---:|---:|---:|
| 0 | 1 | 0 | 6 | 0 | 1 | 6.00 | 0.620 | 4.84 | 1.24 |
| 1 | 7 | 6 | 30 | 1 | 6 | 4.29 | 1.187 | 17.70 | 1.70 |
| **2** | **25** | **36** | **78** | **7** | **18** | **3.12** | **1.814** | **41.35** | **1.89** |
| 3 | 63 | 114 | 150 | 25 | 38 | 2.38 | 2.468 | 76.57 | 1.96 |
| 4 | 129 | 264 | 246 | 63 | 66 | 1.91 | 3.134 | 123.47 | 1.99 |
| 5 | 231 | 510 | 366 | 129 | 102 | 1.58 | 3.806 | 182.06 | 2.01 |
| 6 | 377 | 876 | 510 | 231 | 146 | 1.35 | 4.481 | 252.37 | 2.02 |

(Anisotropy = boundary-faces / equivalent-sphere-surface; 1.0 would be a perfect sphere.)

### Three structural observations

**(i) The interior is recursive: interior(r) = O(r−1).** A voxel is "interior" iff all 6 face-neighbors are in the cluster, iff it's at L¹ distance ≤ r−1 from the cluster center. So the inner volume of the L¹-ball-radius-r is the L¹-ball-radius-(r−1) — it's nested octahedra all the way down. For the 25-voxel cluster, the 7-voxel "core" is itself a smaller L¹-ball (= O(1) = center + SC ring).

**(ii) Lattice anisotropy converges to ≈ 2.0** as $r$ grows. The boundary-face count of the L¹-ball-radius-r is exactly $4r² + 2 + 4r² + 2 + ... = $ (per-shell count summed over the surface shell only) = $4r² + 2$ at the outer shell, but each of those voxels has variable boundary-face exposure, so the total boundary-face count approaches $\sim 4 \pi r^2 \cdot 2$ asymptotically. The factor 2 is the standard cubic-lattice anisotropy — an octahedron's surface in a cubic lattice has twice the lattice-face count of the equivalent-volume sphere's continuum surface area. This is independent of r in the limit.

**(iii) Per-orbit binding asymmetry inside the 25-voxel cluster:**

| Orbit | Voxels | In-cluster face-neighbors | Out-of-cluster face-neighbors |
|---|---:|:---:|:---:|
| Center | 1 | **6/6** | 0/6 |
| SC (face1) | 6 | **6/6** | 0/6 |
| FCC (edge) | 12 | 2/6 | 4/6 |
| face2 (axis-2) | 6 | 1/6 | **5/6** |

The center and SC voxels are fully interior — every face-neighbor is in the cluster. The face2 voxels at $(\pm 2, 0, 0)$ etc. have only ONE in-cluster face-neighbor (the SC voxel toward the center) and **5 out-of-cluster faces** — they're the "exposed antlers" of the octahedron, barely held together. The FCC edge voxels are intermediate (2/6 internal).

This binding asymmetry suggests the bound state's stability against thermal decay is **not uniform** — under Langevin pressure, the face2 voxels are the first to decay, and the center+SC core is the last. The cluster is structurally "robust at the core, fragile at the axis-tips."

This makes a falsifiable engine prediction: **at increased Langevin T, the face2 voxels should decay first, leaving the 19-voxel core (center + SC + FCC) intact temporarily, then dissolve fully when the core's surface tension is overcome.**

---

## 2 · The cluster as the dual polytope of the cubic lattice cell

**Structural pontification** (treating this as exposition, not [THEOREM]):

The cubic lattice's primitive cell is a **cube**. The dual polytope of the cube — under the standard Platonic-solid duality — is the **octahedron**. The 25-voxel cluster is the integer-point fillout of a regular octahedron with vertex-distance 2 from center. **The bound state realizes the dual of the lattice cell.**

This is a clean structural statement. The lattice's geometric frame is cubic; the bound state that emerges is octahedral. This kind of "primal-dual" pairing is well-known in crystallography (the reciprocal lattice of a cubic Bravais lattice with one symmetry IS the cubic lattice with the dual symmetry — FCC and BCC are reciprocal pairs; SC is self-dual).

**The Moore Layer Theorem connection** (per `THEOREM_MOORE_LAYER_DECOMPOSITION.md` and CLAUDE.md):

> "Moore neighborhood polyhedral decomposition (octahedron + cuboctahedron + stella octangula)"

The Moore-26 neighborhood decomposes as:
- **Octahedron**: 6 vertices = SC sub-stencil (face neighbors)
- **Cuboctahedron**: 12 vertices = FCC sub-stencil (edge neighbors)
- **Stella octangula**: 8 vertices = BCC sub-stencil (corner neighbors)

The octahedron and the stella octangula are *dual* in the Platonic-compound sense (the stella octangula is the compound of two tetrahedra, dual via cube). The cuboctahedron sits between them.

**The 25-voxel cluster occupies the octahedron + cuboctahedron content** (6 + 12 + center + 6 face2 axial extensions), **excluding the stella-octangula content** (the 8 BCC corners). The face2 voxels at $(\pm 2, 0, 0)$ ARE the vertices of the LARGER octahedron of radius 2 — the cluster naturally extends along the octahedral axis-directions.

**FTD's algebraic spine** (per `DERIV_BCC_MULTIPLICATIVE_STRUCTURE.md`) lives on the **stella-octangula** content: G\* and the Watson identity W₃ = G\*²/(2π) emerge from the BCC eigenvalue triple-cosine product. This is the algebra-side of the Moore Layer decomposition.

So the structural picture is:

```
                Moore-26 polyhedral decomposition
                ─────────────────────────────────
   Octahedron (SC, 6 vertices)  ──┐
                                  │  ENGINE BOUND STATE 
   Cuboctahedron (FCC, 12 ver.) ──┤  (FTD-0107 25-voxel cluster
                                  │   extending to radius-2 octahedron)
   center (1)                    ──┘
   ─────────────────────────────────
   Stella octangula (BCC, 8 v.) ──── ALGEBRAIC SPINE
                                     (G*, master quadratic, 
                                      Watson identity W₃ = G*²/(2π))
```

**The bound state and the algebra occupy DUAL parts of the Moore Layer Theorem decomposition.** This is the closest structural connection FTD has yet identified between its number-theoretic content and its engine phenomenology. It's a [STRUCTURAL HYPOTHESIS], not a [THEOREM] — but it's testable.

---

## 3 · What the duality means for FTD's structural gap

Per `WHERE_WE_LEFT_OFF.md` §10, the load-bearing gap is the absence of a derivation chain between the algebraic spine (number theory) and engine phenomenology (lattice physics). The Moore-Layer dual pairing does NOT close this gap, but it sharpens its shape.

**Standard physics' bridge structure:**
$$\text{Lagrangian} \xrightarrow{\text{variational}} \text{equation of motion} \xrightarrow{\text{solve}} \text{observable}$$

There's a single chain. Math derives observable.

**FTD's dual-pillar structure (proposed):**
$$\underbrace{\text{algebra (BCC)}}_{\text{Watson identity, G*, master quadratic}} \quad \perp \quad \underbrace{\text{phenomenology (SC+FCC)}}_{\text{cluster size, deterministic counts}}$$

There are two chains, on dual sub-stencils, structurally orthogonal. **Neither derives the other directly; both describe complementary aspects of the same lattice.** The bridge is the Moore-Layer Theorem itself: the decomposition Moore-26 = octahedron ⊔ cuboctahedron ⊔ stella octangula is the structural fact connecting both chains.

If this picture is correct, FTD is not a "physics-recovery" framework in the standard sense — it's a **dual-aspect framework** where the lattice's combinatorial / algebraic content (BCC sub-stencil → Watson → G\*) and physical / dynamical content (SC+FCC sub-stencil → cluster shape → bound states) are structurally distinct but related by polytope-duality.

This is a meaningful claim. It either:
- **Survives engine testing** (if cluster dynamics on SC+FCC track Watson-identity quantities on BCC in dual ratios), in which case it's a structural [SELECTION], or
- **Fails engine testing** (if no such dual relationship is measurable), in which case the morning doc's complementarity hypothesis is closed-negative and we revisit.

**The engine-testable prediction**: dynamics on the SC+FCC sub-stencil (where the cluster lives) and on the BCC sub-stencil (where the algebra lives) should exhibit **dual scaling**. Specifically, cluster-related observables (manifestation rate, cluster surface tension, 25-voxel saturation point) measured on SC+FCC voxels should track the BCC-sub-stencil eigenvalue spectrum (the FTD-0093 measurement, which closed-negative for a specific 45.31 ratio prediction but which has more spectrum content than that single ratio). If they're structurally related by O_h Plücker-type duality, ratios of cluster-side observables to algebra-side observables should have specific values predictable from the polytope duality.

This would need a dedicated pre-registration (probably FTD-0108 or 0109 in the queue).

---

## 4 · Volumetric extrapolation: predictions for varying injection amplitude

**The bound state at injection 10·K_GENESIS produces 25 voxels (= O(2)). What does it produce at higher injection?**

This is **FALSIFIABLE BY ENGINE MEASUREMENT** with no new pre-registration overhead — same campaign, varied parameter. Three plausible scaling hypotheses:

### Hypothesis A: Discrete jumps to next centered octahedral number

- Injection 10·K_GENESIS → cluster size O(2) = 25 (observed)
- Injection $N \cdot K_\text{GENESIS}$ with $N > N_3^*$ → cluster size O(3) = 63
- Injection $N > N_4^*$ → cluster size O(4) = 129
- Discrete plateaus separated by phase-transition-like jumps at thresholds $N_r^*$

**Engine-testable prediction**: scan injection amplitude N ∈ {5, 10, 15, 20, 30, 50, 100} × K_GENESIS at L=64, observe whether cluster size jumps from 25 → 63 → 129 at specific N values, OR scales smoothly (which would falsify the discrete-octahedral hypothesis).

### Hypothesis B: Smooth scaling proportional to N^p

- $\text{volume}(N) \propto N^p$ for some exponent p
- p = 1 (linear): cluster size ∝ N
- p = 1/3 (cube root): radius ∝ N^(1/3); volume ∝ N
- p = log: r ∝ log(N)

If p = 1, then cluster volume = ~2.5 × N at large N. At N = 100, volume ≈ 250 ≈ O(5) = 231. Plausible scaling but not centered-octahedral exact.

### Hypothesis C: Saturation at finite size

- Cluster size grows with N up to a maximum, then plateaus
- Plateau set by lattice / Langevin equilibrium
- Above plateau, additional injection produces multi-cluster fragmentation or runaway

This would be the same structural mechanism behind FTD-0102's runaway phase (ic2, ic5) — at sufficient injection / temperature, the bound state can't stay localized.

**Recommended next campaign**: vary injection amplitude N at L=64 with single-point ic1 protocol; report cluster sizes per N. ~1 GPU hour for a 7-point sweep. **Either confirms one of A/B/C OR reveals a fourth pattern.**

If Hypothesis A (discrete octahedral plateaus) holds, **the FTD lattice has discrete bound-state states with sizes given exactly by the centered octahedral number sequence** {1, 7, 25, 63, 129, 231, ...}. This would be a STRONG structural finding: the engine produces a "spectrum" of bound-state sizes that's directly tabulated by a number-theoretic sequence (OEIS A001845).

---

## 5 · The ic3 collision case: 3-5 voxels per cluster

ic3 (collision) produces 2 clusters of 3-5 voxels at both L=32 and L=64. These are smaller than O(1) = 7 (the minimum cubic-symmetric octahedral cluster). What shape can 3-5 voxels take?

Cubic-lattice arrangements of 3-5 voxels:

| Voxels | Shapes | Symmetry |
|---:|---|---|
| 3 | linear chain (1×1×3); L-shape; corner triangle | $D_{4h}$ for chain; lower for L-shape |
| 4 | square (1×2×2 face); T-shape; tetrahedron NOT possible | $D_{4h}$ for square; $C_{2v}$ for T |
| 5 | plus-sign (center + 4 face1 in plane); T+ shape; X | $C_{4v}$ for plus; lower otherwise |

**The most cubic-symmetric small cluster is the 5-voxel "plus-sign"**: a center plus 4 face1 neighbors in a plane. This has $C_{4v}$ symmetry, NOT full $O_h$. If ic3's clusters are plus-signs in a plane perpendicular to the collision axis, that's structurally consistent with:
- The collision deposits energy on a plane (perpendicular to the injection axis)
- The bound state extends perpendicular to the collision direction (symmetric in the plane, but not along the impact axis)

For 3-voxel clusters: a linear chain of 3 voxels along the collision axis would be the simplest. For 4-voxel: a 2×2 plate.

**Engine-testable prediction**: ic3's clusters should have **planar / axial** symmetry (perpendicular or parallel to the collision axis), NOT full cubic symmetry like the ic1 25-voxel cluster. This is a falsifiable shape-prediction that engine instrumentation (per-voxel positional dump) would resolve.

If ic3 clusters are plus-signs (5-voxel) in a perpendicular plane, then the cluster size of 3-5 represents a plus-sign with possibly 1-2 voxels missing — partial cubic-symmetric structure, asymmetric due to the collision geometry's symmetry breaking.

---

## 6 · Pontification — what this all means for FTD's structural picture

Stepping back from the specific clusters:

**The cubic lattice has its own structural identity.** It's not just a discretization of continuous physics; it has combinatorial and topological content of its own. The Moore-26 decomposition into octahedron + cuboctahedron + stella octangula is one expression of that identity. The L¹-ball-radius-r centered octahedral numbers $\{1, 7, 25, 63, ...\}$ are another. Watson's BCC integral $W_3 = G^{*2}/(2\pi)$ is a third. **None of these are physics; they're properties of the cubic lattice itself.**

FTD's algebraic spine (G\*, master quadratic, CM uniqueness, etc.) extracts the algebraic content of the cubic lattice. FTD's engine measurements (cluster sizes, phase structure, deterministic counts) extract the dynamical content. **Both are real.** What's been missing is the chain that says they describe THE SAME OBJECT viewed from different angles.

The 25-voxel cluster's identification as **the octahedral-radius-2 fillout, on the OCTAHEDRON+CUBOCTAHEDRON content, dual to the BCC stella-octangula content where the algebra lives** — is the closest structural unification we've found. It's not a derivation; it's a structural identification of the cluster with a polytope and the algebra with the dual polytope.

**The deeper interpretation:** maybe physical observables in FTD always live on the SC+FCC content (the convex polytope structure), and informational/algebraic observables live on the BCC content (the corners / dual structure). Standard physics' Lagrangian formulation puts everything on a single substrate; FTD's structural picture suggests the substrate is *naturally split*.

If this holds, it's a structural reading that says **the lattice has two complementary content layers, one physical and one informational**, and they cohere into a single cubic structure. Engine measurements live on one layer; algebraic theorems live on the other. The Watson identity bridges them at the [THEOREM] level (it relates BCC eigenvalues to the lemniscatic constant G\*); the cluster duality bridges them at the [STRUCTURAL HYPOTHESIS] level (the cluster's octahedral shape is the dual of the BCC corners).

Whether this picture survives further engine testing — particularly the volumetric scaling experiment in §4 (does cluster size jump to O(3)=63 at higher injection?) — determines whether FTD has a structural unification or just a parallel pair of pillars.

**The honest read:** the project is at the boundary between "two pillars without connection" (where it's been since at least 2026-04-19) and "two pillars connected by polytope duality" (which is the new hypothesis from today). The engine-testable predictions in §1-§4 are the way to decide which.

---

## 7 · Engine-testable predictions (recommended next moves)

In order of leverage:

**(1) Cluster-size scan vs injection amplitude.** At L=64, vary N ∈ {5, 10, 15, 20, 30, 50, 100} × K_GENESIS, single-point ic1 protocol. Measure cluster size per N. Tests Hypothesis A vs B vs C from §4. ~1 GPU hour. **If A holds (discrete jumps at O(r) sequence), it's a major structural finding.**

**(2) Per-voxel positional dump.** Modify `campaign_emergent_spectrum_2026-04-27.cpp` to emit cluster-voxel coordinates at terminal state. Verifies the L¹-ball-radius-2 hypothesis directly. ~20 LOC change; ~1 hour engineering + rebuild + re-run.

**(3) Decay sequence under increased Langevin T.** At L=64, run ic1 with Langevin T ∈ {0.005, 0.01, 0.02, 0.05, 0.1}. Predict (per §1's binding-asymmetry observation): face2 voxels decay first; FCC edges next; SC core last. Tests the per-orbit stability hypothesis. ~1-2 GPU hours.

**(4) Cross-substencil dual measurement.** Measure cluster dynamics on SC+FCC sub-stencils AND BCC eigenvalue spectrum on the same configuration; check whether ratios match predicted polytope-duality values. ~1-2 GPU hours; requires careful analysis.

**(5) ic3 cluster shape verification.** Per-voxel positional dump for ic3 too; verify the 3-5 voxel shapes are plus-signs/T-shapes in the perpendicular plane (per §5 prediction). Same engineering cost as (2).

---

## 8 · What this document does NOT do

- Does not derive O(r) sequence from FTD axioms (the centered octahedral numbers are pure cubic-lattice combinatorics, established by Klein 1880s; FTD recovers them via measurement).
- Does not promote the polytope-duality hypothesis (§2-§3) to [SELECTION]; it's [STRUCTURAL HYPOTHESIS] until engine measurements (§7) test it.
- Does not propose any specific physical interpretation of the bound states (e.g., "25-voxel cluster = particle X"). The cluster is engine-native phenomenology; mass/charge/identity readings remain [OPEN].
- Does not bypass FTD-0097's anti-pattern-matching discipline. Per §1's binding-asymmetry observation: the 25-voxel cluster's structural decomposition (1 + 6 + 12 + 6) is verifiable lattice geometry, not numerical fishing.

---

## 9 · Single-line summary

**The 25-voxel ic1 cluster is the L¹-ball-radius-2 = centered octahedral number O(2) integer-point fillout of a regular octahedron in ℤ³. Volumetrically it has 7 fully-interior voxels (= O(1) inner core), 18 surface voxels with per-orbit binding asymmetry (face2 voxels held by only 1/6 face-neighbors — barely-attached "antlers"), and lattice anisotropy ≈ 1.89× equivalent-sphere surface area. The cluster occupies the SC + FCC + face2 sub-stencils — exactly the OCTAHEDRON + CUBOCTAHEDRON content of the Moore Layer Theorem decomposition — and is structurally DUAL to the BCC stella-octangula content where FTD's algebraic spine (G\*, Watson identity, master quadratic) lives. This is the closest structural connection FTD has found between number-theoretic content and engine phenomenology: the bound state and the algebra occupy dual parts of the Moore polyhedral decomposition. Engine-testable prediction: cluster size at higher injection should jump discretely through the centered octahedral sequence {1, 7, 25, 63, 129, 231, ...} = O(r) at thresholds N_r*; verifying this would establish FTD's lattice has a discrete bound-state spectrum tabulated exactly by OEIS A001845. Per-orbit binding asymmetry predicts decay sequence under thermal stress: face2 → FCC edges → SC core. The polytope-duality hypothesis is [STRUCTURAL HYPOTHESIS] pending these engine measurements; if confirmed, it's the structural unification FTD's two pillars have been missing.**

---

## 10 · Corrigendum — polytope-duality hypothesis REFUTED by engine measurement (2026-04-27, same day)

**See `EXPLR_25_VOXEL_CLUSTER_GEOMETRY.md` §11 for the primary corrigendum.** This document built on that document's §3 hypothesis; both are refuted by the same measurement.

**Engine measurement** (`engine/tests/test_emergent_ic1_topology.cpp`, L=32, seed 0xE0102000) produced this per-orbit decomposition:

| Orbit | This document predicted | Engine measured | Status |
|---|---:|---:|---|
| center (L¹=0) | 1 | 1 | ✓ |
| SC face1 (L¹=1) | 6 | 6 | ✓ |
| FCC edge (L¹=2, L∞=1) | 12 | **7** | ✗ |
| face2 axis (L¹=2, L∞=2) | 6 | **3** | ✗ |
| **BCC corner (L¹=3, L∞=1)** | **0** | **8** | **✗ — REFUTES the duality** |
| **Total** | **25** | **25** | ✓ |

**The cluster INCLUDES all 8 BCC corners** — directly contradicting the §2-§3 reading that the bound state lives on the SC+FCC+face2 sub-stencils EXCLUDING BCC, and is therefore "structurally DUAL" to the BCC stella-octangula content where FTD's algebraic spine lives.

**What this refutes specifically:**

- §2's "cluster IS the dual polytope of the lattice cell" — REFUTED. Not the L¹-ball-radius-2 octahedron.
- §2's "cluster occupies octahedron + cuboctahedron content; algebra occupies stella octangula" — REFUTED. The cluster occupies SC + FCC + BCC + partial face2. There is **no clean SC+FCC vs BCC split**.
- §3's "polytope-duality bridge candidate" between algebra and engine — REFUTED. The two pillars do NOT live on dual parts of Moore-26; the engine cluster overlaps the BCC corners where the algebra lives.
- §1's per-orbit binding asymmetry analysis (face2 only 1/6, FCC 2/6, SC 6/6) — predictions about which voxels are most fragile under thermal stress are based on a wrong cluster shape; the actual stability gradient is unknown and would need re-derivation from the actual topology.
- §4 Hypothesis A (cluster size jumps through O(r) at higher injection) — STILL UNTESTED, but the underlying assumption that the cluster IS an L¹-ball is now refuted, so the discrete-O(r) prediction is also probably wrong. The actual cluster-size scaling rule is unknown.

**What survives:**

- §1's volumetric properties of the L¹-ball family (centered octahedral numbers, surface counts, anisotropy → 2.0) — **survives as pure cubic-lattice geometry**, but is **not what the FTD bound state realizes**. The math is unaffected; the *physical interpretation* (claiming the cluster IS the L¹-ball) is refuted.
- The recursive interior identity O(r) interior = O(r-1) — pure math, survives.
- The lattice-anisotropy → 2.0× sphere-surface scaling — pure math, survives.
- §4's CONCEPTUAL framing of "cluster size scaling with injection" as an engine-testable question — survives; but Hypothesis A's specific prediction (discrete O(r) plateau) needs replacement.

**What this opens:**

- The actual cluster topology (Moore-1 + center MINUS 5 FCC + 3 face2 = 25 voxels for this seed/L) is itself a NEW [HYPOTHESIS] needing characterisation.
- The cluster-size-vs-injection question is still open (a future pre-registered campaign could test it), but the PREDICTIONS need rebuilding from the actual cluster shape, not the L¹-ball-2 shape.
- The "structural bridge between algebra and engine" question (per `WHERE_WE_LEFT_OFF.md` §10) is unchanged: the polytope-duality candidate is now closed-negative, and the bridge remains an open structural problem.

### Methodological note

The polytope-duality reading was elegant and seemed to map cleanly onto the existing Moore Layer Theorem. The engine measurement directly refuted it within hours of being proposed — exactly the discipline working as intended. The morning's pontification is now a documented dead end, and the actual cluster topology is the new starting point.

The strongest positive structural finding (FTD-0107: 25-voxel cluster, deterministic, L-invariant) survives and is now **harder to interpret structurally** than this document claimed. The cluster shape involves all four Moore sub-stencils (center, SC, FCC, BCC), not the SC+FCC+face2 subset. The "WHY 25 voxels?" question persists; the morning's polytope-duality answer is wrong.

The structural gap (algebra ↔ engine) remains the load-bearing problem.

