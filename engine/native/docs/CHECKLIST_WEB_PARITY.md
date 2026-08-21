# CHECKLIST — Native App Web-Dashboard Parity

**Status:** `[TRACKER]` · **Created:** 2026-08-20 · **Source:** derived from [`REF_WEB_ENGINE_ARCHITECTURE.md`](REF_WEB_ENGINE_ARCHITECTURE.md) (the web-dashboard map) scored against the current `engine/native` rebuild.

This is the running parity checklist: every capability the web dashboard (`engine/web`) offers, marked by where the **native** app (`engine/native`, RmlUi + D3D12, in-process C++/CUDA) stands. It supersedes the reference's §11.3 gap table, which described the *old* `engine/native_desktop` shell before this rebuild.

**Legend:** ✅ done · ◐ partial · ▢ not started · ⭐ native-only (beyond web)

**Where we stand:** the whole **Scale-0 substrate surface is at or beyond web parity** — real in-process engine (CPU+CUDA interop), the full 7-column overlay menu (32/33 overlays, incl. movable rubber sheets that the web can't do), the scenario picker over a native 130-row catalog, click-to-inspect, a live energy chart, and 125 fps. The **big remaining frontier is the other scales (1 GPU, 2/3, 4, 5, 6)** and the **specialized telemetry/chart panels**.

---

## A. Multi-scale hosting

| Item | Status | Note |
|---|---|---|
| `ScaleEngine`-per-scale seam (replaces the 4-bridge web transport) | ✅ | `ScaleHost`/`ScaleAdapter`; host names no concrete scale type |
| In-process (no bridge/worker/WASM/WebSocket) | ✅ | §12.1 collapse — done by construction |
| Scale 0 Lattice — `RenderBridge`, in-process, CUDA↔D3D12 interop | ✅ | real engine, GPU device-resident particles |
| Scale 1 Particle — `ParticleEngine` adapter | ◐ | adapter live; **CPU-only** (`set_use_gpu(false)`) — the `gpu_particle_engine` CUDA backend is not wired yet |
| Scale 2/3 Atom/Molecule — `AtomEngine` adapter | ▢ | native engine exists; needs the **Planck↔Bohr unit shim** + adapter + delete-mock (the biggest reuse lever, §5.2) |
| Scale 4 Planetary | ▢ | **no native engine** — write new C++ N-body (reuse cosmic Barnes-Hut/Verlet) |
| Scale 5 Cosmic — `CosmicEngine` adapter | ▢ | native engine compiled + CTest-covered but unwired; adapter + delete-mock |
| Scale 6 Meta — geometry | ▢ | pure-geometry D3D12 port (27-site Moore, polyhedra, labels) |
| Live scale switch in-app | ✅ | Lattice ⇄ Particles toolbar switcher (extends to more scales trivially) |

---

## B. Scale-0 rendering & field overlays

| Item | Status | Note |
|---|---|---|
| Lattice / particle / flux draw (D3D12) | ✅ | sprite + line PSOs; interop particle path |
| Point-sprite billboard system (the big renderer item — D3D12 has no `gl_PointSize`) | ✅ | instanced camera-facing quads |
| Multi-overlay compositing + 7-column menu | ✅ | data-driven `scale0_overlays.h` registry, mirrors the web's Volume/Fields/Forces/Quantum/Topology/Stress-Energy/Phenomena columns |
| COVERED overlays (∇·J, State, Latency+PoissonLatency, Gauss, Horizon, Poynting, EM/Gravity/Strong/∇×J arrows, Flux Volume) — 11 | ✅ | real engine fields via `copy_visual_field_sample` |
| EXTEND overlays (\|ψ\|², ℒ, Entropy, Chirality, Dual J, DM Halo, Genesis, Damping, Confinement, Phase, Flux Slice, Color charge) — 12 | ✅ | derived scalars / dense bands / line emission |
| Streamlines — Flux Lines / E Field / B Field (RK4) — 3 | ✅ | `streamlines.{h,cpp}`, importance/particle-anchored/ring seeds |
| Rubber sheets — Φ/EM-energy/Charge/Vorticity/P_E/P_B — 6 | ✅ | new triangle-mesh vertex-color PSO |
| **Movable rubber sheets** (slice at adjustable height) | ⭐ | slice probe via panel −/＋ + Shift+scroll — the web sheets are fixed |
| **Knot Zones** (the 33rd overlay) | ▢ | needs a knot-detection pass over the E/B streamlines |
| Force render-styles: Arrows | ✅ | |
| Force render-styles: Heatmap (gaussian sprite) / Glyphs (instanced cone) / Flow (dashed streamlines) | ▢ | 3 remaining styles for the 4 force overlays |
| OIT / P1 translucency | ◐ | sheets use depth-test/no-write + draw-order; no true OIT (acceptable, mild blend-order artifacts) |
| `ℒ`/`∇` glyphs render in the shell font | ▢ | RmlUi font gap — labels show as boxes; swap font or ASCII-ize |

---

## C. Telemetry & charts

| Item | Status | Note |
|---|---|---|
| Live telemetry chart element | ✅ | custom `<ftd-chart>` (RmlUi + D3D12); total-energy series, CPU **and** GPU (energy-ledger fix) |
| Full telemetry hub (s0-core 9ch, audit 15ch, lagrangian 10ch, s1 25ch, …) | ▢ | `History` + `NativeTelemetryScheduler` exist in-engine but are largely unwired to the UI |
| Chart **panels** (diagnostics, audit, lagrangian, conservation) | ▢ | only the single energy series is charted so far |
| Per-group telemetry provenance / staleness model (§7.2 — worth keeping) | ▢ | maps onto `NativeTelemetryScheduler` cadences; not ported |

---

## D. Inspector

| Item | Status | Note |
|---|---|---|
| Click-to-inspect (viewport pick → panel) | ✅ | ray-pick; Scale-0 voxel State/Flux/\|J\|/Div/Curl, Scale-1 particle charge/pos/vel |
| "Pending, never fabricated void" honesty | ✅ (n/a async) | native reads are synchronous — no async budget needed |
| Full 20+ scalar voxel readout + 26-neighbour cursor | ◐ | core fields shown; the full field set + neighbour walk not yet |

---

## E. Scenarios & configuration

| Item | Status | Note |
|---|---|---|
| In-app scenario picker (searchable, category-grouped, live load) | ✅ | 5 honest classes, collapsed-by-default (fps), `LoadScenario` |
| Native scenario catalog / `ScenarioMeta` (id/title/category/tags/epistemic) | ✅ | `scenario_catalog.h`, 130 rows, set-equality guard vs `scale0_scenario_ids()` |
| `dispatch_scenario` live path (delete JS seed mirror + parity guard) | ✅ | in-process; W9 half-mutation guarded in `ScaleHost::boot` |
| All 130 scenarios verified (audit) | ✅ | 129 render / 1 intended-empty / 0 rejected / 0 hard errors ([AUDIT_SCENARIOS.md](AUDIT_SCENARIOS.md)) |
| Constants from `ontic.h` (delete JS mirror) | ✅ | native uses `ftd::` constants; no JS copy |
| Lattice-size change in-app | ▢ | shown but not editable (needs a `SetRunConfig` reload knob in the UI) |
| Prime-tick-on-load | ✅ | one tick at boot so paused overlays have data |

---

## F. Toggles (physics-term + config)

| Item | Status | Note |
|---|---|---|
| Physics-term toggles panel (click to toggle, reflects engine truth) | ◐ | works from the snapshot's `term_toggles`; need to confirm all 44 `TOGGLE_SPECS` rows + the 10 config fields + `validate()` conflicts are surfaced |
| Config knobs (dt, SOR iters, boundary, damping…) | ◐ | `SetDt`/`SetSorIterations`/`SetBoundary` commands exist; not all exposed in the UI |
| Toggle table from `TOGGLE_SPECS[]` directly (no JS whitelist) | ✅ | native reads `TermToggles` |
| Render-only overlay toggles kept separate from engine toggles | ✅ | overlays are adapter view-state, never touch the bridge |

---

## G. UI shell & panels

| Item | Status | Note |
|---|---|---|
| Shell (toolbar / viewport hole / setup / physics+overlays / status bar) | ✅ | RmlUi + RCSS, composited over the live D3D12 scene |
| Scale-aware panels (Scale-0 toggles vs Scale-1 readout) | ✅ | data-if per active scale |
| Scrollable panel region | ✅ | overflow-y auto |
| The 22 web panels (diagnostics, charts, telemetry-grid, lagrangian, scene, zoo, ontic, flux-slice, wave-lab, p1-observables, spectrum, gravity, time, thermo, dispersion, knots, scale-context, symmetry, genesis, conservation) | ▢ | ~4 core surfaces done (setup, physics/overlays, chart, inspector); the ~18 specialized panels not ported |
| Docking (drag / resize / float panels) | ▢ | native uses fixed flex layout, not draggable docks |
| Settings (theme / density / persisted prefs) | ▢ | single dark theme only |
| Knowledge base / FAQ / keyboard-help / tooltips | ▢ | deferred by owner ("worry about FTD/FAQ later") — static string tables when wanted |
| JetBrains Mono font | ▢ | currently Inter aliased; needs a real TTF |

---

## H. Build, test, CI

| Item | Status | Note |
|---|---|---|
| Native build (`build_native.bat`, MSVC 14.44) | ✅ | one binary, real threads + CUDA |
| Golden gate green (physics invariant) | ✅ | 7/7 every commit |
| RmlUi headless smoke + scenario-catalog set-equality tests | ✅ | run directly |
| Native app tests registered in CTest / CI | ◐ | built every build; **not** ctest-registered (deferred R0 — parent `engine/CMakeLists.txt`, entangled with the held-off bundle) |
| Full CTest pyramid (device / interop / neutrality / journal-replay) | ◐ | some device/interop tests exist; not all wired for the new tree |

---

## I. Architecture invariants (§12 — keep, don't regress)

| Item | Status |
|---|---|
| Sim-thread-owns-engine / GUI-thread-reads-snapshot | ✅ |
| Tick-boundary command drain (no mid-tick writes) | ✅ |
| Golden gate never perturbed by the UI | ✅ |
| GPU-native renderer data (CUDA↔D3D12 interop, no CPU marshaling for particles) | ✅ |
| One force implementation (real engine `force_diag_` / field kinds, not the divergent WASM samplers) | ✅ (overlays) |
| Clean process exit (no `TerminateProcess`) | ✅ |
| Epistemic tags visible (scenario catalog carries them) | ◐ (catalog yes; not yet surfaced in the picker UI) |

---

## Recommended order (highest leverage first)

1. **Finish Scale-0 overlays** — Knot Zones + the 3 force render-styles (Heatmap/Glyphs/Flow) + the `ℒ`/`∇` font fix. Small, closes the overlay surface to 100%.
2. **Scale 1 on CUDA** — wire `gpu_particle_engine` in the Scale-1 adapter (remove the CPU-only cap). Pure win, seam already proven.
3. **Scale 5 Cosmic + Scale 2/3 Atom** — wire the existing dark native engines (Cosmic is a straight adapter; Atom needs the Planck↔Bohr unit shim), delete the mocks. The accuracy win the port promises.
4. **Telemetry panels** — wire `NativeTelemetryScheduler`/`History` to real chart panels (diagnostics/audit/lagrangian/conservation) with the provenance model.
5. **Scale 4 Planetary + Scale 6 Meta** — one small new N-body + one geometry port.
6. **Shell polish** — config knobs (lattice/dt) in-app, docking, settings, JetBrains Mono; content (KB/FAQ) last.
7. **CI hardening** — register the native tests in CTest once the held-off shared-tree bundle clears.
