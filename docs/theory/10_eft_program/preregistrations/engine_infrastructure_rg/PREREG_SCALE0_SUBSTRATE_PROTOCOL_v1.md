# Pre-Registration — Scale-0 Substrate Experimental Protocol (v1)

**Tag:** [PRE-REGISTRATION] — locks, *before measurement*, the predicted behaviour of the FTD
Scale-0 lattice engine for every Scale-0 scenario, with the falsification criteria. Contains **no
results**. The engine runs against the **hash-locked** version of this file; verdicts land in a
separate result doc.

**Date:** 2026-05-31
**Hash-lock target tag:** `preregister-scale0-substrate-protocol-v1`
**Apparatus:** the FTD Scale-0 lattice engine as served by `engine/web/` (WASM core
`engine/web/wasm/ftd_core.{js,wasm}`, with the pure-JS MockBridge as fallback).
**LEDGER row:** to assign at lock (grep `docs/` for next-free; 0238–0243 contended).

---

## §0 — Purpose, and the honest scope (read first)

This protocol treats the discrete substrate as a laboratory: **the engine is the apparatus, the
five FTD postulates + their lattice realization are the theory, and the predictions below are
computed analytically *before* the engine is run.** It is "pure math" in the sense the request
intends — deterministic predictions from the rules, tested against a deterministic machine.

**What this protocol can falsify/confirm:** whether the FTD discrete substrate *actually exhibits
the emergent behaviour the theory says it must* — propagation speed, conservation laws, the genesis
threshold, the FTD-0107 cluster structure, the Phase-G Coulomb geometry, determinism. A deviation
falsifies **either the theory-prediction or the implementation**; agreement confirms the model has
that property.

**What it CANNOT do (stated so it is never overclaimed):**
1. It is **not** a comparison to physical nature. No lab measurement of a real electron/photon is
   involved. This is internal validation of a mathematical model, not empirical confirmation of
   physics.
2. It is **not** a derivation of the constants. The engine *seeds* α (as `G_C²`), `K_B = m_e`, and
   `K_GENESIS = N_c·K_B` as **inputs**. The MC-T4.3 boundary theorem stands: those values are not
   forced. Any scenario whose "prediction" is just *the engine reproducing a number it was seeded
   with* is an **implementation-faithfulness test**, not evidence for FTD's physics.

**The load-bearing distinction this protocol is built around:** the genuinely falsifiable
predictions are the **calibration-independent, emergent, geometric/topological** ones —
`c_lat = 1/√3` (lattice geometry), cluster **count** determinism + **L-invariance** + the **k = ¼
scaling exponent** (O_h representation theory), the Phase-G Coulomb **shape** (lattice Green's
function), and the conservation laws. None of these depends on α or `K_B` being correct, so they can
fail. These form the **Core** (§3–§4). Everything seeded/assumed is **Illustrative** (§5) and
explicitly tagged *not a falsifiable theory test*.

---

## §1 — The apparatus and readout (LOCKED)

- **Backends.** `createBridge()` loads the **WASM** C++ core (badge "WASM Engine"); falls back to
  **MockBridge** (pure JS). **Firm predictions in §3–§4 are stated for the WASM core** (the same C++
  engine the FTD-0107 campaign used). MockBridge is an *approximation*: it may not reproduce exact
  cluster counts, the determinism hash, or dual-substrate quantities (per the engine audit). Each
  test below names its required backend.
- **Determinism.** The WASM/C++ core is deterministic: identical initial state + seed → bit-exact
  trajectory (golden-tick hash `0xcd957b601d47868a` at L=16, CPU/CUDA parity).
- **Observables (programmatic readout).** Via the live bridge handle (`window.__ftdCtx.bridge`):
  - `getDiagnostics()` → `{tick, manifested, positive, negative, chargeBalance, totalEnergy,
    totalFlux, entropy, angMom{X,Y,Z}}`
  - `getEnergyAudit()` → `{fieldEnergy, waveEnergy, particleKE, coulombPE, EFieldEnergy,
    BFieldEnergy, totalPoynting, gaussViolation, maxGaussError, …}`
  - `getConservationTotals()` (physics harness) → `{E, px,py,pz, Lx,Ly,Lz, Q, tick}`
  - sampled fields: `getFluxVectorSampled(stride)`, `getDivJSampled`, `getEFieldSampled`, …
  - history: `window.__telemetryHub` ring buffers (500 samples) for energy/flux/gauss.
