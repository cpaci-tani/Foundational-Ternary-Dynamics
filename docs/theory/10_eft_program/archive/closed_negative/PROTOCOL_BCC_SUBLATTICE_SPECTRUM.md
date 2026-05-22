# PROTOCOL — BCC sub-stencil two-state spectrum measurement

**Tag:** [PROTOCOL]
**Date:** 2026-04-26
**Implements:** Mechanism C falsifier (`archive/closed_negative/DERIV_MECHANISM_C_GC_BCC_BRIDGE.md` §6); subsidiary tests of L2 candidate identity (FTD-0094) and Bridge Functional ontology (FTD-0095, §2 [OPEN]).
**Unblocked by:** FTD-0051 (Langevin thermostat infrastructure), Cluster A engine build (E1–E8 complete 2026-04-26).
**Status:** Smoke test landed (`engine/tests/campaign_bcc_band_spectrum.cpp`); publication-grade run pending git tag `preregister-cluster-A-vN`.
**Artifacts:**
- `engine/tests/campaign_bcc_band_spectrum.cpp` (harness)
- `engine/include/ftd/sublattice.h` (BCC projector + Laplacian)
- `engine/include/ftd/correlations.h` (sublattice + diagonal correlators)
- `engine/include/ftd/spectrum_extraction.h` (Prony + GEVP)
- `engine/include/ftd/term_toggles.h` (bcc_stencil, langevin_site_filter)

---

## 0 · Physics preamble — what is being measured

Phase G (`AUDIT_ALPHA_EXTRACTION.md`, FTD-0004) showed that the engine's
default 18-pt propagator under the (σ_SC + σ_FCC)/2 stencil is the lattice
Poisson Green's function — geometric, fixed under blocking, zero
fine-structure content. Phase G is therefore not the right place to look
for a master-quadratic spectrum.

FTD-0050 (Link 8 closure) showed that the engine's coupling stencil is
*structurally orthogonal* to the σ_BCC sub-stencil where the master quadratic
lives at the algebraic level. The 18-pt path cannot probe the master
quadratic's dynamical reading.

This protocol probes σ_BCC directly, by:
1. Switching the engine's phase_read Laplacian to BCC-only (`bcc_stencil = BCC`).
2. Thermalizing only BCC voxels with the Langevin OU update
   (`langevin_site_filter = BCC_SITES`) so the BCC sub-stencil dynamics
   reach equilibrium independent of SC/FCC contamination.
3. Sampling a time series of the BCC-projected flux energy
   `ψ(t) = Σ_{i ∈ BCC} |J(i,t)|²` per tick.
4. Computing the temporal autocorrelation `C(τ) = ⟨ψ(t)ψ(t+τ)⟩_t`.
5. Extracting two-state decay rates (λ₊, λ₋) via Prony — and via GEVP
   when a second observable is available.

**Pre-commit (calibration-invariant prediction).** Under Mechanism C
(FTD-0093), the BCC band-edge spectrum is `α · {x₊, x₋}` where (x₊, x₋)
are the master-quadratic roots. The ratio `λ₊/λ₋ = x₊/x₋ ≈ 45.31` is
calibration-invariant and the load-bearing test.

**Pre-commit (control conditions).** SC, FCC, and FULL stencils should NOT
recover ratio 45.31 (per FTD-0050 orthogonality). All four stencil modes
are run in the same harness invocation so no winner can be retrofit.

**Pre-commit (null reading).** A clean 1/L² extrapolation of the residual
to the L→∞ limit IS the structural signal. Anything else
(constant residual, 1/L, 1/L^p with p ≠ 2) indicates a different systematic
and re-routes to D6 [PARTIAL].

## 1 · Ensemble generation

### 1.1 Backend

CPU single-substrate single-precision-double. Hard requirement until OPEN-7
(GPU dual-substrate Langevin) lands. Campaigns must call `rb.force_cpu()`
before any other configuration. The harness already does this; user-side
runs must not bypass.

### 1.2 Toggle profile (canonical)

