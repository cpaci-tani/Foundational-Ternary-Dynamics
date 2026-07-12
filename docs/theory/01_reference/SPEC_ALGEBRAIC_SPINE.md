# SPEC — FTD Algebraic Spine (Theorems Only)

**Tag:** [REFERENCE] / canonical
**Subsidiary note.** FTD-0110's cluster-efficiency coefficient `k = 1/N_base = 1/4` is **[DERIVED at linear level]** in `docs/theory/03_derivations/DERIV_K_FROM_OH_A1G_MULTIPLICITY.md`. The underwriting subsidiary `mult(A_{1g}) = 4` in the natural 27-dim permutation rep of O_h on the 3³ Moore block is a **[THEOREM]** (character-table formula `192/48 = 4`), independent of any physics interpretation. It does NOT add an 8th theorem to this spine — FTD-0110's coefficient is tagged [DERIVED], not [THEOREM]. Theorem 8 (harmonic invariant of the master-quadratic tower, §8) is `1/y_+ + 1/y_− = 1` for the (1+i)-tower of master quadratics, where `y_± := x_±/G*`; LEDGER FTD-0111; full derivation in `docs/theory/03_derivations/electromagnetism/THEOREM_HARMONIC_INVARIANT_TOWER.md`. Theorem 9 (field-theoretic characterization of `Q(G*)` as a `π`-free subfield of `Q(π, Γ(1/4))`, §9) is conditional on Chudnovsky 1976; LEDGER FTD-0112; full derivation in `docs/theory/07_assessment/campaigns/archive_session_outputs/SESSION_SYNTHESIS_2026-04-30.md` §1. **Maximality is NOT claimed:** only π-freeness is proven; maximality is [OPEN] and is false as literally stated since Q(Γ(1/4)) is a larger π-free subfield. The spine has **nine numbered results** — **seven theorem-grade** (Theorems 1, 2, 3, 5, 6, 8, 9) and **two honestly tiered below theorem grade** (Theorem 4, a value-level identity with conjectural structural necessity, and Theorem 7, [THEOREM at L=2] only). **Count convention (reconciled 2026-07-01, per the provisional specialist review):** Theorem 3 counts at its **arithmetic core only** — the uniqueness of the `|μ_K| = |disc(K)|` coincidence to ℚ(i) among imaginary quadratic fields, a genuine `[THEOREM]` (`TRACKER_ONTIC_TRUTH.md` OT-1.9, T1); its *physics* dual-match privilege of d = −4 is separately `[NUMERICAL FACT — not a proof]` (it fails under the rational-multiplier criterion, see §3's Status block) and is **never part of the theorem-grade count**. Prior copies of this document stated the count inconsistently ("six + three" in §§4/7/12/14, counting Theorem 3 in the tiered bucket) — the 7+2-with-split formulation here is the single canonical count, and every count statement below now matches it. This matches `TRACKER_ONTIC_TRUTH.md`'s tiering (T1/T2 for the seven; T4/T3 for the two).
**Purpose:** state the load-bearing algebraic content of FTD in
**[THEOREM]-only form, with no physics interpretation. This is the
citation target for paper drafts, manuscript chapters, and any future
work that wants to lean on the rigorous core. Read it as a list of
mathematical objects and proven identities, NOT as a derivation of
the Standard Model.

---

## 0 · What this document is and is not

**This document IS:** a canonical statement of FTD's rigorous
mathematical core — nine numbered results, of which **seven are
theorem-grade** (Theorems 1, 2, 3, 5, 6, 8, 9 — Theorem 3 at its
arithmetic core only; its physics dual-match landing is separately
`[NUMERICAL FACT]`, see the count convention in the Subsidiary note
above) and **two are honestly
tiered below theorem grade**, so marked in their own sections
(Theorem 4 — a value-level identity whose structural necessity is
conjectural; Theorem 7 — `[THEOREM at all L ≥ 2]` per FTD-0350,
matched-stencil / Gauss-realizable scope, conditional on the
exact-constraint AXIOM + stencil-consistency SELECTION — kept in the
honestly-tiered bucket pending the owner's bucket-move decision, §7).
The tiering matches `TRACKER_ONTIC_TRUTH.md`.
Each result is independent of any physics interpretation. The objects
involved (Γ-function values, CM elliptic curves, lattice Green's
functions, Watson integrals) are standard mathematical objects with
established literatures.

**This document IS NOT:** a derivation of the fine-structure constant,
the QCD color number, electron mass, or any other physical quantity.
The numerical match between two roots of the master quadratic and (1/α,
N_c) is a separate empirical observation — recorded in §10 below as
[STRONGLY MOTIVATED CONJECTURE], NOT promoted to theorem.

**Why this distinction matters.** The FTD program has
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

**Visualization (FTD-0207).** A spine-theorem view of
the multi-layer math node map renders the 9 spine theorems (T1-T9) + 4
subsidiaries + their LEDGER anchors + `ledger-depends-on` edges as:
`scripts/visualization/results/math_node_map/spine_only.{svg,png}` (high-
res), `docs/papers/figures/node_map_tikz.tex` (paper-inclusion-ready
TikZ), and the **Theorems** layer of
`dissemination/interactive/math_node_map.html` (filterable). The full
corpus-wide map (objects + identities + all 189 LEDGER claims) sits
beside this spine view -- see LEDGER row FTD-0207 for cross-refs and
the reproduction recipe.

---

## 1 · Theorem 1 — G* algebraic identity

**Statement.** Let G* := Γ(1/4) / Γ(3/4). Then

$$G^* = \frac{\Gamma(1/4)}{\Gamma(3/4)}
       = \frac{\Gamma(1/4)^2}{2^{1/2}\cdot \pi}
       = \varpi \cdot \frac{2}{\pi^{1/2}}$$

where ϖ = Γ(1/4)² / (2 · (2\pi)^{1/2}) ≈ 2.62205755... is the Bernoulli/Gauss lemniscate constant (a *different* number from G*). Numerically G* = 2.95867512... and 16·G*² = 140.060... .

The two equivalent closed forms follow from the Γ-function reflection identity Γ(1/4)·Γ(3/4) = π/sin(π/4) = π · 2^{1/2}.

**The Gauss AGM Constant Bridge.** The reflection ratio $G^*$ is exactly twice the product of the Gauss Constant $G = 1 / M(2^{1/2}, 1) \approx 0.83462684$ and the square root of $\pi$:
$$G^* = 2 G \pi^{1/2}$$
Substituting this identity into the FTD electrostatic self-energy formula expresses the electron rest mass $m_e$ in lattice units as a pure transcendental function of the Gauss constant $G$:
$$m_e = \frac{2}{2 + \left(4 - \frac{1}{2G\pi^{1/2}}\right)^{1/2}} \approx 0.51103345$$

**Notational warning.** G* (project canonical, ≈ 2.959) and the Bernoulli/Gauss lemniscate constant ϖ (≈ 2.622) are sometimes both called "the lemniscate constant" in informal usage. They are distinct: the master quadratic `x² − 16G*²x + 16G*³ = 0` produces x_+ = 137.036 (= 1/α numerically) ONLY at G* = 2.959, not at ϖ = 2.622 (which would give x_+ = 107.3, far from 1/α). Always cross-check against `scripts/constants.py` (`G_STAR`) when a numerical value is needed.

An earlier erroneous formula `Γ(1/4)²/(2 · (2\pi)^{1/2}·Γ(1/2))` (which evaluates to 1.479, not 2.622) and an erroneous asserted value 2.622 (which is ϖ, not G*) are superseded by the closed forms above, per LEDGER FTD-0117.

**Proof reference (corrected 2026-07-01 — the prior wording misattributed this step; the value was always correct, only the named mechanism was wrong).** The base identity `G* = Γ(1/4)/Γ(3/4)` follows directly from the **elementary Γ-function reflection formula** `Γ(z)Γ(1−z) = π/sin(πz)` at `z=1/4` (giving `Γ(1/4)Γ(3/4) = π√2`, already used at line 70 above) — **not** from a Chowla-Selberg L-function evaluation. `L(1,χ_{−4}) = π/4` (Leibniz) is a pure-π quantity carrying zero Γ(1/4) content, so it cannot by itself yield G*. Chowla-Selberg's genuine, load-bearing role is one theorem deeper: it explains *why* this specific ratio is the natural period-ratio of the CM curve `y² = x³ − x` — see Theorem 5 (`W₃ = G*²/(2π)`, the Watson bridge), where Chowla-Selberg is correctly load-bearing. See `MONOGRAPH_GSTAR_BRIDGE_CONSTANT.md` for the four independent derivations (Γ-function ratio via elementary reflection, Watson period integral, lemniscate arc length, modular-form value) — of these, the first is elementary; the latter three genuinely invoke Chowla-Selberg-adjacent period theory.

**LEDGER:** FTD-0002 (corrected 2026-07-01, FTD-0348 — this line previously said FTD-0001, which is the master-quadratic row; the spine's own §13 table already said FTD-0002).

**Dependencies:** Γ-function functional equation (elementary reflection — the sole load-bearing input for the identity itself; dependency line reconciled 2026-07-01 to the corrected proof-reference paragraph above, which reserves Chowla-Selberg for Theorem 5).

**What it does NOT claim.** Nothing about physics. G* is a specific real number defined by gamma-function values; the theorem records its algebraic identity, nothing more.

---

## 2 · Theorem 2 — Master quadratic polynomial

**Statement.** Define the polynomial

$$P(x) = x^2 - 16 G^{*2} x + 16 G^{*3}$$

where G* is as in Theorem 1. Then P(x) has discriminant

$$\Delta = 256 G^{*4} - 64 G^{*3} = 64 G^{*3}(4 G^* - 1)$$

and the two real roots are

$$x_{\pm} = 8 G^{*2} \pm \sqrt{64 G^{*4} - 16 G^{*3}}.$$

Numerically: x+ = 137.0362..., x− = 3.0240... .

**Proof.** Direct algebra. P has integer-and-G*-power coefficients;
roots follow from the quadratic formula. The discriminant is positive
because G* > 1/4 (Theorem 1). See
`docs/theory/03_derivations/DERIV_MASTER_QUADRATIC_GAP_EQUATION.md`
(an algebraic identity + physical match) and
`scripts/proofs/proof_motivic_master_quadratic.py`.

**LEDGER:** FTD-0001 (Master Quadratic Polynomial + Roots, [THEOREM]);
the polynomial coefficients are deterministic functions of G*. The
physical identification x+  1/α is tracked separately under FTD-0013
([STRONGLY MOTIVATED CONJECTURE]). The x−  N_c identification was
historically tracked as FTD-0014; it is **RETIRED** per v1.4 §5 (LEDGER
row removed in commit `ca7eb61`) — x− ≈ 3.024 is a mathematical artifact
of the polynomial only.

**Dependencies:** Theorem 1.

**What it does NOT claim.** That P(x) describes a physical system, an
RG flow, a partition function, or any dynamical object. It is a
quadratic polynomial in one variable. The numerical proximity x+ ≈ 1/α
is recorded separately in §11 (and as a physics identification in
FTD-0013). The proximity x− ≈ N_c is mathematical only; the physics
identification x−  N_c is retired per v1.4 §5.

---

## 3 · Theorem 3 — CM curve uniqueness (under the trivial-multiplier criterion)

**Status.** **[THEOREM]** at the arithmetic level (the uniqueness of the lemniscatic $| \mu_K | = | \text{disc}(K) |$ coincidence among imaginary quadratic fields — the PROOF below covers all squarefree $d$, matching `TRACKER_ONTIC_TRUTH.md` OT-1.9; an earlier copy of this line understated the scope as "the class-number-1 fields"). The underlying numerical scans over Heegner and higher class numbers are rigorous verification steps for the numerical values of the CM-elliptic-curve roots. **[NUMERICAL FACT — not a proof]:** the structural privilege of $d = -4$ *for the physical dual-match* is an exhaustive but finite, criterion-dependent scan (via the Γ-product analogue) — it is **not** mathematically proven. It holds under the trivial-multiplier criterion (below) and FAILS under the rational-multiplier criterion, where e.g. $(d=-3,\ q=3)$ rescaled by framework integers lands at **+0.9077 ppm** vs $m_\mu/m_e$ — *tighter* than the canonical $d=-4$ fit. A genuine theorem would not flip under a definitional choice; the proven content is the arithmetic $|\mu_K|=|\text{disc}(K)|$ fact, not the physics landing.

**Scan-domain restatement (PERMANENT, 2026-07-01, FTD-0355 — closes FTD-0348 math flag F3; precision fix, no result change).** The FTD-0123 scan domain is: **all 43 fields of class number h ≤ 3** (complete lists — 9 at h=1, 18 at h=2, 16 at h=3, with largest $|d|$ = 163, 427, 907 respectively; the imaginary-quadratic class-number problem is solved for h ≤ 3) **plus the 20 smallest h = 4 discriminants ($|d| \le 312$)** — **63 discriminants in total, with h = 4 deliberately truncated** (20 of the 54 known h = 4 fields; the 23 with $312 < |d| \le 907$ and the 11 with $907 < |d| \le 1555$ were NOT scanned). The domain phrase in the Statement and Verification bullets below — "63 fundamental discriminants (h ∈ {1, 2, 3, 4} with $|d| \le 907$)" — is **superseded by this restatement**: "h ∈ {1..4} with $|d| \le 907$" describes an **86**-element set, not a 63-element set. All counts were independently recomputed at finalization (reduced-binary-quadratic-form enumeration over every fundamental $|d| \le 2000$). The scan's registered result (d = −4 as the sole trivial-multiplier dual-matcher *within the 63-element domain*) is unchanged; the frozen `PREREG_DAMERELL_SCAN_v1.md` is untouched by this note (its own complement-count fix is owner-only, per FTD-0348 §3).

**Criterion declaration (load-bearing, FTD-0124).** This theorem holds under the **trivial-multiplier criterion**: a "match" requires the natural root x_± of P_d(x) to equal the target dimensionless constant directly (q = 1 in the rational-multiplier search). The analogous statement under the **rational-multiplier criterion** (allow rescaling by any q ≤ 200 with framework-integer factorability) FAILS — 20 additional non-canonical matches exist in the 5814-grid. **Cite this criterion explicitly when invoking Theorem 3.**

**Statement.** Let E_d denote the CM elliptic curve with complex multiplication by the ring of integers of K = ℚ(√−d) for d a fundamental imaginary-quadratic discriminant. Construct the analogue of P(x) (Theorem 2) using the lemniscatic-analogue constant G*_d defined via the Γ-product `G*_d := ∏_{a=1}^{|d|−1} Γ(a/|d|)^{χ_d(a)}` (which reproduces canonical G* exactly at d = −4). Among the 63 fundamental discriminants checked (h ∈ {1, 2, 3, 4} with |d| ≤ 907), the discriminant d = −4 is the **unique** value for which both roots simultaneously match dimensionless physical constants under the trivial-multiplier criterion at master-quadratic precision (1.26 ppm on x_+ vs 1/α; the historical 0.80% match on x_− vs N_c used the pre-v1.4 target pair — `x_-  N_c` is retired per v1.4 §5).

The mathematical privilege of the lemniscatic curve is grounded in the following uniqueness theorem.

**THEOREM (Uniqueness of the Lemniscatic Coincidence).** Among all imaginary quadratic fields K = ℚ(√−d) (d a positive squarefree integer), the field K = ℚ(i) (d = 1, corresponding to fundamental discriminant d = −4) is the **unique** one satisfying

$$|\mu_K| = |\text{disc}(K)|$$

For ℚ(i): $|\mu_K| = 4$ and $|\text{disc}(K)| = 4$. For every other imaginary quadratic field, $|\mu_K| \neq |\text{disc}(K)|$.

**PROOF.**

The unit group order $|\mu_K|$ of an imaginary quadratic field K is classically determined as:
* $|\mu_K| = 4$ if d = 1 (K = ℚ(i)),
* $|\mu_K| = 6$ if d = 3 (K = ℚ(ρ), the Eisenstein field),
* $|\mu_K| = 2$ for all other squarefree d ≥ 2.

The discriminant $\text{disc}(K)$ of K = ℚ(√−d) is:
* $\text{disc}(K) = -4d$ if d ≡ 1, 2 (mod 4),
* $\text{disc}(K) = -d$  if d ≡ 3 (mod 4).

In particular, the absolute discriminant satisfies $|\text{disc}(K)| \geq 3$ for every imaginary quadratic field, with $|\text{disc}(K)| = 3$ only for d = 3, and $|\text{disc}(K)| = 4$ only for d = 1.

We check each case of $|\mu_K|$:

1. **Case $|\mu_K| = 2$** (all squarefree d ≥ 2 except d = 3): The coincidence requires $|\text{disc}(K)| = 2$. However, $|\text{disc}(K)| \geq 3$ for all imaginary quadratic fields, which makes this case impossible.
2. **Case $|\mu_K| = 4$** (d = 1, K = ℚ(i)): The absolute discriminant is $|\text{disc}(ℚ(i))| = 4 \cdot 1 = 4 = |\mu_K|$. Thus, the coincidence holds uniquely.
3. **Case $|\mu_K| = 6$** (d = 3, K = ℚ(ρ)): The absolute discriminant is $|\text{disc}(ℚ(ρ))| = 3 \neq 6$.

Hence, d = 1 is the unique solution. ∎

*(Note: This structural fact is numerically verified for all squarefree d ∈ [1, 200] by the unit test `test_mu_disc_coincidence_unique_to_Q_i` in `scripts/tests/test_gstar_sym_k_eigenlines.py`.)*

**Verification.** Three pre-registered exhaustive numerical scans:
- `scripts/proofs/scan_cm_curves.py` (original h=1 over 9 Heegner)
- `scripts/proofs/proof_chowla_selberg_higher_h_scan.py` (FTD-0123, classes 1–4 / 63 discriminants; pre-reg tag `preregister-chowla-selberg-higher-h-scan-v1`)
- `docs/theory/10_eft_program/archive/campaign_complete/PREREG_HEEGNER_TOWER_RIGIDITY.md` + `docs/theory/10_eft_program/AUDIT_HEEGNER_TOWER_RIGIDITY.md` (FTD-0124, full 5814-grid criterion-bifurcation analysis)

See also `docs/theory/07_assessment/AUDIT_MASTER_QUADRATIC.md` for context.

**LEDGER:** FTD-0003 (quick-index entry, `[THEOREM]`); FTD-0123 (Γ-product extension to h ≥ 2); FTD-0124 (rigidity-scan + criterion bifurcation finding); FTD-0181 (Integer-4 Unification and $|\mu| = |\text{disc}|$ coincidence).

**Dependencies:** Theorems 1, 2; arithmetic of imaginary quadratic fields; Damerell-style identities at h ≥ 2 (theory note in `EXPLR_CHOWLA_SELBERG_HIGHER_H.md` for the proper analogue).

**What it does NOT claim.**
- Uniqueness under the rational-multiplier criterion. **20 non-canonical strict matches exist** in the FTD-0124 5814-grid under q ≤ 200 + FC-factorability. The cleanest non-canonical example: (d=−3, c=3, x_+, m_μ/m_e) at +0.908 ppm via multiplier 13³/(2·7²) = N_eff³/(2·b_3²) — every parameter a framework integer. This rational-multiplier reading is [SELECTION], not [THEOREM]; see SSB-3 (`DERIV_SPIN_STATISTICS_BRIDGE.md`) for an example where the framework already invokes a non-trivial multiplier (91/732) explicitly.
- The full per-ideal-class Damerell formula at h ≥ 2 has not been used; FTD-0123 uses the Γ-product analogue G*_d. The full Damerell scan would multiply the search space by ~1.5× without changing the result type.

---

## 4 · Theorem 4 — Coefficient 16 from |Aut(E)|²

**Statement.** Let E: y² = x³ − x be the lemniscatic CM elliptic curve.
Its automorphism group over ℚ̄ has order 4, so |Aut(E)|² = 16. The
coefficient 16 in the master quadratic P(x) (Theorem 2) coincides with
this automorphism-group order squared.

**Status (PERMANENT classification, 2026-07-01, FTD-0355 — finalized;
no longer promotion-pending).** The arithmetic
fact |Aut(E)| = 4, hence |Aut(E)|² = 16, is a `[THEOREM]`. That the
master quadratic's coefficient is *forced* to equal |Aut(E)|² is
**[SELECTION — declared, no longer awaiting proof]**: a bounded-effort
finalization search (FTD-0355) found no corpus result that forces it —
the stabilizer bridge |Aut(E_i)|² = |Stab_{O_h}(e₃)|
(`DERIV_DUAL_DERIVATION_OF_16.md`, whose own Honesty Note disclaims
forcing of the master-quadratic power) and the tower-level k = 4
unification (FTD-0122 / Paper B Thm 5.1) are partial structural
unifications, not forcing theorems (`TRACKER_ONTIC_TRUTH.md` OT-4.1,
Tier 4). The Proof below establishes only the arithmetic fact, not the
forcing. **Closure evidence for declaring rather than awaiting:** Paper
A's three negative tests N1–N3 (`PAPER_GSTAR_INTRODUCTION.tex`, Remark
`rem:three-negatives` in the "Where the polynomial form comes from —
and where it doesn't" subsection `sec:not-CM-derived`; cited elsewhere
in the corpus as "Paper A §13.5") found no CM-internal arrow — class
polynomial, η-quotient PSLQ, or Hecke eigenvalue — that produces the
polynomial *form*; per Paper A's own scope remark (`rem:n1-n3-scope`)
these bound the searched depth and are not a nonexistence proof, which
is exactly why the permanent tag is [SELECTION — declared], not
[CLOSED NEGATIVE]. Any future forcing proof would be a **new result
requiring its own LEDGER row**, not a pending promotion of this one.
Theorem 4 remains one of the two honestly-tiered
subsidiary results (with Theorem 7), not one of the seven
theorem-grade results (count convention: see the Subsidiary note
above §0).

**Proof.** Three independent arithmetic routes establish 16 =
|Aut(E)|²:
(a) automorphism-group computation on E directly;
(b) the j-invariant j(E) = 1728 = 12³ has stabilizer of order 4 in the
    moduli space M_1,1;
(c) the period-lattice ratio τ = i has automorphism group ℤ/4ℤ in
    SL(2,ℤ) under the modular action.

See `docs/theory/08_structural/EXPLR_COEFFICIENT_16.md` and
`MONOGRAPH_GSTAR_BRIDGE_CONSTANT.md` §6.

**LEDGER:** FTD-0006 (Coefficient 16 from |Aut(E)|² — Route A, [THEOREM]). *(Previous pointer to FTD-0014 was a typo; FTD-0014 was the unrelated x_-  N_c identification, now retired per v1.4 §5 / commit `ca7eb61`.)*

**Dependencies:** Theorems 1, 2; basic CM curve theory.

**What it does NOT claim.** The coefficient 16 has any physical
interpretation. It is an arithmetic invariant of E.

---

## 5 · Theorem 5 — Watson identity

**Statement.** The Watson integral W₃ on the BCC sub-lattice of the 3D cubic lattice satisfies

$$W_3 = \frac{G^{*2}}{2\pi} = 2 G^2.$$

Equivalently, the BCC eigenvalue triple cosine product evaluates to G*²/(2\pi), which is exactly twice the square of the Gauss Constant $G$:
$$W_3 = 2 G^2 \approx 1.39320393$$

**Proof.** Watson's original 1939 computation evaluates W₃ in closed form via the lemniscatic period integral; the connection to G* is direct application of Theorem 1. Substituting the Gauss AGM bridge constant $G^* = 2 G \pi^{1/2}$ into Watson's expression collapses it identically to $W_3 = 2 G^2$. See `docs/theory/08_structural/DERIV_BCC_MULTIPLICATIVE_STRUCTURE.md` and the references therein to Watson 1939.

**LEDGER:** Theorem-level subsidiary of FTD-0001; explicitly named in
Watson's literature.

**Dependencies:** Theorem 1; Watson's integral identities.

**What it does NOT claim.** That the Watson identity drives any
physical mechanism. The attempt to use the
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

**Subsidiary (retarded extension).** Phase G is the
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

**Status (retagged 2026-07-01, FTD-0350 — adversarially verified, CONFIRMED-WITH-SCOPE-CORRECTION).**
**[THEOREM at all L ≥ 2]**: for any real-coefficient translation-invariant
first-difference stencil D used consistently in both the divergence and the
kinetic norm (the FTD-0090 matched-stencil discipline, a [SELECTION] whose
choice fixes only the realizable domain *within the consistent class*), on the
Gauss-realizable space S_phys(D) = {s : s-hat vanishing on Ker(D)}, conditional
on the exact-constraint lambda_G-to-infinity [AXIOM] (SPEC_FTD_LAGRANGIAN.md
S3.3; softened by ultralocality at every finite lambda_G > 0 on S_phys),
S_E = (c^2/2 + g_c)*Sum s^2. Instances: **forward** (engine-Poisson-consistent)
— no exclusions beyond global neutrality, nonvacuous at every L including L=2
(all 1107 neutral configs); **centered** — nonvacuous at odd L, restricted at
even L to the 8-parity-sublattice-neutral subspace, **vacuous at L=2**. The
engine's historical mismatched pairing (FTD-0090 SOR) is genuinely
non-ultralocal at L >= 4 and is **not covered** by this theorem. The prior
[AMBIGUOUS/OPEN at L >= 4] is closed as a **proven masking artifact** (the old
3–28% spread is exactly the kernel content of constraint-unsolvable
configurations) — proof + verification:
`docs/theory/09_mathematical/ANALYSIS_PHASE_J_ZERO_MODES_v1.md` +
`scripts/proofs/proof_phase_j_zero_modes.py` (44/44). **Headline-count note:**
the 7+2 count convention (S0) is NOT changed by this retag — whether the all-L
theorem moves Theorem 7 into the theorem-grade bucket is an owner decision
pending, since the result is conditional on the stencil-consistency
[SELECTION]; until then Theorem 7 stays counted as honestly-tiered. The L = 2 case is proven by explicit
construction. `scripts/proofs/proof_phase_j_general_L.py`
shows L = 3 charge-neutral configurations are **also ultralocal to machine
precision** (action spread 8.9e-16), because the matched-stencil Laplacian
λ(k) is non-degenerate on every nonzero k at L = 3. At L ≥ 4 the Laplacian
acquires zero modes (e.g. k = (0,0,π) at L = 4) that lie in the Gauss-excluded
kernel; the naive scan then shows placement-dependent S_E (≈3–28% spread), but
this is plausibly a setup/masking artifact, not a structural failure — so
L ≥ 4 is **ambiguous**, neither confirmed nor cleanly disconfirmed. Honest
reading: ultralocality is PROVEN at L = 2, holds numerically at L = 3, and is
OPEN at L ≥ 4. Spine count is unchanged at seven theorem-grade + two
honestly-tiered — Theorem 7 sits in the tiered bucket precisely because
of this L ≥ 4 openness (count convention: see the Subsidiary note
above §0).

**Statement (L = 2, [THEOREM]).** The classical FTD partition function
on a 2³ lattice has Euclidean action S_E that depends on the state
field s ∈ {−1, 0, +1}^{8} only through Σ_i s_i² (the count of
manifested sites). The action is invariant under arbitrary spatial
permutations of charge placement at fixed charge count.

**Statement (general L, [NUMERICAL EVIDENCE / OPEN]).** With the matched
centered first-derivative stencil, charge-neutral configurations at fixed
Σ_i s_i² remain **ultralocal at L = 3** (action spread ~9e-16, machine
precision). At L ≥ 4 the scan shows placement-dependent S_E (≈3–28% spread),
but those configurations have support on Gauss-excluded zero modes of λ(k),
so the result is **ambiguous** — it may reflect a setup issue rather than a
structural failure of ultralocality. See
`scripts/proofs/proof_phase_j_general_L.py` (Test 2 + the `main()` note) for
the L ∈ {3, 4, 6, 8} scan. The earlier "DISCONFIRMED at L ≥ 3" claim is
**retracted** (L = 3 is ultralocal).

**Origin of the L=2 phenomenon.** On a 2³ lattice with centered
first-derivative ∂_i (eigenvalue i·sin(k_i)), the only available
momenta are k_i ∈ {0, π}, giving sin(k_i) = 0 for all non-zero modes.
The kinetic term Σ |∇J|² is therefore identically zero for every
configuration — trivially ultralocal. The continuum Parseval identity
Σ |∇J|² = Σ s² holds cleanly at L = 2 (all nonzero modes have sin(k_i) = 0)
**and also at L = 3** (λ(k) non-degenerate on every nonzero k). It becomes
ambiguous only at L ≥ 4, where Gauss-excluded zero modes of λ(k) require
special treatment (L = 3 is ultralocal).

**Proof (L = 2 only).** Explicit construction of the L=2 partition
function in `docs/theory/10_eft_program/DERIV_PARTITION_FUNCTION_L2.md`.

**LEDGER.** FTD-0005 (Phase J partition-function ultralocality at L=2)
tag UNAFFECTED — the LEDGER row never claimed general-L ultralocality,
so the L=2-only scope of the spine §7 entry is consistent with the
canonical LEDGER scope. The methodological clarification is recorded as a
`[METHODOLOGICAL CLARIFICATION]` LEDGER row; no claim promotion or demotion.

**Dependencies:** None beyond the FTD axioms (5 postulates per
SPEC_FTD.md).

**Consequence (NOT promoted to theorem in this document).**
Ultralocality of the classical action at L=2 means classical
extremization on the 2³ lattice cannot fix the gauge coupling g_c —
informationally, the action sees only Σ s², not the spatial
structure that g_c would couple to. **(Corrected 2026-07-01, FTD-0350:
this paragraph previously called the obstruction "L=2-specific," claiming
placement dependence at L >= 3 — contradicting this section's own L=3
machine-precision ultralocality statement above. Under the all-L theorem the
informational obstruction holds at every L on the realizable space — a
strengthened negative for the g_c program; g_c stays [PARAMETRIC], nothing
promoted.)** A quantum
extension (Mechanism B per FTD-0031) was attempted as the remaining
first-principles route for g_c and closed NEGATIVE (circular in the
boundary of the projection). g_c remains [PARAMETRIC].

**What it does NOT claim.** That the L=2 ultralocality has any
particular physical content beyond its information structure at L=2.
The proven scope is L = 2; L = 3 holds numerically; L ≥ 4 is OPEN/ambiguous
(not "DISCONFIRMED").

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
in `docs/theory/03_derivations/electromagnetism/THEOREM_HARMONIC_INVARIANT_TOWER.md`.
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
the [STRONGLY MOTIVATED CONJECTURE] tag of the x₊  1/α identification
(see §11). The x_-  N_c identification was retired per v1.4 §5 (see §11).

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
  The Tier-I closure pass reclassifies this as an
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

**Statement.** `Q(G*)` is a `π`-free subfield of
`Q(π, Γ(1/4))` (conditional on Chudnovsky 1976). Specifically:

<!-- NOTE: "maximal" is deliberately NOT claimed. The proof below establishes only
     containment + π-freeness (Q(G*) ∩ Q(π) = Q); no maximality argument exists, and as stated
     "maximal" would be false — Q(Γ(1/4)) is a strictly larger π-free subfield by the identical
     Chudnovsky argument. The "What it does NOT claim" block (below) states this.
     Maximality is [OPEN/CONJECTURE], not part of the theorem. -->


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
characterizes `Q(G*)` as **a distinguished π-free subfield** of
`Q(π, Γ(1/4))` — the algebraic content of the spine that is invisible
to `Q(π)` alone. (It is **not** claimed maximal or canonical; see "What
it does NOT claim" below.)

**LEDGER:** FTD-0112.

**Dependencies:** Theorem 1 (`G*` identity), Chudnovsky 1976.

**What it does NOT claim.**
- That `G*` is in OEIS under its own A-number. (A085565 is the
  lemniscate constant `L = 2ϖ`, not `G*`. No
  A-number for `Γ(1/4)/Γ(3/4)` itself is confirmed.)
- That Gauss computed `G*` as a privileged object. (Gauss computed
  `Γ(1/4)` and `ϖ`; the specific ratio `Γ(1/4)/Γ(3/4)` is not
  attributed to Gauss unless a citation is produced.)
- Maximality of `Q(G*)` as a subfield in any sense beyond
  `π`-freeness. There may be larger π-free subfields of
  `Q(π, Γ(1/4))`; the theorem does not exclude them.

**Provenance.** Archived at
`docs/theory/07_assessment/campaigns/archive_session_outputs/SESSION_SYNTHESIS_2026-04-30.md` §1,
where this is "Theorem 3".

### 9.1 · Operational reading via the parity-twist (FTD-0127)

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
`docs/theory/09_mathematical/EXPLR_GSTAR_ARITHMETIC_IDENTITIES.md`. Verification
scripts: `scripts/proofs/proof_g_star_parity_twist.py` (identity check)
and `scripts/proofs/proof_lprime_chi4_boundary.py` (Identities A/B/C at
high precision). LEDGER row: FTD-0127.

**External ensemble exhibit (FTD-0366; no spine change — the count stays
7+2).** The ℤ₄-sector decomposition of the strongly-coupled quartic matrix
model of Córdova–Heidenreich–Popolitov–Shakirov (Commun. Math. Phys. 361
(2018) 1235–1274, arXiv:1611.03142) realizes this parity split in ensemble
language: the (ℤ/4)^× = {1,3} conjugate contour sectors carry Γ(1/4)- and
Γ(3/4)-classes, their χ₋₄-symmetric combination (product) is π-valued and
their χ₋₄-antisymmetric combination (ratio) is `G*`-valued, and the even
sector is pure π-class — machine-verified in
`scripts/proofs/proof_gstar_matrix_models.py` (155/155); see
`docs/theory/09_mathematical/general_math/EXPLR_GSTAR_MATRIX_MODELS.md`
(scholarly attribution, not third-party validation).

---

## 10 · Subsidiary theorems and structural nulls

These are smaller [THEOREM]-level claims that depend on the seven
above:

- **D = 3 from |Aut(E)|² = 2^D · (D−1)!** — the *arithmetic* uniqueness
  (f(D) = 2^D·(D−1)! equals 16 only at D = 3; f(1..5) = {2, 4, 16, 96, 768})
  is a **[THEOREM]**. The *dimension-forcing* itself is **[SELECTION]**, not
  forced: the LHS |Aut(E_i)|² = 16 is a
  D-independent constant, and the RHS target value is justified via
  |O_h|/3 = 48/3, which already presupposes D = 3 (circular). See
  `docs/theory/02_foundations/DERIV_D3_FROM_AUTOMORPHISM.md` (the previously
  cited `THEOREM_D_EQUALS_3.md` does not exist in the checkout).
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
  `FOUND_AXIOM_ZERO.md`. A future stylistic refactor
  could group these under a unified "$Z_4$ algebraic-spine" subsection;
  not done in the current spine version.

### 10.Y · G\* opus subsidiaries

The G\* publication opus (Papers A/B/C/D/E in `docs/papers/`,
~48 pages total) consolidates the algebraic spine into publication-ready
mathematics. None of the opus results introduce a 10th theorem to this spine
in the sense of new independent algebraic content; they refine and
explicitly state material already covered by Theorems 1-9. Spine count remains 9.

- **χ\_{-4} four-level unification (Paper A §16, [THEOREM], FTD-0163)** —
  the Kronecker character $\chi_{-4}$ on $(\Z/4\Z)^\times$ generates the
  entire $G^*/G_G$ identity algebra through four functorial projections
  (lattice, Chowla-Selberg, Hecke, Dirichlet). Subsidiary to Theorems 1, 2, 9.
  This is the explicit motivic-weight tower statement; consistency of the
  four projections at $\tau = i$ is Deligne's period conjecture restricted to
  the CM case (proved unconditionally; Blasius 1986, Anderson 1986, Shimura 1979).

- **Sym²⊕Sym³ exponent constraint set (Paper A §16.5, [THEOREM: constraint set]
  + [SELECTION: (2,3) choice], FTD-0175 — corrected 2026-07-01/02, FTD-0351)** —
  among leading-period polynomials $x^2 - 16\,G^{*a}\,x + 16\,G^{*b}$ with
  $a < b$ positive integers and prefactor 16, the criteria
  (roots not *constant* multiples of a single $G^{*k}$; positive discriminant)
  constrain $(a,b)$ to $\{a < b < 2a\} \cup \{b = 2a+1\}$, whose minimal-$a$
  element is $(1, 3)$. **The former "(2,3) uniquely minimal-$a$" [THEOREM] is
  retracted** (FTD-0348 §3.1 finding: the old proof's Case-A/Case-C split was
  notationally vacuous — the roots are identically
  $8G^{*a} \pm 4\sqrt{4G^{*2a} - G^{*b}}$ in every case — and $(1,3)$ survives
  every substantive criterion, $\Delta(1,3) = 64G^{*2}(4 - G^*) > 0$). The
  $(2,3)$ selection is conditional: $a = 2$ from the independently proven
  Watson trace $16G^{*2} = 32\pi W_3$ (Theorems 1/5 lineage; FTD-0002/0006),
  $b = a{+}1$ from the $\mathrm{Det} = \mathrm{Tr}\cdot G^*$ ansatz — the
  [UNDERDETERMINED] W-CRIT-2 assembly (FTD-0235). Residual Conjecture 16.5.2
  (full $\text{Sym}^a$ coefficients, restated as an admissible-set conjecture)
  remains open. Subsidiary to Theorem 2.

- **L(E\_lemn, 1) = ϖ/4 closed form (Paper A §11, [THEOREM], FTD-0159 corrected)** —
  the central L-value of the lemniscatic curve has clean closed form
  $L(E_{\mathrm{lemn}}, 1) = \varpi/4 = \pi G_G / 4 = G^* \sqrt{\pi}/8$
  via BSD on CM rank-0 curves (Rubin 1991, Inventiones 103). **Errata note**:
  earlier session work had ϖ/2 due to BSD-formula convention-mixing; the
  factor-of-2 correction was caught by ivy-league CM-theorist red-team
  (FTD-0174). Subsidiary to Theorem 1 (existence of clean Γ-product
  closed forms) and Theorem 5 (Phase G geometric Coulomb at lattice scale).

- **h=1 atlas + η-tower (Papers C, D, [THEOREM])** — extends the
  lemniscatic structure to all nine class-number-one IQ fields via
  $G_K = \prod \Gamma(a/|d_K|)^{\chi_{d_K}(a)\,w_K/4}$. The η-tower formula
  $|\eta(\tau_K)|^{2 w_K} = G_K^{w_K}/(2\pi|d_K|)^{w_K/2}$ unifies the
  Heegner near-integer phenomenon $e^{\pi\sqrt{163}} \approx 640320^3 + 744$
  as a χ\_{-163}-projection. Subsidiary to Theorems 1-3 (extending the
  CM-uniqueness theorem from a single field to the full atlas).

---

## 11 · The empirical observation (NOT a theorem — explicit boundary)

The larger root of the master quadratic (Theorem 2) matches the
inverse fine-structure constant:

$$x_+ = 137.0362\ldots \approx 1/\alpha \quad (\text{1.26 ppm})$$

This is the central physics identification carried by the algebraic
spine. It is recorded in the LEDGER as

- **FTD-0013** [STRONGLY MOTIVATED CONJECTURE] x₊ identification with 1/α

It is NOT promoted to theorem. The motivation is the algebraic rigidity
of `P(x)` (Theorem 2 + uniqueness from Theorem 3) plus the **adversarial
look-elsewhere scan** (FTD-0319; formerly cited as "FTD-0189", which
is the graviton-audit id; pre-reg tag
`preregister-adversarial-look-elsewhere-v1`): across **2.65 M degree-2
polynomials** over an 18-constant basket FTD did not design, the master
quadratic is the **unique dual-matcher** — zero non-G* dual-matchers,
rank 1 by ~130×. That is structural evidence of polynomial-template
uniqueness; it is not a derivation of α from FTD axioms. **Honest caveats:**
the ~130× rank gap is *within the G\*-family*
(the rank-2 is itself a G\* polynomial); uniqueness is asymmetric-
tolerance-conditioned (x₊ at 2 ppm vs x₋ at 1% — under a symmetric 1% gate
~32 dual-matchers appear across 11 constants); and the "~4×10⁵:1 Bayes"
figure cited elsewhere is **not** computed by the runner (which yields only
a ~19× scan-size factor). Read this as a `[NUMERICAL FACT]` (unique
dual-matcher under the registered gate), not a structural Bayes result.

The polynomial's smaller root, `x_- = 3.0240…`, is a mathematical
artifact of the quadratic. **The identification `x_-  N_c` is
RETIRED** per FTD/FQCR Cleanup Taxonomy v1.4 §5 — the original LEDGER
row FTD-0014 was removed in commit `ca7eb61`. FTD's `N_c = 3` is
independently sourced — see `DERIV_NC_FROM_TOPOLOGY.md` (four routes)
and the Moore Layer Theorem (`DERIV_BCC_MULTIPLICATIVE_STRUCTURE.md`,
where `W_3 = G*²/(2π)` and `SU(3)` both arise from the BCC eigenvalue's
triple cosine product). The FTD-0319 scan's "dual-matcher" criterion
historically used `(1/α, N_c)` as the target pair; the polynomial-
uniqueness fact it establishes is independent of which physical
constant one tries to identify with `x_-`, and is unchanged by the
retirement.

The methodological gap that prevents promotion of the α identification:
there is no derivation chain from the FTD axioms to it. All three
attempted derivation routes for the gauge
coupling g_c (Mechanisms A, B, C) have closed negative, so the route
from "P(x) root x₊" to "physical α" runs through `g_c` at
[PARAMETRIC] status.

The honest reading: `P(x)` is a specific polynomial (Theorem 2) whose
larger root numerically matches 1/α. The FTD-0319 scan shows no other
polynomial in a large, fairly-chosen neighborhood produces a comparable
match. That is structural evidence the polynomial is special; it is
not a derivation of α from FTD axioms.

---

## 12 · What this document allows you to claim

In order from most to least defensible:

1. "FTD has a rigorous algebraic core: seven theorem-grade results
   (Theorems 1, 2, 3, 5, 6, 8, 9 — Theorem 3 at its arithmetic core
   only, |μ_K| = |disc(K)| unique to ℚ(i); its physics dual-match
   landing is separately [NUMERICAL FACT]) plus two honestly-tiered
   subsidiary results (Theorems 4, 7 — see the count convention in
   the Subsidiary note above §0), centered on the constant
   G* = Γ(1/4)/Γ(3/4) = √2·Γ(1/4)²/(2π) ≈ 2.9587 (distinct from the
   Bernoulli/Gauss lemniscate constant ϖ ≈ 2.622; see §1)." — §§1-9.

2. "A specific polynomial P(x) = x² − 16G*²x + 16G*³ has a root
   matching 1/α to 1.26 ppm (the historical x₋ ↔ N_c reading, 0.8%,
   is RETIRED — see §2/§11); under the FTD-0319 adversarial scan the
   polynomial is the unique dual-matcher over an 18-constant basket,
   with the tolerance-conditioning caveat declared in §11 and the
   Theorem-3 criterion declaration of §3 required whenever the CM
   uniqueness is invoked." — Theorems 2, 3 + observation §11.
   *(Corrected 2026-07-01, FTD-0348 — this item previously claimed a
   live dual match "to permille precision" [x₊ is ppm, x₋ was 0.8%],
   pointed at §9 for an observation that lives in §11, and omitted
   both mandatory caveats.)*

3. "The corresponding lattice simulator reproduces the lattice Poisson
   Green's function as its Coulomb interaction exactly, with no
   fine-structure-constant content in the coupling-free limit." —
   Theorem 6.

4. "The classical FTD action is ultralocal in state space; classical
   extremization cannot fix the gauge coupling, and
   all three first-principles derivation routes for g_c have closed
   negative." — Theorem 7 + LEDGER FTD-0031 + FTD-0093.

5. "The physical identification of P(x)'s dominant root with 1/α is a
   structurally-motivated conjecture, not a derivation; it sits in the
   LEDGER at [STRONGLY MOTIVATED CONJECTURE]. The look-elsewhere scans
   HAVE been run — FTD-0097 [MEASURED, 2026-04-27] and the FTD-0319
   adversarial 18-constant scan [NUMERICAL FACT, with the §11
   tolerance-conditioning caveat] — and support selectivity without
   promoting the identification; promotion still requires a
   derivation." — §11 + FTD-0013 / FTD-0097 / FTD-0319.
   *(Corrected 2026-07-01, FTD-0348 — this item previously described
   FTD-0097 as awaiting a run it completed on 2026-04-27, cited the
   retired x₋ identification under FTD-0017 — which is the Higgs-mass
   parametric row, not a Bell/N_c id — and pointed at §9 for §11.)*

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
| 7 Phase J ultralocal (THEOREM at L=2; NUMERICAL EVIDENCE at L=3; OPEN/ambiguous L≥4) | `DERIV_PARTITION_FUNCTION_L2.md` | `partition_function_L2.py` + `proof_phase_j_general_L.py` | FTD-0005 area |
| 8 Harmonic invariant tower (rows added 2026-07-01, FTD-0348 — this table previously stopped at Theorem 7) | `docs/theory/03_derivations/electromagnetism/THEOREM_HARMONIC_INVARIANT_TOWER.md` | `proof_harmonic_invariant_tower.py` | FTD-0111 |
| 9 Q(G*) π-free subfield (conditional on Chudnovsky 1976) | `docs/theory/07_assessment/campaigns/archive_session_outputs/SESSION_SYNTHESIS_2026-04-30.md` §1 | `proof_field_theoretic_qgstar.py` | FTD-0112 |

| Subsidiary | Primary doc | LEDGER |
|---|---|---|
| D = 3 (arithmetic uniqueness THEOREM; dimension-forcing [SELECTION]) | `DERIV_D3_FROM_AUTOMORPHISM.md` | FTD-0010 / FTD-0036 area |
| Moore integers | `THEOREM_MOORE_LAYER_DECOMPOSITION.md` | FTD-0008 area |
| a_phys ≡ ℓ_P no-go | `THEOREM_A_PHYS_NO_GO.md` | FTD-0059 |
| Phase H scaling | `DERIV_EMERGENT_COULOMB_GEOMETRIC.md` §H | (Phase H) |

| Empirical observation | LEDGER |
|---|---|
| x+ = 1/α | FTD-0013 [STRONGLY MOTIVATED CONJECTURE] |
| x− = N_c | **RETIRED** (was FTD-0014, removed in `ca7eb61`; see §2/§11 — N_c=3 is independently sourced. This row previously mis-cited FTD-0017, the Higgs-mass parametric row; corrected 2026-07-01, FTD-0348) |
| Look-elsewhere scans | FTD-0097 [MEASURED, 2026-04-27] + FTD-0319 adversarial scan [NUMERICAL FACT, tolerance-conditioned — see §11] (this row previously said "not yet run"; corrected 2026-07-01) |

---

## 14 · Single-line summary

**FTD's algebraic spine is seven theorem-grade results plus two honestly-tiered subsidiary results (nine numbered sections; Theorem 3 counting at its arithmetic core only — see the count convention above §0), centered on G* =
Γ(1/4)/Γ(3/4) ≈ 2.9587: the G* identity (Chowla-Selberg), the master
quadratic P(x) = x² − 16G*²x + 16G*³ and its two roots, CM uniqueness
within class-number-1 fields, the coefficient 16 = |Aut(E)|² for E:
y² = x³ − x, the Watson identity W₃ = G*²/(2π), the Phase G geometric
Coulomb α_r = 2r·G_L(r), Phase J classical-action ultralocality, the
(1+i)-tower harmonic invariant 1/y₊ + 1/y₋ = 1 with anomaly transcendence
A_k ∉ Q for k ≥ 4, and the field-theoretic characterization of Q(G*)
as a π-free subfield of Q(π, Γ(1/4)) (conditional on Chudnovsky
1976; "maximal" not claimed — only π-freeness is proven). All nine
are independent of physics interpretation. The
numerical match x+ ≈ 1/α (1.26 ppm) is recorded as
[STRONGLY MOTIVATED CONJECTURE], not theorem (the historical
x− ≈ N_c reading is RETIRED, see §2/§11 — corrected 2026-07-01,
FTD-0348, this summary previously presented it live); promotion
requires a derivation (all three first-principles routes for g_c are
closed-negative) — the look-elsewhere scans have been run (FTD-0097
[MEASURED, 2026-04-27]; FTD-0319 adversarial scan, tolerance-conditioned
per §11) and support selectivity without promoting the identification.

---

## 15 · From theorems to physics — see the dimensional map

The nine theorems above are dimensionless. Their connection to
physical-unit observables (m_e in MeV, lifetimes in seconds, lengths
in meters) goes through the **calibration interface**: exactly two
SI-dimensional anchors (`a_phys ≡ ℓ_P` and `K_B = m_e`) are
theorem-enforced as the irreducible minimum (FTD-0059 + FTD-0096).

The full dimensionless  dimensional bridge — including the seven
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
