// engine/web/js/scales/scale0/runtime/field-line-knots.js
//
// Field-line KNOT detection + quantification + identity tracking — JS-native.
//
// A "knot" is a coarse-grid region where RENDERED field-lines BUNCH densely
// (and, optionally, CROSS). Adaptive detection seeds local density maxima so a
// strong clump cannot hide a weaker isolated one. Knots are detected from the
// pooled StreamlineResult geometry alone — independent of manifested particles,
// so they are found even when Particles:0 (e.g. "Tau neutrino in vacuum").
//
// Each knot is quantified with the Feynman-diagram ANALOGY: segment count,
// crossings (vertices), legs (connections to other knots), total length, plus
// spatial extent, net flux and dominant direction. Knots are matched across
// stream-line rebuilds to assign persistent IDs and a birth/death/fission/
// fusion lifecycle — a JS port of the C++ cluster_tracker.h + cluster_genealogy.h
// overlap-matching. Observation-only: reads the streamline buffer + field
// samples, never the engine state; no golden-hash coupling.
//
// IMPORTANT: the pooled StreamlineResult is recycled by the field-overlay ring;
// record() reads it synchronously and retains NOTHING.

import { attributeSegmentsToKnots } from './knot-line-attribution.js';
import { RingBuffer } from '../../../telemetry-hub.js';

// Event-type integers — must match the panel's EVENT_NAMES / the C++ enum.
const EV_BIRTH = 0, EV_DEATH = 1, EV_PERSIST = 2, EV_FISSION = 3, EV_FUSION = 4, EV_AMBIG = 5;

const HIST_LEN = 240;                 // per-knot contribution history depth (ticks)
const FLUX_DENSE_MAX_N = 128;         // above this, skip the O(N³) dense flux integral

export class FieldLineKnotTracker {
    constructor(opts = {}) {
        this.cellSize = opts.cellSize ?? 2;   // finer grid separates adjacent bundles
        this.densityThreshold = opts.densityThreshold ?? null; // null → adaptive
        this.crossingThreshold = opts.crossingThreshold ?? 1;
        this.minCellsPerKnot = opts.minCellsPerKnot ?? 1;
        this.crossingDist = opts.crossingDist ?? 1.0;
        // Two segments count as a CROSSING only if they pass within crossingDist
        // AND are not near-parallel (|cosθ| < parallelCos). The parallel cut is
        // what rejects a dense PARALLEL bundle (bunching, not a tangle).
        this.parallelCos = opts.parallelCos ?? 0.9;
        this.maxKnots = opts.maxKnots ?? 256;
        this.minOverlapCells = opts.minOverlapCells ?? 1;
        this.maxEvents = opts.maxEvents ?? 200;
        // Detection mode: when requireCrossings is OFF (default), a knot is any
        // dense field-line clump — including a parallel BUNDLE that bunches but
        // doesn't tangle. When ON, only genuine tangles (crossings) qualify.
        // Crossings are always COUNTED for the per-knot diagram either way.
        this.requireCrossings = opts.requireCrossings ?? false;
        this.reset();
        // User preferences (survive reset / scenario change).
        this._perKnotColor = opts.perKnotColor ?? true;
        // Sensitivity ∈ [0,1]: higher → lower density threshold → more knots.
        this._sensitivity = clamp01(opts.sensitivity ?? 0.5);
        // Scientific contribution measurement runs only while the panel is live.
        this._contribEnabled = false;
    }

    reset() {
        this._prevCellToId = new Map();  // cellIndex → knotId (previous record)
        this._prevIdSize = new Map();    // knotId → cellCount (previous record)
        this._histories = new Map();     // id → { birth, peak, lastTick }
        this._nextId = 0;
        this._agg = { alive: 0, births: 0, deaths: 0, fissions: 0, fusions: 0, sumSegs: 0, found: 0, dropped: 0 };
        this._events = [];
        this._tel = emptyTelemetry();
        this._zones = { count: 0, centroids: new Float32Array(0), extents: new Float32Array(0), ids: new Int32Array(0), latticeSize: 0 };
        this._selectedId = -1;           // panel-selected knot id (-1 = none)
        this._contrib = emptyContrib();          // last per-knot field contributions
        this._contribHistory = new Map();        // id → { energyFrac,fluxFrac,chargeFrac: RingBuffer }
        // grown-on-demand scratch (per-cell + per-segment)
        this._density = null; this._cross = null; this._hot = null;
        this._fx = null; this._fy = null; this._fz = null;
        this._comp = null; this._stack = null;
        this._segEnds = null; this._segCell = null;
        this._cellHasSeg = null; this._cellSegHead = null; this._cellSegNext = null;
        this._capCells = 0; this._capSegs = 0;
    }

    // streamlines : pooled { count, buffer, offsets, lengths } (NOT retained)
    // fieldSamples: { positions, vectors, count } in lattice coords, or null
    // tick        : integer engine tick
    // latticeSize : N
    record(streamlines, fieldSamples, tick, latticeSize) {
        const N = latticeSize | 0;
        const cs = this.cellSize;
        const G = Math.max(1, Math.ceil(N / cs));
        const totalCells = G * G * G;
        this._ensureCells(totalCells);

