/**
 * Panel 0: Master Quadratic Phase Diagram.
 *
 * Extracted from consciousness-pedagogy.js (ticket CP-3).
 */

import { K_CRIT } from '../../constants.js';
import {
    COL, SYM, FONT_LABEL, FONT_VALUE,
    dot, label, clamp,
} from '../canvas-primitives.js';
import { Qk, discriminant, realRoots, GSTAR2 } from './math-helpers.js';

export function drawQuadratic(ctx, w, h, state) {
    const k = state._kValue;
    const pad = { l: 50, r: 20, t: 40, b: 50 };
    const pw = w - pad.l - pad.r;
    const ph = h - pad.t - pad.b;

    // Adaptive x-range: smooth transition around k=5
    const blend = clamp((k - 3) / 4, 0, 1); // 0 at k<=3, 1 at k>=7
    const xMax = 10 + blend * 150;

    // Compute y-range by sampling
    let yMin = Infinity, yMax = -Infinity;
    const N = 200;
    for (let i = 0; i <= N; i++) {
        const x = (i / N) * xMax;
        const y = Qk(k, x);
        if (y < yMin) yMin = y;
        if (y > yMax) yMax = y;
    }
    // Ensure y-axis includes 0
    yMin = Math.min(yMin, -Math.abs(yMax) * 0.1);
    yMax = Math.max(yMax, Math.abs(yMin) * 0.1);
    const yRange = yMax - yMin || 1;

    // Coordinate transforms
    const toX = (x) => pad.l + (x / xMax) * pw;
    const toY = (y) => pad.t + (1 - (y - yMin) / yRange) * ph;

    // ── k-axis indicator at top ──────────────────────────────────
    const kBarY = 14;
    const kBarL = pad.l;
    const kBarR = w - pad.r;
    const kBarW = kBarR - kBarL;

    // k ranges from 0 to 20 for the indicator
    const kMax = 20;
    const kToX = (kv) => kBarL + (kv / kMax) * kBarW;
    const kCritX = kToX(K_CRIT);

    // Domain A (physics): k > K_CRIT — green tint
    ctx.fillStyle = 'rgba(74, 222, 128, 0.15)';
    ctx.fillRect(kCritX, kBarY - 5, kBarR - kCritX, 10);

    // Domain B (consciousness): k < K_CRIT — purple tint
    ctx.fillStyle = 'rgba(124, 77, 255, 0.15)';
    ctx.fillRect(kBarL, kBarY - 5, kCritX - kBarL, 10);

    // Domain C (measurement): gold line at K_CRIT
    ctx.strokeStyle = COL.gold;
    ctx.lineWidth = 2;
    ctx.beginPath();
    ctx.moveTo(kCritX, kBarY - 7);
    ctx.lineTo(kCritX, kBarY + 7);
    ctx.stroke();

    // k marker
    const kMarkerX = kToX(clamp(k, 0, kMax));
    dot(ctx, kMarkerX, kBarY, 4, COL.primary);

    // Labels
    label(ctx, 'k', kBarL - 14, kBarY, COL.textSec, FONT_LABEL, 'right');
    label(ctx, '0', kBarL, kBarY + 14, COL.textMuted, FONT_VALUE, 'center');
    label(ctx, K_CRIT.toFixed(2), kCritX, kBarY + 14, COL.gold, FONT_VALUE, 'center');
    label(ctx, '20', kBarR, kBarY + 14, COL.textMuted, FONT_VALUE, 'center');

    // ── Background tint for main plot ────────────────────────────
    const disc = discriminant(k);
    if (disc > 0) {
        ctx.fillStyle = 'rgba(74, 222, 128, 0.03)';
    } else if (Math.abs(disc) < 0.01) {
        ctx.fillStyle = 'rgba(255, 215, 0, 0.03)';
    } else {
        ctx.fillStyle = 'rgba(124, 77, 255, 0.03)';
    }
    ctx.fillRect(pad.l, pad.t, pw, ph);

    // ── Axes ─────────────────────────────────────────────────────
    const zeroY = toY(0);
    ctx.strokeStyle = COL.textMuted;
    ctx.lineWidth = 1;

    // x-axis
    ctx.beginPath();
    ctx.moveTo(pad.l, clamp(zeroY, pad.t, pad.t + ph));
    ctx.lineTo(pad.l + pw, clamp(zeroY, pad.t, pad.t + ph));
    ctx.stroke();

    // y-axis
    ctx.beginPath();
    ctx.moveTo(pad.l, pad.t);
    ctx.lineTo(pad.l, pad.t + ph);
    ctx.stroke();

    // Axis labels
    label(ctx, 'x', w - pad.r + 5, clamp(zeroY, pad.t, pad.t + ph), COL.textSec);
    label(ctx, `Q${SYM.sub_C}(x)`, pad.l - 5, pad.t - 5, COL.textSec, FONT_LABEL, 'right');

    // ── Parabola ─────────────────────────────────────────────────
    ctx.strokeStyle = COL.primary;
    ctx.lineWidth = 2;
    ctx.beginPath();
    for (let i = 0; i <= N; i++) {
        const x = (i / N) * xMax;
        const y = Qk(k, x);
        const px = toX(x);
        const py = toY(y);
        if (i === 0) ctx.moveTo(px, py);
        else         ctx.lineTo(px, py);
    }
    ctx.stroke();

    // ── Roots / annotations ──────────────────────────────────────
    if (disc > 0) {
        // Two real roots
        const roots = realRoots(k);
        if (roots) {
            const [xm, xp] = roots;
            const zeroYClamped = clamp(zeroY, pad.t, pad.t + ph);

            // Smaller root
            if (xm >= 0 && xm <= xMax) {
                dot(ctx, toX(xm), zeroYClamped, 5, COL.positive);
                const lbl = k > 12 ? `${SYM.approx} N_c` : xm.toFixed(2);
                label(ctx, lbl, toX(xm), zeroYClamped - 12, COL.positive, FONT_VALUE, 'center');
            }

            // Larger root
            if (xp >= 0 && xp <= xMax) {
                dot(ctx, toX(xp), zeroYClamped, 5, COL.accent);
                const lbl = k > 12 ? `1/${SYM.alpha}` : xp.toFixed(1);
                label(ctx, lbl, toX(xp), zeroYClamped - 12, COL.accent, FONT_VALUE, 'center');
            } else if (xp > xMax) {
                // Root off-screen — note it
                label(ctx, `x\u208A = ${xp.toFixed(1)} \u2192`, w - pad.r - 5, pad.t + 20, COL.accent, FONT_VALUE, 'right');
            }
        }
    } else if (Math.abs(disc) < 0.5) {
        // Near-degenerate
        const degen = k * GSTAR2 / 2;
        if (degen >= 0 && degen <= xMax) {
            const zy = clamp(zeroY, pad.t, pad.t + ph);
            dot(ctx, toX(degen), zy, 6, COL.gold);
            label(ctx, 'Born rule', toX(degen) + 8, zy - 12, COL.gold, FONT_LABEL);
        }
    } else {
        // Complex roots
        const re = k * GSTAR2 / 2;
        const imSq = -disc;
        const im = Math.sqrt(imSq) / 2;
        label(ctx, `Complex roots: y = ${re.toFixed(3)} ${SYM.pm} ${im.toFixed(3)}i`,
              pad.l + 10, pad.t + ph - 10, COL.secondary, FONT_VALUE);
    }

    // ── Discriminant bar at bottom ───────────────────────────────
    const barY = h - 14;
    const barH = 8;
    const barMaxW = pw * 0.5;
    const discNorm = clamp(disc / 5000, -1, 1); // normalize for display
    const barW = Math.abs(discNorm) * barMaxW;
    const barColor = disc >= 0 ? COL.positive : COL.negative;
    const barX = pad.l + pw * 0.5;

    ctx.fillStyle = barColor;
    if (disc >= 0) {
        ctx.fillRect(barX, barY - barH / 2, barW, barH);
    } else {
        ctx.fillRect(barX - barW, barY - barH / 2, barW, barH);
    }
    label(ctx, `${SYM.Delta} = ${disc.toFixed(1)}`, barX + (disc >= 0 ? barW + 5 : -barW - 5),
          barY, disc >= 0 ? COL.positive : COL.negative, FONT_VALUE,
          disc >= 0 ? 'left' : 'right');
}
