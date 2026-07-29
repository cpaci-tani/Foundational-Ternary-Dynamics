/**
 * Scale-1 state store.
 *
 * Replaces the 14 module-level `let`s the old controller carried (audit
 * defect: state mutated from many places, unresettable per-instance,
 * leaking across scenario switches). One state object + one reset,
 * mirroring scales/scale0/state/store.js.
 */

import { createTickAccumulator } from '../../scale-utils.js';

function defaultOverlays() {
    return {
        efield: false,        // E-field streamlines
        potential: false,     // Coulomb potential heatmap
        gravityField: false,  // gravity vectors (XZ grid)
        forceCoulomb: false,
        forceGravity: false,
        forceStrong: false,
        forceNet: false,      // net per-particle force arrows
        system: false,        // CoM + momentum + ang. mom.
        velocities: false,
        trails: false,
        voxelDebug: false,    // promotion-source ghost layer
    };
}

export const scale1State = {
    overlays: defaultOverlays(),

    // Field computation cache
    fieldGrid: null,
    srcParticlesBuf: [],

    // Paused-state status-bar dedup cache
    statusCache: { tick: '', ptime: '', particles: '', energy: '', state: '' },

    // Sub-1-speed fractional tick accumulator
    tickAcc: createTickAccumulator(),

    // Last consumed promotion payload (kept after the single-shot registry
    // take so s1-voxel-debug and the promotion info card can re-read it).
    lastPromotion: null,

    currentScenarioId: null,

    // Chart-push gating: last engine tick pushed to the hub ring buffers
    // (fixes the paused-sampling audit defect — charts advance on tick
    // progress, not wall-clock frames).
    lastPushedTick: -1,
};

export function resetScale1State() {
    scale1State.overlays = defaultOverlays();
    scale1State.fieldGrid = null;
    scale1State.srcParticlesBuf.length = 0;
    scale1State.statusCache = { tick: '', ptime: '', particles: '', energy: '', state: '' };
    scale1State.tickAcc.reset();
    scale1State.lastPushedTick = -1;
    // lastPromotion and currentScenarioId survive a visual reset on purpose:
    // the promotion payload is consumed/replaced by scenario loads, not by
    // mode-switch cache resets.
}
