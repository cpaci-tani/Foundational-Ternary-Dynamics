# CHECKLIST_MATH_COMPLETE — Bridge-Complete Math Roadmap

**Created:** 2026-05-01 (post-foundational-audit Phase A)
**Scope:** Bridge-complete — closes the algebraic spine in the strong sense AND the axioms→1/α derivation chain, including items that may have foundational obstructions in the current FTD ontology.
**Authoritative status source:** [`07_assessment/LEDGER.md`](../07_assessment/LEDGER.md) wins on any disagreement.
**Audience:** Project owner + future AI agents resuming the project. Written to be picked up cold.

> **Honest framing.** This is the *full* mathematical-completion roadmap. The framework as of 2026-05-01 has roughly 3 genuinely structural theorems + 2 borrowed classical results + 4 in mixed status (audit 2026-05-01). **Closing every item below is decade-scale work for a small team.** Paper A does *not* require Tier II/III/IV closure — it is publishable from the current "complete" set. This document exists to make the gap legible, not to gate any near-term deliverable.

---

## Tier definitions

| Tier | Scope | What closure unlocks |
|---|---|---|
| **I** | Spine completion — every claim currently tagged [THEOREM] in `SPEC_ALGEBRAIC_SPINE.md` has a full proof from FTD axioms (no L=2-only specializations, no h=1-only scans, no narrative-only THEOREMs) | Honest 9/9 spine; cleaner Paper A; removes "[THEOREM at L=2] + [CONJECTURE for general L]" hedges |
| **II** | Structural-uniqueness completion — Bayesian evidence for x_+ = 1/α and x_- = N_c crosses 10⁶:1 in a substantially broader polynomial/multiplier search space | Converts FTD-0013/0014 from [STRONGLY MOTIVATED CONJECTURE] to "uniqueness-validated identification"; survives reviewer skepticism about lucky polynomial families |
| **III** | Engine ↔ algebra bridge — every empirical engine match (FTD-0110 SM masses, FTD-0107 emergent spectrum, m_e formula structure, BCC/(SC+FCC) reproduction) has a derivation chain | FTD-0110 promotes to [DERIVED]/[THEOREM]; engine-as-instrument program becomes scientifically forceful rather than confirmation-bias-prone |
| **IV** | Axioms→α derivation chain — non-action-level mechanism produces the master-quadratic polynomial structure from the 5 axioms, closing the structural decoupling diagnosis | Converts x_+ = 1/α from [SMC] to [DERIVED]/[THEOREM]. **This is the central FTD claim.** Items here may have foundational obstructions in the current ontology |

---

## Effort taxonomy

| Code | Meaning |
|---|---|
| **D** | 1–5 days session work |
| **W** | 1–3 weeks focused work |
| **M** | 1–3 months focused work |
| **RP** | Research program (3+ months, open timeline) |
| **FO** | Foundational obstruction — closure may require ontology change, not session-tractable |

---

## Tier I — Spine completion

### MC-T1.1 — Phase J ultralocality at general L
- **Closes**: Theorem 7 of `SPEC_ALGEBRAIC_SPINE.md`; FTD-0005.
- **Current status**: [THEOREM at L=2] + [CONJECTURE for general L] (restated 2026-05-01 audit).
- **Exit criterion**: From the 5 FTD axioms, derive that the classical Euclidean action on every finite L³ lattice depends on the state field s only through Σ_i s_i², with permutation invariance at fixed charge count. Proof must be `proof_phase_j_ultralocality_general_L.py` or equivalent and pass at L ∈ {2, 3, 4, 8, 16}.
- **Dependencies**: none beyond axioms.
- **Effort**: **W** (1–2 weeks). Likely route: induction on L using the locality structure of the action; the L=2 explicit calculation should generalize once charge-permutation invariance is shown to be preserved by neighbor extension.
- **Risk**: low. The structural claim is well-motivated; the proof is mostly bookkeeping.
- **Closure path**: write recursive partition-function generating functional; show charge-permutation preserves S_E increment; finite-induction.

