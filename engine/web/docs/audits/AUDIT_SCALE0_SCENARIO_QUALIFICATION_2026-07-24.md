# AUDIT — Scale-0 Scenario Qualification Closure (2026-07-24)

**Status:** complete for the frozen scenario catalog and production tick.

**Scope:** all 142 Scale-0 scenario IDs in
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
| Validated Native Dynamics | 104 | A measured property of the frozen native field/update map, including exact ansätze and null controls |
| Validated State Dynamics | 34 | A measured manifested-state or selected reaction response |
| Qualified Selected Extensions | 1 | A measured toggle-gated extension whose surviving behavior and failed physical interpretation are both explicit |
| Validated Initial Data | 2 | A prepared geometry whose construction, not physical identity, is certified |
| Macroscopic Physics & Measurement | 1 | A bounded prepared-response probe with its macroscopic interpretation explicitly rejected where unsupported |
| **Total** | **142** | **All catalog entries admitted with behavioral evidence** |

The registry is the complete per-scenario run-of-record manifest. It contains
one qualification, one test location, and one concrete assertion for every ID.

## 2. Closure result

- Menu, internal catalog, JS implementations, C++ implementations, and evidence
  records are exactly 142/142/142/142/142.
- There are no hidden or mechanically smoke-only scenarios.
- 121 scenarios cite `engine/tests/test_scenario_behavior.cpp`; the remaining
  21 cite the focused boundary, genesis, reaction, dynamic-flux-dressing,
  reciprocal-moving-source, velocity, Thomson, and flux-cell tests recorded in
  the registry (9 distinct files).
- Cumulative browser evidence has loaded all 142 production WASM scenarios.
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
2. Store the 142-row mechanical campaign as a versioned JSON artifact, including
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

## Addendum 2026-09-02 — `s0-cell-*` flux-cell group (12 ids, 130 → 142)

Seven flux-cell scenarios were added under the same admission rule. A flux
cell is a localized field configuration whose energy is meant to stay above
vacuum after its pump is disconnected; the group implements the V0–V2 build
order of the flux-battery programme (a Gauss-charged plate capacitor, an
azimuthal ring reservoir with four controls, and a three-axis standing-arm
cell). Every id seeds initial data on an isolated profile; storage, hold, and
leakage are measured by `engine/tests/test_flux_cell_scenario_physics.cpp`
through the regional ledger in `engine/include/ftd/flux_cell.h` (U_E, U_B,
U_J, the kick-drift Hamiltonian, Poynting leak, ring circulation, disk flux,
flux dyad, support radius). Measurements of record (L=33, 300-tick hold, CPU):

