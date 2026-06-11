/**
 * PE Telemetry Panel — Scientist-grade physics telemetry for Scale 1.
 *
 * Five sections:
 *   1. Conservation Laws (energy, momentum, angular momentum) with sparklines + alarms
 *   2. System Properties (virial, temperature, RMS velocity, CoM, radius)
 *   3. Per-Particle Table (ID, q, m, |r|, |v|, |a|, |F|, KE, locked)
 *   4. Orbital Mechanics (2-body only: Kepler orbit parameters + phase space)
 *   5. Time-Series Charts (energy, |p|, |L|, virial — multi-trace sparklines)
 */

import { Sparkline } from './diagnostics.js';
import { ALPHA, G_N, COULOMB_K_FORCE } from './constants.js';
import { formatEnergy, formatVelocity, formatLength, formatForce, formatTemperature } from './units.js';
import { createCachedCanvasRect } from './dom-utils.js';
import { resolveChartColor } from './ui/charts/theme.js';

const TS_LEN = 200;  // Time-series buffer length (longer than sparkline 80)

// ── Multi-trace time-series chart ────────────────────────────────────
class TimeSeriesChart {
    constructor(canvas, traces) {
        this.canvas = canvas;
        this.traces = traces; // [{color, buf: Float32Array, head, count}]
        for (const t of this.traces) {
            t.buf = new Float32Array(TS_LEN);
            t.head = 0;
            t.count = 0;
        }
        this._refLine = null; // optional horizontal reference line
        // Phase C.3: cache rect dimensions; refreshed by ResizeObserver
        this._rectCache = canvas ? createCachedCanvasRect(canvas) : null;
    }

    setRefLine(val) { this._refLine = val; }

    push(values) {
        for (let i = 0; i < this.traces.length; i++) {
            const t = this.traces[i];
            t.buf[t.head] = values[i];
            t.head = (t.head + 1) % TS_LEN;
            if (t.count < TS_LEN) t.count++;
        }
    }

    _get(trace, i) {
        return trace.buf[(trace.head - trace.count + i + TS_LEN) % TS_LEN];
    }

    draw() {
        const canvas = this.canvas;
        if (!canvas) return;
        const ctx = canvas.getContext('2d');
        const dpr = window.devicePixelRatio || 1;
        const rect = this._rectCache ? this._rectCache.get() : canvas.getBoundingClientRect();
        const w = rect.width, h = rect.height;
        if (w === 0 || h === 0) return;

        if (canvas.width !== w * dpr || canvas.height !== h * dpr) {
            canvas.width = w * dpr;
            canvas.height = h * dpr;
            ctx.setTransform(dpr, 0, 0, dpr, 0, 0);
        }
        ctx.clearRect(0, 0, w, h);

        let totalCount = 0;
        for (const t of this.traces) totalCount += t.count;
        if (totalCount === 0) return;

        // Find global min/max across all traces
        let min = Infinity, max = -Infinity;
        for (const t of this.traces) {
            for (let i = 0; i < t.count; i++) {
                const v = this._get(t, i);
                if (isFinite(v)) {
                    if (v < min) min = v;
                    if (v > max) max = v;
                }
            }
        }
        if (this._refLine !== null) {
            if (this._refLine < min) min = this._refLine;
            if (this._refLine > max) max = this._refLine;
        }
        const range = max - min || 1;

        // Reference line
        if (this._refLine !== null) {
            const ry = h - ((this._refLine - min) / range) * (h - 4) - 2;
            ctx.strokeStyle = '#4b556380';
            ctx.lineWidth = 1;
            ctx.setLineDash([4, 3]);
            ctx.beginPath();
            ctx.moveTo(0, ry);
            ctx.lineTo(w, ry);
            ctx.stroke();
            ctx.setLineDash([]);
        }

        // Draw each trace
        for (const t of this.traces) {
            if (t.count < 2) continue;
            ctx.beginPath();
            for (let i = 0; i < t.count; i++) {
                const x = (i / (t.count - 1)) * w;
                const v = this._get(t, i);
                const y = h - ((v - min) / range) * (h - 4) - 2;
                if (i === 0) ctx.moveTo(x, y); else ctx.lineTo(x, y);
            }
            ctx.strokeStyle = t.color;
            ctx.lineWidth = 1.2;
            ctx.stroke();
        }
    }

    clear() {
        for (const t of this.traces) { t.head = 0; t.count = 0; }
        this.draw();
    }
}

