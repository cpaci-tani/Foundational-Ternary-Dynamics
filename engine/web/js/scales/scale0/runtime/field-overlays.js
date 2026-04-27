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

    // Source the particle frame from whichever bridge is currently being
    // ticked. Reading ctx.bridge unconditionally would miss particles
    // created by the mock when state.useFluxMock=true and silently fall
    // back to importance-sampling — which renders, but anchors seeds to
    // the |E|/|B| field instead of the actual sources. Lift the lookup
    // once for both E and B so the two overlays stay coherent.
    const activeScale0 = (state.useFluxMock && state.fluxMock)
        ? state.fluxMock.capabilities.scale0
        : ctx.bridge.capabilities.scale0;

    if (state.fieldFlags.showEField && sampled.eField?.count > 0) {
        // E-field: lines start on positive charges and terminate on negative ones.
        // When particles exist we anchor seeds to them (real sources); otherwise
        // we importance-sample from |E| so seeds cluster where the field is strong
        // (iron-filings effect). Bidirectional integration draws from each seed
        // both toward the source and toward the sink, so the visible line spans
        // the natural field-line path.
        const particleData = activeScale0.getScale0ParticleFrame();
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
        const particleData = activeScale0.getScale0ParticleFrame();
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
