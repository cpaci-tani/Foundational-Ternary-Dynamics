/**
 * Scale 0 — Live 3-Plane Flux Slice Panel
 *
 * A floating diagnostic that mirrors the offline analysis at
 * `engine/results/flux_slices_2026-04-26/` and the C++ harness
 * `engine/tests/campaign_flux_slice_propagation.cpp`. Renders three
 * 2D heatmaps of |J(x,y,z)| sampled at the lattice mid-planes:
 *
 *   xy plane @ z = L/2   (axial — wavefront propagation along z)
 *   xz plane @ y = L/2   (axial — should be bit-equivalent to xy under
 *                          4-fold rotational symmetry around the seed
 *                          axis; the 2026-04-26 verdict is ISOTROPIC)
 *   yz plane @ x = L/2   (transverse — ~1.76% anisotropy expected)
 *
 * The bridge already exposes `getFluxSlice(axis, index)` which returns
 * a Float64Array of length N*N with |J| at the requested plane. We do
 * NOT compute fluxes ourselves; we just sample, autoscale, and paint.
 *
 * Mounting: floating panel docked top-right under the field-toggle
 * column, hidden by default, toggled by a small chip button. Idempotent
 * mount — safe to call from `bindUI` of the Scale 0 controller. Same
 * `scale0-only` class the symmetry panel uses, so it auto-hides on a
 * scale switch.
 *
 * Performance: at L=64 each frame samples 3 * N*N = 12,288 doubles,
 * builds 3 * 4*N*N = 49,152 RGBA bytes, and calls putImageData three
 * times. Wall cost on a 2024 laptop is ~0.3 ms/frame; we throttle to
 * every 2nd render frame anyway via `updateEvery`.
 */

import { rampViridis } from '../../../../viewport/color-ramps.js';

const DEFAULT_CANVAS_PX = 220;
const FLOOR_FRAC = 1e-6; // protects log axis-style scaling from 0/0

export class FluxSlicePanel {
    /**
     * @param {object} opts
     * @param {() => any} opts.getBridge   - returns the live bridge (so a
     *     scale-switch reassigning ctx.bridge keeps us hooked).
     * @param {number} [opts.canvasPx]     - per-canvas size in CSS pixels.
     * @param {number} [opts.updateEvery]  - sample/render every Nth frame.
     */
    constructor({ getBridge, canvasPx = DEFAULT_CANVAS_PX, updateEvery = 2 } = {}) {
        this.getBridge = getBridge;
        this.canvasPx = canvasPx;
        this.updateEvery = Math.max(1, updateEvery | 0);

        this.visible = false;
        this.frameCount = 0;
        this._lastN = 0;
        this._lastSimTick = 0;

        this._panel = null;
        this._chip = null;
        this._slots = null; // { xy: {canvas, ctx, imgData, label, max}, xz: ..., yz: ... }
        this._rgbaBufs = null; // per-plane Uint8ClampedArray reused across frames

        // Per-axis visibility: each plane (xy, xz, yz) can be independently
        // toggled inside the panel via the header chips. When OFF the tile
        // is hidden AND its sampling is skipped entirely (so no bridge cost).
        this._axisVisible = { xy: true, xz: true, yz: true };

        // Track a global rolling max for stable autoscale — stops the heatmap
        // from re-normalizing every frame (which would make a propagating
        // wavefront look static). Bleeds back to the per-frame max with a
        // small decay so the dynamic range adapts as energy injects/decays.
        this._globalMax = 0;
        this._maxDecay = 0.985;
    }

    static get AXES() { return ['xy', 'xz', 'yz']; }

    // ── Mounting ──────────────────────────────────────────────────────