        const density = this._density, cross = this._cross, hot = this._hot;
        const fx = this._fx, fy = this._fy, fz = this._fz;
        density.fill(0, 0, totalCells); cross.fill(0, 0, totalCells); hot.fill(0, 0, totalCells);
        fx.fill(0, 0, totalCells); fy.fill(0, 0, totalCells); fz.fill(0, 0, totalCells);

        const cellOf = (x, y, z) => {
            let cx = (x / cs) | 0, cy = (y / cs) | 0, cz = (z / cs) | 0;
            if (cx < 0) cx = 0; else if (cx >= G) cx = G - 1;
            if (cy < 0) cy = 0; else if (cy >= G) cy = G - 1;
            if (cz < 0) cz = 0; else if (cz >= G) cz = G - 1;
            return (cz * G + cy) * G + cx;
        };

        // ── Pass 1: segments → density + flat segment store ──────────────
        let S = 0;
        if (streamlines && streamlines.count) {
            const { count, buffer, offsets, lengths } = streamlines;
            // count segments first to size scratch
            let segTotal = 0;
            for (let i = 0; i < count; i++) segTotal += Math.max(0, (lengths[i] / 3 | 0) - 1);
            this._ensureSegs(segTotal);
            const segEnds = this._segEnds, segCell = this._segCell;
            for (let i = 0; i < count; i++) {
                const start = offsets[i], len = lengths[i];
                for (let p = start; p + 5 < start + len; p += 3) {
                    const ax = buffer[p], ay = buffer[p + 1], az = buffer[p + 2];
                    const bx = buffer[p + 3], by = buffer[p + 4], bz = buffer[p + 5];
                    const mc = cellOf((ax + bx) * 0.5, (ay + by) * 0.5, (az + bz) * 0.5);
                    density[mc] += 1;
                    const o = S * 6;
                    segEnds[o] = ax; segEnds[o + 1] = ay; segEnds[o + 2] = az;
                    segEnds[o + 3] = bx; segEnds[o + 4] = by; segEnds[o + 5] = bz;
                    segCell[S] = mc;
                    S++;
                }
            }
        }

        // ── Net flux per cell (from field samples) ───────────────────────
        if (fieldSamples && fieldSamples.count) {
            const pos = fieldSamples.positions, vec = fieldSamples.vectors;
            const n = fieldSamples.count;
            for (let s = 0; s < n; s++) {
                const ci = cellOf(pos[s * 3], pos[s * 3 + 1], pos[s * 3 + 2]);
                fx[ci] += vec[s * 3]; fy[ci] += vec[s * 3 + 1]; fz[ci] += vec[s * 3 + 2];
            }
        }

        // ── Density floor ────────────────────────────────────────────────
        // Explicit densityThreshold keeps the global cut (unit tests pin it).
        // Adaptive (production): a sensitivity-scaled floor, then local-maxima
        // basins — a busy clump must not hide a weaker isolated one.
        const adaptive = this.densityThreshold == null;
        const dMin = adaptive
            ? Math.max(1, Math.round(4 - 4 * this._sensitivity))   // sens 0→4, 0.5→2, 1→1
            : this.densityThreshold;

        // ── Pass 2: crossings, only for density-passing cells ────────────
        // Bucket segments into per-cell singly-linked lists, then count
        // non-parallel near-intersections within each candidate cell.
        if (S > 0) {
            const cellHead = this._cellSegHead, segNext = this._cellSegNext;
            cellHead.fill(-1, 0, totalCells);
            const segCell = this._segCell;
            for (let s = 0; s < S; s++) { segNext[s] = cellHead[segCell[s]]; cellHead[segCell[s]] = s; }
            const segEnds = this._segEnds;
            const cd2 = this.crossingDist * this.crossingDist;
            for (let c = 0; c < totalCells; c++) {
                if (density[c] < dMin) continue;
                // collect this cell's segments
                let xings = 0;
                for (let s = cellHead[c]; s !== -1; s = segNext[s]) {
                    for (let t = segNext[s]; t !== -1; t = segNext[t]) {
                        if (segsCross(segEnds, s, t, cd2, this.parallelCos)) xings++;
                    }
                }
                cross[c] = xings;
            }
        }

        let comps;
        if (adaptive) {
            comps = this._peakBasins(density, G, totalCells, dMin);
            if (this.requireCrossings) {
                comps = comps.filter((cc) => {
                    let xs = 0;
                    for (let j = 0; j < cc.cells.length; j++) xs += cross[cc.cells[j]];
                    return xs >= this.crossingThreshold;
                });
            }
        } else {
            const needX = this.requireCrossings;
            for (let c = 0; c < totalCells; c++) {
                if (density[c] >= dMin && (!needX || cross[c] >= this.crossingThreshold)) hot[c] = 1;
            }
            comps = this._floodFill(hot, G, totalCells);
        }
        // drop tiny, keep largest maxKnots (report how many real clumps we hid)
        let knots = comps.filter((cc) => cc.cells.length >= this.minCellsPerKnot);
        const found = knots.length;
        let dropped = 0;
        if (knots.length > this.maxKnots) {
            knots.sort((a, b) => b.cells.length - a.cells.length);
            dropped = knots.length - this.maxKnots;
            knots = knots.slice(0, this.maxKnots);
        }
        const K = knots.length;

