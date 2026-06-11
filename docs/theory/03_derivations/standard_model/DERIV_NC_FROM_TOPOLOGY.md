# N_c = 3 from the D = 3 Lattice (with Geometric and Topological Corroboration)

## One Fact, Several Lenses

**Date:** April 14, 2026
**Reframed:** 2026-05-30 (honesty pass — see §0 and Part II)
**Framework:** Foundational Ternary Dynamics v5.30
**Status:** [SELECTION] — N_c = 3 follows from D = 3 (color = spatial flux axis ⟹ gauge group SU(D) = SU(3)); the geometric/topological routes are corroboration of the *same* D = 3 fact, not logically independent proofs. [SELECTION] for the QCD identification.
**Depends on:** THEOREM_MOORE_LAYER_DECOMPOSITION.md, DERIV_BCC_MULTIPLICATIVE_STRUCTURE.md, DERIV_D3_FROM_AUTOMORPHISM.md, DERIV_MOORE_GAUGE_STRUCTURE.md, DERIV_CONFINEMENT_FROM_GAP_EQUATION.md

---

## §0 Epistemic Status (2026-05-30 reframe)

This document originally claimed an **over-determination [THEOREM]**: "four logically
independent routes converge on N_c = 3." Two auditors (adversarial + mathematical) flagged that
framing as the single most externally-exposed claim in the corpus, for two reasons:

1. **The routes are not logically independent.** Routes 1–3 are the spatial dimension D = 3
   counted in three ways (orthogonal axes / cuboctahedral face-diagonal planes / Wilson-loop
   winding sectors). Counting one fact several ways does **not** multiply the evidence, so the
   "negligible probability of chance convergence" argument does not hold. The honest reading,
   matching `DERIV_LATTICE_SU3_GAUGE.md` (SU3-2, Theorem 1.1), is **[SELECTION]**: given D = 3
   and the identification color = spatial flux axis, the gauge group is SU(D) = SU(3), hence
   N_c = 3; the geometric and topological observations corroborate this.
2. **Route 4 cited a retired LEDGER row as live.** The former "Route 4" derived N_c via
   `floor(x_-) = floor(3.024) = 3`. The `x_-  N_c` identification is **RETIRED** (LEDGER
   FTD-0014, removed in commit `ca7eb61`, per FTD/FQCR Cleanup Taxonomy v1.4 §5). `x_- ≈ 3.024`
   is a mathematical artifact of the master quadratic with no live physics identification.
   Route 4 and its associated claim NCT-7 are **deleted** (see the note where Route 4 stood).

What is genuinely correct and retained: the geometric content of each route (D = 3 orthogonal
axes; cuboctahedron with 3 face-diagonal planes; π_1(SU(N)/Z_N) = Z_N standard topology; the
exact algebra giving x_- = 3.024). What is corrected: the *epistemic framing* — these are one
D = 3 fact seen through several lenses, tagged **[SELECTION]**, not four independent **[THEOREM]**
derivations.

---

## Abstract

We argue that the number of color charges N_c = 3 is the value **selected** by the discrete
lattice ontology via D = 3, under the identification color = spatial flux axis. We present one
selection argument plus geometric/topological corroborations — **all expressions of the single
fact D = 3**, not independent proofs:

1. **Geometry:** the BCC sublattice has 3 mutually orthogonal flux axes (D = 3). *[SELECTION] — load-bearing route.*
2. **Group theory:** the cuboctahedron's face-diagonal decomposition has C(3,2) = 3 planes, and the relevant covering group π_1(SU(3)/Z_3) = Z_3 has order 3. *Corroboration — re-counts the same 3 axes.*
3. **Gauge topology:** Wilson-loop topological charges in the confined phase fall in N_c equivalence classes; counting on the FTD lattice gives 3. *Corroboration — same 3 axes as winding sectors.*

