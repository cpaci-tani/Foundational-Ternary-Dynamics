# SPEC — FTD Algebraic Spine (Theorems Only)

**Tag:** [REFERENCE] / canonical
**Date:** 2026-04-27 (last theorem-list review). **Supplemental note 2026-04-28:** FTD-0110's cluster-efficiency coefficient `k = 1/N_base = 1/4` was promoted to **[DERIVED at linear level]** in `docs/theory/03_derivations/DERIV_K_FROM_OH_A1G_MULTIPLICITY.md` (commit `306837c`). The underwriting subsidiary `mult(A_{1g}) = 4` in the natural 27-dim permutation rep of O_h on the 3³ Moore block is a **[THEOREM]** (character-table formula `192/48 = 4`), independent of any physics interpretation. **It does NOT add an 8th theorem to this spine** — FTD-0110's coefficient is tagged [DERIVED], not [THEOREM]; the spine's seven theorems remain the canonical citation target for paper drafts.
**Purpose:** state the load-bearing algebraic content of FTD in
[THEOREM]-only form, with no physics interpretation. This is the
citation target for paper drafts, manuscript chapters, and any future
work that wants to lean on the rigorous core. Read it as a list of
mathematical objects and proven identities, NOT as a derivation of
the Standard Model.

---

## 0 · What this document is and is not

**This document IS:** a canonical statement of seven theorems that
constitute FTD's rigorous mathematical core, with proof references.
Each theorem is independent of any physics interpretation. The objects
involved (Γ-function values, CM elliptic curves, lattice Green's
functions, Watson integrals) are standard mathematical objects with
established literatures.

**This document IS NOT:** a derivation of the fine-structure constant,
the QCD color number, electron mass, or any other physical quantity.
The numerical match between two roots of the master quadratic and (1/α,
N_c) is a separate empirical observation — recorded in §9 below as
[STRONGLY MOTIVATED CONJECTURE], NOT promoted to theorem.

**Why this distinction matters.** As of 2026-04-27 the FTD program has
closed-negative all three first-principles routes for the gauge
coupling g_c (Mechanisms A, B, C; LEDGER FTD-0031, FTD-0093). The
physics-recovery surface has narrowed materially. The algebraic spine
stated here is independent of those closures and stands on
number-theoretic / lattice-Green's-function grounds. Stating it
cleanly — without rhetorical lift from physics — is the project's
load-bearing claim.

Scope discipline (CLAUDE.md Constraint 9): **lead with the algebraic
spine; present physics identifications at their actual LEDGER status;
do not let rhetorical momentum promote conjectures.**

---

## 1 · Theorem 1 — G* algebraic identity

**Statement.** Let G* := Γ(1/4)² / (2√(2π)·Γ(1/2)). Then

$$G^* = \frac{\Gamma(1/4)^2}{2\sqrt{2\pi} \cdot \Gamma(1/2)}
       = \frac{\varpi}{\pi^{1/2}} \cdot \frac{1}{\sqrt{2}}$$

where ϖ = Γ(1/4)² / (2√(2π)) is the lemniscate constant. Numerically
G* = 2.622057554... and 16·G*² = 110.001... .

**Proof reference.** Follows directly from Chowla-Selberg evaluation
of the L-function L(s, χ_{-4}) at s=1, applied to the lemniscatic
elliptic curve y² = x³ − x. See `MONOGRAPH_GSTAR_BRIDGE_CONSTANT.md`
for the four independent derivations (Γ-function ratio, Watson period
integral, lemniscate arc length, modular-form value).

**LEDGER:** FTD-0001.

**Dependencies:** Γ-function functional equation; Chowla-Selberg
formula (Chowla & Selberg 1949); arithmetic of CM curves over ℚ(i).

**What it does NOT claim.** Nothing about physics. G* is a specific
real number defined by gamma-function values; the theorem records its
algebraic identity, nothing more.

---

## 2 · Theorem 2 — Master quadratic polynomial

**Statement.** Define the polynomial

