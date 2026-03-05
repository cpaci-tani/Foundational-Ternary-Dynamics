/**
 * Lagrangian Panel — stacked area chart of 5 Lagrangian density terms.
 *
 * Terms: Born-Infeld (red), Coupling (orange), Velocity (gold),
 *        Gauss (blue), Dissipation (gray).
 *
 * Also displays Hamiltonian and total Lagrangian as overlaid lines.
 */

const BUFFER_SIZE = 400;
const TERM_COLORS = {
    bornInfeld:  '#ef5350',
    coupling:    '#fb8c00',
    velocity:    '#fdd835',
    gauss:       '#42a5f5',
    dissipation: '#78909c',
};

class TermBuffer {
    constructor(size = BUFFER_SIZE) {
        this.size = size;
        this.bi          = new Float32Array(size);
        this.coup        = new Float32Array(size);
        this.vel         = new Float32Array(size);
        this.gauss       = new Float32Array(size);
        this.diss        = new Float32Array(size);
        this.total       = new Float32Array(size);
        this.hamiltonian = new Float32Array(size);
        this._head = 0;
        this._count = 0;
    }

    push(lag) {
        const h = this._head;
        this.bi[h]          = Math.abs(lag.bornInfeld || 0);
        this.coup[h]        = Math.abs(lag.coupling || 0);
        this.vel[h]         = Math.abs(lag.velocity || 0);
        this.gauss[h]       = Math.abs(lag.gauss || 0);
        this.diss[h]        = Math.abs(lag.dissipation || 0);
        this.total[h]       = lag.total || 0;
        this.hamiltonian[h] = lag.hamiltonian || 0;
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
        this.visible = {
            bornInfeld: true,
            coupling: true,
            velocity: true,
            gauss: true,
            dissipation: true,
        };

        // Wire up term toggle checkboxes
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
        const set = (id, val) => {
            const el = document.getElementById(id);
            if (el) el.textContent = typeof val === 'number' ? fmtVal(val) : val;
        };
        set('lag-gauss-viol', lag.gaussViolation);
        set('lag-max-gauss', lag.maxGaussError);
        set('lag-flux-mag', lag.totalFluxMag);
        set('lag-wave-ke', lag.totalWaveEnergy);
        set('lag-manifested', lag.manifested);
        set('lag-locked', lag.locked);
    }

    draw() {
        const canvas = this.canvas;
        const ctx = canvas.getContext('2d');
        const dpr = window.devicePixelRatio || 1;
        const rect = canvas.getBoundingClientRect();
        const w = rect.width;
        const h = rect.height;

        if (w === 0 || h === 0) return;

        if (canvas.width !== w * dpr || canvas.height !== h * dpr) {
            canvas.width = w * dpr;
            canvas.height = h * dpr;
            ctx.scale(dpr, dpr);
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
