# Audit — Scale 0 UI Interfaces (2026-08-28)

**Status:** active; Gates 10A, 10B, 10C, and 16 passed by user-directed priority; Gate 10 remainder paused without verdict
**Plan:** `../PLAN_SCALE0_UI_INTERFACE_AUDIT_60FPS.md`
**Progression rule:** only one interface is audited at a time; a failing gate blocks the next interface

## Gate 0 — Measurement harness and evidence format

**Verdict:** PASS

### Automated probe validation

Command:

```text
cd engine/web/tests
npx playwright test scale0-ui-audit-harness.spec.js --reporter=list
```

Result: **2/2 passed**. The self-test verified rAF sampling, selected `rafCoordinator`
callback timing, DOM/canvas activity, method-call counting, action-to-paint timing,
resource deltas, and deliberate Long Task detection.

### Foreground Edge validation

| Field | Value |
|---|---:|
| Browser | Edge 151.0.0.0 |
| Device pixel ratio | 1.25 |
| Scenario | `flux-pulse` |
| Lattice | `N=33` |
| Active physics owner | `WasmBridgeProxy` |
| Panel | Controls, docked active |
| Simulation | Playing |
| Warm-up | 3 s |
| Capture | 12.065 s / 1,737 intervals |
| Effective FPS | **144.05** |
| Median interval | **6.94 ms** |
| p95 interval | **6.97 ms** |
| p99 interval | **7.01 ms** |
| Maximum interval | **7.05 ms** |
| Intervals >20 ms / >33.4 ms | **0 / 0** |
| Long Tasks >=50 ms | **0** |
| rAF subscriber delta | **0** |
| DOM node / canvas delta | **0 / 0** |
| Captured page errors | **0** |

The first attempted capture was excluded: the Edge window was occluded and Chromium
throttled rAF to a regular ~1 Hz while reporting zero Long Tasks. `Page.bringToFront`
restored the physical 144 Hz cadence; a three-second 440-interval check confirmed
144.05 FPS before the certified capture began. This is browser throttling evidence,
not an application performance result.

### Harness files

- `tests/scale0-ui-audit-probe.js`
- `tests/scale0-ui-audit-harness.spec.js`

## Gate 1 — Panel shell lifecycle

**Verdict:** PASS

Scope: dock/float/collapse/resize/mobile behavior, panel visibility semantics,
Scale 0 exit/re-entry, listener/subscription teardown, and interaction-frame cost.

### Findings fixed

1. Destroying or docking a floating panel during an active drag retained global
   pointer listeners until a later pointer-up. Drag completion now owns
   pointer-up, pointer-cancel, blur, and teardown cleanup; direct destruction also
   removes the manager entry.
2. Rapid collapse/expand queued redundant 250 ms viewport callbacks. The callback
   is now debounced to the final state.
3. Mount-toggle resize work ran both directly and again through its attribute
   observer. It is now rAF-coalesced and state-synchronized once, with an owned
   destroy path.
4. The mobile sheet listened on both the resizer and its ancestor, processing
   bubbling touch gestures twice. It now binds one effective target, handles
   `touchcancel`, resets transient transforms on teardown, and has idempotent init.
5. Dock changes initially produced 50–95 ms tasks. The causes were a forced
   `getComputedStyle()` immediately after mutation and inherited
   `--viewport-safe-*` updates on `<html>`, which invalidated the entire 5,043-node
   dashboard. Mount-dependent styles now use localized shell/active-panel classes;
   safe-edge variables live on `#viewport`; no root mount selector remains.
6. The panel-dock component discarded its mount-toggle owner. It now retains and
   destroys that controller, and supports safe re-initialization.

### Automated verification

```text
cd engine/web/tests
npx playwright test panel-mount-integration.spec.js panel-mount.spec.js --reporter=list
# 27 passed

npx playwright test scale0-panel-shell-audit.spec.js --reporter=list
# 5 passed
```

The shell suite verifies balanced global drag listeners, ten float/drag/collapse/
dock cycles, debounced viewport notification, mobile swipe/cancel/scroll-lock
behavior, and Scale 0 exit/re-entry with one persistent shell owner. Syntax checks,
`git diff --check`, and a CSS audit confirmed no remaining root
`data-panel-mount` selectors.

### Foreground Edge certification

Workload: `flux-pulse`, `N=33`, active `WasmBridgeProxy`, simulation playing,
2,042 px viewport width at DPR 1.25, three-second warm-up, then ten complete
left/bottom/right mount plus float/drag/collapse/dock cycles during a twelve-second
capture.

| Field | Result |
|---|---:|
| Effective FPS | **142.72** |
| Median / p95 / p99 interval | **6.94 / 6.97 / 7.03 ms** |
| Maximum interval | **27.76 ms** |
| Intervals >33.4 ms | **0** |
| Long Tasks >=50 ms | **0** |
| Action latency p95 / max | **17.75 / 19.89 ms** |
| rAF subscriber delta | **0** |
| DOM node / canvas delta | **0 / 0** |
| Heap delta | **-459,529 bytes** |
| Floating windows / floated tabs after cycles | **0 / 0** |
| Captured page errors | **0** |

All state returned to baseline: the Inspector returned to the dock, the original
left mount and Controls tab were restored, exactly one active panel retained the
localized mount class, and the tab remained foreground/focused throughout.

## Gate 2 — Playback and Scale 0 mode controls

**Verdict:** PASS

Scope: play, pause, step, reset, speed nudge/presets/fine slider, step-by-N,
prime-tick visibility/state, and engine-mode handoff. Each input must dispatch one
owned action, reject stale chains after teardown, and preserve the foreground
frame budget.

### Findings fixed

1. The hidden settings popover still ran `PlayBar.refresh()` every six animation
   frames, queried six zoom chips, and rewrote selection attributes. A pre-fix
   Edge probe counted **432 DOM mutation records in three seconds while hidden**.
   Refresh now returns before throttling when the popover is hidden, and chip
   setters skip unchanged ARIA values. The identical post-fix probe counted zero.
2. Speed presets and the fine slider used two incompatible mappings. Preset values
   above 1× were converted through a linear application path while the UI used a
   different inverse logarithm, so the labeled 5× and 10× controls did not request
   those speeds. `speed-scale.js` is now the shared logarithmic source:
   `speed = 10^((slider - 50) / 25)`, with one inverse and label formatter.
3. Rapid slider input wrote the idle main bridge and could send redundant worker
   updates. Input is now coalesced to one rAF; the Scale 0 store tracks the active
   playback owner plus last requested running/speed values and dispatches only
   changed values to that owner.
4. A single-step click changed application state before the worker received its
   pause, allowing the worker loop to race the explicit tick. The unified pause
   path now cancels queued main-thread ticks and synchronously sends
   `setRunning(false)` to the active owner before `tickOnce()`.
5. The +N chain retained zero-delay timers across reset, scenario changes, resize,
   mode changes, or play-bar teardown. The play bar is now a lifecycle-owned,
   idempotently mounted component with a generation-tagged cancellable step chain.
6. Playback state had split writes in the keyboard path, application loop, worker
   loop, and prime-tick control. Play/pause/reset/keyboard routes now share the
   same application actions; the prime-tick button paints one persisted Scale 0
   state and is hidden outside Scale 0.

### Automated verification

```text
cd engine/web/tests
npx playwright test scale0-playback-controls-audit.spec.js --reporter=list
# 7 passed

npx playwright test playback-smoke.spec.js --reporter=list
# 5 passed
```

