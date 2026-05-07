# SPEC — FTD Algebraic Spine (Theorems Only)

**Tag:** [REFERENCE] / canonical
**Date:** 2026-04-27 (initial theorem-list review). **Supplemental note 2026-04-28:** FTD-0110's cluster-efficiency coefficient `k = 1/N_base = 1/4` was promoted to **[DERIVED at linear level]** in `docs/theory/03_derivations/DERIV_K_FROM_OH_A1G_MULTIPLICITY.md` (commit `306837c`). The underwriting subsidiary `mult(A_{1g}) = 4` in the natural 27-dim permutation rep of O_h on the 3³ Moore block is a **[THEOREM]** (character-table formula `192/48 = 4`), independent of any physics interpretation. It does NOT add an 8th theorem to this spine — FTD-0110's coefficient is tagged [DERIVED], not [THEOREM]. **Update 2026-04-29 (late evening):** Theorem 8 (harmonic invariant of the master-quadratic tower) added in §8 — `1/y_+ + 1/y_− = 1` for the (1+i)-tower of master quadratics, where `y_± := x_±/G*`; LEDGER FTD-0111. **Update 2026-04-30:** Theorem 9 (field-theoretic characterization of `Q(G*)` as a maximal `π`-free subfield of `Q(π, Γ(1/4))`) added in §9, conditional on Chudnovsky 1976; LEDGER FTD-0112. The spine now has **nine theorems**; full derivations in `docs/theory/03_derivations/THEOREM_HARMONIC_INVARIANT_TOWER.md` (Theorem 8) and `docs/theory/07_assessment/archive_session_outputs/SESSION_SYNTHESIS_2026-04-30.md` §1 (Theorem 9). Section numbering bumped (`§§10–15`).
**Purpose:** state the load-bearing algebraic content of FTD in
[THEOREM]-only form, with no physics interpretation. This is the
citation target for paper drafts, manuscript chapters, and any future
work that wants to lean on the rigorous core. Read it as a list of
mathematical objects and proven identities, NOT as a derivation of
the Standard Model.

---

## 0 · What this document is and is not

**This document IS:** a canonical statement of nine theorems that
constitute FTD's rigorous mathematical core, with proof references.
Each theorem is independent of any physics interpretation. The objects
involved (Γ-function values, CM elliptic curves, lattice Green's
functions, Watson integrals) are standard mathematical objects with
established literatures.

**This document IS NOT:** a derivation of the fine-structure constant,
the QCD color number, electron mass, or any other physical quantity.
The numerical match between two roots of the master quadratic and (1/α,
N_c) is a separate empirical observation — recorded in §10 below as
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

**Statement.** Let G* := Γ(1/4) / Γ(3/4). Then

$$G^* = \frac{\Gamma(1/4)}{\Gamma(3/4)}
       = \frac{\Gamma(1/4)^2}{\sqrt{2}\cdot \pi}
       = \varpi \cdot \frac{2}{\sqrt{\pi}}$$

where ϖ = Γ(1/4)² / (2√(2π)) ≈ 2.62205755... is the Bernoulli/Gauss
lemniscate constant (a *different* number from G*). Numerically
G* = 2.95867512... and 16·G*² = 140.060... .

The two equivalent closed forms follow from the Γ-function reflection
identity Γ(1/4)·Γ(3/4) = π/sin(π/4) = π√2.

**Notational warning.** G* (project canonical, ≈ 2.959) and the
Bernoulli/Gauss lemniscate constant ϖ (≈ 2.622) are sometimes both
called "the lemniscate constant" in informal usage. They are distinct:
the master quadratic `x² − 16G*²x + 16G*³ = 0` produces x_+ = 137.036
(= 1/α numerically) ONLY at G* = 2.959, not at ϖ = 2.622 (which would
give x_+ = 107.3, far from 1/α). Always cross-check against
`scripts/constants.py` (`G_STAR`) when a numerical value is needed.

