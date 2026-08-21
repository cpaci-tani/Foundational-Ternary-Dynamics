# CHECKLIST — Native App Web-Dashboard Parity

**Status:** `[TRACKER]` · **Created:** 2026-08-20 · **Updated:** 2026-08-21 · **Source:** derived from [`REF_WEB_ENGINE_ARCHITECTURE.md`](REF_WEB_ENGINE_ARCHITECTURE.md) (the web-dashboard map) scored against the current `engine/native` rebuild.

This is the running parity checklist: every capability the web dashboard (`engine/web`) offers, marked by where the **native** app (`engine/native`, RmlUi + D3D12, in-process C++/CUDA) stands. It supersedes the reference's §11.3 gap table, which described the *old* `engine/native_desktop` shell before this rebuild.

**Legend:** ✅ done · ◐ partial · ▢ not started · ⭐ native-only (beyond web)

**Where we stand:** the **Scale-0 substrate surface is functionally COMPLETE and at/beyond web parity** — real in-process engine (CPU + CUDA interop), all 33 field overlays incl. the 4 force render-styles + movable rubber sheets (⭐ beyond web), the full 44-toggle + config-knob control panel with live validation, the diagnostics/conservation/lagrangian telemetry charts, the **full click-to-inspect with a walkable 26-neighbour cursor**, the scenario picker over a native 130-row catalog, and 60–140 fps. **Remaining on Scale 0:** only optional render polish — OIT (◐, acceptable) and JetBrains Mono (▢, needs a TTF) — plus surfacing epistemic tags in the picker (◐). **The bigger frontier is the other scales (1 GPU, 2/3, 4, 5, 6)** — deferred while Scale 0 is perfected.

---

## A. Multi-scale hosting

| Item | Status | Note |
|---|---|---|
| `ScaleEngine`-per-scale seam (replaces the 4-bridge web transport) | ✅ | `ScaleHost`/`ScaleAdapter`; host names no concrete scale type |
| In-process (no bridge/worker/WASM/WebSocket) | ✅ | §12.1 collapse — done by construction |
| Scale 0 Lattice — `RenderBridge`, in-process, CUDA↔D3D12 interop | ✅ | real engine, GPU device-resident particles |
| Scale 1 Particle — `ParticleEngine` adapter | ◐ | adapter live; **CPU-only** (`gpu_particle_engine` CUDA backend not wired) |
| Scale 2/3 Atom/Molecule — `AtomEngine` adapter | ▢ | native engine exists; needs the **Planck↔Bohr unit shim** + adapter + delete-mock |
| Scale 4 Planetary | ▢ | **no native engine** — write new C++ N-body |
| Scale 5 Cosmic — `CosmicEngine` adapter | ▢ | native engine compiled + CTest-covered but unwired |
| Scale 6 Meta — geometry | ▢ | pure-geometry D3D12 port |
| Live scale switch in-app | ✅ | Lattice ⇄ Particles toolbar switcher |

---

## B. Scale-0 rendering & field overlays — ✅ COMPLETE

| Item | Status | Note |
|---|---|---|
| Lattice / particle / flux draw (D3D12) | ✅ | sprite + line PSOs; interop particle path |
| Point-sprite billboard system (D3D12 has no `gl_PointSize`) | ✅ | instanced camera-facing quads |
| Multi-overlay compositing + 7-column menu | ✅ | data-driven `scale0_overlays.h` registry |
| **All 33 Scale-0 overlays** | ✅ | 11 COVERED + 12 EXTEND + 3 streamlines + 6 sheets + Knot Zones |
| Force render-styles: Arrows / Heatmap / Glyphs / Flow | ✅ | global selector; Glyphs = new instanced-cone PSO, Heatmap = gaussian sprite PSO, Flow = dashed streamlines |
| **Movable rubber sheets** (slice at adjustable height) | ⭐ | web sheets are fixed |
| `ℒ`/`∇` label glyphs | ✅ | Inter lacks them + no fallback face → ASCII-ized (div J / curl J / L(x)) |
| OIT / P1 translucency | ◐ | depth-test/no-write + draw-order; no true OIT (acceptable) |

---

## C. Telemetry & charts — ✅

| Item | Status | Note |
|---|---|---|
| Live telemetry chart element | ✅ | custom multi-series `<ftd-chart>` (RmlUi + D3D12) |
| Engine telemetry scheduler wired | ✅ | `NativeTelemetryScheduler` demand activated (was inert); GPU stale-epoch bug fixed |
| Diagnostics chart (energy / manifested / entropy / net charge) | ✅ | cadence-1 group, always on |
| Conservation/audit chart (accounted E / drift dE·dt / Gauss) | ✅ | demanded when the panel is open |
| Lagrangian chart (ℒ / ℋ) | ✅ | |
| Per-group provenance / freshness tick | ✅ | `t=NNN` per group |
| Inspector | see D | |

---

## D. Inspector — ✅

| Item | Status | Note |
|---|---|---|
| Click-to-inspect (viewport pick → panel) | ✅ | ray-pick; Scale-0 voxel State/Flux/\|J\|/Div/Curl, Scale-1 particle charge/pos/vel |
| "Pending, never fabricated void" honesty | ✅ (n/a async) | native reads synchronous |
| Full 20+ scalar voxel readout (flux L/R, wave L/R, chirality, strong/weak substrate, 5 forces) | ✅ | `97184a15` |
| 26-Moore-neighbour shell — display + **click-to-walk cursor** | ✅ | 6+12+8 cells (state glyph + \|flux\|); click a cell to retarget the inspection by its Moore offset (`94cca1e5`, verified walk == direct-inspect of the summed voxel); `--walk-neigh` headless hook |

---

## E. Scenarios & configuration — ✅

