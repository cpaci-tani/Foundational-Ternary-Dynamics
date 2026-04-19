import {
    computeStreamlines,
    generateEFieldSeeds,
    generateBFieldSeeds,
    generateGridSeeds,
    generateImportanceSeeds,
    generateBImportanceSeeds,
} from '../../../fieldlines.js';
import { DUAL_DELTA } from '../../../constants.js';

export function sampleFieldState(fieldCapability, flags, stride) {
    const sampled = {};
    // Tier 1 quantum overlays all derive from fluxVector + optional poynting,
    // so pull those samples whenever any quantum toggle is active.
    const needFlux = flags.showFluxLines || flags.showDualSubstrate || flags.showChirality ||
        flags.showForceWeak || flags.showPsiSquared || flags.showPhase ||
        flags.showLagrangianDensity || flags.showEntropyDensity || flags.showGravPotential;
    if (needFlux) {
        sampled.fluxVector = fieldCapability.getScale0FieldSamples({ kind: 'fluxVector', stride });
    }
    if (flags.showPoynting || flags.showLight || flags.showLagrangianDensity) {
        sampled.poynting = fieldCapability.getScale0FieldSamples({ kind: 'poynting', stride });
    }
    if (flags.showEField || flags.showEmEnergy || flags.showEPressure)
        sampled.eField = fieldCapability.getScale0FieldSamples({ kind: 'e', stride });
    if (flags.showBField || flags.showEmEnergy || flags.showBPressure)
        sampled.bField = fieldCapability.getScale0FieldSamples({ kind: 'b', stride });
    if (flags.showDivField || flags.showLagrangianDensity || flags.showChargeDensity) {
        sampled.divergence = fieldCapability.getScale0FieldSamples({ kind: 'divJ', stride });
    }
    if (flags.showVorticity) {
        sampled.vorticity = fieldCapability.getScale0FieldSamples({ kind: 'vorticity', stride });
    }
    // Tier 1/2/3 samplers (2026-04-18)
    if (flags.showHelicity)
        sampled.helicity = fieldCapability.getScale0FieldSamples({ kind: 'helicity', stride });
    if (flags.showKretschmann)
        sampled.kretschmann = fieldCapability.getScale0FieldSamples({ kind: 'kretschmann', stride });
    if (flags.showHorizon)
        sampled.latency = fieldCapability.getScale0FieldSamples({ kind: 'latency', stride });
    if (flags.showFisher)
        sampled.fisher = fieldCapability.getScale0FieldSamples({ kind: 'fisher', stride });
    if (flags.showCoherence)
        sampled.coherence = fieldCapability.getScale0FieldSamples({ kind: 'coherence', stride });
    // Weak force is parity-violating; its natural vector proxy is the curl
    // of J (pseudovector), not J itself. Pull the curl-J sample when the
    // weak overlay is active.
    if (flags.showForceWeak)
        sampled.curlJ = fieldCapability.getScale0FieldSamples({ kind: 'curlJ', stride });
    return sampled;
}

// ══════════════════════════════════════════════════════════════════════
// Tier 1 quantum-overlay derivation helpers
// See docs/SPEC_S0_QUANTUM_OVERLAYS.md §3.
// ══════════════════════════════════════════════════════════════════════

function ensureTier1Buffers(state, N) {
    if (!state.t1 || state.t1.size !== N) {
        state.t1 = {
            size: N,
            psi2:     new Float32Array(N),
            phase:    new Float32Array(N),
            lagr:     new Float32Array(N),
            entropy:  new Float32Array(N),
            gravPot:  new Float32Array(N),
            emEnergy: new Float32Array(N),
            rho:      new Float32Array(N),
            vort:     new Float32Array(N),
            normalizer: { psi2Max: 0, lagMax: 0, gravMax: 0, emMax: 0, rhoMax: 0, vortMax: 0 },
        };
    }
    return state.t1;
}

function computePsiSquaredFrame(sampled, state, dualActive) {
    if (!sampled.fluxVector?.count) return null;
    const buf = ensureTier1Buffers(state, sampled.fluxVector.count);
    const { vectors, positions, count } = sampled.fluxVector;
    // |ψ|² = |J_L|² + |J_R|² when dual substrate is active, else |J|².
    // When dual is on the state store already tracks dualLVecs/dualRVecs, but
    // for Tier 1 we use the (J_L + J_R) invariant = |J|², which equals
    // |J_L|² + |J_R|² + 2·J_L·J_R. The cross term vanishes for orthogonal
    // chiralities, so using |J|² is a faithful approximation at this tier.
    let max = 0;
    for (let i = 0; i < count; i++) {
        const x = vectors[i * 3];
        const y = vectors[i * 3 + 1];
        const z = vectors[i * 3 + 2];
        const v = x * x + y * y + z * z;
        buf.psi2[i] = v;
        if (v > max) max = v;
    }
    buf.normalizer.psi2Max = max;
    return { positions, values: buf.psi2, count, normalizer: max, dualActive };
}