Routes 1–3 are **not** logically independent; they are the same D = 3 structure viewed through
several lenses. (A former arithmetic "Route 4" — `floor(x_-) = 3` — is **deleted**: the
`x_-  N_c` identification is RETIRED; see §0 and the Route 4 note.) The honest tag is therefore
**[SELECTION]**, not the over-determination [THEOREM] previously claimed. **What remains
[SELECTION] additionally is "is *this 3* the same 3 that QCD measures?"** — conditional on
compact lattice gauge theory recovering continuum QCD, a standard (but nontrivial) result of
Wilson's program (Wilson 1974; Creutz 1983).

---

## Part I: One D = 3 Fact, Seen Through Several Lenses

> **Framing (2026-05-30):** the routes below are not logically independent. Route 1 is the
> load-bearing [SELECTION] (D = 3 ⟹ SU(3)); Routes 2 and 3 re-count the *same* three spatial
> axes geometrically and topologically. The geometric content of each is correct; only the prior
> claim of mutual independence is withdrawn. A former arithmetic Route 4 (`floor(x_-) = 3`) is
> deleted — see the Route 4 note.

### Route 1: Spatial Axes (Geometric) — [SELECTION], the load-bearing route

The BCC sublattice consists of the 8 Moore neighbors at offsets (±1, ±1, ±1). By the Moore Layer Theorem (THEOREM_MOORE_LAYER_DECOMPOSITION.md, Theorem 1), this is the k = D = 3 layer of the ternary D-cube: the unique layer whose neighbors have **all D coordinates nonzero**. By Theorem 2 (J-component counting) of the same document, a neighbor at offset δ excites flux component J_μ if and only if δ_μ ≠ 0. Therefore each BCC neighbor excites all D components simultaneously.

**Definition (Color label):** Assign to a BCC-sublattice excitation the ℤ_D-valued label given by its dominant flux axis. There are exactly D such axes, one per coordinate direction.

**Consequence:** The number of distinct color labels equals D, the spatial dimensionality of the lattice. Combined with the independent fact (DERIV_D3_FROM_AUTOMORPHISM.md) that **D = 3 is forced** — by Watson-integral self-consistency *and* by the algebraic constraint |Aut(E_i)|² = 2^D · (D−1)! — the color count is

> N_c (geometric) = D = 3.

This route uses only counting and the dimensional-uniqueness theorem. No gauge theory is invoked.
The step from "D orthogonal axes" to "N_c color charges" rests on the identification color =
spatial flux axis (`DERIV_LATTICE_SU3_GAUGE.md`, Theorem 1.1), which is **[SELECTION]** — hence
this route, as a source of N_c, is [SELECTION], not [THEOREM]. (The *geometric* sub-facts it uses
— the BCC layer has D nonzero-coordinate neighbors, D = 3 is forced — remain [THEOREM] in their
home documents.)

### Route 2: Cuboctahedral Subgroup Structure (Group-Theoretic) — corroboration of D = 3, not an independent proof

> **Framing (2026-05-30):** the 3 face-diagonal planes counted here are organized by choosing 2
> of the *same* 3 coordinate axes of Route 1. This is the D = 3 fact viewed group-theoretically,
> not a logically independent derivation of N_c. The counting below is correct; its independence
> is disclaimed.

The k = 2 layer of the Moore neighborhood is the cuboctahedron (Theorem 3 of THEOREM_MOORE_LAYER_DECOMPOSITION.md). It has 12 vertices, organized by the choice of which 2 of D coordinate axes a face-diagonal lies in: C(3,2) = **3** mutually orthogonal face-diagonal planes (Theorem 5).

The 3-fold symmetry that permutes these planes is the order-3 cyclic subgroup C_3 ⊂ O_h. This subgroup is identified group-theoretically as

> π_1(SU(3) / Z_3) = Z_3,

