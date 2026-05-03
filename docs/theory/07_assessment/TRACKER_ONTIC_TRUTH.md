# TRACKER · Ontic Truth

> **One document, one source of bedrock truth.** What is *actually* mathematically established in FTD — distilled, ranked, and tied to verification scripts.
>
> **This is the canonical bedrock reference.** The [LEDGER](./LEDGER.md) tracks every load-bearing claim with full provenance; this document distills LEDGER entries down to "what survives skeptical mathematical review." If the two disagree, this tracker is correct on tier-assignment; the LEDGER is correct on detailed history.

**Last regenerated:** 2026-05-02 evening (manual; this is a hand-curated tracker, not auto-generated)
**Maintainer rule:** update when a claim's tier changes, when a new theorem closes, or when a previously-rigorous claim is found to have a hidden gap. Do NOT add aspirational claims. Every entry must point at a verification artifact.

---

## Truth tiers

| Tier | Symbol | Meaning | Test |
|---|---|---|---|
| **T1** | `★★★★★` | **Rock-solid.** Pure algebra or 3-line proof; cannot be wrong. | Trivially verifiable |
| **T2** | `★★★★` | **Conditional.** Depends on a published classical theorem. Rigor equals the source. | Citation chain |
| **T3** | `★★★` | **Numerical fact.** Verified by computation across a *stated* domain. NOT a structural theorem. | Pre-registered scan |
| **T4** | `★★` | **Identification.** Coincidence at the value level; a *structural reason* is conjectured but not proved. | Open research question |
| **T5** | `★` | **Assertion.** Claimed but not proved at this tier. Includes [STRONGLY MOTIVATED CONJECTURE]. | Acknowledged gap |

Anything below T5 (e.g. [PARAMETRIC] formula insertions, [SELECTION] arguments, engine measurements) is **not raw math** and is not in this tracker. See [LEDGER.md](./LEDGER.md) for those.

---

## TIER 1 — Rock-solid theorems (6)

These are pure algebra. They cannot be wrong without an arithmetic mistake. Each has been verified to machine precision or in exact rational arithmetic.

| ID | Statement | Proof | Verification |
|---|---|---|---|
| **OT-1.1** | Master quadratic `P(x) = x² − 16G*²x + 16G*³` has roots `x_± = 8G*² ± 4G*√(4G*² − G*)` | Quadratic formula on positive discriminant `64G*³(4G* − 1)` since `G* > 1/4`. | Paper A Thm 2.2; `proof_master_verification.py` (54/54 PASS) |
| **OT-1.2** | `G* := Γ(1/4)/Γ(3/4) = Γ(1/4)²/(π√2) = 2ϖ/√π` | Euler reflection `Γ(1/4)·Γ(3/4) = π/sin(π/4) = π√2` | Paper A Thm 2.1; verified at 50-digit precision |
| **OT-1.3** | (1+i)-tower harmonic invariant: `1/y_+ + 1/y_- = 1` for `y := x/G*` at every level `k ≥ 3` of `M_k(x) = x² − 2^k G*^{k-2}x + 2^k G*^{k-1}` | Three-line Vieta: `1/x_+ + 1/x_- = (x_++x_-)/(x_+x_-) = 1/G*`; multiply by G*. | Paper A Thm 3.1(i); `proof_harmonic_invariant_tower.py` (14/14 PASS at 50 digits) |
| **OT-1.4** | Phase G geometric Coulomb: engine's gauss-projection step computes the lattice Poisson Green's function `G_L(r)` on the L³ torus by construction; `α_r(r,L) := 2 r G_L(r)` is therefore zero-free-parameter geometry, not a fine-structure observable | Direct: gauss-projection inverts the discrete Laplacian; the Green's function is what it returns | `DERIV_EMERGENT_COULOMB_GEOMETRIC.md`; `AUDIT_ALPHA_EXTRACTION.md`; R²=1.0000 at L=384, 0.07% median residual |
| **OT-1.5** | BCC complex-structure theorem: 8 BCC corners under 90° rotation form 2 orbits of size 4; `Z[BCC] ⊗ Q = V_triv² ⊕ V_sign² ⊕ V_complex²` with `V_complex` carrying natural `Z[i]`-module structure ≅ `Z[i]²` | Per-orbit Z/4 regular-rep decomposition; standard rep theory | Paper B Thm 3.1; `proof_bcc_complex_structure.py` (5/5 PASS, exact rationals) |
| **OT-1.6** | `Z[i]^× → O_h^ab` no-go: no injective homomorphism exists since Z[i]^× ≅ Z/4 has an order-4 element but O_h^ab ≅ Z/2 × Z/2 (Klein) does not | One-line group-order argument | Paper B Thm 6.1; `DERIV_BCC_COMPLEX_STRUCTURE.md` §3.2 |

