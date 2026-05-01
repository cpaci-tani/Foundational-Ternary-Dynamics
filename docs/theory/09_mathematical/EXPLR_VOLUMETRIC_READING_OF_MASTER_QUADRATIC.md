# EXPLR — Volumetric Reading of the Master Quadratic (Correcting the 2×2 Abstraction)

**Document type:** Brainstorm correction (volumetric interpretation)
**Status:** [STRUCTURAL OBSERVATION] — clarifies that the master quadratic is volumetrically embedded via its coefficients (BCC Watson + A_{1g} multiplicity), NOT via the abstract 2×2 mixing matrix presentation
**Created:** 2026-05-01 evening (continuing harmonic-conjugacy brainstorm with volumetric correction)
**Provenance:** User correction "look at it volumetrically, not in flat space 2D" following the 2×2 mixing matrix interpretation
**Related:** `EXPLR_MASTER_QUADRATIC_AS_MIXING_MATRIX.md` (the 2×2 abstract reading); `EXPLR_3X3_MIXING_NEGATIVE.md` (3×3 generalization failure); `DERIV_BCC_MULTIPLICATIVE_STRUCTURE.md` (BCC Watson identity); `DERIV_K_FROM_OH_A1G_MULTIPLICITY.md` (A_{1g} structure)

---

## 0 · The correction

The 2×2 mixing matrix interpretation in `EXPLR_MASTER_QUADRATIC_AS_MIXING_MATRIX.md` was **abstract algebra**, not volumetric content. Specifically, the matrix

```
M_abstract = [[8G*²,   √(64G*⁴ − 16G*³)],
              [√(64G*⁴ − 16G*³),   8G*²]]
```

has the master quadratic as its characteristic polynomial, but its **entries are algebraic rearrangements** of the master quadratic's coefficients — not direct volumetric quantities.

The master quadratic IS volumetric, but via its **coefficients' origin** in the 3D 27-block structure, not via a literal 2-partition of voxels.

This document records the corrected reading.

---

## 1 · Where the master quadratic actually lives in 3D

### 1.1 · Two volumetric ingredients

The master quadratic `x² − 16·G*²·x + 16·G*³ = 0` has coefficients built from two distinct 3D structures:

**Ingredient 1: G* from the BCC Watson integral.** Per Theorem 5
(`DERIV_BCC_MULTIPLICATIVE_STRUCTURE.md`):

```
W₃ = G*²/(2π) = (3D Brillouin-zone integral on BCC sublattice)
```

This is a **3D volume integral** over the cubic-lattice Brillouin zone, evaluated on the BCC sublattice's spectral structure. It's intrinsically volumetric.

**Ingredient 2: Coefficient 16 = |Aut(E)|² = N_base²** (Theorem 4):

```
N_base = mult(A_{1g}) of O_h on the 27-voxel Moore neighborhood = 4
N_base² = 16 = master quadratic linear-coefficient prefactor
```

The 4 emerges from O_h (cubic point group) acting on the 3³ = 27 voxels of the Moore block. **3D representation theory** on a 3D voxel structure. Squaring N_base gives the 16.

### 1.2 · Combination

```
Master quadratic coefficient 16·G*² ≈ 140
        ↑           ↑
        |           +— from BCC Watson (3D BZ integral)
        +— from O_h trivial-irrep multiplicity squared (3D Moore neighborhood)
```

Both ingredients are 3D-structural. The master quadratic is **algebraically combined from these 3D ingredients**, but is not itself a literal 3D operator.

---

## 2 · Voxel-scale vs cluster-scale: distinct 2×2 systems

There are TWO distinct 2×2 (or larger) structures in FTD that are sometimes conflated. Keeping them separate is essential to the volumetric reading.

### 2.1 · Voxel-scale: 18-point Laplacian on natural 2-partitions

The 18-point Laplacian `L_27` (on 27 voxels) projected onto volumetric 2-partitions gives 2×2 matrices with **O(1) eigenvalues**:

| Partition | Eigenvalues |
|---|---|
| BCC (8 corners) vs non-BCC (19) | (−4.41, −1.69) |
| Polar (CENTER + BCC, 9) vs Equatorial (SC + FCC, 18) | (−4.70, −1.74) |
| Center (1) vs Shell (26) | (−4.27, −1.73) |

These are **microscopic per-voxel coupling scales** related to flux propagation inside the cluster.

**Notable:** The BCC-vs-non-BCC partition gives eigenvalue −4.41, **exactly one of the 4 A_{1g} eigenvalues** from the linear theorem (DERIV §3.1). So this partition picks up real spectral content — but at O(1) scale, not O(100).

### 2.2 · Cluster-scale: the master quadratic

The master quadratic has **O(100) eigenvalues** (137 and 3). These operate at the **whole-cluster macroscopic scale**, not per-voxel.

The master quadratic is NOT a literal sub-block of the per-voxel Laplacian. It emerges from 3D structure via the algebraic combination of G* (BCC Watson) and N_base² (O_h trivial-irrep), not via direct extraction of a voxel-level 2×2 sub-block.

### 2.3 · Why these scales differ

```
Voxel-scale (per-voxel coupling):  O(1)   ≈ 18-point Laplacian eigenvalues
Cluster-scale (master quadratic):  O(100) ≈ 1/α and N_c
```

These are **fundamentally different abstraction levels** in FTD's two-layer ontology:
- Per-voxel: flux dynamics on individual lattice sites
- Cluster-integrated: macroscopic mode structure across the whole cluster

The master quadratic operates at the cluster-integrated level; the 4×4 A_{1g} Laplacian operates at the per-voxel level.

---

## 3 · What the volumetric reading actually says

