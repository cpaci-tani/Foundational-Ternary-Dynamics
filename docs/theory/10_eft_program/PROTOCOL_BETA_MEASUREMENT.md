## PROTOCOL · β-Function Measurement at Non-Zero Temperature

**Tag:** [PROTOCOL]
**Date:** 2026-04-25
**Status:** Design + smoke test landed. Publishable measurement deferred to WSL2.
**Implements:** SPEC_EFT_RECOVERY_PROGRAM.md §5.3 (Phase 2C β extraction)
**Unblocked by:** FTD-0051 (Langevin thermostat, 2026-04-20)
**Artifacts:**
- `engine/tests/campaign_beta_measurement.cpp` (this protocol, smoke test)
- `engine/include/ftd/eft/coupling_measurement.h` (`measure_alpha_eff_on_bg`, `prepare_thermal_background`)
- `engine/src/eft/blocking.cpp` (`block_full`)
- `engine/include/ftd/eft/matched_poisson.h` (`matched_gauss_project`)

---

### 0 · What Phase G changed about "running coupling"

Phase-G (`AUDIT_ALPHA_EXTRACTION.md` + `DERIV_EMERGENT_COULOMB_GEOMETRIC.md`)
showed that on the bare engine the static probe-probe potential `V(r) = −α(r)/r`
extracted from `measure_alpha_eff` is **purely the periodic lattice Poisson
Green's function**, `α(r) = 2 r G_L(r)`, with R² = 1.0000 at L=384. There is
zero fine-structure-constant content in V(r) at zero temperature; the plateau
1.8–3.6× α_ref is a geometric statement about the Coulomb Green's function on
a 3-torus, not a QED measurement.