function computePhaseFrame(sampled, state, dualLVecs, dualRVecs) {
    if (!sampled.fluxVector?.count) return null;
    const buf = ensureTier1Buffers(state, sampled.fluxVector.count);
    const { positions, count } = sampled.fluxVector;
    // FTD phase is arg(J_L + i*J_R) — a signed quantity spanning [-pi, pi].
    // We project the chirality pair onto the flux direction and take the
    // signed scalar components J_L.Jhat and J_R.Jhat. Using magnitudes
    // would collapse the output to [0, pi/2] and discard chirality sign.
    const hasDual = dualLVecs && dualRVecs && dualLVecs.length >= count * 3;
    const { vectors } = sampled.fluxVector;
    for (let i = 0; i < count; i++) {
        if (hasDual) {
            const jx = vectors[i * 3], jy = vectors[i * 3 + 1], jz = vectors[i * 3 + 2];
            const jmag = Math.sqrt(jx * jx + jy * jy + jz * jz);
            if (jmag < 1e-12) { buf.phase[i] = 0; continue; }
            const inv = 1 / jmag;
            const hx = jx * inv, hy = jy * inv, hz = jz * inv;
            const lx = dualLVecs[i * 3], ly = dualLVecs[i * 3 + 1], lz = dualLVecs[i * 3 + 2];
            const rx = dualRVecs[i * 3], ry = dualRVecs[i * 3 + 1], rz = dualRVecs[i * 3 + 2];
            // Signed projections onto J-direction preserve sign information.
            const lProj = lx * hx + ly * hy + lz * hz;
            const rProj = rx * hx + ry * hy + rz * hz;
            buf.phase[i] = Math.atan2(rProj, lProj);  // in [-pi, pi]
        } else {
            buf.phase[i] = 0;
        }
    }
    return { positions, values: buf.phase, count, dualAvailable: hasDual };
}

function computeLagrangianDensityFrame(sampled, state) {
    // [TIER-1 VISUAL] Per-voxel L(x) ~ (1/2)|J|^2 - (1/2)(div J)^2.
    // NOTE: (div J)^2 != |grad J|^2 in general — the Frobenius norm of
    // the 3x3 Jacobian captures shear flow, while (div J)^2 only captures
    // compressive divergence. This overlay is a faithful approximation
    // for kinetic-vs-gradient dominance, NOT a strict Lagrangian density.
    // For a true |grad J|^2 overlay we'd need to expose the full Jacobian
    // tensor through the capability interface.
    if (!sampled.fluxVector?.count) return null;
    const buf = ensureTier1Buffers(state, sampled.fluxVector.count);
    const { vectors, positions, count } = sampled.fluxVector;
    const divVals = sampled.divergence?.values;
    const hasDiv = divVals && divVals.length >= count;
    let maxAbs = 0;
    for (let i = 0; i < count; i++) {
        const x = vectors[i * 3];
        const y = vectors[i * 3 + 1];
        const z = vectors[i * 3 + 2];
        const kinetic = 0.5 * (x * x + y * y + z * z);
        // Use |divJ|² as a proxy for |∇J|² (not exactly equal, but a defensible
        // stand-in when we only sample the divergence-scalar field).
        const gradProxy = hasDiv ? 0.5 * divVals[i] * divVals[i] : 0;
        const L = kinetic - gradProxy;
        buf.lagr[i] = L;
        const a = Math.abs(L);
        if (a > maxAbs) maxAbs = a;
    }
    buf.normalizer.lagMax = maxAbs;
    return { positions, values: buf.lagr, count, normalizer: maxAbs };
}

function computeEntropyDensityFrame(sampled, state) {
    // Shannon entropy of the ternary state in a 3×3×3 Moore neighborhood.
    // We don't have per-voxel access to the state field from the overlay
    // runtime yet, so we proxy with a rank-based estimator: entropy is high
    // where |J| is near the median (disordered) and low where |J| is either
    // near zero (empty / crystallized) or near the maximum (saturated).
    // This is a Tier 1 stand-in; a true neighborhood-sampling estimator
    // will land when the state field is exposed through a capability call.
    if (!sampled.fluxVector?.count) return null;
    const buf = ensureTier1Buffers(state, sampled.fluxVector.count);
    const { vectors, positions, count } = sampled.fluxVector;
    // Pass 1: find max |J|
    let max = 0;
    for (let i = 0; i < count; i++) {
        const x = vectors[i * 3];
        const y = vectors[i * 3 + 1];
        const z = vectors[i * 3 + 2];
        const m = Math.sqrt(x * x + y * y + z * z);
        if (m > max) max = m;
    }
    const eps = 1e-9;
    // Pass 2: mapped entropy — 4·p·(1-p) where p = |J|/max gives a smooth
    // 0→1→0 bump (Gini-style impurity, equivalent to Shannon up to scale).
    for (let i = 0; i < count; i++) {
        const x = vectors[i * 3];
        const y = vectors[i * 3 + 1];
        const z = vectors[i * 3 + 2];
        const m = Math.sqrt(x * x + y * y + z * z);
        const p = max > eps ? m / max : 0;
        buf.entropy[i] = 4 * p * (1 - p);
    }
    return { positions, values: buf.entropy, count };
}

function computeGravPotentialFrame(ctx, sampled, state) {
    // If the bridge already exposes a gravitational potential field, prefer
    // that. Otherwise we approximate Φ(x) by a smoothed |J|² mass density:
    // true Φ satisfies ∇²Φ = 4πGρ, and a Gaussian smoothing of ρ is the
    // lowest-pass-filter analogue at fixed resolution — good enough to show
    // wells and peaks qualitatively.
    if (!sampled.fluxVector?.count) return null;
    if (typeof ctx.bridge?.getGravPotentialSamples === 'function') {
        const data = ctx.bridge.getGravPotentialSamples();
        if (data?.count > 0) return data;
    }
    const buf = ensureTier1Buffers(state, sampled.fluxVector.count);
    const { vectors, positions, count } = sampled.fluxVector;
    // Build |J|² as a pseudo-mass, normalize, then invert (negative for wells).
    let maxAbs = 0;
    for (let i = 0; i < count; i++) {
        const x = vectors[i * 3];
        const y = vectors[i * 3 + 1];
        const z = vectors[i * 3 + 2];
        const m = x * x + y * y + z * z;
        // Φ is negative where mass is concentrated → use -m as a monotone
        // proxy. Real Φ would be a spatial integral; smoothing happens at
        // render time.
        buf.gravPot[i] = -m;
        if (m > maxAbs) maxAbs = m;
    }
    buf.normalizer.gravMax = maxAbs;
    return { positions, values: buf.gravPot, count, normalizer: maxAbs };
}

