# TRACKER · Ontic Truth

> **One document, one source of bedrock truth.** What is *actually* mathematically established in FTD — distilled, ranked, and tied to verification scripts.
>
> **This is the canonical bedrock reference.** The [LEDGER](LEDGER.md) tracks every load-bearing claim with full provenance; this document distills LEDGER entries down to "what survives skeptical mathematical review." If the two disagree, this tracker is correct on tier-assignment; the LEDGER is correct on detailed history.

**Last regenerated:** 2026-06-25 (manual; this is a hand-curated tracker, not auto-generated) — propagated the **FTD-0318 spine-audit demotions** (Phase-J `L ≥ 4` → [OPEN]; the "~4×10⁵ Bayes" retired to [NUMERICAL FACT]; the d=−4 *dual-match privilege* → [NUMERICAL FACT]); repointed the look-elsewhere scan **FTD-0189 → FTD-0319**; added the **FC-W / FTD-0314 / FTD-0315** α-selection pinning to OT-5.1 — **no tier change**. Prior: 2026-06-01 (route-invariance note to OT-5.1, FTD-0242); 2026-05-02 evening.
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

## TIER 1 — Rock-solid theorems (8)

> **Count corrected 2026-08-04: 9 → 8.** OT-1.4 (Phase G geometric Coulomb) is
> **demoted** — see its row. This propagates FTD-0785's demotion of spine
> Theorem 6, which the spine flagged on 2026-08-03 as *"still lists Theorem 6 in
> the upper tier and needs the same correction — flagged, not yet applied, since
> that tracker is outside this document's scope."* Applied here. Since
> `CLAUDE.md` makes **this tracker authoritative on tier assignment**, leaving it
> at Tier 1 meant the authoritative document rated a regression fit as a
> rock-solid theorem.

These are pure algebra. They cannot be wrong without an arithmetic mistake. Each has been verified to machine precision or in exact rational arithmetic.

