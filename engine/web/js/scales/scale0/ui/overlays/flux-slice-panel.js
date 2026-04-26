/**
 * Scale 0 — Live Multi-Field Flux Slice Panel
 *
 * Mirrors the visualization panel's enabled fields (FIELDS column +
 * |J|), rendering each enabled field as a row of three 2D heatmaps at
 * the lattice mid-planes (xy @ z=L/2, xz @ y=L/2, yz @ x=L/2). The
 * panel's per-row mirror chip lets the user manually override the
 * visualization-panel state per-field — handy for inspecting a field
 * without lighting up the 3D viewport's volumetric overlay.
 *
 * Supported fields (FIELD_DRIVERS):
 *   |J|     bridge.getFluxSlice(axis, mid)        scalar density
 *   |E|     bridge.getEFieldSampled(1)            vector → magnitude
 *   |B|     bridge.getBFieldSampled(1)            vector → magnitude
 *   |S|     bridge.getPoyntingSampled(1)          vector → magnitude
 *   ∇·J    bridge.getDivJSampled(1)              signed scalar
 *
 * Mirror semantics:
 *   - Each row's visibility = (override === 'on') ? true
 *                           : (override === 'off') ? false
 *                           : !!mirroredFlags[vizFlagKey]
 *   - "Reset mirror" header link clears all overrides.
 *   - Scenario change (simTick decreasing) auto-clears all overrides
 *     and resets per-field rolling autoscale.
 *
 * Performance: 5 fields × 3 axes = 15 tiles. At L=32 each frame samples
 * up to 4 stride=1 sparse fields plus the |J| volume scan, then paints
 * 15 × 1024 cells × 4 RGBA bytes. Throttled to every Nth render frame
 * via `updateEvery` (default 2). Hidden rows skip sampling AND paint.
 */

import {
    rampViridis,
    rampEmEnergy,
    rampVorticity,
    rampCharge,
} from '../../../../viewport/color-ramps.js';
import { getFieldStateSnapshot } from '../../state/store.js';

const DEFAULT_CANVAS_PX = 220;
const DENSE_CANVAS_PX = 160; // shrink when >2 active rows are visible
const FLOOR_FRAC = 1e-6;
const DENSE_THRESHOLD = 2;

// ── Per-field driver registry ───────────────────────────────────────
//
// One entry per supported field. The slice panel iterates this list,
// renders one row per active driver, and dispatches to the driver's
// sample/ramp pair to paint each tile.
//
// `vizFlagKey` matches the keys in store.js::fieldFlags
// (FIELD_TOGGLE_KEYS, lines 3–42 of state/store.js).
//
// `sample(bridge, axis, mid, N)` MUST return a Float64Array(N*N)
// scoped to a single plane. Implementations live below.
//
// `signed` controls the autoscale + ramp input: false → t = v / vmax in
// [0, 1]; true → t = clamp(v / vmax, -1, +1) for diverging ramps.
//
// `ramp` is one of the named ramps from viewport/color-ramps.js.
const FIELD_DRIVERS = [
    {
        key: 'fluxJ',
        label: '|J|',
        vizFlagKey: 'showFluxLines',
        signed: false,
        ramp: rampViridis,
        // Per-frame source: returns whatever the driver needs to slice;
        // for fluxJ the source is unused because getFluxSlice slices directly.
        source: (/* bridge */) => null,
        sample: (bridge, axis, mid /*, N, source */) => {
            // getFluxSlice reuses an internal _sliceBuf. .slice() snapshots
            // before the next call clobbers it.
            const s = bridge.getFluxSlice?.(axis, mid);
            return s ? s.slice() : null;
        },
    },
    {
        key: 'eField',
        label: '|E|',
        vizFlagKey: 'showEField',
        signed: false,
        ramp: rampEmEnergy,
        // Pull the sparse sample ONCE per frame (cached in `source`); the
        // sample() callback then filters it per-axis at zero extra bridge
        // cost. Saves ~2× per (field, axis) pair vs calling per-axis.
        source: (bridge) => bridge.getEFieldSampled?.(1),
        sample: (bridge, axis, mid, N, source) =>
            sliceVectorMag(source, axis, mid, N),
    },
    {
        key: 'bField',
        label: '|B|',
        vizFlagKey: 'showBField',
        signed: false,
        ramp: rampVorticity,
        source: (bridge) => bridge.getBFieldSampled?.(1),
        sample: (bridge, axis, mid, N, source) =>
            sliceVectorMag(source, axis, mid, N),
    },
    {
        key: 'poynting',
        label: '|S|',
        vizFlagKey: 'showPoynting',
        signed: false,
        ramp: rampEmEnergy,
        source: (bridge) => bridge.getPoyntingSampled?.(1),
        sample: (bridge, axis, mid, N, source) =>
            sliceVectorMag(source, axis, mid, N),
    },
    {
        key: 'divJ',
        label: '∇·J',
        vizFlagKey: 'showDivField',
        signed: true,
        ramp: rampCharge,
        source: (bridge) => bridge.getDivJSampled?.(1),
        sample: (bridge, axis, mid, N, source) =>
            sliceScalarSigned(source, axis, mid, N),
    },
];