- **Measurement convention.** Every test states: scenario key, locked toggles/params, observable +
  readout path, measurement window (ticks), predicted value, tolerance, and the falsifier.

---

## §2 — Methodological commitments (LOCKED)

- **M1 — Predictions are theory-derived, pre-computed.** Each expected value comes from FTD
  analytics independent of running the engine (cited per entry). Reading a value off the engine and
  calling it the prediction is forbidden (it cannot falsify).
- **M2 — Three outcomes per test:** **CONFIRMED** (within tolerance), **DEVIATION** (outside
  tolerance but plausibly discretization/equilibration — flagged for investigation, not yet a
  falsification), **FALSIFIED** (outside tolerance with the deviation structurally inconsistent with
  the prediction). Tolerances are locked here.
- **M3 — Tiering (no manufactured predictions).** Each test carries a tier:
  **[CORE-FIRM]** (theorem/derived, calibration-independent — hard falsifier);
  **[CORE-STRUCTURAL]** (confirmed emergent structure — falsifiable);
  **[IMPLEMENTATION]** (faithful reproduction of a seeded input — tests the code, not the physics);
  **[ILLUSTRATIVE]** (seeded/assumed configuration — **not** a falsifiable theory test; cataloged
  for completeness only). A scenario with no honest quantitative prediction is marked ILLUSTRATIVE,
  never given a fabricated number.
- **M4 — No post-hoc tuning.** No tolerance, scenario param, or prediction may be edited after
  hash-lock; a defective entry requires a v2.
- **M5 — Seeds & windows fixed.** Stochastic scenarios use the engine's fixed seeds; each test fixes
  its measurement window and (where relevant) seed count.

---

## §3 — Cross-cutting invariants (LOCKED) — the universal falsifiers

These must hold across **all** applicable scenarios; they are the strongest tests because a single
violation falsifies a structural claim regardless of any single scenario.

| # | Invariant | Prediction (theory) | Conditions | Readout | Falsifier | Tier |
|---|---|---|---|---|---|---|
| **I1** | Signal speed | wavefront speed = `c_lat = 1/√3 ≈ 0.5774` voxels/tick | `flux-pulse` or `s0-field-photon-pulse`, **genesis OFF, damping OFF** | outermost `|J|>ε` radius vs tick (`getFluxVectorSampled`) | front speed deviates **>5%** from 1/√3 after ≥10 ticks (early-tick integer rounding allowed) | [CORE-FIRM] (FTD-0041) |
| **I2** | Energy conservation | `|ΔE|/E₀ < 0.5%` over 200 ticks | **conservative config only**: genesis OFF, langevin OFF, damping OFF (`light-*`, `s0-field-plane/standing-wave`) | `getEnergyAudit().fieldEnergy+waveEnergy` | monotone/secular drift **>0.5%** (golden-tick is <0.1%/100t) | [CORE-FIRM] |
| **I3** | Gauss constraint | post-projection `maxGaussError → 0` (decreasing each tick to `<1e-3`) | any **gauss_projection ON** scenario | `getEnergyAudit().{gaussViolation,maxGaussError}` before/after | residual **grows** or stays `O(1)` after projection | [CORE-FIRM] (FTD-0054) |
| **I4** | Charge conservation | `Q = Σs` constant absent genesis/decay events | conservative config (no genesis) | `getConservationTotals().Q` | `Q` drifts with **no** manifestation/decay event | [CORE-FIRM] |
| **I5** | Determinism | identical scenario+seed ⇒ bit-exact observable trajectory | **WASM** core | rerun, compare `getDiagnostics()` trajectories | trajectories diverge beyond float epsilon (WASM) | [CORE-FIRM] |
| **I6** | Locality | no observable change propagates **>1 voxel/tick** | all | front-arrival vs distance | superluminal (faster-than-1-voxel/tick) influence | [CORE-FIRM] (Postulate 4) |

*Note I2/I4:* genesis-, langevin-, and damping-ON scenarios **deliberately do not conserve energy/
charge** (genesis creates matter; langevin is a thermostat; damping dissipates). Applying I2/I4 to
those is a category error — they apply only to the conservative configs named.