The gate suite verifies all six speed preset round trips, zero hidden DOM work,
one active-owner speed update for a 25-event slider burst, worker pause-before-tick
ordering, reset cancellation of an in-flight +100 chain, ten idempotent remount and
settings cycles with stable resources, and single-owned persisted prime-tick state
across Scale 0 exit/re-entry. The legacy layout, mount, nudge, and stepping smokes
then passed 5/5 on a clean repeat. Syntax checks and `git diff --check` passed for
all changed production and test modules.

### Foreground Edge certification

Workload: `flux-pulse`, `N=33`, active `WasmBridgeProxy`, initially paused,
2,042 px viewport width at DPR 1.25, three-second warm-up, then settings open,
0.1×/1×/10×/1× presets, a 25-event fine-slider burst, three play/pause cycles,
single-step, +10, +100, prime off/on, reset, settings close, and idle completion
during a twelve-second capture.

| Field | Result |
|---|---:|
| Effective FPS | **144.05** |
| Median / p95 / p99 interval | **6.94 / 6.98 / 7.01 ms** |
| Maximum interval | **7.05 ms** |
| Intervals >20 ms / >33.4 ms | **0 / 0** |
| Long Tasks >=50 ms | **0** |
| Action latency p95 / max | **7.50 / 7.50 ms** |
| rAF subscriber delta | **0** |
| DOM node / canvas delta | **0 / 0** |
| Captured page errors | **0** |

Final state matched the initial state: paused, slider 50 = 1×, one active 1×
preset, prime tick on, and settings closed. A separate ten-cycle Edge lifecycle
probe retained one play bar, 11 rAF subscribers, 44 play-bar DOM nodes, and zero
canvases; it ran at 144.03 FPS with zero >20 ms frames, zero Long Tasks, and
7.75 ms p95 action latency. Heap decreased by 10,739,675 bytes after collection.

## Gate 3 — Scenario toolbar and epistemic-status disclosure

**Verdict:** PASS

Scope: scenario selection controls, registry-to-option parity, epistemic badges and
disclosure copy, active physics-owner handoff, rapid selection ordering, async
setup cancellation, and hidden/re-entry behavior.

### Findings fixed

1. Every admitted registry record carried an `epistemicStatus`, and the registry
   comments promised that full status in the disclosure, but the live
   “Epistemic status” panel rendered only the technical title and behavioral
   validation. It now renders the exact canonical registry status plus evidence
   level before any validation or seed metadata. A modified physics profile keeps
   that registered status visible while leading with “qualification suspended.”
2. The toolbar component created the registry-backed 130-option menu, then the
   binding layer immediately destroyed and rebuilt the identical tree during
   owner reconciliation. Population now compares the grouped option IDs first;
   an already-current tree performs zero DOM mutations and only reconciles the
   selected value when needed.

### Automated verification

```text
cd engine/web/tests
npx playwright test scale0-scenario-toolbar-audit.spec.js --reporter=list
# 4 passed

npx playwright test scale0-scenario-health.spec.js scale0-worker-teardown.spec.js \
  scenario-closure-parity.spec.js --reporter=list
# 6 passed
```

The gate suite proves exact ordered optgroup/registry parity, 130 unique entries,
zero repeat-population DOM work, exact canonical status and evidence disclosure
for all 130 admitted scenarios, one pause/load generation/worker replacement per
input during ten immediate selections, latest-only commit after late callbacks,
and one preserved control/select after Scale 0 exit/re-entry. The existing
mechanical campaign then loaded all 130 scenarios through the production worker
path: every entry published finite telemetry, every non-allowlisted entry mounted
field or particle content, and none logged a real error. Worker conservation,
heavy `quantum-tunnel`, `flux-annihilation`, and closed-negative card parity also
passed. Syntax checks and `git diff --check` passed.

### Foreground Edge certification

Workload: `N=33`, active `WasmBridgeProxy`, paused, three-second warm-up, then ten
representative scenario changes (`flux-dipole`, genesis, `quantum-tunnel`, a
closed-negative particle model, uniform field, mass gravity, Moore geometry,
empty, standing wave, and the original `flux-pulse`) followed by ten disclosure
open/close cycles during a twelve-second capture.

| Field | Result |
|---|---:|
| Effective FPS | **143.55** |
| Median / p95 / p99 interval | **6.94 / 6.97 / 7.02 ms** |
| Maximum interval | **27.77 ms** |
| Intervals >20 ms / >33.4 ms | **2 / 0** |
| Long Tasks >=50 ms | **0** |
| Action latency p95 / max | **11.17 / 12.17 ms** |
| rAF subscriber delta | **0** |
| DOM node / canvas delta | **0 / 0** |
| Heap delta | **-1,612,656 bytes** |
| Captured page/status errors | **0** |

Worker counters moved from 1 created / 0 terminated / 1 live to 11 / 10 / 1,
exactly matching the ten inputs. Final selector, state store, and worker owner all
returned to `flux-pulse`; its exact registry status remained visible; the menu
retained 130 options and one toolbar owner; the disclosure ended closed.

## Gate 4 — Lattice size and boundary toolbar

**Verdict:** PASS

Scope: odd-size menu parity, resize validation and ordering, active-owner worker
replacement, stale async resize rejection, viewport alignment, flux-boundary
command/readback parity, scenario defaults, and upload invalidation.

### Findings fixed

1. Boundary changes wrote both the active worker-backed physics owner and the idle
   main bridge, and scenario loading separately repeated the viewport reflective
   flag. The application action now normalizes the mode, writes the active owner
   exactly once, and owns the single viewport update.
2. The size menu exposed `N=113`, `145`, and `181` to browser/WASM even though a
   live `N=181` attempt blocked the foreground thread for 3.3 seconds and failed
   to produce a ready worker. `N=113` also produced five 145–188 ms Long Tasks.
   A progressive capture through `N=97` remained clean, making `N=97` the measured
   strict-interactivity ceiling for this backend on the target host. The larger
   sizes now remain available to native GPU only and are disabled with explicit
   labels in browser/WASM mode.
3. The backend limit is enforced in both presentation and resize execution. A
   synthetic disabled-option change cannot bypass it, replace the active worker,
   or alter selector, main bridge, active owner, or viewport state. Native GPU
   retains the full odd-size menu through `N=181`.
4. A late native resize could commit after a newer scenario/resize generation.
   The asynchronous resize path now rechecks its generation before reloading and
   cannot overwrite the winning request.

### Automated verification

```text
cd engine/web/tests
npx playwright test scale0-lattice-boundary-toolbar-audit.spec.js \
  scale0-resize-guard.spec.js scale0-worker-teardown.spec.js --reporter=list
# 7 passed
```

The five focused gate checks prove exact menu/boundary values and backend labels;
one active-owner command plus one viewport update per boundary input; exact worker
conservation and latest-only state after ten rapid resizes; rejection of a
programmatic native-only size before worker replacement; and stale native resize
cancellation. Existing oversize-allocation and teardown regression checks also
passed. JavaScript syntax checks and `git diff --check` passed.

### Foreground Edge certification

Workload: `flux-pulse`, browser/WASM `WasmBridgeProxy`, paused, 2,042 × 982 CSS px
at DPR 1.25, three-second warm-up, then all three boundary modes, ten accepted
resizes through `N=97` and back to `N=33`, and a forced `N=113` request during a
twelve-second capture.

| Field | Result |
|---|---:|
| Effective FPS | **144.05** |
| Median / p95 / p99 interval | **6.94 / 6.97 / 7.02 ms** |
| Maximum interval | **7.09 ms** |
| Intervals >20 ms / >33.4 ms | **0 / 0** |
| Long Tasks >=50 ms | **0** |
| Action latency p95 / max | **16.15 / 16.15 ms** |
| Accepted worker creations / terminations | **10 / 10** |
| Forced `N=113` worker creations / terminations | **0 / 0** |
| Live workers before / after | **1 / 1** |
| rAF subscriber delta | **0** |
| DOM node / canvas delta | **0 / 0** |
| Captured page errors | **0** |

