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
    'showColorCharge',    // recolours manifested particles by colour charge — was
                          // bound in dom.js + dispatched by the adapter but MISSING
                          // here, so setFieldToggle silently no-op'd it (dead toggle).
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

// Shared presentation controls for the three RK4 streamline channels. Density
// and length are capped at 1 because 1 is the audited per-size 60 FPS work
// envelope; opacity is renderer-only and can use the full visible range.
export const DEFAULT_FLOW_LINE_SETTINGS = Object.freeze({
    density: 1,
    length: 1,
    opacity: 0.7,
});
export const FLOW_LINE_SETTING_LIMITS = Object.freeze({
    density: Object.freeze([0.25, 1]),
    length: Object.freeze([0.4, 1]),
    opacity: Object.freeze([0.2, 1]),
});
const FLOW_LINE_PREF_KEY = 'ftd.scale0.flowLines';

export const SCALE0_MUTATION_REASONS = Object.freeze({
    INJECT_PARTICLE: 'inject-particle',
    INJECT_WAVEPACKET: 'inject-wavepacket',
    INJECT_FLUX: 'inject-flux',
    INJECT_PAIR: 'inject-pair',
    CLEAR_FIELD: 'clear-field',
    RANDOM_FLUX: 'random-flux',
    PHYSICS_TOGGLE: 'physics-toggle',
    PARAMETER_CHANGE: 'parameter-change',
    FLUX_BOUNDARY: 'flux-boundary',
    WAVE_LAB_RESEED: 'wave-lab-reseed',
    GENESIS_EXPERIMENT: 'genesis-experiment',
});

export const SCALE0_MUTATION_SOURCES = Object.freeze({
    SUBSTRATE_CONTROLS: 'controls.substrate',
    PHYSICS_TOGGLES: 'controls.physics-toggles',
    TOOLBAR_BOUNDARY: 'toolbar.boundary',
    THERMODYNAMICS: 'panel.thermodynamics',
    P1_FINE_STRUCTURE: 'panel.p1.fine-structure',
    P1_THOMSON: 'panel.p1.thomson',
    WAVE_LAB: 'panel.wave-lab',
    GENESIS_BURST: 'panel.genesis-burst',
});

const SCALE0_MUTATION_REASON_SET = new Set(Object.values(SCALE0_MUTATION_REASONS));
const SCALE0_MUTATION_SOURCE_SET = new Set(Object.values(SCALE0_MUTATION_SOURCES));
const qualificationListeners = new Set();

function clampFlowLineSetting(key, value) {
    const range = FLOW_LINE_SETTING_LIMITS[key];
    if (!range) return null;
    const numeric = Number(value);
    if (!Number.isFinite(numeric)) return DEFAULT_FLOW_LINE_SETTINGS[key];
    return Math.max(range[0], Math.min(range[1], numeric));
}

function readFlowLineSettings() {
    const settings = { ...DEFAULT_FLOW_LINE_SETTINGS };
    try {
        if (typeof localStorage !== 'undefined') {
            const saved = JSON.parse(localStorage.getItem(FLOW_LINE_PREF_KEY) || 'null');
            if (saved && typeof saved === 'object') {
                for (const key of Object.keys(settings)) {
                    settings[key] = clampFlowLineSetting(key, saved[key]);
                }
            }
        }
    } catch { /* malformed or blocked storage — use audited defaults */ }
    return settings;
}

