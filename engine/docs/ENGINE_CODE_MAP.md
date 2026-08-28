# Engine Code Map — a navigation guide for future AI agents

This is a **file- and subsystem-level map** of `engine/`. It answers "what code
lives where, what does each part do, and which files are worth splitting." It is
an **engineering map, not a proof ledger** — physics-claim status lives in
`docs/SPEC_FTD.md` and the assessment ledgers.

It complements, and does not duplicate, the existing conceptual docs:

| Read this for… | Doc |
|---|---|
| Mental model of the simulation | [`engine/VISUAL_GUIDE.md`](../VISUAL_GUIDE.md) |
| Tick call stack, memory ownership, backend sync | [`engine/ARCHITECTURE.md`](../ARCHITECTURE.md) |
| Living canonical engine reference + golden hash | [`engine/SPEC_ENGINE.md`](../SPEC_ENGINE.md) |
| Feature → phase/kernel call graphs | [`engine/CALLSTACKS.md`](../CALLSTACKS.md) |
| Scenario lifecycle + cross-scale seeding | [`engine/SCENARIO_ARCHITECTURE.md`](../SCENARIO_ARCHITECTURE.md) |
| Web cross-module contracts | [`CONTRACTS.md`](../../CONTRACTS.md), [`engine/web/ARCHITECTURE.md`](../web/ARCHITECTURE.md) |
| **Every file, one line each (machine-readable)** | [`ENGINE_FILE_MANIFEST.json`](ENGINE_FILE_MANIFEST.json) / [`.md`](ENGINE_FILE_MANIFEST.md) |

> **The manifest is the card catalog; this map is the floor plan.** When you need
> "which file is X," grep the manifest. When you need "how is this subsystem
> shaped and where are the seams," read this.

---

## 0. The engine at a glance

**~236,000 LOC across 1,174 tracked files** (419 `.cpp`, 383 `.js`, 129 headers,
21 `.cu`/`.cuh`, plus CSS/MD/JSON/etc.). Two halves:

```
engine/
├── C++/CUDA NATIVE ENGINE  (~112k LOC, src+include+cuda+wasm+tests+sim)
│   ├── src/        17.4k  the production physics (Scale-0 RenderBridge + macro engines)
│   ├── include/    ~14k   headers (voxel, toggles, render_bridge API, constants chain)
│   ├── cuda/       ~6k    GPU mirror of the tick ladder (RTX 5090 via WSL2)
│   ├── wasm/       ~2k    Emscripten Embind bindings → browser
│   └── tests/      94.8k  ← 84% of the C++ LOC. CTest suite + measurement campaigns
│
└── WEB FRONTEND  (engine/web, 512 files, 383 JS, ~120k LOC)
    └── js/
        ├── app.js               composition root + RAF loop + CONTROLLERS map
        ├── scales/scale0..5/    per-scale controllers (0=lattice … 5=cosmic)
        ├── bridge/              3 backends behind one contract (WASM / worker / WebSocket)
        ├── viewport/            Three.js renderers (scene/flux/field/particle/molecular)
        ├── telemetry-hub.js     ring-buffer telemetry singleton (high fan-in)
        └── ui/                  dashboard shell, panels, components, charts
```

**Single source of truth for constants:** native side → `engine/include/ftd/ontic.h`
+ `constants.h`; web side → `engine/web/js/constants.js` (fan-in ≈ 66 files). All
physics numbers chain from `D=3` + `ϖ` (lemniscatic). Never hardcode; import.

**The one hard constraint on the native engine:** a **golden-hash regression gate**
(`engine/tests/test_render_bridge_golden.cpp`) freezes a CPU `RenderBridge` tick
sweep at L=17 to the hash **`0xb604d81a3d79366e`**. Any
change reachable from `RenderBridge::tick()` must be **byte-identical** — see §6.

---

## 1. C++ native engine — `src/`

The production physics. Already decomposed by prior refactor sweeps; the hot tick
loops live in their own translation units.