### MC-T1.2 — CM uniqueness at class number ≥ 2
- **Closes**: Theorem 3 of spine; FTD-0003.
- **Current status**: numerical scan over 9 class-number-1 discriminants. The audit honestly classified this as `[NUMERICAL FACT, exhaustive over 9-element set]` rather than a structural theorem.
- **Exit criterion**: One of (a) prove that no CM curve with h(K) ≥ 2 produces dual permille match for the master quadratic structure, OR (b) explicitly accept Theorem 3 as `[NUMERICAL FACT, h=1 only]` and update spine + LEDGER + Paper A accordingly.
- **Dependencies**: MC-T2.3 (Chowla–Selberg at h ≥ 2) provides the analytic tools.
- **Effort**: **M** (route a) or **D** (route b).
- **Risk**: medium for (a). Path (b) is acceptance, not derivation; weakens the spine but is honest.
- **Closure path (a)**: use Chowla–Selberg-type identities at h ≥ 2 to compute analogue master-quadratic coefficients; show the dual-permille match either fails universally or has structural reason to be h=1-specific.

### MC-T1.3 — FTD-0112 verification script (Q(G*) π-free)
- **Closes**: Theorem 9 of spine.
- **Current status**: narrative-only proof in `SPEC_ALGEBRAIC_SPINE.md §9`; no programmatic check.
- **Exit criterion**: `scripts/proofs/proof_field_theoretic_qgstar.py` exists, encodes the Chudnovsky 1976 conditional, verifies G* ∈ Q(π, Γ(1/4)) symbolically (sympy), checks G* generates Γ(1/4)-content not π-content via concrete witness construction, exits 0.
- **Dependencies**: none.
- **Effort**: **D** (1–2 days). ~100 LOC.
- **Risk**: very low. Theorem is conditional on a 50-year-old transcendence result; verification is mostly symbolic algebra + a literature note.

### MC-T1.4 — FTD-0044 per-voxel mass gap proof script
- **Closes**: FTD-0044 (the lone surviving theorem from the YM retraction).
- **Current status**: lives only in `docs/papers/archive/retracted_under_reframe/FTD_Yang_Mills_Mass_Gap.tex`. No active proof script; audit flagged this as untested load-bearing.
- **Exit criterion**: `scripts/proofs/proof_per_voxel_mass_gap.py` derives the per-voxel gap from FTD axioms with explicit reframe-compatible (finite-L, undefined-boundary) statement; exits 0 at L ∈ {4, 8, 16, 32}.
- **Dependencies**: none.
- **Effort**: **D-W** (3–10 days depending on whether the YM-paper proof transfers cleanly under the 2026-04-19 reframe).
- **Risk**: medium. The original proof predated the reframe; finite-L restatement may require non-trivial work.

### MC-T1.5 — Resolve A_{1g} dual-4 identification at structural level
- **Closes**: a load-bearing structural claim in FTD-0110 + FTD-0111 link.
- **Current status**: A_{1g} multiplicity = 4 in 27-block (Burnside, [DERIVED]) AND tower-level k = 4 (FTD-0111 [THEOREM]) are both established. The claim that these are the *same* 4 — i.e. that the framework integer N_base = 4 is uniquely determined and identifies these — is structurally suggestive but not proven.
- **Exit criterion**: Either a structural theorem identifying the two `4`s as the same group-theoretic / category-theoretic object, OR explicit acceptance that the agreement is empirical and tagging both as `[STRUCTURALLY MOTIVATED, identification empirical]`.
- **Dependencies**: none.
- **Effort**: **W-M** (1–4 weeks). Likely route: O_h → SU(2) reduction; representation-theory bridge to the (1+i)-tower's harmonic invariant.
- **Risk**: medium-high. May require new mathematics not yet in the FTD literature.

**Tier I total effort:** ~5–10 weeks if all routes succeed; the dominant uncertainty is MC-T1.2 path (a) and MC-T1.5.

