# engine/web/js/bridge — Bridge layer

**Purpose.** Adapter layer between the JS dashboard (scales, viewport, panels)
and the C++/WASM physics engine. Owns two interchangeable bridge
implementations (`MockBridge` and `WasmBridge`) and the live-ref factory
modules they compose to provide a uniform external surface.

## Public API

Consumers import from one of:
- `../wasm-bridge-dag.js` — top-level `MockBridge`, `WasmBridge`, capability factories (Phase 2 will split these into the files in this directory)
- `./mock-diagnostics.js`, `./mock-particle-engine.js`, `./mock-lattice-samplers.js`, `./mock-atom-engine.js` — live-ref factories that the MockBridge composes
- `./scenarios/` — scenario dispatcher; see `./scenarios/README.md`
- `./boundary.js` — pure boundary-shape geometry (`insideBoundary`, `reflectIntoBoundary`)
- `./mock-scale4.js`, `./mock-scale5.js` — Scale-4/5 mock bridges (planetary, cosmic)

## Internal structure

| File | Role |
|---|---|
| `mock-diagnostics.js` | Energy cache + audit decomposition (STATE CONTRACT exemplar) |
| `mock-particle-engine.js` | Scale-1 N-body Velocity Verlet |
| `mock-lattice-samplers.js` | 17 lattice samplers (E/B/Poynting fields, gradients, Kretschmann) + latency-proxy cache |
| `mock-atom-engine.js` | Scale-2 molecular dynamics (ionic / vdW / covalent / H-bonds) |
| `mock-scale4.js` | Planetary N-body bridge (PlanetaryMockBridge class) |
| `mock-scale5.js` | Cosmic bridge (CosmicMockBridge class) |
| `boundary.js` | Pure functions for boundary-shape geometry |
| `scenarios/` | 5 scenario group files + dispatcher (see scenarios/README.md) |
| `bridge-contract.js` | JSDoc typedef of the ScaleBridge interface (no runtime code) |

## Dependencies

- **Imports from**: `../constants.js`, `../elements.js`, `../atomic-props.js`, `../core/log.js`
- **Imported by**: `../wasm-bridge-dag.js` (composes the live-ref factories), scale controllers (consume capabilities)
- **No cross-scale imports** — the bridge layer is scale-agnostic.

## State contract (live-reference pattern)

All `mock-*.js` files in this directory follow the **live-reference factory
pattern** documented in [CONTRACTS.md §1](../../../../CONTRACTS.md#1--bridge-state-contract-live-reference-pattern).
Each factory function takes the live MockBridge instance and returns
methods that re-read state on every call. Factories MUST NOT destructure
state; otherwise cache invalidation breaks.

Reference exemplar: `mock-diagnostics.js` lines 26–50 (STATE CONTRACT block).

## How to extend

- **New scenario** → drop into `scenarios/<group>-scenarios.js` (auto-routed by prefix dispatch)
- **New diagnostic field** → extend the audit returned by `mock-diagnostics.js::createDiagnosticsProvider` AND mirror in WasmBridge binding (`engine/wasm/bindings_render_bridge.cpp`); add row to `engine/web/js/ui/panels/diagnostics-panel/descriptors/scale0.js`
- **New mock subsystem** → create `mock-<name>.js` following the live-ref factory pattern; mirror the WasmBridge equivalent
- **New scale bridge** (e.g., `mock-scale6.js`) → mirror `mock-scale4.js` / `mock-scale5.js` class structure

## Invariants

- MockBridge and WasmBridge MUST expose identical capability surfaces (see CONTRACTS.md §2)
- All state mutation routes through bridge methods; consumers MUST NOT poke `bridge._toggles` or `bridge._fluxJ` directly
- Mock-* factories MUST NOT destructure their `state` parameter

## Related docs

- [CONTRACTS.md](../../../../CONTRACTS.md) §1, §2, §3, §4
- [docs/adr/0002-capability-factories.md](../../../../docs/adr/0002-capability-factories.md)
- [docs/adr/0003-wasm-bridge-dag-refactor.md](../../../../docs/adr/0003-wasm-bridge-dag-refactor.md)
- [docs/adr/0005-live-reference-pattern.md](../../../../docs/adr/0005-live-reference-pattern.md)
- [META_PROJECT_ATLAS.md](../../../../META_PROJECT_ATLAS.md) §1, §2
