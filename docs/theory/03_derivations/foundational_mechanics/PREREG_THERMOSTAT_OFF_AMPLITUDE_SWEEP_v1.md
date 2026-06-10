# PREREG — Thermostat-OFF Amplitude Sweep v1 (the FTD-0110 k(A)-drift discriminator)

**Status:** `[PRE-REGISTRATION — design locked before any run]`
**Date:** 2026-06-09
**LEDGER row:** FTD-0260 (reserved)
**Provenance:** follows FTD-0259 (`EXPLR_FTD0110_MECHANISM_ALPHA_LEAKAGE_CLOSED.md`): Mechanism α (multi-block irrep leakage) is `[CLOSED NEGATIVE]` as the k(A)-drift mechanism; Mechanism γ (Langevin thermostat) is elevated by two untuned signatures — the drift onset matches `A* = √(L³·T_L) = 12.8`, and the historical rig provably ran thermostat-active (`campaign_amplitude_time_series.cpp`: `langevin=true, γ=0.02, T=0.005`, source-verified).
**Runner (frozen):** `engine/tests/campaign_thermostat_off_sweep.cpp` — SHA256 `2f1b10d20ddb05958c8a36cd13efe278d39d0f6afd49376e0cb14ecbd8565402`
**Analysis (frozen):** `scripts/exploration/analyze_thermostat_off_sweep.py` — SHA256 `324369eea22abc830848cf7f25cb5bc162c263937048221bba884e81d4dfc680`
**Git tag:** `preregister-thermostat-off-sweep-v1` (applied at the lock commit).

---

## 1 · Question

Is the empirical FTD-0110 cluster-efficiency drift `k(A) ≈ ¼·(1 − 0.0257·ln(A/2))` (re-fit, FTD-0259) a property of the **substrate dynamics**, or of the **Langevin thermostat** the historical campaign ran with (γ = 0.02, T = 0.005)?

## 2 · Design (frozen)

Faithful clone of the historical protocol (`campaign_amplitude_time_series.cpp`): L = 32, canonical ic1 toggles (`wave_propagation + gauss_projection + genesis`, `dual_substrate=false`), x-axial point injection `A·K_GENESIS` at the lattice center, burn = 200 ticks, then mean largest 26-connected nonzero-state cluster size over a 500-tick window sampled every 10 ticks; `k = N̄/A²`; seed base `0xE0102000+s`. Amplitude grid = the historical 11 points {2, 10, 15, 20, 28.77, 30, 33.05, 50, 62.42, 85.70, 117.93}.

| Arm | Thermostat | Grid | Seeds | Role |
|---|---|---|---|---|
| **C** (control) | ON, γ = 0.02, T = 0.005 (historical) | 11 A | 5 | rig validation against the historical table |
| **X** (treatment) | **OFF** (`langevin=false`; fluctuation–dissipation ties σ = √(2γT), so this kills friction *and* noise) | 11 A | 2 | the discriminator (2 seeds confirm determinism) |
| **G** (descriptive) | ON, γ ∈ {0.01, 0.04}, T = 0.005 | A = 50 | 3 | friction-dose attribution — **no verdict power** |
| **T** (descriptive) | ON, γ = 0.02, T ∈ {0, 0.00125, 0.0025, 0.01} | A = 50 | 3 | noise-dose attribution (T = 0 isolates pure friction) — **no verdict power** |

Platform: WSL2 build from a clean worktree at the lock commit (the shared tree carries a concurrent session's uncommitted engine edits; the worktree excludes them). Backend as built; bit-exact CPU/GPU parity (70/0) makes the verdict backend-independent; the V-1 gate below is the empirical platform check regardless.

## 3 · Frozen quantities

Historical reference `k_hist(A)`: the 11 values of `FOUND_LATTICE_SPACING_GAUGE_FREEDOM.md` §6.5 (encoded in the analysis script). Bands: FLAT = {2, 10}; KNEE = {15} (descriptive); DEEP = {50, 62.42, 85.70, 117.93}. Drift metric per arm: `D = mean over DEEP of (0.25 − k(A))`. Historical deep drift `D_hist = 0.034`.

## 4 · Decision rules (mechanical; no post-run adjustment)

- **V-1 rig gate:** `|k_C(A) − k_hist(A)| ≤ 0.025` for ≥ 8/11 amplitudes. **FAIL ⇒ RUN INVALID** — no outcome may be claimed; diagnose and re-pre-register.
- **V-2 determinism (off-arm):** the 2 X-seeds give identical `N̄` at every A (tolerance 10⁻⁹). Fail ⇒ flag (RNG leaking into the langevin-off path) and investigate before any verdict.
- **Outcome A — drift is thermostat physics (Mechanism γ dominant):** `D_X ≤ 0.25·D_C` **AND** `|k_X(A) − 0.25| ≤ 0.02` for every DEEP-band A.
- **Outcome B — Mechanism γ closed negative as dominant:** `D_X ≥ 0.75·D_C`.
- **Outcome C — partial:** anything between. No closure claimed; arms G/T inform the follow-up design only.

## 5 · Outcome → tag map (pre-blessed)

- **A:** Mechanism γ `[CONFIRMED — dominant at engine level]`; the substrate-native coefficient is **consistent with the linear theorem's k = ¼** in the deep band `[MEASURED]`; FTD-0110's nonlinear bridge reframes to "linear theorem + thermostat correction" — the *bridge-to-substrate* question collapses, the thermostat correction becomes the new (smaller) `[OPEN]`. **No promotion of FTD-0110's cluster↔mass identification (stays `[SMC]`); the linear theorem's tag is unchanged (already `[DERIVED]`).**
- **B:** Mechanism γ `[CLOSED NEGATIVE]` as dominant; with α closed (FTD-0259), surviving candidates are β (genesis-kink) and front-energetics; bridge stays `[OPEN]`.
- **C:** `[PARTIAL]` — thermostat contribution quantified (`1 − D_X/D_C`); residual non-thermal drift becomes the sharpened `[OPEN]`.

**Under every outcome: nothing about FTD-0013, MC-T4.3, or the spine moves. Prior (stated): A 45 %, C 35 %, B 20 %.**

## 6 · Falsifier / hygiene rules

- F-a: If V-1 fails, the run is invalid — outcome language is banned.
- F-b: Tolerances (0.025 / 0.25 / 0.75 / 0.02) are frozen here; any change requires a v2 pre-registration.
- F-c: Arms G/T may not be promoted into verdict criteria post-hoc.
- F-d: If the off-arm produces qualitatively different objects (no steady cluster; runaway), report as INVALID-X with the raw table — not as Outcome A.
- F-e: Banned: re-running with adjusted seeds/windows to move a marginal verdict; the first valid run is the run of record.

## 7 · What this is NOT

Not a derivation of k(A) under the thermostat (no closed-form γ-model is locked here); not a test of the cluster↔mass identification; not anything about α, FTD-0013, or Born. Engine-level mechanism attribution only.