---

## Tier II — Structural-uniqueness completion

### MC-T2.1 — Polynomial-scan extension to Bayes ≥ 10⁶
- **Closes**: the structural-uniqueness argument cited in FTD-0121 [SYNTHESIS].
- **Current status**: 147,456 polynomials of form `x² − n·G*^p·x + m·G*^q` with `n, m ∈ [1, 64]`, `p, q ∈ [0, 5]` scanned 2026-05-01 (commit f36b741, hash-locked as `hashlock-polynomial-scan-v1` 2026-05-01 audit pass). Master quadratic uniquely dual-selective. Bayes ~20,000:1 within natural FTD polynomial family.
- **Exit criterion**: extended scan over rational coefficients (n, m ∈ ℚ with bounded denominator), higher polynomial degree (deg ≤ 4), and non-Gaussian-integer multipliers (Eisenstein, quaternion-ring) returns master quadratic uniquely dual-selective AND Bayes ratio crosses 10⁶ versus null. **Pre-registration via `git tag preregister-polynomial-scan-extended-v1` BEFORE the run** — this discipline gap was flagged in the 2026-05-01 audit as the central remaining methodological weakness.
- **Dependencies**: none.
- **Effort**: **W** (1–2 weeks for runner + scan + analysis).
- **Risk**: low computationally; medium epistemically. If extended scan finds *additional* dual-matchers, the structural-uniqueness argument weakens or inverts.
- **Closure path**: write `tools/scan_polynomial_extended.py` runner; pre-register; run on GPU; analyze; commit results to `engine/results/poly_scan_extended_<date>/`.

### MC-T2.2 — Theorem 8 multiplier-level rigidity (full polynomial-coefficient version)
- **Closes**: the (1+i, k=4) selection in FTD-0111.
- **Current status**: 58 (m, k) pairs scanned 2026-05-01 in the natural Gaussian-integer-tower family; (m=2, k=4) rank-1 with 5-orders gap to rank-2. Structurally suggestive but the full multiplier-level scan analogous to the 60k-polynomial scan has not been done.
- **Exit criterion**: scan over multipliers `c = G* · b` with `b` ranging over a substantially broader algebraic class (Gaussian integers, Eisenstein integers, units in real quadratic fields, low-norm imaginary quadratic integers) AND levels k ∈ [3, 12] returns (m=2, k=4) rank-1 with structural justification for the selection, OR identifies an alternative multiplier with comparable rank (which would weaken FTD-0111 selection materially).
- **Dependencies**: none.
- **Effort**: **W** (1–2 weeks).
- **Risk**: low computationally; medium epistemically (same as MC-T2.1).
- **Closure path**: extend `scripts/proofs/proof_tower_multiplier_uniqueness.py` to broader multiplier space; pre-register; run.

### MC-T2.3 — Chowla–Selberg extension to h ≥ 2
- **Closes**: supports MC-T1.2; theory infrastructure for non-h=1 CM.
- **Current status**: classical Chowla–Selberg covers h=1; the analogue for h ≥ 2 (Damerell, etc.) is in the literature but has not been written into the FTD framework.
- **Exit criterion**: theory note `docs/theory/09_mathematical/EXPLR_CHOWLA_SELBERG_HIGHER_H.md` reproduces the relevant identities for h ∈ {2, 3, ...} CM fields, includes the analogue master-quadratic-coefficient construction, and provides the analytic tools MC-T1.2 needs.
- **Dependencies**: none.
- **Effort**: **W-M** (2–6 weeks). Mostly literature synthesis + careful normalization.
- **Risk**: low. Mature classical mathematics.

**Tier II total effort:** ~4–10 weeks.

---

## Tier III — Engine ↔ algebra bridge

