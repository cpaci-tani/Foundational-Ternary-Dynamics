# AUDIT — Scale-0 Scenario Health & Physics-Sense (2026-06-05)

**Status:** complete. Two dimensions: **(A) mechanical health** — does every
scenario mount and do its telemetries work? — and **(B) physics-sense** — do the
scenarios actually represent their named physics *on the lattice substrate*?

**Scope:** all **97** Scale-0 registry scenarios
(`engine/web/js/scales/scale0/scenario-registry.js`).

**Method — verify, don't infer.** Dimension A is measured by an automated
Playwright sweep that loads every scenario and reads real state
(`engine/web/tests/scale0-scenario-health.spec.js`). Dimension B is grounded in
reading every flagged scenario's implementation body (JS source of record + C++
mirror for the deployed WASM path) — verdicts cite `file:line`, not names.

**Bridge-ownership fact that bounds severity.** The deployed dashboard runs the
**WASM bridge → C++ scenarios** (`engine/src/scenarios/*.cpp`) for everything
*except* `flux-*`, which always runs on the JS flux mock / worker
(`shouldUseFluxMock`, `scenario-loader.js:70-78`: `flux-*` → mock; all others →
WASM when the WASM bridge has a flux volume, mock only as fallback). JS↔C++
parity is guarded (97 = 97 registry/JS, 96 C++ + 1 legacy). So a **JS-only** bug
is latent (fallback path only); a defect in **both** JS and C++ ships.

**Companions:** [`AUDIT_SCALE0_SCENARIO_LIFECYCLE_2026-06-05.md`](AUDIT_SCALE0_SCENARIO_LIFECYCLE_2026-06-05.md),
[`../SPEC_SCALE0_SCENARIO_ARCHITECTURE.md`](../SPEC_SCALE0_SCENARIO_ARCHITECTURE.md),
[`AUDIT_BRIDGE_WIRING_2026-06-03.md`](AUDIT_BRIDGE_WIRING_2026-06-03.md).

> **Label convention.** This audit uses *health/UX* tags — `[MISLABEL]`,
> `[VISUALISATION]`, `[FIELD ANSATZ]`, `[SETUP-ONLY]`, `[GEOMETRIC SEED]` — which
> are **distinct from the project's claim-status tags** ([THEOREM]/[CONJECTURE]/…).
> `[VISUALISATION]` here is *not* a demotion of a physics claim; it describes what
> the scenario depicts on the lattice.

---

## 0. Executive summary

| | Result |
|---|---|
| Scenarios audited | **97** at audit time → **96** after retiring `frw-patch` (see §B.2) |
| Mount + telemetry | **97 / 97** (after fixing **1** broken scenario) |
| Broken, found + **FIXED** | `flux-annihilation` (worker create crash → mounted nothing) — `7fc4296b` |
| Latent bug, **FIXED** | `vacuum-scenarios.js` called an undefined `harness` → every `s0-vacuum-*` threw on the **MockBridge fallback** path; signature realigned + mock-path test added (2026-06-05) |
| Dead code (open) | `_quantumZenoMode` / `_quantumExperimentMode` set, never read |
| Telemetry defect (open) | WASM `totalEnergy` is a `K_B·N³` rest baseline (~18363.8) that swamps scenario energy; mock omits it — WASM energy readout is non-informative |
| Physics-sense → action | retired the cosmology scenario (`frw-patch`); kept the gravity ones (owner decision). Substrate-native categories are **genuine and well-tagged** (`flux-zero-point` + the FTD-0107 `ic*` set are the honesty gold standard); the named quantum *effects* still implement setup geometry without the effect's readout. |

