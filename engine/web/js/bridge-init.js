/**
 * @file engine/web/js/bridge-init.js
 * @purpose Bridge barrel + capability-getter installer. Re-exports the
 *          three bridge classes (MockBridge, WasmBridge, CosmicMockBridge)
 *          and the createBridge() factory; on module load, also installs
 *          the lazy `bridge.capabilities` getter on MockBridge and
 *          WasmBridge prototypes (CONTRACTS.md §2).
 *
 *          Renamed from wasm-bridge-dag.js (2026-05-27, audit ticket P2-14).
 *          The pre-2026-04-27 file was a 2395-LOC monolith containing the
 *          actual MockBridge / WasmBridge / capability-factory bodies; the
 *          Phase 2 refactor sweep split them into focused modules under
 *          ./bridge/, leaving this file as a thin barrel.
 * @consumers app.js, scale controllers (engine/web/js/scales/*),
 *            scenario loaders, viewport, panels — anywhere that imports
 *            MockBridge, WasmBridge, CosmicMockBridge, or createBridge.
 * @related ./bridge/mock-bridge.js   (MockBridge class — Phase 2a)
 *          ./bridge/wasm-bridge.js   (WasmBridge class — Phase 2b)
 *          ./bridge/capabilities/    (Scale capability factories — Phase 2c)
 *          ./bridge/mock-scale5.js   (CosmicMockBridge class)
 *          ./bridge/bridge-factory.js (createBridge() factory)
 *
 * Phase trajectory of this file:
 *   pre-Phase 2:  2395 LOC (monolith)
 *   after 2a:      879 LOC (MockBridge extracted)
 *   after 2b:      213 LOC (WasmBridge extracted)
 *   after 2c:    ~50 LOC (capability factories extracted)
 *   2026-05-27:   renamed wasm-bridge-dag.js → bridge-init.js
 */

// ── Bridge classes (Phase 2a + 2b) ──────────────────────────────────
import { MockBridge } from './bridge/mock-bridge.js';
import { WasmBridge } from './bridge/wasm-bridge.js';

// ── Capability factories (Phase 2c) — installs the lazy
//    `bridge.capabilities` getter on both prototypes so consumers see
//    the symmetric Scale 0/1/2 surface (CONTRACTS.md §2). ────────────
import { installCapabilityGetter } from './bridge/capabilities/install.js';
installCapabilityGetter(MockBridge.prototype);
installCapabilityGetter(WasmBridge.prototype);

// ── Public re-exports ──────────────────────────────────────────────
export { MockBridge, WasmBridge };
export { CosmicMockBridge } from './bridge/mock-scale5.js';
export { createBridge } from './bridge/bridge-factory.js';