const DRIVER_BY_KEY = Object.fromEntries(FIELD_DRIVERS.map(d => [d.key, d]));

// ── File-local helpers ───────────────────────────────────────────────

/**
 * Rasterize a sparse sampled vector field (positions = voxel centers,
 * vectors = 3-tuples per sample) onto an N×N grid covering the chosen
 * mid-plane. Cells without a matching sample stay zero.
 *
 * @param {{positions:Float32Array, vectors:Float32Array, count:number}|null|undefined} sample
 * @param {0|1|2} axis  0 → x=mid (yz plane); 1 → y=mid (xz); 2 → z=mid (xy)
 * @param {number} mid  integer voxel index of the slice plane
 * @param {number} N    lattice size
 * @returns {Float64Array}  N*N scalar magnitudes
 */
function sliceVectorMag(sample, axis, mid, N) {
    const out = new Float64Array(N * N);
    if (!sample || !sample.count) return out;
    const pos = sample.positions;
    const vec = sample.vectors;
    if (!pos || !vec) return out;
    for (let s = 0, p = 0, v = 0; s < sample.count; s++, p += 3, v += 3) {
        // Voxel centers come in as (x + 0.5, y + 0.5, z + 0.5).
        // Floor the center to recover the integer voxel index.
        const ix = (pos[p]     - 0.5) | 0;
        const iy = (pos[p + 1] - 0.5) | 0;
        const iz = (pos[p + 2] - 0.5) | 0;
        let a, b;
        if (axis === 0) {
            if (ix !== mid) continue;
            a = iy; b = iz;
        } else if (axis === 1) {
            if (iy !== mid) continue;
            a = ix; b = iz;
        } else {
            if (iz !== mid) continue;
            a = ix; b = iy;
        }
        if (a < 0 || a >= N || b < 0 || b >= N) continue;
        const m = Math.hypot(vec[v], vec[v + 1], vec[v + 2]);
        out[a * N + b] = m;
    }
    return out;
}

/**
 * Same as sliceVectorMag but for sparse scalar samples. Preserves sign
 * (no abs/hypot) so signed fields like ∇·J light up with diverging ramps.
 *
 * @param {{positions:Float32Array, values:Float32Array, count:number}|null|undefined} sample
 */