The final selector, main bridge, active owner, and viewport all agreed on `N=33`;
the active worker was ready; the original `flux-pulse` scenario remained selected;
and the three native-only options remained disabled and accurately labeled. The
16.3 MB heap increase was bounded active-lattice allocation after exercising
larger sizes, not lifecycle leakage: worker/subscriber/DOM/canvas ownership all
returned to baseline.

## Gate 5 — Physics Toggles controls card

**Verdict:** PASS

Scope: control inventory and defaults, scenario-to-toggle parity, active-owner
wiring, rapid toggle races, reload/reset behavior, duplicate dispatch, hidden-work
gating, ten-cycle lifecycle stability, and foreground frame cost.

### Findings fixed

1. Every user edit wrote both the idle in-thread `WasmBridge` and the active
   worker-backed `WasmBridgeProxy`. Standard and research controls now resolve
   the canonical active Scale 0 owner and issue exactly one command.
2. The card labeled one checkbox “Genesis / Evaporation,” but it only controlled
   `genesis`; configured `evaporation` had no DOM control. The two engine terms
   now have separate, accurately labeled inputs, leaving only the intentionally
   scenario-owned `de_broglie_clock` without a dashboard checkbox.
3. Engine readback repainted only the standard whitelist. In the admitted
   Pair Production scenario the engine ran `pair_production=true` while the
   visible research checkbox claimed false. All 27 visible standard/research
   controls now repaint from authoritative engine truth. During scenario handoff
   the card is `aria-busy` and inputs are disabled until that first readback, so
   no edit can race a worker replacement.
4. Research edits did not trigger the modified-profile warning, even though they
   change the registered term set and invalidate scenario qualification. The
   first engine readback of each load generation now captures the full visible
   baseline; every later standard or research edit is compared to that baseline.
   Native/worker rejection can repaint the control and recompute the warning.
5. Ten immediate Restore clicks created redundant scenario reloads, workers, and
   flash timers. The button now locks synchronously, collapses the burst to one
   canonical reload, and owns one replaceable flash timer.
6. Toggle acknowledgements caused 1,522 mutation records in the first live
   capture because every row class and busy attribute was rewritten even when
   unchanged. Conditional DOM writes reduced the identical workload to 248
   records (**84% fewer**), with zero node additions/removals. Scenario metadata
   rendering is also idempotent and preserves an open disclosure during profile
   edits.
7. The live browser initially mixed a cached pre-fix listener with current
   modules. Explicit app/controller/Scale 0 control-runtime module versions now
   make the ownership and readback changes atomic under cached deployment.

### Automated verification

```text
cd engine/web/tests
npx playwright test scale0-physics-toggles-audit.spec.js --reporter=list
# 4 passed

npx playwright test scale0-toggle-engine-parity.spec.js \
  scale0-dynamical-flux-dressing.spec.js \
  scale0-persisted-scenario-boot.spec.js toggle-coverage.spec.js --reporter=list
# 10 passed
```

The focused gate proves exact configured-control inventory, full standard and
research engine parity, 40 edits routed only to the active owner with stable
resources, blocked edits during scenario handoff, authoritative Pair Production
readback, and one reload/worker replacement for a ten-click Restore burst. The
compatibility sweep preserves dynamical flux-dressing restore, persisted selection
and reconnect replay, massive-body/octahedron engine profiles, gravity-only
applicability, and the complete field-toggle coverage contract. Syntax checks and
`git diff --check` passed.

### Foreground Edge certification

Workload: `flux-pulse`, `N=33`, paused, active `WasmBridgeProxy`, Controls docked
and visible at 2,042 × 982 CSS px / DPR 1.25, three-second warm-up, both advanced
sections, ten Gauss and Knot Tracking on/off cycles, Evaporation on/off, and a
ten-click Restore burst during a twelve-second capture.

| Field | Result |
|---|---:|
| Effective FPS | **144.05** |
| Median / p95 / p99 interval | **6.94 / 6.98 / 7.05 ms** |
| Maximum interval | **7.14 ms** |
| Intervals >20 ms / >33.4 ms | **0 / 0** |
| Long Tasks >=50 ms | **0** |
| Action latency p95 / max | **6.56 / 8.01 ms** |
| Direct active-owner / idle-main commands | **42 / 0** |
| Restore worker creations / terminations | **1 / 1** |
| Live workers before / after | **1 / 1** |
| Card mutation records | **248** (from 1,522 pre-fix) |
| Added / removed nodes; canvas draws | **0 / 0; 0** |
| rAF subscriber delta | **0** |
| Heap delta | **−1,439,705 bytes** |
| Captured page errors | **0** |

The final worker was ready with authoritative readback; standard/research engine
values and checkboxes agreed; profile warning was clear; card/input counts stayed
at 1 / 27; scenario and generation committed latest-only; and one live worker
remained.

### Hidden-card demand check

With Diagnostics active, Controls had `display:none` and the physics card had no
rendered box. A separate 3.002-second Edge probe measured **144.05 FPS**, p99
**7.03 ms**, zero intervals over 20 ms, zero Long Tasks, **zero DOM mutations**,
zero canvas work, zero rAF/DOM/canvas resource delta, and zero page errors.

## Gate 6 — Substrate controls card

**Verdict:** PASS

Scope: injection coordinates and state, steppers, randomized/explicit field and
particle injection, K_B/G_N/damping parameters, clear/random-field actions,
active-owner ownership, read-only constant truth, pause/reload behavior, hidden-work
gating, lifecycle stability, and foreground frame cost.

### Findings fixed

1. Every injection and field action wrote both the idle in-thread `WasmBridge`
   and the active worker-backed `WasmBridgeProxy`. All seven action paths now
   resolve the canonical active owner and issue exactly one engine command.
2. The Particle and Wave buttons both called `injectWavepacket`, so the visible
   Particle action was only a duplicate label. Particle now calls the supported
   `injectParticle` command; Wave and randomized injection retain wavepacket
   semantics.
3. K_B, G_N, and damping were presented as writable sliders, but neither the
   browser/WASM bridge nor the worker proxy exposes `setParam`; the C++ values
   are compile-time constants. All three controls are now explicit disabled,
   accessible `fixed` readouts. Browser/WASM mirrors the canonical constants and
   native mode may mirror the acknowledged engine-profile echo without implying
   a writable setting. The dead input listeners were removed.
4. A typed out-of-range coordinate could be dispatched before its `change`
   event, and input `max` attributes were not synchronized by scenario/resize
   transactions. Every action now clamps and reflects coordinates at the bridge
   boundary, while the scenario loader synchronizes all bounds to the active
   lattice. Stepper selection is scoped to this card, and randomized coordinates
   remain valid even for a small synthetic lattice.
5. Clear Field called `viewport.setLatticeSize` after clearing. That cascades
   mesh rebuilds across every viewport sub-renderer even though lattice geometry
   did not change. Clearing now invalidates the lattice upload and charts only;
   the live workload recorded zero viewport resize calls.
6. Coordinate validation rewrote the same three `max` attributes on every
   action. The first live capture produced 125 attribute mutation records. An
   idempotent bounds write reduced the identical workload to 20 records
   (**84% fewer**), all of them necessary positive/negative state-class changes;
   there were zero node additions or removals.

### Automated verification

```text
cd engine/web/tests
npm test -- scale0-substrate-controls-audit.spec.js \
  scale0-inject-paused.spec.js ws-bridge-visual-cache.spec.js \
  --grep "Scale 0 substrate-controls|Scale-0 substrate inject|native-only constants" \
  --reporter=line
# 5 passed
```