| Id | Certified | Measured status |
|---|---|---|
| `s0-cell-capacitor` | Gauss projection charges the gap (U_J 0 → 11.76 in one tick), plates inert | Under the wave map the gap rings and relaxes to 0.315 of its tick-1 value; capacitor identity [OPEN] |
| `s0-cell-torus` | H conserved to 5e-14; U_E reaches 0.714 of U_E+U_B; net Poynting < 6e-16; pump-off ledger closes (W_in = H_hold) | Ring region retains 0.363 = uniform-fill fraction 0.35: energy is held by the periodic box, not by the ring geometry |
| `s0-cell-torus-reverse` | Exact mirror (Γ_J, Φ_B equal and opposite to 1e-12) | Control |
| `s0-cell-torus-scrambled` | Identical pointwise \|J\|, Γ_J = 0, H 2.44× larger | **Closed negative** for “coherence aids retention”: retains 0.914 (2.52× the coherent ring) because zone-edge content has small lattice group velocity |
| `s0-cell-torus-open` | Dispersal law: H retention 2.5e-7 | No-membrane control; the periodic hold is box recurrence |
| `s0-cell-torus-walled` | Reflective law: interior H conserved to 2.5e-14 | Walls hold energy in the box, not in the ring (region retention 0.336) |
| `s0-cell-triad` | Equal axial flux moments (1.4e-16), zero net flux and Poynting (1e-16), H conserved to 4.4e-14 | Off-diagonal overlap terms 0.019 of the trace; arm region disperses to uniform fill 0.155 |
| `s0-cell-torus-membrane` | Clock-inclusive Hamiltonian conserved to 3.3e-14; inner ball retains 1.003 (uniform fill 0.199); transparent control (ω₀=0.05) falls to 0.223; thickness 1/2/3 retains 0.576/0.991/1.003 | The locked shell is a mass-gap wall only through the [IMPOSED] de Broglie clock; storage identity [OPEN] |
| `s0-cell-torus-membrane-gated` | Zero port flux before tick 150; 79 shell sites expire on schedule; W_out=0.03808 through the plug’s outer face against a whole-cell Hamiltonian loss of 0.0457 (0.834 raw, 1.05 after subtracting the closed-phase wall leak of −0.58%) | Port ledger uses the wave-Hamiltonian current c²ΣE_a∇J_a; the Poynting integral (0.03390) is kept as a diagnostic |
| `s0-cell-membrane-pumped` | Engine-booked pump work W_in=0.049280 equals the Hamiltonian at disconnection (1e-9); 20 increments then hard-off; hold drift 6e-14; retention 0.835 | Twenty time-shifted increments add incoherently (0.049 vs the 1.62 one-shot seed); the ledger books what was delivered |
| `s0-cell-membrane-transfer` | L=49: receiver Hamiltonian exactly 0 and zero port flux until tick 100; then A loses 0.0530, B gains 0.0137 (0.0133 of A’s start), W_port=0.0195 across the contact plane | Two-way channel between two leaking cells; B keeps 0.70 of what crossed; no receiver identity |
| `s0-cell-membrane-pumped-resonant` | Scan of W_in over spacings 1–24 ticks: constructive optimum at 8 (1.37× every-tick), destructive at 3–4 (0.10×); W_in=0.06742 = H at disconnection; count-scaling exponent 1.16 | Phase dependence of −2J·Lδ confirmed; far below coherent N² (leaky multi-mode reservoir) |

The three membrane-family ids ride on three engine additions in
`engine/include/ftd/flux_cell.h`, each with a stated mechanism: the membrane is
the existing [IMPOSED] de Broglie clock term evaluated on an imposed locked shell
(a Klein–Gordon mass gap, evanescent for ω < ω₀ exactly as a metal reflects
light below its plasma frequency); the pump (`flux_pump`) is a time-gated source
term whose work is booked exactly per tick from the bilinear change of the
kick-drift Hamiltonian; the port (`flux_cell_port`) is a scheduled P5-style
expiry of shell sites whose outgoing energy is integrated with the wave
equation’s own energy current c²Σ_a E_a∇J_a (the EM-like c²E×B differs by a
curl-type term and is kept as a diagnostic). Both toggles default OFF
(golden-neutral) and are classified host-mirror hybrids on CUDA.

The flux-cell ledger was also turned on the engine’s own electron seeds as the
spec’s “electron as a self-confined flux cell” falsifier: `s0-vacuum-electron`
(curl-free radial J, no wall, no clock) disperses to uniform fill (region
Hamiltonian retention 0.073 vs fill 0.050) with net Poynting circulation and
angular-flow moment at rounding level, and the clocked 7×7×7 block of
`s0-seed-de-broglie-clock` radiates its k=0 oscillation away (0.046 vs 0.026).
Both are **closed negatives** for a self-confined flux cell in the current
engine; the measurement is recorded in `test_flux_cell_scenario_physics`.

None of the twelve asserts a capacitor, inductor, battery, persistent current,
matter clock, or particle identity. The phase-winding gate with packetized
discharge (V3) and the charged-versus-empty inertia/gravity comparison (V4)
are not scenarios: V3 needs a finite phase carrier and a gated transaction the
engine does not have, V4 is a measurement campaign. Class-table effect:
Validated Native Dynamics 92 → 104; the twelve cite the new test file (the
`test_scenario_behavior.cpp` count stays 121).
