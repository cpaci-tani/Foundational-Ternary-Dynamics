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
import { C_SPEED, G_N } from '../../../constants.js';
import { posKey } from './manifestation-flash.js';

function sampleGridMetadata(sample) {
    const meta = {};
    if (Number.isInteger(sample?.effectiveStride) && sample.effectiveStride > 0) {
        meta.effectiveStride = sample.effectiveStride;
    }
    if (Number.isInteger(sample?.origin) && sample.origin >= 0) {
        meta.origin = sample.origin;
    }
    return meta;
}

// Peak-hold-with-decay normalizer (audit fix — dynamical accuracy). Several
// overlays below used to normalize by THIS FRAME's own instant max, which
// stretches a trivial field and an extreme field into the identical color/
// height range and hides whether the underlying field is growing or decaying.
// This tracks a VU-meter-style running peak per overlay instead: fast attack
// (jumps immediately to a new instant max), slow release (decays
// geometrically when the instant max isn't re-hit), so a decaying field
// visibly fades over ~seconds instead of snapping back to full saturation
// every frame. `key` must be unique per overlay so decay histories don't leak
// into each other; state is the same per-controller scratch object every
// compute*Frame already threads through.
export function updateDecayingMax(state, key, instantMax, decay = 0.985) {
    if (!state.decayingMax) state.decayingMax = {};
    const prev = state.decayingMax[key] ?? 0;
    const next = Math.max(instantMax, prev * decay);
    state.decayingMax[key] = next;
    return next;
}

