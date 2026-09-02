import assert from 'node:assert/strict';
import test from 'node:test';

import {
    DEFAULT_TRAIL_SETTINGS,
    TRAIL_GLOBAL_SAMPLE_BUDGET,
    normalizeTrailSettings,
    trailRetentionAlpha,
} from '../js/scales/scale1/trail-settings.js';
import {
    clearCloudAndTrails,
    getTrailHistory,
    updateTrailHistory,
} from '../js/scales/scale1/pe-cloud-expander.js';

function particleData(position = 0) {
    return {
        count: 1,
        ids: new Uint32Array([7]),
        positions: new Float32Array([position, 0, 0]),
        velocities: new Float32Array([0.1, 0, 0]),
    };
}

const emptyData = {
    count: 0,
    ids: new Uint32Array(0),
    positions: new Float32Array(0),
    velocities: new Float32Array(0),
};

test('trajectory settings normalize to bounded tick-based values', () => {
    const settings = normalizeTrailSettings({
        historyTicks: 99999,
        sampleEveryTicks: 0,
        disappearDelayTicks: -4,
        opacity: 2,
        pointSize: 0,
        renderMode: 'not-a-mode',
    });
    assert.equal(settings.historyTicks, 1200);
    assert.equal(settings.sampleEveryTicks, 1);
    assert.equal(settings.disappearDelayTicks, 0);
    assert.equal(settings.opacity, 1);
    assert.equal(settings.pointSize, 0.08);
    assert.equal(settings.renderMode, 'breadcrumbs');
    assert.equal(settings.fadeExponent, DEFAULT_TRAIL_SETTINGS.fadeExponent);
});

test('trajectory allocation stays within the global sample budget at load', () => {
    clearCloudAndTrails();
    const count = 400;
    const data = {
        count,
        ids: Uint32Array.from({ length: count }, (_, id) => id),
        positions: new Float32Array(count * 3),
        velocities: new Float32Array(count * 3),
    };
    updateTrailHistory(data, 1, DEFAULT_TRAIL_SETTINGS);
    const trails = Array.from(getTrailHistory().values());
    assert.equal(trails.length, count);
    assert.ok(trails.reduce((sum, trail) => sum + trail.capacity, 0)
        <= TRAIL_GLOBAL_SAMPLE_BUDGET);
    clearCloudAndTrails();
});

test('trajectory sampling follows PE ticks and removed histories fade before pruning', () => {
    clearCloudAndTrails();
    const settings = normalizeTrailSettings({
        historyTicks: 100,
        sampleEveryTicks: 2,
        disappearDelayTicks: 4,
    });

    const nativeDensity = new Map([[7, 12.5]]);
    updateTrailHistory(particleData(0), 0, settings, nativeDensity);
    updateTrailHistory(particleData(1), 1, settings, nativeDensity);
    updateTrailHistory(particleData(2), 2, settings, nativeDensity);

    const trail = getTrailHistory().get(7);
    assert.equal(trail.length, 2, 'render frames inside the tick stride must not add samples');
    assert.equal(trail.lastSampleTick, 2);
    assert.equal(trail.energyDensities[0], 12.5);
    assert.equal(trail.energyDensities[1], 12.5);

    updateTrailHistory(emptyData, 3, settings);
    assert.equal(getTrailHistory().has(7), true);
    assert.equal(trail.inactiveSinceTick, 3);
    assert.equal(trailRetentionAlpha(trail, 5, settings), 0.5);

    updateTrailHistory(emptyData, 7, settings);
    assert.equal(getTrailHistory().has(7), true, 'the configured terminal tick is still visible');
    updateTrailHistory(emptyData, 8, settings);
    assert.equal(getTrailHistory().has(7), false, 'history prunes after the fade delay elapses');
    clearCloudAndTrails();
});
