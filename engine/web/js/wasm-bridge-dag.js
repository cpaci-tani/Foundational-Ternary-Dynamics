/**
 * @file engine/web/js/wasm-bridge-dag.js
 * @purpose Backward-compatibility re-export shim for the bridge layer.
 *          Pre-2026-04-27 this file contained MockBridge (~1500 LOC),
 *          WasmBridge (~670 LOC), and the capability factories
 *          (~150 LOC) all in one place. The Phase 2 refactor sweep
 *          (.claude/plans/i-want-to-try-crispy-charm.md) split them
 *          into focused modules under engine/web/js/bridge/. This file
 *          now exists ONLY so existing consumers
 *          `import { MockBridge, WasmBridge, ... } from './wasm-bridge-dag.js'`
 *          continue to resolve.
 * @consumers app_dag.js, scale controllers (engine/web/js/scales/*),
 *            scenario loaders, viewport, panels — anywhere that imported
 *            from this file before the split.
 * @related ./bridge/mock-bridge.js   (MockBridge class — Phase 2a)
 *          ./bridge/wasm-bridge.js   (WasmBridge class — Phase 2b)
 *          ./bridge/capabilities/    (Scale capability factories — Phase 2c)
 *          ./bridge/mock-scale5.js   (CosmicMockBridge class)
 *          ./bridge/bridge-factory-dag.js (createBridge() factory)
 *
 * Phase trajectory of this file:
 *   pre-Phase 2:  2395 LOC (monolith)
 *   after 2a:      879 LOC (MockBridge extracted)
 *   after 2b:      213 LOC (WasmBridge extracted)
 *   after 2c:    ~50 LOC (capability factories extracted) — current
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
export { createBridge } from './bridge/bridge-factory-dag.js';
