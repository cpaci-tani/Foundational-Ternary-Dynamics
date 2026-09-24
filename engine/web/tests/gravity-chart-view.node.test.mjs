import test from 'node:test';
import assert from 'node:assert/strict';
import {
    gravitySparklineGeometry,
    histogramAriaLabel,
    histogramPath,
} from '../js/scales/scale0/ui/overlays/gravity-chart-view.js';

test('Gravity sparkline x positions follow actual irregular observation ticks', () => {
    const geometry = gravitySparklineGeometry([
        { tick: 10, value: 0 },
        { tick: 20, value: 1 },
        { tick: 50, value: 0 },
    ], row => row.value, 100, 40);
    assert.equal(geometry.tickMin, 10);
    assert.equal(geometry.tickMax, 50);
    assert.match(geometry.path, /^M0\.00,38\.00 L25\.00,2\.00 L100\.00,38\.00$/);
});

test('Gravity constant traces are centered and a single sample stays visible', () => {
    const constant = gravitySparklineGeometry([
        { tick: 7, value: 3 }, { tick: 11, value: 3 },
    ], row => row.value, 100, 40);
    assert.equal(constant.path, 'M0.00,20.00 L100.00,20.00');
    const single = gravitySparklineGeometry([{ tick: 9, value: 3 }], row => row.value, 100, 40);
    assert.equal(single.path, 'M48.00,20.00 L52.00,20.00');
});

test('Gravity sparkline rejects null or unknown ticks without revision fallback', () => {
    const geometry = gravitySparklineGeometry([
        { tick: null, ver: 91, value: 1 },
        { ver: 92, value: 2 },
        { tick: -1, ver: 93, value: 3 },
    ], row => row.value, 100, 40);
    assert.deepEqual(geometry, {
        path: '', count: 0, tickMin: null, tickMax: null, valueMin: null, valueMax: null,
    });
});

test('Gravity histogram omits empty bars and reports its sampled range', () => {
    assert.equal(histogramPath({ counts: [0, 0], min: 0, max: 0 }), '');
    const path = histogramPath({ counts: [1, 2], min: 0.25, max: 0.75 }, 70, 20);
    assert.match(path, /^M0\.00,20V10\.00H34\.70V20Z/);
    assert.match(histogramAriaLabel('L proxy', {
        counts: [1, 2], min: 0.25, max: 0.75,
    }), /3 sampled values; range .* to .*; 2 bins/);
});
