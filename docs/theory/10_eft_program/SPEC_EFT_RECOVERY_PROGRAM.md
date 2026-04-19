# SPEC · EFT Recovery Program

**Tag:** [REFERENCE]
**Version:** 1.0 (Pre-registration)
**Date:** 2026-04-19
**Status:** Phase 0 — foundation documents being written; no experiments started

> **Pre-registration.** This document is committed to the repository *before any of the measurements described in it are run.* The expectations, pass/fail criteria, and canonical reference regime are all specified up-front. No result downstream of this spec is permitted to retrofit its expected value after the fact. If a measurement produces a surprising number, the spec is *not* edited to match — the measurement is reported honestly and the theory must explain the surprise.

---

## 0 · Why This Program Exists

The FTD engine reproduces phenomenological fragments of the Standard Model (Coulomb 1/r², hydrogen 1/n², QCD-like confinement, time dilation) and derives one genuine coupling relation (α = G_C²). It does not yet constitute a Wilsonian EFT. The five pillars of an EFT — Ward identities, Lorentz covariance, RG flow, operator expansion, matching to continuum — have not been measured on the lattice.

This program closes that gap. Success means: FTD can stand as a Wilsonian effective field theory whose β-function, operator content, Ward-identity closure, and continuum limit are *measured* quantities with error bars, publishable as a standalone paper and testable against — and distinguishable from — the Standard Model.

The program has six phases (Phase 0–5) over ~13 weeks. Phase 0 pre-registers the expectations. Phases 1–4 run the experiments. Phase 5 writes the manuscript.

Full plan: `C:\Users\cpaci\.claude\plans\vivid-marinating-pudding.md` (local). Program catalog of current parametric insertions: `../07_assessment/CATALOG_PARAMETRIC_INSERTIONS.md`.

---

## 1 · Epistemic Discipline

This program adheres to three rules that override any implementation convenience:

1. **No numerical fishing.** Every measurement has a pre-registered expectation committed to this spec *before the measurement runs*. Agreement below the pre-registered threshold is a confirmation; agreement above is a null result or a discovery. Under no circumstances is the expectation adjusted after seeing the measurement.
2. **No relabeled insertions.** If the measured β(g) matches QED's one-loop form to 10%, that is *evidence* for RG-flow emergence — but the continuum β-function is not thereby "derived by FTD." It is *consistent with* FTD's continuum-limit theorem (`DERIV_CONTINUUM_LIMIT_QED_EQUIVALENCE.md`) conditional on C1–C5. Phrasing stays precise.
3. **Discrete, named, testable.** Every new engine module is ≤ 500 LOC with one clear responsibility. Every measurement has a CTest that is [PASS] or [FAIL] against the pre-registered expectation. No silent successes, no opinion-based verdicts.

---

## 2 · The Five-Pillar EFT Checklist

An effective field theory in the Wilsonian sense requires:

| Pillar | FTD-side question | Phase | Deliverable |
|---|---|---|---|
| **β-function** | Does the lattice coupling flow under coarse-graining, and does β(g) match any known continuum theory? | 2 | Measured β(g) curve with 3-scale blocking, error bars |
| **Ward identities** | Is ⟨∂_μ J^μ⟩ = ⟨ρ⟩ satisfied for composite operators, not just Gauss's law by construction? | 1 | 12 Ward-identity CTest entries at permille |
| **Lorentz covariance** | After rescaling by c = 1/√3, do temporal and spatial correlators collapse? At what scale? | 1 | Anisotropy exponent p > 0 with error |
| **Operator expansion** | Do lattice operators classify as relevant / marginal / irrelevant under blocking? | 3 | 12 operators × 3 blocking stages, Wilson coefficients measured |
| **Continuum matching** | Do α, σ, m_p/m_e converge to a finite limit as L → ∞? | 4 | a → 0 extrapolation + residual error bar |

All five must produce honest answers — including null answers — for the program to succeed. A null answer on any pillar is a finding, not a failure.

---

## 3 · Canonical Reference Regime

**Every EFT measurement in this program uses the following configuration unless it is explicitly a sweep parameter.**