| Path | Role | Notable files |
|---|---|---|
| `src/render_bridge.cpp` (795) | Scale-0 tick orchestrator + accessors. Delegates hot loops. | the `tick()` ladder |
| `src/render_bridge_phases/` | The decomposed phase loops | `phase_read.cpp`, `phase_write.cpp` (manifestation/genesis), `phase_forces.cpp`, `phase_movement.cpp` |
| `src/poisson_solvers.cpp` | Gauss projection, Coulomb, latency Poisson (warm-started SOR) | — |
| `src/transmutation_phases.cpp` | Pair production, weak transmutation, proper time, **triad binding** | — |
| `src/injection.cpp` | How scenario seeds enter the lattice (feeds the golden tick) | — |
| `src/scenarios/` | Scenario library (C++ port of all Scale-0 scenarios) | **`s0_seed.cpp` (1109)** ← split candidate, `s0_field.cpp`, `flux/light/quantum.cpp` |
| `src/cli_demos/` | Headless demo runners (orchestration only, no tick physics) | **`cli_demo_scenarios.cpp` (981)** ← split candidate |
| `src/atom/` | Scale-2 atom engine forces | `atom_forces.cpp` (632) |
| `src/cognition/` | Standalone `ftd_cognition` lib (own exe; **not** in the tick path) | `cognitive_lattice.cpp` (835) |
| `src/vtk_export.cpp` (636) | ParaView/CSV export (pure I/O, zero physics) | ← split candidate |
| `src/particle_engine.cpp` (669) | Scale-1 macro engine (Barnes-Hut N-body) | — |
| `src/scale_bridge.cpp` | Cross-scale coarsen/refine between engines | — |

**Macro engines** (`ParticleEngine`, `AtomEngine`, `CosmicEngine`, `DagEngine`)
implement the `ScaleEngine` interface but are coarser analytical layers, **separate
from the Scale-0 golden path**. `DagEngine` is an experimental sparse-lattice
prototype, not used for physics claims.

## 2. Headers — `include/ftd/` (129 files)

The public API + data model. Read these first when extending physics:

- `voxel.h` — what a lattice site carries (state, flux, wave_vel, velocity, identity, optional sectors).
- `term_toggles.h` — the runtime feature surface (every physics extension is a default-OFF toggle). **Adding a field here requires a clean rebuild** — a stale struct layout breaks the golden hash silently.
- `render_bridge.h` (556) — Scale-0 public API + inline differential operators (laplacian/divergence/curl — intentionally hot-path inlines, golden-risk to touch).
- `ontic.h` + `constants.h` — the canonical constant chain (`ALPHA`, `G_STAR`, `N_C`, `K_B`, `K_GENESIS`, `M_REST`, `C_SPEED`, …). Not magic numbers — the single source of truth.
- `engine_state.h` (408) — `TernaryField` (carries ~100 LOC of inlined bodies → compile-fan-out split candidate, golden-risk).
- `test_telemetry.h` — the `ftd::test` framework (`init/section/check/finalize`).

## 3. GPU — `cuda/` (21 files)

A SoA mirror of the CPU tick ladder for the RTX 5090 (**run via WSL2, never
Windows-native CUDA for campaigns**). Already partly consolidated (F-8/ADR-0007
gave `cuda_index.cuh`, `kernels_stencil_common.cuh`). **Not covered by the golden
hash** — guarded by separate `gpu_parity` tests instead.

| File | Role |
|---|---|
| `gpu_engine.cu` (666) | GPU tick orchestration + host/device sync (clean) |
| `gpu_buffers.cu` (770) | Device allocation + lazy PCIe transfer (clean) |
| `kernels_stencil_single.cu` / `_dual.cu` | 18-point Moore stencil, single vs dual substrate (~85% shared — templating is high parity-risk, deferred) |
| `kernels_forces.cu` (1027) | Force kernels (Coulomb/gravity/Lorentz/color) |
| `kernels_poisson.cu`, `kernels_eft.cu`, `kernels_aux.cu` | Solvers + EFT + auxiliary |
| `experimental_discrete_universe.cu` (459) | Deliberate standalone prototype (built; uses its own namespace — not the production path) |

Known small duplication: `CUDA_CHECK` redefined 11×, local `wrap`/`idx3d` in two
kernels — see §7 tickets T2/T8.

## 4. WASM bridge — `wasm/` (6 files)

`ftd_wasm.cpp` (1500) + `bindings_{render_bridge,particle,atom}.cpp` — each its own
`EMSCRIPTEN_BINDINGS` block (the canonical 4-TU Emscripten split). Surfaces the
engine to the browser as typed-array views (zero per-frame serialization). This is
the **only** physics path the web dashboard's **Scale 0** uses.

## 5. Web frontend — `engine/web/js/` (383 JS)