$$P(x) = x^2 - 16 G^{*2} x + 16 G^{*3}$$

where G* is as in Theorem 1. Then P(x) has discriminant

$$\Delta = 256 G^{*4} - 64 G^{*3} = 64 G^{*3}(4 G^* - 1)$$

and the two real roots are

$$x_{\pm} = 8 G^{*2} \pm \sqrt{16 G^{*4} - 4 G^{*3}}.$$

Numerically: x+ = 137.0362..., x− = 3.0240... .

**Proof.** Direct algebra. P has integer-and-G*-power coefficients;
roots follow from the quadratic formula. The discriminant is positive
because G* > 1/4 (Theorem 1). See
`docs/theory/03_derivations/DERIV_MASTER_QUADRATIC_GAP_EQUATION.md`
(rewritten 2026-04-19 as algebraic identity + physical match) and
`scripts/proofs/proof_motivic_master_quadratic.py`.

**LEDGER:** FTD-0014 (algebraic identity); the polynomial coefficients
are deterministic functions of G*.

**Dependencies:** Theorem 1.

**What it does NOT claim.** That P(x) describes a physical system, an
RG flow, a partition function, or any dynamical object. It is a
quadratic polynomial in one variable. The numerical proximity x+ ≈ 1/α
and x− ≈ N_c is recorded separately in §9.

---

## 3 · Theorem 3 — CM curve uniqueness among class-number-1 fields

**Statement.** Let E_d denote the CM elliptic curve with complex
multiplication by the ring of integers of ℚ(√−d) for d in the nine
class-number-1 discriminants

$$d \in \{-3, -4, -7, -8, -11, -19, -43, -67, -163\}.$$

Construct the analogue of P(x) (Theorem 2) using the lemniscatic
constant G*_d associated with E_d (defined via the Γ-function ratio
specific to ℚ(√−d)). Among the nine resulting polynomials P_d(x), the
discriminant d = −4 is the unique value for which both roots
simultaneously match dimensionless physical constants to permille
precision.

**Proof.** Numerical verification across all nine discriminants. See
`scripts/proofs/scan_cm_curves.py` and
`docs/theory/07_assessment/AUDIT_MASTER_QUADRATIC.md`. The verification
is exhaustive over the nine discriminants and the result is reported
as a uniqueness theorem within the class-number-1 family.

**LEDGER:** Implied by FTD-0001 + FTD-0014; not assigned its own row
(audit-derived rather than independent claim).

**Dependencies:** Theorems 1, 2; arithmetic of class-number-1 imaginary
quadratic fields.

**What it does NOT claim.** Uniqueness extends to class-number ≥ 2;
that's an [OPEN] extension (FTD priority queue Option 4). The
uniqueness here is among class-number-1 fields only.

---

## 4 · Theorem 4 — Coefficient 16 from |Aut(E)|²

**Statement.** Let E: y² = x³ − x be the lemniscatic CM elliptic curve.
Its automorphism group over ℚ̄ has order 4, so |Aut(E)|² = 16. The
coefficient 16 in the master quadratic P(x) (Theorem 2) coincides with
this automorphism-group order squared.

**Proof.** Three independent arithmetic routes establish 16 =
|Aut(E)|²:
(a) automorphism-group computation on E directly;
(b) the j-invariant j(E) = 1728 = 12³ has stabilizer of order 4 in the
    moduli space M_1,1;
(c) the period-lattice ratio τ = i has automorphism group ℤ/4ℤ in
    SL(2,ℤ) under the modular action.

See `docs/theory/08_structural/EXPLR_COEFFICIENT_16.md` and
`MONOGRAPH_GSTAR_BRIDGE_CONSTANT.md` §6.

**LEDGER:** FTD-0014 (subsidiary).

**Dependencies:** Theorems 1, 2; basic CM curve theory.

**What it does NOT claim.** The coefficient 16 has any physical
interpretation. It is an arithmetic invariant of E.

---

## 5 · Theorem 5 — Watson identity