---

## §4 — The falsifiable Core (LOCKED, full detail)

### §4.1 — Genesis & emergent bound states (FTD-0107 / FTD-0110) — the crown jewel

This family carries the strongest *emergent* (calibration-independent) predictions: cluster
topology and scaling derived from O_h symmetry, not seeded.

**Locked params** (per scenario-registry; toggles: wave_propagation ON, gauss_projection ON,
genesis ON, langevin ON, dual_substrate OFF):

| Scenario | Injection | Amplitude | Langevin T | Predicted observable | Value | Tolerance | Falsifier | Tier |
|---|---|---|---|---|---|---|---|---|
| `s0-seed-emergent-ic1` | point, +x axis | `10·K_GENESIS` | 0.005 | cluster **count** | **1**, identical across **5/5 seeds** and at **L∈{32,64}** | exact | count ≠1, or varies by seed/L | [CORE-STRUCTURAL] |
| `s0-seed-emergent-ic3-collision` | two opposed pulses ±N/4 | `±5·K_GENESIS` | 0.005 | cluster **count** | **2**, 5/5 seeds, L-invariant | exact | count ≠2 or seed/L-dependent | [CORE-STRUCTURAL] |
| `s0-seed-emergent-ic4-subthreshold` | point | `0.5·K_GENESIS` (**sub-threshold**) | 0.005 | manifested voxels | **0**, 5/5 seeds (negative control) | exact | **any** manifestation >0 | [CORE-STRUCTURAL] |
| `s0-seed-emergent-ic1` (size) | point | `10·K_GENESIS` | 0.005 | cluster **size** N | ~**25** voxels (ic1); ~**3–5** (ic3 per cluster) | ±~20% | size grossly off (e.g. ∝ L³) | [EMPIRICAL] (abs 25 [OPEN]) |
| ic1 **amplitude sweep** | point | vary A | 0.005 | scaling `N(A)` | `N ≈ ¼·(A/K_GENESIS)²` → exponent **2.0**, coeff **¼** | exponent ±0.2; coeff ±~30% | exponent ≠2 or coeff ≠¼ beyond tol | [CORE-FIRM (linear)] / [SMC (nonlinear)] (FTD-0110) |
| `s0-seed-emergent-ic1-diagonal` vs `-isotropic` | body-diagonal / 6-star | scaled | 0.005 | **direction-invariance** of k | k same as axial ic1 | ±~30% | k axis-dependent | [CORE-STRUCTURAL] (O_h) |

**Why this is the real evidence:** the *count* (1/2/0), *seed-invariance*, *L-invariance*, and the
*exponent-2 / coeff-¼ scaling* are **emergent** — derived from the O_h A₁g multiplicity = N_base = 4
(`DERIV_K_FROM_OH_A1G_MULTIPLICITY.md`), **not** seeded. If the engine's cluster scaling exponent
isn't ≈2 with coefficient ≈¼, the FTD-0110 derivation is **falsified**. The absolute "25" is
empirical ([OPEN] why 25); a deviation there is informative, not fatal.

**Backend:** WASM. The `-viz` variants (A=`20·K_GENESIS`, T=0) are for visualization only (higher
amplitude for visibility) — use the **non-viz** ic1/ic3/ic4 (T=0.005) for the falsifiable counts.

### §4.2 — Wave & light dynamics

| Scenario | Locked | Predicted observable | Value | Tolerance | Falsifier | Tier |
|---|---|---|---|---|---|---|
| `light-photon-race` | 2 pulses, amp 0.05 vs 0.5, genesis OFF | **linearity**: both fronts same speed | speed independent of amplitude (= `c_lat`) | <5% | large pulse travels at different speed | [CORE-FIRM] |
| `flux-standing` | counter-propagating, genesis OFF | non-dispersive standing pattern; energy conserved | stable nodes; `|ΔE|/E₀<0.5%` | per I2 | dispersion/decay absent damping | [CORE-FIRM] |
| `flux-pulse` (genesis OFF) | single Gaussian | spherical expansion at `c_lat`; energy conserved | I1 + I2 | I1/I2 | — | [CORE-FIRM] |
| `flux-interference`, `flux-nested-standing` | multi-source, genesis OFF | linear superposition of wavefronts | additive field at overlap | <5% | non-additive overlap (absent genesis) | [CORE-FIRM] |
| `s0-field-uniform-e` | E-field, **genesis OFF** | sub-threshold E ⇒ **no** Schwinger pair production | manifested = 0 | exact | spontaneous manifestation from sub-threshold uniform E | [CORE-STRUCTURAL] |