where Z_3 is the center of SU(3). The cardinality |Z_3| = 3 is the **same integer** that classifies face-diagonal planes of the cuboctahedron. This is not a coincidence: the cuboctahedron is the natural geometric realization of the SU(3) weight lattice's C_3 sub-symmetry on a 3-cube.

**[THEOREM]** The Moore-neighborhood cuboctahedron has exactly 3 face-diagonal planes.
**[THEOREM]** The center of SU(3) has order 3.
**[SELECTION]** These two 3's name the same physical structure.

The selection — that the geometric C_3 is identified with the SU(3) center, not merely an order-3 group — is what carries this route from "an integer 3 appears" to "the color group SU(3) appears". It is the same selection identified in MGS-3 of DERIV_MOORE_GAUGE_STRUCTURE.md.

### Route 3: Wilson-Loop Topological Quantization (Gauge-Theoretic) — corroboration of D = 3, not an independent proof

> **Framing (2026-05-30):** the winding-sector count below reduces (line "the closure condition
> ... forces the count ... to equal the number of independent axes, D = 3") to the *same* 3 axes
> as Route 1. The standard topology π_1(SU(N)/Z_N) = Z_N is correct and remains [THEOREM] as
> standard mathematics; but as a route to *N_c in FTD* it re-expresses D = 3, so it is
> corroboration, not a logically independent derivation. (Note: this route mentions
> β = x_- = 3.024 only as the confined-phase coupling value; it does **not** use the retired
> `x_-  N_c` identification — the color count here comes from axis-counting, not from `floor(x_-)`.)

In compact lattice gauge theory at the confined-phase coupling β = x_- = 3.024, Wilson loops obey the area law σ(x_-) = −ln(I_1(x_-)/I_0(x_-)) ≈ 0.209 > 0 (DERIV_CONFINEMENT_FROM_GAP_EQUATION.md, Theorem 2.1). For a non-Abelian gauge group G, the topological charge of a Wilson loop is classified by

> π_1(G/Z(G)),

where Z(G) is the center. For G = SU(N), this is Z_N, giving N distinct topological classes. **The number of confined-phase topological charges equals N.**

On the FTD Moore lattice, the only sublattice supporting Wilson loops that wrap all three flux directions simultaneously is the BCC sublattice (DERIV_BCC_MULTIPLICATIVE_STRUCTURE.md, Section 2.1). The BCC structure factor is the multiplicative product 1 − cos k_1 cos k_2 cos k_3, and the corresponding gauge field has D = 3 independent flux components J_x, J_y, J_z.

Counting the inequivalent winding sectors of these 3 flux components in the confined phase:

- a flux line in axis-x can be unwound only against an axis-y or axis-z partner;
- the closure condition (a Wilson loop must return to the origin) forces the count of independent topological sectors to equal the number of independent axes, **D = 3**.

This is the lattice realization of π_1(SU(3)/Z_3) = Z_3: three axes, three classes, three colors.

> N_c (Wilson loop) = 3.

The construction is honest only insofar as compact lattice gauge theory of the BCC sublattice recovers SU(3) gauge-theory observables for arbitrarily fine spacing a, with the standard error rate O(a^2). That the BCC sublattice is the *correct* lattice realization of SU(3) is the content of DERIV_BCC_MULTIPLICATIVE_STRUCTURE.md (BMS-4) and is [THEOREM] for the counting, [SELECTION] for the named gauge group.

### Route 4: REMOVED — the master-quadratic `floor(x_-)` route is RETIRED

A former "floor(x_-) = floor(3.024) = 3" route is intentionally removed: the x_-  N_c
identification is RETIRED (FTD-0014, commit `ca7eb61`); x_- ≈ 3.024 is a mathematical artifact
of the master quadratic and N_c = 3 is sourced independently here (Route 1, D = 3).

> The arithmetic that x_- = 8 G*² − 4 G*^(3/2) √(4G* − 1) = 3.023964... is exact algebra and is
> preserved (as a property of the master quadratic) in `MATH_MASTER_QUADRATIC.md` and the
> algebraic-spine documents. What is deleted is only the *physics identification* of that root
> with the color number, and the [THEOREM] claim built on it (former NCT-7). The retired status
> is canonical: see LEDGER FTD-0014 (removed), `DERIV_LATTICE_SU3_GAUGE.md` §1.4, and the
> FTD/FQCR Cleanup Taxonomy v1.4 §5.

---

## Part II: One Fact, Several Lenses (the over-determination claim, withdrawn)

> **Correction (2026-05-30).** This section previously argued that four *independent* routes
> converging on 3 is "not credible as accident," upgrading N_c = 3 to [THEOREM]. That argument is
> **withdrawn**: the routes are not independent. Routes 1–3 all reduce to the single spatial fact
> D = 3 (orthogonal axes; the same axes taken 2-at-a-time as face-diagonal planes; the same axes
> as Wilson-loop winding sectors). Counting one fact three ways does not multiply the evidence,
> so there is no over-determination and no licence to upgrade above [SELECTION]. The former
> fourth route (`floor(x_-)`) is deleted as a RETIRED identification (§0, Route 4 note).

The routes are three views of the same D = 3 structure:

| Route | Mathematical object | Branch | Returns | Independent? |
|-------|---------------------|--------|---------|--------------|
| 1 | BCC orthogonal axes | Geometry / linear algebra | D = 3 | load-bearing [SELECTION] |
| 2 | Cuboctahedron C_3 ⊂ O_h | Finite group theory | \|Z_3\| = 3 | no — re-counts the 3 axes |
| 3 | π_1(SU(3)/Z_3) wrapping | Algebraic topology of Lie groups | 3 | no — same 3 axes as winding sectors |
| ~~4~~ | ~~Roots of x² − Kx + KG* = 0~~ | ~~Number theory~~ | ~~floor(3.024) = 3~~ | **DELETED — RETIRED identification (FTD-0014)** |

Because Routes 2–3 re-express Route 1, the honest reading is that N_c = 3 is **the value selected
by D = 3** under the identification color = spatial flux axis, with geometric and topological
corroboration. The integer 3 is therefore **[SELECTION]**, not a separately-established
topological invariant. Whether the color = spatial-direction identification is itself *forced*
(which would lift this to a theorem) is **[OPEN]**.

(Contrast with the genuine D = 3 result in `DERIV_D3_FROM_AUTOMORPHISM.md`, where two *genuinely
independent* constraints — Watson-integral self-consistency and the algebraic identity
|Aut(E)|² = 2^D·(D−1)! — agree. That is what real over-determination looks like; the routes here
do not have that structure.)

---

## Part III: What This Document Establishes

> **Correction (2026-05-30).** This section previously claimed the document *upgrades* N_c = 3
> from [SELECTION] to [THEOREM] via over-determination. That upgrade is **withdrawn** (Part II).
> The document establishes a [SELECTION], with corroboration — not a theorem.

**What this document establishes:**

- **N_c = 3 [SELECTION]**: given D = 3 and color = spatial flux axis, the gauge group is
  SU(D) = SU(3), so N_c = 3. (Canonical statement: `DERIV_LATTICE_SU3_GAUGE.md`, Theorem 1.1 /
  SU3-2.)
- The geometric (3 orthogonal axes / 3 cuboctahedral face-diagonal planes) and topological
  (π_1(SU(3)/Z_3) = Z_3) facts are correct and **corroborate** this same D = 3 fact; they are
  not independent derivations.

**What this document does NOT establish:**

- It does **not** upgrade N_c = 3 to [THEOREM] — the routes are not independent, so there is no
  over-determination.
- It does **not** use the master-quadratic root. The former `floor(x_-)` route is deleted; the
  `x_-  N_c` identification is RETIRED (FTD-0014, commit `ca7eb61`). The exact value
  x_- = 3.024 remains a property of the master quadratic with no live physics reading.

---

## Part IV: What Remains [SELECTION]

This document does **not** prove that the integer 3 derived above is identically the QCD color number measured experimentally. The remaining [SELECTION] step is:

> **The lattice gauge group whose topological-charge classification gives 3 is the same SU(3)_color whose representation theory governs hadronic matter.**

This identification is conditional on:

1. **Wilson's continuum-limit conjecture:** compact SU(3) lattice gauge theory in 3+1 dimensions recovers continuum QCD as the lattice spacing a → 0 with the appropriate scaling of the bare coupling. This is standard but unproven in full mathematical rigor (Yang–Mills mass gap is a Millennium Prize problem).
2. **Coupling identification:** the strong coupling at the confined-phase scale must match the measured α_s. From x_- = 3.024 we read α_s(lattice) = 1/x_- ≈ 0.331, which matches measured α_s at scales ≈ 1–2 GeV (PDG: α_s(M_τ) ≈ 0.32). This match is good but inherits any uncertainty in the lattice-to-continuum mapping.
3. **Hadronic spectrum match:** baryons are 3-quark singlets, mesons are quark–antiquark, and the FTD picture (DERIV_MOORE_GAUGE_STRUCTURE.md §5–8) reproduces these structurally (J² saturation requires 3 orthogonal components → baryons stable, mesons unstable).

Items 1–3 collectively comprise what we call the **QCD identification**. Each is well-supported by standard physics, but none is proved from FTD alone. The honest tag is therefore: **N_c = 3 in the FTD lattice is [SELECTION] (from D = 3 under color = spatial flux axis); its identification with QCD's color number is additionally [SELECTION] of standard scope.**

There are thus *two* [SELECTION] steps, not the single one previously claimed: (a) "is the lattice integer 3?" — [SELECTION], from D = 3 (the geometric/topological routes corroborate but do not independently prove it); and (b) "is *this* 3 the same as QCD's 3?" — [SELECTION], the standard lattice-gauge-theory question. The 2026-05-30 reframe withdrew the claim that step (a) had been promoted to [THEOREM].

---

## Claims Table

> **2026-05-30:** NCT-7, NCT-8 and the [THEOREM] tag on NCT-9 are corrected below. The geometric
> sub-facts (NCT-1, -2, -4, -5, -6) remain correct in their home documents; what changes is the
> *N_c-level* claim, which is [SELECTION], not [THEOREM].

| ID | Claim | Status |
|----|-------|--------|
| NCT-1 | BCC sublattice has D = 3 orthogonal flux axes | [THEOREM] (geometric sub-fact) |
| NCT-2 | Cuboctahedron has C(3,2) = 3 face-diagonal planes | [THEOREM] (geometric sub-fact) |
| NCT-3 | C_3 ⊂ O_h is identified with Z_3 = Z(SU(3)) | [SELECTION] |
| NCT-4 | π_1(SU(N)/Z_N) = Z_N gives N topological classes | [THEOREM] (standard mathematics) |
| NCT-5 | BCC sublattice supports Wilson loops with 3 winding classes | [THEOREM] given BMS-4 (geometric sub-fact) |
| NCT-6 | x_- = 3.024 is exact algebraic consequence of D = 3 + W_3 + k_phys = 16 | [THEOREM] (pure algebra; no physics identification of the root) |
| ~~NCT-7~~ | ~~floor(x_-) = 3 is forced by integer quantization of color~~ | **RETIRED / DELETED** — relies on the retired x_-  N_c identification (FTD-0014, commit `ca7eb61`) |
| ~~NCT-8~~ | ~~Four independent routes return the same integer 3 (over-determination)~~ | **WITHDRAWN** — Routes 1–3 are not independent; they re-count the same D = 3 fact |
| NCT-9 | N_c = 3 in the FTD lattice (from D = 3, color = spatial flux axis) | **[SELECTION]** (corrected from [THEOREM] 2026-05-30; corroborated by NCT-1/-2/-4/-5, not independently proven) |
| NCT-10 | This integer is identically QCD's color number | [SELECTION] (Wilson, fine-spacing recovery) |

---

## Cross-References

- [THEOREM_MOORE_LAYER_DECOMPOSITION.md](../08_structural/THEOREM_MOORE_LAYER_DECOMPOSITION.md) — Routes 1 and 2 foundation
- [DERIV_BCC_MULTIPLICATIVE_STRUCTURE.md](../08_structural/DERIV_BCC_MULTIPLICATIVE_STRUCTURE.md) — BCC sublattice and SU(3)
- [DERIV_D3_FROM_AUTOMORPHISM.md](../02_foundations/DERIV_D3_FROM_AUTOMORPHISM.md) — D = 3 from Watson integral self-consistency and |Aut(E_i)|² = 16
- [DERIV_MOORE_GAUGE_STRUCTURE.md](DERIV_MOORE_GAUGE_STRUCTURE.md) — Gauge group from sublattice
- [DERIV_CONFINEMENT_FROM_GAP_EQUATION.md](../foundational_mechanics/DERIV_CONFINEMENT_FROM_GAP_EQUATION.md) — Confined phase Wilson loops
- [SPEC_FTD_COMPLETE_CHAIN.md](../01_reference/SPEC_FTD_COMPLETE_CHAIN.md) — §2.2 (the [SELECTION] statement of N_c = 3; this document corroborates it, and per the 2026-05-30 reframe does **not** upgrade it to [THEOREM])
- [DERIV_LATTICE_SU3_GAUGE.md](DERIV_LATTICE_SU3_GAUGE.md) — §1.4 + Theorem 1.1 + SU3-2: the canonical [SELECTION] statement N_c = 3 from D = 3, and the canonical retirement of the x_-  N_c identification (FTD-0014)

## Computation

- `scripts/proofs/proof_d3_uniqueness.py` — Route 1 numerical confirmation (Watson integral)
- ~~`scripts/proofs/proof_integer_identification.py`~~ — was Route 4 (master quadratic root and integer floor); route **DELETED** (RETIRED x_-  N_c identification, FTD-0014). The script, if it still computes x_-, computes a pure-algebra property of the polynomial, not a source of N_c.
- `scripts/proofs/proof_confinement_wilson.py` — Route 3 (Wilson loop area law at x_-) — corroboration only
- `scripts/proofs/proof_moore_gauge_structure.py` — Route 2 (cuboctahedron and gauge groups) — corroboration only

---

## Honest Accounting

**This document is bookkeeping, not a new computation.** Every individual geometric/topological
sub-fact (NCT-1, -2, -4, -5, -6) already exists in the FTD corpus and is correct in its home
document. What this document does is collect them around the value N_c = 3.

**Correction (2026-05-30).** This section previously claimed the document demonstrates "four
logically independent paths converge on the same integer," licensing an upgrade from [SELECTION]
to [THEOREM]. **That claim is withdrawn.** The paths are not independent — Routes 1–3 all re-count
the single spatial fact D = 3, and the former fourth path (`floor(x_-)`) relied on the RETIRED
x_-  N_c identification (FTD-0014, commit `ca7eb61`). Counting one fact several ways does not
multiply the evidence, so there is no over-determination and no licence to upgrade. The N_c-level
claim is **[SELECTION]** (from D = 3 under color = spatial flux axis), corroborated by the
geometric/topological sub-facts.

**What this does NOT do:** (i) upgrade N_c = 3 above [SELECTION] — the routes are not
independent; (ii) source N_c from the master quadratic — that route is deleted; (iii) prove that
compact SU(3) lattice gauge theory recovers continuum QCD (Wilson's program; Yang–Mills mass gap
is the Clay problem). The QCD identification inherits whatever uncertainty attaches to that
program — no more, no less.
