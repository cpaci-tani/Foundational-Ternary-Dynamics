/**
 * Bridge Factory — creates the WASM simulation bridge.
 */

import { WasmBridge } from './wasm-bridge.js';

/**
 * Create a simulation bridge for the given lattice size.
 * @param {number} latticeSize - Cubic lattice dimension (default 33)
 * @returns {Promise<WasmBridge>}
 * @throws if WASM initialization fails
 */
export async function createBridge(latticeSize = 33) {
    const wasm = new WasmBridge();
    const ok = await wasm.init(latticeSize);
    if (ok) return wasm;
    throw new Error('WASM engine initialization failed — check console for details.');
}
