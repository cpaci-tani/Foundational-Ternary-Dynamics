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
        // Single kind-dispatched chokepoint (bridge.getSamplerOr, defined once in
        // bridge-contract.js): maps `kind` → the bridge's concrete sampler, keeps
        // the empty-sample fallback (CONTRACTS.md §2.3 — the optional pattern is
        // intentional), and — unlike the old per-site `?.() ?? empty` — logs loudly
        // if a bridge has DROPPED a sampler (§2.4 surface drift) instead of blanking
        // the overlay silently. Consolidation only; behavior is unchanged.
        getScale0FieldSamples: ({ kind, stride = 2 } = {}) => bridge.getSamplerOr(kind, stride),
        getScale0ForceField: (type, stride = 2) => {
            if (type === 'em') return bridge.getEMForceField(stride);
            if (type === 'gravity') return bridge.getGravityForceField(stride);
            if (type === 'strong') return bridge.getStrongForceField(stride);
            return { positions: new Float32Array(0), vectors: new Float32Array(0), count: 0 };
        },
        getScale0Diagnostics: () => bridge.getDiagnostics(),
        getScale0EnergyAudit: () => bridge.getEnergyAudit(),
        // Explicit O(L^3) scientific qualification capture. Direct WASM may
        // return synchronously; worker/native transports return a Promise.
        // Consumers must use `await Promise.resolve(...)` and must never poll
        // this from requestAnimationFrame.
        getScale0DynamicalStateDigest: () => bridge.getDynamicalStateDigest?.() ?? null,
        getScale0ArtifactIdentity: () => bridge.getWasmArtifactIdentity?.() ?? null,
        getScale0Lagrangian: () => bridge.getLagrangian(),
        getScale0KnotTelemetry: () => bridge.getKnotTelemetry?.() ?? null,
        getScale0KnotEvents: () => bridge.getKnotEvents?.() ?? null,
        getScale0KnotAggregate: () => bridge.getKnotAggregate?.() ?? null,
        getScale0DerivedOverlayData: (kind) => {
            if (typeof bridge.getScale0DerivedOverlayData === 'function') return bridge.getScale0DerivedOverlayData(kind);
            return null;
        },
        setBoundaryShape: (shape) => bridge.setBoundaryShape?.(shape),
        setReflectiveBoundary: (on) => bridge.setReflectiveBoundary?.(on),
        setFluxBoundaryMode: (mode) => bridge.setFluxBoundaryMode?.(mode),
        setupScenario: (name) => bridge.setupScenario(name),
        setToggle: (key, value) => bridge.setToggle?.(key, value),
        // Phase 2 gravity panel: REAL C++ latency field (voxel.latency Poisson),
        // distinct from the |J|² proxy. Both bridges expose these (WASM = real,
        // MockBridge = inactive stub), so the panel contract is symmetric.
        getScale0GravityMetricAgg: () => bridge.getGravityMetricAgg?.()
            ?? { active: false, latencyMax: 0, latencyMean: 0, fMin: 1, gammaMax: 1, dilationMaxPct: 0, voxelCount: 0 },
        getScale0LatencyVolume: () => bridge.getLatencyVolume?.() ?? new Float64Array(0),
        get latticeSize() { return bridge.latticeSize || 33; },
    };
}
