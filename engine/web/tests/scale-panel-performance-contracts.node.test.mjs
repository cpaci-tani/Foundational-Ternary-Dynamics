import assert from 'node:assert/strict';
import { readFile } from 'node:fs/promises';
import test from 'node:test';

import { effectiveFluxSliceUpdateEvery } from '../js/scales/scale0/ui/overlays/flux-slice-panel.js';
import {
    circularFluxDecayFromZSlice,
} from '../js/scales/scale0/ui/overlays/p1-observables/anisotropy.js';
import { interactiveProbeStride } from '../js/scales/scale0/ui/overlays/p1-observables/coulomb.js';
import { p1GravitySampleStride } from '../js/scales/scale0/ui/overlays/p1-observables/gravity.js';
import { getSpectrumComparatorMetrics } from '../js/scales/scale0/analysis/wave-spectrum.js';
import { reduceProperTimeSamples } from '../js/scales/scale0/analysis/proper-time-metrics.js';
import { projectHistoryIndices } from '../js/ui/charts/history-index.js';

test('custom gravity and thermo All views keep actual extrema within a pixel budget', () => {
    const count = 100_000;
    const rows = Array.from({ length: count }, (_, index) => ({
        tick: index,
        a: index === 12345 ? 1e6 : Math.sin(index / 50),
        b: index === 87654 ? -1e6 : Math.cos(index / 70),
    }));
    const buffer = key => ({ count, total: count, generation: 0,
        get: index => rows[index][key], getTick: index => rows[index].tick });
    const indices = projectHistoryIndices([buffer('a'), buffer('b')], count, 116);
    assert.ok(indices.length < 500);
    assert.equal(indices[0], 0);
    assert.equal(indices.at(-1), count - 1);
    assert.ok(indices.includes(12345));
    assert.ok(indices.includes(87654));
});

test('proper-time reduction preserves one coherent source observation', () => {
    const shared = { sampleTick: 41, sourceEpoch: 7, epoch: 9, source: 'wasm' };
    const result = reduceProperTimeSamples({
        tau: { ...shared, values: new Float32Array([2, 4]), count: 2, effectiveStride: 3 },
        lapse: { ...shared, values: new Float32Array([0.8, 1]), count: 2, effectiveStride: 3 },
        phase: { ...shared, values: new Float32Array([0, Math.PI]), count: 2, effectiveStride: 3 },
    });
    assert.equal(result.coherent, true);
    assert.equal(result.sampleTick, 41);
    assert.equal(result.sourceEpoch, 7);
    assert.equal(result.source, 'wasm');
    assert.equal(result.properTimeMean, 3);
    assert.ok(Math.abs(result.lapseMean - 0.9) < 1e-6);
});

test('proper-time reduction rejects mixed sample provenance', () => {
    const base = { sourceEpoch: 7, epoch: 9, source: 'wasm', values: new Float32Array([1]), count: 1 };
    const result = reduceProperTimeSamples({
        tau: { ...base, sampleTick: 41 },
        lapse: { ...base, sampleTick: 42 },
        phase: { ...base, sampleTick: 41 },
    });
    assert.equal(result.coherent, false);
    assert.equal(result.sampleTick, null);
    assert.ok(Number.isNaN(result.properTimeMean));
    assert.ok(Number.isNaN(result.lapseMean));
});

test('proper-time reduction cannot borrow provenance for an unversioned channel', () => {
    const stamped = { sampleTick: 41, sourceEpoch: 7, epoch: 9, source: 'wasm',
        values: new Float32Array([1]), count: 1 };
    const missing = { sampleTick: null, sourceEpoch: null, epoch: null, source: null,
        values: new Float32Array([2]), count: 1 };
    const result = reduceProperTimeSamples({ tau: stamped, lapse: missing, phase: stamped });
    assert.equal(result.coherent, false);
    assert.equal(result.sampleTick, null);
    assert.equal(result.sourceEpoch, null);
    assert.ok(Number.isNaN(result.properTimeMean));
    assert.ok(Number.isNaN(result.lapseMean));
});