// ── Phase-space ring buffer ──────────────────────────────────────────
// Fixed-capacity FIFO of {r, vr} samples. Mirrors the head/count modular
// indexing of telemetry-hub.js RingBuffer (O(1) append, O(1) eviction by
// overwrite) but stores paired objects instead of Float32 scalars, since
// the phase-space plot needs both coordinates per sample. Logical order is
// identical to the previous Array push()/shift() FIFO: index 0 is the
// oldest retained sample, index (length-1) the newest.
const PHASE_BUF_CAPACITY = 300;

class PhaseRingBuffer {
    constructor(capacity = PHASE_BUF_CAPACITY) {
        this.capacity = capacity;
        this.data = new Array(capacity);
        this.head = 0;   // next write slot
        this.count = 0;  // number of valid samples (≤ capacity)
    }

    /** Append one sample; overwrites the oldest in place once full. O(1). */
    push(r, vr) {
        this.data[this.head] = { r, vr };
        this.head = (this.head + 1) % this.capacity;
        if (this.count < this.capacity) this.count++;
    }

    /** Sample at logical index i (0 = oldest, count-1 = newest). */
    get(i) {
        return this.data[(this.head - this.count + i + this.capacity) % this.capacity];
    }

    /** Number of valid samples currently held. */
    get length() { return this.count; }

    clear() { this.head = 0; this.count = 0; }
}

// ── Formatting helpers ───────────────────────────────────────────────
function fmt(v, digits = 6) {
    if (typeof v !== 'number' || !isFinite(v)) return '—';
    if (Math.abs(v) >= 1e6 || (Math.abs(v) < 1e-4 && v !== 0)) return v.toExponential(3);
    return v.toFixed(digits);
}

function fmtShort(v) {
    if (typeof v !== 'number' || !isFinite(v)) return '—';
    if (Math.abs(v) >= 1e4) return v.toExponential(2);
    if (Math.abs(v) < 1e-3 && v !== 0) return v.toExponential(2);
    return v.toFixed(4);
}

// ── Main Panel Class ─────────────────────────────────────────────────
export class PETelemetryPanel {
    constructor() {
        // Section 1: Conservation
        this._els = {
            energy: document.getElementById('pet-energy'),
            momentum: document.getElementById('pet-momentum'),
            angmom: document.getElementById('pet-angmom'),
            drift: document.getElementById('pet-drift'),
            energyAlarm: document.getElementById('pet-energy-alarm'),
            momentumAlarm: document.getElementById('pet-momentum-alarm'),
            angmomAlarm: document.getElementById('pet-angmom-alarm'),
            driftAlarm: document.getElementById('pet-drift-alarm'),
            // Section 2: System
            count: document.getElementById('pet-count'),
            virial: document.getElementById('pet-virial'),
            temp: document.getElementById('pet-temp'),
            vrms: document.getElementById('pet-vrms'),
            radius: document.getElementById('pet-radius'),
            tick: document.getElementById('pet-tick'),
            ke: document.getElementById('pet-ke'),
            pe: document.getElementById('pet-pe'),
            com: document.getElementById('pet-com'),
            // Decomposed PE
            peCoulomb: document.getElementById('pet-pe-coulomb'),
            peGravity: document.getElementById('pet-pe-gravity'),
            // Section 4: Orbital
            orbSection: document.getElementById('pet-orbital-section'),
            orbR: document.getElementById('pet-orb-r'),
            orbMu: document.getElementById('pet-orb-mu'),
            orbH: document.getElementById('pet-orb-h'),
            orbA: document.getElementById('pet-orb-a'),
            orbE: document.getElementById('pet-orb-e'),
            orbT: document.getElementById('pet-orb-T'),
            orbVisviva: document.getElementById('pet-orb-visviva'),
        };

        // Conservation sparklines
        this._sparks = {
            energy: new Sparkline(document.getElementById('pet-spark-energy')),
            momentum: new Sparkline(document.getElementById('pet-spark-momentum')),
            angmom: new Sparkline(document.getElementById('pet-spark-angmom')),
            drift: new Sparkline(document.getElementById('pet-spark-drift')),
        };

        // Time-series charts
        this._tsEnergy = new TimeSeriesChart(document.getElementById('pet-ts-energy'), [
            { color: resolveChartColor('var(--chart-pe-ke, #4ade80)') },
            { color: resolveChartColor('var(--chart-pe-coulomb, #f87171)') },
            { color: resolveChartColor('var(--chart-pe-total, #e8e8e8)') },
        ]);
        this._tsMomentum = new TimeSeriesChart(document.getElementById('pet-ts-momentum'), [
            { color: resolveChartColor('var(--chart-pe-momentum, #a78bfa)') },
        ]);
        this._tsAngmom = new TimeSeriesChart(document.getElementById('pet-ts-angmom'), [
            { color: resolveChartColor('var(--chart-pe-angmom, #60a5fa)') },
        ]);
        this._tsVirial = new TimeSeriesChart(document.getElementById('pet-ts-virial'), [
            { color: resolveChartColor('var(--chart-pe-virial, #fbbf24)') },
        ]);
        this._tsVirial.setRefLine(1.0);

        // Phase space chart
        this._phaseCanvas = document.getElementById('pet-phase-space');
        this._phaseBuf = new PhaseRingBuffer(PHASE_BUF_CAPACITY); // {r, vr} ring buffer
        // Phase C.3: cache rect dims, refreshed via ResizeObserver
        this._phaseRectCache = this._phaseCanvas ? createCachedCanvasRect(this._phaseCanvas) : null;

        // Particle table body
        this._tbody = document.getElementById('pet-particle-tbody');

        // Tracking for drift + alarms
        this._initialEnergy = null;
        this._initialMomentum = null;
        this._initialAngmom = null;
    }

