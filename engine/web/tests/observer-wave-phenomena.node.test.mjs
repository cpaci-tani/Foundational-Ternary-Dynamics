import test from 'node:test';
import assert from 'node:assert/strict';
import {
    MAX_WAVE_SEGMENTS, WAVE_SOURCE_CORE_RADIUS, INTERFERENCE_GRID_CELLS, MAX_INTERFERENCE_GRID_CELLS,
    interferenceGridCells, waveConfiguration, sampleInterference, sampleStandingWave, samplePolarization, drawWavePhenomena,
} from '../js/observer/wave-phenomena.js';

function close(actual, expected, tolerance = 1e-12) {
    assert.ok(Math.abs(actual - expected) <= tolerance, `${actual} differs from ${expected}`);
}
const dot = (a, b) => a.reduce((sum, value, i) => sum + value * b[i], 0);
const cross = (a, b) => [a[1] * b[2] - a[2] * b[1], a[2] * b[0] - a[0] * b[2], a[0] * b[1] - a[1] * b[0]];
function collect(time, settings) {
    const lines = [];
    const result = drawWavePhenomena({ time }, settings, (a, b, color, width) => lines.push({ a, b, color, width }));
    return { lines, result };
}

test('settings stay finite and bounded for imported presentation data', () => {
    assert.deepEqual(waveConfiguration(), { wavelength: 4, separation: 4, amplitude: 0.65, phase: 0, polarization: 'linear' });
    assert.deepEqual(waveConfiguration({ waveWavelength: -10, waveSeparation: 99, waveAmplitude: 90, wavePhase: 12, polarizationMode: 'unknown' }),
        { wavelength: 0.5, separation: 12, amplitude: 2, phase: Math.PI, polarization: 'linear' });
    assert.deepEqual(waveConfiguration({ waveWavelength: NaN, waveSeparation: Infinity, waveAmplitude: -Infinity, wavePhase: '7' }), waveConfiguration());
});

test('coherent sources give constructive and destructive interference at equal path length', () => {
    const settings = { waveAmplitude: 1, waveSeparation: 4, waveWavelength: 4 };
    const constructive = sampleInterference(0, 0, 2, settings);
    close(constructive.first, 0.5); close(constructive.second, 0.5); close(constructive.value, 1); close(constructive.intensity, 0.5);
    for (const time of [0, 0.5, 1.25, 2, 200]) {
        const destructive = sampleInterference(0, 0, time, { ...settings, wavePhase: Math.PI });
        close(destructive.value, 0); close(destructive.intensity, 0);
    }
});

test('source cores are explicitly regularized and the period-averaged intensity is correct', () => {
    assert.equal(WAVE_SOURCE_CORE_RADIUS, 1);
    const settings = { waveAmplitude: 1, waveSeparation: 0, waveWavelength: 3.5, wavePhase: 0.7 };
    const core = sampleInterference(0, 0, 0, settings);
    close(core.first, 1); assert.ok(Number.isFinite(core.second));
    const x = 2.2, z = -0.6, samples = 256;
    let meanSquare = 0;
    for (let i = 0; i < samples; i++) meanSquare += sampleInterference(x, z, settings.waveWavelength * i / samples, settings).value ** 2 / samples;
    close(meanSquare, sampleInterference(x, z, 0, settings).intensity);
});

test('outgoing source phase and electromagnetic waves travel at unit coordinate speed', () => {
    const settings = { waveWavelength: 3.5, waveSeparation: 4, waveAmplitude: 0.8 };
    const first = sampleInterference(-5, 0, 0.5, settings), later = sampleInterference(-6.25, 0, 1.75, settings);
    // Distance from the first source grows 3 -> 4.25; remove known 1/r attenuation.
    close(first.first * 3, later.first * 4.25);
    for (const polarizationMode of ['linear', 'circular', 'elliptical']) {
        const a = samplePolarization(1.1, 0.2, { ...settings, polarizationMode });
        const b = samplePolarization(2.35, 1.45, { ...settings, polarizationMode });
        a.electric.forEach((value, i) => close(value, b.electric[i]));
        a.magnetic.forEach((value, i) => close(value, b.magnetic[i]));
    }
});

test('standing-wave nodes are fixed and the field equals two counter-propagating waves', () => {
    const settings = { waveWavelength: 3, waveAmplitude: 1.2 }, k = 2 * Math.PI / settings.waveWavelength;
    close(sampleStandingWave(0, 0, settings), 1.2);
    for (const time of [0, 0.375, 0.75, 1.13, 98]) {
        for (let n = -4; n <= 4; n++) close(sampleStandingWave(settings.waveWavelength * (0.25 + n / 2), time, settings), 0);
        for (const x of [-3.1, 0, 2.7]) close(sampleStandingWave(x, time, settings), 0.6 * (Math.cos(k * (x - time)) + Math.cos(k * (x + time))));
    }
});