---

## TIER 2 — Conditional on classical published theorems (3)

These depend on named external results from analytic number theory or transcendence theory. Rigor equals the source; the dependency is explicitly named.

| ID | Statement | Conditional on | Verification |
|---|---|---|---|
| **OT-2.1** | Watson identity: `W₃ = G*²/(2π)`, where `W₃ = (1/π³)∫∫∫_[0,π]³ dk_1 dk_2 dk_3 / (1 − cos k_1 cos k_2 cos k_3)` is the BCC Watson integral | Watson 1939; closed form via Glasser-Zucker 1980 | `DERIV_WATSON_GSTAR_IDENTITY.md`; numerically verified at 100-digit precision in PARI |
| **OT-2.2** | Tower discriminant transcendence: `A_k := 2^{k-2} G*^{k-3} − 1 ∉ Q̄` for all `k ≥ 4`. (Rational at `k = 3`.) | Schneider–Chudnovsky 1949/1976 (`Γ(1/4)` transcendental over Q ⇒ G* transcendental over Q ⇒ non-rational polynomial in transcendental over Q with rational coefficients takes transcendental values, Waldschmidt §1.4) | Paper A Thm 3.1(iii) |
| **OT-2.3** | `Q(G*)` is π-free in `Q(π, Γ(1/4))`: `Q(G*) ∩ Q(π) = Q` | Chudnovsky 1976 (algebraic independence of `{π, Γ(1/4)}` over Q) | Paper A Thm 4.1; `proof_field_theoretic_qgstar.py` |

**Note**: T2 entries are no weaker than the published source. Chudnovsky 1976 is a foundational result of contemporary transcendence theory, consolidated in Waldschmidt's Grundlehren volume. "Conditional" here means "depends on this established theorem", not "depends on a conjecture."

---

## TIER 3 — Numerical facts, exhaustive over stated domains (4)

These are **rigorous numerical results** verified across explicitly stated finite domains. They are NOT structural theorems — they say "across the domain checked, X holds," not "X holds in general."

| ID | Statement | Domain | Verification |
|---|---|---|---|
| **OT-3.1** | Phase J partition-function ultralocality at `L = 2` | `L = 2` only (Nyquist-mode degeneracy origin); general L numerically disconfirmed at L ≥ 4 | `proof_phase_j_general_L.py`; honestly retagged 2026-05-02 |
| **OT-3.2** | CM-curve numerical uniqueness: among 63 fundamental imaginary-quadratic discriminants spanning class numbers 1–4 with `|d| ≤ 907`, only `d = −4` dual-matches `(1/α, N_c)` via the natural Γ-product analogue `G*_d = ∏_{a=1}^{|d|−1} Γ(a/|d|)^{χ_d(a)}` | 9 h=1 + 18 h=2 + 16 h=3 + 20 h=4 = 63 discriminants; `|d| ≤ 907` | `proof_chowla_selberg_higher_h_scan.py` (FTD-0123); pre-reg tag `preregister-chowla-selberg-higher-h-scan-v1` |
| **OT-3.3** | Polynomial-shape uniqueness: across 2,871,576 polynomials/multipliers in the natural `M_{n,p,m,q}(x) = x² − n G*^p x + m G*^q` family + cubic embeddings + Eisenstein-integer multiplier extension, only the master quadratic dual-matches; **0 dual-matchers in the Eisenstein-integer family** | `n, m ∈ {1,…,64}`, `p, q ∈ {0,…,5}`; rational-coefficient extension; cubic embeddings; Eisenstein/Gaussian-integer multiplier sweeps | `proof_polynomial_look_elsewhere_extended.py` (FTD-0121); pre-reg tag `preregister-polynomial-scan-extended-v1`; ~4×10⁵:1 Bayes weight |
| **OT-3.4** | Linear FTD-0110 cluster coefficient: `k = 1/N_base = 1/4` from `O_h` representation theory of the 27-block (mult(A_{1g}) = 4 from character-table formula; `δ_center` is A_{1g}-pure; 4 A_{1g} eigenmodes have mean energy 1/4) | Linear regime only; nonlinear FTD-0110 is [STRONGLY MOTIVATED CONJECTURE], NOT in this tracker | `DERIV_K_FROM_OH_A1G_MULTIPLICITY.md`; engine cross-check at GPU L=128 |

**Honest caveat (OT-3.2)**: the Γ-product analogue `G*_d` reproduces canonical G* exactly at `d = −4` but at `h ≥ 2` it is a single-number analogue, not the full per-ideal-class Damerell formula. A full Damerell scan at h ≥ 2 has not been run. Reviewer pressure point.