The three focused checks prove unique control inventory, exact canonical
constant/display parity, read-only accessibility semantics, active-worker-only
dispatch for every injection and field method, Particle/Wave command separation,
dispatch-time coordinate clamping, zero redundant viewport rebuilds, stable
DOM/resources/subscribers during bursts, and resize-to-coordinate-limit parity.
The compatibility checks prove an entangled pair is visible while paused without
advancing a tick and preserve native acknowledged-constant readback. JavaScript
syntax checks and `git diff --check` passed.

### Foreground Edge certification

Workload: `flux-pulse`, `N=33`, paused, active `WasmBridgeProxy`, Controls docked
and visible at 2,042 × 982 CSS px / DPR 1.25. Five cycles exercised Particle,
Wave, Flux, Pair, randomized wavepacket, Clear Field, Random Flux, state polarity,
and Center—50 actions during a twelve-second capture.

| Field | Result |
|---|---:|
| Effective FPS | **144.13** |
| Median / p95 / p99 interval | **6.94 / 6.97 / 7.04 ms** |
| Maximum interval | **7.26 ms** |
| Intervals >16.7 ms / >20 ms / >33.4 ms | **0 / 0 / 0** |
| Long Tasks >=50 ms | **0** |
| Action latency p95 / max | **0.20 / 0.28 ms** |
| Direct active-owner / idle-main commands | **35 / 0** |
| Viewport lattice rebuilds | **0** |
| Card mutations | **20**, all valid state-class changes |
| Added / removed nodes | **0 / 0** |
| Card nodes / inputs before and after | **53 / 6** |
| rAF subscribers before / after | **11 / 11** |
| Resource entries before / after | **250 / 250** |
| Live workers before / after | **1 / 1** |
| Heap delta | **+1,310,385 bytes** |
| Captured page errors | **0** |

The worker remained the sole live physics owner; the idle bridge received no
commands; and the card remained visible and structurally stable. The small heap
increase followed real field/particle commands and was not accompanied by any
worker, subscriber, DOM, or resource growth.

### Hidden-card demand check

With Diagnostics active, Controls had `display:none` and the Substrate Controls
card had no rendered box. A steady 3.005-second Edge probe measured **144.14
FPS**, p99 **7.00 ms**, maximum **7.08 ms**, zero intervals over 16.7 ms, zero
Long Tasks, **zero DOM mutations**, and zero page errors.

## Gate 7 — Flux Volume controls card

**Verdict:** PASS

Scope: visibility and style state, shape/opacity/point-size/threshold/scenario
scale controls, upload invalidation, active viewport ownership, input coalescing,
hidden-work gating, lifecycle stability, and foreground frame cost.

### Findings fixed

1. Six range inputs synchronously executed their full viewport path on every
   `input` event. The card now owns one latest-value `requestAnimationFrame`
   transaction: a burst commits each logical control at most once per frame,
   and point-size plus threshold share one upload invalidation.
2. A queued input could commit after an asynchronous scenario load had won.
   Pending jobs now carry the Scale 0 load generation and are discarded when
   that generation is stale, so loader-owned state cannot be overwritten by an
   older user event.
3. Flux-slice opacity and shape lived only on the current material. A lattice
   resize rebuilt the slice mesh with constructor defaults and silently erased
   the user's values. Both settings are now persisted by `FieldRenderer` and
   reapplied on every mesh build.
4. Volume, slice, scene-scale, spacing, and wireframe setters rewrote equivalent
   renderer state. All eleven target paths are now idempotent, eliminating
   redundant uniform, transform, and traversal work.
5. Display updates used `textContent`, replacing six text nodes per transaction.
   The first live workload therefore allocated and removed 240 nodes. The final
   path reuses each existing text node and records only necessary
   `characterData` changes: zero additions and zero removals.
6. The live deployment could otherwise combine cached control code with current
   renderer persistence. App, Scale 0 controller, viewport renderer, and
   control-wire versions were advanced as one cache boundary.

### Automated verification

```text
cd engine/web/tests
npm test -- scale0-flux-volume-controls-audit.spec.js \
  flux-slice-axes.spec.js ws-bridge-visual-cache.spec.js \
  --grep "Scale 0 Flux Volume|Flux Volume Opacity|scenario-local visibility" \
  --reporter=line
# 5 passed
```

The focused gate proves unique inventory and renderer/display parity, 600 input
events collapsed to one latest-value frame transaction and one upload, stale
generation rejection, stable card structure, and opacity/shape persistence
through an actual lattice resize mesh rebuild. Compatibility coverage preserves
the shared volume/slice controls and scenario-local visual preferences.
JavaScript syntax checks and `git diff --check` passed.

### Foreground Edge certification

Workload: `flux-pulse`, `N=33`, paused, active `WasmBridgeProxy`, Controls docked
and visible at 2,835.6 × 1,293.1 CSS px / DPR 0.90. Forty interaction groups
issued 2,400 range-input events and 40 shape changes during a twelve-second
capture. All range streams changed their displayed value on every group.

| Field | Result |
|---|---:|
| Effective FPS | **144.05** |
| Median / p95 / p99 interval | **6.94 / 6.97 / 7.02 ms** |
| Maximum interval | **7.09 ms** |
| Intervals >16.7 ms / >20 ms / >33.4 ms | **0 / 0 / 0** |
| Long Tasks >=50 ms | **0** |
| Interaction batch p95 / max | **0.58 / 0.63 ms** |
| Input events / shape changes | **2,400 / 40** |
| Calls per renderer setter | **40** for each of 11 paths |
| Card mutations | **240**, all necessary text-node value changes |
| Added / removed nodes | **0 / 0** |
| Card nodes / inputs before and after | **35 / 7** |
| rAF subscribers before / after | **11 / 11** |
| Resource entries before / after | **250 / 250** |
| Live workers before / after | **1 / 1** |
| Heap delta | **+1,134,396 bytes** |
| Captured page errors | **0** |

An additional six-segment isolation profile measured baseline, material-only
controls, point-size uploads, threshold uploads, combined uploads, and all
controls separately. Every segment sustained **144.05 FPS** with zero intervals
over 16.7 ms, ruling out a hidden upload-specific stall.

### Hidden-card demand check

With Diagnostics active, Controls and the Flux Volume card had no rendered box.
A steady 3.006-second Edge probe measured **144.04 FPS**, p99 **7.01 ms**,
maximum **7.06 ms**, zero intervals over 16.7 ms, zero Long Tasks, **zero DOM
mutations**, stable 35-node/7-control structure, stable 11 subscribers and 250
resources, one live worker, and zero page errors.

## Gate 8 — Particle Display controls card

**Verdict:** PASS

Scope: shape, positive/negative size, opacity and glow controls; particle flags;
empty-frame behavior; scenario and lattice transitions; input coalescing;
hidden-work gating; lifecycle stability; and foreground frame cost.

### Findings fixed

1. All four sliders synchronously rewrote their display and renderer state on
   every `input` event. Particle Display now uses the same card-wide
   latest-value frame scheduler as Flux Volume. This also removes duplicate
   batching code and reuses the existing text nodes instead of replacing them.
2. Positive and negative size changes only patched the current buffer. The next
   worker particle frame restored the engine's generic size `6`, so the visible
   controls became false while running. Lattice-mode frames now derive rendered
   size from the retained sign-specific presentation settings on every update.
3. The old reactive-size path inferred sign from the currently displayed color.
   Enabling the real color-charge overlay replaced those decorative sign colors
   and broke both size controls. `ViewportParticleRenderer` now maintains a
   bounded CPU sign cache from the source charge-sign colors, independent of the
   active color presentation.
