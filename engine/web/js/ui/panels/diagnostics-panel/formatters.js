/**
 * Value formatters for the diagnostics table.
 * Rules:
 *  - Integers: exact, no decimal.
 *  - Floats in [1e-3, 1e4): up to 6 sig-figs, trailing zeros trimmed.
 *  - Outside that range: scientific notation with 2 sig-figs after decimal.
 *  - Vectors: "x, y, z" each formatted via the scalar rule.
 *  - Pair / triple (badge groups): "a / b" / "a / b / c".
 *  - NaN / null / undefined: em-dash.
 */

const DASH = '\u2014';

function formatScalar(v) {
    if (v === null || v === undefined || Number.isNaN(v)) return DASH;
    if (v === 0) return '0';
    if (Number.isInteger(v) && Math.abs(v) < 1e6) return String(v);
    const abs = Math.abs(v);
    if (abs >= 1e4 || abs < 1e-3) return v.toExponential(2);
    const fixed = v.toPrecision(6);
    return fixed.indexOf('.') >= 0 ? fixed.replace(/(\.\d*?)0+$/, '$1').replace(/\.$/, '') : fixed;
}

/**
 * @param {number|number[]} value
 * @param {{ kind?: 'scalar'|'vector'|'pair'|'triple' }} [opts]
 */
export function formatValue(value, opts = {}) {
    const kind = opts.kind || 'scalar';
    if (kind === 'scalar') return formatScalar(value);
    if (!Array.isArray(value)) return DASH;
    if (kind === 'vector') return value.map(formatScalar).join(', ');
    if (kind === 'pair')   return value.slice(0, 2).map(formatScalar).join(' / ');
    if (kind === 'triple') return value.slice(0, 3).map(formatScalar).join(' / ');
    return DASH;
}
