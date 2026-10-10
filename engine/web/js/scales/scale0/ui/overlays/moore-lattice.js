import { formatEnergy, spectrumColor } from '../../../../ui/components/live-rulers/measure.js';

const STATE_COLOR = {
    positive: [61, 222, 122],
    negative: [255, 93, 93],
    'locked positive': [125, 255, 168],
    'locked negative': [255, 176, 176],
    voxel: [138, 160, 184],
};

const FACES = [
    { corners: [[1, 1, 1], [1, -1, 1], [-1, -1, 1], [-1, 1, 1]] },
    { corners: [[-1, 1, -1], [-1, -1, -1], [1, -1, -1], [1, 1, -1]] },
    { corners: [[1, 1, -1], [1, 1, 1], [-1, 1, 1], [-1, 1, -1]] },
    { corners: [[-1, -1, -1], [-1, -1, 1], [1, -1, 1], [1, -1, -1]] },
    { corners: [[1, 1, -1], [1, -1, -1], [1, -1, 1], [1, 1, 1]] },
    { corners: [[-1, 1, 1], [-1, -1, 1], [-1, -1, -1], [-1, 1, -1]] },
];

function stateName(site) {
    const name = site?.name;
    if (!name || name === 'voxel') return 'voxel';
    return name;
}

function stateColor(name, energy, peak) {
    const rgb = STATE_COLOR[name] || STATE_COLOR.voxel;
    const gain = peak > 0 ? 0.45 + 0.55 * Math.min(1, Math.max(0, energy) / peak) : 0.7;
    return `rgba(${rgb[0]}, ${rgb[1]}, ${rgb[2]}, ${(name === 'voxel' ? 0.28 : 0.9) * gain})`;
}

function heatRGB(t) {
    const u = Math.min(1, Math.max(0, t));
    const r = Math.round(u < 0.5 ? 30 + 80 * (u / 0.5) : 110 + 145 * ((u - 0.5) / 0.5));
    const g = Math.round(u < 0.35 ? 24 + 40 * (u / 0.35) : u < 0.75 ? 64 + 160 * ((u - 0.35) / 0.4) : 224 - 80 * ((u - 0.75) / 0.25));
    const b = Math.round(u < 0.4 ? 90 + 80 * (1 - u / 0.4) : 40 * (1 - (u - 0.4) / 0.6));
    return [r, g, b];
}

function heatColor(t) {
    const [r, g, b] = heatRGB(t);
    return `rgba(${r}, ${g}, ${b}, 0.92)`;
}

function divergeRGB(t) {
    const u = Math.min(1, Math.max(-1, t));
    if (u >= 0) {
        return [Math.round(80 + 175 * u), Math.round(90 - 40 * u), Math.round(110 - 70 * u)];
    }
    const v = -u;
    return [Math.round(70 - 30 * v), Math.round(110 - 20 * v), Math.round(140 + 90 * v)];
}

function divergeColor(t) {
    const [r, g, b] = divergeRGB(t);
    return `rgba(${r}, ${g}, ${b}, 0.92)`;
}

function compactNumber(value) {
    if (!Number.isFinite(value)) return '—';
    if (value === 0) return '0';
    const abs = Math.abs(value);
    if (abs >= 1000 || abs < 0.01) return value.toExponential(3);
    return value.toFixed(3);
}

const LINK_STEPS = [
    [1, 0, 0], [0, 1, 0], [0, 0, 1],
    [1, 1, 0], [1, -1, 0], [1, 0, 1], [1, 0, -1], [0, 1, 1], [0, 1, -1],
];

function linksFromCells(cells) {
    const byKey = new Map(cells.map((cell) => [cell.key, cell]));
    const links = [];
    let max = 0;
    for (const cell of cells) {
        if (!cell.present) continue;
        const [ox, oy, oz] = cell.offset;
        for (const [dx, dy, dz] of LINK_STEPS) {
            const nx = ox + dx;
            const ny = oy + dy;
            const nz = oz + dz;
            if (Math.abs(nx) > 1 || Math.abs(ny) > 1 || Math.abs(nz) > 1) continue;
            const other = byKey.get(`${nx},${ny},${nz}`);
            if (!other?.present) continue;
            const value = cell.joules - other.joules;
            if (!value) continue;
            max = Math.max(max, Math.abs(value));
            links.push({ ox, oy, oz, dx, dy, dz, value });
        }
    }
    const floor = max * 0.05;
    return {
        links: links.filter((link) => Math.abs(link.value) >= floor),
        max,
        source: 'neighborhood',
    };
}

