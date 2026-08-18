// Scale-0 manifestation spawn-flash — a REAL "just manifested this tick"
// visual, distinct from the pre-existing genesis isosurface (which shows
// the |J|~K_GENESIS precondition band, not the discrete event) and from the
// bulk particle cloud (which silently gains a new point with no visual cue
// at all).
//
// bridge.getParticleData() carries no persistent per-particle id for Scale-0
// (unlike Scale-1's PE cloud) — a manifested voxel is only ever addressed by
// its lattice position. This module diffs POSITION KEYS against the previous
// sampled frame (option (b) from the engine-visualization checklist: cheaper
// and less precise than a WASM-exposed "ticks since manifestation" array,
// but a fast, no-C++-change way to validate the flash reads well before
// committing to that larger change). Known imprecision: a particle that
// moves one voxel between sampled frames looks like a new spawn (position
// key changes) — acceptable for this first cut; the WASM-array approach
// would not have this artifact.
//
// Reuses the exact shader pipeline Scale-1 already drives (manifestPhase /
// manifestRate attributes -> uManifestEnabled/uManifestTime/uManifestThresh
// uniforms in viewport/shaders.js) via viewport.setPEManifestation() — that
// method name is a historical artifact (PE = Particle Engine, Scale-1) but
// the shader/material it drives is shared by the whole particle renderer,
// Scale-0 included.

const SPAWN_FLASH_DURATION = 0.6; // seconds, matches Scale-1's spawn flash
const SPAWN_FLASH_RATE_BOOST = 3.5;
const BASE_RATE = 1.6;
const _spawnTimes = new Map(); // packed voxel key -> frameSec first seen

function posKey(x, y, z) {
    // Native Scale 0 is capped at L=256, so 10 bits per wrapped coordinate is
    // collision-free. Numeric Map/Set keys avoid allocating and hashing up to
    // 100K "x,y,z" strings on every large-lattice visual refresh.
    const xi = Math.floor(x) & 0x3ff;
    const yi = Math.floor(y) & 0x3ff;
    const zi = Math.floor(z) & 0x3ff;
    return (xi | (yi << 10) | (zi << 20)) >>> 0;
}

// Cheap integer mix -> a stable per-voxel phase offset in [0, 2*pi), so
// simultaneous nearby spawns don't all blink in lockstep (a "wall of
// flashes" look).
function phaseFromKey(key) {
    let h = key | 0;
    h ^= h >>> 16;
    h = Math.imul(h, 0x7feb352d);
    h ^= h >>> 15;
    h = Math.imul(h, 0x846ca68b);
    h ^= h >>> 16;
    return ((h >>> 0) % 6283) / 1000; // ~[0, 2*pi)
}

/**
 * @param {{positions: Float32Array, count: number}} particleData
 * @param {number} frameSec
 * @returns {{phases: Float32Array, rates: Float32Array}}
 */
export function computeManifestationBlink(particleData, frameSec) {
    const n = particleData.count;
    const phases = new Float32Array(n);
    const rates = new Float32Array(n);
    if (!n || !Number.isFinite(frameSec)) {
        return { phases, rates };
    }

    const seenThisFrame = new Set();
    for (let i = 0; i < n; i++) {
        const key = posKey(
            particleData.positions[i * 3],
            particleData.positions[i * 3 + 1],
            particleData.positions[i * 3 + 2],
        );
        seenThisFrame.add(key);
        if (!_spawnTimes.has(key)) _spawnTimes.set(key, frameSec);

        const age = frameSec - _spawnTimes.get(key);
        let rate = BASE_RATE;
        if (age >= 0 && age < SPAWN_FLASH_DURATION) {
            const t = 1 - age / SPAWN_FLASH_DURATION;
            rate += SPAWN_FLASH_RATE_BOOST * t * t;
        }
        rates[i] = rate;
        phases[i] = phaseFromKey(key);
    }

    // Prune positions no longer manifested (evaporated/moved on) so the
    // tracking map doesn't grow unboundedly across a long-running session.
    for (const key of _spawnTimes.keys()) {
        if (!seenThisFrame.has(key)) _spawnTimes.delete(key);
    }

    return { phases, rates };
}

export function clearManifestationTracking() {
    _spawnTimes.clear();
}
