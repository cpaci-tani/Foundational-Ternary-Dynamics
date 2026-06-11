# SPEC: Vacuum Particle Scenarios (`s0-vacuum-*` family)

**Status:** Draft 2026-04-28
**Owner:** William
**Companion:** existing scenario system at `engine/web/js/bridge/scenarios/`,
existing C++ scenarios at `engine/src/scenarios/`,
parity guard `engine/web/tests/scenario-parity.spec.js`.

## Purpose

A curated catalog of **15 scenarios** showcasing each canonical
elementary or near-elementary particle as a single isolated entity in
the existing main lattice viewport. Each scenario is a standard
MockBridge scenario with the standard live-telemetry pipeline — no new
UI, no new viewport, no new panel. The user picks one from the existing
scenario dropdown and the engine renders the particle's volumetric
shape on the lattice.

Distinguished from the existing `s0-seed-*` family by uniform vacuum
boundary conditions and uniform lattice configuration, so the 15
scenarios act as a comparable set rather than a research grab-bag.

## Catalog

| # | Scenario name | Particle | Status | Source |
|---|---|---|---|---|
| 1 | `s0-vacuum-electron` | electron e⁻ | wrap existing | s0-seed-electron |
| 2 | `s0-vacuum-muon` | muon μ⁻ | wrap existing | s0-seed-muon |
| 3 | `s0-vacuum-tau` | tau τ⁻ | wrap existing | s0-seed-tau |
| 4 | `s0-vacuum-electron-neutrino` | ν_e | **new** | flavor=e (1.0× baseline amplitude) |
| 5 | `s0-vacuum-muon-neutrino` | ν_μ | **new** | flavor=μ (1.3× amplitude) |
| 6 | `s0-vacuum-tau-neutrino` | ν_τ | **new** | flavor=τ (1.6× amplitude) |
| 7 | `s0-vacuum-photon` | γ | wrap existing | s0-seed-photon |
| 8 | `s0-vacuum-w-boson` | W± | wrap existing | s0-seed-w-boson |
| 9 | `s0-vacuum-z-boson` | Z⁰ | wrap existing | s0-seed-z-boson |
| 10 | `s0-vacuum-higgs` | H | wrap existing | s0-seed-higgs-boson |
| 11 | `s0-vacuum-proton` | p | wrap existing | s0-seed-proton-l4 |
| 12 | `s0-vacuum-neutron` | n | wrap existing | s0-seed-neutron |
| 13 | `s0-vacuum-pion-charged` | π± | wrap existing | s0-seed-pion (charge=+1) |
| 14 | `s0-vacuum-pion-neutral` | π⁰ | **new** | derived from s0-seed-pion with charge=0 + 2γ decay coupling |
| 15 | `s0-vacuum-kaon-charged` | K± | **new** | pion injector with elevated amplitude (parametric — chosen so the manifested cluster size matches FTD-0110's m_K prediction at ~5%) |

**10 wrappers + 5 new scenarios** = 5 net-new injectors in the catalog (electron-neutrino, muon-neutrino, tau-neutrino, π⁰, K±).

## Uniform configuration

Every `s0-vacuum-*` scenario applies the same vacuum environment before
particle injection. This is what distinguishes them from `s0-seed-*`:

| Setting | Value | Rationale |
|---|---|---|
| Lattice size | Respects user-set `L`; recommended `L ≥ 64` documented in scenario description | Scenarios do not change lattice size on load (matches existing convention); user controls L through the standard UI |
| Boundaries | Periodic (current engine default; modular neighbor wrapping in phase_read) | The `absorbing_boundary` toggle does not exist as a discrete toggle in current term_toggles.h despite CLAUDE.md narrative. Periodic + L ≥ 64 keeps image-charge artifacts ≤ ~few% over 1000 ticks. Adding a true absorbing-boundary toggle is a separate spec. |
| Initial flux | Zero everywhere (`this.reset()`) | Clean vacuum baseline |
| Particle position | Lattice center (uses existing `mid` from `ctx`) | Maximum distance from boundaries; uniform across catalog |
| Particle velocity | Zero (rest frame) | Static observables read cleanly; decay products radiate isotropically |
| Other particles | None | "Single particle in vacuum" |

The MockBridge scenario function calls a new helper `applyVacuumEnvironment(ctx)`
before delegating to the per-particle injection logic. In v1, this helper is
effectively a no-op beyond the standard `this.reset()` (which is already done
by the dispatcher in `index.js`); it exists as a documented extension point so
that when an `absorbing_boundary` toggle is added in a future spec, only this
one helper changes — not all 15 scenarios.

## Physical meaning of "particle visual shape"

The scenario does NOT prescribe overlay state — the user controls the
field-renderer toggles like in any other scenario. What each scenario
DOES prescribe is the **initial condition** that produces the
particle's volumetric signature when rendered:

- **electron** — point-like charge density at center; static Coulomb field develops outward
- **photon** — running EM wavepacket along ẑ; Poynting overlay reveals propagation if user enables it
- **muon** — same as electron; decays after ~τ_μ engine ticks; manifest events log decay products
- **neutrinos** — minimal flux thread at center with weak-only coupling toggle; faint by design (telemetry shows the small charge integral)
- **proton/neutron** — composite color-triplet seed (already implemented in s0-seed-proton-l4)
- **W/Z/H** — high-amplitude unstable seed; decays within ~tens of ticks
- **π⁰** — neutral 2-component oscillator that couples to the EM channel; decays to 2γ radiating outward
- **K±** — pion-style injector with elevated amplitude. The FTD-0110 cluster-sizemass map (`A = 2·√(m/m_e)`) gives the K-amplitude. **[PARAMETRIC]**: the kaon mass itself is not derived in FTD; the amplitude is set to reproduce the observed mass via the FTD-0110 map.

The "shape" the user sees is whatever the engine produces — the existing
volumetric flux/charge density renderer is the answer. Atlas does not
override it.

## Live telemetry

Same as every other scenario. The existing diagnostics pipeline
(`getDiagnostics()`, `getEnergyAudit()`, `getManifestEvents()`,
`getParticleStateHistogram()`) reports on the particle in flight.
For unstable particles, the manifest-event stream captures decay
events and the telemetry charts plot decay-product trajectories.

No new telemetry channels. No new readout panels.

## Naming

Prefix `s0-vacuum-` is new and distinct from `s0-seed-` (research
seeds with arbitrary lattice/boundary configurations). Keeps the
two families separable in the scenario dropdown and in CI parity.

## File layout

| File | Status | Purpose |
|---|---|---|
| `engine/web/js/bridge/scenarios/vacuum-scenarios.js` | **new** | `setupVacuumScenario(name, ctx)` — prefix-dispatch group file, mirrors existing `s0-seed-scenarios.js` |
| `engine/web/js/bridge/scenarios/index.js` | edit | register `setupVacuumScenario` in dispatch chain |
| `engine/web/js/bridge/scenarios/_helpers.js` | edit | add `applyVacuumEnvironment(ctx)` helper |
| `engine/src/scenarios/vacuum.cpp` | **new** | C++ mirror of vacuum-scenarios.js |
| `engine/include/ftd/scenarios.h` | edit | declare `setupVacuumScenario` |
| `engine/src/scenarios/_helpers.h` | edit | add C++ `applyVacuumEnvironment` |
| `engine/web/tests/scenario-parity.spec.js` | edit | extend parity guard to cover the 15 new scenarios |
| `engine/web/js/scenarios.js` (catalog/UI dropdown) | edit | register 15 new entries in the menu under "Vacuum particles" group |

## Out of scope (v1)

- **Absorbing-boundary toggle.** Currently does not exist as a
  discrete toggle (per audit 2026-04-28). Periodic boundaries are used.
  A future spec can add an `absorbing_boundary` term_toggle that drains
  flux + wave_vel in an N-voxel-thick boundary shell; vacuum scenarios
  would opt in once available.
- **Verify button / scoreboard hooks.** The existing Verify panel
  (`SPEC_VERIFICATION_LAB.md`) already provides the static evidence
  scoreboard. Atlas scenarios are *live* simulations; they don't post
  to the scoreboard. If a future verify integration is wanted, that's a
  separate spec.
- **Per-particle overlay defaults.** User controls overlays.
- **Side-by-side comparison view.** Single viewport only.
- **Quark/gluon scenarios.** These can't exist alone in vacuum
  (confinement). The existing `s0-seed-{up,down,...}-quark` scenarios
  remain in the seed family for research use; they are not promoted to
  the vacuum family. A future `s0-bound-*` family for confined states
  is a separate spec.
- **Antiparticles.** The existing `s0-seed-positron` and antiquark
  scenarios remain in the seed family. The vacuum family is one
  representative per particle species; charge-conjugates can be added
  in a v2 if the catalog wants symmetry.

## Acceptance criteria

1. All 15 `s0-vacuum-*` scenarios appear in the scenario dropdown,
   loadable without errors.
2. Each scenario produces a non-trivial volumetric signature in the
   default viewport overlay set within 10 ticks of load.
3. `engine/web/tests/scenario-parity.spec.js` passes — every
   `s0-vacuum-*` JS scenario has a C++ counterpart.
4. Decaying particles produce manifest events visible in the existing
   particle-events telemetry within their expected lifetime
   (e.g. muon decays within ~few hundred ticks; Higgs within ~tens).
5. Stable particles (electron, photon, proton, neutrino) maintain
   their initial particle count for at least 1000 ticks at L=64 with
   absorbing boundaries on.
6. Total energy is finite and positive at tick 1000 for all 15
   scenarios.

## Implementation order (proposed)

1. `_helpers.js` + `_helpers.h` — `applyVacuumEnvironment`
2. JS `vacuum-scenarios.js` — 12 wrappers (delegate to existing inject helpers post-environment-apply)
3. JS — 5 new injectors (3 neutrino flavors, π⁰, K±)
4. C++ mirror in `engine/src/scenarios/vacuum.cpp`
5. Scenario dropdown menu entries
6. Parity guard extension
7. Smoke verification: load each scenario, run 1000 ticks headless, check acceptance criteria 4-6

Five steps cleanly split into commits.
