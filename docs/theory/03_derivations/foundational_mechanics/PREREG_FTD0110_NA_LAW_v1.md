# PREREG — FTD-0110 Nonlinear Bridge: Quantitative N(A) Law from Substrate Parameters

**Status:** `[PRE-REGISTRATION — design locked before any run-of-record]`
**Date:** 2026-06-11
**LEDGER row:** FTD-0110 (nonlinear bridge) / new id at adjudication
**Git tag:** `preregister-ftd0110-na-law-v1` (applied at the lock commit)

## 0 · Purpose

Convert the FTD-0110 nonlinear bridge from `[OPEN]` into either a **derivation** or a
**mapped boundary**. FTD-0267 pinned the mechanism (genesis is a one-shot burst;
cluster size ≈ genesis-firing count). FTD-0261 measured the law (broken power, knee
A≈16). This campaign tests whether a **substrate-parameter forward model that includes
the two flux-injection channels the prior model omitted** — the coupling source
`G_C·∇s` and the Gauss-projection boost `flux[void] −= ∇φ` — reproduces the full N(A)
curve, the knee, and both exponents, and whether it does so from framework-derived
inputs alone or only with the engine-tuning constants (`K_GENESIS_KINETIC_DRAIN`, the
coupling term's √α). Supersedes the over-tagged `DERIV_FTD0110_GENESIS_THROTTLE.md`
(a 3-point count match, no Gauss, knee 23.5≠16).

## 1 · Frozen artifacts (SHA256)

| Role | Path | SHA256 |
|---|---|---|
| Forward model | `scripts/exploration/genesis_na_law_forward.py` | `ea17ccc294e87eac1fcd7d8b6ae9e7c6525b18167fbf0fa27ebd366850ff59b1` |
| Adjudicator | `scripts/exploration/analyze_na_law.py` | `867d99dfbef6187f945c7953b111e3c0f0d9b14dec1bdfbcfa39d369dadcd6c2` |
| Engine instrument | `engine/tests/campaign_genesis_geometry.cpp` | `7bda40a6e57c63e926e3b9183f3565093b96b1b570f32600b7103949b6b2cc36` |

## 2 · Measurement platform (frozen)

- **Engine:** the current canonical post-optimization stack atop HEAD `761daa75` +
  the uncommitted 8-color-SOR Gauss optimization (owner-declared canonical 2026-06-11).
  Physics-diff fingerprint SHA256 `961916b56569d1409984994121f51f3b897c02fe993ebf2ce0e2b03b3d07e381`
  (`git diff engine/src/poisson_solvers.cpp engine/src/render_bridge_phases/phase_forces.cpp`).
- **Determinism:** the genesis/flux FIELD is **bit-reproducible** — verified: two
  identical-seed `campaign_genesis_geometry --cpu` runs at A=14, L=32 produce
  byte-identical firing geometry. (The golden *hash* floats run-to-run due to a
  separate energy-audit parallel-reduction regression that does NOT touch the genesis
  field; flagged for separate fix. The 8-color SOR sweep is race-free; φ-mean removal
  is gauge-irrelevant to ∇φ.)
- **Stack:** canonical ic1 — `wave_propagation + gauss_projection + genesis + coupling
  + langevin(γ=0.02, T=0.005)`; L=32; x-axial point injection `A·K_GENESIS` at center;
  CPU (genesis counters are CPU-only). Forward model: 8 seeds; FFT-exact Gauss
  (`--gauss-mode fft`) for the analytic curve, SOR cross-check available.

## 3 · Frozen target (FTD-0261, public)

N̄(A): 10→4.0, 12→8.4, 14→16.4, 16→21.6, 20→27.4, 25→32.6, 30→45.0, 40→91.8,
50→130.2, 70→260.2, 90→383.3. Knee A≈16; sub-knee exponent p_lo≈3.69; super-knee
p_hi≈1.86; engine fit log10-RMS 0.037.

## 4 · Input taxonomy (decides derivation vs boundary)

- **Framework-derived** (admissible in `[DERIVED]`): `K_GENESIS = N_c·K_MANIFEST`,
  `K_MANIFEST`, `N_c`, `c²=1/3`, the 18-pt O_h Laplacian, `charge_coupling=1`.
- **Engine-tuning** (presence ⇒ boundary): `K_GENESIS_KINETIC_DRAIN=0.5`, `DAMPING`,
  Langevin γ/T, and the coupling term's `G_C=√α` (α is the framework's central
  constant but `[SMC]`, so it is flagged as a separate explicit dependency).

## 5 · Gates and outcome map (mechanical — `analyze_na_law.py`)

- **Broken-power fit** of the model curve (segmented log-log, knee over interior grid):
  returns knee, p_lo, p_hi, fit-RMS. **Curve-RMS** = log10-RMS of model N(A) vs the §3
  target on the shared grid.
- **Firing geometry**: normalized shell-occupancy profiles (center/SC/FCC/BCC/SC2/outer)
  of model vs engine at A∈{14,30}; agreement = L1 distance.
- **PROMOTE → toward `[DERIVED]`** (ALL must hold, framework-only config —
  `--gauss on`, `--coupling on` with √α flagged): knee∈[14,18] **and** p_lo∈[3.3,4.1]
  **and** p_hi∈[1.6,2.1] **and** curve-RMS ≤ 0.10 **and** shell-L1 ≤ 0.30 at both A.
- **BOUNDARY (engine-emergent)**: a drain sweep 0.5→{0.25,0.75} shifts the knee by
  |Δ|>2 (knee is set by the engine-tuning drain), **or** `--coupling off` diverges from
  `--coupling on` by curve log10-RMS > 0.10 (the law is α-load-bearing). Substrate-only
  claim stays `[OPEN]`; the load-bearing constant is named.
- **FALSIFY**: curve-RMS > 0.25 under every config (no framework config gets close).
- **UNDETERMINED**: otherwise (some bands hit, others missed).
- **Geometric-regime report (committed regardless of verdict):** report the model's
  27-block escape amplitude A\* (sub-knee compact vs super-knee bulk Green's-function
  regimes) and compare to the empirical knee, so the two-regimes hypothesis is tested,
  not assumed.

## 6 · Frozen data rules

- F-1: `--gauss off` is a diagnostic arm only (must regress toward the old ~23.5 knee
  to confirm Gauss is the active ingredient); it has no verdict power.
- F-2: the first valid run is the run of record; no seed/window re-rolls.
- F-3: no edit to §5 criteria or the §1 artifacts after the tag; demotion of any tag
  is free, promotion requires this prereg's bands.

## 7 · Stated priors and scope

Priors: **PROMOTE 35% · BOUNDARY 45% · FALSIFY/UNDETERMINED 20%** (the canonical stack
runs coupling-ON, so α-load-bearing BOUNDARY is the modal expectation; clean derivation
is a real possibility if the Gauss boost alone sets the knee). Under every outcome: no
promotion of FTD-0013, MC-T4.3, or the SM cluster-mass identification; the linear k=¼
theorem (O_h) is untouched mathematics. This is not an SM-mass re-assessment.
