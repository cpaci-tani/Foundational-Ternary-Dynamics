/** Canvas views of a published snapshot. Never calls the physics worker. */
export const fmt = n => n === null || !Number.isFinite(n) ? '—' : Math.abs(n) < 1e-10 ? '0' : n.toPrecision(4);
export const planeAxes = plane => ({ XY: [0, 1, 2], XZ: [0, 2, 1], YZ: [1, 2, 0] })[plane];
export function sliceBlock(analysis, plane, slice, col, row) {
    const axes = planeAxes(plane), xyz = [0, 0, 0]; xyz[axes[0]] = col; xyz[axes[1]] = row; xyz[axes[2]] = slice;
    return { xyz, block: analysis.blocks[(xyz[0] * analysis.side + xyz[1]) * analysis.side + xyz[2]] };
}
function context(canvas, aspect = 1) {
    const width = Math.max(1, Math.round(canvas.clientWidth * Math.min(devicePixelRatio, 2)));
    const height = Math.max(1, Math.round(width / aspect));
    if (canvas.width !== width || canvas.height !== height) { canvas.width = width; canvas.height = height; }
    const ctx = canvas.getContext('2d'); ctx.fillStyle = '#08121c'; ctx.fillRect(0, 0, width, height);
    return ctx;
}
export function renderSlice(canvas, analysis, options, valueOf, signed = false, arrows = false) {
    const ctx = context(canvas), side = analysis.side, cell = canvas.width / side, axes = planeAxes(options.plane);
    const cells = [];
    let max = 0, speedMax = 0, missing = 0;
    for (let row = 0; row < side; row++) for (let col = 0; col < side; col++) {
        const { block } = sliceBlock(analysis, options.plane, options.slice, col, row);
        const value = valueOf(block, axes[2]);
        if (value === null) missing++; else max = Math.max(max, Math.abs(value));
        if (block.velocity) speedMax = Math.max(speedMax, Math.hypot(block.velocity[axes[0]], block.velocity[axes[1]]));
        cells.push({ block, value, col, row });
    }
    for (const { block, value, col, row } of cells) {
        const x = col * cell, y = (side - 1 - row) * cell;
        if (value === null) {
            ctx.fillStyle = '#1a2533'; ctx.fillRect(x, y, cell, cell);
            ctx.strokeStyle = '#68778a'; ctx.lineWidth = 1; ctx.beginPath();
            ctx.moveTo(x + cell * .2, y + cell * .8); ctx.lineTo(x + cell * .8, y + cell * .2); ctx.stroke();
        } else {
            const t = max ? Math.abs(value) / max : 0;
            ctx.fillStyle = signed ? `hsl(${value >= 0 ? 29 : 207} 65% ${12 + 52 * t}%)` : `hsl(${170 - 15 * t} ${36 + 25 * t}% ${10 + 48 * t}%)`;
            ctx.fillRect(x, y, Math.ceil(cell), Math.ceil(cell));
        }
        if (arrows && block.velocity && speedMax > 0) {
            const vx = block.velocity[axes[0]], vy = block.velocity[axes[1]];
            if (!vx && !vy) continue;
            const scale = cell * .39 / speedMax, cx = x + cell / 2, cy = y + cell / 2;
            const ex = cx + vx * scale, ey = cy - vy * scale, angle = Math.atan2(ey - cy, ex - cx), head = cell * .13;
            ctx.lineWidth = Math.max(1, cell * .04); ctx.strokeStyle = '#fff1c8';
            ctx.beginPath(); ctx.moveTo(cx, cy); ctx.lineTo(ex, ey);
            ctx.moveTo(ex - head * Math.cos(angle - .5), ey - head * Math.sin(angle - .5)); ctx.lineTo(ex, ey);
            ctx.lineTo(ex - head * Math.cos(angle + .5), ey - head * Math.sin(angle + .5)); ctx.stroke();
        }
    }
    // Axes and legends are mathematical coordinates (upwards vertical axis).
    ctx.font = `${Math.max(10, canvas.width / 35)}px system-ui`; ctx.fillStyle = '#fff';
    ctx.fillText(`${'XYZ'[axes[0]]} →   ${'XYZ'[axes[1]]} ↑`, 8, canvas.height - 9);
    return { max, speedMax, missing, range: `${signed ? fmt(-max) : '0'} … ${fmt(max)}` };
}
export function renderChart(canvas, history) {
    const ctx = context(canvas, 1.85), w = canvas.width, h = canvas.height, m = 30;
    const max = history.reduce((s, v) => Number.isFinite(v.amplitude) ? Math.max(s, v.amplitude) : s, 0);
    const lo = history[0]?.cycle || 0, hi = history.at(-1)?.cycle || lo + 1;
    ctx.strokeStyle = '#395069'; ctx.beginPath(); ctx.moveTo(m, 10); ctx.lineTo(m, h - m); ctx.lineTo(w - 10, h - m); ctx.stroke();
    ctx.strokeStyle = '#a4e8d4'; ctx.lineWidth = 2; ctx.beginPath(); let connected = false;
    for (const point of history) {
        if (point.amplitude === null || !Number.isFinite(point.amplitude)) { connected = false; continue; }
        const x = m + (point.cycle - lo) / Math.max(1, hi - lo) * (w - m - 10), y = h - m - (max ? point.amplitude / max : 0) * (h - m - 15);
        if (connected) ctx.lineTo(x, y); else ctx.moveTo(x, y);
        connected = true;
    }
    ctx.stroke(); ctx.fillStyle = '#b1c1d3'; ctx.font = '11px system-ui';
    ctx.fillText(String(lo), m, h - 8); ctx.fillText(String(hi), w - 40, h - 8); ctx.fillText(fmt(max), m + 5, 18);
}