    /**
     * Main update entry point — called every ~3 frames from app.js
     * @param {object} diag - from bridge.peGetDiagnostics()
     * @param {object|null} ext - from bridge.peGetExtendedData()
     */
    update(diag, ext) {
        this._updateConservation(diag);
        this._updateSystemProps(diag, ext);
        if (ext) this._updateParticleTable(ext);
        this._updateOrbitalMechanics(ext);
        this._updateTimeSeries(diag);
    }

    // ── Section 1: Conservation Laws ─────────────────────────────────
    _updateConservation(diag) {
        const E = diag.totalEnergy;
        const pMag = Math.sqrt(diag.momentumX ** 2 + diag.momentumY ** 2 + diag.momentumZ ** 2);
        const lMag = Math.sqrt(diag.angMomX ** 2 + diag.angMomY ** 2 + diag.angMomZ ** 2);

        this._els.energy.textContent = formatEnergy(E, 1).text;
        this._els.momentum.textContent = fmt(pMag) + ' MeV/c';
        this._els.angmom.textContent = fmt(lMag) + ' \u0127';

        // Track initial values for drift/alarm
        if (this._initialEnergy === null && E !== 0) this._initialEnergy = E;
        if (this._initialMomentum === null && pMag > 0) this._initialMomentum = pMag;
        if (this._initialAngmom === null && lMag > 0) this._initialAngmom = lMag;

        // Energy drift
        let driftPct = 0;
        if (this._initialEnergy !== null && this._initialEnergy !== 0) {
            driftPct = ((E - this._initialEnergy) / Math.abs(this._initialEnergy)) * 100;
        }
        this._els.drift.textContent = driftPct.toFixed(4) + '%';

        // Alarms
        const absDrift = Math.abs(driftPct);
        this._setAlarm(this._els.driftAlarm, absDrift < 0.1 ? 'green' : absDrift < 1.0 ? 'yellow' : 'red');
        this._setAlarm(this._els.energyAlarm, absDrift < 0.1 ? 'green' : absDrift < 1.0 ? 'yellow' : 'red');
        this._setAlarm(this._els.momentumAlarm, pMag < 1e-6 ? 'green' : pMag < 1e-3 ? 'yellow' : 'red');
        this._setAlarm(this._els.angmomAlarm, 'green'); // Angular momentum — just show green for now

        // Push sparklines
        this._sparks.energy.push(E);
        this._sparks.momentum.push(pMag);
        this._sparks.angmom.push(lMag);
        this._sparks.drift.push(driftPct);
    }

    _setAlarm(el, level) {
        if (!el) return;
        el.className = 'pe-cons-alarm';
        if (level) el.classList.add('alarm-' + level);
    }

