import test from 'node:test';
import assert from 'node:assert/strict';
import { readFileSync } from 'node:fs';
import { SCALE0_GRID_CHANNELS } from '../js/telemetry/registry/scale0-grid-channels.js';
import {
    properTimeAvailabilityText,
    telemetryValueState,
} from '../js/ui/panels/telemetry-grid/component.js';

const properKeys = new Set([
    'properTimeMean', 'properTimeSpread', 'lapseMean', 'dbPhaseMean', 'dbPhaseCircVar',
]);
const properChannels = SCALE0_GRID_CHANNELS.filter(channel => properKeys.has(channel.key));

test('all proper-time Grid channels use canonical group freshness', () => {
    assert.equal(properChannels.length, properKeys.size);
    assert.deepEqual(properChannels.map(channel => channel.telemetryGroup),
        Array(properKeys.size).fill('properTime'));
});

test('retained finite history is never presented as current after proper-time invalidation', () => {
    const channel = properChannels[0];
    assert.equal(telemetryValueState('0', channel, {
        status: 'available', stale: false, tick: 24,
    }, 1.25), 'current');
    assert.equal(telemetryValueState('0', channel, {
        status: 'unavailable', stale: true, tick: 24,
        unavailableReason: 'source-owner-changed',
    }, 1.25), 'stale');
    assert.equal(telemetryValueState('0', channel, {
        status: 'unavailable', stale: false, tick: null,
        unavailableReason: 'no-manifested-voxel-support',
    }, 1.25), 'unavailable');
});

test('proper-time reasons distinguish pending, disabled, zero support, mismatch, and stale owner', () => {
    assert.equal(properTimeAvailabilityText(null), 'Waiting for proper-time samples.');
    assert.match(properTimeAvailabilityText({ status: 'unavailable', unavailableReason: 'proper-time-disabled' }), /disabled/);
    assert.match(properTimeAvailabilityText({ status: 'unavailable', unavailableReason: 'proper-time-sampler-unavailable' }), /cannot provide/);
    assert.match(properTimeAvailabilityText({ status: 'unavailable', unavailableReason: 'no-manifested-voxel-support' }), /No manifested clock samples/);
    assert.match(properTimeAvailabilityText({ status: 'unavailable', unavailableReason: 'proper-time-disabled', unavailableDetail: 'no-manifested-voxel-support' }), /Open Time.*No manifested clock samples/);
    assert.match(properTimeAvailabilityText({ status: 'unavailable', unavailableReason: 'proper-time-provenance-mismatch' }), /one source and tick/);
    assert.match(properTimeAvailabilityText({ status: 'unavailable', stale: true, unavailableReason: 'source-owner-changed' }), /Source changed/);
    assert.match(properTimeAvailabilityText({ status: 'available', stale: false, tick: 24 }), /No current sample/);
    assert.equal(properTimeAvailabilityText({ status: 'available', stale: false, tick: 24 }, true), '');
});

test('Grid template and CSS expose the reason inside the plot region', () => {
    const component = readFileSync(new URL('../js/ui/panels/telemetry-grid/component.js', import.meta.url), 'utf8');
    const css = readFileSync(new URL('../css/ui/panels/telemetry-grid.css', import.meta.url), 'utf8');
    assert.match(component, /class="telemetry-card-status" role="status"/);
    assert.match(component, /entry\.statusEl\.hidden = !reason/);
    assert.match(css, /\.telemetry-card-status\s*\{/);
    assert.match(css, /\.telemetry-card-status\[hidden\]/);
});
