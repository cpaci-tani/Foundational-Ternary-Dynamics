# ANALYSIS — Vertex Program v1 Results: DK Evolution + Noise-Controlled Bivector Closure

**Tag:** M1 = **[CLOSED NEGATIVE]** for FTD-0089's literal (unit-coefficient) discrete DK equation at the tested protocol ([MEASURED]: every grade better described by the KG comparator than by DK — a relative discriminator; absolute KG residuals 0.39–0.76, so "the composites are KG fields" is NOT claimed); M2 = **[CLOSED NEGATIVE]** for noise-recoverable su(2) closure under the FTD-0088-prescribed controls
**Post-run adversarial review (2026-07-10):** physics-redteam AND math-redteam passes applied — instrument-scope caveats §1.3a/§1.3b, the §1.3c non-Diracness proof (the FTD-0089 §A1.3 system is provably not the DK operator), protocol quantifiers corrected ("tested", not "accessible"), M2 baseline-drift and closes-pattern corrections §2.2, low-A power caveat §2.3. **The math review's prescribed decisive follow-up was then executed same-day as M1 v1.1** (§1.5: true δ = d\* operator, free speed, per-grade weighting — pre-registered, lock `07a03489`/`280e5d86`): verdict **DK-STATIC-ONLY again, with the fitted operator speed a\* ≈ 0** — the strongest form of the negative. Both M1 loopholes closed; the verdicts stand at measured scope.
**Ledger rows:** FTD-0379 (M1), FTD-0380 (M2)
**Pre-registration:** [`PREREG_VERTEX_DK_CLOSURE_v1.md`](../../10_eft_program/preregistrations/PREREG_VERTEX_DK_CLOSURE_v1.md), lock commit `b46fdfe0`, tag `preregister-vertex-dk-closure-v1` — expectations, criteria, and priors committed before either measurement ran
**Runners (SHA256 locked in the prereg):** `engine/tests/test_dk_evolution.cpp`, `engine/tests/test_bivector_closure_v2.cpp`
**Raw output:** `engine/results/vertex_dk_closure_2026-07-10/{m1_dk_evolution,m2_bivector_closure_v2}.log` (local, gitignored per convention)
**Companions:** [`DERIV_DIRAC_KAHLER_IDENTIFICATION.md`](DERIV_DIRAC_KAHLER_IDENTIFICATION.md) (FTD-0089 — M1 answers its §A1.5), [`DERIV_BIVECTOR_ALGEBRA_PROGRAM_F.md`](DERIV_BIVECTOR_ALGEBRA_PROGRAM_F.md) (FTD-0086/0087/0088 — M2 answers its §3.5.3), [`DERIV_MODE_ERASURE_AND_SPIN_ALGEBRA.md`](../number_theory/DERIV_MODE_ERASURE_AND_SPIN_ALGEBRA.md) (FTD-0073 — untouched)

---

## 0 · Executive statement

The two decisive, previously-scoped-but-never-executed measurements of the vertex program both returned clean negatives on their pre-registered criteria:

- **M1 — DK-STATIC-ONLY.** The engine's evolution does **not** satisfy FTD-0089's literal discrete Dirac–Kähler equation on the local grade fields at the tested protocol. The FTD-0089 A1 identification is **kinematic only** at that scope: the 4-grade response has the *component structure* of a DK field, but the grades do not evolve by the unit-coefficient DK operator as written. Every grade is *better described* by the second-order Klein–Gordon comparator (discriminator KG-FORM on 4/4 grades in both configurations; absolute KG residuals 0.39–0.76, so the composites are not claimed to *be* KG fields — see §1.3).
- **M2 — CLOSURE-ROBUST-FAIL.** The FTD-0087 iterated-commutator closure failure does **not** recover under any pre-registered noise control (larger L, lower A, time-averaged readouts): 0/18 cells close 3/3, no monotone improvement in either control direction (A-monotone 2/6 columns, L-monotone 0/6; threshold was 4/6). FTD-0088's "4-injection dynamical noise" reinterpretation is **REFUTED** under its own prescribed controls — the non-closure is robust to them (strongest in the L-direction and time-averaging; the low-A direction was partially power-limited, §2.3).

