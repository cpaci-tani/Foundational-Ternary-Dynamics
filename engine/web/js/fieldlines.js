// ── fieldlines.js ── Streamline computation for 3D field visualization ──
// RK4 integration through sampled vector fields.
// Returns a POOLED StreamlineResult ({count, buffer, offsets, lengths}) whose
// flat Float32 buffer feeds Three.js LineSegments (see computeStreamlines).
//
// ── Allocation discipline (web/engine-optimization-2026-05-31) ──────────
// computeStreamlines was the dominant Scale-0 overlay cost (~12.8 ms/call at
// L=32) AND the dominant GC source in the overlay path: its hot RK4 loop
// allocated ~10–14 short-lived arrays PER integration step (each fieldFn /
// lookupField / normFieldFn / rk4Step returned a fresh `[x,y,z]`), times
// ~96–144 steps × up to ~300 seeds × 2 directions = millions of arrays per
// call. Plus a fresh `negFieldFn`/`normFieldFn` closure per seed/direction,
// and a fresh spatial index (Array-of-Arrays) rebuilt every call.
//
// The hot path is now allocation-free:
//   • the RK4 integrator mutates MODULE-LEVEL scratch scalars instead of
//     returning arrays — no per-step `[x,y,z]`;
//   • the grid lookup writes its result into the same scratch (lookupFieldInto)
//     instead of returning `[bx,by,bz]`;
//   • the per-seed negation closure is replaced by a `dir ∈ {+1,-1}` sign
//     folded into the (sign-symmetric) normalization, so no closure per seed;
//   • the spatial index is a PERSISTENT module-level structure (flat CSR /
//     counting-sort over reused typed arrays, mirrors mock-lattice-samplers.js)
//     grown on demand, instead of a fresh Array-of-Arrays each call;
//   • per-line vertices accumulate in a reused scratch Float64Array, then are
//     copied ONCE into a POOLED flat Float32 output buffer (a ring of reused
//     StreamlineResult slots) instead of a fresh per-line Float32Array — the
//     last remaining per-call allocation, now also eliminated (see the
//     "Persistent pooled streamline OUTPUT" section near computeStreamlines).
// Output is bit-identical to the previous implementation: same seed order
// (the RNG lives entirely in the seed generators, unchanged), same RK4
// arithmetic in the same evaluation order, same `dir`-folded backward pass
// (`(-v)/|v|` ≡ `-(v/|v|)` in IEEE-754), same boundary truncation, same
// Float32 quantization, same output vertex order.

// ── Named constants ─────────────────────────────────────────────────────
const CELL_SIZE_MIN = 4;            // floor on spatial-index cell size (voxels)
const CELL_SIZE_STRIDE_MULT = 2;    // cell size = max(stride*MULT, MIN)
const RK4_HALF = 0.5;               // RK4 midpoint coefficient (exactly representable)
const RK4_WEIGHT_DIV = 6;           // RK4 weighted-average divisor; kept as a DIVISION
                                    // (h / RK4_WEIGHT_DIV) — NOT h * (1/6) — so the
                                    // rounding is bit-identical to the original `h / 6`.
const DIR_FORWARD = 1;              // forward integration sign
const DIR_BACKWARD = -1;            // backward integration sign (folded negation)
const MIN_VERTS_FLOATS = 3;         // a line shorter than one extra point is empty (> 3 floats kept)

// ══════════════════════════════════════════════════════════════════════
// Persistent spatial index — flat CSR (counting-sort) layout, reused per call
// ──────────────────────────────────────────────────────────────────────
// The old buildFieldIndex allocated a fresh `Array(totalCells)` of empty JS
// arrays plus a `.push` per sample on EVERY computeStreamlines call (one of the
// two big GC sources in this file). Here the index is a flat counting-sort over
// PERSISTENT dense typed arrays, mirroring the codebase's grow-in-place scratch
// pattern (cf. mock-lattice-samplers.js / fillFieldParticleBuf):
//   _cellStart[ci] .. _cellStart[ci]+_cellCount[ci]  → the run of `_order`
//   entries (sample indices) that fall in cell ci.
// The counting sort fills each cell's run in ASCENDING sample-index order —
// exactly the insertion order the old Array-of-Arrays `.push` produced — so the
// nearest-neighbour scan visits samples in the same order and the strict-`<`
// tie-break keeps the identical sample on exact distance ties. That is what
// makes lookupFieldInto bit-identical to the old lookupField.
//
// All four arrays are grown only when `totalCells` / `count` exceed their
// high-water mark, so after warm-up a rebuild allocates nothing. The only
// per-build clear is `_cellCount.fill(0)` over totalCells — for the realistic
// L≤64 / stride≥2 grids totalCells is ≤ a few thousand, vectorized and cheap.
let _cellCount = new Int32Array(0);   // _cellCount[ci]: #samples in cell ci
let _cellStart = new Int32Array(0);   // _cellStart[ci]: start offset into _order
let _cellCursor = new Int32Array(0);  // scratch write-cursor per cell (pass 2)
let _order = new Int32Array(0);       // sample indices, grouped by cell, asc within cell

// The persistent index "view" handed to the integrator. Field refs are
// repointed each rebuild; no new object is allocated per call. The CSR buffers
// (cellStart/cellCount/order) are mirrored here so the hot lookup can pull them
// into true function-locals (faster typed-array element access in V8 than a
// repeated module-global load on every cell).
const _persistIndex = {
    cellSize: 0, gridDim: 0, gridDim2: 0, totalCells: 0,
    positions: null, vectors: null, count: 0,
    cellStart: null, cellCount: null, order: null,
};

