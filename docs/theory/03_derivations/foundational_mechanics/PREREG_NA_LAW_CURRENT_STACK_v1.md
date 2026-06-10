# PREREG — Current-Stack N(A) Law Characterization + Thermostat Discriminator v2

**Status:** `[PRE-REGISTRATION — design locked before any run]`
**Date:** 2026-06-10
**LEDGER row:** FTD-0261 (reserved)
**Provenance:** successor to FTD-0260 (resolved by owner decision: current stack canonical; the pre-correction `N ≈ ¼·A²` law and SM cluster-mass matches are `[STACK-PINNED — historical]`). This campaign answers the two questions FTD-0260 left open in one pre-registered run: **(Q1)** what law, if any, governs the canonical current stack's ic1 cluster size N(A)? **(Q2)** is that law's shape thermostat physics (Mechanism γ, elevated by FTD-0259)?
**Runner (frozen):** `engine/tests/campaign_thermostat_off_sweep.cpp` (v2: adds `--coupling=on|off`) — SHA256 `2795b5b52af27cfb8a684ba7ac08b17dd9db5c6761d25b039cbadcdbe9667bc4`
**Analysis (frozen):** `scripts/exploration/analyze_na_law_current_stack.py` — SHA256 `270dea767e02a7bd1eaeab4dee684c00c4864f1fdc1e2e3e728b05ee7d699e49`
**Git tag:** `preregister-na-law-current-stack-v1` (applied at the lock commit).

## 1 · Design (frozen)

Protocol = the v1 rig **aligned to the canonical test's toggle set**: `wave_propagation + gauss_projection + genesis + coupling` (the `g_c·∇s` term — engine-default-true; the historical April campaign's coupling-OFF was a `disable_all()` artifact), thermostat per arm; L = 32; x-axial point injection `A·K_GENESIS` at center; burn 200; mean largest 26-connected cluster over a 500-tick window, stride 10; seeds `0xE0102000+s`. Platform: the canonical WSL2 CUDA build (`engine/build_wsl`). v1 lessons baked in: no determinism gate (genesis is RNG-driven regardless of thermostat — v1's V-2 design error); a **flooding rule** instead of an off-arm energy-exit redesign.

| Arm | Thermostat | Coupling | Grid | Seeds |
|---|---|---|---|---|
| **N** (characterization) | ON (γ=0.02, T=0.005) | ON | A ∈ {2,4,6,8,10,12,14,16,20,25,30,40,50,70,90} (knee region 8–16 densified; A* = √(L³T_L) = 12.8) | 5 |
| **X** (discriminator) | OFF | ON | same 15-point grid | 3 |
| **G/T** (descriptive, no verdict power) | ON, γ∈{0.01,0.04}; T∈{0,0.0025,0.01} | ON | A = 30 | 3 |

## 2 · Frozen data rules

- **F-1 flooding:** a grid point with seed-mean N̄ > 1000 is tagged FLOODED and excluded from fits and ratios (the periodic box without friction reverberates — v1 measured N ~ 14k at A ≥ 20 thermostat-off, coupling-off).
- **F-2 trivial:** N̄ < 1.5 is tagged TRIVIAL (sub-threshold/single-voxel anchor) and excluded from fits.
- **F-3:** no re-runs with adjusted seeds/windows; the first valid run is the run of record.

## 3 · Gates and outcome map (mechanical)

- **V-1 rig gate:** arm N's k = N̄/A² at A ∈ {10,14,20,30,50} must match the current-stack canonical-test T5b anchors {0.040, 0.088, 0.068, 0.050, 0.052} within `max(0.02, 60%·ref)` at ≥ 4/5 anchors (loose — catches rig breakage, tolerates windowed-mean vs single-shot protocol difference). FAIL ⇒ RUN INVALID, no outcome claimed.
- **Q1 characterization (arm N, OK points):** candidates **L1** `N = k·A²` (1 param), **L2** `N = c·A^p` (2 params), **L3** broken power-law with knee at a grid point (5 params); fits on log₁₀N. Verdicts: **CLEAN-LAW** (winner log₁₀-RMS ≤ 0.10 and AIC margin ≥ 2), **AMBIGUOUS**, **NO-LAW** (all candidates RMS > 0.25).
- **Q2 thermostat (common valid grid):** R(A) = N̄_X/N̄_N. **Outcome A** (γ active): median R ≥ 1.5 or ≤ 2/3. **Outcome B** (γ inactive): every R ∈ [0.8, 1.25]. **Outcome C**: otherwise. **UNDETERMINED** if < 3 common valid points survive F-1/F-2.
- Tag map: Q1 result lands `[MEASURED — current-stack baseline]` whatever it is; Q2 A/B/C lands at `[MEASURED]`/`[CLOSED NEGATIVE for γ-on-current-stack]`/`[PARTIAL]`. **Under every outcome: no promotion of FTD-0110's [SMC] identification, FTD-0013, or anything else.** Stated priors: Q1 — AMBIGUOUS 40%, CLEAN-LAW 35%, NO-LAW 25%; Q2 — C 40%, A 35%, B 25%.

## 4 · What this is NOT

Not a re-derivation of the linear theorem (k = ¼ from O_h is mathematics, untouched); not a resurrection of the stack-pinned historical table; not an SM cluster-mass re-assessment (that needs the Q1 law first and is the queued follow-up).
