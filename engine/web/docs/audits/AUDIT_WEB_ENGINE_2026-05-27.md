# FTD Web Engine — Master Audit (2026-05-27)

Audit-only pass over `engine/web/` covering scientific accuracy, visualization
fidelity, performance, control wiring, and consolidation. Produced by 12
agents in parallel (7 scale experts + 5 cross-cutting), then synthesized.
No code edits performed.

**Audit perimeter:** `engine/web/` (JS, HTML, CSS, shaders, tests, WASM-bridge
boundary on the JS side). The C++/CUDA engine and Python scripts are cited
by path only.

**Key surface facts:**
- Sole HTML entry is `engine/web/index.html`. `index_dag.html` does NOT exist
  on disk (the "DAG" suffix in `app.js` / `bridge-init.js` /
  `bridge-factory.js` is residue from the reconciled pre-2026-04-27 split).
- Active scales: 0 lattice, 1 particle, 2 atom, 3 molecular, 4 planetary,
  5 cosmic, 6 meta. Scale 11 deletion confirmed clean in JS source; doc and
  test-helper references are stale.
- Single source of truth for web constants: `engine/web/js/constants.js`
  (487 LOC, well organized; one P0 cross-language drift documented in §G).

---

## Prioritized ticket list

### P0 — correctness bugs and load-bearing epistemic overclaims

| # | Ticket | File:line |
|---|---|---|
| P0-1 | Scale 4 uses lattice `G_N = 0.01` for heliocentric AU·M_sun·yr scenarios → Earth year wrong by factor ~63×. `G_HELIOCENTRIC = 4π²` exists in `constants.js:275` and is never imported. | `bridge/mock-scale4.js:35,66` |
| P0-2 | `PROTON_RATIO` formula drift JS  C++. JS (canonical, FTD-0016): `N_eff/α + N_base·N_eff + N_c ≈ 1836.47`. C++ + `proof_complete_sm.py`: pre-F9 wrong formula ≈ 3520 (1.91× too large). `engine/tests/campaign_triad_binding.cpp:154-159` will silently fail. | JS `constants.js:101` vs C++ `engine/include/ftd/ontic/particle_masses.h:52-55` |
| P0-3 | Telemetry hub Hubble key mismatch — Scale-5 emits `hubbleParameter`, hub reads `hubble`/`hubbleParam`. `csHubble` ring buffer is dead-on-arrival. | `telemetry-hub.js:324` vs `bridge/mock-scale5.js:307` |
| P0-4 | `WebSocketBridge` has no `bridge.capabilities` getter installed; `installCapabilityGetter` is only mounted on Mock + Wasm prototypes. `scales/scale0/runtime/tick.js:19` will throw `TypeError` on native-GPU mode. | `ws-bridge.js` (missing surface) + `bridge-init.js:36-37` |
| P0-5 | Scale 4 `setInterval` captures `ctx.running` from snapshot — planetary mode cannot be paused after load. | `scales/scale4/controller.js:65-88` |
| P0-6 | Scale 5 telemetry labels lattice mass with `M☉` glyph without multiplying by `LATTICE_TO_SOLAR_MASS = 50`. Reported masses are 50× off. | `bridge/mock-scale5.js:153,175,184,191` |
| P0-7 | Scale 5 cosmic info panel claims `r_s = 2 G_N M`; engine actually renders `cbrt(M) · 0.35`. Panel formula is wrong for what the visualization shows. | `ui/components/panel-resources/template.js:546` vs `bridge/cosmic-postupdates.js:37` |
| P0-8 | Scale 5 scenarios spawn DM:baryon at 0.85/0.15 while the "Cosmology (FTD)" panel advertises `DM_FRACTION = 17/27 ≈ 63%`. `BARYON_FRACTION` and `GAMMA_ADIABATIC` are exported and advertised but unused. | `bridge/cosmic-scenarios/galaxies.js:21-22,395,430` |
| P0-9 | Friedmann / Hubble not implemented. `H(t)` and `a(t)` are static (`_a = 1.0`, `_adot = 0.0`, never integrated) but presented as live diagnostics. | `bridge/mock-scale5.js:33-36,46` |
| P0-10 | Scale 2 `Temperature K` card is sim-unit `2·KE/(3N)` with no `k_B` conversion, but the card label and tooltip claim kelvin. | `bridge/mock-atom-engine.js:750`, `definitions.js:203`, `units.js:239` |
| P0-11 | Scale 2 `Electron B.E.` tooltip says "Slater-shielded hydrogenic orbitals"; implementation is Thomas-Fermi `−20.93·Z^(7/3) eV`. | `definitions.js:214` vs `atomic-energy.js:143-146` |
| P0-12 | Scale 3 acetylene central C–C distance 4.0 exceeds the auto-bond threshold 2.64 — the advertised "C≡C triple bond" never forms. | `molecules.js:267`, threshold at `bridge/mock-atom-engine.js:409` |
| P0-13 | Scale 3 auto-bonding writes `order: 1` unconditionally. Every double / triple / aromatic bond renders as a single line: O₂, N₂, CO₂, ethylene, acetylene, carbonyls, benzene. Molecule panel strings advertise the multi-orders verbatim. | `bridge/mock-atom-engine.js:413-414`, renderer at `viewport/molecular-renderer.js:215` |
| P0-14 | Retired `x₋  N_c` identification (FTD-0014 RETIRED per Cleanup Taxonomy v1.4 §5, removed in commit `ca7eb61`) still surfaces in the Ontic-Chain panel and the canonical constants file. | `ui/app-ontic.js:111`, `constants.js:42` |
| P0-15 | `constants.js` tags `Omega_Lambda = 2/3`, `DM_FRACTION = 17/27`, `BARYON_FRACTION = 10/27`, and `GAMMA_ADIABATIC = 5/3` as `[THEOREM]`. The FAQ honestly tags Ω_Λ as `[PARAMETRIC]` and 17/27 as `[SELECTION]` with the "does not match Planck 2018" still-open. Internal dashboard contradiction. | `constants.js:430-439` vs `ui/components/faq/data.js:124,147` |
| P0-16 | Born rule scenario description tagged `[THEOREM]`; FAQ tags the same claim `[SELECTION]` with explicit "10× lattice bias unaccounted for" caveat. | `config/scenarios.js:10` vs `ui/components/faq/data.js:57` |
| P0-17 | Scale 6 BCC/FCC sublattice labeling swapped. Per-site `userData.sublattice` is computed from coord-sum parity (`'BCC'` if even, `'FCC'` if odd), but Moore Layer Theorem §4 says the canonical mapping is shell-based (octahedron = SC, cuboctahedron = FCC, cube corners = BCC). Cube corners are therefore mislabeled `'FCC'`. | `meta-unit.js:150` vs `docs/theory/08_structural/THEOREM_MOORE_LAYER_DECOMPOSITION.md §4` |

### P1 — drift, brittleness, missing-wiring