test('E and B remain transverse and oriented with positive propagation flux', () => {
    for (const polarizationMode of ['linear', 'circular', 'elliptical']) {
        for (const time of [0, 0.3, 0.7, 1.4]) {
            const fields = samplePolarization(0.1, time, { polarizationMode, waveAmplitude: 1.3 });
            const { electric: e, magnetic: b, direction: n } = fields;
            close(dot(e, b), 0); close(dot(e, n), 0); close(dot(b, n), 0); close(dot(e, e), dot(b, b));
            cross(n, e).forEach((value, i) => close(value, b[i]));
            close(dot(cross(e, b), n), dot(e, e));
            if (polarizationMode === 'circular') close(dot(e, e), 1.3 ** 2);
            if (polarizationMode === 'linear') close(e[1], 0);
        }
    }
    const ellipse = samplePolarization(1, 0, { polarizationMode: 'elliptical', waveAmplitude: 2, waveWavelength: 4 });
    close(ellipse.electric[0], 0); close(ellipse.electric[1], 0.9);
});

test('a period returns the same fields, including deterministic time zero', () => {
    const settings = { waveWavelength: 2.75, polarizationMode: 'circular' };
    for (const time of [0, 0.625, -2]) {
        close(sampleInterference(1, 2, time, settings).value, sampleInterference(1, 2, time + 2.75, settings).value);
        close(sampleStandingWave(1, time, settings), sampleStandingWave(1, time + 2.75, settings));
        samplePolarization(1, time, settings).electric.forEach((value, i) => close(value, samplePolarization(1, time + 2.75, settings).electric[i]));
    }
});

test('drawing is finite, bounded, deterministic when paused, and does not mutate input', () => {
    assert.equal(INTERFERENCE_GRID_CELLS, 28);
    assert.equal(MAX_INTERFERENCE_GRID_CELLS, 56);
    for (const waveWavelength of [0.5, 4, 12]) for (const time of [0, 3.125, Number.MAX_VALUE]) {
        const settings = Object.freeze({ layers: Object.freeze({ interference: true, standingWaves: true, polarization: true }), waveWavelength, waveAmplitude: 2, waveSeparation: 12, wavePhase: Math.PI, polarizationMode: 'circular' });
        const first = collect(time, settings), second = collect(time, settings);
        assert.deepEqual(first, second);
        assert.equal(first.lines.length, first.result.segments);
        assert.equal(first.result.interferenceGridCells, interferenceGridCells(waveWavelength));
        assert.ok(first.result.segments > 1800 && first.result.segments < MAX_WAVE_SEGMENTS);
        assert.equal(first.result.coordinateTime, time);
        for (const line of first.lines) {
            assert.ok([...line.a, ...line.b, line.width].every(Number.isFinite));
            assert.ok(line.width > 0); assert.match(line.color, /^#[0-9a-f]{6}([0-9a-f]{2})?$/i);
        }
    }
});

test('short wavelengths receive at least four samples per wavelength without exceeding the budget', () => {
    for (const wavelength of [0.5, 0.501, 0.6, 0.73, 0.9, 1, 4, 12]) {
        const cells = interferenceGridCells(wavelength);
        assert.ok(cells >= 28 && cells <= 56 && Number.isInteger(cells));
        assert.ok(cells * wavelength / 7 >= 4);
    }
    assert.equal(interferenceGridCells(0.5), 56);
    assert.equal(interferenceGridCells(4), 28);
    const maximum = collect(0, { layers: { interference: true, standingWaves: true, polarization: true }, waveWavelength: 0.5 });
    assert.equal(maximum.result.segments, 6784);
    assert.ok(maximum.result.segments < MAX_WAVE_SEGMENTS);
});

test('shared grid vertices preserve scalar heights, connectivity, and callback order', () => {
    for (const wavelength of [0.5, 0.73, 4]) {
        const settings = { layers: { interference: true }, waveWavelength: wavelength, waveSeparation: 3.7, wavePhase: 0.8 };
        const { lines } = collect(1.125, settings), cells = interferenceGridCells(wavelength), step = 7 / cells;
        const grid = lines.slice(0, 2 * cells * (cells + 1));
        assert.equal(new Set(grid.flatMap(line => [line.a, line.b])).size, (cells + 1) ** 2);
        let index = 0;
        for (let row = 0; row <= cells; row++) for (let column = 0; column < cells; column++) {
            const fixed = -3.5 + row * step, value = -3.5 + column * step;
            for (const [points, color] of [
                [[[value, fixed], [value + step, fixed]], '#6df2cb90'],
                [[[fixed, value], [fixed, value + step]], '#84c7ff70'],
            ]) {
                const line = grid[index++]; assert.equal(line.color, color); assert.equal(line.width, 1);
                for (let end = 0; end < 2; end++) {
                    const [x, z] = points[end], vertex = end ? line.b : line.a;
                    close(vertex[0], x); close(vertex[2], z - 4.5);
                    close(vertex[1], 1.5 + sampleInterference(x, z, 1.125, settings).value);
                }
            }
        }
    }
});

test('each layer draws independently; all-off allocates no segments', () => {
    assert.equal(collect(0, {}).result.segments, 0);
    for (const name of ['interference', 'standingWaves', 'polarization']) {
        const start = collect(0, { layers: { [name]: true } }), evolved = collect(0.375, { layers: { [name]: true } });
        assert.ok(start.result.segments > 0); assert.notDeepEqual(start.lines, evolved.lines);
    }
});
