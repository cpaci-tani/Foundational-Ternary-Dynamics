/**
 * Bridge Factory — creates the appropriate simulation bridge.
 *
 * Tries WASM first (WasmBridge); falls back to JS MockBridge.
 * Extracted from wasm-bridge.js to reduce monolith size.
 */

import { WasmBridge, MockBridge } from '../wasm-bridge.js';
import { CosmicMockBridge } from './mock-scale5.js';

/**
 * Create a simulation bridge for the given lattice size.
 * Attempts WASM initialization; returns MockBridge on failure.
 * @param {number} latticeSize - Cubic lattice dimension (default 32)
 * @returns {Promise<WasmBridge|MockBridge>}
 */
export async function createBridge(latticeSize = 32) {
    const wasm = new WasmBridge();
    const ok = await wasm.init(latticeSize);
    if (ok) return wasm;
    const mock = new MockBridge(latticeSize);
    return mock;
}

// Re-export for backward compatibility
export { MockBridge, CosmicMockBridge };