**Two headlines.** (1) The scenarios are mechanically healthy — only one was
actually broken, now fixed and guarded. (2) Where scenarios stopped "making sense on
the lattice" was **cosmology** — Scale 4–5 physics a 33³ substrate cannot carry;
`frw-patch` (which didn't even implement FRW cosmology) was retired, the gravity
visualizations kept.

---

## A. Mechanical health (mounting + telemetry)

`scale0-scenario-health.spec.js` loads each scenario on its natural owner and
records **mounted** (peak `|J|` over the flux volume **or** E/B field-sample count
**or** particle count — deliberately **not** `totalEnergy`, see A2), **telemetryOK**
(finite diagnostics), and **clean** (no real console error). `empty`,
`emergent-ic4-subthreshold`, and `emergent-ic2-thermal-runaway` are allowlisted
(intentionally empty at load). **Result: 97/97** mount with valid telemetry and no
console errors (post-fix). Permanent regression guard.

### A.1 `flux-annihilation` mounted nothing — worker create crash · **[FIXED]**

Found mounting nothing + throwing `[Scale0 worker] create Cannot read properties
of null (reading 'ctrl')`, reproducible even loaded first on a fresh page (a real
scenario bug, not a race). **Root cause:** its flux-"kick" loop iterates the
centre (`midF±3`, `flux-scenarios.js:117-118`) but evaluates Gaussians centred on
the particles `pL=5`/`pR=27` (~11 voxels away, `:121-125`) → every value `≈7e-4`,
below the `>0.001` guard → **no flux injected**, only 4 particles → `_initFluxGrid`
never runs → the worker's SAB (`_sharedField`) is never allocated → `publishShared`
crashes on `getSharedField().ctrl` (`mock-bridge.worker.js:30-31`). **Fix**
(`applyInit`): force `_initFluxGrid()` if `getSharedField()` is still null, so any
particle-only / no-flux scenario mounts on the worker path. Committed `7fc4296b`.

### A.2 WASM `totalEnergy` telemetry is a rest-mass baseline · **[OPEN]**

Every WASM scenario incl. **`empty`** reports `totalEnergy = 18363.807` =
**exactly `K_B·N³`** (`0.511 × 33³`): a per-voxel rest-energy baseline that
**swamps** the scenario field energy (~0.1–15; only the last digits move). The
**mock** omits it and reads correctly (`flux-pulse`=95.8, `flux-soliton`=3510).
The WASM energy readout (status bar, charts) is therefore non-informative, and the
bridges are inconsistent. Particle counts + E/B field telemetry *are*
scenario-responsive on WASM; only energy is baseline-dominated.
**Recommendation:** reconcile the energy convention (separate rest baseline from
field energy, or apply it on both bridges). Cross-ref `CONTRACTS.md` (energy).

### A.3 `flux-annihilation`'s flux kicks are dead · **[OPEN, secondary]**

Same centre-vs-particle mismatch (A1): the "dramatic head-on collision" kicks
(`flux-scenarios.js:115`) never fire on **any** backend — it's 4 static particles.
The worker fix makes it *mount*; it does not make the kicks work. Fix = position
the kick loop around the particles.

### A.4 `vacuum-scenarios.js` references an undefined `harness` · **[FIXED 2026-06-05]**

`setupVacuumScenario(name, ctx)` is declared **2-arg** (`vacuum-scenarios.js:32`)
but the dispatcher calls it **3-arg** `(name, harness, ctx)` like every sibling
(`index.js:88`). So `ctx` binds to the harness, the real `ctx` is dropped
(`N`/`midF` become `undefined`), and the body's 7 `harness.*` references
(`:44, :46, :58, …`) hit a **free variable that doesn't exist** → `ReferenceError`.
The C++ mirror (`vacuum.cpp`) is correct, so the **deployed WASM path is fine** —
only the MockBridge *fallback* throws when any `s0-vacuum-*` is selected (Safari /
no-COOP-COEP). **Fixed 2026-06-05:** signature → `(name, harness, ctx)`,
`this`→`harness.bridge` (the whole switch body already used `harness` correctly —
only the signature + two stragglers were wrong); guarded by a `?engine=mock` test in
`scale0-scenario-health.spec.js` (electron/proton/photon/higgs mount, no error).
*(This is why `s0-vacuum-*` shows `owner=wasm` in the sweep — the bug was on the path
the sweep didn't exercise.)*

### A.5 Dead quantum measurement-mode flags · **[OPEN, dead code]**

`quantum-zeno` sets `_quantumZenoMode`/`_quantumZenoInterval`
(`quantum-scenarios.js:285-286`) and `quantum-entangle` sets
`_quantumExperimentMode` (`:201`) — **zero readers** anywhere in `engine/web`.
No repeated-measurement / decay-suppression loop, no correlation tracker. Remove
the flags (and see §B: these scenarios implement the *setup* but not the *effect*).

### A.6 Telemetry that *does* work (for the record)

Mock energy (varies correctly), particle counts (both bridges), E/B field samples
(WASM, e.g. `s0-field-uniform-e` seeds an E-field with zero `|J|`), and zero
console errors across all 97.

---

## B. Physics-sense on the lattice

> Scale 0 *is* the substrate. Per scenario: **GENUINE** substrate phenomenon /
> **CROSS-SCALE-defensible** (constituents live on the substrate) / **QUESTIONABLE**
> (higher-scale phenomenon cosplayed on 33³) / **MISLABEL** (impl ≠ name).

### B.1 Category verdicts

| Category | Verdict | Note |
|---|---|---|
| Empty | GENUINE | reset only |
| Wave Dynamics (`flux-*` waves) | **GENUINE** | direct J-field configs; `flux-soliton` honestly sets `genesis=false` |
| Genesis & Manifestation | **GENUINE** | super/near-threshold flux → the actual genesis rule (K_GENESIS=1.533) |
| Substrate Physics (cyclotron/screening/thermalization/triad/**zero-point**) | **GENUINE** | `flux-zero-point` is the honesty gold standard — in-code "*pedagogical, NOT a derivation of ½ℏω; amplitude is a [SELECTION]*" |
| Light & EM (`light-*`) | **GENUINE** | uses the correct **lattice dispersion** `ω=2c·sin(k/2)`, not continuum `ω=ck` |
| Field Configurations (`s0-field-*`) | **GENUINE** | textbook E/B/wave configs realized as J/wave_vel |
| Moore Seeds | **GENUINE** (most substrate-native) | exact polyhedral Moore shells — instantiate `THEOREM_MOORE_LAYER_DECOMPOSITION` |
| Emergent Bound States (FTD-0107 `ic*`) | **GENUINE** (highest scientific value) | reproduce the FTD-0107 measurement; correctly `[STRUCTURAL HYPOTHESIS]` |
| Confinement (`flux-meson/baryon/string-breaking`) | CROSS-SCALE-defensible | 2–3 ±1 charges + flux tubes; "quark" labels interpretive |
| SM Quarks/Bosons/Processes (`s0-seed-*`) | CROSS-SCALE-defensible | substrate excitations; honestly `[CONJECTURE]` |
| Vacuum Particles (`s0-vacuum-*`) | CROSS-SCALE-defensible | honest in-code ("[SELECTION] amplitude cue", kaon "[PARAMETRIC]") — **but see A.4 bug** |
| Atoms & Molecules | CROSS-SCALE-defensible | helium fixed 2026-04-28 (14 real constituents) |
| Gauge/Topological (`monopole`/`instanton`) | CROSS-SCALE-defensible (as *static field ansätze*) | correct Wu-Yang / BPST *profiles*; not verified emergent solitons |
| **Quantum Lab** (effects) | **MIXED — mostly QUESTIONABLE** | see B.3 |
| **Gravity / Cosmology** | **QUESTIONABLE / MISLABEL** | see B.2 |
| Reference frame / Observer | QUESTIONABLE / redundant | see B.2 |
| Life / Abiogenesis (`spark-of-life`) | QUESTIONABLE but honestly `[DEMO]` | a choreographed tableau; strip the `autocatalytic`/`abiogenesis` tags |

`quantum-double-slit`, `-tunnel`, `-well` are **GENUINE** (real lattice wave
interference / tunneling / mode quantization); `quantum-born-rule` is a defensible
Born-rule *demo*.

### B.2 Gravity + Reference frame (code-grounded)

> **Owner decision (2026-06-05): removed the cosmology scenario, kept the gravity
> ones.** `s0-seed-frw-patch` was retired across all layers; the category
> `Gravity / Cosmology` → `Gravity` (it now holds only schwarzschild, lensing,
> gravitational-wave). The gravity visualizations were kept — they read well in the
> dashboard — with the label caveats below standing.

- **`s0-seed-frw-patch`** — **`[REMOVED 2026-06-05]`.** It injected an **alternating
  +1/−1 charge checkerboard** on a stride-5 sublattice — no scale factor a(t), no
  metric, no fluid; zero relation to FRW cosmology. Retired from the registry, the
  JS/C++ impls, toggles, knowledge-base, and the gravity-observables set (the
  separate `ctor::frw_patch` *constructor* in the C++ constructor library is a
  different subsystem and was left intact).
- **`s0-seed-gravitational-wave`** (`s0-seed-scenarios.js:732`, `s0_seed.cpp:674`) —
  **KEPT** (owner: works well as a visualization). Code-honest caveat stands:
  `Jy = A·sin(kx)` is a **transverse flux (spin-1-like) wave**, not the **spin-2
  metric (h_μν) quadrupolar strain** of a real GW (FTD's emergent spin-2 mode is
  **[OPEN]**, Frontier 4). Tag it `[VISUALISATION]`, not a literal GW claim.
- **`s0-seed-schwarzschild`** (`:713-730`) — static inward flux bias; the comment
  itself says *"a visualization aid, NOT engine gravity (does not gate on the
  gravity toggle …)"*. **`[VISUALISATION]`** — honest in-code, misnamed in the UI.
- **`s0-seed-gravitational-lensing`** (`:627-669`) — Schwarzschild well + off-axis
  photon pulse; whether it actually *deflects* is unverified at runtime.
  **`[VISUALISATION]`** (→ `[EMERGENT]` only if runtime-confirmed).
- **`s0-seed-sloop`** (`:747-756`) — 12 +1 particles on a ring with tangential
  flux: a **12-site flux vortex**, sibling of `flux-vortex`/`s0-field-vortex-line`.
  The "self-referential reference-frame" framing has **no mechanism**.
  **QUESTIONABLE** → regroup as a flux vortex, drop the framing.
- **`s0-seed-observer-cell`** (`:757-765`) — **correction to my preliminary note:**
  it injects the **full 27-site Moore cell** (center +1 / 6 / 12 / 8), i.e. an
  exact **sign-inversion of `s0-seed-moore-decomposition`**, not a 7-site seed.
  GENUINE *geometry* but **redundant**, with no observer mechanism. → retire as a
  duplicate, or relabel "Moore cell (inverted shells)" and drop "observer".

### B.3 Named quantum *effects* — setup geometry without the readout

`quantum-casimir` (`:245-269`, plates + foam, **no force readout**),
`quantum-zeno` (`:270-288`, near-threshold blob + the dead flag, **no
measurement loop**), `quantum-aharonov-bohm` (`:204-244`, correct solenoid+packets
geometry but **no fringe-shift readout**), `quantum-eraser` (`:72-122`, polarized
slits + polarizer but **no which-way-vs-erased comparison**), `quantum-entangle`
(`:188-203`, real pair genesis but **no correlation tracker** + dead flag). Each
builds the named effect's *setup* but does not *measure* the effect.
**`[SETUP-ONLY]`** → relabel "… (setup)"; best ROI is implementing the readout for
**aharonov-bohm** (fringe shift) and **casimir** (mode-density force), whose
geometry is already correct.

---

## C. Recommendations (prioritized)

1. **[DONE]** Fix `flux-annihilation` worker crash (A1) — `7fc4296b`.
2. **[DONE]** Fix the `vacuum-scenarios.js` `harness` signature (A4) — all
   `s0-vacuum-*` restored on the MockBridge fallback path; mock-path test added.
3. **[DONE]** Retire the cosmology scenario `frw-patch` + rename `Gravity / Cosmology`
   → `Gravity` (B.2; owner decision — the gravity visualizations were kept).
4. **[telemetry, open]** Reconcile WASM vs mock energy convention (A2).
5. **[physics-sense, open]** Relabel the named quantum effects "… (setup)" (B.3);
   delete the dead `_quantumZeno*` / `_quantumExperimentMode` flags (A.5).
6. **[hygiene, open]** `observer-cell` retire-as-duplicate or relabel (B.2); strip
   `spark-of-life`'s `autocatalytic`/`abiogenesis` tags; relabel
   `monopole`/`instanton` as "field ansatz".
7. **[scenario quality, open]** Fix `flux-annihilation`'s dead kicks (A3); the kept
   `gravitational-wave` carries a `[VISUALISATION]` label caveat (not a literal GW).

The substrate-native categories need **no retraction**; the cosmology scenario was
the most clear-cut removal, and the named quantum effects need either a
readout or an honest "(setup)" label.

---

## D. Artifacts

- **Mechanical sweep:** `engine/web/tests/scale0-scenario-health.spec.js` (guard).
- **Worker fix:** `engine/web/js/bridge/mock-bridge.worker.js` (`applyInit`).
- **Physics-sense pass:** code-grounded review of all six scenario group files +
  C++ mirrors + dispatcher + diagnostics + the quantum-overlay spec.
- **Commits:** `7fc4296b` (sweep + worker fix), `34c19160` (`flux-zero-point`).

*Health numbers + `file:line` verified against 2026-06-05 source; re-run the sweep
to reproduce.*
