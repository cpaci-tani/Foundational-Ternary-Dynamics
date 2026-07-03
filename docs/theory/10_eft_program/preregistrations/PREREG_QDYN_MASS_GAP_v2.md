# PRE-REGISTRATION — The nonlinear-loop native mass gap, v2 (FTD-0270 closure swing P2, rebuilt per FTD-0333/FTD-0337)

**Status:** PROTOCOL — to be hash-locked (SHA256 + git tag `preregister-qdyn-mass-gap-v2`) **before** the canonical measurement run.
**Date:** 2026-07-02 · **LEDGER (on verdict):** row minted by the controller · **Arc:** FTD-0270 quantum-dynamics-ceiling closure, phase P2, second attempt.
**Frozen artifact (§1):** `engine/tests/campaign_mass_gap_v2.cpp` — SHA256 to be recorded at the lock commit.
**Predecessor verdict (binding):** `ANALYSIS_QDYN_MASS_GAP_v1.md` (FTD-0333) — **INVALID per pre-reg**: the G2 instability gate failed on every run (ρ ≈ 1.0025 > 1.0005), the G1 linear control was mis-calibrated (a dispersing wavepacket rings at 0.25–0.38, not ω ≈ 0), and L=32 flooded (N = L³). The strong no-gap **hint** (native ω₀ pinned at the FFT floor in all 10 runs) was recorded, not claimed.
**Mechanism correction (binding):** FTD-0337 — the instability is **bare-wave leapfrog amplitude growth** (a discretization signature: coupling-OFF leaves ρ unchanged; a no-cluster wavepacket still drifts; ρ shrinks ~dt² on the dt-honoring symplectic path; the prior "dt-invariance" was a `set_dt` clamp artifact), **not** a parametric KG-well instability. E1 therefore = a stable **bare-wave** integrator.

---

## 0 · Purpose and honesty ceiling (unchanged from v1)

The question is v1 §0's, verbatim: does the **full nonlinear genesis↔Gauss back-reaction loop** dynamically generate a k=0 restoring oscillation ω₀ > 0 of a manifested resting cluster's flux that the linear operator analysis structurally cannot see? The linear massless baseline (clock-OFF rest mode flat to ~4e-15; s = 0.944 dispersion; FTD-0270/0271) is settled and is **not** the question. Prior: CLOSED-NEGATIVE, sharpened by the v1 hint. A null **hardens** FTD-0270 [MEASURED — BOUNDARY]. **Zero promotions under any outcome** (§5). Golden hash `0xb604d81a3d79366e` untouched (the E1 toggle is default-OFF in the engine; this campaign enables it only in its own bridge instances).

## 1 · Instrument and the three v1 lessons implemented

`engine/tests/campaign_mass_gap_v2.cpp`, CPU canonical (`force_cpu()`, `OMP_NUM_THREADS=1`), CTest `campaign_mass_gap_v2` (no-arg = CI smoke, NOT a measurement). Canonical run:

```
OMP_NUM_THREADS=1 engine/build_wsl/campaign_mass_gap_v2 --sweep --output-dir engine/results/mass_gap_v2
```

CSV of record: `engine/results/mass_gap_v2/mass_gap_v2_v2.csv`.

1. **E1 stable integrator** — `toggles.verlet_wave_integrator` (new engine toggle, default OFF ⇒ golden-neutral; velocity-Verlet KDK: half-kick + drift in phase_write, second half-kick after a post-drift re-read; honors dt < 1). Canonical sweep: **verlet at dt = 0.5**. The legacy and symplectic-leapfrog paths remain selectable (`--integrator=`) for cross-checks only.
2. **k=0-isolating control** — the uniform-J (spatial-mean) mode Jbar(t) is tracked in every run. On the periodic lattice Jbar is exactly conserved by the wave term, the Gauss projection, and the coupling source (all zero-mean lattice differences), so the control's k0 channel must be flat; the control's probe channel must ring in the massless dispersing band. This replaces v1's mis-calibrated "control ω ≈ 0 on the probe readout".
3. **Non-flooding setup** — L ∈ {48, 64}, stable-island amplitudes A ∈ {9, 9.5, 13} (v1 postmortem: L=32 floods at every swept A; A ∈ {9–9.5}, 13 are the localized islands).

**Config (frozen).** Native: ON `wave_propagation, coupling, genesis, gauss_projection, verlet_wave_integrator`; OFF `dual_substrate, de_broglie_clock, langevin, damping, forces`. Linear control: genesis+gauss OFF. Bare-wave control: only `wave_propagation` (+ integrator) ON. Seed of record 0xD0270002; window 4096; probe radius 5; dt 0.5. Injection: `inject_flux(center, A·K_GENESIS·x̂)` (v1 convention). Genesis rates are per-tick; dt is held fixed across the sweep so rows are internally comparable.

**Observables (frozen).** Probe-ball autocorrelation C(t) = Σ_probe J(0)·J(t) (v1 continuity; the sensitive local detector) and the uniform-mode series C0(t) = Jbar(0)·Jbar(t), both DC-removed, FFT'd, peak-picked above 1e-3·PSD_max, leapfrog-corrected ω_phys = (2/dt)·sin(ω_raw/2). Two windows: forming (from injection) and quiescent (genesis rate < 10% of its running peak; J(0) re-baselined). FFT floor at window 4096, dt 0.5: ω_floor = (2/0.5)·sin(π/4096) ≈ 3.07e-3.