4. Each sign-size setter called `render()` even though the dashboard owns a
   continuous render loop. Those synchronous extra renders were removed. Empty
   particle frames now return without dirtying the size buffer, and equivalent
   shape, size, opacity, and glow writes are idempotent.
5. Moving both sign sliders in one frame still traversed and uploaded the whole
   particle buffer twice. The card now commits both values through one atomic
   `setParticleSizes` call and one `updateParticleSizes` upload.
6. Scale 1 owns the shared particle shader while active and applies Circle plus
   glow `0.28`. Returning to Scale 0 left that Scale 1 state active while the
   Scale 0 card displayed its retained values. Every Scale 0 mount now replays
   the card's five presentation values before the scenario worker commits.
7. Shape input is validated to the supported integer range `0..7`, queued card
   jobs are discarded after Scale 0 exit, and the retained values are replayed
   on re-entry. App/controller/viewport/particle-renderer/wire versions were
   advanced together as cache boundary `v30`.

### Automated verification

```text
cd engine/web/tests
npm test -- scale0-particle-display-controls-audit.spec.js \
  scale0-flux-volume-controls-audit.spec.js --reporter=line
# 6 passed
```

The three focused checks prove unique inventory and renderer/display parity,
sign-specific size persistence across repeated live frames with color-charge
rendering enabled, zero upload on an empty frame, idempotence, zero direct
render calls, 400 input events collapsed to one structurally stable transaction,
discard-on-exit/replay-on-entry behavior, scenario persistence, and a real Scale
0 → Scale 1 → Scale 0 round trip. The three Flux Volume regressions re-certify
the shared scheduler extraction. JavaScript syntax checks and `git diff --check`
passed.

### Foreground Edge certification

Workload: `flux-pair-production`, `N=33`, paused, active `WasmBridgeProxy`,
Controls docked and visible at 2,835.6 × 1,293.1 CSS px / DPR 0.90. The live
particle buffer held 1,018 records: 509 positive and 509 negative. Forty groups
issued 1,600 range-input events and 40 shape changes during twelve seconds.

| Field | Result |
|---|---:|
| Effective FPS | **144.04** |
| Median / p95 / p99 interval | **6.94 / 6.97 / 7.01 ms** |
| Maximum interval | **7.07 ms** |
| Intervals >16.7 ms / >20 ms / >33.4 ms | **0 / 0 / 0** |
| Long Tasks >=50 ms | **0** |
| Interaction batch p95 / max | **0.35 / 0.44 ms** |
| Input events / shape changes | **1,600 / 40** |
| Shape / atomic-size / opacity / glow calls | **40 / 40 / 40 / 40** |
| Individual positive / negative setter calls | **0 / 0** |
| Size-buffer updates / GPU dirty-version delta | **40 / 40** |
| Direct redundant render calls | **0** |
| Final positive / negative control-buffer parity | **14.5 = 14.5 / 12.5 = 12.5** |
| Card mutations | **160**, all necessary text-node value changes |
| Added / removed nodes | **0 / 0** |
| Card nodes / inputs before and after | **27 / 5** |
| rAF subscribers before / after | **11 / 11** |
| Resource entries before / after | **250 / 250** |
| Live workers before / after | **1 / 1** |
| Heap delta | **−550,641 bytes** |
| Captured page errors | **0** |

### Hidden-card demand check

With Diagnostics active, Controls and Particle Display had no rendered box. A
steady 3.003-second Edge probe on the same 1,018-particle frame measured
**143.88 FPS**, p99 **7.01 ms**, maximum **7.18 ms**, zero intervals over 16.7
ms, zero Long Tasks, **zero DOM mutations**, **zero size-buffer version change**,
stable nodes/controls/subscribers/resources/worker count, and zero page errors.

### Ten-cycle lifecycle check

Ten real Scale 0 → Scale 1 → Scale 0 cycles each observed Scale 1's shader
preset (`shape=0`, `glow=0.28`) and then exact restoration of Scale 0's retained
DOM and renderer truth (`shape=3`, sizes `18/9`, opacity `0.71`, glow `0.22`).
Worker counters advanced by exactly ten creations and ten terminations while
live workers stayed at one. Card nodes/inputs remained **27/5**, total document
nodes **7,211**, canvases **162**, subscribers **11**, resources **250**, card
mutations **0**, and heap delta **−2,087 bytes**.

## Gate 9 — Selection controls card

**Verdict:** PASS

Scope: coordinate fields and steppers, single/area selection, radius, inspector
handoff, viewport highlight ownership, resize/scenario invalidation, rapid-event
races, hidden-work gating, lifecycle stability, and foreground frame cost.

### Findings fixed

1. The SELECT path wrote `inspector._selectedPos` directly, then used a dynamic
   import whose promise could resolve after a scale or scenario owner changed.
   `Inspector.selectLatticePosition()` now provides one synchronous, bounded
   public transaction; the card no longer mutates private inspector state.
2. Scenario loads and lattice resizes synchronized injection bounds but not
   selection bounds. Coordinates, `max` attributes, an existing inspector
   selection, and active highlight position now reconcile together through
   `ctx.syncScale0SelectionBounds(activeN)` once the live owner size is known.
3. Inspector Clear, an empty viewport click, and scale exit hid the voxel box
   but left the Selection card's area box visible. Lattice inspector teardown
   now hides every selection overlay and publishes an idempotent clear event.
4. Click-to-select updated the coordinate fields but left an enabled area box
   at its previous position. The document handoff now validates and clamps the
   event, ignores it outside Scale 0, and moves the area overlay without a
   redundant voxel write.
5. Every radius `input` rebuilt and disposed a Three.js box geometry and
   material. SceneCore now owns one unit-box highlight for its lifetime and
   changes only its transform. Radius bursts use the shared latest-value frame
   scheduler, reuse the display text node, reject stale load generations, and
   cannot commit outside Scale 0.
6. Selection axis queries are scoped to `#sel-card`; all navigation buttons
   have explicit `type="button"`; coordinate, radius, highlight visibility, and
   renderer writes are idempotent. App, controller, loader, inspector,
   viewport/SceneCore, card, and wire imports advanced together as cache
   boundary `v31`.

### Automated verification

```text
cd engine/web/tests
npm test -- scale0-selection-controls-audit.spec.js --reporter=line
# 3 passed

npm test -- scale0-selection-controls-audit.spec.js \
  scale0-particle-display-controls-audit.spec.js \
  scale0-flux-volume-controls-audit.spec.js --reporter=line
# 9 passed
```

The focused suite proves unique inventory and current lattice bounds, public
inspector ownership, exact SELECT/Clear behavior, area resource reuse across
400 changes, 600 events collapsed to one frame transaction, zero card-node
churn, resize reconciliation, inactive-scale rejection, click-to-select area
parity, and a real Scale 0 → Scale 1 → Scale 0 round trip. Six Gates 7–8 tests
re-certify the shared scheduler. JavaScript syntax checks and targeted
`git diff --check` passed.

### Foreground Edge certification

Workload: `flux-pulse`, `N=33`, paused, active `WasmBridgeProxy`, Controls
visible. Forty groups issued 2,400 radius events, 40 coordinate-navigation
clicks, and 10 full inspector SELECT transactions during an 11.953-second
capture.

