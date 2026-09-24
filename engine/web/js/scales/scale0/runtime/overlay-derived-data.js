/** Scalar/derived overlay computation and its matching adapter routes. */
import { DUAL_DELTA, K_GENESIS } from '../../../constants.js';
import { getFieldLineKnotTracker } from './field-line-knots.js';
import {
    computePsiSquaredFrame,
    computePhaseFrame,
    computeLagrangianDensityFrame,
    computeEntropyDensityFrame,
    computeGravPotentialFrame,
    computeEmEnergyFrame,
    computeChargeDensityFrame,
    computeVorticityFrame,
    computeHorizonFrame,
    computeEPressureFrame,
    computeBPressureFrame,
    computeStateFieldFrame,
    computeLatencyFrame,
    computeGaussResidualFrame,
    computeProperTimeFrame,
    computeLapseFrame,
    computeDbPhaseFrame,
} from './overlay-frames.js?v=2';

export function buildDerivedSubstrateData(state, sampled, fieldCapability, N) {
    const frame = {};
    const flags = state.fieldFlags;

    if (flags.showDarkMatterHalo || flags.showGenesisIsosurface) {
        // Mock/WASM return dense N³ |J| magnitudes. Native FTV2 returns a
        // compact regular-grid descriptor {data,latticeSize,stride,axisCount}
        // so large CUDA lattices never perform a full N³ D2H/socket copy.
        // Both are x-fastest and are consumed without expanding FTV2.
        const fluxVol = fieldCapability?.getScale0FluxVolume?.();
        const compactData = fluxVol?.data;
        const compactAxis = Math.trunc(Number(fluxVol?.axisCount) || 0);
        const compactValid = ArrayBuffer.isView(compactData)
            && Math.trunc(Number(fluxVol?.latticeSize)) === N
            && compactAxis > 0
            && compactData.length === compactAxis * compactAxis * compactAxis;
        const denseValid = ArrayBuffer.isView(fluxVol) && fluxVol.length >= N * N * N;
        if (compactValid || denseValid) {
            const magnitude = fluxVol;
            if (flags.showDarkMatterHalo) {
                const parts = fieldCapability?.getScale0ParticleFrame?.();
                frame.darkMatterHalo = { particles: parts?.positions ?? null, magnitude, latticeSize: N };
            }
            if (flags.showGenesisIsosurface) {
                frame.genesisIsosurface = { magnitude, latticeSize: N, threshold: K_GENESIS };
            }
        }
    }
    if (flags.showDampingZones) {
        const parts = fieldCapability?.getScale0ParticleFrame?.();
        if (parts) frame.dampingZones = { particles: parts.positions, latticeSize: N };
    }
    if (flags.showKnotZones) {
        // Boxes follow the detected FIELD-LINE knots (clumps of crossing E/B/J
        // streamlines), not manifested particles. Triple frame: E, B, and flux
        // families; each empty when that family's streamlines have not been recorded.
        const e = getFieldLineKnotTracker('e').getKnotZones();
        const b = getFieldLineKnotTracker('b').getKnotZones();
        const flux = getFieldLineKnotTracker('flux').getKnotZones();
        // Always publish the effective overlay's current detector frame. The
        // all-empty frame is semantically meaningful: it clears the renderer's
        // previous draw range when the last detected clump disappears.
        frame.knotZones = { e, b, flux };
    }

    if (state.fieldFlags.showDualSubstrate && sampled.fluxVector?.count > 0) {
        // [TIER-1 VISUAL] Scalar (1+/-delta)/2 decomposition is an amplitude
        // asymmetry demonstration with no handedness content. It does not
        // sample the engine's independently stored L/R records. Surfaced
        // as "dual substrate" for visualization only.
        prepareDualAmplitudeSplit(sampled.fluxVector, state);
        frame.dualFlux = {
            left: { positions: sampled.fluxVector.positions, vectors: state.dualLVecs, count: sampled.fluxVector.count },
            right: { positions: sampled.fluxVector.positions, vectors: state.dualRVecs, count: sampled.fluxVector.count },
        };
    }

    if (state.fieldFlags.showChirality && sampled.fluxVector?.count > 0) {
        if (!state.chiralValues || state.chiralValues.length < sampled.fluxVector.count) {
            state.chiralValues = new Float32Array(sampled.fluxVector.count);
        }
        for (let i = 0; i < sampled.fluxVector.count; i++) {
            const x = sampled.fluxVector.vectors[i * 3];
            const y = sampled.fluxVector.vectors[i * 3 + 1];
            const z = sampled.fluxVector.vectors[i * 3 + 2];
            const mag = Math.sqrt(x * x + y * y + z * z);
            state.chiralValues[i] = mag * DUAL_DELTA;
        }
        frame.chirality = {
            positions: sampled.fluxVector.positions,
            values: state.chiralValues,
            count: sampled.fluxVector.count,
        };
    }

    return frame;
}