**Honest caveat (OT-3.3)**: the scan is over a *natural* polynomial family. Broader families (rational coefficients, π in coefficients, degree ≥ 3 not factored) would change the count. The interpretation as Bayes evidence depends on the prior choice of family.

---

## TIER 4 — Identification with structural backing but no proof of necessity (1)

This is the framework's softest mathematical spot. The numerical equalities hold; the *structural identification* is conjectured.

| ID | Statement | True at value level | Structural identification (NOT proved) |
|---|---|---|---|
| **OT-4.1** | Coefficient 16 in the master quadratic equals `\|Aut_{Q̄}(E)\|²` for `E: y² = x³ − x` | `\|Aut_{Q̄}(E)\| = \|Z[i]^×\| = 4`; `4² = 16`; the prefactor in the master quadratic is also 16. **TRUE.** | That the master-quadratic prefactor *must* equal `\|Aut(E)\|²` is conjectural. The tower-level identification (FTD-0122 / Paper B Thm 5.1) gives a structural reason for `k = 4`, hence `2^k = 16`, in terms of `\|Z[i]^×\|² = 16`. This is a partial structural unification, not a forcing theorem. |

This is the entry that would draw the most reviewer pressure. The honest framing is: "Two distinct objects both equal 16; the structural reason for this coincidence is conjectured but not proved."

---

## TIER 5 — Strongly motivated conjectures (2)

These are the load-bearing identifications between FTD's algebraic structure and Standard Model constants. They are **honestly tagged as conjectures**. Substantial structural-uniqueness evidence exists; no derivation chain from FTD axioms exists.

| ID | Conjecture | Empirical match | Structural evidence |
|---|---|---|---|
| **OT-5.1** | `x_+ = 1/α` | 1.26 ppm to CODATA 2022 (`α^{-1} = 137.035999084(21)`) | OT-3.2 + OT-3.3 (combined ~4×10⁵:1 Bayes weight); OT-1.5 (Z[i] structure unifying CM Aut count and tower level k=4) |
| **OT-5.2** | `x_- = N_c = 3` | 0.80% to integer 3 | Same as OT-5.1; the dual-matching property is what distinguishes the master quadratic in OT-3.3 |

**Closed-negative routes** (preserved for provenance, do not attempt):
- R1 transverse stiffness — closed
- R2 source-current normalization — closed
- R3 two-sector response eigenvalue — closed
- R4 projected Dirac matter — closed
- Z-factor reading (FTD-0116) — closed
- RG-running, algebraic combinations, 1/√d, Langevin-equipart, monomial scans — all closed

**Lead-physicist diagnosis**: Phase J ultralocality structurally decouples the algebraic spine from the dynamical EFT — action data does not contain polynomial data. Closure plausibly requires non-action injection mechanism (boundary conditions, observable selection, quantization choice) and may require ontology extension (sixth axiom). MC-T4.3 in `CHECKLIST_MATH_COMPLETE.md`.

---

## What is explicitly NOT in this tracker

The following are **not raw math** and are tracked elsewhere:

| Category | Tag | Tracked in |
|---|---|---|
| Standard QFT formula with FTD numbers substituted | [PARAMETRIC] | `CATALOG_PARAMETRIC_INSERTIONS.md` (162 rows: ~23 [DERIVED]/[THEOREM], ~129 [PARAMETRIC], ~10 [IMPOSED]/[SELECTION]) |
| Consistency argument, not uniqueness theorem | [SELECTION] | LEDGER.md per-claim |
| Engine measurement | [MEASURED] | LEDGER.md per-claim |
| Closed by negative result | [CLOSED NEGATIVE] | LEDGER.md per-claim (preserved for provenance) |
| Calibration declaration (`a_phys ≡ ℓ_P`, `K_B = m_e`, `t_phys`) | [IMPOSED] | `SPEC_DIMENSIONAL_MAP.md`; SPEC_FTD.md |
| FTD-0110 nonlinear cluster-mass identification | [STRONGLY MOTIVATED CONJECTURE] | `DERIV_FTD0110_NONLINEAR_BRIDGE.md`; not in this tracker |

The 9 [THEOREM]-grade entries of the algebraic spine (OT-1.1 through OT-2.3) plus FTD-0110 linear (OT-3.4) plus the BCC complex-structure theorem (OT-1.5/1.6) are the bedrock. **Everything else is downstream**.

---

## Pressure points — what a hostile reviewer would push on

