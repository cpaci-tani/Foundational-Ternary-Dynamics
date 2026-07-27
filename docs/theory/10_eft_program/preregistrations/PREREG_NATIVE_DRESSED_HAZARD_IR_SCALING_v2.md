# FTD-0436 — Native Dressed-Hazard Infrared Scaling Pre-Registration v2

**Status:** `[PRE-REGISTRATION — LOCKED BEFORE IMPLEMENTATION RUN]`
**Date locked:** 2026-07-23
**Identifier:** `FTD-0436`
**Supersedes in scope:** extends FTD-0433 (outcome C — unresolved scaling);
does not reopen or modify any FTD-0431/0432/0433 locked artifact.

## 1. Question and licensed scope

FTD-0433 measured the pole-phased dressed production hazard for the fixed
`<100>, n=1` square source over `L = 12...48` and returned outcome C: the
largest-volume tail decreased, but the full sequence was nonmonotonic and the
locked monotonicity gates assigned no infrared inference. A deterministic
surrogate replica (exploration of record 2026-07-23, not lock-grade)
attributes the nonmonotonicity to integer-tick sampling of the antinode phase
and, at extended volumes, favors a finite hazard floor over pure power decay.

FTD-0436 asks:

> Under a phase-corrected estimator and volumes extended to `L = 192`, does
> the dressed first-antinode hazard follow pure power decay toward zero, or
> approach a finite nonzero floor?

Both outcomes are finite-volume statements at a pole-defined phase for one
source family. Neither licenses exact polarity conservation, a zero-momentum
decay law, `U(1)`, gauge redundancy, a hydrodynamic pole, long-time survival,
or any structural constant.

## 2. Frozen source family, phase, and dynamics

Identical to FTD-0433 §2: full-occupancy globally neutral axial square source
(`+1` for `0 <= x < L/2`, `-1` otherwise), Fourier family `d = <100>, n = 1`,
`k_L = 2*pi/L`, production 18-point symbol `M_18`, pole
`omega_L = arccos(1 - C_WAVE^2 * M_18(k_L,0,0)/2)`, first-antinode transition
`t_L^* = round(pi/omega_L) - 1`, phase error at most `omega_L/2 + 1e-14`.

The exact continuous transition is

    tau_L = pi/omega_L - 1,

recorded per volume. Active production terms: `wave_propagation`, `coupling`,
`evaporation` only; all other toggles off; `dt = 1`; production constants
unchanged. The FTD-0432 hazard observer
(`engine/include/ftd/eft/native_evaporation_hazard_observer.h`) is reused
WITHOUT modification.

## 3. Execution matrix

Primary WSL2 CUDA/GCC volumes:

    L in {48, 64, 96, 128, 192},

each with seeds `0..7`, recording every transition `t = 0 ... t_L^* + 2`
(the `+2` supplies the interpolation bracket). Independent Windows/MSVC CPU
reproduction at `L = 48`, same seeds and transitions, history journal
enabled; every accepted removal must be journaled as evaporation.

`L = 48` deliberately overlaps FTD-0433's largest volume: its tick-sampled
`h_48^*` must reproduce the FTD-0433 record within combined jackknife
uncertainty (gate G8).

## 4. Structural and observer-validity gates

Gates G1–G7 are FTD-0433 §4 verbatim (registered backend/volume/seeds/
transitions; full initial occupancy, zero global signed state,
`|S_k(0)| >= 0.3`; monotone occupancy and positive source projection through
`t_L^* + 2`; exact registered `omega_L`, `t_L^*`, phase-error bound; site
probabilities in `[0, 0.1]`; finite nonnegative expected removals and
variance; CPU history equals occupancy loss with zero other events; CPU/CUDA
`L = 48` agreement within `1e-10`; ensemble conditional-expectation
standardized residuals max `<= 6`, RMS `<= 2.5` across the primary matrix).

- **G8 (continuity with FTD-0433):** tick-sampled `h_48^*` (v1 estimator)
  within `3 * sqrt(SE_v1^2 + SE_v2^2)` of the FTD-0433 recorded
  `0.00351325 (SE 0.00001889)`.
- **G9 (plane-decomposition closure):** at every volume and recorded
  transition where plane sums are emitted, the plane contributions must sum
  to the observer's `expected_loss_source` within `1e-12` relative.

Failure of any gate is outcome D.

## 5. Locked estimators

**Phase-corrected hazard.** Per seed, from the recorded per-transition
projected hazards `h(t)` (v1 definition `h = Re(Q S*)/|S|^2` at each
transition), form the quadratic through the three points
`t in {t_L^*-1, t_L^*, t_L^*+1}` and evaluate at `tau_L`:

    d  = tau_L - t_L^*
    a2 = (h(t^*+1) - 2 h(t^*) + h(t^*-1)) / 2
    b1 = (h(t^*+1) - h(t^*-1)) / 2
    h_L^phase = h(t^*) + b1*d + a2*d^2.

Ensemble mean over the eight seeds; uncertainty is the delete-one-seed
jackknife SE. Tick-sampled `h_L^*` (v1 estimator) and survival `A_L^*` are
recorded alongside for continuity.

**Model contest.** With the five `(L, h_L^phase, sigma_L)`:

