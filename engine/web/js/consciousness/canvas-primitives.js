/**
 * Shared Canvas 2D primitives for consciousness pedagogy panels.
 *
 * Extracted from consciousness-pedagogy.js (ticket CP-1).
 */

// ── Color Palette (mirrors CSS variables) ────────────────────────────

export const COL = {
    primary:    '#00e5ff',
    secondary:  '#7c4dff',
    glow:       '#00bcd4',
    gold:       '#ffd700',
    bgCard:     '#283548',
    bgSurface:  '#1f2937',
    bgDeep:     '#111827',
    textPri:    '#f3f4f6',
    textSec:    '#9ca3af',
    textMuted:  '#6b7280',
    positive:   '#4ade80',
    negative:   '#f87171',
    warning:    '#fbbf24',
    accent:     '#60a5fa',
};

// ── Unicode helpers ──────────────────────────────────────────────────

export const SYM = {
    theta:  '\u03B8',
    alpha:  '\u03B1',
    pi:     '\u03C0',
    Delta:  '\u0394',
    infty:  '\u221E',
    beta:   '\u03B2',
    sup2:   '\u00B2',
    approx: '\u2248',
    pm:     '\u00B1',
    leq:    '\u2264',
    sub_C:  '\u1D9C',  // modifier letter small c (approximation)
};

// ── Font constants ───────────────────────────────────────────────────

export const FONT_LABEL = '11px Inter, sans-serif';
export const FONT_VALUE = '11px JetBrains Mono, monospace';
export const FONT_TITLE = 'bold 12px Inter, sans-serif';

// ── Drawing primitives ───────────────────────────────────────────────

/** Scale a canvas for devicePixelRatio and return [ctx, w, h]. */
export function prepCanvas(canvas) {
    const dpr = window.devicePixelRatio || 1;
    const rect = canvas.getBoundingClientRect();
    const w = rect.width;
    const h = rect.height;
    canvas.width  = w * dpr;
    canvas.height = h * dpr;
    const ctx = canvas.getContext('2d');
    ctx.setTransform(dpr, 0, 0, dpr, 0, 0);
    return [ctx, w, h];
}

/** Clear a canvas with the background color. */
export function clearCanvas(ctx, w, h) {
    ctx.fillStyle = COL.bgDeep;
    ctx.fillRect(0, 0, w, h);
}

/** Draw a dashed line. */
export function dashedLine(ctx, x0, y0, x1, y1, color, segments = [4, 3]) {
    ctx.save();
    ctx.strokeStyle = color;
    ctx.setLineDash(segments);
    ctx.beginPath();
    ctx.moveTo(x0, y0);
    ctx.lineTo(x1, y1);
    ctx.stroke();
    ctx.restore();
}

/** Draw a filled circle. */
export function dot(ctx, x, y, r, color) {
    ctx.fillStyle = color;
    ctx.beginPath();
    ctx.arc(x, y, r, 0, 2 * Math.PI);
    ctx.fill();
}

/** Draw a stroked (hollow) circle. */
export function ring(ctx, x, y, r, color, lw = 1.5) {
    ctx.strokeStyle = color;
    ctx.lineWidth = lw;
    ctx.beginPath();
    ctx.arc(x, y, r, 0, 2 * Math.PI);
    ctx.stroke();
}

/** Draw text with font + color. */
export function label(ctx, text, x, y, color, font = FONT_LABEL, align = 'left', baseline = 'middle') {
    ctx.font = font;
    ctx.fillStyle = color;
    ctx.textAlign = align;
    ctx.textBaseline = baseline;
    ctx.fillText(text, x, y);
}

/** Clamp a value. */
export function clamp(v, lo, hi) { return v < lo ? lo : v > hi ? hi : v; }

/** Simple seeded PRNG (mulberry32). */
export function mulberry32(seed) {
    return function () {
        seed |= 0;
        seed = seed + 0x6D2B79F5 | 0;
        let t = Math.imul(seed ^ seed >>> 15, 1 | seed);
        t = t + Math.imul(t ^ t >>> 7, 61 | t) ^ t;
        return ((t ^ t >>> 14) >>> 0) / 4294967296;
    };
}