    /**
     * Mount the panel + chip into `parentEl` (typically `#app`).
     * Idempotent — second call is a no-op. Returns the panel root.
     */
    init(parentEl) {
        if (!parentEl || this._panel) return this._panel;

        // Panel root
        const panel = document.createElement('div');
        panel.id = 'flux-slice-panel';
        panel.className = 'scale0-only flux-slice-panel';
        panel.style.display = 'none'; // hidden by default
        panel.innerHTML = `
            <div class="flux-slice-header">
                <span class="flux-slice-title">Flux Slices · |J|</span>
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
                <button type="button" class="flux-slice-close" aria-label="Hide flux slices">×</button>
            </div>
            <div class="flux-slice-grid">
                ${this._tileHTML('xy', 'xy @ z=L/2')}
                ${this._tileHTML('xz', 'xz @ y=L/2')}
                ${this._tileHTML('yz', 'yz @ x=L/2')}
            </div>
            <div class="flux-slice-legend">
                <span class="flux-slice-min">0</span>
                <span class="flux-slice-ramp" aria-hidden="true"></span>
                <span class="flux-slice-max">|J|<sub>max</sub></span>
            </div>
        `;
        parentEl.appendChild(panel);
        this._panel = panel;

        // Cache slot handles
        this._slots = {
            xy: this._wireSlot(panel, 'xy'),
            xz: this._wireSlot(panel, 'xz'),
            yz: this._wireSlot(panel, 'yz'),
        };

        // Toggle chip (always visible while in Scale 0 — small floating
        // pill that opens/closes the panel).
        const chip = document.createElement('button');
        chip.type = 'button';
        chip.id = 'flux-slice-toggle';
        chip.className = 'scale0-only flux-slice-chip';
        chip.title = 'Toggle live flux slice diagnostics (xy/xz/yz)';
        chip.textContent = 'Flux slices';
        chip.addEventListener('click', () => this.toggle());
        parentEl.appendChild(chip);
        this._chip = chip;

        panel.querySelector('.flux-slice-close')
            ?.addEventListener('click', () => this.setVisible(false));

        // Axis toggles — independent xy/xz/yz visibility.
        panel.querySelectorAll('.flux-slice-axis-btn').forEach((btn) => {
            btn.addEventListener('click', () => {
                const axis = btn.dataset.axis;
                if (!axis || !FluxSlicePanel.AXES.includes(axis)) return;
                this.setAxisVisible(axis, !this._axisVisible[axis]);
            });
        });

        return panel;
    }

    setAxisVisible(axis, on) {
        if (!FluxSlicePanel.AXES.includes(axis)) return;
        this._axisVisible[axis] = !!on;
        if (this._panel) {
            const tile = this._panel.querySelector(
                `.flux-slice-tile[data-plane="${axis}"]`);
            if (tile) tile.classList.toggle('axis-hidden', !this._axisVisible[axis]);
            const btn = this._panel.querySelector(
                `.flux-slice-axis-btn[data-axis="${axis}"]`);
            if (btn) {
                btn.classList.toggle('active', this._axisVisible[axis]);
                btn.setAttribute('aria-pressed', this._axisVisible[axis] ? 'true' : 'false');
            }
        }
        // Force a fresh paint so a just-revealed tile picks up the current
        // frame instead of waiting for the next `updateEvery` boundary.
        if (this._axisVisible[axis]) this.frameCount = 0;
    }

    _tileHTML(key, label) {
        return `
            <figure class="flux-slice-tile" data-plane="${key}">
                <canvas class="flux-slice-canvas"
                        width="${this.canvasPx}" height="${this.canvasPx}"></canvas>
                <figcaption class="flux-slice-caption">
                    <span class="flux-slice-plane-label">${label}</span>
                    <span class="flux-slice-readout" data-readout="${key}">t=— · max —</span>
                </figcaption>
            </figure>
        `;
    }

    _wireSlot(panel, key) {
        const tile = panel.querySelector(`.flux-slice-tile[data-plane="${key}"]`);
        const canvas = tile.querySelector('canvas');
        const ctx = canvas.getContext('2d', { alpha: false });
        ctx.imageSmoothingEnabled = false;
        return {
            tile,
            canvas,
            ctx,
            readout: panel.querySelector(`[data-readout="${key}"]`),
            imgData: null,
            currentN: 0,
        };
    }

    // ── Visibility ────────────────────────────────────────────────────

