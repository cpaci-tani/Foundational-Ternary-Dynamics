import { FORCE_FIELD_KEYS, markFieldDirty } from './state/store.js';
import { createScalarOverlayAdapter } from './viewport-scalar-adapter.js';

const NON_FORCE_OVERLAYS = {
    showEField: 'toggleEFieldLines',
    showBField: 'toggleBFieldLines',
    showPoynting: 'togglePoyntingVectors',
    showDivField: 'toggleDivergenceField',
    showFluxLines: 'toggleFluxStreamlines',
    showStateField: 'toggleStateField',
    showDualSubstrate: 'toggleDualFluxVolume',
    showChirality: 'toggleChiralityField',
    showDarkMatterHalo: 'toggleDarkMatterHalo',
    showDampingZones: 'toggleDampingZones',
    showKnotZones: 'toggleKnotZones',
    showGenesisIsosurface: 'toggleGenesisIsosurface',
    showConfinement: 'toggleConfinement',
    showColorCharge: 'toggleColorChargeRender',
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
    // Tier 1/2 (2026-04-18) — horizon, stress-energy split.
    showHorizon:           'toggleHorizonField',
    showEPressure:         'toggleEPressureField',
    showBPressure:         'toggleBPressureField',
    // New substrate overlays (2026-06-03)
    showLatency:           'toggleLatencyField',
    showGaussResidual:     'toggleGaussResidualField',
};

const FORCE_ARROW_OVERLAYS = {
    showForceEM: 'showEMForce',
    showForceGravity: 'showGravityForce',
    showForceStrong: 'showStrongForce',
    showForceWeak: 'showWeakField',
};

export function createScale0ViewportAdapter(viewport) {
    const scalarOverlays = createScalarOverlayAdapter(viewport);
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
        // Slide a rubber-sheet overlay's slice plane (frac 0..0.999 of the box).
        setTopologySheetHeight(key, frac) {
            viewport?.setTopologySheetHeight?.(key, frac);
        },
        setOverlayVisible(name, on) {
            if (!viewport) return;
            // Volumetric scalar overlay: in Heat-Map mode the glow cloud is the
            // visible surface and the native sheet/cloud stays hidden (and the
            // reverse in default mode), so the two never render on top of each other.
            if (scalarOverlays.setVisible(name, on)) return;
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
        setFluxOrganic(on) {
            viewport?.setFluxOrganic?.(on);
        },
        setFluxGlow(on) {
            viewport?.setFluxGlow?.(on);
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
        setScalarRenderMode(mode) {
            scalarOverlays.setMode(mode);
        },
        // Re-sync every volumetric scalar overlay's two render surfaces to the new
        // meta-mode: native sheet/cloud visible only in 'default', glow heat-map
        // only in 'heatmap'. Mirrors syncForceStyle for the Forces column.
        syncScalarRenderMode(mode, fieldState) {
            scalarOverlays.syncMode(mode, fieldState);
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
            viewport.toggleDarkMatterHalo?.(false);
            viewport.toggleDampingZones?.(false);
            viewport.toggleGenesisIsosurface?.(false);
            viewport.toggleConfinement?.(false);
            // Directional/discrete/threshold overlays remain native; scalar
            // surfaces are cleared as one owned group below.
            viewport.togglePhaseField?.(false);
            viewport.toggleHorizonField?.(false);
            viewport.toggleStateField?.(false);
            scalarOverlays.clear();
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
        applyEFieldLines(lines, knotColoring) {
            viewport?.updateEFieldLines?.(lines, knotColoring);
        },
        applyBFieldLines(lines, knotColoring) {
            viewport?.updateBFieldLines?.(lines, knotColoring);
        },
        applyPoynting(data) {
            viewport?.updatePoyntingVectors?.(data);
        },
        applyDivergence(data) {
            viewport?.updateDivergenceField?.(data);
        },
        applyFluxStreamlines(lines, maxFlux, mags) {
            viewport?.updateFluxStreamlines?.(lines, maxFlux, mags);
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
        applyKnotZones(frame) {
            // Field-line-knot frame: { centroids, extents, count, latticeSize }.
            // updateKnotZones reads centroids+extents directly (back-compat: a
            // bare Float32Array of particle positions still works).
            viewport?.updateKnotZones?.(frame, frame.latticeSize);
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
        // ── Tier 1 quantum overlay data handoffs ─────────
        // Volumetric scalar densities route through applyScalarField so the Heat-Map
        // meta-toggle sends the SAME frame to the glow renderer instead of the native
        // sheet/cloud. phase (directional needles), state (discrete ternary), and
        // horizon (threshold shell) are NOT densities — they keep their native render.
        applyPsiSquared(data)        { scalarOverlays.apply('psiSquared', data); },
        applyPhase(data)             { viewport?.updatePhaseField?.(data); },
        applyLagrangianDensity(data) { scalarOverlays.apply('lagrangian', data); },
        applyEntropyDensity(data)    { scalarOverlays.apply('entropy', data); },
        applyGravPotential(data)     { scalarOverlays.apply('gravPotential', data); },
        applyEmEnergy(data)          { scalarOverlays.apply('emEnergy', data); },
        applyChargeDensity(data)     { scalarOverlays.apply('chargeDensity', data); },
        applyVorticity(data)         { scalarOverlays.apply('vorticity', data); },
        // ── Tier 1/2 (2026-04-18) ──────────────────────
        applyHorizon(data)       { viewport?.updateHorizonField?.(data); },
        applyEPressure(data)     { scalarOverlays.apply('ePressure', data); },
        applyBPressure(data)     { scalarOverlays.apply('bPressure', data); },
        applyStateField(data)    { viewport?.updateStateField?.(data); },
        applyLatency(data)       { scalarOverlays.apply('latency', data); },
        applyGaussResidual(data) { scalarOverlays.apply('gaussResidual', data); },
        render() {
            viewport?.render?.();
        },
    };
}
