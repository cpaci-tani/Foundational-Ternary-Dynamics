# ANALYSIS — The nonlinear-loop mass-gap measurement (FTD-0270 P2): verdict INVALID, with a strong no-gap hint

**Tag:** `[MEASURED — INVALID per pre-reg]` (the run trips the frozen instability gate; FTD-0270 unchanged)
**Date:** 2026-06-27 · **LEDGER:** FTD-0333 · **Pre-registration:** `PREREG_QDYN_MASS_GAP_v1.md`, git tag `preregister-qdyn-mass-gap-v1`, commit `399093e9`, instrument SHA256 `44a20d76a53779297a8bbf96d84e23a4cd15573be1cc094469ea017bc2fdd21d`.

> **Verdict (pre-reg-faithful): INVALID.** The canonical sweep does not produce a valid measurement — the G2 instability gate fails on every run (`ρ ≈ 1.0025 > 1.0005`), the G1 linear control was mis-calibrated (it measures a dispersing wavepacket mode, not a pure `k=0`), and L=32 floods (`N = L³`). The strong, uniform *science hint* — native `ω₀` pinned at the FFT resolution floor (`≈ 0`, no gap) while the control rings at 0.25–0.38 — points CLOSED-NEGATIVE, but **cannot be claimed as the verdict from runs that fail the frozen gates.** FTD-0270 stays `[MEASURED — BOUNDARY]`; nothing is promoted. The pre-registration did its job: the gates were locked before the run, they flagged it invalid, and no clean story was cherry-picked from compromised data.

---

## 1 · Run of record

`engine/build/Release/campaign_mass_gap.exe --sweep --output-dir engine/results/mass_gap_sweep`, CPU, `OMP_NUM_THREADS=1`, deterministic seed `0xD0270002`, window=4096, probe-radius=5, dt=1.0. Native config (clock OFF): ON `wave_propagation, coupling, genesis, gauss_projection`; OFF `dual_substrate, de_broglie_clock, langevin`. Golden-neutral (observation-only); golden hash `0xb604d81a3d79366e` unaffected. CSV: `engine/results/mass_gap_default/mass_gap_p2.csv`.

## 2 · Data (10 runs)

| L | A | ω₀_forming | ω₀_quiescent | ω₀_control | ρ | g_rate_peak | N |
|---|---|---|---|---|---|---|---|
| 32 | 6 | 1.534e-3 | 3.068e-3 | 0.2509 | 1.00261 | 0 | 32768 |
| 32 | 8 | 1.534e-3 | 1.534e-3 | 0.2509 | 1.00256 | 0 | 32768 |
| 32 | 10 | 1.534e-3 | 1.534e-3 | 0.2509 | 1.00251 | 0 | 32768 |
| 32 | 12 | 1.534e-3 | 1.534e-3 | 0.2509 | 1.00245 | 0 | 32768 |
| 32 | 16 | 1.534e-3 | 1.534e-3 | 0.2509 | 1.00236 | 66 | 32768 |
| 48 | 6 | 1.534e-3 | 1.534e-3 | 0.3781 | 1.00279 | 0 | 1 |
| 48 | 8 | 1.534e-3 | 1.534e-3 | 0.3781 | 1.00271 | 0 | 3 |
| 48 | 10 | 1.534e-3 | 1.534e-3 | 0.3781 | 1.00268 | 0 | 6 |
| 48 | 12 | 1.534e-3 | 1.534e-3 | 0.3781 | 1.00247 | 0 | 10 |
| 48 | 16 | 1.534e-3 | 1.534e-3 | 0.3781 | 1.00239 | 0 | 24 |

`1.533981e-3 = 2π/4096` — the lowest non-zero FFT bin. The native peak-pick lands on it in every run (both windows, both lattices): there is **no resolved oscillation above the floor**.

## 3 · Gate analysis (frozen, §3 of the pre-reg)