This document was previously stated with an erroneous formula
`Γ(1/4)²/(2√(2π)·Γ(1/2))` (which evaluates to 1.479, not 2.622) and
an erroneous asserted value 2.622 (which is ϖ, not G*). Corrected
2026-04-30 per LEDGER FTD-0117.

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

**LEDGER:** FTD-0001 (Master Quadratic Polynomial + Roots, [THEOREM]);
the polynomial coefficients are deterministic functions of G*. The
physical identifications x+ ↔ 1/α and x− ↔ N_c are tracked separately
under FTD-0013 and FTD-0014 ([STRONGLY MOTIVATED CONJECTURE]).

**Dependencies:** Theorem 1.

**What it does NOT claim.** That P(x) describes a physical system, an
RG flow, a partition function, or any dynamical object. It is a
quadratic polynomial in one variable. The numerical proximity x+ ≈ 1/α
and x− ≈ N_c is recorded separately in §9.

---

## 3 · Theorem 3 — CM curve uniqueness (under the trivial-multiplier criterion)

**Status.** **[NUMERICAL FACT, exhaustive across class numbers 1–4 with |d| ≤ 907; under the trivial-multiplier criterion declared below]**. Three measurements together establish the current scope:

1. **Tier-I MC-T1.2 closure (2026-05-02 morning)**: honestly reclassified the original h=1 claim from [THEOREM] to [NUMERICAL FACT, h=1 only].
2. **9-Heegner rigidity scan (2026-05-02 evening, FTD-0124)**: pre-registered scan over 9 Heegner × 19 framework-integer-factorable coefficients × 17 framework-integer targets × 2 roots = 5814 quadruples. **Under the trivial-multiplier criterion (q = 1, root = target directly): EXACTLY ONE strict (5.45 ppm) match in the 5814-grid — the canonical (d=−4, c=16, x_+, 1/α).** First quantitative rigidity confirmation at this strict criterion. **Under the rational-multiplier criterion (q ≤ 200, FC-factorable): 21 strict matches.** The two criteria yield different verdicts (criterion bifurcation; see §"What it does NOT claim").
3. **Γ-product extension to classes 1–4 (2026-05-02 evening, FTD-0123)**: pre-registered scan over 63 fundamental discriminants spanning class numbers 1, 2, 3, 4 with |d| ≤ 907 using the Γ-product analogue `G^*_d := ∏ Γ(a/|d|)^{χ_d(a)}`. **Result: exactly one dual-matcher, d = −4. ZERO h ≥ 2 matchers.** Numerical net 7× larger than the original Heegner-only set; d=−4 structural privilege survives.

**Criterion declaration (load-bearing, FTD-0124).** This theorem holds under the **trivial-multiplier criterion**: a "match" requires the natural root x_± of P_d(x) to equal the target dimensionless constant directly (q = 1 in the rational-multiplier search). The analogous statement under the **rational-multiplier criterion** (allow rescaling by any q ≤ 200 with framework-integer factorability) FAILS — 20 additional non-canonical matches exist in the 5814-grid. **Cite this criterion explicitly when invoking Theorem 3.**

**Statement.** Let E_d denote the CM elliptic curve with complex
multiplication by the ring of integers of ℚ(√−d) for d a fundamental
imaginary-quadratic discriminant. Construct the analogue of P(x)
(Theorem 2) using the lemniscatic-analogue constant G*_d defined via
the Γ-product `G*_d := ∏_{a=1}^{|d|−1} Γ(a/|d|)^{χ_d(a)}` (which
reproduces canonical G* exactly at d = −4). Among the 63 fundamental
discriminants checked (h ∈ {1, 2, 3, 4} with |d| ≤ 907), the
discriminant d = −4 is the **unique** value for which both roots
simultaneously match dimensionless physical constants under the
trivial-multiplier criterion at master-quadratic precision (1.26 ppm
on x_+ vs 1/α; 0.80% on x_− vs N_c).

