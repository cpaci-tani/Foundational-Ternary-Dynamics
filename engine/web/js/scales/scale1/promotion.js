/**
 * Scale-0 → Scale-1 promotion pipeline ("⤴ Scale up").
 *
 * Captures the live lattice's manifested clusters (KnotTracker telemetry)
 * and promotes each cluster to one continuous particle:
 *
 *   position  = cluster centroid, re-centered to the PE origin frame
 *               (uniformly display-scaled for L ≥ 65 — [IMPOSED display
 *               mapping], velocities untouched)
 *   velocity  = cluster centroid velocity
 *   mass      = N · K_B   (engine convention, phase_forces cluster
 *               integrator; [DERIVED-linear]/[SMC], FTD-0110)
 *   charge    = sign · N, clamped to int8 ±127 (native field width)
 *
 * Admissibility is ANNOTATED, never gating: a JS heuristic of the
 * ScaleContextTracker criteria (r_eff/a ≥ 3 ⇒ N ≳ 113; sub-relativistic
 * centroid speed) tags each seed so the UI can mark which promoted objects
 * are legitimate scale-separated candidates vs. demo-sized fragments.
 *
 * The capture also grabs a voxel-level coarsenToParticles() snapshot
 * (one manifested voxel → one record) for the debug ghost layer.
 *
 * Worker-path notes (default path — Scale 0 usually runs off-thread):
 *   - all reads resolve the ACTIVE Scale-0 owner via the scale0 store
 *     selectors, never ctx.bridge (idle at tick 0 when the worker owns
 *     physics);
 *   - knot_tracking (default OFF) is enabled AT CAPTURE TIME, after any
 *     setupScenario — the C++-defaults rebuild on scenario load would
 *     clobber an earlier enable — and restored afterwards;
 *   - a paused sim is single-stepped (proxy stepScale0 / main tickScale0)
 *     so KnotTracker fills, then telemetry is polled with a timeout.
 *
 * Handoff: seeds are stashed in appRegistry under PROMOTION_SEEDS_TOKEN
 * (survives switchEngineMode's destroy/reset), the Scale-1 scenario select
 * is pointed at 's1-promoted-lattice', and the engine-mode select change is
 * dispatched — this module never imports app.js (CONTRACTS §3 Rule 1).
 */

import { K_B, C_SPEED } from '../../constants.js';
import { appRegistry } from '../../core/registry.js';
import {
    getActiveScale0Bridge, getActiveScale0Capability, getActiveLatticeSize,
} from '../scale0/state/store.js';

export const PROMOTION_SEEDS_TOKEN = 'scale1:promotionSeeds';
export const PROMOTED_SCENARIO_ID = 's1-promoted-lattice';

const KNOT_WAIT_MS = 600;
const KNOT_POLL_MS = 60;
// PE visual boundary is the r=35 reference shell; keep promoted centroids
// comfortably inside it.
const DISPLAY_HALF_EXTENT = 31.5;
const KNOT_FIELD_STRIDE = 11;   // cx,cy,cz, vx,vy,vz, fluxMag, fdx,fdy,fdz, org
const CHARGE_CLAMP = 127;       // native Particle.charge is int8

function _sleep(ms) {
    return new Promise(resolve => setTimeout(resolve, ms));
}

function _knotCount(knot) {
    if (!knot) return 0;
    return knot.count ?? knot.ids?.length ?? 0;
}

/**
 * Map a knot-telemetry snapshot to promotion seeds.
 * Exported for tests.
 */
export function clustersToSeeds(knot, latticeSize) {
    const n = _knotCount(knot);
    const center = (latticeSize - 1) / 2;
    const halfExtent = Math.max(center, 1);
    const displayScale = halfExtent > DISPLAY_HALF_EXTENT
        ? DISPLAY_HALF_EXTENT / halfExtent : 1;

    const seeds = [];
    for (let k = 0; k < n; k++) {
        const f = k * KNOT_FIELD_STRIDE;
        const size = knot.size?.[k] ?? 0;
        if (size <= 0) continue;
        const sign = knot.signs?.[k] ?? 0;
        const vx = knot.fields[f + 3];
        const vy = knot.fields[f + 4];
        const vz = knot.fields[f + 5];
        const speed = Math.sqrt(vx * vx + vy * vy + vz * vz);

        const rawCharge = sign * size;
        const charge = Math.max(-CHARGE_CLAMP, Math.min(CHARGE_CLAMP, rawCharge));

        // ScaleContextTracker heuristic (annotation only): equivalent-sphere
        // radius in lattice units vs. the R_eff/a >= 3 criterion, plus a
        // sub-relativistic centroid-speed proxy for stationarity.
        const rEff = Math.cbrt((3 * size) / (4 * Math.PI));
        const admissible = rEff >= 3 && speed < 0.5 * C_SPEED;

        seeds.push({
            clusterId: knot.ids?.[k] ?? k,
            position: [
                (knot.fields[f] - center) * displayScale,
                (knot.fields[f + 1] - center) * displayScale,
                (knot.fields[f + 2] - center) * displayScale,
            ],
            velocity: [vx, vy, vz],
            mass: size * K_B,
            charge,
            chargeClamped: charge !== rawCharge,
            size,
            org: knot.fields[f + 10],
            admissible,
            source: 'lattice',
        });
    }
    return { seeds, displayScale };
}