### 3.1 · Corrected statement

**The master quadratic emerges from 3D volumetric structure via:**

1. **Volumetric integration**: BCC Watson integral over the Brillouin zone gives G*.
2. **Symmetry decomposition**: O_h trivial-irrep multiplicity on the 27-voxel Moore neighborhood gives N_base = 4, hence 16 = N_base².
3. **Algebraic combination**: master quadratic is the polynomial in these ingredients (linear coefficient 16·G*², constant term 16·G*³).

**The "EM-color mixing" interpretation (commit `09a1569`) works at the algebraic level**, where the master quadratic's roots can be presented as eigenvalues of a 2×2 symmetric matrix. **But the volumetric mechanism** is through the BCC Watson integral and A_{1g} multiplicity — NOT a literal 2-partition of voxels.

### 3.2 · What this clarifies about the conjecture

The dual prediction `x_+ ≈ 1/α` and `x_- ≈ N_c` is the empirical IDENTIFICATION (FTD-0013, FTD-0014) of the master quadratic's roots with physical SM constants. The 2×2 mixing matrix interpretation provided one **algebraic picture** for this identification. The volumetric reading says:

> *EM coupling 1/α and color count N_c are not literally "two modes" in a 2D abstract space. They are algebraic projections of 3D volumetric structure: the BCC sublattice Watson integral (parameterized by G*) and the O_h trivial-irrep multiplicity on the 27-voxel Moore neighborhood (parameterized by N_base = 4). The master quadratic is the polynomial that combines these 3D ingredients into a form whose roots match the SM constants.*

This is structurally cleaner than the 2×2 abstract reading. It says the dual prediction is rooted in **specific 3D properties** of FTD's lattice (BCC sublattice + Moore neighborhood + O_h symmetry), not in an abstract 2-mode coupling.

---

## 4 · What this clarifies for Paper A

The 2×2 mixing matrix reading (commit `09a1569`) is useful as a **visual analogy** for non-physicists ("EM and color as bonding/antibonding modes"), but should be presented as such — an analogy, not a structural claim.

**The structural claim is volumetric:** the master quadratic emerges from specific 3D properties (BCC Watson + A_{1g} multiplicity), and the dual prediction is the empirical identification of its roots with SM constants.

For Paper A:
- Lead with the **volumetric structural origin** (Theorems 4, 5 + their 3D content)
- Present the master quadratic as the algebraic combination of these 3D ingredients
- Show the dual-prediction empirical match with explicit conjecture tag
- Use the 2×2 mixing matrix interpretation as a pedagogical aid where helpful, but not as a structural claim

This is the corrected scope.

---

## 5 · What this does NOT change

- The master quadratic theorem (FTD-0001) — still a theorem.
- The dual-prediction conjecture (FTD-0013, FTD-0014) — still [STRONGLY MOTIVATED CONJECTURE].
- The 2×2 mixing reading is still **mathematically equivalent** to the master quadratic — same eigenvalues, same characteristic polynomial.
- The structural-uniqueness scans (commits `0074f92`, `f36b741`) — still valid evidence for the dual-prediction's structural significance.

What changes is the **proper scope** of the 2×2 mixing matrix interpretation: it's an algebraic re-presentation, not a volumetric reading. The volumetric content is in the COEFFICIENTS, not the matrix entries.

---

## 6 · Open volumetric questions

The volumetric reading raises new questions worth exploring:

1. **What does 16 = |Aut(E)|² mean physically as a 3D quantity?** It's the squared multiplicity of A_{1g} on the Moore neighborhood. Squaring suggests a "two-copies" structure — perhaps left/right chirality, or particle/antiparticle, or two parity sectors.

2. **What does G* mean as a 3D quantity?** It's the BCC Watson integral's value (modulo 2π). The BCC sublattice is the body-diagonal-corner subset of the 27-block. G\* encodes the spectral content of the cubic lattice's BCC eigenvalue.

3. **Can the 3D content be made more concrete?** E.g., can we compute the master quadratic's eigenvalues *directly* from a 3D simulation that solves the BCC Watson integral and projects onto A_{1g}? This would convert the abstract algebra into a directly-computable volumetric quantity.

4. **Does the master quadratic's "scale jump" (O(1) voxel-scale → O(100) cluster-scale) have a clean physical mechanism?** The genesis cascade in the engine integrates per-voxel dynamics over the cluster volume; this is the natural amplification mechanism, but the precise relationship between voxel-scale Laplacian eigenvalues and cluster-scale master-quadratic eigenvalues hasn't been derived.

These are open follow-ups that pursue the volumetric reading further. None is session-tractable.

---

## 7 · LEDGER status

This document does NOT introduce a new LEDGER entry. It clarifies the SCOPE of the 2×2 mixing matrix reading (commit `09a1569`) and records the volumetric correction. Existing LEDGER tags unchanged.

---

## 8 · Single-line summary

**The master quadratic IS volumetric — through its coefficients' origin in the 3D BCC Watson integral (giving G*) and the O_h trivial-irrep multiplicity on the 27-voxel Moore neighborhood (giving 16 = N_base²) — but the 2×2 mixing matrix presentation in `EXPLR_MASTER_QUADRATIC_AS_MIXING_MATRIX.md` is abstract algebraic re-presentation, not a literal 3D 2-partition; voxel-scale 2-partitions of the 18-point Laplacian give O(1) eigenvalues distinct from the master quadratic's O(100) eigenvalues, demonstrating that the master quadratic operates at cluster-integrated scale rather than per-voxel scale, with the volumetric content concentrated in the coefficients (G* and N_base²) rather than in any literal sub-block decomposition.**

---

*End of brainstorm.*
