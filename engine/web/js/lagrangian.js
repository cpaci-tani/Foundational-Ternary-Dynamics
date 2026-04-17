/**
 * Lagrangian Panel — stacked area chart of 7 Lagrangian density terms.
 *
 * Field sector:      Field Kinetic (green), Field Gradient (teal)
 * Particle sector:   Born-Infeld (red)
 * Interaction:       Coupling (orange), Velocity (gold)
 * Constraint:        Gauss (blue)
 * Dissipation:       Rayleigh (gray)
 *
 * Also displays Hamiltonian, total Lagrangian, and discrete Action S.
 */

import { createCachedCanvasRect } from './dom-utils.js';
import { RingBuffer } from './telemetry-hub.js';

const BUFFER_SIZE = 400;
const TERM_COLORS = {
    fieldKinetic:  '#66bb6a',
    fieldGradient: '#26a69a',
    bornInfeld:  '#ef5350',
    coupling:    '#fb8c00',
    velocity:    '#fdd835',
    gauss:       '#42a5f5',
    dissipation: '#78909c',
};

export class LagrangianChart {
    /**
     * @param {HTMLCanvasElement} canvas
     * @param {object} [lagBufs]  hub.lag — keyed RingBuffer instances.
     *   When supplied the chart reads from those buffers directly (single write path).
     *   When omitted it allocates its own local RingBuffers (standalone / test use).
     */
    constructor(canvas, lagBufs = null) {
        this.canvas = canvas;
        // If hub buffers are provided, use them; otherwise allocate locally.
        this.lag = lagBufs || {
            fieldKinetic:  new RingBuffer(BUFFER_SIZE),
            fieldGradient: new RingBuffer(BUFFER_SIZE),
            bornInfeld:    new RingBuffer(BUFFER_SIZE),
            coupling:      new RingBuffer(BUFFER_SIZE),
            velocity:      new RingBuffer(BUFFER_SIZE),
            gauss:         new RingBuffer(BUFFER_SIZE),
            dissipation:   new RingBuffer(BUFFER_SIZE),
            total:         new RingBuffer(BUFFER_SIZE),
            hamiltonian:   new RingBuffer(BUFFER_SIZE),
            action:        new RingBuffer(BUFFER_SIZE),
        };
        // Keep a legacy TermBuffer alias so existing push() callers still work
        // when hub buffers are NOT injected (standalone use).
        this._ownedBuffers = !lagBufs;
        // Phase C.3: cache rect, refreshed via ResizeObserver
        this._rectCache = canvas ? createCachedCanvasRect(canvas) : null;
        this.visible = {
            fieldKinetic: true,
            fieldGradient: true,
            bornInfeld: true,
            coupling: true,
            velocity: true,
            gauss: true,
            dissipation: true,
        };

        // Wire up term toggle checkboxes
        this._wireToggle('lt-field-kinetic', 'fieldKinetic');
        this._wireToggle('lt-field-gradient', 'fieldGradient');
        this._wireToggle('lt-bi', 'bornInfeld');
        this._wireToggle('lt-coupling', 'coupling');
        this._wireToggle('lt-velocity', 'velocity');
        this._wireToggle('lt-gauss', 'gauss');
        this._wireToggle('lt-dissipation', 'dissipation');
    }

    _wireToggle(elId, key) {
        const el = document.getElementById(elId);
        if (el) {
            el.addEventListener('change', () => {
                this.visible[key] = el.checked;
            });
        }
    }

    /**
     * Push Lagrangian data.
     * When hub buffers are injected this is a no-op (hub already wrote via
     * telemetryHub.collectScale0Lagrangian). Called directly only in
     * standalone/fallback mode.
     */
    push(lag) {
        if (this._ownedBuffers) {
            this.lag.fieldKinetic.push( Math.abs(lag.fieldKinetic  || 0));
            this.lag.fieldGradient.push(Math.abs(lag.fieldGradient || 0));
            this.lag.bornInfeld.push(   Math.abs(lag.bornInfeld    || 0));
            this.lag.coupling.push(     Math.abs(lag.coupling      || 0));
            this.lag.velocity.push(     Math.abs(lag.velocity      || 0));
            this.lag.gauss.push(        Math.abs(lag.gauss         || 0));
            this.lag.dissipation.push(  Math.abs(lag.dissipation   || 0));
            this.lag.total.push(         lag.total                  || 0);
            this.lag.hamiltonian.push(   lag.hamiltonian            || 0);
            this.lag.action.push(        lag.totalAction            || 0);
        }
        // Always update constraint DOM display regardless of buffer ownership
        this._updateConstraints(lag);
    }

    _updateConstraints(lag) {
        const set = (id, val, unit = '') => {
            const el = document.getElementById(id);
            if (el) el.textContent = (typeof val === 'number' ? fmtVal(val) : val) + (unit ? ' ' + unit : '');
        };
        set('lag-action', lag.totalAction, '\u0127');
        set('lag-gauss-viol', lag.gaussViolation, '/vox');
        set('lag-max-gauss', lag.maxGaussError, '/vox');
        set('lag-flux-mag', lag.totalFluxMag, 'Pl');
        set('lag-wave-ke', lag.totalWaveEnergy, 'Pl');
        set('lag-manifested', lag.manifested);
        set('lag-locked', lag.locked);
    }