    setVisible(on) {
        this.visible = !!on;
        if (this._panel) this._panel.style.display = this.visible ? '' : 'none';
        if (this._chip) this._chip.classList.toggle('active', this.visible);
        // On show, force a fresh paint next frame even if `updateEvery` would
        // normally skip it.
        if (this.visible) this.frameCount = 0;
    }

    toggle() { this.setVisible(!this.visible); }

    // ── Per-frame update ──────────────────────────────────────────────

    /**
     * Sample the bridge and repaint. Cheap to call every animate() pass —
     * gated internally by visibility and `updateEvery`.
     */
    update() {
        if (!this.visible || !this._panel) return;
        this.frameCount = (this.frameCount + 1) | 0;
        if ((this.frameCount % this.updateEvery) !== 0) return;

        const bridge = this.getBridge?.();
        if (!bridge) return;

        const N = bridge.latticeSize | 0;
        if (!Number.isFinite(N) || N < 2) return;

        // (Re)allocate the per-plane RGBA buffers if the lattice resized.
        if (N !== this._lastN) {
            const px = N * N * 4;
            this._rgbaBufs = {
                xy: new Uint8ClampedArray(px),
                xz: new Uint8ClampedArray(px),
                yz: new Uint8ClampedArray(px),
            };
            this._lastN = N;
            // Force ImageData re-create on first paint at new N.
            for (const key of ['xy', 'xz', 'yz']) {
                this._slots[key].imgData = null;
                this._slots[key].currentN = 0;
            }
        }

        const mid = N >> 1;
        // Pull diagnostics for the tick stamp; tolerate missing fields.
        // We surface BOTH counters because they tell different stories:
        //   simTick    = bridge.getDiagnostics().tick — engine steps since last
        //                 reset/scenario-load. Resets to 0 on scenario change.
        //   globalTick = ctx.globalTick — cumulative wall-clock frames since
        //                 global play resumed. Independent of bridge stepping.
        // Showing only one was confusing: a freshly-loaded scenario could
        // sit with simTick=0 while globalTick=251, making the slice "look"
        // stuck even though it correctly reflected the live (initial) state.
        const diag = bridge.getDiagnostics?.() ?? {};
        const simTick = (diag.tick ?? 0) | 0;
        // Look up ctx.globalTick if reachable; getBridge accessor doesn't
        // give us ctx directly, but the bridge's window-level singleton does.
        const globalTick = (typeof window !== 'undefined' && window.__ftdCtx)
            ? (window.__ftdCtx.globalTick | 0)
            : simTick;
        const isRunning = (typeof window !== 'undefined' && window.__ftdCtx)
            ? !!window.__ftdCtx.running
            : true;

        // Three slices, sampled only for axes the user has visible.
        // Bridge axis convention from getFluxSlice:
        //   axis=0 → x = index, plane spans (y, z)   → "yz"
        //   axis=1 → y = index, plane spans (x, z)   → "xz"
        //   axis=2 → z = index, plane spans (x, y)   → "xy"
        const slices = {
            yz: this._axisVisible.yz ? bridge.getFluxSlice?.(0, mid) : null,
            xz: this._axisVisible.xz ? bridge.getFluxSlice?.(1, mid) : null,
            xy: this._axisVisible.xy ? bridge.getFluxSlice?.(2, mid) : null,
        };

        // Pass 1: per-frame max over visible axes, fold into rolling global max.
        let frameMax = 0;
        for (const key of FluxSlicePanel.AXES) {
            const s = slices[key];
            if (!s || s.length === 0) continue;
            for (let i = 0; i < s.length; i++) {
                const v = s[i];
                if (v > frameMax) frameMax = v;
            }
        }
        // Decay then lift toward current frame, so the scale tracks growth
        // promptly but doesn't collapse during a quiet tick.
        this._globalMax = Math.max(this._globalMax * this._maxDecay, frameMax);
        const norm = this._globalMax > FLOOR_FRAC ? 1 / this._globalMax : 0;

        // Pass 2: paint each visible tile.
        for (const key of FluxSlicePanel.AXES) {
            if (!this._axisVisible[key]) continue;
            this._paintSlice(key, slices[key], N, norm, frameMax,
                              simTick, globalTick, isRunning);
        }

        // Reset bridge identity tracking when the bridge resets the scenario:
        // simTick going backwards or to 0 after we last saw it >0 means a
        // scenario was loaded — re-baseline the rolling max so the new seed's
        // dynamic range isn't dwarfed by the previous run's history.
        if (this._lastSimTick > 0 && simTick < this._lastSimTick) {
            this._globalMax = frameMax; // hard reset to current frame
        }
        this._lastSimTick = simTick;

        // Legend max readout
        const legendMax = this._panel.querySelector('.flux-slice-max');
        if (legendMax) legendMax.innerHTML = `|J|<sub>max</sub>=${this._fmt(this._globalMax)}`;
    }