| # | Ticket | File:line |
|---|---|---|
| P1-1 | `inspectorRuntime.setBridge` is defined but never called. Inspector keeps stale lattice-bridge reference across all scale switches; Scale 5 cosmic has no compensation; Scale 4 partially compensates via `setPlanetaryContext`. | `inspector/app-runtime.js:41-43` (defined, no callers) |
| P1-2 | AE WASM backend hardcoded disabled. `_aeHasWasm = false` regardless of binding availability; Scale 2/3 always run JS-only even when WASM is present. | `bridge/wasm-bridge.js:623-629` |
| P1-3 | `WebSocketBridge` toggle defaults drift from `MockBridge` (`weak_transmutation`, `selective_damping`). Silent behavioural divergence in native-GPU mode. | `ws-bridge.js:36,38` vs `bridge/mock-bridge.js:92,97` |
| P1-4 | Four Scale 0 overlay panels (`p1-observables`, `conservation-micropanel`, `spectrum`, `flux-slice`) have `dispose()` paths but are never called on `engineMode` switch. They keep calling `bridge.getDiagnostics()` and building DOM strings in non-lattice scales. | scenarios across `scales/scale0/ui/overlays/*` |
| P1-5 | `decay-rates.js:95` computes `dM = M_NEUTRON − M_PROTON` mixing PDG-anchored `M_NEUTRON = M_P_PHYS + DELTA_NP` with framework-anchored `M_PROTON = K_B · PROTON_RATIO`. Intended `~1.293` is hit by construction; if `K_B` is ever rescaled, drift is silent. Replace with `DELTA_NP` directly. | `decay-rates.js:95` |
| P1-6 | Coulomb-convention attribution lost at two sites — should import `COULOMB_K_PE` and `COULOMB_K_FORCE` named aliases. Numerically identical but loses convention provenance. | `bridge/mock-diagnostics.js:239`; `bridge/scenarios/s0-field-scenarios.js:136` |
| P1-7 | Scale 6 gerade/ungerade labeling is not the irrep-parity classification it claims. The "first nonzero coord positive" heuristic colors an antipodal half-orbit; that is the inversion fundamental domain, not g/u. Variable name `inversionParity` is fine; labels `'gerade'/'ungerade'` are wrong. | `meta-unit.js:74-85` |
| P1-8 | Scale 4 and Scale 5 lifecycle controllers never restore camera near/far/controls on exit. Switching from Scale 4 (near=0.001) or Scale 5 (far=50000) back to Scale 0 leaves the lattice with the previous scale's clip planes, causing z-fighting and culling artifacts. | `scales/scale4/controller.js:47-54,145-156`; `scales/scale5/controller.js` destroy path |
| P1-9 | Four scenarios.js entries claim `[THEOREM]` for value-level mass identifications that LEDGER tags as `[STRONGLY MOTIVATED CONJECTURE]` (downstream of FTD-0013). Also `m_e` error stated as 0.27% — CLAUDE.md says 0.19%. | `config/scenarios.js:56,74,84,229` |
| P1-10 | Higgs self-coupling `λ_H = m_H²/(2v²)` tagged `[DERIVED]` — pure parametric insertion of the SM tree-level formula. Should be `[PARAMETRIC]`. | `config/scenarios.js:105` |
| P1-11 | Thomas-Fermi prefactor `20.93 eV·Z^(7/3)` tagged `[DERIVED]` — standard Lieb–Simon Thomas-Fermi theory, not FTD-derived. Should be `[IMPOSED]` / `[EXTERNAL]`. | `constants.js:393-398` |
| P1-12 | `ALPHA = G_C² [DERIVED]` masks the calibration step `G_C ≡ sqrt(1/X_PLUS_PRECISION)`. Add explicit `[CALIBRATED]` note or pointer to FTD-0013 conjecture status. | `constants.js:60-62` |
| P1-13 | `ontic-observatory.js` surfaces "Theorem 3.1" / "Theorem 4.1" / "Theorem 5.4" / "Corollary 6.3" / "Theorem A.1" attributed to Steinmetz (2026) — no LEDGER row, no `docs/papers/` entry, no arXiv id. Live panel ships unaudited theorem labels. | `ontic-observatory.js:1-14` (header), surfaced via `renderFcCard`, `renderInfoDynamics` |
| P1-14 | 7 of ~14 FAQ `theoryRefs` 404 (broken post-May 2026 corpus consolidation). FAQ "Theory Refs" footer routinely sends users to missing files. | `ui/components/faq/data.js:41,42,88,178,201,290,320` |
| P1-15 | Reference frame structure vocabulary sweep (`REF_REFERENCE_FRAME_VOCABULARY.md`, 2026-05-01) was not applied to the dashboard FAQ or knowledge base. Load-bearing physics text still uses "reference frame context" framing. | `ui/components/faq/data.js:32,33,37,38,41,42`; `ui/components/knowledge-base/data.js:1154-1162` |
| P1-16 | FTD-0131 closed-negative status of `G_N = 1/(b_3+N_c)²` as a *framework-integer reading of physical G_N* is not surfaced. FAQ presents the identity as merely `[PARAMETRIC]`, omitting that the LEDGER closes it as a physical-G_N identification. | `ui/components/faq/data.js:193` |
| P1-17 | Scale 0 force-column "Weak" toggle (`#toggle-force-weak`) renders `∇×J · δ` and is self-flagged `[PROXY]` in tooltip, but the label "Weak Force" makes a physical claim the implementation does not support. Either rename to "∇×J pseudovector" or remove. | `scales/scale0/ui/overlays/template.js` weak-toggle entry |
| P1-18 | Scale 0 chirality tooltip says `\|J_L\| − \|J_R\|`; implementation writes `\|J\| · DUAL_DELTA` (positive scaled magnitude). Tooltip-code drift. | `scales/scale0/ui/overlays/template.js:207` vs `runtime/field-overlays.js:346-356` |
| P1-19 | Scale 2 AE diagnostic cards labelled `Kinetic Energy eV`, `Total Energy eV`, `PE Ionic eV`, etc. The underlying values are unconverted sim units. Either calibrate or relabel "sim units". | `bridge/mock-atom-engine.js:701-758` + AE card templates |
| P1-20 | Dead UI (10+ items): see §E Wiring fixes for the complete enumeration. |  |

### P2 — hygiene, consolidation, minor

| # | Ticket | File:line |
|---|---|---|
| P2-1 | `PARTICLE_VERT` GLSL string duplicated 3× across viewport modules; `FLUX_VOL_VERT` 2×. `PARTICLE_FRAG` is already centralized at `viewport/shaders.js:9-65` — move the verts in alongside. ~52 LOC dead weight. | `viewport.js:126-162`, `viewport/{field-renderer,particle-renderer,flux-renderer}.js` |
| P2-2 | "Hide all Scale-0 overlays" preamble repeats verbatim in Scale 4, 5, 6 controllers. Promote to `BaseLifecycleController.hideScale0Visuals()`. | `scales/scale{4,5,6}/controller.js` |
| P2-3 | `Light` overlay duplicates `Poynting` — both consume `sampled.poynting`. Fold into a single overlay with vector / bloom render-style toggle. | `scales/scale0/runtime/field-overlays.js:42,186,364` |
| P2-4 | `Genesis Iso` and `DM Halo` Scale 0 overlays are near-identical fluxMag scans (one gates `mag < K_GENESIS`, the other `\|mag − K_GENESIS\| < band`). Merge into one parametrized shell-scan. | `viewport/field-renderer.js:1423-1463, 1591-1631` |
| P2-5 | Scale 6 split-brain naming: controller at `scales/scale6/`, toolbar at `scales/scale12/ui/`. Class is `Scale6LifecycleController`, CSS class is `.scale12-only`, registry key is `'12'`. Pick one number or rename to `MetaLifecycleController` + drop the numeric key. | `scales/scale6/` + `scales/scale12/ui/` |
| P2-6 | `ALPHA_S` name collision inside C++: `constants.h:219` ships `ALPHA_S = 1.0` (lattice imposed), `gauge_couplings.h:145` ships `ALPHA_S_MZ = 7/59`. Mirror JS resolution — rename to `ALPHA_S_LATTICE`. | C++ engine headers |
| P2-7 | JS `STRONG_*` block  C++ `COLOR_*` block: values match, names disagree. Add C++ aliases or rename JS. | `constants.js:327-334` vs `engine/include/ftd/constants.h:219,228` + `constants_gpu.cuh:14-17` |
| P2-8 | `OMEGA_LAMBDA` JS vs `OMEGA_LAMBDA_CONJ` C++ name drift. The `_CONJ` suffix in C++ (post 2026-05-08 audit) marks `[CONJECTURE]` status; JS dropped it. Either add `_CONJ` to JS export or expose `using` alias in C++. | JS `constants.js:431` vs C++ `master_quadratic.h:135` |
| P2-9 | Scale 0 hardcoded `kGen = 1.533` (`field-renderer.js:1429`) — should import `K_GENESIS`. Confinement-string magic number `J2_threshold_dist2 = 120` is unmotivated. Bare `1/3`, `1/6` Laplacian weights (`mock-lattice-samplers.js:390`) should import `LAPLACIAN_FACE_WEIGHT`, `LAPLACIAN_EDGE_WEIGHT`. | per cell |
| P2-10 | `createListenerBag`, `throttleBySize` exported by `scales/scale-utils.js` with zero importers (superseded by `BaseLifecycleController.bindEvent`). Delete. | `scales/scale-utils.js:102-108, 161-184` |
| P2-11 | Test-spec boilerplate: 30+ specs duplicate the goto + waitForFunction(_ftdBridge) pattern when `_helpers.js:18 gotoAndReady` already exists. ~90 LOC savings, race-safe. | `engine/web/tests/*.spec.js` |
| P2-12 | Scale 5 `cosmic-super-cluster` is a duplicate of `cosmic-web` (same camera, same fallback). Collapse. `cosmic-gravitational-wave` registers `_gwEvents` array that no consumer reads. `cosmic-viewport-overlay` element is an empty shell. Orphaned camera preset `quasar` (in dispatcher, not in `<select>`). Duplicate tick readout (`#cosmic-tb-tick` + `#cosmic-tick`). | `scales/scale5/**` |
| P2-13 | Three bridge facades (Mock / Wasm / WS) with no shared contract enforcement. `bridge-contract.js` is a typedef-only file; `WebSocketBridge` doesn't even claim to implement it. Install `capabilities` getter on `WebSocketBridge` to eliminate P0-4, or extract a `BridgeBase`. | `bridge/{mock-bridge,wasm-bridge}.js` + `ws-bridge.js` |
| P2-14 | "DAG" suffix residue (`app.js`, `bridge-init.js`, `bridge/bridge-factory.js`) — the split sibling no longer exists. Strip on next touch. | three filenames |
| P2-15 | Many minor: `decay-rates.js` mass duplication with `particle-catalog`; orphaned `cross-sections.js` / `decay-rates.js` / `spectroscopy.js` render functions; Scale 4 biome heuristic duplicated (renderer + inspector); Scale 4 inlined `snoise` shadows `GLSL_SIMPLEX_NOISE_3D`; Scale 1 BH constants duplicated (`app.js:144-152` vs `scales/scale1/controller.js:64-69`); Scale 3 two bond renderers (lines + cylinders) under one "Bonds" toggle; `MAX_PARTICLES` / `MAX_FIELD_GRID` / `VOXEL_CENTER_OFFSET` declared 4×. | see §D |

