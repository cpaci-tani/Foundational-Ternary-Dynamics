import { BOHR_RADIUS_M, E_REST, FTD_ELECTRON_PRIMARY_PLANCK_LENGTH_M, J_PER_EV, K_B } from '../../../constants.js';
import { PE_VIS_BOUNDARY_R } from '../../../viewport/constants.js';

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
    mooreEnergy: true,
    mooreHz: 20,
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

/** Same base-ten turn as the global ordinal clock hand. */
export const CLOCK_TICKS_PER_TURN = 10;

/** Fractional turn of the global ordinal clock. Tick 0 is phase 0. */
export function globalClockPhase(tick) {
    const turns = (Number(tick) || 0) / CLOCK_TICKS_PER_TURN;
    return turns - Math.floor(turns);
}

/**
 * Where this voxel sits in the neighborhood environment, from -1 to 1.
 * The neighborhood mean is 0, so a uniform environment stays on the global clock.
 */
export function relativeClockEnvironment(value, mean, span) {
    if (!(span > 0)) return 0;
    const t = (Number(value) - mean) / span;
    if (!Number.isFinite(t)) return 0;
    return Math.max(-1, Math.min(1, t));
}

/**
 * Fractional turn of one voxel clock. Offset 0 matches the global clock,
 * including at the start of a scenario. The offset is the accumulated
 * effect of that voxel's environment relative to its neighborhood.
 */
export function voxelClockPhase(tick, offset = 0) {
    const extra = Number(offset);
    const turns = (Number(tick) || 0) / CLOCK_TICKS_PER_TURN + (Number.isFinite(extra) ? extra : 0);
    return turns - Math.floor(turns);
}

/** Turn advance caused by a relative environment over a span of ticks. Zero at the start. */
export function clockEnvironmentStep(deltaTicks, relativeEnvironment) {
    const steps = Number(deltaTicks);
    if (!(steps > 0)) return 0;
    const relative = Number(relativeEnvironment);
    const shift = Number.isFinite(relative) ? Math.max(-1, Math.min(1, relative)) : 0;
    return (steps / CLOCK_TICKS_PER_TURN) * 0.25 * shift;
}