function ensureIndexCapacity(totalCells, count) {
    if (_cellCount.length < totalCells) {
        _cellCount = new Int32Array(totalCells);
        _cellStart = new Int32Array(totalCells);
        _cellCursor = new Int32Array(totalCells);
    }
    if (_order.length < count) _order = new Int32Array(count);
}

// Compute the clamped cell index for sample component triple at base `b3`.
// Factored out so the count pass and the placement pass use identical math.
function cellIndexOf(positions, b3, cellSize, gridDim) {
    const cx = Math.floor(positions[b3]     / cellSize) | 0;
    const cy = Math.floor(positions[b3 + 1] / cellSize) | 0;
    const cz = Math.floor(positions[b3 + 2] / cellSize) | 0;
    return Math.min(cx, gridDim - 1) + Math.min(cy, gridDim - 1) * gridDim
         + Math.min(cz, gridDim - 1) * gridDim * gridDim;
}

// Build the PERSISTENT spatial index over (positions, vectors) via counting
// sort. Reuses the module-level dense arrays. Returns _persistIndex (a stable
// module-level object whose refs are repointed in place).
//
// Bucket membership is byte-identical to the old buildFieldIndex: same cellSize,
// same gridDim, same clamped cell index. Per-cell ordering is ascending sample
// index (== old push order), so lookups tie-break identically.
function buildPersistentIndex(positions, vectors, count, N, stride) {
    const cellSize = Math.max(stride * CELL_SIZE_STRIDE_MULT, CELL_SIZE_MIN);
    const gridDim = Math.ceil(N / cellSize);
    const totalCells = gridDim * gridDim * gridDim;
    ensureIndexCapacity(totalCells, count);

    // Pass 1: count per cell. (Clear only the live prefix of _cellCount.)
    _cellCount.fill(0, 0, totalCells);
    for (let i = 0; i < count; i++) {
        const ci = cellIndexOf(positions, i * 3, cellSize, gridDim);
        if (ci >= 0 && ci < totalCells) _cellCount[ci]++;
    }

    // Prefix sum → start offsets; seed the placement cursor from starts.
    let running = 0;
    for (let c = 0; c < totalCells; c++) {
        _cellStart[c] = running;
        _cellCursor[c] = running;
        running += _cellCount[c];
    }

    // Pass 2: scatter sample indices into _order, ascending i within each cell
    // (cursor advances in i order) — matching the old .push insertion order.
    for (let i = 0; i < count; i++) {
        const ci = cellIndexOf(positions, i * 3, cellSize, gridDim);
        if (ci >= 0 && ci < totalCells) _order[_cellCursor[ci]++] = i;
    }

    _persistIndex.cellSize = cellSize;
    _persistIndex.gridDim = gridDim;
    _persistIndex.gridDim2 = gridDim * gridDim;
    _persistIndex.totalCells = totalCells;
    _persistIndex.positions = positions;
    _persistIndex.vectors = vectors;
    _persistIndex.count = count;
    _persistIndex.cellStart = _cellStart;
    _persistIndex.cellCount = _cellCount;
    _persistIndex.order = _order;
    return _persistIndex;
}

// Nearest-sample field lookup against the PERSISTENT index, writing the result
// into module scratch (_fx,_fy,_fz) instead of returning an array. Same 27-cell
// neighbour scan and same nearest-by-squared-distance tie-break order as
// lookupField, so the selected sample (and thus the field value) is identical.
let _fx = 0, _fy = 0, _fz = 0;  // scratch: last sampled raw field components

function lookupFieldInto(index, px, py, pz) {
    // Pull index buffers/sizes into locals: V8 indexes typed-array LOCALS faster
    // than repeated module-global loads, and this is 90%+ of streamline cost.
    const cellSize = index.cellSize;
    const gridDim = index.gridDim;
    const gridDim2 = index.gridDim2;
    const positions = index.positions;
    const vectors = index.vectors;
    const cellStart = index.cellStart;
    const cellCount = index.cellCount;
    const order = index.order;

    const cx = Math.floor(px / cellSize) | 0;
    const cy = Math.floor(py / cellSize) | 0;
    const cz = Math.floor(pz / cellSize) | 0;

    // Clamp the 3×3×3 neighbour box to the grid ONCE, so the cell loops carry no
    // per-cell bounds branch (the old lookupField paid a 3-way `continue` test on
    // every one of the 27 cells). Visitation order is still ascending (nz,ny,nx)
    // over exactly the in-range cells the old code visited → identical nearest
    // sample under the strict-`<` tie-break.
    const zlo = cz > 0 ? cz - 1 : 0, zhi = cz + 1 < gridDim ? cz + 1 : gridDim - 1;
    const ylo = cy > 0 ? cy - 1 : 0, yhi = cy + 1 < gridDim ? cy + 1 : gridDim - 1;
    const xlo = cx > 0 ? cx - 1 : 0, xhi = cx + 1 < gridDim ? cx + 1 : gridDim - 1;

    let bestDist = Infinity;
    let bx = 0, by = 0, bz = 0;

    for (let nz = zlo; nz <= zhi; nz++) {
        const zoff = nz * gridDim2;
        for (let ny = ylo; ny <= yhi; ny++) {
            const yzoff = zoff + ny * gridDim;
            for (let nx = xlo; nx <= xhi; nx++) {
                const ci = yzoff + nx;
                const start = cellStart[ci];
                const end = start + cellCount[ci];
                for (let k = start; k < end; k++) {
                    const i3 = order[k] * 3;
                    const ddx = positions[i3]     - px;
                    const ddy = positions[i3 + 1] - py;
                    const ddz = positions[i3 + 2] - pz;
                    const d2 = ddx * ddx + ddy * ddy + ddz * ddz;
                    if (d2 < bestDist) {
                        bestDist = d2;
                        bx = vectors[i3];
                        by = vectors[i3 + 1];
                        bz = vectors[i3 + 2];
                    }
                }
            }
        }
    }
    _fx = bx; _fy = by; _fz = bz;
}