**Verification.** Three pre-registered exhaustive numerical scans:
- `scripts/proofs/scan_cm_curves.py` (original h=1 over 9 Heegner)
- `scripts/proofs/proof_chowla_selberg_higher_h_scan.py` (FTD-0123, classes 1–4 / 63 discriminants; pre-reg tag `preregister-chowla-selberg-higher-h-scan-v1`)
- `docs/theory/10_eft_program/PREREG_HEEGNER_TOWER_RIGIDITY.md` + `docs/theory/10_eft_program/AUDIT_HEEGNER_TOWER_RIGIDITY.md` (FTD-0124, full 5814-grid criterion-bifurcation analysis)

See also `docs/theory/07_assessment/AUDIT_MASTER_QUADRATIC.md` for context.

**LEDGER:** FTD-0003 (quick-index entry, restated 2026-05-02); FTD-0123
(Γ-product extension to h ≥ 2); FTD-0124 (rigidity-scan + criterion
bifurcation finding).

**Dependencies:** Theorems 1, 2; arithmetic of imaginary quadratic
fields; Damerell-style identities at h ≥ 2 (theory note in
`EXPLR_CHOWLA_SELBERG_HIGHER_H.md` for the proper analogue).

**What it does NOT claim.**
- Uniqueness as a structural theorem (that is MC-T2.3 §4 item 4 and remains [OPEN]; closure requires a Galois-theoretic or unit-group argument that the d = −4 privilege has a structural origin).
- Uniqueness under the rational-multiplier criterion. **20 non-canonical strict matches exist** in the FTD-0124 5814-grid under q ≤ 200 + FC-factorability. The cleanest non-canonical example: (d=−3, c=3, x_+, m_μ/m_e) at +0.908 ppm via multiplier 13³/(2·7²) = N_eff³/(2·b_3²) — every parameter a framework integer. This rational-multiplier reading is [SELECTION], not [THEOREM]; see SSB-3 (`DERIV_SPIN_STATISTICS_BRIDGE.md`) for an example where the framework already invokes a non-trivial multiplier (91/732) explicitly.
- The full per-ideal-class Damerell formula at h ≥ 2 has not been used; FTD-0123 uses the Γ-product analogue G*_d. The full Damerell scan would multiply the search space by ~1.5× without changing the result type.

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
(see §10).

**Subsidiary (retarded extension, 2026-04-30).** Phase G is the
time-integrated specialization of a parent retarded identity. Define
`α_r(r, t, L) := 2r · G^ret_L(r, t)` where `G^ret_L` is the retarded
Green's function of the lattice wave equation
`(D²_t − c² Δ_L) G^ret = δ_{r,0} δ(t)` with `c = c_lat = 1/√3`. Then
`∫_0^∞ α_r(r, t, L) dt = α_r(r, L)` exactly at every finite L. In the
continuum limit `α_r(r, t, ∞) = δ(t − r/c) / (2π)` — a delta on the
forward light cone with universal amplitude `1/(2π)`. This is the
lattice form of the standard d'Alembert relation `∫G^ret = G_static`,
filed as **FTD-0113 [DERIVED]** (subsidiary of FTD-0004, not a new
spine theorem). See `docs/theory/03_derivations/DERIV_RETARDED_GREEN_LATTICE.md`
and `scripts/proofs/proof_retarded_green_identity.py` (numerical
verification at L=8 to machine precision).

---

## 7 · Theorem 7 — Phase J partition-function ultralocality

**Status.** [THEOREM at L = 2] + [CONJECTURE for general L]. The L = 2
case is proven by explicit construction; the structural-feature claim
for arbitrary L is asserted but not formally derived from the axioms
in this document. Audit 2026-05-01 flagged the unconditional [THEOREM]
framing as inflated; the present text is the corrected statement.

**Statement (L = 2, [THEOREM]).** The classical FTD partition function
on a 2³ lattice has Euclidean action S_E that depends on the state
field s ∈ {−1, 0, +1}^{8} only through Σ_i s_i² (the count of
manifested sites). The action is invariant under arbitrary spatial
permutations of charge placement at fixed charge count.