function prepareDualAmplitudeSplit(flux, state) {
    const vecLen = flux.count * 3;
    if (!state.dualLVecs || state.dualLVecs.length < vecLen) {
        state.dualLVecs = new Float32Array(vecLen);
        state.dualRVecs = new Float32Array(vecLen);
    }
    const leftFactor = (1 + DUAL_DELTA) / 2;
    const rightFactor = (1 - DUAL_DELTA) / 2;
    for (let i = 0; i < vecLen; i++) {
        state.dualLVecs[i] = flux.vectors[i] * leftFactor;
        state.dualRVecs[i] = flux.vectors[i] * rightFactor;
    }
}

function computeCurrentPhaseFrame(sampled, state) {
    if (!sampled.fluxVector?.count) return null;
    // Phase is the declared scalar-split proxy. Its two inputs must come from
    // this exact owned sample, even when the separate Dual J view is hidden.
    // Reusing a previous sweep's split can manufacture a pi phase jump when
    // the current vector changes direction or the retained sample order moves.
    prepareDualAmplitudeSplit(sampled.fluxVector, state);
    return computePhaseFrame(sampled, state, state.dualLVecs, state.dualRVecs);
}

// Static scalar-overlay table, allocated ONCE at module load (never per sweep).
// Each entry is [flag, computeFn, applyFn]. Splitting compute/apply lets a
// scalar job run with ZERO per-run allocation: it computes the frame value and
// applies it directly via the adapter call, instead of boxing it in a fresh
// `{ key: value }` object as the old `(s) => ({ key: ... })` closures did. The
// compute/apply pair for each row pairs an overlay-frames.js compute*Frame
// builder with the matching viewport-adapter apply call.
export const SCALAR_JOBS = [
    ['showPsiSquared',        (s, ctx, state) => computePsiSquaredFrame(s, state, state.fieldFlags.showDualSubstrate), (va, v) => va.applyPsiSquared(v)],
    ['showPhase',             (s, ctx, state) => computeCurrentPhaseFrame(s, state),                                (va, v) => va.applyPhase(v)],
    ['showLagrangianDensity', (s, ctx, state) => computeLagrangianDensityFrame(s, state),                             (va, v) => va.applyLagrangianDensity(v)],
    ['showEntropyDensity',    (s, ctx, state) => computeEntropyDensityFrame(s, state),                                (va, v) => va.applyEntropyDensity(v)],
    ['showGravPotential',     (s, ctx, state) => computeGravPotentialFrame(ctx, s, state),                            (va, v) => va.applyGravPotential(v)],
    ['showEmEnergy',          (s, ctx, state) => computeEmEnergyFrame(s, state),                                      (va, v) => va.applyEmEnergy(v)],
    ['showChargeDensity',     (s, ctx, state) => computeChargeDensityFrame(s, state),                                 (va, v) => va.applyChargeDensity(v)],
    ['showVorticity',         (s, ctx, state) => computeVorticityFrame(s, state),                                     (va, v) => va.applyVorticity(v)],
    ['showHorizon',           (s, ctx, state) => computeHorizonFrame(s, state),                                       (va, v) => va.applyHorizon(v)],
    ['showEPressure',         (s, ctx, state) => computeEPressureFrame(s, state),                                     (va, v) => va.applyEPressure(v)],
    ['showBPressure',         (s, ctx, state) => computeBPressureFrame(s, state),                                     (va, v) => va.applyBPressure(v)],
    ['showStateField',        (s, ctx, state) => computeStateFieldFrame(s, state),                                    (va, v) => va.applyStateField(v)],
    ['showLatency',           (s, ctx, state) => computeLatencyFrame(s, state),                                       (va, v) => va.applyLatency(v)],
    ['showGaussResidual',     (s, ctx, state) => computeGaussResidualFrame(s, state),                                 (va, v) => va.applyGaussResidual(v)],
    ['showProperTime',        (s, ctx, state) => computeProperTimeFrame(s, state),                                    (va, v) => va.applyProperTime(v)],
    ['showLapse',             (s, ctx, state) => computeLapseFrame(s, state),                                         (va, v) => va.applyLapse(v)],
    ['showDBPhase',           (s, ctx, state) => computeDbPhaseFrame(s, state),                                       (va, v) => va.applyDBPhase(v)],
];

// Scalar / derived / passthrough applies are 1:1 with their overlayFrame key.
export function applyDerivedJob(frame, viewportAdapter) {
    if (frame.darkMatterHalo) viewportAdapter.applyDarkMatterHalo(frame.darkMatterHalo);
    if (frame.dampingZones) viewportAdapter.applyDampingZones(frame.dampingZones);
    if (frame.knotZones) viewportAdapter.applyKnotZones(frame.knotZones);
    if (frame.genesisIsosurface) viewportAdapter.applyGenesisIsosurface(frame.genesisIsosurface);
    if (frame.dualFlux) viewportAdapter.applyDualFlux(frame.dualFlux.left, frame.dualFlux.right);
    if (frame.chirality) viewportAdapter.applyChirality(frame.chirality);
}