/**
 * Build a spatial index for fast nearest-neighbor field lookup.
 * Bins sample points into a grid of cells for O(1) lookup.
 *
 * NOTE: this is the original Array-of-Arrays index, retained as exported API
 * for any out-of-tree caller (and for lookupField below). The internal hot
 * path uses buildPersistentIndex / lookupFieldInto instead, which produce
 * byte-identical bucket assignment with zero per-call allocation.
 *
 * @param {Float32Array} positions
 * @param {Float32Array} vectors
 * @param {number}       count
 * @param {number}       N       - Lattice size
 * @param {number}       stride  - Sampling stride
 * @returns {object} Spatial index { grid, cellSize, gridDim, positions, vectors, count }
 */
export function buildFieldIndex(positions, vectors, count, N, stride) {
    const cellSize = Math.max(stride * CELL_SIZE_STRIDE_MULT, CELL_SIZE_MIN);
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
 * Returns a fresh [bx,by,bz] (retained as exported API; the hot path uses the
 * allocation-free lookupFieldInto instead).
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
 *
 * NOTE: retained as exported API. The internal hot path inlines this same
 * arithmetic into integrateGridInto / integrateFieldFnInto (operating on
 * scratch scalars) so it allocates no intermediate `[x,y,z]` arrays; the
 * math/evaluation order is identical, so results match this function
 * bit-for-bit.
 *
 * @param {function} fieldFn  - (x,y,z) => [vx,vy,vz]
 * @param {number}   x,y,z   - Current position
 * @param {number}   h       - Step size
 * @returns {[number,number,number]} New position [x',y',z']
 */
export function rk4Step(fieldFn, x, y, z, h) {
    const [k1x, k1y, k1z] = fieldFn(x, y, z);
    const [k2x, k2y, k2z] = fieldFn(x + RK4_HALF * h * k1x, y + RK4_HALF * h * k1y, z + RK4_HALF * h * k1z);
    const [k3x, k3y, k3z] = fieldFn(x + RK4_HALF * h * k2x, y + RK4_HALF * h * k2y, z + RK4_HALF * h * k2z);
    const [k4x, k4y, k4z] = fieldFn(x + h * k3x, y + h * k3y, z + h * k3z);

    return [
        x + (h / RK4_WEIGHT_DIV) * (k1x + 2 * k2x + 2 * k3x + k4x),
        y + (h / RK4_WEIGHT_DIV) * (k1y + 2 * k2y + 2 * k3y + k4y),
        z + (h / RK4_WEIGHT_DIV) * (k1z + 2 * k2z + 2 * k3z + k4z)
    ];
}

// ══════════════════════════════════════════════════════════════════════
// Allocation-free integrator internals
// ──────────────────────────────────────────────────────────────────────
// Field-sampling mode, selected ONCE per computeStreamlines call. Each mode has
// its OWN specialized integrator (integrateGridInto / integrateFieldFnInto), so
// the field-sampling choice costs no per-step branch — `integrateInto` dispatches
// once per line:
//   MODE_GRID    — nearest-sample lookup against the persistent index (Scale-0
//                  hot path; this is the cost the profile flagged);
//   MODE_FIELDFN — call the caller-supplied fieldFn, read its [x,y,z] return
//                  (PE / Scale-1; fieldFn lives in fields.js and still returns a
//                  fresh array — reading its 3 components is bit-identical to the
//                  old `[vx,vy,vz] = fieldFn(...)`).
const MODE_GRID = 0;
const MODE_FIELDFN = 1;

let _mode = MODE_GRID;
let _gridIndex = null;   // persistent index when _mode === MODE_GRID
let _userFieldFn = null; // caller fieldFn when _mode === MODE_FIELDFN

// Per-line vertex scratch. Grown on demand to hold (maxSteps+1) points × 3
// components in double precision (matching the old JS-number `verts` array),
// then copied to a correctly-sized Float32Array once per line. Reused across
// every line of every call.
let _vertScratch = new Float64Array((100 + 1) * 3);

function ensureVertScratch(maxSteps) {
    const need = (maxSteps + 1) * 3;
    if (_vertScratch.length < need) _vertScratch = new Float64Array(need);
}

// Step-local fallback direction (normalized raw field at the step's start),
// updated once per accepted step and reused by the 4 RK4 sub-evaluations —
// exactly as the old `fbx/fby/fbz` closure capture did.
let _fbx = 0, _fby = 0, _fbz = 0;

// Scratch for the normalized, dir-folded field (output of normGridInto /
// normFieldFnInto). The `dir` fold is the crux of the closure-free backward
// pass: the old code built a per-seed negFieldFn that negated the RAW field
// before normalization; here the backward pass multiplies the NORMALIZED field
// by dir = -1. These are bit-identical in IEEE-754 because (-v)/|v| and
// -(v/|v|) agree exactly (sign-symmetric correctly-rounded division), and the
// magnitude / minMag test is on |v| (sign-independent), so every break/fallback
// decision is unchanged.
let _nx = 0, _ny = 0, _nz = 0;

// Normalized field at (px,py,pz) for the GENERIC (fieldFn) path. Reads the
// caller-supplied fieldFn array into scratch, normalizes with dir folded in.
// Used only by integrateFieldFnInto (the Scale-1 / PE path); the grid hot path
// inlines its own lookup (see normGridInto) to avoid this extra frame.
function normFieldFnInto(fieldFn, px, py, pz, dir, minMag) {
    const v = fieldFn(px, py, pz);
    const vx = v[0], vy = v[1], vz = v[2];
    const m = Math.sqrt(vx * vx + vy * vy + vz * vz);
    if (m < minMag) {
        _nx = _fbx; _ny = _fby; _nz = _fbz; // fallback (already dir-signed)
        return;
    }
    _nx = dir * (vx / m);
    _ny = dir * (vy / m);
    _nz = dir * (vz / m);
}

// Normalized field at (px,py,pz) for the GRID path. Inlines lookupFieldInto's
// nearest-sample scan directly (no wrapper frame) and normalizes with dir
// folded in. `index` is the persistent index; its fields are passed by the
// caller's locals. Writes (_nx,_ny,_nz). Bit-identical to normFieldFnInto over
// the same sampled field because the lookup selects the same sample.
function normGridInto(px, py, pz, dir, minMag) {
    lookupFieldInto(_gridIndex, px, py, pz);
    const vx = _fx, vy = _fy, vz = _fz;
    const m = Math.sqrt(vx * vx + vy * vy + vz * vz);
    if (m < minMag) {
        _nx = _fbx; _ny = _fby; _nz = _fbz;
        return;
    }
    _nx = dir * (vx / m);
    _ny = dir * (vy / m);
    _nz = dir * (vz / m);
}

// Integrate one streamline over the GRID-sampled field from (x0,y0,z0) in
// direction `dir`, writing vertices into _vertScratch and returning the live
// float length. Allocation-free: RK4 intermediates are local scalars; the
// field is sampled into module scratch via the persistent index. The arithmetic
// is the inlined equivalent of integrateDirection + rk4Step(normFieldFn) in the
// same evaluation order, so vertices match the old code bit-for-bit. This is
// the Scale-0 hot path.
function integrateGridInto(x0, y0, z0, h, maxSteps, minMag, bounds, originCentered, dir) {
    const v = _vertScratch;
    v[0] = x0; v[1] = y0; v[2] = z0;
    let len = 3;
    let x = x0, y = y0, z = z0;

    for (let step = 0; step < maxSteps; step++) {
        // Raw field at current position (magnitude/break test + step fallback).
        lookupFieldInto(_gridIndex, x, y, z);
        const vx = _fx, vy = _fy, vz = _fz;
        const mag = Math.sqrt(vx * vx + vy * vy + vz * vz);
        if (mag < minMag) break;

        // Step-local fallback = dir-signed normalized raw field at (x,y,z).
        // dir*(vx/mag) ≡ (dir*vx)/mag bit-for-bit, matching the old fbx/fby/fbz
        // (which for backward came from the pre-negated field, (-vx)/mag).
        _fbx = dir * (vx / mag);
        _fby = dir * (vy / mag);
        _fbz = dir * (vz / mag);

        // ── Inlined RK4 over the normalized (dir-folded) field ──
        normGridInto(x, y, z, dir, minMag);
        const k1x = _nx, k1y = _ny, k1z = _nz;
        normGridInto(x + RK4_HALF * h * k1x, y + RK4_HALF * h * k1y, z + RK4_HALF * h * k1z, dir, minMag);
        const k2x = _nx, k2y = _ny, k2z = _nz;
        normGridInto(x + RK4_HALF * h * k2x, y + RK4_HALF * h * k2y, z + RK4_HALF * h * k2z, dir, minMag);
        const k3x = _nx, k3y = _ny, k3z = _nz;
        normGridInto(x + h * k3x, y + h * k3y, z + h * k3z, dir, minMag);
        const k4x = _nx, k4y = _ny, k4z = _nz;

        const x1 = x + (h / RK4_WEIGHT_DIV) * (k1x + 2 * k2x + 2 * k3x + k4x);
        const y1 = y + (h / RK4_WEIGHT_DIV) * (k1y + 2 * k2y + 2 * k3y + k4y);
        const z1 = z + (h / RK4_WEIGHT_DIV) * (k1z + 2 * k2z + 2 * k3z + k4z);

        if (originCentered) {
            if (x1 * x1 + y1 * y1 + z1 * z1 > bounds * bounds) break;
        } else {
            if (x1 < 0 || x1 >= bounds || y1 < 0 || y1 >= bounds || z1 < 0 || z1 >= bounds) break;
        }

        x = x1; y = y1; z = z1;
        v[len] = x; v[len + 1] = y; v[len + 2] = z;
        len += 3;
    }

    return len;
}

// Integrate one streamline over a caller-supplied fieldFn (PE / Scale-1 path).
// Same RK4 arithmetic and dir-fold as integrateGridInto; differs only in how
// the field is sampled (fieldFn array vs grid lookup). Kept separate so neither
// path pays a per-sample mode branch.
function integrateFieldFnInto(fieldFn, x0, y0, z0, h, maxSteps, minMag, bounds, originCentered, dir) {
    const v = _vertScratch;
    v[0] = x0; v[1] = y0; v[2] = z0;
    let len = 3;
    let x = x0, y = y0, z = z0;

    for (let step = 0; step < maxSteps; step++) {
        const raw = fieldFn(x, y, z);
        const vx = raw[0], vy = raw[1], vz = raw[2];
        const mag = Math.sqrt(vx * vx + vy * vy + vz * vz);
        if (mag < minMag) break;

        _fbx = dir * (vx / mag);
        _fby = dir * (vy / mag);
        _fbz = dir * (vz / mag);

        normFieldFnInto(fieldFn, x, y, z, dir, minMag);
        const k1x = _nx, k1y = _ny, k1z = _nz;
        normFieldFnInto(fieldFn, x + RK4_HALF * h * k1x, y + RK4_HALF * h * k1y, z + RK4_HALF * h * k1z, dir, minMag);
        const k2x = _nx, k2y = _ny, k2z = _nz;
        normFieldFnInto(fieldFn, x + RK4_HALF * h * k2x, y + RK4_HALF * h * k2y, z + RK4_HALF * h * k2z, dir, minMag);
        const k3x = _nx, k3y = _ny, k3z = _nz;
        normFieldFnInto(fieldFn, x + h * k3x, y + h * k3y, z + h * k3z, dir, minMag);
        const k4x = _nx, k4y = _ny, k4z = _nz;

        const x1 = x + (h / RK4_WEIGHT_DIV) * (k1x + 2 * k2x + 2 * k3x + k4x);
        const y1 = y + (h / RK4_WEIGHT_DIV) * (k1y + 2 * k2y + 2 * k3y + k4y);
        const z1 = z + (h / RK4_WEIGHT_DIV) * (k1z + 2 * k2z + 2 * k3z + k4z);

        if (originCentered) {
            if (x1 * x1 + y1 * y1 + z1 * z1 > bounds * bounds) break;
        } else {
            if (x1 < 0 || x1 >= bounds || y1 < 0 || y1 >= bounds || z1 < 0 || z1 >= bounds) break;
        }

        x = x1; y = y1; z = z1;
        v[len] = x; v[len + 1] = y; v[len + 2] = z;
        len += 3;
    }

    return len;
}

// Dispatch one integration to the mode-specialized integrator. Branch is per
// LINE (per direction), not per step — negligible.
function integrateInto(x0, y0, z0, h, maxSteps, minMag, bounds, originCentered, dir) {
    if (_mode === MODE_GRID) {
        return integrateGridInto(x0, y0, z0, h, maxSteps, minMag, bounds, originCentered, dir);
    }
    return integrateFieldFnInto(_userFieldFn, x0, y0, z0, h, maxSteps, minMag, bounds, originCentered, dir);
}

// NOTE: the original internal `integrateDirection(fieldFn, ...)` helper (which
// allocated a JS `verts` array, a per-call `normFieldFn` closure, and per-step
// `[x,y,z]` arrays) has been replaced by the allocation-free, mode-specialized
// integrateGridInto / integrateFieldFnInto above. The exported rk4Step /
// lookupField / buildFieldIndex are retained as the public API.

/**
 * Compute streamlines through a vector field.
 *
 * @param {object}   fieldData   - { positions: Float32Array, vectors: Float32Array, count: number }
 *                                  or { fieldFn: (x,y,z)=>[vx,vy,vz] } for direct-function (PE) mode
 * @param {Array}    seeds       - Array of [x,y,z] seed positions
 * @param {object}   opts        - Options
 * @param {number}   opts.N          - Lattice size (default 32)
 * @param {number}   opts.stride     - Field sampling stride (default 2)
 * @param {number}   opts.maxSteps   - Max integration steps per line (default 100)
 * @param {number}   opts.stepSize   - RK4 step size in voxels (default 0.5)
 * @param {number}   opts.minMag     - Stop if field magnitude drops below (default 1e-10)
 * @param {boolean}  opts.bidirectional - Integrate both directions (default true)
 * @returns {{count:number, buffer:Float32Array, offsets:Int32Array, lengths:Int32Array}}
 *          A POOLED StreamlineResult (reused across calls — see the result-ring
 *          note below; do NOT retain it past the next computeStreamlines call
 *          unless it is the ring's only live result). Line i is the float run
 *          buffer[offsets[i] .. offsets[i]+lengths[i]) laid out
 *          [x0,y0,z0, x1,y1,z1, ...] — the SAME bytes the old per-line
 *          Float32Array held, in the same line order. Iterate i in [0,count);
 *          use lengths[i] (NOT buffer.length — the flat buffer is over-long).
 */
export function computeStreamlines(fieldData, seeds, opts = {}) {
    const {
        N = 32,
        stride = 2,
        maxSteps = 100,
        stepSize = 0.5,
        minMag = 1e-10,
        bidirectional = true,
        bounds = 0,  // if > 0, uses origin-centered sphere bounds instead of lattice [0,N)
        maxLines = 200  // Global cap (callers can raise for large lattices)
    } = opts;

    if (seeds.length === 0) return _emptyResult;

    // Select the field-sampling mode ONCE for this call (no per-step closure).
    // Direct fieldFn (PE mode) vs grid-based nearest-sample lookup (Scale 0).
    if (fieldData && fieldData.fieldFn) {
        _mode = MODE_FIELDFN;
        _userFieldFn = fieldData.fieldFn;
        _gridIndex = null;
    } else {
        if (!fieldData || fieldData.count === 0) return _emptyResult;
        _mode = MODE_GRID;
        _userFieldFn = null;
        _gridIndex = buildPersistentIndex(fieldData.positions, fieldData.vectors, fieldData.count, N, stride);
    }

    const effectiveBounds = bounds > 0 ? bounds : N;
    const originCentered = bounds > 0;

    // Backward integration needs (maxSteps+1) points of scratch just like
    // forward; the combined line is at most fwdLen + bwdLen floats.
    ensureVertScratch(maxSteps);

    // Acquire this call's pooled output slot (next ring slot) and size its flat
    // buffer to the worst case for these opts, so the append loop below never
    // grows it (and thus never allocates) in steady state. Worst case per line
    // is fwd + bwd at full length: 2·(maxSteps+1) points × 3 floats for the
    // bidirectional case, (maxSteps+1)×3 for unidirectional. The line count is
    // bounded by min(seeds, maxLines).
    const lineCap = Math.min(seeds.length, maxLines);
    const perLinePts = bidirectional ? 2 * (maxSteps + 1) : (maxSteps + 1);
    const out = nextResultSlot();
    ensureResultCapacity(out, lineCap * perLinePts * 3, lineCap);
    const buffer = out.buffer;
    const offsets = out.offsets;
    const lengths = out.lengths;
    let count = 0;   // live line count (== old lines.length)
    let cursor = 0;  // running float offset into the flat buffer

    for (let s = 0; s < seeds.length && count < maxLines; s++) {
        const seed = seeds[s];
        const sx = seed[0], sy = seed[1], sz = seed[2];

        if (bidirectional) {
            // FORWARD then BACKWARD, matching the old order (fwd computed first).
            // Both integrate from the same seed over the read-only field, so the
            // two passes are independent; the only reason order matters here is
            // that both write the SHARED _vertScratch. So: run forward, copy its
            // floats out into the persistent _fwdHold scratch (replacing the old
            // per-seed `fwd` Float32Array), then run backward into _vertScratch.
            // The final combined layout is [reversed backward, forward] — byte
            // for byte the old layout.
            const fwdLen = integrateInto(sx, sy, sz, stepSize, maxSteps, minMag, effectiveBounds, originCentered, DIR_FORWARD);
            ensureFwdHold(fwdLen);
            const fwd = _fwdHold;
            for (let i = 0; i < fwdLen; i++) fwd[i] = _vertScratch[i];

            const bwdLen = integrateInto(sx, sy, sz, stepSize, maxSteps, minMag, effectiveBounds, originCentered, DIR_BACKWARD);
            const bwd = _vertScratch;

            // Combine: reverse backward + forward (identical to the old layout).
            // The old code built a per-line `combined = new Float32Array(...)`;
            // here the SAME [reversed-backward, forward] float run is written
            // directly into the pooled flat buffer at `cursor`, recording its
            // offset+length. buffer is Float32 and _vertScratch/_fwdHold are
            // Float64, so each value still takes exactly ONE double→Float32
            // rounding — bit-identical to the old per-line array.
            if (bwdLen > MIN_VERTS_FLOATS || fwdLen > MIN_VERTS_FLOATS) {
                const base = cursor;
                const bwdPts = bwdLen / 3;
                for (let i = 0; i < bwdPts; i++) {
                    const ri = bwdPts - 1 - i;
                    buffer[base + i * 3]     = bwd[ri * 3];
                    buffer[base + i * 3 + 1] = bwd[ri * 3 + 1];
                    buffer[base + i * 3 + 2] = bwd[ri * 3 + 2];
                }
                for (let i = 0; i < fwdLen; i++) buffer[base + bwdLen + i] = fwd[i];
                offsets[count] = base;
                lengths[count] = bwdLen + fwdLen;
                count++;
                cursor = base + bwdLen + fwdLen;
            }
        } else {
            const fwdLen = integrateInto(sx, sy, sz, stepSize, maxSteps, minMag, effectiveBounds, originCentered, DIR_FORWARD);
            if (fwdLen > MIN_VERTS_FLOATS) {
                const base = cursor;
                for (let i = 0; i < fwdLen; i++) buffer[base + i] = _vertScratch[i];
                offsets[count] = base;
                lengths[count] = fwdLen;
                count++;
                cursor = base + fwdLen;
            }
        }
    }

    // Release fieldFn ref so a per-call closure can't be retained between calls.
    _userFieldFn = null;
    _gridIndex = null;

    out.count = count;
    return out;
}

// Secondary persistent scratch holding the forward-pass floats while the
// backward pass overwrites the primary scratch. Grown on demand, reused across
// calls. (Replaces the old per-seed `fwd` Float32Array.)
let _fwdHold = new Float64Array((100 + 1) * 3);
function ensureFwdHold(len) {
    if (_fwdHold.length < len) _fwdHold = new Float64Array(len);
}

// ══════════════════════════════════════════════════════════════════════
// Persistent pooled streamline OUTPUT (web/engine-optimization-2026-05-31)
// ──────────────────────────────────────────────────────────────────────
// The previous pass eliminated all per-RK4-step churn but still returned the
// line set as a FRESH outer Array of FRESH per-line Float32Arrays on every
// call — ~300 lines × 4 streamline overlays = ~1200 Float32Array allocations
// per overlay rebuild. Once the overlay scheduler spread each rebuild across
// many frames, those allocations landed on MORE frames, raising the GC pause
// RATE (+37%) even though the per-frame volume fell. This is the residual the
// previous pass called the "unavoidable output allocation". It is now pooled.
//
// SHAPE returned to consumers (a StreamlineResult, design (b)):
//   { count, buffer:Float32Array, offsets:Int32Array, lengths:Int32Array }
// Line i occupies buffer[offsets[i] .. offsets[i]+lengths[i]) — the SAME
// [x0,y0,z0, x1,y1,z1, ...] float run the old per-line Float32Array held, in
// the same line order, with the same single double→Float32 rounding (the flat
// buffer is Float32; each value is assigned exactly once from the Float64
// _vertScratch/_fwdHold, so the rounding is bit-identical to the old code's
// `combined = new Float32Array(...)` assignment). Consumers iterate [0,count)
// and read buffer[offsets[i]+k]; they MUST use lengths[i], not buffer.length,
// because the flat buffer is grown to a high-water mark and is over-long.
//
// CONCURRENT-HOLD CORRECTNESS (legacy applyOverlayFrame path):
// the live controller path (the amortized scheduler) builds then APPLIES each
// streamline overlay back-to-back in one runJob, so a single persistent result
// would suffice for it. But the retained-as-API applyOverlayFrame path stores
// multiple streamline results simultaneously and applies them later; if they
// aliased ONE persistent object the last-computed would win. So the result
// objects are drawn from a small RING (depth RESULT_RING). Each
// computeStreamlines call advances the ring cursor and returns the next slot,
// whose flat buffer it overwrites. With depth >= the max number of results held
// live at once, distinct concurrently-held results never share a slot → no
// aliasing. Depth budget by path:
//   • live amortized scheduler: builds then APPLIES each streamline overlay
//     back-to-back in one runJob (E, B, flux, and each force-flow are each
//     consumed before the next computeStreamlines call) → only 1 result live;
//   • scale-1 PE: 1 result, consumed immediately;
//   • retained-API applyOverlayFrame + inline-flow buildForceOverlayData: holds
//     E + B + flux (3) PLUS up to 4 force-flow results at once = 7.
// Depth 8 covers the worst case (7) with margin. The ring is allocated lazily
// and each slot grows in place; after warm-up a call reuses its slot's buffers
// and allocates nothing.
const RESULT_RING = 8;
const _resultRing = [];
let _ringCursor = 0;

// A frozen, shared empty result for the early-out cases (no seeds / empty
// field). Reused — never allocated per call — so the empty path is also
// allocation-free. count=0 ⇒ consumers' [0,count) loop is a no-op, matching the
// old `return []` (an empty array's for…of is likewise a no-op). It is a stable
// object (truthy), so the legacy `if (overlayFrame.eFieldLines)` /
// `applyFluxStreamlines(fs.lines, …)` checks behave exactly as before.
const _emptyResult = { count: 0, buffer: new Float32Array(0), offsets: new Int32Array(0), lengths: new Int32Array(0) };

// Acquire the next ring slot, growing the ring by one reusable slot only until
// it reaches depth RESULT_RING (a one-time amortized cost). Buffers start empty
// and are grown on demand by the caller via ensureResultCapacity.
function nextResultSlot() {
    let slot = _resultRing[_ringCursor];
    if (slot === undefined) {
        slot = { count: 0, buffer: new Float32Array(0), offsets: new Int32Array(0), lengths: new Int32Array(0) };
        _resultRing[_ringCursor] = slot;
    }
    _ringCursor = (_ringCursor + 1) % RESULT_RING;
    return slot;
}

// Grow a result slot's flat vertex buffer and per-line side arrays in place.
// `totalFloats` is an upper bound on the concatenated vertex count; `maxLines`
// bounds the offsets/lengths arrays. Grown only past the high-water mark.
function ensureResultCapacity(slot, totalFloats, maxLines) {
    if (slot.buffer.length < totalFloats) slot.buffer = new Float32Array(totalFloats);
    if (slot.offsets.length < maxLines) {
        slot.offsets = new Int32Array(maxLines);
        slot.lengths = new Int32Array(maxLines);
    }
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
                seeds.push([x + 0.5, y + 0.5, z + 0.5]);
            }
        }
    }
    return seeds;
}

