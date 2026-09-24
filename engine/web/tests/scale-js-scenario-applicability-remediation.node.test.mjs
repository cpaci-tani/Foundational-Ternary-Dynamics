import test from 'node:test';
import assert from 'node:assert/strict';

import { ScenarioApplicabilityBinding } from '../js/ui/utils/scenario-applicability-binding.js';
import {
    SCALE0_SCENARIO_VISUAL_PROFILES,
    applyScenarioCameraFocus,
} from '../js/scales/scale0/runtime/scenario-visual-profile.js';

function installFrameQueue() {
    const priorRaf = globalThis.requestAnimationFrame;
    const priorCancel = globalThis.cancelAnimationFrame;
    const frames = new Map();
    let next = 0;
    globalThis.requestAnimationFrame = (cb) => { frames.set(++next, cb); return next; };
    globalThis.cancelAnimationFrame = (id) => frames.delete(id);
    return {
        flush() {
            const callbacks = [...frames.values()];
            frames.clear();
            callbacks.forEach((cb) => cb(0));
        },
        count: () => frames.size,
        restore() {
            globalThis.requestAnimationFrame = priorRaf;
            globalThis.cancelAnimationFrame = priorCancel;
        },
    };
}

test('selector replacement has one listener and old selectors become inert', () => {
    const priorDocument = globalThis.document;
    const first = new EventTarget();
    first.value = 'first';
    const second = new EventTarget();
    second.value = 'second';
    let active = first;
    globalThis.document = { getElementById: () => active };
    const intents = [];
    const binding = new ScenarioApplicabilityBinding({ onIntent: (id) => intents.push(id) });
    try {
        binding.bind();
        active = second;
        binding.bind();
        first.value = 'stale';
        first.dispatchEvent(new Event('change'));
        second.value = 'live';
        second.dispatchEvent(new Event('change'));
        assert.deepEqual(intents, ['first', 'second', 'live']);
        binding.dispose();
        second.value = 'disposed';
        second.dispatchEvent(new Event('change'));
        assert.deepEqual(intents, ['first', 'second', 'live']);
    } finally {
        binding.dispose();
        globalThis.document = priorDocument;
    }
});

test('new scenario intent cancels superseded reconciliation frames', () => {
    const priorDocument = globalThis.document;
    globalThis.document = { getElementById: () => null };
    const queue = installFrameQueue();
    let readyId = '';
    const settled = [];
    let binding;
    binding = new ScenarioApplicabilityBinding({
        onIntent: (id) => binding.reconcile({
            scenarioId: id,
            isReady: (scenarioId) => scenarioId === readyId,
            onReady: (scenarioId) => settled.push(scenarioId),
            maxFrames: 5,
        }),
    });
    try {
        binding.intent('old');
        assert.equal(queue.count(), 1);
        binding.intent('new');
        assert.equal(queue.count(), 1);
        readyId = 'new';
        queue.flush();
        assert.deepEqual(settled, ['new']);
        assert.equal(queue.count(), 0);
    } finally {
        binding.dispose();
        queue.restore();
        globalThis.document = priorDocument;
    }
});

test('extracted scenario visual owner preserves compact-seed camera framing', () => {
    assert.equal(SCALE0_SCENARIO_VISUAL_PROFILES['s0-seed-moore-cell'].focusRadius, 5);
    const position = { x: 10, y: 8, z: 12, set(x, y, z) { Object.assign(this, { x, y, z }); } };
    const target = { x: 0, y: 0, z: 0, set(x, y, z) { Object.assign(this, { x, y, z }); } };
    let updates = 0;
    const ctx = { viewport: {
        camera: { position, fov: 60 },
        controls: { target, minDistance: 1, maxDistance: 100, update: () => updates++ },
    } };
    assert.equal(applyScenarioCameraFocus(ctx, 's0-seed-moore-cell', 181, true), true);
    assert.deepEqual([target.x, target.y, target.z], [90.5, 90.5, 90.5]);
    assert.equal(updates, 1);
    assert.equal(applyScenarioCameraFocus(ctx, 's0-seed-moore-cell', 181, false), false);
});
