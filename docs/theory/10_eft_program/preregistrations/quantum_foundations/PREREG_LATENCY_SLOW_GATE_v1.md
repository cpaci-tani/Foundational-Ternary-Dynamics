# PREREG · The latency sector as slow gate (v1)

**Status:** `[PRE-REGISTRATION — LOCKED BEFORE THE DECISIVE RUNS]`
**Date locked:** 2026-08-07 *(git-tag lock rides the owner's next commit;
the SHA-256s below are the binding locks)*
**Parents:** `FOUND_SPA_CHAIN_RELATIVITY_EXTENSION_v1.md` §13/§14 (the
`[OPEN CANDIDATE — UNSCORED]` registration), `PREREG_BORN_DENSITY_
SATURATION_v2.md` (Outcome B — the slow-gate condition),
`PREREG_BORN_REGIME_MAP_ENGINE_v1.md` (Outcome N — flux noise closed),
temporal-interior front T3.
**Production impact:** none — read-only campaign binary + standalone
mechanism instrument.

## 1. Question

Occupation (Born) weighting of threshold statistics requires a
stochastic element slower than the flux band; flux-borne thermal noise
cannot supply it. **Is the latency sector — the engine's gravitational
degree of freedom, `∇²L = 4πGρ` over manifested matter — slow enough
(Stage A), and does a channel at its timescale actually carry the
weighting toward Born (Stage B)?**

## 2. Locked artifacts

| Artifact | SHA-256 |
|---|---|
| `engine/tests/campaign_latency_slow_gate.cpp` (Stage A, v7) | `1f7b6aac9d7f8c7ba935869b8a240f6dd51b2777bf036144e9d73ffb96ec6d82` |
| `scripts/experiments/temporal_interior/verify_latency_gate_mechanism.py` (Stage B) | `1e58d24d1f92f88e197fd16c779de1f5d1462bae9c4e8228d9fdf8f822b59f0d` |

**Stage A instrument** (seven shakedown revisions, all pre-lock, each
recorded): L = 32, CPU backend (`force_cpu`), profile `disable_all` +
wave + gauss + movement + forces + gravity + latency_field; **no
thermostat and no genesis** (every driven variant floods — the
thermostatted profiles via the unpinned DC zero mode walking `|J|` to
`K_GENESIS` on three different timescales, the genesis-enabled
no-thermostat variant via the evaporation→flux→pair-genesis chain
reaction; both recorded as profile-level findings). Seed: 515-body
checkerboard crystal ball (R = 5, polarity by site parity — the SC bound
phase); under gravity it crunches, annihilation collisions reduce it to
a sparse bound remnant whose configuration dynamics the latency Poisson
tracks. Measured: probe series of `phi_latency` and flux `J_z`
(6 probes, 20,000 ticks after 4,000 equilibration), integrated
autocorrelation times `τ_lat`, `τ_flux`, population trajectory.
**Locked run: fresh seed `20260808` (argv), independent of the
shakedown's `20260807`.**

**Stage B instrument**: the two-mode equal-occupation discrimination
(modes (16,8), `A₁ = 0.10`, `K = 0.5054620197`, total noise σ = 0.17)
with COMPOSITE per-site noise — fast channel `τ = 1.5` (the measured
flux value) mixed with slow channel `τ = τ_lat` — slow share
`f ∈ {0, 0.25, 0.5, 0.75, 1.0}`, 32 seeds, Born-fraction per f with 99%
bootstrap CI. **Handoff rule (locked): `τ_slow` := the locked Stage-A
run's `τ_lat`, rounded to three significant figures, passed as argv.**

## 3. Pre-blessed outcomes

**Stage A** (validity first: `σ_lat ≥ 10⁻⁹`; `τ_lat < 2000` =
AC_MAX/3, converged; population within `[8, 16384]` throughout; no
guard trip — else **I, INVALID**):

| Letter | Condition | Consequence |
|---|---|---|
| **S — SLOW** | `τ_lat/τ_flux ≥ 10` AND `Ω_mid·τ_lat ≥ 30` (Ω_mid = 0.4456) | necessary condition passes; Stage B fires |
| **M — MARGINAL** | `Ω_mid·τ_lat ∈ [5, 30)` | candidacy weakened; Stage B still fires, reported as marginal |
| **F — FAST** | `Ω_mid·τ_lat < 5` | **candidacy DEAD**; the slow-gate search narrows to the s-sector and genesis history; Stage B does not run |

**Stage B** (per its locked runner): **V** — VIABLE-ADDITIVE (BF
nondecreasing in f, BF(1) CI-floor ≥ 0.5); **P** — PARTIAL
(nondecreasing, BF(1) < 0.5); **N** — NOT VIABLE (flat/non-monotone);
**D** — INVALID.

