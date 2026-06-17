/**
 * @file engine/web/js/bridge-init.js
 * @purpose Bridge barrel + capability-getter installer. Re-exports the
 *          bridge classes (WasmBridge, CosmicMockBridge) and the
 *          createBridge() factory; on module load, also installs the lazy
 *          `bridge.capabilities` getter on WasmBridge and WebSocketBridge
 *          prototypes (CONTRACTS.md §2).
 */

// ── Bridge classes ───────────────────────────────────────────────────
import { WasmBridge } from './bridge/wasm-bridge.js';
import { WebSocketBridge } from './ws-bridge.js';

// ── Capability factories — installs the lazy `bridge.capabilities` getter
//    so consumers see the symmetric Scale 0/1/2 surface (CONTRACTS.md §2).
import { installCapabilityGetter } from './bridge/capabilities/install.js';
installCapabilityGetter(WasmBridge.prototype);
installCapabilityGetter(WebSocketBridge.prototype);

// ── Public re-exports ──────────────────────────────────────────────
export { WasmBridge, WebSocketBridge };
export { CosmicMockBridge } from './bridge/mock-scale5.js';
export { createBridge } from './bridge/bridge-factory.js';
