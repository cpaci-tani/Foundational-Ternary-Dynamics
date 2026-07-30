/**
 * @file engine/web/js/bridge/capabilities/scale1.js
 * @purpose Scale-1 (particle engine) capability factory. Returns the
 *          object surfaced as `bridge.capabilities.scale1` for the live
 *          bridges (WasmBridge, WebSocketBridge). Backed by the native
 *          C++/WASM ParticleEngine via bridge/native-particle-engine.js.
 * @consumers ./install.js, engine/web/js/scales/scale1/*
 * @contract CONTRACTS.md §2 (Capability Factory Contract).
 * @related ./scale0.js, ./scale2.js (sibling factories).
 */

export function createScale1Capabilities(bridge) {
    return {
        tickScale1: () => bridge.peTick?.(),
        getScale1ParticleFrame: () => bridge.peGetParticleData?.(),
        getScale1Diagnostics: () => bridge.peGetDiagnostics?.(),
        getScale1ExtendedData: () => bridge.peGetExtendedData?.(),
        getScale1Forces: () => bridge.peGetForces?.(),
        getScale1ForceDecomposition: () => bridge.peGetForceDecomposition?.(),
        getScale1FieldSources: () => bridge.peGetFieldSources?.(),
        getScale1ParticleTypes: () => bridge.peGetParticleTypes?.(),
        /** 'wasm' when the native module is live, 'unavailable' otherwise. */
        getScale1Backend: () =>
            bridge.peGetBackendCapabilities?.()?.backend ?? 'unavailable',
        getScale1Capabilities: () => bridge.peGetBackendCapabilities?.() ?? {
            backend: 'unavailable',
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