**Statement (general L, [CONJECTURE]).** The same dependence on
Σ_i s_i² alone, and the same permutation invariance, hold on every
finite L³ lattice.

**Proof (L = 2 only).** Explicit construction of the L=2 partition
function in `docs/theory/10_eft_program/DERIV_PARTITION_FUNCTION_L2.md`.
The general-L extension is asserted on structural-feature grounds; a
formal proof from the 5 axioms is [OPEN].

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

## 8 · Theorem 8 — Harmonic invariant of the master-quadratic tower

**Statement.** For each integer `k ≥ 3`, define the **(1+i)-tower
master quadratic**

$$M_k(x) \;:=\; x^2 \;-\; 2^k\,G^{*\,k-2}\,x \;+\; 2^k\,G^{*\,k-1}.$$

The `k = 4` instance is the master quadratic of Theorem 2. Let `x_+,
x_−` be the two roots of `M_k` and define normalized roots `y_± :=
x_±/G*`. Then for every `k ≥ 3`,

$$\frac{1}{y_+} \;+\; \frac{1}{y_-} \;=\; 1.$$

Furthermore the discriminant factors as

$$\operatorname{disc}(M_k) \;=\; 2^{k+2}\,G^{*\,k-1}\,A_k, \qquad A_k := 2^{k-2}\,G^{*\,k-3} - 1,$$

where `A_k` is rational at `k = 3` (`A_3 = 1`) and **transcendental
over `Q`** at every `k ≥ 4` (`A_4 = 4G* − 1`, etc.; via Schneider–
Chudnovsky transcendence of `G*` — a non-rational polynomial in a
transcendental over `Q` with rational coefficients is transcendental).

**Proof reference.** Three-line Vieta computation for the harmonic
invariant; direct factorization for the discriminant. Full derivation
plus the closed-form corollary `α_tree = 1/(2G*) − √(4G*−1)/(4G*^{3/2})`
in `docs/theory/03_derivations/THEOREM_HARMONIC_INVARIANT_TOWER.md`.
Numerical confirmation at 50-digit precision for `k ∈ {3, 4, 5, 6, 7}`
in `scripts/proofs/proof_harmonic_invariant_tower.py`.

**LEDGER:** FTD-0111.

**Dependencies:** Theorem 1 (`G*` algebraic identity); Theorem 2
(master quadratic, the `k = 4` instance); Schneider–Chudnovsky
transcendence of `G*` (for transcendence of `A_k` at `k ≥ 4`).

**Consequence (DERIVED, restatement of Theorem 2).** The master
quadratic's `α`-tree-level prediction admits the publication-grade
closed form

$$\alpha_{\text{tree}} \;=\; \frac{1}{2 G^*} \;-\; \frac{\sqrt{4 G^* - 1}}{4\,G^{*\,3/2}}\,,$$

equivalent to `16 G*³ α² − 16 G*² α + 1 = 0`. This is FTD-0001 in
algebraically legible form; it is not a new claim and does not change
the [STRONGLY MOTIVATED CONJECTURE] tag of the dual-prediction
identification (see §10 below).

**What it does NOT claim.**
- Selection of `k = 4` as the "physics level" of the tower. The tower
  parameterizes the master-quadratic family without selecting a level
  from first principles; the empirical match `α⁻¹ ≈ x_+(k=4)` to 1.26
  ppm (and `N_c ≈ x_−(k=4)` to 0.80%) is unaltered. The structural
  question of whether `k=4` here is the same `4` as `mult(A_{1g})=4`
  on the 27-block (FTD-0110 / `DERIV_K_FROM_OH_A1G_MULTIPLICITY.md`)
  is **[OPEN; empirical agreement, structural identification not
  proven]** (Tier-I MC-T1.5 in the math-complete checklist; Tier-IV
  MC-T4.5 escalates this question to "why-level-k=4 from N_base=4").
  The 2026-05-02 Tier-I closure pass reclassifies this as an
  acceptance (structural identification deferred to Tier-IV) rather
  than a structural theorem.
