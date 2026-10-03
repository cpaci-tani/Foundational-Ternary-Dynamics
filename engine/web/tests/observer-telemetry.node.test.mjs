import { test } from 'node:test';
import assert from 'node:assert/strict';
import { observationTelemetry, properRadialDistance } from '../js/observer/telemetry.js';
import { ObserverSession } from '../js/observer/session.js';
import { compactStar, LIGHT_SPEED, NOMINAL_SOLAR_GM } from '../js/observer/compact-star.js';
import { traceObserverRay } from '../js/observer/optics.js';

const cell = (sections, id) => {
    const found = sections.flatMap(section => section.rows).find(row => row.id === id);
    assert.ok(found, `Missing telemetry row ${id}`);
    return found;
};
const read = (sections, id) => cell(sections, id).value;
const close = (actual, expected, tolerance = 1e-11) => assert.ok(Math.abs(actual - expected) <= tolerance * Math.max(1, Math.abs(expected)), `${actual} != ${expected}`);
const snapshot = (preset = 'baseline', profile = 'sr') => new ObserverSession({ preset, profile, playing: false }).snapshot();

test('Minkowski telemetry reports SR quantities without fabricating SI clock, distance or mass calibration', () => {
    const s = snapshot();
    s.observer.velocity = [.6, 0, 0];
    const before = JSON.stringify(s), data = observationTelemetry(s, { selectedId: s.entities[0].id });
    close(read(data, 'speed'), .6); close(read(data, 'gamma'), 1.25);
    close(read(data, 'rapidity'), Math.log(2)); close(read(data, 'sr-clock-rate'), .8);
    close(read(data, 'energy-rest'), 1.25); close(read(data, 'kinetic-rest'), .25); close(read(data, 'momentum-rest'), .75);
    assert.equal(read(data, 'reference'), 'Minkowski SR');
    for (const id of ['coordinate-time-si', 'proper-time-si', 'length-scale', 'star-mass-kg', 'observer-mass', 'lapse', 'killing-energy']) assert.equal(read(data, id), null);
    assert.equal(read(data, 'selected-mass'), s.entities[0].mass);
    assert.equal(JSON.stringify(s), before, 'Telemetry must never change simulation state');
});

test('clock differences pair elapsed coordinate time with the current worldline origin', () => {
    const s = snapshot();
    s.time = 1000; s.observer.worldlineStart = 998; s.observer.properTime = 1.6;
    const data = observationTelemetry(s);
    close(read(data, 'coordinate-elapsed'), 2); close(read(data, 'clock-slip'), .4);
    assert.equal(read(data, 'proper-time'), 1.6);
    delete s.observer.worldlineStart;
    assert.equal(read(observationTelemetry(s), 'clock-slip'), null, 'Legacy origin is unspecified');
    s.observer.worldlineStart = 998; s.scrubTime = 999;
    s.observer.velocity = [.6, 0, 0];
    const scrubbed = observationTelemetry(s);
    assert.equal(read(scrubbed, 'clock-slip'), null, 'Scrubbed time is not paired with the observer clock');
    assert.equal(read(scrubbed, 'proper-time'), 1.6, 'Source-history scrub retains the current observer clock');
    assert.match(cell(scrubbed, 'proper-time').label, /Retained observer/);
    assert.match(cell(scrubbed, 'proper-time').note, /does not rewind/);
    close(read(scrubbed, 'clock-rate'), .8);
    assert.match(cell(scrubbed, 'clock-rate').note, /current observer velocity/);
    s.scrubTime = null;
    assert.equal(read(observationTelemetry(s, { cameraOverride: { position: [0, 0, 3] } }), 'clock-slip'), null, 'Camera preview does not accumulate a new proper clock');
});

test('compact-star local speeds, coordinate drift, clocks and calibrated SI quantities use the declared metric', () => {
    const s = snapshot('compact-star'), star = s.spacetime;
    s.observer.velocity = [.3, .4, 0]; s.time = 2; s.observer.properTime = .8;
    const rho = 3, q = star.rs / (4 * rho), A = (1 - q) / (1 + q), B = (1 + q) ** 2;
    const gamma = 2 / Math.sqrt(3), unitTime = 48000 / 299792458;
    const data = observationTelemetry(s);
    close(read(data, 'speed'), .5); close(read(data, 'speed-si'), .5 * LIGHT_SPEED);
    close(read(data, 'gamma'), gamma); close(read(data, 'coordinate-speed'), .5 * A / B);
    close(read(data, 'coordinate-speed-si'), .5 * A / B * LIGHT_SPEED);
    close(read(data, 'clock-rate'), A / gamma); close(read(data, 'killing-energy'), A * gamma);
    close(read(data, 'coordinate-time-si'), 2 * unitTime); close(read(data, 'proper-time-si'), .8 * unitTime);
    close(read(data, 'clock-slip-si'), 1.2 * unitTime); close(read(data, 'time-scale'), unitTime);
    close(read(data, 'lapse'), A); close(read(data, 'spatial-factor'), B); close(read(data, 'optical-index'), B / A);
    close(read(data, 'coordinate-light-speed'), A / B);
    close(read(data, 'isotropic-radius-si'), 144000); close(read(data, 'areal-radius'), rho * B * 48000);
});