| Field | Result |
|---|---:|
| Effective FPS | **143.97** |
| Median / p95 / p99 interval | **6.94 / 6.97 / 7.00 ms** |
| Maximum interval | **13.89 ms** |
| Intervals >16.7 ms / >20 ms / >33.4 ms | **0 / 0 / 0** |
| Long Tasks >=50 ms | **0** |
| Action-to-next-paint p95 / max | **5.43 / 21.05 ms** |
| Radius events / committed radius transactions | **2,400 / 40** |
| Coordinate clicks / inspector SELECT transactions | **40 / 10** |
| Voxel / area highlight calls | **100 / 100**, exactly expected |
| Card mutations | **40**, all necessary text-node value changes |
| Added / removed card nodes | **0 / 0** |
| Card nodes / inputs before and after | **41 / 4** |
| Scene highlight / geometry / material identity | **stable / stable / stable** |
| Renderer geometry count before / after | **10 / 10** |
| rAF subscribers before / after | **11 / 11** |
| Resource entries before / after | **250 / 250** |
| Live workers before / after | **1 / 1** |
| Captured page errors | **0** |

The first inspector SELECT lazily initialized two inspector-owned document
nodes and produced the 21.05 ms action-to-next-paint sample; the measured frame
containing that work remained below 16.7 ms. Subsequent lifecycle sampling held
the complete document at 7,213 nodes, so this was bounded first use rather than
growth.

### Hidden-card demand check

With Diagnostics active, Selection had no rendered box. The settled
5.001-second Edge capture measured **144.05 FPS**, p99 **7.00 ms**, maximum
**7.04 ms**, zero intervals over 16.7 ms, zero Long Tasks, **zero highlight
calls**, **zero DOM mutations**, stable 41-node/4-input structure, 10 renderer
geometries, 11 subscribers, 250 resources, one live worker, and zero page
errors. An earlier three-second sample contained one 20.86 ms browser interval,
but recorded zero Selection calls/mutations and no Long Task; it did not recur
after settling and is retained here rather than attributed to the hidden card.

### Ten-cycle lifecycle check

Ten real Scale 0 → Scale 1 → Scale 0 cycles hid voxel and area highlights on
every exit and re-entry while retaining coordinates `9/8/7`, radius `6`, and
area-mode intent. Worker counters advanced by exactly ten creations and ten
terminations while live workers returned to one. Card nodes/inputs stayed
**41/4**, document nodes **7,213**, renderer geometries **10**, subscribers
**11**, resources **250**, card mutations **0**, and all highlight resource
identities remained stable. A post-cycle 500-event burst produced exactly one
voxel call and one area call, proving listeners were not duplicated. Edge
reported no warnings or errors.

## Gate 10 — Visualization overlay

**Verdict:** PAUSED — USER-DIRECTED PRIORITY CHANGE; NO PASS/FAIL VERDICT

Scope: all visualization toggles, applicability states, overlay style/axis/
height controls, search and count behavior, clear/collapse interactions,
runtime field-demand ownership, hidden-work gating, lifecycle stability, and
foreground frame cost.

The user redirected the active audit to the Telemetry Grid sidepanel before
this gate was certified. Gate 10 is deliberately not marked as passed and must
resume from its existing evidence before the normal sequence advances.

## Gate 10A — Flux, electric, and magnetic flow lines

**Verdict:** PASS — focused subset of Gate 10

Scope: Flux Lines, Radiative E Field, and B Field at every registered lattice
size; seed/integration bounds; field-snapshot ownership; stale-result rejection;
renderer capacity/empty-draw behavior; shared presentation controls; lifecycle;
and foreground frame pacing.

### Findings and repairs

- The existing one-channel-per-frame scheduler prevented E/B/Flux from stacking,
  but one B-line RK4 job still measured 19–135 ms at larger sizes. Capping the
  number of lines reduced the stall frequency but could not make a single long
  trajectory safe. Complete E/B/Flux integration and Flux magnitude coloring now
  run in one lifecycle-owned module worker. The main thread submits an immutable
  sampled-field snapshot and atomically uploads only a complete result.
- Generic `fieldNeedsUpdate` also represents ordinary sampler deliveries. Using
  it to cancel asynchronous jobs terminated every worker request just before its
  response. Cancellation now keys on actual ownership changes: Scale 0 load
  generation or density/length settings version. Toggle-off remains race-safe
  because apply paths re-check current requested visibility.
- Streamline seeds could land exactly on the upper boundary (`L=17`, spacing 3)
  because the generator added the half-voxel center offset after its loop bound.
  The bound now reserves that offset; all generated coordinates satisfy
  `0 <= coordinate < N` at all ten registered sizes.
- `maxLines` exceeded the seed cap by 50 even though every builder had already
  capped its seed union. It now equals `maxSeeds`. Lattice-aware visual budgets
  are 60 lines through L=17, 40 at L=25, 36 at L=33, and 24 at L>=49; the
  in-thread emergency fallback remains capped at 16.
- E/Flux/B meshes reserved 96k/96k/144k vertices for obsolete 300-line sweeps.
  Audited capacities are now 16k/16k/24k. Flux now shares E/B's requested-versus-
  drawable rule, so a requested empty layer does not submit a WebGL draw.
- A persisted Flow Lines card now exposes shared Density (25–100%), Line Length
  (40–100%), and Opacity (20–100%) plus the current per-size line/step budget.
  Density/length invalidate seeds and cancel only obsolete work; opacity writes
  the three materials directly and never dirties geometry. WebGL line width was
  deliberately not exposed because `LineBasicMaterial.linewidth` is not portable
  across the Edge/ANGLE path.
- The worker is terminated on Scale 0 exit and on true ownership invalidation;
  its pending map and sampled references are cleared in the same transaction.

### Automated evidence

```text
cd engine/web/tests
npx playwright test scale0-flow-lines-audit.spec.js \
  scale0-visualization-overlay-audit.spec.js --project=chromium
```

The focused gate covers all ten registered sizes (L=9, 17, 25, 33, 49, 65,
97, 113, 145, 181), finite/bounded outputs, byte-identical synchronous versus
incremental RK4 results, persisted control semantics, material-only opacity,
bounded renderer buffers, empty-draw suppression, nonzero atomic worker results,
stable sampler ownership, and worker teardown on Scale 0 exit.

### Foreground Edge evidence

Target: connected Microsoft Edge extension, foreground tab, 144 Hz display,
`flux-pulse`, simulation playing, Flux/E/B lines all visible at the audited
maximum density and length. Each browser-supported lattice used a 120-frame
capture; L=33 and L=65 used a longer 180-frame confirmation.

| L | Lines | p95 | p99 | >20 ms | >33.4 ms | Long Tasks | Drawable E/B/Flux |
|---:|---:|---:|---:|---:|---:|---:|---:|
| 9 | 60 | 7.02 ms | 7.06 ms | 0 | 0 | 0 | yes/yes/yes |
| 17 | 60 | 7.00 ms | 7.04 ms | 0 | 0 | 0 | yes/yes/yes |
| 25 | 40 | 7.00 ms | 7.06 ms | 0 | 0 | 0 | yes/yes/yes |
| 33 | 36 | 7.05 ms | 7.12 ms | 0 | 0 | 0 | yes/yes/yes |
| 49 | 24 | 7.03 ms | 7.05 ms | 0 | 0 | 0 | yes/yes/yes |
| 65 | 24 | 7.01 ms | 7.05 ms | 0 | 0 | 0 | yes/yes/yes |
| 97 | 24 | 7.02 ms | 7.12 ms | 0 | 0 | 0 | yes/yes/yes |

Every capture averaged **6.94 ms** per foreground rAF interval (~144 FPS).
The native-only L=113/145/181 options are correctly disabled in browser/WASM;
their deterministic parameter, seed-bound, finite-output, and worker-compute
paths passed, but a native-GPU UI capture remains part of the eventual native
backend matrix rather than being misreported as browser evidence.

## Gate 10B — Discrete gravity force and potential overlays

**Verdict:** PASS — focused subset of Gate 10