        // ── Per-knot geometry ────────────────────────────────────────────
        const centroids = new Float32Array(K * 3);
        const extents = new Float32Array(K * 3);
        const dirs = new Float32Array(K * 3);
        const fluxMag = new Float32Array(K);
        const xPerKnot = new Int32Array(K);
        const sizes = new Int32Array(K);
        const half = cs * 0.5;
        for (let k = 0; k < K; k++) {
            const cells = knots[k].cells;
            sizes[k] = cells.length;
            let wsum = 0, cxA = 0, cyA = 0, czA = 0;
            let minx = Infinity, miny = Infinity, minz = Infinity;
            let maxx = -Infinity, maxy = -Infinity, maxz = -Infinity;
            let fX = 0, fY = 0, fZ = 0, xs = 0;
            for (let j = 0; j < cells.length; j++) {
                const ci = cells[j];
                const gx = ci % G, gy = ((ci / G) | 0) % G, gz = (ci / (G * G)) | 0;
                const wcx = (gx + 0.5) * cs, wcy = (gy + 0.5) * cs, wcz = (gz + 0.5) * cs;
                const w = density[ci];
                wsum += w; cxA += wcx * w; cyA += wcy * w; czA += wcz * w;
                if (wcx < minx) minx = wcx; if (wcx > maxx) maxx = wcx;
                if (wcy < miny) miny = wcy; if (wcy > maxy) maxy = wcy;
                if (wcz < minz) minz = wcz; if (wcz > maxz) maxz = wcz;
                fX += fx[ci]; fY += fy[ci]; fZ += fz[ci];
                xs += cross[ci];
            }
            centroids[k * 3] = wsum ? cxA / wsum : 0;
            centroids[k * 3 + 1] = wsum ? cyA / wsum : 0;
            centroids[k * 3 + 2] = wsum ? czA / wsum : 0;
            extents[k * 3] = (maxx - minx) * 0.5 + half;
            extents[k * 3 + 1] = (maxy - miny) * 0.5 + half;
            extents[k * 3 + 2] = (maxz - minz) * 0.5 + half;
            const fm = Math.sqrt(fX * fX + fY * fY + fZ * fZ);
            fluxMag[k] = fm;
            if (fm > 1e-9) { dirs[k * 3] = fX / fm; dirs[k * 3 + 1] = fY / fm; dirs[k * 3 + 2] = fZ / fm; }
            xPerKnot[k] = xs;
        }

        // ── Segments / length / legs via the shared attribution helper ───
        const attr = attributeSegmentsToKnots(streamlines, centroids, K);
        const segCount = new Int32Array(K), legCount = new Int32Array(K);
        const segLen = new Float32Array(K);
        let sumSegs = 0;
        for (let k = 0; k < K; k++) {
            const rec = attr.get(k);
            segCount[k] = rec ? rec.segments : 0;
            segLen[k] = rec ? rec.length : 0;
            legCount[k] = rec ? rec.legSet.size : 0;
            sumSegs += segCount[k];
        }

        // ── Identity matching (births/deaths/fission/fusion) ─────────────
        const cellSets = knots.map((cc) => cc.cells);
        const ids = this._matchAndUpdate(cellSets, sizes, tick);

        // ── Assemble telemetry ───────────────────────────────────────────
        const STRIDE = 8;
        const fields = new Float32Array(K * STRIDE);
        const age = new Int32Array(K), peak = new Int32Array(K), birth = new Int32Array(K);
        for (let k = 0; k < K; k++) {
            const h = this._histories.get(ids[k]);
            age[k] = h ? (tick - h.birth) : 0;
            peak[k] = h ? h.peak : sizes[k];
            birth[k] = h ? h.birth : tick;
            const o = k * STRIDE;
            fields[o] = centroids[k * 3]; fields[o + 1] = centroids[k * 3 + 1]; fields[o + 2] = centroids[k * 3 + 2];
            fields[o + 3] = segCount[k]; fields[o + 4] = xPerKnot[k]; fields[o + 5] = legCount[k];
            fields[o + 6] = segLen[k]; fields[o + 7] = fluxMag[k];
        }
        const idsCopy = Int32Array.from(ids);
        this._tel = { count: K, ids: idsCopy, age, size: sizes, peak, birth,
                      stride: STRIDE, fields, dirs, extents, found, dropped };
        this._zones = { count: K, centroids, extents, ids: idsCopy, latticeSize: N };
        this._agg.alive = K;
        this._agg.sumSegs = sumSegs;
        this._agg.found = found;
        this._agg.dropped = dropped;
        return this._tel;
    }

    getTelemetry() { return this._tel; }
    getAggregate() { return { ...this._agg }; }
    getKnotZones() { return { ...this._zones, selectedId: this._selectedId, perKnotColor: this._perKnotColor }; }
    setSelected(id) { this._selectedId = (id === null || id === undefined) ? -1 : (id | 0); }
    getSelected() { return this._selectedId; }
    setPerKnotColor(on) { this._perKnotColor = !!on; }
    getPerKnotColor() { return this._perKnotColor; }
    setSensitivity(v) { this._sensitivity = clamp01(v); }
    getSensitivity() { return this._sensitivity; }
    setRequireCrossings(on) { this.requireCrossings = !!on; }