test('proper radial distance agrees with independent Schwarzschild quadrature, including near-surface and flat limits', () => {
    const lower = 12000, upper = 145000, rs = 4134.550107;
    const n = 20000, h = (upper - lower) / n, f = r => 1 / Math.sqrt(1 - rs / r);
    let sum = f(lower) + f(upper);
    for (let i = 1; i < n; i++) sum += (i % 2 ? 4 : 2) * f(lower + i * h);
    close(properRadialDistance(upper, lower, rs), h * sum / 3, 2e-12);
    assert.ok(properRadialDistance(upper, lower, rs) > upper - lower);
    const delta = 1e-6;
    close(properRadialDistance(lower + delta, lower, rs), (lower + delta - lower) * f(lower), 1e-15);
    assert.equal(properRadialDistance(lower, lower, rs), 0);
    assert.equal(properRadialDistance(upper, lower, 0), upper - lower);
    assert.equal(properRadialDistance(upper, lower, lower), null);
    assert.equal(properRadialDistance(upper, lower, -1), null);
    assert.equal(properRadialDistance(lower - 1, lower, rs), null);
});

test('stellar mass, redshift, hover acceleration, tidal fields and curvature are distinct reference quantities', () => {
    const s = snapshot('compact-star'), data = observationTelemetry(s), GM = 1.4 * NOMINAL_SOLAR_GM;
    const rs = 2 * GM / LIGHT_SPEED ** 2, R = 12000, rho = 144000, r = rho * (1 + rs / (4 * rho)) ** 2;
    const A = Math.sqrt(1 - rs / r), surfaceA = Math.sqrt(1 - rs / R);
    close(read(data, 'star-gm'), GM); close(read(data, 'star-mass-kg'), GM / 6.67430e-11);
    close(read(data, 'stellar-radius'), R); close(read(data, 'schwarzschild-radius'), rs);
    close(read(data, 'surface-redshift'), 1 / surfaceA - 1);
    close(read(data, 'hover-acceleration'), GM / (r * r * A));
    close(read(data, 'surface-hover-acceleration'), GM / (R * R * surfaceA));
    close(read(data, 'radial-tidal'), 2 * GM / r ** 3); close(read(data, 'transverse-tidal'), -GM / r ** 3);
    // Multiply out the tiny curvature before comparison so absolute tolerance cannot hide an error.
    close(read(data, 'kretschmann') * r ** 6, 12 * rs ** 2);
    close(read(data, 'mean-density-proxy'), GM / 6.67430e-11 / (4 * Math.PI * R ** 3 / 3));
    close(read(data, 'escape-speed'), Math.sqrt(rs / r)); close(read(data, 'photon-sphere'), 1.5 * rs); close(read(data, 'isco-radius'), 3 * rs);
    assert.ok(read(data, 'proper-radial-distance') > read(data, 'areal-height'));
});

test('zero stellar GM yields finite flat reference telemetry without singular radius ratios', () => {
    const s = snapshot('compact-star');
    s.spacetime = { ...compactStar(0), sourceId: s.spacetime.sourceId };
    const data = observationTelemetry(s);
    for (const id of ['star-gm', 'star-mass-kg', 'surface-redshift', 'hover-acceleration', 'radial-tidal', 'kretschmann', 'escape-speed']) assert.equal(read(data, id), 0);
    assert.equal(read(data, 'radius-rs'), null);
    close(read(data, 'proper-radial-distance'), 144000 - 12000);
    assert.ok(data.flatMap(section => section.rows).every(row => typeof row.value !== 'number' || Number.isFinite(row.value)));
});