```yaml
lattice_size:       L = 64           # voxels per side; total 64^3 = 262144 sites
boundary:           periodic         # every sampler uses lattice.h::wrap
coupling:           G_C from engine/include/ftd/constants.h::ALPHA_EFT
                                     # G_C = √ALPHA ≈ 0.08542454...
reference_scenario: flux-pulse       # already in engine/src/scenarios/flux.cpp
measurement_tick:   t = 2000         # steady-state; past transient (~500 ticks)
rng_seed:           42               # fixed for reproducibility
dt:                 default from engine/src/render_bridge.cpp ctor
toggles_off:        [genesis, larmor_radiation, selective_damping]
                                     # off to isolate EFT physics from phenomenology
toggles_on:         [wave_propagation, coupling, gauss_projection, forces]
                                     # minimal dynamics for EFT measurements
```

**Rationale for each choice:**
- **L = 64** — large enough that lattice-artifact scales (a ~ 1) are separated from IR by 6 e-foldings; small enough that three blocking stages (64 → 32 → 16) fit a single run.
- **flux-pulse scenario** — isolated Gaussian flux pulse with no seeded charges; tests free-field and small-perturbation dynamics. No manifestation events (state = 0 everywhere initially), so Ward-identity measurements are not polluted by scenario-injected charges.
- **t = 2000** — benchmarked by `benchmark_emergent_alpha.cpp` as past-transient; E2 two-charge potential is stationary there.
- **Fixed seed** — reproducibility. Three additional seeds (43, 44, 45) used for error-bar estimation only.
- **Toggles** — genesis/larmor/selective-damping are phenomenological extensions that alter RG flow; they are off to keep the measured β reflecting the minimal lattice theory. Future work can reintroduce them and test *their* impact separately.

**Sweep regimes.** When a phase varies a parameter, it varies *only that parameter* away from the canonical regime:
- Phase 1A (anisotropy): L ∈ {32, 48, 64, 96}
- Phase 2C (β-function): L ∈ {16, 32, 64} via blocking; three coupling strengths G_C ∈ {0.06, 0.0854, 0.12}
- Phase 4C (continuum limit): L ∈ {32, 48, 64, 96, 128}

---

## 4 · Pre-Registered Expectations — Phase 1 (Symmetry Recovery)

### 4.1 Rotational anisotropy (§1A of the plan)

**Measurement.** Compute `spatial_flux_correlation` separately along the three axis-direction classes `(1,0,0)`, `(1,1,0)`, `(1,1,1)` (currently the library averages over axes; extend it). Fit each C(r) = A·exp(−r/ξ) + B·r^(−η). Report anisotropy coefficient:

δ(r) = (ξ_face − ξ_diag) / ξ̄

**Pre-registered expectations:**

| Observable | Expected value | Tolerance | Rationale |
|---|---|---|---|
| δ(r = 4) | ~0.05–0.15 (anisotropic at short range) | — | Cubic lattice breaks O(3) at a-scale |
| δ(r = L/4 = 16) | < 0.02 | ±0.01 | IR regime should be approximately isotropic |
| Scaling exponent p in δ(r) ∝ (a/r)^p | p > 0 | p > 0.5 strongly preferred | O(a²/r²) leading correction from discretization |

**[PASS]** if δ(L/4) < 0.02 AND p > 0 measured with ≥ 2σ confidence over four lattice sizes.
**[FAIL]** if δ(L/4) > 0.05 at L = 96 (indicates that rotational invariance does not emerge at lattice scales the program can reach).

### 4.2 Lorentz recovery (§1B)

**Measurement.** Temporal correlator C_t(τ) = ⟨J(τ, 0) · J(0, 0)⟩ and spatial correlator C_s(r) = ⟨J(0, r) · J(0, 0)⟩. Rescale τ → c·τ with c = 1/√3 and compare curves.

**Pre-registered expectations:**

| Observable | Expected value | Tolerance |
|---|---|---|
| Residual |C_t(cτ) − C_s(τ)| / C_s(τ) for τ > 4a | < 0.01 (1%) | ±0.5% |
| Residual decays as (a/r)^q with q ≥ 2 | q ≥ 2 | measured exponent |
| Dispersion ω²(k) matches 4c²·sin²(k/2) for k < π/4 | Yes | ±2% |

**[PASS]** if residual < 1% for 4a ≤ r ≤ L/4 and dispersion match < 2%.
**[FAIL]** if residual > 5% anywhere in the fit window.

### 4.3 Ward identities (§1C)