- **G2 — instability rejection (`ρ < 1.0005`): FAILS on all 10 runs** (`ρ ≈ 1.0024–1.0028`). Over the 4096-tick window this is `(1.0025)^4096 ≈ e^{10} ≈ 10^{4.5}×` amplitude growth — the field is non-stationary, so the FFT readout is not trustworthy. **The growth may be partly physical** (genesis + coupling inject flux energy — the dark-energy "leak" of FTD-0273/DERIV_DARK_SECTOR), not purely the FTD-0308 numerical leapfrog instability; the locked G2 metric does not separate the two. Either way the run is invalid for an FFT readout.
- **G1 — control validity (`ω₀_ctrl < 0.01`): mis-calibrated.** The linear control (genesis+gauss OFF) was expected to reproduce the established massless `ω₀ ≈ 0`, but a *localized injected wavepacket* disperses as a sum of `k≠0` modes, so its autocorrelation peaks at the wavepacket's dominant mode (0.25 at L=32, 0.38 at L=48), not at `k=0`. The control does not isolate the `k=0` rest mode; the gate's threshold was optimistic.
- **G3 — localized cluster: L=32 floods** (`N = 32768 = L³` — genesis cascades and fills the box; the known flooding regime, only narrow amplitude islands stay localized), **L=48 localized** (`N = 1,3,6,10,24`, monotone in A). So the swept amplitudes mostly miss the stable islands at L=32.
- **G4 — determinism:** deterministic (single seed of record; the GPU constructor banner prints before `force_cpu()` rebuilds the CPU backend — the CPU loop is what ran).

Per the frozen verdict map, **any gate failure ⇒ INVALID.** G2 fails universally; G1 is mis-calibrated.

## 4 · The science hint (recorded, not claimed)

Across all 10 runs — both lattices, both windows, the full amplitude sweep — the native `ω₀` is pinned at the FFT floor (`≈ 0`), far below both the linear control (0.25–0.38) and the FORCED threshold (0.02). The nonlinear genesis↔Gauss loop does **not** generate a `k=0` restoring oscillation; if anything, manifestation **freezes** the flux (the manifested core's flux is more static than the dispersing linear wave). This is the signature of **no native mass gap**, consistent with — and tending to strengthen — the CLOSED-NEGATIVE prior. It is **a hint, not the verdict**: it comes from runs that fail G2, and the pre-reg's CLOSED-NEGATIVE condition (`ω₀^q ≈ ω₀^ctrl`) does not apply because the mis-calibrated control is not `≈ 0`.

## 5 · What a clean v2 needs (the locked-in lessons)

1. **E1 stable integrator** (the pre-reg's parallel phase): an implicit/symplectic/energy-conserving scheme so `ρ → 1`, and a growth metric that **subtracts the physical genesis-injection** rate so G2 isolates numerical instability.
2. **A `k=0`-isolating control** for G1: project the flux onto the `k=0` (spatial-mean) mode, or compare against the analytic massless baseline, rather than the raw wavepacket autocorrelation.
3. **Non-flooding setup for G3:** use `L ≥ 48` (localized) and the stable-island amplitudes (`A ∈ {9.0–9.5}`, `13`), not the flooding-prone `{6,8,10,12,16}` at L=32.

These are a v2 pre-registration, not a re-interpretation of this locked run.

## 6 · Non-promotion

FTD-0270 stays `[MEASURED — BOUNDARY]` (the P2 swing produced no valid measurement — neither hardened nor opened). FTD-0271 `[CONDITIONAL]`, FTD-0013 `[SMC]`, MC-T4.3 `[FOUNDATIONAL OBSTRUCTION]` — all unchanged. No α derived; golden gate untouched. The honest outcome: the swing's first canonical run hit a methodological wall (instability + flooding + an optimistic control), so it is INVALID; the no-gap hint reinforces the boundary-likely prior but is not banked as a result.
