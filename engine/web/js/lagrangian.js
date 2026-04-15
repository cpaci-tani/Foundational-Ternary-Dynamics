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

class TermBuffer {
    constructor(size = BUFFER_SIZE) {
        this.size = size;
        this.fk          = new Float32Array(size);  // field kinetic
        this.fg          = new Float32Array(size);  // field gradient
        this.bi          = new Float32Array(size);
        this.coup        = new Float32Array(size);
        this.vel         = new Float32Array(size);
        this.gauss       = new Float32Array(size);
        this.diss        = new Float32Array(size);
        this.total       = new Float32Array(size);
        this.hamiltonian = new Float32Array(size);
        this.action      = new Float32Array(size);
        this._head = 0;
        this._count = 0;
    }

    push(lag) {
        const h = this._head;
        this.fk[h]          = Math.abs(lag.fieldKinetic || 0);
        this.fg[h]          = Math.abs(lag.fieldGradient || 0);
        this.bi[h]          = Math.abs(lag.bornInfeld || 0);
        this.coup[h]        = Math.abs(lag.coupling || 0);
        this.vel[h]         = Math.abs(lag.velocity || 0);
        this.gauss[h]       = Math.abs(lag.gauss || 0);
        this.diss[h]        = Math.abs(lag.dissipation || 0);
        this.total[h]       = lag.total || 0;
        this.hamiltonian[h] = lag.hamiltonian || 0;
        this.action[h]      = lag.totalAction || 0;
        this._head = (h + 1) % this.size;
        if (this._count < this.size) this._count++;
    }

    get(arr, i) {
        return arr[(this._head - this._count + i + this.size) % this.size];
    }

    get length() { return this._count; }

    clear() {
        this._head = 0;
        this._count = 0;
    }
}

export class LagrangianChart {
    constructor(canvas) {
        this.canvas = canvas;
        this.buffer = new TermBuffer();
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

    push(lag) {
        this.buffer.push(lag);
        // Update constraint display
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

        const buf = this.buffer;
        const n = buf.length;
        if (n < 2) return;

        const margin = { top: 8, right: 8, bottom: 20, left: 50 };
        const plotW = w - margin.left - margin.right;
        const plotH = h - margin.top - margin.bottom;

        // Compute stacked values
        const stacks = [];
        const termArrays = [];
        const termColors = [];

        if (this.visible.fieldKinetic)  { termArrays.push(buf.fk);    termColors.push(TERM_COLORS.fieldKinetic); }
        if (this.visible.fieldGradient) { termArrays.push(buf.fg);    termColors.push(TERM_COLORS.fieldGradient); }
        if (this.visible.bornInfeld)  { termArrays.push(buf.bi);    termColors.push(TERM_COLORS.bornInfeld); }
        if (this.visible.coupling)    { termArrays.push(buf.coup);  termColors.push(TERM_COLORS.coupling); }
        if (this.visible.velocity)    { termArrays.push(buf.vel);   termColors.push(TERM_COLORS.velocity); }
        if (this.visible.gauss)       { termArrays.push(buf.gauss); termColors.push(TERM_COLORS.gauss); }
        if (this.visible.dissipation) { termArrays.push(buf.diss);  termColors.push(TERM_COLORS.dissipation); }

        if (termArrays.length === 0) return;

        // Compute cumulative stacks
        for (let i = 0; i < n; i++) {
            let sum = 0;
            const row = [0];
            for (const arr of termArrays) {
                sum += buf.get(arr, i);
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
        for (let t = termArrays.length - 1; t >= 0; t--) {
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
        this.buffer.clear();
    }
}

function fmtVal(v) {
    if (Math.abs(v) >= 10000) return v.toExponential(1);
    if (Math.abs(v) >= 100) return v.toFixed(0);
    if (Math.abs(v) >= 1) return v.toFixed(1);
    return v.toFixed(3);
}