    // ── Section 2: System Properties ─────────────────────────────────
    _updateSystemProps(diag, ext) {
        const N = diag.particleCount;
        this._els.count.textContent = N;
        this._els.tick.textContent = diag.tick;
        this._els.ke.textContent = formatEnergy(diag.totalKE, 1).text;
        this._els.pe.textContent = formatEnergy(diag.totalPE, 1).text;
        if (this._els.peCoulomb) this._els.peCoulomb.textContent = formatEnergy(diag.coulombPE || 0, 1).text;
        if (this._els.peGravity) this._els.peGravity.textContent = formatEnergy(diag.gravityPE || 0, 1).text;

        // Virial ratio
        const virial = diag.totalPE !== 0 ? (2 * diag.totalKE / Math.abs(diag.totalPE)) : NaN;
        this._els.virial.textContent = fmtShort(virial);
        if (this._els.virial && isFinite(virial)) {
            this._els.virial.style.color = virial < 0.8 ? '#f87171' : virial > 1.2 ? '#fbbf24' : '#4ade80';
        }

        if (!ext || ext.count === 0) {
            this._els.temp.textContent = '—';
            this._els.vrms.textContent = '—';
            this._els.com.textContent = '—';
            this._els.radius.textContent = '—';
            return;
        }

        // Temperature: (2/3) KE / N (equipartition, 3 translational DoF)
        const temp = N > 0 ? (2 / 3) * diag.totalKE / N : 0;
        this._els.temp.textContent = formatTemperature(temp, 1).text;

        // RMS velocity
        let v2sum = 0;
        for (let i = 0; i < ext.count; i++) {
            const vx = ext.velocities[i * 3], vy = ext.velocities[i * 3 + 1], vz = ext.velocities[i * 3 + 2];
            v2sum += vx * vx + vy * vy + vz * vz;
        }
        this._els.vrms.textContent = formatVelocity(Math.sqrt(v2sum / ext.count), 1).text;

        // Center of mass
        let cmx = 0, cmy = 0, cmz = 0, totalMass = 0;
        for (let i = 0; i < ext.count; i++) {
            const m = ext.masses[i];
            cmx += m * ext.positions[i * 3];
            cmy += m * ext.positions[i * 3 + 1];
            cmz += m * ext.positions[i * 3 + 2];
            totalMass += m;
        }
        if (totalMass > 0) { cmx /= totalMass; cmy /= totalMass; cmz /= totalMass; }
        this._els.com.textContent = `(${cmx.toFixed(1)}, ${cmy.toFixed(1)}, ${cmz.toFixed(1)}) lu`;

        // System radius: max distance from CoM
        let maxR = 0;
        for (let i = 0; i < ext.count; i++) {
            const dx = ext.positions[i * 3] - cmx;
            const dy = ext.positions[i * 3 + 1] - cmy;
            const dz = ext.positions[i * 3 + 2] - cmz;
            const r = Math.sqrt(dx * dx + dy * dy + dz * dz);
            if (r > maxR) maxR = r;
        }
        this._els.radius.textContent = formatLength(maxR, 1).text;
    }

    // ── Section 3: Per-Particle Table ────────────────────────────────
    _updateParticleTable(ext) {
        if (!this._tbody) return;
        const N = Math.min(ext.count, 20); // Cap at 20 rows

        // Build rows — reuse existing DOM if count matches
        while (this._tbody.rows.length > N) this._tbody.deleteRow(-1);
        while (this._tbody.rows.length < N) {
            const row = this._tbody.insertRow();
            for (let c = 0; c < 9; c++) row.insertCell();
        }

        for (let i = 0; i < N; i++) {
            const row = this._tbody.rows[i];
            const cells = row.cells;
            const q = ext.charges[i];
            const m = ext.masses[i];
            const px = ext.positions[i * 3], py = ext.positions[i * 3 + 1], pz = ext.positions[i * 3 + 2];
            const vx = ext.velocities[i * 3], vy = ext.velocities[i * 3 + 1], vz = ext.velocities[i * 3 + 2];
            const ax = ext.accelerations[i * 3], ay = ext.accelerations[i * 3 + 1], az = ext.accelerations[i * 3 + 2];
            const fx = ext.forces[i * 3], fy = ext.forces[i * 3 + 1], fz = ext.forces[i * 3 + 2];
            const rMag = Math.sqrt(px * px + py * py + pz * pz);
            const vMag = Math.sqrt(vx * vx + vy * vy + vz * vz);
            const aMag = Math.sqrt(ax * ax + ay * ay + az * az);
            const fMag = Math.sqrt(fx * fx + fy * fy + fz * fz);
            const ke = 0.5 * m * vMag * vMag;

            cells[0].textContent = ext.ids[i];
            cells[1].textContent = q > 0 ? '+1' : q < 0 ? '-1' : '0';
            cells[1].style.color = q > 0 ? '#4ade80' : q < 0 ? '#f87171' : '#9ca3af';
            cells[2].textContent = fmtShort(m);
            cells[3].textContent = fmtShort(rMag);
            cells[4].textContent = fmtShort(vMag);
            cells[5].textContent = fmtShort(aMag);
            cells[6].textContent = fmtShort(fMag);
            cells[7].textContent = fmtShort(ke);
            cells[8].textContent = ext.locked[i] ? '✓' : '✗';
            cells[8].style.color = ext.locked[i] ? '#fbbf24' : '#6b7280';
        }
    }

