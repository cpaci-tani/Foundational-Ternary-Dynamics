import test from 'node:test';
import assert from 'node:assert/strict';

import { registerScaleToolbarFactories } from '../js/ui/utils/scale-toolbar.js';
import {
    getScale5ComovingGrid,
    setScale5ComovingGrid,
    syncScale5Overlays,
} from '../js/scales/scale5/ui/overlays/component.js';

test('toolbar factory registration supplies one shared slot and scale contract', () => {
    const registered = [];
    registerScaleToolbarFactories({ registerFactory: (value) => registered.push(value) }, 4, [
        { id: 'scenario', order: 10, factory: () => 'scenario' },
        { id: 'telemetry', order: 20, factory: () => 'telemetry' },
    ]);
    assert.deepEqual(registered.map(({ id, order, slot, scales }) => ({ id, order, slot, scales })), [
        { id: 'scenario', order: 10, slot: 'secondary', scales: ['4'] },
        { id: 'telemetry', order: 20, slot: 'secondary', scales: ['4'] },
    ]);
});

test('Scale 5 shared grid setter persists and synchronizes renderer plus private checkbox', () => {
    const priorDocument = globalThis.document;
    const priorEvent = globalThis.Event;
    const checkbox = { checked: false };
    const events = [];
    globalThis.Event = priorEvent || class { constructor(type) { this.type = type; } };
    globalThis.document = {
        getElementById: (id) => id === 'cosmic-overlay-comoving-grid' ? checkbox : null,
        dispatchEvent: (event) => events.push(event.type),
    };
    const values = [];
    const renderer = { setComovingGrid: (value) => values.push(value) };
    try {
        syncScale5Overlays(renderer);
        setScale5ComovingGrid(true);
        assert.equal(getScale5ComovingGrid(), true);
        assert.equal(checkbox.checked, true);
        assert.equal(values.at(-1), true);
        syncScale5Overlays({ setComovingGrid: (value) => values.push(value) });
        assert.equal(values.at(-1), true);
        assert.ok(events.every((type) => type === 'ftd:view-controls-changed'));
    } finally {
        setScale5ComovingGrid(false);
        syncScale5Overlays(null);
        globalThis.document = priorDocument;
        globalThis.Event = priorEvent;
    }
});