function sliceScalarSigned(sample, axis, mid, N) {
    const out = new Float64Array(N * N);
    if (!sample || !sample.count) return out;
    const pos = sample.positions;
    const val = sample.values;
    if (!pos || !val) return out;
    for (let s = 0, p = 0; s < sample.count; s++, p += 3) {
        const ix = (pos[p]     - 0.5) | 0;
        const iy = (pos[p + 1] - 0.5) | 0;
        const iz = (pos[p + 2] - 0.5) | 0;
        let a, b;
        if (axis === 0) {
            if (ix !== mid) continue;
            a = iy; b = iz;
        } else if (axis === 1) {
            if (iy !== mid) continue;
            a = ix; b = iz;
        } else {
            if (iz !== mid) continue;
            a = ix; b = iy;
        }
        if (a < 0 || a >= N || b < 0 || b >= N) continue;
        out[a * N + b] = val[s];
    }
    return out;
}

// ── FluxSlicePanel class ────────────────────────────────────────────

export class FluxSlicePanel {
    /**
     * @param {object} opts
     * @param {() => any} opts.getBridge   - returns the live bridge
     * @param {number} [opts.canvasPx]     - per-canvas size in CSS pixels (default 220)
     * @param {number} [opts.updateEvery]  - sample/render every Nth frame (default 2)
     */
    constructor({ getBridge, canvasPx = DEFAULT_CANVAS_PX, updateEvery = 2 } = {}) {
        this.getBridge = getBridge;
        this.canvasPx = canvasPx | 0;
        this.updateEvery = Math.max(1, updateEvery | 0);

        this.visible = false;
        this.frameCount = 0;
        this._lastN = 0;
        this._lastSimTick = 0;
        // Self-driven rAF loop: kept running while the panel is visible so
        // the heatmaps refresh whether or not the Scale 0 controller's animate
        // tail call fires. The controller-driven update() is still wired up
        // (controller.js:322) and remains the primary path; this loop is the
        // safety net for stale-cache / mount-order races where the external
        // hook isn't reliably calling us.
        this._rafId = null;
        this._lastSelfTickMs = 0;

        this._panel = null;
        this._chip = null;
        this._rowsContainer = null;
        this._resetMirrorBtn = null;

        // Per-axis (xy/xz/yz) visibility — applies globally across all rows.
        this._axisVisible = { xy: true, xz: true, yz: true };

        // Per-field override map. null = follow viz panel; 'on' / 'off' force.
        // |J| defaults to 'on' so opening the panel always shows |J| even
        // when no viz toggles are enabled (preserves prior UX).
        this._fieldOverride = {
            fluxJ: 'on', eField: null, bField: null, poynting: null, divJ: null,
        };

        // Snapshot of viz panel flags, refreshed once per frame.
        this._mirroredFlags = {};

        // Per-field rolling autoscale max — each field gets its own decay
        // because magnitudes differ by orders.
        this._fieldGlobalMax = {
            fluxJ: 0, eField: 0, bField: 0, poynting: 0, divJ: 0,
        };
        this._maxDecay = 0.985;

        // Per-field × per-axis slot bookkeeping. Filled in init().
        // Shape: { [fieldKey]: { row, label, chip, slots: { xy, xz, yz } } }
        this._fields = {};
    }

    static get AXES() { return ['xy', 'xz', 'yz']; }

    // ── Mounting ──────────────────────────────────────────────────────