| Item | Status | Note |
|---|---|---|
| In-app scenario picker (searchable, category-grouped, live load) | ✅ | collapsed-by-default (fps) |
| Native scenario catalog / `ScenarioMeta` (130 rows) | ✅ | `scenario_catalog.h`, set-equality guarded |
| `dispatch_scenario` live path (delete JS seed mirror) | ✅ | W9 half-mutation guarded |
| All 130 scenarios verified (audit) | ✅ | 129 render / 1 intended-empty / 0 rejected / 0 hard errors ([AUDIT_SCENARIOS.md](AUDIT_SCENARIOS.md)) |
| Constants from `ontic.h` (delete JS mirror) | ✅ | |
| Lattice-size change in-app | ✅ | editable knob → `SetRunConfig` reboot, [4,256] |
| Prime-tick-on-load | ✅ | |

---

## F. Toggles (physics-term + config) — ✅

| Item | Status | Note |
|---|---|---|
| All 44 `TOGGLE_SPECS` toggles in the panel | ✅ | data-driven from the table, 4 collapsible categories, descriptions + requires/conflicts |
| Config knobs (dt, SOR, boundary, langevin×6, BCC stencil, Coulomb coupling/Z, omega0, kinetic drain) | ✅ | −/＋ nudge, live values, wired to SetDt/SetSorIterations/SetBoundary/SetDouble/SetUInt/SetEnum |
| Live validation (requires/conflicts/gpu) | ✅ | amber banner from `TermToggles::validate()` + gated rings |
| Reset-to-defaults | ✅ | `ResetToDefaults` |
| Render-only overlay toggles kept separate from engine toggles | ✅ | overlays are adapter view-state |

---

## G. UI shell & panels

| Item | Status | Note |
|---|---|---|
| Shell (toolbar / viewport hole / setup / physics+overlays+telemetry / status) | ✅ | RmlUi + RCSS over the live D3D12 scene |
| Scale-aware panels | ✅ | data-if per active scale |
| Scrollable + collapsible panel sections | ✅ | collapse-by-default keeps fps ~100+ |
| The ~18 specialized web panels (wave-lab, spectrum, gravity, time, thermo, dispersion, scale-context, symmetry, genesis, …) | ▢ | core surfaces done; specialized analysis panels not ported |
| Docking (drag / resize / float) | ▢ | fixed flex layout |
| Settings (theme / density / persisted prefs) | ▢ | single dark theme |
| Knowledge base / FAQ / keyboard-help / tooltips | ▢ | deferred by owner |
| JetBrains Mono font | ▢ | Inter aliased; needs a real TTF |

---

## H. Build, test, CI

| Item | Status | Note |
|---|---|---|
| Native build (`build_native.bat`, MSVC 14.44) | ✅ | |
| Golden gate green (physics invariant) | ✅ | 7/7 every commit |
| Smoke + scenario-catalog set-equality tests | ✅ | run directly |
| Native app tests registered in CTest / CI | ◐ | built every build; not ctest-registered (deferred R0, entangled with the held-off bundle) |
| Full CTest pyramid (device / interop / neutrality / journal-replay) | ◐ | some device/interop tests exist; not all wired for the new tree |

---

## I. Architecture invariants (§12 — keep, don't regress)

| Item | Status |
|---|---|
| Sim-thread-owns-engine / GUI-thread-reads-snapshot | ✅ |
| Tick-boundary command drain | ✅ |
| Golden gate never perturbed by the UI | ✅ |
| GPU-native renderer data (CUDA↔D3D12 interop) | ✅ |
| One force implementation (real engine fields, not divergent WASM samplers) | ✅ |
| Clean process exit (no `TerminateProcess`) | ✅ |
| Epistemic tags visible | ◐ (catalog carries them; not yet surfaced in the picker UI) |

---

## Progress log

- **2026-08-20/21 — Scale-0 push (owner focus: "perfect Scale 0 first"):** overlays → **33/33** (force styles + Knot Zones + font) ✅ · **full physics control** (44 toggles + config knobs + validation) ✅ · **telemetry charts** (diagnostics/conservation/lagrangian, scheduler activated + GPU epoch fix) ✅ · fps fix (5→125) ✅ · scenario picker + catalog + 130-scenario audit ✅.
- **2026-08-21 — inspector done + infra:** **inspector COMPLETE** — full 20+ scalar readout + forces (`97184a15`) and the **walkable 26-neighbour cursor** (`94cca1e5`) ✅ · **RmlUi reflow fps fix** — status bar moved to its own document, 5.7→62.7 fps with the 92-row picker open (`d7abf3c5`) + `--profile-ui` harness ✅ · FIELD OVERLAYS collapsible + panel-role swap ✅ · **main.cpp modularized 3,425→~1,950 lines** into 7 focused `src/app/` TUs (behavior-neutral) ✅.
- **Remaining on Scale 0:** only optional render polish — OIT (cleaner translucency; current draw-order is acceptable) and JetBrains Mono (needs a TTF) — plus surfacing epistemic tags in the picker.
- **Deferred (post-Scale-0):** the other scales (§A), the specialized analysis panels (§G), CI hardening (§H).

## Recommended order (Scale-0 first, per owner focus)

1. ~~Finish the Scale-0 overlays~~ ✅ done
2. ~~Full physics control~~ ✅ done · ~~Telemetry charts~~ ✅ done
3. ~~Inspector completeness~~ ✅ done — full readout + forces + walkable 26-neighbour cursor
4. **Scale-0 render polish** — OIT for cleaner translucency; JetBrains Mono (needs a TTF) ← only Scale-0 items left (both optional)
5. *(Scale 0 now functionally complete)* Scale 1 on CUDA → Scale 5 Cosmic + Scale 2/3 Atom → Scale 4/6 → the specialized panels → CI hardening
