// ── fieldlines.js ── Streamline computation for 3D field visualization ──
// RK4 integration through sampled vector fields.
// Returns arrays of Float32Array vertex positions for Three.js LineSegments.

/**
 * Trilinear interpolation of a vector field on a regular grid.
 * @param {Float32Array} positions  - Flat array [x0,y0,z0, x1,y1,z1, ...]
 * @param {Float32Array} vectors    - Flat array [vx0,vy0,vz0, ...]
 * @param {number}       count      - Number of sample points
 * @param {number}       N          - Lattice size (assumes cubic NxNxN)
 * @param {number}       stride     - Sampling stride used when generating data
 * @param {number}       px         - Query x
 * @param {number}       py         - Query y
 * @param {number}       pz         - Query z
 * @returns {[number,number,number]} Interpolated vector [vx,vy,vz]
 */
export function interpolateField(positions, vectors, count, N, stride, px, py, pz) {
    // Nearest-neighbor lookup (fast; good enough for streamlines at stride 2-4)
    let bestDist = Infinity;
    let bx = 0, by = 0, bz = 0;
    for (let i = 0; i < count; i++) {
        const dx = positions[i * 3]     - px;
        const dy = positions[i * 3 + 1] - py;
        const dz = positions[i * 3 + 2] - pz;
        const d2 = dx * dx + dy * dy + dz * dz;
        if (d2 < bestDist) {
            bestDist = d2;
            bx = vectors[i * 3];
            by = vectors[i * 3 + 1];
            bz = vectors[i * 3 + 2];
        }
    }
    return [bx, by, bz];
}

/**
 * Build a spatial index for fast nearest-neighbor field lookup.
 * Bins sample points into a grid of cells for O(1) lookup.
 * @param {Float32Array} positions
 * @param {Float32Array} vectors
 * @param {number}       count
 * @param {number}       N       - Lattice size
 * @param {number}       stride  - Sampling stride
 * @returns {object} Spatial index { grid, cellSize, gridDim, positions, vectors, count }
 */
export function buildFieldIndex(positions, vectors, count, N, stride) {
    const cellSize = Math.max(stride * 2, 4);
    const gridDim = Math.ceil(N / cellSize);
    const totalCells = gridDim * gridDim * gridDim;
    const grid = new Array(totalCells);
    for (let i = 0; i < totalCells; i++) grid[i] = [];

    for (let i = 0; i < count; i++) {
        const cx = Math.floor(positions[i * 3]     / cellSize) | 0;
        const cy = Math.floor(positions[i * 3 + 1] / cellSize) | 0;
        const cz = Math.floor(positions[i * 3 + 2] / cellSize) | 0;
        const ci = Math.min(cx, gridDim - 1) + Math.min(cy, gridDim - 1) * gridDim
                 + Math.min(cz, gridDim - 1) * gridDim * gridDim;
        if (ci >= 0 && ci < totalCells) grid[ci].push(i);
    }

    return { grid, cellSize, gridDim, positions, vectors, count };
}

/**
 * Fast field lookup using spatial index.
 */
export function lookupField(index, px, py, pz) {
    const { grid, cellSize, gridDim, positions, vectors } = index;
    const cx = Math.floor(px / cellSize) | 0;
    const cy = Math.floor(py / cellSize) | 0;
    const cz = Math.floor(pz / cellSize) | 0;

    let bestDist = Infinity;
    let bx = 0, by = 0, bz = 0;

    // Search cell and 26 neighbors
    for (let dz = -1; dz <= 1; dz++) {
        for (let dy = -1; dy <= 1; dy++) {
            for (let dx = -1; dx <= 1; dx++) {
                const nx = cx + dx, ny = cy + dy, nz = cz + dz;
                if (nx < 0 || nx >= gridDim || ny < 0 || ny >= gridDim || nz < 0 || nz >= gridDim) continue;
                const ci = nx + ny * gridDim + nz * gridDim * gridDim;
                const cell = grid[ci];
                for (let k = 0; k < cell.length; k++) {
                    const i = cell[k];
                    const ddx = positions[i * 3]     - px;
                    const ddy = positions[i * 3 + 1] - py;
                    const ddz = positions[i * 3 + 2] - pz;
                    const d2 = ddx * ddx + ddy * ddy + ddz * ddz;
                    if (d2 < bestDist) {
                        bestDist = d2;
                        bx = vectors[i * 3];
                        by = vectors[i * 3 + 1];
                        bz = vectors[i * 3 + 2];
                    }
                }
            }
        }
    }
    return [bx, by, bz];
}

/**
 * Single RK4 step.
 * @param {function} fieldFn  - (x,y,z) => [vx,vy,vz]
 * @param {number}   x,y,z   - Current position
 * @param {number}   h       - Step size
 * @returns {[number,number,number]} New position [x',y',z']
 */