    draw() {
        const canvas = this.canvas;
        if (!canvas) return;
        const rect = this._rectCache ? this._rectCache.get() : canvas.getBoundingClientRect();
        const w = rect.width;
        const h = rect.height;
        // PERF: Skip drawing when canvas is hidden (zero-size or offscreen)
        if (w === 0 || h === 0) return;

        const ctx = canvas.getContext('2d');
        const dpr = window.devicePixelRatio || 1;

        if (canvas.width !== w * dpr || canvas.height !== h * dpr) {
            canvas.width = w * dpr;
            canvas.height = h * dpr;
            ctx.setTransform(dpr, 0, 0, dpr, 0, 0);
        }

        ctx.clearRect(0, 0, w, h);

        // n = number of samples available (from any term buffer)
        const n = this.lag.fieldKinetic.count;
        if (n < 2) return;

        const margin = { top: 8, right: 8, bottom: 20, left: 50 };
        const plotW = w - margin.left - margin.right;
        const plotH = h - margin.top - margin.bottom;

        // Select visible term RingBuffers
        const termBufs = [];
        const termColors = [];
        if (this.visible.fieldKinetic)  { termBufs.push(this.lag.fieldKinetic);  termColors.push(TERM_COLORS.fieldKinetic); }
        if (this.visible.fieldGradient) { termBufs.push(this.lag.fieldGradient); termColors.push(TERM_COLORS.fieldGradient); }
        if (this.visible.bornInfeld)    { termBufs.push(this.lag.bornInfeld);    termColors.push(TERM_COLORS.bornInfeld); }
        if (this.visible.coupling)      { termBufs.push(this.lag.coupling);      termColors.push(TERM_COLORS.coupling); }
        if (this.visible.velocity)      { termBufs.push(this.lag.velocity);      termColors.push(TERM_COLORS.velocity); }
        if (this.visible.gauss)         { termBufs.push(this.lag.gauss);         termColors.push(TERM_COLORS.gauss); }
        if (this.visible.dissipation)   { termBufs.push(this.lag.dissipation);   termColors.push(TERM_COLORS.dissipation); }

        if (termBufs.length === 0) return;

        // Compute cumulative stacks using RingBuffer.get(i)
        const stacks = [];
        for (let i = 0; i < n; i++) {
            let sum = 0;
            const row = [0];
            for (const rb of termBufs) {
                sum += rb.get(i);
                row.push(sum);
            }
            stacks.push(row);
        }

        // Y range
        let yMax = 0;
        for (const row of stacks) {
            const top = row[row.length - 1];
            if (top > yMax) yMax = top;
        }
        yMax = yMax || 1;
        yMax *= 1.1;

        // Grid
        ctx.strokeStyle = '#2a3a5a';
        ctx.lineWidth = 0.5;
        for (let i = 0; i <= 4; i++) {
            const y = margin.top + (plotH * i) / 4;
            ctx.beginPath();
            ctx.moveTo(margin.left, y);
            ctx.lineTo(w - margin.right, y);
            ctx.stroke();

            const val = yMax * (1 - i / 4);
            ctx.fillStyle = '#6b7280';
            ctx.font = '10px JetBrains Mono, monospace';
            ctx.textAlign = 'right';
            ctx.fillText(fmtVal(val), margin.left - 6, y + 4);
        }

        // Draw stacked areas (bottom to top)
        for (let t = termBufs.length - 1; t >= 0; t--) {
            ctx.beginPath();
            ctx.moveTo(margin.left, margin.top + plotH); // bottom-left

            // Top edge
            for (let i = 0; i < n; i++) {
                const x = margin.left + (i / (n - 1)) * plotW;
                const y = margin.top + plotH - (stacks[i][t + 1] / yMax) * plotH;
                ctx.lineTo(x, y);
            }

            // Bottom edge (reverse)
            for (let i = n - 1; i >= 0; i--) {
                const x = margin.left + (i / (n - 1)) * plotW;
                const y = margin.top + plotH - (stacks[i][t] / yMax) * plotH;
                ctx.lineTo(x, y);
            }

            ctx.closePath();
            ctx.fillStyle = termColors[t] + '60';
            ctx.fill();
            ctx.strokeStyle = termColors[t];
            ctx.lineWidth = 1;
            ctx.stroke();
        }

        // X-axis label
        ctx.fillStyle = '#6b7280';
        ctx.font = '10px Inter, sans-serif';
        ctx.textAlign = 'center';
        ctx.fillText(`${n} ticks`, w / 2, h - 2);
    }

    clear() {
        if (this._ownedBuffers) {
            for (const b of Object.values(this.lag)) b.clear();
        }
        // When hub-owned, hub.resetScale(0) clears the buffers instead.
    }
}

function fmtVal(v) {
    if (Math.abs(v) >= 10000) return v.toExponential(1);
    if (Math.abs(v) >= 100) return v.toFixed(0);
    if (Math.abs(v) >= 1) return v.toFixed(1);
    return v.toFixed(3);
}