One ES-module graph, no bundler. **Single live dashboard:** `index.html` →
`js/app.js`. (The historical `index_dag.html` dual-dashboard is fully retired.)

### 5.1 Layering

```
index.html → app.js  (composition root, RAF loop, CONTROLLERS map, switchEngineMode)
   │
   ├── Scale controllers   scales/scaleN/controller.js   uniform {mount,destroy,animate,reset,loadScenario}
   ├── Bridge layer        bridge/ + ws-bridge.js          3 backends, 1 contract
   ├── Viewport/renderers  viewport.js + viewport/*        Three.js
   └── telemetry-hub.js → ui/ (shell, panels, components, charts)
```

### 5.2 Scale-controller pattern

`app.js` holds a `CONTROLLERS` map: `lattice→0, particles→1, atoms→2, molecules→3,
planetary→4, cosmic→5`. `switchEngineMode()` is the **sole** mode-transition entry
point (tears down prev, mounts next). Controllers read live `app.js` state through
a single `ctx` object (`_makeCtx()` getters/setters) — never snapshots.

- **Scale 0 (lattice)** is by far the richest: its own `scale0/{runtime,state,ui,analysis,data}` subtree (~14k LOC) + lavish docs (8+ `SPEC_SCALE0_*`, `TOGGLE_REGISTRY.md`, `TELEMETRY_CATALOG_SCALE0.md`).
- **Scales 1–5** are single-controller + scenarios + small `ui/`. **Largely undocumented** (see §8) — the per-scale subagents hold that knowledge.
- **Scale 6 (Meta) is orphaned** and **Scale 11 (reference-frame-context) is not wired** into `CONTROLLERS` (see §9).

### 5.3 Bridge layer — 3 backends, 1 contract

- **Contract:** `bridge/bridge-contract.js` (`@typedef ScaleBridge` + the `SCALE0_DIRECT_READS` anti-drift list of 28 methods the worker proxy must forward).
- **Backends:** `WasmBridge` (in-thread Embind), `WasmBridgeProxy` → `wasm-bridge.worker.js` (off-thread, same C++ engine, zero-copy flux over a `SharedArrayBuffer`), `WebSocketBridge` (`ws-bridge.js` → native `ws_server.exe`, auto-GPU). `app.js init()` tries WebSocket first (5 s timeout), then falls back to WASM.
- **Capability factories** (`capabilities/install.js`, CONTRACTS.md §2): controllers only touch `bridge.capabilities.scaleN.*`; factories take the **live** bridge and return delegating closures — so the backend is interchangeable. **Never destructure the bridge** (live-ref invariant).

### 5.4 Viewport/renderer layer — the best-refactored part

`viewport.js` (1257) is a thin orchestrator composing Phase-3 sub-renderers:
`scene-core.js`, `flux-renderer.js`, **`field-renderer.js` (2807 — largest file in
the engine)**, `particle-renderer.js`, `molecular-renderer.js`,
`topology-sheet-renderer.js`, plus `mesh-factory.js`, `shaders.js`, `color-ramps.js`.

> **Renderer invariant:** `field-renderer.js` line ~43 — `VOXEL_CENTER_OFFSET = 0.0`
> ("DO NOT fix to 0.5"). Any split must keep this in the shared core, not duplicate it.

## 6. Test architecture & the golden gate

The test tree is **84% of the engine's C++ LOC** (94.8k / 339 files) — the biggest
refactor lever, and the **lowest-risk** because tests don't touch the golden hash.

- **Framework:** `ftd::test` (`include/ftd/test_telemetry.h`) — `init/section/check/finalize`, feeds the live runner. Newer tests use it; two legacy GPU megafiles (`test_gpu_physics.cpp`, `test_gpu_experiments.cpp`) still roll their own `CHECK` macros + duplicate helpers.
- **Shared fixtures:** `tests/support/bridge_fixtures.{h,cpp}` (`make_bridge`, `run_for`, `inject_particle_at_center`, `assert_energy_conserved`) — **underused** by the large files; adopting it is where boilerplate savings land.
- **CTest labels:** a 29-tag taxonomy already exists (`eft` 76, `native` 70, `unit` 29, `gpu` 16, …); **177/186 registrations labeled** — extend it, don't replace it (9 unlabeled + the megafiles need tags).
- **The golden gate** (`test_render_bridge_golden.cpp`, hash `0xb604d81a3d79366e`): hashes a CPU tick sweep at L=17. **GOLDEN-RISK** = anything reachable from `RenderBridge::tick()` (`render_bridge*`, `render_bridge_phases/`, `transmutation_phases.cpp`, `injection.cpp`, `poisson_solvers.cpp`, scenario *seeding*). Treat any hash change from a "pure code-motion" refactor as a regression to investigate — **never re-pin to make a refactor pass**.

