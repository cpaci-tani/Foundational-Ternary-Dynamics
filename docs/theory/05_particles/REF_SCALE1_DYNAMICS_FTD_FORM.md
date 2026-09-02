# REF — Scale-1 Particle Dynamics, in FTD-Native Form

**Tag:** `[REFERENCE]`
**Status:** `[REFERENCE]` — updated 2026-09-01 for the v4 Scale-1 particle context: a code-grounded cross-walk of the read-only native observer, canonical scenario manifest, and effective continuous-particle reference laboratory.
**Scope:** the shared Scale-1 domain (`engine/include/ftd/scale1/domain.h`), native `ParticleEngine`, native/WASM adapters, and Scale-1 web modules. Scale 0 dynamics and Scale 2/3 dynamics remain out of scope; runtime scale handoff is retired.
**Supersedes:** the pre-revision edition of this document, which described the retired pure-JS engine (`mock-particle-engine.js`), the 26-scenario `pe-*` library, and the cross-sections/decay-rates/spectroscopy analysis panels — all deleted 2026-07-29 (see `docs/audits/AUDIT_2026-07_scale1-particle-engine.md` for why).

> **Epistemic banner (read first).** Re-expressing a textbook formula in FTD constants (`α → G_C²`, `m_e → K_B`, `c → 1/√3`, …) is **notation, not derivation.** The Scale-1 *dynamical laws* are imported physics (Coulomb, Newton, Velocity-Verlet); only the **constants plugged into them** are FTD quantities. The epistemic tags below describe the *law*, and are **unchanged** by the FTD-form rewrite. Conflict precedence: LEDGER > this doc.

---

## §0 · One-line summary

Scale 1 is the particle context immediately above Scale 0. One scenario selector exposes the complete runnable program. The dynamics owner remains an independent field on every scenario: a read-only native observer, effective `ParticleEngine`, or parametric catalog. Only effective-reference scenarios advance the analytical point-particle engine; they are not the primitive substrate and do not establish Standard Model particle emergence, quantum statistics, or QFT recovery. The Scale Context sidepanel supplies pedagogical scale comparison without transferring state.

## §1 · FTD constant substitution key (engine-defined)

| Textbook symbol | FTD-native form | Engine source | Epistemic status |
|---|---|---|---|
| α (fine structure) | **G_C²** (`ALPHA_EFT = G_C·G_C`) = **1/x₊** ≈ 1/137.036 (master-quadratic root) | `constants.js` / `ftd/constants.h` | physical ID `[STRONGLY MOTIVATED CONJECTURE]` (FTD-0013) |
| electron mass m_e | **K_B** = 0.511 (mass anchor) | `constants.js` | engine anchor `[IMPOSED]`; physical relation `[SMC]` (FTD-0015) |
| speed of light c | **1/√3** (`C_SPEED`; the production 18-point stencil permits c ≤ √3/2, FTD-0407) | `constants.js` | `[SELECTION]` |
| gravity coupling | **G_PE = G_DERIVED = 1/(4π·m_P²)** ≈ 5.34×10⁻⁴⁶ MeV⁻² | `particle_engine.cpp:148`; `constants.js` | `[SMC]`-floored magnitude (FTD-0131). The legacy `1/(b₃+N_c)² = 1/100` identification is **FALSIFIED** (FTD-0131) and appears nowhere in Scale 1 |
| Coulomb prefactor | **G_C²/(4π)** | `particle_engine.cpp:136` | 1/r² **form** `[THEOREM]`-grade lattice geometry for r ≳ 8 (Phase G, `DERIV_EMERGENT_COULOMB_GEOMETRIC.md`); the α **coupling** `[PARAMETRIC]` |
| source cluster inertia convention | **N·K_B** (N = cluster voxel count) | Scale-0 cluster record / FTD-0110 scope | recorded substrate scaling; physical identification remains `[SMC]` |
| framework integers | N_c=3, N_base=4, b₃=7, N_eff=13 | `constants.js` | `[THEOREM]`/`[SELECTION]` |

## §2 · Dynamics owners and the effective engine

Scale 1 has two dynamics owners, not two competing force implementations:

- `NativeMatterObserver` publishes an immutable FTD-0760 registered M3 replay. It exposes two relational constituents, registered momentum summaries, field-channel availability, identity margins, provenance, and explicit unavailable claims. It cannot tick or accept particle injection, and does not invent mass, charge, spin, statistics, an SM identity, or an outgoing/radiation channel.
- `ParticleEngine` owns the Effective Lab. Native and WASM frontends consume its shared `Scale1Snapshot`; overlays and telemetry read the same native state the integrator advances.

