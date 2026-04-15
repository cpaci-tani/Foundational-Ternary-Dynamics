/**
 * Boundary shape geometry for the FTD particle engine.
 * Pure functions — stateless, no MockBridge dependency.
 *
 * RF-13: extracted from MockBridge in wasm-bridge-dag.js.
 */

/** Face normals for dodecahedron containment test (phi ≈ 1.618). */
export const DODECAHEDRON_NORMALS = [
    [0, 1, 1.618033988749895], [0, -1, 1.618033988749895],
    [0, 1, -1.618033988749895], [0, -1, -1.618033988749895],
    [1, 1.618033988749895, 0], [-1, 1.618033988749895, 0],
    [1, -1.618033988749895, 0], [-1, -1.618033988749895, 0],
    [1.618033988749895, 0, 1], [-1.618033988749895, 0, 1],
    [1.618033988749895, 0, -1], [-1.618033988749895, 0, -1],
];

/** Face normals for icosahedron containment test (phi ≈ 1.618). */
export const ICOSAHEDRON_NORMALS = [
    [1, 1, 1], [1, 1, -1], [1, -1, 1], [1, -1, -1],
    [-1, 1, 1], [-1, 1, -1], [-1, -1, 1], [-1, -1, -1],
    [0, 1.618033988749895, 1 / 1.618033988749895],
    [0, 1.618033988749895, -1 / 1.618033988749895],
    [0, -1.618033988749895, 1 / 1.618033988749895],
    [0, -1.618033988749895, -1 / 1.618033988749895],
    [1 / 1.618033988749895, 0, 1.618033988749895],
    [-1 / 1.618033988749895, 0, 1.618033988749895],
    [1 / 1.618033988749895, 0, -1.618033988749895],
    [-1 / 1.618033988749895, 0, -1.618033988749895],
    [1.618033988749895, 1 / 1.618033988749895, 0],
    [1.618033988749895, -1 / 1.618033988749895, 0],
    [-1.618033988749895, 1 / 1.618033988749895, 0],
    [-1.618033988749895, -1 / 1.618033988749895, 0],
];

/**
 * Returns true if the normalized point (nx, ny, nz) is inside the named
 * boundary shape.  Coordinates are expected in the range −1..+1, where
 * ±1 corresponds to the lattice edge (i.e. already divided by halfN).
 *
 * @param {string} shape  — one of 'none', 'cube', 'sphere', 'octahedron',
 *                          'dodecahedron', 'icosahedron', 'cylinder', 'torus'
 * @param {number} nx
 * @param {number} ny
 * @param {number} nz
 * @returns {boolean}
 */
export function insideBoundary(shape, nx, ny, nz) {
    switch (shape) {
        case 'none':
        case 'cube':
            return true;
        case 'sphere':
            return (nx * nx + ny * ny + nz * nz) <= 1.0;
        case 'octahedron':
            return (Math.abs(nx) + Math.abs(ny) + Math.abs(nz)) <= 1.0;
        case 'dodecahedron': {
            const ir = 0.7946;
            for (const n of DODECAHEDRON_NORMALS) {
                const len = Math.sqrt(n[0]*n[0] + n[1]*n[1] + n[2]*n[2]);
                if ((nx * n[0] + ny * n[1] + nz * n[2]) / len > ir) return false;
            }
            return true;
        }
        case 'icosahedron': {
            const ir = 0.7558;
            for (const n of ICOSAHEDRON_NORMALS) {
                const len = Math.sqrt(n[0]*n[0] + n[1]*n[1] + n[2]*n[2]);
                if ((nx * n[0] + ny * n[1] + nz * n[2]) / len > ir) return false;
            }
            return true;
        }
        case 'cylinder':
            return (nx * nx + nz * nz) <= 1.0 && Math.abs(ny) <= 1.0;
        case 'torus': {
            const dist_xz = Math.sqrt(nx * nx + nz * nz);
            const dx = dist_xz - 0.7;
            return (dx * dx + ny * ny) <= 0.09; // 0.3²
        }
        default:
            return true;
    }
}

/**
 * Reflects particle/atom p back inside the boundary when it has escaped.
 * Modifies p in-place (p.x, p.y, p.z, p.vx, p.vy, p.vz).
 *
 * cx, cy, cz = center of the boundary region in world-space coordinates.
 * R           = half-extent (radius) in world-space coordinates.
 * reflective  = if false, use absorbing boundary (particle is not reflected;
 *               it simply passes through).
 *
 * @param {string} shape
 * @param {{x:number,y:number,z:number,vx:number,vy:number,vz:number}} p
 * @param {number} cx
 * @param {number} cy
 * @param {number} cz
 * @param {number} R
 * @param {boolean} reflective
 */
