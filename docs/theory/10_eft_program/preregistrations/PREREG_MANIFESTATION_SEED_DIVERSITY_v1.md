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

## OUTCOME (2026-07-19) — **CIRCUMSTANCE-DEPENDENT**, decisively, and the pattern is monotonic

Data: `engine/build/seed_diversity_v1/run.csv`. V4 confirms Seed A reproduces the known baseline exactly (`1.709171333089`).

**Three seeds pass every gate cleanly:**

| Seed | fire_tick | `e_half` (relaxed) |
|---|---|---|
| E_cold (amp 2.15, marginal) | 2 | **0.781682031101** |
| A_baseline (amp 3.00) | 2 | **1.709171333089** |
| C_hot (amp 5.00) | 2 | **7.221033315847** |

`CV = std/mean = 0.878` — more than double the `≥0.40` circumstance-dependence threshold. **Max/min ratio = 9.24×.** This is not a borderline call.

**The spread is not noise — it is monotonic in injection energy.** Cold < baseline < hot, in that order, by a wide margin at every step (A/E = 2.19×, C/A = 4.23×). The locked energy scales with how much energy was available at birth, not with some fixed "cost of becoming a particle." This is physically legible even before any deeper mechanism is derived: a hotter local field has more content available to be shaped into the transverse (non-minimal) configuration the Gauss projector cannot remove.

**Two seeds did not produce clean data, both informative in their own right:**
- **B_position: VOID — no manifestation within the 200-tick cap**, despite passing V1 (exactly one eligible site, same margin as A). At the *same* margin that fires A within 2 ticks with ~93% per-tick probability, B's target site apparently never sustains eligibility long enough to fire, or fires-window draws happened not to trigger across the whole cap. Consistent with `PREREG_GENESIS_TIMING_DEPENDENCE_v1`'s finding that the eligible window can be short and position/boundary-sensitive — evidence that *where* a birth happens, not just its energy, can qualitatively change whether (and when) it happens at all, a further axis of circumstance-dependence beyond the energy-scaling found above.
- **D_broad: fails its own V1 gate** (2 sites eligible pre-seed, not the required 1) — a real single-manifestation cascade risk that happened not to materialize this run (`V2` confirms exactly 1 manifested). Its `e_half=3.751513` sits between E and C, consistent with the monotonic pattern, but is **not included in the CV calculation** — its own stated validity condition was not met, and per this line of inquiry's standing discipline, a violated gate voids the point regardless of how "reasonable" the resulting number looks.

**Verdict: CIRCUMSTANCE-DEPENDENT.** Genesis does not have a single, stable operating point on the curl-response curve characterized by `PREREG_PERTURBATION_MAGNITUDE_CURL_SWEEP_v1`. The locked energy — and hence, under the (already-refuted-as-unconditional) constraint-locked-energy proposal, the "rest mass" a manifestation event would carry — depends materially on the birth's energy budget, and possibly on its location. **"The" mass excess is not a single number without specifying how the particle was made.**

**Consequence, stated plainly:** this closes the line of inquiry opened by `DERIV_REST_MASS_FROM_CONSTRAINT_ENERGY.md` more thoroughly than a mechanism-refutation alone would have. Even setting aside *why* real dynamics inject transverse content (kinetic drain: refuted; single-site symmetry-breaking: quantified but only 40% of the story), the *amount* injected is not fixed — it scales with circumstance. A derivation of rest mass along this route would need to explain not only the injection mechanism but why real particles (which the framework wants to have one well-defined mass each) would always land at the *same* point on a response surface this campaign has now shown to vary by an order of magnitude with birth energy alone.

---

*Registered 2026-07-19, before the instrument's first execution. Author: session 8294fddb, following LOCK-STD v1. Companion/parent: `preregister-genesis-timing-dependence-v1`, `preregister-perturbation-magnitude-curl-sweep-v1`, `preregister-kinetic-drain-curl-isolation-v1`, `preregister-genesis-energy-ledger-v1`.*