export function ensureTier1Buffers(state, N) {
    // Grow-only: co-active Tier-1 overlays pass DIFFERENT N (E/B/J have
    // independent per-field magnitude floors), so an exact-match guard thrashed
    // ~8 reallocations 2–3× per overlay sweep. `size` is a capacity; each caller
    // writes only its own `count` prefix, so a larger buffer is safe to reuse.
    if (!state.t1 || state.t1.size < N) {
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
    return { positions, values: buf.psi2, count, normalizer: max, dualActive, ...sampleGridMetadata(sampled.fluxVector) };
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
    return { positions, values: buf.phase, count, dualAvailable: hasDual, ...sampleGridMetadata(sampled.fluxVector) };
}

export function computeLagrangianDensityFrame(sampled, state) {
    // [TIER-1 VISUAL] Per-voxel pedagogical stand-in ~ (1/2)|E|^2 - (1/2)(div J)^2.
    // Kinetic term uses the true field time-derivative E == -d_t J (sampled.eField),
    // NOT |J|^2 itself — J is a potential-like quantity, not the field's kinetic
    // term. NOTE: (div J)^2 != |grad J|^2 in general — the Frobenius norm of the
    // 3x3 Jacobian captures shear flow, while (div J)^2 only captures compressive
    // divergence, so the gradient term is also a stand-in, not exact. There is no
    // V(s,J) potential term here — the ternary state s is not sampled in this
    // function — so this is a two-term kinetic-vs-gradient balance, NOT the
    // engine's true Lagrangian density. E and divJ are sampled independently by
    // the engine with different magnitude floors (same hazard as
    // computeEmEnergyFrame's E/B pairing above), so they are paired by
    // position — NOT by raw loop index — via the shared buildPositionLookup.
    const eF = sampled.eField;
    if (!eF || !eF.count) return null;
    const buf = ensureTier1Buffers(state, eF.count);
    const { positions, count } = eF;
    const eVec = eF.vectors;
    const divF = sampled.divergence;
    const divMap = divF && divF.count ? buildPositionLookup(divF) : null;
    let maxAbs = 0;
    for (let i = 0; i < count; i++) {
        const x = eVec[i * 3];
        const y = eVec[i * 3 + 1];
        const z = eVec[i * 3 + 2];
        const kinetic = 0.5 * (x * x + y * y + z * z);
        // Use |divJ|² as a proxy for |∇J|² (not exactly equal, but a defensible
        // stand-in when we only sample the divergence-scalar field).
        let gradProxy = 0;
        if (divMap) {
            const key = posKey(positions[i * 3], positions[i * 3 + 1], positions[i * 3 + 2]);
            const di = divMap.get(key);
            if (di !== undefined) {
                const d = divF.values[di];
                gradProxy = 0.5 * d * d;
            }
        }
        const L = kinetic - gradProxy;
        buf.lagr[i] = L;
        const a = Math.abs(L);
        if (a > maxAbs) maxAbs = a;
    }
    buf.normalizer.lagMax = maxAbs;
    return { positions, values: buf.lagr, count, normalizer: maxAbs, ...sampleGridMetadata(eF) };
}

export function computeEntropyDensityFrame(sampled, state) {
    // Disorder proxy, NOT the Shannon entropy of the ternary state over a
    // Moore neighborhood. This is a pointwise function of |J| (a rank-based
    // estimator: high where |J| is near the median/disordered, low where |J|
    // is near zero or near the max) normalized by a GLOBAL |J|_max, not a
    // per-neighborhood quantity. The ternary state field is already exposed
    // at stride 1 elsewhere in this runtime (see computeStateFieldFrame /
    // the 'state' sample slot), so the blocker is implementing a real
    // neighborhood-sampling Shannon estimator, not state-field access.
    // This Tier 1 stand-in is retained until that upgrade lands.
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
    return { positions, values: buf.entropy, count, ...sampleGridMetadata(sampled.fluxVector) };
}

export function computeGravPotentialFrame(ctx, sampled, state) {
    // Prefer the actual finite Poisson-latency well whenever that engine term
    // is active.  The solver stores L=sqrt(max(-phi_latency,0)); therefore
    // -L² reconstructs its clamped negative well potential without importing a
    // continuum 1/r formula into the visual layer.
    const activeBridge = getActiveScale0Bridge(ctx, state) ?? ctx.bridge;
    const poisson = sampled.poissonLatency;
    const latencyToggle = activeBridge?.getToggle?.('latency_field');
    if (poisson?.count > 0 && latencyToggle !== false) {
        const buf = ensureTier1Buffers(state, poisson.count);
        const { positions, values, count } = poisson;
        let instantMax = 0;
        for (let i = 0; i < count; i++) {
            const well = values[i] * values[i];
            buf.gravPot[i] = -well;
            if (well > instantMax) instantMax = well;
        }
        const normalizer = updateDecayingMax(
            state, 'gravPotentialPoisson', instantMax,
        );
        buf.normalizer.gravMax = normalizer;
        return {
            positions,
            values: buf.gravPot,
            count,
            normalizer,
            source: 'poisson-latency',
            operator: 'phi=-L^2',
            ...sampleGridMetadata(poisson),
        };
    }

    // The default selected Scale-0 gravity force is exactly
    // F=G_N*delta_2|J| on the finite periodic quotient.  Its matching local
    // scalar is Phi_local=-G_N|J|, because -delta_2(Phi_local)=F under the same
    // radius-2 centred difference.  This is deliberately NOT labelled as the
    // Poisson/Newton potential; it is the potential of the engine's local
    // selected force law.
    if (!sampled.fluxVector?.count) return null;
    const buf = ensureTier1Buffers(state, sampled.fluxVector.count);
    const { vectors, positions, count } = sampled.fluxVector;
    let instantMax = 0;
    for (let i = 0; i < count; i++) {
        const x = vectors[i * 3];
        const y = vectors[i * 3 + 1];
        const z = vectors[i * 3 + 2];
        const well = G_N * Math.sqrt(x * x + y * y + z * z);
        buf.gravPot[i] = -well;
        if (well > instantMax) instantMax = well;
    }
    const normalizer = updateDecayingMax(
        state, 'gravPotentialLocal', instantMax,
    );
    buf.normalizer.gravMax = normalizer;
    return {
        positions,
        values: buf.gravPot,
        count,
        normalizer,
        source: 'local-density-gradient',
        operator: 'phi=-G_N|J|',
        ...sampleGridMetadata(sampled.fluxVector),
    };
}

// ══════════════════════════════════════════════════════════════════════
// Physics-topology overlays — Maxwell energy, charge, vorticity.
// All three are rubber-sheet scalars; they go flat in vacuum/stillness
// and deform only where the underlying field has structure.
// ══════════════════════════════════════════════════════════════════════

/** Build a "x,y,z" → sample-index lookup for one field's sparse sample set. */
function buildPositionLookup(field) {
    const map = new Map();
    if (!field || !field.count) return map;
    const { positions, count } = field;
    for (let i = 0; i < count; i++) {
        map.set(posKey(positions[i * 3], positions[i * 3 + 1], positions[i * 3 + 2]), i);
    }
    return map;
}

/**
 * EM energy density u(x) = ½|E|² + (c²/2)|B|² (C_SPEED carries the magnetic
 * channel — see diagnostics_compute.cpp's own "the magnetic channel carries
 * c^2" comment; a bare ½(|E|²+|B|²) overweights B by 1/c² ≈ 3×).
 * The engine compacts E and B samples INDEPENDENTLY (get_e_field_sampled /
 * get_b_field_sampled in ftd_wasm.cpp each skip voxels below their own
 * 1e-15 magnitude floor), so raw loop index i does not generally address the
 * same physical voxel in both arrays. E and B are paired here by a
 * position-keyed lookup instead of by index; a position present in only one
 * field contributes 0 from the other (that field is genuinely ~0 there, not
 * missing data).
 * Iterates the UNION of both fields' sampled positions, not just one field's
 * reference set — a voxel with E≈0 but nonzero curl (pure-B) still owns a
 * real (c²/2)|B|² contribution and must not be silently dropped from the
 * rendered cloud just because it never appeared in E's sparse sample set
 * (and symmetrically for pure-E voxels with B≈0).
 * When only one of E/B has samples we render that one alone; both absent
 * means there is no field to render and we return null.
 */
export function computeEmEnergyFrame(sampled, state) {
    const eF = sampled.eField;
    const bF = sampled.bField;
    const eCount = eF?.count || 0;
    const bCount = bF?.count || 0;
    if (!eCount && !bCount) return null;
    const upperBound = eCount + bCount;
    const buf = ensureTier1Buffers(state, upperBound);
    if (!state.emPositions || state.emPositions.length < upperBound * 3) {
        state.emPositions = new Float32Array(upperBound * 3);
    }
    const outPositions = state.emPositions;
    const eMap = buildPositionLookup(eF);
    const bMap = buildPositionLookup(bF);
    const c2 = C_SPEED * C_SPEED;
    let max = 0;
    let count = 0;
    const seen = new Set();
    const emit = (field) => {
        if (!field) return;
        const { positions: p, count: n } = field;
        for (let i = 0; i < n; i++) {
            const key = posKey(p[i * 3], p[i * 3 + 1], p[i * 3 + 2]);
            if (seen.has(key)) continue;
            seen.add(key);
            let e2 = 0, b2 = 0;
            const ei = eMap.get(key);
            if (ei !== undefined) {
                const x = eF.vectors[ei * 3], y = eF.vectors[ei * 3 + 1], z = eF.vectors[ei * 3 + 2];
                e2 = x * x + y * y + z * z;
            }
            const bi = bMap.get(key);
            if (bi !== undefined) {
                const x = bF.vectors[bi * 3], y = bF.vectors[bi * 3 + 1], z = bF.vectors[bi * 3 + 2];
                b2 = x * x + y * y + z * z;
            }
            const u = 0.5 * e2 + 0.5 * c2 * b2;
            outPositions[count * 3] = p[i * 3];
            outPositions[count * 3 + 1] = p[i * 3 + 1];
            outPositions[count * 3 + 2] = p[i * 3 + 2];
            buf.emEnergy[count] = u;
            if (u > max) max = u;
            count++;
        }
    };
    emit(eF);
    emit(bF);
    const positions = outPositions;
    buf.normalizer.emMax = max;
    const heldMax = updateDecayingMax(state, 'emEnergy', max);
    return {
        positions, values: buf.emEnergy, count, normalizer: heldMax, signed: false,
        ...sampleGridMetadata(eF || bF),
    };
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
export function computeVorticityFrame(sampled, state) {
    const v = sampled.vorticity;
    if (!v || !v.count) return null;
    let max = 0;
    for (let i = 0; i < v.count; i++) if (v.values[i] > max) max = v.values[i];
    const heldMax = updateDecayingMax(state, 'vorticity', max);
    return {
        positions: v.positions,
        values:    v.values,
        count:     v.count,
        normalizer: heldMax,
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
 *
 * [PROXY, peak-hold] Same upstream-renormalization caveat as
 * computeLatencyFrame: L(x) arrives here already ratio-normalized against
 * the CURRENT TICK's own global peak, so a bare cut against the raw ratio
 * would mark the tick's own peak sampled voxel as "horizon" on essentially
 * every tick regardless of absolute field strength — manufacturing a fake
 * horizon in any scenario with structured flux. This computes its own
 * separate normalization (independent of computeLatencyFrame's 'latency'
 * decay key), so it gets the identical peak-hold treatment here under a
 * distinct 'horizon' key: the fixed 0.95 cut is compared against a decaying
 * reference of this overlay's own recent peak rather than the instantaneous
 * per-tick ratio, so a sampled peak that genuinely falls off shrinks the
 * horizon set instead of snapping back to full on the next tick. This
 * cannot recover absolute-magnitude history the engine's own per-tick
 * renormalization already discarded before the data reached JS.
 */
export function computeHorizonFrame(sampled, state) {
    const L = sampled.latency;
    if (!L || !L.count) return null;
    const threshold = 0.95;
    let instantMax = 0;
    for (let i = 0; i < L.count; i++) if (L.values[i] > instantMax) instantMax = L.values[i];
    const heldMax = updateDecayingMax(state, 'horizon', instantMax);
    const cutoff = heldMax > 1e-9 ? threshold * heldMax : threshold;
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
        if (L.values[i] >= cutoff) {
            positions[count * 3]     = L.positions[i * 3];
            positions[count * 3 + 1] = L.positions[i * 3 + 1];
            positions[count * 3 + 2] = L.positions[i * 3 + 2];
            values[count] = L.values[i];
            count++;
        }
    }
    if (count === 0) return null;
    return { positions, values, count, threshold, normalizer: heldMax, ...sampleGridMetadata(L) };
}

/**
 * Electric-channel energy density P_E(x) = ½|E|², E = −∂J/∂t. This is the
 * substrate's wave-KINETIC channel (identical formula to computeEmEnergyFrame's
 * E-term / the energy audit's wave_energy) — NOT electrostatic pressure. It
 * peaks on fast-changing flux and falls to ~0 in a settled configuration, even
 * directly on top of a stationary charge; the Poisson-solved electrostatic
 * potential φ_C is a separate field this overlay does not read.
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
    const heldMax = updateDecayingMax(state, 'ePressure', max);
    return {
        positions: eF.positions,
        values,
        count:     eF.count,
        normalizer: heldMax,
        signed:    false,
        ...sampleGridMetadata(eF),
    };
}

/**
 * Magnetic-channel energy density P_B(x) = (c²/2)|B|², c = C_SPEED, B = ∇×J.
 * The c² factor matches the engine's own Hamiltonian convention (see
 * computeEmEnergyFrame's identical c² treatment) — without it P_B was 3x too
 * large and not magnitude-comparable with P_E. Rises where the flux field has
 * spatial curl (shear or twist), rather than on charge concentrations.
 */
export function computeBPressureFrame(sampled, state) {
    const bF = sampled.bField;
    if (!bF || !bF.count) return null;
    if (!state.bPressureValues || state.bPressureValues.length < bF.count) {
        state.bPressureValues = new Float32Array(bF.count);
    }
    const values = state.bPressureValues;
    const c2 = C_SPEED * C_SPEED;
    let max = 0;
    for (let i = 0; i < bF.count; i++) {
        const x = bF.vectors[i * 3];
        const y = bF.vectors[i * 3 + 1];
        const z = bF.vectors[i * 3 + 2];
        const p = 0.5 * c2 * (x * x + y * y + z * z);
        values[i] = p;
        if (p > max) max = p;
    }
    const heldMax = updateDecayingMax(state, 'bPressure', max);
    return {
        positions: bF.positions,
        values,
        count:     bF.count,
        normalizer: heldMax,
        signed:    false,
        ...sampleGridMetadata(bF),
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
 * and time dilation (f = 1 − L²). [PROXY]: |J|²_max is computed by the
 * engine itself as the CURRENT TICK's own global peak (get_latency_sampled,
 * ftd_wasm.cpp) — L arrives here already ratio-normalized, so its own top
 * value saturates toward ~0.998 whenever any nonzero flux exists anywhere,
 * independent of the field's absolute strength. This applies the shared
 * decaying-max hold to that ratio's own instant peak so a scene that goes
 * fully quiet fades rather than snapping back to a saturated core on the
 * next tick; it cannot recover absolute-magnitude history the engine's own
 * per-tick renormalization already discarded before the data reached JS (a
 * field that stays peaked while its absolute magnitude decays can still
 * under-report the decline — that would need the engine to expose the raw
 * |J|² alongside a persistently-held |J|²_max, not a JS-side change).
 * Otherwise a pass-through of the engine's latency sampler; rendered as a
 * blue→red volumetric point cloud.
 */
export function computeLatencyFrame(sampled, state) {
    const L = sampled.latency;
    if (!L || !L.count) return null;
    let max = 0;
    for (let i = 0; i < L.count; i++) if (L.values[i] > max) max = L.values[i];
    const heldMax = updateDecayingMax(state, 'latency', max);
    return {
        positions: L.positions,
        values: L.values,
        count: L.count,
        normalizer: heldMax,
        signed: false,
        source: L.kind === 'poissonLatency' ? 'poisson' : 'flux-proxy',
        ...sampleGridMetadata(L),
    };
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
