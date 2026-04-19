/**
 * Panel 1: Complex Plane (consciousness roots on K_C circle).
 *
 * Extracted from consciousness-pedagogy.js (ticket CP-3).
 */

import { Y_REAL, Y_IMAG, K_C, THETA_C_RAD, THETA_C_DEG } from '../../constants.js';
import {
    COL, SYM, FONT_LABEL, FONT_VALUE,
    dashedLine, dot, label, ring,
} from '../canvas-primitives.js';

export function drawComplexPlane(ctx, w, h, state) {
    const cx = w / 2;
    const cy = h / 2;
    const pad = 30;
    const scale = (Math.min(w, h) - 2 * pad) / (2 * K_C * 1.3);

    // ── Axes ─────────────────────────────────────────────────────
    ctx.strokeStyle = COL.textMuted;
    ctx.lineWidth = 1;

    // Re axis
    ctx.beginPath();
    ctx.moveTo(pad, cy);
    ctx.lineTo(w - pad, cy);
    ctx.stroke();

    // Im axis
    ctx.beginPath();
    ctx.moveTo(cx, pad);
    ctx.lineTo(cx, h - pad);
    ctx.stroke();

    label(ctx, 'Re', w - pad + 3, cy - 8, COL.textSec, FONT_LABEL);
    label(ctx, 'Im', cx + 8, pad - 3, COL.textSec, FONT_LABEL);

    // ── K_C circle (dashed) ──────────────────────────────────────
    ctx.save();
    ctx.strokeStyle = COL.gold;
    ctx.lineWidth = 1.5;
    ctx.setLineDash([5, 4]);
    ctx.beginPath();
    ctx.arc(cx, cy, K_C * scale, 0, 2 * Math.PI);
    ctx.stroke();
    ctx.restore();

    label(ctx, `K_C = ${K_C.toFixed(3)}`, cx + K_C * scale + 5, cy - K_C * scale * 0.3,
          COL.gold, FONT_VALUE);

    // ── Rotating phase vector ────────────────────────────────────
    // The roots are at (Y_REAL, ±Y_IMAG). Animation traces K_C circle.
    const theta = state._phaseAngle;
    const tipX = K_C * Math.cos(theta);
    const tipY = K_C * Math.sin(theta);
    const tipPx = cx + tipX * scale;
    const tipPy = cy - tipY * scale;

    // Vector from origin
    ctx.strokeStyle = COL.primary;
    ctx.lineWidth = 2;
    ctx.beginPath();
    ctx.moveTo(cx, cy);
    ctx.lineTo(tipPx, tipPy);
    ctx.stroke();
    dot(ctx, tipPx, tipPy, 4, COL.primary);

    // ── Angle arc from positive x-axis to theta_C ────────────────
    const arcR = 30;
    ctx.strokeStyle = COL.warning;
    ctx.lineWidth = 1.5;
    ctx.beginPath();
    ctx.arc(cx, cy, arcR, 0, -THETA_C_RAD, true); // canvas y is inverted
    ctx.stroke();

    label(ctx, `${SYM.theta}_C = ${THETA_C_DEG.toFixed(1)}\u00B0`,
          cx + arcR + 5, cy - arcR * 0.5, COL.warning, FONT_VALUE);

    // ── Fixed consciousness root marker ──────────────────────────
    const rootPx = cx + Y_REAL * scale;
    const rootPyPos = cy - Y_IMAG * scale;
    const rootPyNeg = cy + Y_IMAG * scale;

    // Filled circle at (Y_REAL, Y_IMAG)
    dot(ctx, rootPx, rootPyPos, 5, COL.secondary);
    label(ctx, `y = ${Y_REAL.toFixed(3)} + ${Y_IMAG.toFixed(3)}i`,
          rootPx + 8, rootPyPos - 5, COL.secondary, FONT_VALUE);

    // Hollow circle at conjugate (Y_REAL, -Y_IMAG)
    ring(ctx, rootPx, rootPyNeg, 5, COL.secondary);
    label(ctx, `y\u0304`, rootPx + 8, rootPyNeg, COL.textMuted, FONT_VALUE);

    // ── Projections from fixed root ──────────────────────────────
    // Horizontal dashed to Re axis
    dashedLine(ctx, rootPx, rootPyPos, rootPx, cy, COL.textMuted);
    // Vertical dashed to Im axis
    dashedLine(ctx, rootPx, rootPyPos, cx, rootPyPos, COL.textMuted);

    // Re projection label
    dot(ctx, rootPx, cy, 3, COL.positive);
    label(ctx, `E(y) = ${Y_REAL.toFixed(3)}`, rootPx + 5, cy + 14, COL.positive, FONT_VALUE);

    // cos/sin annotations
    label(ctx, `cos ${SYM.theta}_C = ${Math.cos(THETA_C_RAD).toFixed(3)}`,
          pad + 5, h - pad - 20, COL.textSec, FONT_VALUE);
    label(ctx, `sin ${SYM.theta}_C = ${Math.sin(THETA_C_RAD).toFixed(3)}`,
          pad + 5, h - pad - 6, COL.textSec, FONT_VALUE);
}