    // ── Section 4: Orbital Mechanics (2-body) ────────────────────────
    _updateOrbitalMechanics(ext) {
        if (!ext || ext.count !== 2) {
            if (this._els.orbSection) this._els.orbSection.style.display = 'none';
            return;
        }
        if (this._els.orbSection) this._els.orbSection.style.display = '';

        const m1 = ext.masses[0], m2 = ext.masses[1];
        const r1 = [ext.positions[0], ext.positions[1], ext.positions[2]];
        const r2 = [ext.positions[3], ext.positions[4], ext.positions[5]];
        const v1 = [ext.velocities[0], ext.velocities[1], ext.velocities[2]];
        const v2 = [ext.velocities[3], ext.velocities[4], ext.velocities[5]];

        // Relative position and velocity
        const dr = [r2[0] - r1[0], r2[1] - r1[1], r2[2] - r1[2]];
        const dv = [v2[0] - v1[0], v2[1] - v1[1], v2[2] - v1[2]];
        const r = Math.sqrt(dr[0] ** 2 + dr[1] ** 2 + dr[2] ** 2);
        const v = Math.sqrt(dv[0] ** 2 + dv[1] ** 2 + dv[2] ** 2);

        // Reduced mass
        const mu = m1 * m2 / (m1 + m2);

        // Effective coupling: k = COULOMB_K_FORCE·q1·q2 + G_N·m1·m2 (with signs)
        const q1 = ext.charges[0], q2 = ext.charges[1];
        const k_em = -COULOMB_K_FORCE * q1 * q2; // attractive when opposite
        const k_grav = G_N * m1 * m2;                   // always attractive
        const k = k_em + k_grav; // net coupling constant (positive = attractive)

        // Specific angular momentum: h = |r × v|
        const hx = dr[1] * dv[2] - dr[2] * dv[1];
        const hy = dr[2] * dv[0] - dr[0] * dv[2];
        const hz = dr[0] * dv[1] - dr[1] * dv[0];
        const h = Math.sqrt(hx ** 2 + hy ** 2 + hz ** 2);

        // Orbital energy (per reduced mass)
        const E_orb = 0.5 * mu * v * v - k / r;

        // Semi-major axis: a = -k/(2E) for bound orbits (E < 0)
        const a = E_orb < 0 ? -k / (2 * E_orb) : NaN;

        // Eccentricity
        const ecc_arg = 1 + 2 * E_orb * h * h / (mu * k * k);
        const ecc = ecc_arg >= 0 ? Math.sqrt(ecc_arg) : NaN;

        // Period: T = 2π·a^(3/2) / √(k/μ)
        const T = (isFinite(a) && a > 0 && k > 0) ? 2 * Math.PI * Math.pow(a, 1.5) / Math.sqrt(k / mu) : NaN;

        // Vis-viva check: v² should equal k/μ·(2/r - 1/a)
        const v2_actual = v * v;
        const v2_visviva = (isFinite(a) && k > 0) ? (k / mu) * (2 / r - 1 / a) : NaN;

        this._els.orbR.textContent = formatLength(r, 1).text;
        this._els.orbMu.textContent = formatEnergy(mu, 1).text;
        this._els.orbH.textContent = fmtShort(h) + ' lu\u00b2/tick';
        this._els.orbA.textContent = formatLength(a, 1).text;
        this._els.orbE.textContent = fmtShort(ecc);
        this._els.orbT.textContent = fmtShort(T) + ' ticks';

        if (isFinite(v2_actual) && isFinite(v2_visviva)) {
            const err = Math.abs(v2_actual - v2_visviva) / (Math.abs(v2_visviva) || 1) * 100;
            this._els.orbVisviva.textContent = `v²=${fmtShort(v2_actual)} vs ${fmtShort(v2_visviva)} (${err.toFixed(1)}%)`;
            this._els.orbVisviva.style.color = err < 5 ? '#4ade80' : err < 20 ? '#fbbf24' : '#f87171';
        } else {
            this._els.orbVisviva.textContent = '—';
            this._els.orbVisviva.style.color = '';
        }

        // Phase space: radial velocity v_r = (dr · dv) / |dr|
        const vr = r > 0 ? (dr[0] * dv[0] + dr[1] * dv[1] + dr[2] * dv[2]) / r : 0;
        this._phaseBuf.push(r, vr);  // ring buffer evicts oldest in place once full
        this._drawPhaseSpace();
    }

