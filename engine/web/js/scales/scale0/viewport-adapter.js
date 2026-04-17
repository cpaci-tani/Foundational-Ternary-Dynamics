import { FORCE_FIELD_KEYS } from './state/store.js?v=s1';

const NON_FORCE_OVERLAYS = {
    showEField: 'toggleEFieldLines',
    showBField: 'toggleBFieldLines',
    showPoynting: 'togglePoyntingVectors',
    showDivField: 'toggleDivergenceField',
    showFluxLines: 'toggleFluxStreamlines',
    showDualSubstrate: 'toggleDualFluxVolume',
    showChirality: 'toggleChiralityField',
    showLight: 'toggleLightField',
    showDarkMatterHalo: 'toggleDarkMatterHalo',
    showDampingZones: 'toggleDampingZones',
    showGenesisIsosurface: 'toggleGenesisIsosurface',
    showConfinement: 'toggleConfinement',
    // Tier 1 quantum overlays — see docs/SPEC_S0_QUANTUM_OVERLAYS.md
    showPsiSquared:        'togglePsiSquaredField',
    showPhase:             'togglePhaseField',
    showLagrangianDensity: 'toggleLagrangianDensityField',
    showEntropyDensity:    'toggleEntropyDensityField',
    showGravPotential:     'toggleGravPotentialField',
};

const FORCE_ARROW_OVERLAYS = {
    showForceEM: 'showEMForce',
    showForceGravity: 'showGravityForce',
    showForceStrong: 'showStrongForce',
    showForceWeak: 'showWeakField',
};

