# Plan — Scale 0 UI Interface Audit and 60 FPS Gate

**Status:** active audit plan; Gates 10A, 10B, 10C, 16, and 27 passed by user-directed priority; Gate 10 remainder remains paused without verdict
**Scope:** every user-facing interface available while Scale 0 (`lattice`) is active
**Rule:** only one numbered interface may be `IN PROGRESS`; the next interface does not start until the current one is `PASS`
**Target host:** the project Windows 11 workstation, foreground browser tab, dev server with COOP/COEP enabled

## Objective

Verify that every Scale 0 interface is correctly wired to the active physics owner, race-safe across lifecycle transitions, free of redundant hidden work and unbounded allocation/listener growth, and compatible with a sustained 60 FPS UI frame budget.

The 60 FPS requirement applies to foreground UI responsiveness and rendering. A panel may intentionally refresh measured data at 1–4 Hz; that slower scientific sampling cadence is acceptable only when it does not stall the browser frame loop or delay interaction.

“Always” is operationalized as the full registered test matrix on the target host. It cannot cover background-tab throttling, power-saving modes, other hardware, or external OS/GPU interruptions.

## Per-interface release gate

An interface passes only when all six checks pass:

1. **Ownership and wiring** — every control/readout has one named owner and a traceable path through state, telemetry, the active Scale 0 capability, or the physics harness. Worker-owned scenarios must not read the idle main-thread bridge.
2. **Lifecycle and races** — rapid input, pause/play/step/reset, scenario and lattice-size changes, panel float/dock/collapse, resize, Scale 0 exit/re-entry, worker fallback, and disposal produce no stale writes, out-of-order commits, duplicate listeners, orphaned timers, or post-dispose work.
3. **Performance** — after a three-second warm-up and during a minimum twelve-second capture:
   - effective foreground rAF rate is at least 59.5 FPS;
   - p95 frame interval is at most 17.0 ms and p99 at most 20.0 ms;
   - no reproducible panel-attributable frame interval exceeds 33.4 ms;
   - no panel-attributable Long Task is at least 50 ms;
   - measured panel update work is p95 at most 2 ms and max at most 8 ms;
   - action-to-next-paint latency is p95 at most 50 ms during the interaction burst.
4. **Demand and redundancy** — a hidden or collapsed interface performs zero bridge/sampler reads, DOM writes, canvas paints, and chart updates. Repeated reads within one telemetry/field epoch require an explicit correctness reason.
5. **Resource stability** — ten open/close or mount/unmount cycles return subscriptions, listeners, DOM nodes, canvases, workers, and retained memory to baseline; no monotonic growth is allowed.
6. **Verification evidence** — focused automated tests, browser measurements, console/page error capture, and a recorded result exist. A code-review-only pass is not sufficient.

Any failing check leaves the interface `BLOCKED`. The issue is fixed and the entire gate is rerun before the audit proceeds.

## Workload matrix

Each applicable interface is measured in these states:

- paused and playing;
- docked-active, floated-expanded, floated-collapsed, and hidden;
- default lattice size, large interactive size (`N=97`), and the maximum supported by
  the active backend (`N=97` for browser/WASM; `N=181` for native GPU);
- worker-backed WASM through its measured `N=97` ceiling and the supported
  in-thread fallback through its measured `N=33` ceiling;
- empty/static, flux-heavy, particle-rich, and interface-specific scenarios selected from the live registry;
- steady state plus a rapid interaction burst and a teardown/re-entry burst.

The all-scenario wiring contracts remain separate correctness gates. The performance matrix uses representative worst-case scenario classes so one interface audit remains reproducible; any discovered worse case is added to the matrix.

## Audit sequence