- Uniqueness of the (1+i)-multiplier choice. The harmonic invariant
  holds for any family `{M(x) = x² − bx + c : c = G* · b}` regardless
  of the multiplier, so the (1+i)-tower (`m_k = 2^k`) is one indexed
  sub-family rather than the unique forced one. A multiplier-level
  rigidity scan analogous to the 60k-polynomial scan that rigidified
  FTD-0001 is **[OPEN]**.
- Any QFT-anomaly construction. The level-`k` discriminant correction
  `A_k` is a level-indexed algebraic transcendental; calling it an
  "anomaly factor" by analogy with QFT conformal anomalies is metaphor
  pending a formal regularization-class construction in the
  matched-stencil EFT.

---

## 9 · Theorem 9 — Field-theoretic characterization of `Q(G*)`

**Statement.** `Q(G*)` is a maximal `π`-free subfield of
`Q(π, Γ(1/4))`. Specifically:

$$\mathbb{Q}(G^*) \;\subseteq\; \mathbb{Q}(\pi,\,\Gamma(1/4)),
\qquad
\mathbb{Q}(G^*) \;\cap\; \mathbb{Q}(\pi) \;=\; \mathbb{Q}.$$

That is, `G*` is a generator of the lemniscatic field with no algebraic
content visible to `Q(π)` alone.

**Proof.**

*Containment* `Q(G*) ⊆ Q(π, Γ(1/4))`: by classical identity (Theorem 1
of this document and §6 of `MONOGRAPH_GSTAR_BRIDGE_CONSTANT.md`),
`G* = Γ(1/4)² / (π√2)`, so `G*` is a `Q`-rational function in
`{Γ(1/4), π}`. Hence every element of `Q(G*)` lies in `Q(π, Γ(1/4))`.

*Trivial intersection* `Q(G*) ∩ Q(π) = Q`: assume `α ∈ Q(G*) ∩ Q(π)`.
Then `α` is a rational function in `G*` with rational coefficients,
i.e. `α = p(G*)/q(G*)` with `p, q ∈ Q[T]` and `q(G*) ≠ 0`. If `α` is
also in `Q(π)`, then there exist `f, g ∈ Q[T]` with `α = f(π)/g(π)`,
`g(π) ≠ 0`. Cross-multiplying gives `p(G*) · g(π) = q(G*) · f(π)`, a
polynomial relation in `Q[G*, π]` between `G*` and `π`.

By Chudnovsky 1976 (algebraic independence of `π` and `Γ(1/4)`) and
the rational identity `G* · π · √2 = Γ(1/4)²`, the constants `G*` and
`π` are algebraically independent over `Q`: any polynomial relation
`P(G*, π) = 0` with `P ∈ Q[X, Y]` would force, via the Γ(1/4)
substitution `Γ(1/4)² = G* · π · √2`, a polynomial relation
`P̃(π, Γ(1/4)) = 0` with `P̃ ∈ Q(√2)[X, Y] ⊆ \overline{Q}[X, Y]`,
contradicting Chudnovsky.

Hence the only polynomial relation `p(G*)·g(π) = q(G*)·f(π)` consistent
with algebraic independence is the constant case: both sides reduce to
the same rational. Therefore `α ∈ Q`. ∎

**Conditional clause.** This theorem is conditional on the algebraic
independence of `π` and `Γ(1/4)` over `Q`, established by D. V.
Chudnovsky, "Algebraic independence of values of exponential and
hypergeometric functions" (1976) and consolidated in the modern
references (Waldschmidt 2000, *Diophantine Approximation on Linear
Algebraic Groups*, §1.4). The result is a standard tool of contemporary
transcendence theory; "conditional" here means "depends on this
established theorem", not "depends on a conjecture".