test('received-light telemetry uses the current trace and keeps Doppler and display controls separate', () => {
    const s = snapshot('compact-star');
    s.observer.velocity = [0, 0, -.3];
    const hit = traceObserverRay(s, {}, 0, 0);
    assert.ok(hit);
    const before = JSON.stringify(hit), on = observationTelemetry(s, { hit, settings: { doppler: true, beaming: true } });
    const off = observationTelemetry(s, { hit, settings: { doppler: false, beaming: false } });
    close(read(on, 'frequency-ratio'), hit.doppler); close(read(on, 'redshift'), 1 / hit.doppler - 1);
    close(read(on, 'optical-delay'), s.time - hit.emissionTime); close(read(on, 'source-clock'), hit.properTime);
    close(read(on, 'optical-path'), hit.distance); close(read(on, 'optical-path-si'), hit.distance * 48000);
    close(read(on, 'optical-delay-si'), (s.time - hit.emissionTime) * 48000 / LIGHT_SPEED);
    close(read(on, 'local-doppler-ratio'), 1.3 / Math.sqrt(1 - .3 ** 2));
    assert.notEqual(read(on, 'surface-redshift'), read(on, 'redshift'), 'Static-infinity redshift differs from moving reception');
    for (const id of ['frequency-ratio', 'redshift', 'bolometric-factor', 'optical-delay', 'optical-path']) assert.equal(read(on, id), read(off, id));
    assert.equal(read(off, 'color-display'), 'Off'); assert.equal(read(off, 'intensity-display'), 'Off');
    assert.equal(JSON.stringify(hit), before);
});

test('missing, outside-history or nonoptical rays leave received-light measurements unavailable', () => {
    const s = snapshot('compact-star'), hit = traceObserverRay(s, {}, 0, 0);
    for (const candidate of [null, { ...hit, emissionTime: s.time + 1 }, { ...hit, emissionTime: s.historyStart - 1 }]) {
        const data = observationTelemetry(s, { hit: candidate });
        assert.equal(read(data, 'optical-status'), 'No current source hit');
        for (const id of ['source', 'source-revision', 'frequency-ratio', 'redshift', 'optical-delay', 'optical-path', 'source-clock', 'source-lapse']) assert.equal(read(data, id), null);
    }
    const sr = snapshot();
    const data = observationTelemetry(sr, { hit, settings: { optical: false } });
    assert.equal(read(data, 'optical-mode'), 'Simultaneous geometry'); assert.equal(read(data, 'frequency-ratio'), null);
});

test('classical Playground telemetry does not apply SR or GR formulas', () => {
    const s = snapshot('baseline', 'playground');
    s.observer.velocity = [3, 4, 0];
    const data = observationTelemetry(s);
    assert.equal(read(data, 'reference'), 'Classical Playground');
    assert.equal(read(data, 'speed'), 5); assert.equal(read(data, 'coordinate-speed'), 5); assert.equal(read(data, 'clock-rate'), 1);
    for (const id of ['gamma', 'rapidity', 'energy-rest', 'kinetic-rest', 'momentum-rest', 'sr-clock-rate', 'killing-energy', 'frequency-ratio', 'lapse', 'star-mass-kg']) assert.equal(read(data, id), null);
});

test('renderer and read-only lattice channels retain source provenance and withhold stale publications', () => {
    const s = snapshot();
    const rendering = { internalResolution: [1280, 720], internalScale: .8, requestedScale: 1, gpuTimeMs: 4.2, gpu: { vendor: 'NVIDIA', renderer: 'RTX 5090', floatReadback: true } };
    const lattice = { available: true, stale: false, sampleTick: '9007199254740993001', manifested: '9007199254740993002', backend: 'Finite records', sourceId: 'records-owner', lawId: 'Phi', status: 'Completed record observation' };
    const data = observationTelemetry(s, { rendering, lattice });
    assert.equal(read(data, 'resolution'), '1280 × 720'); assert.equal(read(data, 'render-time'), 4.2);
    assert.match(cell(data, 'render-time').label, /GPU render duration/);
    assert.match(cell(data, 'render-time').note, /GPU timer-query measurement/);
    assert.equal(read(data, 'gpu-renderer'), 'RTX 5090'); assert.equal(read(data, 'float-readback'), 'Available');
    assert.equal(read(data, 'lattice-tick'), lattice.sampleTick); assert.equal(read(data, 'lattice-manifested'), lattice.manifested);
    assert.equal(read(observationTelemetry(s, { lattice: { ...lattice, stale: true } }), 'lattice-manifested'), null);
    assert.equal(read(observationTelemetry(s), 'render-time'), null);
    assert.equal(read(observationTelemetry(s, { rendering: { ...rendering, gpuTimeMs: 0 } }), 'render-time'), null, 'No completed GPU timer query is unavailable, not a frame-time substitute');
    const IDs = data.flatMap(section => section.rows).map(row => row.id);
    assert.equal(new Set(IDs).size, IDs.length, 'Stable row IDs must be unique for DOM updates');
});
