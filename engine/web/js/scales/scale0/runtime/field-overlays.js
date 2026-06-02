import {
    computeStreamlines,
    generateEFieldSeeds,
    generateBFieldSeeds,
    generateImportanceSeeds,
    generateBImportanceSeeds,
} from '../../../fieldlines.js';
import { DUAL_DELTA } from '../../../constants.js';
import {
    computePsiSquaredFrame,
    computePhaseFrame,
    computeLagrangianDensityFrame,
    computeEntropyDensityFrame,
    computeGravPotentialFrame,
    computeEmEnergyFrame,
    computeChargeDensityFrame,
    computeVorticityFrame,
    computeHelicityFrame,
    computeKretschmannFrame,
    computeHorizonFrame,
    computeEPressureFrame,
    computeBPressureFrame,
    computeKineticEnergyFrame,
    computeFisherFrame,
    computeCoherenceFrame,
} from './overlay-frames.js';
import {
    computeStreamlineParams,
    fillFieldParticleBuf,
} from './streamline-integrator.js';

export function sampleFieldState(fieldCapability, flags, stride, acScale0) {
    const sampled = {};
    // Snapshot the particle frame ONCE per sweep so every job in the sweep
    // (E-field, B-field) seeds from the same tick's particle positions. Without
    // this, each builder called getScale0ParticleFrame() live on different rAF
    // frames — causing B's seeds to be one frame newer than E's when both fields
    // were enabled, producing the "B translates offset" visual bug.
    if (acScale0 && (flags.showEField || flags.showBField)) {
        sampled.particleData = acScale0.getScale0ParticleFrame();
    }
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

// ── Per-overlay EM streamline builders ──────────────────────────────────
// Extracted verbatim from buildElectromagneticOverlayData so the overlay
// scheduler can build E / B / flux as INDEPENDENT jobs (one heavy streamline
// integration per frame) instead of stacking all three in a single frame.
// The exported buildElectromagneticOverlayData below still calls all three in
// order, so its behaviour is byte-for-byte unchanged for any non-scheduled
// caller. `activeScale0` is the particle-frame source (mock vs main bridge),
// lifted once and threaded in so E and B stay coherent.
function emActiveScale0(ctx, state) {
    return (state.useFluxMock && state.fluxMock)
        ? state.fluxMock.capabilities.scale0
        : ctx.bridge.capabilities.scale0;
}

function buildEFieldLines(activeScale0, state, sampled, latticeSize, stride, p) {
    // E-field: lines start on positive charges and terminate on negative ones.
    // When particles exist we anchor seeds to them (real sources); otherwise
    // we importance-sample from |E| so seeds cluster where the field is strong
    // (iron-filings effect). Bidirectional integration draws from each seed
    // both toward the source and toward the sink, so the visible line spans
    // the natural field-line path.
    //
    // Use the pre-snapshotted particleData from sampleFieldState so E and B
    // both seed from the same tick's particle positions (fixes the offset bug).
    const particleData = sampled.particleData ?? activeScale0.getScale0ParticleFrame();
    fillFieldParticleBuf(state, particleData);
    const seeds = particleData.count > 0
        ? generateEFieldSeeds(state.fieldParticleBuf, p.eOffset, p.maxSeeds)
        : generateImportanceSeeds(sampled.eField, p.maxSeeds);
    return computeStreamlines(sampled.eField, seeds, {
        N: latticeSize, stride, maxSteps: p.maxSteps, stepSize: p.stepSize,
        maxLines: p.maxLines, bidirectional: true,
    });
}

function buildBFieldLines(activeScale0, state, sampled, latticeSize, stride, p) {
    // B-field is divergence-free (∇·B=0), so lines must form closed loops.
    // Anchor seeds to particles when present, else importance-sample with a
    // perpendicular offset so seeds land on the loop circumference rather
    // than at the center (where they'd integrate in place). Bidirectional
    // integration is mandatory — half the loop runs each direction.
    //
    // Use the pre-snapshotted particleData so B seeds from the same particle
    // positions as E (same tick, same snapshot — fixes the offset bug).
    const particleData = sampled.particleData ?? activeScale0.getScale0ParticleFrame();
    fillFieldParticleBuf(state, particleData);
    const seeds = particleData.count > 0
        ? generateBFieldSeeds(state.fieldParticleBuf, p.bRadius, p.maxSeeds)
        : generateBImportanceSeeds(sampled.bField, p.maxSeeds, p.bRadius);
    return computeStreamlines(sampled.bField, seeds, {
        N: latticeSize, stride,
        // Loops need ~ 2·π·radius worth of steps to close — give B 1.5× the
        // baseline so a typical loop completes inside the integration budget.
        maxSteps: Math.ceil(p.maxSteps * 1.5),
        stepSize: p.stepSize, bidirectional: true, maxLines: p.maxLines,
    });
}

function buildFluxStreamlines(sampled, latticeSize, stride, p) {
    // Flux ∇·J carries divergence (sources/sinks), same topology as E.
    // Importance-sample by |J| so streamlines cluster on flux concentrations.
    const seeds = generateImportanceSeeds(sampled.fluxVector, p.maxSeeds);
    const lines = computeStreamlines(sampled.fluxVector, seeds, {
        N: latticeSize, stride, maxSteps: p.maxSteps, stepSize: p.stepSize,
        maxLines: p.maxLines, bidirectional: true,
    });
    let maxFlux = 0;
    for (let i = 0; i < sampled.fluxVector.count; i++) {
        const x = sampled.fluxVector.vectors[i * 3];
        const y = sampled.fluxVector.vectors[i * 3 + 1];
        const z = sampled.fluxVector.vectors[i * 3 + 2];
        const mag = Math.sqrt(x * x + y * y + z * z);
        if (mag > maxFlux) maxFlux = mag;
    }
    return { lines, maxFlux };
}

export function buildElectromagneticOverlayData(ctx, state, sampled, latticeSize, stride, stepsScale, seedSpacing, params = {}) {
    const frame = {};
    const p = {
        stepSize: 0.5, maxSteps: 100, maxSeeds: 150, maxLines: 200,
        eOffset: 2, bRadius: 4, ...params,
    };

    // Source the particle frame from whichever bridge is currently being
    // ticked. Reading ctx.bridge unconditionally would miss particles
    // created by the mock when state.useFluxMock=true and silently fall
    // back to importance-sampling — which renders, but anchors seeds to
    // the |E|/|B| field instead of the actual sources. Lift the lookup
    // once for both E and B so the two overlays stay coherent.
    const activeScale0 = emActiveScale0(ctx, state);

    if (state.fieldFlags.showEField && sampled.eField?.count > 0) {
        frame.eFieldLines = buildEFieldLines(activeScale0, state, sampled, latticeSize, stride, p);
    }

    if (state.fieldFlags.showBField && sampled.bField?.count > 0) {
        frame.bFieldLines = buildBFieldLines(activeScale0, state, sampled, latticeSize, stride, p);
    }

    if (state.fieldFlags.showPoynting && sampled.poynting?.count > 0) {
        frame.poynting = sampled.poynting;
    }

    if (state.fieldFlags.showDivField && sampled.divergence?.count > 0) {
        frame.divergence = sampled.divergence;
    }

    if (state.fieldFlags.showFluxLines && sampled.fluxVector?.count > 0) {
        frame.fluxStreamlines = buildFluxStreamlines(sampled, latticeSize, stride, p);
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

    // The flow-style streamline integration (one full computeStreamlines per
    // force, the heaviest part of this builder) is deferred to the caller when
    // `deferFlow` is set, so the overlay scheduler can spread it one force per
    // frame. Default-false ⇒ unchanged behaviour for any non-scheduled caller:
    // the whole flow loop runs inline and `item.flowLines` is populated here.
    if (state.forceStyle === 'flow' && !params.deferFlow) {
        for (const item of items) {
            item.flowLines = computeForceItemFlow(item, latticeSize, stride, params);
        }
    }

    return { anyForceOn, style: state.forceStyle, items };
}

/**
 * Compute the flow streamlines for ONE force item. Verbatim extraction of the
 * per-item body of buildForceOverlayData's flow loop, so the geometry is
 * identical whether run inline or as a scheduled per-force job.
 */
function computeForceItemFlow(item, latticeSize, stride, params = {}) {
    const { stepSize = 0.5, maxSteps = 100, maxSeeds = 150, maxLines = 200 } = params;
    // Force flow lines stay shorter than EM streamlines (≈ 40% of full length)
    // so the field-arrow visualization stays visually distinct from B/E lines.
    const flowMaxSteps = Math.max(20, Math.ceil(maxSteps * 0.4));
    // Weak is the flux-vector field itself (chirality transmutation follows
    // flux flow); give it denser coverage and longer lines so it reads as a
    // coherent field instead of a sparse cluster.
    const isWeak = item.type === 'weak';
    const seedCount = isWeak ? Math.min(maxSeeds * 2, 320) : maxSeeds;
    const stepCount = isWeak ? maxSteps : flowMaxSteps;
    const lineCount = isWeak ? Math.min(maxLines * 2, 400) : maxLines;
    // Importance-sample by |force| so streamlines cluster where the interaction
    // is strongest (e.g., near charges for EM, near masses for gravity),
    // matching the iron-filing visualization metaphor.
    const seeds = generateImportanceSeeds(item.data, seedCount);
    return computeStreamlines(item.data, seeds, {
        N: latticeSize, stride, maxSteps: stepCount, stepSize,
        maxLines: lineCount, bidirectional: true,
    });
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

// NOTE: superseded by the amortized scheduler (buildOverlayJobs +
// updateFieldOverlays), which builds and applies each overlay per-job to spread
// the work across frames. This monolithic build-then-apply-all entry point is
// retained as exported API for any out-of-tree caller but is no longer used by
// the controller. If you change overlay apply behaviour, update the per-job
// dispatcher runJob (and its apply helpers applyForceFieldsJob / applyDerivedJob
// + the SCALAR_JOBS table's per-row apply fns) — not just this function.
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

// ══════════════════════════════════════════════════════════════════════
// Amortized overlay scheduler (web/engine-optimization-2026-05-31)
// ──────────────────────────────────────────────────────────────────────
// PROBLEM: the throttled overlay-update frame used to build EVERY active
// overlay — E/B/flux streamlines (RK4 over a freshly-built spatial index),
// up to four force-flow streamline fields, and ~15 scalar topology sheets —
// and upload them all inside a SINGLE animate() frame. The median frame is
// cheap; only this one frame spiked (40–55 ms with one light overlay,
// 160–215 ms with streamlines, 130–146 ms with all 33). Pure CPU work
// landing in one tick.
//
// FIX: split the build+apply of each active overlay into independently
// schedulable "jobs", give each a cost weight, and process them across N
// consecutive frames under a per-frame work budget. A persistent round-
// robin cursor remembers where the previous frame stopped, so the next
// frame resumes the same sweep. The expensive streamline jobs (EM/flux/
// force-flow) carry a large weight, so at most ONE of them lands per frame;
// the cheap scalar jobs pack in behind it up to the budget.
//
// VISUAL CORRECTNESS: the field is sampled ONCE at the start of a sweep
// (state.overlaySched.sampled) and every job in that sweep reads the SAME
// snapshot, so overlays remain mutually coherent — they just finish
// painting over 1–few frames instead of all at once. The maximum lag of
// an overlay behind the live field is (jobs_in_sweep / jobs_per_frame)
// frames, bounded by OVERLAY_SWEEP_MAX_FRAMES; for diagnostic overlays a
// sub-100 ms catch-up lag is visually imperceptible. Each overlay still
// renders the exact same geometry/values it did before — only the frame on
// which it lands moves.
//
// SKIP-UNCHANGED: a fresh sweep is only started when the underlying data
// actually changed since the last sweep finished (a tick advanced, or the
// user toggled/dirtied an overlay). If the field is static between throttle
// boundaries, no sweep runs and zero overlay CPU is spent.

// Per-frame compute budget, in abstract "cost units". One streamline-field
// rebuild ≈ COST_STREAMLINE units; one scalar/topology pass ≈ COST_SCALAR.
// The budget is sized so a single streamline job fits in a frame but a
// second one is deferred — that is the whole point (spread the streamlines
// across frames). Cheap scalar jobs keep packing until the budget is spent.
const OVERLAY_FRAME_BUDGET = 100;
// Relative cost weights (calibrated from reading the job bodies, not timed):
//   streamline jobs build a spatial index + bidirectional RK4 over up to
//   ~300 lines × ~maxSteps steps × a 27-cell neighbour scan → dominant.
// Lowered from 100 → 50 so E and B both fit in one frame budget when both
// are enabled. At 100 (= OVERLAY_FRAME_BUDGET) the scheduler could only fit
// one streamline job per frame, causing E and B to update on consecutive
// frames. Since they seed from particle positions and the particle frame was
// re-fetched live inside each job body, B's seeds would be one frame newer
// than E's — producing the "B field shifts/translates offset" visual bug
// when both fields were enabled simultaneously. At 50, both E and B land on
// the same frame and read the same snapshotted particle positions.
const COST_STREAMLINE = 50;   // E / B / flux / each force-flow field
const COST_FORCE_FIELD = 25;  // a force arrow/heatmap/glyph field (sampler + O(count))
const COST_DERIVED = 20;      // dual-substrate / chirality / mock-derived overlays
const COST_SCALAR = 12;       // a Tier-1/2/3 scalar topology sheet (one O(count) pass)
const COST_PASSTHROUGH = 4;   // poynting / divField / light — forward a sampled buffer
// Safety-valve ceiling on how many frames a single sweep may span. This is a
// LAG cap, not the primary spreading mechanism: because the budget loop always
// runs at least the first remaining job each frame (the first-job exception
// below), a sweep of N jobs already finishes in ≤ N frames on its own. The
// ceiling only matters if a future change ever made a frame run zero jobs; it
// then force-drains the remainder so an overlay can never be stranded. It is
// set well above the realistic active-overlay count (~21 jobs at all-33) so it
// does NOT force a large catch-up batch in normal operation — that would
// re-stack the very spike we are removing. Worst-case overlay lag at all-33 is
// therefore the natural ~N-frame spread (≈ a few hundred ms), not this ceiling.
const OVERLAY_SWEEP_MAX_FRAMES = 30;

// ── Allocation-free job model (web/engine-optimization-2026-05-31) ───────
// The scheduler MUST NOT allocate per sweep or per frame in steady state, or
// the GC pause rate regresses. The original design built a fresh `jobs` array
// of object-literals-with-closures every sweep (a sweep starts each throttle
// boundary — every few frames under live physics), plus a fresh SCALAR_JOBS
// table of 16 arrow closures, plus per-run `{ ...params }` spreads and per-run
// `{ key: frame }` objects. All of that is converted to PERSISTENT REUSED
// state here, mirroring the codebase's grow-in-place scratch pattern
// (cf. fillFieldParticleBuf / state.weakVectors): a fixed pool of mutable job
// slots lives on `sched`, `rebuildOverlayJobs` refills it IN PLACE (no new
// array, no new slot objects — it mutates slot fields by index), and a single
// module-level `runJob` dispatches on an integer `kind` instead of a per-job
// closure. The per-sweep context the old closures captured (ctx, state,
// viewportAdapter, latticeSize, params, capabilities, acScale0) is stashed on
// `sched` once at sweep start so `runJob` reaches it without closing over it.

// Job-kind discriminants for the closure-free dispatcher. Each maps 1:1 to one
// of the old job closures; the scheduling semantics (cost weights, ordering,
// one-streamline-per-frame, last-flow dash latch) are unchanged.
const JOB_EFIELD = 0;       // E-field streamline (COST_STREAMLINE)
const JOB_BFIELD = 1;       // B-field streamline (COST_STREAMLINE)
const JOB_FLUX = 2;         // flux streamline (COST_STREAMLINE)
const JOB_PASS = 3;         // poynting / divField passthrough (COST_PASSTHROUGH)
const JOB_FORCE_FIELDS = 4; // force sample + non-flow apply (n·COST_FORCE_FIELD)
const JOB_FORCE_FLOW = 5;   // one force-flow streamline (COST_STREAMLINE)
const JOB_DERIVED = 6;      // derived substrate group (COST_DERIVED)
const JOB_SCALAR = 7;       // one scalar/topology sheet (COST_SCALAR)

// Static scalar-overlay table, allocated ONCE at module load (never per sweep).
// Each entry is [flag, computeFn, applyFn]. Splitting compute/apply lets a
// scalar job run with ZERO per-run allocation: it computes the frame value and
// applies it directly via the adapter call, instead of boxing it in a fresh
// `{ key: value }` object as the old `(s) => ({ key: ... })` closures did. The
// compute/apply pair for each row is an exact extraction of the corresponding
// build branch (buildQuantumOverlayData) + the matching viewport apply call.
const SCALAR_JOBS = [
    ['showPsiSquared',        (s, ctx, state) => computePsiSquaredFrame(s, state, state.fieldFlags.showDualSubstrate), (va, v) => va.applyPsiSquared(v)],
    ['showPhase',             (s, ctx, state) => computePhaseFrame(s, state, state.dualLVecs, state.dualRVecs),        (va, v) => va.applyPhase(v)],
    ['showLagrangianDensity', (s, ctx, state) => computeLagrangianDensityFrame(s, state),                             (va, v) => va.applyLagrangianDensity(v)],
    ['showEntropyDensity',    (s, ctx, state) => computeEntropyDensityFrame(s, state),                                (va, v) => va.applyEntropyDensity(v)],
    ['showGravPotential',     (s, ctx, state) => computeGravPotentialFrame(ctx, s, state),                            (va, v) => va.applyGravPotential(v)],
    ['showEmEnergy',          (s, ctx, state) => computeEmEnergyFrame(s, state),                                      (va, v) => va.applyEmEnergy(v)],
    ['showChargeDensity',     (s, ctx, state) => computeChargeDensityFrame(s, state),                                 (va, v) => va.applyChargeDensity(v)],
    ['showVorticity',         (s, ctx, state) => computeVorticityFrame(s, state),                                     (va, v) => va.applyVorticity(v)],
    ['showHelicity',          (s, ctx, state) => computeHelicityFrame(s, state),                                      (va, v) => va.applyHelicity(v)],
    ['showKretschmann',       (s, ctx, state) => computeKretschmannFrame(s, state),                                   (va, v) => va.applyKretschmann(v)],
    ['showHorizon',           (s, ctx, state) => computeHorizonFrame(s, state),                                       (va, v) => va.applyHorizon(v)],
    ['showEPressure',         (s, ctx, state) => computeEPressureFrame(s, state),                                     (va, v) => va.applyEPressure(v)],
    ['showBPressure',         (s, ctx, state) => computeBPressureFrame(s, state),                                     (va, v) => va.applyBPressure(v)],
    ['showKineticEnergy',     (s, ctx, state) => computeKineticEnergyFrame(ctx, state),                               (va, v) => va.applyKineticEnergy(v)],
    ['showFisher',            (s, ctx, state) => computeFisherFrame(s, state),                                        (va, v) => va.applyFisher(v)],
    ['showCoherence',         (s, ctx, state) => computeCoherenceFrame(s, state),                                     (va, v) => va.applyCoherence(v)],
];

// Force-flow type table, allocated ONCE at module load. Filtered into the
// persistent `sched.flowTypes` scratch (in place) per sweep — no new array.
const FLOW_TYPES = [
    ['showForceEM', 'em'], ['showForceGravity', 'gravity'],
    ['showForceStrong', 'strong'], ['showForceWeak', 'weak'],
];

function ensureOverlaySched(state) {
    if (!state.overlaySched) {
        state.overlaySched = {
            // Sweep liveness: `active` replaces the old `jobs !== null` sentinel
            // so the pooled `jobs` array can persist across sweeps (its slots are
            // reused; `jobCount` is the live length for the current sweep).
            active: false,     // a sweep is in flight (false = idle)
            jobs: [],          // PERSISTENT pool of reusable job slots (grown, never re-created)
            jobCount: 0,       // number of live slots in the current sweep
            cursor: 0,         // index of the next job to run in this sweep
            sampled: null,     // field snapshot shared by every job in the sweep
            running: false,    // ctx.running latched at sweep start (for sub-anims)
            sweepFrames: 0,    // frames elapsed in the current sweep
            forceAnimated: false, // force-streamline dash advanced once per sweep
            lastVersion: -1,   // fieldDataVersion sampled at the last sweep start
            forceFrame: null,  // force-fields-job output (read by flow jobs)
            flowTypes: [],     // PERSISTENT scratch for active force-flow types
            // Per-sweep context the closure-free dispatcher reads in place of a
            // captured closure. All stable for the sweep's duration; set once at
            // sweep start in buildOverlayJobs.
            ctx: null,
            state: null,
            viewportAdapter: null,
            latticeSize: 0,
            params: null,
            fieldCapability: null,
            mockCapability: null,
            acScale0: null,
        };
    }
    return state.overlaySched;
}

// Acquire job slot at `index` from the persistent pool, growing the pool by one
// reusable slot object only when the pool has never been that long (a one-time
// amortized allocation that stops once the pool reaches its high-water mark;
// zero allocation on every subsequent sweep). Returns the slot so the caller
// mutates its fields in place.
function jobSlot(sched, index) {
    let slot = sched.jobs[index];
    if (slot === undefined) {
        slot = { kind: -1, cost: 0, scalarIndex: -1, flowType: '', isLastFlow: false };
        sched.jobs[index] = slot;
    }
    return slot;
}

// ── Per-overlay apply dispatch ──────────────────────────────────────────
// Each job builds its slice into a scratch frame object and applies it
// immediately, so both the CPU build AND the viewport upload for that
// overlay land on the job's frame (spreading both halves of the cost).
// The build/apply pairs below are exact extractions of the corresponding
// branches in applyOverlayFrame + the build* functions — same inputs,
// same viewport-adapter calls, same order.

// Apply the force overlay's NON-flow styles (arrows / heatmap / glyphs). These
// are cheap (the data was already sampled by buildForceOverlayData) so they all
// run in the single force-fields job. Flow streamlines are NOT applied here —
// they are computed+applied per force in their own scheduled jobs (see below),
// because each is a full streamline integration.
function applyForceFieldsJob(sched, viewportAdapter) {
    const forceFrame = sched.forceFrame;
    if (!forceFrame || !forceFrame.anyForceOn) return;
    if (forceFrame.style === 'arrows') {
        for (const item of forceFrame.items) {
            viewportAdapter.applyForceArrowField(item.type, item.type === 'weak' ? item.weakScalar : item.data);
        }
    } else if (forceFrame.style === 'heatmap') {
        for (const item of forceFrame.items) viewportAdapter.applyForceHeatmap(item.data, item.type);
    } else if (forceFrame.style === 'glyphs') {
        for (const item of forceFrame.items) viewportAdapter.applyForceGlyphs(item.data, item.type);
    }
    // 'flow' falls through: handled by the per-force flow jobs.
}

// Linear lookup of a force item by type. Replaces `items.find((it) => ...)` in
// the flow job so the drain loop allocates no per-job arrow closure even when
// force-flow is active. Order/result are identical to Array.prototype.find.
function findForceItem(items, type) {
    if (!items) return undefined;
    for (let i = 0; i < items.length; i++) {
        if (items[i].type === type) return items[i];
    }
    return undefined;
}

// Scalar / derived / passthrough applies are 1:1 with their overlayFrame key.
function applyDerivedJob(frame, viewportAdapter) {
    if (frame.darkMatterHalo) viewportAdapter.applyDarkMatterHalo(frame.darkMatterHalo);
    if (frame.dampingZones) viewportAdapter.applyDampingZones(frame.dampingZones);
    if (frame.genesisIsosurface) viewportAdapter.applyGenesisIsosurface(frame.genesisIsosurface);
    if (frame.dualFlux) viewportAdapter.applyDualFlux(frame.dualFlux.left, frame.dualFlux.right);
    if (frame.chirality) viewportAdapter.applyChirality(frame.chirality);
    if (frame.light) viewportAdapter.applyLight(frame.light);
}

// ── Closure-free job dispatcher ─────────────────────────────────────────
// Single module-level function that executes one pooled job slot. Replaces the
// per-job `run()` closures the old buildOverlayJobs allocated every sweep: the
// slot carries only plain-data fields (kind + payload), and every value the old
// closure captured is read from `sched` (set once per sweep). The body of each
// case is the exact same build+apply the corresponding closure performed, in
// the same order, so visual output is byte-identical.
function runJob(sched, slot) {
    const { ctx, state, viewportAdapter, latticeSize, params, sampled } = sched;
    const { stride } = params;
    switch (slot.kind) {
        case JOB_EFIELD: {
            const lines = buildEFieldLines(sched.acScale0, state, sampled, latticeSize, stride, params);
            viewportAdapter.applyEFieldLines(lines);
            break;
        }
        case JOB_BFIELD: {
            const lines = buildBFieldLines(sched.acScale0, state, sampled, latticeSize, stride, params);
            viewportAdapter.applyBFieldLines(lines);
            break;
        }
        case JOB_FLUX: {
            const fs = buildFluxStreamlines(sampled, latticeSize, stride, params);
            viewportAdapter.applyFluxStreamlines(fs.lines, fs.maxFlux);
            break;
        }
        case JOB_PASS: {
            const flags = state.fieldFlags;
            if (flags.showPoynting && sampled.poynting?.count > 0) viewportAdapter.applyPoynting(sampled.poynting);
            if (flags.showDivField && sampled.divergence?.count > 0) viewportAdapter.applyDivergence(sampled.divergence);
            break;
        }
        case JOB_FORCE_FIELDS: {
            // params.deferFlow was set true once for this sweep (see
            // buildOverlayJobs), so the heavy flow integration is deferred out
            // of this fields job into the per-force JOB_FORCE_FLOW jobs.
            sched.forceFrame = buildForceOverlayData(
                state, sched.fieldCapability, sampled, latticeSize, stride,
                params.stepsScale, params.seedSpacing, params);
            applyForceFieldsJob(sched, viewportAdapter);
            break;
        }
        case JOB_FORCE_FLOW: {
            const ff = sched.forceFrame;
            const item = findForceItem(ff?.items, slot.flowType);
            // item may be absent if the sampler returned zero count (e.g. no
            // particles for that force) — that force simply has nothing to draw,
            // matching the original loop.
            if (item) {
                item.flowLines = computeForceItemFlow(item, latticeSize, stride, params);
                viewportAdapter.applyForceStreamlines(item.flowLines, item.type);
            }
            // Advance the dash-offset animation exactly ONCE per sweep, on the
            // last flow job, only while running — matching the pre-amortization
            // cadence (one advance per overlay refresh).
            if (slot.isLastFlow && sched.running && !sched.forceAnimated) {
                viewportAdapter.animateForceStreamlines(0.016);
                sched.forceAnimated = true;
            }
            break;
        }
        case JOB_DERIVED: {
            const frame = buildDerivedSubstrateData(state, sampled, sched.mockCapability);
            applyDerivedJob(frame, viewportAdapter);
            break;
        }
        case JOB_SCALAR: {
            // compute returns the bare frame value (NOT a `{ key: value }`
            // wrapper) and apply forwards it directly — eliminating the per-run
            // object the old `(s) => ({ key: ... })` closures boxed every frame.
            const entry = SCALAR_JOBS[slot.scalarIndex];
            const value = entry[1](sampled, ctx, state);
            entry[2](viewportAdapter, value);
            break;
        }
    }
}

// Refill the persistent job pool IN PLACE for one sweep. Mutates pre-allocated
// slot objects by index (sched.jobs / jobSlot) and sets sched.jobCount — it
// allocates no new array and (after the pool reaches its high-water mark) no
// new slot objects, so a steady-state sweep is allocation-free. The ordering
// and per-job cost weights are identical to the old closure-building version,
// so the scheduling semantics (one streamline/frame, budget packing, last-flow
// latch) are unchanged. The per-sweep context every job needs is stashed on
// `sched` here, once, in place of the closures' captured variables.
function buildOverlayJobs(ctx, state, sched, viewportAdapter, latticeSize, params) {
    const fieldCapability = (state.useFluxMock ? state.fluxMock : ctx.bridge).capabilities.scale0;
    const mockCapability = state.fluxMock?.capabilities?.scale0 || null;
    const sampled = sched.sampled;
    const flags = state.fieldFlags;
    const acScale0 = emActiveScale0(ctx, state);

    // Stash the sweep context the closure-free dispatcher reads (replaces the
    // old per-closure captured variables). All stable for the sweep duration.
    sched.ctx = ctx;
    sched.state = state;
    sched.viewportAdapter = viewportAdapter;
    sched.latticeSize = latticeSize;
    sched.params = params;
    sched.fieldCapability = fieldCapability;
    sched.mockCapability = mockCapability;
    sched.acScale0 = acScale0;
    // The force-fields job builds with flow deferred. Set the flag once on the
    // sweep params object instead of spreading `{ ...params, deferFlow: true }`
    // per run (an allocation). `deferFlow` is read only by buildForceOverlayData;
    // the streamline builders ignore the extra key, so this is inert for them.
    params.deferFlow = true;

    let n = 0; // running job count; jobSlot(sched, n) reuses the pooled slot

    // ── EM streamline overlays — E, B, flux each as an INDEPENDENT job ────
    // These three are the heaviest work (each a full bidirectional-RK4 line
    // integration over a fresh spatial index). Splitting them into one job
    // apiece is what unstacks the 160–215 ms "flux + E/B" spike: each carries
    // COST_STREAMLINE, which exceeds the per-frame budget, so the budget gate
    // admits at most ONE streamline integration per frame and defers the rest
    // to the next frame(s) of the same sweep. Each job's output is a complete,
    // atomic line set (identical geometry to the monolithic build) applied in
    // a single full-replace call — only the frame it lands on moves.
    if (flags.showEField && sampled.eField?.count > 0) {
        const slot = jobSlot(sched, n++); slot.kind = JOB_EFIELD; slot.cost = COST_STREAMLINE;
    }
    if (flags.showBField && sampled.bField?.count > 0) {
        const slot = jobSlot(sched, n++); slot.kind = JOB_BFIELD; slot.cost = COST_STREAMLINE;
    }
    if (flags.showFluxLines && sampled.fluxVector?.count > 0) {
        const slot = jobSlot(sched, n++); slot.kind = JOB_FLUX; slot.cost = COST_STREAMLINE;
    }
    // Poynting / divergence are zero-cost passthroughs (forward a sampled
    // buffer); batch them as one cheap job.
    const passActive = (flags.showPoynting && sampled.poynting?.count > 0) ||
        (flags.showDivField && sampled.divergence?.count > 0);
    if (passActive) {
        const slot = jobSlot(sched, n++); slot.kind = JOB_PASS; slot.cost = COST_PASSTHROUGH;
    }

    // ── Force group ──────────────────────────────────────────────────────
    // One job samples + builds the force fields and applies the non-flow style
    // (arrows / heatmap / glyphs — all cheap). When the style is 'flow', the
    // heavy part is up to four extra full streamline integrations; those are
    // split into ONE job per force so the budget admits a single force-flow
    // integration per frame, exactly as for E/B/flux. This is what stops the
    // 4-force flow configuration from re-stacking a multi-streamline spike.
    const anyForceOn = flags.showForceEM || flags.showForceGravity ||
        flags.showForceStrong || flags.showForceWeak;
    if (anyForceOn) {
        const isFlow = state.forceStyle === 'flow';
        let activeForces = 0;
        if (flags.showForceEM) activeForces++;
        if (flags.showForceGravity) activeForces++;
        if (flags.showForceStrong) activeForces++;
        if (flags.showForceWeak) activeForces++;
        // Fields job: sample all forces, apply non-flow style. Cost is the
        // sampler + O(count) passes; flow integration is deferred out of it.
        const fSlot = jobSlot(sched, n++);
        fSlot.kind = JOB_FORCE_FIELDS;
        fSlot.cost = activeForces * COST_FORCE_FIELD;
        if (isFlow) {
            // One flow job per active force type. Each looks its item up in the
            // fields-job output (built on an earlier frame of this sweep) and
            // computes+applies just that force's streamlines. The last one to
            // run advances the dash animation once (forceAnimated latch). Filter
            // the static FLOW_TYPES into the persistent flowTypes scratch in
            // place (no new array), then emit one pooled slot per active type.
            const flowTypes = sched.flowTypes;
            flowTypes.length = 0;
            for (let i = 0; i < FLOW_TYPES.length; i++) {
                if (flags[FLOW_TYPES[i][0]]) flowTypes.push(FLOW_TYPES[i][1]);
            }
            for (let i = 0; i < flowTypes.length; i++) {
                const slot = jobSlot(sched, n++);
                slot.kind = JOB_FORCE_FLOW;
                slot.cost = COST_STREAMLINE;
                slot.flowType = flowTypes[i];
                slot.isLastFlow = i === flowTypes.length - 1;
            }
        }
    }

    // ── Derived substrate group (dual / chirality / light / mock overlays) ─
    const derivedActive = flags.showDarkMatterHalo || flags.showDampingZones ||
        flags.showGenesisIsosurface ||
        (flags.showDualSubstrate && sampled.fluxVector?.count > 0) ||
        (flags.showChirality && sampled.fluxVector?.count > 0) ||
        (flags.showLight && sampled.poynting?.count > 0);
    if (derivedActive) {
        const slot = jobSlot(sched, n++); slot.kind = JOB_DERIVED; slot.cost = COST_DERIVED;
    }

    // ── Quantum / topology scalar sheets ─────────────────────────────────
    // Each is a single O(count) pass; cheap individually but there are up to
    // ~15 of them. Emit ONE job per active scalar so the budget can pack as
    // many as fit per frame and defer the rest — this is what unstacks the
    // "all 33 overlays" spike. The scalar table (SCALAR_JOBS) is module-scope
    // (allocated once); a scalar job stores only its row index, and runJob
    // calls that row's compute+apply with no per-job allocation. Buffer reuse
    // (overlay-frames ensureTier1Buffers and the per-overlay state caches) is
    // unaffected: a job still computes its full frame in one shot, it just may
    // run a frame later.
    for (let i = 0; i < SCALAR_JOBS.length; i++) {
        if (!flags[SCALAR_JOBS[i][0]]) continue;
        const slot = jobSlot(sched, n++);
        slot.kind = JOB_SCALAR;
        slot.cost = COST_SCALAR;
        slot.scalarIndex = i;
    }

    sched.jobCount = n;
}

export function updateFieldOverlays(ctx, state, viewportAdapter) {
    state.fieldFrame += 1;
    const latticeSize = ctx.bridge.latticeSize || 32;
    const fieldThrottle = latticeSize > 96 ? 12 : (latticeSize > 48 ? 6 : 3);
    const sched = ensureOverlaySched(state);

    if (!state.anyFieldActive) {
        // Nothing to draw — abandon any half-finished sweep so a later
        // re-activation starts clean rather than resuming stale jobs. The job
        // pool itself persists (its slots are reused next sweep); only the
        // sweep liveness + shared snapshot are cleared.
        sched.active = false;
        sched.sampled = null;
        return;
    }

    // An explicit dirty (overlay toggle, force-style change, scenario load /
    // reset — all set fieldNeedsUpdate) means the world the in-flight sweep is
    // painting is stale. Preempt it: drop the half-finished sweep and its old
    // snapshot so the gate below opens a fresh sweep against current data.
    // Without this, loading a new scenario mid-sweep would paint one frame of
    // the previous scenario's overlays before catching up.
    if (state.fieldNeedsUpdate && sched.active) {
        sched.active = false;
        sched.sampled = null;
    }

    // A sweep already in flight always continues to completion regardless of
    // the throttle — its field snapshot is fixed, and finishing it is what
    // bounds overlay lag. Only the START of a new sweep is throttle/dirty
    // gated.
    const sweepInFlight = sched.active && sched.cursor < sched.jobCount;

    if (!sweepInFlight) {
        // ── Trigger gate for a NEW sweep ─────────────────────────────────
        // A sweep starts only on a throttle boundary (or an explicit dirty),
        // and only when the underlying field data actually changed since the
        // last sweep:
        //
        //   • fieldNeedsUpdate — a one-shot dirty (overlay toggle / style
        //     change / scenario load), honoured even under global pause so the
        //     user sees a single frame of the frozen state after toggling.
        //
        //   • version moved    — `fieldDataVersion` (monotonic, bumped once per
        //     real tick in tick.js; a counter we own, NOT the frame-sync-
        //     consumed `latticeNeedsUpload` flag) differs from the value we
        //     latched at the previous sweep ⇒ a tick advanced the field.
        //
        // SKIP-UNCHANGED (optimization point 2): if neither holds — the field
        // is byte-for-byte the state we last rendered — we run NO sweep and
        // spend zero overlay CPU, whether the sim is globally paused or merely
        // scenario-paused (residual-motion mode). A static field therefore
        // shows static overlays. This intentionally drops the old behaviour
        // where importance-sampled streamlines re-randomised their seeds every
        // throttle frame against a frozen field (a visible jitter, and wasted
        // work); frozen field → frozen lines is both cheaper and more correct.
        // The live-physics hot path (running + scenario-running) advances the
        // version every frame, so it is unaffected.
        const version = state.fieldDataVersion || 0;
        const onBoundary = state.fieldNeedsUpdate || state.fieldFrame % fieldThrottle === 0;
        const dataChanged = state.fieldNeedsUpdate || version !== sched.lastVersion;
        if (!onBoundary || !dataChanged) return;

        // Latch the trigger and open a fresh sweep: sample the field ONCE so
        // every job in this sweep sees one coherent snapshot. buildOverlayJobs
        // refills the persistent slot pool IN PLACE and sets sched.jobCount;
        // it allocates no new job array/objects in steady state.
        state.fieldNeedsUpdate = false;
        sched.lastVersion = version;
        const params = computeStreamlineParams(latticeSize);
        const fieldCapability = (state.useFluxMock ? state.fluxMock : ctx.bridge).capabilities.scale0;
        const acScale0ForSnapshot = emActiveScale0(ctx, state);
        sched.sampled = sampleFieldState(fieldCapability, state.fieldFlags, params.stride, acScale0ForSnapshot);
        sched.running = !!ctx.running;
        sched.cursor = 0;
        sched.sweepFrames = 0;
        sched.forceAnimated = false;
        sched.forceFrame = null;
        buildOverlayJobs(ctx, state, sched, viewportAdapter, latticeSize, params);
        sched.active = true;
        // An empty job list (all active flags gated out by zero-count samples)
        // is a completed no-op sweep.
        if (sched.jobCount === 0) { sched.active = false; sched.sampled = null; return; }
    }

    // ── Drain jobs under the per-frame budget ────────────────────────────
    // Run jobs from the cursor until the budget is spent or the sweep ends.
    // A job whose individual cost exceeds the whole budget (a single heavy
    // streamline job) is still allowed to run when it is the FIRST job this
    // frame — otherwise it could never make progress. The hard frame ceiling
    // forces the remainder through if a sweep has dragged on too long. The loop
    // indexes into the persistent pool and dispatches via runJob — no per-frame
    // allocation.
    sched.sweepFrames += 1;
    const forceFinish = sched.sweepFrames >= OVERLAY_SWEEP_MAX_FRAMES;
    let spent = 0;
    while (sched.cursor < sched.jobCount) {
        const job = sched.jobs[sched.cursor];
        const isFirstThisFrame = spent === 0;
        if (!forceFinish && !isFirstThisFrame && spent + job.cost > OVERLAY_FRAME_BUDGET) break;
        runJob(sched, job);
        spent += job.cost;
        sched.cursor += 1;
    }

    if (sched.cursor >= sched.jobCount) {
        // Sweep complete. Release the snapshot so the next trigger re-samples;
        // the slot pool persists for reuse on the next sweep.
        sched.active = false;
        sched.sampled = null;
    }
}
