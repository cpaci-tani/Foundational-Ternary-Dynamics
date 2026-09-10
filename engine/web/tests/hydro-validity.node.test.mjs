// Small observation-arithmetic fixtures only; no runtime or physics evolution.
import assert from 'node:assert/strict';
import test from 'node:test';
import {
    hydroDecayFit, hydroObservationValidity, hydroFailureValidity,
} from '../../strict/web/hydro/hydro-validity.js';

test('insufficient samples and disabled sound fits are pending, not numerical issues', () => {
    for (const history of [[], [1], [0]]) {
        assert.deepEqual(hydroDecayFit(history, 1), { gamma: null, diffusivity: null, issue: null });
    }
    assert.deepEqual(hydroDecayFit([0, 0], 0, false), { gamma: null, diffusivity: null, issue: null });
});

test('positive exponential observations produce a finite unconstrained decay fit', () => {
    const fit = hydroDecayFit([1, Math.exp(-0.5), Math.exp(-1)], 0.25);
    assert.equal(fit.issue, null);
    assert.ok(Math.abs(fit.gamma - 0.5) < 1e-14);
    assert.ok(Math.abs(fit.diffusivity - 2) < 1e-14);
    const growth = hydroDecayFit([1, 2, 4], 1);
    assert.ok(growth.gamma < 0, 'growth must not be silently clamped to decay');
});

test('zero, negative, missing and non-finite samples make the log fit undefined', () => {
    for (const value of [0, -1, null, undefined, NaN, Infinity, -Infinity]) {
        const fit = hydroDecayFit([1, value], 1);
        assert.equal(fit.gamma, null);
        assert.equal(fit.diffusivity, null);
        assert.match(fit.issue, /No logarithm clamp/);
    }
});

test('subnormal positive observations use their actual logarithms', () => {
    const fit = hydroDecayFit([1e-320, 2e-320], 1);
    assert.equal(fit.issue, null);
    assert.ok(Math.abs(fit.gamma + Math.log(2)) < 1e-12);
});

test('undefined wave number or quotient overflow is surfaced instead of displayed', () => {
    for (const kSquared of [0, -1, NaN, Infinity, Number.MIN_VALUE]) {
        const fit = hydroDecayFit([1, Math.exp(-2)], kSquared);
        assert.equal(fit.gamma, null);
        assert.equal(fit.diffusivity, null);
        assert.equal(typeof fit.issue, 'string');
    }
});

test('fit checks only its last 32 samples and can recover after an old gap leaves', () => {
    const fit = hydroDecayFit([null, ...Array.from({ length: 32 }, (_, i) => Math.exp(-i / 8))], 1);
    assert.equal(fit.issue, null);
    assert.ok(Math.abs(fit.gamma - 0.125) < 1e-14);
});

test('derived projection failure gives Undefined without requiring a runtime stop', () => {
    const checked = hydroObservationValidity({ amplitude: Infinity, history: [1, null],
        kSquared: 1, fitEnabled: true, microtick: '9007199254740993' });
    assert.equal(checked.amplitudeValid, false);
    assert.equal(checked.status.label, 'Undefined');
    assert.equal(checked.status.severity, 'warning');
    assert.match(checked.status.details, /9007199254740993/);
    assert.match(checked.status.details, /runtime can continue/);
});

test('Clear describes bounded arithmetic coverage, including pending sample fits', () => {
    const checked = hydroObservationValidity({ amplitude: 1, history: [1],
        kSquared: 1, fitEnabled: true, microtick: '0' });
    assert.equal(checked.status.label, 'Clear');
    assert.equal(checked.status.severity, 'ok');
    assert.match(checked.status.details, /Full-grid validity and continuum recovery are not certified/);
});

test('runtime failure retains its exact error text and last observed decimal tick', () => {
    const text = 'microtick overflow at uint64 maximum';
    const failure = hydroFailureValidity(new Error(text), '18446744073709551615');
    assert.equal(failure.label, 'Overflow');
    assert.equal(failure.severity, 'error');
    assert.ok(failure.details.includes(text));
    assert.match(failure.details, /18446744073709551615/);
    assert.equal(hydroFailureValidity(new Error('checkpoint header mismatch')).label, 'Invalid');
});
