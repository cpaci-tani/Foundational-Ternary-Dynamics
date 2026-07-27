# AUDIT — Scale-0 Scenario Qualification Closure (2026-07-24)

**Status:** complete for the frozen scenario catalog and production tick.

**Scope:** all 117 Scale-0 scenario IDs in
`engine/web/js/scales/scale0/scenario-registry.js`, their C++ production
implementations, JS fallback mirrors, browser mounting path, and evidence
records. This supersedes the physics-sense conclusions of the 2026-06-05
health audit while preserving that document as historical provenance.

## 1. What “qualified” means

Admission certifies the claim written in each scenario's `qualification` and
`validation.assertion`. It does not certify the legacy scenario ID or the
physical object suggested by that ID.

A scenario is admitted only when all of the following hold:

1. Its active terms and boundary condition are isolated and explicit.
2. Its stated observable is tested against a control or exact invariant.
3. Deterministic cases replay exactly; stochastic cases use fixed seeds and a
   stated cohort.
4. The result is finite and tied to a stated lattice size and tick window.
5. A failed physical interpretation is retained as a closed-negative control,
   not promoted by relabeling the same behavior.
6. C++ and JS contain the same ID, and the deployed WASM scenario publishes a
   finite mounted frame in the browser.

This produces five honest menu classes:

| Class | Count | Meaning |
|---|---:|---|
| Validated Native Dynamics | 79 | A measured property of the frozen native field/update map, including exact ansätze and null controls |
| Validated State Dynamics | 34 | A measured manifested-state or selected reaction response |
| Qualified Selected Extensions | 1 | A measured toggle-gated extension whose surviving behavior and failed physical interpretation are both explicit |
| Validated Initial Data | 2 | A prepared geometry whose construction, not physical identity, is certified |
| Macroscopic Physics & Measurement | 1 | A bounded prepared-response probe with its macroscopic interpretation explicitly rejected where unsupported |
| **Total** | **117** | **All catalog entries admitted with behavioral evidence** |

The registry is the complete per-scenario run-of-record manifest. It contains
one qualification, one test location, and one concrete assertion for every ID.

## 2. Closure result

- Menu, internal catalog, JS implementations, C++ implementations, and evidence
  records are exactly 117/117/117/117/117.
- There are no hidden or mechanically smoke-only scenarios.
- 108 scenarios cite `engine/tests/test_scenario_behavior.cpp`; the remaining
  9 cite the focused boundary, genesis, reaction, dynamic-flux-dressing,
  reciprocal-moving-source, velocity, and Thomson tests recorded in the
  registry (8 distinct files).
- Cumulative browser evidence has loaded all 117 production WASM scenarios.
  The all-catalog campaign was rerun 2026-07-25 when the catalog reached 116
  (`npx playwright test scale0-scenario-health.spec.js`, 4/4, all entries on
  the `wasm-worker` owner); FTD-0477 then adds a focused production-WASM test
  for the 117th scenario, including its sub-voxel mechanical position and
  overlay/profile contract. The count is derived from the registry and guarded
  by `scenario-parity.spec.js`, not trusted as a free-standing prose count.
- `quantum-tunnel` exposed a defect in the audit rather than the engine: its
  three locked sheets can take more than a fixed 650 ms to publish their first
  worker frame. The health gate now waits on finite active-owner telemetry and
  records readiness latency. Its isolated regression mounted 3267 states with
  finite tick-1 telemetry.
- The three deployed WASM variants were rebuilt from source. `build_info.txt`
  records source revision `89ae1149489c13571dfd8b48a8de0bc63c960820` and
  variants `wasm32,wasm64,wasm32-threads`.

The native golden battery remains unchanged and passes 7/7.

## 3. Physical interpretations that did not survive

Qualification legitimized many setups as native experiments, but it closed or
withheld the stronger names when the required observable was absent:

- Locked marker planes are coupling sources or inert markers. They do not
  implement tunneling barriers, quantum wells, or Casimir boundaries.
- The eraser, Zeno, entanglement, Aharonov–Bohm, and double-slit names lack the
  necessary measurement, phase, or single-event observable. Their certified
  content is a classical/native control.
- Quark, charged-lepton, W, Z, Higgs, pion, kaon, proton, and neutron templates
  do not establish corresponding poles, quantum numbers, binding, masses, or
  decay channels. Several composite candidates disappear under their isolated
  native dynamics.
- Hydrogen, helium, and molecular labels do not yet carry a native bound-state
  spectrum. The prepared Coulomb response is the only certified content.
- The EW drive is nonnegative and has no down-sweep, so it cannot demonstrate
  hysteresis or a phase transition.