export function buildQuantumOverlayData(ctx, state, sampled) {
    const frame = {};
    if (state.fieldFlags.showPsiSquared) {
        frame.psiSquared = computePsiSquaredFrame(sampled, state, state.fieldFlags.showDualSubstrate);
    }
    if (state.fieldFlags.showPhase) {
        frame.phase = computePhaseFrame(sampled, state, state.dualLVecs, state.dualRVecs);
    }
    if (state.fieldFlags.showLagrangianDensity) {
        frame.lagrangianDensity = computeLagrangianDensityFrame(sampled, state);
    }
    if (state.fieldFlags.showEntropyDensity) {
        frame.entropyDensity = computeEntropyDensityFrame(sampled, state);
    }
    if (state.fieldFlags.showGravPotential) {
        frame.gravPotential = computeGravPotentialFrame(ctx, sampled, state);
    }
    if (state.fieldFlags.showEmEnergy) {
        frame.emEnergy = computeEmEnergyFrame(sampled, state);
    }
    if (state.fieldFlags.showChargeDensity) {
        frame.chargeDensity = computeChargeDensityFrame(sampled, state);
    }
    if (state.fieldFlags.showVorticity) {
        frame.vorticity = computeVorticityFrame(sampled, state);
    }
    // Tier 1/2/3 (2026-04-18)
    if (state.fieldFlags.showHelicity) {
        frame.helicity = computeHelicityFrame(sampled, state);
    }
    if (state.fieldFlags.showKretschmann) {
        frame.kretschmann = computeKretschmannFrame(sampled, state);
    }
    if (state.fieldFlags.showHorizon) {
        frame.horizon = computeHorizonFrame(sampled, state);
    }
    if (state.fieldFlags.showEPressure) {
        frame.ePressure = computeEPressureFrame(sampled, state);
    }
    if (state.fieldFlags.showBPressure) {
        frame.bPressure = computeBPressureFrame(sampled, state);
    }
    if (state.fieldFlags.showKineticEnergy) {
        frame.kineticEnergy = computeKineticEnergyFrame(ctx, state);
    }
    if (state.fieldFlags.showFisher) {
        frame.fisher = computeFisherFrame(sampled, state);
    }
    if (state.fieldFlags.showCoherence) {
        frame.coherence = computeCoherenceFrame(sampled, state);
    }
    return frame;
}

// ══════════════════════════════════════════════════════════════════════
// Physics-topology overlays — Maxwell energy, charge, vorticity.
// All three are rubber-sheet scalars; they go flat in vacuum/stillness
// and deform only where the underlying field has structure.
// ══════════════════════════════════════════════════════════════════════

/**
 * EM energy density u(x) = ½(|E|² + |B|²).
 * Classical Maxwell energy density in natural units (ε₀ = μ₀ = 1).
 * When only one of E/B has samples we fall back to that one; both absent
 * means there is no field to render and we return null.
 */
function computeEmEnergyFrame(sampled, state) {
    const eF = sampled.eField;
    const bF = sampled.bField;
    // Prefer the buffer with more samples as the position reference so the
    // rubber sheet follows whichever field is actually populated.
    const ref = (eF && eF.count) ? eF : bF;
    if (!ref || !ref.count) return null;
    const buf = ensureTier1Buffers(state, ref.count);
    const { positions, count } = ref;
    const eVec = eF?.vectors, eCount = eF?.count || 0;
    const bVec = bF?.vectors, bCount = bF?.count || 0;
    let max = 0;
    for (let i = 0; i < count; i++) {
        let e2 = 0, b2 = 0;
        if (i < eCount && eVec) {
            const x = eVec[i * 3], y = eVec[i * 3 + 1], z = eVec[i * 3 + 2];
            e2 = x * x + y * y + z * z;
        }
        if (i < bCount && bVec) {
            const x = bVec[i * 3], y = bVec[i * 3 + 1], z = bVec[i * 3 + 2];
            b2 = x * x + y * y + z * z;
        }
        const u = 0.5 * (e2 + b2);
        buf.emEnergy[i] = u;
        if (u > max) max = u;
    }
    buf.normalizer.emMax = max;
    return { positions, values: buf.emEnergy, count, normalizer: max, signed: false };
}

/**
 * Charge density ρ(x) = ∇·J.
 * FTD-native: the flux-field divergence IS the source/sink density that
 * drives Gauss. Already sampled as `divJ`; we just forward the buffer with
 * a signed-surface hint so the viewport renders sources as hills and
 * sinks as wells.
 */
function computeChargeDensityFrame(sampled, _state) {
    const d = sampled.divergence;
    if (!d || !d.count) return null;
    let maxAbs = 0;
    for (let i = 0; i < d.count; i++) {
        const a = Math.abs(d.values[i]);
        if (a > maxAbs) maxAbs = a;
    }
    return {
        positions: d.positions,
        values:    d.values,
        count:     d.count,
        normalizer: maxAbs,
        signed:    true,
    };
}

/**
 * Vorticity |ω|(x) = |∇×J|.
 * Engine-computed (see bridge getVorticitySampled). Curl-free fields
 * (purely radial flux, uniform flow) stay flat — swirl structures,
 * vortex rings, and rotational solitons rise as peaks.
 */
function computeVorticityFrame(sampled, _state) {
    const v = sampled.vorticity;
    if (!v || !v.count) return null;
    let max = 0;
    for (let i = 0; i < v.count; i++) if (v.values[i] > max) max = v.values[i];
    return {
        positions: v.positions,
        values:    v.values,
        count:     v.count,
        normalizer: max,
        signed:    false,
    };
}

// ══════════════════════════════════════════════════════════════════════
// Tier 1/2/3 additions (2026-04-18) — helicity, curvature, horizon,
// stress-energy split (P_E, P_B, kinetic), Fisher information, coherence.
// All share the rubber-sheet / scatter-scalar convention so they plug
// straight into _topologySheetConfigs in viewport.js (except horizon,
// which is rendered as an isosurface).
// ══════════════════════════════════════════════════════════════════════