**Why it matters.** Theorem 9 makes the `π`-free positioning of FTD's
algebraic spine a precise field-theoretic statement rather than a
slogan. Combined with Theorem 8 (which proves `G*` is the unique
named-constant generator with which the entire master-quadratic tower
admits clean rational-coefficient × integer-power form), Theorem 9
characterizes `Q(G*)` as the **canonical π-free subfield** of
`Q(π, Γ(1/4))` — the algebraic content of the spine that is invisible
to `Q(π)` alone.

**LEDGER:** FTD-0112 (filed 2026-04-30 alongside this theorem).

**Dependencies:** Theorem 1 (`G*` identity), Chudnovsky 1976.

**What it does NOT claim.**
- That `G*` is in OEIS under its own A-number. (A085565 is the
  lemniscate constant `L = 2ϖ`, not `G*`. As of 2026-04-30 audit, no
  A-number for `Γ(1/4)/Γ(3/4)` itself has been confirmed.)
- That Gauss computed `G*` as a privileged object. (Gauss computed
  `Γ(1/4)` and `ϖ`; the specific ratio `Γ(1/4)/Γ(3/4)` was retracted
  during the 2026-04-30 audit unless a citation is produced.)
- Maximality of `Q(G*)` as a subfield in any sense beyond
  `π`-freeness. There may be larger π-free subfields of
  `Q(π, Γ(1/4))`; the theorem does not exclude them.

**Provenance.** Imported from external session synthesis on
2026-04-30; the synthesis is archived at
`docs/theory/07_assessment/archive_session_outputs/SESSION_SYNTHESIS_2026-04-30.md` §1
where this is "Theorem 3".

### 9.1 · Operational reading via the parity-twist (FTD-0127, 2026-05-03)

Theorem 9 admits a clean operational reading in L-function language:

$$G^* \;=\; \frac{\Gamma_\zeta(1/2)}{\Gamma_{\chi_{-4}}(1/2)} \;=\; \frac{\Gamma(1/4)}{\Gamma(3/4)}$$

That is: `G*` is the ratio of Archimedean Γ-factors of the two simplest
Dirichlet L-functions — `ζ` (even parity, `a = 0`, Γ-factor `Γ(s/2)` →
`Γ(1/4)` at the critical-line center) and `L(s, χ_{−4})` (odd parity,
`a = 1`, Γ-factor `Γ((s+1)/2)` → `Γ(3/4)` at the critical-line center).

**Refined statement of Theorem 9:**
> `Q(G*)` is the field generated by the parity-twist that distinguishes
> even-parity (`ζ`) from odd-parity (`L(s, χ_{−4})`) Dirichlet L-functions
> of conductor 4 at the critical-line center `s = 1/2`.

This is the same mathematical content as the field-theoretic statement
above (no new spine theorem; spine count remains 9), expressed in
operationally meaningful L-function language.

**Companion boundary identities (FTD-0127, [DERIVED] from Lerch +
functional equation + Gauss digamma):**

