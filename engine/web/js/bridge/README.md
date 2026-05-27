# engine/web/js/bridge — Bridge layer

**Purpose.** Adapter layer between the JS dashboard (scales, viewport, panels)
and the C++/WASM physics engine. Owns two interchangeable bridge
implementations (`MockBridge` and `WasmBridge`) and the live-ref factory
modules they compose to provide a uniform external surface.

## Public API

Consumers import from one of:
- `../bridge-init.js` — 42-LOC re-export shim (post-Phase 2 split, commits 2db67ca…87158ae). Surfaces `MockBridge`, `WasmBridge`, and capability factories from this directory.
- `./mock-bridge.js` (1578 LOC, `MockBridge` class — Phase 2a)
- `./wasm-bridge.js` (715 LOC, `WasmBridge` class — Phase 2b)
- `./capabilities/scale0.js`, `./capabilities/scale1.js`, `./capabilities/scale2.js`, `./capabilities/install.js` — capability factories (Phase 2c)
- `./mock-diagnostics.js`, `./mock-particle-engine.js`, `./mock-lattice-samplers.js`, `./mock-atom-engine.js` — live-ref factories that `MockBridge` composes
- `./scenarios/` — scenario dispatcher; see `./scenarios/README.md`
- `./boundary.js` — pure boundary-shape geometry (`insideBoundary`, `reflectIntoBoundary`)
- `./mock-scale4.js`, `./mock-scale5.js` — Scale-4/5 mock bridges (planetary, cosmic)

## Internal structure

Phase 2 split (post-refactor): `bridge-init.js` shrank from 2395 → 42 LOC; the bridge classes and their capability surfaces now live in this directory.

| File | Role |
|---|---|
| `mock-bridge.js` | `MockBridge` class (Phase 2a; 1578 LOC) |
| `wasm-bridge.js` | `WasmBridge` class (Phase 2b; 715 LOC) — thin wrapper over Embind RenderBridge |
| `capabilities/install.js` | Wires `bridge.capabilities.{scale0,scale1,scale2}` onto a bridge instance |
| `capabilities/scale0.js` | Scale-0 capability factory (lattice / substrate surface) |
| `capabilities/scale1.js` | Scale-1 capability factory (particle surface) |
| `capabilities/scale2.js` | Scale-2 capability factory (atomic surface) |
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
- **Imported by**: `../bridge-init.js` (re-export shim — `MockBridge` composes the live-ref factories internally, `capabilities/install.js` wires the per-scale surfaces), scale controllers (consume capabilities)
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
- **New diagnostic field** → extend the audit returned by `mock-diagnostics.js::createDiagnosticsProvider` AND mirror in `wasm-bridge.js` (Embind binding lives in `engine/wasm/bindings_render_bridge.cpp`); add row to `engine/web/js/ui/panels/diagnostics-panel/descriptors/scale0.js`
- **New mock subsystem** → create `mock-<name>.js` following the live-ref factory pattern; compose it inside `mock-bridge.js`; mirror the equivalent in `wasm-bridge.js`
- **New capability method** → add to the capability factory in `capabilities/scaleN.js`; both `MockBridge` and `WasmBridge` MUST satisfy the surface (CONTRACTS.md §2)
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
