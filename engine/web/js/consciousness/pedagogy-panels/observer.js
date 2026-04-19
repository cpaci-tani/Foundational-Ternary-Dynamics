/**
 * Panel 5: Observer Boundary (Ring 0 clock + causal past expansion).
 *
 * Extracted from consciousness-pedagogy.js (ticket CP-3).
 */

import { K_B, TICK_PHASES } from '../../constants.js';
import {
    COL, FONT_LABEL, FONT_VALUE, FONT_TITLE,
    label, clamp,
} from '../canvas-primitives.js';

export function drawObserverBoundary(ctx, w, h, state) {
    const gridSize = 16;
    const sidebarW = w * 0.2;
    const gridArea = w - sidebarW;
    const cellSize = Math.min(
        (gridArea - 20) / gridSize,
        (h - 50) / gridSize,
    );
    const gridOffX = (gridArea - cellSize * gridSize) / 2;
    const gridOffY = 30;

    // ── Title ────────────────────────────────────────────────────
    label(ctx, 'Ring 0 Clock: The tick IS the computation',
          gridArea / 2, 12, COL.primary, FONT_TITLE, 'center');

    // ── Grid ─────────────────────────────────────────────────────
    const observerI = 8;
    const observerJ = 8;
    const tick = state._boundaryTick;

    for (let i = 0; i < gridSize; i++) {
        for (let j = 0; j < gridSize; j++) {
            const px = gridOffX + j * cellSize;
            const py = gridOffY + i * cellSize;

            // Chebyshev distance from observer
            const dist = Math.max(Math.abs(i - observerI), Math.abs(j - observerJ));
            const inCausal = dist <= tick;
            const onBoundary = dist === tick && tick > 0;

            // Cell color
            if (i === observerI && j === observerJ) {
                ctx.fillStyle = COL.gold;
            } else if (inCausal) {
                const stateVal = state._grid[i][j];
                if (stateVal === 1)       ctx.fillStyle = COL.accent;    // +1 blue
                else if (stateVal === -1)  ctx.fillStyle = COL.negative; // -1 red
                else                    ctx.fillStyle = '#1e293b';    // 0 void (shouldn't happen in causal past)
            } else {
                ctx.fillStyle = COL.bgDeep;
            }

            ctx.fillRect(px, py, cellSize - 1, cellSize - 1);

            // Boundary glow
            if (onBoundary) {
                ctx.strokeStyle = COL.primary;
                ctx.lineWidth = 1.5;
                ctx.strokeRect(px, py, cellSize - 1, cellSize - 1);
            }
        }
    }

    // ── ReLU sidebar ─────────────────────────────────────────────
    const sbX = gridArea + 5;
    const sbW = sidebarW - 15;
    const sbTop = gridOffY;
    const sbH = cellSize * gridSize;

    label(ctx, 'ReLU Filter', sbX + sbW / 2, sbTop - 8, COL.textSec, FONT_LABEL, 'center');

    // Background
    ctx.fillStyle = COL.bgSurface;
    ctx.fillRect(sbX, sbTop, sbW, sbH);

    // Random incoming flux (varies per frame slowly)
    const fluxValue = 0.3 + 0.5 * Math.sin(state._frameCount * 0.02) *
                      Math.sin(state._frameCount * 0.007 + 1);
    const normFlux = clamp(fluxValue, 0, 1);

    // Threshold line at K_B / max_scale
    const threshNorm = K_B; // K_B ≈ 0.511 works well as normalized threshold
    const threshY = sbTop + sbH * (1 - threshNorm);

    // Flux bar
    const barTop = sbTop + sbH * (1 - normFlux);
    const passesThreshold = normFlux > threshNorm;

    if (passesThreshold) {
        // Show the part above threshold in color, below in dark
        const aboveH = sbTop + sbH - threshY;
        ctx.fillStyle = 'rgba(0, 229, 255, 0.3)';
        ctx.fillRect(sbX + 2, threshY, sbW - 4, aboveH);

        ctx.fillStyle = COL.primary;
        ctx.fillRect(sbX + 2, barTop, sbW - 4, sbTop + sbH - barTop);
    } else {
        // Below threshold — zeroed
        ctx.fillStyle = 'rgba(107, 114, 128, 0.2)';
        ctx.fillRect(sbX + 2, barTop, sbW - 4, sbTop + sbH - barTop);
    }

    // Threshold line
    ctx.strokeStyle = COL.warning;
    ctx.lineWidth = 1.5;
    ctx.beginPath();
    ctx.moveTo(sbX, threshY);
    ctx.lineTo(sbX + sbW, threshY);
    ctx.stroke();

    label(ctx, `K_B = ${K_B}`, sbX + sbW / 2, threshY - 8, COL.warning, FONT_VALUE, 'center');
    label(ctx, 'max(0, |J|\u2212K_B)', sbX + sbW / 2, sbTop + sbH + 12,
          COL.textMuted, FONT_VALUE, 'center');

    // ── Bottom: tick counter + phase ─────────────────────────────
    const phaseIdx = tick % TICK_PHASES.length;
    const phase = TICK_PHASES[phaseIdx];
    const bottomY = gridOffY + cellSize * gridSize + 8;

    label(ctx, `Tick: ${tick}`, gridOffX, bottomY + 12, COL.textPri, FONT_VALUE);
    label(ctx, phase, gridOffX + 60, bottomY + 12, COL.glow, FONT_VALUE);
}
