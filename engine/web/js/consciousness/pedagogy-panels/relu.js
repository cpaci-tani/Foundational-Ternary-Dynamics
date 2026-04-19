/**
 * Panel 3: ReLU / Softplus crystallization.
 *
 * Extracted from consciousness-pedagogy.js (ticket CP-3).
 */

import {
    COL, SYM, FONT_LABEL, FONT_VALUE,
    dot, label, clamp,
} from '../canvas-primitives.js';

export function drawRelu(ctx, w, h, state) {
    const beta = state._betaValue;
    const pad = { l: 40, r: 15, t: 15, b: 30 };
    const pw = w - pad.l - pad.r;
    const ph = h - pad.t - pad.b;

    const xMin = -4, xMax = 5;
    const yMin = -0.5, yMax = 5;
    const xRange = xMax - xMin;
    const yRange = yMax - yMin;

    const toX = (x) => pad.l + ((x - xMin) / xRange) * pw;
    const toY = (y) => pad.t + (1 - (y - yMin) / yRange) * ph;

    // ── Axes ─────────────────────────────────────────────────────
    ctx.strokeStyle = COL.textMuted;
    ctx.lineWidth = 1;

    const zeroX = toX(0);
    const zeroY = toY(0);

    // x-axis
    ctx.beginPath();
    ctx.moveTo(pad.l, zeroY);
    ctx.lineTo(pad.l + pw, zeroY);
    ctx.stroke();

    // y-axis
    ctx.beginPath();
    ctx.moveTo(zeroX, pad.t);
    ctx.lineTo(zeroX, pad.t + ph);
    ctx.stroke();

    label(ctx, 'x', pad.l + pw + 3, zeroY - 3, COL.textSec, FONT_LABEL);
    label(ctx, 'y', zeroX + 8, pad.t - 2, COL.textSec, FONT_LABEL);

    // ── KMS strip ────────────────────────────────────────────────
    const kmsHeight = Math.PI / beta;
    const kmsCenter = kmsHeight / 2;
    const kmsTopPx = toY(kmsCenter + kmsHeight / 2);
    const kmsBotPx = toY(kmsCenter - kmsHeight / 2);
    ctx.fillStyle = 'rgba(0, 229, 255, 0.08)';
    ctx.fillRect(pad.l, kmsTopPx, pw, kmsBotPx - kmsTopPx);

    label(ctx, `KMS strip: ${SYM.pi}/${SYM.beta} = ${kmsHeight.toFixed(2)}`,
          pad.l + pw - 5, kmsTopPx + 12, COL.glow, FONT_VALUE, 'right');

    // ── Reference curves (thin, semi-transparent) ────────────────
    const refBetas = [1, 5, 20];
    for (const rb of refBetas) {
        if (Math.abs(rb - beta) < 0.5) continue; // skip if matches current
        ctx.strokeStyle = 'rgba(156, 163, 175, 0.3)';
        ctx.lineWidth = 1;
        ctx.beginPath();
        const N = 150;
        for (let i = 0; i <= N; i++) {
            const x = xMin + (i / N) * xRange;
            const y = (1 / rb) * Math.log(1 + Math.exp(rb * x));
            const py = clamp(toY(y), pad.t, pad.t + ph);
            if (i === 0) ctx.moveTo(toX(x), py);
            else         ctx.lineTo(toX(x), py);
        }
        ctx.stroke();
    }

    // ReLU limit (dashed gold)
    ctx.save();
    ctx.strokeStyle = COL.gold;
    ctx.lineWidth = 1;
    ctx.setLineDash([4, 3]);
    ctx.beginPath();
    ctx.moveTo(toX(xMin), toY(0));
    ctx.lineTo(toX(0), toY(0));
    ctx.lineTo(toX(xMax), toY(xMax));
    ctx.stroke();
    ctx.restore();

    label(ctx, `ReLU (${SYM.beta}\u2192${SYM.infty})`, toX(xMax) - 60, toY(xMax) - 12,
          COL.gold, FONT_VALUE);

    // ── Main curve M_β(x) ────────────────────────────────────────
    ctx.strokeStyle = COL.primary;
    ctx.lineWidth = 2.5;
    ctx.beginPath();
    const N = 200;
    for (let i = 0; i <= N; i++) {
        const x = xMin + (i / N) * xRange;
        const y = (1 / beta) * Math.log(1 + Math.exp(beta * x));
        const py = clamp(toY(y), pad.t, pad.t + ph);
        if (i === 0) ctx.moveTo(toX(x), py);
        else         ctx.lineTo(toX(x), py);
    }
    ctx.stroke();

    // ── Annotations ──────────────────────────────────────────────
    // Kink at x=0
    dot(ctx, zeroX, toY((1 / beta) * Math.log(2)), 3, COL.primary);
    label(ctx, `Irreversible: (-${SYM.infty}, 0] \u2192 0`,
          pad.l + 5, pad.t + ph - 20, COL.negative, FONT_VALUE);

    // Type labels
    label(ctx, 'Type III\u2081', pad.l + 5, pad.t + 15, COL.secondary, FONT_LABEL);
    label(ctx, 'Type I', pad.l + pw - 5, pad.t + 15, COL.positive, FONT_LABEL, 'right');

    // Beta value
    label(ctx, `${SYM.beta} = ${beta.toFixed(1)}`, pad.l + pw - 5, pad.t + ph - 5,
          COL.primary, FONT_VALUE, 'right');
}