---

## 7. Largest files & split status

Verified by deep read of the top ~40 files. **"GOLDEN-SAFE"** = test/CLI/VTK/web
(no golden exposure); **"GOLDEN-RISK"** = tick-path, requires byte-identical
code-motion + golden re-verify after each step.

### Native (C++/CUDA)

| File | LOC | Split? | Seam | Risk |
|---|--:|---|---|---|
| `tests/test_gpu_physics.cpp` | 2618 | ✅ → 5 TUs + `gpu_test_fixtures.h` | 27 independent `test_*()` by domain | GOLDEN-SAFE |
| `tests/campaign_dark_sector.cpp` | 1762 | ✅ → 3 TUs | 7 independent `section()` blocks | GOLDEN-SAFE |
| `tests/test_gpu_experiments.cpp` | 1679 | ✅ → 3–4 TUs + helpers | 8 independent experiments | GOLDEN-SAFE |
| `wasm/ftd_wasm.cpp` | 1500 | ⚠️ already 4-TU split | per-engine `EMSCRIPTEN_BINDINGS` | leave |
| `tests/test_constructors.cpp` | 1354 | ❌ leave | Level 0→8 hierarchy is meaningful | — |
| `tests/campaign_hydrogen_spectrum.cpp` | 1281 | ✅ → 3 TUs + `hydrogen_scales()` | 5 sections, 3× dup block | GOLDEN-SAFE |
| `tests/campaign_graviton_tt_correlator.cpp` | 1220 | ❌ extract headers, don't split | single instrument (FFT→Prony) | — |
| `tests/benchmark_engine_theory.cpp` | 1145 | ✅ → 3 TUs + `benchmark_utils.h` | 16 independent benchmarks | GOLDEN-SAFE |
| `tests/benchmark_black_hole_thermo.cpp` | 1132 | ✅ → 2 TUs + shared utils | 6 benchmarks | GOLDEN-SAFE |
| `src/scenarios/s0_seed.cpp` | 1109 | ✅ → 6–8 family TUs (one at a time) | ~42 scenarios by family | **GOLDEN-RISK** |
| `tests/test_pe_forces.cpp` | 1080 | ❌ leave | already clean by force type | — |
| `cuda/kernels_forces.cu` | 1027 | ❌ extract device helpers | tier-2 gradient/density dup | parity-risk |
| `src/cli_demos/cli_demo_scenarios.cpp` | 981 | ✅ → 4 thematic TUs | 11 self-contained demos | GOLDEN-SAFE |
| `src/cognition/cognitive_lattice.cpp` | 835 | ⚠️ defer | decoupled subsystem | not golden |
| `src/render_bridge.cpp` | 795 | ❌ already decomposed | hot loops already in phases/ | — |
| `src/vtk_export.cpp` | 636 | ✅ → 6 I/O TUs | pure I/O by export type | GOLDEN-SAFE |

### Web (JS)

| File | LOC | Split? | Seam | Risk |
|---|--:|---|---|---|
| `viewport/field-renderer.js` | 2807 | ✅ → core + em/forces/quantum/substrate | 27 `_build*`/`update*` pairs by field family | low |
| `ui/components/knowledge-base/data.js` | 1807 | ✅ shard by section (best value/risk) | pure data | near-zero |
| `app.js` | 1730 | ✅ → app-wire/{controls,scenarios} + app-frame/reset | lifecycle vs UI-wiring (35 fns) | low-med |
| `bridge/mock-atom-engine.js` | 1284 | ✅ → ae-{tables,bonding,forces,integrator,readouts} | by responsibility | med (no parity gate) |
| `viewport.js` | 1257 | ❌ leave | thin forwarding orchestrator | — |
| `scales/scale0/runtime/field-overlays.js` | 1102 | ✅ → builders + scheduler | build* vs frame-budget | med |
| `scales/scale0/ui/overlays/flux-slice-panel.js` | 1078 | ✅ → sampling helpers + panel | pure slice-math vs DOM | low-med |
| `scales/scale0/scenario-registry.js` | 1006 | ✅ shard 115 decls by family | data + 4 fns | low |
| `telemetry-hub.js` | 927 | ⚠️ extract ring-buffers now; defer the singleton | 4 classes; hub is a god-object | low / high |

