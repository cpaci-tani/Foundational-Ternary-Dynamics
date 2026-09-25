import { E_REST, FTD_ELECTRON_PRIMARY_PLANCK_LENGTH_M, J_PER_EV, K_B } from '../../../constants.js';

/**
 * One voxel is the electron-primary lattice spacing a_phys = ℓ_P
 * (FTD-0137 §4.5 / FTD-0041). Lengths below are in metres.
 */
export const VOXEL_LENGTH_M = FTD_ELECTRON_PRIMARY_PLANCK_LENGTH_M;

/** Presentation defaults for the live rulers, energy string, and voxel clocks. */
export const LIVE_MEASURE_DEFAULTS = Object.freeze({
    viewRuler: true,
    latticeRuler: true,
    energyString: true,
    spectrum: true,
    voxelClocks: true,
    mooreBars: true,
    mooreWaves: true,
    mooreJoules: true,
    smoothOrbit: true,
    rulerScale: 1,
    clockSize: 16,
    stringWidth: 148,
    clockRadius: 2.5,
    valueSize: 16,
    lineThickness: 1.5,
    barThickness: 8,
});

/** The lattice mesh stops drawing once the whole cube is under this many pixels. */
export const OUTER_WORLD_PX = 5;

/** Flux point-size slider limits. Screen size follows the flux volume shader. */
export const POINT_SCALE_MIN = 0.1;
export const POINT_SCALE_MAX = 3;

/** Attribute size at full energy for a point-scale slider value. Stride matches the flux writer. */
export function pointAttributeSize(pointScale, stride = 1) {
    const span = Math.min(Math.max(stride, 0), 1.5);
    return 1 + Math.max(POINT_SCALE_MIN, pointScale) * 9 * span;
}

/** Display name for a manifested site. Empty when the voxel has no particle. */
export function manifestedSiteName(kind) {
    if (kind === 1) return 'positive';
    if (kind === -1) return 'negative';
    if (kind === 2) return 'locked positive';
    if (kind === -2) return 'locked negative';
    return '';
}

/** Per-voxel clock in turns, from the published tick and the voxel's own address. No extra field. */
export function voxelClockPhase(tick, x, y, z) {
    const turns = (Number(tick) || 0) / 12 + x * 0.173 + y * 0.317 + z * 0.519;
    return turns - Math.floor(turns);
}

/** gl_PointSize for a flux dot: size * sqrt(60 / depth), clamped like the shader. */
export function pointSpritePixels(size, depth) {
    const d = Math.max(depth, 0.1);
    return Math.min(512, Math.max(1, size * Math.sqrt(60 / d)));
}

/**
 * Voxel activation is sqrt(2 epsilon). Lattice rest energy E_REST = K_B/3
 * is the electron rest energy K_B MeV, so one lattice energy unit is 3 MeV.
 */
export function activationEnergyEv(activation) {
    const amplitude = Number(activation);
    if (!Number.isFinite(amplitude)) return 0;
    const latticeEnergy = amplitude * amplitude * 0.5;
    return latticeEnergy * (K_B / E_REST) * 1e6;
}

/** String path of one voxel's energy. The top of the string is that voxel's max, the bottom its min. */
export function energyStringPath(samples, min, max) {
    const count = samples?.length || 0;
    if (count === 0) return '';
    const span = max - min;
    let path = '';
    for (let i = 0; i < count; i++) {
        const x = count === 1 ? 50 : (i / (count - 1)) * 100;
        const t = span > 0 ? (samples[i] - min) / span : 0.5;
        const y = 14 - Math.min(1, Math.max(0, t)) * 12;
        path += `${i ? 'L' : 'M'}${x.toFixed(2)} ${y.toFixed(2)}`;
    }
    return path;
}

/** Spectrum color from the smallest point (blue) to the largest (red). `t` is 0..1. */
export function spectrumColor(t) {
    const u = Math.min(1, Math.max(0, t));
    const hue = (1 - u) * 240;
    return `hsl(${hue.toFixed(1)} 78% 58%)`;
}

/** Spectrum color from the smallest point scale (blue) to the largest (red). */
export function pointSizeColor(pointScale) {
    const t = (pointScale - POINT_SCALE_MIN) / (POINT_SCALE_MAX - POINT_SCALE_MIN);
    return spectrumColor(t);
}

/** Circumscribed spherical shell: the outer shell around a cubic body of side `domain`. */
export function shellDiameter(domainUnits) {
    return Math.max(1, domainUnits) * Math.sqrt(3);
}

const SUPER = { '-': '⁻', '0': '⁰', '1': '¹', '2': '²', '3': '³', '4': '⁴', '5': '⁵', '6': '⁶', '7': '⁷', '8': '⁸', '9': '⁹' };