```
disable_all()
wave_propagation     = true        // Laplacian IS the dynamical operator measured
gauss_projection     = true        // longitudinal projection (warning: 18-pt; see §6 confounds)
langevin             = true
langevin_T           = T_target    // see canonical regime below
langevin_gamma       = 0.05        // per FTD-0051 stability margin
langevin_seed        = SEED        // per-run deterministic
bcc_stencil          = MODE        // SC, FCC, BCC, or FULL — looped over
langevin_site_filter = matching SiteClass   // SC_SITES / FCC_SITES / BCC_SITES / ALL_SITES
```

Validation must pass; the toggle pair `(bcc_stencil != FULL, dual_substrate)`
is mutually exclusive at the single-substrate path until OPEN-7 closes.

### 1.3 Canonical thermal regime

| Knob | Value | Rationale |
|---|---|---|
| T_target | sweep T ∈ {0.005, 0.01, 0.02} | Below K_B = 0.51 to avoid genesis triggers; sweep tests T-dependence of spectrum |
| gamma | 0.05 | Per FTD-0051 acceptance test; equilibration time ~ 1/gamma = 20 ticks |
| burn_in | 5000 ticks per seed | 250× equilibration time; ensures T-independence of post-burn state |
| n_seeds | smoke 2; publication ≥ 8 | SEM scaling 1/√n; ratio precision target 5σ requires ≥ 8 |
| L | smoke 16; publication {16, 24, 32, 48, 64} | finite-size scan for P4 1/L² extrapolation |
| N_measure | smoke 800; publication 4000 | Long enough for τ ∈ [τ₀, N/2] to span ~10 e-folds of the larger eigenvalue |

### 1.4 Equilibration validation

The Langevin equipartition test
(`engine/tests/test_langevin_sublattice_equipartition.cpp`) confirms that
under the BCC_SITES filter, BCC voxels reach `<|v|²> = 3T` within 15% and
non-selected voxels remain at zero (with `wave_propagation = false`). For
the spectrum measurement `wave_propagation = true`, so leakage into SC/FCC
is expected; report the leakage ratio as a systematic in §5.

## 2 · Measurement procedure

### 2.1 Per-seed loop

```
for stencil in [SC, FCC, BCC, FULL]:
    for seed in seeds:
        rb = RenderBridge(L); rb.force_cpu()
        configure(rb, stencil, seed)
        validate toggles
        rb.run(N_BURN)
        psi_series = []
        for t in 0..N_MEASURE:
            rb.run(1)
            psi_series.append(sum_flux_energy_sublattice(rb, matching SiteClass))
        C_tau = temporal_autocorrelation(psi_series, max_tau = min(60, N_MEASURE/2))
        prony = extract_two_state_prony(C_tau, tau0 = 2)
        // optionally: gevp = extract_two_state_gevp(...) when second operator available
        emit_csv_row(stencil, seed, ..., prony.x_plus, prony.x_minus)
```

### 2.2 Cross-seed aggregation

For each (stencil, L, T), report:
- mean(λ₊), mean(λ₋) across seeds
- SEM(λ₊) = std/√n, SEM(λ₋) = std/√n
- mean(ratio) = mean(λ₊)/mean(λ₋), with SEM by error propagation
- mean(sum) = mean(λ₊) + mean(λ₋), SEM by linear error propagation

### 2.3 Finite-size extrapolation

For each (stencil, T), fit ratio(L) = a + b/L² over L ∈ {16, 24, 32, 48, 64}.
The L→∞ extrapolated value is `a`. The pre-registered comparison is
`a` vs 45.31 (BCC), or vs whatever theoretical value applies to SC/FCC/FULL.

### 2.4 GEVP cross-check

For the GEVP extractor, two operators are needed. Recommended pair (per
plan §11 OQ-4):
- O₁(t) = Σ_{i ∈ BCC} |J(i,t)|²            (flux energy)
- O₂(t) = Σ_{i ∈ BCC} |∇·J(i,t)|²          (divergence energy)

Build C₀₀, C₀₁, C₁₁ as time-series autocorrelations of O₁ and the cross-
product. GEVP and Prony spectra should agree within statistical error;
disagreement is a falsification signal.