**Measurement.** For four configurations (vacuum, static charge pair at separation r = 8, oscillating dipole period T = 16, plane-wave EM field k = 2π/L): measure ⟨∂_μ J^μ(x)⟩ − ⟨ρ(x)⟩ per voxel. Histogram the residual.

**Pre-registered expectations:**

| Identity | Test | Expected | Tolerance |
|---|---|---|---|
| Gauss ∇·J = ρ (basic) | All 4 configs | ≤ 10⁻⁸ (machine precision, because `gauss_projection` enforces it) | 10⁻¹⁰–10⁻⁷ |
| Current conservation ∂_t ρ + ∇·J = 0 | Oscillating dipole, plane wave | ≤ 10⁻⁶ | Residual from Euler integrator |
| Composite 2-point ⟨∂_μ J^μ · J^ν⟩ = ⟨ρ · J^ν⟩ | Static charge pair | < 10⁻³ | First non-trivial test of Ward for composite |
| Vertex Ward Γ_μ(p,p) = ∂Σ/∂p^μ | — | [OPEN] — needs fermion infrastructure | See §10 |

**[PASS]** if first three identities are satisfied at their tolerances.
**[FAIL]** if Gauss > 10⁻⁶ (implies solver failure) or composite Ward > 10⁻² (implies gauge invariance violated at operator level).
**[OPEN]** for vertex Ward — documented as a Phase-4+ extension; not a blocker.

---

## 5 · Pre-Registered Expectations — Phase 2 (RG & β-Function)

### 5.1 Charge conservation under blocking (§2A validation)

Before any β measurement, blocking must itself be physically consistent.