// ── Importance sampling helpers (iron-filings visualization) ───────────
// Field-line density should be proportional to |field| so the visualization
// reflects field strength the way iron filings do. We sample seed locations
// from the field-magnitude distribution: voxels with strong field are picked
// more often, voxels with weak field are picked rarely.
//
// Implementation uses a two-pass weighted reservoir:
//   pass 1 — compute |field|^exponent at every sample, sum them
//   pass 2 — pick `count` indices via inverse-CDF on the cumulative weight
// `exponent > 1` exaggerates clustering near sources/poles (filings clump);
// `exponent < 1` evens it out. Default 1.5 matches the look of iron filings
// in a real magnetic field — strong concentration at poles, sparse halo.
//
// NOTE (allocation): this is a SEED generator, not the hot integration loop.
// Its RNG (Math.random) call sequence is load-bearing for output identity —
// the streamlines depend on the exact seeds chosen — so it is left untouched.
// Its modest per-call temporaries (the weights buffer, the seeds list) are not
// the GC hot spot the profile flagged (that was the per-RK4-step arrays inside
// computeStreamlines, now eliminated).

function sampleByFieldMagnitude(fieldData, count, exponent = 1.5, jitter = 0.5) {
    const { positions, vectors, count: nSamples } = fieldData;
    if (!nSamples || count <= 0) return [];

    // Find the minimum and maximum coordinates in positions to identify and exclude
    // boundary voxels (within 1 voxel of the lattice boundaries) to prevent periodic
    // boundary spikes from polluting the importance-sampling weights.
    let minC = Infinity, maxC = -Infinity;
    for (let i = 0; i < nSamples * 3; i++) {
        const v = positions[i];
        if (v < minC) minC = v;
        if (v > maxC) maxC = v;
    }
    const borderMin = minC + 0.5;
    const borderMax = maxC - 0.5;

    // Pass 1: weight per sample
    const weights = new Float32Array(nSamples);
    let total = 0;
    for (let i = 0; i < nSamples; i++) {
        const px = positions[i * 3];
        const py = positions[i * 3 + 1];
        const pz = positions[i * 3 + 2];
        if (px < borderMin || px > borderMax ||
            py < borderMin || py > borderMax ||
            pz < borderMin || pz > borderMax) {
            weights[i] = 0;
            continue;
        }

        const x = vectors[i * 3];
        const y = vectors[i * 3 + 1];
        const z = vectors[i * 3 + 2];
        const m = Math.sqrt(x * x + y * y + z * z);
        const w = Math.pow(m, exponent);
        weights[i] = w;
        total += w;
    }
    if (total <= 0) return [];

    // Pass 2: stratified inverse-CDF — divide [0, total] into `count` strata
    // and pick one sample per stratum. Stratification spreads seeds evenly
    // through the distribution, avoiding the clumping that pure random
    // sampling would produce on top of the iron-filing clumping we WANT.
    const seeds = [];
    const stratum = total / count;
    let cum = 0;
    let target = stratum * Math.random(); // random offset in first stratum
    let s = 0;
    for (let i = 0; i < nSamples && seeds.length < count; i++) {
        cum += weights[i];
        while (cum >= target && seeds.length < count) {
            // small jitter so multiple seeds at the same voxel don't overlap
            const jx = (Math.random() - 0.5) * jitter;
            const jy = (Math.random() - 0.5) * jitter;
            const jz = (Math.random() - 0.5) * jitter;
            seeds.push([
                positions[i * 3]     + jx,
                positions[i * 3 + 1] + jy,
                positions[i * 3 + 2] + jz,
            ]);
            target += stratum;
        }
    }
    return seeds;
}