**Combined consequence for the vertex program:** the Branch-A hypothesis — that fermionic (Clifford/Dirac) structure emerges *dynamically* from the substrate — is closed negative at every protocol **tested** (this campaign plus the FTD-0061/0071–0075/0126 family). The non-site-local construction space is *not* exhausted: FTD-0073's "fermion emergence requires non-site-local structure" clause is itself [CONJECTURE], and accessible-but-unrun variants exist (velocity-fitted/per-grade-normalized M1 v2, §1.3; a prerequisite-valid full-toggle configuration, §1.4; higher L). At the current state of evidence, fermion content is **Branch-B**: an imported matter sector with an imposed coupling, constrained (not generated) by the substrate's genuine kinematic structure. What survives as native content: the Cl(3,0) kinematic grade skeleton (FTD-0088, 12/12 — untouched, subject to the §1.4 audit), the leading-order matching-bivector signature (FTD-0086 — untouched), and the mode-erasure theorem (FTD-0073 — untouched by construction).

Both negatives are *productive*: they were the two named hypotheses on which "native fermions" most concretely rested, and they were closed by the experiments their own source documents prescribed (with the §1.4 caveat that the refuted noise hypothesis was itself partly premised on phases that never acted).

## 1 · M1 — Dirac–Kähler evolution (FTD-0379)

### 1.1 What ran

The FTD-0089 §A1.5 verification path, formalized: local grade fields S(x) = |J|², Vᵢ(x) = Jᵢ, Pᵢⱼ(x) (plaquette bilinear), T(x) = JₓJᵧJ_z on the full periodic 8³ lattice; the 8-component discrete DK system (d−δ)Φ = mΦ with ε-corrected signs (prereg §2.2); midpoint time discretization; single m fitted jointly by least squares; per-grade KG comparator ∂ₜ²φ = c²Δφ − μ²φ. Protocol: 2-injection (all three off-diagonal WH pairs), 30 recorded ticks, fit window t ∈ [4, 28), 8 seeds, CONFIG-N (the FTD-0088 full non-local toggle set) primary + CONFIG-M (wave+gauss) control. Harness gates (signed adjointness, nilpotency) all passed at 0 relative error before dynamics ran.

### 1.2 Results

Pooled over pairs and seeds (CONFIG-N):

| Grade | ρ_DK(m*) | ρ_DK(m=0) | ρ_KG | Form verdict |
|---|---|---|---|---|
| 0 (scalar S) | 1.0015 | 1.0128 | 0.7611 | KG-FORM |
| 1 (vector V) | 8.6107 | 8.6076 | 0.3895 | KG-FORM |
| 2 (bivector P) | 1.4118 | 1.4151 | 0.6710 | KG-FORM |
| 3 (pseudoscalar T) | 2.3757 | 2.3744 | 0.5271 | KG-FORM |

Joint ρ_all = 1.7134. Verdict per the locked criteria: **DK-STATIC-ONLY** (all ρₖ ≥ 0.50). CONFIG-M control: ρ_all = 1.7235, same form verdicts; the pre-registered sanity anchor held exactly as expected — grade 1 in CONFIG-M is the raw wave field and fits KG at **ρ_KG = 0.0880**, an order of magnitude tighter than anything DK achieves anywhere.

### 1.3 Reading the numbers honestly

