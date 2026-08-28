/**
 * @file engine/web/js/bridge-init.js
 * @purpose Bridge barrel + capability-getter installer. Re-exports the
 *          Scale-0 bridge classes and the
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

/**
 * Create the canonical in-thread WASM bridge.
 * @param {number} latticeSize cubic lattice dimension
 * @returns {Promise<WasmBridge>}
 */
export async function createBridge(latticeSize = 33) {
    const wasm = new WasmBridge();
    if (await wasm.init(latticeSize)) return wasm;
    throw new Error('WASM engine initialization failed — check console for details.');
}