| # | Interface | Primary ownership | Required sub-checks | Status |
|---:|---|---|---|---|
| 0 | Measurement harness and evidence format | `tests/`, browser Performance APIs, `raf-coordinator.js` | Reproducible rAF, Long Task, callback-cost, request-count, lifecycle-count capture | PASS — automated probe 2/2; focused Edge capture 144.05 FPS, p99 7.01 ms, zero Long Tasks |
| 1 | Panel shell lifecycle | `app-shell.js`, `panel-dock-controller.js`, floating/mobile/mount controllers, `panel-visibility.js` | Dock, float, collapse, resize, mobile, scale exit/re-entry, subscriber teardown | PASS — 5/5 shell + 27/27 mount checks; Edge 142.72 FPS, p99 7.03 ms, zero Long Tasks, zero resource growth |
| 2 | Playback and Scale 0 mode controls | `app.js`, workspace/topbar components, Scale 0 controller | Play, pause, step, reset, speed, mode switch; one action per input | PASS — 7/7 gate + 5/5 smoke checks; Edge 144.05 FPS, p99 7.01 ms, zero Long Tasks; hidden work 432→0 mutations |
| 3 | Scenario toolbar and epistemic-status disclosure | `ui/toolbar/*`, `scenario-registry.js`, `ui/dom.js`, scenario loader | Rapid selection, stale async setup rejection, current-owner/status parity | PASS — 4/4 gate + 6/6 health/teardown/closure checks; all 130 scenarios healthy; Edge 143.55 FPS, p99 7.02 ms, zero Long Tasks |
| 4 | Lattice size and boundary toolbar | `ui/toolbar/*`, `ui/bindings.js`, active harness | Resize ordering, boundary parity, worker replacement, upload invalidation | PASS — a focused check invokes the production worker-init fallback path from a healthy L49 worker and verifies clamp/reload at direct-WASM L33, re-disabled larger options, and a refused later L49 resize; off-thread browser/WASM is capped at measured `N=97`, direct main-thread fallback at measured `N=33`, and larger sizes are native-GPU only; Edge 144.05 FPS, p99 7.02 ms, zero Long Tasks, exact worker conservation |
| 5 | Physics Toggles controls card | `ui/controls/physics-toggles.js`, `wire.js` | Toggle parity, defaults, scenario reload, no duplicate dispatch | PASS — 4/4 focused + 10/10 compatibility checks; Edge 144.05 FPS, p99 7.05 ms, zero Long Tasks; 42/42 active-owner commands, zero idle writes; hidden card zero work |
| 6 | Substrate controls card | `ui/controls/substrate-controls.js`, `wire.js` | Bounded coordinates, honest constant readouts, active owner, pause/reload behavior | PASS — 3/3 focused + 2/2 compatibility checks; Edge 144.13 FPS, p99 7.04 ms, zero intervals over 16.7 ms and zero Long Tasks; 35/35 active-owner commands, zero idle writes |
| 7 | Flux Volume controls card | `ui/controls/flux-volume.js`, `wire.js` | Visibility/style state, upload invalidation, no redundant volume reads | PASS — 5/5 focused/compatibility checks; Edge 144.05 FPS, p99 7.02 ms, zero intervals over 16.7 ms and zero Long Tasks; 2,400 inputs coalesced to 40 transactions with zero node churn |
| 8 | Particle Display controls card | `ui/controls/flux-volume.js`, `wire.js` | Particle flags, empty frames, scenario transitions | PASS — 3/3 focused + 3/3 shared-scheduler regression checks; Edge 144.04 FPS, p99 7.01 ms, zero intervals over 16.7 ms and zero Long Tasks; 1,600 inputs coalesced to 40 atomic particle-buffer updates |
| 9 | Selection controls card | `ui/controls/flux-volume.js`, inspector/viewport selection | Clear/select handoff, stale selection after resize/scenario change | PASS — 3/3 focused + 6/6 shared-path regression checks; Edge 143.97 FPS, p99 7.00 ms, zero intervals over 16.7 ms and zero Long Tasks; 2,400 inputs coalesced to 40 frame commits with lifetime-stable highlight resources |
| 10 | Visualization overlay | `ui/overlays/template.js`, `bindings.js`, `panel-shell.js`, field overlay runtime | All 34 toggles, style/axis/height controls, search, counts, clear, collapse, applicability | PAUSED — user redirected the audit to Gate 16; no pass/fail verdict |
| 10a | Flux/E/B flow lines | `field-overlays.js`, `streamline-worker.js`, `fieldlines.js`, viewport field/flux renderers, Flow Lines card | All registered sizes, seed bounds, worker ownership/cancellation, density/length/opacity, empty draws, lifecycle | PASS — worker-backed atomic results; all-size deterministic gate; browser L=9…97 Edge p99 <=7.12 ms, zero >20 ms/Long Tasks; native-only UI capture explicitly deferred |
| 10b | Gravity force + potential overlays | compact CPU/CUDA/WASM samplers, `overlay-frames.js`, `field-overlays.js`, field-force/topology renderers | Exact radius-2 operators, selected Poisson-latency potential, backend parity, per-style ownership/races, bounded work, empty draws, lifecycle | PASS — CPU/CUDA/WASM operator parity; all-size deterministic bounds; the release test has explicit `native`, `wasm`, `direct-wasm`, and backend-derived `auto` contracts, refuses impossible size/owner pairs, and enforces p95 <=17 ms; the explicit-native 40-combination `L=9…181` × arrows/heatmap/flow/glyphs matrix recorded a 60.00 FPS minimum and 16.67 ms worst p99; live Edge L=97 held 144.05 FPS in all four styles with p99 <=7.07 ms and zero >20 ms/Long Tasks |
| 10c | Visualization panel UX shell | `ui/overlays/template.js`, `panel-shell.js`, Scale 0 overlay stylesheet, `dom.js` | Information hierarchy, active rail, render-mode ownership, accordion/filter/context controls, accessibility, responsive bounds, resource stability | PASS — complete layer-inspector redesign; 6/6 focused + 4/4 overlay-wiring checks; live Edge interaction burst 71.97 FPS, p99 14.01 ms, zero >33.4 ms/Long Tasks; steady 72.03 FPS |
| 11 | Symmetry Aggregation overlay | `ui/overlays/symmetry-panel.js` | Resolve live disabled/pending surface: wire it correctly or remove it from the live UI | NOT STARTED |
| 12 | Conservation micropanel | `conservation-micropanel.js`, telemetry hub/active owner | Demand gate, audit reuse, drift history, float/collapse lifecycle | NOT STARTED |
| 13 | Genesis Burst scenario overlay | `genesis-burst-panel.js`, scenario loader, physics harness | Async sweep cancellation, scenario-change disposal, interval cleanup | NOT STARTED |
| 14 | Controls sidepanel integration | `Scale0ControlsComponent`, panel shell | Five-card composition, idempotent mount, responsive layout, combined request load | NOT STARTED |
| 15 | Diagnostics sidepanel | diagnostics component + Scale 0 descriptors | Live/floated gating, audit catch-up, row parity, no hidden collection | NOT STARTED |
| 16 | Telemetry Grid sidepanel | telemetry-grid component + Scale 0 channel registry | 23-channel draw cost, buffer reuse, collapsed/floated behavior, resize | PASS — scroll-vs-float race removed; expandable 220 px title rail + bidirectional side resize; 41 automated checks; Edge 144.09 FPS, p99 7.00 ms, zero >20 ms gaps/Long Tasks; chart/source 48.07/48.02 Hz with 100% sample coverage; hidden work and deep telemetry demand both zero |
| 17 | Charts sidepanel | charts component + Scale 0 descriptors | uPlot update/rescale cost, buffer epochs, float/collapse/resize | NOT STARTED |
| 18 | Lagrangian sidepanel | Lagrangian component + Scale 0 descriptors | Active-owner telemetry, demand mask, chart/table update cost | NOT STARTED |
| 19 | Inspector sidepanel | inspector component + `inspector/scales/lattice.js` | Bounded Moore reads, epoch cache, selection churn, teardown | NOT STARTED |
| 20 | Scene sidepanel | scene component + viewport/background adapters | Render-setting parity, resize, no duplicate viewport updates | NOT STARTED |
| 21 | Flux Slice sidepanel | `flux-slice-panel.js`, helpers, sampler cache | All field rows/axes, deep expansion, sampling budget, canvas reuse | NOT STARTED |
| 22 | Wave Lab sidepanel | `wave-lab-panel.js`, `wave-lab/wave-info.js` | Reseed cancellation, sparkline lifecycle, scenario applicability | NOT STARTED |
| 23 | P1 Observables sidepanel | panel orchestrator + eight child instruments | Coulomb, anisotropy, hydrogen, Bell, gravity, g−2, Thomson, fine structure; modal lifecycle | NOT STARTED |
| 24 | Spectrum sidepanel | `spectrum-panel.js`, spectrum/topology analysis | Live/deep transition, cancellation, FFT/sampler cost, active owner | NOT STARTED |
| 25 | Dispersion sidepanel | `dispersion-panel.js` | Live measurement trigger, scenario applicability, arm subscription | NOT STARTED |
| 26 | Knots sidepanel | `knots-panel.js`, knot runtime/cache | Detection/identity/attribution, canvas interaction, history bounds | NOT STARTED |
| 27 | Gravity sidepanel | `gravity-panel.js`, gravity analysis/samplers | Quantity switch, three canvases, active owner, hidden-work gate | PASS — Empty/pending/stale generations fail closed; the selected radius-2 engine support field and Poisson-derived `[IMPOSED]` latency map are separated from labeled `L_p/K_p/|F_p|` presentation proxies; branch status honors both umbrella and gravity toggles and states that `phase_forces` applies the enabled branch only at manifested sites. Center-anchored FTV2/direct-WASM reductions, sampler revision coherence, 4 Hz demand, rotating-plane reuse, collapse/visibility teardown, worker reuse/disposal, and cache generations are pinned by 9/9 scientific/lifecycle checks plus 2/2 worker lifecycle checks; adversarial tests poison the zero-copy volume at the first later engine call and invoke the L49 worker-failure recovery through clamp, reload, option disabling, and subsequent resize refusal. Worker L=97 and direct fallback L=33 clear the absolute foreground gate; forced direct L=97 measured only 16.49 FPS live and 18.47 FPS collapsed because the main-thread physics tick remained blocking, so direct fallback now refuses L>33 rather than weakening the gate. The explicit-native overlay matrix passed all 40 size/style combinations at a 60.00 FPS minimum, p95 <=17 ms, and 16.67 ms worst p99. Edge verified branch-status changes, scientific tooltips, collapse/restore, and site-style parity; its extension automation footer was throttled and was not used as performance evidence. |
| 28 | Time sidepanel | `time-panel.js`, time analysis | Slider interaction, imposed/measured separation, five-card draw cost | NOT STARTED |
| 29 | Thermo sidepanel | `thermo-panel.js`, telemetry + field slice | Slider/presets, heatmap allocation, active owner, hidden-work gate | NOT STARTED |
| 30 | Scale Context sidepanel | `scale-context-panel.js` | Live/static value ownership, SVG update cost, hidden-work gate | NOT STARTED |
| 31 | Integrated Scale 0 all-interface regression | full Scale 0 UI | Cross-panel demand masks, float combinations, rapid switching, final 60 FPS matrix | NOT STARTED |

## Evidence recorded for each PASS

- exact source paths and ownership chain;
- focused tests and commands;
- scenario, lattice size, bridge mode, panel state, sample duration, and browser viewport;
- rAF mean/p95/p99/max, Long Tasks, panel callback p95/max, input-to-paint p95;
- request/DOM/canvas counts while active versus hidden;
- lifecycle counts before and after ten cycles;
- findings and patches, followed by the post-fix measurements.

## Existing evidence to reuse, not assume

The existing Scale 0 panel-render, panel-wiring, request-budget, telemetry-gating, worker-teardown, sampler-lifetime, and resize-guard tests are inputs to this audit. Their current presence does not count as a fresh PASS. The active `SPEC_SCALE0_PERF_TELEMETRY_PANELS.md` documents earlier optimization work and several deferred items; this audit raises its earlier `>30 FPS at N=97` performance target to the gate above.