/**
 * Wrap a per-axis delta to the nearest periodic image: [-L/2, L/2).
 */
function _wrapDelta(d, L) {
    if (d > L / 2) return d - L;
    if (d < -L / 2) return d + L;
    return d;
}

/**
 * Build cluster seeds directly from a coarsenToParticles voxel snapshot:
 * Moore-26 connected components of same-sign manifested voxels on the
 * periodic lattice, no minimum size. Fallback for when KnotTracker
 * (min_cluster_size = 4) filters everything a demo-sized scenario
 * manifests. Same mapping conventions as clustersToSeeds; centroid
 * velocity is the member mean. Exported for tests.
 */
export function voxelsToClusters(coarsen, latticeSize) {
    const n = coarsen?.count ?? 0;
    if (n === 0) return { seeds: [], displayScale: 1 };

    // Occupancy by rounded (wrapped) integer coordinate for neighbor lookup.
    const L = latticeSize;
    const keyOf = (x, y, z) => ((x * L) + y) * L + z;
    const byKey = new Map();
    const ix = new Int32Array(n), iy = new Int32Array(n), iz = new Int32Array(n);
    for (let i = 0; i < n; i++) {
        const x = ((Math.round(coarsen.positions[i * 3]) % L) + L) % L;
        const y = ((Math.round(coarsen.positions[i * 3 + 1]) % L) + L) % L;
        const z = ((Math.round(coarsen.positions[i * 3 + 2]) % L) + L) % L;
        ix[i] = x; iy[i] = y; iz[i] = z;
        byKey.set(keyOf(x, y, z), i);
    }

    const visited = new Uint8Array(n);
    const clusters = [];
    for (let s = 0; s < n; s++) {
        if (visited[s]) continue;
        const sign = Math.sign(coarsen.charges[s]);
        const members = [];
        const stack = [s];
        visited[s] = 1;
        while (stack.length) {
            const i = stack.pop();
            members.push(i);
            for (let dx = -1; dx <= 1; dx++) {
                for (let dy = -1; dy <= 1; dy++) {
                    for (let dz = -1; dz <= 1; dz++) {
                        if (!dx && !dy && !dz) continue;
                        const j = byKey.get(keyOf(
                            (ix[i] + dx + L) % L,
                            (iy[i] + dy + L) % L,
                            (iz[i] + dz + L) % L));
                        if (j === undefined || visited[j]) continue;
                        if (Math.sign(coarsen.charges[j]) !== sign) continue;
                        visited[j] = 1;
                        stack.push(j);
                    }
                }
            }
        }
        clusters.push({ sign, members });
    }

    // Reduce each component to a knot-telemetry-shaped row, then reuse the
    // canonical mapping. Centroid unwraps periodically relative to the
    // anchor member so wrap-spanning clusters don't average across the box.
    const ids = new Int32Array(clusters.length);
    const signs = new Int8Array(clusters.length);
    const size = new Int32Array(clusters.length);
    const fields = new Float64Array(clusters.length * KNOT_FIELD_STRIDE);
    clusters.forEach((c, k) => {
        const a = c.members[0];
        let cx = 0, cy = 0, cz = 0, vx = 0, vy = 0, vz = 0;
        for (const i of c.members) {
            cx += coarsen.positions[a * 3] + _wrapDelta(coarsen.positions[i * 3] - coarsen.positions[a * 3], L);
            cy += coarsen.positions[a * 3 + 1] + _wrapDelta(coarsen.positions[i * 3 + 1] - coarsen.positions[a * 3 + 1], L);
            cz += coarsen.positions[a * 3 + 2] + _wrapDelta(coarsen.positions[i * 3 + 2] - coarsen.positions[a * 3 + 2], L);
            vx += coarsen.velocities[i * 3];
            vy += coarsen.velocities[i * 3 + 1];
            vz += coarsen.velocities[i * 3 + 2];
        }
        const m = c.members.length;
        ids[k] = k;
        signs[k] = c.sign;
        size[k] = m;
        const f = k * KNOT_FIELD_STRIDE;
        fields[f] = ((cx / m) % L + L) % L;
        fields[f + 1] = ((cy / m) % L + L) % L;
        fields[f + 2] = ((cz / m) % L + L) % L;
        fields[f + 3] = vx / m;
        fields[f + 4] = vy / m;
        fields[f + 5] = vz / m;
        // fluxMag/fdir/org unavailable from the voxel snapshot → 0.
    });

    return clustersToSeeds({ count: clusters.length, ids, signs, size, fields }, latticeSize);
}