1. **OT-4.1 (coefficient 16)**: the only T4 entry. Honest framing: "true coincidence with structural conjecture, not proved necessity." A reviewer can press here, and the framework's response is FTD-0122 (Paper B) — partial structural unification, with explicit no-go for the rest.

2. **OT-3.2 Γ-product caveat**: we tested a natural extension at h ≥ 2, not the full per-ideal-class Damerell formula. The honest fix is to run the proper Damerell scan; effort estimate 2–6 weeks.

3. **OT-2.3 (Q(G*) π-free) and OT-2.2 (transcendence)** depend on Chudnovsky 1976. This is a published, foundational result, but a reviewer could note that algebraic-independence proofs in this domain are not always self-evident. Cite Waldschmidt explicitly.

4. **OT-5.1 / OT-5.2 (the central conjectures)**: a reviewer who insists "you must derive α from axioms or the framework is empty" cannot be answered with current methods. Honest response: the framework's mathematical content is the spine + numerical-uniqueness scans + Z[i] structural unification; the empirical match is conjecture-grade with explicit Bayes weight; a derivation chain is structurally blocked, not merely unsolved.

5. **Pre-registration discipline**: every numerical scan in T3 is git-tagged BEFORE measurement. This is methodological hygiene the framework practices. A reviewer will check tag dates against measurement dates; they should match.

---

## Update protocol

When a claim's tier changes:

1. Update this document **first** (the bedrock changes).
2. Update [LEDGER.md](./LEDGER.md) entry to match.
3. Update [CHECKLIST_MATH_COMPLETE.md](../01_reference/CHECKLIST_MATH_COMPLETE.md) if the claim is on the bridge-completeness checklist.
4. Update [SPEC_ALGEBRAIC_SPINE.md](../01_reference/SPEC_ALGEBRAIC_SPINE.md) if the claim is one of the 9 spine theorems.
5. Re-run any verification script that backs the claim.
6. Commit with message that names the tier change and the verification that backs it.

When adding a new claim:

- It must point at a verification artifact (proof script with PASS, or paper section with proof).
- It must NOT live below T3. Below T3 is not raw math; it goes in the LEDGER per-claim category.
- It must be assigned a unique `OT-N.M` ID that does not collide with existing IDs.

---

## Quick reference: the ten that matter

If you need to defend FTD's mathematical core to a skeptical mathematician in a hallway, these ten are the bedrock:

1. **OT-1.1** Master quadratic + roots (★★★★★)
2. **OT-1.2** G* algebraic identity (★★★★★)
3. **OT-1.3** Harmonic invariant tower (★★★★★)
4. **OT-1.4** Phase G geometric Coulomb (★★★★★)
5. **OT-1.5** BCC complex-structure theorem (★★★★★)
6. **OT-1.6** Z[i]^× → O_h^ab no-go (★★★★★)
7. **OT-2.1** Watson identity (★★★★, Watson 1939 / Glasser–Zucker 1980)
8. **OT-2.2** Tower discriminant transcendence (★★★★, Schneider–Chudnovsky)
9. **OT-2.3** Q(G*) π-free (★★★★, Chudnovsky 1976)
10. **OT-3.4** FTD-0110 linear (★★★, O_h rep theory)

Plus 3 numerical-uniqueness facts (OT-3.1/3.2/3.3) supporting the central conjectures.

Plus 2 honestly-tagged conjectures (OT-5.1 / OT-5.2) that the framework explicitly does NOT claim as theorems.

This is what is publishable. This is what survives review. This is the truth.

---

## Cross-references

- [LEDGER.md](./LEDGER.md) — full provenance per claim, including closed-negative results
- [CHECKLIST_MATH_COMPLETE.md](../01_reference/CHECKLIST_MATH_COMPLETE.md) — bridge-completeness roadmap
- [SPEC_ALGEBRAIC_SPINE.md](../01_reference/SPEC_ALGEBRAIC_SPINE.md) — the 9 [THEOREM]-grade entries with proofs
- [SPEC_DIMENSIONAL_MAP.md](../01_reference/SPEC_DIMENSIONAL_MAP.md) — dimensionless ↔ dimensional bridge
- [Paper A](../../../dissemination/papers/PAPER_A_PI_FREE_GENERATOR.tex) — π-free generator (T1.1, 1.2, 1.3, 2.2, 2.3, 3.3 incl Eisenstein null)
- [Paper B](../../../dissemination/papers/PAPER_B_BCC_COMPLEX_STRUCTURE.tex) — BCC complex structure (T1.5, 1.6, 4.1)
- [Paper C](../../../dissemination/papers/PAPER_FTD_AS_WILSONIAN_EFT.tex) — Branch-A native EFT (T1.4 anchored, downstream measurements)