export function rk4Step(fieldFn, x, y, z, h) {
    const [k1x, k1y, k1z] = fieldFn(x, y, z);
    const [k2x, k2y, k2z] = fieldFn(x + 0.5 * h * k1x, y + 0.5 * h * k1y, z + 0.5 * h * k1z);
    const [k3x, k3y, k3z] = fieldFn(x + 0.5 * h * k2x, y + 0.5 * h * k2y, z + 0.5 * h * k2z);
    const [k4x, k4y, k4z] = fieldFn(x + h * k3x, y + h * k3y, z + h * k3z);

    return [
        x + (h / 6) * (k1x + 2 * k2x + 2 * k3x + k4x),
        y + (h / 6) * (k1y + 2 * k2y + 2 * k3y + k4y),
        z + (h / 6) * (k1z + 2 * k2z + 2 * k3z + k4z)
    ];
}

/**
 * Compute streamlines through a vector field.
 *
 * @param {object}   fieldData   - { positions: Float32Array, vectors: Float32Array, count: number }
 * @param {Array}    seeds       - Array of [x,y,z] seed positions
 * @param {object}   opts        - Options
 * @param {number}   opts.N          - Lattice size (default 32)
 * @param {number}   opts.stride     - Field sampling stride (default 2)
 * @param {number}   opts.maxSteps   - Max integration steps per line (default 100)
 * @param {number}   opts.stepSize   - RK4 step size in voxels (default 0.5)
 * @param {number}   opts.minMag     - Stop if field magnitude drops below (default 1e-10)
 * @param {boolean}  opts.bidirectional - Integrate both directions (default true)
 * @returns {Array<Float32Array>}  Array of streamline vertex arrays [x0,y0,z0, x1,y1,z1, ...]
 */
export function computeStreamlines(fieldData, seeds, opts = {}) {
    const {
        N = 32,
        stride = 2,
        maxSteps = 100,
        stepSize = 0.5,
        minMag = 1e-10,
        bidirectional = true,
        bounds = 0  // if > 0, uses origin-centered sphere bounds instead of lattice [0,N)
    } = opts;

    if (seeds.length === 0) return [];

    // Support direct fieldFn (for PE mode) or grid-based lookup (for Scale 0)
    let fieldFn;
    if (fieldData && fieldData.fieldFn) {
        fieldFn = fieldData.fieldFn;
    } else {
        if (!fieldData || fieldData.count === 0) return [];
        const idx = buildFieldIndex(fieldData.positions, fieldData.vectors, fieldData.count, N, stride);
        fieldFn = (x, y, z) => lookupField(idx, x, y, z);
    }

    const effectiveBounds = bounds > 0 ? bounds : N;
    const originCentered = bounds > 0;

    const lines = [];
    const maxLines = 200; // Global cap

    for (let s = 0; s < seeds.length && lines.length < maxLines; s++) {
        const [sx, sy, sz] = seeds[s];

        // Forward integration
        const fwd = integrateDirection(fieldFn, sx, sy, sz, stepSize, maxSteps, minMag, effectiveBounds, originCentered);

        if (bidirectional) {
            // Backward integration (negate field)
            const negFieldFn = (x, y, z) => {
                const [vx, vy, vz] = fieldFn(x, y, z);
                return [-vx, -vy, -vz];
            };
            const bwd = integrateDirection(negFieldFn, sx, sy, sz, stepSize, maxSteps, minMag, effectiveBounds, originCentered);

            // Combine: reverse backward + forward
            if (bwd.length > 3 || fwd.length > 3) {
                const combined = new Float32Array(bwd.length + fwd.length);
                // Reverse backward
                const bwdPts = bwd.length / 3;
                for (let i = 0; i < bwdPts; i++) {
                    const ri = bwdPts - 1 - i;
                    combined[i * 3]     = bwd[ri * 3];
                    combined[i * 3 + 1] = bwd[ri * 3 + 1];
                    combined[i * 3 + 2] = bwd[ri * 3 + 2];
                }
                combined.set(fwd, bwd.length);
                lines.push(combined);
            }
        } else {
            if (fwd.length > 3) {
                lines.push(fwd);
            }
        }
    }

    return lines;
}

/**
 * Integrate a single streamline in one direction.
 * @param {boolean} originCentered — if true, bounds is a radius from origin; else [0, bounds)
 */
