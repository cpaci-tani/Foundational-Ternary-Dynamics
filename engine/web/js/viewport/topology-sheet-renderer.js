/**
 * viewport/topology-sheet-renderer.js — deformable rubber-sheet visualization
 *
 * Extracted from viewport.js as refactoring-analyst ticket RF-1 of the
 * post-modularization cleanup (see engine/web/docs/INDEX.md).
 *
 * ## What this owns
 *
 * Eleven "rubber sheet" visualizations stacked vertically across the lattice,
 * each deforming a flat plane by a scalar field sampled on the lattice:
 *
 *   Φ gravitational potential (`gravPotential`) — special case rendered at
 *      y = halfN, depth 0.25·N, signed (wells dip, peaks rise)
 *   emEnergy         — EM energy density
 *   ePressure        — |E|² / 2 (electric pressure)
 *   bPressure        — |B|² / 2 (magnetic pressure)
 *   chargeDensity    — signed voxel state
 *   vorticity        — |∇×J|² (thin band)
 *
 * Each sheet is a 40×40 PlaneGeometry deformed by the scatter-heights
 * pipeline (bilinear splat → 2-pass box-blur → per-vertex bilinear lookup).
 * Colors come from per-sheet ramp functions in viewport/color-ramps.js.
 *
 * ## Public API
 *
 *   new TopologySheetRenderer({ scene, getLatticeSize, getHalfN, onVisibilityChange })
 *
 *   .toggleGravPotential(on)
 *   .updateGravPotential(data)
 *   .toggle(key, on)                 — any topology sheet by key
 *   .update(key, data)
 *   .dispose()
 *
 * ## Contract with viewport
 *
 * `onVisibilityChange` is invoked whenever a toggle changes visibility. The
 * viewport uses it to coordinate with the quantum renderer's visibility
 * (`_quantumSetVisibility`). If null, coordination is skipped.
 *
 * The `getLatticeSize`/`getHalfN` getters are live — the renderer re-queries
 * them on every build/update so lattice-size changes propagate without
 * needing a reconfigure call (matches the existing `_rebuild…IfResized`
 * pattern that was inline in viewport.js).
 */

import * as THREE from 'three';
import {
    rampGravWell,
    rampEmEnergy, rampCharge, rampVorticity,
    rampEPressure, rampBPressure,
} from './color-ramps.js';

/**
 * Per-sheet layout + color configuration. 10 sheets stacked across y∈[0.05, 0.97]
 * with per-sheet depth chosen so |yFrac·N ± depth·N| doesn't cross an adjacent
 * sheet's band. Previous layout (0.62/0.72/0.78 inside 0.12N-deep bands) caused
 * visible intersection at N≥64; this layout guarantees ≥(depth_i + depth_{i+1})
 * separation between any two adjacent sheets.
 */
const TOPOLOGY_CONFIGS = Object.freeze({
    emEnergy:      { yFrac: 0.05, depthFrac: 0.08, signed: false, ramp: rampEmEnergy },
    ePressure:     { yFrac: 0.35, depthFrac: 0.08, signed: false, ramp: rampEPressure },
    bPressure:     { yFrac: 0.45, depthFrac: 0.08, signed: false, ramp: rampBPressure },
    chargeDensity: { yFrac: 0.87, depthFrac: 0.08, signed: true,  ramp: rampCharge },
    vorticity:     { yFrac: 0.97, depthFrac: 0.03, signed: false, ramp: rampVorticity },
});

// Default slice height (fraction of the box) for each slideable rubber sheet —
// where it floats + samples until the user moves its height slider. A compact
// band around the mid-plane (not the box edges) so a freshly-toggled sheet reads
// a meaningful slice, while still separating the sheets when several are on.
// Mirror these on the slider `value=` attrs in scale0/ui/overlays/template.js.
const DEFAULT_SHEET_HEIGHTS = Object.freeze({
    gravPotential: 0.50,
    chargeDensity: 0.62,
    emEnergy:      0.56,
    vorticity:     0.68,
    ePressure:     0.44,
    bPressure:     0.38,
});