### §4.3 — Phase-G geometric Coulomb (FTD-0004)

| Test | Predicted | Value | Tolerance | Falsifier | Tier |
|---|---|---|---|---|---|
| Static pair energy `V(r)` (emergent-forces / static-pair probe; **requires the campaign measurement mode**, not a one-click web scenario) | `V(r) = −2·G_L(r)`, the lattice Poisson Green's function; `α_r(r,L)=2r·G_L(r) → 1/(4π)` as `r≪L` | R²→1.0000 at L=384, r≥34; median residual ≤0.1% | R² and residual as stated | shape departs from `2r·G_L(r)` beyond tolerance in the continuum regime | [CORE-FIRM] (geometric) |

**Honest note:** this is the *lattice Coulomb geometry* (zero free parameters) — it is **not** a test
of α (no coupling enters this code path). Related web scenarios (`s0-field-electric-dipole`,
`flux-screening`) visualize Coulomb dressing but the clean falsifiable measurement is the static-pair
probe of the engine, which may require the campaign harness rather than the dashboard.

---

## §5 — Illustrative catalog (LOCKED params; NOT falsifiable theory tests)

The following ~60 scenarios are **seeded/assumed configurations** (tags [SELECTION]/[CONJECTURE]/
[IMPOSED]/[OPEN] per the catalog). They are cataloged with their locked injection params for
completeness and reproducibility, but per **M3** they are **[ILLUSTRATIVE]**: their "expected"
behaviour is *that the engine renders the configuration it was given*, which is an
**implementation/visualization** check, **not** evidence for FTD's physical identifications. They
**must not** be scored as confirmations of the framework.

| Class | Scenarios | Locked-param source | Honest expectation | Why not a theory test |
|---|---|---|---|---|
| SM vacuum particles | `s0-vacuum-{electron,muon,tau,*-neutrino,photon,w,z,higgs,proton,neutron,pion-±/0,kaon}` (15) | §I catalog | renders the seeded envelope/charge/triad; mass *ratios* m_μ/m_e, m_τ/m_e, m_p/m_e are [THEOREM] **inputs**, not engine outputs | amplitudes/envelopes are [SELECTION]; masses seeded |
| SM quark flavours | `s0-seed-{up,down,strange,charm,bottom,top}-quark` (6) | §VII | renders color/charge/amplitude-boost | quark masses [OPEN]; boosts [SELECTION] |
| Bosons / fields | `s0-seed-{higgs-field,gluon}` (2) | §VII | gluon massless at `c_lat` (that part is [CORE-FIRM] via I1); Higgs VEV is [SELECTION] | VEV/mass seeded |
| Atoms/molecules | `s0-seed-{hydrogen,helium,h2-bond-formation}` (3) | §VIII | renders nuclei+electron envelopes; dynamic bond is [CONJECTURE] | geometry assumed |
| SM processes | `s0-seed-{beta-decay,ee-annihilation,quark-gluon-plasma}` (3) | §IX | β-decay leptons **preseeded** (not produced); annihilation via phase_movement; QGP thermalizes | mechanisms assumed/preseeded |
| Gauge / topological | `s0-seed-{wilson-loop,flux-tube,monopole,instanton}` (4) | §X | renders the seeded field topology | [CONJECTURE] |
| Gravity / cosmology | `s0-seed-{schwarzschild,gravitational-lensing,frw-patch,gravitational-wave}` (4) | §XI | inflow/lensing are **visualization aids** (not engine gravity); `G_N=0.01` is [PARAMETRIC] | not substrate gravity |
| Observer / RF | `s0-seed-{sloop,observer-cell}` (2) | §XII | renders the 12-ring / 27-site geometry | [CONJECTURE] |
| Moore seeds | `s0-seed-{octahedron,cuboctahedron,stella-octangula,moore-cell,moore-decomposition}` (5) | §VI | the 6/12/8 shell decomposition is [THEOREM] **geometry**; renders correctly | structure is definitional, not a dynamical prediction |
| QCD / quantum demos | `flux-{meson,baryon,string-breaking,cyclotron,screening}`, `quantum-{born,double-slit,eraser,tunnel,well,entangle,aharonov-bohm,casimir,zeno}` | §I,III | qualitative phenomenology (interference, tunneling, cyclotron orbits) | mostly [SELECTION]/[CONJECTURE]; some sub-tests (e.g. quantum-well mode quantization, tunnel `T∝e^{-2κW}`) could be promoted to [CORE] in v2 if a clean theory value is derived first |

