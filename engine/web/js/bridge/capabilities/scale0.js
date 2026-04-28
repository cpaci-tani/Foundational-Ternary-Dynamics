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
 * from wasm-bridge-dag.js. Body unchanged — every method delegates to
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
        get latticeSize() { return bridge.latticeSize || 32; },

        /**
         * Readback current lattice state + flux into plain typed arrays plus
         * particle list and an audit scalar snapshot. Used by the TimelineBuffer.
         * Returns null if the bridge lacks full readback.
         */
        getScale0Snapshot: () => {
            const N = bridge.latticeSize || 32;
            const lattice = bridge.getScale0LatticeBuffer?.();
            const flux    = bridge.getScale0FluxBuffer?.();
            const wave    = bridge.getScale0WaveBuffer?.();
            const particles = bridge.getScale0ParticleList?.() || [];
            if (!lattice || !flux) return null;
            return {
                tick: bridge.getDiagnostics?.().tick ?? 0,
                ts: performance.now(),
                lod: 0,
                N,
                lattice,
                flux,
                wave,
                particles,
                audit: bridge.getDiagnostics?.() ?? {},
            };
        },

        /**
         * Write a snapshot back into the engine. LOD 0 snapshots go in as-is;
         * LOD 1/2 are upsampled to N^3 on the fly (nearest neighbor) so scrub
         * playback works across the entire timeline — including coarse-grained
         * zones. LOD 3 is telemetry-only and cannot be loaded. Returns true
         * on success, false otherwise.
         */
        loadScale0Snapshot: (snap) => {
            if (!snap) return false;
            const write     = bridge.setScale0LatticeBuffer?.bind(bridge);
            const writeFlux = bridge.setScale0FluxBuffer?.bind(bridge);
            if (!write || !writeFlux) return false;

            let lattice = snap.lattice;
            let flux    = snap.flux;
            const wave  = snap.wave;
            if (!lattice || !flux) return false;

            if (snap.lod && snap.lod > 0 && snap.lod < 3) {
                // Lazy-load the upsampler to avoid a static import cycle.
                // eslint-disable-next-line no-undef
                const N = bridge.latticeSize || snap.N || 32;
                const mod = (typeof window !== 'undefined') ? window.__ftdTimelineLod : null;
                if (!mod) {
                    // Fallback: fail instead of corrupting state with mismatched sizes.
                    return false;
                }
                lattice = mod.upsampleScalar(lattice, N, snap.lod);
                flux    = mod.upsampleVec3(flux, N, snap.lod);
            } else if (snap.lod >= 3) {
                return false; // telemetry-only snapshot cannot reconstruct state
            }

            write(lattice);
            writeFlux(flux);
            if (wave && bridge.setScale0WaveBuffer) bridge.setScale0WaveBuffer(wave);
            if (bridge.setScale0Tick) bridge.setScale0Tick(snap.tick);
            if (bridge.setScale0ParticleList) bridge.setScale0ParticleList(snap.particles);
            return true;
        },
    };
}