export function reflectIntoBoundary(shape, p, cx, cy, cz, R, reflective) {
    if (shape === 'cube' || shape === 'none') return;
    const nx = (p.x - cx) / R;
    const ny = (p.y - cy) / R;
    const nz = (p.z - cz) / R;
    if (insideBoundary(shape, nx, ny, nz)) return;
    // Absorbing boundary: let particle pass through (no reflection)
    if (!reflective) return;

    // Compute outward normal at boundary surface for reflection
    let snx = 0, sny = 0, snz = 0;
    switch (shape) {
        case 'sphere': {
            const r = Math.sqrt(nx*nx + ny*ny + nz*nz);
            if (r < 1e-10) return;
            snx = nx / r; sny = ny / r; snz = nz / r;
            // Project back onto sphere surface
            p.x = cx + snx * R * 0.99;
            p.y = cy + sny * R * 0.99;
            p.z = cz + snz * R * 0.99;
            break;
        }
        case 'octahedron': {
            // Normal = sign of coordinates (octahedron faces)
            snx = Math.sign(nx) || 1;
            sny = Math.sign(ny) || 1;
            snz = Math.sign(nz) || 1;
            const len = Math.sqrt(snx*snx + sny*sny + snz*snz);
            snx /= len; sny /= len; snz /= len;
            // Project back: move inward along normal
            const dist = Math.abs(nx) + Math.abs(ny) + Math.abs(nz) - 1.0;
            p.x = cx + (nx - snx * dist * 1.01) * R;
            p.y = cy + (ny - sny * dist * 1.01) * R;
            p.z = cz + (nz - snz * dist * 1.01) * R;
            break;
        }
        case 'cylinder': {
            const rXZ = Math.sqrt(nx*nx + nz*nz);
            if (rXZ > 1.0) {
                snx = nx / rXZ; snz = nz / rXZ;
                p.x = cx + snx * R * 0.99;
                p.z = cz + snz * R * 0.99;
            }
            if (Math.abs(ny) > 1.0) {
                sny = Math.sign(ny);
                p.y = cy + sny * R * 0.99;
            }
            snx = nx / Math.max(rXZ, 0.01);
            sny = Math.abs(ny) > 1.0 ? Math.sign(ny) : 0;
            snz = nz / Math.max(rXZ, 0.01);
            const nlen = Math.sqrt(snx*snx + sny*sny + snz*snz) || 1;
            snx /= nlen; sny /= nlen; snz /= nlen;
            break;
        }
        case 'torus': {
            const dist_xz = Math.sqrt(nx*nx + nz*nz) || 0.001;
            const cx_ring = 0.7 * nx / dist_xz;
            const cz_ring = 0.7 * nz / dist_xz;
            const dx = nx - cx_ring, dz = nz - cz_ring;
            const dr = Math.sqrt(dx*dx + ny*ny) || 0.001;
            snx = dx / dr; sny = ny / dr; snz = dz / dr;
            p.x = cx + (cx_ring + snx * 0.29) * R;
            p.y = cy + sny * 0.29 * R;
            p.z = cz + (cz_ring + snz * 0.29) * R;
            break;
        }
        default: {
            // Dodecahedron / Icosahedron: use gradient of distance function.
            // Nudge inward along the most-violated face normal.
            const normals = shape === 'dodecahedron'
                ? DODECAHEDRON_NORMALS
                : ICOSAHEDRON_NORMALS;
            const ir = shape === 'dodecahedron' ? 0.7946 : 0.7558;
            let maxD = -Infinity, bestN = null;
            for (const n of normals) {
                const len = Math.sqrt(n[0]*n[0] + n[1]*n[1] + n[2]*n[2]);
                const d = (nx * n[0] + ny * n[1] + nz * n[2]) / len;
                if (d > maxD) { maxD = d; bestN = [n[0]/len, n[1]/len, n[2]/len]; }
            }
            if (bestN) {
                const push = (maxD - ir) * 1.01;
                p.x = cx + (nx - bestN[0] * push) * R;
                p.y = cy + (ny - bestN[1] * push) * R;
                p.z = cz + (nz - bestN[2] * push) * R;
                snx = bestN[0]; sny = bestN[1]; snz = bestN[2];
            }
            break;
        }
    }
    // Reflect velocity: v = v - 2(v·n)n
    const dot = p.vx * snx + p.vy * sny + p.vz * snz;
    if (dot > 0) { // only reflect if moving outward
        p.vx -= 2 * dot * snx;
        p.vy -= 2 * dot * sny;
        p.vz -= 2 * dot * snz;
    }
}