**Growth metrics (frozen; the injection-subtracted G2).** E(t) = ½Σ(|J|² + |wave_vel|²).
- `drift_bare` = mean_t (E(t+1)−E(t))/E(t) in the bare-wave control — zero-injection by construction, so it isolates the FTD-0337 integrator growth.
- `drift_adj_q` = mean_t (E(t+1)−E(t)−P_c(t))/E(t) over the native **quiescent** window, with P_c(t) = Σ_{manifested sites + 6-neighbors} wave_vel·(G_C·∇s)·dt the first-order coupling-work estimate (the curl(s·v) source is exactly zero: forces OFF ⇒ velocities identically 0; genesis-drain events are ~absent in quiescence by the window definition).
- v1's ⟨|J|⟩ ratio ρ is reported for continuity but **not gated** (it conflates wavepacket spreading — which raises Σ|J| at fixed Σ|J|² — with instability).

## 2 · Gates (all must pass for a row to count)

- **G1a — k0 null channel:** the linear control's uniform-mode readout yields no peak, or ω₀_k0_ctrl < 0.01.
- **G1b — positive channel:** the linear control's probe readout finds a dispersing-band peak 0.05 < ω < 1.0 with sharpness > 1e-3 (the readout demonstrably detects a real oscillation).
- **G2a — bare-wave stability:** |drift_bare| < 1e-5 per tick. (Recalibrated from v1's ρ < 1.0005: an energy metric on a zero-injection control, per FTD-0337. The no-arg smoke may be used to sanity-check the instrument before the lock, per the v1 precedent; the threshold is frozen here regardless.)
- **G2b — native quiescent drift, injection-subtracted:** |drift_adj_q| < 2e-4 per tick (margin for the Gauss-projection energy exchange, which is constraint bookkeeping, not integrator growth).
- **G3 — cluster formed, not flooded:** N_final ≥ 3 and N_final < L³/2.
- **G4 — determinism:** the canonical sweep is invoked twice with the seed of record (identical CSVs required) and once with a second seed 0xD0270003 (qualitatively consistent rows required: same outcome classification per §4).

Any gate failure on a row ⇒ that row INVALID; G2a failure is a *global* invalidator (the instrument itself is unstable — do not read any row).

## 3 · Frozen discriminators

Let ω₀^q, ω₀^f = quiescent/forming probe peaks; floor = 3.07e-3 (§1).

| Discriminator | Genuine mass gap | Genesis-relaxation artifact | No gap |
|---|---|---|---|
| ω₀^q | > 0.02, coherent (sharpness > 0.1), ≥ 4× floor | vanishes into quiescence | pinned at/below 2× floor, or no peak |
| persistence ω₀^q/ω₀^f | ∈ [0.5, 2] | ≪ 1 (tracks g_rate) | n/a |
| scaling vs A (∝ N) | monotone with N_final across the sweep | tracks g_rate_peak, not N | flat |
| k0 channel (native) | consistent (may be weak/absent for radially symmetric breathing — vector cancellation; the probe channel is primary) | — | flat |

## 4 · Outcome table (frozen)

| Outcome | Condition | Consequence |
|---|---|---|
| **FORCED gap** | ω₀^q > 0.02, coherent, persistent, N-scaling per §3, **all gates pass on all 6 sweep rows** | `[OPEN → candidate]` only — requires a fresh pre-registration + adversarial red-team before ANY tag on FTD-0270/0271 moves. Prior-disfavoured (~5%). |
| **CLOSED-NEGATIVE (no gap)** | All valid rows show no quiescent peak above 2× floor (or only genesis-tracking transients per §3 column 2), gates pass | **Hardens FTD-0270 [MEASURED — BOUNDARY]**; the v1 hint is confirmed as a banked measurement; feeds P4. Prior-favoured (~70%). Cite FTD-0333 + this doc together. |
| **INVALID** | Any G2a failure, or gate failures on > 2 of 6 rows | Re-scope with a postmortem (the v1 discipline); no tag moves; a v3 needs a new pre-reg. (~25% combined with partial-invalid.) |

Rows between the bands (e.g. a coherent peak at 0.005–0.02) adjudicate as **INVALID-borderline → NEITHER-banked-as-open**, not as a weak FORCED: record, do not claim.

## 5 · Tags and priors

Standing invariants under every outcome: FTD-0270 `[MEASURED — BOUNDARY]`, FTD-0271 `[CONDITIONAL]`, FTD-0013 `[SMC]`, MC-T4.3 `[FOUNDATIONAL OBSTRUCTION]`; no α derived; golden gate untouched; the E1 toggle stays default-OFF engine-wide. A FORCED outcome promotes nothing by itself.

## 6 · Banned moves (v1 §7 carries over verbatim, plus)

1. Do NOT report "native flux is massless at k=0" as the result (established baseline; here it is the control).
2. Do NOT conflate FTD-0044 (manifestation-energy gap) with a flux ω(k=0) oscillation.
3. Do NOT read any residual growth as a gap; G2a/G2b exist to prevent exactly this.
4. Do NOT enable `de_broglie_clock` / `db_clock_coulomb`.
5. Do NOT tune ω₀, couplings, thresholds, or the amplitude list post-lock.
6. Do NOT promote on a FORCED outcome without a fresh pre-reg + red-team.
7. Do NOT switch integrator or dt after seeing sweep data; `--integrator`/`--dt` exist for pre-lock instrument validation and post-verdict cross-checks only.

## 7 · Hash-lock

`campaign_mass_gap_v2.cpp` SHA256 recorded and git-tagged `preregister-qdyn-mass-gap-v2` at the lock commit, **before** any `--sweep` invocation (only the no-arg smoke — L=16, window 256, a trivial CI sanity run producing no §3 discriminator reading — may run pre-lock, per the v1 precedent). Run-of-record, analysis, and verdict go in a separate `ANALYSIS_QDYN_MASS_GAP_v2.md` after the lock.
