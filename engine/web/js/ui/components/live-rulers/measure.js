import { FTD_ELECTRON_PRIMARY_PLANCK_LENGTH_M } from '../../../constants.js';

/**
 * One voxel is the electron-primary lattice spacing a_phys = ℓ_P
 * (FTD-0137 §4.5 / FTD-0041). Lengths below are in metres.
 */
export const VOXEL_LENGTH_M = FTD_ELECTRON_PRIMARY_PLANCK_LENGTH_M;

/** The lattice mesh stops drawing once the whole cube is under this many pixels. */
export const OUTER_WORLD_PX = 5;

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
