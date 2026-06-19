import { createTickAccumulator } from '../../scale-utils.js';

export const FIELD_TOGGLE_KEYS = [
    // Visual-only overlay flags — toggling these affects rendering ONLY.
    // They never change physics toggles, tick cadence, or scenario physics.
    // Derived quantities are read from the active bridge's last-tick state.
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
    'showDarkMatterHalo',
    'showDampingZones',
    'showKnotZones',
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
    'showHorizon',        // L(x) ≥ 0.95             — event horizon isosurface
    // Tier 2 additions (2026-04-18) — stress-energy split
    'showEPressure',      // P_E = ½|E|²             — electric pressure
    'showBPressure',      // P_B = ½|B|²             — magnetic pressure
    // New substrate overlays (2026-06-03)
    'showStateField',     // s(x) ∈ {-1,0,+1}        — ternary manifestation field [AXIOM]
    'showLatency',        // L(x) = √(|J|²/|J|²max)  — time-dilation / Born-Infeld latency
    'showGaussResidual',  // ∇·J − s_charge          — Gauss-projection conservation leak
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

// "Prime tick on load" preference — when on, loadScale0Scenario runs exactly one
// physics tick right after seeding so that motion-derived field overlays (E, B,
// Poynting, vorticity, …) and particle/manifestation overlays (state, forces, …)
// have data to render at the initial paused view, instead of staying blank until
// the user presses Play. Persisted across sessions; toggled from the play bar.
const PRIME_TICK_PREF_KEY = 'ftd.scale0.primeTickOnLoad';
function readPrimeTickOnLoadPref() {
    try {
        if (typeof localStorage !== 'undefined') {
            const v = localStorage.getItem(PRIME_TICK_PREF_KEY);
            if (v !== null) return v === '1';
        }
    } catch { /* localStorage may be blocked (privacy mode) — fall through to default */ }
    return true; // default ON
}

const state = {
    currentScenarioId: 'flux-pulse',
    fieldFlags: createFieldFlags(),
    // Field-line knot tracking is NOT a visual overlay flag (it does not map to a
    // renderer toggle and must not count toward anyFieldActive), so it lives
    // outside fieldFlags. It gates the JS FieldLineKnotTracker.record() call in
    // the E-field overlay job. Survives resetFieldFlags() (scenario change).
    knotTracking: false,
    fieldFrame: 0,
    fieldNeedsUpdate: false,
    // Monotonic field-data version. Bumped once per real physics tick (tick.js);
    // the overlay sweep gate (field-overlays.js) compares it against the value
    // latched at the last sweep to decide whether the field actually changed.
    // Initialized to 0 (was previously `undefined` until the first tick, which
    // left the `version !== sched.lastVersion` gate in an ambiguous -1/0/undefined
    // state at scenario load).
    fieldDataVersion: 0,
    anyFieldActive: false,
    forceStyle: 'arrows',
    fluxMock: null,
    useFluxMock: false,
    primeTickOnLoad: readPrimeTickOnLoadPref(),
    latticeNeedsUpload: true,
    tickAccumulator: createTickAccumulator(),
    fieldParticleBuf: [],
    dualLVecs: null,
    dualRVecs: null,
    chiralValues: null,
    weakValues: null,
    weakVectors: null,
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

// Enable/disable the JS field-line knot tracker. Read by the E-field overlay job
// in field-overlays.js. Dirties the overlay so the next sweep records even if no
// other field state changed.
export function setKnotTracking(on) {
    state.knotTracking = !!on;
    state.fieldNeedsUpdate = true;
}

export function setForceStyle(style) {
    state.forceStyle = style;
    state.fieldNeedsUpdate = true;
}

export function getPrimeTickOnLoad() {
    return state.primeTickOnLoad;
}

// Toggle the "prime tick on load" preference and persist it. Read by
// loadScale0Scenario; the play-bar button reflects the returned value.
export function setPrimeTickOnLoad(on) {
    state.primeTickOnLoad = !!on;
    try {
        if (typeof localStorage !== 'undefined') {
            localStorage.setItem(PRIME_TICK_PREF_KEY, state.primeTickOnLoad ? '1' : '0');
        }
    } catch { /* ignore persistence failure — preference stays in-memory only */ }
    return state.primeTickOnLoad;
}

export function setFluxMock(mock, useMock = false) {
    // Dispose the prior mock before overwriting. Prior to this, scenario
    // churn leaked every previous MockBridge for the page lifetime —
    // ~21 MB of typed arrays each at L=96. dispose() is idempotent and
    // a no-op for non-MockBridge values.
    const prev = state.fluxMock;
    if (prev && prev !== mock && typeof prev.dispose === 'function') {
        try { prev.dispose(); } catch { /* defensive: never block scenario load on cleanup */ }
    }
    state.fluxMock = mock;
    state.useFluxMock = !!useMock;
}

export function clearFluxMock() {
    const prev = state.fluxMock;
    if (prev && typeof prev.dispose === 'function') {
        try { prev.dispose(); } catch { /* defensive */ }
    }
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
    state.fieldDataVersion = 0;
    state.latticeNeedsUpload = true;
    state.tickAccumulator.reset();
    // Drop cached streamline seeds so a new scenario/field never reuses stale
    // (wrong-field) seeds — they're keyed on fieldDataVersion, which a new
    // scenario may reset to 0 and collide with the previous field's cache.
    state.streamlineSeedCache = null;
}

export function setCurrentScenarioId(id) {
    state.currentScenarioId = id || 'flux-pulse';
}

/** Bridge that owns live Scale-0 physics (mock when useFluxMock). */
export function getActiveScale0Bridge(ctx, st = state) {
    if (st?.useFluxMock && st?.fluxMock) return st.fluxMock;
    return ctx?.bridge ?? null;
}

/** scale0 capability on the active physics owner. */
export function getActiveScale0Capability(ctx, st = state) {
    return getActiveScale0Bridge(ctx, st)?.capabilities?.scale0 ?? null;
}

/** Panel/runtime helper: resolve live physics owner from window.__ftdCtx + store. */
export function resolveActiveScale0BridgeFromWindow() {
    const ctx = (typeof window !== 'undefined') ? window.__ftdCtx : null;
    return getActiveScale0Bridge(ctx, state);
}

/** Lattice N for the bridge that owns live Scale-0 physics (mock when active). */
export function getActiveLatticeSize(ctx, st = state) {
    const active = getActiveScale0Bridge(ctx, st);
    return active?.latticeSize ?? ctx?.bridge?.latticeSize ?? 33;
}