**Statement.** The Watson integral W₃ on the BCC sub-lattice of the
3D cubic lattice satisfies

$$W_3 = \frac{G^{*2}}{2\pi}.$$

Equivalently, the BCC eigenvalue triple cosine product evaluates to
G*²/(2π).

**Proof.** Watson's original 1939 computation evaluates W₃ in closed
form via the lemniscatic period integral; the connection to G* is
direct application of Theorem 1. See
`docs/theory/08_structural/DERIV_BCC_MULTIPLICATIVE_STRUCTURE.md` and
the references therein to Watson 1939.

**LEDGER:** Theorem-level subsidiary of FTD-0001; explicitly named in
Watson's literature.

**Dependencies:** Theorem 1; Watson's integral identities.

**What it does NOT claim.** That the Watson identity drives any
physical mechanism. The 2026-04-26 / 2026-04-27 attempt to use the
BCC sub-stencil two-state spectrum as a derivation route for g_c
(Mechanism C, FTD-0093) closed NEGATIVE; the Watson identity itself
remains a [THEOREM] in pure number theory but does not yield a
physics derivation.

---

## 6 · Theorem 6 — Phase G geometric Coulomb identity

**Statement.** Let G_L(r) denote the lattice Poisson Green's function
on the L³ periodic cubic lattice, evaluated at lattice separation r.
Define α_r(r, L) := 2 · r · G_L(r). Then α_r(r, L) is the engine's
emergent radial Coulomb-mode coefficient at every finite L, with zero
free parameters and zero fine-structure-constant content.

**Proof.** Direct computation of the lattice Poisson kernel and
comparison to the engine's measured Coulomb potential. R² = 1.0000 at
L = 384 in the Coulomb tail; median 0.07% residual. See
`docs/theory/10_eft_program/DERIV_EMERGENT_COULOMB_GEOMETRIC.md` and
`scripts/proofs/fit_geometric_coulomb.py`.

**LEDGER:** Phase G theorem (April 19 closure of EFT Recovery
Program Phase F).

**Dependencies:** Lattice Green's function theory (Glasser & Zucker
1980; standard in lattice-Green's-function literature).

**What it does NOT claim.** That the engine's Coulomb interaction
recovers QED's α at any scale. The theorem is exactly what it says: a
geometric identity for the lattice Poisson kernel. The physical α
enters separately through the coupling g_c, which is [PARAMETRIC]
(see §9).

---

## 7 · Theorem 7 — Phase J partition-function ultralocality

**Statement.** The classical FTD partition function on a finite L³
lattice has Euclidean action S_E that depends on the state field s ∈
{−1, 0, +1}^{L³} only through Σ_i s_i² (the count of manifested
sites). The action is invariant under arbitrary spatial permutations
of charge placement at fixed charge count.

**Proof.** Explicit construction of the L=2 partition function in
`docs/theory/10_eft_program/DERIV_PARTITION_FUNCTION_L2.md`; the
ultralocality is a structural feature, not a coincidence at small L.

**LEDGER:** FTD-0042 area (classical partition function).

**Dependencies:** None beyond the FTD axioms (5 postulates per
SPEC_FTD.md).

**Consequence (NOT promoted to theorem in this document).**
Ultralocality of the classical action means classical extremization
alone cannot fix the gauge coupling g_c — informationally, the action
sees only Σ s², not the spatial structure that g_c would couple to. A
quantum extension (Mechanism B per FTD-0031) was attempted as the
remaining first-principles route for g_c and closed NEGATIVE (circular
in the boundary of the projection). g_c remains [PARAMETRIC] as of
2026-04-27.

**What it does NOT claim.** That the classical action has any
particular physical content beyond its information structure. The
ultralocality is the structural finding; downstream consequences are
separate.

---

## 8 · Subsidiary theorems and structural nulls

These are smaller [THEOREM]-level claims that depend on the seven
above:

- **D = 3 from |Aut(E)|² = 2^D · (D−1)!** — combinatorial identity
  forcing dimensionality. See `THEOREM_D_EQUALS_3.md`.
