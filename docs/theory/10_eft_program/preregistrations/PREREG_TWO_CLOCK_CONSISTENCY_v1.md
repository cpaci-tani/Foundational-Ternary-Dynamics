# PREREG — The Two-Clock Consistency Test: does the substrate's decay clock dilate?

**Tag:** [PRE-REGISTRATION — ENGINE MEASUREMENT] (LOCK-STD v1; git tag `preregister-two-clock-consistency-v1` at the registration commit)
**Engine state at lock:** post Term-2 electric-coupling amendment (2026-07-18) and post-FTD-0388 kinetics (K_MANIFEST = W_SC = 0.5054620197173260, K_GENESIS = 3·W_SC). Instrument born after both.

---

## 1 · The question

FTD's gravity sector of record is **chronometric**: latency `L` consumes each voxel's per-tick budget (`v²/C² + L² ≤ 1`), local bandwidth is `C·√(1−L²)`, and proper time runs slow where `L` is high — the sector that produced the 0.004 % time-dilation match and the blind L=257 confirmation (FTD-0268).

Separately, the substrate carries a **decay clock**: every manifested, unlocked voxel evaporates each tick with hazard

    p = K_EVAP_RATE · exp( −E_local / K_MANIFEST² ),   K_EVAP_RATE = 0.1

`E_local` = (|J|² + |wave_vel|²) summed over the site and its 6 face neighbours (`phase_write.cpp`).

In physics, decay statistics **are** clocks — muon-lifetime dilation is among the most precisely confirmed predictions of relativity. So FTD's two clocks must agree: **a metastable configuration deeper in a latency well must decay slower, per tick, by exactly the proper-time factor `√(1−L²)`.**

**Code-derived expectation, declared as such [not a measurement]:** the hazard expression contains no latency factor, so the expectation is NO dilation — the decay clock runs on tick-time while the motion/proper-time sector runs on `τ`. The measurement exists because code-derived expectations are not measurements: this week the same engine was found enforcing its central constraint at −9.5 % where every prior reading assumed ~100 %, and four campaign cycles launched "moving" particles that were at rest.

## 2 · Instrument

`engine/tests/campaign_two_clock_consistency.cpp`, locked at the registration commit. CPU-forced, deterministic, emits CSV only; the verdict is applied afterward against §4.

**Frozen-field design (makes the hazard exactly computable).** Toggles on a fresh `RenderBridge` after `disable_all()`: **ON** — `evaporation`, `forces`, `gravity`, `latency_field`. **OFF** — everything else, in particular `wave_propagation`, `coupling`, `gauss_projection`, `damping`, `selective_damping`, `movement`, `genesis`. Consequences, all deliberate:
- With wave + coupling + Gauss off and `wave_vel ≡ 0`, the flux field is exactly what the instrument injects and never evolves ⇒ `E_local` is **constant per voxel for the whole run** ⇒ the per-tick hazard is an exact constant.
- With `movement` off, test voxels cannot fall into the well ⇒ their latency is constant.
- With `genesis` off and `evaporation` on (the toggle exists for exactly this isolation), no new matter appears to contaminate the counts.
- The massive body therefore contributes **only latency** — no flux, no forces on the frozen field. `latency_field`'s Poisson solve is independent of the wave sector (`render_bridge.cpp`: gated solely on `toggles.latency_field`).

