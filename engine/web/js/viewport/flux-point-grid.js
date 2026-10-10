/**
 * @file engine/web/js/viewport/flux-point-grid.js
 * @purpose Index arithmetic for the dense flux point cloud: which buffer
 *          entry is lattice cell (x, y, z), which entries are its Moore
 *          neighbours, and which drawn point is nearest a position. Pure
 *          functions over typed arrays, so they run under node:test.
 * @consumers ../viewport.js (live rulers, voxel clocks, Moore neighbourhood)
 *
 * The flux point cloud is x-fastest: entry (z*L + y)*L + x holds cell
 * (x, y, z). The native link sample is x-major (link-geometry.js siteIndex).
 * The two orders agree only where x === z, so they must not be mixed.
 *
 * `source` is the sourcePosition attribute (exact cell centres, c + 0.5).
 * `points` is the position attribute, which Organic mode jitters by up to
 * half a cell. Cells are identified on `source`; distances to a view target
 * are taken on `points`, where the dots are drawn.
 */

/** Buffer entry of lattice cell (x, y, z) in the flux point cloud. */
export function fluxPointIndex(axis, x, y, z) {
    return (z * axis + y) * axis + x;
}

/**
 * Entry of cell (x, y, z) when `source` is the dense axis^3 grid.
 * @returns {number} the entry; -1 if the cell is outside the lattice or
 *          hidden; -2 if this buffer is not that grid.
 */
export function fluxCellIndex(source, count, axis, visibility, x, y, z) {
    if (!(axis > 1) || count !== axis * axis * axis) return -2;
    if (x < 0 || y < 0 || z < 0 || x >= axis || y >= axis || z >= axis) return -1;
    const index = fluxPointIndex(axis, x, y, z);
    const i3 = index * 3;
    if (Math.abs(source[i3] - (x + 0.5)) > 0.2
        || Math.abs(source[i3 + 1] - (y + 0.5)) > 0.2
        || Math.abs(source[i3 + 2] - (z + 0.5)) > 0.2) return -2;
    if (visibility && visibility[index] < 0.5) return -1;
    return index;
}

/**
 * The visible cells of the 3x3x3 block around a lattice position, each with
 * its squared distance from that position. Null when the buffer is not a
 * dense grid; the caller then scans.
 */
export function mooreCellIndices(source, count, axis, visibility, ax, ay, az, includeSelf) {
    const fx = Math.round(ax - 0.5);
    const fy = Math.round(ay - 0.5);
    const fz = Math.round(az - 0.5);
    if (fluxCellIndex(source, count, axis, null, fx, fy, fz) < 0) return null;
    const found = [];
    for (let dz = -1; dz <= 1; dz++) {
        for (let dy = -1; dy <= 1; dy++) {
            for (let dx = -1; dx <= 1; dx++) {
                if (!includeSelf && dx === 0 && dy === 0 && dz === 0) continue;
                const index = fluxCellIndex(source, count, axis, visibility, fx + dx, fy + dy, fz + dz);
                if (index === -2) return null;
                if (index < 0) continue;
                const i3 = index * 3;
                const ddx = source[i3] - ax;
                const ddy = source[i3 + 1] - ay;
                const ddz = source[i3 + 2] - az;
                found.push({ index, d: ddx * ddx + ddy * ddy + ddz * ddz });
            }
        }
    }
    return found;
}

// Shells searched before giving up on locality. 13^3 cells at most; beyond
// that the visible region is far away and one linear pass is cheaper.
const NEAREST_SHELL_LIMIT = 6;

function scanNearestVisible(points, count, visibility, lx, ly, lz) {
    let best = -1;
    let bestD = Infinity;
    for (let i = 0; i < count; i++) {
        if (visibility && visibility[i] < 0.5) continue;
        const i3 = i * 3;
        const dx = points[i3] - lx;
        const dy = points[i3 + 1] - ly;
        const dz = points[i3 + 2] - lz;
        const d = dx * dx + dy * dy + dz * dz;
        if (d < bestD) {
            bestD = d;
            best = i;
        }
    }
    return best;
}

/**
 * Entry of the visible point nearest (lx, ly, lz); -1 if none is visible.
 * Same answer as a scan of every point (ties go to the lower entry), found
 * by growing cubic shells around the target's cell on a dense grid.
 *
 * A point in a cell r shells out is at least r - 1 away: the target lies
 * within its own cell or beyond the lattice edge, and a drawn point stays
 * within its cell. The search stops once that bound passes the best found.
 */
export function nearestVisiblePoint(points, source, count, axis, visibility, lx, ly, lz) {
    const clamp = (v) => Math.max(0, Math.min(axis - 1, Math.round(v - 0.5)));
    const cx = clamp(lx);
    const cy = clamp(ly);
    const cz = clamp(lz);
    if (!Number.isFinite(lx + ly + lz)
        || fluxCellIndex(source, count, axis, null, cx, cy, cz) < 0) {
        return scanNearestVisible(points, count, visibility, lx, ly, lz);
    }
    let best = -1;
    let bestD = Infinity;
    for (let r = 0; r < axis; r++) {
        if (best >= 0 && r - 1 > Math.sqrt(bestD)) return best;
        if (best < 0 && r > NEAREST_SHELL_LIMIT) {
            return scanNearestVisible(points, count, visibility, lx, ly, lz);
        }
        const z0 = Math.max(0, cz - r);
        const z1 = Math.min(axis - 1, cz + r);
        const y0 = Math.max(0, cy - r);
        const y1 = Math.min(axis - 1, cy + r);
        const x0 = Math.max(0, cx - r);
        const x1 = Math.min(axis - 1, cx + r);
        for (let z = z0; z <= z1; z++) {
            const onZ = Math.abs(z - cz) === r;
            for (let y = y0; y <= y1; y++) {
                const onY = onZ || Math.abs(y - cy) === r;
                // Interior rows of the shell touch it only at their two ends.
                const step = onY ? 1 : Math.max(1, x1 - x0);
                for (let x = x0; x <= x1; x += step) {
                    if (!onY && Math.abs(x - cx) !== r) continue;
                    const index = fluxPointIndex(axis, x, y, z);
                    if (visibility && visibility[index] < 0.5) continue;
                    const i3 = index * 3;
                    const dx = points[i3] - lx;
                    const dy = points[i3 + 1] - ly;
                    const dz = points[i3 + 2] - lz;
                    const d = dx * dx + dy * dy + dz * dz;
                    if (d < bestD || (d === bestD && index < best)) {
                        bestD = d;
                        best = index;
                    }
                }
            }
        }
    }
    return best;
}
