/**
 * Physics module entry point. Lazily attaches a `PhysicsHarness` to a
 * bridge (MockBridge or WasmBridge) and exposes the canonical accessor
 * helpers panels need.
 *
 * The harness is a thin wrapper, NOT a replacement for the bridge —
 * scenario dispatch defers to whichever underlying implementation the
 * active bridge owns (C++ canonical for WASM, native JS for MockBridge).
 * See `physics-harness.js` for the contract.
 */

import { PhysicsHarness } from './physics-harness.js';

// One harness instance per bridge. Stored on the bridge itself so each
// bridge's harness is collected with the bridge.
const HARNESS_KEY = '__ftdPhysicsHarness__';

/**
 * Lazily attach + return the PhysicsHarness for a given bridge.
 * Creates the harness on first call and stores it on the bridge.
 *
 * @param {object} bridge - WasmBridge or MockBridge instance
 * @returns {PhysicsHarness}
 */
export function getPhysicsHarness(bridge) {
    if (!bridge) return null;
    if (!bridge[HARNESS_KEY]) {
        bridge[HARNESS_KEY] = new PhysicsHarness(bridge);
    }
    return bridge[HARNESS_KEY];
}

// Re-export so consumers need only one import line for both the
// harness factory and the canonical accessor helpers.
export {
    PhysicsHarness,
    getParticleCharge,
    findOppositeChargePairFromList,
} from './physics-harness.js';
