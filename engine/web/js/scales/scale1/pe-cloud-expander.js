/**
 * Scale 1 — PE Cloud Expander
 * ────────────────────────────────────────────────────────────────────
 *
 * Owns the Gaussian-cloud rendering buffers, per-type template cache,
 * per-particle trail-history circular buffers, and the expansion routine
 * that turns PE particle centers into a colored, breathing point cloud
 * for the viewport renderer.
 *
 * Extracted verbatim from scales/scale1/controller.js (ticket S1-2).
 * No behavioral changes — this is a pure lift.
 *
 * EXPORTS:
 *   ensureCloudTemplate(catalogId, mass_mev)
 *     Get (or build) the Gaussian offset/brightness template for a
 *     particle type. Lazy-allocated; cached by catalog ID.
 *
 *   expandPEToCloud(peData, typeMap, t)
 *     Expand particle centers into a cloud of N points per particle
 *     with per-frame breathing motion. Returns the module-level
 *     pre-allocated buffers (zero-copy).
 *
 *   updateTrailHistory(peData)
 *     Record current positions into per-particle circular buffers and
 *     prune trails for particles that no longer exist.
 *
 *   getCloudParticleMap()
 *     Read-only access to the cloud-index → PE particle ID mapping.
 *     Used by Inspector for click-to-inspect.
 *
 *   getTrailHistory()
 *     Read-only access to the per-particle trail Map. Used by the
 *     viewport's updateTrails() call.
 *
 *   clearCloudAndTrails()
 *     Reset the template cache and trail history. Called from
 *     resetScale1() on mode switch.
 *
 *   MAX_CLOUD_TOTAL, TRAIL_MAX_LENGTH
 *     Exported constants for tests and external consumers.
 */

import { getById } from '../../particle-catalog.js';
import { K_B } from '../../constants.js';


// =====================================================================
// Cloud rendering buffers (pre-allocated, reused every frame)
// =====================================================================
// Each PE particle is rendered as a Gaussian flux cloud, not a point.
// Cloud point count ~ mass (electron 0.511 MeV -> 511 cloud points).
export const MAX_CLOUD_TOTAL = 100000;
const _cloudPos  = new Float32Array(MAX_CLOUD_TOTAL * 3);
const _cloudCol  = new Float32Array(MAX_CLOUD_TOTAL * 3);
const _cloudSize = new Float32Array(MAX_CLOUD_TOTAL);
const _cloudParticleMap = new Int32Array(MAX_CLOUD_TOTAL); // cloud index -> PE particle ID

// -- Cloud template cache (one per particle catalog ID) ---------------
const _cloudTemplates = new Map();

// -- Trail history (circular buffers per particle) --------------------
export const TRAIL_MAX_LENGTH = 200;
const _trailHistory = new Map(); // particleId -> { positions: Float32Array, head, length }


// =====================================================================
// ensureCloudTemplate
// =====================================================================

/**
 * Generate (or retrieve cached) a Gaussian cloud template for a given
 * particle type.  Point count scales sub-linearly with mass so heavier
 * particles get denser clouds without blowing the budget.
 *
 * Template fields: { n, radius, offsets: Float32Array, brightness: Float32Array }
 *
 * Originally app_dag.js lines ~416-453.
 */
export function ensureCloudTemplate(catalogId, mass_mev) {
    if (_cloudTemplates.has(catalogId)) return _cloudTemplates.get(catalogId);

    // Point count: electron (0.511 MeV) -> 511 pts; proton (938) -> ~3000
    const nRaw = Math.round(603 * Math.pow(mass_mev, 0.238));
    const n = Math.min(Math.max(nRaw, 50), 5000);

    // Cloud radius: lighter particles are more spread out (Compton-like)
    const radius = 2.0 + 3.0 * Math.pow(K_B / mass_mev, 0.15);
    const sigma = radius / 2.5; // ~95% within radius

    const offsets    = new Float32Array(n * 3);
    const brightness = new Float32Array(n);

    for (let i = 0; i < n; i++) {
        // Box-Muller for 3D Gaussian
        const u1 = Math.random() || 1e-10, u2 = Math.random();
        const u3 = Math.random() || 1e-10, u4 = Math.random();
        const sq1 = Math.sqrt(-2 * Math.log(u1));
        const sq3 = Math.sqrt(-2 * Math.log(u3));

        const ox = sq1 * Math.cos(2 * Math.PI * u2) * sigma;
        const oy = sq1 * Math.sin(2 * Math.PI * u2) * sigma;
        const oz = sq3 * Math.cos(2 * Math.PI * u4) * sigma;

        offsets[i * 3]     = ox;
        offsets[i * 3 + 1] = oy;
        offsets[i * 3 + 2] = oz;

        const dist = Math.sqrt(ox * ox + oy * oy + oz * oz) / radius;
        brightness[i] = Math.exp(-dist * dist * 2.0); // Gaussian falloff
    }

    const tmpl = { n, radius, offsets, brightness };
    _cloudTemplates.set(catalogId, tmpl);
    return tmpl;
}