/**
 * Helicity density h(x) = J · (∇×J).  Signed scalar (left/right-handed
 * field-line linking).  Bridge-computed — we just pass through with a
 * symmetric normalizer so positive and negative cancel around zero.
 */
function computeHelicityFrame(sampled, _state) {
    const h = sampled.helicity;
    if (!h || !h.count) return null;
    let maxAbs = 0;
    for (let i = 0; i < h.count; i++) {
        const a = Math.abs(h.values[i]);
        if (a > maxAbs) maxAbs = a;
    }
    return {
        positions: h.positions,
        values:    h.values,
        count:     h.count,
        normalizer: maxAbs,
        signed:    true,
    };
}

/**
 * Kretschmann-like curvature proxy K(x) = (∇²L)² where L(x) is the
 * latency proxy sqrt(|J|² / |J|²_max).  Always non-negative; we
 * log-compress the normalizer to keep the black-hole singularity from
 * dominating the colour ramp.  [PROXY]: see bridge metadata.
 */
function computeKretschmannFrame(sampled, state) {
    const k = sampled.kretschmann;
    if (!k || !k.count) return null;
    // A Schwarzschild-like 1/r⁶ tail means one near-horizon voxel can be
    // 10⁴× the background. If we just normalize by max, 99% of the rubber
    // sheet collapses to ≈ 0 and the curvature structure vanishes into
    // vacuum. Log-compress values and normalizer in lockstep — preserves
    // the monotonic ordering and zero→zero, but compresses the tail so
    // the background still reads above the baseline.
    if (!state.kretschmannValues || state.kretschmannValues.length < k.count) {
        state.kretschmannValues = new Float32Array(k.count);
    }
    const values = state.kretschmannValues;
    let max = 0;
    for (let i = 0; i < k.count; i++) {
        const v = Math.log1p(k.values[i]);
        values[i] = v;
        if (v > max) max = v;
    }
    return {
        positions: k.positions,
        values,
        count:     k.count,
        normalizer: max,
        signed:    false,
    };
}

/**
 * Event-horizon overlay — points where latency proxy L(x) ≥ 0.95.
 * Below that, the well is sub-horizon (light still escapes).  We emit
 * positions + values so the viewport can either render them as an
 * isosurface (preferred) or fall back to a point cloud.
 */
function computeHorizonFrame(sampled, state) {
    const L = sampled.latency;
    if (!L || !L.count) return null;
    const threshold = 0.95;
    // Grow-only state-cached buffers — sized to the full sampler capacity
    // (post-filter count is usually tiny, but the upper bound equals L.count
    // at the moment of capture; pre-allocating avoids per-frame GC).
    if (!state.horizonPositions || state.horizonPositions.length < L.count * 3) {
        state.horizonPositions = new Float32Array(L.count * 3);
        state.horizonValues    = new Float32Array(L.count);
    }
    const positions = state.horizonPositions;
    const values    = state.horizonValues;
    let count = 0;
    for (let i = 0; i < L.count; i++) {
        if (L.values[i] >= threshold) {
            positions[count * 3]     = L.positions[i * 3];
            positions[count * 3 + 1] = L.positions[i * 3 + 1];
            positions[count * 3 + 2] = L.positions[i * 3 + 2];
            values[count] = L.values[i];
            count++;
        }
    }
    if (count === 0) return null;
    return { positions, values, count, threshold };
}

/**
 * Electric pressure P_E(x) = ½|E|².  Half of the EM energy density;
 * rises on charge concentrations (fields terminating on particles).
 */
function computeEPressureFrame(sampled, state) {
    const eF = sampled.eField;
    if (!eF || !eF.count) return null;
    if (!state.ePressureValues || state.ePressureValues.length < eF.count) {
        state.ePressureValues = new Float32Array(eF.count);
    }
    const values = state.ePressureValues;
    let max = 0;
    for (let i = 0; i < eF.count; i++) {
        const x = eF.vectors[i * 3];
        const y = eF.vectors[i * 3 + 1];
        const z = eF.vectors[i * 3 + 2];
        const p = 0.5 * (x * x + y * y + z * z);
        values[i] = p;
        if (p > max) max = p;
    }
    return {
        positions: eF.positions,
        values,
        count:     eF.count,
        normalizer: max,
        signed:    false,
    };
}

/**
 * Magnetic pressure P_B(x) = ½|B|².  Sister field to P_E — rises on
 * circulating currents and field-line loops rather than on charges.
 */
function computeBPressureFrame(sampled, state) {
    const bF = sampled.bField;
    if (!bF || !bF.count) return null;
    if (!state.bPressureValues || state.bPressureValues.length < bF.count) {
        state.bPressureValues = new Float32Array(bF.count);
    }
    const values = state.bPressureValues;
    let max = 0;
    for (let i = 0; i < bF.count; i++) {
        const x = bF.vectors[i * 3];
        const y = bF.vectors[i * 3 + 1];
        const z = bF.vectors[i * 3 + 2];
        const p = 0.5 * (x * x + y * y + z * z);
        values[i] = p;
        if (p > max) max = p;
    }
    return {
        positions: bF.positions,
        values,
        count:     bF.count,
        normalizer: max,
        signed:    false,
    };
}

/**
 * Kinetic energy density K(x) = ½|v|² sampled at particle positions.
 * This is a particle-anchored scalar (not a voxel-grid scalar), so we
 * emit it as a point scatter — the viewport drops a small volumetric
 * bump at each particle's location proportional to K.
 */
