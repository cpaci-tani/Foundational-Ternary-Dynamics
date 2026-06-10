/**
 * @file engine/web/js/bridge/capabilities/scale1.js
 * @purpose Scale-1 (particle engine) capability factory. Returns the
 *          object surfaced as `bridge.capabilities.scale1` for both
 *          MockBridge and WasmBridge.
 * @consumers ./install.js, engine/web/js/scales/scale1/controller.js
 * @contract CONTRACTS.md §2 (Capability Factory Contract).
 * @related ./scale0.js, ./scale2.js (sibling factories).
 *
 * Phase 2c of the refactor sweep extracted createScale1Capabilities
 * from bridge-init.js. Body unchanged.
 */

export function createScale1Capabilities(bridge) {
    return {
        tickScale1: () => bridge.peTick?.(),
        getScale1ParticleFrame: () => bridge.peGetParticleData?.(),
        getScale1Diagnostics: () => bridge.peGetDiagnostics?.(),
        getScale1ExtendedData: () => bridge.peGetExtendedData?.(),
        getScale1Forces: () => bridge.peGetForces?.(),
        getScale1FieldSources: () => bridge.peGetFieldSources?.(),
        getScale1ParticleTypes: () => bridge.peGetParticleTypes?.(),
        getScale1Capabilities: () => bridge.peGetBackendCapabilities?.() ?? {
            velocities: false,
            masses: false,
            locked: false,
            forces: false,
            extended: false,
            nativeExtended: false,
            nativeForces: false,
            advancedForces: false,
        },
    };
}