// =====================================================================
// expandPEToCloud
// =====================================================================

/**
 * Expand PE particle centers into flux cloud point data suitable for
 * the viewport point cloud renderer.
 *
 * Each particle is replaced by N Gaussian-distributed cloud points with
 * per-frame sinusoidal "breathing" motion for organic visual quality.
 *
 * Returns { positions, colors, sizes, count } referencing the module-level
 * pre-allocated buffers (zero-copy for the viewport).
 *
 * Originally app_dag.js lines ~455-510.
 */
export function expandPEToCloud(peData, typeMap, t) {
    const srcCount = peData.count;
    let out = 0;

    for (let i = 0; i < srcCount && out < MAX_CLOUD_TOTAL; i++) {
        const cx = peData.positions[i * 3];
        const cy = peData.positions[i * 3 + 1];
        const cz = peData.positions[i * 3 + 2];

        const pid   = peData.ids ? peData.ids[i] : -1;
        const catId = typeMap ? typeMap.get(pid) : null;
        const p     = catId ? getById(catId) : null;

        if (p) {
            const tmpl = ensureCloudTemplate(catId, p.mass_mev);
            const [cr, cg, cb] = p.display_color;
            const n = Math.min(tmpl.n, MAX_CLOUD_TOTAL - out);
            const wiggle = 0.15 * tmpl.radius; // 15% of cloud radius

            for (let j = 0; j < n; j++) {
                // Per-point sinusoidal perturbation for organic "breathing" motion.
                // Golden angle phase spacing ensures adjacent points move independently.
                const phase = j * 2.39996323;
                const fx = Math.sin(t * 1.7 + phase) * wiggle;
                const fy = Math.sin(t * 2.3 + phase * 1.3) * wiggle;
                const fz = Math.sin(t * 1.1 + phase * 0.7) * wiggle;

                _cloudPos[out * 3]     = cx + tmpl.offsets[j * 3]     + fx;
                _cloudPos[out * 3 + 1] = cy + tmpl.offsets[j * 3 + 1] + fy;
                _cloudPos[out * 3 + 2] = cz + tmpl.offsets[j * 3 + 2] + fz;

                const b = tmpl.brightness[j];
                _cloudCol[out * 3]     = cr * b;
                _cloudCol[out * 3 + 1] = cg * b;
                _cloudCol[out * 3 + 2] = cb * b;

                _cloudSize[out] = 1.5 + b * 1.5; // 1.5 at edge -> 3.0 at center
                _cloudParticleMap[out] = pid;
                out++;
            }
        } else {
            // Fallback: single point for untyped particles
            _cloudPos[out * 3]     = cx;
            _cloudPos[out * 3 + 1] = cy;
            _cloudPos[out * 3 + 2] = cz;
            _cloudCol[out * 3]     = 0.5;
            _cloudCol[out * 3 + 1] = 0.5;
            _cloudCol[out * 3 + 2] = 0.5;
            _cloudSize[out] = 3.0;
            _cloudParticleMap[out] = pid;
            out++;
        }
    }

    return { positions: _cloudPos, colors: _cloudCol, sizes: _cloudSize, count: out };
}


// =====================================================================
// updateTrailHistory
// =====================================================================

/**
 * Record current particle positions into per-particle circular trail buffers.
 * Prunes trails for particles that no longer exist.
 *
 * Originally app_dag.js lines ~512-535.
 */
export function updateTrailHistory(peData) {
    for (let i = 0; i < peData.count; i++) {
        const id = peData.ids[i];
        if (!_trailHistory.has(id)) {
            _trailHistory.set(id, {
                positions: new Float32Array(TRAIL_MAX_LENGTH * 3),
                head: 0, length: 0
            });
        }
        const trail = _trailHistory.get(id);
        const h = trail.head;
        trail.positions[h * 3]     = peData.positions[i * 3];
        trail.positions[h * 3 + 1] = peData.positions[i * 3 + 1];
        trail.positions[h * 3 + 2] = peData.positions[i * 3 + 2];
        trail.head   = (h + 1) % TRAIL_MAX_LENGTH;
        trail.length = Math.min(trail.length + 1, TRAIL_MAX_LENGTH);
    }

    // Remove trails for particles that no longer exist
    const activeIds = new Set();
    for (let i = 0; i < peData.count; i++) activeIds.add(peData.ids[i]);
    for (const [id] of _trailHistory) {
        if (!activeIds.has(id)) _trailHistory.delete(id);
    }
}


// =====================================================================
// External accessors / reset
// =====================================================================

/** Read-only access to the cloud-to-particle mapping array. */
export function getCloudParticleMap() { return _cloudParticleMap; }

/** Read-only access to trail history for external consumers. */
export function getTrailHistory() { return _trailHistory; }

/** Reset template cache and trail history (called on mode switch). */
export function clearCloudAndTrails() {
    _cloudTemplates.clear();
    _trailHistory.clear();
}
