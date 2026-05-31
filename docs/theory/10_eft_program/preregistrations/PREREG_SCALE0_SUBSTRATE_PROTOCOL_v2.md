# Pre-Registration — Scale-0 Substrate Experimental Protocol (v2)

> **STATUS: DRAFT — NOT YET LOCKED, NOT YET RUN.** This file is being prepared for hash-lock; it has
> not been committed/tagged and no engine run has executed against it. (Per the owner's instruction:
> build the fixes, do not lock-and-run yet.)

**Tag (target, not yet applied):** `preregister-scale0-substrate-protocol-v2`
**Date:** 2026-05-31
**Supersedes:** the **defective entries** of v1 (I1, I2, the ic1/genesis size test, I5). v1 remains
**locked and immutable** for provenance: `PREREG_SCALE0_SUBSTRATE_PROTOCOL_v1.md`, tag
`preregister-scale0-substrate-protocol-v1`, commit `e8ce032b`, SHA256
`225eded86df28233522f2260f220cd5ec7024c0f1eb9449e4c365a5ee93b431e`.
**Basis for revision:** `../../07_assessment/audits/AUDIT_SCALE0_SUBSTRATE_RESULTS.md` (Run 1).

---

## §R0 — What changed and why (the four Run-1 findings → four fixes)

| Run-1 finding | v1 defect | v2 fix |
|---|---|---|
| **F1** `selective_damping` toggle-trap | v1 didn't account for `#t-selective` forcing damping on | §R1 conservative-config recipe turns `#t-selective` **off first**, with a console-error gate |
| **F2** I2 measured the wrong quantity | v1's `fieldEnergy+waveEnergy` (`½|J|²`) is not the wave Hamiltonian (it sloshes) | §R2: readout = `EFieldEnergy+BFieldEnergy` (= `½(E²+B²)`, the conserved Maxwell energy) |
| **F3** `c_lat` under-instrumented | v1's absolute max-radius is dominated by the large initial pulse; center hard-coded wrong | §R3: front-tracking via `Δr/Δt`, center = `L/2`, minimal-width pulse, + strict locality bound |
| **F4** ic1 size build-dependent; lattice default L=64 | v1 tested the `[EMPIRICAL]` absolute ~25 as if firm | §R4: test the *calibration-independent scaling* (exponent 2, the firm part), set L explicitly, separate the `[EMPIRICAL]` coefficient |

**Inherited unchanged from the LOCKED v1** (cite v1 by its SHA; not re-litigated): §0 honest scope,
§1 apparatus + readout, §2 methodology (M1–M5, three outcomes, tiering), §5 the Illustrative catalog
(the ~60 seeded scenarios, still `[ILLUSTRATIVE]`, not theory tests), §6 falsifier rules + banned
moves, §7 scoring, §8 hash-lock protocol. **CONFIRMED-in-Run-1 tests carried forward unchanged:**
I3 (Gauss constraint → 0) and ic4 (sub-threshold null). I6 (locality) is folded into §R3.

---

## §R1 — Config recipes (LOCKED macros)

**`conservative_config()`** — for the conservation/wave tests. Apply toggles **in this order**
(the order matters — F1):
1. `#t-selective` OFF  (selective_damping — must go first; it *requires* damping)
2. `#t-damping` OFF, `#t-genesis` OFF, `#t-coupling` OFF, `#t-movement` OFF, `#t-forces` OFF (+ all
   force sub-toggles), Langevin OFF (T=0)
3. `#t-wave` ON; `#t-gauss`/`#t-poisson` may stay ON (Gauss residual ≈ 0 ⇒ projection is
   energy-neutral, confirmed Run-1 I3).
4. **Gate:** read `preview_console_logs(level:error)` — **zero** `[TermToggles] Invalid combination`
   entries, else the config is not conservative and the test is inadmissible (F4 of v1).

**`genesis_config()`** — for the genesis/cluster tests: `#t-wave`, `#t-gauss`, `#t-genesis` ON;
Langevin as specified per test; `#t-damping`/`#t-forces`/`#t-movement` per the scenario default.

