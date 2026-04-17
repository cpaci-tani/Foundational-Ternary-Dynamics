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
        font:      '11px ' + readVar('--font-body', 'Inter, sans-serif'),
        fontMono:  '11px ' + readVar('--font-mono', 'JetBrains Mono, monospace'),
    };
}

/**
 * Build a uPlot axis config from a theme object and per-axis overrides.
 * @param {ReturnType<getChartTheme>} theme
 * @param {{ label?: string, scale?: string, side?: number }} [opts]
 */
export function makeAxis(theme, opts = {}) {
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
    };
}
