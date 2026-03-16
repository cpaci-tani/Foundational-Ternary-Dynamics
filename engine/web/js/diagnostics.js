/**
 * Diagnostics Panel — live number displays with sparkline mini-charts.
 *
 * Updates DOM stat elements and draws tiny sparklines on small canvases.
 * Now includes spin/color statistics and energy audit data.
 * All values display with proper physical unit labels via units.js.
 */

import { formatEnergy, formatEntropy } from './units.js';

const SPARKLINE_LEN = 80;

export class Sparkline {
    constructor(canvas) {
        this.canvas = canvas;
        this._buf = new Float32Array(SPARKLINE_LEN);
        this._head = 0;
        this._count = 0;
    }

    push(value) {
        this._buf[this._head] = value;
        this._head = (this._head + 1) % SPARKLINE_LEN;
        if (this._count < SPARKLINE_LEN) this._count++;
    }

    _get(i) {
        return this._buf[(this._head - this._count + i + SPARKLINE_LEN) % SPARKLINE_LEN];
    }

    draw(color = '#60a5fa') {
        const canvas = this.canvas;
        if (!canvas) return;
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

        const n = this._count;
        if (n < 2) return;

        let min = Infinity, max = -Infinity;
        for (let i = 0; i < n; i++) {
            const v = this._get(i);
            if (v < min) min = v;
            if (v > max) max = v;
        }
        const range = max - min || 1;

        // Fill under curve
        ctx.beginPath();
        ctx.moveTo(0, h);
        for (let i = 0; i < n; i++) {
            const x = (i / (n - 1)) * w;
            const y = h - ((this._get(i) - min) / range) * (h - 2) - 1;
            ctx.lineTo(x, y);
        }
        ctx.lineTo(w, h);
        ctx.closePath();
        ctx.fillStyle = color + '15';
        ctx.fill();

        // Stroke line
        ctx.beginPath();
        for (let i = 0; i < n; i++) {
            const x = (i / (n - 1)) * w;
            const y = h - ((this._get(i) - min) / range) * (h - 2) - 1;
            if (i === 0) ctx.moveTo(x, y);
            else ctx.lineTo(x, y);
        }
        ctx.strokeStyle = color;
        ctx.lineWidth = 1.2;
        ctx.stroke();
    }

    clear() {
        this._head = 0;
        this._count = 0;
    }
}

export class DiagnosticsPanel {
    constructor() {
        // DOM elements
        this.els = {
            manifested: document.getElementById('diag-manifested'),
            positive:   document.getElementById('diag-positive'),
            negative:   document.getElementById('diag-negative'),
            flux:       document.getElementById('diag-flux'),
            energy:     document.getElementById('diag-energy'),
            entropy:    document.getElementById('diag-entropy'),
            charge:     document.getElementById('diag-charge'),
            spinUp:     document.getElementById('diag-spin-up'),
            spinDown:   document.getElementById('diag-spin-down'),
            colorR:     document.getElementById('diag-color-r'),
            colorG:     document.getElementById('diag-color-g'),
            colorB:     document.getElementById('diag-color-b'),
            // Energy audit
            fieldEnergy:    document.getElementById('diag-field-energy'),
            waveEnergy:     document.getElementById('diag-wave-energy'),
            coulombPE:      document.getElementById('diag-coulomb-pe'),
            gaussViolation: document.getElementById('diag-gauss-violation'),
        };

        // Sparklines
        this.sparklines = {
            manifested: new Sparkline(document.getElementById('spark-manifested')),
            charges:    new Sparkline(document.getElementById('spark-charges')),
            flux:       new Sparkline(document.getElementById('spark-flux')),
            energy:     new Sparkline(document.getElementById('spark-energy')),
            entropy:    new Sparkline(document.getElementById('spark-entropy')),
        };
    }

    update(diag) {
        // Update number displays
        this.els.manifested.textContent = diag.manifested;
        this.els.positive.textContent = diag.positive;
        this.els.negative.textContent = diag.negative;
        this.els.flux.textContent = formatEnergy(diag.totalFlux, 0).text;
        this.els.energy.textContent = formatEnergy(diag.totalEnergy, 0).text;
        this.els.entropy.textContent = formatEntropy(diag.entropy || 0, 0).text;
        this.els.charge.textContent = diag.chargeBalance || 0;

        // Spin/color
        this.els.spinUp.textContent = diag.spinUp || 0;
        this.els.spinDown.textContent = diag.spinDown || 0;
        this.els.colorR.textContent = diag.colorRed || 0;
        this.els.colorG.textContent = diag.colorGreen || 0;
        this.els.colorB.textContent = diag.colorBlue || 0;

        // Push to sparklines
        this.sparklines.manifested.push(diag.manifested);
        this.sparklines.charges.push(diag.positive - diag.negative);
        this.sparklines.flux.push(diag.totalFlux);
        this.sparklines.energy.push(diag.totalEnergy);
        this.sparklines.entropy.push(diag.entropy || 0);
    }

    updateEnergyAudit(ea) {
        if (!ea) return;
        this.els.fieldEnergy.textContent = formatEnergy(ea.fieldEnergy, 0).text;
        this.els.waveEnergy.textContent = formatEnergy(ea.waveEnergy, 0).text;
        this.els.coulombPE.textContent = formatEnergy(ea.coulombPE, 0).text;
        this.els.gaussViolation.textContent = fmtSci(ea.gaussViolation) + ' Pl';
    }

    drawSparklines() {
        this.sparklines.manifested.draw('#60a5fa');
        this.sparklines.charges.draw('#4ade80');
        this.sparklines.flux.draw('#fb8c00');
        this.sparklines.energy.draw('#42a5f5');
        this.sparklines.entropy.draw('#a78bfa');
    }

    clear() {
        for (const s of Object.values(this.sparklines)) s.clear();
    }
}

function fmt(v) {
    if (typeof v !== 'number' || isNaN(v)) return '0.0000';
    if (Math.abs(v) >= 10000) return v.toExponential(2);
    return v.toFixed(4);
}

function fmtSci(v) {
    if (typeof v !== 'number' || isNaN(v)) return '0';
    if (v === 0) return '0';
    return v.toExponential(3);
}