    init(parentEl) {
        if (!parentEl || this._panel) return this._panel;

        const panel = document.createElement('div');
        panel.id = 'flux-slice-panel';
        panel.className = 'scale0-only flux-slice-panel';
        panel.style.display = 'none';

        // Validate ramp registry — fall back to viridis with a warning if
        // anything went wrong with imports.
        for (const drv of FIELD_DRIVERS) {
            if (typeof drv.ramp !== 'function') {
                console.warn(`[flux-slice-panel] ramp missing for ${drv.key}; falling back to viridis`);
                drv.ramp = rampViridis;
            }
        }

        panel.innerHTML = `
            <div class="flux-slice-header">
                <span class="flux-slice-title">Flux Slices</span>
                <div class="flux-slice-axis-toggles" role="group" aria-label="Axis toggles">
                    <button type="button" class="flux-slice-axis-btn active"
                            data-axis="xy" aria-pressed="true"
                            title="Toggle xy plane (z=L/2)">xy</button>
                    <button type="button" class="flux-slice-axis-btn active"
                            data-axis="xz" aria-pressed="true"
                            title="Toggle xz plane (y=L/2)">xz</button>
                    <button type="button" class="flux-slice-axis-btn active"
                            data-axis="yz" aria-pressed="true"
                            title="Toggle yz plane (x=L/2)">yz</button>
                </div>
                <button type="button" class="flux-slice-reset-mirror"
                        title="Clear all per-field overrides; revert to mirroring the visualization panel"
                        disabled>Reset mirror</button>
                <button type="button" class="flux-slice-close"
                        aria-label="Hide flux slices">×</button>
            </div>
            <div class="flux-slice-rows">
                ${FIELD_DRIVERS.map(d => this._rowHTML(d)).join('')}
            </div>
        `;
        parentEl.appendChild(panel);
        this._panel = panel;
        this._rowsContainer = panel.querySelector('.flux-slice-rows');
        this._resetMirrorBtn = panel.querySelector('.flux-slice-reset-mirror');

        // Wire per-field rows
        for (const drv of FIELD_DRIVERS) {
            const row = panel.querySelector(`.flux-slice-row[data-field="${drv.key}"]`);
            const chip = row.querySelector('.flux-slice-mirror-chip');
            const slots = {};
            for (const axis of FluxSlicePanel.AXES) {
                slots[axis] = this._wireSlot(row, drv.key, axis);
            }
            this._fields[drv.key] = { row, label: row.querySelector('.flux-slice-field-label'), chip, slots };

            chip.addEventListener('click', () => this._cycleOverride(drv.key));
        }

        // Toggle chip (panel show/hide)
        const chip = document.createElement('button');
        chip.type = 'button';
        chip.id = 'flux-slice-toggle';
        chip.className = 'scale0-only flux-slice-chip';
        chip.title = 'Toggle live flux slice diagnostics (xy/xz/yz across enabled fields)';
        chip.textContent = 'Flux slices';
        chip.addEventListener('click', () => this.toggle());
        parentEl.appendChild(chip);
        this._chip = chip;

        panel.querySelector('.flux-slice-close')
            ?.addEventListener('click', () => this.setVisible(false));

        // Header axis toggles — independent xy/xz/yz visibility, applies
        // to every visible row simultaneously.
        panel.querySelectorAll('.flux-slice-axis-btn').forEach((btn) => {
            btn.addEventListener('click', () => {
                const axis = btn.dataset.axis;
                if (!axis || !FluxSlicePanel.AXES.includes(axis)) return;
                this.setAxisVisible(axis, !this._axisVisible[axis]);
            });
        });

        // Reset mirror clears every per-field override at once.
        this._resetMirrorBtn.addEventListener('click', () => this.resetMirror());

        // Initial chip-label refresh + reconcile.
        for (const drv of FIELD_DRIVERS) this._refreshChip(drv.key);
        this._refreshResetButton();

        return panel;
    }

    _rowHTML(drv) {
        const tilesHTML = FluxSlicePanel.AXES.map(axis => `
            <figure class="flux-slice-tile" data-plane="${axis}" data-field="${drv.key}">
                <canvas class="flux-slice-canvas"
                        width="${this.canvasPx}" height="${this.canvasPx}"></canvas>
                <figcaption class="flux-slice-caption">
                    <span class="flux-slice-plane-label">${this._planeLabel(axis)}</span>
                    <span class="flux-slice-readout"
                          data-readout="${drv.key}-${axis}">t=— · max —</span>
                </figcaption>
            </figure>
        `).join('');
        return `
            <div class="flux-slice-row" data-field="${drv.key}">
                <div class="flux-slice-row-label">
                    <span class="flux-slice-field-label" title="${drv.label} field">${drv.label}</span>
                    <button type="button" class="flux-slice-mirror-chip"
                            data-field="${drv.key}"
                            title="Cycle: mirror → force-on → force-off → mirror">mirror</button>
                </div>
                <div class="flux-slice-row-tiles">${tilesHTML}</div>
            </div>
        `;
    }

