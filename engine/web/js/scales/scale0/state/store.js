import { createTickAccumulator } from '../../scale-utils.js';

export const FIELD_TOGGLE_KEYS = [
    'showEField',
    'showBField',
    'showPoynting',
    'showDivField',
    'showFluxLines',
    'showForceEM',
    'showForceGravity',
    'showForceStrong',
    'showForceWeak',
    'showDualSubstrate',
    'showChirality',
    'showLight',
    'showDarkMatterHalo',
    'showDampingZones',
    'showGenesisIsosurface',
    'showConfinement',
    // Tier 1 quantum overlays — see docs/SPEC_S0_QUANTUM_OVERLAYS.md
    'showPsiSquared',
    'showPhase',
    'showLagrangianDensity',
    'showEntropyDensity',
    'showGravPotential',
    // Physics-topology overlays — rubber-sheet surfaces derived from sampled fields.
    // Each goes flat in stillness and deforms as topology develops.
    'showEmEnergy',       // u(x) = ½(|E|² + |B|²)   — Maxwell energy density
    'showChargeDensity',  // ρ(x) = ∇·J              — FTD-native charge (signed)
    'showVorticity',      // |ω|(x) = |∇×J|          — flux-field swirl magnitude
    // Tier 1 additions (2026-04-18) — geometric + gravitational invariants
    'showHelicity',       // h(x) = J·(∇×J)          — signed, field-line linking
    'showKretschmann',    // K(x) = (∇²L)²           — gravitational curvature
    'showHorizon',        // L(x) ≥ 0.95             — event horizon isosurface
    // Tier 2 additions (2026-04-18) — stress-energy split
    'showEPressure',      // P_E = ½|E|²             — electric pressure
    'showBPressure',      // P_B = ½|B|²             — magnetic pressure
    'showKineticEnergy',  // K_k = ½|v|² particles   — kinetic energy density
    // Tier 3 additions (2026-04-18) — quantum / info
    'showFisher',         // F(x) = |∇ρ|²/ρ, ρ=|J|²  — Fisher information
    'showCoherence',      // C(x) = J_L·J_R/(|L||R|) — dual-substrate coherence
];

export const FORCE_FIELD_KEYS = new Set([
    'showForceEM',
    'showForceGravity',
    'showForceStrong',
    'showForceWeak',
]);

// Single source of truth: derive the all-off defaults bag from
// FIELD_TOGGLE_KEYS directly. Previously this was a hand-maintained
// mirror — adding a key to the list without adding it here left
// `resetFieldFlags` unable to clear the new flag. Auditors caught
// multiple drift incidents; keeping the two in lockstep programmatically
// removes the hazard.
function createFieldFlags() {
    return Object.fromEntries(FIELD_TOGGLE_KEYS.map((k) => [k, false]));
}

const state = {
    currentScenarioId: 'flux-pulse',
    fieldFlags: createFieldFlags(),
    fieldFrame: 0,
    fieldNeedsUpdate: false,
    anyFieldActive: false,
    forceStyle: 'arrows',
    fluxMock: null,
    useFluxMock: false,
    latticeNeedsUpload: true,
    tickAccumulator: createTickAccumulator(),
    fieldParticleBuf: [],
    dualLVecs: null,
    dualRVecs: null,
    chiralValues: null,
    weakValues: null,
    weakVectors: null,
    // When the user is dragging the scrub thumb we freeze physics: sim ticks
    // would otherwise overwrite the snapshot we just loaded. Flipped true by
    // hydrateToTick and back to false by resumeLive (onScrubEnd).
    scrubbing: false,
    // While a RenderController is fast-forwarding snapshots for a clip we
    // also freeze the live animate loop's tick path. Otherwise the main
    // loop and the render controller both call bridge.tick() and clobber
    // each other's state. Flipped true by startScale0Render and back to
    // false by the controller's 'done' / 'cancel' / 'error' listeners.
    rendering: false,
};

export function getScale0State() {
    return state;
}

export function recomputeAnyFieldActive() {
    state.anyFieldActive = FIELD_TOGGLE_KEYS.some((key) => !!state.fieldFlags[key]);
    return state.anyFieldActive;
}

export function resetFieldFlags() {
    state.fieldFlags = createFieldFlags();
    state.fieldNeedsUpdate = false;
    recomputeAnyFieldActive();
}

export function setFieldToggle(key, value) {
    if (!Object.prototype.hasOwnProperty.call(state.fieldFlags, key)) return;
    const prev = state.fieldFlags[key];
    const next = !!value;
    state.fieldFlags[key] = next;
    // Mark dirty on ANY state transition, not just on→off. When turning a
    // force off under `glyphs` style, `syncForceStyle` calls
    // `hideAllForceStyles` which resets every glyph mesh's instance count
    // to 0, then `showForceGlyphs({per-type})` re-shows the still-active
    // forces — but their meshes stay at count=0 until the next
    // `updateForceGlyphs` refill. That refill depends on `fieldNeedsUpdate`
    // being true (via the throttle gate in `updateFieldOverlays`). Without
    // dirtying on turn-off, disabling any force left every OTHER active
    // force's glyph mesh empty until an unrelated event nudged the dirty
    // flag — visually "turning off gravity" when the user toggled EM.
    if (prev !== next) state.fieldNeedsUpdate = true;
    recomputeAnyFieldActive();
}

export function getFieldStateSnapshot() {
    return {
        ...state.fieldFlags,
        anyFieldActive: state.anyFieldActive,
        fieldNeedsUpdate: state.fieldNeedsUpdate,
        fluxMock: state.fluxMock,
    };
}

export function setForceStyle(style) {
    state.forceStyle = style;
    state.fieldNeedsUpdate = true;
}

export function setFluxMock(mock, useMock = false) {
    state.fluxMock = mock;
    state.useFluxMock = !!useMock;
}

export function clearFluxMock() {
    state.fluxMock = null;
    state.useFluxMock = false;
}

export function setLatticeNeedsUpload(value = true) {
    state.latticeNeedsUpload = !!value;
}

export function markFieldDirty() {
    state.fieldNeedsUpdate = true;
}

export function resetFrameState() {
    state.fieldFrame = 0;
    state.fieldNeedsUpdate = false;
    state.latticeNeedsUpload = true;
    state.tickAccumulator.reset();
}

export function setCurrentScenarioId(id) {
    state.currentScenarioId = id || 'flux-pulse';
}
