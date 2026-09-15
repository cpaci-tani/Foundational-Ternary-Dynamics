// engine/web/js/link-geometry.js
//
// Lattice-native link geometry for the native transport overlay (spec
// 2026-09-15 native transport overlays, section 4). Pure functions, no Three.js.
// Sites index x-major: (x·L + y)·L + z. Each site owns nine links; a positive
// link value is transport from the owner toward owner + d_k. A site's centre is
// drawn at index + 0.5, the same convention every Scale-0 sampler uses.

export const LINK_DISPLACEMENT = Object.freeze([
    [1, 0, 0], [0, 1, 0], [0, 0, 1],
    [1, 1, 0], [1, -1, 0], [1, 0, 1], [1, 0, -1], [0, 1, 1], [0, 1, -1],
].map((d) => Object.freeze(d)));

export const LINK_COUNT_PER_SITE = 9;

export function siteIndex(L, x, y, z) {
    return (x * L + y) * L + z;
}

// Writes link (site, k) as two half-segments, 12 floats starting at out[offset]:
//   owner centre -> owner centre + d/2, then neighbour centre - d/2 -> neighbour centre.
// For an interior link the two inner points coincide. For a link that wraps the
// periodic box, each half ends on the face or edge it crosses.
export function writeLinkHalves(L, site, k, out, offset) {
    const z = site % L;
    const y = Math.floor(site / L) % L;
    const x = Math.floor(site / (L * L));
    const d = LINK_DISPLACEMENT[k];
    const nx = (x + d[0] + L) % L;
    const ny = (y + d[1] + L) % L;
    const nz = (z + d[2] + L) % L;
    const ax = x + 0.5, ay = y + 0.5, az = z + 0.5;
    const bx = nx + 0.5, by = ny + 0.5, bz = nz + 0.5;
    out[offset] = ax; out[offset + 1] = ay; out[offset + 2] = az;
    out[offset + 3] = ax + d[0] / 2; out[offset + 4] = ay + d[1] / 2; out[offset + 5] = az + d[2] / 2;
    out[offset + 6] = bx - d[0] / 2; out[offset + 7] = by - d[1] / 2; out[offset + 8] = bz - d[2] / 2;
    out[offset + 9] = bx; out[offset + 10] = by; out[offset + 11] = bz;
}

// Link ids (site·9 + k) whose |value| exceeds fraction·max|value|, strongest
// first, at most `cap` of them. Returns { ids: Int32Array, max }.
export function selectLinks(values, fraction = 0.05, cap = 20000) {
    let max = 0;
    for (let i = 0; i < values.length; i++) {
        const a = Math.abs(values[i]);
        if (a > max) max = a;
    }
    if (!(max > 0)) return { ids: new Int32Array(0), max: 0 };
    const threshold = fraction * max;
    const candidates = [];
    for (let i = 0; i < values.length; i++) if (Math.abs(values[i]) > threshold) candidates.push(i);
    candidates.sort((p, q) => Math.abs(values[q]) - Math.abs(values[p]));
    return { ids: Int32Array.from(candidates.slice(0, Math.max(0, cap | 0))), max };
}