    // Assign each streamline to a knot (the nearest knot centroid to the line's
    // midpoint), returning that knot's persistent id per line, or -1 if there are
    // no knots. Lets the E-field renderer color each flowline to match the panel
    // row + box of the knot it belongs to. Reads only the last-recorded zones.
    assignLinesToKnots(streamlines) {
        const n = (streamlines && streamlines.count) ? streamlines.count : 0;
        const out = new Int32Array(n).fill(-1);
        const K = this._zones.count;
        if (!n || !K) return out;
        const ids = this._zones.ids, cen = this._zones.centroids;
        const { buffer, offsets, lengths } = streamlines;
        for (let li = 0; li < n; li++) {
            const start = offsets[li], len = lengths[li];
            const mv = start + (((len / 3) >> 1) * 3);   // midpoint vertex
            const x = buffer[mv], y = buffer[mv + 1], z = buffer[mv + 2];
            let best = -1, bestD = Infinity;
            for (let k = 0; k < K; k++) {
                const dx = x - cen[k * 3], dy = y - cen[k * 3 + 1], dz = z - cen[k * 3 + 2];
                const d = dx * dx + dy * dy + dz * dz;
                if (d < bestD) { bestD = d; best = k; }
            }
            out[li] = best >= 0 ? ids[best] : -1;
        }
        return out;
    }