- ρ ≈ 1 means the DK spatial operator explains essentially none of the time evolution (residual as large as the signal).
- **§1.3a — instrument-scope caveat (velocity normalization; post-run redteam finding).** The harness fits the literal FTD-0089 system with a *hard-coded unit coefficient* on (d−δ), while the KG comparator gets a freely fitted c² per grade — an asymmetry. A hypothetical genuine first-order mode at the lattice's native speed, ∂ₜΦ = v(d−δ)Φ with v = 1/√3, would register ρ ≈ (1−v)/v ≈ 0.73 under this instrument and be classified DK-STATIC-ONLY. **The [CLOSED NEGATIVE] is therefore sound against FTD-0089 §A1.3's equation as literally written (which is what §A1.5 prescribed testing), not against every velocity-normalized DK variant.** A v2 with a fitted velocity coefficient is the accessible follow-up.
- **§1.3b — instrument-scope caveat (amplitude degree; post-run redteam finding).** The grade fields carry different J-degrees (V ~ A; S, P ~ A²; T ~ A³), so the literal *linear* DK system relating them is amplitude-inconsistent a priori: each component equation carries an uncompensated O(A) imbalance, and at A = 10 the predicted eq1 imbalance ~A matches the measured ρ₁ = 8.6. Two consequences, pulling in opposite directions: (i) the degree obstruction is itself an *a-priori structural argument* that the literal DK reading could never have held at generic amplitude — reinforcing kinematic-only; (ii) the specific value ρ₁ = 8.6 is bookkeeping-confounded, so "the first-order grade coupling actively mismatches" is **not** established as a dynamical fact over and above the scaling obstruction. An A-scan (and/or per-grade normalization) was accessible and not run in M1; it belongs in the same v2 as §1.3a.
- The fitted m* is strikingly consistent across injection pairs (+0.2066/+0.2191/+0.2076, spread 5.9%) — but at ρ ≥ 1 this consistency carries **no evidential weight**: the fit minimizes over a parameter that improves nothing (ρ(m*) ≈ ρ(0)), and per §1.3b the joint m is over-constrained by incompatible A-scalings across grades. Recording this explicitly to pre-empt a future numerology temptation: **do not cite m* ≈ 0.21 as a discovered mass.**
- The KG-side victory is the structurally expected outcome and was pre-registered (priors DK-STATIC-ONLY 45%) — but note its honest ceiling: KG residuals 0.39–0.76 mean the composites are *better described by* KG than DK, not that they are KG fields. Indeed for J solving a wave equation, S = |J|² provably fails KG (products of solutions of a linear equation are not solutions); the genuinely-KG benchmark is the sanity anchor's ρ_KG = 0.088 on the raw wave field.
- Scope: the negative is protocol-scoped (L=8, A=10 only, this injection family, these toggle sets, unit-coefficient harness). But it answers the question FTD-0089 §A1.5 actually posed, at the protocol that document specified, in the configuration in which the static skeleton was measured (FTD-0088's).

### 1.3c Deeper defect found by the adversarial math review — the §A1.3 system is not the DK operator

Independent re-derivation established that the sign alternation in the adjoint identities (§1.3's conventions, inherited faithfully from FTD-0089 §A1.3) is not a bookkeeping quirk but a **proof of non-Diracness**: the Becher–Joos/Rabin DK operator requires δ = d\* uniformly, making D = d−δ skew-adjoint with D² = −Δ_Hodge; the §A1.3 convention instead yields a mixed symmetric/skew block structure whose square is not ±Δ (its m = 0 grade-0 flow is elliptic/growing, not hyperbolic). So M1 v1, executing §A1.5 literally, tested a **variant** system. Bounding from the printed sums: grades 1 and 3 are convention-invariant (v1's ρ = 8.61, 2.38 stand), grade 0 provably fails at least as hard under correction (ρ′ ≥ 0.987), grade 2 not computable from v1's output. The review also noted v1's negative is independently rescued by the gross **non-uniformity** of the per-grade residuals (a single-speed DK predicts uniform ρ; measured 1.00/8.61/1.41/2.38, and grade 1's implied speed contradicts its own KG c² by ~13×) — an argument not pre-registered, hence the v1.1 below.

### 1.5 M1 v1.1 — corrected-operator, free-scale re-test (run same day; PREREG_VERTEX_DK_CLOSURE_v1_1.md, lock `07a03489` + gate-sign amendment `280e5d86`, pre-measurement)

v1.1 rebuilt the harness with the **true DK operator** (uniform δ = d\*; harness gates now assert and pass exactly: uniform adjointness, skew-adjointness ⟨DΦ,Ψ⟩ = −⟨Φ,DΨ⟩, and D² = +lap = −Δ_Hodge) and a **freely fitted scalar speed** a alongside m, with per-grade weighting neutralizing the §1.3b amplitude-degree dominance. Same protocol, seeds, pairs, configs, window as v1. Result (CONFIG-N pooled):

| Quantity | Value |
|---|---|
| Fitted operator scale a\* per pair | −0.00158 / −0.00125 / +0.00106 — **consistent with zero** (sign-indefinite; spread 204%) |
| Per-grade ρ_DK at (a\*, m\*) | 0.9929 / 1.0020 / 0.9960 / 0.9997 |
| ρ_all | 0.9933 |
| KG comparator | wins 4/4 grades (0.38–0.77); CONFIG-M grade-1 anchor holds (ρ_KG = 0.0880) |
| Verdict (locked v1.1 taxonomy, prior STATIC-ONLY 65%) | **DK-STATIC-ONLY** |

The free-scale fit is the sharpest formulation of the negative: **allowed to choose any speed for the genuine DK operator, the least-squares fit chooses a ≈ 0** — the DK spatial coupling contributes nothing to explaining the grade evolution, at any velocity normalization. This certifies grade 2's band (0.996 ≥ 0.50), closes the §1.3a/§1.3c instrument loopholes, and upgrades the FTD-0379 scope from "the §A1.3 literal variant at unit scale" to **the true DK operator at fitted scale, at the tested protocol**. The remaining scope limits are the honest ones: L=8, A=10, this injection family, these toggle sets.

### 1.4 Disclosed anomaly — predecessor toggle-set validity

Running CONFIG-N (copied verbatim from `test_clifford_multigrade.cpp`) surfaced live `TermToggles` validation warnings: `weak_transmutation requires dual_substrate`, `exchange_force requires poisson_coulomb`, `triad_binding requires dual_substrate`. Validation is warning-only (non-strict), so runs proceed, but three of the nominal "full non-local" phases likely acted degenerately or inertly — **in FTD-0088's original runs exactly as in M1's** (same code path). This does not affect the M1 comparison (identical configuration on both sides) but it means the Program-F family's "full non-local dynamics" label may overstate the effective dynamics. A separate audit task has been spawned; conclusions of FTD-0085–0089 should be re-read against the *effective* toggle set once that audit lands. FTD-0087's set (with `dual_substrate`, without `exchange_force`/`strong_force`) produced no warnings — M2's execution is unaffected.

Two second-order consequences (post-run redteam): (i) FTD-0088's noise-mechanism statement named "forces, triad, exchange" as the per-tick mixing — physics that partly never acted — so M2 refuted a hypothesis that was itself partly premised on inert phases; refuting a confused hypothesis is still closure, but the point belongs on the record. (ii) A *prerequisite-valid* full-non-local configuration (e.g. adding `dual_substrate` and `poisson_coulomb` so the flagged phases actually act) is an accessible, untested cell for both M1 and M2 — one of the named reasons the campaign's quantifier is "protocols tested," not "protocols accessible."

## 2 · M2 — Noise-controlled bivector closure (FTD-0380)

### 2.1 What ran

Exact replication of FTD-0087 Part D (8 signed 4-injection sequences per cyclic triple, prefactor 1/4, block-toroidal 2³ plaquette readout, the F″ toggle set), swept over L ∈ {8, 16, 32} × A ∈ {1, 3, 10} × {instant, time-averaged} readouts, 16 seeds (the 8 original + 8 new). Criteria (locked): a triple closes iff |mean| > 2·max-off AND > 2·sem; a cell closes iff 3/3 triples; trend = monotone concentration-ratio improvement in ≥ 2/3 triples in ≥ 4/6 columns.

### 2.2 Results

- **Baseline replication:** (L=8, A=10, instant, original 8 seeds) → 0/3 closure, expected-plaquette signs all negative (the Cl(3,0) structure-constant sign), off-axis mass dominant — the **qualitative closure outcome replicates FTD-0087** (which is all the locked gate demanded). **Correction (math redteam): the magnitudes did NOT replicate** — expected-plaquette means −8.3/−4.2/−3.7 vs FTD-0087's documented −12.4/−45.8/−5.0, off-axis ≲ 11 vs 40–76; the nonlinear A=10 response has drifted materially since FTD-0087 (the 2026-06-17 remediation changed the golden hash intentionally). Honest scope: **M2 refutes the noise hypothesis on today's engine**; the qualitative structure (no closure, negative ε signs, off-axis dominance) carries over, so the refutation plausibly transfers to the FTD-0087-era engine, but that is inference, not measurement.
- **Cells closing 3/3: 0 of 18.**
- **Isolated triple closes: 3 of 54** triple-cells ([B_xy,B_yz] at L=8/A=1/time-avg r=2.94; [B_xy,B_yz] at L=16/A=1/time-avg r=2.14; [B_yz,B_xz] at L=16/A=10/time-avg r=2.19) — a ~5.6% rate against the ~6% two-sided rate the 2σ-style criteria admit under a null. **Correction (math redteam):** the closes DO share features — all three are time-averaged, two share the triple [B_xy,B_yz], two sit at the maximally noise-controlled corner (A=1, time-avg) at adjacent L — the one suggestive pattern in the data, in FTD-0088's predicted direction. It stays sub-significant (P ≈ 0.11–0.25 for such coincidences under a null, before the seed-sharing dependence that inflates them), **and both A=1 closes have the wrong sign** (+0.154, +0.136 vs the negative Cl(3,0) structure-constant sign every genuine signal in the family carries, including the v2 baseline) — strengthening the noise reading. Reported in full rather than summarized away.
- **Monotonicity: A-monotone 2/6 columns, L-monotone 0/6** (threshold 4/6) — no criterion-level trend (2/6 is above the ~0.44-column null expectation, P ≈ 0.07, but below the locked bar). At low A the commutator signal shrinks *into* the noise floor rather than cleaning up (A=1 signals 0.0023–0.154 with comparable sem); at larger L nothing improves.
- **Power caveat on the A=1 third of the grid (math redteam):** if the A=10 signal (~4) were genuine closure scaling as A² (the prereg's own stated scaling), the A=1 prediction ~0.04 sits below the measured sems (0.03–0.06) — those six cells could hardly have passed even if closure were real, so the informative denominator is ~12 cells, not 18. The verdict survives on the A=3 cells, where an A²-scaled signal ~0.36 would have been detectable at ~6σ and 0.02–0.19 was measured — genuine evidence against.

Verdict per the locked criteria: **CLOSURE-ROBUST-FAIL** (forced by the printed numbers; independently recomputed by the adversarial review).

### 2.3 What this means

FTD-0088 §3.4.2's reinterpretation — "the F″ closure failure is 4-injection dynamical noise, not an algebraic defect" — is **refuted by its own prescribed experiment**: the non-closure survives every control that hypothesis predicted would cure it. **Power caveat (post-run redteam):** in the low-A direction the commutator signal shrinks into the seed-noise floor (signals 0.03–0.15 with comparable sem at 16 seeds), so the concentration statistic r = signal/max(off, sem) loses discriminating power exactly where the noise hypothesis predicted recovery; sem ∝ 1/√N was reducible at accessible cost. The robustness conclusion therefore rests most strongly on the L-direction (0/6 monotone) and the time-averaged readouts, where power was adequate. The honest standing description of the plaquette bivector sector is FTD-0087's original one, strengthened: a genuine, robust *leading-order matching signature* ([Êᵢ, Êⱼ] → Pᵢⱼ, untouched) sitting on an algebra that **does not close** into su(2) under composition at any protocol tested. FTD-0087 Path 2 ("accept approximate closure, quantify the deviation") is what remains of the native-bivector branch; Path 3 (Branch-B selection constrained by the matching signature) is the current accounting.

## 3 · Tag movements (exactly these, nothing else)

| Claim | Before | After |
|---|---|---|
| Dynamical DK evolution at the FTD-0088 protocol (FTD-0379) — v1: FTD-0089's literal system at unit scale; **v1.1: the true DK operator (δ = d\*) at freely fitted scale, per-grade weighted** | untested ([OPEN] implicit in FTD-0089 §A1.5) | **[CLOSED NEGATIVE]** — DK-STATIC-ONLY in both instruments; v1.1's fitted operator speed a\* ≈ 0 (§1.5) closes the velocity and operator-convention loopholes. Remaining scope: L=8, A=10, this injection family, these toggle sets |
| Grade-composite evolution form (FTD-0379) | untested | **[MEASURED]** — better described by KG than by DK (discriminator KG-FORM 4/4 grades, both configs; absolute KG residuals 0.39–0.76 — not "the composites are KG fields") |
| FTD-0089 A1 DK identification | [STRUCTURAL IDENTIFICATION] | unchanged, now explicitly **kinematic-only at the tested scope** (literal dynamical clause closed) |
| FTD-0088 noise reinterpretation of the F″ closure failure (FTD-0380) | leading reading | **[CLOSED NEGATIVE / REFUTED under its own prescribed controls]** — CLOSURE-ROBUST-FAIL (§2.3 low-A power caveat) |
| su(2) closure of plaquette bivectors | [NOT VERIFIED], noise-hypothesis pending | **[CLOSED NEGATIVE at protocols tested]** — non-closure robust to the prescribed controls |
| FTD-0086 matching-bivector signature | [MEASURED], robust | unchanged (M2's baseline re-confirms the signature's sign structure) |
| FTD-0073 mode-erasure theorem | [THEOREM] | unchanged (both probes non-site-local by construction; note its "fermions require non-site-local structure" clause is [CONJECTURE] — the non-site-local space is not theorem-covered) |
| Branch-A native fermion derivation | "plausible at leading order, contingent on noise control" (FTD-0088) | **closed negative at protocols tested; Branch-B is the current accounting** (accessible-but-unrun variants named in §0/§1.3/§1.4) |
| x₊ = 1/α (FTD-0013), FC-W, MC-T4.3, D=3, all spine tags | — | **untouched** (ramification grade 0 throughout; no α/δ content anywhere in this campaign) |

## 4 · Consequence for the vertex program

The program's charter ([`SCOPE_VERTEX_PROGRAM.md`](../../10_eft_program/scopes_and_specs/SCOPE_VERTEX_PROGRAM.md)) now proceeds on the derive-given-imposed path: matter sector imported (Wilson–Dirac per `SPEC_WILSON_DIRAC_FTD.md`), vertex coupling imposed as a declared calibration (the composition of import-ledger lines IMP-E1 and IMP-E3 — no new priced import for the *value*; the gauge-connection identification A_μ = 𝒫_T J_μ [SELECTION] is separately flagged as an unpriced selected type, charter §2), with the substrate's native contribution being *constraints*: any Branch-B selection must reproduce the kinematic 4-grade skeleton (FTD-0088, subject to the §1.4 effective-toggle audit), the matching-bivector signature (FTD-0086), and must not claim dynamical DK or closed native su(2) at the tested scopes (this document).

## 5 · Reproduction

```
git checkout preregister-vertex-dk-closure-v1
cmake --build engine/build --config Release --target test_dk_evolution test_bivector_closure_v2 --parallel 32
engine/build/Release/test_dk_evolution.exe
engine/build/Release/test_bivector_closure_v2.exe
```

Platform note: runs executed on the Windows-native CUDA backend (`[RenderBridge] GPU backend active`), acceptable here per project convention (small lattices, structural measurements, not a wall-clock-sensitive sweep); bit-identical reproduction is not guaranteed across platforms, statistical equivalence of the summary statistics is.
