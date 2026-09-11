/**
 * @file viewport/point-cloud-draw-order.js
 * @purpose Camera-aware draw order for translucent THREE.Points clouds.
 *
 * A Points cloud with `depthWrite: false` and `NormalBlending` is composited
 * in draw order, and draw order is buffer order unless an index buffer says
 * otherwise. Normal (over) blending is not commutative, so a fixed buffer
 * order looks different from opposite sides: from one side the near points
 * are drawn last and read as a solid shell; from the other the far points are
 * drawn last and bleed through the front, so the same cloud reads hollow and
 * inside-out. Reversing the buffer order at a fixed camera reproduces the
 * swap, which is the diagnostic that established this.
 *
 * Additive blending is commutative and needs none of this; the hook skips
 * clouds whose material is additive.
 *
 * Two orderings are offered:
 *   - 'axis'  : for a regular lattice cloud. Six precomputed orderings, one
 *               per signed dominant view axis, built by an O(n) counting sort
 *               on the integer coordinate along that axis. Re-indexing happens
 *               only when the camera crosses into a new signed-axis octant, so
 *               the per-frame cost is a dot product and a comparison.
 *   - 'depth' : for moving off-lattice points (particles). An O(n) counting
 *               sort on the quantised view depth, re-run every frame.
 *
 * This module is THREE-free so it can be unit-tested headlessly. The renderer
 * passes `wrapIndex` (typically `a => new THREE.Uint32BufferAttribute(a, 1)`);
 * a raw typed array handed to `BufferGeometry.setIndex` is NOT converted by
 * three.js, so the wrap is required in production.
 */

const AXIS_BUCKETS = 4096;   // lattice coordinates are clamped to [0, 4095]
const DEPTH_BUCKETS = 65536; // 16-bit quantised view depth

// Scratch buffers for the per-frame depth sort. Without these the sort
// allocated ~262 KB of counting buffers on every frame, i.e. ~15 MB/s of
// garbage at 60 Hz for a cloud that re-sorts each tick.
const countsScratch = new Uint32Array(DEPTH_BUCKETS + 1);
let depthScratch = new Float32Array(0);
let bucketScratch = new Uint16Array(0);

// Minimal target vector for `camera.getWorldDirection(target)`. three.js's
// Object3D version does `target.set(...).normalize()` and the Camera override
// then calls `.negate()` on the result (cameras look down -Z), so all three
// must exist and chain. A raw object without `negate` throws inside three.
class Vec3 {
    constructor() { this.x = 0; this.y = 0; this.z = 0; }
    set(x, y, z) { this.x = x; this.y = y; this.z = z; return this; }
    normalize() {
        const l = Math.hypot(this.x, this.y, this.z) || 1;
        this.x /= l; this.y /= l; this.z /= l; return this;
    }
    negate() { this.x = -this.x; this.y = -this.y; this.z = -this.z; return this; }
}

/**
 * Signed dominant axis of a view direction: axis*2 + (positive ? 1 : 0).
 * @param {ArrayLike<number>} dir
 * @returns {0|1|2|3|4|5}
 */
export function dominantAxisKey(dir) {
    const ax = Math.abs(dir[0]), ay = Math.abs(dir[1]), az = Math.abs(dir[2]);
    const axis = ax >= ay && ax >= az ? 0 : (ay >= az ? 1 : 2);
    return axis * 2 + (dir[axis] > 0 ? 1 : 0);
}

/**
 * Far-to-near draw order along the signed dominant axis, by counting sort on
 * the integer coordinate. Stable within a coordinate slab.
 * @param {ArrayLike<number>} positions interleaved xyz
 * @param {number} count live points (stale slots beyond it are excluded)
 * @param {number} key from dominantAxisKey
 * @param {number} [stride=3]
 * @returns {Uint32Array}
 */
export function buildAxisOrder(positions, count, key, stride = 3) {
    const axis = key >> 1, positive = (key & 1) === 1;
    const counts = new Uint32Array(AXIS_BUCKETS + 1);
    const bucket = new Uint16Array(count);
    for (let i = 0; i < count; i++) {
        let b = Math.floor(positions[i * stride + axis]);
        if (!(b >= 0)) b = 0; else if (b >= AXIS_BUCKETS) b = AXIS_BUCKETS - 1;
        bucket[i] = b;
        counts[b + 1]++;
    }
    for (let b = 0; b < AXIS_BUCKETS; b++) counts[b + 1] += counts[b];
    const out = new Uint32Array(count);
    if (positive) {
        // View direction points toward +axis: far = large coordinate, draw it first.
        for (let i = 0; i < count; i++) {
            const b = bucket[i];
            out[count - 1 - counts[b]++] = i; // fill from the back so large b lands first
        }
        // The loop above places slab b in reverse; restore stability within each slab.
        reverseSlabs(out, bucket, count);
    } else {
        for (let i = 0; i < count; i++) out[counts[bucket[i]]++] = i;
    }
    return out;
}