    // ── Scientific contributions: each knot's share of the scenario field totals ──
    // GENUINE engine-field integrals over each knot's centroid±extent box (not the
    // seeding-dependent geometric counts). Live measurement uses the overlay's
    // stride-sampled E/B/J/divJ records with physical cell-volume weights, avoiding
    // a full N³ main-thread read. Explicit dense/compact flux-volume callers retain
    // their original exact/weighted integration paths.
    // Each sample/voxel is assigned to ONE knot (nearest containing box) so the
    // fractions never double-count and Σ frac == captured.
    measureContributions({ eField, bField, fluxField, fluxVolume, divJ, latticeSize, sampleStride = 1, tick = null }) {
        const K = this._zones.count;
        const cen = this._zones.centroids, ext = this._zones.extents, zids = this._zones.ids;
        const N = latticeSize | 0;
        const energy = new Float64Array(K), flux = new Float64Array(K), charge = new Float64Array(K);
        let totE = 0, totF = 0, totQ = 0;

        const knotAt = (x, y, z) => {                       // nearest knot whose box contains (x,y,z), or -1
            let best = -1, bestD = Infinity;
            for (let k = 0; k < K; k++) {
                const dx = Math.abs(x - cen[k * 3]), dy = Math.abs(y - cen[k * 3 + 1]), dz = Math.abs(z - cen[k * 3 + 2]);
                if (dx <= ext[k * 3] && dy <= ext[k * 3 + 1] && dz <= ext[k * 3 + 2]) {
                    const d = dx * dx + dy * dy + dz * dz;
                    if (d < bestD) { bestD = d; best = k; }
                }
            }
            return best;
        };

        const strideFor = (samp) => Math.max(1,
            Math.trunc(Number(samp?.effectiveStride) || Number(sampleStride) || 1));
        const sampleWeight = (pos, i, stride) => {
            if (stride <= 1 || N <= 0) return 1;
            const x = Math.max(0, Math.min(N - 1, Math.round(pos[i * 3])));
            const y = Math.max(0, Math.min(N - 1, Math.round(pos[i * 3 + 1])));
            const z = Math.max(0, Math.min(N - 1, Math.round(pos[i * 3 + 2])));
            return Math.min(stride, N - x) * Math.min(stride, N - y) * Math.min(stride, N - z);
        };

        const addVecEnergy = (samp) => {                   // ½|v|² times represented cell volume
            if (!samp || !samp.count || !samp.vectors) return;
            const pos = samp.positions, vec = samp.vectors, m = samp.count;
            const sampStride = strideFor(samp);
            for (let i = 0; i < m; i++) {
                const vx = vec[i * 3], vy = vec[i * 3 + 1], vz = vec[i * 3 + 2];
                const e = 0.5 * (vx * vx + vy * vy + vz * vz) * sampleWeight(pos, i, sampStride);
                totE += e;
                if (K) { const k = knotAt(pos[i * 3], pos[i * 3 + 1], pos[i * 3 + 2]); if (k >= 0) energy[k] += e; }
            }
        };
        addVecEnergy(eField); addVecEnergy(bField);

        if (divJ && divJ.count && divJ.values) {           // charge |∇·J|
            const pos = divJ.positions, val = divJ.values, m = divJ.count;
            const divStride = strideFor(divJ);
            for (let i = 0; i < m; i++) {
                const q = Math.abs(val[i]) * sampleWeight(pos, i, divStride); totQ += q;
                if (K) { const k = knotAt(pos[i * 3], pos[i * 3 + 1], pos[i * 3 + 2]); if (k >= 0) charge[k] += q; }
            }
        }

        const compactFlux = fluxVolume && !ArrayBuffer.isView(fluxVolume)
            && ArrayBuffer.isView(fluxVolume.data) ? fluxVolume : null;
        const compactAxis = Math.trunc(Number(compactFlux?.axisCount) || 0);
        const compactStride = Math.max(1, Number(compactFlux?.stride) || 1);
        const compactValid = compactFlux
            && Math.trunc(Number(compactFlux.latticeSize)) === N
            && compactAxis > 0
            && compactFlux.data.length === compactAxis * compactAxis * compactAxis;
        let fluxMode = 'none';
        if (fluxField?.count && fluxField.positions && fluxField.vectors) {
            fluxMode = 'sampled-vector';
            const pos = fluxField.positions, vec = fluxField.vectors, m = fluxField.count;
            const fluxStride = strideFor(fluxField);
            for (let i = 0; i < m; i++) {
                const vx = vec[i * 3], vy = vec[i * 3 + 1], vz = vec[i * 3 + 2];
                const f = Math.hypot(vx, vy, vz) * sampleWeight(pos, i, fluxStride);
                totF += f;
                if (K) { const k = knotAt(pos[i * 3], pos[i * 3 + 1], pos[i * 3 + 2]); if (k >= 0) flux[k] += f; }
            }
        } else if (compactValid) {
            fluxMode = 'compact-volume';
            // FTV2 represents each retained sample's integer-stride cell. Use
            // the covered cell volume as an integration weight, including the
            // shorter terminal cells when N is not divisible by the stride.
            for (let zi = 0; zi < compactAxis; zi++) {
                const z = Math.min(zi * compactStride, N - 1);
                const wz = Math.min(compactStride, N - z);
                for (let yi = 0; yi < compactAxis; yi++) {
                    const y = Math.min(yi * compactStride, N - 1);
                    const wy = Math.min(compactStride, N - y);
                    const base = (zi * compactAxis + yi) * compactAxis;
                    for (let xi = 0; xi < compactAxis; xi++) {
                        const x = Math.min(xi * compactStride, N - 1);
                        const wx = Math.min(compactStride, N - x);
                        const f = compactFlux.data[base + xi] * wx * wy * wz;
                        totF += f;
                        if (K) { const k = knotAt(x, y, z); if (k >= 0) flux[k] += f; }
                    }
                }
            }
        } else if (fluxVolume && N > 0 && fluxVolume.length >= N * N * N && N <= FLUX_DENSE_MAX_N) {  // flux |J| exact
            fluxMode = 'dense-volume';
            for (let z = 0; z < N; z++) for (let y = 0; y < N; y++) {
                const base = (z * N + y) * N;
                for (let x = 0; x < N; x++) {
                    const f = fluxVolume[base + x]; totF += f;
                    if (K) { const k = knotAt(x, y, z); if (k >= 0) flux[k] += f; }
                }
            }
        }

        const energyFrac = new Float64Array(K), fluxFrac = new Float64Array(K), chargeFrac = new Float64Array(K);
        let capE = 0, capF = 0, capQ = 0;
        for (let k = 0; k < K; k++) {
            energyFrac[k] = totE > 0 ? energy[k] / totE : 0;
            fluxFrac[k] = totF > 0 ? flux[k] / totF : 0;
            chargeFrac[k] = totQ > 0 ? charge[k] / totQ : 0;
            capE += energy[k]; capF += flux[k]; capQ += charge[k];
        }
        const ids = Int32Array.from(zids);
        this._contrib = {
            count: K, ids, energy, flux, charge, energyFrac, fluxFrac, chargeFrac,
            totals: { energy: totE, flux: totF, charge: totQ },
            captured: {
                energyFrac: totE > 0 ? capE / totE : 0,
                fluxFrac: totF > 0 ? capF / totF : 0,
                chargeFrac: totQ > 0 ? capQ / totQ : 0,
            },
            sampling: {
                energyStride: Math.max(strideFor(eField), strideFor(bField)),
                fluxStride: fluxMode === 'sampled-vector' ? strideFor(fluxField)
                    : fluxMode === 'compact-volume' ? compactStride : 1,
                chargeStride: strideFor(divJ),
                fluxMode,
                approximate: Math.max(strideFor(eField), strideFor(bField), strideFor(divJ),
                    fluxMode === 'sampled-vector' ? strideFor(fluxField) : compactStride) > 1,
            },
        };

        // accumulate per-knot history; prune knots that died
        const alive = new Set();
        for (let k = 0; k < K; k++) {
            const id = ids[k]; alive.add(id);
            let h = this._contribHistory.get(id);
            if (!h) { h = { energyFrac: new RingBuffer(HIST_LEN), fluxFrac: new RingBuffer(HIST_LEN), chargeFrac: new RingBuffer(HIST_LEN) }; this._contribHistory.set(id, h); }
            h.energyFrac.push(energyFrac[k], tick);
            h.fluxFrac.push(fluxFrac[k], tick);
            h.chargeFrac.push(chargeFrac[k], tick);
        }
        for (const id of this._contribHistory.keys()) if (!alive.has(id)) this._contribHistory.delete(id);
        return this._contrib;
    }

    getContributions() { return this._contrib; }