Total tickets: **17 P0 + 20 P1 + 15 P2 = 52** discrete items.

---

## Section A — Motivated overlays kept (per scale)

Listed only where motivation gate passed (citation into theory corpus or
standard pedagogy). See §B for removals, §C for keepers needing correction.

### Scale 0 (lattice)
- Flux Volume / Slice / Streamlines — `engine/SPEC_ENGINE.md` Postulate 3
- ∇·J Gauss-residual — `engine/SPEC_ENGINE.md` gauss_project + `TRACKER_ONTIC_TRUTH.md`
- E = −∂_t J — `docs/theory/03_derivations/DERIV_CONTINUUM_LIMIT_QED_EQUIVALENCE.md`
- B = ∇×J — same
- Force-EM / Gravity / Strong arrows — `SPEC_ENGINE.md` phase_forces
- Dual-Substrate (1±δ)/2 — `constants.js:154` `DELTA_SQUARED` with self-flag at `runtime/field-overlays.js:327-328`
- Genesis isosurface at `K_GENESIS` — `constants.js:87` + LEDGER FTD-0110
- Damping zones — `SPEC_ENGINE.md` selective damping
- EM energy density u = ½(\|E\|² + \|B\|²) — standard Maxwell
- E/B pressure half-components — Maxwell stress-energy
- Kinetic energy — standard particle KE
- All quantum-column overlays (`ψ²`, phase, ℒ, entropy) — self-tagged `[PROXY]` in DOM titles; tag-consistent

### Scale 1 (particle)
- Coulomb, gravity, damping toggles — canonical PE physics `F = α q_i q_j / (4π r²)`
- Velocities / Trails / E-Field / Potential / Gravity-F / Forces overlays — standard PE diagnostics
- PE telemetry per-particle table (9 columns) — debug-grade visibility
- Orbital Mechanics 2-body card group (separation, reduced mass, h, semi-major, eccentricity, period, vis-viva, phase-space) — vis-viva is exactly the conservation gate we want; pedagogically motivated
- Particle catalog standard entries (e, μ, τ, p, n, π, K, etc.) — standard pedagogy
- Hawking-pair micro-scenario, tagged `[SELECTION]` in source (line-level comment is honest)

### Scale 2 (atom)
- Periodic table 1–86 with empirical Pauling χ + atomic radii — standard chemistry data
- Orbital cloud renderer s/p/d/f (post-2026-04-26 Laguerre fix) — standard hydrogenic
- Slater shielding (1s = 0.30, n-l = 0.35, inner = 0.85, deep = 1.00) — standard chemistry, tagged `[IMPOSED]` in `constants.js`
- SEMF Wapstra coefficients — standard nuclear physics, tagged `[IMPOSED]`
- Thomas-Fermi prefactor 20.93 eV·Z^(7/3) — Lieb–Simon, but mistagged as `[DERIVED]` (P1-11)
- LJ 12-6 and harmonic bond MD — standard MD
- Aufbau ordering + 20 exceptions — standard chemistry

### Scale 3 (molecular)
- 25 molecules across standard intro-chem, organic, biochem (water, ammonia, methane, benzene, ethanol, glycine, urea, adenine, caffeine, diamond, NaCl crystal) — standard curriculum
- VSEPR steric-number → equilibrium-angle map for steric ≤ 4 — canonical
- MD tuning constants `AE_*` — tagged `[IMPOSED]` and honestly self-described

### Scale 4 (planetary)
- All Scale 4 content is **standard pedagogical celestial mechanics**, not FTD-native. The viewport overlay label "Orbital mechanics simulator" is honest about this. Acceptable as a "standard physics demo" provided the `G_N` bug (P0-1) is fixed.

### Scale 5 (cosmic)
- `OMEGA_LAMBDA = 2/3`, `DM_FRACTION = 17/27`, `BARYON_FRACTION = 10/27`, `GAMMA_ADIABATIC = 5/3` — all in `constants.js:431-439`; values match canonical theory (Moore Layer Theorem). Tag-grade is the issue (P0-15), not the values.
- Stellar lifecycle thresholds (fuel-stage at fractions {0.3, 0.15, 0.05, 0}) — qualitative HR-track pedagogy
- Speed-of-light clamp `\|v\| < 1/√3` — canonical lattice c
- Mass-luminosity `L = M^3.5` — observational power-law for main-sequence
- Chandrasekhar / TOV thresholds in lattice units — `[IMPOSED]` anchors, used consistently

### Scale 6 (meta)
- All four shell-orbit toggles (Center, Octahedron, Cuboctahedron, Cube) with d² = 0/1/2/3 — `THEOREM_MOORE_LAYER_DECOMPOSITION.md`
- Stella octangula T+/T− partition with \|T±\| = 4 — Moore Layer §5
- Rotation axes (3 C₄ + 4 C₃ + 6 C₂) and 9 mirror planes (3 σ_h + 6 σ_d) — standard O_h
- Stabilizer labels (center → O_h, oct → C_4v, cuboct → C_2v, cube → C_3v) — correct orbit-stabilizer
- Cubic self-consistency `N_c + N_base + b_3 + N_eff = N_c³` with factorization `(N_c−3)(N_c²+2N_c+4) = 0` — verified algebra; uniqueness gloss should be softened (P2-15)
- Vieta `e₁ = 27, e₂ = 243, e₄ = 1092` of `P(x) = (x−3)(x−4)(x−7)(x−13)` — verified

---

## Section B — Overlays / scenarios flagged for removal