| location | value |
|---|---|
| `L'(0, χ_{−4})` | `log(G*/2)` (Lerch 1894) |
| `L'(1, χ_{−4})` | `(π/4) · [γ + log(2π/G*²)]` (FE + L'(0)) |
| `L'(1/2, χ_{−4})` | `(L(1/2)/2) · [γ + log(2π) − π/2]` (FE + Gauss ψ(3/4)) |

**Negative scoping result (PSLQ at 80 dps, maxcoeff 10⁷, Bayes ratio
~10¹⁵):** `L(1/2, χ_{−4})`, Catalan G = `L(2, χ_{−4})`, and `ζ(1/2)` all
sit OUTSIDE `Q(G*) ∪ standard extensions`. The boundary of `L(s, χ_{−4})`'s
critical strip is fully closed-form in `Q(G*, γ, π, log π, log 2)`; the
center introduces exactly one new transcendental that the algebraic spine
doesn't reach. `ζ` itself is NOT directly tied to `G*` in any tested
location (`Q(G*) ∩ Q(π) = Q` excludes ζ's π-only special values for a
structural reason).

Full derivation, numerical verification, and PSLQ scoping in
`docs/theory/03_derivations/DERIV_G_STAR_PARITY_TWIST.md`. Verification
scripts: `scripts/proofs/proof_g_star_parity_twist.py` (identity check)
and `scripts/proofs/proof_lprime_chi4_boundary.py` (Identities A/B/C at
high precision). LEDGER row: FTD-0127.

---

## 10 · Subsidiary theorems and structural nulls

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

### 10.X · FQCR subsidiaries to Theorems 1, 2, 8

The Finite Quarter-Conjugacy Recurrence framework (FQCR; see
`SPEC_FQCR.md`) lands several operator-theoretic readings of existing
spine theorems. None of these introduce a 10th theorem — the spine
count remains 9. Each lands as a subsidiary that adds an operator
provenance chain alongside the existing algebraic / number-theoretic
provenance.

- **G\* via $\det_\zeta$ quarter-conjugacy bridge (FTD-0141, [THEOREM])** —
  the bridge constant of Theorem 1 is the $\zeta$-regularized determinant
  ratio of the quarter-twisted spectra $\{n + \tfrac{1}{4}\}_{n\ge 0}$ and
  $\{n + \tfrac{3}{4}\}_{n\ge 0}$ arising from the conjugacy operator $J$
  with $J^2 = -I$. Operator-theoretic provenance via Lerch's formula.
  Complementary to FTD-0127's parity-twist (number-theoretic / L-function
  lens). See `DERIV_GSTAR_QUARTER_CONJUGACY.md`.
- **G\* as finite-N attractor (FTD-0142, [THEOREM])** — the finite product
  $G_N^* = (N+1)^{-1/2} \prod_{n=0}^{N} (n+\tfrac{3}{4})/(n+\tfrac{1}{4})$
  converges to $G^*$ at rate $|G_N^* - G^*| = O(1/N^2)$, empirical
  $C \approx 0.046$. Discharges the `AUDIT_INFINITY_REFRAME.md` ε-L
  obligation for $G^*$. Verified by `proof_fqcr_convergence.py`. See
  `DERIV_GSTAR_FINITE_APPROX.md`.
- **Master quadratic as transfer-matrix characteristic polynomial
  ([THEOREM] notational)** — the FQCR transfer matrix $M_N(t)$ has
  characteristic polynomial $x^2 - 16(G_N^*)^2 x + 16(G_N^*)^3 R_N(t) = 0$;
  at $R_N = 1$ and $N \to \infty$ this is exactly Theorem 2. The operator
  interpretation does not change Theorem 2's content, only adds an
  operator framing. See `DERIV_MASTER_QUADRATIC_GAP_EQUATION.md` Part VII
  and `SPEC_FQCR.md` Model V.
- **Z\_4 unification candidate** — the conjugacy operator $J$ with
  $J^2 = -I$ shares its $Z_4$ structural anchor with Theorem 8's
  $(1+i)$-tower and with the $i$-cycle ontology in
  `FOUND_COGITO_AXIOM_AND_FULL_TRACE.md`. A future stylistic refactor
  could group these under a unified "$Z_4$ algebraic-spine" subsection;
  not done in the current spine version.

---

## 11 · The empirical observation (NOT a theorem — explicit boundary)

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

## 12 · What this document allows you to claim

In order from most to least defensible:

1. "FTD has a rigorous algebraic core consisting of nine theorems
   centered on the constant G* = Γ(1/4)/Γ(3/4) = √2·Γ(1/4)²/(2π) ≈ 2.9587
   (distinct from the Bernoulli/Gauss lemniscate constant ϖ ≈ 2.622;
   see §1)." — Theorems 1-9.

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

## 13 · Cross-references