**Measurement.** Initialize a single +1 voxel at position (32, 32, 32). Block once (L' = 32). Total charge must remain 1 voxel.

**Pre-registered expectation:** Total state sum pre-block = total state sum post-block.

**[FAIL]** if this is violated — blocking scheme is wrong and must be replaced before β measurement runs.

### 5.2 Measured α_eff at three scales

**Measurement.** Fit two-charge potential V(r) = −α_eff / r + C in window r ∈ [4, L/4] at L = 64, L = 32, L = 16 (via successive blocking).

**Pre-registered expectations:**

| Scale | Expected α_eff | Tolerance |
|---|---|---|
| L = 64 (fine) | 1/137.036 ± (1/137)·0.15 | ±15% (lattice artifacts) |
| L = 32 (one block) | Slightly different from fine scale; direction of shift determines β sign | — |
| L = 16 (two blocks) | Further shifted | — |

**The measurement-of-interest is not the values themselves but the trend.** α_eff increasing or decreasing monotonically under blocking = meaningful RG flow. α_eff jittering randomly = lattice artifact.

### 5.3 β(g) extraction (§2C)

**Measurement.** β(g) = [g(scale/2) − g(scale)] / ln 2 computed from α_eff at the three scales above. Three seeds × three initial couplings × three measurement windows = 27 β-function samples.

**Pre-registered expectations:**

| Hypothesis | Prediction | Verdict if confirmed |
|---|---|---|
| QED one-loop form | β_QED(g) = g³ / (12π²) | Strong evidence FTD recovers QED RG flow |
| QCD one-loop form | β_QCD(g) = −(b₀/2π)·g³ with b₀ > 0 | Strong evidence FTD has confining flow |
| No RG flow (β ≈ 0) | Coupling invariant under blocking | Trivial UV fixed point; unexpected |
| Non-standard form | β(g) with coefficient differing from QED and QCD by > 30% | **New FTD prediction** — a Phase-5 manuscript result |

**[PASS]** if β(g) is measurable (error bars don't straddle zero) and fits *any* of the four categories above with statistical confidence.
**[FAIL]** if measurements are so noisy that β error bars straddle zero over the entire g range tested.

### 5.4 Scaling dimensions (§2D)

**Measurement.** Fit C(r) ∝ r^(−2Δ) on unblocked and blocked lattice.

**Pre-registered expectation:** Δ measured at L = 64 and L = 32 agree to within 5% → fixed-point signature.

---

## 6 · Pre-Registered Expectations — Phase 3 (Operator Expansion)

### 6.1 Operator classification by blocking flow

For each of 12 dimension-6 operators enumerated in `SPEC_OPERATOR_BASIS.md`, measure Wilson coefficient c(scale) at 3 blocking stages.

**Pre-registered expectations:**

| Operator (continuum naive dim) | Expected flow | Classification |
|---|---|---|
| J² (dim 4) | Grows slightly under blocking | Marginal / marginally relevant |
| (∇·J)² (dim 6) | Decays under blocking | Irrelevant |
| J·(∇×J) (dim 5) | Decays or CP-violating; expected zero in parity-symmetric vacuum | Irrelevant / forbidden |
| F_μν F^μν lattice version (dim 4) | Grows (this is the gauge-kinetic term) | Marginal |
| (J²)² (dim 8) | Strongly decays | Highly irrelevant |
| …8 more operators | — | See `SPEC_OPERATOR_BASIS.md` |

**[PASS]** if at least 10 of 12 operators match their expected classification with confidence ≥ 2σ.
**[FAIL]** if majority (> 6 of 12) contradict expectation.

### 6.2 Anomalous dimensions

**Pre-registered expectation:** γ_J² (anomalous dimension of J²) ∈ [−0.1, +0.3] (consistent with QED one-loop expectations).

---

## 7 · Pre-Registered Expectations — Phase 4 (Dynamical SM Emergence)

### 7.1 EWSB cold start (§4A) — *This is the riskiest test in the program*

**Measurement.** Cold-start scenario `sm_ewsb.cpp` with uniform zero state, bare SU(2)×U(1) flux pattern, no Higgs seed. Run 20 000 ticks. Measure ⟨|J|⟩(t) and the spectrum of the 2-point correlator (mass gap extraction).

**Pre-registered expectations — two branches:**

**Branch A: Dynamical EWSB succeeds.**
- ⟨|J|⟩ develops non-zero value in a finite-time window
- Mass gap in correlator ≈ M_W within ±30%
- W/Z mass ratio M_W/M_Z ≈ cos θ_W within ±10%
- **Verdict:** EWSB reclassified [SELECTION] → [DERIVED]. Manuscript headline result.

**Branch B: Dynamical EWSB fails.**
- ⟨|J|⟩ stays zero or oscillates around zero
- No mass gap or mass gap ≠ M_W
- **Verdict:** EWSB stays [SELECTION]. Reported honestly. Does not invalidate Phases 1–3.

**Either branch is a valid outcome** — we pre-commit to reporting either.

### 7.2 Three-generation cold start (§4B)

**Measurement.** Count distinct stable particle species from cold-start with symmetric ternary seed. Run 50 000 ticks. Classify each stable species by Moore-layer membership.

**Pre-registered expectation:** If exactly 3 stable species per shell type × 4 fermion types = **12 species** emerge, three-generation claim upgrades to [DERIVED] dynamically.

Any other count (0, 4, 6, 9, 18, …) is reported as-is. 12 is not adjusted-to.

### 7.3 Continuum limit (§4C)

**Measurement.** α_eff, σ, and hydrogen 1/n² coefficient at L ∈ {32, 48, 64, 96, 128}. Fit a + b/L² + c/L⁴ → extract continuum value.

**Pre-registered expectations:**

| Observable | Expected continuum value | Tolerance |
|---|---|---|
| α_eff(∞) | 1/137.036 | ±1% |
| σ(∞) | 0.209 (from `proof_complete_sm.py:275`) | ±10% |
| m_p/m_e (∞) | 1836.47 | ±1% |

**[PASS]** if all three continuum extrapolations are within tolerance.
**[FAIL]** if α_eff(∞) disagrees with CODATA by > 5% — this would mean the continuum-limit theorem's conditions (C1–C5) are not satisfied on this lattice.

---

## 8 · Cross-Phase Consistency Requirements

After all phases run, the following internal consistency conditions must hold. Any violation is a finding that triggers auditor review.

1. **α agreement.** Phase 2's α_eff at fine scale (L = 64) must agree with Phase 4C's continuum α_eff(∞) extrapolation to within Phase 2's error bar.
2. **Scaling dimensions.** Phase 1A's anisotropy exponent p and Phase 3's operator scaling dimensions must both be consistent with the same underlying scaling law.
3. **Continuum consistency.** Phase 4C continuum values for α, σ, m_p/m_e must reproduce the values in `scripts/proofs/proof_master_verification.py` (54 checks). No regression allowed.
4. **Ward closure.** Phase 1C Ward-identity precision must be ≤ Phase 2B's measured α_eff uncertainty — otherwise gauge-invariance violation dominates the coupling measurement.

---

## 9 · Verification Procedure

**Per-phase gate** (each phase closes with):
1. CTest suite green: `cd engine/build && ctest -L eft -C Release`
2. Python analysis green: `python scripts/benchmarks/measure_beta_function.py` (Phase 2), etc.
3. Pre-registered expectation met OR negative result documented in the phase's theory doc.
4. Epistemic-auditor agent reviews new tags — no [PARAMETRIC] silently upgraded to [DERIVED] without justification.

**End-to-end** (after all phases):
1. `cd engine/build && ctest -L eft --output-on-failure` — all ~25 EFT tests pass.
2. `python scripts/proofs/proof_master_verification.py` — still passes 54/54 (no regression).
3. `scripts/benchmarks/results/eft_beta/beta_curve.pdf` — manual inspection.
4. `quarto render dissemination/papers/PAPER_FTD_AS_WILSONIAN_EFT/` — manuscript builds.

**Regression guard.** Existing test suites (267 Python tests, ~148 CTest entries) must not decrease. EFT tests are additive only.

---

## 10 · Known Limitations Up-Front

These are *acknowledged* before measurement, not discovered after.

1. **No lattice fermions.** The engine's voxels carry ternary state s ∈ {−1, 0, +1} but not Grassmann-valued fermion fields. Vertex-level Ward identities Γ_μ(p, p) = ∂Σ/∂p^μ require fermion propagators and so cannot be tested in this program. Phase 1C notes this gap explicitly and defers to a future fermion-sector extension.

2. **Cubic lattice breaks rotations at a-scale.** O(3) rotational invariance is expected only in the IR (r ≫ a), not at short range. Phase 1A measures *how* this invariance emerges; it does not claim exact rotational symmetry at the lattice scale.

3. **Three blocking stages is the minimum for β extraction.** Two stages give one β value (no derivative); three give a trend. More stages (L = 128 → 64 → 32 → 16) would reduce statistical uncertainty but were not budgeted for Phase 2; could be added in a follow-up.

4. **EWSB test is binary.** Phase 4A either produces ⟨|J|⟩ condensation or doesn't. A weak positive signal would be ambiguous — we pre-commit to requiring ⟨|J|⟩ stably non-zero for 10 000 ticks with mass gap within 30% of M_W. Less = negative result.

5. **No CP violation measurement.** The lattice dynamics are parity-symmetric by construction; CP-violating operators like J·(∇×J) are expected to have zero coefficient. This is a constraint on what FTD can address, not a deficiency to fix.

6. **Gravity is tested only via latency.** The time-dilation match at 0.004% is strong, but full Einstein-equation recovery (including nonlinear terms beyond post-Newtonian) is outside this program's scope. Phase 5's manuscript section 4 notes this.

---

## 11 · Maintenance

**This spec is updated only when:**
- A new phase is added — add pre-registered expectations before any code runs.
- A risk disclosed up-front materialises — note the outcome; do not edit the original expectation.
- An auditor identifies ambiguity in an expectation — clarify without changing the numerical target.

**Never edit to:**
- Adjust a pre-registered expectation to match a measurement
- Remove a test that produced a negative result
- Retroactively add caveats to a failing prediction

**Cross-references:**
- `../07_assessment/CATALOG_PARAMETRIC_INSERTIONS.md` — what this program aims to reduce
- `../03_derivations/DERIV_CONTINUUM_LIMIT_QED_EQUIVALENCE.md` — the conditional theorem this program tests empirically
- `../07_assessment/TRACKER_OPEN_ITEMS.md` — [OPEN] items whose resolution is a Phase deliverable
- `DERIV_SYMMETRY_RECOVERY.md` (Phase 1 output) — to be written
- `DERIV_BETA_FUNCTION_MEASURED.md` (Phase 2 output) — to be written
- `SPEC_OPERATOR_BASIS.md` (Phase 3 input) — to be written
- `DERIV_OPERATOR_SPECTRUM.md` (Phase 3 output) — to be written
- `DERIV_DYNAMICAL_SM_EMERGENCE.md` (Phase 4 output) — to be written