const HEAT_MODES = {
    state: { label: 'state' },
    energy: { label: 'energy', key: 'joules', unit: 'energy' },
    flux: { label: 'flux |J|', key: 'flux' },
    activation: { label: 'activation', key: 'activation' },
    gradient: { label: 'energy gradient', key: 'gradient', unit: 'energy' },
    contrast: { label: 'contrast', key: 'contrast', unit: 'energy', signed: true },
};

/** Rotatable 3×3×3 Moore block. Drag orbits it; a click selects one voxel. */
export function createMooreLattice(canvas, readout, modes, fieldModes) {
    const ctx = canvas.getContext('2d');
    let yaw = -0.62;
    let pitch = 0.48;
    let cells = [];
    let selected = '0,0,0';
    let drag = null;
    let mode = 'state';
    let style = 'heat';
    let linkPack = null;
    let zoom = 1;
    let showCubes = true;
    let heatSpans = { joules: 1, flux: 1, activation: 1, gradient: 1, contrast: 1, curl: 1 };

    const project = (x, y, z, width, height) => {
        const cy = Math.cos(yaw);
        const sy = Math.sin(yaw);
        const x1 = x * cy + z * sy;
        const z1 = -x * sy + z * cy;
        const cp = Math.cos(pitch);
        const sp = Math.sin(pitch);
        const y1 = y * cp - z1 * sp;
        const z2 = y * sp + z1 * cp;
        const scale = height * 0.18 * zoom;
        const perspective = 7.5 / (7.5 - z2);
        return {
            x: width / 2 + x1 * scale * perspective,
            y: height / 2 - y1 * scale * perspective,
            z: z2,
        };
    };

    // CSS size of the canvas, kept current by the ResizeObserver below. Read
    // from layout inside draw(), it forced a reflow on every snapshot, just
    // after the panel had rewritten its table.
    let cssWidth = 0;
    let cssHeight = 0;
    let measured = false;

    const layout = () => {
        if (!measured) {
            const rect = canvas.getBoundingClientRect();
            cssWidth = rect.width;
            cssHeight = rect.height;
            measured = true;
        }
        const dpr = Math.min(2, window.devicePixelRatio || 1);
        const width = Math.max(1, Math.round(cssWidth * dpr));
        const height = Math.max(1, Math.round(cssHeight * dpr));
        if (canvas.width !== width || canvas.height !== height) {
            canvas.width = width;
            canvas.height = height;
        }
        return { width, height };
    };

    const drawAxes = (width, height) => {
        const origin = [-1.7, -1.7, -1.7];
        const axes = [
            { dir: [3.5, 0, 0], color: '#ff6b6b', label: 'X' },
            { dir: [0, 3.5, 0], color: '#5ddea0', label: 'Y' },
            { dir: [0, 0, 3.5], color: '#7eb6ff', label: 'Z' },
        ];
        const from = project(origin[0], origin[1], origin[2], width, height);
        ctx.lineWidth = Math.max(1.5, height * 0.006);
        ctx.font = `${Math.max(12, height * 0.055)}px sans-serif`;
        ctx.textAlign = 'center';
        ctx.textBaseline = 'middle';
        for (const axis of axes) {
            const tip = [
                origin[0] + axis.dir[0],
                origin[1] + axis.dir[1],
                origin[2] + axis.dir[2],
            ];
            const to = project(tip[0], tip[1], tip[2], width, height);
            ctx.strokeStyle = axis.color;
            ctx.fillStyle = axis.color;
            ctx.beginPath();
            ctx.moveTo(from.x, from.y);
            ctx.lineTo(to.x, to.y);
            ctx.stroke();
            const dx = to.x - from.x;
            const dy = to.y - from.y;
            const len = Math.hypot(dx, dy) || 1;
            const ux = dx / len;
            const uy = dy / len;
            const head = Math.max(8, height * 0.035);
            ctx.beginPath();
            ctx.moveTo(to.x, to.y);
            ctx.lineTo(to.x - ux * head - uy * head * 0.45, to.y - uy * head + ux * head * 0.45);
            ctx.lineTo(to.x - ux * head + uy * head * 0.45, to.y - uy * head - ux * head * 0.45);
            ctx.closePath();
            ctx.fill();
            ctx.fillText(axis.label, to.x + ux * head * 0.8, to.y + uy * head * 0.8);
        }
    };

    const VOXEL = 0.16;

    const activeVector = (cell) => {
        if (style === 'curl') return cell.curl;
        if (mode === 'flux') return cell.grad.flux;
        if (mode === 'activation') return cell.grad.activation;
        return cell.grad.joules;
    };

    const drawLattice = (width, height) => {
        ctx.strokeStyle = 'rgba(180, 198, 214, 0.28)';
        ctx.lineWidth = Math.max(1, height * 0.003);
        const seen = new Set();
        for (const cell of cells) {
            const [ox, oy, oz] = cell.offset;
            const steps = [[1, 0, 0], [0, 1, 0], [0, 0, 1]];
            for (const [dx, dy, dz] of steps) {
                if (ox + dx > 1 || oy + dy > 1 || oz + dz > 1) continue;
                const id = `${ox},${oy},${oz}:${dx}${dy}${dz}`;
                if (seen.has(id)) continue;
                seen.add(id);
                const a = project(ox, oy, oz, width, height);
                const b = project(ox + dx, oy + dy, oz + dz, width, height);
                ctx.beginPath();
                ctx.moveTo(a.x, a.y);
                ctx.lineTo(b.x, b.y);
                ctx.stroke();
            }
        }
    };

    const drawArrow = (origin, vector, length, color, width, height, tipColor) => {
        const mag = Math.hypot(vector.x, vector.y, vector.z);
        if (!(mag > 0) || !(length > 0)) return;
        const tip = [
            origin[0] + (vector.x / mag) * length,
            origin[1] + (vector.y / mag) * length,
            origin[2] + (vector.z / mag) * length,
        ];
        const from = project(origin[0], origin[1], origin[2], width, height);
        const to = project(tip[0], tip[1], tip[2], width, height);
        if (tipColor) {
            const paint = ctx.createLinearGradient(from.x, from.y, to.x, to.y);
            paint.addColorStop(0, color);
            paint.addColorStop(1, tipColor);
            ctx.strokeStyle = paint;
        } else {
            ctx.strokeStyle = color;
        }
        ctx.fillStyle = tipColor || color;
        ctx.lineWidth = Math.max(1.5, height * 0.005);
        ctx.beginPath();
        ctx.moveTo(from.x, from.y);
        ctx.lineTo(to.x, to.y);
        ctx.stroke();
        const dx = to.x - from.x;
        const dy = to.y - from.y;
        const len = Math.hypot(dx, dy) || 1;
        const ux = dx / len;
        const uy = dy / len;
        const head = Math.max(6, height * 0.022);
        ctx.beginPath();
        ctx.moveTo(to.x, to.y);
        ctx.lineTo(to.x - ux * head - uy * head * 0.45, to.y - uy * head + ux * head * 0.45);
        ctx.lineTo(to.x - ux * head + uy * head * 0.45, to.y - uy * head - ux * head * 0.45);
        ctx.closePath();
        ctx.fill();
    };

    const drawCurl = (origin, vector, radius, color, width, height) => {
        const mag = Math.hypot(vector.x, vector.y, vector.z);
        if (!(mag > 0) || !(radius > 0)) return;
        const axis = [vector.x / mag, vector.y / mag, vector.z / mag];
        const helper = Math.abs(axis[1]) < 0.85 ? [0, 1, 0] : [1, 0, 0];
        let ux = axis[1] * helper[2] - axis[2] * helper[1];
        let uy = axis[2] * helper[0] - axis[0] * helper[2];
        let uz = axis[0] * helper[1] - axis[1] * helper[0];
        const ulen = Math.hypot(ux, uy, uz) || 1;
        ux /= ulen;
        uy /= ulen;
        uz /= ulen;
        const vx = axis[1] * uz - axis[2] * uy;
        const vy = axis[2] * ux - axis[0] * uz;
        const vz = axis[0] * uy - axis[1] * ux;
        ctx.strokeStyle = color;
        ctx.fillStyle = color;
        ctx.lineWidth = Math.max(1.5, height * 0.005);
        ctx.beginPath();
        const steps = 18;
        for (let i = 0; i <= steps; i++) {
            const t = (i / steps) * Math.PI * 1.65;
            const point = project(
                origin[0] + (ux * Math.cos(t) + vx * Math.sin(t)) * radius,
                origin[1] + (uy * Math.cos(t) + vy * Math.sin(t)) * radius,
                origin[2] + (uz * Math.cos(t) + vz * Math.sin(t)) * radius,
                width,
                height,
            );
            if (i === 0) ctx.moveTo(point.x, point.y);
            else ctx.lineTo(point.x, point.y);
        }
        ctx.stroke();
        const tipT = Math.PI * 1.65;
        const tip = [
            origin[0] + (ux * Math.cos(tipT) + vx * Math.sin(tipT)) * radius,
            origin[1] + (uy * Math.cos(tipT) + vy * Math.sin(tipT)) * radius,
            origin[2] + (uz * Math.cos(tipT) + vz * Math.sin(tipT)) * radius,
        ];
        const tangent = [
            -ux * Math.sin(tipT) + vx * Math.cos(tipT),
            -uy * Math.sin(tipT) + vy * Math.cos(tipT),
            -uz * Math.sin(tipT) + vz * Math.cos(tipT),
        ];
        drawArrow(tip, { x: tangent[0], y: tangent[1], z: tangent[2] }, radius * 0.45, color, width, height);
    };

    const drawLinkArrows = (width, height) => {
        const pack = linkPack;
        if (!pack?.links?.length) return;
        const peak = pack.max > 0 ? pack.max : 1;
        const byKey = new Map(cells.map((cell) => [cell.key, cell]));
        let low = Infinity;
        let high = -Infinity;
        for (const cell of cells) {
            if (!cell.present) continue;
            low = Math.min(low, cell.joules);
            high = Math.max(high, cell.joules);
        }
        if (!Number.isFinite(low)) low = 0;
        if (!Number.isFinite(high)) high = low;
        const energySpan = high > low ? high - low : 1;
        const phaseColor = (x, y, z) => {
            const x0 = Math.floor(x);
            const y0 = Math.floor(y);
            const z0 = Math.floor(z);
            const x1 = Math.ceil(x);
            const y1 = Math.ceil(y);
            const z1 = Math.ceil(z);
            const joulesAt = (ix, iy, iz) => byKey.get(`${ix},${iy},${iz}`)?.joules;
            const start = joulesAt(x0, y0, z0);
            if (x0 === x1 && y0 === y1 && z0 === z1) {
                return spectrumColor(Number.isFinite(start) ? (start - low) / energySpan : 0.5);
            }
            const stop = joulesAt(x1, y1, z1);
            const along = Math.max(Math.abs(x - x0), Math.abs(y - y0), Math.abs(z - z0));
            const from = Number.isFinite(start) ? start : low;
            const to = Number.isFinite(stop) ? stop : high;
            return spectrumColor((from + (to - from) * along - low) / energySpan);
        };
        for (const link of pack.links) {
            const end = [link.ox + link.dx, link.oy + link.dy, link.oz + link.dz];
            const mid = [
                (link.ox + end[0]) / 2,
                (link.oy + end[1]) / 2,
                (link.oz + end[2]) / 2,
            ];
            const gain = Math.min(1, Math.abs(link.value) / peak);
            const towardNeighbor = link.value > 0;
            const span = Math.max(1.2, height * 0.003) * (0.7 + 1.5 * gain);
            const fromColor = phaseColor(link.ox, link.oy, link.oz);
            const toColor = phaseColor(end[0], end[1], end[2]);
            const stroke = (from, to, startColor, stopColor) => {
                const a = project(from[0], from[1], from[2], width, height);
                const b = project(to[0], to[1], to[2], width, height);
                const paint = ctx.createLinearGradient(a.x, a.y, b.x, b.y);
                paint.addColorStop(0, startColor);
                paint.addColorStop(1, stopColor);
                ctx.strokeStyle = paint;
                ctx.lineWidth = span;
                ctx.lineCap = 'round';
                ctx.beginPath();
                ctx.moveTo(a.x, a.y);
                ctx.lineTo(b.x, b.y);
                ctx.stroke();
            };
            const midColor = phaseColor(mid[0], mid[1], mid[2]);
            stroke([link.ox, link.oy, link.oz], mid, fromColor, midColor);
            stroke(mid, end, midColor, toColor);
            const dir = towardNeighbor
                ? { x: link.dx, y: link.dy, z: link.dz }
                : { x: -link.dx, y: -link.dy, z: -link.dz };
            const tip = towardNeighbor ? end : [link.ox, link.oy, link.oz];
            const mag = Math.hypot(dir.x, dir.y, dir.z) || 1;
            drawArrow([
                tip[0] - (dir.x / mag) * 0.32,
                tip[1] - (dir.y / mag) * 0.32,
                tip[2] - (dir.z / mag) * 0.32,
            ], dir, 0.32, phaseColor(tip[0], tip[1], tip[2]), width, height);
        }
    };

    const draw = () => {
        const { width, height } = layout();
        ctx.clearRect(0, 0, width, height);
        drawLattice(width, height);
        const drawable = [];
        for (const cell of cells) {
            const faces = [];
            for (const face of FACES) {
                const points = face.corners.map((corner) => project(
                    cell.offset[0] + corner[0] * VOXEL,
                    cell.offset[1] + corner[1] * VOXEL,
                    cell.offset[2] + corner[2] * VOXEL,
                    width,
                    height,
                ));
                const ax = points[1].x - points[0].x;
                const ay = points[1].y - points[0].y;
                const bx = points[2].x - points[1].x;
                const by = points[2].y - points[1].y;
                if (ax * by - ay * bx <= 0) continue;
                const depth = points.reduce((sum, point) => sum + point.z, 0) / points.length;
                faces.push({ points, depth });
            }
            if (!faces.length) continue;
            const center = project(cell.offset[0], cell.offset[1], cell.offset[2], width, height);
            drawable.push({
                cell,
                faces,
                center,
                depth: faces.reduce((sum, face) => sum + face.depth, 0) / faces.length,
            });
        }
        drawable.sort((a, b) => a.depth - b.depth);
        if (showCubes) {
            if (style === 'links') {
                ctx.save();
                ctx.globalAlpha = 0.32;
            }
            for (const item of drawable) {
                const selectedCell = item.cell.key === selected;
                ctx.fillStyle = fillFor(item.cell);
                ctx.strokeStyle = selectedCell ? '#f4f7fb' : 'rgba(232, 238, 245, 0.55)';
                ctx.lineWidth = selectedCell ? Math.max(1.6, height * 0.005) : Math.max(1, height * 0.0025);
                for (const face of item.faces) {
                    ctx.beginPath();
                    face.points.forEach((point, index) => {
                        if (index === 0) ctx.moveTo(point.x, point.y);
                        else ctx.lineTo(point.x, point.y);
                    });
                    ctx.closePath();
                    ctx.fill();
                    const lift = face.depth - item.depth;
                    ctx.fillStyle = lift > 0 ? 'rgba(255,255,255,0.16)' : 'rgba(0,0,0,0.22)';
                    ctx.fill();
                    ctx.strokeStyle = selectedCell ? '#f4f7fb' : 'rgba(232, 238, 245, 0.55)';
                    ctx.stroke();
                    ctx.fillStyle = fillFor(item.cell);
                }
            }
            if (style === 'links') ctx.restore();
        } else {
            for (const item of drawable) {
                const selectedCell = item.cell.key === selected;
                ctx.beginPath();
                ctx.fillStyle = selectedCell ? '#f4f7fb' : fillFor(item.cell);
                ctx.arc(item.center.x, item.center.y, selectedCell ? Math.max(3.5, height * 0.012) : Math.max(2, height * 0.006), 0, Math.PI * 2);
                ctx.fill();
            }
        }
        if (style === 'gradient' || style === 'curl') {
            const span = style === 'curl' ? heatSpans.curl : (heatSpans[scalarKey()] || 1);
            const key = scalarKey();
            let low = Infinity;
            let high = -Infinity;
            if (style === 'gradient') {
                for (const item of drawable) {
                    const value = Number(item.cell[key]) || 0;
                    low = Math.min(low, value);
                    high = Math.max(high, value);
                }
            }
            const fieldSpan = high > low ? high - low : 1;
            const phase = (value) => spectrumColor((value - (Number.isFinite(low) ? low : 0)) / fieldSpan);
            for (const item of drawable) {
                const vector = activeVector(item.cell);
                const mag = Math.hypot(vector.x, vector.y, vector.z);
                const gain = span > 0 ? mag / span : 0;
                if (style === 'curl') {
                    drawCurl(item.cell.offset, vector, 0.12 + 0.28 * gain, heatColor(gain), width, height);
                } else {
                    const value = Number(item.cell[key]) || 0;
                    const length = 0.2 + 0.55 * gain;
                    const tipValue = Math.min(high, Math.max(low, value + mag * length));
                    drawArrow(item.cell.offset, vector, length, phase(value), width, height, phase(tipValue));
                }
            }
        }
        if (style === 'links') drawLinkArrows(width, height);
        drawAxes(width, height);
    };

    const scalarKey = () => {
        if (mode === 'flux') return 'flux';
        if (mode === 'activation') return 'activation';
        return 'joules';
    };

    const fillFor = (cell) => {
        if (style !== 'heat') {
            return cell.present ? 'rgba(186, 204, 220, 0.78)' : 'rgba(138, 160, 184, 0.22)';
        }
        if (!cell.present) return 'rgba(138, 160, 184, 0.22)';
        if (mode === 'state') return stateColor(cell.name, cell.joules, cell.peak);
        const spec = HEAT_MODES[mode];
        const value = cell[spec.key];
        const span = heatSpans[spec.key] || 1;
        if (spec.signed) return divergeColor(value / span);
        return heatColor(span > 0 ? value / span : 0);
    };

    const linksAt = (cell) => {
        if (!linkPack?.links) return null;
        const [ox, oy, oz] = cell.offset;
        const touching = [];
        for (const link of linkPack.links) {
            const arrives = link.ox + link.dx === ox && link.oy + link.dy === oy && link.oz + link.dz === oz;
            const leaves = link.ox === ox && link.oy === oy && link.oz === oz;
            if (arrives || leaves) touching.push(link);
        }
        return touching;
    };

    const quantityText = (cell) => {
        if (style === 'links') {
            const touching = linksAt(cell) || [];
            if (!linkPack?.links?.length) return 'no link in this neighborhood';
            if (!touching.length) return 'no link on this voxel';
            let peak = 0;
            for (const link of touching) peak = Math.max(peak, Math.abs(link.value));
            const peakText = linkPack.source === 'neighborhood' ? formatEnergy(peak) : compactNumber(peak);
            return `${touching.length} links · peak ${peakText}`;
        }
        if (style === 'curl') {
            const curl = cell.curl || { x: 0, y: 0, z: 0 };
            return `∇E × ∇|J| (${compactNumber(curl.x)}, ${compactNumber(curl.y)}, ${compactNumber(curl.z)})`;
        }
        if (style === 'gradient') {
            const vector = activeVector(cell);
            if (scalarKey() === 'joules') {
                return `∇ (${formatEnergy(vector.x)}, ${formatEnergy(vector.y)}, ${formatEnergy(vector.z)}) / voxel`;
            }
            return `∇ (${compactNumber(vector.x)}, ${compactNumber(vector.y)}, ${compactNumber(vector.z)}) ${scalarKey()}`;
        }
        const spec = HEAT_MODES[mode];
        if (mode === 'state') return cell.name;
        const value = cell[spec.key];
        if (spec.unit === 'energy') {
            const text = formatEnergy(value);
            return mode === 'gradient' ? `${text} / voxel` : mode === 'contrast' ? `${text} vs mean` : text;
        }
        return `${compactNumber(value)} ${spec.label}`;
    };

    let described = null;
    const describe = (cell) => {
        const place = cell ? `${cell.x}, ${cell.y}, ${cell.z}` : '';
        const text = !cell ? 'Drag to rotate. Scroll to zoom. Click a voxel.'
            : cell.present ? `${place} · ${quantityText(cell)}`
            : `${place} · vacant`;
        if (text === described) return;
        described = text;
        readout.textContent = text;
    };

    const selectAt = (event) => {
        const rect = canvas.getBoundingClientRect();
        const dpr = canvas.width / Math.max(1, rect.width);
        const px = (event.clientX - rect.left) * dpr;
        const py = (event.clientY - rect.top) * dpr;
        const { width, height } = { width: canvas.width, height: canvas.height };
        let best = null;
        let bestScore = Infinity;
        for (const cell of cells) {
            const center = project(cell.offset[0], cell.offset[1], cell.offset[2], width, height);
            const edge = project(cell.offset[0] + 0.34, cell.offset[1], cell.offset[2], width, height);
            const radius = Math.max(8, Math.hypot(edge.x - center.x, edge.y - center.y));
            const score = Math.hypot(px - center.x, py - center.y);
            if (score > radius) continue;
            if (best && (center.z < best.z || (center.z === best.z && score >= bestScore))) continue;
            bestScore = score;
            best = { cell, z: center.z };
        }
        if (!best) return;
        selected = best.cell.key;
        describe(best.cell);
        draw();
    };

    const onPointerDown = (event) => {
        drag = { x: event.clientX, y: event.clientY, moved: 0 };
        canvas.setPointerCapture(event.pointerId);
    };
    const onPointerMove = (event) => {
        if (!drag) return;
        const dx = event.clientX - drag.x;
        const dy = event.clientY - drag.y;
        drag.x = event.clientX;
        drag.y = event.clientY;
        drag.moved += Math.abs(dx) + Math.abs(dy);
        yaw += dx * 0.01;
        pitch = Math.max(-1.15, Math.min(1.15, pitch + dy * 0.01));
        draw();
    };
    const onPointerUp = (event) => {
        const moved = drag?.moved || 0;
        drag = null;
        if (moved < 5) selectAt(event);
    };
    const onMode = (event) => {
        const button = event.target.closest('[data-heat]');
        if (!button || !modes.contains(button)) return;
        mode = button.dataset.heat;
        for (const item of modes.querySelectorAll('[data-heat]')) {
            item.setAttribute('aria-pressed', item === button ? 'true' : 'false');
        }
        describe(cells.find((cell) => cell.key === selected));
        draw();
    };
    const onStyle = (event) => {
        const button = event.target.closest('[data-style]');
        if (!button || !fieldModes?.contains(button)) return;
        style = button.dataset.style;
        for (const item of fieldModes.querySelectorAll('[data-style]')) {
            item.setAttribute('aria-pressed', item === button ? 'true' : 'false');
        }
        describe(cells.find((cell) => cell.key === selected));
        draw();
    };
    const onCubes = (event) => {
        const button = event.target.closest('[data-cubes]');
        if (!button || !fieldModes?.contains(button)) return;
        showCubes = !showCubes;
        button.setAttribute('aria-pressed', showCubes ? 'true' : 'false');
        draw();
    };
    const onWheel = (event) => {
        event.preventDefault();
        const factor = event.deltaY < 0 ? 1.12 : 1 / 1.12;
        zoom = Math.min(5, Math.max(0.35, zoom * factor));
        draw();
    };
    modes?.addEventListener('click', onMode);
    fieldModes?.addEventListener('click', onStyle);
    fieldModes?.addEventListener('click', onCubes);
    canvas.addEventListener('pointerdown', onPointerDown);
    canvas.addEventListener('pointermove', onPointerMove);
    canvas.addEventListener('pointerup', onPointerUp);
    canvas.addEventListener('pointercancel', () => { drag = null; });
    canvas.addEventListener('wheel', onWheel, { passive: false });
    const resize = new ResizeObserver((entries) => {
        // The canvas has no border or padding, so its content box is its box.
        const box = entries?.[entries.length - 1]?.contentRect;
        if (box) {
            cssWidth = box.width;
            cssHeight = box.height;
            measured = true;
        }
        draw();
    });
    resize.observe(canvas);

    return {
        setSnapshot(detail) {
            const focus = detail?.focus || {};
            const fx = Number(focus.x) || 0;
            const fy = Number(focus.y) || 0;
            const fz = Number(focus.z) || 0;
            const byOffset = new Map();
            byOffset.set('0,0,0', {
                x: fx, y: fy, z: fz,
                name: stateName(focus),
                joules: Number(focus.joules) || 0,
                activation: Number(focus.activation) || 0,
                flux: Number(focus.flux) || 0,
                present: true,
            });
            for (const site of detail?.sites || []) {
                const ox = Math.round(site.x - fx);
                const oy = Math.round(site.y - fy);
                const oz = Math.round(site.z - fz);
                if (Math.abs(ox) > 1 || Math.abs(oy) > 1 || Math.abs(oz) > 1) continue;
                byOffset.set(`${ox},${oy},${oz}`, {
                    x: site.x, y: site.y, z: site.z,
                    name: stateName(site),
                    joules: Number(site.joules) || 0,
                    activation: Number(site.activation) || 0,
                    flux: Number(site.flux) || 0,
                    present: true,
                });
            }
            let peak = 0;
            for (const site of byOffset.values()) peak = Math.max(peak, site.joules);
            cells = [];
            for (let oz = -1; oz <= 1; oz++) {
                for (let oy = -1; oy <= 1; oy++) {
                    for (let ox = -1; ox <= 1; ox++) {
                        const key = `${ox},${oy},${oz}`;
                        const site = byOffset.get(key);
                        cells.push({
                            key,
                            offset: [ox, oy, oz],
                            x: site ? site.x : fx + ox,
                            y: site ? site.y : fy + oy,
                            z: site ? site.z : fz + oz,
                            name: site?.name || 'voxel',
                            joules: site?.joules || 0,
                            activation: site?.activation || 0,
                            flux: site?.flux || 0,
                            present: !!site?.present,
                            peak,
                        });
                    }
                }
            }
            const lookup = new Map(cells.map((cell) => [cell.key, cell]));
            const occupied = cells.filter((cell) => cell.present);
            const mean = occupied.reduce((sum, cell) => sum + cell.joules, 0) / Math.max(1, occupied.length);
            const slopeOf = (cell, key, dx, dy, dz) => {
                const [ox, oy, oz] = cell.offset;
                const plus = lookup.get(`${ox + dx},${oy + dy},${oz + dz}`);
                const minus = lookup.get(`${ox - dx},${oy - dy},${oz - dz}`);
                if (plus && minus) return (plus[key] - minus[key]) / 2;
                if (plus) return plus[key] - cell[key];
                if (minus) return cell[key] - minus[key];
                return 0;
            };
            const gradientOf = (cell, key) => ({
                x: slopeOf(cell, key, 1, 0, 0),
                y: slopeOf(cell, key, 0, 1, 0),
                z: slopeOf(cell, key, 0, 0, 1),
            });
            heatSpans = { joules: 0, flux: 0, activation: 0, gradient: 0, contrast: 0, curl: 0 };
            for (const cell of cells) {
                const joules = gradientOf(cell, 'joules');
                const flux = gradientOf(cell, 'flux');
                cell.grad = { joules, flux, activation: gradientOf(cell, 'activation') };
                cell.gradient = Math.hypot(joules.x, joules.y, joules.z);
                cell.curl = {
                    x: joules.y * flux.z - joules.z * flux.y,
                    y: joules.z * flux.x - joules.x * flux.z,
                    z: joules.x * flux.y - joules.y * flux.x,
                };
                cell.contrast = cell.joules - mean;
                heatSpans.joules = Math.max(heatSpans.joules, cell.joules);
                heatSpans.flux = Math.max(heatSpans.flux, cell.flux);
                heatSpans.activation = Math.max(heatSpans.activation, cell.activation);
                heatSpans.gradient = Math.max(heatSpans.gradient, cell.gradient);
                heatSpans.contrast = Math.max(heatSpans.contrast, Math.abs(cell.contrast));
                heatSpans.curl = Math.max(heatSpans.curl, Math.hypot(cell.curl.x, cell.curl.y, cell.curl.z));
            }
            for (const key of Object.keys(heatSpans)) {
                if (!(heatSpans[key] > 0)) heatSpans[key] = 1;
            }
            const engineLinks = detail?.links;
            linkPack = engineLinks?.links?.length
                ? { ...engineLinks, source: 'engine' }
                : linksFromCells(cells);
            if (!cells.some((cell) => cell.key === selected)) selected = '0,0,0';
            // Draw before writing the readout: canvas text makes the browser
            // bring styles up to date, so it should find nothing changed.
            draw();
            describe(cells.find((cell) => cell.key === selected));
        },
        destroy() {
            modes?.removeEventListener('click', onMode);
            fieldModes?.removeEventListener('click', onStyle);
            fieldModes?.removeEventListener('click', onCubes);
            canvas.removeEventListener('pointerdown', onPointerDown);
            canvas.removeEventListener('pointermove', onPointerMove);
            canvas.removeEventListener('pointerup', onPointerUp);
            canvas.removeEventListener('wheel', onWheel);
            resize.disconnect();
        },
    };
}
