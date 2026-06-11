const DASH = '\u2014';

function escapeHtml(value) {
    return String(value)
        .replace(/&/g, '&amp;')
        .replace(/</g, '&lt;')
        .replace(/>/g, '&gt;')
        .replace(/"/g, '&quot;')
        .replace(/'/g, '&#39;');
}

export function formatChartValue(value, unit = '') {
    if (value === undefined || value === null || !Number.isFinite(value)) return DASH;
    const abs = Math.abs(value);
    const text = (abs !== 0 && (abs < 1e-7 || abs >= 1e7))
        ? value.toExponential(8)
        : value.toLocaleString(undefined, {
            maximumFractionDigits: abs >= 1000 ? 4 : 8,
            minimumFractionDigits: 0,
            useGrouping: abs >= 10000,
        });
    return unit ? `${text} ${unit}` : text;
}

export function formatChartSample(value) {
    if (value === undefined || value === null || !Number.isFinite(value)) return DASH;
    return Number.isInteger(value) ? String(value) : value.toFixed(3);
}

export class ChartHoverTooltip {
    constructor(container) {
        this.container = container;
        this.el = document.createElement('div');
        this.el.className = 'chart-hover-tooltip';
        this.el.hidden = true;
        container.classList.add('chart-hover-scope');
        container.appendChild(this.el);
    }

    hide() {
        this.el.hidden = true;
        this.el.innerHTML = '';
    }

    render({ title = '', xLabel = 'sample', xValue = null, rows = [], anchorLeft = 0, anchorTop = 0 }) {
        const visibleRows = rows.filter(Boolean);
        if (visibleRows.length === 0) {
            this.hide();
            return;
        }

        this.el.innerHTML = `
            <div class="chart-hover-title">${escapeHtml(title)}</div>
            <div class="chart-hover-meta">
                <span>${escapeHtml(xLabel)}</span>
                <strong>${escapeHtml(formatChartSample(xValue))}</strong>
            </div>
            <div class="chart-hover-rows">
                ${visibleRows.map((row) => `
                    <div class="chart-hover-row">
                        <span class="chart-hover-swatch" style="--chart-hover-color:${escapeHtml(row.color || 'var(--accent)')}"></span>
                        <span class="chart-hover-label">${escapeHtml(row.label || '')}</span>
                        <strong>${escapeHtml(row.value || DASH)}</strong>
                    </div>
                `).join('')}
            </div>
        `;
        this.el.hidden = false;
        this._position(anchorLeft, anchorTop);
    }

    _position(anchorLeft, anchorTop) {
        const margin = 8;
        const gap = 12;
        const width = this.el.offsetWidth || 180;
        const height = this.el.offsetHeight || 84;
        const maxLeft = Math.max(margin, this.container.clientWidth - width - margin);
        const maxTop = Math.max(margin, this.container.clientHeight - height - margin);
        let left = anchorLeft + gap;
        let top = anchorTop + gap;
        if (left > maxLeft) left = anchorLeft - width - gap;
        if (top > maxTop) top = anchorTop - height - gap;
        this.el.style.left = `${Math.max(margin, Math.min(left, maxLeft))}px`;
        this.el.style.top = `${Math.max(margin, Math.min(top, maxTop))}px`;
    }

    destroy() {
        this.el.remove();
    }
}