**Combined verdict table:** S+V = *the latency sector is a viable slow
gate at mechanism level* — the strongest gravity–quantum coupling
statement available to the framework at this scope; S+P = timescale
right, strength short at these parameters; S+N = slow but the additive
coupling class does not transfer; M+\* = as above, marginal; F = dead.

## 4. Pre-run observations (recorded before the locked runs)

1. Shakedown (seed 20260807, non-evidential, recorded against hindsight
   bias): remnant N ≈ 17→12, `σ_lat = 3.3e-4`, `τ_lat ≈ 318`,
   `τ_lat/τ_flux ≈ 391`, `Ω_mid·τ_lat ≈ 142` — deep in S territory. The
   locked run is an independent draw of the collapse and remnant.
2. Stage B at `Ω·τ_slow` in the hundreds sits far beyond the v2 curve's
   crossover (≈ 17); the descriptive fit predicts BF(f=1) ≈ 0.86 — i.e.
   V — **if** the composite-noise mechanism behaves like the pure-noise
   scans. The f-scan's real content is the mixed regime.
3. Whatever the outcome, no substrate-level tag moves: Stage A is a
   native-dynamics measurement but the *gate coupling* in Stage B is a
   declared model class (`[IMPOSED]` ensemble; quick-check platform).
   A native latency→genesis coupling channel is a separate, future
   question about the engine's actual manifestation rule.

## 5. Execution

```
engine/build/Release/campaign_latency_slow_gate.exe 0.13 20260808
python scripts/experiments/temporal_interior/verify_latency_gate_mechanism.py <tau_lat>
```

Outcome letters reported verbatim; LEDGER row minted only by the owner.

---

## v1 EXECUTION RECORD (2026-08-07)

**Stage A (locked, seed 20260808): VALID → OUTCOME S — SLOW.**
Crystal 515 → annihilation collapse → bound remnant N ∈ [12, 17] for
20,000 ticks; `σ_lat = 3.253e-4` (≥ 10⁻⁹ ✓); `τ_lat = 317.9` ticks
(converged ✓); `τ_flux = 0.81`; **slowness ratio 390.9 ≥ 10 ✓;
`Ω_mid·τ_lat = 141.7 ≥ 30 ✓** (`Ω_top·τ_lat = 391.3`). *Honesty rider:
the output is bit-identical to the shakedown — this profile has no RNG
consumer (no thermostat, no genesis), so the run is fully deterministic
and RNG-seed independence is vacuous; configuration-varied robustness
(different crystal geometries/radii) is the registered v2 refinement.*

**Stage B (locked, `τ_slow = 318` per the handoff rule): OUTCOME V —
VIABLE-ADDITIVE.**

| f (slow share) | net excess | BF | 99% CI |
|---|---|---|---|
| 0.00 | 2,413,301 | 0.0406 | [0.0314, 0.0490] |
| 0.25 | 2,178,634 | 0.0530 | [0.0400, 0.0665] |
| 0.50 | 1,863,634 | 0.0462 | [0.0315, 0.0601] |
| 0.75 | 1,417,244 | 0.0907 | [0.0737, 0.1112] |
| **1.00** | 666,280 | **0.9361** | **[0.8953, 0.9700]** |

Monotone within the locked tolerance; `BF(1)` CI-floor `0.8953 ≥ 0.5`.

> **COMBINED VERDICT S+V: the latency sector is a viable slow gate at
> mechanism level** — per the pre-blessed table, the strongest
> gravity–quantum coupling statement available to the framework at this
> scope.

Two findings ride with the verdict. **(i) The purity requirement:** the
mixed cells show BF pinned at 0.04–0.09 up to f = 0.75 and leaping to
0.94 only at f = 1 — crossing statistics are dominated by the fastest
stochastic component present, so Born weighting requires the slow
channel to be essentially the *sole* stochastic element at the
threshold. A native realization must therefore shield the manifestation
comparison from band-fast flux noise, not merely add a slow channel.
**(ii) The saturation-question update:** BF = 0.936 at `Ω·τ ≈ 142`
extends the v2 curve (0.836 at `Ω·τ ≈ 78`) — the approach toward 1
continues; the "exactly 1 vs ≈ 0.86" residual now reads as *approaching
1*. Scope unchanged throughout: Stage B's coupling is a declared
additive model class (`[IMPOSED]` ensemble, quick-check platform);
whether the engine's *native* manifestation rule can couple to latency
with the required purity is the registered next question. No
substrate-level tag moves; the LEDGER row is the owner's to mint.