Scope: the gravity force overlay in Arrows, Heatmap, Flow, and Glyphs styles;
the topology-height gravity-potential surface; exact backend ownership; stale
async rejection; empty-result cleanup; bounded GPU/CPU work; and foreground
frame pacing.

### Scientific status and operator truth

- The default effective branch now visualizes the engine's exact radius-2
  centered lattice operator
  `G_N * delta2 |J|`, where
  `delta2 f = [f(x+2)-f(x-2)]/4` on the finite periodic computational quotient.
- The geometric branch uses the same discrete stencil as the engine:
  `M_INERTIAL * C_SPEED^2 * L * delta2 L`.
- When the selected Poisson-latency branch is active, the potential surface is
  reconstructed from the real latency field as `Phi = -L^2`. Its explicitly
  labeled fallback is the local effective proxy `-G_N |J|`; it is not presented
  as a Poisson solve.
- Gravity remains an effective `[SELECTION]`/`[IMPOSED]` recovery layer, not a
  primitive of the strict-discrete ontology. `G_N=0.01` remains the lattice toy
  value whose physical identification was falsified in FTD-0131. The periodic
  compute box is not relabeled as the framework's undefined boundary.

### Findings and repairs

- Compact CPU, CUDA, and WASM sampling previously reused sparse/stale diagnostic
  vectors. Each path now computes the exact active radius-2 operator directly at
  every requested sample.
- Paused CUDA sampling exposed a real owner bug: the compact sampler had not
  synchronized the active toggle set and could silently render the default force
  while geometric gravity was selected. `GpuBackend` now synchronizes toggles
  before every compact visual sample; explicit CPU/CUDA parity covers both
  branches.
- The potential overlay previously displayed `-|J|^2` while implying a solved
  gravitational potential. WASM now exposes the real Poisson latency samples;
  the renderer consumes `-L^2` when available and carries source/operator
  metadata so the local-density fallback cannot be mistaken for that branch.
- Renderer state is separated by force type. Requested-but-empty layers clear
  and hide resident geometry immediately, async style changes cannot repaint a
  stale style, and disposal clears typed state, materials, geometry, workers,
  pending jobs, and sampled references.
- Gravity flow integration runs in the existing module worker and uploads one
  complete result. All streamlines share one `LineSegments` draw instead of one
  draw per line. Arrows, heatmap points, and glyphs use deterministic global
  decimation with caps of 256, 64, and 128 respectively; the potential surface
  is capped at 32 segments because the visual sampler exposes at most about 33
  lattice levels.
- Heatmap falloff no longer evaluates `exp()` per fragment, and glyphs use
  four-face opaque depth-tested directional cones to reduce fill and overdraw
  without changing the encoded direction or sign.

### Automated evidence

```text
cd engine/build
ctest -j 24 --output-on-failure -C Release \
  -R "^(visual_field_sample|gpu_visual_field_sample)$"
# 2/2 passed

cd engine/web/tests
npx playwright test scale0-gravity-overlay-audit.spec.js --project=chromium
# 5/5 passed, including 28 warmed performance combinations

npx playwright test scale0-gravity-overlay-audit.spec.js \
  scale0-visualization-overlay-audit.spec.js \
  scale0-flow-lines-audit.spec.js \
  --project=chromium --grep-invert "sustains the 60 FPS"
# 13/13 passed
```

The canonical native build and all three canonical WASM variants completed and
the updated bindings were deployed. The focused suite covers operator semantics,
requested/drawable and per-type renderer state, bounded work for L=9, 17, 25,
33, 49, 65, 97, 113, 145, and 181, worker-backed flow results, stale-style race
rejection, disposal, and warmed performance at every browser-supported size
through L=97. The 28-case matrix's minimum was **59.50 FPS** (L=97 Flow) and
worst p99 was **16.67 ms** (L=49 Glyphs), with zero intervals over 33.4 ms,
Long Tasks, or page errors.

### Foreground Edge evidence

Target: connected Microsoft Edge extension, foreground tab, 144 Hz display,
`A Black Hole Field (model)`, L=97, gravity and potential enabled. Each force
style received a 0.7-second post-switch settle followed by a three-second
main-world rAF/Long Task capture.

| Style | Effective FPS | Frames | p95 | p99 | Max | >20 ms | >33.4 ms | Long Tasks |
|---|---:|---:|---:|---:|---:|---:|---:|---:|
| Arrows | 144.05 | 432 | 7.01 ms | 7.05 ms | 7.09 ms | 0 | 0 | 0 |
| Heatmap | 144.05 | 432 | 7.01 ms | 7.05 ms | 7.09 ms | 0 | 0 | 0 |
| Flow | 144.05 | 432 | 7.01 ms | 7.05 ms | 7.32 ms | 0 | 0 | 0 |
| Glyphs | 144.05 | 432 | 7.01 ms | 7.07 ms | 7.09 ms | 0 | 0 | 0 |

The native-only L=113/145/181 work bounds and deterministic output paths are
covered automatically. A native-GPU UI capture remains deferred rather than
being represented as browser/WASM evidence.

## Gate 10C — Visualization panel UX shell

**Verdict:** PASS — focused subset of Gate 10 (2026-08-29)

Scope: the Scale 0 overlay panel's visual hierarchy, search, active-layer
summary, scalar/vector presentation controls, category navigation, contextual
subcontrols, accessibility state, responsive bounds, resource stability, and
foreground interaction cost. Physics ownership and overlay math were preserved.

### Findings and redesign

- The prior active strip reserved three complete rows even when only one layer
  was active, leaving a large dead area between search and render controls. It is
  now a one-line horizontally scrollable rail whose height follows its content.
- Scalar presentation lived above the category list while vector presentation
  was buried inside Forces. Both now live in one explicitly presentation-only
  Render card with independent `Surface/Heat map` and
  `Arrows/Heat/Flow/Glyphs` groups; the existing state owners and element IDs are
  unchanged.
- Seven visually identical sections read as one long undifferentiated list.
  They are now independently persisted, color-coded inspector cards. Volume is
  the fresh-install primary disclosure; other cards start summarized, while
  multiple cards may remain open when the user chooses.
- Volume treatment, slice planes, and topology/stress-energy height sliders were
  displayed even while their owning layer was off. Context controls now appear
  only with an active owner. Search still exposes matching volume/slice details,
  and inapplicable wrappers no longer reserve phantom grid cells.
- The header now carries a compact Scale 0 identity, a live active-count badge,
  and a complete collapse state. Search has a dedicated clear action and Escape
  path. The whole-panel collapse hides every subordinate region instead of
  leaving search/render controls behind.
- Toggle and segmented-control truth is mirrored to `aria-pressed`; category
  headers expose current `aria-expanded`; search clear and active-chip removal
  retain explicit accessible names and keyboard focus behavior.
- The design is isolated in a Scale 0 stylesheet loaded after shared primitives,
  avoiding specificity drift across other scales. The panel is 348 px on the
  primary desktop layout, bounded to the viewport, with only its category body
  scrolling. Glass behavior continues to use the shared glass tokens, so the
  default glass-off state remains opaque. The focused responsive check confirms
  a 326 px inspector at a 1024×768 viewport with the panel fully bounded inside
  the viewport and only the category body scrolling.

### Verification

```text
cd engine/web/tests
npx playwright test s0-overlay-accordion.spec.js --project=chromium
# 6/6 passed (one startup-harness timeout on an earlier combined run; clean retry)

npx playwright test scale0-visualization-overlay-audit.spec.js --project=chromium
# 4/4 passed
```

The focused suite verifies the new hierarchy and 348 px bounds, one-line active
rail, progressive disclosure, independent persistence, chip removal,
filter/auto-expand/clear, contextual-control visibility, ARIA truth, exact
render-style and toggle wiring, per-column clear behavior, incremental DOM
reconciliation, request coalescing, and zero final rAF-subscriber, DOM-node, or
canvas growth. Syntax checks and scoped `git diff --check` passed.