/**
 * Engine-truth-only knot_tracking readback: true/false when known, null
 * when undetermined. Never trusts WasmBridgeProxy.getToggle's optimistic
 * unknown-toggle-is-ON default — acting on that guess is exactly how the
 * capture would silently skip enabling the tracker.
 */
function _engineTruthKnotToggle(owner) {
    if (typeof owner?.getEngineTruthToggle === 'function') {
        return owner.getEngineTruthToggle('knot_tracking');
    }
    if (owner?.isWasm && typeof owner.getToggle === 'function') {
        return !!owner.getToggle('knot_tracking');   // in-thread: real engine read
    }
    return null;
}

/**
 * Capture the live lattice's clusters + voxel snapshot from the active
 * Scale-0 owner. Cluster source: KnotTracker telemetry when it reports
 * anything (enriched: org, flux direction), else Moore-26 connected
 * components over the coarsenToParticles snapshot (covers clusters below
 * the tracker's min_cluster_size = 4). Returns the promotion payload, or
 * null when nothing is manifested.
 */
export async function captureLatticeClusters(ctx) {
    const owner = getActiveScale0Bridge(ctx);
    const caps = getActiveScale0Capability(ctx) ?? owner?.capabilities?.scale0;
    if (!owner || !caps) return null;

    const latticeSize = getActiveLatticeSize(ctx);

    const setToggle = caps.setToggle
        ? (k, v) => caps.setToggle(k, v)
        : (k, v) => owner.setToggle?.(k, v);
    const priorKnot = _engineTruthKnotToggle(owner);   // true | false | null

    const step = caps.stepScale0 || caps.tickScale0 || (() => owner.tick?.());

    try {
        if (priorKnot !== true) setToggle('knot_tracking', true);

        // KnotTracker records inside tick(); one step fills it even while
        // paused. On the worker path the step command also posts a fresh
        // frame, refreshing the proxy's knot snapshot.
        step();

        let knot = caps.getScale0KnotTelemetry?.() ?? owner.getKnotTelemetry?.() ?? null;
        const deadline = performance.now() + KNOT_WAIT_MS;
        while (_knotCount(knot) === 0 && performance.now() < deadline) {
            await _sleep(KNOT_POLL_MS);
            knot = caps.getScale0KnotTelemetry?.() ?? owner.getKnotTelemetry?.() ?? null;
        }

        // Voxel snapshot: debug ghost layer + cluster fallback.
        // Promise on the worker proxy, synchronous data on the in-thread bridge.
        let voxelDebug = null;
        try {
            voxelDebug = await Promise.resolve(owner.coarsenToParticles?.() ?? null);
        } catch {
            voxelDebug = null;
        }

        let clusterResult;
        let clusterSource;
        if (_knotCount(knot) > 0) {
            clusterResult = clustersToSeeds(knot, latticeSize);
            clusterSource = 'knot-tracker';
        } else {
            clusterResult = voxelsToClusters(voxelDebug, latticeSize);
            clusterSource = 'voxel-components';
        }
        if (clusterResult.seeds.length === 0) return null;

        return {
            seeds: clusterResult.seeds,
            voxelDebug,
            displayScale: clusterResult.displayScale,
            clusterSource,
            latticeSize,
            sourceTick: caps.getScale0Diagnostics?.()?.tick
                ?? owner.currentTick?.() ?? null,
            sourceScenario: (typeof document !== 'undefined')
                ? document.getElementById('scenario-select')?.value ?? null : null,
            capturedAt: null,   // display stamps this; keep the payload replay-safe
        };
    } finally {
        // Restore only when we know it was previously OFF (or undetermined —
        // the engine default). A confirmed-ON stays ON.
        if (priorKnot !== true) setToggle('knot_tracking', false);
    }
}

/**
 * The "⤴ Scale up" action: capture → stash → switch to Scale 1 with the
 * promoted-lattice scenario selected. Returns true if the switch happened.
 */
export async function scaleUpToParticles(ctx, { notify } = {}) {
    const payload = await captureLatticeClusters(ctx);
    if (!payload) {
        notify?.('No clusters manifested — nothing to promote. Run a genesis '
            + 'scenario (or let the current one evolve) and try again.');
        return false;
    }

    appRegistry.register(PROMOTION_SEEDS_TOKEN, payload);

    const scenarioSel = document.getElementById('pe-scenario-select');
    if (scenarioSel) scenarioSel.value = PROMOTED_SCENARIO_ID;

    const modeSel = document.getElementById('engine-mode');
    if (!modeSel) return false;
    modeSel.value = 'particles';
    modeSel.dispatchEvent(new Event('change', { bubbles: true }));
    return true;
}

/** Consume the stashed promotion payload (single-shot). */
export function takePromotionSeeds() {
    const payload = appRegistry.get(PROMOTION_SEEDS_TOKEN);
    if (payload) appRegistry.unregister(PROMOTION_SEEDS_TOKEN);
    return payload;
}

/** Non-consuming peek (promotion info card). */
export function peekPromotionSeeds() {
    return appRegistry.get(PROMOTION_SEEDS_TOKEN);
}
