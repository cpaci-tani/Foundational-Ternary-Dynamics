/**
 * DiagnosticsTable — renders one section's table + owns per-row cells,
 * reset-scoped running stats, and per-row Sparkline instances.
 *
 *   const table = new DiagnosticsTable(section, hub);
 *   containerEl.appendChild(table.el);
 *   // every frame:
 *   table.update();
 *   // on scale exit:
 *   table.destroy();
 */

import { Sparkline } from '../../charts/sparkline.js';
import { formatValue } from './formatters.js';

const DASH = '\u2014';
const DYNAMIC_COLS = 6;
const TABLE_SPARK_VISIBLE_SAMPLES = 48;

function resolvePath(obj, path) {
    const parts = path.split('.');
    let cur = obj;
    for (const p of parts) {
        if (cur == null) return undefined;
        cur = cur[p];
    }
    return cur;
}

function readSource(hub, row) {
    if (typeof row.compute === 'function') return row.compute(hub);
    const src = row.source;
    if (Array.isArray(src)) return src.map((p) => resolvePath(hub, p));
    return resolvePath(hub, src);
}

function resolveBuffer(hub, trend) {
    if (!trend) return null;
    return resolvePath(hub, trend);
}

function numericSample(row, raw) {
    const kind = row.format || 'scalar';
    if (kind !== 'scalar') return null;
    return (typeof raw === 'number' && Number.isFinite(raw)) ? raw : null;
}

function bufferStamp(buf) {
    return buf ? `${buf.head}:${buf.count}` : '';
}

function resetVersion(hub, scope) {
    return (scope == null || typeof hub.getResetVersion !== 'function')
        ? 0
        : hub.getResetVersion(scope);
}

function scopeTick(hub, scope) {
    if (scope === 0) return hub.s0?.diag?.tick ?? null;
    if (scope === 1) return hub.s1?.diag?.tick ?? null;
    if (scope === 2 || scope === 3) return hub.s2?.diag?.tick ?? null;
    if (scope === 5) return hub.s5?.diag?.tick ?? null;
    return null;
}

class RunningStats {
    constructor() {
        this.reset();
    }

    reset() {
        this.count = 0;
        this.min = Infinity;
        this.max = -Infinity;
        this.sum = 0;
    }

    push(value) {
        if (!Number.isFinite(value)) return;
        this.count++;
        this.min = Math.min(this.min, value);
        this.max = Math.max(this.max, value);
        this.sum += value;
    }

    get avg() {
        return this.count > 0 ? this.sum / this.count : null;
    }
}

export class DiagnosticsTable {
    constructor(section, hub, { resetScope = null } = {}) {
        this.section = section;
        this.hub     = hub;
        this.resetScope = resetScope;
        this.resetVersion = resetVersion(hub, resetScope);
        this.el      = document.createElement('section');
        this.el.className = 'diag-section';
        this.el.dataset.section = section.id;

        const isStatic = section.variant === 'static';

        this.el.innerHTML = `
            <h3 class="diag-section-title">${section.title}</h3>
            <table class="diag-table${isStatic ? ' diag-table-static' : ''}">
                <colgroup>
                    <col class="diag-col-metric">
                    <col class="diag-col-value">
                    <col class="diag-col-unit">
                    ${isStatic ? '' : `
                        <col class="diag-col-stat">
                        <col class="diag-col-stat">
                        <col class="diag-col-stat">
                    `}
                </colgroup>
                <thead>
                    <tr>
                        <th scope="col">Metric</th>
                        <th scope="col">Value</th>
                        <th scope="col">Unit</th>
                        ${isStatic ? '' : `
                            <th scope="col">Min</th>
                            <th scope="col">Max</th>
                            <th scope="col">Avg</th>
                        `}
                    </tr>
                </thead>
                <tbody></tbody>
            </table>
        `;

        const tbody = this.el.querySelector('tbody');
        this.cells = new Map();
        this.stats = new Map();
        this.pulseTokens = new Map();
        this.sparks = [];
        this.trendBuffers = new Map();
        this.sampleStamps = new Map();

        section.rows.forEach((row, idx) => {
            const tr = document.createElement('tr');
            tr.className = 'diag-data-row';
            tr.dataset.row = row.id;
            tr.classList.add(idx % 2 === 0 ? 'diag-band-odd' : 'diag-band-even');
            if (!isStatic && row.trend) tr.classList.add('diag-has-trend');
            if (row.variant) tr.classList.add(`diag-row-${row.variant}`);

            const metricCell = document.createElement('td');
            metricCell.className = 'diag-metric';
            metricCell.textContent = row.label;
            tr.appendChild(metricCell);

            const valueCell = document.createElement('td');
            valueCell.className = 'diag-value';
            valueCell.dataset.value = '';
            valueCell.textContent = DASH;
            tr.appendChild(valueCell);
            this.cells.set(row.id, valueCell);

            const unitCell = document.createElement('td');
            unitCell.className = 'diag-unit';
            unitCell.textContent = row.unit || DASH;
            tr.appendChild(unitCell);

            if (!isStatic) {
                const minCell = document.createElement('td');
                minCell.className = 'diag-stat diag-stat-min';
                minCell.textContent = DASH;
                tr.appendChild(minCell);

                const maxCell = document.createElement('td');
                maxCell.className = 'diag-stat diag-stat-max';
                maxCell.textContent = DASH;
                tr.appendChild(maxCell);

                const avgCell = document.createElement('td');
                avgCell.className = 'diag-stat diag-stat-avg';
                avgCell.textContent = DASH;
                tr.appendChild(avgCell);

                this.cells.set(`${row.id}:min`, minCell);
                this.cells.set(`${row.id}:max`, maxCell);
                this.cells.set(`${row.id}:avg`, avgCell);
                this.stats.set(row.id, new RunningStats());

                tbody.appendChild(tr);

                if (row.trend) {
                    const trendRow = document.createElement('tr');
                    trendRow.className = 'diag-trend-row';
                    trendRow.dataset.row = row.id;
                    trendRow.classList.add(idx % 2 === 0 ? 'diag-band-odd' : 'diag-band-even');
                    if (row.variant) trendRow.classList.add(`diag-row-${row.variant}`);

                    const trendCell = document.createElement('td');
                    trendCell.className = 'diag-trend-cell';
                    trendCell.colSpan = DYNAMIC_COLS;

                    const sparkHost = document.createElement('div');
                    sparkHost.className = 'diag-spark-host';
                    trendCell.appendChild(sparkHost);
                    const buf = resolveBuffer(hub, row.trend);
                    if (buf && typeof buf.get === 'function') {
                        const spark = new Sparkline(sparkHost, {
                            buffer: buf,
                            color:  'var(--accent)',
                            height: 22,
                            visibleSamples: TABLE_SPARK_VISIBLE_SAMPLES,
                        });
                        this.sparks.push(spark);
                        this.trendBuffers.set(row.id, buf);
                    }
                    trendRow.appendChild(trendCell);
                    tbody.appendChild(trendRow);
                }
            } else {
                tbody.appendChild(tr);
            }
        });
    }

