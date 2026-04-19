/**
 * Panel 4: Von Neumann Chain (measurement cascade terminates).
 *
 * Extracted from consciousness-pedagogy.js (ticket CP-3).
 */

import { K_CRIT, COEFFICIENT } from '../../constants.js';
import {
    COL, SYM, FONT_LABEL, FONT_VALUE,
    label, ring, clamp,
} from '../canvas-primitives.js';
import { discriminant } from './math-helpers.js';

export function drawVonNeumannChain(ctx, w, h, state) {
    const numLinks = 18;
    const pad = { l: 30, r: 30, t: 25, b: 60 };
    const chainY = pad.t + 35;
    const pw = w - pad.l - pad.r;
    const spacing = pw / (numLinks - 1);
    const radius = Math.min(10, spacing * 0.35);

    // ── Compute k and Δ for each link ────────────────────────────
    const ks = [];
    const deltas = [];
    for (let i = 0; i < numLinks; i++) {
        const k_i = COEFFICIENT - i * (COEFFICIENT - K_CRIT) / (numLinks - 1);
        ks.push(k_i);
        deltas.push(discriminant(k_i));
    }

    // Find Δ range for color mapping
    const deltaMax = Math.max(...deltas.map(d => Math.abs(d)));

    // ── Connecting lines ─────────────────────────────────────────
    ctx.strokeStyle = COL.textMuted;
    ctx.lineWidth = 1;
    for (let i = 0; i < numLinks - 1; i++) {
        const x0 = pad.l + i * spacing + radius;
        const x1 = pad.l + (i + 1) * spacing - radius;
        ctx.beginPath();
        ctx.moveTo(x0, chainY);
        ctx.lineTo(x1, chainY);
        ctx.stroke();
    }

    // ── Pulse glow ───────────────────────────────────────────────
    const pulsePos = state._chainProgress * (numLinks - 1);

    // ── Draw links ───────────────────────────────────────────────
    for (let i = 0; i < numLinks; i++) {
        const cx = pad.l + i * spacing;
        const delta = deltas[i];

        // Color: green (high Δ) → yellow → red (small Δ) → white (Δ≈0)
        let color;
        if (delta > 0) {
            const t = clamp(delta / (deltaMax || 1), 0, 1);
            if (t > 0.5) {
                // green to yellow
                const u = (t - 0.5) * 2;
                const r = Math.round(74 + (251 - 74) * (1 - u));
                const g = Math.round(222 + (191 - 222) * (1 - u));
                const b = Math.round(128 * (1 - u) + 36 * (1 - u));
                color = `rgb(${r}, ${g}, ${b})`;
            } else {
                // yellow to red
                const u = t * 2;
                const r = Math.round(248 + (251 - 248) * u);
                const g = Math.round(113 + (191 - 113) * u);
                const b = Math.round(113 * (1 - u) + 36 * u);
                color = `rgb(${r}, ${g}, ${b})`;
            }
        } else {
            color = '#ffffff';
        }

        // Pulse glow
        const dist = Math.abs(i - pulsePos);
        const glowAlpha = Math.max(0, 1 - dist / 2);

        if (glowAlpha > 0) {
            ctx.save();
            ctx.shadowColor = COL.primary;
            ctx.shadowBlur = 12 * glowAlpha;
            ctx.fillStyle = COL.primary;
            ctx.beginPath();
            ctx.arc(cx, chainY, radius + 2, 0, 2 * Math.PI);
            ctx.fill();
            ctx.restore();
        }

        // Circle fill
        ctx.fillStyle = color;
        ctx.beginPath();
        ctx.arc(cx, chainY, radius, 0, 2 * Math.PI);
        ctx.fill();

        // Stroke
        ctx.strokeStyle = i === numLinks - 1 ? COL.gold : COL.textMuted;
        ctx.lineWidth = i === numLinks - 1 ? 2.5 : 1;
        ctx.beginPath();
        ctx.arc(cx, chainY, radius, 0, 2 * Math.PI);
        ctx.stroke();

        // Double circle for final link
        if (i === numLinks - 1) {
            ring(ctx, cx, chainY, radius + 4, COL.gold, 1.5);
            label(ctx, `k_meas = 4/G* ${SYM.approx} ${K_CRIT.toFixed(3)}`,
                  cx, chainY - radius - 10, COL.gold, FONT_VALUE, 'center');
        }
    }

    // ── Category labels ──────────────────────────────────────────
    const categories = ['Structural', 'Algebraic', 'Self-referential', 'Discriminant'];
    const catY = chainY + radius + 22;
    for (let i = 0; i < categories.length; i++) {
        const cx = pad.l + (i / (categories.length - 1)) * pw;
        label(ctx, categories[i], cx, catY, COL.textMuted, FONT_LABEL, 'center');
    }

    // ── Mini discriminant bar chart ──────────────────────────────
    const barBaseY = h - pad.b + 20;
    const barMaxH = 25;
    const barW = Math.max(2, (pw / numLinks) - 2);

    for (let i = 0; i < numLinks; i++) {
        const cx = pad.l + i * spacing;
        const delta = deltas[i];
        const normD = clamp(delta / (deltaMax || 1), 0, 1);
        const bh = normD * barMaxH;

        // Color gradient matching the circles
        let barCol;
        if (delta > 0) {
            const t = normD;
            if (t > 0.3) barCol = COL.positive;
            else if (t > 0.05) barCol = COL.warning;
            else barCol = COL.negative;
        } else {
            barCol = COL.textMuted;
        }

        ctx.fillStyle = barCol;
        ctx.fillRect(cx - barW / 2, barBaseY - bh, barW, bh);
    }

    // Bar chart label
    label(ctx, `${SYM.Delta}_k`, pad.l - 20, barBaseY - barMaxH / 2,
          COL.textSec, FONT_LABEL, 'right');
}