## 3 · Pre-registered falsification thresholds

| ID | Quantity | Pre-registered value | PASS / FAIL criterion |
|---|---|---|---|
| F1 | BCC ratio (L→∞) | 45.31 | `\|measured − 45.31\| / 45.31 < 5σ_stat` |
| F2 | SC ratio  | NOT 45.31 | SC measured ratio differs from 45.31 by > 5σ_stat |
| F3 | FCC ratio | NOT 45.31 | FCC measured ratio differs from 45.31 by > 5σ_stat |
| F4 | FULL ratio | NOT 45.31 | FULL measured ratio differs from 45.31 by > 5σ_stat |
| F5 | BCC sum (L=64) | 16 G*² ≈ 140.06 (modulo dimensional rescaling factor — see §3.1) | within 1% |
| F6 | Finite-size scaling exponent | p = 2 (ratio(L) = a + b/L^p) | χ²/dof of p=2 fit < χ²/dof of p=1 fit |
| F7 | Ward residual on σ_BCC | ≤ 10⁻⁸ | matched-stencil CG path; AUDIT_WARD_IDENTITY |
| F8 | Equipartition `<\|v\|²>_BCC / 3T` at burn-end | 1.0 ± 0.15 | per-seed; report systematic if drift |

PASS = all of F1, F5, F6, F7, F8 pass AND all of F2, F3, F4 pass (controls).
FAIL = any F1/F5/F6/F7 fails (load-bearing) OR any F2/F3/F4 fails (control falsifies bridge specificity).
PARTIAL = mixed; per-criterion table in D6.

### 3.1 Note on F5 (sum)

The raw `λ₊ + λ₋` extracted from the temporal correlator decay rate is in
units of (1/tick) and depends on the BCC stencil's per-step decay
multiplier `e^{-λ}`. The master-quadratic algebra gives `x₊ + x₋ = 16G*²
≈ 140.06` in the dimensionless lattice eigenvalue convention.

The conversion factor between extracted decay rate and master-quadratic
eigenvalue is the BCC Laplacian's per-step scaling (the factor 3 from the
√3 distance to the corner neighbor, see `sublattice.h` Taylor analysis).
Specifically, for the BCC kernel: `eigenvalue_continuum = 3 · eigenvalue_per_step`.

We will report both raw and rescaled. The ratio `x₊/x₋` is invariant
under this rescaling — that is why F1 is the load-bearing test, not F5.

## 4 · Blocking

Block-spin transformation b=2 from L=64 → L=32 → L=16 → L=8. At each scale,
re-extract the spectrum and check that the ratio is preserved within
statistical error. Drift > 1% between adjacent scales indicates that the
BCC channel is not a closed sub-sector under blocking, which itself is
informative (re-routes to D6 [PARTIAL] for sublattice identification).

Implementation: re-use `engine/include/ftd/eft/blocking.h` with sublattice
labels preserved.

## 5 · Systematics

| Source | Magnitude | Mitigation |
|---|---|---|
| Seed-to-seed scatter | ~10% per seed at L=16, smoke; ~3% at L=64, n_seeds=8, publication | report SEM, scale n_seeds |
| Equilibration bias | < 1% if N_BURN = 5000 (250× 1/γ) | verified by AUDIT_LANGEVIN runs |
| Wave-propagation leakage to non-BCC sublattices | unbounded with wave on; ratio still measurable but background contaminates absolute scale | report ratio (calibration-invariant) as the load-bearing quantity, not sum |
| Genesis trigger contamination | T < K_B avoids; test that no manifestation events fire | log per-tick state-change count; assert 0 |
| 18-pt gauss_projection on BCC stencil | inconsistency between Laplacian (BCC) and longitudinal projection (18-pt) | run also with `gauss_projection=false` and bound systematic |
| Prony / GEVP disagreement | extractor sensitivity to noise | report both; D6 verdict requires agreement within 1σ |
| Lattice anisotropy on BCC kernel | per AUDIT_LORENTZ_ANISOTROPY (FTD-0092) δ ≈ 6.5e-8 at L=64 — 6 OOM below 5σ | negligible |
| CFL stability at default C_WAVE | factor-3 Taylor coefficient on BCC; may require dt reduction | test in E2 long-run; if unstable, dt → dt/2 (no kernel rescaling) |

