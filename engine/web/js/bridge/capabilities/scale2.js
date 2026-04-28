/**
 * @file engine/web/js/bridge/capabilities/scale2.js
 * @purpose Scale-2 (atom engine) capability factory. Returns the
 *          object surfaced as `bridge.capabilities.scale2` for both
 *          MockBridge and WasmBridge.
 * @consumers ./install.js, engine/web/js/scales/scale2/controller.js
 * @contract CONTRACTS.md §2 (Capability Factory Contract).
 * @related ./scale0.js, ./scale1.js (sibling factories).
 *
 * Phase 2c of the refactor sweep extracted createScale2Capabilities
 * from wasm-bridge-dag.js. Body unchanged.
 */

export function createScale2Capabilities(bridge) {
    return {
        tickScale2: () => bridge.aeTick?.(),
        getScale2AtomFrame: () => bridge.aeGetAtomData?.(),
        getScale2Diagnostics: () => bridge.aeGetDiagnostics?.(),
    };
}
