import { FORCE_FIELD_KEYS, markFieldDirty } from './state/store.js';

const NON_FORCE_OVERLAYS = {
    showEField: 'toggleEFieldLines',
    showBField: 'toggleBFieldLines',
    showPoynting: 'togglePoyntingVectors',
    showDivField: 'toggleDivergenceField',
    showFluxLines: 'toggleFluxStreamlines',
    showStateField: 'toggleStateField',
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
    // Physics-topology overlays
    showEmEnergy:          'toggleEmEnergyField',
    showChargeDensity:     'toggleChargeDensityField',
    showVorticity:         'toggleVorticityField',
    // Tier 1/2/3 (2026-04-18) — helicity, curvature, horizon, stress-
    // energy split, Fisher information, dual-substrate coherence.
    showHelicity:          'toggleHelicityField',
    showKretschmann:       'toggleKretschmannField',
    showHorizon:           'toggleHorizonField',
    showEPressure:         'toggleEPressureField',
    showBPressure:         'toggleBPressureField',
    showKineticEnergy:     'toggleKineticEnergyField',
    showFisher:            'toggleFisherField',
    showCoherence:         'toggleCoherenceField',
    // New substrate overlays (2026-06-03)
    showLatency:           'toggleLatencyField',
    showGaussResidual:     'toggleGaussResidualField',
    showMooreDecomp:       'toggleMooreDecomp',
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
            else if (style === 'glyphs') {
                // Per-type visibility so only the enabled force's glyph mesh
                // renders. Previously this passed a global `true` and every
                // force's InstancedMesh went visible — but the meshes share
                // no buffer anymore, so a disabled force left stale glyphs
                // on screen from its last update. Map flag→bool per force.
                viewport.showForceGlyphs?.({
                    em:      !!fieldState.showForceEM,
                    gravity: !!fieldState.showForceGravity,
                    strong:  !!fieldState.showForceStrong,
                    weak:    !!fieldState.showForceWeak,
                });
            }
            // hideAllForceStyles zeros every glyph-mesh instance count;
            // the subsequent per-type show() only flips visibility, not
            // count. Force a dirty flag so the next `updateFieldOverlays`
            // tick refills the still-active meshes instead of leaving
            // them visible-but-empty until an unrelated event nudges the
            // dirty bit. Belt-and-braces alongside `setFieldToggle`'s
            // always-dirty fix — guards direct scenario-loader calls too.
            markFieldDirty();
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
            viewport.toggleEmEnergyField?.(false);
            viewport.toggleChargeDensityField?.(false);
            viewport.toggleVorticityField?.(false);
            // Tier 1/2/3 (2026-04-18).
            viewport.toggleHelicityField?.(false);
            viewport.toggleKretschmannField?.(false);
            viewport.toggleHorizonField?.(false);
            viewport.toggleEPressureField?.(false);
            viewport.toggleBPressureField?.(false);
            viewport.toggleKineticEnergyField?.(false);
            viewport.toggleFisherField?.(false);
            viewport.toggleCoherenceField?.(false);
            viewport.toggleStateField?.(false);
            viewport.toggleLatencyField?.(false);
            viewport.toggleGaussResidualField?.(false);
            viewport.toggleMooreDecomp?.(false);
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
        applyFluxSlices(planes, latticeSize, index) {
            viewport?.updateFluxSlices?.(planes, latticeSize, index);
        },
        setFluxSliceAxisEnabled(axis, on) {
            viewport?.setFluxSliceAxisEnabled?.(axis, on);
        },
        getEnabledFluxSliceAxes() {
            return viewport?.getEnabledFluxSliceAxes?.() ?? [0, 1, 2];
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
        applyEmEnergy(data) {
            viewport?.updateEmEnergyField?.(data);
        },
        applyChargeDensity(data) {
            viewport?.updateChargeDensityField?.(data);
        },
        applyVorticity(data) {
            viewport?.updateVorticityField?.(data);
        },
        // ── Tier 1/2/3 (2026-04-18) ──────────────────────
        applyHelicity(data)      { viewport?.updateHelicityField?.(data); },
        applyKretschmann(data)   { viewport?.updateKretschmannField?.(data); },
        applyHorizon(data)       { viewport?.updateHorizonField?.(data); },
        applyEPressure(data)     { viewport?.updateEPressureField?.(data); },
        applyBPressure(data)     { viewport?.updateBPressureField?.(data); },
        applyKineticEnergy(data) { viewport?.updateKineticEnergyField?.(data); },
        applyFisher(data)        { viewport?.updateFisherField?.(data); },
        applyCoherence(data)     { viewport?.updateCoherenceField?.(data); },
        applyStateField(data)    { viewport?.updateStateField?.(data); },
        applyLatency(data)       { viewport?.updateLatencyField?.(data); },
        applyGaussResidual(data) { viewport?.updateGaussResidualField?.(data); },
        render() {
            viewport?.render?.();
        },
    };
}
