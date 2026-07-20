# PREREG — Genesis's Operating Point, Take 2: does it depend on the circumstances of birth?

**Tag:** [PRE-REGISTRATION — ENGINE MEASUREMENT] (LOCK-STD v1; git tag `preregister-manifestation-seed-diversity-v1` at the registration commit)
**Parent finding:** `PREREG_GENESIS_TIMING_DEPENDENCE_v1` VOIDED its own T≥1 arms — forcing a delay past a threshold-crossing site's natural window doesn't test "late manifestation," because the field *sloshes* and the site drops back below `K_GENESIS` within a single further tick (a real, informative finding: the eligible window is narrow, consistent with genesis's observed ~93%/tick firing probability from a comfortably-eligible seed). **This document asks the same underlying question — is the locked energy a stable, characteristic property of manifestation, or does it depend on the specific circumstances of birth — without depending on sustaining threshold-eligibility at all.**

## 1 · Design: independently constructed births, each firing naturally

No forced delay, no genesis-off intervention, no manual state flip. **Five genuinely different seeds**, each run through *unmodified*, fully natural dynamics (`wave_propagation, coupling, gauss_projection, genesis, damping, selective_damping` ON — identical toggle set to every prior campaign in this line) until manifestation fires on its own, at whatever tick the real stochastic hazard selects — exactly mirroring `PREREG_GENESIS_ENERGY_LEDGER_v1`'s original G-early protocol, repeated across diverse circumstances instead of one fixed seed:

| Seed | Position offset from (8,8,8) | Peak amplitude | σ | What it varies |
|---|---|---|---|---|
| **A** (baseline) | (0.31, 0.17, 0.07) | 3.0 | 0.45 | — (reproduces `PREREG_GENESIS_ENERGY_LEDGER_v1`'s known G-early exactly: 1.709171333089) |
| **B** (position) | (−3.21, 4.13, −2.09) — a different lattice region | 3.0 | 0.45 | location |
| **C** (hot) | (0.31, 0.17, 0.07) | 5.0 | 0.45 | amplitude, larger margin above threshold |
| **D** (broad) | (0.31, 0.17, 0.07) | 3.0 | 0.75 | spatial width of the birth pulse |
| **E** (cold/marginal) | (0.31, 0.17, 0.07) | 2.15 | 0.45 | amplitude, *small* margin (+0.044 over K_GENESIS, p≈8.3%/tick, mean wait ≈12 ticks by the memoryless hazard law) — genuinely uncertain natural firing tick, the one seed in this set expected to test real, unforced timing variation. (An initial choice of 1.85 was checked arithmetically before locking and found sub-threshold — peak 1.342 < K_GENESIS — and corrected; this is instrument-parameter verification, not a run, and licensed by the same determinism-based discipline as every prior campaign today.) |

Each seed: build a fresh bridge, inject, run ticks (cap 200) until exactly one voxel manifests, freeze, relax to the Gauss fixed point (residual-1e-8, cap 5000 — identical protocol throughout this line), measure `e_half`.

## 2 · Validity gates (per seed — a failure VOIDs that seed's point, not the whole campaign)

- **V1:** exactly one lattice site exceeds `K_GENESIS` immediately after injection (single-site isolation, as every prior seed in this line required).
- **V2:** exactly one voxel is manifested at the moment freezing occurs (no cascade).
- **V3:** the relaxation converges (residual gate met, not cap-exhausted).
- **V4 (Seed A only):** reproduces `1.709171333089` exactly — confirms this campaign's harness matches the original instrument bit-for-bit.

## 3 · Frozen reading (stated before running)

Across whichever seeds pass all gates, compute `CV = std(e_half) / mean(e_half)`.

| Band | Reading |
|---|---|
| `CV < 0.10` | Genesis's locked energy is **stable across circumstance** — position, amplitude, and width within the tested ranges do not materially change the operating point. Strong support for treating the mass excess as a genuine, characteristic (if not yet derived) property of manifestation. |
| `CV ≥ 0.40`, or values differing by more than 2× | **Circumstance-dependent** — the birth event's specific parameters materially set the locked energy. A significant finding: "the" mass excess is not a single number without specifying how the particle was made. |
| Between | Report the actual spread, and specifically whether Seed E (the low-margin, genuinely-uncertain-timing case) is an outlier relative to A–D — isolating whether *timing* specifically (as opposed to position/amplitude/width) is the sensitive parameter. |

`fire_tick` is recorded for every seed; Seed E's is genuinely not known in advance (this is the one point in the whole day's line of inquiry where the *actual* firing tick, not just the outcome, is unpredicted before running).

---

*Registered 2026-07-19, before the instrument's first execution. Author: session 8294fddb, following LOCK-STD v1. Companion/parent: `preregister-genesis-timing-dependence-v1`, `preregister-perturbation-magnitude-curl-sweep-v1`, `preregister-kinetic-drain-curl-isolation-v1`, `preregister-genesis-energy-ledger-v1`.*