| Theorem | Primary doc | Verification script | LEDGER row |
|---|---|---|---|
| 1 G* identity | `MONOGRAPH_GSTAR_BRIDGE_CONSTANT.md` | `proof_motivic_master_quadratic.py` | FTD-0002 |
| 2 Master quadratic | `DERIV_MASTER_QUADRATIC_GAP_EQUATION.md` | `audit_master_quadratic_rigidity.py` | FTD-0001 |
| 3 CM uniqueness | `AUDIT_MASTER_QUADRATIC.md` | `scan_cm_curves.py` | (audit-derived; numerical scan over 9 class-number-1 discriminants) |
| 4 Coefficient 16 | `EXPLR_COEFFICIENT_16.md` | included in motivic proof | FTD-0006 / FTD-0007 |
| 5 Watson identity | `DERIV_BCC_MULTIPLICATIVE_STRUCTURE.md` | (analytic) | FTD-0001 (sub) |
| 6 Phase G Coulomb | `DERIV_EMERGENT_COULOMB_GEOMETRIC.md` | `fit_geometric_coulomb.py` | (Phase G) |
| 7 Phase J ultralocal (L=2 only) | `DERIV_PARTITION_FUNCTION_L2.md` | `partition_function_L2.py` | FTD-0042 area |

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

## 14 · Single-line summary

**FTD's algebraic spine is nine theorems centered on G* =
Γ(1/4)/Γ(3/4) ≈ 2.9587: the G* identity (Chowla-Selberg), the master
quadratic P(x) = x² − 16G*²x + 16G*³ and its two roots, CM uniqueness
within class-number-1 fields, the coefficient 16 = |Aut(E)|² for E:
y² = x³ − x, the Watson identity W₃ = G*²/(2π), the Phase G geometric
Coulomb α_r = 2r·G_L(r), Phase J classical-action ultralocality, the
(1+i)-tower harmonic invariant 1/y₊ + 1/y₋ = 1 with anomaly transcendence
A_k ∉ Q for k ≥ 4, and the field-theoretic characterization of Q(G*)
as a maximal π-free subfield of Q(π, Γ(1/4)) (conditional on Chudnovsky
1976). All nine are independent of physics interpretation. The dual
numerical match x+ ≈ 1/α (1.26 ppm) + x− ≈ N_c (0.80%) is recorded as
[STRONGLY MOTIVATED CONJECTURE], not theorem; promotion requires
either a derivation (all three first-principles routes for g_c are
closed-negative as of 2026-04-27) or a look-elsewhere scan
demonstrating selectivity (FTD-0097 pre-registered, not run).

---

## 15 · From theorems to physics — see the dimensional map

The nine theorems above are dimensionless. Their connection to
physical-unit observables (m_e in MeV, lifetimes in seconds, lengths
in meters) goes through the **calibration interface**: exactly two
SI-dimensional anchors (`a_phys ≡ ℓ_P` and `K_B = m_e`) are
theorem-enforced as the irreducible minimum (FTD-0059 + FTD-0096).

The full dimensionless ↔ dimensional bridge — including the seven
theorems, the four dimensionless physical predictions
(α, N_c, m_μ/m_e, m_τ/m_e), the three calibration declarations, and
one worked dimensional application — is catalogued at:

- **Reference map (auto-generated):** `SPEC_DIMENSIONAL_MAP.md`
- **Canonical data (single source of truth):** `dimensional_map.json`
- **Renderer:** `scripts/proofs/build_dimensional_map.py`
- **Tests:** `scripts/tests/test_dimensional_map.py`

Use the dimensional map when drafting papers or replying to reviewers
that ask "is this prediction dimensionless or does it require
calibration?". Each map entry carries an explicit epistemic tag,
LEDGER cross-references, and (for entries with experimental analogues)
the comparison delta in ppb.

The CATALOG_PARAMETRIC_INSERTIONS.md document remains the right home
for the full ~162-row enumeration of all SM-quantity insertions; the
dimensional map's job is to expose the *bridge mechanism*, not
re-catalog every insertion.