The table-backed physics/capability/observer registries are the control and status source of truth for native and web clients. No runtime projector owns or transfers state between scales.

**Context boundary:** "discrete" at Scale 1 means a finite effective record transacted once per integer global tick. Position, velocity, mass, and analytical force evaluation are continuous floating-point **effective variables** `[IMPOSED representation]`; they are not a second primitive lattice and are not promoted to `[AXIOM]`. The native injection/control surface rejects nonfinite vectors, nonpositive mass or `dt`, and negative/nonfinite effective radius or softening; every tick validates the record before and after the transaction and fails closed rather than propagating an inadmissible state.

| Process | Law | Status | Source |
|---|---|---|---|
| Coulomb force | `F = −(G_C²/4π)·q₁q₂/r²` (softened) | form `[THEOREM]` (r ≳ 8), coupling `[PARAMETRIC]` | `particle_engine.cpp:136` |
| Gravity | `F = +G_PE·m₁m₂/r²` | `[SMC]`-floored magnitude (FTD-0131) | `particle_engine.cpp:148` |
| Integrator | Velocity-Verlet KDK (relativistic-momentum variant toggle) | `[IMPOSED]` (numerics) | `particle_engine.cpp:582+` |
| Light-speed cap | clamp `\|v\| ≤ 1/√3` | `[SELECTION]` (FTD-0407) | native speed-limit pass |
| Exchange / strong / Lorentz / magnetic-dipole / spin-orbit / radiation | toggle-gated advanced terms, excluded from the verified profile and state-energy completeness unless their missing channel is supplied | quarantined `[IMPOSED]` effective extensions | `particle_engine.cpp` + `scale1/domain.cpp` registry |
| Isotropic `relativistic` force rescale | unavailable; attempts to enable it fail closed | `[RETIRED]` non-covariant approximation (FTD-0401) | physics registry / toggle validation |
| Pair contact removal | deterministic closest-first disjoint selection, explicit event records, default OFF | `[SELECTION]` (not annihilation, decay, or a QED cross-section) | native contact-event pass |
| Boundary | **none** — the engine is unbounded; the r=35 sphere is a visual reference shell only | — | (deliberate revision change) |

## §3 · Runtime scale handoff retirement

The former "Project to Scale 1" toolbar action, browser capture pipeline,
native `Scale1Projector`, projection ledger, source-voxel comparison overlays,
and projection scenarios were removed on 2026-09-01. Their proximity and
display mappings were pedagogical `[IMPOSED]` conversions, not particle
emergence or a state-complete cross-scale transaction.

`NativeMatterObserver` remains a read-only scientific interface: it may inspect
a coherent source record without creating a `ParticleEngine` body. The disabled
live-capture scenario was removed with the rest of the non-executable registry
rows; the six qualified FTD-0760 replay views remain. Relative-scale pedagogy
belongs to the Scale Context sidepanel, which changes presentation only.

## §4 · Unified selector and canonical scenario manifest (39)

`scale1_scenario_registry()` in the shared C++ domain is the status source of
truth. WASM exposes the rows to the dashboard; the JavaScript registry owns
only setup handlers and presentation defaults keyed by the native `setup_id`.
Every row declares an internal provenance category, dynamics owner, scenario class, epistemic status,
canonical source, availability, expected observable, prohibited claim, backend,
performance class, validation verdict/evidence/criterion, and a native-owned
physics mask. Every live row is executable; research proposals and unsupported
capabilities are documented outside the scenario selector.

| scenario class | current role |
|---|---|
| `qualified_replay` | six views over the immutable FTD-0760 M3 record |
| `effective_reference` | thirty analytical comparison experiments advanced by `ParticleEngine` |
| `parametric_catalog` | three catalog-reference views with no emergence claim |

The live registry contains exactly thirty-nine runnable scenarios: six native
anatomy views, thirty effective reference experiments, and three parametric
catalog views. The former 48 unavailable placeholders were removed on
2026-09-01. Their scientific questions remain in the theory and audit record;
they are not represented as dashboard scenarios until they acquire an
executable owner and validation path.

Orbit initial conditions come from a native force-balance probe at t=0 (zero
velocity, read the kernel force, solve the imposed circular balance, then write
velocity). This is a numerical setup operation against the live effective
kernel, not a substrate derivation of an orbit.

