/**
 * Panel 2: Existence Filter (four-level hierarchy + interactive complex plane).
 *
 * Extracted from consciousness-pedagogy.js (ticket CP-3).
 */

import {
    COL, SYM, FONT_LABEL, FONT_VALUE,
    dashedLine, dot, label, ring,
} from '../canvas-primitives.js';

export function drawExistenceFilter(ctx, w, h, state) {
    const divX = w * 0.4;
    drawFilterHierarchy(ctx, divX, h);
    drawFilterPlane(ctx, divX, w, h, state);
}

function drawFilterHierarchy(ctx, boxW, h) {
    const levels = [
        { label: 'First Distinction',  detail: 's \u2208 {-1, 0, +1}', color: COL.accent },
        { label: 'Magnitude',          detail: `|x| ${SYM.geq || '>'} K_C`,   color: COL.positive },
        { label: 'Born Rule',          detail: `P = E(x)${SYM.sup2} + E(ix)${SYM.sup2}`, color: COL.gold },
        { label: 'Collapse',           detail: 'ReLU crystallization', color: COL.negative },
    ];

    const padY = 25;
    const boxH = (h - 2 * padY) / levels.length;
    const bw = boxW - 30;
    const bx = 15;

    for (let i = 0; i < levels.length; i++) {
        const ly = padY + i * boxH;
        const lev = levels[i];

        // Box
        ctx.strokeStyle = lev.color;
        ctx.lineWidth = 1.5;
        ctx.fillStyle = 'rgba(40, 53, 72, 0.6)';
        const rh = boxH * 0.6;
        const ry = ly + (boxH - rh) / 2;
        ctx.fillRect(bx, ry, bw, rh);
        ctx.strokeRect(bx, ry, bw, rh);

        // Level number
        label(ctx, `L${i === 0 ? '-1' : i === 1 ? '0' : i === 2 ? '0.5' : '1'}`,
              bx + 5, ry + rh / 2 - 8, COL.textMuted, FONT_VALUE);
        // Name
        label(ctx, lev.label, bx + 5, ry + rh / 2 + 2, lev.color, FONT_LABEL);
        // Detail
        label(ctx, lev.detail, bx + 5, ry + rh / 2 + 14, COL.textSec, FONT_VALUE);

        // Arrow to next level
        if (i < levels.length - 1) {
            const ax = bx + bw / 2;
            const ay0 = ry + rh;
            const ay1 = ry + boxH + (boxH - rh) / 2;
            ctx.strokeStyle = COL.textMuted;
            ctx.lineWidth = 1;
            ctx.beginPath();
            ctx.moveTo(ax, ay0 + 2);
            ctx.lineTo(ax, ay1 - 2);
            ctx.stroke();
            // Arrowhead
            ctx.fillStyle = COL.textMuted;
            ctx.beginPath();
            ctx.moveTo(ax, ay1 - 2);
            ctx.lineTo(ax - 4, ay1 - 8);
            ctx.lineTo(ax + 4, ay1 - 8);
            ctx.closePath();
            ctx.fill();
        }
    }
}

function drawFilterPlane(ctx, x0, w, h, state) {
    const pw = w - x0;
    const cx = x0 + pw / 2;
    const cy = h / 2;
    const pad = 25;
    const maxR = 5; // max absolute range on re/im axes
    const scale = (Math.min(pw, h) - 2 * pad) / (2 * maxR);

    // Axes
    ctx.strokeStyle = COL.textMuted;
    ctx.lineWidth = 1;
    ctx.beginPath();
    ctx.moveTo(x0 + pad, cy);
    ctx.lineTo(w - pad, cy);
    ctx.stroke();
    ctx.beginPath();
    ctx.moveTo(cx, pad);
    ctx.lineTo(cx, h - pad);
    ctx.stroke();

    label(ctx, 'Re', w - pad + 2, cy - 8, COL.textSec, FONT_LABEL);
    label(ctx, 'Im', cx + 6, pad - 3, COL.textSec, FONT_LABEL);

    // Plot the point z
    const z = state._filterZ;
    const zPx = cx + z.re * scale;
    const zPy = cy - z.im * scale;

    // z filled
    dot(ctx, zPx, zPy, 5, COL.primary);
    label(ctx, `z = ${z.re.toFixed(2)} + ${z.im.toFixed(2)}i`,
          zPx + 8, zPy - 8, COL.primary, FONT_VALUE);

    // Conjugate z-bar (hollow)
    const zBarPy = cy + z.im * scale;
    ring(ctx, zPx, zBarPy, 4, COL.primary);
    label(ctx, `z\u0304`, zPx + 8, zBarPy, COL.textMuted, FONT_VALUE);

    // E(z) = Re(z) = a  — projection on Re axis
    dashedLine(ctx, zPx, zPy, zPx, cy, COL.positive);
    dot(ctx, zPx, cy, 3, COL.positive);
    label(ctx, `E(z) = ${z.re.toFixed(3)}`, zPx + 5, cy + 12, COL.positive, FONT_VALUE);

    // E(iz) = Re(iz) = -b  — projection
    const eiz = -z.im;
    const eizPx = cx + eiz * scale;
    dashedLine(ctx, cx, zPy, eizPx, cy, 'rgba(251, 191, 36, 0.4)');
    dot(ctx, eizPx, cy, 3, COL.warning);
    label(ctx, `E(iz) = ${eiz.toFixed(3)}`, eizPx, cy + 24, COL.warning, FONT_VALUE, 'center');

    // Born rule: P = a² + b²
    const P = z.re * z.re + z.im * z.im;
    label(ctx, `Born rule P = ${P.toFixed(3)}`, x0 + pad + 5, h - 12, COL.gold, FONT_VALUE);
}