/** Live scale-bar math. Geometry stays in voxels. Scenario scale is the scene scalar. */

export function visibleLatticeSpan({ fovDeg, distance, viewWidth, viewHeight, scenarioScale }) {
    const scale = Number.isFinite(scenarioScale) && scenarioScale > 0 ? scenarioScale : 1;
    const width = Math.max(1, viewWidth);
    const height = Math.max(1, viewHeight);
    const fov = Number.isFinite(fovDeg) && fovDeg > 0 ? fovDeg : 45;
    const dist = Number.isFinite(distance) && distance > 0 ? distance : 1;
    const worldHeight = 2 * Math.tan((fov * Math.PI) / 360) * dist;
    const worldWidth = worldHeight * (width / height);
    const latticeWidth = worldWidth / scale;
    return {
        latticeWidth,
        latticeHeight: worldHeight / scale,
        pixelsPerVoxel: width / latticeWidth,
    };
}

/**
 * 1-2-5 tick step. While a full voxel or more is in view, the step stays
 * at one voxel — the finest discrete lattice level.
 */
export function niceStep(span, divisions = 8) {
    if (!(span > 0) || !Number.isFinite(span)) return 1;
    const raw = span / Math.max(1, divisions);
    const exp = Math.floor(Math.log10(Math.max(raw, 1e-12)));
    const pow = 10 ** exp;
    const frac = raw / pow;
    const nice = frac <= 1 ? 1 : frac <= 2 ? 2 : frac <= 5 ? 5 : 10;
    let step = nice * pow;
    if (span >= 1 && step < 1) step = 1;
    return step;
}

function superscript(exp) {
    return String(exp).replace(/./g, (ch) => SUPER[ch] ?? ch);
}

const AU_M = 1.495978707e11;

/** World-unit length and the name of the body drawn at this engine scale. */
export function lengthGauge(engineMode) {
    switch (engineMode) {
        case 'particles':
            return { metresPerUnit: VOXEL_LENGTH_M, subject: 'cloud', domainUnits: 64 };
        case 'atoms':
            return { metresPerUnit: VOXEL_LENGTH_M, subject: 'atom', domainUnits: 64 };
        case 'molecules':
            return { metresPerUnit: VOXEL_LENGTH_M, subject: 'molecule', domainUnits: 64 };
        case 'planetary':
            return { metresPerUnit: AU_M, subject: 'system', domainUnits: 30 };
        case 'cosmic':
            return { metresPerUnit: VOXEL_LENGTH_M, subject: 'cosmic', domainUnits: 200 };
        default:
            return { metresPerUnit: VOXEL_LENGTH_M, subject: 'lattice', domainUnits: null };
    }
}

/** True once the drawn body is too small to see and the view is the space around it. */
export function outerWorldActive(domainUnits, pixelsPerVoxel) {
    const px = Math.max(0, domainUnits) * Math.max(0, pixelsPerVoxel);
    return px < OUTER_WORLD_PX;
}

/**
 * One bracket. Zoomed in it matches the nearest voxel, then the whole body,
 * then the quasi-domain. The domain fades out as its screen size falls away.
 */
export function anchorRuler({ domainUnits, pixelsPerUnit, viewPx, subject = 'lattice', quasiUnits }) {
    const domain = Math.max(1, domainUnits);
    const ppv = Math.max(0, pixelsPerUnit);
    const cap = Math.max(1, viewPx) * 0.92;
    const voxelPx = ppv;
    const bodyPx = domain * ppv;
    const shell = quasiUnits > 0 ? quasiUnits : shellDiameter(domain);
    const quasiUnitsResolved = shell;
    const quasiPx = quasiUnitsResolved * ppv;
    if (voxelPx >= 80) {
        return { subject: 'voxel', units: 1, px: Math.min(cap, voxelPx), opacity: 1, hidden: false };
    }
    // The environment is the outer shell. Hand off as soon as that sphere fits
    // in the view, while the lattice is still drawn.
    const environmentFits = shell > domain * 2 && quasiPx <= cap && quasiPx > OUTER_WORLD_PX;
    if (bodyPx >= OUTER_WORLD_PX && !environmentFits) {
        return { subject, units: domain, px: Math.min(cap, Math.max(2, bodyPx)), opacity: 1, hidden: false };
    }
    const opacity = quasiPx <= OUTER_WORLD_PX ? 0 : Math.min(1, (quasiPx - OUTER_WORLD_PX) / 48);
    return {
        subject: 'quasi-domain',
        units: quasiUnitsResolved,
        px: Math.min(cap, Math.max(0, quasiPx)),
        opacity,
        hidden: opacity <= 0.02,
    };
}