/**
 * Generate seeds for E-field / divergence-bearing lines using importance
 * sampling: lines start where |E| is strongest. Combined with bidirectional
 * integration this produces lines that visibly originate at sources and
 * terminate at sinks, with density proportional to field strength.
 */
export function generateImportanceSeeds(fieldData, count, exponent = 1.5) {
    return sampleByFieldMagnitude(fieldData, count, exponent, 0.5);
}

/**
 * Generate B-field seeds via importance sampling, then offset each seed
 * perpendicular to the local field direction. This places seeds on the
 * circumference of B's natural loop structure (∇·B = 0 means lines close)
 * rather than at the loop center where integration would just spin in place.
 *
 * `offset` is the perpendicular displacement in voxels — should be a few
 * voxels at small lattices, growing with lattice size for large ones.
 */
export function generateBImportanceSeeds(fieldData, count, offset = 3, exponent = 1.5) {
    const baseSeeds = sampleByFieldMagnitude(fieldData, count, exponent, 0.0);
    if (baseSeeds.length === 0) return [];

    // Build a quick spatial lookup for picking up the local field at each seed.
    // We don't have N here, so use a small radius search through positions.
    const { positions, vectors, count: nSamples } = fieldData;
    const out = [];
    for (const [sx, sy, sz] of baseSeeds) {
        // Find nearest sample to read local field direction.
        // (Linear scan is fine — `count` ≤ 250 and nSamples is bounded by the
        // sampled-stride field, also ≤ a few thousand.)
        let bestD = Infinity, bx = 0, by = 0, bz = 0;
        for (let i = 0; i < nSamples; i++) {
            const dx = positions[i * 3]     - sx;
            const dy = positions[i * 3 + 1] - sy;
            const dz = positions[i * 3 + 2] - sz;
            const d = dx * dx + dy * dy + dz * dz;
            if (d < bestD) {
                bestD = d;
                bx = vectors[i * 3];
                by = vectors[i * 3 + 1];
                bz = vectors[i * 3 + 2];
            }
        }
        const m = Math.sqrt(bx * bx + by * by + bz * bz);
        if (m < 1e-10) { out.push([sx, sy, sz]); continue; }
        // Build a perpendicular axis via Gram-Schmidt against world-X (or Y if B≈X).
        const fx = bx / m, fy = by / m, fz = bz / m;
        let ax, ay, az;
        if (Math.abs(fx) < 0.9) { ax = 1; ay = 0; az = 0; }
        else { ax = 0; ay = 1; az = 0; }
        const dot = ax * fx + ay * fy + az * fz;
        let ux = ax - dot * fx, uy = ay - dot * fy, uz = az - dot * fz;
        const umag = Math.sqrt(ux * ux + uy * uy + uz * uz) || 1;
        ux /= umag; uy /= umag; uz /= umag;
        // Random angle around the local B axis so seeds spread on the loop ring.
        const theta = Math.random() * Math.PI * 2;
        const c = Math.cos(theta), s = Math.sin(theta);
        // v = f × u
        const vx = fy * uz - fz * uy;
        const vy = fz * ux - fx * uz;
        const vz = fx * uy - fy * ux;
        out.push([
            sx + offset * (c * ux + s * vx),
            sy + offset * (c * uy + s * vy),
            sz + offset * (c * uz + s * vz),
        ]);
    }
    return out;
}