    _planeLabel(axis) {
        if (axis === 'xy') return 'xy @ z=L/2';
        if (axis === 'xz') return 'xz @ y=L/2';
        return 'yz @ x=L/2';
    }

    _wireSlot(row, fieldKey, axis) {
        const tile = row.querySelector(
            `.flux-slice-tile[data-plane="${axis}"][data-field="${fieldKey}"]`);
        const canvas = tile.querySelector('canvas');
        const ctx = canvas.getContext('2d', { alpha: false });
        ctx.imageSmoothingEnabled = false;
        return {
            tile, canvas, ctx,
            readout: row.querySelector(`[data-readout="${fieldKey}-${axis}"]`),
            imgData: null,
            currentN: 0,
            rgbaBuf: null,
            tmpCanvas: null,
            tmpCtx: null,
        };
    }

    // ── Visibility (panel-level) ──────────────────────────────────────

    setVisible(on) {
        this.visible = !!on;
        if (this._panel) this._panel.style.display = this.visible ? '' : 'none';
        if (this._chip) this._chip.classList.toggle('active', this.visible);
        if (this.visible) {
            // Force a fresh paint + reconcile mirror state right away —
            // the panel may have been hidden across a scenario change.
            this.frameCount = 0;
            this._startSelfDrive();
        } else {
            this._stopSelfDrive();
        }
    }

    _startSelfDrive() {
        if (this._rafId !== null) return;
        if (typeof requestAnimationFrame !== 'function') return;
        const loop = (now) => {
            // Stop loop if the panel was hidden out from under us.
            if (!this.visible) { this._rafId = null; return; }
            // Run our update — internal updateEvery throttle still applies,
            // and a no-op fast-out fires when the panel is hidden anyway.
            try { this.update(); }
            catch (e) {
                console.warn('[flux-slice-panel] self-drive update failed:', e);
            }
            this._lastSelfTickMs = now || performance.now();
            this._rafId = requestAnimationFrame(loop);
        };
        this._rafId = requestAnimationFrame(loop);
    }

    _stopSelfDrive() {
        if (this._rafId !== null && typeof cancelAnimationFrame === 'function') {
            cancelAnimationFrame(this._rafId);
        }
        this._rafId = null;
    }

    toggle() { this.setVisible(!this.visible); }

    setAxisVisible(axis, on) {
        if (!FluxSlicePanel.AXES.includes(axis)) return;
        this._axisVisible[axis] = !!on;
        if (this._panel) {
            // Toggle the matching tile column across every row.
            this._panel
                .querySelectorAll(`.flux-slice-tile[data-plane="${axis}"]`)
                .forEach(t => t.classList.toggle('axis-hidden', !this._axisVisible[axis]));
            const btn = this._panel.querySelector(
                `.flux-slice-axis-btn[data-axis="${axis}"]`);
            if (btn) {
                btn.classList.toggle('active', this._axisVisible[axis]);
                btn.setAttribute('aria-pressed', this._axisVisible[axis] ? 'true' : 'false');
            }
        }
        if (this._axisVisible[axis]) this.frameCount = 0;
    }

    // ── Per-field override + mirror logic ─────────────────────────────

    _isFieldRowVisible(fieldKey) {
        const ov = this._fieldOverride[fieldKey];
        if (ov === 'on') return true;
        if (ov === 'off') return false;
        const drv = DRIVER_BY_KEY[fieldKey];
        return !!this._mirroredFlags?.[drv?.vizFlagKey];
    }