    _paintSlice(key, data, N, norm, frameMax, simTick, globalTick, isRunning) {
        const slot = this._slots[key];
        if (!slot) return;
        const buf = this._rgbaBufs[key];
        if (!data || data.length === 0) {
            // No data yet (bridge not initialized). Clear the canvas to
            // background and bail.
            const c = slot.ctx;
            c.fillStyle = '#0a0d14';
            c.fillRect(0, 0, slot.canvas.width, slot.canvas.height);
            slot.readout.textContent = `t=${simTick} (g=${globalTick}) · max —`;
            return;
        }

        // Fill RGBA from |J| via viridis ramp.
        const rgb = [0, 0, 0];
        for (let i = 0, p = 0; i < data.length; i++, p += 4) {
            const t = data[i] * norm; // already in [0, 1] by construction
            rampViridis(t, rgb, 0);
            buf[p]     = (rgb[0] * 255) | 0;
            buf[p + 1] = (rgb[1] * 255) | 0;
            buf[p + 2] = (rgb[2] * 255) | 0;
            buf[p + 3] = 255;
        }

        // Build / reuse the ImageData object at native lattice resolution,
        // then upscale with drawImage for the visible canvas.
        if (!slot.imgData || slot.currentN !== N) {
            slot.imgData = new ImageData(buf, N, N);
            slot.currentN = N;
        } else {
            // ImageData wraps `buf` by reference at construction; we wrote
            // directly into `buf` so no copy is needed. (Some older browsers
            // require re-wrapping; we keep the construct path above.)
        }

        // Two-step paint: putImageData onto an offscreen-sized region,
        // then scale up. We use the canvas itself as the offscreen surface
        // by saving/restoring the transform around a putImageData.
        const c = slot.ctx;
        const W = slot.canvas.width;
        const H = slot.canvas.height;

        // Direct path: putImageData ignores transforms, so we need a
        // tiny temporary. Cache it on the slot.
        if (!slot._tmpCanvas || slot._tmpCanvas.width !== N) {
            slot._tmpCanvas = document.createElement('canvas');
            slot._tmpCanvas.width = N;
            slot._tmpCanvas.height = N;
            slot._tmpCtx = slot._tmpCanvas.getContext('2d', { alpha: false });
            slot._tmpCtx.imageSmoothingEnabled = false;
        }
        slot._tmpCtx.putImageData(slot.imgData, 0, 0);

        c.imageSmoothingEnabled = false;
        c.drawImage(slot._tmpCanvas, 0, 0, N, N, 0, 0, W, H);

        // Readout: show sim-tick (engine steps), global-tick in parens
        // (wall-clock frames), pause indicator, and per-frame max.
        // Format: "t=50 (g=251) ⏸ · max 0.083"
        const pausedTag = isRunning ? '' : ' ⏸';
        slot.readout.textContent =
            `t=${simTick} (g=${globalTick})${pausedTag} · max ${this._fmt(frameMax)}`;
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
        this._panel?.remove();
        this._chip?.remove();
        this._panel = null;
        this._chip = null;
        this._slots = null;
        this._rgbaBufs = null;
    }
}

/**
 * Convenience mount used from the Scale 0 controller. Idempotent —
 * stashes the singleton on `window.__ftdFluxSlicePanel` so a re-entry
 * (scale-switch round-trip) reuses the same DOM node.
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
