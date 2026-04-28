/**
 * @file wasm-bridge-dag.js
 * @brief WASM Bridge — abstraction layer between UI and simulation engine.
 *
 * [EXTENDED] Provides a MockBridge for development (no WASM needed) and a WasmBridge
 * for production (loads compiled ftd_core.wasm). The UI code only talks
 * to the Bridge interface, never directly to WASM or mock internals.
 */


// ── Phase 2a refactor sweep (2026-04-27) ────────────────────────────
// MockBridge moved to bridge/mock-bridge.js; imported and re-exported
// here so existing consumers (`import { MockBridge } from './wasm-bridge-dag.js'`)
// see no API change AND the local installCapabilityGetter call near the
// bottom of this file still resolves the symbol. WasmBridge + capability
// factories remain below — Phase 2b will extract WasmBridge, Phase 2c
// the factories. See docs/adr/0003-wasm-bridge-dag-refactor.md and
// .claude/plans/i-want-to-try-crispy-charm.md Phase 2.
import { MockBridge } from './bridge/mock-bridge.js';
export { MockBridge };

// ── Imports for the capability factories that remain in this file ──
// (K_GENESIS used by the genesisIsosurface overlay path on MockBridge.prototype.)
import { K_GENESIS } from './constants.js';


// ── Phase 2b refactor sweep (2026-04-27) ────────────────────────────
// WasmBridge moved to bridge/wasm-bridge.js along with its helpers
// (_wasmCallOr, EMPTY_* sampler-fallback singletons, _wasmLoadPromise).
// Imported and re-exported here so existing consumers see no API
// change AND the local installCapabilityGetter call below still
// resolves the symbol. The capability factories stay in this file
// for Phase 2c. See docs/adr/0003-wasm-bridge-dag-refactor.md and
// .claude/plans/i-want-to-try-crispy-charm.md Phase 2.
import { WasmBridge } from './bridge/wasm-bridge.js';
export { WasmBridge };


/**
 * Returns derived-overlay data shaped per `kind`. Most return objects
 * include a live reference to `_fluxMag` (and/or `_particles`) — these
 * are mutated each tick. Callers must treat the buffers as read-only
 * within the current frame; if you need a stable snapshot, `.slice()`
 * the magnitude at the call site. Same retention-foot-gun applies to
 * `_particles`: the array identity is stable but per-particle fields
 * mutate in place.
 */
MockBridge.prototype.getScale0DerivedOverlayData = function (kind) {
    if (kind === 'darkMatterHalo') {
        if (!this._fluxJ) return null;
        this._ensureEnergyCache();
        return { particles: this._particles, magnitude: this._fluxMag, latticeSize: this.latticeSize };
    }
    if (kind === 'dampingZones') {
        return { particles: this._particles, latticeSize: this.latticeSize };
    }
    if (kind === 'genesisIsosurface') {
        if (!this._fluxJ) return null;
        this._ensureEnergyCache();
        return { magnitude: this._fluxMag, latticeSize: this.latticeSize, threshold: K_GENESIS };
    }
    return null;
};

function createScale0Capabilities(bridge) {
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
         * LOD 1/2 are upsampled to N³ on the fly (nearest neighbor) so scrub
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

function createScale1Capabilities(bridge) {
    return {
        tickScale1: () => bridge.peTick?.(),
        getScale1ParticleFrame: () => bridge.peGetParticleData?.(),
        getScale1Diagnostics: () => bridge.peGetDiagnostics?.(),
    };
}

function createScale2Capabilities(bridge) {
    return {
        tickScale2: () => bridge.aeTick?.(),
        getScale2AtomFrame: () => bridge.aeGetAtomData?.(),
        getScale2Diagnostics: () => bridge.aeGetDiagnostics?.(),
    };
}

function installCapabilityGetter(proto) {
    Object.defineProperty(proto, 'capabilities', {
        configurable: true,
        get() {
            if (!this._capabilities) {
                this._capabilities = {
                    scale0: createScale0Capabilities(this),
                    scale1: createScale1Capabilities(this),
                    scale2: createScale2Capabilities(this),
                };
            }
            return this._capabilities;
        },
    });
}

installCapabilityGetter(MockBridge.prototype);
installCapabilityGetter(WasmBridge.prototype);

// ── Re-exports from extracted modules ────────────────────────────────
// CosmicMockBridge moved to bridge/mock-scale5.js (Scale 5 N-body sim)
// createBridge factory moved to bridge/bridge-factory-dag.js
export { CosmicMockBridge } from './bridge/mock-scale5.js';
export { createBridge } from './bridge/bridge-factory-dag.js';