**Lattice size (F4):** every test **sets L explicitly** (the web default is L=64; FTD-0107 used
L∈{32,64}). Resolve the size control at run-time (substrate-controls "SIZE" selector) and record the L used.

**Stepping:** app **paused** (no RAF contention); advance by exact `bridge.tick()` counts; keep
per-eval `getEnergyAudit()` calls ≤ ~6 and tick batches ≤ ~150 (Run-1: 18 audits + 225 ticks timed out).

---

## §R2 — I2 energy conservation (REVISED, LOCKED)

| Field | Spec |
|---|---|
| **Scenario** | `s0-field-standing-wave` (or `flux-pulse`, genesis off), L = 64, `conservative_config()` |
| **Conserved quantity** | `E_H = EFieldEnergy + BFieldEnergy` (= `½∑(E²+B²)`, the Maxwell/wave Hamiltonian). **Not** `fieldEnergy+waveEnergy` (that is `½|J|²+½|wave_vel|²`, which is not conserved — it sloshes; this was v1's error). |
| **Readout** | `bridge.getEnergyAudit().EFieldEnergy + .BFieldEnergy`, sampled every 40 ticks over 200 ticks |
| **Prediction** | `E_H` constant: **endpoint drift AND peak-to-peak < 0.5%** over 200 ticks |
| **Falsifier** | drift or peak-to-peak **> 0.5%** with `conservative_config()` verified (console-gate passed) |
| **Tier** | [CORE-FIRM] |
| **Sanity cross-check** (record, not a verdict) | `fieldEnergy+waveEnergy` is *expected* to oscillate (it is not the Hamiltonian) — confirms the v1 metric was the defect, not the engine. |

---

## §R3 — I1 signal speed + I6 locality (REVISED, LOCKED)

The wave front, not the absolute extent. Two sub-tests, both from a point pulse at center = `L/2`
(L=64 ⇒ center 32; **v1 used 16 — the artifact**), genesis OFF, `conservative_config()` (damping
state does not affect front *speed*).

**Injection:** a **minimal-width** flux pulse (σ ≤ 1.5 voxels) at center — inject via the
substrate-controls "Inject Flux" with the width set to minimum, or a `bridge` inject call (resolve at
run-time), so there is no large initial extent confounding the front. Record σ_init used.

| Sub-test | Spec |
|---|---|
| **I1 front speed** | Define `R(t)` = outermost radius from center with `|J| > 0.1·peak|J|(t)` (adaptive front). Measure `R(t₁)`, `R(t₂)` at `t₁=10`, `t₂=30`. **Predict** `dR/dt = (R(t₂)−R(t₁))/(t₂−t₁) = c_lat = 1/√3 ≈ 0.577`. **Falsify** if outside **±10%** (0.519–0.635) after the front has formed. [CORE-FIRM] |
| **I6 locality (binary)** | After `T` ticks, **no** `|J| > noise` beyond radius `σ_init + T` voxels (strict ≤1-voxel/tick causality). **Falsify** if any flux appears beyond `σ_init + T`. [CORE-FIRM] — the cleanest binary test; supersedes the v1 max-radius probe. |

*Fallback if minimal-width injection is unavailable:* use `flux-pulse` (σ≈N/10) and measure `dR/dt`
between `t₁=8`, `t₂=24` (the initial extent cancels in the difference); the I6 bound uses the measured
initial radius as `σ_init`.

---

## §R4 — Genesis cluster: test the SCALING, not the absolute size (REVISED, LOCKED)

Run-1 showed the absolute `~25` (a `[EMPIRICAL]` figure) did not reproduce on the web build (got 4).
The **firm, calibration-independent** prediction is the *scaling exponent* (from O_h rep theory,
FTD-0110), not the coefficient. v2 tests the scaling directly.

| Sub-test | Spec | Tier |
|---|---|---|
| **G1 — scaling exponent** | Point flux injection at center, `genesis_config()`, Langevin T=0, L=64. Sweep amplitude `A/K_GENESIS ∈ {2,4,6,8,10,14,20}` (inject programmatically per A). Step to stable `N(A)` (stable = unchanged over 40 ticks). Fit `log N` vs `log A`. **Predict slope = 2.0**. **Falsify** if slope outside **2.0 ± 0.3**. | [CORE-FIRM (linear-regime, FTD-0110)] |
| **G2 — coefficient** | From G1, `k = N/(A/K_GENESIS)²`. **Predict ≈ ¼** but tagged `[EMPIRICAL]` — record value; a deviation is a *build-reconciliation* item (web-WASM vs C++/CUDA campaign), **not** a falsification. | [EMPIRICAL] |
| **G3 — L-invariance** | Same A (e.g. 10·K_GENESIS) at L=32 and L=64. **Predict** stable `N` equal within **±20%** (size is intensive, not ∝ L³). **Falsify** if `N(L=64)/N(L=32)` tracks the volume ratio (8×). | [CORE-STRUCTURAL] |
| **G4 — cluster count** | ic1 → 1 cluster, ic3 → 2, ic4 → 0. Uses connected-components counting over `getParticleData().positions` with Moore-neighborhood adjacency. **Falsify** if ic1 is not one component, ic3 is not two components, or ic4 manifests any component. | [CORE-STRUCTURAL] |
| **ic4 null** (carried from v1, CONFIRMED) | A=0.5·K_GENESIS → manifested = 0. | [CORE-STRUCTURAL] |

---

## §R5 — I5 determinism (REVISED, LOCKED)

| Field | Spec |
|---|---|
| **Method** | Load scenario `S` (e.g. `s0-seed-emergent-ic1`, fixed seed), L=64, paused; step 120 ticks recording the trajectory of `{manifested, totalFlux, EFieldEnergy, BFieldEnergy}` at ticks {0,40,80,120}. **Reload** `S` fresh; repeat. |
| **Prediction** | The two trajectories are **bit-exact identical** (WASM core is deterministic). |
| **Falsifier** | any checkpoint differs beyond float epsilon (`>1e-12` relative) on the WASM backend |
| **Tier** | [CORE-FIRM]. (MockBridge is excluded — F5.) |

---

## §R6 — Carried forward from LOCKED v1, unchanged

- **I3 — Gauss constraint** (CONFIRMED Run-1: `maxGaussError ≈ 1.9e-16`): unchanged. [CORE-FIRM]
- **ic4 — sub-threshold null** (CONFIRMED Run-1: 0 manifested): unchanged (also in §R4). [CORE-STRUCTURAL]
- **I4 — charge conservation**: implemented in the v2 Playwright protocol as conservative stepping
  of `s0-vacuum-electron`; `chargeBalance` and manifested count must remain exactly fixed.
- **§4.2 wave linearity/superposition**, **§4.3 Phase-G Coulomb**: as in v1 (the Phase-G clean
  measurement still needs the static-pair/campaign probe, per v1).
- **§5 Illustrative catalog**, **§6 falsifier rules/banned moves**, **§7 scoring**, **§0/§1/§2
  framework**: inherited verbatim from the locked v1 (commit `e8ce032b`).

---

## §R7 — Hash-lock protocol (to run when the owner says "lock and run")

Same as v1 §8: finalise → `sha256sum` this file → record SHA + tag + the (then-confirmed) FTD-id in
`../REF_PREREGISTER_MANIFEST.md` + LEDGER → `git tag preregister-scale0-substrate-protocol-v2` →
run **only** against the tagged commit → verdicts into `AUDIT_SCALE0_SUBSTRATE_RESULTS.md` (Run 2
section), never as edits here. v2 supersedes v1's I1/I2/ic1/I5 rows; v1 stays locked for provenance.
Proposed registry id remains in the contended range (confirm next-free at lock; v1's proposed FTD-0247
is not yet registered).

---

## §R8 — One-line summary

v2 repairs the four things Run 1 exposed — the `selective_damping` config trap, the mis-specified
energy metric (`½|J|²` → `½(E²+B²)`), the under-instrumented `c_lat` test (absolute radius → `Δr/Δt`
+ strict locality), and the build-dependent cluster size (absolute ~25 → the calibration-independent
**scaling exponent 2**) — and carries forward the two clean Run-1 confirmations (Gauss, sub-threshold
null). **Draft; awaiting "lock and run."**