| ID | Statement | Proof | Verification |
|---|---|---|---|
| **OT-1.1** | Master quadratic `P(x) = x² − 16G*²x + 16G*³` has roots `x_± = 8G*² ± 4G*√(4G*² − G*)` | Quadratic formula on positive discriminant `64G*³(4G* − 1)` since `G* > 1/4`. | Paper A Thm 2.2; `proof_master_verification.py` (54/54 PASS) |
| **OT-1.2** | `G* := Γ(1/4)/Γ(3/4) = Γ(1/4)²/(π√2) = 2ϖ/√π` | Euler reflection `Γ(1/4)·Γ(3/4) = π/sin(π/4) = π√2` | Paper A Thm 2.1; verified at 50-digit precision |
| **OT-1.3** | (1+i)-tower harmonic invariant: `1/y_+ + 1/y_- = 1` for `y := x/G*` at every level `k ≥ 3` of `M_k(x) = x² − 2^k G*^{k-2}x + 2^k G*^{k-1}` | Three-line Vieta: `1/x_+ + 1/x_- = (x_++x_-)/(x_+x_-) = 1/G*`; multiply by G*. | Paper A Thm 3.1(i); `proof_harmonic_invariant_tower.py` (14/14 PASS at 50 digits) |
| **OT-1.4** ⚠ **DEMOTED — NOT TIER 1** | ~~Phase G geometric Coulomb: `α_r(r,L) := 2 r G_L(r)` is zero-free-parameter geometry~~ **The exact-identity claim is withdrawn (FTD-0785, propagated here 2026-08-04).** The spine demoted Theorem 6 to `[NUMERICAL FACT — VALIDATED FIT]`: the evidence is a **regression against simulator output at one finite L**, and **a 0.07% median residual is incompatible with a "zero free parameters" exact identity** — a by-construction identity would agree to machine precision. The row's own cited evidence refutes its own "by construction" justification. FTD-0785's honest split: **(a)** an exact lattice-Green's-function identity with stated hypotheses would be `[THEOREM]` — **not shown**; **(b)** "the engine reproduces the lattice Poisson kernel to 0.07% median residual at L=384" is `[MEASURED]`. **Only (b) is established**, and per `CLAUDE.md` engine measurements do not belong in this tracker at all — (b)'s home is the LEDGER. | ~~Direct: gauss-projection inverts the discrete Laplacian~~ — **a regression fit, not a derivation** | `DERIV_EMERGENT_COULOMB_GEOMETRIC.md`; `AUDIT_ALPHA_EXTRACTION.md`; `scripts/benchmarks/fit_geometric_coulomb.py`. **What survives independently of the fit:** the *deflationary* reading — the projector returns a Green's function, so the engine's Coulomb carries **no fine-structure content** — which is structural and consistent with FTD-0792. That reading is retained; the exact identity is not. |
| **OT-1.5** | BCC complex-structure theorem: 8 BCC corners under 90° rotation form 2 orbits of size 4; `Z[BCC] ⊗ Q = V_triv² ⊕ V_sign² ⊕ V_complex²` with `V_complex` carrying natural `Z[i]`-module structure ≅ `Z[i]²` | Per-orbit Z/4 regular-rep decomposition; standard rep theory | Paper B Thm 3.1; `proof_bcc_complex_structure.py` (5/5 PASS, exact rationals) |
| **OT-1.6** | `Z[i]^× → O_h^ab` no-go: no injective homomorphism exists since Z[i]^× ≅ Z/4 has an order-4 element but O_h^ab ≅ Z/2 × Z/2 (Klein) does not | One-line group-order argument | Paper B Thm 6.1; `DERIV_BCC_COMPLEX_STRUCTURE.md` §3.2 |
| **OT-1.7** | `G* via det_ζ quarter-conjugacy bridge`: $\det_\zeta D_{3/4}/\det_\zeta D_{1/4} = G^*$ where $D_a = \{n + a\}_{n\ge 0}$ are the spectra of operators on $S^1$ with quarter-twisted boundary $\psi(\phi+2\pi) = J\,\psi(\phi)$, $J^2 = -I$. **Arithmetic content**: $4 D_{1/4} = \{n \equiv 1\pmod 4\}$ and $4 D_{3/4} = \{n \equiv 3\pmod 4\}$ are exactly the two non-trivial residue classes mod 4; restricted to primes these are the split and inert prime classes of $\mathbb{Z}[i]$ (Fermat's two-square theorem). $G^*$ is the regularized asymmetry between them. | Lerch's formula: $\det_\zeta\{n+a\}_{n\ge 0} = \sqrt{2\pi}/\Gamma(a)$; $\sqrt{2\pi}$ cancels in the ratio leaving $\Gamma(1/4)/\Gamma(3/4) = G^*$. Equivalently $G^* = \exp[\zeta_H'(0, 1/4) - \zeta_H'(0, 3/4)]$. | FTD-0141 (2026-05-06); `DERIV_GSTAR_QUARTER_CONJUGACY.md` §5; OT-1.2 (algebraic) and FTD-0127 (parity-twist) are two further readings of the same residue-class decomposition; the three identities are unified by $G^* = (\sqrt{2\pi}/\Gamma(3/4))/(\sqrt{2\pi}/\Gamma(1/4)) = \exp[\zeta_H'(0,1/4) - \zeta_H'(0,3/4)] = \Gamma_\zeta(1/2)/\Gamma_{\chi_{-4}}(1/2)$ |
| **OT-1.8** | `G* as finite-N attractor`: $G_N^* := (N+1)^{-1/2}\prod_{n=0}^{N}(n+3/4)/(n+1/4) \to G^*$ at rate $|G_N^* - G^*| = O(1/N^2)$, empirical $C \approx 0.046$ | Stirling expansion of $\Gamma(N+7/4)/\Gamma(N+5/4) \sim (N+1)^{1/2}$ | FTD-0142 (2026-05-06); `DERIV_GSTAR_FINITE_APPROX.md`; verified by `proof_fqcr_convergence.py` (all assertions PASS); discharges `AUDIT_INFINITY_REFRAME.md` ε-L obligation for $G^*$ |
| **OT-1.9** | `CM-curve uniqueness`: $K = \mathbb{Q}(i)$ ($d = 1$, discriminant $d = -4$) is the unique imaginary quadratic field satisfying unit-group and discriminant order coincidence $|\mu_K| = |\text{disc}(K)|$ | Arithmetic evaluation of $|\mu_K|$ and $|\text{disc}(K)|$ across squarefree $d$ | `SPEC_ALGEBRAIC_SPINE.md` §3 (unit-group uniqueness proof); verified to $d \le 200$ |

---

## TIER 2 — Conditional on classical published theorems (7) <!-- header count corrected 2026-07-01, FTD-0348; previously said (3) over a 7-row table -->

These depend on named external results from analytic number theory or transcendence theory. Rigor equals the source; the dependency is explicitly named.

| ID | Statement | Conditional on | Verification |
|---|---|---|---|
| **OT-2.1** | Watson identity: `W₃ = G*²/(2π)`, where `W₃ = (1/π³)∫∫∫_[0,π]³ dk_1 dk_2 dk_3 / (1 − cos k_1 cos k_2 cos k_3)` is the BCC Watson integral | Watson 1939; closed form via Glasser-Zucker 1980 | `DERIV_WATSON_GSTAR_IDENTITY.md`; numerically verified at 100-digit precision in PARI |
| **OT-2.2** | Tower discriminant transcendence: `A_k := 2^{k-2} G*^{k-3} − 1 ∉ Q̄` for all `k ≥ 4`. (Rational at `k = 3`.) | Chudnovsky 1976 — *algebraic independence* of {π, Γ(1/4)} ⇒ G* = Γ(1/4)²/(π√2) transcendental over Q (a nonconstant rational function of algebraically independent transcendentals) ⇒ a non-constant rational-coefficient polynomial in G* takes transcendental values (Waldschmidt §1.4). *(Proof sketch corrected 2026-07-01, FTD-0348 — the prior sketch "Γ(1/4) transcendental ⇒ G* transcendental" was a non-sequitur [a quotient involving π can be algebraic; independence is required], and "Schneider–Chudnovsky 1949" matched no relevant Schneider result. The load-bearing docs — THEOREM_HARMONIC_INVARIANT_TOWER.md, spine §8/§9 — already used the correct route; only this sketch was wrong.)* | Paper A Thm 3.1(iii) |
| **OT-2.3** | `Q(G*)` is π-free in `Q(π, Γ(1/4))`: `Q(G*) ∩ Q(π) = Q` | Chudnovsky 1976 (algebraic independence of `{π, Γ(1/4)}` over Q) | Paper A Thm 4.1; `proof_field_theoretic_qgstar.py` |
| **OT-2.4** | Lemniscatic L-value: `L(E_lemn, 1) = ϖ/4 = πG_G/4 = G*√π/8 ≈ 0.6555143885...`, where E_lemn: y² = x³ − x (Cremona 32.a3). Full BSD formula with c_∞ = 2 real components, c_2 = 2 Tamagawa (Kodaira III), \|E_tors\| = 4, \|Sha\| = 1. | Rubin 1991 (Inventiones 103); full BSD formula for CM rank-0 case via main conjecture for imaginary quadratic fields. (Coates-Wiles 1977 proves only L≠0 ⇒ rank=0 implication, not the precise BSD ratio.) | Paper A §11 Thm Lvalue; Paper E (overview); FTD-0159 [THEOREM]; verified to 27 digits vs LMFDB 32.a3 direct. **Errata note**: earlier session work (pre-2026-05-19) had this as ϖ/2 due to BSD-formula convention-mixing (using Ω_E^+ = 2ϖ AND c_∞ = 2, which double-counts). Caught by ivy-league CM-theorist red team; FTD-0174 / FTD-0159 (revised). |
| **OT-2.5** | χ_{-4} four-level unification: the Kronecker character χ_{-4} on (Z/4Z)^× generates the entire G*/G_G identity algebra through four functorial projections: (L1) lattice \|Z[i]^×\| = 4, (L2) Chowla-Selberg ∏Γ(a/4)^{χ_{-4}(a)} = G*, (L3) Hecke a_p splitting in L(E_lemn,s), (L4) Dirichlet L(χ_{-4},1) = π/4 | Deligne's period conjecture for CM motives (Blasius 1986, Anderson 1986, Shimura 1979); standard CFT for imaginary quadratic fields | Paper A §16 Thm character-unification; FTD-0163 [THEOREM]; consistent with the algebraic-spine structure of OT-1.2 and OT-1.7. The four levels form a motivic-weight tower; consistency across L2-L3 is Deligne's conjecture restricted to the lemniscatic motive. |
| **OT-2.6** | η-tower across the h=1 atlas: for each class-number-one IQ field K with discriminant d_K and unit-group order w_K, \|η(τ_K)\|^{2w_K} = G_K^{w_K} / (2π\|d_K\|)^{w_K/2} where G_K is the Chowla-Selberg constant of K | Chowla-Selberg 1967 at h_K = 1; Selberg-Chowla Gauss-sum evaluation | Paper D Thm eta-tower; FTD covered as part of FTD-0163's extended family; verified at all 9 atlas fields to relative error < 10^-70 via `eta_atlas_verify.py`. Specialises to η(i)^8 = G*^4/(64π²) [Paper A Cor 9.2] and η(ρ)^12 = G_K^6/(216π³) [Paper A §15] |
| **OT-2.7** | Sym²⊕Sym³ exponent constraint set *(corrected 2026-07-01/02, FTD-0351 — the former "(2,3) uniquely minimal-a" uniqueness claim is **RETRACTED**)*: among leading-period polynomials x² − 16G*^a x + 16G*^b (a < b positive integers, prefactor 16), the criteria (roots not *constant* multiples of any single G*^k; positive discriminant) constrain (a,b) to **{a < b < 2a} ∪ {b = 2a+1}** — the scalar-multiple criterion excludes exactly b = 2a, and Δ > 0 excludes exactly b ≥ 2a+2 (ln4/lnG* = 1.2779…; G* < 4 < G*² keeps b = 2a+1 alive). The minimal-a element is **(1,3)**, not (2,3): Δ(1,3) = 64G*²(4−G*) = 583.39… > 0, roots 35.746…/11.592…. *Defect of the old proof (FTD-0348 §3.1): the Case-A (2a>b) vs Case-C (2a<b) split was notationally vacuous — the roots are identically 8G*^a ± 4√(4G*^{2a}−G*^b) in every case, so the criterion could not pass one side and fail the other.* The (2,3) pair-selection is now **[SELECTION]**: a = 2 is forced by matching the independently proven Watson trace 16G*² = 32π·W₃ (OT-2.1; FTD-0002/0006), and b = a+1 by the Det = Tr·G* Vieta ansatz — which is exactly the [UNDERDETERMINED] W-CRIT-2 assembly (FTD-0235); conditional selection, not a derivation | Chudnovsky 1976 (G* transcendental — treated as an indeterminate in the constant-multiple criterion); elementary algebra + the numeric bound G* < 4 < G*² otherwise | Paper A §16.5 Thm (corrected statement + dated correction Remark + [SELECTION] Remark); FTD-0175 demoted under FTD-0351 to [THEOREM: constraint set] + [SELECTION: (2,3) choice]; `sym23_uniqueness_proof.py` repaired to honest enumeration (now reports the (1,3) survivor and the b = 2a+1 branch). The residual Conjecture 16.5.2 (general Sym^a coefficients, restated as an admissible-set conjecture) remains open. |

**Note**: T2 entries are no weaker than the published source. Chudnovsky 1976 is a foundational result of contemporary transcendence theory, consolidated in Waldschmidt's Grundlehren volume. Rubin 1991 is the standard reference for BSD on CM rank-0 curves. Deligne's period conjecture is proved unconditionally in the CM case (Blasius/Anderson/Shimura). "Conditional" here means "depends on this established theorem", not "depends on a conjecture."

---

## TIER 3 — Numerical facts, exhaustive over stated domains (3)

These are **rigorous numerical results** verified across explicitly stated finite domains. They are NOT structural theorems — they say "across the domain checked, X holds," not "X holds in general."

| ID | Statement | Domain | Verification |
|---|---|---|---|
| **OT-3.1** | Phase J partition-function ultralocality | `L = 2` [THEOREM] (Nyquist-mode degeneracy origin); `L = 3` numerically ultralocal (spread 8.9e-16) [NUMERICAL EVIDENCE]; **`L ≥ 4` [OPEN]** (Gauss-zero-mode masked) — the prior "disconfirmed for general L" was itself an overclaim, corrected 2026-06-24 spine audit | `proof_phase_j_general_L.py`; retagged 2026-05-02, corrected 2026-06-24 **(Reconciled 2026-07-02, FTD-0350/FTD-0360: the L≥4 ambiguity is CLOSED — [THEOREM at all L ≥ 2, matched-stencil, Gauss-realizable space]; this row's older wording predates the retag — spine §7 is the statement of record.)** |
| **OT-3.3** | Polynomial-shape uniqueness: across 2,871,576 polynomials/multipliers in the natural `M_{n,p,m,q}(x) = x² − n G*^p x + m G*^q` family + cubic embeddings + Eisenstein-integer multiplier extension, only the master quadratic dual-matches among **quadratics**; 0 in the Eisenstein family. **⚠ COUNT CORRECTED 2026-08-04 (FTD-0802): the cubic extension contributes 4 further dual-matchers, so the space holds 5 distinct matchers, not 1. The '0 genuinely-new cubic' figure was a hardcoded literal, never a computed test, and is false.** | `n, m ∈ {1,…,64}`, `p, q ∈ {0,…,5}`; rational-coefficient extension; cubic embeddings; Eisenstein/Gaussian-integer multiplier sweeps | `proof_polynomial_look_elsewhere_extended.py` (scan ids disambiguated 2026-07-01, FTD-0348); pre-reg tag `preregister-polynomial-scan-extended-v1 [the 2,871,576-polynomial natural-family/cubic/Eisenstein EXTENDED scan, `proof_polynomial_look_elsewhere_extended.py` — NOT FTD-0319, whose row-of-record is the separate ~2.65M adversarial 18-constant-basket scan, tag `preregister-adversarial-look-elsewhere-v1`, run 2026-05-21; this row previously conflated the two under one id]`. **[SELECTION]** — *retagged from [NUMERICAL FACT] 2026-08-04 by FTD-0802 under the locked PREREG_OT33_BASERATE_v1 Outcome B; withdrawn as support for OT-5.1* — measured result is 4 non-master dual-matchers / 2,871,576 polynomials (was reported as 0); the historical "~4×10⁵:1 Bayes weight" is **retracted** (NOT runner-computed — the runner yields only a ~19× scan-size factor — and tolerance-conditioned; corrected 2026-06-24 spine audit) |
| **OT-3.4** | FTD-0110 cluster coefficient `k = 1/N_base = 1/4` derived from `O_h` representation theory, orbit-equipartition, and timescale separation (mult(A_{1g}) = 4; center is A_{1g}-pure; energy distributes equally across $O_h$-orbits; cluster forms before local $A_{1g}$ fraction decays, locked by nonlinear feedback) | Restored to full [DERIVED] nonlinear pipeline status; physical cluster-mass identification across SM particles remains [STRONGLY MOTIVATED CONJECTURE] | `DERIV_K_FROM_OH_A1G_MULTIPLICITY.md`, `DERIV_FTD0110_NONLINEAR_BRIDGE.md`; C++ dump_a1g_decay and characterization tests |

**Honest caveat (OT-1.9)**: the Γ-product analogue `G*_d` reproduces canonical G* exactly at `d = −4` but at `h ≥ 2` it is a single-number analogue, not the full per-ideal-class Damerell formula. ~~A full Damerell scan at h ≥ 2 has not been run. Reviewer pressure point.~~ **RUN 2026-08-04 (FTD-0321) — and the privilege did not survive it.** The full per-ideal-class scan (periods from η at each class's CM point; Chowla–Selberg used as an independent gate, passing at h ≥ 2 to 8.1e-50) confirms `d = −4` as the unique dual-matcher across all 270 fundamental h ≥ 2 fields with |d| ≤ 907 (2,558 ideal classes) — and then **fails at |d| = 7,895**, with **1,271 fundamental dual-matchers across 696 distinct discriminants** by |d| ≤ 500,000 (61.6M reduced forms). Worse, the criterion has no discriminating power at scale: over |d| ≤ 200,000 a *random* target near `G*` is matched by **~204 ideal classes with P(≥1) = 1.0000**, because `G*² = 8π|η|⁴Im τ` peaks at `Im τ = 3/π`, just above the reduced-form floor, and canonical `G*` sits essentially at that maximum — the densest region available. **The `d = −4` dual-match privilege is a range artifact.** Note the scope: OT-1.9's Tier-1 content is the *arithmetic* coincidence `|μ_K| = |disc(K)|`, which is untouched; what falls is the dual-match reading (already `[NUMERICAL FACT]` per FTD-0318, already criterion-fragile per FTD-0124). See `ANALYSIS_DAMERELL_IDEAL_CLASS_SCAN_v1.md`. **Scan-domain restatement (PERMANENT, 2026-07-01, FTD-0355 — closes FTD-0348 math flag F3):** the FTD-0123 scan domain is all 43 fields of h ≤ 3 (complete: 9 + 18 + 16, largest \|d\| = 163/427/907) plus the 20 smallest h = 4 discriminants (\|d\| ≤ 312) — 63 total, h = 4 deliberately truncated (20 of the 54 known; the 23 with 312 < \|d\| ≤ 907 not scanned). The historical domain phrase "63 fundamental discriminants, h ∈ {1..4} with \|d\| ≤ 907" was wrong — that set has 86 elements. Counts independently recomputed at finalization (reduced-form enumeration to \|d\| ≤ 2000); frozen `PREREG_DAMERELL_SCAN_v1.md` untouched. **2026-06-24 spine audit:** the arithmetic `|μ_K| = |disc(K)|` uniqueness at d=−4 stays [THEOREM] (T1, above); but the *physics dual-match privilege* of d=−4 (that the master quadratic dual-matches there specifically) is only a [NUMERICAL FACT] — criterion-dependent, and flips under a rational-multiplier criterion ((d=−3, q=3) lands at +0.9077 ppm, tighter than canonical).

**Honest caveat (OT-3.3)**: the scan is over a *natural* polynomial family. Broader families (rational coefficients, π in coefficients, degree ≥ 3 not factored) would change the count. The interpretation as Bayes evidence depends on the prior choice of family.

> 🛑 **RESOLVED 2026-08-04 (FTD-0802) — the control below was run, and OT-3.3
> did not survive it. OT-3.3 is retagged `[NUMERICAL FACT]` → `[SELECTION]` and
> **withdrawn as support for OT-5.1**, exactly as FTD-0319 was.** The
> pre-registration (`PREREG_OT33_BASERATE_v1.md`, outcomes locked with both
> runner SHA-256s before execution) returned **Outcome B**: `N_null = 0.0014`
> dual-matchers under displaced targets, `P(>=1) = 0.0009`, against a
> pre-blessed B threshold of `< 0.1`. Finding no other matcher is precisely what
> chance predicts, so the zero count discriminates nothing. The `x_-` leg
> eliminates **zero** candidates at the registered gate (16 pass `x_+`, 16 pass
> both), so "dual-match" and "match" are one predicate here — and `x_- ↔ N_c` is
> retired (FTD-0014). The `x_+` leg alone matches a random target near 137 about
> **one time in three**. **Additionally, the count itself was wrong:** the
> "0 genuinely-new cubic dual-matchers" line was a hardcoded literal, never a
> computed test, and it is false — the corrected runner reports **5 distinct
> dual-matchers, 4 beyond the master quadratic**. Consequence: after FTD-0791
> removed the FTD-0319 leg, **`x_+ = 1/α` retains no numerical-uniqueness
> support at all** and rests on OT-1.9 and OT-1.5, both structural. The 1.26 ppm
> arithmetic agreement is untouched; OT-5.1 stays T5 `[SMC]`.
>
> <details><summary>Superseded 2026-08-03 flag (FTD-0791), kept for provenance</summary>
>
> ⚠ **Base-rate flag added 2026-08-03 (FTD-0791) — OT-3.3 has NOT been refuted, and must not be cited as if unaffected.** FTD-0791 audited the *sibling* FTD-0319 scan (the ~2.65M adversarial 18-constant-basket run, tag `preregister-adversarial-look-elsewhere-v1`) and found that its "unique dual-matcher" result is exactly what its own null predicts — 1.42–1.67 matchers expected, 1 found. **OT-3.3 is a different runner** (`proof_polynomial_look_elsewhere_extended.py`, the 2,871,576-polynomial extended natural-family/cubic/Eisenstein scan, tag `preregister-polynomial-scan-extended-v1`) and **the equivalent base-rate control has never been run against it.** Until it is, OT-3.3's zero-dual-matcher count is of unknown significance: a null expectation near zero and a null expectation near one look identical in the raw count. Running that control is `[OPEN]` and is the direct analogue of `verify_look_elsewhere_baserate.py`.
>
> </details>
>
> *(That control was run on 2026-08-04 — see the FTD-0802 banner above. This
> `[OPEN]` is now closed.)*

---

## TIER 4 — Identification with structural backing but no proof of necessity (1)

This is the framework's softest mathematical spot. The numerical equalities hold; the *structural identification* is conjectured.

| ID | Statement | True at value level | Structural identification (NOT proved) |
|---|---|---|---|
| **OT-4.1** | Coefficient 16 in the master quadratic equals `\|Aut_{Q̄}(E)\|²` for `E: y² = x³ − x` | `\|Aut_{Q̄}(E)\| = \|Z[i]^×\| = 4`; `4² = 16`; the prefactor in the master quadratic is also 16. **TRUE.** | That the master-quadratic prefactor *must* equal `\|Aut(E)\|²` is **[SELECTION — declared, no longer awaiting proof]** (PERMANENT classification, 2026-07-01, FTD-0355; previously carried as "conjectural"/promotion-pending). Closure evidence for declaring rather than awaiting: Paper A's N1–N3 negative tests (`PAPER_GSTAR_INTRODUCTION.tex` Remark `rem:three-negatives`; class polynomial, η-quotient PSLQ, Hecke eigenvalue — no CM-internal arrow to the polynomial *form*; bounded-scope per Paper A's own `rem:n1-n3-scope`, hence [SELECTION — declared] and not [CLOSED NEGATIVE]), plus the FTD-0355 finalization search finding no corpus forcing result (`DERIV_DUAL_DERIVATION_OF_16.md`'s Honesty Note itself disclaims forcing of the power). The tower-level identification (FTD-0122 / Paper B Thm 5.1) gives a structural reason for `k = 4`, hence `2^k = 16`, in terms of `\|Z[i]^×\|² = 16`. This is a partial structural unification, not a forcing theorem. A future forcing proof would be a new row, not a promotion of this one. Tier unchanged (T4). |

This is the entry that would draw the most reviewer pressure. The honest framing is: "Two distinct objects both equal 16; the structural reason for this coincidence is conjectured but not proved."

---

## TIER 5 — Strongly motivated conjectures (1)

This is the load-bearing identification between FTD's algebraic structure and the inverse fine-structure constant. It is **honestly tagged as a conjecture**. Substantial structural-uniqueness evidence exists; no derivation chain from FTD axioms exists.

| ID | Conjecture | Empirical match | Structural evidence |
|---|---|---|---|
| **OT-5.1** | `x_+ = 1/α` | 1.26 ppm to CODATA 2022 (`α^{-1} = 137.035999177(21)`) | ⚠ **THE PRIMARY EVIDENCE LINE IS WITHDRAWN (FTD-0791, 2026-08-03).** This row rested on the FTD-0319 adversarial look-elsewhere scan; that scan was audited refute-by-default, replicated to the digit, and **sits at the chance base rate** — the null expects **1.42–1.67** matchers at the registered `2e-6` gate and it found **1** (79% Monte Carlo match rate), and at that gate the second leg eliminates nothing (1 → 1). Both tolerances were set by the observed deviation; the preregistered outcome had probability 0.9942 under its own null. **FTD-0319 retagged `[MEASURED]` → `[SELECTION]`; the scan-rigid count is ZERO.** Separately (FTD-0792) the engine never ran on this root. *(Formerly read: "zero non-G* dual-matchers across 2.65 M degree-2 polynomials over an 18-constant basket FTD did not design, rank 1 by ~130× (note: the "~4×10⁵ Bayes" headline is retracted to [NUMERICAL FACT] per OT-3.3)".)* ⚠ **AND THE SECOND EVIDENCE LINE IS NOW WITHDRAWN TOO (FTD-0802, 2026-08-04):** OT-3.3 failed its own pre-registered base-rate control (`N_null = 0.0014`, Outcome B) and its published count was additionally found wrong (4 non-master dual-matchers, not 0). OT-3.3 is retagged `[SELECTION]` and no longer supports this row. **`x_+ = 1/α` therefore retains NO numerical-uniqueness support of any kind.** Remaining support is structural only: OT-1.9 and OT-1.5 (Z[i] structure unifying CM Aut count and tower level k=4). ⚠ **AND THE OT-1.9 LEG AS CITED HERE IS NOW GONE TOO (FTD-0321, 2026-08-04).** This sentence cited OT-1.9 in its *Chowla–Selberg h-scan / d = −4 uniqueness* reading. The full per-ideal-class Damerell scan has now been run and that reading **fails**: `d = −4` is the unique dual-matcher across all 270 fundamental h ≥ 2 fields with |d| ≤ 907, and then breaks at |d| = 7,895, with 1,271 fundamental dual-matchers across 696 discriminants by |d| ≤ 500,000. Worse, the criterion has no discriminating power at scale (a random target is matched by ~204 ideal classes, `P(≥1) = 1.0000`) — and FTD-0803 shows why: `G*` is the value of an SL₂(ℤ)-invariant at the order-2 elliptic fixed point `τ = i`, hence a **forced critical value**, so its window is structurally crowded. **What survives of OT-1.9 is only its Tier-1 arithmetic — the coincidence `|μ_K| = |disc(K)|`, which carries no dual-match content whatsoever.** So OT-5.1's support is now OT-1.5 plus an arithmetic coincidence about unit groups. Note: OT-1.9 / OT-3.3 used the pre-v1.4 `(1/α, N_c)` dual-target pair — the polynomial-template-uniqueness facts they establish are unchanged; only the `x_-  N_c` identification is retired. **2026-06-01:** the route-invariant MC-T4.3 boundary (FTD-0242) classifies α as **DYNAMICAL, not structural** — 0/4 FTD-native routes force the `(Tr,Det)=(16G*²,16G*³)` operator assembly; the trace and a clean odd source `G*` are forward-forced but the assembly is not (W-CRIT-2). **No tier change** — remains T5 [STRONGLY MOTIVATED CONJECTURE]; the boundary *sharpens*, does not move, this entry. **2026-06-24:** FTD-0314 (carrier-narrowing theorem) extends the no-go to all native finite symmetries — the surd `√(G*(4G*−1))` is transcendental over ℚ, so no native carrier realizes it; FTD-0315 adopts the external binding law as **FC-W** (the constitution's FC-4), under which `x₊=1/α` is `[CONDITIONAL THEOREM given W]`, still `[SMC]` unconditionally (no tier change) |
| ~~**OT-5.2**~~ | ~~`x_- = N_c = 3`~~ | — | **REMOVED 2026-05-22** per FTD/FQCR Cleanup Taxonomy v1.4 §5 (LEDGER FTD-0014 removed in commit `ca7eb61`); `N_c = 3` is independently sourced — see `DERIV_NC_FROM_TOPOLOGY.md` (four routes) and `DERIV_BCC_MULTIPLICATIVE_STRUCTURE.md` (Moore Layer Theorem) |

**Closed-negative routes** (preserved for provenance, do not attempt):
- R1 transverse stiffness — closed
- R2 source-current normalization — closed
- R3 two-sector response eigenvalue — closed
- R4 projected Dirac matter — closed
- Z-factor reading (FTD-0116) — closed
- RG-running, algebraic combinations, 1/√d, Langevin-equipart, monomial scans — all closed

**Lead-physicist diagnosis, scope-corrected by FTD-0412**: Phase J ultralocality proves decoupling only for its stated action/readout class; it does not prove that every non-action mechanism is impossible. MC-T4.3 in `SPEC_OPEN_MATH_BY_SECTOR.md` §10 is therefore an **unfinished mechanism search with scoped no-go packages**, not a foundational obstruction. **Route-invariance update (2026-06-01, FTD-0242, `audits/AUDIT_ALPHA_OPERATOR_FORCING_ROUTE_INVARIANCE.md`):** four FTD-native routes (J-twisted ζ-determinant, BCC body-diagonal transfer, lemniscatic-CM arithmetic, variational/valuation/Hodge) were each force-attempted and none forced the master-quadratic operator assembly. The unforced ingredient is route-invariant within those routes (the 2×2 Tr/det independence = W-CRIT-2). FTD-0244 upgrades the axiomatized operator-calculus exclusion to theorem-negative, but neither result quantifies over all possible observables, boundary data, enlarged local algebras, or dynamical mechanisms. The physical identification remains [STRONGLY MOTIVATED CONJECTURE]; the broader search remains [OPEN].

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

The **6** [THEOREM]-grade entries of the algebraic spine (OT-1.1, OT-1.2, OT-1.3, OT-1.9, OT-2.1, and OT-2.3 — list corrected 2026-07-01, FTD-0348: OT-2.4, a §10.Y subsidiary L-value rather than one of the nine numbered results, had been listed in place of OT-1.4; **OT-1.4 then removed 2026-08-04, see below**) plus the BCC complex-structure theorem (OT-1.5/1.6) and FTD-0110 (OT-3.4) are the bedrock. **Everything else is downstream**.

<!-- [CORRECTION 2026-08-04] This sentence read "The 7 ... (OT-1.1, OT-1.2,
OT-1.3, OT-1.4, ...)" — still listing OT-1.4 as theorem-grade bedrock hours
after the same-day edit demoted it out of Tier 1 in three other places in this
file (the Tier-1 header count 9→8, the OT-1.4 row itself, and the
"ten that matter" list). Since CLAUDE.md makes THIS tracker authoritative on
tier assignment, the authoritative document was rating a regression fit as
bedrock in one place of four. Count corrected 7→6 and OT-1.4 struck. -->


---

## Pressure points — what a hostile reviewer would push on

1. **OT-4.1 (coefficient 16)**: the only T4 entry. Honest framing: "true coincidence with structural conjecture, not proved necessity." A reviewer can press here, and the framework's response is FTD-0122 (Paper B) — partial structural unification, with explicit no-go for the rest.

2. **OT-1.9 Γ-product caveat**: we tested a natural extension at h ≥ 2, not the full per-ideal-class Damerell formula. The honest fix is to run the proper Damerell scan; effort estimate 2–6 weeks.

3. **OT-2.3 (Q(G*) π-free) and OT-2.2 (transcendence)** depend on Chudnovsky 1976. This is a published, foundational result, but a reviewer could note that algebraic-independence proofs in this domain are not always self-evident. Cite Waldschmidt explicitly.

4. **OT-5.1 (the central conjecture)**: a reviewer who insists "you must derive α from axioms or the framework is empty" cannot be answered with current methods. The current evidence is the spine, the numerical-uniqueness scan, and Z[i] structural unification; the empirical match remains conjecture-grade. Three channel-limited engine diagnostics remain valid (FTD-0004 Phase G, FTD-0005 Phase J, FTD-0125 Phase I). FTD-0412 retracts FTD-0126's matter-sector result because it evolved spatial `D_W` as the real-time Hamiltonian and initialized a Dirac-basis spinor while declaring the chiral basis. Those three negatives and the FTD-0244 operator-calculus theorem exclude their stated classes; they do not establish universal structural blockage. MC-T4.3 is an unfinished search outside those classes.

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
4. ~~**OT-1.4** Phase G geometric Coulomb (★★★★★)~~ — **DEMOTED 2026-08-04**, no longer Tier 1 (FTD-0785: regression fit, not a proof)
5. **OT-1.5** BCC complex-structure theorem (★★★★★)
6. **OT-1.6** Z[i]^× → O_h^ab no-go (★★★★★)
7. **OT-2.1** Watson identity (★★★★, Watson 1939 / Glasser–Zucker 1980)
8. **OT-2.2** Tower discriminant transcendence (★★★★, Chudnovsky 1976 algebraic independence — quick-reference label reconciled 2026-07-01, FTD-0351, to the corrected OT-2.2 row)
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
- [SPEC_DIMENSIONAL_MAP.md](../../01_reference/SPEC_DIMENSIONAL_MAP.md) — dimensionless  dimensional bridge
- [Paper A](../../../../dissemination/papers/PAPER_A_PI_FREE_GENERATOR.tex) — π-free generator (T1.1, 1.2, 1.3, 2.2, 2.3, 3.3 incl Eisenstein null)
- [Paper B](../../../../dissemination/papers/PAPER_B_BCC_COMPLEX_STRUCTURE.tex) — BCC complex structure (T1.5, 1.6, 4.1)
- [Paper C](../../../../dissemination/papers/PAPER_FTD_AS_WILSONIAN_EFT.tex) — Branch-A native EFT (T1.4 anchored, downstream measurements)