    getKnotHistory(id) {
        const h = this._contribHistory.get(id);
        if (!h) return { n: 0, ticks: new Float64Array(0), energyFrac: new Float32Array(0), fluxFrac: new Float32Array(0), chargeFrac: new Float32Array(0) };
        const n = h.energyFrac.count;
        const ticks = new Float64Array(n);
        const ef = new Float32Array(n), ff = new Float32Array(n), cf = new Float32Array(n);
        h.energyFrac.flattenTicksInto(ticks, n);
        h.energyFrac.flattenInto(ef, n); h.fluxFrac.flattenInto(ff, n); h.chargeFrac.flattenInto(cf, n);
        return { n, ticks, energyFrac: ef, fluxFrac: ff, chargeFrac: cf };
    }

    setContribEnabled(on) { this._contribEnabled = !!on; }
    isContribEnabled() { return this._contribEnabled; }

    getEvents() {
        const n = this._events.length;
        const tickA = new Int32Array(n), typeA = new Int32Array(n);
        const npA = new Int32Array(n), ncA = new Int32Array(n);
        for (let i = 0; i < n; i++) {
            tickA[i] = this._events[i].tick; typeA[i] = this._events[i].type;
            npA[i] = this._events[i].np; ncA[i] = this._events[i].nc;
        }
        return { count: n, tick: tickA, type: typeA, nparents: npA, nchildren: ncA };
    }

    // ── identity: DSU bipartite overlap of prev ids ↔ current knots ──────
    _matchAndUpdate(cellSets, sizes, tick) {
        const K = cellSets.length;
        // overlap[k] : Map<prevId, sharedCells>
        const overlap = new Array(K);
        const parentIdSet = new Set();
        for (let k = 0; k < K; k++) {
            const m = new Map();
            const cells = cellSets[k];
            for (let j = 0; j < cells.length; j++) {
                const pid = this._prevCellToId.get(cells[j]);
                if (pid === undefined) continue;
                m.set(pid, (m.get(pid) || 0) + 1);
            }
            overlap[k] = m;
            for (const [pid, c] of m) if (c >= this.minOverlapCells) parentIdSet.add(pid);
        }
        const parentIds = [...parentIdSet];
        const P = parentIds.length;
        const pidIndex = new Map(); // prevId → node index
        for (let i = 0; i < P; i++) pidIndex.set(parentIds[i], K + i);

        // DSU over nodes [0..K) children, [K..K+P) parents
        const parent = new Int32Array(K + P);
        for (let i = 0; i < K + P; i++) parent[i] = i;
        const find = (x) => { while (parent[x] !== x) { parent[x] = parent[parent[x]]; x = parent[x]; } return x; };
        const union = (a, b) => { const ra = find(a), rb = find(b); if (ra !== rb) parent[ra] = rb; };
        for (let k = 0; k < K; k++) {
            for (const [pid, c] of overlap[k]) {
                if (c >= this.minOverlapCells && pidIndex.has(pid)) union(k, pidIndex.get(pid));
            }
        }

        // group nodes by root
        const groups = new Map(); // root → { kids:[k...], pids:[id...] }
        for (let k = 0; k < K; k++) {
            const r = find(k);
            let g = groups.get(r); if (!g) { g = { kids: [], pids: [] }; groups.set(r, g); }
            g.kids.push(k);
        }
        for (let i = 0; i < P; i++) {
            const r = find(K + i);
            let g = groups.get(r); if (!g) { g = { kids: [], pids: [] }; groups.set(r, g); }
            g.pids.push(parentIds[i]);
        }

        const ids = new Int32Array(K);
        const usedParents = new Set();
        for (const g of groups.values()) {
            const np = g.pids.length, nc = g.kids.length;
            if (nc === 0) continue; // pure-parent group → all its parents die (handled below)
            g.pids.forEach((p) => usedParents.add(p));
            // largest child inherits the biggest prev id
            const kids = g.kids.slice().sort((a, b) => sizes[b] - sizes[a]);
            let inherit = -1, bestSz = -1;
            for (const p of g.pids) { const sz = this._prevIdSize.get(p) || 0; if (sz > bestSz) { bestSz = sz; inherit = p; } }
            for (let j = 0; j < kids.length; j++) {
                const k = kids[j];
                if (j === 0 && inherit >= 0) ids[k] = inherit;
                else { ids[k] = this._nextId++; this._agg.births++; }
            }
            // classify the group event
            let type = EV_PERSIST;
            if (np === 0) type = EV_BIRTH;
            else if (np === 1 && nc === 1) type = EV_PERSIST;
            else if (np === 1 && nc >= 2) { type = EV_FISSION; this._agg.fissions++; }
            else if (np >= 2 && nc === 1) { type = EV_FUSION; this._agg.fusions++; }
            else type = EV_AMBIG;
            if (type !== EV_PERSIST) this._pushEvent(tick, type, np, nc);
        }

        // deaths: prev ids that were never claimed by a surviving group
        for (const pid of this._prevIdSize.keys()) {
            if (!usedParents.has(pid)) {
                this._agg.deaths++;
                this._pushEvent(tick, EV_DEATH, 1, 0);
                this._histories.delete(pid);
            }
        }

        // update histories + rebuild prev maps
        const nextCellToId = new Map();
        const nextIdSize = new Map();
        for (let k = 0; k < K; k++) {
            const id = ids[k];
            let h = this._histories.get(id);
            if (!h) { h = { birth: tick, peak: sizes[k], lastTick: tick }; this._histories.set(id, h); }
            else { if (sizes[k] > h.peak) h.peak = sizes[k]; h.lastTick = tick; }
            const cells = cellSets[k];
            for (let j = 0; j < cells.length; j++) nextCellToId.set(cells[j], id);
            nextIdSize.set(id, sizes[k]);
        }
        this._prevCellToId = nextCellToId;
        this._prevIdSize = nextIdSize;
        return ids;
    }