function integrateDirection(fieldFn, x0, y0, z0, h, maxSteps, minMag, bounds, originCentered = false) {
    const verts = [x0, y0, z0];
    let x = x0, y = y0, z = z0;

    for (let step = 0; step < maxSteps; step++) {
        const [vx, vy, vz] = fieldFn(x, y, z);
        const mag = Math.sqrt(vx * vx + vy * vy + vz * vz);
        if (mag < minMag) break;

        // Normalize field direction for uniform step length
        const nx = vx / mag, ny = vy / mag, nz = vz / mag;
        const normFieldFn = () => [nx, ny, nz];

        const [x1, y1, z1] = rk4Step(normFieldFn, x, y, z, h);

        // Boundary check
        if (originCentered) {
            if (x1 * x1 + y1 * y1 + z1 * z1 > bounds * bounds) break;
        } else {
            if (x1 < 0 || x1 >= bounds || y1 < 0 || y1 >= bounds || z1 < 0 || z1 >= bounds) break;
        }

        x = x1; y = y1; z = z1;
        verts.push(x, y, z);
    }

    return new Float32Array(verts);
}

/**
 * Generate seed points for E-field streamlines around particles.
 * 6 seeds per particle (±x, ±y, ±z offset).
 * @param {Array} particles  - Array of { x, y, z, state } from lattice
 * @param {number} offset    - Offset distance in voxels (default 2)
 * @param {number} maxSeeds  - Max total seeds (default 200)
 * @returns {Array<[number,number,number]>}
 */
export function generateEFieldSeeds(particles, offset = 2, maxSeeds = 200) {
    const seeds = [];
    const dirs = [[1,0,0],[-1,0,0],[0,1,0],[0,-1,0],[0,0,1],[0,0,-1]];

    for (const p of particles) {
        if (seeds.length >= maxSeeds) break;
        for (const [dx, dy, dz] of dirs) {
            if (seeds.length >= maxSeeds) break;
            seeds.push([p.x + dx * offset, p.y + dy * offset, p.z + dz * offset]);
        }
    }
    return seeds;
}

/**
 * Generate seed points for B-field streamlines (rings around particles).
 * 8 ring points per particle, perpendicular to flux direction.
 * @param {Array} particles  - Array of { x, y, z, fx, fy, fz } (with flux direction)
 * @param {number} radius    - Ring radius in voxels (default 4)
 * @param {number} maxSeeds  - Max total seeds (default 200)
 * @returns {Array<[number,number,number]>}
 */
export function generateBFieldSeeds(particles, radius = 4, maxSeeds = 200) {
    const seeds = [];
    const nRing = 8;

    for (const p of particles) {
        if (seeds.length >= maxSeeds) break;

        // Flux direction (or default to z-axis if zero)
        let fx = p.fx || 0, fy = p.fy || 0, fz = p.fz || 1;
        const fmag = Math.sqrt(fx * fx + fy * fy + fz * fz);
        if (fmag > 1e-10) { fx /= fmag; fy /= fmag; fz /= fmag; }
        else { fx = 0; fy = 0; fz = 1; }

        // Perpendicular vectors (Gram-Schmidt)
        let ax, ay, az;
        if (Math.abs(fx) < 0.9) { ax = 1; ay = 0; az = 0; }
        else { ax = 0; ay = 1; az = 0; }
        // u = a - (a·f)f
        const dot = ax * fx + ay * fy + az * fz;
        let ux = ax - dot * fx, uy = ay - dot * fy, uz = az - dot * fz;
        const umag = Math.sqrt(ux * ux + uy * uy + uz * uz);
        ux /= umag; uy /= umag; uz /= umag;

        // v = f × u
        const vx = fy * uz - fz * uy;
        const vy = fz * ux - fx * uz;
        const vz = fx * uy - fy * ux;

        for (let k = 0; k < nRing && seeds.length < maxSeeds; k++) {
            const theta = (2 * Math.PI * k) / nRing;
            const cos = Math.cos(theta), sin = Math.sin(theta);
            seeds.push([
                p.x + radius * (cos * ux + sin * vx),
                p.y + radius * (cos * uy + sin * vy),
                p.z + radius * (cos * uz + sin * vz)
            ]);
        }
    }
    return seeds;
}

/**
 * Generate uniform grid seeds for flux streamlines.
 * @param {number} N         - Lattice size
 * @param {number} spacing   - Seed spacing (default 8)
 * @param {number} maxSeeds  - Max seeds (default 200)
 * @returns {Array<[number,number,number]>}
 */
export function generateGridSeeds(N, spacing = 8, maxSeeds = 200) {
    const seeds = [];
    for (let z = spacing / 2; z < N && seeds.length < maxSeeds; z += spacing) {
        for (let y = spacing / 2; y < N && seeds.length < maxSeeds; y += spacing) {
            for (let x = spacing / 2; x < N && seeds.length < maxSeeds; x += spacing) {
                seeds.push([x, y, z]);
            }
        }
    }
    return seeds;
}
