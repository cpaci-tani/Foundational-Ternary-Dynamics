# TRACKER · Ontic Truth

> **One document, one source of bedrock truth.** What is *actually* mathematically established in FTD — distilled, ranked, and tied to verification scripts.
>
> **This is the canonical bedrock reference.** The [LEDGER](LEDGER.md) tracks every load-bearing claim with full provenance; this document distills LEDGER entries down to "what survives skeptical mathematical review." If the two disagree, this tracker is correct on tier-assignment; the LEDGER is correct on detailed history.

**Last regenerated:** 2026-06-01 (manual; this is a hand-curated tracker, not auto-generated) — added route-invariance note to OT-5.1 (FTD-0242; α DYNAMICAL not structural; **no tier change**). Prior: 2026-05-02 evening.
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

Anything below T5 (e.g. [PARAMETRIC] formula insertions, [SELECTION] arguments, engine measurements) is **not raw math** and is not in this tracker. See [LEDGER.md](LEDGER.md) for those.

---

## TIER 1 — Rock-solid theorems (9)

These are pure algebra. They cannot be wrong without an arithmetic mistake. Each has been verified to machine precision or in exact rational arithmetic.

| ID | Statement | Proof | Verification |
|---|---|---|---|
| **OT-1.1** | Master quadratic `P(x) = x² − 16G*²x + 16G*³` has roots `x_± = 8G*² ± 4G*√(4G*² − G*)` | Quadratic formula on positive discriminant `64G*³(4G* − 1)` since `G* > 1/4`. | Paper A Thm 2.2; `proof_master_verification.py` (54/54 PASS) |
| **OT-1.2** | `G* := Γ(1/4)/Γ(3/4) = Γ(1/4)²/(π√2) = 2ϖ/√π` | Euler reflection `Γ(1/4)·Γ(3/4) = π/sin(π/4) = π√2` | Paper A Thm 2.1; verified at 50-digit precision |
| **OT-1.3** | (1+i)-tower harmonic invariant: `1/y_+ + 1/y_- = 1` for `y := x/G*` at every level `k ≥ 3` of `M_k(x) = x² − 2^k G*^{k-2}x + 2^k G*^{k-1}` | Three-line Vieta: `1/x_+ + 1/x_- = (x_++x_-)/(x_+x_-) = 1/G*`; multiply by G*. | Paper A Thm 3.1(i); `proof_harmonic_invariant_tower.py` (14/14 PASS at 50 digits) |
| **OT-1.4** | Phase G geometric Coulomb: engine's gauss-projection step computes the lattice Poisson Green's function `G_L(r)` on the L³ torus by construction; `α_r(r,L) := 2 r G_L(r)` is therefore zero-free-parameter geometry, not a fine-structure observable | Direct: gauss-projection inverts the discrete Laplacian; the Green's function is what it returns | `DERIV_EMERGENT_COULOMB_GEOMETRIC.md`; `AUDIT_ALPHA_EXTRACTION.md`; R²=1.0000 at L=384, 0.07% median residual |
| **OT-1.5** | BCC complex-structure theorem: 8 BCC corners under 90° rotation form 2 orbits of size 4; `Z[BCC] ⊗ Q = V_triv² ⊕ V_sign² ⊕ V_complex²` with `V_complex` carrying natural `Z[i]`-module structure ≅ `Z[i]²` | Per-orbit Z/4 regular-rep decomposition; standard rep theory | Paper B Thm 3.1; `proof_bcc_complex_structure.py` (5/5 PASS, exact rationals) |
| **OT-1.6** | `Z[i]^× → O_h^ab` no-go: no injective homomorphism exists since Z[i]^× ≅ Z/4 has an order-4 element but O_h^ab ≅ Z/2 × Z/2 (Klein) does not | One-line group-order argument | Paper B Thm 6.1; `DERIV_BCC_COMPLEX_STRUCTURE.md` §3.2 |
| **OT-1.7** | `G* via det_ζ quarter-conjugacy bridge`: $\det_\zeta D_{3/4}/\det_\zeta D_{1/4} = G^*$ where $D_a = \{n + a\}_{n\ge 0}$ are the spectra of operators on $S^1$ with quarter-twisted boundary $\psi(\phi+2\pi) = J\,\psi(\phi)$, $J^2 = -I$. **Arithmetic content**: $4 D_{1/4} = \{n \equiv 1\pmod 4\}$ and $4 D_{3/4} = \{n \equiv 3\pmod 4\}$ are exactly the two non-trivial residue classes mod 4; restricted to primes these are the split and inert prime classes of $\mathbb{Z}[i]$ (Fermat's two-square theorem). $G^*$ is the regularized asymmetry between them. | Lerch's formula: $\det_\zeta\{n+a\}_{n\ge 0} = \sqrt{2\pi}/\Gamma(a)$; $\sqrt{2\pi}$ cancels in the ratio leaving $\Gamma(1/4)/\Gamma(3/4) = G^*$. Equivalently $G^* = \exp[\zeta_H'(0, 1/4) - \zeta_H'(0, 3/4)]$. | FTD-0141 (2026-05-06); `DERIV_GSTAR_QUARTER_CONJUGACY.md` §5; OT-1.2 (algebraic) and FTD-0127 (parity-twist) are two further readings of the same residue-class decomposition; the three identities are unified by $G^* = (\sqrt{2\pi}/\Gamma(3/4))/(\sqrt{2\pi}/\Gamma(1/4)) = \exp[\zeta_H'(0,1/4) - \zeta_H'(0,3/4)] = \Gamma_\zeta(1/2)/\Gamma_{\chi_{-4}}(1/2)$ |
| **OT-1.8** | `G* as finite-N attractor`: $G_N^* := (N+1)^{-1/2}\prod_{n=0}^{N}(n+3/4)/(n+1/4) \to G^*$ at rate $|G_N^* - G^*| = O(1/N^2)$, empirical $C \approx 0.046$ | Stirling expansion of $\Gamma(N+7/4)/\Gamma(N+5/4) \sim (N+1)^{1/2}$ | FTD-0142 (2026-05-06); `DERIV_GSTAR_FINITE_APPROX.md`; verified by `proof_fqcr_convergence.py` (all assertions PASS); discharges `AUDIT_INFINITY_REFRAME.md` ε-L obligation for $G^*$ |
| **OT-1.9** | `CM-curve uniqueness`: $K = \mathbb{Q}(i)$ ($d = 1$, discriminant $d = -4$) is the unique imaginary quadratic field satisfying unit-group and discriminant order coincidence $|\mu_K| = |\text{disc}(K)|$ | Arithmetic evaluation of $|\mu_K|$ and $|\text{disc}(K)|$ across squarefree $d$ | `SPEC_ALGEBRAIC_SPINE.md` §3 (unit-group uniqueness proof); verified to $d \le 200$ |

---

## TIER 2 — Conditional on classical published theorems (3)

These depend on named external results from analytic number theory or transcendence theory. Rigor equals the source; the dependency is explicitly named.

| ID | Statement | Conditional on | Verification |
|---|---|---|---|
| **OT-2.1** | Watson identity: `W₃ = G*²/(2π)`, where `W₃ = (1/π³)∫∫∫_[0,π]³ dk_1 dk_2 dk_3 / (1 − cos k_1 cos k_2 cos k_3)` is the BCC Watson integral | Watson 1939; closed form via Glasser-Zucker 1980 | `DERIV_WATSON_GSTAR_IDENTITY.md`; numerically verified at 100-digit precision in PARI |
| **OT-2.2** | Tower discriminant transcendence: `A_k := 2^{k-2} G*^{k-3} − 1 ∉ Q̄` for all `k ≥ 4`. (Rational at `k = 3`.) | Schneider–Chudnovsky 1949/1976 (`Γ(1/4)` transcendental over Q ⇒ G* transcendental over Q ⇒ non-rational polynomial in transcendental over Q with rational coefficients takes transcendental values, Waldschmidt §1.4) | Paper A Thm 3.1(iii) |
| **OT-2.3** | `Q(G*)` is π-free in `Q(π, Γ(1/4))`: `Q(G*) ∩ Q(π) = Q` | Chudnovsky 1976 (algebraic independence of `{π, Γ(1/4)}` over Q) | Paper A Thm 4.1; `proof_field_theoretic_qgstar.py` |
| **OT-2.4** | Lemniscatic L-value: `L(E_lemn, 1) = ϖ/4 = πG_G/4 = G*√π/8 ≈ 0.6555143885...`, where E_lemn: y² = x³ − x (Cremona 32.a3). Full BSD formula with c_∞ = 2 real components, c_2 = 2 Tamagawa (Kodaira III), \|E_tors\| = 4, \|Sha\| = 1. | Rubin 1991 (Inventiones 103); full BSD formula for CM rank-0 case via main conjecture for imaginary quadratic fields. (Coates-Wiles 1977 proves only L≠0 ⇒ rank=0 implication, not the precise BSD ratio.) | Paper A §11 Thm Lvalue; Paper E (overview); FTD-0159 [THEOREM]; verified to 27 digits vs LMFDB 32.a3 direct. **Errata note**: earlier session work (pre-2026-05-19) had this as ϖ/2 due to BSD-formula convention-mixing (using Ω_E^+ = 2ϖ AND c_∞ = 2, which double-counts). Caught by ivy-league CM-theorist red team; FTD-0174 / FTD-0159 (revised). |
| **OT-2.5** | χ_{-4} four-level unification: the Kronecker character χ_{-4} on (Z/4Z)^× generates the entire G*/G_G identity algebra through four functorial projections: (L1) lattice \|Z[i]^×\| = 4, (L2) Chowla-Selberg ∏Γ(a/4)^{χ_{-4}(a)} = G*, (L3) Hecke a_p splitting in L(E_lemn,s), (L4) Dirichlet L(χ_{-4},1) = π/4 | Deligne's period conjecture for CM motives (Blasius 1986, Anderson 1986, Shimura 1979); standard CFT for imaginary quadratic fields | Paper A §16 Thm character-unification; FTD-0163 [THEOREM]; consistent with the algebraic-spine structure of OT-1.2 and OT-1.7. The four levels form a motivic-weight tower; consistency across L2-L3 is Deligne's conjecture restricted to the lemniscatic motive. |
| **OT-2.6** | η-tower across the h=1 atlas: for each class-number-one IQ field K with discriminant d_K and unit-group order w_K, \|η(τ_K)\|^{2w_K} = G_K^{w_K} / (2π\|d_K\|)^{w_K/2} where G_K is the Chowla-Selberg constant of K | Chowla-Selberg 1967 at h_K = 1; Selberg-Chowla Gauss-sum evaluation | Paper D Thm eta-tower; FTD covered as part of FTD-0163's extended family; verified at all 9 atlas fields to relative error < 10^-70 via `eta_atlas_verify.py`. Specialises to η(i)^8 = G*^4/(64π²) [Paper A Cor 9.2] and η(ρ)^12 = G_K^6/(216π³) [Paper A §15] |
| **OT-2.7** | Sym²⊕Sym³ uniqueness of (2,3): among leading-period polynomials x² - 16G*^a x + 16G*^b (a < b positive integers), the pair (a,b) = (2,3) is uniquely minimal-a satisfying (i) integer prefactor 16, (ii) roots not scalar multiples of any single G*^k, (iii) positive discriminant. The proof is elementary case analysis: criterion (ii) forces 2a > b; combined with a < b gives a < b < 2a, which has unique solution (2,3) at minimal a | Direct algebraic case analysis (no external dependency) | Paper A §16.5 Thm 16.5.1; FTD-0175 [THEOREM]; verified by enumeration in `sym23_uniqueness_proof.py`. The residual Conjecture 16.5.2 (general Sym^a coefficients, not just leading-period ω^a) remains open. |

**Note**: T2 entries are no weaker than the published source. Chudnovsky 1976 is a foundational result of contemporary transcendence theory, consolidated in Waldschmidt's Grundlehren volume. Rubin 1991 is the standard reference for BSD on CM rank-0 curves. Deligne's period conjecture is proved unconditionally in the CM case (Blasius/Anderson/Shimura). "Conditional" here means "depends on this established theorem", not "depends on a conjecture."

---

## TIER 3 — Numerical facts, exhaustive over stated domains (3)

These are **rigorous numerical results** verified across explicitly stated finite domains. They are NOT structural theorems — they say "across the domain checked, X holds," not "X holds in general."

| ID | Statement | Domain | Verification |
|---|---|---|---|
| **OT-3.1** | Phase J partition-function ultralocality at `L = 2` | `L = 2` only (Nyquist-mode degeneracy origin); general L numerically disconfirmed at L ≥ 4 | `proof_phase_j_general_L.py`; honestly retagged 2026-05-02 |
| **OT-3.3** | Polynomial-shape uniqueness: across 2,871,576 polynomials/multipliers in the natural `M_{n,p,m,q}(x) = x² − n G*^p x + m G*^q` family + cubic embeddings + Eisenstein-integer multiplier extension, only the master quadratic dual-matches; **0 dual-matchers in the Eisenstein-integer family** | `n, m ∈ {1,…,64}`, `p, q ∈ {0,…,5}`; rational-coefficient extension; cubic embeddings; Eisenstein/Gaussian-integer multiplier sweeps | `proof_polynomial_look_elsewhere_extended.py` (FTD-0121); pre-reg tag `preregister-polynomial-scan-extended-v1`; ~4×10⁵:1 Bayes weight |
| **OT-3.4** | FTD-0110 cluster coefficient `k = 1/N_base = 1/4` derived from `O_h` representation theory, orbit-equipartition, and timescale separation (mult(A_{1g}) = 4; center is A_{1g}-pure; energy distributes equally across $O_h$-orbits; cluster forms before local $A_{1g}$ fraction decays, locked by nonlinear feedback) | Restored to full [DERIVED] nonlinear pipeline status; physical cluster-mass identification across SM particles remains [STRONGLY MOTIVATED CONJECTURE] | `DERIV_K_FROM_OH_A1G_MULTIPLICITY.md`, `DERIV_FTD0110_NONLINEAR_BRIDGE.md`; C++ dump_a1g_decay and characterization tests |

**Honest caveat (OT-1.9)**: the Γ-product analogue `G*_d` reproduces canonical G* exactly at `d = −4` but at `h ≥ 2` it is a single-number analogue, not the full per-ideal-class Damerell formula. A full Damerell scan at h ≥ 2 has not been run. Reviewer pressure point.

**Honest caveat (OT-3.3)**: the scan is over a *natural* polynomial family. Broader families (rational coefficients, π in coefficients, degree ≥ 3 not factored) would change the count. The interpretation as Bayes evidence depends on the prior choice of family.

---

## TIER 4 — Identification with structural backing but no proof of necessity (1)

This is the framework's softest mathematical spot. The numerical equalities hold; the *structural identification* is conjectured.

| ID | Statement | True at value level | Structural identification (NOT proved) |
|---|---|---|---|
| **OT-4.1** | Coefficient 16 in the master quadratic equals `\|Aut_{Q̄}(E)\|²` for `E: y² = x³ − x` | `\|Aut_{Q̄}(E)\| = \|Z[i]^×\| = 4`; `4² = 16`; the prefactor in the master quadratic is also 16. **TRUE.** | That the master-quadratic prefactor *must* equal `\|Aut(E)\|²` is conjectural. The tower-level identification (FTD-0122 / Paper B Thm 5.1) gives a structural reason for `k = 4`, hence `2^k = 16`, in terms of `\|Z[i]^×\|² = 16`. This is a partial structural unification, not a forcing theorem. |

This is the entry that would draw the most reviewer pressure. The honest framing is: "Two distinct objects both equal 16; the structural reason for this coincidence is conjectured but not proved."

---

## TIER 5 — Strongly motivated conjectures (1)

This is the load-bearing identification between FTD's algebraic structure and the inverse fine-structure constant. It is **honestly tagged as a conjecture**. Substantial structural-uniqueness evidence exists; no derivation chain from FTD axioms exists.

| ID | Conjecture | Empirical match | Structural evidence |
|---|---|---|---|
| **OT-5.1** | `x_+ = 1/α` | 1.26 ppm to CODATA 2022 (`α^{-1} = 137.035999177(21)`) | FTD-0189 adversarial look-elsewhere scan: zero non-G* dual-matchers across 2.65 M degree-2 polynomials over an 18-constant basket FTD did not design, rank 1 by ~130×; OT-1.9 / OT-3.3 (Chowla–Selberg h-scan, d = −4 uniqueness); OT-1.5 (Z[i] structure unifying CM Aut count and tower level k=4). Note: OT-1.9 / OT-3.3 used the pre-v1.4 `(1/α, N_c)` dual-target pair — the polynomial-template-uniqueness facts they establish are unchanged; only the `x_- ↔ N_c` identification is retired. **2026-06-01:** the route-invariant MC-T4.3 boundary (FTD-0242) classifies α as **DYNAMICAL, not structural** — 0/4 FTD-native routes force the `(Tr,Det)=(16G*²,16G*³)` operator assembly; the trace and a clean odd source `G*` are forward-forced but the assembly is not (W-CRIT-2). **No tier change** — remains T5 [STRONGLY MOTIVATED CONJECTURE]; the boundary *sharpens*, does not move, this entry |
| ~~**OT-5.2**~~ | ~~`x_- = N_c = 3`~~ | — | **REMOVED 2026-05-22** per FTD/FQCR Cleanup Taxonomy v1.4 §5 (LEDGER FTD-0014 removed in commit `ca7eb61`); `N_c = 3` is independently sourced — see `DERIV_NC_FROM_TOPOLOGY.md` (four routes) and `DERIV_BCC_MULTIPLICATIVE_STRUCTURE.md` (Moore Layer Theorem) |

**Closed-negative routes** (preserved for provenance, do not attempt):
- R1 transverse stiffness — closed
- R2 source-current normalization — closed
- R3 two-sector response eigenvalue — closed
- R4 projected Dirac matter — closed
- Z-factor reading (FTD-0116) — closed
- RG-running, algebraic combinations, 1/√d, Langevin-equipart, monomial scans — all closed

**Lead-physicist diagnosis**: Phase J ultralocality structurally decouples the algebraic spine from the dynamical EFT — action data does not contain polynomial data. Closure plausibly requires non-action injection mechanism (boundary conditions, observable selection, quantization choice) and may require ontology extension (sixth axiom). MC-T4.3 in `SPEC_OPEN_MATH_BY_SECTOR.md` §10. **Route-invariance update (2026-06-01, FTD-0242, `audits/AUDIT_ALPHA_OPERATOR_FORCING_ROUTE_INVARIANCE.md`):** four independent FTD-native routes (J-twisted ζ-determinant, BCC body-diagonal transfer, lemniscatic-CM arithmetic, variational/valuation/Hodge) were each force-attempted then adversarially refuted; **0/4 forced** the master-quadratic operator assembly. The unforced ingredient is route-invariant (the 2×2 Tr/det independence = W-CRIT-2), so α is **DYNAMICAL, not structural** — the discrete ontology forces the menu (trace + odd source) but not the dish (the assembly W). Stays [FOUNDATIONAL OBSTRUCTION]; the no-go is `[STRONGLY MOTIVATED CONJECTURE]`, not [THEOREM] (the FORCED-escape / RSI Leg 3 remains live).

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

The 7 [THEOREM]-grade entries of the algebraic spine (OT-1.1, OT-1.2, OT-1.3, OT-1.9, OT-2.1, OT-2.3, and OT-2.4) plus the BCC complex-structure theorem (OT-1.5/1.6) and FTD-0110 (OT-3.4) are the bedrock. **Everything else is downstream**.

---

## Pressure points — what a hostile reviewer would push on

1. **OT-4.1 (coefficient 16)**: the only T4 entry. Honest framing: "true coincidence with structural conjecture, not proved necessity." A reviewer can press here, and the framework's response is FTD-0122 (Paper B) — partial structural unification, with explicit no-go for the rest.

2. **OT-1.9 Γ-product caveat**: we tested a natural extension at h ≥ 2, not the full per-ideal-class Damerell formula. The honest fix is to run the proper Damerell scan; effort estimate 2–6 weeks.

3. **OT-2.3 (Q(G*) π-free) and OT-2.2 (transcendence)** depend on Chudnovsky 1976. This is a published, foundational result, but a reviewer could note that algebraic-independence proofs in this domain are not always self-evident. Cite Waldschmidt explicitly.

4. **OT-5.1 / OT-5.2 (the central conjectures)**: a reviewer who insists "you must derive α from axioms or the framework is empty" cannot be answered with current methods. Honest response: the framework's mathematical content is the spine + numerical-uniqueness scans + Z[i] structural unification; the empirical match is conjecture-grade with explicit Bayes weight; a derivation chain is structurally blocked, not merely unsolved. **As of 2026-05-03 evening, the structural-blockage claim is empirically reinforced**: four independent engine tests (FTD-0004 Phase G + FTD-0005 Phase J + FTD-0125 Phase I + FTD-0126 Phase II) return the same answer — α = 1/x_+ does NOT flow into engine matter-sector dynamical observables under any classical-gauge protocol tested. The "structural decoupling" diagnosis is no longer just theoretical. This *strengthens* the framework's external position by making the limit defensible: we now know what doesn't work and why.

5. **Pre-registration discipline**: every numerical scan in T3 is git-tagged BEFORE measurement. This is methodological hygiene the framework practices. A reviewer will check tag dates against measurement dates; they should match.

---

## Update protocol

When a claim's tier changes:

1. Update this document **first** (the bedrock changes).
2. Update [LEDGER.md](LEDGER.md) entry to match.
3. Update [SPEC_OPEN_MATH_BY_SECTOR.md](../../01_reference/SPEC_OPEN_MATH_BY_SECTOR.md) if the claim is on the sector-organised research queue.
4. Update [SPEC_ALGEBRAIC_SPINE.md](../../01_reference/SPEC_ALGEBRAIC_SPINE.md) if the claim is one of the nine numbered spine results (see §0).
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
10. **OT-3.4** FTD-0110 (★★★, O_h rep theory at linear level + Bridge-I global O_h-equivariance + 5% empirical match across 11 amplitudes × 5 SM × 3 L × 2 geom; nonlinear-pipeline closure [OPEN] — see [`AUDIT_FTD0110_2026-05-27_RESOLUTION.md`](../audits/AUDIT_FTD0110_2026-05-27_RESOLUTION.md))

**FQCR additions (2026-05-06; complementary lens, do not change tier 1 above):**

- **OT-1.7** G* via det_ζ quarter-conjugacy bridge (★★★★★, FTD-0141)
- **OT-1.8** G* as finite-N attractor / discharges reframe ε-L obligation (★★★★★, FTD-0142)

Plus 2 numerical-uniqueness facts (OT-3.1/3.3) supporting the central conjectures.

Plus 1 honestly-tagged conjecture (OT-5.1) that the framework explicitly does NOT claim as a theorem.

This is what is publishable. This is what survives review. This is the truth.

---

## Cross-references

- [LEDGER.md](LEDGER.md) — full provenance per claim, including closed-negative results
- [SPEC_OPEN_MATH_BY_SECTOR.md](../../01_reference/SPEC_OPEN_MATH_BY_SECTOR.md) — sector-organised research-questions queue (replaces archived `CHECKLIST_MATH_COMPLETE.md`)
- [SPEC_ALGEBRAIC_SPINE.md](../../01_reference/SPEC_ALGEBRAIC_SPINE.md) — nine numbered results with proofs: seven theorem-grade + two honestly tiered below theorem grade (see §0)
- [SPEC_DOCTRINE_LEDGER.md](../../01_reference/SPEC_DOCTRINE_LEDGER.md) — FTD/FQCR Doctrine Ledger v1.2 (2026-05-08, FTD-0145 SYNTHESIS): single-page status map rolling up T1–T5 tier assignments below alongside LEDGER and CATALOG tags. **Read this when navigating; come back here for atomic tier disputes.**
- [SPEC_DIMENSIONAL_MAP.md](../../01_reference/SPEC_DIMENSIONAL_MAP.md) — dimensionless ↔ dimensional bridge
- [Paper A](../../../../dissemination/papers/PAPER_A_PI_FREE_GENERATOR.tex) — π-free generator (T1.1, 1.2, 1.3, 2.2, 2.3, 3.3 incl Eisenstein null)
- [Paper B](../../../../dissemination/papers/PAPER_B_BCC_COMPLEX_STRUCTURE.tex) — BCC complex structure (T1.5, 1.6, 4.1)
- [Paper C](../../../../dissemination/papers/PAPER_FTD_AS_WILSONIAN_EFT.tex) — Branch-A native EFT (T1.4 anchored, downstream measurements)