function computeKineticEnergyFrame(ctx, state) {
    const particleCap = ctx.bridge?.capabilities?.scale0;
    if (!particleCap || typeof particleCap.getScale0ParticleFrame !== 'function') return null;
    const frame = particleCap.getScale0ParticleFrame();
    if (!frame || !frame.count) return null;
    // Velocities live on the particle frame under `velocities` when the
    // bridge exposes them; some mocks only expose positions, in which case
    // we fall back to 0 (no bumps). Reuse/resize buffers on the state.
    const n = frame.count;
    if (!state.kineticValues || state.kineticValues.length < n) {
        state.kineticValues = new Float32Array(n);
    }
    if (!state.kineticPositions || state.kineticPositions.length < n * 3) {
        state.kineticPositions = new Float32Array(n * 3);
    }
    const vels = frame.velocities;
    let max = 0;
    for (let i = 0; i < n; i++) {
        state.kineticPositions[i * 3]     = frame.positions[i * 3];
        state.kineticPositions[i * 3 + 1] = frame.positions[i * 3 + 1];
        state.kineticPositions[i * 3 + 2] = frame.positions[i * 3 + 2];
        let k = 0;
        if (vels && vels.length >= (i + 1) * 3) {
            const vx = vels[i * 3], vy = vels[i * 3 + 1], vz = vels[i * 3 + 2];
            k = 0.5 * (vx * vx + vy * vy + vz * vz);
        }
        state.kineticValues[i] = k;
        if (k > max) max = k;
    }
    return {
        positions: state.kineticPositions,
        values:    state.kineticValues,
        count:     n,
        normalizer: max,
        signed:    false,
    };
}

/**
 * Fisher information F(x) = |∇ρ|² / ρ with ρ = |J|².
 * Bridge-computed (needs neighbour lookup). Brightens sharp edges of
 * localized field modes — soliton shells, wave-packet envelopes.
 */
function computeFisherFrame(sampled, state) {
    const f = sampled.fisher;
    if (!f || !f.count) return null;
    // Same tail-compression rationale as Kretschmann: Fisher information
    // diverges at the edge of a compact-support mode, so log1p keeps the
    // sheet readable instead of dominated by a single spike.
    if (!state.fisherValues || state.fisherValues.length < f.count) {
        state.fisherValues = new Float32Array(f.count);
    }
    const values = state.fisherValues;
    let max = 0;
    for (let i = 0; i < f.count; i++) {
        const v = Math.log1p(f.values[i]);
        values[i] = v;
        if (v > max) max = v;
    }
    return {
        positions: f.positions,
        values,
        count:     f.count,
        normalizer: max,
        signed:    false,
    };
}

/**
 * Dual-substrate coherence C(x) = (J·∇×J) / (|J|·|∇×J|) in [-1, +1].
 * Cosine of the angle between flow and curl. +1 = right-handed
 * Beltrami, -1 = left-handed, 0 = orthogonal (purely rotational or
 * purely translational, no helicity). Bridge-computed; pass-through.
 */
function computeCoherenceFrame(sampled, _state) {
    const c = sampled.coherence;
    if (!c || !c.count) return null;
    return {
        positions: c.positions,
        values:    c.values,
        count:     c.count,
        normalizer: 1,  // already in [-1, 1]
        signed:    true,
    };
}

function fillFieldParticleBuf(state, particleData) {
    while (state.fieldParticleBuf.length < particleData.count) {
        state.fieldParticleBuf.push({ x: 0, y: 0, z: 0 });
    }
    state.fieldParticleBuf.length = particleData.count;
    for (let i = 0; i < particleData.count; i++) {
        state.fieldParticleBuf[i].x = particleData.positions[i * 3];
        state.fieldParticleBuf[i].y = particleData.positions[i * 3 + 1];
        state.fieldParticleBuf[i].z = particleData.positions[i * 3 + 2];
    }
}

export function buildElectromagneticOverlayData(ctx, state, sampled, latticeSize, stride, stepsScale, seedSpacing, params = {}) {
    const frame = {};
    const {
        stepSize = 0.5,
        maxSteps = 100,
        maxSeeds = 150,
        maxLines = 200,
        eOffset = 2,
        bRadius = 4,
    } = params;

    if (state.fieldFlags.showEField && sampled.eField?.count > 0) {
        // E-field: lines start on positive charges and terminate on negative ones.
        // When particles exist we anchor seeds to them (real sources); otherwise
        // we importance-sample from |E| so seeds cluster where the field is strong
        // (iron-filings effect). Bidirectional integration draws from each seed
        // both toward the source and toward the sink, so the visible line spans
        // the natural field-line path.
        const particleData = ctx.bridge.capabilities.scale0.getScale0ParticleFrame();
        fillFieldParticleBuf(state, particleData);
        const seeds = particleData.count > 0
            ? generateEFieldSeeds(state.fieldParticleBuf, eOffset, maxSeeds)
            : generateImportanceSeeds(sampled.eField, maxSeeds);
        frame.eFieldLines = computeStreamlines(sampled.eField, seeds, {
            N: latticeSize, stride, maxSteps, stepSize, maxLines, bidirectional: true,
        });
    }

    if (state.fieldFlags.showBField && sampled.bField?.count > 0) {
        // B-field is divergence-free (∇·B=0), so lines must form closed loops.
        // Anchor seeds to particles when present, else importance-sample with a
        // perpendicular offset so seeds land on the loop circumference rather
        // than at the center (where they'd integrate in place). Bidirectional
        // integration is mandatory — half the loop runs each direction.
        const particleData = ctx.bridge.capabilities.scale0.getScale0ParticleFrame();
        fillFieldParticleBuf(state, particleData);
        const seeds = particleData.count > 0
            ? generateBFieldSeeds(state.fieldParticleBuf, bRadius, maxSeeds)
            : generateBImportanceSeeds(sampled.bField, maxSeeds, bRadius);
        frame.bFieldLines = computeStreamlines(sampled.bField, seeds, {
            N: latticeSize, stride,
            // Loops need ~ 2·π·radius worth of steps to close — give B 1.5× the
            // baseline so a typical loop completes inside the integration budget.
            maxSteps: Math.ceil(maxSteps * 1.5),
            stepSize, bidirectional: true, maxLines,
        });
    }

    if (state.fieldFlags.showPoynting && sampled.poynting?.count > 0) {
        frame.poynting = sampled.poynting;
    }

    if (state.fieldFlags.showDivField && sampled.divergence?.count > 0) {
        frame.divergence = sampled.divergence;
    }

    if (state.fieldFlags.showFluxLines && sampled.fluxVector?.count > 0) {
        // Flux ∇·J carries divergence (sources/sinks), same topology as E.
        // Importance-sample by |J| so streamlines cluster on flux concentrations.
        const seeds = generateImportanceSeeds(sampled.fluxVector, maxSeeds);
        const lines = computeStreamlines(sampled.fluxVector, seeds, {
            N: latticeSize, stride, maxSteps, stepSize, maxLines, bidirectional: true,
        });
        let maxFlux = 0;
        for (let i = 0; i < sampled.fluxVector.count; i++) {
            const x = sampled.fluxVector.vectors[i * 3];
            const y = sampled.fluxVector.vectors[i * 3 + 1];
            const z = sampled.fluxVector.vectors[i * 3 + 2];
            const mag = Math.sqrt(x * x + y * y + z * z);
            if (mag > maxFlux) maxFlux = mag;
        }
        frame.fluxStreamlines = { lines, maxFlux };
    }

    return frame;
}