test('large lattice panel sampling stays bounded for browser WASM owners', () => {
    assert.equal(p1GravitySampleStride(33), 2);
    assert.equal(p1GravitySampleStride(65), 3);
    assert.equal(p1GravitySampleStride(97), 4);
    assert.equal(interactiveProbeStride({ latticeSize: 33 }), 1);
    assert.equal(interactiveProbeStride({ latticeSize: 65 }), 2);
    assert.equal(interactiveProbeStride({ latticeSize: 97 }), 3);

    assert.equal(effectiveFluxSliceUpdateEvery({ latticeSize: 33 }, 2), 2);
    assert.equal(effectiveFluxSliceUpdateEvery({ latticeSize: 65 }, 2), 6);
    assert.equal(effectiveFluxSliceUpdateEvery({ latticeSize: 97 }, 2), 8);
    assert.equal(effectiveFluxSliceUpdateEvery({ latticeSize: 33, isNativeGPU: true }, 2), 4);
});

test('anisotropy reads a measured plane without requiring an L cubed volume', () => {
    const L = 33;
    const slice = new Float64Array(L * L).fill(2);
    const rows = circularFluxDecayFromZSlice(slice, L, 16, 16);
    assert.equal(rows.length, 8);
    assert.ok(rows.every(row => row.mean === 2 && row.aniso === 0));
    assert.equal(circularFluxDecayFromZSlice(new Float64Array(2), L, 16, 16), null);
});

test('Wave Lab owns coherent sampler demand without direct worker getter owners', () => {
    const wants = [];
    const emptyVector = () => ({
        positions: new Float32Array(0),
        vectors: new Float32Array(0),
        count: 0,
    });
    const bridge = {
        latticeSize: 9,
        capabilities: { scale0: { getScale0FieldSamples: () => emptyVector() } },
        replaceSamplerWants: (owner, keys) => wants.push([owner, keys]),
        hasSamplerSnapshot: () => true,
        getSamplerSnapshotVersion: () => 7,
    };
    const result = getSpectrumComparatorMetrics(bridge, undefined, { samplerOwner: 'wave-test' });
    assert.equal(result.active, true);
    assert.deepEqual(wants, [['wave-test', ['fluxVector@1', 'e@1']]]);

    bridge.getSamplerSnapshotVersion = kind => kind === 'fluxVector' ? 8 : 9;
    const mixed = getSpectrumComparatorMetrics(bridge, undefined, { samplerOwner: 'wave-test' });
    assert.equal(mixed.active, false);
    assert.equal(mixed.reason, 'waiting for coherent field frame');
});

test('owned live panels use bounded sampler and coordinator contracts', async () => {
    const root = new URL('../js/scales/scale0/ui/overlays/', import.meta.url);
    const [time, transaction, anisotropy, p1Gravity, gravity, thermo] = await Promise.all([
        readFile(new URL('time-panel.js', root), 'utf8'),
        readFile(new URL('transaction-panel.js', root), 'utf8'),
        readFile(new URL('p1-observables/anisotropy.js', root), 'utf8'),
        readFile(new URL('p1-observables/gravity.js', root), 'utf8'),
        readFile(new URL('gravity-panel.js', root), 'utf8'),
        readFile(new URL('thermo-panel.js', root), 'utf8'),
    ]);
    assert.doesNotMatch(time, /['`]tau@1['`]|['`]lapse@1['`]|['`]dbPhase@1['`]/);
    assert.doesNotMatch(time, /publishScale0ProperTimeMetrics|reduceProperTimeSamples/);
    assert.match(time, /getScale0TelemetryMeta\?\.\('properTime'\)/);
    assert.match(time, /replaceSamplerWants\?\.\('time-panel', \[\s*`latency@\$\{sampleStride\}`/);
    assert.doesNotMatch(transaction, /setInterval\s*\(/);
    assert.match(transaction, /rafCoordinator\.subscribe/);
    assert.doesNotMatch(anisotropy, /bridge\.getFluxVolume\?\.\(/);
    assert.match(p1Gravity, /caps\?\.getScale0FieldSamples\?\.\(\{ kind: 'latency', stride \}\)/);
    assert.match(p1Gravity, /replaceSamplerWants\?\.\(PANEL_SAMPLER_OWNER, \[sampleKey\]\)/);
    assert.match(gravity, /projectHistoryIndices\(historyBuffers, count, 116\)/);
    assert.match(thermo, /projectHistoryIndices\(\[historyBuffer\], mHist\.length, 240\)/);
});