    update() {
        const nextResetVersion = resetVersion(this.hub, this.resetScope);
        if (nextResetVersion !== this.resetVersion) {
            this.resetVersion = nextResetVersion;
            this.resetStats();
        }
        const tick = scopeTick(this.hub, this.resetScope);

        for (const row of this.section.rows) {
            const raw = readSource(this.hub, row);
            const formatted = formatValue(raw, { kind: row.format || 'scalar' });
            const cell = this.cells.get(row.id);
            if (cell.textContent !== formatted) {
                cell.textContent = formatted;
                if (this.pulseTokens.get(row.id) !== undefined) {
                    cell.classList.remove('is-pulsing');
                    void cell.offsetWidth;
                    cell.classList.add('is-pulsing');
                }
                this.pulseTokens.set(row.id, formatted);
            }

            const sample = numericSample(row, raw);
            if (sample !== null) {
                const buf = this.trendBuffers.get(row.id);
                const stamp = buf ? bufferStamp(buf) : String(tick ?? formatted);
                if (stamp !== this.sampleStamps.get(row.id)) {
                    this.sampleStamps.set(row.id, stamp);
                    this.updateStats(row.id, sample);
                }
            }
        }
        for (const spark of this.sparks) spark.update();
    }

    resetStats() {
        for (const stats of this.stats.values()) stats.reset();
        this.sampleStamps.clear();
        for (const row of this.section.rows) this.renderStats(row.id);
    }

    updateStats(rowId, sample) {
        const stats = this.stats.get(rowId);
        if (!stats) return;
        stats.push(sample);
        this.renderStats(rowId);
    }

    renderStats(rowId) {
        const stats = this.stats.get(rowId);
        const minCell = this.cells.get(`${rowId}:min`);
        const maxCell = this.cells.get(`${rowId}:max`);
        const avgCell = this.cells.get(`${rowId}:avg`);
        if (!stats || !minCell || !maxCell || !avgCell) return;
        if (stats.count === 0) {
            minCell.textContent = DASH;
            maxCell.textContent = DASH;
            avgCell.textContent = DASH;
            return;
        }
        minCell.textContent = formatValue(stats.min, { kind: 'scalar' });
        maxCell.textContent = formatValue(stats.max, { kind: 'scalar' });
        avgCell.textContent = formatValue(stats.avg, { kind: 'scalar' });
    }

    destroy() {
        for (const s of this.sparks) s.destroy();
        this.sparks.length = 0;
        this.cells.clear();
        this.stats.clear();
        this.trendBuffers.clear();
        this.sampleStamps.clear();
        this.el.remove();
    }
}