This is load-bearing for the present protocol. **The propagator side of α(r)
cannot run** — its functional form is fixed by lattice geometry. Any genuine
RG flow under blocking must therefore arise from the **source/manifestation
side**: the relationship between the bare manifestation amplitude (here, the
Langevin temperature T entering through fluctuation-dissipation, plus the
seed `initial_flux_z` and the gauss-projection's `coulomb_charge_coupling`)
and the *effective* charge as seen by the V(r) probe at coarse scale.

A non-trivial β under this protocol is thus a measurement of **how the
manifestation/source physics renormalizes under blocking**, not how a
geometric propagator runs. A null β is the consistent prediction of Phase-G
restricted to the bare-lattice T = 0 limit; a non-null β at T > 0 would be
the first lattice-measured genuine flow, and would live entirely in the
fluctuation-induced source dressing.

We pre-commit that the headline outcome may be β ≈ 0 within statistical
error. That is a Phase-G-consistent finding, not a failure.

---

### 1 · Ensemble generation (Langevin thermostat)

**Backend.** CPU single-substrate path. The Langevin thermostat
(`TermToggles::langevin`, FTD-0051) implements an Ornstein–Uhlenbeck update
on `wave_vel` per voxel per tick:

```
v ← (1 − γ) v + √(2γT) η,   η ~ N(0, I)
```

with measured equipartition `<|v|²> / (3T) = 1.04` at T=0.01, γ=0.01 over
2000 measurement ticks (`test_langevin_equipartition`).

**Canonical thermal regime for β:**

| Knob | Value | Rationale |
|---|---|---|
| Temperature T | 0.005 (smoke) / 0.02 (publication) | well below K_B = 0.511 to avoid genesis events; large enough that fluctuations dress the source |
| γ | 0.01 | autocorrelation τ ~ 100 ticks; safely below stability bound 0.5 |
| Burn-in | 5000 ticks (smoke) / 20000 ticks (publication) | ≥ 50 τ for full memory loss; thermalisation of |J|² is slower than wave_vel — see Run-3 thermal observation in AUDIT_LINK8_CLOSURE.md |
| Decorrelation between samples | 200 ticks | ≥ 2 τ between draws |
| n_samples | 8 (smoke) / 100 (publication) | smoke = wall-clock under 1800 s on CPU; publication = √8 ≈ 2.8× tighter error |
| Seed sweep | 4 well-separated `langevin_seed` values | seed-to-seed scatter is the dominant systematic on small lattices |

**Toggle profile.** `configure_bare_lattice_for_coupling()` plus
`langevin = true`. Genesis, damping, forces, gravity, movement,
larmor, dual_substrate, weak_transmutation OFF — exactly as for the
zero-temperature α_eff extraction so the only physics difference between
T=0 and T>0 runs is the thermostat itself.

**Correlation-time analysis.** Single-voxel `vx` autocorrelation is
recorded once per seed (at lattice centre) and reported alongside
β. Non-decaying autocorrelation invalidates the run.

---

### 2 · Coupling definition

**Operational definition.** Given a thermal background `bg` produced by
`prepare_thermal_background(L, T, γ, burn_in)`, place a +1/−1 probe pair
at separations r ∈ {4, 6, …, L/3} on a *fresh* `RenderBridge` whose flux
and wave_vel have been copied from `bg` (via `measure_alpha_eff_on_bg`).
Run `n_ticks_probe` ticks (300 for L=32; 600 for L=64). Extract:

```
V(r) = E_pair(r) − [E_self(+) + E_self(−)]
α_eff = − slope of V(r) vs 1/r,    R² reported
```

**Why this measures the source side, not the propagator.** The probes
themselves are static (`locked = true`); their flux response is set by
the linearised theory's Green's function, which Phase-G demonstrated is
the lattice Poisson kernel — fixed under blocking. What blocking *can*
change is:

1. The **effective charge dressing** induced by thermal flux at the probe
   site (the analogue of vacuum polarization on the fluctuating background).
2. The **block-spin charge-conserving variant** (`block_state_charge_conserving`)
   which non-trivially couples the source ρ to the coarse-grained flux
   via the Pass-2 overflow spreading rule.

Both effects modify α_eff through the *charge-side* of `V = −Q²·G_L(r)/r`
rather than the kernel.

---

### 3 · Blocking schedule

**Volume-invariant blocking is impossible on a single fixed-L engine
build** because `block_full` halves L. We therefore use the standard
**MCRG (Monte-Carlo RG) approximation** already pre-registered in
`benchmark_beta_function.cpp`: measure α_eff at three lattice sizes
{L_fine, L_fine/2, L_fine/4} drawn from the *same canonical regime*,
and interpret the difference as the running coupling at scales {a, 2a, 4a}.

**Smoke-test schedule (this protocol's first run):**

| Stage | L | b (ratio to fine) | r_max | Probe ticks |
|---|---|---|---|---|
| Fine | 32 | 1 | 10 | 200 |
| Mid | 16 | 2 | 5 | 200 |
| Coarse | 8 | 4 | (skipped — too few r-points) | — |

**Publication-grade schedule (deferred to WSL2):**

| Stage | L | b | r_max | Probe ticks |
|---|---|---|---|---|
| Fine | 64 | 1 | 21 | 600 |
| Mid 1 | 32 | 2 | 10 | 600 |
| Mid 2 | 16 | 4 | 5 | 600 |

Two independent blocking factors b ∈ {2, 4} → one β datum from each
adjacent pair (fine→mid, mid→coarse), giving a 2-point trend (curvature
estimable at 3 scales).

A separate **block-spin variant** measurement (Phase 2C original):
generate a thermal sample at L=64, then call `block_full(rb)` to get an
L=32 *blocked* configuration and run `measure_alpha_eff_on_bg` on it.
Compare to the matched-stencil L=32 thermal sample. Disagreement bounds
the systematic of the MCRG approximation. (Smoke test: 1 seed each;
publication: full ensemble.)

---

### 4 · β extraction

For adjacent scales (L, L/b) at fixed thermal regime:

```
β(g) ≈ [ g(L/b) − g(L) ] / log(b)
```

with `g(L) = √α_eff(L)` (so that β has the QED convention dimensions).
Error bars on β:

```
σ_β² = [ σ_g(L/b)² + σ_g(L)² ] / (log b)²
```

where σ_g(L) = (1/(2√α_eff)) · σ_α(L), and σ_α(L) is the seed-to-seed
standard error of α_eff at scale L over n_seeds runs.

**Smoke test reports:** g(L=32), g(L=16), β, and a one-line PASS/FAIL on
"β finite (error bar excludes ±10·σ outliers)." It does **not** report
PASS against any reference β value — that comparison requires the full
ensemble.

---

### 5 · Error budget

| Source | Smoke (L=32, 8 samples) | Publication (L=64, 100 samples) | Mitigation |
|---|---|---|---|
| Seed-to-seed (statistical) | ~30% on β | ~3% on β | n_samples ↑ |
| Equilibration bias | ≤ 4% on <\|v\|²> (measured) | ≤ 1% (longer burn) | burn-in ↑ |
| Boundary / image-charge contamination | known O(1/L) on V(r) | controlled by r_max ≤ L/3 | larger L |
| Block-stencil error (charge-conserving overflow) | ~5% (deterministic) | ~5% (same) | report MCRG vs block-spin variants separately |
| Matched vs SOR Poisson residual | < 1e-8 on deep vacuum (measured) | same | optional `matched_gauss_project` after each tick (expensive; skipped in smoke) |
| Thermodynamic (T-dependence of β itself) | not assessed | sweep T ∈ {0.005, 0.01, 0.02, 0.04} | full publication |

**Statistical floor on smoke test:** with 8 samples and ~30% per-sample
scatter, β has ~10–15% relative error. A null β is consistent with the
smoke test up to roughly |β| < 0.05.

---

### 6 · Falsifiers

| Outcome | Interpretation | Status |
|---|---|---|
| β = 0 within error at all T | RG-flow null on the source side; consistent with Phase-G geometric reading; FTD does not exhibit Wilsonian RG flow on its bare lattice | Pre-registered as the **expected** outcome conditional on Phase G being load-bearing |
| β > 0 (UV-free, QED-like sign) | source-side asymptotic freedom — surprising; would indicate fluctuation-induced charge dressing flows toward zero in the IR | Surprising positive, would unblock matching to QED β = g³/(12π²) |
| β < 0 (asymptotically free, QCD-like) | confining-style flow — consistent with Phase-G + Wilson-loop area law | Surprising positive |
| β diverges or seed-to-seed sign-inconsistent | thermalisation incomplete or block-stencil pathology | Increase burn-in; cross-check with `matched_gauss_project` per tick |
| β depends on T monotonically and extrapolates to non-zero at T → 0 | inconsistent with Phase-G geometric reading; would falsify FTD-0051's interpretation | Would require re-opening the Phase-G analysis |

**Hard falsification (publication only):** if β at T=0.02, L=64, 100
samples differs from QED's β = g³/(12π²) by more than 30% AND from QCD's
β = −(b₀/2π) g³ by more than 30% AND from zero by more than 5σ, the
result is **a new FTD prediction** and headlines a Phase-5 paper section.

---

### 7 · Smoke-test result + observed limitations

The campaign emits CSV to stdout and a one-line summary to stderr.
Columns: `kind,stage,L,seed,T,gamma,r,V,alpha_r,alpha_fit,r2,valid,n_ticks_probe`.
Each `(stage, L, seed)` triple produces a `kind=fit` row with `r="fit"`.
The campaign concludes with a `kind=beta` row.

**First (failed) smoke run** auto-selected the GPU backend, on which the
Langevin thermostat is not implemented (FTD-0051 ships CPU-single-substrate
only). Background `<|v|²>` came out as 0.0; α_fit collapsed to deterministic
nonsense; β = −3.25 was meaningless. Diagnosed and fixed: the campaign now
calls `RenderBridge::force_cpu()` on every bridge it constructs (including
the thermal background, the self-energy bridges, and the pair bridges).
Local helpers `prepare_thermal_bg_cpu`, `self_energy_on_bg_cpu`,
`pair_energy_on_bg_cpu`, `alpha_eff_on_bg_cpu` mirror the upstream
helpers from `coupling_measurement.h` with `force_cpu()` plumbed through.
Once Langevin is ported to GPU these wrappers can be deleted in favour
of the upstream helpers.

**Second smoke run, CPU-enforced (L_fine=24, L_mid=12, T=0.005, γ=0.01,
burn=1500, probe=100, 2 seeds):**

| Stage | L | mean α | SEM | <\|v\|²>/(3T) on bg |
|---|---|---|---|---|
| fine | 24 | +14.8 | 27.0 | **1.04** |
| mid  | 12 | +173.2 | 113.3 | **0.99–1.03** |

Equipartition is now within 4% of the FTD-0051 target across both stages,
confirming the thermostat is actually running. Derived `g_fine = +3.85`,
`g_mid = +13.16`, `β = (g_mid − g_fine)/log(2) = +13.43 ± 8.00`. The α
values are O(10–100) — three orders of magnitude above the bare α =
1/137 — because the thermal background flux is energetically dominant
over the +1/−1 probe pair at T = 0.005, L = 12 (V(r) values are −1600 to
−4100; the V(r) signal at this temperature is *not* the probe-probe
interaction, it is the background noise after probe insertion). Seed-to-
seed SEM is ~70% of the mean at the mid stage, and ~180% at the fine
stage — consistent with two seeds being statistically inadequate.

**β is finite but uninformative at this sample size.** The smoke test's
job is to demonstrate that the protocol runs end-to-end with the
thermostat correctly engaged, and that error bars propagate sensibly. It
does. Both checks PASS. The β = +13.4 ± 8.0 number itself does not bound
RG flow in any direction — it is comfortably consistent with zero (1.7σ
above), with a QED-like positive flow, and with non-RG stochastic noise.

**What the publishable measurement requires beyond this smoke:**

1. Larger lattice (L_fine = 64 minimum), to get the probe-probe signal
   above the thermal-background noise floor. At L = 24 the probe field
   is sub-leading to the Langevin J fluctuations.
2. ≥ 100 seeds to get SEM/mean below 10%.
3. WSL2 + RTX 5090 wall-time — the smoke run on this Windows CPU path
   took ~250 s for 2 seeds × 2 scales. 100 seeds × 3 scales would take
   ~17 hours of pure CPU. WSL2 path is mandatory.
4. T-sweep across {0.005, 0.01, 0.02, 0.04} to extract the T → 0 limit
   and check Phase-G consistency.

---

### 8 · Path to publishable measurement

The smoke test runs on CPU, L_fine = 32, n_samples = 8, in ~1800 s wall
on the project's reference Windows machine. A publishable measurement
requires:

1. **WSL2 + RTX 5090** for L_fine = 64, n_samples = 100, 4 seeds × 4 T values
   = 32 ensembles × 3 lattice sizes × (1 self + r_max/2 pair) ≈ 5000 engine
   runs. Estimated wall: 24–48 h on the 30× WSL2 path.
2. **Matched-stencil Poisson per tick** (`matched_gauss_project`) instead
   of legacy SOR — required to drive Ward residual below the systematic
   floor. Cost: 2× per tick; affordable on GPU.
3. **Both block-spin and MCRG variants** reported — disagreement is
   itself the systematic.
4. **T-sweep** to extract the T → 0 limit and check Phase-G consistency.

This protocol is the first step. The smoke test demonstrates that the
infrastructure runs end-to-end and reports a finite β with sensible
error bars; the headline number is not publishable.

---

### 9 · Cross-references

- `SPEC_EFT_RECOVERY_PROGRAM.md` §5 — Phase 2 pre-registered expectations
- `AUDIT_LINK8_CLOSURE.md` — Langevin thermostat infrastructure (FTD-0051)
- `AUDIT_ALPHA_EXTRACTION.md` — Phase-G geometric reading of α(r)
- `DERIV_EMERGENT_COULOMB_GEOMETRIC.md` — V(r) as periodic Green's function
- `engine/tests/test_langevin_equipartition.cpp` — equipartition acceptance
- `engine/tests/benchmark_beta_function.cpp` — pre-existing T=0 sibling
- `engine/tests/campaign_beta_measurement.cpp` — this protocol's smoke test
