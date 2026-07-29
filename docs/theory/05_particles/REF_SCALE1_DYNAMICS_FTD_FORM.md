# REF — Scale-1 Particle Dynamics, in FTD-Native Form

**Tag:** `[REFERENCE]`
**Status:** `[REFERENCE]` — a code-grounded cross-walk of every dynamic the web dashboard's **Scale 1** computes after the 2026-07-29 revision (continuous particle system promoted from the lattice, native C++/WASM engine), with each formula re-expressed in FTD's own constants where one genuinely exists.
**Scope:** the native `ParticleEngine` (`engine/src/particle_engine.cpp`, reached from `engine/web/js/` via the embind adapter `bridge/native-particle-engine.js`) plus the Scale-1 web modules. Scale 0 (lattice/substrate) and Scale 2/3 (atoms/molecules) are out of scope.
**Supersedes:** the pre-revision edition of this document, which described the retired pure-JS engine (`mock-particle-engine.js`), the 26-scenario `pe-*` library, and the cross-sections/decay-rates/spectroscopy analysis panels — all deleted 2026-07-29 (see `docs/audits/AUDIT_2026-07_scale1-particle-engine.md` for why).

> **Epistemic banner (read first).** Re-expressing a textbook formula in FTD constants (`α → G_C²`, `m_e → K_B`, `c → 1/√3`, …) is **notation, not derivation.** The Scale-1 *dynamical laws* are imported physics (Coulomb, Newton, Velocity-Verlet); only the **constants plugged into them** are FTD quantities, and only the cluster promotion *mapping* carries FTD-derived content at its recorded tags. The epistemic tags below describe the *law*, and are **unchanged** by the FTD-form rewrite. Conflict precedence: LEDGER > this doc.

---

## §0 · One-line summary

Scale 1 is a continuous-coordinate N-body system whose particles arrive by **coarse-graining the live Scale-0 lattice** (one particle per manifested cluster, mass = N·K_B) or from the `[PARAMETRIC]` catalog Zoo; the dynamics are imported classical mechanics (native Velocity-Verlet) parameterized by `G_C²` (α), `K_B` (m_e), `1/√3` (c, `[SELECTION]`), and `G_PE = 1/(4π·m_P²)` (FTD-0131).

## §1 · FTD constant substitution key (engine-defined)

| Textbook symbol | FTD-native form | Engine source | Epistemic status |
|---|---|---|---|
| α (fine structure) | **G_C²** (`ALPHA_EFT = G_C·G_C`) = **1/x₊** ≈ 1/137.036 (master-quadratic root) | `constants.js` / `ftd/constants.h` | physical ID `[STRONGLY MOTIVATED CONJECTURE]` (FTD-0013) |
| electron mass m_e | **K_B** = 0.511 (mass anchor) | `constants.js` | engine anchor `[IMPOSED]`; physical relation `[SMC]` (FTD-0015) |
| speed of light c | **1/√3** (`C_SPEED`; the production 18-point stencil permits c ≤ √3/2, FTD-0407) | `constants.js` | `[SELECTION]` |
| gravity coupling | **G_PE = G_DERIVED = 1/(4π·m_P²)** ≈ 5.34×10⁻⁴⁶ MeV⁻² | `particle_engine.cpp:148`; `constants.js` | `[SMC]`-floored magnitude (FTD-0131). The legacy `1/(b₃+N_c)² = 1/100` identification is **FALSIFIED** (FTD-0131) and appears nowhere in Scale 1 |
| Coulomb prefactor | **G_C²/(4π)** | `particle_engine.cpp:136` | 1/r² **form** `[THEOREM]`-grade lattice geometry for r ≳ 8 (Phase G, `DERIV_EMERGENT_COULOMB_GEOMETRIC.md`); the α **coupling** `[PARAMETRIC]` |
| cluster mass | **N·K_B** (N = cluster voxel count) | promotion pipeline; matches `phase_forces_integrate_clusters` (`phase_forces.cpp:335`) | `[DERIVED-linear]`/`[SMC]` (FTD-0110) |
| framework integers | N_c=3, N_base=4, b₃=7, N_eff=13 | `constants.js` | `[THEOREM]`/`[SELECTION]` |