### MC-T3.1 — FTD-0110 nonlinear bridge perturbation theorem
- **Closes**: FTD-0110 nonlinear identification; converts cluster↔mass from [SMC] to [DERIVED]/[THEOREM].
- **Current status**: linear `k = 1/N_base = 1/4` [DERIVED] from O_h rep theory (FTD-0110 linear closure 2026-04-28). Nonlinear engine empirically matches across 5 SM particles + 11 amplitudes at ~5%. Mechanisms α (1/√d) and β (Langevin-equipart) [FALSIFIED] 2026-05-01. Mechanism γ (Langevin amplitude-crossover at A* ≈ 13) and finite-amplitude irrep-leakage corrections remain candidates.
- **Exit criterion**: perturbation-theory expansion in genesis density (or amplitude) showing the linear-mode mean is preserved at zeroth order, with explicit logarithmic correction term `≈ −0.030·ln(A/2)` derived from a structural mechanism. Either (a) Mechanism γ confirmed via engine experiments D3a-D3d, OR (b) a fourth mechanism δ identified and proven.
- **Dependencies**: none.
- **Effort**: **W-M** (1–8 weeks). The two failed mechanisms cost ~1 week each; γ + new candidate δ likely costs 2–6 more.
- **Risk**: high. Both natural representation-theoretic frameworks already failed. The nonlinear regime may not have a closed-form perturbative answer in irrep variables.
- **Closure path**: see `EXPLR_FTD_0110_NONLINEAR_BRIDGE_ANALYSIS.md` §8.7. Engine experiments D3a (vary K_GENESIS_KINETIC_DRAIN), D3b (K_EVAP_RATE), D3c (T_L), D3d (L) discriminate γ vs δ. ~2–3 GPU days each.

### MC-T3.2 — m_e exponent n=11 first-principles ladder-position theorem
- **Closes**: FTD-0015 / FTD-0077 m_e exponent [SELECTION] → [DERIVED].
- **Current status**: 3/6 components of `m_e = M_P√(2π)(16/3)α¹¹` are [THEOREM] (√(2π), 16, D=3). Exponent `n=11` from cumulative-sum ladder `{4, 4, 3, 3, 6} → {8, 11, 14, 20}` is physically motivated but not first-principles forced.
- **Exit criterion**: theorem deriving `n=11` from O_h ladder structure + electron's role as smallest-charge lepton, with explicit subgroup-chain reduction. Pole-mass calculation on a non-ultralocal ensemble (per FTD-0075) is the alternative route.
- **Dependencies**: MC-T1.5 (A_{1g} = 4 dual identification) likely supports this; MC-T3.1 success provides framework.
- **Effort**: **M-RP** (1–6 months).
- **Risk**: high. The O_h subgroup-chain approach is the cleanest route; pole-mass route is blocked by FTD-0075 (ultralocal flux propagator).

### MC-T3.3 — Engine (SC+FCC)/2 ↔ BCC Watson agreement bridge theorem
- **Closes**: a structural mystery — the algebraic spine wants BCC (Watson identity W₃ = G*²/(2π) is a BCC integral), the engine runs on (SC+FCC)/2 (BCC-orthogonal per `AUDIT_LINK8_CLOSURE.md`), and yet the engine reproduces spine numerics empirically.
- **Current status**: Watson integrals computed for SC, BCC, FCC, Moore-18 (FTD-0079, 2026-04-24); ratios 0.83–0.95, none matching the conjectured 27/8. No bridge theorem.
- **Exit criterion**: theorem stating that for the FTD-relevant observable class, (SC+FCC)/2 and BCC give numerically equivalent answers up to a structural correction expressible from FTD primitives, OR explicit acceptance that the empirical agreement is not yet understood.
- **Dependencies**: none.
- **Effort**: **W-M** (2–8 weeks).
- **Risk**: medium-high. May require new lattice-Fourier identities not yet in the FTD literature.
- **Closure path**: investigate (SC+FCC)/2 ↔ BCC structural duality via 27-block decomposition; look for a `2² × 1` vs `1 × 8` reorganization that bridges them.