**Configuration.** L = 96. Locked, charge-neutral (alternating ±1 by site parity) ball of radius 3 at the lattice centre. Equilibration 200 ticks (latency SOR warm-start) **before** test voxels exist. Test voxels: unlocked `+1` states on spherical shells at radii **r ∈ {5, 7, 10, 14, 20, 30}** from the ball centre, laid out by spherical-Fibonacci and rounded to lattice sites, with a **minimum separation of 2** lattice units (so no test voxel enters another's `E_local` neighbourhood) and none inside the ball. Each test voxel is dressed at its **own site only** with |J| = √0.588 = 0.7668 ⇒ `E_local = 0.588` exactly and identically for every test voxel, giving a uniform predicted hazard

    p_pred = 0.1 · exp(−0.588 / 0.2554918…) = 1.0 × 10⁻² per tick   (mean lifetime ≈ 100 ticks)

for **every** test voxel regardless of shell. After injection, 3 ticks let the latency solve re-converge; the surviving population at that point is the cohort of record, and its per-voxel `L` and `E_local` are snapshotted. Then 400 ticks are run, recording survivors per shell per tick.

**Paired arms (identical voxel indices, identical dressing, identical RNG streams):**
| Arm | Configuration |
|---|---|
| **M** | mass ball present ⇒ latency well |
| **F** | no mass ⇒ latency ≡ 0 everywhere (flat control) |

The engine's per-voxel RNG is indexed by `(seed, voxel_index, tick, stream)`, so the two arms draw **the same numbers for the same voxel at the same tick**. This makes the comparison bit-level, not merely statistical.

## 3 · The two frozen predictions

For each shell, with `L̄` the cohort-mean latency in arm M:

- **Prediction A (latency-blind decay clock):** decay is identical in both arms — `rate_M/rate_F = 1` for every shell, and, because the RNG draws coincide, **every test voxel decays at the identical tick in both arms** (bit-level identity, `n_diff = 0`).
- **Prediction B (τ-dilated decay clock):** `rate_M/rate_F = √(1 − L̄²)` per shell, monotone in depth, with `n_diff` large.

## 4 · Pre-declared outcomes

| Outcome | Condition | Consequence |
|---|---|---|
| **A — clocks disagree** | `n_diff = 0` (or `\|rate_M/rate_F − 1\| < 0.03` in every shell) **and** the deepest shell has `√(1−L̄²) ≤ 0.93` — i.e. a ≥7 % dilation would have been resolvable and was absent | **[MEASURED — internal inconsistency of the time sector]**: the substrate's matter ages on tick-time while its motion/proper-time sector runs on `τ`. Books a decision point for the owner (not an automatic bug-fix): either the hazard must carry the `τ` factor — sharpening the clock hypothesis from "latency-budget = clock rate" to the testable "**hazard rates integrate proper time, not tick time**" — or the engine declares decay to be tick-native, which then contradicts the muon-lifetime class of evidence and must be booked as a boundary. **No tag moves without the owner's ruling.** |
| **B — clocks agree** | `rate_M/rate_F` matches `√(1−L̄²)` within ±15 % in every shell with `L̄ > 0.1`, monotone in depth | The chronometric picture is self-consistent across the matter sector — decay clocks and proper-time clocks are one clock. A genuine positive; the code reading was wrong (a latency dependence enters by a route not visible in the hazard expression) and the mechanism must be located before anything is claimed. |
| **Indeterminate** | Gates fail (below), or the arms differ in a way matching neither prediction | Characterize; re-register v2. Never laundered into A. |

**Validity gates (failure ⇒ VOID, not an outcome):**
- **V1 — the well exists:** cohort-mean `L̄` in arm M exceeds 0.25 in the innermost shell and is < 0.02 in arm F everywhere.
- **V2 — the field is frozen:** max |ΔJ| over any test voxel between cohort snapshot and end of run < 10⁻¹² (verifies the frozen-field construction; a drifting field would change `E_local` and invalidate the exact hazard).
- **V3 — statistics:** ≥ 40 surviving cohort voxels per shell at snapshot, and ≥ 60 % of each shell's cohort has decayed by the end of the run (so the rate fit is not extrapolation).
- **V4 — hazard uniformity:** every cohort voxel's snapshot `E_local` equals 0.588 to within 10⁻⁹ (the design's uniform-hazard premise, verified rather than assumed).

## 5 · Anti-gaming

- Shells, radii, dressing, tick counts, and the hazard target are fixed above; no post-hoc parameter search.
- The code-derived expectation in §1 does not soften §4: a result contradicting the code reading is booked as B and the mechanism hunted.
- Outcome A is **not** a licence to "fix" the engine in the same session. It books a decision point; the fix (if any) is a separate, separately-verified change with its own golden-pin consequences.
- No claim about real-world decay physics follows from either outcome — this is a statement about FTD's internal consistency only.

---

*Registered 2026-07-18, before the instrument's first execution. Author: session 8294fddb, following LOCK-STD v1.*