## §2 · The engine (native C++/WASM)

The sole Scale-1 backend is the native `ParticleEngine` (`engine/src/particle_engine.cpp`, CTest-covered: `test_particle_engine`, `test_pe_forces`, `test_particle_toggles`, `test_particle_lifetime`), reached through embind bindings (`engine/wasm/bindings_particle.cpp`) and the JS adapter `bridge/native-particle-engine.js`. The former pure-JS engine is deleted; there is no second implementation of the force law anywhere in the web tree — overlays and telemetry read the same native kernel the integrator runs.

| Process | Law | Status | Source |
|---|---|---|---|
| Coulomb force | `F = −(G_C²/4π)·q₁q₂/r²` (softened) | form `[THEOREM]` (r ≳ 8), coupling `[PARAMETRIC]` | `particle_engine.cpp:136` |
| Gravity | `F = +G_PE·m₁m₂/r²` | `[SMC]`-floored magnitude (FTD-0131) | `particle_engine.cpp:148` |
| Integrator | Velocity-Verlet KDK (relativistic-momentum variant toggle) | `[IMPOSED]` (numerics) | `particle_engine.cpp:582+` |
| Light-speed cap | clamp `\|v\| ≤ 1/√3` | `[SELECTION]` (FTD-0407) | native speed-limit pass |
| Exchange / strong / Lorentz / magnetic-dipole / spin-orbit / radiation / relativistic | toggle-gated advanced terms | `[IMPOSED]` toys; relativistic rescale explicitly non-covariant (no covariant EOM exists, FTD-0401) | `particle_engine.cpp:158–318` |
| Pair annihilation | geometric contact → remove | `[SELECTION]` (not a QED cross-section) | native annihilation pass |
| Boundary | **none** — the engine is unbounded; the r=35 sphere is a visual reference shell only | — | (deliberate revision change) |

## §3 · The promotion pipeline ("⤴ Scale up") — the FTD-bearing content

`engine/web/js/scales/scale1/promotion.js`. Captures the live Scale-0 lattice's clusters and promotes each to one continuous particle. Cluster source: KnotTracker telemetry (`getKnotTelemetry`, production-wired observation) when it reports anything; else Moore-26 connected components over the `coarsenToParticles` voxel snapshot (covers clusters below the tracker's `min_cluster_size = 4`).

| Promoted quantity | Mapping | Status |
|---|---|---|
| position | cluster centroid, re-centered to the PE origin frame; uniform display scale for L ≥ 65 | `[DERIVED from telemetry]` + `[IMPOSED display mapping]` |
| velocity | cluster centroid velocity | `[DERIVED from telemetry]` |
| mass | **N·K_B** (N = voxel count) | `[DERIVED-linear]`/`[SMC]` (FTD-0110) — the engine's own cluster-inertia convention |
| charge | sign·N, clamped to int8 ±127 (clamps surfaced in the UI) | `[DERIVED from telemetry]` |
| spin / color | 0 (cluster telemetry carries neither) | — |
| admissibility | annotated (never gating): r_eff/a ≥ 3 ⇒ N ≳ 113 + sub-relativistic centroid speed, a JS heuristic of the ScaleContextTracker criteria | `[REFERENCE]` heuristic |

**Voxel-mass convention tension (recorded, not hidden):** the native scale bridge (`scale_bridge.cpp:37`) stamps per-voxel `mass = max(density, K_B)` — a flux-density convention — while the physics phases and this promotion path use **N·K_B**. The voxel debug ghost layer displays the scale-bridge value verbatim, labeled `[IMPOSED, display only]`; it never feeds dynamics. Reconciling the two conventions is an engine-side `[OPEN]` item.

**No SM identification.** Promoted objects are lattice clusters, not electrons or protons — lattice genesis produces hybrid colored objects (`DERIV_MATERIAL_EMERGENCE_FROM_LATTICE.md`), and no UI copy claims otherwise. SM catalog particles enter only via the Zoo, explicitly labeled `[PARAMETRIC]`.

