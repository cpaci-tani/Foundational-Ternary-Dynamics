# `window.__ftd*` debug globals — reference catalogue

The dashboard publishes a small set of globals on `window` for
console-debugging and cross-module wiring. They are **not** part of any
public API; consumers in JS code should import the canonical module
exports instead. Use these from the browser DevTools console only.

If you find yourself reaching for one of these from production JS, that's
a sign a missing export or factory is needed in the relevant module.

| Global | Type | Source | Purpose |
|---|---|---|---|
| `window.__ftdCtx` | object | [scale0/controller.js](../js/scales/scale0/controller.js) | Live Scale-0 controller context: `{ bridge, viewport, controls, globalTick, running, … }`. Bridge accessor for panels, scenario-loader, and ad-hoc console probes. |
| `window.__ftdScale0State` | function `() ⇒ State` | [scale0/state/store.js](../js/scales/scale0/state/store.js) | Returns a snapshot of the Scale-0 store: `currentScenarioId`, `useFluxMock`, `fluxMock`, field-overlay flags, etc. Polled per-frame by panels via the function form so reads stay live. |
| `window.__ftdRAF` | object | [lib/raf-coordinator.js](../js/lib/raf-coordinator.js) | The shared rAF coordinator singleton. Use `__ftdRAF.size()` to inspect current subscriber count, `__ftdRAF.clear()` for HMR/test teardown. |
| `window.__ftdTimelineLod` | module namespace | [scale0/controller.js](../js/scales/scale0/controller.js) | Lazy-imported timeline LoD helpers. Read by `wasm-bridge-dag.js` to pick a snapshot stride during scrub. |
| `window.__ftdStartRender` | function `(seconds=5) ⇒ void` | scale0/controller.js | Console helper: kick off the offline render controller for `N` seconds of simulated lattice time. |
| `window.__ftdCancelRender` | function | scale0/controller.js | Cancel a running offline render. |
| `window.__ftdFluxSlicePanel` | object \| null | [overlays/flux-slice-panel.js](../js/scales/scale0/ui/overlays/flux-slice-panel.js) | Singleton handle to the live flux-slice panel. Cleared by the panel's `dispose()` so detached subtrees become GC-eligible. |
| `window.__ftdConservationPanel` | object \| null | [overlays/conservation-micropanel.js](../js/scales/scale0/ui/overlays/conservation-micropanel.js) | Singleton handle to the conservation micropanel; same lifecycle rules as flux-slice. |
| `window.__ftdSpectrumPanel` | object \| null | [overlays/spectrum-panel.js](../js/scales/scale0/ui/overlays/spectrum-panel.js) | Singleton handle to the spectrum scanner panel. |
| `window.__ftdP1Panel` | object \| null | [overlays/p1-observables-panel.js](../js/scales/scale0/ui/overlays/p1-observables-panel.js) | Singleton handle to the P1 observables panel; api is `{ update, element, dispose }`. |

## Lifecycle contract for panel globals

Every panel that publishes a `window.__ftd*Panel` global MUST clear it
inside its `dispose()` once the singleton is being torn down — without
that, the detached panel subtree (and any Maps / ring buffers it
owns) cannot be GC'd. Today this is enforced manually in each
`dispose()` body; see the audit-pass-2 leak-plug commit for the full
discussion.

## Privacy / scope

These globals are page-singletons. They are visible in production
builds (the dashboard ships unbundled JS — there is no minification or
dead-code elimination step). On the GitHub Pages deployment the same
globals are exposed.

## Adding a new global

Don't, unless you have a debug-only need that can't be served by an
exported factory. If you must:

1. Add the global write only inside an `if (typeof window !== 'undefined')` guard.
2. Pair it with a clear-on-dispose write so the global doesn't pin a
   detached object indefinitely.
3. Add a row to this table.
