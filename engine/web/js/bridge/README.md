# engine/web/js/bridge — Bridge layer

**Purpose.** Adapter layer between the JS dashboard (scales, viewport, panels)
and the C++/WASM physics engine. Scale-0 physics is owned by the WASM
`RenderBridge` (in-thread `WasmBridge` or off-thread `WasmBridgeProxy`).
Higher scales use native PE (Scale-1), JS atom MD (Scale-2), and mock
planetary/cosmic bridges (Scale-4/5).

> **Retired:** `MockBridge` / `mock-bridge.js` / `mock-bridge-proxy.js` /
> `mock-particle-engine.js` are gone. Do not reintroduce them as the Scale-0
> path. The JS tree under `scenarios/` is a **parity mirror** of
> `engine/src/scenarios/*.cpp`, not the live seed path.

## Public API

Consumers import from:
- `../bridge-init.js` — re-export shim (`WasmBridge`, capability factories)
- `./wasm-bridge.js` — in-thread Embind `WasmBridge` (prefers Memory64 when supported)
- `./wasm-bridge-proxy.js` — off-thread worker proxy (`ftd_core_mt`, SharedArrayBuffer flux)
- `../ws-bridge.js` — WebSocket → native `ws_server` (optional GPU)
- `./capabilities/{install,scale0,scale1,scale2}.js` — capability factories
- `./native-particle-engine.js` — Scale-1 C++ ParticleEngine adapter
- `./mock-atom-engine.js` — Scale-2 JS MD (WASM AE incomplete)
- `./mock-scale4.js`, `./mock-scale5.js` — planetary / cosmic mocks
- `./scenarios/` — JS parity mirror only; see `./scenarios/README.md`
- `./boundary.js` — boundary-shape geometry helpers
- `./pe-catalog-map.js` — particle catalog id map (ex-mock-PE)

## Internal structure

| File | Role |
|---|---|
| `wasm-bridge.js` | In-thread WASM Scale-0/1 (+ AE fallback stub) |
| `wasm-bridge-proxy.js` | Worker-backed Scale-0 owner |
| `wasm-bridge.worker.js` | Worker entry (`createFTDModuleMT`) |
| `capabilities/install.js` | Lazy `bridge.capabilities.{scale0,scale1,scale2}` |
| `capabilities/scaleN.js` | Per-scale capability factories |
| `native-particle-engine.js` | Scale-1 native PE via embind |
| `mock-atom-engine.js` | Scale-2 JS molecular dynamics |
| `mock-atom-valence.js` | Valence / bond-order helpers for AE |
| `mock-scale4.js` / `mock-scale5.js` | Planetary / cosmic mock bridges |
| `boundary.js` | Pure boundary geometry |
| `scenarios/` | Name-parity twin of C++ scenario library |
| `bridge-contract.js` | JSDoc `ScaleBridge` typedef |

## Dependencies

- **Imports from:** `../constants.js`, `../elements.js`, `../atomic-props.js`, `../core/log.js`
- **Imported by:** `../bridge-init.js`, scale controllers, scenario-loader
- **No cross-scale imports** — the bridge layer is scale-agnostic

## How to extend

- **New Scale-0 scenario** → add C++ body under `engine/src/scenarios/` first, then JS parity case + registry entry; run `tests/scenario-parity.spec.js`
- **New diagnostic field** → embind in `engine/wasm/bindings_render_bridge.cpp` + `wasm-bridge.js` + diagnostics descriptor
- **New capability method** → `capabilities/scaleN.js`; every live backend that owns that scale must satisfy it (CONTRACTS.md §2)

## Invariants

- Live Scale-0 seeds go through WASM `setupScenario` → `ftd::dispatch_scenario`
- Consumers use `bridge.capabilities.scaleN.*`, not ad-hoc duck typing
- State mutation routes through bridge methods

## Related docs

- [CONTRACTS.md](../../../../CONTRACTS.md)
- [SPEC_SCALE0_BRIDGE_ARCHITECTURE.md](../../docs/SPEC_SCALE0_BRIDGE_ARCHITECTURE.md)
- [docs/adr/0002-capability-factories.md](../../../../docs/adr/0002-capability-factories.md)
- [docs/adr/0005-live-reference-pattern.md](../../../../docs/adr/0005-live-reference-pattern.md)