## 6 · Confound checks

(a) **Toggle profile sensitivity.** Re-run with `langevin = false, T = 0`
   (deterministic dynamics on σ_BCC). The ratio extraction should fail
   (no thermal correlator). If it succeeds, the spectrum is coming from
   somewhere unintended — probably initial conditions or genesis residue.

(b) **Single-substrate verification.** Confirm `dual_substrate = false`
   throughout. Toggle validation enforces this when `bcc_stencil != FULL`.

(c) **Equipartition on selected sublattice.** Per-seed
   `<|v|²>_BCC at end of burn-in / (3T) = 1.0 ± 0.15` per F8.

(d) **Deterministic-only sanity (zero T).** Run with
   `langevin = true, T = 0, gamma = 0.05`. The Langevin update reduces
   to pure damping; the spectrum extractor should report decay rates
   matching the engine's deterministic damping coefficient. If not,
   the BCC dispatch path has a bug.

(e) **Non-default G_C control (Mechanism C circularity test, per D1 §7).**
   Run with `G_C = 0.1` (factor 1.17 off √α). If the ratio λ₊/λ₋ remains
   45.31, the prediction is calibration-invariant (R1, Mechanism C
   constructive). If the ratio shifts, R2 (circular) is correct and
   Mechanism C closes negative.

(f) **CSV completeness.** Every (stencil, seed) pair must produce a CSV
   row, even if Prony fails. Use `valid` and `failure_reason` fields to
   record why.

## 7 · Resource budget

| Run | Hardware | Wall time | Storage |
|---|---|---|---|
| Smoke (current, L=16, n_seeds=2, T=0.005) | CPU single-thread, default Windows MSVC | 9.5 s | <1 KB CSV |
| Publication tier 1 (L=32, n_seeds=8, T-sweep × 3) | CPU single-thread | ~ 90 min | ~10 KB CSV |
| Publication tier 2 (L ∈ {16, 24, 32, 48, 64}, n_seeds=8, T-sweep × 3) | GPU once OPEN-7 lands; CPU workaround ~ 15 hrs | ~30 KB CSV |

Tier-2 publication run requires either OPEN-7 (GPU dual-substrate Langevin)
or a long-running CPU campaign on WSL2 + RTX 5090 path. Until OPEN-7,
publication runs proceed at tier 1 only; tier 2 results held until GPU
path is verified.

## 8 · Outputs

CSV columns (matches `campaign_bcc_band_spectrum.cpp` header):
```
stencil, seed, L, T, gamma, N_burn, N_measure,
x_plus, x_minus, sum, ratio, valid_prony, prony_failure
```

JSON manifest (publication run, written separately):
- pre-registration git tag (`preregister-cluster-A-vN`)
- `git rev-parse HEAD` at run start
- toggles snapshot
- timestamps (start, end)
- environment (CPU model, MSVC version)

## 9 · Cross-references

- **D1 derivation:** `archive/closed_negative/DERIV_MECHANISM_C_GC_BCC_BRIDGE.md`
- **D6 results audit (post-run):** `AUDIT_BCC_SUBLATTICE_RESULTS.md`
- **D5 look-elsewhere:** `PROTOCOL_LOOK_ELSEWHERE_SCAN.md` (cross-validation)
- **AUDIT_LORENTZ_ANISOTROPY.md** (FTD-0092) — lattice anisotropy bound
- **AUDIT_OPERATOR_SPECTRUM.md** (FTD-0091) — operator basis
- **AUDIT_WARD_IDENTITY.md** — Ward residual baseline
- **AUDIT_LINK8_CLOSURE.md** (FTD-0050) — orthogonality theorem motivating BCC dispatch
- **DERIV_LANGEVIN_THERMALIZATION** (referenced from `term_toggles.h`) — Langevin acceptance test