### Foreground Edge evidence

Target: connected Edge extension on the 144 Hz workstation display, `flux-pulse`,
L=33, simulation active. A nine-second main-world capture exercised all category
open/close paths, filter/clear, every scalar/vector render selector, Flux Slice
on/off, and whole-panel collapse/expand.

| Capture | Effective FPS | p95 | p99 | Max | >20 ms | >33.4 ms | Long Tasks |
|---|---:|---:|---:|---:|---:|---:|---:|
| Interaction burst | 71.97 | 13.96 ms | 14.01 ms | 20.82 ms | 1 | 0 | 0 |
| Steady compact panel | 72.03 | 13.95 ms | 13.98 ms | 14.01 ms | 0 | 0 | 0 |

The browser was presenting at an effective 72 Hz during these captures. Both
states therefore remained above the 60 FPS gate without a reproducible dropped
frame or Long Task.

## Gate 16 — Telemetry Grid sidepanel

**Verdict:** PASS

Scope: the 23-channel Scale 0 Telemetry Grid, workspace icon rail, docked side
panel resize path, hidden/collapsed telemetry demand, chart/value update cost,
lazy chart lifecycle, and left/right mount parity.

### Findings and repairs

- Vertical motion beginning on a side-rail tab crossed the same 15 px gesture
  threshold as horizontal motion and floated the panel instead of scrolling the
  overflowing icon list. Direction is now resolved before ownership transfers:
  vertical mouse/stylus motion slides the rail, touch retains native momentum
  scrolling, and horizontal motion retains panel floating.
- The shared rail now has a dedicated pointer-captured separator with rAF-
  coalesced width writes, bounded/persisted state, keyboard Home/End/arrow
  support, and a 220 px expanded preset. All 17 Scale 0 titles fit without
  clipping at that preset. A separate separator owns the panel width so rail
  expansion and content expansion cannot race.
- The panel-width separator works in both left and right mounts, reverses drag
  direction correctly, clamps to the live viewport, persists one shared side
  width, updates safe overlay edges without a forced-style read, and exposes
  separator ARIA values. Pointer up, cancel, lost capture, blur, and compact-
  mode transitions all terminate the drag and clear the pending rAF/cursor.
- Collapsed docked panels were still considered live because the shared
  visibility predicate checked only the active tab id. Both render and telemetry
  demand predicates now reject collapsed and immersive-hidden panels.
- Steady-state Telemetry Grid updates now touch only intersecting cards. Buffer
  paths and typed-array views are cached, duplicate pointer/mouse hover listeners
  and duplicate value writes are gone, and ResizeObserver bursts collapse to one
  rAF reflow using cached plot nodes/widths. One activation value sync prevents
  placeholder flashes without restoring off-screen steady-state writes.
- The user-supplied 16.02 s recording `2026-08-28_19-23-52.mp4` showed a defect
  that the original frame-loop trace did not measure: Telemetry Grid visibly
  stepped while Charts and Lagrangian consumed each published sample. Scale 0
  published at display-refresh / 3, but the grid's 125 ms app gate reduced its
  plot commits to about 8 Hz. The docked Scale 0 grid now updates in the exact
  publication frame, and app-level fallback is 33 ms for floated/non-Scale-0
  use. A follow-up 144 Hz trace exposed a second drop at the component's shared
  33 ms limiter; docked source-synchronized Scale 0 bypasses that redundant
  limiter while all fallback paths retain it. No interpolation was introduced
  and telemetry values/sampling semantics are unchanged.

### Automated evidence

Commands and results:

```text
cd engine/web/tests
npx playwright test telemetry-sidepanel-audit.spec.js panel-mount.spec.js \
  scale0-panel-render.spec.js scale0-panel-shell-audit.spec.js \
  scale0-telemetry-gating.spec.js responsive-overflow.spec.js \
  --workers=1 --reporter=list
# 41 passed (8.7 m)
```

The focused spec verifies real pointer paths in left and right mounts, vertical
rail sliding without a floating-window handoff, title expansion, persisted
bounded widths, ten collapse/expand cycles, singleton handles, off-screen value
culling, one-per-frame resize reflow, stable 23-card/23-entry ownership, and zero
collapsed draws/value refreshes. Its cadence gate also records source advances,
uPlot commits, panel-update cost, and rAF pacing together; the former 125 ms path
fails its rendered/source coverage and relative-cadence assertions. The
regression sweep covers mobile bottom-sheet
behavior, every width breakpoint, global drag-listener cleanup, ten float/dock
cycles, Scale 0 exit/re-entry, and demand-gated audit telemetry. JavaScript syntax
checks and scoped `git diff --check` also passed.

During implementation, the first combined mount/render run exposed a real
activation regression: viewport culling left the first visible value as `--`
until IntersectionObserver delivered its callback. A one-time value-only sync
on the hidden→live transition repaired the placeholder flash; the dedicated
render suite then passed 3/3. This development failure is not included in the
final pass count.

### Foreground Edge evidence

Target: the connected Microsoft Edge extension session, foreground/focused tab,
`http://localhost:8080/`, live Scale 0 wave simulation, Telemetry Grid active.

The video-specific post-fix trace is the chart-motion gate of record. Over an
uninterrupted **8.006 s** at a 144 Hz display, Edge measured **144.09 effective
FPS**, frame p50 **6.940 ms**, p95 **6.965 ms**, p99 **6.995 ms**, and max
**7.055 ms**, with **zero intervals over 20 ms** and **zero Long Tasks**. The
source advanced **387** times at **48.02 Hz** and the Total Flux sparkline made
**387** uPlot commits at **48.07 Hz**: **100% sample coverage**. Commit p50 was
**20.805 ms** and p95 **21.655 ms**, tracking source p50 **20.825 ms** and p95
**20.855 ms**. Across eight visible charts, Telemetry Grid update cost was
**0.130 ms p95**, **0.470 ms p99**, and **0.490 ms max**.

The earlier trace below remains the interaction/lifecycle record, but it sampled
only browser rAF and panel callback cost; it did not count plot commits and was
therefore insufficient to certify visible chart smoothness by itself.

An uninterrupted 12.003 s trace recorded **864 frames**: **71.98 effective
FPS**, frame p50 **13.885 ms**, p95 **13.920 ms**, p99 **13.955 ms**, and max
**14.015 ms**. There were **zero intervals over 20 ms**, **zero intervals over
33.4 ms**, and **zero Long Tasks**. Across 87 panel updates, update p95 was
**0.735 ms** and max **1.045 ms**.

The rail was dragged from icon-only into label mode and the panel from 520 px to
700 px, then both were exercised again during chart scrolling. Double-click
expanded the rail to the 220 px preset; all visible Scale 0 titles reported no
clipping. A reload restored the persisted widths with exactly one rail handle
and one panel handle.

While collapsed for 2.5 s, the live browser recorded **0 update calls, 0 chart
draws, and 0 value refreshes**; `isPanelVisible('telemetry-grid')`, `wantAudit`,
and `wantLag` were all false. Ten collapse/expand lifecycles retained one panel,
23 cards, a 23-entry map, 19 lazily built charts, one ResizeObserver, one
IntersectionObserver, no pending resize/reflow rAF, no active drag, and no
floating windows. Edge reported no warnings or errors.

An earlier 53 s automation-interaction trace contained five 61–82 ms Long Tasks
at automation command boundaries, while the panel update maximum remained
1.975 ms. Those tasks did not reproduce in the uninterrupted certification
trace and are not attributed to Telemetry Grid; the clean trace is the gate of
record.