### §4.1 · Quantum reference scenarios

The Quantum reference group contains twelve executable controls over
advanced terms that already exist in the effective `ParticleEngine`. They test
software behavior and selector dependence; they do not claim that Scale 1 owns
a wavefunction, Hilbert-space state, exchange statistics, quantized spin,
photons, amplitudes, or QFT. This preserves the FTD-1024 boundary that physical
Bell recovery, CAR/Fock structure, and native exchange statistics remain open.

| control family | executable comparisons | explicit boundary |
|---|---|---|
| exchange | eligible typed pair, spinless null, near/far range | an imposed eligibility-conditioned repulsion is not antisymmetrization or Pauli exclusion |
| spin-orbit | parallel and antiparallel injected spin axes | the imposed `L dot S` term is not a Dirac reduction or spin quantization |
| magnetic dipole | antiparallel and transverse axes | injected classical axes are not a derived Pauli moment or spin state |
| Lorentz | opposite-charge and opposite-velocity sign controls | a partner-dipole `v cross B` response is not photon/gauge-field recovery |
| radiation | accelerated Coulomb scattering with the sink isolated | an acceleration-dependent sink is not an emitted-photon record or bremsstrahlung amplitude |
| relativistic integrator | force-free counterstream | momentum-form numerical kinematics is not a quantum wave packet or Lorentz-group derivation |
| three-color toy | three neutral typed records spanning color labels 1, 2, and 3 | catalog labels and a pair kernel are not QCD, a gauge field, or confinement recovery |

The color control is intentionally electrically neutral: the current engine
record stores integer electric charge and therefore cannot faithfully encode
fractional quark charge. Its mass/type labels are parametric catalog inputs.

### §4.2 · QED reference scenarios

The QED reference group is an external/effective comparison surface, not a claim
that Scale 1 recovers quantum electrodynamics. Seven runnable scenarios expose
the sectors the present `ParticleEngine` can actually render:

| visible sector | implemented owner | explicit boundary |
|---|---|---|
| static Coulomb | softened effective pair kernel plus field/potential visualization | no photon-exchange amplitude |
| Moller-style same-sign scattering | classical effective Coulomb trajectories | no antisymmetrization, exchange amplitude, or cross-section |
| Bhabha-style opposite-sign scattering | elastic effective Coulomb trajectories | no annihilation channel or interference amplitude |
| magnetic dipole | imposed point-dipole kernel over injected spin axes | no derived Pauli moment |
| Lorentz response | imposed `v cross B` response sourced by a partner dipole | no dynamical photon field or closed field-energy ledger |
| spin-orbit | imposed `L dot S` force term | no Dirac reduction or fine-structure derivation |
| radiation reaction | imported acceleration-dependent sink | no emitted photon record, spectrum, or exact radiated-energy channel |

Compton scattering, electron-positron annihilation, pair production, vacuum
polarization, and loop observables (`g-2`/Lamb shift) are not scenarios. They
remain documented implementation/recovery boundaries because Scale 1 has no
photon degree, quantum amplitude owner, loop expansion, or complete event
receiver. Standard QED loop formulas with FTD inputs remain `[PARAMETRIC]`
catalog evidence only.

### §4.3 · What “validated” means

Validation never changes an epistemic tag. The shared registry distinguishes
`contract_qualified`, `kernel_validated`, `conditional_evidence`,
`boundary_confirmed`, `open_blocked`, and `invalid_retired`. Thus the Coulomb
and gravity implementations may pass exact pair-force and accounting tests
without deriving their physical couplings; an exchange toy may pass kernel
selectors while native exchange statistics remain open; and a closed-negative
motion or gravity campaign is validated by preserving its obstruction, not by
turning it into an enabled positive demonstration.

All twelve physics rows have an evidence target and pass criterion. All 34
scenario rows have the same and are executable. The six views of the registered
M3 artifact are one read-only replay with six observation subviews, not six
different dynamics scenarios. For each row, the native mask overrides frontend toggle
defaults. The browser qualification sweep loads the scenario, confirms the
exact toggle profile, advances interactive records through four transactions,
checks finite diagnostics, and confirms Native Matter remains immutable. The
complete effective scenario profiles are advertised only for CPU and the three
WASM build targets; the browser-selected WASM path receives the live runtime
sweep and all three variants must compile. Individual CUDA pair kernels do not
yet constitute an end-to-end CUDA-qualified scenario.

