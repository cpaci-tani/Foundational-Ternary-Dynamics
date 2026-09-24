/**
 * Coordinate-frame analytic wave illustrations, with adopted c = 1.
 * These are world-anchored presentation instruments, not native lattice fields,
 * optical history, forces, or a relativistic electromagnetic-field solver.
 */

/** @typedef {'linear'|'circular'|'elliptical'} PolarizationMode */
/** @typedef {{layers?:Record<string,boolean>,waveWavelength?:number,waveSeparation?:number,waveAmplitude?:number,wavePhase?:number,polarizationMode?:PolarizationMode}} WaveSettings */
/** @typedef {{wavelength:number,separation:number,amplitude:number,phase:number,polarization:PolarizationMode}} WaveConfiguration */
/** @typedef {(a:number[],b:number[],color:string,width?:number)=>void} WaveLine */

export const MAX_WAVE_SEGMENTS = 7000;
export const INTERFERENCE_GRID_CELLS = 28;
export const MAX_INTERFERENCE_GRID_CELLS = 56;
/** Unit-radius source cores flatten 1/r attenuation; the core is illustrative. */
export const WAVE_SOURCE_CORE_RADIUS = 1;
const TAU = 2 * Math.PI;

/** @param {unknown} value @param {number} fallback @param {number} low @param {number} high */
function bounded(value, fallback, low, high) {
    return typeof value === 'number' && Number.isFinite(value) ? Math.max(low, Math.min(high, value)) : fallback;
}

/** @param {WaveSettings} settings @returns {WaveConfiguration} */
export function waveConfiguration(settings = {}) {
    return {
        wavelength: bounded(settings.waveWavelength, 4, 0.5, 12),
        separation: bounded(settings.waveSeparation, 4, 0, 12),
        amplitude: bounded(settings.waveAmplitude, 0.65, 0, 2),
        phase: bounded(settings.wavePhase, 0, -Math.PI, Math.PI),
        polarization: ['linear', 'circular', 'elliptical'].includes(settings.polarizationMode || '') ? /** @type {PolarizationMode} */ (settings.polarizationMode) : 'linear',
    };
}

/** At least four spatial samples per wavelength across the 7-unit board.
 * Default wavelengths retain 28 cells; the shortest allowed wavelength needs 56.
 * @param {number} wavelength
 */
export function interferenceGridCells(wavelength) {
    return Math.min(MAX_INTERFERENCE_GRID_CELLS, Math.max(INTERFERENCE_GRID_CELLS, Math.ceil(28 / bounded(wavelength, 4, 0.5, 12))));
}

/** Reducing before multiplication avoids overflow at long session times.
 * @param {number} distance @param {number} time @param {number} wavelength
 */
function phaseAt(distance, time, wavelength) {
    return TAU * ((distance % wavelength) - ((Number.isFinite(time) ? time : 0) % wavelength)) / wavelength;
}

/**
 * Two stationary coherent sources at (+/-separation/2, 0), viewed on a plane.
 * Outside each unit core the scalar amplitude is A/r. Inside it is clamped to A;
 * the regularized core is not a solution of the source-free wave equation.
 * intensity is the exact period-average of value squared, not physical energy.
 * @param {number} x @param {number} z @param {number} time @param {WaveConfiguration} config
 */
function interferenceAt(x, z, time, config) {
    const r1 = Math.hypot(x + config.separation / 2, z), r2 = Math.hypot(x - config.separation / 2, z);
    const a = config.amplitude / Math.max(WAVE_SOURCE_CORE_RADIUS, r1), b = config.amplitude / Math.max(WAVE_SOURCE_CORE_RADIUS, r2);
    const p1 = phaseAt(r1, time, config.wavelength), p2 = phaseAt(r2, time, config.wavelength) + config.phase;
    const first = a * Math.cos(p1), second = b * Math.cos(p2);
    return { first, second, value: first + second, intensity: Math.max(0, 0.5 * (a * a + b * b + 2 * a * b * Math.cos(p2 - p1))) };
}

/** Local x/z coordinates relative to the interference board center.
 * @param {number} x @param {number} z @param {number} time @param {WaveSettings} settings
 */
export function sampleInterference(x, z, time, settings = {}) {
    return interferenceAt(x, z, time, waveConfiguration(settings));
}

/** Equal counter-propagating waves: A cos(kx) cos(kt), k = omega = 2pi/lambda.
 * @param {number} x @param {number} time @param {WaveSettings} settings
 */
export function sampleStandingWave(x, time, settings = {}) {
    const config = waveConfiguration(settings);
    return config.amplitude * Math.cos(phaseAt(x, 0, config.wavelength)) * Math.cos(phaseAt(0, time, config.wavelength));
}

/** Plane wave travelling along n = (0,0,-1), with B = n cross E at c = 1.
 * distance increases along n. Elliptical mode has minor/major amplitude ratio 0.45.
 * @param {number} distance @param {number} time @param {WaveConfiguration} config
 */
function polarizationAt(distance, time, config) {
    const phase = phaseAt(distance, time, config.wavelength);
    const ratio = config.polarization === 'circular' ? 1 : config.polarization === 'elliptical' ? 0.45 : 0;
    const x = config.amplitude * Math.cos(phase), y = config.amplitude * ratio * Math.sin(phase);
    return { electric: [x, y, 0], magnetic: [y, -x, 0], direction: [0, 0, -1] };
}