function reverseSlabs(out, bucket, count) {
    let start = 0;
    while (start < count) {
        const b = bucket[out[start]];
        let end = start;
        while (end + 1 < count && bucket[out[end + 1]] === b) end++;
        for (let lo = start, hi = end; lo < hi; lo++, hi--) {
            const t = out[lo]; out[lo] = out[hi]; out[hi] = t;
        }
        start = end + 1;
    }
}

/**
 * Back-to-front order for arbitrary points by 16-bit quantised view depth.
 * O(n); ties within one bucket keep index order.
 * @param {ArrayLike<number>} positions interleaved xyz
 * @param {number} count
 * @param {ArrayLike<number>} dir unit view direction (camera forward)
 * @param {Uint32Array} [out] reusable buffer of length >= count
 * @returns {Uint32Array} the first `count` entries of `out`
 */
export function sortBackToFront(positions, count, dir, out) {
    const dx = dir[0], dy = dir[1], dz = dir[2];
    if (depthScratch.length < count) {
        depthScratch = new Float32Array(count);
        bucketScratch = new Uint16Array(count);
    }
    const depth = depthScratch;
    let min = Infinity, max = -Infinity;
    for (let i = 0; i < count; i++) {
        const d = positions[i * 3] * dx + positions[i * 3 + 1] * dy + positions[i * 3 + 2] * dz;
        depth[i] = d;
        if (d < min) min = d;
        if (d > max) max = d;
    }
    const result = out && out.length >= count ? out : new Uint32Array(count);
    const span = max - min;
    const scale = span > 0 ? (DEPTH_BUCKETS - 1) / span : 0;
    const counts = countsScratch; counts.fill(0);
    const bucket = bucketScratch;
    for (let i = 0; i < count; i++) {
        // Far (large depth) must come first: invert the bucket index.
        const b = DEPTH_BUCKETS - 1 - Math.round((depth[i] - min) * scale);
        bucket[i] = b;
        counts[b + 1]++;
    }
    for (let b = 0; b < DEPTH_BUCKETS; b++) counts[b + 1] += counts[b];
    for (let i = 0; i < count; i++) result[counts[bucket[i]]++] = i;
    return result.length === count ? result : result.subarray(0, count);
}

/**
 * Install an onBeforeRender hook that keeps `points.geometry` indexed
 * back-to-front for the rendering camera. Returns a detach function that
 * removes the hook and the index buffer.
 *
 * @param {{geometry: object, material?: object, onBeforeRender?: Function|null}} points
 * @param {{
 *   mode?: 'axis'|'depth',
 *   positionAttr?: string,
 *   wrapIndex?: (order: Uint32Array) => any,
 * }} [options]
 * @returns {() => void} detach
 */
export function attachBackToFrontOrdering(points, options = {}) {
    const mode = options.mode === 'depth' ? 'depth' : 'axis';
    const positionAttr = options.positionAttr || 'position';
    const wrapIndex = typeof options.wrapIndex === 'function' ? options.wrapIndex : (a => a);
    const dir = new Vec3();
    const cache = new Map();          // axis mode: `${key}:${count}` -> wrapped index
    let lastKey = -1, lastCount = -1;
    let depthArray = null, depthIndex = null;

    points.onBeforeRender = (_renderer, _scene, camera) => {
        const geometry = points.geometry;
        if (!geometry || !camera || typeof camera.getWorldDirection !== 'function') return;
        const count = geometry.drawRange ? geometry.drawRange.count : 0;
        if (!(count > 0) || count === Infinity) return;
        // Additive blending composites order-independently; nothing to do.
        if (points.material && points.material.blending === 2) return;
        const attr = geometry.getAttribute(positionAttr) || geometry.getAttribute('position');
        if (!attr || !attr.array) return;
        camera.getWorldDirection(dir);
        const d = [dir.x, dir.y, dir.z];

        if (mode === 'axis') {
            const key = dominantAxisKey(d);
            if (key === lastKey && count === lastCount && geometry.index) return;
            const cacheKey = key + ':' + count;
            let wrapped = cache.get(cacheKey);
            if (!wrapped) {
                wrapped = wrapIndex(buildAxisOrder(attr.array, count, key));
                cache.set(cacheKey, wrapped);
            }
            geometry.setIndex(wrapped);
            geometry.setDrawRange(0, count);
            lastKey = key; lastCount = count;
            return;
        }

        if (!depthArray || depthArray.length < count) {
            depthArray = new Uint32Array(Math.max(count, depthArray ? depthArray.length * 2 : 1024));
            depthIndex = wrapIndex(depthArray);
        }
        sortBackToFront(attr.array, count, d, depthArray);
        if (depthIndex && typeof depthIndex === 'object') depthIndex.needsUpdate = true;
        if (geometry.index !== depthIndex) geometry.setIndex(depthIndex);
        geometry.setDrawRange(0, count);
    };

    return function detach() {
        // three.js calls object.onBeforeRender(...) unconditionally on every
        // render, so it must remain callable: restore an empty function, never
        // null (same reset scalar-volume-renderer.js uses).
        points.onBeforeRender = () => {};
        if (points.geometry && typeof points.geometry.setIndex === 'function') {
            points.geometry.setIndex(null);
        }
        cache.clear();
        depthArray = null; depthIndex = null;
        lastKey = -1; lastCount = -1;
    };
}