export class TopologySheetRenderer {
    constructor({ scene, getLatticeSize, getHalfN, onVisibilityChange = null }) {
        this.scene = scene;
        this._getLatticeSize = getLatticeSize;
        this._getHalfN = getHalfN;
        this._onVisibilityChange = onVisibilityChange;

        // Φ gravitational potential (separate from the 10 topology sheets).
        this._gravSurface = null;
        this._gravSurfaceWire = null;
        this._gravSurfaceSize = 0;
        this._gravPotVisible = false;
        this._gravPotDrawable = false;
        this._gravPotData = null;

        // Topology sheets { key: { solid, wire, size, lastData } }
        this._topoSheets = {};

        // Per-sheet slice HEIGHT (fraction of N, 0..0.999). The sheet floats at
        // y = frac·N AND the field is re-sampled in a thin y-slab at that plane,
        // so sliding the value sweeps the slice through the volume (mirrors the
        // native SetSheetHeight mechanism). Undefined ⇒ the config default.
        this._sheetHeights = {};

        // Scratch buffers reused across _scatterHeights calls.
        this._scatterBufs = null;
        this._rampScratch = null;
    }

    // Default slice height (fraction of N) for a sheet — where it floats + slices
    // before the user moves the slider. A compact band around the mid-plane so a
    // freshly-toggled sheet shows a meaningful slice (the old TOPOLOGY_CONFIGS
    // yFracs sat at the box edges — correct for the whole-column average, but an
    // empty slice), while still separating the sheets when several are on.
    // Keep these in sync with the `value=` attrs on the sliders in
    // scale0/ui/overlays/template.js.
    _defaultHeightFrac(key) {
        return DEFAULT_SHEET_HEIGHTS[key] ?? 0.5;
    }

    _heightFrac(key) {
        const h = this._sheetHeights[key];
        return (typeof h === 'number') ? h : this._defaultHeightFrac(key);
    }

    // y-slab half-width for the slice, in voxels. Matches the overlay sample
    // stride (max(2, min(8, round(N/16)))) so the slab captures the nearest
    // sampled y-level(s) — the field is only sampled every `stride` voxels in y.
    _slabHalf(N) {
        return Math.max(2, Math.min(8, Math.round(N / 16)));
    }

    /**
     * Set the slice height of a sheet and immediately re-slice + reposition it.
     * @param {string} key — a TOPOLOGY_CONFIGS key or 'gravPotential'
     * @param {number} frac — height as a fraction of the lattice box, 0..0.999
     */
    setHeight(key, frac) {
        const f = Math.max(0, Math.min(0.999, Number(frac) || 0));
        if (this._heightFrac(key) === f) return;
        this._sheetHeights[key] = f;
        if (key === 'gravPotential') {
            const N = this._getLatticeSize();
            if (this._gravSurface) {
                this._gravSurface.position.y = f * N;
                this._gravSurfaceWire.position.y = f * N + 0.02;
            }
            if (this._gravPotData) this.updateGravPotential(this._gravPotData);
            return;
        }
        const s = this._topoSheets[key];
        if (s) {
            const N = this._getLatticeSize();
            s.solid.position.y = f * N;
            s.wire.position.y = f * N + 0.02;
            if (s.lastData) this.update(key, s.lastData);
        }
    }

    onLatticeSizeChanged(size, halfN) {
        if (this._gravSurface) {
            this._rebuildGravSurfaceIfResized();
            if (this._gravPotData) {
                this.updateGravPotential(this._gravPotData);
            }
        }
        for (const key of Object.keys(this._topoSheets)) {
            this._rebuildSheetIfResized(key);
        }
    }

    // ── Gravitational potential Φ (special-case rubber sheet) ──────────