export function createScale0ViewportAdapter(viewport) {
    return {
        raw: viewport,
        isFluxVolumeVisible() {
            return !!viewport?.showFlux;
        },
        isFluxSliceVisible() {
            return !!viewport?.showHeatmap;
        },
        setBoundaryShape(shape) {
            viewport?.setBoundaryShape?.(shape);
        },
        setOverlayVisible(name, on) {
            if (!viewport) return;
            if (NON_FORCE_OVERLAYS[name] && typeof viewport[NON_FORCE_OVERLAYS[name]] === 'function') {
                viewport[NON_FORCE_OVERLAYS[name]](on);
                return;
            }
            if (FORCE_ARROW_OVERLAYS[name] && typeof viewport[FORCE_ARROW_OVERLAYS[name]] === 'function') {
                viewport[FORCE_ARROW_OVERLAYS[name]](on);
            }
        },
        setFluxVolumeVisible(on) {
            viewport?.toggleFluxVolume?.(on);
        },
        setFluxSliceVisible(on) {
            viewport?.toggleFluxSlice?.(on);
        },
        hideAllForceStyles() {
            viewport?.hideAllForceStyles?.();
        },
        syncForceStyle(style, fieldState) {
            if (!viewport) return;
            const anyForceOn = Object.keys(fieldState).some((key) => FORCE_FIELD_KEYS.has(key) && fieldState[key]);
            viewport.hideAllForceStyles?.();
            if (!anyForceOn) return;
            if (style === 'arrows') viewport.showArrowForces?.(fieldState);
            else if (style === 'heatmap') viewport.showForceHeatmap?.(true);
            else if (style === 'glyphs') viewport.showForceGlyphs?.(true);
        },
        clearScaleVisuals() {
            if (!viewport) return;
            viewport.toggleEFieldLines?.(false);
            viewport.toggleBFieldLines?.(false);
            viewport.togglePoyntingVectors?.(false);
            viewport.toggleDivergenceField?.(false);
            viewport.toggleFluxStreamlines?.(false);
            viewport.showEMForce?.(false);
            viewport.showGravityForce?.(false);
            viewport.showStrongForce?.(false);
            viewport.showWeakField?.(false);
            viewport.showForceHeatmap?.(false);
            viewport.showForceStreamlines_vis?.(false);
            viewport.showForceGlyphs?.(false);
            viewport.toggleDualFluxVolume?.(false);
            viewport.toggleChiralityField?.(false);
            viewport.toggleLightField?.(false);
            viewport.toggleDarkMatterHalo?.(false);
            viewport.toggleDampingZones?.(false);
            viewport.toggleGenesisIsosurface?.(false);
            viewport.toggleConfinement?.(false);
            // Tier 1 quantum overlays — includes dedicated render objects
            // for Phase needles + Φ rubber-sheet landscape.
            viewport.togglePsiSquaredField?.(false);
            viewport.togglePhaseField?.(false);
            viewport.toggleLagrangianDensityField?.(false);
            viewport.toggleEntropyDensityField?.(false);
            viewport.toggleGravPotentialField?.(false);
        },
        applyParticleFrame(frame) {
            viewport?.updateParticles?.(frame);
        },
        applyFluxVolume(volume, latticeSize) {
            viewport?.updateFluxVolume?.(volume, latticeSize);
        },
        applyFluxSlice(slice, latticeSize, axis, index) {
            viewport?.updateFluxSlice?.(slice, latticeSize, axis, index);
        },
        applyConfinementStrings(bridge) {
            viewport?.updateConfinementStrings?.(bridge);
        },
        applyEFieldLines(lines) {
            viewport?.updateEFieldLines?.(lines);
        },
        applyBFieldLines(lines) {
            viewport?.updateBFieldLines?.(lines);
        },
        applyPoynting(data) {
            viewport?.updatePoyntingVectors?.(data);
        },
        applyDivergence(data) {
            viewport?.updateDivergenceField?.(data);
        },
        applyFluxStreamlines(lines, maxFlux) {
            viewport?.updateFluxStreamlines?.(lines, maxFlux);
        },
        applyForceArrowField(type, data) {
            if (!viewport) return;
            if (type === 'em') viewport.updateEMForceField?.(data);
            else if (type === 'gravity') viewport.updateGravityForceField?.(data);
            else if (type === 'strong') viewport.updateStrongForceField?.(data);
            else if (type === 'weak') viewport.updateWeakField?.(data);
        },
        applyForceHeatmap(data, type) {
            viewport?.updateForceHeatmap?.(data, type);
        },
        applyForceStreamlines(lines, type) {
            viewport?.updateForceStreamlines?.(lines, type);
        },
        animateForceStreamlines(dt) {
            viewport?.animateForceStreamlines?.(dt);
        },
        applyForceGlyphs(data, type) {
            viewport?.updateForceGlyphs?.(data, type);
        },
        applyDarkMatterHalo(frame) {
            viewport?.updateDarkMatterHalo?.(frame.particles, frame.magnitude, frame.latticeSize);
        },
        applyDampingZones(frame) {
            viewport?.updateDampingZones?.(frame.particles, frame.latticeSize);
        },
        applyGenesisIsosurface(frame) {
            viewport?.updateGenesisIsosurface?.(frame.magnitude, frame.latticeSize, frame.threshold);
        },
        applyDualFlux(leftFrame, rightFrame) {
            viewport?.updateDualFluxVolume?.(leftFrame, rightFrame);
        },
        applyChirality(frame) {
            viewport?.updateChiralityField?.(frame);
        },
        applyLight(frame) {
            viewport?.updateLightField?.(frame);
        },
        // ── Tier 1 quantum overlay data handoffs ─────────
        applyPsiSquared(data) {
            viewport?.updatePsiSquaredField?.(data);
        },
        applyPhase(data) {
            viewport?.updatePhaseField?.(data);
        },
        applyLagrangianDensity(data) {
            viewport?.updateLagrangianDensityField?.(data);
        },
        applyEntropyDensity(data) {
            viewport?.updateEntropyDensityField?.(data);
        },
        applyGravPotential(data) {
            viewport?.updateGravPotentialField?.(data);
        },
        render() {
            viewport?.render?.();
        },
    };
}