    _drawPhaseSpace() {
        const canvas = this._phaseCanvas;
        if (!canvas) return;
        const ctx = canvas.getContext('2d');
        const dpr = window.devicePixelRatio || 1;
        const rect = this._phaseRectCache ? this._phaseRectCache.get() : canvas.getBoundingClientRect();
        const w = rect.width, h = rect.height;
        if (w === 0 || h === 0) return;

        if (canvas.width !== w * dpr || canvas.height !== h * dpr) {
            canvas.width = w * dpr;
            canvas.height = h * dpr;
            ctx.setTransform(dpr, 0, 0, dpr, 0, 0);
        }
        ctx.clearRect(0, 0, w, h);

        if (this._phaseBuf.length < 2) return;

        const buf = this._phaseBuf;
        const n = buf.length;
        let rMin = Infinity, rMax = -Infinity, vrMin = Infinity, vrMax = -Infinity;
        for (let i = 0; i < n; i++) {
            const p = buf.get(i);
            if (p.r < rMin) rMin = p.r;
            if (p.r > rMax) rMax = p.r;
            if (p.vr < vrMin) vrMin = p.vr;
            if (p.vr > vrMax) vrMax = p.vr;
        }
        const rRange = rMax - rMin || 1;
        const vrRange = vrMax - vrMin || 1;

        // Axes
        ctx.strokeStyle = '#374151';
        ctx.lineWidth = 0.5;
        const zeroY = h - ((-vrMin) / vrRange) * (h - 8) - 4;
        ctx.beginPath(); ctx.moveTo(0, zeroY); ctx.lineTo(w, zeroY); ctx.stroke();

        // Points (fade older ones)
        for (let i = 0; i < n; i++) {
            const p = buf.get(i);
            const alpha = 0.15 + 0.85 * (i / n);
            const x = ((p.r - rMin) / rRange) * (w - 8) + 4;
            const y = h - ((p.vr - vrMin) / vrRange) * (h - 8) - 4;
            ctx.fillStyle = `rgba(96, 165, 250, ${alpha})`;
            ctx.beginPath();
            ctx.arc(x, y, 1.5, 0, 2 * Math.PI);
            ctx.fill();
        }

        // Labels
        ctx.fillStyle = '#6b7280';
        ctx.font = '8px sans-serif';
        ctx.fillText('r', w - 10, zeroY - 3);
        ctx.fillText('v_r', 2, 10);
    }

    // ── Section 5: Time-Series ───────────────────────────────────────
    _updateTimeSeries(diag) {
        const pMag = Math.sqrt(diag.momentumX ** 2 + diag.momentumY ** 2 + diag.momentumZ ** 2);
        const lMag = Math.sqrt(diag.angMomX ** 2 + diag.angMomY ** 2 + diag.angMomZ ** 2);
        const virial = diag.totalPE !== 0 ? 2 * diag.totalKE / Math.abs(diag.totalPE) : 0;

        this._tsEnergy.push([diag.totalKE, diag.totalPE, diag.totalEnergy]);
        this._tsMomentum.push([pMag]);
        this._tsAngmom.push([lMag]);
        this._tsVirial.push([virial]);
    }

    /** Render all charts (call from rAF, throttled) */
    drawCharts() {
        this._sparks.energy.draw('#42a5f5');
        this._sparks.momentum.draw('#4ade80');
        this._sparks.angmom.draw('#a78bfa');
        this._sparks.drift.draw('#fbbf24');
        this._tsEnergy.draw();
        this._tsMomentum.draw();
        this._tsAngmom.draw();
        this._tsVirial.draw();
    }

    /** Reset all state (call on scenario change) */
    clear() {
        this._initialEnergy = null;
        this._initialMomentum = null;
        this._initialAngmom = null;
        this._phaseBuf.clear();
        this._drawPhaseSpace();
        for (const s of Object.values(this._sparks)) s.clear();
        this._tsEnergy.clear();
        this._tsMomentum.clear();
        this._tsAngmom.clear();
        this._tsVirial.clear();
        if (this._tbody) this._tbody.innerHTML = '';
    }
}