    _buildGravSurface() {
        const N = this._getLatticeSize();
        this._gravSurfaceSize = N;
        // The visual field sampler exposes at most ~33 support levels per
        // dimension at large browser lattices. More surface subdivisions add
        // interpolated wire fragments, not discrete information.
        const segments = Math.max(24, Math.min(N, 32));
        const geo = new THREE.PlaneGeometry(N * 0.95, N * 0.95, segments, segments);
        geo.rotateX(-Math.PI / 2);
        const colors = new Float32Array(geo.attributes.position.count * 3);
        geo.setAttribute('color', new THREE.Float32BufferAttribute(colors, 3));
        const mat = new THREE.MeshBasicMaterial({
            vertexColors: true, transparent: true, opacity: 0.55,
            side: THREE.DoubleSide, wireframe: false, depthWrite: false,
        });
        const wireMat = new THREE.MeshBasicMaterial({
            vertexColors: true, transparent: true, opacity: 0.35,
            wireframe: true, depthWrite: false,
        });
        const gy = N * this._heightFrac('gravPotential');
        this._gravSurface = new THREE.Mesh(geo, mat);
        this._gravSurface.position.set(N / 2, gy, N / 2);
        this._gravSurface.visible = this._gravPotVisible && this._gravPotDrawable;
        this._gravSurface.renderOrder = 3;
        // Deformable Y-vertices; keep frustum culling off so the camera can
        // dip inside a deep well without the mesh disappearing.
        this._gravSurface.frustumCulled = false;
        this._gravSurfaceWire = new THREE.Mesh(geo, wireMat);
        this._gravSurfaceWire.position.set(N / 2, gy + 0.02, N / 2);
        this._gravSurfaceWire.visible = this._gravPotVisible && this._gravPotDrawable;
        this._gravSurfaceWire.renderOrder = 3;
        this._gravSurfaceWire.frustumCulled = false;
        this.scene.add(this._gravSurface);
        this.scene.add(this._gravSurfaceWire);
    }

    _rebuildGravSurfaceIfResized() {
        if (!this._gravSurface) return;
        const N = this._getLatticeSize();
        if (this._gravSurfaceSize === N) return;
        this._gravSurface.geometry?.dispose();
        this._gravSurface.material?.dispose();
        this._gravSurfaceWire.geometry?.dispose();
        this._gravSurfaceWire.material?.dispose();
        this.scene.remove(this._gravSurface);
        this.scene.remove(this._gravSurfaceWire);
        this._gravSurface = null;
        this._gravSurfaceWire = null;
        this._buildGravSurface();
        this._gravSurface.visible = this._gravPotVisible && this._gravPotDrawable;
        this._gravSurfaceWire.visible = this._gravPotVisible && this._gravPotDrawable;
    }

    toggleGravPotential(on) {
        const next = !!on;
        if (this._gravPotVisible === next && (this._gravSurface || !next)) return;
        this._gravPotVisible = next;
        if (!this._gravSurface) { if (!next) return; this._buildGravSurface(); }
        const drawable = next && this._gravPotDrawable;
        this._gravSurface.visible = drawable;
        this._gravSurfaceWire.visible = drawable;
        if (drawable && this._gravPotData) this.updateGravPotential(this._gravPotData);
        if (this._onVisibilityChange) this._onVisibilityChange();
    }