### MC-T3.4 — Bridge Functional arithmetic-mean rule (FTD-0095)
- **Closes**: the open sub-claim that mass = `α · (x_+ + x_-)/2` rather than geometric/harmonic/power-mean alternatives.
- **Current status**: [SELECTION] (2026-04-26). Three closure routes named: (i) variational principle on σ_BCC, (ii) 't Hooft beable equiprobability, (iii) Beilinson regulator slot.
- **Exit criterion**: one of three closure routes succeeds with a derivation that uniquely picks arithmetic mean over alternatives; FTD-0095 promotes from [SELECTION] to [DERIVED].
- **Dependencies**: MC-T3.3 (BCC/(SC+FCC) bridge) likely supports route (i).
- **Effort**: **M** (1–3 months).
- **Risk**: medium. All three routes have technical machinery available but require careful FTD-side adaptation.

### MC-T3.5 — FTD-0110 multi-scale boundary-correction closure
- **Closes**: the multi-scale extension currently empirically verified at 5% but with `[OPEN]` analytical boundary-correction.
- **Current status**: cluster size formula at single 27-block [DERIVED]; multi-scale to clusters spanning many 27-blocks empirically verified across 4 dimensions of variation; analytical closure [OPEN].
- **Exit criterion**: derivation of the boundary-correction term that quantitatively reproduces the cross-block residuals observed at L ∈ {32, 64, 128}; closure of FTD-0110 multi-scale tag.
- **Dependencies**: MC-T3.1 success.
- **Effort**: **W-M** (2–6 weeks after MC-T3.1).
- **Risk**: medium. Likely tractable if MC-T3.1 closes cleanly.

**Tier III total effort:** ~3–9 months.

---

## Tier IV — Axioms→α derivation chain [foundational tier]

> Items here have closure paths that may require ontology change rather than within-framework derivation. The lead-physicist diagnosis (audit 2026-05-01) is that **Phase J ultralocality structurally decouples the algebraic spine from the dynamical EFT** — standard EFT machinery cannot bridge axioms to α because the action data does not contain the polynomial data. Closure requires a non-action-level injection mechanism (boundary conditions, observable selection rules, quantization choice, or an ontology extension that adds polynomial structure as an axiom rather than deriving it). **Whether FTD's current 5-axiom commitment can support this closure is itself an open question.**

### MC-T4.1 — Two-layer ontology axiomatization
- **Closes**: a structural gap in the postulate set. The 5 axioms name only the state field s ∈ {−1, 0, +1}; the flux field J ∈ ℝ³ is added separately. The engine implements two fields; the axiomatization names one.
- **Current status**: `[OPEN]`. Two routes: (a) add J to postulates (raising count from 5 to 6 axioms), (b) derive J from state-only dynamics.
- **Exit criterion**: either (a) explicit 6-axiom system with J as a primitive, all current derivations reverified under the extended axiom set, OR (b) constructive derivation `J = F(s, neighborhood)` from axioms 1–5 with no smuggled assumptions.
- **Dependencies**: none.
- **Effort**: **M** (route a) or **RP** (route b).
- **Risk**: route (a) is bookkeeping; route (b) may not be achievable. The prevailing FTD literature treats J as a derived quantity but the derivation is not fully written down.
- **Foundational note**: this affects all downstream claims. Route (a) is the safe path; the framework's self-description as "5 postulates" becomes "6 postulates," which is honest but requires manuscript propagation.

### MC-T4.2 — Phase-2 EFT non-Gaussian flow at b ≥ 4
- **Closes**: bridge contract Gate 6/7 (the open matching parameters from FTD-0064/0070).
- **Current status**: Gates 1–5 closed [POSITIVE]; Gate 6 (QED-facing Z_Q = e_phys) and Gate 7 (Z_A^QED) remain [OPEN]. Phase-2 b=4, b=8 measurements show Gaussian fixed point holding within 1σ; non-Gaussian mixing matrix uncomputed.
- **Exit criterion**: numerical extraction of the four-coupling tuple (C_L, K_T, Z_j, g_sJ) under mixed dynamics at b ∈ {4, 8, 16}, with explicit non-Gaussian mixing matrix; comparison to QED matching prediction; verdict on whether the engine's running α matches QED's running α at the matched-stencil scale.
- **Dependencies**: none.
- **Effort**: **M-RP** (3+ months — substantial GPU campaign + analysis pipeline).
- **Risk**: high. May find the engine's running matches QED (would be a major positive result) OR diverges (would close another α-derivation route negative).