const state = {
    currentScenarioId: 'flux-pulse',
    // Monotonic provenance for user/experiment scientific write intents. It is
    // deliberately never reset by scenario loads; a successful authoritative
    // load moves the qualification anchor to the current epoch instead.
    mutationEpoch: 0,
    qualificationAnchor: null,
    authoritativeLoad: null,
    lastScientificMutation: null,
    fieldFlags: createFieldFlags(),
    // Field-line knot tracking is NOT a visual overlay flag (it does not map to a
    // renderer toggle and must not count toward anyFieldActive), so it lives
    // outside fieldFlags. `knotTracking` is the retained user preference;
    // `knotTrackingApplicable` is the scenario/runtime gate. The effective
    // conjunction gates FieldLineKnotTracker.record() and E/B/flux streamline
    // jobs. This preserves the preference across an inapplicable scenario
    // without spending work or manufacturing an observation there.
    knotTracking: false,
    knotTrackingApplicable: true,
    // Knot-zone boxes have a retained user preference and a separate effective
    // renderer flag. An inapplicable/pending scenario must hide the boxes
    // without erasing what the user asked to see when a qualified scenario is
    // restored. `fieldFlags.showKnotZones` is always the effective value.
    knotZonesRequested: false,
    knotZonesApplicable: true,
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
    // 'default' | 'heatmap' — overlays-panel "Heat Map" meta-toggle. When 'heatmap'
    // the volumetric scalar overlays (EM energy, pressures, charge, vorticity, Φ,
    // |ψ|², Lagrangian, entropy, latency, Gauss residual) render as thermal glow
    // clouds instead of their default rubber-sheet / native scalar cloud.
    scalarRenderMode: 'default',
    // Off-thread WASM Scale-0 owner (WasmBridgeProxy). Legacy names from the
    // MockBridge era — prefer getWasmWorker() / isUsingWasmWorker() accessors.
    fluxMock: null,
    useFluxMock: false,
    primeTickOnLoad: readPrimeTickOnLoadPref(),
    flowLineSettings: readFlowLineSettings(),
    flowLineSettingsVersion: 0,
    latticeNeedsUpload: true,
    tickAccumulator: createTickAccumulator(),
    playbackOwner: null,
    requestedRunning: null,
    requestedSpeed: null,
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

function finiteTick(value) {
    const tick = Number(value);
    return Number.isFinite(tick) ? tick : null;
}

function ownerTick(owner) {
    try {
        return finiteTick(owner?.getDiagnostics?.()?.tick);
    } catch {
        return null;
    }
}

function cloneRecord(record) {
    return record ? { ...record } : null;
}

export function getScale0QualificationState() {
    const anchor = state.qualificationAnchor;
    const load = state.authoritativeLoad;
    const anchorMatches = !!anchor
        && anchor.mutationEpoch === state.mutationEpoch
        && anchor.scenarioId === state.currentScenarioId;
    let status = 'suspended';
    if (load?.status === 'pending') status = 'pending';
    else if (!load && anchorMatches) status = 'within-contract';
    return Object.freeze({
        status,
        suspended: status !== 'within-contract',
        scenarioId: state.currentScenarioId,
        mutationEpoch: state.mutationEpoch,
        qualificationBaselineEpoch: anchor?.mutationEpoch ?? null,
        anchor: cloneRecord(anchor),
        authoritativeLoad: cloneRecord(load),
        lastMutation: cloneRecord(state.lastScientificMutation),
    });
}

function publishQualificationState() {
    const snapshot = getScale0QualificationState();
    for (const listener of qualificationListeners) {
        try { listener(snapshot); } catch (error) {
            console.error('[Scale0] qualification listener failed:', error);
        }
    }
    return snapshot;
}

export function subscribeScale0Qualification(listener) {
    if (typeof listener !== 'function') return () => {};
    qualificationListeners.add(listener);
    try { listener(getScale0QualificationState()); } catch (error) {
        console.error('[Scale0] qualification listener failed:', error);
    }
    return () => qualificationListeners.delete(listener);
}

export function beginScale0AuthoritativeLoad({ scenarioId, loadGeneration, tick = null } = {}) {
    const generation = Number(loadGeneration);
    if (!Number.isInteger(generation) || generation < 0) {
        throw new TypeError('Scale 0 authoritative load requires a non-negative integer generation');
    }
    state.authoritativeLoad = Object.freeze({
        status: 'pending',
        scenarioId: String(scenarioId || state.currentScenarioId),
        loadGeneration: generation,
        mutationEpochAtStart: state.mutationEpoch,
        tick: finiteTick(tick),
    });
    return publishQualificationState();
}

export function completeScale0AuthoritativeLoad({
    scenarioId,
    loadGeneration,
    tick = null,
    source = 'scenario-loader',
} = {}) {
    const pending = state.authoritativeLoad;
    const id = String(scenarioId || state.currentScenarioId);
    const generation = Number(loadGeneration);
    if (!pending || pending.status !== 'pending'
        || pending.scenarioId !== id
        || pending.loadGeneration !== generation
        || state.currentScenarioId !== id
        || state.mutationEpoch !== pending.mutationEpochAtStart) {
        return false;
    }
    state.qualificationAnchor = Object.freeze({
        scenarioId: id,
        loadGeneration: generation,
        mutationEpoch: state.mutationEpoch,
        tick: finiteTick(tick),
        source: String(source || 'scenario-loader'),
    });
    state.authoritativeLoad = null;
    publishQualificationState();
    return true;
}

export function failScale0AuthoritativeLoad({ scenarioId, loadGeneration, reason = 'setup-failed' } = {}) {
    const pending = state.authoritativeLoad;
    const id = String(scenarioId || state.currentScenarioId);
    const generation = Number(loadGeneration);
    if (!pending || pending.scenarioId !== id || pending.loadGeneration !== generation) return false;
    state.authoritativeLoad = Object.freeze({
        ...pending,
        status: 'failed',
        failureReason: String(reason || 'setup-failed'),
    });
    publishQualificationState();
    return true;
}

export function recordScale0ScientificMutation({
    reason,
    source,
    tick = null,
    loadGeneration,
    dispatchStatus = 'unknown',
} = {}) {
    if (!SCALE0_MUTATION_REASON_SET.has(reason)) {
        throw new TypeError(`Unknown Scale 0 scientific mutation reason: ${reason}`);
    }
    if (!SCALE0_MUTATION_SOURCE_SET.has(source)) {
        throw new TypeError(`Unknown Scale 0 scientific mutation source: ${source}`);
    }
    const generation = Number(loadGeneration);
    if (!Number.isInteger(generation) || generation < 0) {
        throw new TypeError('Scale 0 scientific mutation requires a non-negative integer generation');
    }
    state.mutationEpoch += 1;
    state.lastScientificMutation = Object.freeze({
        mutationEpoch: state.mutationEpoch,
        reason,
        source,
        tick: finiteTick(tick),
        loadGeneration: generation,
        // Dashboard dispatch is fire-and-forget on several transports. Never
        // relabel an accepted UI intent as an acknowledged engine mutation.
        dispatchStatus: ['dispatched', 'rejected', 'unknown'].includes(dispatchStatus)
            ? dispatchStatus
            : 'unknown',
    });
    if (state.authoritativeLoad?.status === 'pending') {
        state.authoritativeLoad = Object.freeze({
            ...state.authoritativeLoad,
            status: 'invalidated',
            invalidatedByMutationEpoch: state.mutationEpoch,
        });
    }
    return publishQualificationState();
}

/**
 * Dashboard gateway for a manual/experiment scientific write intent.
 *
 * A stale generation, inactive scale, or stale owner is rejected before the
 * callback runs and therefore cannot suspend the current record. An accepted
 * callback increments the monotonic epoch exactly once even for an idempotent
 * engine operation such as clearField() on an already-null lattice. Because
 * bridge writes are not uniformly acknowledged, the provenance records the
 * dispatch status rather than claiming that the engine applied the write.
 */
export function commitScale0ScientificMutation(ctx, {
    reason,
    source,
    loadGeneration = ctx?._loadGeneration,
    owner = null,
    dispatchStatus = 'unknown',
} = {}, mutate) {
    const generation = Number(loadGeneration);
    const currentGeneration = Number(ctx?._loadGeneration || 0);
    const activeOwner = getActiveScale0Bridge(ctx, state);
    if ((ctx?.engineMode && ctx.engineMode !== 'lattice')
        || !SCALE0_MUTATION_REASON_SET.has(reason)
        || !SCALE0_MUTATION_SOURCE_SET.has(source)
        || !Number.isInteger(generation)
        || generation < 0
        || generation !== currentGeneration
        || !activeOwner
        || (owner && owner !== activeOwner)
        || typeof mutate !== 'function') {
        return Object.freeze({ accepted: false, dispatchStatus: 'rejected' });
    }

    const normalizedDispatchStatus = ['dispatched', 'rejected', 'unknown'].includes(dispatchStatus)
        ? dispatchStatus
        : 'unknown';

    let result;
    let thrown = null;
    try {
        result = mutate(activeOwner);
    } catch (error) {
        thrown = error;
    } finally {
        recordScale0ScientificMutation({
            reason,
            source,
            tick: ownerTick(activeOwner),
            loadGeneration: generation,
            dispatchStatus: thrown ? 'unknown' : normalizedDispatchStatus,
        });
    }
    if (thrown) throw thrown;
    return Object.freeze({
        accepted: true,
        dispatchStatus: normalizedDispatchStatus,
        result,
        qualification: getScale0QualificationState(),
    });
}

export function recomputeAnyFieldActive() {
    state.anyFieldActive = FIELD_TOGGLE_KEYS.some((key) => !!state.fieldFlags[key]);
    return state.anyFieldActive;
}

export function resetFieldFlags() {
    state.fieldFlags = createFieldFlags();
    state.fieldNeedsUpdate = false;
    // Re-establish retained/effective invariants after replacing the flag bag.
    // This is intentionally store-owned: scenario loads and headless runs must
    // not depend on a mounted Knots panel to restore a qualified user request.
    syncKnotZonesEffective();
}

export function setFieldToggle(key, value) {
    if (!Object.prototype.hasOwnProperty.call(state.fieldFlags, key)) return;
    // Keep every knot-zone write on the desired/effective path. This prevents a
    // future generic caller from bypassing Empty/pending applicability.
    if (key === 'showKnotZones') {
        setKnotZonesRequested(value);
        return;
    }
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
    return syncKnotZonesEffective();
}

export function setKnotTrackingApplicability(on) {
    const next = !!on;
    if (state.knotTrackingApplicable !== next) {
        state.knotTrackingApplicable = next;
        state.fieldNeedsUpdate = true;
    }
    return syncKnotZonesEffective();
}

export function isKnotTrackingActive(snapshot = state) {
    // Scientific applicability must fail closed in the runtime, not depend on
    // the optional Knots UI being mounted to flip a flag. This also covers
    // headless runs and the short interval between scenario commit and panel
    // reconciliation.
    return snapshot?.currentScenarioId !== 'empty'
        && !!snapshot?.knotTracking
        && snapshot?.knotTrackingApplicable !== false;
}

export function isKnotZonesActive(snapshot = state) {
    return snapshot?.currentScenarioId !== 'empty'
        && isKnotTrackingActive(snapshot)
        && !!snapshot?.knotZonesRequested
        && snapshot?.knotZonesApplicable !== false;
}

function syncKnotZonesEffective() {
    const prev = !!state.fieldFlags.showKnotZones;
    const next = isKnotZonesActive(state);
    state.fieldFlags.showKnotZones = next;
    if (prev !== next) state.fieldNeedsUpdate = true;
    recomputeAnyFieldActive();
    return next;
}

export function setKnotZonesRequested(on) {
    state.knotZonesRequested = !!on;
    return syncKnotZonesEffective();
}

export function setKnotZonesApplicability(on) {
    state.knotZonesApplicable = !!on;
    return syncKnotZonesEffective();
}

export function setForceStyle(style) {
    state.forceStyle = style;
    state.fieldNeedsUpdate = true;
}

export function getScalarRenderMode() {
    return state.scalarRenderMode;
}

export function setScalarRenderMode(mode) {
    state.scalarRenderMode = (mode === 'heatmap') ? 'heatmap' : 'default';
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

/** Stable settings object; mutate only through setFlowLineSetting(). */
export function getFlowLineSettings() {
    return state.flowLineSettings;
}

export function setFlowLineSetting(key, value) {
    const next = clampFlowLineSetting(key, value);
    if (next === null) return false;
    if (state.flowLineSettings[key] === next) return false;
    state.flowLineSettings[key] = next;
    try {
        if (typeof localStorage !== 'undefined') {
            localStorage.setItem(FLOW_LINE_PREF_KEY, JSON.stringify(state.flowLineSettings));
        }
    } catch { /* preference remains in memory */ }

    // Opacity writes one material property and never invalidates integration.
    // Density/length change geometry, so discard stochastic seeds and preempt
    // any in-flight multi-frame sweep through the established dirty gate.
    if (key !== 'opacity') {
        state.streamlineSeedCache = null;
        state.flowLineSettingsVersion += 1;
        state.fieldNeedsUpdate = true;
    }
    return true;
}

export function resetFlowLineSettings() {
    let changed = false;
    for (const [key, value] of Object.entries(DEFAULT_FLOW_LINE_SETTINGS)) {
        changed = setFlowLineSetting(key, value) || changed;
    }
    return changed;
}

export function setFluxMock(mock, useMock = false) {
    // Dispose the prior worker/proxy before overwriting. Scenario churn used
    // to leak every previous owner for the page lifetime.
    const prev = state.fluxMock;
    if (prev && prev !== mock && typeof prev.dispose === 'function') {
        try { prev.dispose(); } catch { /* defensive: never block scenario load on cleanup */ }
    }
    state.fluxMock = mock;
    state.useFluxMock = !!useMock;
    state.playbackOwner = null;
    state.requestedRunning = null;
    state.requestedSpeed = null;
}

/** Canonical alias: off-thread WASM Scale-0 owner (WasmBridgeProxy). */
export function setWasmWorker(worker, enabled = false) {
    setFluxMock(worker, enabled);
}

export function getWasmWorker() {
    return state.fluxMock;
}

export function isUsingWasmWorker() {
    return !!state.useFluxMock;
}

export function clearFluxMock() {
    const prev = state.fluxMock;
    if (prev && typeof prev.dispose === 'function') {
        try { prev.dispose(); } catch { /* defensive */ }
    }
    state.fluxMock = null;
    state.useFluxMock = false;
    state.playbackOwner = null;
    state.requestedRunning = null;
    state.requestedSpeed = null;
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
    // Drop the per-overlay peak-hold normalizer histories (overlay-frames.js
    // updateDecayingMax). Without this a strong→weak scenario switch normalizes
    // the new field against the stale peak, so EM-energy/pressure/vorticity
    // clouds render washed-out and the horizon overlay stays blank for ~7 s.
    state.decayingMax = null;
    // Drop cached streamline seeds so a new scenario/field never reuses stale
    // (wrong-field) seeds — they're keyed on fieldDataVersion, which a new
    // scenario may reset to 0 and collide with the previous field's cache.
    state.streamlineSeedCache = null;
}

export function setCurrentScenarioId(id) {
    state.currentScenarioId = id || 'flux-pulse';
    // The store, rather than an optional mounted panel, owns the final Empty
    // fail-closed boundary. On a committed nonempty load this also restores a
    // retained request once the panel/runtime applicability gate is open.
    syncKnotZonesEffective();
}

/** Bridge that owns live Scale-0 physics (WASM worker when useFluxMock). */
export function getActiveScale0Bridge(ctx, st = state) {
    if (st?.useFluxMock && st?.fluxMock) return st.fluxMock;
    return ctx?.bridge ?? null;
}

function syncPlaybackOwner(owner, st = state) {
    if (st.playbackOwner === owner) return;
    st.playbackOwner = owner;
    st.requestedRunning = null;
    st.requestedSpeed = null;
}

/** Send worker run-state only when the requested value or owner changes. */
export function setScale0PlaybackRunning(ctx, running, st = state) {
    const owner = getActiveScale0Bridge(ctx, st);
    syncPlaybackOwner(owner, st);
    if (!st.useFluxMock || owner !== st.fluxMock || typeof owner?.setRunning !== 'function') return false;
    const next = !!running;
    if (st.requestedRunning === next) return false;
    st.requestedRunning = next;
    owner.setRunning(next);
    return true;
}

/** Send playback speed once per owner/value transition. */
export function setScale0PlaybackSpeed(ctx, speed, st = state) {
    const owner = getActiveScale0Bridge(ctx, st);
    syncPlaybackOwner(owner, st);
    const next = Number(speed);
    if (!Number.isFinite(next) || next <= 0 || typeof owner?.setTicksPerFrame !== 'function') return false;
    if (st.requestedSpeed === next) return false;
    st.requestedSpeed = next;
    owner.setTicksPerFrame(next);
    return true;
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
