# Tier B of the G* Opus Follow-up: T-B1 and T-B2 Closed Negative

**Status:** [CLOSED NEGATIVE]
**Date:** 2026-05-19
**Companion:** G* opus follow-up Phase 1, lemma L7, targets T-B1/T-B2. Spec: `docs/superpowers/specs/2026-05-19-gstar-followup-attacks-design.md`.
**Investigation basis:** L7 audit of FTD-0110 and FTD-0122 source documents.

---

## §1 — The Tier B targets and why they were proposed

After the G* opus follow-up Tier A was completed (T-A1 established, T-A2 [THEOREM], T-A3 [DERIVED]), the remaining Tier B targets hoped to extend the pure-math integer-4 unification (T-A2/FTD-0181) into FTD's engine layer.

**Target T-B1** aspired to show that the FTD-engine-side `N_base = 4` — the multiplicity `mult(A_{1g}) = 4` of the trivial O_h-irrep in the 27-block Laplacian eigenspace (FTD-0110) — is "the same 4" as the pure-math Q(i)-arithmetic 4's established in T-A2. The spec described the claimed mechanism as a canonical map `27-block → Z[BCC] ⊗ Q → V_triv² ⊕ V_sign² ⊕ V_complex²`, asserting that the trivial-O_h-rep multiplicity 4 in the 27-block coincides with `rank_Z(V_complex) = 4` from FTD-0122.

**Target T-B2** was downstream of T-B1: if N_base = 4 traces to the same Z[i]²-module structure, then the engine cluster coefficient `k = 1/N_base = 1/4` (FTD-0110) would trace to the same Q(i)-arithmetic source.

Both were [SMC]-grade aspirations in the spec — not yet theorems, but proposed as structurally motivated. The L7 investigation examined the primary source documents (FTD-0110, FTD-0122) and found both targets are **numerical coincidences**, not structural bridges.

---

## §2 — T-B1 closed negative

**The finding:** `mult(A_{1g}) = 4` in the 27-block is the **O_h-orbit count** of the 3³ Moore neighborhood — specifically, the number of distinct shell-types (orbits) that O_h acts on: center (1 voxel), SC face-neighbours (6 voxels), FCC edge-neighbours (12 voxels), BCC corner-neighbours (8 voxels). This is a crystallographic invariant of the cubic lattice with no intrinsic Q(i) content.

Three independent confirmations:

**Confirmation 1 — the 1+1+1+1 decomposition.** The 27-block's A_{1g}-multiplicity is `1 + 1 + 1 + 1` across the four O_h-orbits {center, SC faces, FCC edges, BCC corners}, each orbit contributing exactly one A_{1g} copy (the constant function on its orbit). The BCC sublattice — the only sublattice that FTD-0122's complex-structure theorem touches — contributes exactly **one** A_{1g} copy, not 4. The BCC's A_{1g}-content (1) cannot equal `dim_Z(V_complex) = 4`.

**Confirmation 2 — orthogonal isotypic summands.** FTD-0110's `mult(A_{1g}) = 4` is a **trivial-isotypic** dimension: it counts how many copies of the trivial O_h-representation appear in the 27-block Laplacian's eigenspace. FTD-0122's `dim_Z(V_complex) = 4` is a **complex-isotypic** dimension: it counts the Z-rank of the sub-representation where J (90° rotation) acts as multiplication by i. These two dimensions live in orthogonal isotypic summands of the decomposition `Z[BCC] ⊗ Q = V_triv² ⊕ V_sign² ⊕ V_complex²`. An A_{1g}-multiplicity (trivial-rep count) cannot structurally equal a complex-isotypic rank.

**Confirmation 3 — FTD-0122's own source document.** The source document `DERIV_BCC_COMPLEX_STRUCTURE.md` (§3.2–§3.3, FTD-0122) enumerates four Roles of the integer 4 in FTD's algebraic spine and explicitly states the following about Role 4 (= `mult(A_{1g})`):

> **Role 4**: The 27-block decomposes under O_h into 4 orbits {center, face-neighbours, edge-neighbours, vertex-neighbours} of sizes (1, 6, 12, 8). Because the four orbits have *different sizes*, no group can permute them into each other in a non-trivial way — the only Z[i]^× action on the 4-orbit set is the trivial one. The count "4" here is the number of distinct orbit-types, a structural feature of the cube but **not** a manifestation of |Z[i]^×|.

And the §3.3 net assessment states:

> The dual-4 framework reduces to: **two genuine occurrences of the same Z[i] structure** (Roles 1 and 3), unified by the BCC complex-structure theorem. **Two further occurrences of the integer 4** (Roles 2 and 4) that are *order or count coincidences* with |Z[i]^×|, lacking any group-theoretic homomorphism that identifies them.

The published verdict is therefore **"dual-4 unifies as 2 + 2"** (Roles 1+3 genuine Z[i] structure, Roles 2+4 coincidences). T-B1 asserts exactly the bridge that FTD-0122 disclaims — it contradicts the document it cites.

**[CLOSED NEGATIVE].**

---

## §3 — T-B2 closed negative

T-B2 was downstream of T-B1. The spec's logic was: if N_base = 4 traces to Z[i]-module structure (T-B1), then k = 1/N_base = 1/4 traces to the same source.