### MC-T4.3 — Algebraic-spine ↔ dynamical-EFT non-action injection mechanism **[foundational obstruction]**
- **Closes**: the central FTD claim — derives x_+ = 1/α from FTD axioms.
- **Current status**: all natural action-level routes (R1/R2/R3 EFT, Z-factor, RG-running, 1/√d, Langevin-equipart, monomial scans) [CLOSED NEGATIVE]. Lead-physicist diagnosis: structural decoupling via Phase J ultralocality.
- **Exit criterion**: identification of a non-action mechanism that injects polynomial structure into the FTD observable spectrum. Three candidate classes:
  1. **Boundary-condition mechanism**: master quadratic emerges as a constraint on allowed boundary conditions for finite-L lattice rather than as a property of the action.
  2. **Observable-selection mechanism**: the polynomial structure lives in the *measurement* layer (which observables FTD permits) rather than in dynamics.
  3. **Quantization-choice mechanism**: a non-trivial quantization (analogous to Berry-phase / topological-sector selection in standard QFT) injects polynomial structure that classical extremization cannot see.
- **Dependencies**: MC-T4.1 (axiomatization) likely required first; MC-T4.2 (non-Gaussian flow) provides empirical input.
- **Effort**: **FO**. May require ontology extension (sixth axiom about boundary conditions, or seventh axiom about observables, or a quantization postulate not currently in FTD).
- **Risk**: very high. **This is the central foundational question.** Closure may not be achievable without modifying the 5-axiom commitment. If unachievable, FTD-0013 / FTD-0014 stay [STRONGLY MOTIVATED CONJECTURE] permanently and the framework's epistemic ceiling is "structurally-uniqueness-validated empirical match" rather than "first-principles derivation."
- **Honest assessment**: the prior on this closing in any near-term timeframe is low. Paper A is publishable without this; the framework's external defensibility is acceptable without this; but the strongest possible FTD claim ("we derived 1/α") requires this.
- **Engine-side note (post 2026-05-01)**: the engine's Scale 11 ("Reflexivity") UI was deleted (commit `054b530`). It was interpretive pedagogy of the master-quadratic complex-roots case, not a closure attempt for MC-T4.3. The mathematical content (Existence Filter, projection hierarchy, Tomita–Takesaki modular conjugation, Type III₁ → Type I descent) lives entirely in `docs/theory/06_consciousness/*` and `FOUND_PHENOMENAL_NOUMENAL_BRIDGE.md`. If MC-T4.3 closure is ever attempted via engine instrumentation (option 1 boundary-condition mechanism is the most engine-tractable), it would be a fresh module, not a revival of Scale 11.