## §5 · Snapshot and telemetry honesty rules

- Native and WASM publish the same schema/registry revision, provenance records, forces, events, and capabilities.
- State energy carries covered, missing, and non-conservative masks. Drift is shown only when every active state-energy channel is represented and no active sink/source invalidates a conservative comparison.
- Energy-drift baseline **re-latches** whenever the particle count or toggle set changes; enabling a quarantined force makes the ledger incomplete rather than silently calling the partial sum total energy.
- Momentum, angular momentum, and force readouts are **sim units** (no MeV/c, ħ, or Planck-unit labels — no β=v/C_SPEED-style conversion exists in the engine, FTD-0401); velocity readouts labeled `c` are genuine β = v/C_SPEED ratios computed in the hub.
- Angular momentum in both diagnostics and the System overlay is the native origin-frame quantity; no center-of-mass angular momentum is reconstructed in JavaScript.
- Native Matter publishes unavailable values as unavailable. Its kinetic/mass channels and outgoing field are not replaced by zeros carrying physical meaning.
- Contact counts come from explicit native event records, never inferred particle-count drops.
- Chart pushes advance on engine-tick progress only — paused sims do not overwrite history.
- The Physics card is mounted inside the Controls side panel and hydrated from the available entries in the shared native registry; it is not a second applicability table. Native Matter disables every mutating switch, scenario loads restore the registered profile, and manual edits are visibly marked modified. Bulk profiles operate on registry flags: **Verified** selects the two verified-baseline rows and **All applicable** selects all eleven implemented rows. The retired isotropic rescale remains a fail-closed native registry record for provenance, but it is intentionally omitted from the Controls UI. Scenario behavior, validation evidence, source/artifact revision, object provenance, and observation boundaries are presented in a collapsed **Scenario details** disclosure in Controls. Diagnostics contains runtime snapshot and history telemetry rather than scenario settings or static provenance rows; active unavailable-claim telemetry remains in the state-energy coverage ledger, and every row carries an explanatory hover overlay.
- The viewport force overlay consumes all nine published decomposition channels (Coulomb, gravity, Lorentz, exchange, strong, radiation, magnetic dipole, spin-orbit, and net). Colors and visual gains are presentation only and do not change force ownership or epistemic status.

## §6 · Retired with the 2026-07-29 revision (do not cite as live)

`mock-particle-engine.js`, `pe-force-kernel.js`, `pe-spin-dynamics.js` (the JS engine); `scales/scale1/scenarios.js` + `pe-dynamics.js` (26 pe-* scenarios incl. the Hawking micro-BH toy); `cross-sections.js`, `decay-rates.js` (parametric analysis panels); `pe-telemetry.js` (legacy canvas panel). `spectroscopy.js` survives solely for the Scale-0 hydrogen p1-observable. `particle-catalog.js` survives solely for the Zoo (`[PARAMETRIC]`; its `ftd_status` column copies LEDGER tags, never promotes them).

## §7 · Source modules & cross-references

**Engine:** `engine/include/ftd/scale1/domain.h` + `engine/src/scale1/domain.cpp` (shared schema, registries, registered replay, and read-only observer), `engine/src/particle_engine.cpp` + `engine/include/ftd/particle_engine.h` (effective kernel), `engine/wasm/bindings_particle.cpp` (shared snapshot/registry bindings), and `engine/native/src/host/adapters/scale1_adapter.cpp` (native owner adapter). No runtime cross-scale conversion module remains.
**Web:** `engine/web/js/bridge/native-particle-engine.js` (adapter), `scales/scale1/{controller,scenario-registry}.js`, `scales/scale1/state/store.js`, `bridge/pe-catalog-map.js`, `zoo.js`, and the Scale Context sidepanel.
**Canonical FTD references:** LEDGER rows FTD-0013 (α), FTD-0015 (m_e), FTD-0110 (N·K_B mass law), FTD-0131 (G_PE; 1/100 falsified), FTD-0401 (dual velocity normalization no-go), FTD-0407 (C_SPEED selection); `DERIV_EMERGENT_COULOMB_GEOMETRIC.md` (Phase G geometric Coulomb); `DERIV_MATERIAL_EMERGENCE_FROM_LATTICE.md` (genesis produces hybrid objects); `docs/audits/AUDIT_2026-07_scale1-particle-engine.md` (the audit that drove the revision).