- M1 (pure power): weighted least squares of `ln h` on `ln L`
  (weights `(h/sigma)^2`), 2 parameters; `chi2_1`.
- M2 (floor + power): `h = h_inf + c * L^(-p)`, `p` on the locked grid
  `{0.20, 0.22, ..., 3.00}`; at each `p`, weighted linear LS for
  `(h_inf, c)` with weights `1/sigma^2`; if the unconstrained `h_inf < 0`,
  clamp `h_inf = 0` and refit `c` alone; minimize `chi2_2` over the grid;
  3 parameters.
- `BIC_1 = chi2_1 + 2 ln 5`, `BIC_2 = chi2_2 + 3 ln 5`,
  `dBIC = BIC_1 - BIC_2`.

No other functional families, grids, or estimator variants are authorized.

## 6. Locked outcomes

- **A — FINITE HAZARD FLOOR (tested range):** all gates pass; `dBIC >= +10`;
  fitted `h_inf > 2 * max_L sigma_L`. Licenses: at the pole-defined phase,
  over `L = 48...192`, the dressed hazard of this source family is
  inconsistent with pure power decay and consistent with a finite floor of
  the fitted magnitude. NOT an asymptotic theorem; NOT a conservation
  statement.
- **B — POWER-LAW SUPPRESSION (tested range):** all gates pass;
  `dBIC <= -10`. Licenses continued infrared suppression with no resolved
  floor at the tested volumes.
- **C — UNRESOLVED:** gates pass; `-10 < dBIC < +10`, or `dBIC >= +10` with
  `h_inf <= 2 * max sigma_L`. Reported without fitting further forms.
- **D — INVALID EXECUTION:** any gate G1–G9 fails. No scaling inference.

**Secondary mechanism check (non-gating for A/B/C).** At `L = 96`,
`t = t_L^*`, with wall planes at `x_w in {-0.5, L/2 - 0.5}` (periodic) and
`d(x)` the periodic distance to the nearest wall plane: the summed plane
contributions to `h` from planes with `d <= 8` must be `<= 5%` of the total,
and from planes with `d >= 16` must be `>= 60%`. Reported as
`MECHANISM: CONFIRMED` / `NOT CONFIRMED` alongside the primary outcome.

## 7. Registered external comparators (non-gating)

Deterministic-replica point values (Python surrogate, 8 seeds, different RNG
stream; exploration of record 2026-07-23). Registered for comparison only —
no gate references them:

| L | h^phase (surrogate) | SE |
|---:|---:|---:|
| 48 | 0.00347379 | 0.00002442 |
| 64 | 0.00282694 | 0.00001984 |
| 96 | 0.00215222 | 0.00000680 |
| 128 | 0.00186035 | 0.00000681 |
| 192 | 0.00178713 | 0.00000556 |

(Surrogate recorded transitions only to `t^*`; its `t^*+1` bracket point is
linearly extrapolated where `tau_L > t_L^*` — the engine records the true
bracket, so exact agreement is not expected beyond ~3 combined SE.)

## 8. Artifacts

- campaign: `engine/tests/campaign_native_dressed_hazard_ir_scaling_v2.cpp`
  (new file; v1 campaign and all FTD-0431/0432/0433 sources untouched)
- source lock: `scripts/proofs/native_dressed_hazard_ir_scaling_v2_lock.json`
  + `scripts/proofs/proof_native_dressed_hazard_ir_scaling_v2_lock.py`
- result verifier:
  `scripts/proofs/proof_native_dressed_hazard_ir_scaling_v2_results.py`
- records: `engine/results/ftd_0436/` (hash-locked CSVs + manifest)

No production source, observer formula, constant, toggle default, event
order, or RNG stream may be changed by this campaign.

## Amendment A1 — execution backend for L in {128, 192} (2026-07-24, pre-data)

**State at amendment time (fully disclosed):** WSL2 CUDA arms for
`L = 48, 64, 96` were complete and valid (wall times ~2 min / 16 min /
~5 h). The `L = 128` GPU arm produced **zero records in 3.5 h** (its two
output files carried the sha256 of the empty string,
`e3b0c442...`, and were deleted); the `L = 192` arm was never started. The
cause is the known per-tick CPU-side observer against GPU-resident state
(full-lattice synchronization each call — the same pathology recorded for
FTD-0431's excluded `L = 64` observer arm). Projected GPU wall times
(~14 h for `L = 128`, multiple days for `L = 192`) make the locked
execution matrix infeasible as registered.

**Change (execution only):** the primary backend for `L in {128, 192}`
becomes **Windows/MSVC CPU** (`windows_msvc_cpu`), same binary, same seeds
`0..7`, same transition sets, history journal enabled (adding the CPU
history gate to those volumes). `L in {48, 64, 96}` remain the already
collected WSL2 CUDA arms.

**What does not change:** the source family, pole phase, estimators, model
contest, grids, thresholds, outcome map, mechanism check, and every gate
G1–G9. Backend equivalence is not assumed — it is bound by the locked
`L = 48` CPU/CUDA `1e-10` agreement gate, which both backends' completed
arms already satisfy. No `L >= 128` data of any kind existed when this
amendment was sealed; the amendment cannot have been conditioned on it.

Lock revision 2 reseals this document and the results verifier (whose only
change is the backend-to-volume mapping).