    setFieldOverride(fieldKey, value) {
        if (!(fieldKey in this._fieldOverride)) return;
        if (value !== null && value !== 'on' && value !== 'off') value = null;
        this._fieldOverride[fieldKey] = value;
        this._refreshChip(fieldKey);
        this._refreshResetButton();
        // Force a fresh paint so a just-revealed row gets data immediately.
        this.frameCount = 0;
    }

    _cycleOverride(fieldKey) {
        const cur = this._fieldOverride[fieldKey];
        const next = cur === null ? 'on' : (cur === 'on' ? 'off' : null);
        this.setFieldOverride(fieldKey, next);
    }

    resetMirror() {
        for (const k of Object.keys(this._fieldOverride)) {
            this._fieldOverride[k] = null;
            this._refreshChip(k);
        }
        this._refreshResetButton();
        this.frameCount = 0;
    }

    _refreshChip(fieldKey) {
        const slot = this._fields?.[fieldKey];
        if (!slot) return;
        const ov = this._fieldOverride[fieldKey];
        const drv = DRIVER_BY_KEY[fieldKey];
        const mirrorOn = !!this._mirroredFlags?.[drv?.vizFlagKey];
        slot.chip.classList.remove('override-on', 'override-off');
        if (ov === 'on') {
            slot.chip.textContent = 'force on';
            slot.chip.classList.add('override-on');
            slot.chip.title = `Forced visible. Click to force-off.`;
        } else if (ov === 'off') {
            slot.chip.textContent = 'force off';
            slot.chip.classList.add('override-off');
            slot.chip.title = `Forced hidden. Click to return to mirror.`;
        } else {
            slot.chip.textContent = 'mirror';
            slot.chip.title =
                `Following viz toggle (currently ${mirrorOn ? 'on' : 'off'}). ` +
                `Click to force-on.`;
        }
    }

    _refreshResetButton() {
        if (!this._resetMirrorBtn) return;
        const anyOverride = Object.values(this._fieldOverride).some(v => v !== null);
        this._resetMirrorBtn.disabled = !anyOverride;
    }

    // ── Per-frame update ──────────────────────────────────────────────

