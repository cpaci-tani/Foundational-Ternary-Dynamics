import { computeStreamlines, generateEFieldSeeds, generateBFieldSeeds, generateGridSeeds } from '../../../fieldlines.js';

const DUAL_DELTA = 0.9568;

export function sampleFieldState(fieldCapability, flags, stride) {
    const sampled = {};
    if (flags.showFluxLines || flags.showDualSubstrate || flags.showChirality || flags.showForceWeak) {
        sampled.fluxVector = fieldCapability.getScale0FieldSamples({ kind: 'fluxVector', stride });
    }
    if (flags.showPoynting || flags.showLight) {
        sampled.poynting = fieldCapability.getScale0FieldSamples({ kind: 'poynting', stride });
    }
    if (flags.showEField) sampled.eField = fieldCapability.getScale0FieldSamples({ kind: 'e', stride });
    if (flags.showBField) sampled.bField = fieldCapability.getScale0FieldSamples({ kind: 'b', stride });
    if (flags.showDivField) sampled.divergence = fieldCapability.getScale0FieldSamples({ kind: 'divJ', stride });
    return sampled;
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

export function buildElectromagneticOverlayData(ctx, state, sampled, latticeSize, stride, stepsScale, seedSpacing) {
    const frame = {};
    if (state.fieldFlags.showEField && sampled.eField?.count > 0) {
        const particleData = ctx.bridge.capabilities.scale0.getScale0ParticleFrame();
        fillFieldParticleBuf(state, particleData);
        const seeds = particleData.count > 0
            ? generateEFieldSeeds(state.fieldParticleBuf, 2, 120)
            : generateGridSeeds(latticeSize, seedSpacing, 120);
        frame.eFieldLines = computeStreamlines(sampled.eField, seeds, {
            N: latticeSize, stride, maxSteps: 80 * stepsScale, stepSize: 0.6,
        });
    }

    if (state.fieldFlags.showBField && sampled.bField?.count > 0) {
        const particleData = ctx.bridge.capabilities.scale0.getScale0ParticleFrame();
        fillFieldParticleBuf(state, particleData);
        const seeds = particleData.count > 0
            ? generateBFieldSeeds(state.fieldParticleBuf, 4, 120)
            : generateGridSeeds(latticeSize, seedSpacing, 120);
        frame.bFieldLines = computeStreamlines(sampled.bField, seeds, {
            N: latticeSize, stride, maxSteps: 150 * stepsScale, stepSize: 0.5, bidirectional: false,
        });
    }

    if (state.fieldFlags.showPoynting && sampled.poynting?.count > 0) {
        frame.poynting = sampled.poynting;
    }

    if (state.fieldFlags.showDivField && sampled.divergence?.count > 0) {
        frame.divergence = sampled.divergence;
    }

    if (state.fieldFlags.showFluxLines && sampled.fluxVector?.count > 0) {
        const seeds = generateGridSeeds(latticeSize, seedSpacing, 150);
        const lines = computeStreamlines(sampled.fluxVector, seeds, {
            N: latticeSize, stride, maxSteps: 80 * stepsScale, stepSize: 0.5,
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

export function buildForceOverlayData(state, fieldCapability, sampled, latticeSize, stride, stepsScale, seedSpacing) {
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
        for (const item of items) {
            const seeds = generateGridSeeds(latticeSize, seedSpacing, 150);
            item.flowLines = computeStreamlines(item.data, seeds, {
                N: latticeSize, stride, maxSteps: 30 * stepsScale, stepSize: 0.5,
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
}

export function updateFieldOverlays(ctx, state, viewportAdapter) {
    state.fieldFrame += 1;
    const latticeSize = ctx.bridge.latticeSize || 32;
    const fieldThrottle = latticeSize > 96 ? 12 : (latticeSize > 48 ? 6 : 3);
    if (!state.anyFieldActive || (!state.fieldNeedsUpdate && state.fieldFrame % fieldThrottle !== 0)) return;

    state.fieldNeedsUpdate = false;
    const fieldCapability = (state.useFluxMock ? state.fluxMock : ctx.bridge).capabilities.scale0;
    const mockCapability = state.fluxMock?.capabilities?.scale0 || null;
    const stride = latticeSize > 96 ? 8 : (latticeSize > 48 ? 6 : (latticeSize > 32 ? 4 : 2));
    const stepsScale = Math.ceil(latticeSize / 32);
    const seedSpacing = Math.max(2, Math.min(8, Math.floor(latticeSize / 4)));

    const sampled = sampleFieldState(fieldCapability, state.fieldFlags, stride);
    const overlayFrame = buildElectromagneticOverlayData(ctx, state, sampled, latticeSize, stride, stepsScale, seedSpacing);
    const forceFrame = buildForceOverlayData(state, fieldCapability, sampled, latticeSize, stride, stepsScale, seedSpacing);
    Object.assign(overlayFrame, buildDerivedSubstrateData(state, sampled, mockCapability));
    applyOverlayFrame(viewportAdapter, overlayFrame, forceFrame);
}