    _pushEvent(tick, type, np, nc) {
        this._events.push({ tick, type, np, nc });
        if (this._events.length > this.maxEvents) this._events.shift();
    }

    // Local maxima of density (>= dMin) become seeds; each floods downhill
    // through neighbours that stay >= dMin so a ridge stays one knot and a
    // valley between two peaks keeps them separate.
    _peakBasins(density, G, totalCells, dMin) {
        const seeds = [];
        for (let c = 0; c < totalCells; c++) {
            if (density[c] < dMin) continue;
            if (this._isLocalMax(c, density, G)) seeds.push(c);
        }
        seeds.sort((a, b) => density[b] - density[a] || a - b);
        const claimed = this._hot;
        claimed.fill(0, 0, totalCells);
        const stack = this._stack;
        const out = [];
        for (let s = 0; s < seeds.length; s++) {
            const seed = seeds[s];
            if (claimed[seed]) continue;
            const cells = [];
            let sp = 0;
            stack[sp++] = seed;
            claimed[seed] = 1;
            while (sp > 0) {
                const cur = stack[--sp];
                cells.push(cur);
                const gx = cur % G, gy = ((cur / G) | 0) % G, gz = (cur / (G * G)) | 0;
                for (let dz = -1; dz <= 1; dz++) {
                    const nz = gz + dz; if (nz < 0 || nz >= G) continue;
                    for (let dy = -1; dy <= 1; dy++) {
                        const ny = gy + dy; if (ny < 0 || ny >= G) continue;
                        for (let dx = -1; dx <= 1; dx++) {
                            if (dx === 0 && dy === 0 && dz === 0) continue;
                            const nx = gx + dx; if (nx < 0 || nx >= G) continue;
                            const ni = (nz * G + ny) * G + nx;
                            if (claimed[ni]) continue;
                            if (density[ni] < dMin) continue;
                            if (density[ni] > density[cur]) continue;
                            claimed[ni] = 1;
                            if (sp >= stack.length) break;
                            stack[sp++] = ni;
                        }
                    }
                }
            }
            out.push({ cells });
        }
        return out;
    }

    _isLocalMax(c, density, G) {
        const gx = c % G, gy = ((c / G) | 0) % G, gz = (c / (G * G)) | 0;
        const d = density[c];
        for (let dz = -1; dz <= 1; dz++) {
            const nz = gz + dz; if (nz < 0 || nz >= G) continue;
            for (let dy = -1; dy <= 1; dy++) {
                const ny = gy + dy; if (ny < 0 || ny >= G) continue;
                for (let dx = -1; dx <= 1; dx++) {
                    if (dx === 0 && dy === 0 && dz === 0) continue;
                    const nx = gx + dx; if (nx < 0 || nx >= G) continue;
                    const ni = (nz * G + ny) * G + nx;
                    if (density[ni] > d) return false;
                }
            }
        }
        return true;
    }

    // 26-neighbour flood fill over the binary `hot` grid.
    _floodFill(hot, G, totalCells) {
        const comp = this._comp, stack = this._stack;
        comp.fill(0, 0, totalCells); // 0 = unvisited; we use comp as a visited flag
        const out = [];
        for (let c = 0; c < totalCells; c++) {
            if (!hot[c] || comp[c]) continue;
            const cells = [];
            let sp = 0; stack[sp++] = c; comp[c] = 1;
            while (sp > 0) {
                const cur = stack[--sp];
                cells.push(cur);
                const gx = cur % G, gy = ((cur / G) | 0) % G, gz = (cur / (G * G)) | 0;
                for (let dz = -1; dz <= 1; dz++) {
                    const nz = gz + dz; if (nz < 0 || nz >= G) continue;
                    for (let dy = -1; dy <= 1; dy++) {
                        const ny = gy + dy; if (ny < 0 || ny >= G) continue;
                        for (let dx = -1; dx <= 1; dx++) {
                            if (dx === 0 && dy === 0 && dz === 0) continue;
                            const nx = gx + dx; if (nx < 0 || nx >= G) continue;
                            const ni = (nz * G + ny) * G + nx;
                            if (hot[ni] && !comp[ni]) {
                                comp[ni] = 1;
                                if (sp >= stack.length) break; // safety (shouldn't happen, stack sized totalCells)
                                stack[sp++] = ni;
                            }
                        }
                    }
                }
            }
            out.push({ cells });
        }
        return out;
    }

    _ensureCells(totalCells) {
        if (this._capCells >= totalCells && this._density) return;
        this._capCells = totalCells;
        this._density = new Float32Array(totalCells);
        this._cross = new Float32Array(totalCells);
        this._hot = new Uint8Array(totalCells);
        this._fx = new Float32Array(totalCells);
        this._fy = new Float32Array(totalCells);
        this._fz = new Float32Array(totalCells);
        this._comp = new Uint8Array(totalCells);
        this._stack = new Int32Array(totalCells);
        this._cellSegHead = new Int32Array(totalCells);
    }