    update() {
        if (!this.visible || !this._panel) return;
        this.frameCount = (this.frameCount + 1) | 0;
        if ((this.frameCount % this.updateEvery) !== 0) return;

        const bridge = this.getBridge?.();
        if (!bridge) return;

        const N = bridge.latticeSize | 0;
        if (!Number.isFinite(N) || N < 2) return;

        // Refresh viz-panel flags + chip labels (mirror display may have
        // changed even if no override flips happened).
        try {
            this._mirroredFlags = getFieldStateSnapshot() ?? {};
        } catch (_e) {
            this._mirroredFlags = {};
        }
        for (const drv of FIELD_DRIVERS) this._refreshChip(drv.key);

        // (Re)allocate per-(field,axis) RGBA buffers if the lattice resized.
        if (N !== this._lastN) {
            const px = N * N * 4;
            for (const drv of FIELD_DRIVERS) {
                for (const axis of FluxSlicePanel.AXES) {
                    const slot = this._fields[drv.key].slots[axis];
                    slot.rgbaBuf = new Uint8ClampedArray(px);
                    slot.imgData = null;
                    slot.currentN = 0;
                    slot.tmpCanvas = null;
                    slot.tmpCtx = null;
                }
            }
            this._lastN = N;
        }

        const mid = N >> 1;
        const diag = bridge.getDiagnostics?.() ?? {};
        const simTick = (diag.tick ?? 0) | 0;
        const globalTick = (typeof window !== 'undefined' && window.__ftdCtx)
            ? (window.__ftdCtx.globalTick | 0)
            : simTick;
        const isRunning = (typeof window !== 'undefined' && window.__ftdCtx)
            ? !!window.__ftdCtx.running
            : true;

        // Scenario change → simTick reset to 0 from a positive value.
        // Clear all per-field overrides and rolling autoscale so the new
        // scenario starts clean.
        if (this._lastSimTick > 0 && simTick < this._lastSimTick) {
            for (const k of Object.keys(this._fieldOverride)) {
                if (k === 'fluxJ') continue; // |J| keeps default 'on'
                this._fieldOverride[k] = null;
            }
            for (const k of Object.keys(this._fieldGlobalMax)) {
                this._fieldGlobalMax[k] = 0;
            }
            // Default |J| stays 'on'; everything else mirrors the new scenario's flags.
            this._fieldOverride.fluxJ = 'on';
            for (const drv of FIELD_DRIVERS) this._refreshChip(drv.key);
            this._refreshResetButton();
        }
        this._lastSimTick = simTick;

        // Reconcile row visibility + dense layout.
        let activeCount = 0;
        for (const drv of FIELD_DRIVERS) {
            const visible = this._isFieldRowVisible(drv.key);
            const row = this._fields[drv.key].row;
            const wasHidden = row.classList.contains('row-hidden');
            if (visible && wasHidden) row.classList.remove('row-hidden');
            else if (!visible && !wasHidden) row.classList.add('row-hidden');
            if (visible) activeCount++;
        }
        // Dense mode shrinks tile size when many rows are open.
        this._panel.classList.toggle('dense', activeCount > DENSE_THRESHOLD);

        // Per-field sample → max → paint.
        for (const drv of FIELD_DRIVERS) {
            if (!this._isFieldRowVisible(drv.key)) continue;

            // Pull the per-frame source ONCE (a single bridge call), then
            // filter it per-axis below. Saves ~2× bridge cost vs the naive
            // sample-per-axis pattern when 3 axes are visible.
            const source = drv.source?.(bridge);

            // Sample only the visible axes for this field.
            const slices = {};
            for (const axis of FluxSlicePanel.AXES) {
                slices[axis] = this._axisVisible[axis]
                    ? drv.sample(bridge, axisIndex(axis), mid, N, source)
                    : null;
            }

            // Per-axis max (for the readout) + per-field union max (for color).
            const axisMax = { xy: 0, xz: 0, yz: 0 };
            let fieldFrameMax = 0;
            for (const axis of FluxSlicePanel.AXES) {
                const s = slices[axis];
                if (!s || s.length === 0) continue;
                let m = 0;
                if (drv.signed) {
                    for (let i = 0; i < s.length; i++) {
                        const av = Math.abs(s[i]);
                        if (av > m) m = av;
                    }
                } else {
                    for (let i = 0; i < s.length; i++) {
                        const v = s[i];
                        if (v > m) m = v;
                    }
                }
                axisMax[axis] = m;
                if (m > fieldFrameMax) fieldFrameMax = m;
            }

            // Decay-then-lift rolling max, per field.
            const prevMax = this._fieldGlobalMax[drv.key] ?? 0;
            const newMax = Math.max(prevMax * this._maxDecay, fieldFrameMax);
            this._fieldGlobalMax[drv.key] = newMax;
            const norm = newMax > FLOOR_FRAC ? 1 / newMax : 0;

            // Paint each visible axis.
            for (const axis of FluxSlicePanel.AXES) {
                if (!this._axisVisible[axis]) continue;
                this._paintSlice(drv, axis, slices[axis], N, norm,
                                 axisMax[axis], simTick, globalTick, isRunning);
            }
        }
    }