export function buildForceOverlayData(state, fieldCapability, sampled, latticeSize, stride, stepsScale, seedSpacing, params = {}) {
    const anyForceOn = state.fieldFlags.showForceEM || state.fieldFlags.showForceGravity ||
        state.fieldFlags.showForceStrong || state.fieldFlags.showForceWeak;
    if (!anyForceOn) return { anyForceOn: false, style: state.forceStyle, items: [] };

    // Force samplers need a finer stride than field samplers because particle-
    // anchored physics (Coulomb + flux tubes + nuclear) is sharply peaked at
    // voxel centres. With stride=2, the sampler hits only EVEN voxels, so a
    // Moore-cell scenario anchored at mc=16 (even) captures the centre but
    // SKIPS the neighbour particles at voxels 15 and 17 (odd). The resulting
    // arrow pattern looks off-centre because the tube envelopes between
    // adjacent particles — the most intense region — have zero samples. Drop
    // to stride=1 at small lattices so every voxel is caught regardless of
    // the particle-parity pattern; keep the field stride where it is so E/B
    // streamlines stay cheap.
    const forceStride = latticeSize <= 32 ? 1 : Math.max(1, Math.min(4, Math.floor(stride / 2) || 1));

    const items = [];
    if (state.fieldFlags.showForceEM) {
        const emData = fieldCapability.getScale0ForceField('em', forceStride);
        if (emData.count > 0) items.push({ type: 'em', data: emData });
    }
    if (state.fieldFlags.showForceGravity) {
        const gravityData = fieldCapability.getScale0ForceField('gravity', forceStride);
        if (gravityData.count > 0) items.push({ type: 'gravity', data: gravityData });
    }
    if (state.fieldFlags.showForceStrong) {
        const strongData = fieldCapability.getScale0ForceField('strong', forceStride);
        if (strongData.count > 0) items.push({ type: 'strong', data: strongData });
    }
    if (state.fieldFlags.showForceWeak && sampled.curlJ?.count > 0) {
        // Weak force direction = ∇×J (pseudovector). The weak interaction
        // is parity-violating, so its natural vector proxy is parity-odd.
        // Using the flux vector J directly (the old implementation) made
        // every arrow point in the flux direction — for any polarised
        // scenario that meant uniform unidirectional arrows (e.g. a flux
        // pulse with J = (Gaussian, 0, 0) produced every weak arrow along
        // +X), physically misleading.
        //
        // The curl is zero for irrotational (purely compressive) flow and
        // non-zero wherever J has rotational structure — exactly where
        // chirality asymmetry lives. Magnitude is still scaled by
        // DUAL_DELTA so the overlay reads as "weak" (small relative to
        // EM/strong) in comparison rendering.
        const curl = sampled.curlJ;
        const scalarFactor = DUAL_DELTA;
        if (!state.weakValues || state.weakValues.length < curl.count) {
            state.weakValues = new Float32Array(curl.count);
        }
        if (!state.weakVectors || state.weakVectors.length < curl.count * 3) {
            state.weakVectors = new Float32Array(curl.count * 3);
        }
        for (let i = 0; i < curl.count; i++) {
            const x = curl.vectors[i * 3];
            const y = curl.vectors[i * 3 + 1];
            const z = curl.vectors[i * 3 + 2];
            const mag = Math.sqrt(x * x + y * y + z * z);
            state.weakValues[i] = mag * scalarFactor;
            state.weakVectors[i * 3]     = x * scalarFactor;
            state.weakVectors[i * 3 + 1] = y * scalarFactor;
            state.weakVectors[i * 3 + 2] = z * scalarFactor;
        }
        items.push({
            type: 'weak',
            data: { positions: curl.positions, vectors: state.weakVectors, count: curl.count },
            weakScalar: { positions: curl.positions, values: state.weakValues, count: curl.count },
        });
    }

    if (state.forceStyle === 'flow') {
        const { stepSize = 0.5, maxSteps = 100, maxSeeds = 150, maxLines = 200 } = params;
        // Force flow lines stay shorter than EM streamlines (≈ 40% of full length)
        // so the field-arrow visualization stays visually distinct from B/E lines.
        const flowMaxSteps = Math.max(20, Math.ceil(maxSteps * 0.4));
        for (const item of items) {
            // Weak is the flux-vector field itself (chirality transmutation
            // follows flux flow); give it denser coverage and longer lines so
            // it reads as a coherent field instead of a sparse cluster.
            const isWeak = item.type === 'weak';
            const seedCount = isWeak ? Math.min(maxSeeds * 2, 320) : maxSeeds;
            const stepCount = isWeak ? maxSteps : flowMaxSteps;
            const lineCount = isWeak ? Math.min(maxLines * 2, 400) : maxLines;
            // Importance-sample by |force| so streamlines cluster where the
            // interaction is strongest (e.g., near charges for EM, near masses
            // for gravity), matching the iron-filing visualization metaphor.
            const seeds = generateImportanceSeeds(item.data, seedCount);
            item.flowLines = computeStreamlines(item.data, seeds, {
                N: latticeSize, stride, maxSteps: stepCount, stepSize,
                maxLines: lineCount, bidirectional: true,
            });
        }
    }

    return { anyForceOn, style: state.forceStyle, items };
}