/** Voxel energy in joules, the SI unit paired with the metre labels. */
export function formatEnergy(electronVolts) {
    if (!Number.isFinite(electronVolts)) return '—';
    const joules = electronVolts * J_PER_EV;
    if (Math.abs(joules) < 1e-30) return '0 J';
    const exp = Math.floor(Math.log10(Math.abs(joules)));
    const mant = joules / 10 ** exp;
    return `${mant.toFixed(3)}×10${superscript(exp)} J`;
}

/** Voxel count → metres in scientific notation. `step` is the tick spacing in voxels. */
export function formatLength(units, step = 1, metresPerUnit = VOXEL_LENGTH_M) {
    if (!Number.isFinite(units)) return '—';
    if (Math.abs(units) < 1e-15) return '0';
    const per = Number.isFinite(metresPerUnit) && metresPerUnit > 0 ? metresPerUnit : VOXEL_LENGTH_M;
    const meters = units * per;
    const abs = Math.abs(meters);
    const exp = Math.floor(Math.log10(abs));
    const mant = meters / 10 ** exp;
    let places = 3;
    if (Number.isFinite(step) && step > 0) {
        const stepMant = Math.abs(step * per) / 10 ** exp;
        if (stepMant > 0 && stepMant < 1) {
            places = Math.min(6, Math.max(3, Math.ceil(-Math.log10(stepMant) - 1e-9)));
        }
    }
    return `${mant.toFixed(places)}×10${superscript(exp)} m`;
}

export function viewportScale(span) {
    const step = niceStep(span, 8);
    const ticks = [0];
    if (step > 0 && step < span) {
        for (let v = step; v < span - step * 0.05; v += step) {
            ticks.push(v);
            if (ticks.length > 16) break;
        }
    }
    if (ticks[ticks.length - 1] !== span) ticks.push(span);
    return { span, step, ticks };
}

/**
 * Pixel insets that keep the viewport ruler in the open gap between side panels.
 * A full-width or short bottom sheet is not a side panel.
 */
export function rulerInsets(view, obstacles, gap = 8) {
    const width = Math.max(1, view?.width || 0);
    const height = Math.max(1, view?.height || 0);
    let left = gap;
    let right = gap;
    for (const rect of obstacles || []) {
        if (!rect || rect.width < 8 || rect.height < 8) continue;
        if (rect.width > width * 0.85) continue;
        const overlap = Math.min(rect.bottom, view.bottom) - Math.max(rect.top, view.top);
        const hitsRuler = rect.top < view.top + 64 && rect.bottom > view.top;
        if (overlap < height * 0.35 && !hitsRuler) continue;
        const center = (rect.left + rect.right) / 2;
        if (center < view.left + width * 0.45 && rect.right > view.left && rect.right < view.right - 48) {
            left = Math.max(left, rect.right - view.left + gap);
        }
        if (center > view.left + width * 0.55 && rect.left < view.right && rect.left > view.left + 48) {
            right = Math.max(right, view.right - rect.left + gap);
        }
    }
    if (left + right > width - 48) {
        const scale = (width - 48) / (left + right);
        left *= scale;
        right *= scale;
    }
    return { left, right };
}

const LATTICE_MULTIPLIERS = [1, 2, 5];

/** Absolute voxel bar. Prefers the whole lattice when it fits; never finer than 1 voxel. */
export function latticeScale(latticeSize, pixelsPerVoxel, viewWidth) {
    const N = Math.max(1, Math.round(Number(latticeSize)) || 1);
    const ppv = pixelsPerVoxel > 0 && Number.isFinite(pixelsPerVoxel) ? pixelsPerVoxel : 1;
    const width = Math.max(1, viewWidth);
    const targetPx = Math.min(220, width * 0.28);
    const maxPx = width * 0.9;
    const options = [];
    const expMax = Math.max(0, Math.ceil(Math.log10(N)));
    for (let e = 0; e <= expMax; e++) {
        for (const m of LATTICE_MULTIPLIERS) {
            const voxels = m * 10 ** e;
            if (voxels >= 1 && voxels <= N) options.push(voxels);
        }
    }
    if (!options.includes(N)) options.push(N);

    const fullPx = N * ppv;
    let voxels = N;
    if (fullPx > maxPx || fullPx < 36) {
        let best = 1;
        let bestScore = Infinity;
        for (const candidate of options) {
            const px = candidate * ppv;
            if (px > maxPx) continue;
            const score = Math.abs(Math.log((px || 1) / targetPx));
            if (score < bestScore) {
                best = candidate;
                bestScore = score;
            }
        }
        voxels = best;
    }
    return {
        voxels,
        px: Math.min(maxPx, Math.max(12, voxels * ppv)),
        latticeSize: N,
    };
}