*A note on Bell (`quantum-entangle`, ic-collisions):* the substrate prediction is **S ≤ 2**
(local-causal substrate). Measuring `S > 2` at the substrate level would **falsify** the
local-substrate claim; `S = 2√2` is an [SELECTION]-tagged aggregate claim, not a substrate
prediction. A substrate CHSH probe is a candidate [CORE-STRUCTURAL] test for v2.

---

## §6 — Falsifier rules & banned moves (LOCKED)

- **F1.** A CORE test that lands outside tolerance with a deviation *structurally inconsistent* with
  the prediction (e.g. cluster count seed-dependent; front speed ≠ 1/√3 at equilibrium) is
  **FALSIFIED** — it falsifies the FTD-prediction or the implementation; the result doc must say which.
- **F2.** Predictions read off the engine rather than derived (violating M1) are inadmissible.
- **F3.** Scoring an [ILLUSTRATIVE] scenario as a confirmation of FTD physics fires this rule.
- **F4.** Applying I2/I4 (conservation) to genesis/langevin/damping-ON scenarios is a category error
  and inadmissible.
- **F5.** MockBridge results may not be used to falsify a [CORE-FIRM] WASM prediction (backend
  mismatch); they may corroborate qualitatively.
- **B1–B4 (banned):** no post-hoc tolerance/param edits (M4); no claim that engine agreement proves
  FTD's *physical* correctness or *derives* α/masses (§0.2); no manufacturing a number for an
  [ILLUSTRATIVE] scenario; no presenting seeded-input reproduction ([IMPLEMENTATION]) as theory
  evidence.

---

## §7 — Outcomes & scoring (LOCKED)

For each test: record `{scenario, observable, predicted, tolerance, measured, window, backend,
verdict ∈ {CONFIRMED, DEVIATION, FALSIFIED}, notes}`. The protocol's headline result is the
**CORE** tally (I1–I6 + §4): how many of the calibration-independent, theory-derived predictions the
substrate satisfies. The Illustrative catalog (§5) is reported as rendered/not-rendered only.

A clean run where the CORE all CONFIRM is strong evidence that **the discrete substrate genuinely
has the emergent structure FTD predicts** (the honest, bounded claim of §0). Any FALSIFIED CORE test
is the more valuable result — it pins a real gap between theory and the substrate's actual dynamics.

---

## §8 — Hash-lock protocol

1. Finalise §§0–7. `sha256sum` this file; record SHA + tag in `../REF_PREREGISTER_MANIFEST.md`; add
   a `[PRE-REGISTRATION]` LEDGER row (confirm next-free id against the whole `docs/` tree first).
2. `git tag preregister-scale0-substrate-protocol-v1`.
3. The measurement run executes **only** against the tagged commit; verdicts land in a separate
   `AUDIT_SCALE0_SUBSTRATE_RESULTS.md`, never as edits here.
4. Defective definitions/tolerances ⇒ a v2 (do not edit v1).

---

## §9 — One-line summary

A pre-registered, falsifier-gated protocol that turns the Scale-0 engine into a laboratory: it
tests, against theory-derived and (where it matters) *calibration-independent* predictions, whether
the FTD discrete substrate actually exhibits the propagation speed, conservation laws, genesis
threshold, FTD-0107 cluster topology/scaling, and Phase-G Coulomb geometry the theory requires —
honestly separating the genuinely falsifiable emergent core from the seeded illustrations, and never
mistaking a faithful reproduction of an input for evidence of the framework's physics.
