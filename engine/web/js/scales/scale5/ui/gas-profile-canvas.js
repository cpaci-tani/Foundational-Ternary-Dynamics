import { resizeCanvasSurface } from '../../../ui/utils/canvas-surface.js';

/** Draw one DPR-aware gas-lab profile row in logical CSS pixels. */
export function drawGasProfileBars(canvas, values, color, mode, edges) {
    const surface = resizeCanvasSurface(canvas);
    if (!surface?.ctx) return;
    const { ctx, width, height } = surface;
    ctx.clearRect(0, 0, width, height);
    if (!values?.length) return;

    const labelHeight = 16;
    const plotHeight = Math.max(1, height - labelHeight);
    let maxAbs = 0;
    for (let index = 0; index < values.length; index++) {
        maxAbs = Math.max(maxAbs, Math.abs(values[index]));
    }
    if (!(maxAbs > 0)) return;

    const barWidth = width / values.length;
    ctx.fillStyle = color;
    if (mode === 'signed') {
        const midY = plotHeight / 2;
        ctx.strokeStyle = '#3a4a6a';
        ctx.lineWidth = 1;
        ctx.beginPath();
        ctx.moveTo(0, midY);
        ctx.lineTo(width, midY);
        ctx.stroke();
        for (let index = 0; index < values.length; index++) {
            const barHeight = (values[index] / maxAbs) * (plotHeight / 2 - 2);
            const y = barHeight >= 0 ? midY - barHeight : midY;
            ctx.fillRect(index * barWidth, y, Math.max(1, barWidth - 1), Math.abs(barHeight));
        }
    } else {
        for (let index = 0; index < values.length; index++) {
            const barHeight = (values[index] / maxAbs) * (plotHeight - 4);
            ctx.fillRect(index * barWidth, plotHeight - barHeight, Math.max(1, barWidth - 1), barHeight);
        }
    }

    if (edges?.length >= 2) {
        ctx.fillStyle = '#8a9bbf';
        ctx.font = '16px sans-serif';
        ctx.textBaseline = 'bottom';
        ctx.textAlign = 'left';
        ctx.fillText(edges[0].toFixed(1), 1, height);
        ctx.textAlign = 'right';
        ctx.fillText(edges[edges.length - 1].toFixed(1), width - 1, height);
    }
}