## §4 · Scenarios (registry-driven, 6)

`engine/web/js/scales/scale1/scenario-registry.js` — each carries its epistemic description into the toolbar's status readout.

| id | content | key tags |
|---|---|---|
| `s1-promoted-lattice` | consumes the ⤴ Scale-up capture | §3 table |
| `s1-voxel-debug` | same + per-voxel ghost layer | §3 + `[IMPOSED display]` |
| `s1-coulomb-orbit` | −1 orbiting +1 at r=12, native force-balance IC | 1/r² form `[THEOREM]` window demo; α `[PARAMETRIC]`; IC `[IMPOSED]` |
| `s1-cluster-pair` | synthetic ±N clusters (N=20), mutual orbit | mass law `[DERIVED-linear]`/`[SMC]` |
| `s1-three-body` | three dynamic bodies, chaotic | ICs `[IMPOSED]` |
| `s1-empty-zoo` | empty; Zoo injects catalog particles | `[PARAMETRIC]` extras |

Orbit ICs come from a native force-balance probe at t=0 (zero the velocity, read the kernel force, solve m·v²/r = |F_inward|) — the same "ICs derived from the live kernel, not closed-form" contract as before, now against the kernel that actually integrates.

## §5 · Telemetry honesty rules (2026-07 revision)

- Energy-drift baseline **re-latches** whenever the particle count or toggle set changes (a changed Hamiltonian invalidates the old baseline); drift is integrator error, never unaccounted physics.
- Native diagnostics `totalPE` sums **active potential terms only** — labeled as such in every panel.
- Momentum, angular momentum, and force readouts are **sim units** (no MeV/c, ħ, or Planck-unit labels — no β=v/C_SPEED-style conversion exists in the engine, FTD-0401); velocity readouts labeled `c` are genuine β = v/C_SPEED ratios computed in the hub.
- Angular momentum in the diagnostics table is **about the origin** (native convention); the viewport System overlay's L is about the CoM and labeled "L (CoM)".
- No fabricated channels: the annihilation counter was retired rather than derived from count drops (which would conflate removal causes).
- Chart pushes advance on engine-tick progress only — paused sims do not overwrite history.

## §6 · Retired with the 2026-07-29 revision (do not cite as live)

`mock-particle-engine.js`, `pe-force-kernel.js`, `pe-spin-dynamics.js` (the JS engine); `scales/scale1/scenarios.js` + `pe-dynamics.js` (26 pe-* scenarios incl. the Hawking micro-BH toy); `cross-sections.js`, `decay-rates.js` (parametric analysis panels); `pe-telemetry.js` (legacy canvas panel). `spectroscopy.js` survives solely for the Scale-0 hydrogen p1-observable. `particle-catalog.js` survives solely for the Zoo (`[PARAMETRIC]`; its `ftd_status` column copies LEDGER tags, never promotes them).

## §7 · Source modules & cross-references

**Engine:** `engine/src/particle_engine.cpp` + `engine/include/ftd/particle_engine.h` (kernel), `engine/wasm/bindings_particle.cpp` (bindings incl. `peAddParticleEx`, `getPEForceDecomposition` (Float64), `coarsenToParticles`), `engine/src/scale_bridge.cpp` (voxel-level map).
**Web:** `engine/web/js/bridge/native-particle-engine.js` (adapter), `scales/scale1/{controller,promotion,scenario-registry}.js`, `scales/scale1/state/store.js`, `bridge/pe-catalog-map.js`, `zoo.js`.
**Canonical FTD references:** LEDGER rows FTD-0013 (α), FTD-0015 (m_e), FTD-0110 (N·K_B mass law), FTD-0131 (G_PE; 1/100 falsified), FTD-0401 (dual velocity normalization no-go), FTD-0407 (C_SPEED selection); `DERIV_EMERGENT_COULOMB_GEOMETRIC.md` (Phase G geometric Coulomb); `DERIV_MATERIAL_EMERGENCE_FROM_LATTICE.md` (genesis produces hybrid objects); `docs/audits/AUDIT_2026-07_scale1-particle-engine.md` (the audit that drove the revision).