/** @param {number} distance @param {number} time @param {WaveSettings} settings */
export function samplePolarization(distance, time, settings = {}) {
    return polarizationAt(distance, time, waveConfiguration(settings));
}

/**
 * Draw three bounded instruments in distinct fixed world zones. No wall-clock
 * reads, retained history, simulation mutations, DOM, or renderer dependencies.
 * The callback receives shared world-space vertices and must not mutate them;
 * clipping belongs to the renderer. Vertices live only for this bounded draw call.
 * @param {{time:number}} snapshot @param {WaveSettings} settings @param {WaveLine} lineCallback
 */
export function drawWavePhenomena(snapshot, settings, lineCallback) {
    const config = waveConfiguration(settings), layers = settings.layers || {}, time = Number.isFinite(snapshot.time) ? snapshot.time : 0;
    let segments = 0;
    /** @param {number[]} a @param {number[]} b @param {string} color @param {number} width */
    const line = (a, b, color, width = 1) => {
        if (segments >= MAX_WAVE_SEGMENTS) return;
        segments++;
        lineCallback(a, b, color, width);
    };
    if (layers.interference) {
        // Increase density at short wavelengths without shrinking the world-space board.
        const cells = interferenceGridCells(config.wavelength), half = 3.5, step = 2 * half / cells;
        /** @param {number} x @param {number} z */
        const point = (x, z) => [x, 1.5 + interferenceAt(x, z, time, config).value, z - 4.5];
        const stride = cells + 1;
        /** @type {number[][]} */ const vertices = new Array(stride * stride);
        for (let row = 0; row <= cells; row++) for (let column = 0; column <= cells; column++) {
            vertices[row * stride + column] = point(-half + column * step, -half + row * step);
        }
        for (let row = 0; row <= cells; row++) {
            for (let column = 0; column < cells; column++) {
                line(vertices[row * stride + column], vertices[row * stride + column + 1], '#6df2cb90');
                line(vertices[column * stride + row], vertices[(column + 1) * stride + row], '#84c7ff70');
            }
        }
        // Source markers stay at their preparation positions, not on wave crests.
        for (const x of [-config.separation / 2, config.separation / 2]) {
            line([x - 0.16, 1.5, -4.5], [x + 0.16, 1.5, -4.5], '#fff4b8', 3);
            line([x, 1.34, -4.5], [x, 1.66, -4.5], '#fff4b8', 3);
            line([x, 1.5, -4.66], [x, 1.5, -4.34], '#fff4b8', 3);
        }
    }
    if (layers.standingWaves) {
        const left = -6, right = 6, cells = 96, temporal = Math.cos(phaseAt(0, time, config.wavelength));
        /** @param {number} x */
        const point = x => [x, 3.5 + config.amplitude * Math.cos(phaseAt(x, 0, config.wavelength)) * temporal, -12];
        line([left, 3.5, -12], [right, 3.5, -12], '#b4a2ef66');
        let previous = point(left);
        for (let i = 0; i < cells; i++) {
            const next = point(left + (right - left) * (i + 1) / cells);
            line(previous, next, '#d7b5ff', 2); previous = next;
        }
        // Fixed spatial nodes x = lambda*(1/4 + n/2), independent of time.
        const first = Math.ceil(2 * left / config.wavelength - 0.5), last = Math.floor(2 * right / config.wavelength - 0.5);
        for (let n = first; n <= last; n++) {
            const x = config.wavelength * (0.25 + n / 2);
            line([x, 3.35, -12], [x, 3.65, -12], '#fff0b3', 2);
        }
    }
    if (layers.polarization) {
        const length = 12, cells = 96;
        /** @param {number} distance @param {number[]} vector */
        const tip = (distance, vector) => [6 + vector[0], 3.5 + vector[1], 4 - distance + vector[2]];
        line([6, 3.5, 4], [6, 3.5, 4 - length], '#c4dfec55');
        const initial = polarizationAt(0, time, config);
        let previousElectric = tip(0, initial.electric), previousMagnetic = tip(0, initial.magnetic);
        for (let i = 0; i < cells; i++) {
            const a = length * i / cells, b = length * (i + 1) / cells;
            const end = polarizationAt(b, time, config), nextElectric = tip(b, end.electric), nextMagnetic = tip(b, end.magnetic);
            line(previousElectric, nextElectric, '#ffad78', 2);
            line(previousMagnetic, nextMagnetic, '#77cfff', 2);
            if (i % 8 === 0) {
                const origin = tip(a, [0, 0, 0]);
                line(origin, previousElectric, '#ffad7877');
                line(origin, previousMagnetic, '#77cfff77');
            }
            previousElectric = nextElectric; previousMagnetic = nextMagnetic;
        }
        // Polarization locus at the front plane: linear, circle, or ellipse.
        let previous = tip(0, polarizationAt(0, 0, config).electric);
        for (let i = 0; i < 32; i++) {
            const next = tip(0, polarizationAt(0, config.wavelength * (i + 1) / 32, config).electric);
            line(previous, next, '#ffdebd88'); previous = next;
        }
    }
    return { segments, interferenceGridCells: layers.interference ? interferenceGridCells(config.wavelength) : 0,
        coordinateTime: time, model: 'coordinate-frame analytic illustration' };
}