    updateGravPotential(data) {
        if (!data?.count || !(Number(data.normalizer) > 0)) {
            this._gravPotData = null;
            this._gravPotDrawable = false;
            if (this._gravSurface) this._gravSurface.visible = false;
            if (this._gravSurfaceWire) this._gravSurfaceWire.visible = false;
            return;
        }
        this._gravPotData = data;
        this._gravPotDrawable = true;
        if (!this._gravPotVisible) return;
        if (!this._gravSurface) this._buildGravSurface();
        this._rebuildGravSurfaceIfResized();
        this._gravSurface.visible = true;
        this._gravSurfaceWire.visible = true;
        const geo = this._gravSurface.geometry;
        const pos = geo.attributes.position;
        const col = geo.attributes.color;
        const verts = pos.count;
        const N = this._getLatticeSize();
        const halfN = this._getHalfN();
        const DEPTH = N * 0.25;
        const sliceY = this._heightFrac('gravPotential') * N;
        this._gravSurface.position.y = sliceY;
        this._gravSurfaceWire.position.y = sliceY + 0.02;

        const heights = this._scatterHeights(pos, halfN, N, data, sliceY);

        if (!this._rampScratch) this._rampScratch = new Float32Array(3);
        const rgb = this._rampScratch;
        for (let v = 0; v < verts; v++) {
            // Φ is negative for wells; scale to DEPTH so negative t dips
            // below the reference plane (local-y = 0, world y = halfN).
            const t = heights[v];
            pos.array[v * 3 + 1] = t * DEPTH;
            rampGravWell(Math.min(1, Math.abs(t)), rgb, 0);
            col.array[v * 3]     = rgb[0];
            col.array[v * 3 + 1] = rgb[1];
            col.array[v * 3 + 2] = rgb[2];
        }
        pos.needsUpdate = true;
        col.needsUpdate = true;
        geo.computeVertexNormals();
    }

    // ── Topology sheets (generic key-based) ─────────────────────────────

    _buildSheet(key) {
        const cfg = TOPOLOGY_CONFIGS[key];
        if (!cfg) throw new Error(`TopologySheetRenderer: unknown key "${key}"`);
        const N = this._getLatticeSize();
        // 40×40 quads (≈1600 vertices) is enough — smoothness comes from the
        // grid-blur pipeline in _scatterHeights, not from vertex count.
        const segments = Math.max(24, Math.min(N, 40));
        const geo = new THREE.PlaneGeometry(N * 0.95, N * 0.95, segments, segments);
        geo.rotateX(-Math.PI / 2);
        const colors = new Float32Array(geo.attributes.position.count * 3);
        geo.setAttribute('color', new THREE.Float32BufferAttribute(colors, 3));
        const mat = new THREE.MeshBasicMaterial({
            vertexColors: true, transparent: true, opacity: 0.45,
            side: THREE.DoubleSide, depthWrite: false,
        });
        // Coarse wireframe at half the solid resolution (readable scaffold).
        const wireSeg = Math.max(8, Math.floor(segments / 2));
        const wireGeo = new THREE.PlaneGeometry(N * 0.95, N * 0.95, wireSeg, wireSeg);
        wireGeo.rotateX(-Math.PI / 2);
        const wireMat = new THREE.MeshBasicMaterial({
            color: 0xffffff, transparent: true, opacity: 0.16,
            wireframe: true, depthWrite: false,
        });
        const yWorld = N * this._heightFrac(key);
        const solid = new THREE.Mesh(geo, mat);
        solid.position.set(N / 2, yWorld, N / 2);
        solid.visible = false;
        solid.renderOrder = 3;
        solid.frustumCulled = false;
        const wire = new THREE.Mesh(wireGeo, wireMat);
        wire.position.set(N / 2, yWorld + 0.02, N / 2);
        wire.visible = false;
        wire.renderOrder = 3;
        wire.frustumCulled = false;
        this.scene.add(solid);
        this.scene.add(wire);
        this._topoSheets[key] = { solid, wire, size: N };
    }

    _rebuildSheetIfResized(key) {
        const s = this._topoSheets[key];
        if (!s) return;
        const N = this._getLatticeSize();
        if (s.size === N) return;
        const vis = s.solid.visible;
        const lastData = s.lastData;   // carry across the rebuild so setHeight() can still re-slice
        // Pre-2026-04-26 this disposed only geometries — `_buildSheet` allocates
        // both new geometries AND new materials, so the old materials leaked
        // 2 per sheet × 10 sheets every lattice resize. Dispose materials too
        // before dropping the sheet entry from the registry.
        s.solid.geometry?.dispose();
        s.solid.material?.dispose();
        s.wire.geometry?.dispose();
        s.wire.material?.dispose();
        this.scene.remove(s.solid);
        this.scene.remove(s.wire);
        delete this._topoSheets[key];
        this._buildSheet(key);
        const ns = this._topoSheets[key];
        ns.solid.visible = vis;
        ns.wire.visible  = vis;
        // Re-slice at the new size from the retained data (so a height slider
        // dragged after a paused resize still updates instead of showing flat).
        if (lastData) this.update(key, lastData);
    }