/** Local phase beside the global ordinal readout, plus the signed turn difference. */
export function formatClockVersus(phase, tick) {
    const local = Number(phase);
    const wrapped = Number.isFinite(local) ? local - Math.floor(local) : 0;
    const global = globalClockPhase(tick);
    let delta = wrapped - global;
    if (delta > 0.5) delta -= 1;
    if (delta < -0.5) delta += 1;
    if (Math.abs(delta) < 5e-4) delta = 0;
    const sign = delta < 0 ? '' : '+';
    const ordinal = Math.max(0, Math.trunc(Number(tick) || 0));
    return `${wrapped.toFixed(3)} vs tick ${ordinal} (${sign}${delta.toFixed(3)})`;
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

/** Filled area under a summed energy line. The top is the neighborhood max, the bottom its min. */
export function energyAreaPath(samples, min, max) {
    const line = energyStringPath(samples, min, max);
    if (!line) return '';
    const count = samples.length;
    const endX = count === 1 ? 50 : 100;
    const startX = count === 1 ? 50 : 0;
    return `${line}L${endX.toFixed(2)} 16L${startX.toFixed(2)} 16Z`;
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

/** Radius the atom engine reflects free atoms into (mock-atom-engine.js, drift step). */
const ATOM_CONTAINMENT_R = 35;

/** Reference box the cosmic bridge starts with (mock-scale5.js). Scenarios may set their own. */
const COSMIC_REFERENCE_BOX = 200;

const ATOM_VIEW_TITLE = 'Visible width in metres. One simulation unit is taken as one Bohr radius (a₀), '
    + 'the atom engine\'s declared length convention: an input. Spacings are tuned for the simulation, not measured.';
const ATOM_BODY_TITLE = 'Diameter of the atom engine\'s containment boundary, in metres. '
    + 'One simulation unit is taken as one Bohr radius (a₀): an input.';

/**
 * What one world unit is on each engine scale, and the body its bracket spans.
 *
 * `metresPerUnit` is set only where the engine itself declares a metre length
 * for its unit, and each such value is a calibration input. Where it is null
 * the rulers read in the engine's own unit and show no metres.
 *
 *   lattice    one voxel, the electron-primary lattice spacing (FTD-0137 §4.5).
 *   particles  lattice units (lu): particle_engine.h positions and the Scale 1
 *              formatter in units.js. No Scale 1 readout converts lu to metres.
 *   atoms,     Bohr-scaled simulation units: mock-atom-engine.js softening,
 *   molecules  molecules.js positions, and the Scale 2 formatter in units.js,
 *              which turns one unit into one Bohr radius.
 *   planetary  astronomical units (mock-scale4.js).
 *   cosmic     lattice units (lu): mock-scale5.js scales distances for visible
 *              dynamics, and constants.js records no SI calibration for them.
 *
 * `domainUnits` is an extent the engine defines: the Scale 1 display reference
 * shell, the atom engine's containment sphere, the cosmic bridge's default
 * reference box. Null follows the lattice size.
 */
const GAUGES = Object.freeze({
    lattice: Object.freeze({
        mode: 'lattice', metresPerUnit: VOXEL_LENGTH_M, unit: 'voxel', unitSubject: 'voxel',
        subject: 'lattice', domainUnits: null,
        viewTitle: 'Visible width in metres. One voxel is the electron-primary Planck length.',
        bodyTitle: 'Absolute lattice length. One voxel is the electron-primary Planck length.',
    }),
    particles: Object.freeze({
        mode: 'particles', metresPerUnit: null, unit: 'lu', unitSubject: 'lattice unit',
        subject: 'reference shell', domainUnits: 2 * PE_VIS_BOUNDARY_R,
        viewTitle: 'Visible width in lattice units (lu), the particle engine\'s own length unit. '
            + 'The engine declares no metre length for it, so no metre reading is shown.',
        bodyTitle: 'Diameter of the display reference shell, in lattice units (lu). '
            + 'The particle engine is unbounded and declares no metre length for its unit.',
    }),
    atoms: Object.freeze({
        mode: 'atoms', metresPerUnit: BOHR_RADIUS_M, unit: 'a₀', unitSubject: 'Bohr radius',
        subject: 'boundary', domainUnits: 2 * ATOM_CONTAINMENT_R,
        viewTitle: ATOM_VIEW_TITLE, bodyTitle: ATOM_BODY_TITLE,
    }),
    molecules: Object.freeze({
        mode: 'molecules', metresPerUnit: BOHR_RADIUS_M, unit: 'a₀', unitSubject: 'Bohr radius',
        subject: 'boundary', domainUnits: 2 * ATOM_CONTAINMENT_R,
        viewTitle: ATOM_VIEW_TITLE, bodyTitle: ATOM_BODY_TITLE,
    }),
    planetary: Object.freeze({
        mode: 'planetary', metresPerUnit: AU_M, unit: 'AU', unitSubject: 'AU',
        subject: 'system', domainUnits: 30,
        viewTitle: 'Visible width in metres. One unit is one astronomical unit (AU).',
        bodyTitle: 'Bracket length in metres. One unit is one astronomical unit (AU).',
    }),
    cosmic: Object.freeze({
        mode: 'cosmic', metresPerUnit: null, unit: 'lu', unitSubject: 'lattice unit',
        subject: 'cosmic', domainUnits: COSMIC_REFERENCE_BOX,
        viewTitle: 'Visible width in lattice units (lu), the cosmic engine\'s own length unit. '
            + 'Distances are scaled for visible dynamics and have no metre calibration, so no metre reading is shown.',
        bodyTitle: 'The cosmic engine\'s default reference box, in lattice units (lu). '
            + 'A scenario may use a different box. The unit has no metre calibration.',
    }),
});

/** Length unit, tooltips, and drawn body for an engine scale. Unknown modes read as the lattice. */
export function lengthGauge(engineMode) {
    return Object.hasOwn(GAUGES, engineMode) ? GAUGES[engineMode] : GAUGES.lattice;
}

/** True once the drawn body is too small to see and the view is the space around it. */
export function outerWorldActive(domainUnits, pixelsPerVoxel) {
    const px = Math.max(0, domainUnits) * Math.max(0, pixelsPerVoxel);
    return px < OUTER_WORLD_PX;
}

/**
 * One bracket. Zoomed in it matches the nearest voxel, then the whole body,
 * then the quasi-domain. The domain fades out as its screen size falls away.
 * Off the lattice the closest stage is one engine unit, named by `unitSubject`.
 */
export function anchorRuler({ domainUnits, pixelsPerUnit, viewPx, subject = 'lattice', unitSubject = 'voxel', quasiUnits }) {
    const domain = Math.max(1, domainUnits);
    const ppv = Math.max(0, pixelsPerUnit);
    const cap = Math.max(1, viewPx) * 0.92;
    const voxelPx = ppv;
    const bodyPx = domain * ppv;
    const shell = quasiUnits > 0 ? quasiUnits : shellDiameter(domain);
    const quasiUnitsResolved = shell;
    const quasiPx = quasiUnitsResolved * ppv;
    if (voxelPx >= 80) {
        return { subject: unitSubject, units: 1, px: Math.min(cap, voxelPx), opacity: 1, hidden: false };
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

/** A length in an engine's own unit, for a scale with no declared metre length. */
export function formatUnitLength(units, symbol) {
    if (!Number.isFinite(units)) return '—';
    if (Math.abs(units) < 1e-15) return '0';
    const abs = Math.abs(units);
    if (abs >= 1e6 || abs < 1e-3) {
        const exp = Math.floor(Math.log10(abs));
        return `${(units / 10 ** exp).toFixed(3)}×10${superscript(exp)} ${symbol}`;
    }
    return `${Number(units.toPrecision(4))} ${symbol}`;
}

/** A length on a scale's gauge: metres where the engine declares them, otherwise its own unit. */
export function formatGaugeLength(gauge, units, step = 1) {
    return gauge.metresPerUnit > 0
        ? formatLength(units, step, gauge.metresPerUnit)
        : formatUnitLength(units, gauge.unit);
}

/**
 * Ticks for the view ruler. `divisions` is how many steps fit without their
 * labels touching. `endClearance` is the share of the span the right-aligned
 * end label needs; an interior tick that close to the end is left out so the
 * two labels never print over each other.
 */
export function viewportScale(span, divisions = 8, endClearance = 0) {
    const step = niceStep(span, divisions);
    const ticks = [0];
    if (step > 0 && step < span) {
        const limit = span - Math.max(step * 0.05, span * Math.max(0, endClearance));
        for (let v = step; v < limit; v += step) {
            ticks.push(v);
            if (ticks.length > 16) break;
        }
    }
    if (ticks[ticks.length - 1] !== span) ticks.push(span);
    return { span, step, ticks };
}

/** Height below the view's top edge that the view ruler occupies, in pixels. */
export const RULER_BAND_PX = 70;
/** Below this width the view ruler is hidden rather than squeezed. */
export const RULER_MIN_WIDTH_PX = 240;
/** Pixels one tick label needs, and the end label with its neighbour's half. */
export const RULER_TICK_SPACING_PX = 170;
export const RULER_END_LABEL_PX = 190;

/**
 * Where the view ruler fits: the widest open stretch of its own row, with
 * `gap` pixels of air on both sides. An obstacle is anything whose rectangle
 * reaches into the ruler's row (a rail, a side panel, a floating panel open or
 * collapsed, a bar docked on top). A full-width element is a backdrop, not an
 * obstacle. `hidden` says the stretch is too narrow to carry a ruler.
 */
export function rulerInsets(view, obstacles, gap = 8, { band = RULER_BAND_PX, minWidth = RULER_MIN_WIDTH_PX } = {}) {
    const width = Math.max(1, view?.width || 0);
    const lo = view.left + gap;
    const hi = view.right - gap;
    const blocked = [];
    for (const rect of obstacles || []) {
        if (!rect || rect.width < 8 || rect.height < 8) continue;
        if (rect.width > width * 0.85) continue;
        if (!(rect.top < view.top + band && rect.bottom > view.top)) continue;
        const from = Math.max(lo, rect.left - gap);
        const to = Math.min(hi, rect.right + gap);
        if (to > from) blocked.push([from, to]);
    }
    blocked.sort((a, b) => a[0] - b[0]);
    let start = lo;
    let end = lo;
    let cursor = lo;
    for (const [from, to] of blocked) {
        if (from - cursor > end - start) { start = cursor; end = from; }
        cursor = Math.max(cursor, to);
    }
    if (hi - cursor > end - start) { start = cursor; end = hi; }
    const open = Math.max(0, end - start);
    return { left: start - view.left, right: view.right - end, width: open, hidden: open < minWidth };
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
