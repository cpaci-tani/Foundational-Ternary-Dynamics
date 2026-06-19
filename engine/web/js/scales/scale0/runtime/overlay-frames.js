// ══════════════════════════════════════════════════════════════════════
// Overlay frame builders — one pure function per topology overlay.
//
// Each `compute*Frame(sampled, state, ...)` transforms the raw sampled
// field buffers plus per-controller scratch state into a Float32Array-
// backed frame object that the viewport consumes. All buffers are
// grown-on-demand and owned by `state` so successive frames reuse the
// same allocations.
//
// Extracted from field-overlays.js (FO-1) without behavioural change —
// exact textual copies of the original private helpers.
// ══════════════════════════════════════════════════════════════════════

import { getActiveScale0Bridge } from '../state/store.js';

export function ensureTier1Buffers(state, N) {
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

export function computePsiSquaredFrame(sampled, state, dualActive) {
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

export function computePhaseFrame(sampled, state, dualLVecs, dualRVecs) {
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

export function computeLagrangianDensityFrame(sampled, state) {
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

export function computeEntropyDensityFrame(sampled, state) {
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

export function computeGravPotentialFrame(ctx, sampled, state) {
    // If the bridge already exposes a gravitational potential field, prefer
    // that. Otherwise we approximate Φ(x) by a smoothed |J|² mass density:
    // true Φ satisfies ∇²Φ = 4πGρ, and a Gaussian smoothing of ρ is the
    // lowest-pass-filter analogue at fixed resolution — good enough to show
    // wells and peaks qualitatively.
    if (!sampled.fluxVector?.count) return null;
    const activeBridge = getActiveScale0Bridge(ctx, state) ?? ctx.bridge;
    if (typeof activeBridge?.getGravPotentialSamples === 'function') {
        const data = activeBridge.getGravPotentialSamples();
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
export function computeEmEnergyFrame(sampled, state) {
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
export function computeChargeDensityFrame(sampled, _state) {
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
export function computeVorticityFrame(sampled, _state) {
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
 * Event-horizon overlay — points where latency proxy L(x) ≥ 0.95.
 * Below that, the well is sub-horizon (light still escapes).  We emit
 * positions + values so the viewport can either render them as an
 * isosurface (preferred) or fall back to a point cloud.
 */
export function computeHorizonFrame(sampled, state) {
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
export function computeEPressureFrame(sampled, state) {
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
export function computeBPressureFrame(sampled, state) {
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

// ══════════════════════════════════════════════════════════════════════
// New substrate overlays (2026-06-03) — state field, latency, Gauss residual.
// All three are pass-throughs of an engine sampler; the colouring lives in
// the dedicated renderers (ternary for state, blue→red ramp for latency,
// signed for the Gauss residual).
// ══════════════════════════════════════════════════════════════════════

/**
 * Ternary state field s(x) ∈ {-1,0,+1} — the manifestation layer (Postulate
 * 3). Pass-through of the engine's state sampler; the void (s=0) is already
 * excluded by the sampler, so this just forwards the manifested voxels.
 */
export function computeStateFieldFrame(sampled, _state) {
    const s = sampled.state;
    if (!s || !s.count) return null;
    return { positions: s.positions, values: s.values, count: s.count };
}

/**
 * Latency / time-dilation field L(x) = √(|J|²/|J|²_max) ∈ [0, 0.998]. The
 * Born-Infeld proper-time field that creates gravity wells, event horizons,
 * and time dilation (f = 1 − L²). Pass-through; rendered as a blue→red
 * volumetric point cloud.
 */
export function computeLatencyFrame(sampled, _state) {
    const L = sampled.latency;
    if (!L || !L.count) return null;
    return { positions: L.positions, values: L.values, count: L.count, normalizer: 1, signed: false };
}

/**
 * Gauss-constraint residual r(x) = ∇·J − s_charge. FTD-native charge is the
 * ternary state, so a clean substrate would have r ≈ 0; non-zero r maps the
 * non-variational Gauss-projection conservation leak (SPEC_ENGINE.md).
 * Signed pass-through of the engine's gauss-residual sampler.
 */
export function computeGaussResidualFrame(sampled, _state) {
    const g = sampled.gaussResidual;
    if (!g || !g.count) return null;
    let maxAbs = 0;
    for (let i = 0; i < g.count; i++) { const a = Math.abs(g.values[i]); if (a > maxAbs) maxAbs = a; }
    return { positions: g.positions, values: g.values, count: g.count, normalizer: maxAbs, signed: true };
}
