/**
 * Scale-1 state store.
 *
 * Replaces the 14 module-level `let`s the old controller carried (audit
 * defect: state mutated from many places, unresettable per-instance,
 * leaking across scenario switches). One state object + one reset,
 * mirroring scales/scale0/state/store.js.
 */

import { createTickAccumulator } from '../../scale-utils.js';
import { DEFAULT_TRAIL_SETTINGS } from '../trail-settings.js?v=2';

function defaultOverlays() {
    return {
        efield: false,        // E-field streamlines
        potential: false,     // Coulomb potential heatmap
        fieldBattery: false,  // isolated FTD-0884 matched-face observer
        gravityField: false,  // gravity vectors (XZ grid)
        forceCoulomb: false,
        forceGravity: false,
        forceLorentz: false,
        forceExchange: false,
        forceStrong: false,
        forceRadiation: false,
        forceMagneticDipole: false,
        forceSpinOrbit: false,
        forceNet: false,      // net per-particle force arrows
        system: false,        // CoM + momentum + ang. mom.
        velocities: false,
        trails: false,
        admissibilityRing: false,
        provenanceLabel: false,
    };
}

export const scale1State = {
    mode: 'native_matter',
    registry: null,
    lastSnapshot: null,
    overlays: defaultOverlays(),
    trailSettings: { ...DEFAULT_TRAIL_SETTINGS },

    // Field computation cache
    fieldGrid: null,
    srcParticlesBuf: [],
    inspectionFieldSources: {},
    trailEnergyDensityById: new Map(),

    // Paused-state status-bar dedup cache
    statusCache: { tick: '', ptime: '', particles: '', energy: '', state: '' },

    // Sub-1-speed fractional tick accumulator
    tickAcc: createTickAccumulator(),

    // Native rows are synthesized strictly from shared snapshot provenance
    // and qualification fields (never from catalog identity).
    visualRecordById: new Map(),

    currentScenarioId: null,
    m3ViewId: 'anatomy',
    softening: 0.1,

    // Chart-push gating: last engine tick pushed to the hub ring buffers
    // (fixes the paused-sampling audit defect — charts advance on tick
    // progress, not wall-clock frames).
    lastPushedTick: -1,

    // Observation work is intentionally decoupled from RAF. Rendering still
    // consumes fresh O(N) positions each frame, while exact diagnostics,
    // hierarchy construction, event classification, and force serialization
    // share one load-aware snapshot cadence.
    observationDirty: true,
    lastObservationMs: Number.NEGATIVE_INFINITY,
    lastObservationTick: -1,
    lastObservationRevision: -1,
    lastObservationCount: -1,
    lastForceData: null,
    lastForceDecomposition: null,
    lastFieldSources: null,
    lastInspectionFocusKey: '',
    finitePortBatterySnapshot: null,
};

export function resetScale1State() {
    scale1State.overlays = defaultOverlays();
    scale1State.trailSettings = { ...DEFAULT_TRAIL_SETTINGS };
    scale1State.fieldGrid = null;
    scale1State.srcParticlesBuf.length = 0;
    scale1State.inspectionFieldSources = {};
    scale1State.trailEnergyDensityById.clear();
    scale1State.statusCache = { tick: '', ptime: '', particles: '', energy: '', state: '' };
    scale1State.tickAcc.reset();
    scale1State.lastPushedTick = -1;
    scale1State.observationDirty = true;
    scale1State.lastObservationMs = Number.NEGATIVE_INFINITY;
    scale1State.lastObservationTick = -1;
    scale1State.lastObservationRevision = -1;
    scale1State.lastObservationCount = -1;
    scale1State.lastForceData = null;
    scale1State.lastForceDecomposition = null;
    scale1State.lastFieldSources = null;
    scale1State.lastInspectionFocusKey = '';
    scale1State.finitePortBatterySnapshot = null;
    scale1State.visualRecordById.clear();
    scale1State.lastSnapshot = null;
    scale1State.softening = 0.1;
}