**Recommended sequencing:** zero-risk native wins first (dedup `benchmark_utils.h`,
`cuda_error_check.cuh`, label the 9 tests) → GOLDEN-SAFE test/CLI/VTK splits → web
Tier-A splits (shard `data.js` first) → parity-gated CUDA → GOLDEN-RISK tick-path
last, each with a golden re-run. **Full prioritized ticket list (T1–T19) is in §7 of
the C++ analysis and §1 of the web analysis** captured in the session report.

---

## 8. Documentation gaps for a cold AI agent

What exists is strong but **Scale-0-weighted**. The real gaps:

1. **Scales 1–6 have no README/SPEC/module index.** An agent must reverse-engineer particles/atoms/molecules/planetary/cosmic from controller headers. The knowledge lives in the **per-scale subagents** (`scale1-particle-expert` … `scale6-meta-expert`) — a cold agent that doesn't invoke them is stuck. **Biggest gap.**
2. **No other maintained per-file index for the 383-file JS tree.** This map + the manifest fill it; keep them current.
3. **The renderer/app/telemetry megafiles have no dedicated doc** beyond one ARCHITECTURE paragraph each (the bridge big files *are* well covered by 3 excellent module READMEs).
4. **Zero-doc subsystems:** `js/ui/`, `js/telemetry/`, `js/audio/`, `js/backgrounds/`, `js/cosmic/`, `js/inspector/`, `js/orbitals/`, `js/physics/`, `js/core/`, `js/config/`, `js/atlas/`.
5. **Two stale docs (self-flagged):** `engine/PHYSICS_STATUS.md` (2026-04-17) and `engine/CHECKLIST_PHYSICS.md` (2026-04-13) defer to SPEC_ENGINE. `viewport/REFACTOR_MAP.md` is CLOSED/historical with stale line numbers.
6. **Two ADR directories** (`engine/web/docs/adr/` has 2; the rest resolve to project-root `docs/adr/`) — a navigation trap.

**Highest-leverage doc follow-ups:** one-paragraph READMEs for scales 1–5 mirroring
scale0's; refresh-or-retire `viewport/REFACTOR_MAP.md`; keep this map + manifest in sync.

## 9. Dead / superseded code (flag for owner — `git mv`, don't erase)

| Item | LOC | Status |
|---|--:|---|
| Former `bridge/scenarios/` JS seed dispatcher + 6 group files | ~2000 | **Archived 2026-08-27** — live Scale-0 calls native C++ `setupScenario`; the still-live wave analysis and genesis-panel term profile were extracted into the Scale-0 package. (NB: `cosmic-scenarios/` via `mock-scale5.js` **is** live — don't confuse.) |
| Meta/Scale-6 triad (`meta-unit.js`, `meta-unit-geometry.js`, `meta-pedagogy.js`) | ~1270 | **Live** — reconnected through `scales/scale6/controller.js` and `app.js`. |
| `MockBridge` doc ghosts (`bridge/README.md`, `bridge-contract.js`, `bridge-init.js` JSDoc) | — | Reference deleted `mock-bridge.js`/`mock-diagnostics.js` — cheap doc fix. |
| Scale-1/2 mock-vs-C++ drift | — | Not dead, but **un-gated**: web Scale-1/2 physics is JS-only (`_aeHasWasm=false`); only Scale-0 has the golden gate. Any physics fix must be hand-mirrored. |
| `engine/archive/` (12 files, ~5.6k LOC) | — | **Exemplary** provenance graveyard with `README.md` closure map — the model to follow, no action. |

`engine/` also has several stale build dirs (`build/`, `build_cuda/`,
`build_wasm*/`, `build_wsl/`) — disk clutter; confirm `.gitignore` coverage.

---

## 10. How to keep this current

- **Manifest** regenerates: `python engine/tools/build_file_manifest.py` (rerun after adding/removing/renaming code files).
- **This map** is hand-maintained narrative — update §7/§9 when a split lands or dead code is archived.
- When you split a file, update its row in §7 and re-run the manifest.
