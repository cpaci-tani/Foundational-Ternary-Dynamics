/** Retained Gravity-panel chart rendering driven by exact observation ticks. */
import { formatExp } from './_card-helpers.js';

const SPARK_WIDTH = 116;
const SPARK_HEIGHT = 44;

const DELTA_SERIES = Object.freeze([
    { key: 'Lmax', label: 'L max', color: 'var(--accent)', select: (row) => row.Lmax, current: (m) => m.L.max },
    { key: 'Kmax', label: 'K max', color: 'var(--caution, #fb8c00)', select: (row) => row.Kmax, current: (m) => m.K.max },
    { key: 'Fmean', label: '|F| mean', color: 'var(--positive-text)', select: (row) => row.Fmean, current: (m) => m.F.mean },
    { key: 'dil', label: 'dilation %', color: 'var(--negative-text)', select: (row) => row.dil, current: (m) => m.dilationPct },
]);

function setText(node, value) {
    const text = String(value);
    if (!node || node.textContent === text) return;
    const child = node.firstChild;
    if (node.childNodes.length === 1 && child?.nodeType === 3) child.data = text;
    else node.textContent = text;
}

function setAttr(node, name, value) {
    if (!node) return;
    const text = String(value);
    if (node.getAttribute(name) !== text) node.setAttribute(name, text);
}

function observationTick(row) {
    const tick = row?.tick;
    return Number.isSafeInteger(tick) && tick >= 0 ? tick : null;
}

function formatTick(value) {
    return Number.isSafeInteger(value) ? String(value) : Number(value).toPrecision(6);
}

function formatMetric(value) {
    return formatExp(value).trim();
}

/**
 * Build a sparkline path whose x coordinate is the real observation tick.
 * Rows without an exact nonnegative integer tick are rejected rather than
 * relabelled with a transport revision or array index. Constant series sit on
 * the vertical midpoint instead of masquerading as a lower-bound trace.
 */
export function gravitySparklineGeometry(history, select, width = SPARK_WIDTH, height = SPARK_HEIGHT) {
    const rows = Array.isArray(history) ? history : [];
    const points = [];
    let tickMin = Infinity, tickMax = -Infinity;
    let valueMin = Infinity, valueMax = -Infinity;
    for (let index = 0; index < rows.length; index++) {
        const tick = observationTick(rows[index]);
        const value = Number(select(rows[index]));
        if (tick === null || !Number.isFinite(value)) continue;
        points.push({ tick, value });
        if (tick < tickMin) tickMin = tick;
        if (tick > tickMax) tickMax = tick;
        if (value < valueMin) valueMin = value;
        if (value > valueMax) valueMax = value;
    }
    if (!points.length) return {
        path: '', count: 0, tickMin: null, tickMax: null, valueMin: null, valueMax: null,
    };

    const xSpan = tickMax - tickMin;
    const ySpan = valueMax - valueMin;
    const innerHeight = Math.max(0, height - 4);
    let path = '';
    for (let index = 0; index < points.length; index++) {
        const point = points[index];
        const x = xSpan > 0
            ? ((point.tick - tickMin) / xSpan) * width
            : (points.length > 1 ? (index / (points.length - 1)) * width : width / 2);
        const y = ySpan > 0
            ? 2 + (1 - ((point.value - valueMin) / ySpan)) * innerHeight
            : height / 2;
        path += `${index ? 'L' : 'M'}${x.toFixed(2)},${y.toFixed(2)} `;
    }
    // A lone observation otherwise produces a zero-length, invisible path.
    if (points.length === 1) {
        const x = width / 2, y = height / 2;
        path = `M${Math.max(0, x - 2).toFixed(2)},${y.toFixed(2)} L${Math.min(width, x + 2).toFixed(2)},${y.toFixed(2)}`;
    }
    return { path: path.trim(), count: points.length, tickMin, tickMax, valueMin, valueMax };
}

/** Histogram bars in one retained SVG path. */
export function histogramPath(hist, width = 70, height = 20) {
    const counts = hist?.counts;
    if (!counts?.length) return '';
    let max = 0;
    for (let index = 0; index < counts.length; index++) {
        const value = Number(counts[index]);
        if (Number.isFinite(value) && value > max) max = value;
    }
    if (!(max > 0)) return '';
    const binWidth = width / counts.length;
    let path = '';
    for (let index = 0; index < counts.length; index++) {
        const count = Math.max(0, Number(counts[index]) || 0);
        const x = index * binWidth;
        const y = height - (count / max) * height;
        path += `M${x.toFixed(2)},${height}V${y.toFixed(2)}H${Math.max(x, x + binWidth - 0.3).toFixed(2)}V${height}Z`;
    }
    return path;
}