| Item | Scale | Rationale | Anchor |
|---|---|---|---|
| Force-Weak overlay as currently labeled | 0 | `∇×J · δ` is not the weak force; tooltip is self-flagged `[PROXY]` but the toolbar label still says "Weak". Either relabel ("∇×J pseudovector") or remove. | `scales/scale0/ui/overlays/template.js` `toggle-force-weak` |
| Light overlay (duplicate of Poynting) | 0 | Same `sampled.poynting` source rendered as bloom. Merge into one overlay with style toggle. | `runtime/field-overlays.js:42,186,364` |
| DM Halo overlay duplicate of Genesis Iso | 0 | Both scan `fluxMag` at K_GENESIS gates with marginally different thresholds. Parametrize. | `viewport/field-renderer.js:1423-1463,1591-1631` |
| `#sym-u1` / `#sym-su2` / `#sym-su3` checkboxes | 0 | No listeners anywhere. Orphan UI. | `scales/scale0/ui/overlays/symmetry-panel.js:14-16` |
| Renderable cross-sections / decay-rates / spectroscopy panels | 1 | `renderCrossSections`, `renderDecayRates`, `renderEnergyLevels` exported with **zero call sites**. Wire or delete. | `cross-sections.js`, `decay-rates.js`, `spectroscopy.js` |
| 7 PE "Advanced Forces (Phase 2)" toggles | 1 | `pe-lorentz-p`, `pe-exchange`, `pe-strong`, `pe-magnetic-dipole`, `pe-spin-orbit`, `pe-radiation`, `pe-relativistic` are declared in the controls card but have no listeners and no engine backing. Implement or hide. | `scales/scale1/ui/controls/pe-controls.js:36-67` |
| `#ae-torsional` checkbox (disabled stub) | 2 | "Phase 4 — not yet implemented", `disabled` attribute. Implement or remove. | `scales/scale2/ui/controls/ae-controls.js:69` |
| `Scale3ControlsComponent` (empty stub) | 3 | 22-LOC class with no behavior. Remove or merge into `register-scale3-ui.js`. | `scales/scale3/ui/controls/component.js:8-18` |
| `toggle-mol-field` button | 3 | No event listener anywhere. Dead UI. | `scales/scale3/ui/overlays/template.js:11` |
| `#planetary-opt-orbits`, `#planetary-opt-axes` toggles | 4 | Bound in controller but never rendered in `index.html`. Silent no-op. | `scales/scale4/controller.js:121-132` |
| `EXOPLANET_SEEDS["Kepler-90"] = []` empty | 4 | Menu offers it; loader falls through to single-star fallback. Either populate or remove from menu. | `config/exoplanet-seeds.js:88` + `scales/scale4/ui/toolbar/template.js:14` |
| `cosmic-super-cluster` scenario | 5 | Duplicates `cosmic-web`. Same camera, same fallback. | `bridge/cosmic-scenarios/index.js:53` |
| `cosmic-gravitational-wave` scenario | 5 | Registers `_gwEvents` array that no consumer reads. Pure aesthetic with no GW physics. | `bridge/cosmic-scenarios/exotic.js:177` |
| `cosmic-viewport-overlay` div | 5 | Empty shell with no content. | `scales/scale5/ui/overlays/template.js:5-11` |
| Orphaned `quasar` camera preset | 5 | Dispatcher recognizes it; no `<option>` in selector. | `scales/scale5/controller.js:88`, `cosmic-renderer.js:497` |
| `LATTICE_TO_SOLAR_MASS = 50` (exported but unused) | 5 | Comment says "Exposed for cosmic-physics.js"; not imported there. Either wire (fixes P0-6) or remove the export. | `constants.js:269` |
| `cosmicInspectBody` + `cosmic-inspector-content` HTML block | 5 | Defined + HTML mounted, controller never populates. Dead inspector path. | `bridge/mock-scale5.js:279-293` + `panel-resources/template.js:367-414` |
| `MetaUnit.inspectSite` + `buildSiteInspectPanel` | 6 | Both exported, never called. "Click a site to inspect" UI stub. | `meta-unit.js:515-528`, `meta-pedagogy.js:426` |
| `MetaInfoPanelComponent` (22-LOC noop) | 6 | Empty cleanup, empty work. | `ui/panels/meta-info-panel/component.js` |
| `1296 = 6⁴` "Full group" stat | 6 | No canonical citation for this interpretation of the O_h × S_3 product. Remove or attach a theory pointer. | `meta-pedagogy.js:246` |
| `createListenerBag`, `throttleBySize` | utility | Zero importers; superseded by `BaseLifecycleController.bindEvent`. | `scales/scale-utils.js:102-108,161-184` |

---

## Section C — Overlays needing correction (current vs canonical, required change)