    _paintSlice(drv, axis, data, N, norm, axisFrameMax, simTick, globalTick, isRunning) {
        const slot = this._fields[drv.key]?.slots[axis];
        if (!slot) return;
        const buf = slot.rgbaBuf;
        if (!buf) return;
        if (!data || data.length === 0) {
            const c = slot.ctx;
            c.fillStyle = '#0a0d14';
            c.fillRect(0, 0, slot.canvas.width, slot.canvas.height);
            slot.readout.textContent = `t=${simTick} (g=${globalTick}) · max —`;
            return;
        }

        const ramp = drv.ramp;
        const rgb = [0, 0, 0];
        if (drv.signed) {
            // Map t ∈ [-1, +1] through the diverging ramp (rampCharge etc.).
            for (let i = 0, p = 0; i < data.length; i++, p += 4) {
                let t = data[i] * norm;
                if (t > 1) t = 1; else if (t < -1) t = -1;
                ramp(t, rgb, 0);
                buf[p]     = (rgb[0] * 255) | 0;
                buf[p + 1] = (rgb[1] * 255) | 0;
                buf[p + 2] = (rgb[2] * 255) | 0;
                buf[p + 3] = 255;
            }
        } else {
            for (let i = 0, p = 0; i < data.length; i++, p += 4) {
                let t = data[i] * norm;
                if (t > 1) t = 1; else if (t < 0) t = 0;
                ramp(t, rgb, 0);
                buf[p]     = (rgb[0] * 255) | 0;
                buf[p + 1] = (rgb[1] * 255) | 0;
                buf[p + 2] = (rgb[2] * 255) | 0;
                buf[p + 3] = 255;
            }
        }

        if (!slot.imgData || slot.currentN !== N) {
            slot.imgData = new ImageData(buf, N, N);
            slot.currentN = N;
        }

        // Refresh the visible canvas size if the panel just toggled to/from
        // dense mode (rare; only when the active row count crosses the
        // threshold).
        const desiredPx = this._panel?.classList.contains('dense')
            ? DENSE_CANVAS_PX : this.canvasPx;
        if (slot.canvas.width !== desiredPx || slot.canvas.height !== desiredPx) {
            slot.canvas.width = desiredPx;
            slot.canvas.height = desiredPx;
            slot.ctx.imageSmoothingEnabled = false;
        }

        const c = slot.ctx;
        const W = slot.canvas.width;
        const H = slot.canvas.height;

        if (!slot.tmpCanvas || slot.tmpCanvas.width !== N) {
            slot.tmpCanvas = document.createElement('canvas');
            slot.tmpCanvas.width = N;
            slot.tmpCanvas.height = N;
            slot.tmpCtx = slot.tmpCanvas.getContext('2d', { alpha: false });
            slot.tmpCtx.imageSmoothingEnabled = false;
        }
        slot.tmpCtx.putImageData(slot.imgData, 0, 0);
        c.imageSmoothingEnabled = false;
        c.drawImage(slot.tmpCanvas, 0, 0, N, N, 0, 0, W, H);

        const pausedTag = isRunning ? '' : ' ⏸';
        slot.readout.textContent =
            `t=${simTick} (g=${globalTick})${pausedTag} · max ${this._fmt(axisFrameMax)}`;
    }

    _fmt(v) {
        if (!Number.isFinite(v)) return '—';
        if (v === 0) return '0';
        const abs = Math.abs(v);
        if (abs >= 1000 || abs < 0.01) return v.toExponential(2);
        return v.toFixed(3);
    }

    // ── Teardown ──────────────────────────────────────────────────────

    dispose() {
        this._stopSelfDrive();
        this._panel?.remove();
        this._chip?.remove();
        this._panel = null;
        this._chip = null;
        this._fields = {};
        this._rowsContainer = null;
        this._resetMirrorBtn = null;
    }
}

function axisIndex(axis) {
    if (axis === 'yz') return 0;
    if (axis === 'xz') return 1;
    return 2; // xy
}

/**
 * Idempotent mount used from the Scale 0 controller. Stashes the
 * singleton on `window.__ftdFluxSlicePanel` so a re-entry (scale-switch
 * round-trip) reuses the same DOM node.
 */
export function mountFluxSlicePanel(parentEl, getBridge) {
    if (typeof window !== 'undefined' && window.__ftdFluxSlicePanel) {
        return window.__ftdFluxSlicePanel;
    }
    const panel = new FluxSlicePanel({ getBridge });
    panel.init(parentEl);
    if (typeof window !== 'undefined') window.__ftdFluxSlicePanel = panel;
    return panel;
}