export function histogramAriaLabel(label, hist) {
    const counts = hist?.counts || [];
    const samples = counts.reduce((sum, value) => sum + (Number(value) || 0), 0);
    const min = Number(hist?.min), max = Number(hist?.max);
    const range = Number.isFinite(min) && Number.isFinite(max)
        ? `range ${formatMetric(min)} to ${formatMetric(max)}` : 'range unavailable';
    return `${label} histogram; ${samples} sampled values; ${range}; ${counts.length} bins.`;
}

/** Retained-DOM renderer for the four Gravity delta traces. */
export function renderDelta(container, history, latched, current) {
    if (!container) return;
    if (!history?.length || !current) {
        container.innerHTML = '<div class="grav-empty">No field data yet — load a gravity scenario.</div>';
        container._gravityDeltaView = null;
        return;
    }
    if (!container._gravityDeltaView) {
        container.innerHTML = DELTA_SERIES.map((series) => `
            <div class="grav-spark-row" data-grav-series="${series.key}">
                <span class="grav-spark-label">${series.label}</span>
                <span class="grav-spark-now">—</span>
                <span class="grav-spark-delta">Δ —</span>
                <span class="grav-spark-range">ticks — · range —</span>
                <svg viewBox="0 0 ${SPARK_WIDTH} ${SPARK_HEIGHT}" preserveAspectRatio="none"
                     class="grav-spark" role="img" aria-label="${series.label} history unavailable">
                    <path fill="none" stroke="${series.color}" stroke-width="1.5"
                          vector-effect="non-scaling-stroke"/>
                </svg>
            </div>`).join('');
        container._gravityDeltaView = Object.fromEntries(
            [...container.querySelectorAll('[data-grav-series]')].map((node) => [
                node.dataset.gravSeries,
                {
                    path: node.querySelector('path'),
                    svg: node.querySelector('svg'),
                    now: node.querySelector('.grav-spark-now'),
                    delta: node.querySelector('.grav-spark-delta'),
                    range: node.querySelector('.grav-spark-range'),
                },
            ]),
        );
    }

    for (const series of DELTA_SERIES) {
        const view = container._gravityDeltaView[series.key];
        const geometry = gravitySparklineGeometry(history, series.select);
        const currentValue = Number(series.current(current));
        if (geometry.tickMax != null) setAttr(view.svg, 'data-sample-tick', geometry.tickMax);
        else view.svg.removeAttribute('data-sample-tick');
        setAttr(view.path, 'd', geometry.path);
        setText(view.now, formatExp(currentValue));

        const baseline = latched ? Number(latched[series.key]) : Number.NaN;
        if (!Number.isFinite(baseline) || !Number.isFinite(currentValue)) {
            setText(view.delta, 'Δ —');
            view.delta.style.color = 'var(--text-muted)';
        } else {
            const delta = currentValue - baseline;
            const color = Math.abs(delta) < 1e-12
                ? 'var(--text-muted)'
                : (delta > 0 ? 'var(--positive-text)' : 'var(--negative-text)');
            setText(view.delta, `Δ ${delta > 0 ? '+' : ''}${formatExp(delta)}`);
            if (view.delta.style.color !== color) view.delta.style.color = color;
        }

        if (!geometry.count) {
            setText(view.range, 'ticks — · range —');
            setAttr(view.svg, 'aria-label', `${series.label} history unavailable`);
            continue;
        }
        const tickRange = geometry.tickMin === geometry.tickMax
            ? `tick ${formatTick(geometry.tickMin)}`
            : `ticks ${formatTick(geometry.tickMin)}–${formatTick(geometry.tickMax)}`;
        const valueRange = geometry.valueMin === geometry.valueMax
            ? `value ${formatMetric(geometry.valueMin)}`
            : `range ${formatMetric(geometry.valueMin)}–${formatMetric(geometry.valueMax)}`;
        setText(view.range, `${tickRange} · ${valueRange}`);
        setAttr(view.svg, 'aria-label', `${series.label}; ${tickRange}; ${valueRange}; current ${formatMetric(currentValue)}.`);
    }
}
