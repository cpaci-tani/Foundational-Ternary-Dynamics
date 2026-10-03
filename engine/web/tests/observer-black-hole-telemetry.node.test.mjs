import { test } from 'node:test';
import assert from 'node:assert/strict';
import { observationTelemetry, properHorizonDistance } from '../js/observer/telemetry.js';
import { ObserverSession } from '../js/observer/session.js';
import { starMetric, LIGHT_SPEED, NOMINAL_SOLAR_GM } from '../js/observer/compact-star.js';

const read = (data, id) => data.flatMap(group => group.rows).find(row => row.id === id).value;
const close = (actual, expected, tolerance = 1e-11) => assert.ok(Math.abs(actual - expected) <= tolerance * Math.max(1, Math.abs(expected)), `${actual} versus ${expected}`);

test('proper horizon distance evaluates the integrable static-slice endpoint against regularized quadrature', () => {
    const rs = 30000, r = 110000, upper = Math.sqrt(r - rs), count = 10000, h = upper / count;
    const f = x => 2 * Math.sqrt(rs + x * x);
    let sum = f(0) + f(upper);
    for (let i = 1; i < count; i++) sum += (i % 2 ? 4 : 2) * f(i * h);
    close(properHorizonDistance(r, rs), sum * h / 3, 1e-12);
    assert.equal(properHorizonDistance(rs, rs), 0);
    const delta = 1e-8;
    close(properHorizonDistance(rs + delta, rs), 2 * Math.sqrt(rs * (rs + delta - rs)), 1e-12);
    for (const args of [[rs - 1, rs], [r, 0], [r, -1], [Infinity, rs]]) assert.equal(properHorizonDistance(...args), null);
});

test('black-hole telemetry separates horizon, photon sphere, shadow, stellar surface and local motion', () => {
    const s = new ObserverSession({ preset: 'black-hole', playing: false }).snapshot();
    const data = observationTelemetry(s), GM = 10 * NOMINAL_SOLAR_GM, rs = 2 * GM / LIGHT_SPEED ** 2;
    const m = starMetric(s.spacetime, s.observer.position), impact = 1.5 * Math.sqrt(3) * rs;
    close(read(data, 'horizon-radius'), rs);
    close(read(data, 'photon-sphere'), 1.5 * rs); close(read(data, 'isco-radius'), 3 * rs);
    close(read(data, 'critical-impact'), impact);
    close(read(data, 'horizon-area'), 4 * Math.PI * rs ** 2);
    close(read(data, 'horizon-curvature') * rs ** 4, 12);
    close(read(data, 'static-shadow-angle'), Math.asin(impact / s.spacetime.lengthUnitMeters * m.lapse / m.areal) * 180 / Math.PI);
    for (const id of ['stellar-radius', 'surface-redshift', 'surface-lapse', 'surface-compactness', 'mean-density-proxy', 'surface-hover-acceleration', 'source-clock']) assert.equal(read(data, id), null, id);
    assert.ok(read(data, 'proper-radial-distance') > read(data, 'areal-height'));
    assert.ok(data.flatMap(group => group.rows).every(row => typeof row.value !== 'number' || Number.isFinite(row.value)));
    assert.equal(new Set(data.flatMap(group => group.rows).map(row => row.id)).size, data.flatMap(group => group.rows).length);
});

test('inside the photon sphere the static shadow occupies more than half the sky', () => {
    const s = new ObserverSession({ preset: 'black-hole', playing: false }).snapshot();
    s.observer.position = [0, 1.6, s.spacetime.rs * .4];
    const data = observationTelemetry(s);
    assert.ok(read(data, 'static-shadow-angle') > 90);
    assert.ok(read(data, 'static-shadow-solid-angle') > 2 * Math.PI);
    assert.ok(read(data, 'static-shadow-solid-angle') < 4 * Math.PI);
});

test('finite sky-shell optical witness is current and explicitly CPU sourced; captures expose no emission', () => {
    const s = new ObserverSession({ preset: 'black-hole', playing: false }).snapshot(), m = starMetric(s.spacetime, s.observer.position);
    const shell = starMetric(s.spacetime, [s.spacetime.escapeRadius, 1.6, 0]);
    const ray = { source: 'Float64 CPU geodesic', observationTime: s.time, observerPosition: s.observer.position,
        observerVelocity: s.observer.velocity, observerYaw: 0, observerPitch: 0, observerRoll: 0, cameraFov: 60, ndcX: 0, ndcY: 0,
        status: 'escaped', delay: 42, distance: 40, impactParameter: .8, doppler: shell.lapse / m.lapse, emissionLapse: shell.lapse };
    const data = observationTelemetry(s, { rendering: { blackHoleRay: ray } });
    close(read(data, 'frequency-ratio'), ray.doppler); close(read(data, 'optical-delay'), 42);
    close(read(data, 'gr-frequency-ratio'), shell.lapse / m.lapse); close(read(data, 'local-doppler-ratio'), 1);
    assert.equal(read(data, 'source-clock'), null, 'No horizon/source entity clock is fabricated');
    assert.equal(read(observationTelemetry(s, { rendering: { blackHoleRay: ray }, settings: { fov: 90 } }), 'frequency-ratio'), null, 'An old field of view does not describe the current sight ray');
    for (const stale of [{ ...ray, observationTime: -1 }, { ...ray, observerPosition: [1, 2, 3] }, { ...ray, observerVelocity: [.1, 0, 0] }, { ...ray, observerYaw: 1 }, { ...ray, ndcX: .1 }, { ...ray, source: 'GPU unverified' }]) {
        assert.equal(read(observationTelemetry(s, { rendering: { blackHoleRay: stale } }), 'frequency-ratio'), null);
    }
    for (const status of ['captured', 'budget-exhausted']) {
        const readings = observationTelemetry(s, { rendering: { blackHoleRay: { ...ray, status } } });
        assert.equal(read(readings, 'optical-status'), status);
        for (const id of ['frequency-ratio', 'emission-time', 'source-lapse', 'source-clock', 'optical-delay', 'optical-path']) assert.equal(read(readings, id), null, id);
    }
});
