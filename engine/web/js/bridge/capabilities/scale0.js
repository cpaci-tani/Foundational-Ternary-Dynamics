/**
 * @file engine/web/js/bridge/capabilities/scale0.js
 * @purpose Scale-0 (lattice/substrate) capability factory. Returns the
 *          object surfaced as `bridge.capabilities.scale0` for both
 *          MockBridge and WasmBridge so consumers see one symmetric API.
 * @consumers engine/web/js/bridge/capabilities/install.js (mounts via
 *            installCapabilityGetter), engine/web/js/scales/scale0/
 *            controller.js, scenarios, viewport adapters.
 * @contract CONTRACTS.md §2 (Capability Factory Contract).
 * @related ./scale1.js, ./scale2.js (sibling factories);
 *          ../mock-bridge.js, ../wasm-bridge.js (the bridges this wraps).
 *
 * Phase 2c of the refactor sweep extracted createScale0Capabilities
 * from bridge-init.js. Body unchanged — every method delegates to
 * the underlying bridge instance via closure capture.
 */

export function createScale0Capabilities(bridge) {
    return {
        tickScale0: () => bridge.tick(),
        getScale0ParticleFrame: () => bridge.getParticleData(),
        getScale0FluxVolume: () => bridge.getFluxVolume(),
        getScale0FluxSlice: (axis, index) => bridge.getFluxSlice(axis, index),
        getScale0FieldSamples: ({ kind, stride = 2 } = {}) => {
            if (kind === 'e') return bridge.getEFieldSampled(stride);
            if (kind === 'b') return bridge.getBFieldSampled(stride);
            if (kind === 'poynting') return bridge.getPoyntingSampled(stride);
            if (kind === 'divJ') return bridge.getDivJSampled(stride);
            if (kind === 'fluxVector') return bridge.getFluxVectorSampled(stride);
            if (kind === 'vorticity')  return bridge.getVorticitySampled?.(stride) ?? { positions: new Float32Array(0), values: new Float32Array(0), count: 0 };
            if (kind === 'helicity')   return bridge.getHelicitySampled?.(stride) ?? { positions: new Float32Array(0), values: new Float32Array(0), count: 0 };
            if (kind === 'kretschmann') return bridge.getKretschmannSampled?.(stride) ?? { positions: new Float32Array(0), values: new Float32Array(0), count: 0 };
            if (kind === 'latency')    return bridge.getLatencySampled?.(stride) ?? { positions: new Float32Array(0), values: new Float32Array(0), count: 0 };
            if (kind === 'fisher')     return bridge.getFisherSampled?.(stride) ?? { positions: new Float32Array(0), values: new Float32Array(0), count: 0 };
            if (kind === 'coherence')  return bridge.getCoherenceSampled?.(stride) ?? { positions: new Float32Array(0), values: new Float32Array(0), count: 0 };
            if (kind === 'curlJ')      return bridge.getCurlJSampled?.(stride) ?? { positions: new Float32Array(0), vectors: new Float32Array(0), count: 0 };
            if (kind === 'state')      return bridge.getStateFieldSampled?.(stride) ?? { positions: new Float32Array(0), values: new Float32Array(0), count: 0 };
            if (kind === 'gaussResidual') return bridge.getGaussResidualSampled?.(stride) ?? { positions: new Float32Array(0), values: new Float32Array(0), count: 0 };
            return { positions: new Float32Array(0), vectors: new Float32Array(0), count: 0 };
        },
        getScale0ForceField: (type, stride = 2) => {
            if (type === 'em') return bridge.getEMForceField(stride);
            if (type === 'gravity') return bridge.getGravityForceField(stride);
            if (type === 'strong') return bridge.getStrongForceField(stride);
            return { positions: new Float32Array(0), vectors: new Float32Array(0), count: 0 };
        },
        getScale0Diagnostics: () => bridge.getDiagnostics(),
        getScale0EnergyAudit: () => bridge.getEnergyAudit(),
        getScale0Lagrangian: () => bridge.getLagrangian(),
        getScale0DerivedOverlayData: (kind) => {
            if (typeof bridge.getScale0DerivedOverlayData === 'function') return bridge.getScale0DerivedOverlayData(kind);
            return null;
        },
        setBoundaryShape: (shape) => bridge.setBoundaryShape?.(shape),
        setReflectiveBoundary: (on) => bridge.setReflectiveBoundary?.(on),
        setupScenario: (name) => bridge.setupScenario(name),
        setToggle: (key, value) => bridge.setToggle?.(key, value),
        get latticeSize() { return bridge.latticeSize || 33; },
    };
}