- The beta setup pre-seeds its alleged products; it measures weak state flips,
  not native beta emission.
- The QGP setup is a fixed-seed Langevin transport/outflow cohort with color
  dynamics disabled.
- The life setup creates a finite genesis cohort but has no replication,
  heredity, metabolism, or autocatalytic growth observable.

These remain useful because a closed-negative scenario is a regression test
against exactly the overinterpretation that its old name invited.

## 4. Plan for further physical promotion

The following gates are the only legitimate path from a qualified native probe
to a stronger physical identification. Failure leaves the current scenario and
qualification unchanged.

### Gate A — native waves and photon candidate

1. Extract the retarded on-shell pole and residue over volumes, directions,
   momenta, and amplitudes.
2. Demonstrate a stable low-momentum speed intercept and quantify cubic
   anisotropy rather than identifying a packet by visual motion.
3. Define operational emission, absorption, and energy-transfer events between
   manifested sources and the wave sector.
4. Promote `s0-vacuum-photon` only if the same pole mediates those events and
   survives the common-cone and spectral-positivity gates.

### Gate B — polarity to electromagnetic charge

1. Define charge operationally from the long-distance response to a prepared
   source, not from the `+/-` labels alone.
2. Measure the continuity equation including every native reaction and identify
   the regime in which the reaction term vanishes or is parametrically slow.
3. Recover one universal coupling and Ward-type relation across source species.
4. Promote Coulomb, dipole, cyclotron, and atom scenarios only after the same
   charge observable controls all of them without sector-specific tuning.

### Gate C — matter and bound states

1. Identify positive-residue, long-lived poles from retarded correlators around
   stable manifested configurations.
2. Establish binding by a negative energy difference relative to separated
   constituents, volume convergence, and perturbation recovery.
3. Establish quantum-number distinctions dynamically. Amplitude, color labels,
   and selected geometry do not count.
4. Require a spectrum or scattering observable before using hadron, lepton,
   atom, or molecule names. The present frozen composite candidates are closed
   negative; a new ontology or rule is a new preregistered model cycle.

### Gate D — named quantum effects

1. Aharonov–Bohm: define a gauge-invariant loop/phase observable and demonstrate
   path-dependent shift with field-free arms and topology controls.
2. Casimir: define a force or stress estimator and perform plate/no-plate
   ensemble subtraction with spacing and volume scaling.
3. Double slit/eraser: accumulate localized detection events and preregister a
   which-path intervention and erasure comparison.
4. Zeno: define a physical intervention channel and compare survival curves at
   fixed total elapsed time.
5. Entanglement: define local settings and outcomes; a shared pair ID remains
   bookkeeping and cannot satisfy this gate.

### Gate E — thermodynamic, phase, and macroscopic claims

1. Define temperature, order parameter, susceptibility, correlation length, and
   finite-size scaling before using phase-transition language.
2. Hysteresis requires a preregistered up/down protocol with the same dynamics
   and rate-dependence controls.
3. QGP requires active color interactions plus an observable separating bound
   and deconfined regimes; particle loss through an open boundary is not one.
4. Life requires independent operational tests for sustained replication,
   heredity with variation, and resource-coupled persistence. Until all three
   exist, the spark scenario remains a finite patterned genesis response.

### Gate F — infrastructure and reproducibility

1. Add state-hash parity tests between C++ CPU, deployed WASM, and JS fallback
   for the deterministic subset; ID parity alone is not trajectory parity.
2. Store the 117-row mechanical campaign as a versioned JSON artifact, including
   source revision, lattice size, owner, first-frame latency, tick, energy, field
   peak, particle count, and errors.
3. Add multi-volume and multi-seed campaigns only to scenarios whose claim
   requires them; exact-construction controls do not gain meaning from sweeps.
4. Keep legacy IDs for compatibility, but generate every user-facing title,
   description, and knowledge-base entry from the qualified claim.

## 5. Acceptance policy

No scenario is promoted because it resembles a familiar picture or approaches
a known constant. Promotion requires a new observable, a preregistered control,
finite-size and parameter robustness appropriate to the claim, and a behavioral
test that would fail if the identification were false. Otherwise the current
native, initial-data, or closed-negative qualification is final for the frozen
model.

## 6. Reproduction commands

```powershell
cd engine/build
ctest -C Release --output-on-failure -R "^scenario_behavior$"

cd ../web/tests
npx playwright test scenario-parity.spec.js scale0-toggle-trap.spec.js
npx playwright test scale0-scenario-health.spec.js

cd ../..
.\build_native.bat golden
```