- **Moore integers uniqueness {N_base = 4, N_eff = 13, b_3 = 7}** —
  combinatorial enumeration on the 26-Moore neighborhood with
  ternary states. See
  `docs/theory/08_structural/THEOREM_MOORE_LAYER_DECOMPOSITION.md`.
- **a_phys ≡ ℓ_P no-go (FTD-0059)** — no length expressible from
  Axiom-Zero invariants; the lattice-to-physical-length conversion
  must be calibrated. See
  `docs/theory/10_eft_program/THEOREM_A_PHYS_NO_GO.md`.
- **Phase H coupling scaling** α_r(g_c) = g_c² · α_r(1) — direct from
  Theorem 6 by substitution. See
  `docs/theory/10_eft_program/DERIV_EMERGENT_COULOMB_GEOMETRIC.md` §H.
- **Structural nulls under FTD axioms**: N_monopole = 0 (no SU(2)
  monopoles in the 3-state framework), N_SUSY = 0 (no on-lattice
  superpartners), τ_proton = ∞ (pointwise charge conservation).

---

## 9 · The empirical observation (NOT a theorem — explicit boundary)

The two roots of the master quadratic (Theorem 2) match dimensionless
constants from unrelated physical sectors:

$$x_+ = 137.0362\ldots \approx 1/\alpha \quad (\text{1.26 ppm})$$
$$x_- = 3.0240\ldots \approx N_c \quad (\text{0.80\%})$$

This is the **dual-prediction property**. It is the strongest piece of
empirical motivation for taking FTD's algebraic spine seriously as a
physics-relevant structure. It is recorded in the LEDGER as

- **FTD-0013** [STRONGLY MOTIVATED CONJECTURE] x+ identification with 1/α
- **FTD-0017** [STRONGLY MOTIVATED CONJECTURE] x− identification with N_c

These are NOT promoted to theorem. The motivation is the algebraic
rigidity of P(x) (Theorem 2 + uniqueness from Theorem 3) plus a
60k-polynomial scan (`scripts/proofs/audit_master_quadratic_rigidity.py`)
showing the master quadratic is the tightest dual-match in its
neighborhood of polynomial space. The observation is structural
evidence, not derivation.

The methodological gap that prevents promotion: there is no derivation
of the physics identification from the FTD axioms. As of 2026-04-27,
all three attempted derivation routes for the gauge coupling g_c
(Mechanisms A, B, C) have closed negative, so the route from "P(x)
roots" to "physical α" runs through `g_c` at [PARAMETRIC] status.

The honest reading: P(x) is a specific polynomial (Theorem 2) with two
roots that numerically match (1/α, N_c). That is a real fact. Whether
it is structurally meaningful or a 60k-poly look-elsewhere artifact is
the open methodological question (FTD-0097 pre-registered scan
addresses this; not yet run).

---

## 10 · What this document allows you to claim

In order from most to least defensible:

1. "FTD has a rigorous algebraic core consisting of seven theorems
   centered on the lemniscatic constant G* = Γ(1/4)²/(2√(2π)·Γ(1/2))."
   — Theorems 1-7.

2. "A specific polynomial P(x) = x² − 16G*²x + 16G*³ has roots that
   match 1/α and N_c simultaneously to permille precision; this
   polynomial is unique among class-number-1 CM curve constructions
   to produce this dual match." — Theorems 2, 3 + observation §9.

3. "The corresponding lattice simulator reproduces the lattice Poisson
   Green's function as its Coulomb interaction exactly, with no
   fine-structure-constant content in the coupling-free limit." —
   Theorem 6.

4. "The classical FTD action is ultralocal in state space; classical
   extremization cannot fix the gauge coupling, and as of 2026-04-27
   all three first-principles derivation routes for g_c have closed
   negative." — Theorem 7 + LEDGER FTD-0031 + FTD-0093.