    _ensureSegs(segTotal) {
        if (this._capSegs >= segTotal && this._segEnds) return;
        this._capSegs = segTotal;
        this._segEnds = new Float32Array(segTotal * 6);
        this._segCell = new Int32Array(segTotal);
        this._cellSegNext = new Int32Array(segTotal);
    }
}

function emptyTelemetry() {
    return { count: 0, ids: new Int32Array(0), age: new Int32Array(0), size: new Int32Array(0),
             peak: new Int32Array(0), birth: new Int32Array(0), stride: 8,
             fields: new Float32Array(0), dirs: new Float32Array(0), extents: new Float32Array(0),
             found: 0, dropped: 0 };
}

function emptyContrib() {
    return { count: 0, ids: new Int32Array(0),
             energy: new Float64Array(0), flux: new Float64Array(0), charge: new Float64Array(0),
             energyFrac: new Float64Array(0), fluxFrac: new Float64Array(0), chargeFrac: new Float64Array(0),
             totals: { energy: 0, flux: 0, charge: 0 },
             captured: { energyFrac: 0, fluxFrac: 0, chargeFrac: 0 } };
}

// Squared minimum distance between segments (a=segEnds[s], b=segEnds[t]); returns
// true when they pass within sqrt(cd2) AND are not near-parallel (a crossing,
// not a parallel bunch).
function segsCross(segEnds, s, t, cd2, parallelCos) {
    const so = s * 6, to = t * 6;
    const ax = segEnds[so], ay = segEnds[so + 1], az = segEnds[so + 2];
    const bx = segEnds[so + 3], by = segEnds[so + 4], bz = segEnds[so + 5];
    const cx = segEnds[to], cy = segEnds[to + 1], cz = segEnds[to + 2];
    const dx = segEnds[to + 3], dy = segEnds[to + 4], dz = segEnds[to + 5];
    const ux = bx - ax, uy = by - ay, uz = bz - az;
    const vx = dx - cx, vy = dy - cy, vz = dz - cz;
    const wx = ax - cx, wy = ay - cy, wz = az - cz;
    const a = ux * ux + uy * uy + uz * uz;
    const b = ux * vx + uy * vy + uz * vz;
    const cc = vx * vx + vy * vy + vz * vz;
    const d = ux * wx + uy * wy + uz * wz;
    const e = vx * wx + vy * wy + vz * wz;
    const den = a * cc - b * b;
    let sc, tc;
    if (den < 1e-9) { sc = 0; tc = (b > cc ? d / b : e / cc) || 0; }
    else { sc = (b * e - cc * d) / den; tc = (a * e - b * d) / den; }
    sc = sc < 0 ? 0 : sc > 1 ? 1 : sc;
    tc = tc < 0 ? 0 : tc > 1 ? 1 : tc;
    const px = wx + sc * ux - tc * vx;
    const py = wy + sc * uy - tc * vy;
    const pz = wz + sc * uz - tc * vz;
    const dist2 = px * px + py * py + pz * pz;
    if (dist2 > cd2) return false;
    // parallel rejection: near-parallel segments are bunching, not a crossing
    const la = Math.sqrt(a), lv = Math.sqrt(cc);
    if (la < 1e-9 || lv < 1e-9) return false;
    const cos = Math.abs(b / (la * lv));
    if (cos > parallelCos) return false;
    return true;
}

// Deterministic id → hue in [0,1). Same integer-mix as scale1/pe-cloud-expander's
// hashUint32, so a knot's color is stable across ticks and shared by the panel
// row + the viewport box.
function clamp01(v) { v = +v; return v < 0 ? 0 : v > 1 ? 1 : (Number.isFinite(v) ? v : 0.5); }

// Field-aware: B-field knots are shifted a half-turn from E; flux a quarter-turn
// so the three streamline families stay visually distinct.
export function knotHue(id, field = 'e') {
    let h = (Math.imul(id | 0, 374761393) + 668265263) >>> 0;
    h = (Math.imul(h ^ (h >>> 13), 1274126177)) >>> 0;
    let hue = (h >>> 8) / 0x1000000; // top 24 bits → [0,1)
    if (field === 'b') hue = (hue + 0.5) % 1;
    else if (field === 'flux') hue = (hue + 0.25) % 1;
    return hue;
}

// ── per-field tracker registry ──────────────────────────────────────────
// One independent tracker per streamline family ('e', 'b', 'flux'). Default 'e'
// keeps every existing call working. E and B are the orthogonal EM pair; flux
// is the J-streamline family.
const KNOT_FIELDS = ['e', 'b', 'flux'];
const _trackers = {};
export function getFieldLineKnotTracker(field = 'e') {
    let t = _trackers[field];
    if (!t) { t = _trackers[field] = new FieldLineKnotTracker(); t._field = field; }
    return t;
}
// Apply a shared op (sensitivity / per-knot-color / contrib-enable / reset) to all families.
export function forEachKnotTracker(fn) {
    for (const f of KNOT_FIELDS) fn(getFieldLineKnotTracker(f), f);
}