With T-B1 closed negative, the chain breaks at the first step. The engine coefficient `k = 1/4` is simply the reciprocal of the crystallographic orbit count N_base = 4 — not a Q(i)-arithmetic quantity. There is no Z[i]-module whose rank, unit order, or discriminant equals 1/4; the connection is k = 1/(number of O_h-orbits in the 3³ Moore block), a pure cube-geometry statement.

**Additional caveat.** Even setting aside T-B1, FTD-0110's `k = 1/4` derivation is itself only [DERIVED at linear level]: the 2026-05-04 empirical campaign (SPEC_FTD.md §5.6.21–§5.6.27, FTD-0136) found that `gauss_projection` destroys local 27-block A_{1g} purity in the full-physics nonlinear regime, so FTD-0110 is `[DERIVED at linear level only; nonlinear closure OPEN]`. Any T-B2 claim would be building on a foundation that is itself not yet settled in the regime where the engine actually runs.

**[CLOSED NEGATIVE].**

---

## §4 — The corrected four-class taxonomy of the integer 4

Combining L6 (`DERIV_INTEGER_4_UNIFICATION.md`, three classes, FTD-0181) with this L7 finding, the full taxonomy of every "4" in the FTD lemniscatic and engine sectors is:

**Class (a) — unit-derived.**
`|Z[i]^×| = |Aut_geom(E_lemn)| = |μ_4| = 4`, where μ_4 is the group of 4th roots of unity. The associated `16 = |μ_4|²` is the master-quadratic integer coefficient.
*Q(i)-arithmetic: YES. Specific to K = Q(i): YES.*

**Class (b) — discriminant-derived.**
Conductor of χ_{−4} equals `|disc(Q(i))| = 4`; (1+i)-tower level `k = 4` equals `|(1+i)⁴| = |−4| = 4`.
*Q(i)-arithmetic: YES. Specific to K = Q(i): YES.*

**Class (c) — module-rank.**
`dim_Z(V_complex) = dim_Z(Z[i]²) = 4 = 2·[Q(i):Q]`, where V_complex is the complex-structure-carrying sub-representation in the BCC decomposition (FTD-0122).
*Q(i)-arithmetic: YES. Specific to K = Q(i): NO — generic to all imaginary quadratic CM fields.*

**Class (d) — crystallographic orbit-count.**
`mult(A_{1g}) = 4` = number of distinct O_h-orbits in the 3³ Moore block (center, SC faces, FCC edges, BCC corners); `N_base = 4`; engine cluster coefficient `k = 1/4`.
*Q(i)-arithmetic: NO. No group-theoretic homomorphism to Z[i]^× or any Q(i)-arithmetic invariant. Pure cubic lattice geometry.*

**The unification statement (updated):** Classes (a), (b), (c) are Q(i)-arithmetic. Classes (a) and (b) are specifically characteristic of Q(i) — the unique imaginary quadratic field with `|μ_K| = |disc(K)|` (FTD-0181 [THEOREM]). Class (c) is generic to all imaginary quadratic CM. Class (d) is crystallographic and is **not** connected to the Q(i)-arithmetic classes; it shares the integer 4 by numerical coincidence only.

---

## §5 — Epistemic status and why this is the correct outcome

**[CLOSED NEGATIVE].**

This is the second over-claim caught in the G* opus follow-up Tier work. The first was the T-A2 draft (corrected in L6/FTD-0181: the entry `rank_Z(H¹(E_lemn)) = 4` was factually wrong, and the spec's "single module M" formulation was over-stated). In both cases, the correction came from careful reading of the primary source documents — not from a new computation or external critique.

The closure is not a failure of the research program. It is the epistemic-rigor discipline functioning exactly as intended: the spec proposed [SMC]-grade bridges, the careful L7 audit found that FTD-0122's own source already disclaims the T-B1 connection, and the honest outcome is recorded. Tier A (T-A1, T-A2, T-A3) is unaffected and remains closed positive.

Preserving this closed-negative finding is mandated by CLAUDE.md's Documentation Cleanup Discipline: "Closed-negative findings are documented to prevent zombie re-emergence." The specific prevention value here is that `mult(A_{1g}) = 4` and `|Z[i]^×| = 4` will always share the same integer, and future investigators may be tempted to attempt the T-B1 bridge again. This document records that the attempt was made, the primary source (FTD-0122 §3.2–§3.3) explicitly rules it out, and the three structural reasons are enumerated.

---

## §6 — Cross-references

| Entry | Content |
|---|---|
| FTD-0110 | `DERIV_K_FROM_OH_A1G_MULTIPLICITY.md` — cluster-size derivation; `mult(A_{1g}) = 4` first derived; linear-level only |
| FTD-0122 | `docs/theory/09_mathematical/DERIV_BCC_COMPLEX_STRUCTURE.md` §3.2–§3.3 — the disclaiming passages; Role 4 = count coincidence; "dual-4 unifies as 2+2" |
| FTD-0181 | `DERIV_INTEGER_4_UNIFICATION.md` — L6 three-class taxonomy (a)(b)(c); this document adds class (d) |
| FTD-0182 | `DERIV_CONJECTURE_16_5_2_CLOSURE.md` — T-A3 closure; part of Tier A |
| FTD-0183 | LEDGER row for this closure (this document) |
| Tier A | T-A1 (Sym^k eigenline decomposition, L2/L3), T-A2 (integer-4 unification, L6/FTD-0181), T-A3 (Conjecture 16.5.2 closure, L4+L5/FTD-0180+FTD-0182) — all closed positive; Tier B closure does not affect Tier A |