### MC-T4.4 — General lattice Liénard-Wiechert at accelerating motion
- **Closes**: FTD-0115 [PARTIAL DERIVED] → [DERIVED] for general motion.
- **Current status**: closed-form at uniform velocity [DERIVED]; sinusoidal Larmor case has Bessel-function infinite-series form (FTD-0120 Q5); general accelerating motion only formal Q5★ frequency-domain expression.
- **Exit criterion**: closed-form expression for `A^μ(x, t)` under general piecewise-smooth source trajectories on the lattice, matching continuum Liénard-Wiechert in the L → ∞ limit.
- **Dependencies**: none.
- **Effort**: **W-M** (2–6 weeks).
- **Risk**: medium. Even continuum LW for general motion is not fully closed-form (it's an integral). The lattice case may inherit this limitation; the realistic exit criterion is "closed form for important cases (uniform v, sinusoidal, hyperbolic) plus formal expression for general motion."

### MC-T4.5 — Why-level-k=4 selection from N_base = 4
- **Closes**: the selection of k=4 in the (1+i)-tower from FTD's structural primitives.
- **Current status**: tower-scan rank-1 with 5-orders gap [STRUCTURAL UNIQUENESS DEMONSTRATED]; structural justification for k=4 from N_base = 4 = mult(A_{1g}) is suggested but not proven (see MC-T1.5).
- **Exit criterion**: theorem stating `k_physical = mult(A_{1g}) on 27-block = 4` from a structural identification (representation-theoretic, group-cohomology, or category-theoretic).
- **Dependencies**: MC-T1.5 closure.
- **Effort**: **M** after MC-T1.5.
- **Risk**: medium. Cleanest route is via the O_h → SU(2) subgroup chain.

**Tier IV total effort:** ~6–24 months for items 1, 2, 4, 5; **MC-T4.3 is foundational-obstruction class with no reliable timeline**.

---

## Dependency graph

```
   MC-T1.1 ── (no deps)
   MC-T1.2 ── needs MC-T2.3
   MC-T1.3 ── (no deps)
   MC-T1.4 ── (no deps)
   MC-T1.5 ── (no deps; cross-leverages with MC-T4.5)

   MC-T2.1 ── (no deps)
   MC-T2.2 ── (no deps)
   MC-T2.3 ── (no deps; supports MC-T1.2)

   MC-T3.1 ── (no deps; supports MC-T3.5)
   MC-T3.2 ── needs MC-T1.5; cross-leverages MC-T3.1
   MC-T3.3 ── (no deps; supports MC-T3.4)
   MC-T3.4 ── needs MC-T3.3
   MC-T3.5 ── needs MC-T3.1

   MC-T4.1 ── (no deps; required by MC-T4.3)
   MC-T4.2 ── (no deps; provides input to MC-T4.3)
   MC-T4.3 ── needs MC-T4.1, MC-T4.2 [foundational obstruction]
   MC-T4.4 ── (no deps)
   MC-T4.5 ── needs MC-T1.5
```

**Critical path to bridge-complete:** MC-T4.1 → MC-T4.2 → MC-T4.3, with MC-T4.3 carrying the foundational-obstruction risk that could halt closure indefinitely.

**Critical path to spine-complete:** MC-T1.1 + MC-T1.2 (or its acceptance route) + MC-T1.3 + MC-T1.4 + MC-T1.5 + MC-T2.3, ~3–5 months parallelizable.

**Critical path to Paper A:** **none of the above is required.** Paper A is publishable from the current "complete" set: Theorems 1, 2, 4, 5, 6, 8, 9 + structural-uniqueness scans (with MC-T2.1 as a pre-publication strengthener).

---

## Summary table

| ID | Title | Tier | Effort | Risk | Foundational? |
|---|---|---|---|---|---|
| MC-T1.1 | Phase J ultralocality general L | I | W | low | no |
| MC-T1.2 | CM uniqueness h ≥ 2 | I | M (a) / D (b) | medium | no |
| MC-T1.3 | Q(G*) verification script | I | D | very low | no |
| MC-T1.4 | Per-voxel mass gap proof script | I | D-W | medium | no |
| MC-T1.5 | A_{1g} dual-4 identification | I | W-M | medium-high | partial |
| MC-T2.1 | Polynomial-scan extension to 10⁶ | II | W | low | no |
| MC-T2.2 | Theorem 8 multiplier rigidity full | II | W | low-medium | no |
| MC-T2.3 | Chowla–Selberg h ≥ 2 | II | W-M | low | no |
| MC-T3.1 | FTD-0110 nonlinear perturbation | III | W-M | high | no |
| MC-T3.2 | m_e exponent n=11 derivation | III | M-RP | high | no |
| MC-T3.3 | (SC+FCC)/2 ↔ BCC bridge theorem | III | W-M | medium-high | no |
| MC-T3.4 | Bridge Functional arithmetic-mean | III | M | medium | no |
| MC-T3.5 | FTD-0110 multi-scale boundary | III | W-M | medium | no |
| MC-T4.1 | Two-layer ontology axiomatization | IV | M (a) / RP (b) | medium-high | **yes** |
| MC-T4.2 | Non-Gaussian EFT flow b ≥ 4 | IV | M-RP | high | partial |
| MC-T4.3 | Non-action α-injection mechanism | IV | **FO** | very high | **yes — central** |
| MC-T4.4 | General-motion lattice LW | IV | W-M | medium | no |
| MC-T4.5 | Why-level-k=4 from N_base=4 | IV | M | medium | partial |

**Total tractable effort (Tiers I–III, no MC-T4.3):** ~12–24 months small-team focused.
**Total to bridge-complete (with MC-T4.3 closure):** **unbounded** — depends on ontology questions not currently posed.

---

## Closure verification

This checklist itself is closed when:

1. Every MC-T*.* item has either (a) a [DERIVED]/[THEOREM] entry in `LEDGER.md` with proof_location pointing to a working `scripts/proofs/proof_*.py` script, or (b) an explicit acceptance / acknowledgement in `SPEC_ALGEBRAIC_SPINE.md` documenting that the item is unprovable in the current framework and reframing the affected claim accordingly.
2. `SPEC_ALGEBRAIC_SPINE.md` cross-reference table contains no `[THEOREM at L=2]`-style hedges; every theorem's status is unconditional or explicitly [CONJECTURE for general L].
3. `SPEC_PHYSICS_BRIDGE.md` updates the FTD-0013/FTD-0014 status from [STRONGLY MOTIVATED CONJECTURE] either upward to [DERIVED] (success) or to [PERMANENT SMC due to foundational obstruction at MC-T4.3] (honest non-success).
4. `CHANGELOG_REFRAME.md` records each item's closure session.

A `proof_math_complete_checklist.py` integration script can be added that exits 0 only when every item's `proof_location` resolves and exits 0 itself; this is the operational "math complete" gate for any future "we are done" claim.

---

## Cross-references

- `docs/theory/01_reference/SPEC_ALGEBRAIC_SPINE.md` — the 9-theorem canonical reference
- `docs/theory/01_reference/SPEC_PHYSICS_BRIDGE.md` — FTD-0121 [SYNTHESIS]
- `docs/theory/07_assessment/LEDGER.md` — single source of truth for claim status
- `docs/WHERE_WE_LEFT_OFF.md` — live state + priority queue
- `docs/theory/03_derivations/EXPLR_FTD_0110_NONLINEAR_BRIDGE_ANALYSIS.md` — closure paths for MC-T3.1
- `docs/theory/10_eft_program/THEOREM_A_PHYS_NO_GO.md` — calibration-interface theorem (FTD-0059 / FTD-0096 closure)
- `C:\Users\cpaci\.claude\plans\let-s-audit-the-entire-mellow-sutherland.md` — 2026-05-01 audit deliverable

---

## Honest meta-note

If MC-T4.3 has a foundational obstruction (the lead-physicist diagnosis suggests it does), then **"math-complete" in the bridge sense may not be achievable in the current FTD framework.** The honest version of that outcome is:

> FTD's mathematical content closes at the spine + structural-uniqueness + engine-bridge level (Tiers I–III). The axioms→1/α derivation chain (Tier IV-MC-T4.3) requires either (a) an ontology extension that adds polynomial structure as a non-action axiom, or (b) acceptance that the master-quadratic structure is empirically forced rather than first-principles derived. The framework's external defensibility is fully achievable without (a); the framework's strongest possible self-claim ("derived 1/α") requires (a).

This is not a failure mode; it's a structural outcome of the diagnosis. Paper A reflects this state cleanly. The checklist above is the work that distinguishes "publishable" from "complete in the strong sense."
