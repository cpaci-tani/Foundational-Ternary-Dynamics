/**
 * Render a horizontal σ-deviation strip.
 *
 * The strip is a fixed-range axis from -5σ to +5σ with a 0σ reference line
 * and a single marker at the pull position. Intensity (opacity) scales with
 * |pull| but there are no threshold colors — the reader decides what a
 * given σ means, not the UI. This mirrors PDG-style tension plots.
 *
 * @param {number|null} pull - deviation in σ units; null/undefined renders an empty axis.
 * @param {Object} [opts]
 * @param {number} [opts.width=320]  - SVG width in px
 * @param {number} [opts.height=24]  - SVG height in px
 * @param {number} [opts.range=5]    - axis half-range in σ (clamps beyond)
 * @returns {string} SVG markup
 */
export function renderPullStrip(pull, opts = {}) {
    const W = opts.width ?? 320;
    const H = opts.height ?? 24;
    const R = opts.range ?? 5;
    const midY = H / 2;

    // x-axis line + 0σ tick + ±Rσ end ticks
    let out = `<svg class="verify-pull-strip" viewBox="0 0 ${W} ${H}" width="${W}" height="${H}" role="img" aria-label="pull ${pull == null ? 'n/a' : pull.toFixed(2) + 'σ'}">`;
    out += `<line x1="0" y1="${midY}" x2="${W}" y2="${midY}" class="pull-axis" />`;
    out += `<line x1="${W / 2}" y1="2" x2="${W / 2}" y2="${H - 2}" class="pull-zero" />`;
    out += `<line x1="2" y1="${midY - 4}" x2="2" y2="${midY + 4}" class="pull-tick" />`;
    out += `<line x1="${W - 2}" y1="${midY - 4}" x2="${W - 2}" y2="${midY + 4}" class="pull-tick" />`;

    if (pull != null && Number.isFinite(pull)) {
        const clamped = Math.max(-R, Math.min(R, pull));
        const x = ((clamped + R) / (2 * R)) * W;
        const intensity = Math.min(1, Math.abs(pull) / R);
        out += `<circle class="pull-marker" cx="${x}" cy="${midY}" r="4" style="opacity: ${0.35 + 0.65 * intensity}" />`;
    }

    out += `</svg>`;
    return out;
}