    /**
     * Toggle visibility of a named topology sheet.
     * @param {string} key — one of the keys in TOPOLOGY_CONFIGS
     * @param {boolean} on
     */
    toggle(key, on) {
        const next = !!on;
        if (!this._topoSheets[key]) { if (!next) return; this._buildSheet(key); }
        const s = this._topoSheets[key];
        if (s.solid.visible === next && s.wire.visible === next) return;
        s.solid.visible = next;
        s.wire.visible  = next;
    }

    /**
     * Update a named topology sheet with new sampled data.
     * @param {string} key
     * @param {{positions: Float32Array, values: Float32Array, count: number, normalizer: number}} data
     */
    update(key, data) {
        if (!data?.count) return;
        if (!this._topoSheets[key]) this._buildSheet(key);
        this._rebuildSheetIfResized(key);
        const s = this._topoSheets[key];
        s.lastData = data;   // kept so setHeight() can re-slice without new engine data
        if (!s.solid.visible) return;
        const cfg = TOPOLOGY_CONFIGS[key];
        const geo = s.solid.geometry;
        const pos = geo.attributes.position;
        const col = geo.attributes.color;
        const verts = pos.count;
        const N = this._getLatticeSize();
        const halfN = this._getHalfN();
        const DEPTH = N * cfg.depthFrac;
        const sliceY = this._heightFrac(key) * N;
        // Park the plane at its slice height (a resize rebuild can reset position).
        s.solid.position.y = sliceY;
        s.wire.position.y = sliceY + 0.02;

        const heights = this._scatterHeights(pos, halfN, N, data, sliceY);

        // Reuse scratch RGB triple across all sheets × all frames.
        if (!this._rampScratch) this._rampScratch = new Float32Array(3);
        const rgb = this._rampScratch;
        for (let v = 0; v < verts; v++) {
            const t = heights[v];
            if (cfg.signed) {
                const ts = Math.max(-1, Math.min(1, t));
                pos.array[v * 3 + 1] = ts * DEPTH;
                cfg.ramp(ts, rgb, 0);
            } else {
                const tt = Math.max(0, Math.min(1, t));
                pos.array[v * 3 + 1] = tt * DEPTH;
                cfg.ramp(tt, rgb, 0);
            }
            col.array[v * 3]     = rgb[0];
            col.array[v * 3 + 1] = rgb[1];
            col.array[v * 3 + 2] = rgb[2];
        }
        pos.needsUpdate = true;
        col.needsUpdate = true;
        geo.computeVertexNormals();

        // Deform the coarse wireframe to match — same sampler.
        if (s.wire && s.wire.geometry) {
            const wirePos = s.wire.geometry.attributes.position;
            const wireHeights = this._scatterHeights(wirePos, halfN, N, data, sliceY);
            for (let v = 0; v < wirePos.count; v++) {
                const t = wireHeights[v];
                const tc = cfg.signed
                    ? Math.max(-1, Math.min(1, t))
                    : Math.max(0, Math.min(1, t));
                wirePos.array[v * 3 + 1] = tc * DEPTH;
            }
            wirePos.needsUpdate = true;
        }
    }

    // ── Shared scatter → grid → blur → bilinear lookup pipeline ─────────
    //
    // The naive O(verts × samples) per-vertex Gaussian loop scaled as L⁴
    // and dragged FPS at L ≥ 64 with several sheets on. Replaced with:
    //   1. Rasterize samples into a small 2D heightfield grid (bilinear splat).
    //   2. Separable 3-tap box-blur the grid (2 passes → Gaussian-like kernel).
    //   3. For each mesh vertex, 4-tap bilinear lookup into the blurred grid.
    // O(verts × 4), no transcendentals. At L=64 this drops ~100M exp() calls/sec
    // down to ~10M simple FMAs — well under 1 ms per sheet even with 4 sheets on.