| Item | Current | Canonical | File:line |
|---|---|---|---|
| Scale 0 chirality tooltip | "Chirality field: \|J_L\| − \|J_R\| (net handedness)" | Code writes `state.chiralValues[i] = mag · DUAL_DELTA` (positive scaled magnitude, no L−R subtraction) | tooltip `scales/scale0/ui/overlays/template.js:207` vs code `runtime/field-overlays.js:346-356` |
| Scale 0 force-weak label | "Weak Force" with `[PROXY]` tag | Not the weak force at all; rename "∇×J pseudovector" | `scales/scale0/ui/overlays/template.js` weak entry |
| Scale 0 confinement strings | Lines between every pair with `r² < 120` | Tooltip claims "SU(3) flux strings"; implementation is just pair-proximity, not area-law. Either rename to "Color-pair proximity glyphs" or implement Wilson-loop area-law | `viewport/field-renderer.js:1652-1706` |
| Scale 0 Dark-Matter Halo overlay | Sub-threshold flux envelope | Tooltip implies the 17/27 derivation; implementation is just `mag < kGen` heuristic. Either thread `DM_FRACTION` through or rename to "Sub-genesis flux envelope" | `viewport/field-renderer.js:1423-1463` |
| Scale 0 SPEC reference at `store.js:20` | Cites `docs/SPEC_S0_QUANTUM_OVERLAYS.md` | File does not exist in `docs/`. Either land it or remove the breadcrumb. | `scales/scale0/state/store.js:20` |
| Scale 1 `Temperature MeV` tooltip | `T = (2/3)⟨K⟩/(k_B N)` | Code computes `(2/3)·KE/N` (no `k_B` divide; MeV per particle). Drop `k_B` from tooltip or convert in code. | `definitions.js:152` vs `pe-telemetry.js:289` |
| Scale 1 particle-catalog `ftd_status` labels | 17 entries marked `'derived'` | W±, Z, Higgs, neutron, proton-itself are mass-formula matches with PDG inputs. Tag as `'selection'` or `'parametric'` to align with LEDGER. | `particle-catalog.js` lines 295/304/313/326/340/360 |
| Scale 2 `Temperature K` card | `T = 2·KE/(3N)` displayed with " K" suffix | Sim-unit value with kelvin label is a units-bug. Implement `k_B·K_PER_MEV` conversion or strip the "K" suffix. | `bridge/mock-atom-engine.js:750`, `definitions.js:203`, `units.js:239` |
| Scale 2 `Mass KB` tooltip | "Composite atomic mass in lattice K_B units, … value used by the Scale-0 particle genesis threshold." | Implementation divides by `M_E_PHYS = 0.51099895` (PDG), not the FTD `K_B = 0.511`. No cross-scale wiring to Scale-0 genesis exists. Rewrite. | `definitions.js:216` vs `atomic-energy.js:122-123` |
| Scale 2 `Electron B.E.` tooltip | "Total electron binding energy summed over all shells, from Slater-shielded hydrogenic orbitals." | Implementation is Thomas-Fermi atomic total `−20.93·Z^(7/3) eV`, not shell-summed Slater. | `definitions.js:214` vs `atomic-energy.js:143-146` |
| Scale 2 AE cards labelled "eV" | `Kinetic Energy eV`, `Total Energy eV`, `PE Ionic eV`, `PE vdW eV`, `PE Bonds eV` | Values are unconverted sim units. Either calibrate to eV via the AE_* tuning chain or relabel "sim units". | mock-atom-engine.js:701-758 |
| Scale 2 SEMF asymmetry term | `A_ASYM * (N-Z)² / (4·A)` | Wapstra `a_A = 23.29` convention usually `a_A·(N−Z)²/A` without the `/4`. Verify against Fe-56 (Z=26, N=30, expected B ≈ 492.3 MeV); current code likely gives drift. | `atomic-energy.js:65` |
| Scale 3 molecule headers vs renderer | `o2` "double bond", `n2` "triple bond", `co2` carbonyls, ethylene `C=C`, acetylene `C≡C`, benzene aromatic ring | Auto-bond emits `order: 1` only (`mock-atom-engine.js:413-414`). All multi-bonds render as single lines. Either implement bond-order inference (distance threshold + valence saturation) or strip multi-bond claims from panel strings. | `molecules.js` panel strings + `bridge/mock-atom-engine.js:413-414` |
| Scale 3 acetylene geometry | C–C distance 4.0 sim units | Auto-bond threshold for C–C is 1.2 · (2.2+2.2)/2 = 2.64. Central C–C never bonds. | `molecules.js:267` |
| Scale 3 `molecules.js` header sigma table | Comment claims Na σ ≈ 1.5 | Computed `1/cbrt(11)·4 ≈ 1.79`. 19% drift. Documentation-only. | `molecules.js:8-11` |
| Scale 3 NH₃ geometry | Header asserts 107.8° | Constructed coordinates with manual y-offset deviate from 107.8° (small drift; quotes a value the construction does not exactly achieve). | `molecules.js:146-163` |
| Scale 4 `G` constant | `G_N = 0.01` (lattice) | For AU/M_sun/yr unit system, Kepler's 3rd law needs `G_HELIOCENTRIC = 4π² ≈ 39.478`. Earth year wrong by factor ~63×. | `bridge/mock-scale4.js:35,66` |
| Scale 4 orbit-ring overlay | Circular rings | For e ≥ 0.5 bodies (HR 8799 c/d, Kepler-20 d) draw ellipses with focus at the host star. | `planetary-renderer.js:251-266` |
| Scale 4 biome heuristic | `uTemp = 1.25 − d_AU` | AU-anchored to solar system; mis-colors TRAPPIST-1 (a ≈ 0.01–0.06 AU = all "lava") and HR 8799 (a = 14–68 AU = all "ice"). Aesthetic only — label as such, or scale by stellar luminosity. | `planetary-renderer.js:329-338`, `inspector/scales/planetary.js:73-89` |
| Scale 4 inspector temperature | `T_K = 280 + uTemp·500` | No astrophysical basis. Label as biome class, not temperature. | `inspector/scales/planetary.js:94` |
| Scale 5 cosmic-info `r_s = 2 G_N M` claim | Panel formula text | Engine renders `r_horizon = max(0.8, cbrt(M)·0.12)` (constant-density scaling), not Schwarzschild. Either change the panel formula to match what's drawn or implement Schwarzschild and rescale. | `panel-resources/template.js:546` vs `bridge/cosmic-postupdates.js:37` |
| Scale 5 Hubble / scale-factor display | "Hubble parameter H(t)" + `a(t)` live diagnostic | `_H0` is static, `_adot = 0`, never integrated. Either label "(anchor)" or implement Friedmann `H² = H0²(Ω_M(1+z)³ + Ω_Λ)`. | `bridge/mock-scale5.js:33-36,46`; `scales/scale5/ui/toolbar/template.js:36` |
| Scale 5 mass labels with `M☉` | Reports lattice mass with solar-mass glyph | Multiply by `LATTICE_TO_SOLAR_MASS = 50` first, or strip glyph. | `bridge/mock-scale5.js:153,175,184,191` |
| Scale 5 scenario DM:baryon split | 0.85 / 0.15 | UI advertises FTD theorem `17/27 ≈ 0.63`. Either thread `DM_FRACTION` / `BARYON_FRACTION` through or stop advertising 17/27 in the cosmology copy card. | `bridge/cosmic-scenarios/galaxies.js:21-22,395,430` vs `panel-resources/template.js:38` |
| Scale 6 BCC/FCC sublattice labels | Coord-sum parity (cube corners labeled FCC) | Moore Layer Theorem §4: octahedron=SC, cuboctahedron=FCC, cube corners=BCC. Either rename userData field to `parityClass` (acknowledge it's a parity partition) or fix mapping to shell-based. | `meta-unit.js:150` |
| Scale 6 gerade/ungerade labels | "first nonzero coord positive" heuristic colors antipodal half-orbits | The visual 13+13 split is correct as inversion fundamental domain, but it is not g/u irrep parity. Rename `'gerade'/'ungerade'` to `'orbit_rep'/'antipode'`. | `meta-unit.js:74-85,407,25-26` |
| Scale 6 per-shell irrep labels | `oct = T_1u`, `cuboct = T_2g + E_g`, `cube = A_2u + T_1u` | Incomplete decompositions: 6-site orbit = A_1g ⊕ E_g ⊕ T_1u (dim 6); 12-site orbit decomposes into 6 irreps; 8-site orbit = A_1g ⊕ A_2u ⊕ T_2g ⊕ T_1u (dim 8). Either give full decomposition or link to FTD-0110 character-table derivation. | `meta-unit.js:95-100` vs `docs/theory/03_derivations/DERIV_K_FROM_OH_A1G_MULTIPLICITY.md §2.1` |
| Retired `x₋  N_c` row | Surfaced in Ontic Chain panel as "x₋ ≈ N_c (color charges)" | FTD-0014 RETIRED per Cleanup Taxonomy v1.4 §5. Either remove row or restate as "mathematical root of master quadratic; identification with N_c is RETIRED". | `ui/app-ontic.js:111`, `constants.js:42` |
| `[THEOREM]` overclaims in `constants.js` | Ω_Λ = 2/3, DM 17/27, baryon 10/27, γ = 5/3 all tagged `[THEOREM]` | FAQ tags Ω_Λ as `[PARAMETRIC]` and 17/27 as `[SELECTION]` with still-open "doesn't match Planck 2018". Internal contradiction. Match the FAQ. | `constants.js:430-439` |
| Born rule `[THEOREM]` | scenario description claim | FAQ tags it `[SELECTION]` with "10× lattice bias unaccounted". Downgrade scenarios.js to match. | `config/scenarios.js:10` vs `ui/components/faq/data.js:57` |
| SU(3) identification `[THEOREM]` | Scenario claim | The factorization is `[THEOREM]`; the *identification with SM strong-force gauge group* is `[SELECTION]`, as U(1) and SU(2) entries in the same scenario correctly tag. Match U(1)/SU(2). | `config/scenarios.js:260` vs `:244,252` |
| `[THEOREM]` header in constants.js | "[THEOREM] Values are taken from the C++ ontic.h derivation chain … all physics" | Chain is `[THEOREM]` only up to `x_+`; downstream of `x_+  1/α` is `[STRONGLY MOTIVATED CONJECTURE]` (FTD-0013). Strip blanket tag. | `constants.js:5` |
| Mass closed-form `[THEOREM]` tags | `m_e`, m_μ/m_e, m_τ/m_e, m_p/m_e all `[THEOREM]` in scenario table | These are value-level identifications inheriting FTD-0013 `[STRONGLY MOTIVATED CONJECTURE]`. Also update `m_e` error 0.27% → 0.19% (CLAUDE.md). | `config/scenarios.js:56,74,84,229` |
| Higgs self-coupling `[DERIVED]` | `λ_H = m_H²/(2v²) ≈ 0.129` | SM tree-level formula filled with FTD m_H — pure parametric insertion. Retag `[PARAMETRIC]`. | `config/scenarios.js:105` |
| Thomas-Fermi `[DERIVED]` | "Standard derivation: integrate the Thomas-Fermi electron density …" | Standard Lieb–Simon TF theory; no FTD axiom enters. Retag `[IMPOSED]` or `[EXTERNAL]`. | `constants.js:393-398` |
| ALPHA = G_C² `[DERIVED]` | `// alias: alpha = G_C^2 [DERIVED]` | `G_C` is calibrated to `sqrt(1/X_PLUS_PRECISION) = sqrt(α_CODATA)`. Add `[CALIBRATED]` note or pointer to FTD-0013. | `constants.js:60-62` |
| Steinmetz 2026 theorems | "Theorem 3.1", "Theorem 4.1", "Theorem 5.4", "Corollary 6.3", "Theorem A.1" surfaced in observatory header | No LEDGER row, no `docs/papers/` entry. Demote labels to "Proposition" / "Heuristic" or attach explicit provenance. | `ontic-observatory.js:1-14` |
| `G_N = 1/(b_3+N_c)²` FAQ entry | `[PARAMETRIC]` with "numerical match works at chosen normalisation" | Per FTD-0131 (SPEC_DOCTRINE_LEDGER §10), this is `[CLOSED NEGATIVE]` as a *physical-G_N identification* ("off by ~10²⁰ to ~10⁴³"). Surviving claim is substrate-derived `α_G(e,e)`, not in dashboard. | `ui/components/faq/data.js:193` |

---

## Section D — Consolidation tickets

Cross-cutting (sorted by LOC delta × confidence / risk):

| # | Title | Files touched | LOC delta | Risk |
|---|---|---|---|---|
| D-1 | Move `PARTICLE_VERT` + `FLUX_VOL_VERT` into `viewport/shaders.js` (3+2 duplicate copies) | viewport.js, viewport/{field,particle,flux}-renderer.js, viewport/shaders.js | −52 | Low |
| D-2 | Extract `hideScale0Overlays(viewport)` + `restoreScale0Overlays(viewport)` helpers, replace 3× verbatim block in Scale 4/5/6 controllers | scales/{scale4,scale5,scale6}/controller.js + scales/scale-utils.js | −25 | Low |
| D-3 | Migrate spec files to `_helpers.js:gotoAndReady`; drop manual goto + waitForFunction (5/18 specs already adopted) | engine/web/tests/*.spec.js | −90 | Low |
| D-4 | Delete `createListenerBag`, `throttleBySize` from scale-utils (zero importers) | scales/scale-utils.js | −35 | Low |
| D-5 | Light overlay → render-style toggle on Poynting overlay; DM-Halo → parametrized shell-scan with Genesis-Iso | scales/scale0/runtime/field-overlays.js, viewport/field-renderer.js | −60 | Med |
| D-6 | Centralize `MAX_PARTICLES`, `MAX_FIELD_GRID`, `VOXEL_CENTER_OFFSET` into `viewport/constants.js` (declared 4× in viewport modules) | viewport.js + 3 sub-renderers + new constants file | ±0 LOC, eliminates drift risk | Low |
| D-7 | Split p1-observables-panel.js (1396 LOC) by section (Coulomb, Hydrogen, Bell, Gravity, g−2) into 5 children + thin parent; same pattern for flux-slice-panel.js (1117 LOC) | scales/scale0/ui/overlays/ | 0 net, ≤300 LOC each | Med |
| D-8 | Sub-renderer split of field-renderer.js (2311 LOC) by mesh family — Force / Quantum / Cosmic sub-renderers | viewport/field-renderer.js → 3-4 children | 0 net, ~700 LOC each | High (defer until visual-regression harness) |
| D-9 | Consolidate JS `STRONG_*`  C++ `COLOR_*` naming. Add C++ alias block or rename JS. Resolves P2-7. | engine/include/ftd/constants.h, constants_gpu.cuh | Small | Low |
| D-10 | Three bridge facades — install `capabilities` getter on `WebSocketBridge` (resolves P0-4). Optionally extract `BridgeBase` for shared contract enforcement. | bridge/{mock,wasm,bridge-factory-dag,bridge-contract}.js, ws-bridge.js | +20 / −0 | Med |
| D-11 | Promote AE WASM check from hardcoded false (`wasm-bridge.js:625-629`) to a binding-availability probe (resolves P1-2). | bridge/wasm-bridge.js + the JSWASM scale-conversion shim | +50 to fully port AE; small to enable probe | Med-High |
| D-12 | Delete `Scale1LifecycleController` empty stub (forwards `destroy` only; class wrapper without value) | scales/scale1/controller.js:103-131 | −20 | Low |
| D-13 | Strip `-dag.js` suffix on next touch — `app.js`, `bridge-init.js`, `bridge/bridge-factory.js` (DAG sibling no longer exists) | 3 file renames + ~30 import updates | 0 net | Low |
| D-14 | Scale 6 directory naming — consolidate `scale6/` controller + `scale12/` toolbar under one numeric key; rename class `Scale6LifecycleController` → `MetaLifecycleController` | scales/scale6/, scales/scale12/, css scale12-only class, index.html | Small | Low |
| D-15 | Scale 4 inlined `snoise` in star shader shadows `GLSL_SIMPLEX_NOISE_3D` from constants.js — replace with template-string injection | planetary-renderer.js:113-155 | −40 | Low |
| D-16 | Scale 4 biome heuristic duplicated (renderer + inspector) — extract `_classifyBiome(d)` helper | planetary-renderer.js:329-338, inspector/scales/planetary.js:73-89 | −10 | Low |
| D-17 | Scale 1 BH micro constants duplicated in `app.js:144-152` and `scales/scale1/controller.js:64-69` — delete the app_dag copy | app.js | −10 | Low |
| D-18 | Scale 3 two bond renderers (`bondLines` + `_bondCylinders`) under one "Bonds" toggle — pick one canonical | viewport/molecular-renderer.js:53-115, 158-268 | −60 to −100 | Med |
| D-19 | Delete `cosmic-super-cluster` scenario (≡ `cosmic-web` fallback) | bridge/cosmic-scenarios/index.js, scales/scale5/ui/toolbar/template.js | −15 | Low |
| D-20 | Consolidate diagnostics-throttle (status-cache + activeTab switch) between Scale 1 and Scale 2 controllers into `scale-utils.createDiagnosticsThrottle` | scales/scale-utils.js, scale1/scale2 controllers | −50 | Med |
| D-21 | `_resetAllVisualState` (app.js:275-328) — push Scale-2 AE button-reset block into `scale2/controller.js:resetScale2`, etc. Orchestrator should not know AE checkbox ids. | app.js + scale controllers | −60 in app.js | Med |

---

## Section E — Wiring fixes

### Orphan UI (DOM elements with no listener)

| Element | Scope | File:line |
|---|---|---|
| `#sym-u1` / `#sym-su2` / `#sym-su3` | Scale 0 symmetry panel | `scales/scale0/ui/overlays/symmetry-panel.js:14-16` |
| 7 PE Advanced Forces toggles | Scale 1 | `scales/scale1/ui/controls/pe-controls.js:36-67` (`pe-lorentz-p`, `pe-exchange`, `pe-strong`, `pe-magnetic-dipole`, `pe-spin-orbit`, `pe-radiation`, `pe-relativistic`) |
| `#ae-torsional` (disabled stub) | Scale 2 | `scales/scale2/ui/controls/ae-controls.js:69` |
| `#toggle-mol-field` | Scale 3 | `scales/scale3/ui/overlays/template.js:11` |
| `#planetary-opt-orbits`, `#planetary-opt-axes` | Scale 4 | bound at `scales/scale4/controller.js:121-132`; not rendered in index.html |
| Orphaned `quasar` camera preset | Scale 5 | dispatcher knows it (`scales/scale5/controller.js:88`); `<select>` lacks the `<option>` |
| `cosmic-inspector-content` HTML block | Scale 5 | `ui/components/panel-resources/template.js:367-414`; no controller populates |
| `MetaUnit.inspectSite` + `buildSiteInspectPanel` | Scale 6 | `meta-unit.js:515`, `meta-pedagogy.js:426` — defined, never called; viewport click handler not wired |

### Stale-snapshot / late-binding bugs

| Symptom | File:line |
|---|---|
| Scale 4 setInterval captures `ctx.running` from `loadScenario`-time snapshot; planetary cannot be paused after load | `scales/scale4/controller.js:65-88` |
| `inspectorRuntime.setBridge` defined, never called; inspector keeps stale lattice bridge across scale switches | `inspector/app-runtime.js:41-43` |
| Scale 4 captured `ctx.engineMode` guard at `controller.js:68-69` is dead code (snapshot); actual gate is the interval-clear in `destroy()` | `scales/scale4/controller.js` |

### Lifecycle / cleanup gaps

| Gap | File:line |
|---|---|
| Scale 4 `destroy()` does not restore camera `near`/`far`/`controls` (left at near=0.001) | `scales/scale4/controller.js:47-54, 145-156` |
| Scale 5 `destroy()` does not restore camera near/far (left at far=50000) | `scales/scale5/controller.js` |
| `MountToggleComponent` adds 3 listeners (root click, window keydown, window resize) with no destroy method — re-mounting double-binds | `ui/components/panel-dock/mount-toggle.js:75-77` |
| Four Scale 0 panels (`p1-observables`, `conservation-micropanel`, `spectrum`, `flux-slice`) have `dispose()` but never called on engineMode switch — keep ticking against lattice bridge in non-lattice scales | `scales/scale0/ui/overlays/*` |

### Tooltip  DOM matchers

| Issue | File:line |
|---|---|
| Scale 0 `Charge net` and `Gauss Σdiv J−s²` keys rely on U+2212 minus and parens-stripped normalize — brittle to template label edits | `definitions.js:91,113`; `normalizeLabel` at `:17-22` |
| 30+ Scale 0 overlay toggle buttons (`#toggle-e-field`, etc.) rely on inline `title=` attributes only; no entries in `SELECTOR_TOOLTIPS` array | `definitions.js:24-72` |
| Scale 2 "Mass (K_B)" → "Mass KB" normalization — risk of cross-panel collision if any other card titled "Mass (k_B)" (Boltzmann sense) ever lands | `definitions.js:216` |

### Cross-panel handoff

| Symptom | File:line |
|---|---|
| Scale 5 emits `hubbleParameter` in diagnostics; telemetry-hub reads `hubble` / `hubbleParam`. `csHubble` ring buffer dead | `telemetry-hub.js:324` vs `bridge/mock-scale5.js:307` |
| `cosmic-tb-tick` + `cosmic-tick` duplicated readouts updated independently | `scales/scale5/controller.js:170,176` |
| Cosmic renderer toggle API (`toggleDarkMatter/GasClouds/Stars/BlackHoles/AccretionDisks`) has no UI surface | `cosmic-renderer.js:505-509` |
| `cosmicInspectBody` returned by bridge, never consumed by any panel | `bridge/mock-scale5.js:279-293` |
| `_customTelemetry` collected in cosmic bridge, returned in diagnostics, never displayed | `bridge/mock-scale5.js:134-202` |

### Bridge surface gaps

| Gap | File:line |
|---|---|
| `WebSocketBridge.setBoundaryShape()`, `loadScenario()` are no-op stubs | `ws-bridge.js:331-332` |
| `WebSocketBridge` lacks 16+ field samplers (`getEFieldSampled`, `getBFieldSampled`, `getPoyntingSampled`, `getDivJSampled`, …, `getStrongForceField`) and the entire PE/AE method surface | `ws-bridge.js` |
| `WebSocketBridge` lacks `bridge.capabilities` getter; `scales/scale0/runtime/tick.js:19` will throw `TypeError: bridge.capabilities is undefined` on native-GPU | `bridge-init.js:36-37` mounts capability getter only on Mock + Wasm prototypes |

---

## Section F — Performance tickets prioritized

| # | Issue | Cost | File:line |
|---|---|---|---|
| F-1 | Scale 5 O(N²) force kernel with ~2600 bodies per `cosmic-galaxy` tick (~3.4M pair iterations / tick, every-other-rAF). No tree code, FMM, or spatial partition. | High — ~200ms/frame on mid-range hardware | `bridge/cosmic-physics.js:87-111` |
| F-2 | Scale 0 50 `new Float32Array` per overlay refresh in mock-lattice-samplers. ~30k float allocations per frame at L=32, stride=2 (~3 MB/s GC churn). | Medium | `bridge/mock-lattice-samplers.js:67,93,122,158,188,230,266,293,386,436,458,495` |
| F-3 | Scale 5 `getCosmicData()` allocates 9 typed arrays sized N per physics frame (~80 KB/frame at N=2600 = ~2.4 MB/s churn) | Medium | `bridge/mock-scale5.js:247-277` |
| F-4 | Scale 0 confinement-strings O(N²) particle pair loop, 40k pair-tests/frame at N_particles=200 | Medium-High | `viewport/field-renderer.js:1652-1700` |
| F-5 | Scale 0 `LineDashedMaterial.computeLineDistances()` called every `updateForceStreamlines` for every line (200 lines × CPU recompute) | Medium | `viewport/field-renderer.js:1225,1238-1244` |
| F-6 | Scale 4 `setInterval(..., 16)` × 100 substeps + O(N²) pair force; setInterval throttled to 1Hz when tab backgrounded | Medium | `scales/scale4/controller.js:67`, `bridge/mock-scale4.js:198-202` |
| F-7 | Scale 4 per-body `ShaderMaterial` compile (TRAPPIST-1 = 8 separate shader programs at scenario load) | Medium | `planetary-renderer.js:217-225` |
| F-8 | Scale 2 `aeGetForceDecomposition()` is O(N²) and called every 2nd frame when ANY arrow toggle is on. ae-periodic (118 atoms) = 13924 × 4 channels × 30fps ≈ 1.7M ops/s | Medium | `scales/scale2/controller.js:367-371`, `bridge/mock-atom-engine.js:763` |
| F-9 | Scale 2 ae-periodic orbital cloud can silently exceed `MAX_CLOUD = 100000`; no warning | Low (correctness flag) | `orbitals.js:135,152` |
| F-10 | Scale 2 `atoms.find(at => at.id === b.partner_id)` inside bond-PE loop is O(N²) per bond | Low-Med | `bridge/mock-atom-engine.js:736` |
| F-11 | Scale 6 27 individual `SphereGeometry` instances (24×16 segments = ~62k triangles for site dots). Should be one `InstancedMesh` per shell (4 total) | Low (memory + draw calls) | `meta-unit-geometry.js:9-19` |
| F-12 | Scale 0 200-LineDashedMaterial pool pre-built always, even if force-flow style unused. Lazy-build on first init | Low-Med (memory) | `viewport/field-renderer.js:1174-1198` |
| F-13 | `viewport.js:198-202` `_insideBoundary()` hoisted out in `flux-renderer.js:228-229` but not in `field-renderer.js` arrow/streamline loops (14 sites). Per-voxel function-call overhead aggregates. | High in aggregate | `viewport/field-renderer.js:570,582,731,811,912,1031,1146,1339,1773,1849,1924,2036,2113,2174` |
| F-14 | Scale 0 `field-overlays.js:300,301` `generateImportanceSeeds(...)` re-allocates seed array per overlay refresh inside force-flow loop | Medium | `runtime/field-overlays.js:300-301` |
| F-15 | Scale 0 redundant soft-disc texture build at `field-renderer.js:79-96` (module-level) AND `:1953-1970` (instance method) | Low (memory ~16KB) | as cited |
| F-16 | `_buildScale1Ctx(now)` / `_buildScale2Ctx(now)` / `_makeCtx()` fresh-object allocations every frame in app.js (low cost individually, observably common) | Low | `app.js:163-200, 662-689` |
| F-17 | Scale 1 PE telemetry `_phaseBuf.push() / shift()` on 300-element buffer per 2-body frame — `shift()` is O(n). Replace with circular buffer | Low | `pe-telemetry.js:435-436` |
| F-18 | Scale 4 64×64 sphere geometry per body × 4 octaves of vertex-shader fbm every frame even when geometry is stationary. LOD or bake | Low-Med | `planetary-renderer.js:191` |
| F-19 | Cosmic per-body PointLight (binary-star scenarios duplicate lighting cost) | Low | `cosmic-renderer.js:241-249,350-354` |
| F-20 | Background star field 12000 sprites with `frustumCulled = false` | Low (GPU-cheap) | `cosmic-renderer.js:606` |

---

## Section G — Cross-scale issues

### G-1 Constants cross-language parity

JS / C++ / Python ontic chain (-1 through Layer 7) is in lockstep. Drift surfaces:

- **P0:** `PROTON_RATIO` formula — JS uses canonical FTD-0016 (`N_eff/α + N_base·N_eff + N_c ≈ 1836.47`); C++ + `proof_complete_sm.py:190` ship pre-F9 wrong formula (~3520). `engine/tests/campaign_triad_binding.cpp:154-159` checks against 1836.15 → guaranteed failure. JS comment at `constants.js:99-100` documents the bad formula but the bad formula still ships in C++/Py.
- **P1:** `X_MINUS_PRECISION` (C++ `master_quadratic.h:80`) missing in JS + Py. Any `1/X_MINUS` derived quantity drifts at the 6th digit relative to C++.
- **P1:** `ALPHA_S` collision inside C++: `constants.h:219` = 1.0 (lattice imposed) vs `gauge_couplings.h:145` = 7/59 (M_Z scale). JS resolves with `STRONG_ALPHA_S` vs `ALPHA_S_MZ`; mirror this in C++.
- **P1:** `STRONG_*` JS  `COLOR_*` C++ — values match, names disagree.
- **P2:** `OMEGA_LAMBDA` JS vs `OMEGA_LAMBDA_CONJ` C++ — `_CONJ` suffix in C++ marks `[CONJECTURE]`; JS dropped it.
- **P2:** JS Layer-8 (reference frame context) constants `PHI`, `K_NOETIC`, etc. exist in C++ + Py but not JS; JS Layer-9 cosmic constants (DM_FRACTION, BARYON_FRACTION, GAMMA_ADIABATIC) exist in JS + C++ but not Py.

`scripts/constants.py` parity test (`scripts/tests/test_constants_parity.py:57`) covers PDG mirror block only; should extend to load-bearing ontic-chain symbols.

### G-2 Unit-system mixing

- `constants.js:106-109` ships M_W, M_Z, V_HIGGS, M_HIGGS in **GeV**; `constants.js:196-198` ships M_W_PHYS, M_Z_PHYS, M_HIGGS_PHYS in **MeV**. No unit suffix in the names — `M_Z` vs `M_Z_PHYS` differ by 1000× from unit choice alone. Rename one set with explicit `_GeV` / `_MeV`.
- `G_HELIOCENTRIC = 4π²` exported but no consumer imports it. Scale 4 uses lattice `G_N = 0.01` and labels UI as AU·M_sun·yr. UI implies SI; simulation reports lattice. **P0-1 root cause.**
- `H0_LATTICE = 0.001` reported as "Hubble parameter H(t)" with no SI conversion. Implies km/s/Mpc; is lattice number.
- `LATTICE_TO_SOLAR_MASS = 50` exported but never imported. Cosmic telemetry shows raw lattice mass with `M☉` glyph (factor 50× off). **P0-6 root cause.**

### G-3 Coulomb-convention attribution

`constants.js:299-320` exposes three named aliases:
- `COULOMB_K_PE = α` (lattice-PE convention)
- `COULOMB_K_FORCE = α/(4π)` (classical force law)
- `COULOMB_K_HEP = α` (Gaussian / HEP units)

Audit of every Coulomb callsite passed at almost every site. Two named-alias drift sites:
- `bridge/mock-diagnostics.js:239` uses bare `ALPHA` for PE-convention sum (should import `COULOMB_K_PE`)
- `bridge/scenarios/s0-field-scenarios.js:136` uses bare `ALPHA / (4 * Math.PI)` (should import `COULOMB_K_FORCE`)

Numerically identical; provenance lost. `COULOMB_K_PE` and `COULOMB_K_HEP` have zero callsites otherwise — reserved for future use.

### G-4 Bridge facade drift

Three facades (`MockBridge`, `WasmBridge`, `WebSocketBridge`):
- WS toggle defaults drift from Mock (`selective_damping`, `weak_transmutation`).
- WS lacks `bridge.capabilities` getter entirely → native-GPU mode crashes in Scale 0.
- WS scenario library is empty (`loadScenario()` no-op); native-GPU has no scenario set.
- Wasm AE backend hardcoded disabled (`_aeHasWasm = false`); Scale 2/3 always JS-only.
- `WasmBridge.peGetExtendedData` returns `null` (not implemented); Scale 1 telemetry silently loses data in WASM mode.
- Particle-record field drift: Mock uses `.charge`, Wasm uses `.q`. PhysicsHarness papers over via `getParticleCharge(p)`; callsites bypassing the harness see backend-dependent fields.

### G-5 Cross-scale handoff state

- `aggregation-bridge.js:135` defaults missing particle energy to `K_B` (electron mass) — misleading for non-electrons.
- Scale 1 BH micro-scenario constants duplicated in `app.js:144-152` and `scales/scale1/controller.js:64-69`. The `app.js` copy is dead.
- Scale 2 `massInKB` key is divided by `M_E_PHYS` (PDG), not `K_B` (anchor). Downstream importer reading `massInKB` and multiplying by `K_B` gets 0.2% error.
- K_GENESIS is canonically threaded everywhere except `viewport/field-renderer.js:1429` (hardcoded `1.533`).

### G-6 Reference frame structure vocabulary sweep incomplete

The 2026-05-01 sweep (`REF_REFERENCE_FRAME_VOCABULARY.md`) replaced "reference frame context" with "reference frame structure / frame-relative frame dynamics" in load-bearing physics text. Sweep applied to manuscript + whitepaper + most theory docs; **dashboard FAQ + knowledge base were not swept**. ~10 entries in `ui/components/faq/data.js` and `ui/components/knowledge-base/data.js` retain old vocabulary.

### G-7 Index drift

`docs/SPEC_S0_QUANTUM_OVERLAYS.md` referenced at `scales/scale0/state/store.js:20` and in several tooltip comments — not present in `docs/`. Either land or remove breadcrumb.

7 of ~14 FAQ `theoryRefs` point to files that no longer exist post-May-2026 corpus consolidation. Repoint via `META_INDEX.md` / local `INDEX_*.md`.

### G-8 Test coverage

`engine/web/tests/*.spec.js` (18 specs, ~3500 LOC) covers: panel mount, scene panel, animation clock, force-field samplers, scenario parity, perf baseline, audit regression, scales, playback, panel-mount integration, color ramps, FAQ, math formatting, panels redesign, timeline buffer, tmp scenario audit, verify panel, WASM scenario coverage. **Not covered:** any visual-regression for the 30+ Scale 0 overlay shaders; the Scale 4 G-constant value; the Scale 5 telemetry key format; the Scale 6 BCC/FCC sublattice labeling. These are exactly the P0 bugs that landed silently.

---

## Section H — Open questions

H-1. **Born-rule presentation policy.** The FAQ tags it `[SELECTION]` with explicit "10× lattice bias unaccounted"; the scenarios.js tag is `[THEOREM]`. The honest tag is `[SELECTION]` per LEDGER (no `[THEOREM]` row in `SPEC_ALGEBRAIC_SPINE.md`). Confirm before downgrading scenarios.js.

H-2. **Cosmic DM:baryon scenario split.** Should scenarios match the FTD theorem `17/27 ≈ 63%`, or should the "Cosmology (FTD)" panel acknowledge that the 0.85/0.15 split is `[IMPOSED]` for visual prominence (matching observed Universe Planck 2018, not FTD theorem)? The current state is silently inconsistent; user must choose.

H-3. **Schwarzschild claim in cosmic info panel.** The panel formula `r_s = 2 G_N M` is correct for Schwarzschild physics but is not what the engine renders (`cbrt(M) · 0.35`, constant-density). Either implement Schwarzschild (recompute event-horizon radius linearly in M with appropriate G_N) or replace the panel text with the constant-density formula. Visual scaling matters: if you switch to linear-in-M, BH visual size will change dramatically across the mass range.

H-4. **Friedmann implementation.** Static Hubble + scale factor is honest as "anchor display" but presented as live diagnostic. Implementing Friedmann is non-trivial (initial conditions, normalization, time-step). Punt as an `[OPEN]` feature, or silently label "(anchor)"?

H-5. **Scale 4 G constant.** Switching from `G_N = 0.01` to `G_HELIOCENTRIC = 4π²` will speed every Scale 4 orbit by ~63×. Visual cadence breaks — Earth's year becomes 1 sim year as intended. Acceptable for a "scientific demo" but the current "decorative slow" cadence may be a deliberate UX choice for visibility. Confirm intent before fixing.

H-6. **AE units calibration vs relabel.** Scale 2 cards labelled "eV" carry sim-unit values. Calibrating the AE_* tuning chain to true eV would re-derive all MD parameters. Relabeling "sim units" is honest but loses pedagogical concreteness (room temperature ≈ 298 K is meaningful; "298 sim units" is not). Confirm direction.

H-7. **Scale 6 numeric key.** `scale6/` or `scale12/`? CSS class is `.scale12-only`; controller class is `Scale6LifecycleController`; engine-mode value is `'meta'`. Consolidating to a single number requires choosing one. Recommend renaming to "meta" everywhere and dropping the numeric key.

H-8. **Steinmetz 2026 provenance.** Five "theorems" surface in `ontic-observatory.js` under that attribution. No paper in `docs/papers/`, no LEDGER row, no arXiv id. Either source the paper or demote labels to "Proposition" / "Heuristic".

H-9. **`x₋` row removal vs preservation.** Removing the row from the Ontic Chain panel is one option; preserving as "mathematical root of master quadratic; identification with N_c is RETIRED" is another. The second is more pedagogically informative. Confirm preference.

H-10. **Reference frame structure sweep scope.** Should the dashboard FAQ entry titled "How does FTD address the hard problem of reference frame context?" be retitled per `REF_REFERENCE_FRAME_VOCABULARY.md`, or kept as-is to address users searching for "reference frame context"? The sweep policy is silent on user-discoverability vs canonical vocabulary trade-offs in FAQ contexts.

H-11. **Bond-order inference vs molecule-string fix.** For Scale 3, two paths to fix P0-13: (a) implement bond-order inference (distance threshold tightening + valence saturation) so O₂ renders as a double bond, or (b) strip multi-bond claims from the molecule panel strings. Path (a) is more work but better pedagogy.

H-12. **`G_N` framework-integer reading status surfacing.** The FAQ entry on FTD-0131 should mention the closed-negative status; but the substrate-derived `α_G(e,e)` is not currently surfaced anywhere in the dashboard. Should it be? Surfacing requires a new dashboard element + a clear distinction between "gravitational fine-structure ratio" and "Newton's G".

---

## What is good (audit positives)

- `constants.js` is exemplary — well-commented, layered by ontic level, explicit about which symbol belongs to which Coulomb-convention bucket. K_B / Boltzmann disambiguation is a textbook example of how to handle naming collisions (lines 71-86).
- FAQ at `ui/components/faq/data.js` is the most epistemically honest text surface in the codebase. Drift sits in `constants.js` JSDoc tags, `app-ontic.js` panel labels, and `config/scenarios.js` `[THEOREM]` overclaims — not in the FAQ.
- `BaseLifecycleController` (`lifecycle.js`) handles listener/timer/Three.js-dispose recursion cleanly; Scale 4/5/6 use it well (modulo the camera-restore gap).
- Scale 11 deletion confirmed clean across JS source. Doc and test-helper references are stale (P2-13).
- `engine/web/tests/scenario-parity.spec.js` is the right regression gate for JSC++ scenario drift and is passing.
- No occurrence of retracted "1.23×" Phase-F or "160× QED β" claims in the web codebase (positive audit result).
- Cleanup of the pre-F9 `PROTON_RATIO` formula is complete on the JS side; remaining work is to propagate the same fix to C++ + `proof_complete_sm.py`.

---

## How this audit was produced

12 agents dispatched in parallel:
- 7 scale experts (scale0-lattice, scale1-particle, scale2-atom, scale3-molecular, scale4-planetary, scale5-cosmic, scale6-meta)
- 5 cross-cutting (physics-orchestrator, engine-expert, constants-sentinel, refactoring-analyst, epistemic-auditor)

Each agent returned a structured 6-part report covering inventory,
motivation, accuracy, wiring, performance, and consolidation in its
domain. This document synthesizes those reports into a single ticketed
audit. Original per-agent reports are preserved in the conversation
transcript that produced this file.
