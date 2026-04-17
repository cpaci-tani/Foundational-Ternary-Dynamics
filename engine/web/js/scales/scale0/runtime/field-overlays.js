import {
    computeStreamlines,
    generateEFieldSeeds,
    generateBFieldSeeds,
    generateGridSeeds,
    generateImportanceSeeds,
    generateBImportanceSeeds,
} from '../../../fieldlines.js?v=2';

const DUAL_DELTA = 0.9568;

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
    if (flags.showEField) sampled.eField = fieldCapability.getScale0FieldSamples({ kind: 'e', stride });
    if (flags.showBField) sampled.bField = fieldCapability.getScale0FieldSamples({ kind: 'b', stride });
    if (flags.showDivField || flags.showLagrangianDensity) {
        sampled.divergence = fieldCapability.getScale0FieldSamples({ kind: 'divJ', stride });
    }
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
            psi2:   new Float32Array(N),
            phase:  new Float32Array(N),
            lagr:   new Float32Array(N),
            entropy: new Float32Array(N),
            gravPot: new Float32Array(N),
            normalizer: { psi2Max: 0, lagMax: 0, gravMax: 0 },
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
    // When Dual Substrate is off, dualLVecs / dualRVecs may be null. Fall
    // back to a trivial phase of 0 for every voxel so the renderer still
    // gets a frame (just a uniform field, matching the physics — real J
    // has zero imaginary component).
    const hasDual = dualLVecs && dualRVecs && dualLVecs.length >= count * 3;
    for (let i = 0; i < count; i++) {
        if (hasDual) {
            const lx = dualLVecs[i * 3], ly = dualLVecs[i * 3 + 1], lz = dualLVecs[i * 3 + 2];
            const rx = dualRVecs[i * 3], ry = dualRVecs[i * 3 + 1], rz = dualRVecs[i * 3 + 2];
            const lMag = Math.sqrt(lx * lx + ly * ly + lz * lz);
            const rMag = Math.sqrt(rx * rx + ry * ry + rz * rz);
            buf.phase[i] = Math.atan2(rMag, lMag);
        } else {
            buf.phase[i] = 0;
        }
    }
    return { positions, values: buf.phase, count, dualAvailable: hasDual };
}

function computeLagrangianDensityFrame(sampled, state) {
    // Per-voxel ℒ(x) ≈ ½|J|² − ½|∇J|²
    //   kinetic term     potential-like term
    // The full Lagrangian chart tracks more terms (coupling, Gauss, dissipation)
    // but those are either scalars or require engine-side data we don't sample
    // per-voxel yet. This captures the dominant (kinetic − gradient) split so
    // users see "blue = potential-dominated, red = kinetic-dominated".
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
    return frame;
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

    const items = [];
    if (state.fieldFlags.showForceEM) {
        const emData = fieldCapability.getScale0ForceField('em', stride);
        if (emData.count > 0) items.push({ type: 'em', data: emData });
    }
    if (state.fieldFlags.showForceGravity) {
        const gravityData = fieldCapability.getScale0ForceField('gravity', stride);
        if (gravityData.count > 0) items.push({ type: 'gravity', data: gravityData });
    }
    if (state.fieldFlags.showForceStrong) {
        const strongData = fieldCapability.getScale0ForceField('strong', stride);
        if (strongData.count > 0) items.push({ type: 'strong', data: strongData });
    }
    if (state.fieldFlags.showForceWeak && sampled.fluxVector?.count > 0) {
        const scalarFactor = DUAL_DELTA;
        if (!state.weakValues || state.weakValues.length < sampled.fluxVector.count) {
            state.weakValues = new Float32Array(sampled.fluxVector.count);
        }
        if (!state.weakVectors || state.weakVectors.length < sampled.fluxVector.count * 3) {
            state.weakVectors = new Float32Array(sampled.fluxVector.count * 3);
        }
        for (let i = 0; i < sampled.fluxVector.count; i++) {
            const x = sampled.fluxVector.vectors[i * 3];
            const y = sampled.fluxVector.vectors[i * 3 + 1];
            const z = sampled.fluxVector.vectors[i * 3 + 2];
            const mag = Math.sqrt(x * x + y * y + z * z);
            state.weakValues[i] = mag * scalarFactor;
            state.weakVectors[i * 3] = x * scalarFactor;
            state.weakVectors[i * 3 + 1] = y * scalarFactor;
            state.weakVectors[i * 3 + 2] = z * scalarFactor;
        }
        items.push({
            type: 'weak',
            data: { positions: sampled.fluxVector.positions, vectors: state.weakVectors, count: sampled.fluxVector.count },
            weakScalar: { positions: sampled.fluxVector.positions, values: state.weakValues, count: sampled.fluxVector.count },
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

export function applyOverlayFrame(viewportAdapter, overlayFrame, forceFrame) {
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
            viewportAdapter.animateForceStreamlines(0.016);
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
    applyOverlayFrame(viewportAdapter, overlayFrame, forceFrame);
}
