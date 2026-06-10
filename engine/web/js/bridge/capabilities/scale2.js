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
 * from bridge-init.js. Body unchanged.
 */

export function createScale2Capabilities(bridge) {
    return {
        tickScale2: () => bridge.aeTick?.(),
        getScale2AtomFrame: () => bridge.aeGetAtomData?.(),
        getScale2Diagnostics: () => bridge.aeGetDiagnostics?.(),
        // Scale 2 deep pass (2026-06-10): read surfaces consumed by the
        // force-arrow overlay, AE field overlay, and diagnostics descriptors.
        getScale2ForceDecomposition: (want) => bridge.aeGetForceDecomposition?.(want),
        getScale2FieldSources: () => bridge.aeGetFieldSources?.(),
        getScale2RuntimeState: () => bridge.aeGetRuntimeState?.(),
    };
}
