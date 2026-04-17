/**
 * DiagnosticsTable — renders one section's table + owns per-row cell
 * references and Sparkline instances for the Trend column.
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

export class DiagnosticsTable {
    constructor(section, hub) {
        this.section = section;
        this.hub     = hub;
        this.el      = document.createElement('section');
        this.el.className = 'diag-section';
        this.el.dataset.section = section.id;

        const isStatic = section.variant === 'static';

        this.el.innerHTML = `
            <h3 class="diag-section-title">${section.title}</h3>
            <table class="diag-table${isStatic ? ' diag-table-static' : ''}">
                <thead>
                    <tr>
                        <th scope="col">Metric</th>
                        <th scope="col">Value</th>
                        <th scope="col">Unit</th>
                        ${isStatic ? '' : '<th scope="col">Trend</th>'}
                    </tr>
                </thead>
                <tbody></tbody>
            </table>
        `;

        const tbody = this.el.querySelector('tbody');
        this.cells = new Map();
        this.pulseTokens = new Map();
        this.sparks = [];

        for (const row of section.rows) {
            const tr = document.createElement('tr');
            tr.dataset.row = row.id;
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
                const trendCell = document.createElement('td');
                trendCell.className = 'diag-trend';
                if (row.trend) {
                    const sparkHost = document.createElement('div');
                    sparkHost.className = 'diag-spark-host';
                    trendCell.appendChild(sparkHost);
                    const buf = resolveBuffer(hub, row.trend);
                    if (buf && typeof buf.get === 'function') {
                        const spark = new Sparkline(sparkHost, {
                            buffer: buf,
                            color:  'var(--accent)',
                            height: 22,
                        });
                        this.sparks.push(spark);
                    }
                } else {
                    trendCell.textContent = DASH;
                }
                tr.appendChild(trendCell);
            }

            tbody.appendChild(tr);
        }
    }

    update() {
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
        }
        for (const spark of this.sparks) spark.update();
    }

    destroy() {
        for (const s of this.sparks) s.destroy();
        this.sparks.length = 0;
        this.cells.clear();
        this.el.remove();
    }
}