export function buildDerivedSubstrateData(state, sampled, mockCapability) {
    const frame = {};
    if (state.fieldFlags.showDarkMatterHalo) {
        frame.darkMatterHalo = mockCapability?.getScale0DerivedOverlayData('darkMatterHalo') || null;
    }
    if (state.fieldFlags.showDampingZones) {
        frame.dampingZones = mockCapability?.getScale0DerivedOverlayData('dampingZones') || null;
    }
    if (state.fieldFlags.showGenesisIsosurface) {
        frame.genesisIsosurface = mockCapability?.getScale0DerivedOverlayData('genesisIsosurface') || null;
    }

    if (state.fieldFlags.showDualSubstrate && sampled.fluxVector?.count > 0) {
        // [TIER-1 VISUAL] Scalar (1+/-delta)/2 decomposition is an amplitude
        // asymmetry demonstration, NOT a true chirality projection. A real
        // L/R decomposition requires a pseudovector operation (Helmholtz-
        // style split into curl-free and divergence-free parts). Surfaced
        // as "dual substrate" for visualization only.
        const leftFactor = (1 + DUAL_DELTA) / 2;
        const rightFactor = (1 - DUAL_DELTA) / 2;
        const vecLen = sampled.fluxVector.vectors.length;
        if (!state.dualLVecs || state.dualLVecs.length < vecLen) {
            state.dualLVecs = new Float32Array(vecLen);
            state.dualRVecs = new Float32Array(vecLen);
        }
        for (let i = 0; i < vecLen; i++) {
            state.dualLVecs[i] = sampled.fluxVector.vectors[i] * leftFactor;
            state.dualRVecs[i] = sampled.fluxVector.vectors[i] * rightFactor;
        }
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

    if (state.fieldFlags.showLight && sampled.poynting?.count > 0) {
        frame.light = sampled.poynting;
    }

    return frame;
}

export function applyOverlayFrame(viewportAdapter, overlayFrame, forceFrame, opts = {}) {
    if (overlayFrame.eFieldLines) viewportAdapter.applyEFieldLines(overlayFrame.eFieldLines);
    if (overlayFrame.bFieldLines) viewportAdapter.applyBFieldLines(overlayFrame.bFieldLines);
    if (overlayFrame.poynting) viewportAdapter.applyPoynting(overlayFrame.poynting);
    if (overlayFrame.divergence) viewportAdapter.applyDivergence(overlayFrame.divergence);
    if (overlayFrame.fluxStreamlines) {
        viewportAdapter.applyFluxStreamlines(overlayFrame.fluxStreamlines.lines, overlayFrame.fluxStreamlines.maxFlux);
    }

    if (forceFrame.anyForceOn) {
        if (forceFrame.style === 'arrows') {
            for (const item of forceFrame.items) {
                viewportAdapter.applyForceArrowField(item.type, item.type === 'weak' ? item.weakScalar : item.data);
            }
        } else if (forceFrame.style === 'heatmap') {
            for (const item of forceFrame.items) viewportAdapter.applyForceHeatmap(item.data, item.type);
        } else if (forceFrame.style === 'flow') {
            for (const item of forceFrame.items) viewportAdapter.applyForceStreamlines(item.flowLines, item.type);
            // Advance the dash-offset animation ONLY when the sim is actually
            // running. Previously this ticked on every overlay refresh — and
            // since toggling any overlay dirties `fieldNeedsUpdate`, the
            // forced refresh would bump the dash phase by 0.032 units even
            // while the sim was paused. The user perceived that as "toggling
            // an overlay plays one animation step", exactly the regression
            // they reported. Gated by `opts.running` supplied from the
            // controller's animate() loop (ctx.running).
            if (opts.running) viewportAdapter.animateForceStreamlines(0.016);
        } else if (forceFrame.style === 'glyphs') {
            for (const item of forceFrame.items) viewportAdapter.applyForceGlyphs(item.data, item.type);
        }
    }

    if (overlayFrame.darkMatterHalo) viewportAdapter.applyDarkMatterHalo(overlayFrame.darkMatterHalo);
    if (overlayFrame.dampingZones) viewportAdapter.applyDampingZones(overlayFrame.dampingZones);
    if (overlayFrame.genesisIsosurface) viewportAdapter.applyGenesisIsosurface(overlayFrame.genesisIsosurface);
    if (overlayFrame.dualFlux) viewportAdapter.applyDualFlux(overlayFrame.dualFlux.left, overlayFrame.dualFlux.right);
    if (overlayFrame.chirality) viewportAdapter.applyChirality(overlayFrame.chirality);
    if (overlayFrame.light) viewportAdapter.applyLight(overlayFrame.light);

    // Tier 1 quantum overlays
    if (overlayFrame.psiSquared) viewportAdapter.applyPsiSquared(overlayFrame.psiSquared);
    if (overlayFrame.phase) viewportAdapter.applyPhase(overlayFrame.phase);
    if (overlayFrame.lagrangianDensity) viewportAdapter.applyLagrangianDensity(overlayFrame.lagrangianDensity);
    if (overlayFrame.entropyDensity) viewportAdapter.applyEntropyDensity(overlayFrame.entropyDensity);
    if (overlayFrame.gravPotential) viewportAdapter.applyGravPotential(overlayFrame.gravPotential);

    // Physics-topology overlays (rubber-sheet surfaces)
    if (overlayFrame.emEnergy) viewportAdapter.applyEmEnergy(overlayFrame.emEnergy);
    if (overlayFrame.chargeDensity) viewportAdapter.applyChargeDensity(overlayFrame.chargeDensity);
    if (overlayFrame.vorticity) viewportAdapter.applyVorticity(overlayFrame.vorticity);

    // Tier 1/2/3 additions (2026-04-18)
    if (overlayFrame.helicity)      viewportAdapter.applyHelicity(overlayFrame.helicity);
    if (overlayFrame.kretschmann)   viewportAdapter.applyKretschmann(overlayFrame.kretschmann);
    if (overlayFrame.horizon)       viewportAdapter.applyHorizon(overlayFrame.horizon);
    if (overlayFrame.ePressure)     viewportAdapter.applyEPressure(overlayFrame.ePressure);
    if (overlayFrame.bPressure)     viewportAdapter.applyBPressure(overlayFrame.bPressure);
    if (overlayFrame.kineticEnergy) viewportAdapter.applyKineticEnergy(overlayFrame.kineticEnergy);
    if (overlayFrame.fisher)        viewportAdapter.applyFisher(overlayFrame.fisher);
    if (overlayFrame.coherence)     viewportAdapter.applyCoherence(overlayFrame.coherence);
}

// ── Lattice-size-aware streamline parameters ────────────────────────────
// All E/B/Flux streamline knobs scale with the lattice size N so visual
// density stays roughly constant from N=8 to N=128.
//
//   stride       — grow ~ N/16 (clamped 2..8): keeps sample count near constant
//   stepSize     — fixed at 0.5 voxels until N>96, then grows with N so
//                  maxSteps stays bounded (preserves curvature accuracy)
//   maxSteps     — sized so a streamline can travel ~1.5× the lattice diameter
//   seedSpacing  — ~ N/10 (clamped 3..10): finer at small N, coarser at large N
//   maxSeeds     — grows ~ N²/28, capped at 250: more seeds when bigger
//   maxLines     — grows with maxSeeds (drawn count cap inside fieldlines.js)
function computeStreamlineParams(latticeSize) {
    const stride = Math.max(2, Math.min(8, Math.round(latticeSize / 16)));
    const seedSpacing = Math.max(3, Math.min(10, Math.round(latticeSize / 10)));
    const maxSeeds = Math.max(60, Math.min(250, Math.round((latticeSize * latticeSize) / 28)));
    const stepSize = Math.max(0.5, latticeSize / 96);
    const targetLen = latticeSize * 1.5;
    const maxSteps = Math.max(40, Math.ceil(targetLen / stepSize));
    const maxLines = Math.max(120, Math.min(300, maxSeeds + 50));
    // Particle-anchored seed offsets scale gently with N so seeds at large N
    // don't sit right on top of the source.
    const eOffset = Math.max(2, Math.round(latticeSize / 24));
    const bRadius = Math.max(3, Math.round(latticeSize / 12));
    const stepsScale = Math.ceil(latticeSize / 32); // legacy alias for callers
    return { stride, seedSpacing, maxSeeds, stepSize, maxSteps, maxLines, eOffset, bRadius, stepsScale };
}

export function updateFieldOverlays(ctx, state, viewportAdapter) {
    state.fieldFrame += 1;
    const latticeSize = ctx.bridge.latticeSize || 32;
    const fieldThrottle = latticeSize > 96 ? 12 : (latticeSize > 48 ? 6 : 3);
    if (!state.anyFieldActive || (!state.fieldNeedsUpdate && state.fieldFrame % fieldThrottle !== 0)) return;
    // Global pause freezes the visualization re-compute loop — no re-sampling,
    // no streamline recompute, no random importance-seed reshuffle. We DO allow
    // a single one-shot update when `fieldNeedsUpdate` is set (e.g. user just
    // toggled an overlay during global pause and we need to draw one frame of
    // the frozen state). The `fieldNeedsUpdate = false` reset below ensures
    // we don't keep re-drawing every frame after that one shot.
    //
    // Without this guard B-field would visibly "shimmer" against frozen flux —
    // importance sampling picks fresh random seeds every frame, so even though
    // the underlying physics is paused the streamline geometry would jitter.
    if (!ctx.running && !state.fieldNeedsUpdate) return;

    state.fieldNeedsUpdate = false;
    const fieldCapability = (state.useFluxMock ? state.fluxMock : ctx.bridge).capabilities.scale0;
    const mockCapability = state.fluxMock?.capabilities?.scale0 || null;
    const params = computeStreamlineParams(latticeSize);
    const { stride, seedSpacing, stepsScale } = params;

    const sampled = sampleFieldState(fieldCapability, state.fieldFlags, stride);
    const overlayFrame = buildElectromagneticOverlayData(ctx, state, sampled, latticeSize, stride, stepsScale, seedSpacing, params);
    const forceFrame = buildForceOverlayData(state, fieldCapability, sampled, latticeSize, stride, stepsScale, seedSpacing, params);
    Object.assign(overlayFrame, buildDerivedSubstrateData(state, sampled, mockCapability));
    Object.assign(overlayFrame, buildQuantumOverlayData(ctx, state, sampled));
    // Pass `running` through to applyOverlayFrame so time-based sub-animations
    // (force-streamline dash advance) freeze when the sim is paused, even when
    // this function itself runs for a one-shot refresh on toggle.
    applyOverlayFrame(viewportAdapter, overlayFrame, forceFrame, { running: !!ctx.running });
}
