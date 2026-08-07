# PREREG — Does Injected Curl Scale With Perturbation Size, or Is It a Symmetry-Breaking Floor?

**Tag:** [PRE-REGISTRATION — ENGINE MEASUREMENT] (LOCK-STD v1; git tag `preregister-perturbation-magnitude-curl-sweep-v1` at the registration commit)
**Parent finding:** `PREREG_KINETIC_DRAIN_CURL_ISOLATION_v1` (Outcome REFUTED, 2026-07-19): the kinetic-drain operation is not the source of the transverse contamination found by `PREREG_GENESIS_ENERGY_LEDGER_v1` — an unconfounded isolated test showed a *smaller* localized wave_vel perturbation (drain, scale 0.5×) produces *less* curl (12.653) than a *larger* one (no-drain, scale 1.0×, 19.233), the opposite of "drain injects curl." That comparison — two points, both far from zero — could not distinguish two different readings: (a) curl grows smoothly with the perturbation's magnitude, so *any* nonzero single-site perturbation injects *some* curl proportionally, or (b) curl is closer to a floor/threshold effect set by merely breaking symmetry at one site, with magnitude a secondary factor. **This document tests which.**

## 1 · A structural prediction, stated before running (not a physics guess — a consequence of the operations' own linearity)

The isolated single-leapfrog-step construction (identical to the parent's Test B) applies exactly one operation whose s-dependence is *analytically fixed*: `flux[target] ← flux_old[target] + s·wave_vel_original[target]` (the raw leapfrog step, affine in the scaling factor `s`), while every other voxel's flux is untouched by `s` (the leapfrog is per-voxel; no cross-site coupling in this single step). `curl_total = Σ_lattice |∇×J|²` is a quadratic functional of J. A quadratic functional of an affine function of `s` is **exactly a quadratic polynomial in `s`**: `curl_total(s) = A + B·s + C·s²`, with no higher-order terms possible by construction — this is a mathematical consequence of the two facts just stated, not an empirical hypothesis to be "confirmed" so much as a **structural check on the instrument**: if the sweep does *not* fit a quadratic cleanly, something about the isolation (cross-site leakage, a non-affine step I haven't accounted for) is broken, and the result should be treated as VOID pending investigation, not interpreted physically.

**The physics question lives in the fit's coefficients, not in whether it's quadratic:**
- `A = curl_total(0)` — the curl present when the target's wave_vel is *forced to zero* (removing whatever F_pre's own dynamics put there) plus the ordinary baseline. A large `A` relative to `curl_total(1)` (the parent's already-measured no-drain point, 19.233) would support the *floor/symmetry-breaking* reading: even "erasing" content at one site, not adding anything, injects comparable curl.
- `B, C` — how much *additional* curl accrues per unit of perturbation magnitude beyond that floor. Large `B`/`C` relative to `A` would support the *scales-with-magnitude* reading.

## 2 · Design

`engine/tests/campaign_perturbation_magnitude_curl_sweep.cpp`, locked at the registration commit. Reuses the identical seed, Phase-A prefix (1 normal tick, then 1 genesis-OFF tick — the v1.1-corrected convention from the parent), and F_pre snapshot construction as `campaign_kinetic_drain_curl_isolation.cpp` (same deterministic RNG stream — F_pre will be bit-identical to the parent's, verified as a validity gate).

For each `s ∈ {0, 0.25, 0.5, 0.75, 1.0, 1.25, 1.5, 2.0}`: clone F_pre onto a fresh bridge, `set_state(+1)` at the target site, scale wave_vel at the target site by `s` (multiplicative on F_pre's own value there — `s=1` exactly reproduces the parent's Test-B no-drain point, `s=0.5` exactly reproduces its drain point), one raw leapfrog integration (`disable_all()` + one `tick()`, identical mechanism to the parent), measure `curl_total`.

**Control arm C_null:** clone F_pre, apply *neither* the state flip *nor* any wave_vel change, one raw leapfrog step, measure `curl_total`. Answers: does the ordinary continuation of dynamics (one more ordinary step, no manifestation at all) change curl from F_pre's own baseline (7.065424291986, already measured)?

## 3 · Validity gates

- **V1 (instrument continuity):** F_pre's `e_half` and `curl_total` in this campaign must exactly match the parent's disclosed values (3.129589867365, 7.065424291986) — confirms the shared prefix is bit-identical, as determinism requires.
- **V2 (structural check, §1):** a least-squares quadratic fit to the 8 sweep points must have R² ≥ 0.999. Below this, the sweep is VOID — not physically interpreted — pending investigation of what broke the affine-step assumption.
- **V3:** `s=0.5` and `s=1.0` sweep points must match the parent's already-measured Test-B values (12.653, 19.233) within floating-point round-trip tolerance (identical computation, so this should be exact or near-exact) — a second independent reproducibility check.

## 4 · Reading the result (no pass/fail — a measured characterization)

This is a characterization campaign, not a hypothesis test with a binary verdict: report `A`, `B`, `C`, and the ratio `A / curl_total(1)` (what fraction of the full-perturbation curl is already present at zero perturbation). A ratio near 1 supports the floor/symmetry-breaking reading named in the parent document; a ratio near 0 supports the scales-with-magnitude reading; anything between is reported as such. `C_null` vs. `curl_total(0)` isolates whether the state flip *itself* (with the target's wave_vel merely zeroed, not left alone) contributes anything beyond ordinary continuation.

---

## OUTCOME (2026-07-19, first execution — no VOID this time)

Data: `engine/build/perturbation_sweep_v1/run.csv`.

**All gates pass, several exactly.** V1: `F_pre`'s `e_half` and `curl_total` match the parent campaign's disclosed values to the printed digit (bit-identical prefix, as determinism requires). V3: the `s=0.5` and `s=1.0` sweep points reproduce the parent's Test-B drain/no-drain values **exactly** (`12.653278…` and `19.233461…`, identical to all 12 printed digits). A free bonus check: `C_null` (no state flip, wave_vel untouched) is **numerically identical** to `s=1.0` (wave_vel scaled by exactly 1, i.e. also untouched) — direct empirical confirmation that `set_state` truly has zero effect on this measurement, exactly as the source-code verification predicted.

**V2 (the structural check) passes at the limit of the check's own resolution:** a least-squares quadratic fit gives R² = **1.0000000000**, with residuals at 10⁻¹²–10⁻¹¹ — pure floating-point noise, not a fit residual. A cubic fit's cubic coefficient is 3.6×10⁻¹² — indistinguishable from zero. `curl_total(s)` is quadratic in `s` **to the limit of double precision**, exactly as the affine-leapfrog / quadratic-curl-functional argument in §1 required. This is a strong, independent confirmation that the isolation methodology (this campaign's and the parent's) has no hidden cross-site leakage or unaccounted-for confound — a bug of that kind would show up as departure from exact quadratic behavior, and none appears.

**The physics — in the fitted coefficients:**

`curl_total(s) = 7.706578 + 8.259921·s + 3.266963·s²`

- **A = 7.706578** — the floor: curl present even at `s=0` (target's wave_vel forced to zero, nothing added). This is **40.07%** of the full-perturbation value `curl_total(1) = 19.233462`.
- **B, C > 0**, and the parabola's vertex sits at `s = −B/2C ≈ −1.264` — *outside* the physical range `s ≥ 0`. Consequence: for every physically realizable perturbation magnitude, `curl_total` is **strictly monotonically increasing** in `s`. There is no magnitude at which adding more perturbation reduces curl.

**Reading (per prereg §4 — a characterization, not a binary verdict):** neither pole of the parent's named hypothesis is exactly right. It is **not** a pure floor (60% of the signal scales with magnitude, and does so monotonically and convexly — increasingly steeply — for all `s > 0`) — but it is **not** pure magnitude-scaling either (40% survives at zero added perturbation, from symmetry-breaking alone: merely erasing what one site naturally held). The mechanism is a **mixture, now quantified exactly**: roughly two-fifths structural (breaking symmetry at a single site, independent of what if anything is placed there) and three-fifths proportional (with an accelerating, not merely linear, dependence on magnitude).

**One further fact the exact fit hands us for free:** since the vertex sits at negative `s`, a perturbation applied in the *opposite* direction from what natural dynamics produced (`s ≈ −1.26`, i.e. roughly reversing and slightly overshooting F_pre's own wave_vel there) would minimize curl below even the `s=0` floor. Named as a mathematical consequence of the fit, not measured or claimed as physically meaningful — a natural probe for a future campaign if the sign-dependence of the contamination becomes relevant.

**Consequence for the parent line of inquiry:** `DERIV_REST_MASS_FROM_CONSTRAINT_ENERGY.md`'s [OPEN] "sharper hypothesis" line is updated from a named-but-untested question to a **quantified characterization**: single-site symmetry-breaking accounts for ~40% of the injected transverse content at this test point; the remaining ~60% scales with perturbation magnitude, convexly. The underlying question — what in genesis's *own* mechanics sets the effective magnitude and direction of the site-local perturbation it produces — remains open; this campaign characterizes the *response curve*, not genesis's specific operating point on it (though `s=1.0`, the no-drain point, *is* the closest proxy to genesis's actual undrained mechanics measured so far).

---

*Registered 2026-07-19, before the instrument's first execution. Author: session 8294fddb, following LOCK-STD v1. Companion/parent: `preregister-kinetic-drain-curl-isolation-v1`, `preregister-genesis-energy-ledger-v1`.*
