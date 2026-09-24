/**
 * Chart theme reader — converts CSS custom properties into a uPlot-shaped
 * theme object. Called once per chart at init; does NOT hot-swap on theme
 * change (explicit tradeoff — see design doc).
 */

function readVar(name, fallback) {
    const raw = getComputedStyle(document.documentElement).getPropertyValue(name).trim();
    return raw || fallback;
}

/**
 * Resolve a CSS custom property reference to a concrete color string so it
 * can be used in a canvas context (which doesn't understand var()).
 *
 * Input examples:
 *   'var(--chart-flux, #fb8c00)'  →  value of --chart-flux, else '#fb8c00'
 *   '#ff0000'                     →  '#ff0000'  (passed through unchanged)
 */
export function resolveChartColor(expr) {
    if (!expr || !expr.startsWith('var(')) return expr;
    const m = expr.match(/^var\(\s*(--[\w-]+)\s*(?:,\s*(.+?))?\s*\)$/);
    if (!m) return expr;
    const resolved = getComputedStyle(document.documentElement).getPropertyValue(m[1]).trim();
    return resolved || (m[2] && m[2].trim()) || expr;
}

/**
 * @returns {{
 *   axis: string, grid: string, bg: string,
 *   text: string, textMuted: string,
 *   font: string, fontMono: string
 * }}
 */
export function getChartTheme() {
    return {
        axis:      readVar('--chart-axis',      readVar('--text-muted', '#6b7280')),
        grid:      readVar('--chart-grid',      readVar('--border',     '#2a3a5a')),
        bg:        readVar('--chart-bg',        'transparent'),
        text:      readVar('--text-primary',    '#e8e8e8'),
        textMuted: readVar('--text-muted',      '#6b7280'),
        font:      '16px ' + readVar('--font-body', 'Inter, sans-serif'),
        fontMono:  '12px ' + readVar('--font-mono', 'JetBrains Mono, monospace'),
    };
}

/**
 * Build a uPlot axis config from a theme object and per-axis overrides.
 * @param {ReturnType<getChartTheme>} theme
 * @param {{ label?: string, scale?: string, side?: number }} [opts]
 */
export function makeAxis(theme, opts = {}) {
    const vertical = opts.side === 1 || opts.side === 3;
    return {
        stroke: theme.axis,
        grid:   { stroke: theme.grid, width: 0.5 },
        ticks:  { stroke: theme.axis, width: 0.5, size: 4 },
        font:   theme.fontMono,
        labelFont: theme.font,
        labelSize: opts.label ? 14 : 0,
        label: opts.label,
        scale: opts.scale,
        side:  opts.side,
        // uPlot's fixed 50px gutter let large signed values (e.g. -200,000)
        // overlap the vertical axis title in narrow side panels. Measure the
        // actual tick labels, with a bounded gutter that leaves plot space.
        ...(vertical ? { size: (plot, values) => {
            if (!values?.length || !plot.ctx) return 50;
            const ctx = plot.ctx;
            ctx.save();
            ctx.font = theme.fontMono;
            let width = 0;
            for (const value of values) {
                if (value != null) width = Math.max(width, ctx.measureText(String(value)).width);
            }
            ctx.restore();
            return Math.min(112, Math.max(50, Math.ceil(width) + 12));
        } } : {}),
    };
}