    _scatterHeights(geoPos, halfN, N, data, sliceY = null) {
        const verts = geoPos.count;
        const { positions, values, count, normalizer } = data;
        const denom = Math.max(normalizer || 0, 1e-9);

        // Cached grid buffers (reused across ticks to avoid GC).
        // gridN chosen so cells are ~1 voxel wide; box-blur smooths them.
        const gridN = Math.max(16, Math.min(N, 48));
        if (!this._scatterBufs || this._scatterBufs.gridN !== gridN) {
            this._scatterBufs = {
                gridN,
                grid:   new Float32Array(gridN * gridN),
                weight: new Float32Array(gridN * gridN),
                tmp:    new Float32Array(gridN * gridN),
            };
        }
        const { grid, weight, tmp } = this._scatterBufs;
        const b = this._scatterBufs;
        const scale = (gridN - 1) / N;

        // Build-cache: skip the O(count) rasterize + O(gridN²) blur when the grid
        // was JUST built for the identical (data, sliceY, N) — i.e. the solid then
        // wire mesh of one sheet in a single update() pass. `data` is a fresh
        // object each frame, so this reuse never spans frames or sheets, only the
        // solid→wire call pair (which previously rebuilt the grid twice).
        const gridReady = (b._bd === data && b._bs === sliceY && b._bn === N);
        if (!gridReady) {
        grid.fill(0);
        weight.fill(0);

        // 1. Rasterize samples → grid via bilinear splat (O(count)). When a
        //    sliceY (world-y) is given, gate each sample to a thin y-slab around
        //    that plane and weight it by a triangular y-kernel — so the grid
        //    holds the field AT that height, not the whole-column average. This
        //    is what makes the sheet a slideable slice (mirrors the native
        //    build_sheet slab filter). sliceY == null keeps the legacy projection.
        const slabHalf = (sliceY != null) ? this._slabHalf(N) : 0;
        for (let i = 0; i < count; i++) {
            let yw = 1;
            if (sliceY != null) {
                const dy = Math.abs(positions[i * 3 + 1] - sliceY);
                if (dy >= slabHalf) continue;
                yw = 1 - dy / slabHalf;   // triangular kernel, peak on the plane
            }
            const sx = positions[i * 3]     * scale;
            const sz = positions[i * 3 + 2] * scale;
            if (sx < 0 || sx >= gridN - 1 || sz < 0 || sz >= gridN - 1) continue;
            const xi = sx | 0, zi = sz | 0;
            const xf = sx - xi, zf = sz - zi;
            const v = values[i];
            const w00 = (1 - xf) * (1 - zf) * yw;
            const w01 = xf * (1 - zf) * yw;
            const w10 = (1 - xf) * zf * yw;
            const w11 = xf * zf * yw;
            const row0 = zi * gridN + xi;
            const row1 = row0 + gridN;
            grid[row0]     += v * w00;  weight[row0]     += w00;
            grid[row0 + 1] += v * w01;  weight[row0 + 1] += w01;
            grid[row1]     += v * w10;  weight[row1]     += w10;
            grid[row1 + 1] += v * w11;  weight[row1 + 1] += w11;
        }

        // 2. Normalise by accumulated weight; unsampled cells stay 0 and
        //    get filled by the blur pass below.
        const G2 = gridN * gridN;
        for (let i = 0; i < G2; i++) {
            if (weight[i] > 1e-9) grid[i] /= weight[i];
        }

        // 3. Separable 3-tap box-blur (2 passes). Interior-only; edges
        //    keep their unblurred value which naturally pins boundaries.
        const blurPasses = 2;
        for (let p = 0; p < blurPasses; p++) {
            for (let z = 0; z < gridN; z++) {
                const rowBase = z * gridN;
                tmp[rowBase] = grid[rowBase];
                tmp[rowBase + gridN - 1] = grid[rowBase + gridN - 1];
                for (let x = 1; x < gridN - 1; x++) {
                    tmp[rowBase + x] =
                        (grid[rowBase + x - 1]
                       + grid[rowBase + x]
                       + grid[rowBase + x + 1]) * (1 / 3);
                }
            }
            for (let x = 0; x < gridN; x++) {
                grid[x] = tmp[x];
                grid[(gridN - 1) * gridN + x] = tmp[(gridN - 1) * gridN + x];
            }
            for (let z = 1; z < gridN - 1; z++) {
                const rowPrev = (z - 1) * gridN;
                const rowCurr = z * gridN;
                const rowNext = (z + 1) * gridN;
                for (let x = 0; x < gridN; x++) {
                    grid[rowCurr + x] =
                        (tmp[rowPrev + x]
                       + tmp[rowCurr + x]
                       + tmp[rowNext + x]) * (1 / 3);
                }
            }
        }
        b._bd = data; b._bs = sliceY; b._bn = N;   // grid now valid for this (data, sliceY, N)
        }  // end build-cache guard

        // 4. Per-vertex bilinear lookup into the blurred grid. Grow-only
        //    heights scratch avoids per-call allocation (~2 MB/s at 20Hz
        //    with 10 sheets).
        if (!this._scatterBufs.heights || this._scatterBufs.heights.length < verts) {
            this._scatterBufs.heights = new Float32Array(verts);
        }
        const heights = this._scatterBufs.heights;
        const gridMax = gridN - 1;
        const invDenom = 1 / denom;
        for (let v = 0; v < verts; v++) {
            const wx = geoPos.array[v * 3]     + halfN;
            const wz = geoPos.array[v * 3 + 2] + halfN;
            let gx = wx * scale;
            let gz = wz * scale;
            if (gx < 0) gx = 0; else if (gx > gridMax) gx = gridMax;
            if (gz < 0) gz = 0; else if (gz > gridMax) gz = gridMax;
            const xi = gx | 0, zi = gz | 0;
            const xf = gx - xi, zf = gz - zi;
            const xi1 = xi < gridMax ? xi + 1 : xi;
            const zi1 = zi < gridMax ? zi + 1 : zi;
            const v00 = grid[zi  * gridN + xi];
            const v01 = grid[zi  * gridN + xi1];
            const v10 = grid[zi1 * gridN + xi];
            const v11 = grid[zi1 * gridN + xi1];
            const blended =
                (1 - xf) * (1 - zf) * v00
              +      xf  * (1 - zf) * v01
              + (1 - xf) *      zf  * v10
              +      xf  *      zf  * v11;
            heights[v] = blended * invDenom;
        }
        return heights;
    }

    // ── Cleanup ─────────────────────────────────────────────────────────

    dispose() {
        if (this._gravSurface) {
            this._gravSurface.geometry?.dispose();
            this._gravSurface.material?.dispose();
            this.scene.remove(this._gravSurface);
            this._gravSurface = null;
        }
        if (this._gravSurfaceWire) {
            this._gravSurfaceWire.geometry?.dispose();
            this._gravSurfaceWire.material?.dispose();
            this.scene.remove(this._gravSurfaceWire);
            this._gravSurfaceWire = null;
        }
        for (const key of Object.keys(this._topoSheets)) {
            const s = this._topoSheets[key];
            if (s.solid) {
                s.solid.geometry?.dispose();
                s.solid.material?.dispose();
                this.scene.remove(s.solid);
            }
            if (s.wire) {
                s.wire.geometry?.dispose();
                s.wire.material?.dispose();
                this.scene.remove(s.wire);
            }
        }
        this._topoSheets = {};
        this._scatterBufs = null;
    }

    destroy(ctx) {
        this.dispose();
    }
}