5. "The physical identification of P(x)'s roots with α and N_c is a
   structurally-motivated conjecture, not a derivation; it sits in the
   LEDGER at [STRONGLY MOTIVATED CONJECTURE] and remains there until
   either a derivation is found OR the look-elsewhere scan (FTD-0097)
   demonstrates the dual match is selective rather than expected." —
   §9 + FTD-0013 / FTD-0017 / FTD-0097.

What this document explicitly does NOT allow you to claim:

- That the algebraic spine derives the Standard Model
- That P(x) IS a partition function, characteristic polynomial of an
  RG step, gap equation, or any dynamical object (those readings have
  been audited; the algebraic spine survives, the dynamical readings
  do not — see `AUDIT_LINK8_CLOSURE.md`)
- That the CM uniqueness extends to class-number ≥ 2 (open, plausible,
  not proven)
- That the engine's Coulomb interaction recovers QED's α
- That g_c is derived from anything; it is [PARAMETRIC]

---

## 11 · Cross-references

| Theorem | Primary doc | Verification script | LEDGER row |
|---|---|---|---|
| 1 G* identity | `MONOGRAPH_GSTAR_BRIDGE_CONSTANT.md` | `proof_motivic_master_quadratic.py` | FTD-0001 |
| 2 Master quadratic | `DERIV_MASTER_QUADRATIC_GAP_EQUATION.md` | `audit_master_quadratic_rigidity.py` | FTD-0014 |
| 3 CM uniqueness | `AUDIT_MASTER_QUADRATIC.md` | `scan_cm_curves.py` | (audit-derived) |
| 4 Coefficient 16 | `EXPLR_COEFFICIENT_16.md` | included in motivic proof | FTD-0014 (sub) |
| 5 Watson identity | `DERIV_BCC_MULTIPLICATIVE_STRUCTURE.md` | (analytic) | FTD-0001 (sub) |
| 6 Phase G Coulomb | `DERIV_EMERGENT_COULOMB_GEOMETRIC.md` | `fit_geometric_coulomb.py` | (Phase G) |
| 7 Phase J ultralocal | `DERIV_PARTITION_FUNCTION_L2.md` | `partition_function_L2.py` | FTD-0042 area |

| Subsidiary | Primary doc | LEDGER |
|---|---|---|
| D = 3 | `THEOREM_D_EQUALS_3.md` | FTD-0036 area |
| Moore integers | `THEOREM_MOORE_LAYER_DECOMPOSITION.md` | FTD-0008 area |
| a_phys ≡ ℓ_P no-go | `THEOREM_A_PHYS_NO_GO.md` | FTD-0059 |
| Phase H scaling | `DERIV_EMERGENT_COULOMB_GEOMETRIC.md` §H | (Phase H) |

| Empirical observation | LEDGER |
|---|---|
| x+ = 1/α | FTD-0013 [STRONGLY MOTIVATED CONJECTURE] |
| x− = N_c | FTD-0017 [STRONGLY MOTIVATED CONJECTURE] |
| Look-elsewhere scan | FTD-0097 [HYPOTHESIS] (not yet run) |

---

## 12 · Single-line summary

**FTD's algebraic spine is seven theorems centered on G* =
Γ(1/4)²/(2√(2π)·Γ(1/2)): the G* identity (Chowla-Selberg), the master
quadratic P(x) = x² − 16G*²x + 16G*³ and its two roots, CM uniqueness
within class-number-1 fields, the coefficient 16 = |Aut(E)|² for E:
y² = x³ − x, the Watson identity W₃ = G*²/(2π), the Phase G geometric
Coulomb α_r = 2r·G_L(r), and Phase J classical-action ultralocality.
All seven are independent of physics interpretation. The dual
numerical match x+ ≈ 1/α (1.26 ppm) + x− ≈ N_c (0.80%) is recorded as
[STRONGLY MOTIVATED CONJECTURE], not theorem; promotion requires
either a derivation (all three first-principles routes for g_c are
closed-negative as of 2026-04-27) or a look-elsewhere scan
demonstrating selectivity (FTD-0097 pre-registered, not run).
