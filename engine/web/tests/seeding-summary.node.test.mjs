import { test } from 'node:test';
import assert from 'node:assert/strict';
import { recipeText } from '../js/seeding/recipe-summary.js';

const shapeOptions = [['point', 'Single site'], ['box', 'Box'], ['sphere', 'Sphere'], ['all', 'Entire lattice']];

function finiteRecipe() {
    return {
        version: 2, scenarioId: 'record-base', size: 9, blank: false, randomSeed: 42,
        components: [
            {
                id: 'c1', kind: 'relation_background', enabled: true,
                region: { shape: 'box', x: 1, y: 2, z: 3, dx: 4, dy: 5, dz: 6, radius: 0 },
                parameters: { target: 'both', slots: [0, 1], phase: 2, polarity: -1 },
            },
            {
                id: 'c2', kind: 'field_list', enabled: false,
                region: { shape: 'all', x: 0, y: 0, z: 0, dx: 9, dy: 9, dz: 9, radius: 0 },
                parameters: { channels: [5, 10, 15, 20, 25, 30, 35], occupied: true },
            },
        ],
    };
}
function finiteDescription() {
    return [
        { path: ['size'], label: 'Lattice size', value: 9, default: 9, units: '', options: [[9, '9']] },
        { path: ['randomSeed'], label: 'Recipe random seed', value: 42, default: 0, units: '' },
        { path: ['components', 0, 'enabled'], label: 'Enabled', value: true, default: false, options: [[false, 'Off — disabled'], [true, 'On — enabled']] },
        { path: ['components', 0, 'region', 'shape'], label: 'Shape', value: 'box', default: 'point', options: shapeOptions },
        { path: ['components', 0, 'parameters', 'target'], label: 'Relation background target', value: 'both', default: 'both', options: [['field', 'Field'], ['relation', 'Relation'], ['both', 'Both']] },
        { path: ['components', 0, 'parameters', 'slots'], label: 'Slots', value: [0, 1], default: [0], units: '' },
        { path: ['components', 0, 'parameters', 'phase'], label: 'Phase', value: 2, default: 0, options: [[0, '0'], [1, '1'], [2, '2'], [3, '3']] },
        { path: ['components', 0, 'parameters', 'polarity'], label: 'Polarity', value: -1, default: 1, options: [[1, '+1'], [-1, '−1']] },
        { path: ['components', 1, 'enabled'], label: 'Enabled', value: false, default: true, options: [[false, 'Off — disabled'], [true, 'On — enabled']] },
        { path: ['components', 1, 'region', 'shape'], label: 'Shape', value: 'all', default: 'all', options: shapeOptions },
        { path: ['components', 1, 'parameters', 'channels'], label: 'Channels', value: [5, 10, 15, 20, 25, 30, 35], default: [], units: '' },
        { path: ['components', 1, 'parameters', 'occupied'], label: 'Occupied', value: true, default: true, options: [[false, 'Off — cleared'], [true, 'On — occupied']] },
    ];
}
const finiteScenario = { id: 'record-base', title: 'Base Record', category: '1. Validated Native Dynamics', backend: 'finite-records', epistemicStatus: '[IMPOSED] test fixture' };

test('overview reports the exact custom-property count from resolved defaults, not a guess', () => {
    const text = recipeText(finiteRecipe(), finiteScenario, finiteDescription());
    assert.match(text, /Custom preparation — 8 properties differ from the registered default\./);
    assert.match(text, /Lattice size: 9³ sites/);
});

test('ingredients render in list order with decoded enums, exact array rows, and muted disabled components', () => {
    const text = recipeText(finiteRecipe(), finiteScenario, finiteDescription());
    const first = text.indexOf('1. Relation background');
    const second = text.indexOf('2. Exact field list');
    assert.ok(first >= 0 && second > first, 'components must appear in recipe order');
    assert.match(text, /Component ID: c1/);
    assert.match(text, /Region shape: 1 — Box/);
    assert.match(text, /Origin: \(1, 2, 3\)/);
    assert.match(text, /Box extent: 4 × 5 × 6/);
    assert.match(text, /Relation background target: 2 — Both/);
    assert.match(text, /Slots: 0, 1/);
    assert.match(text, /Phase: 2 — 2/);
    assert.match(text, /Polarity: -1 — −1/);
    assert.match(text, /2\. Exact field list \(disabled\)/);
    assert.match(text, /Channels: 5, 10, 15, 20, 25, 30, 35/);
    assert.doesNotMatch(text, /Channels: 5, 10, 15, 20, 25, 30, 35,/, 'no truncation or invented extra entries');
});

test('randomness section names the recipe seed and never a component seed that is absent', () => {
    const text = recipeText(finiteRecipe(), finiteScenario, finiteDescription());
    assert.match(text, /Recipe random seed: 42/);
    assert.doesNotMatch(text, /c1 ·/);
    assert.doesNotMatch(text, /c2 ·/);
});

test('finite preparation protocol section states the fixed law context and stays read-only', () => {
    const text = recipeText(finiteRecipe(), finiteScenario, finiteDescription());
    assert.match(text, /Fixed law context, read-only/);
});

test('resolved results shows only receipt-supported quantities and nothing when absent', () => {
    const withoutReceipt = recipeText(finiteRecipe(), finiteScenario, finiteDescription());
    assert.match(withoutReceipt, /No compiled receipt is available\. Resolved counts are unknown/);
    const withReceipt = recipeText(finiteRecipe(), finiteScenario, finiteDescription(), {
        receipt: { fieldTokens: 120, relationTokens: 80, manifested: 3, note: 'extra-supplied-field' },
    });
    assert.match(withReceipt, /Field tokens written: 120/);
    assert.match(withReceipt, /Relation tokens written: 80/);
    assert.match(withReceipt, /Manifested markers: 3/);
    assert.match(withReceipt, /note: extra-supplied-field/);
    assert.doesNotMatch(withReceipt, /Checkpoint SHA-256/);
    assert.doesNotMatch(withReceipt, /Tick:/);
});

test('provenance flags a custom finite preparation as not inheriting registered evidence', () => {
    const text = recipeText(finiteRecipe(), finiteScenario, finiteDescription());
    assert.match(text, /This is a custom initial state\. Registered evidence remains attached to the original preset only/);
});

test('an unmodified finite recipe is reported as the original registered preset', () => {
    const recipe = finiteRecipe();
    const description = finiteDescription().map(p => ({ ...p, default: p.value }));
    const text = recipeText(recipe, finiteScenario, description);
    assert.match(text, /Original registered preset, unmodified\./);
    assert.match(text, /Matches the registered preset exactly and inherits its registered evidence\./);
});

test('applied/draft status compares conservatively against a boolean or a full recipe, never guessing', () => {
    const recipe = finiteRecipe(), description = finiteDescription();
    assert.match(recipeText(recipe, finiteScenario, description), /Not compared against the active lattice\./);
    assert.match(recipeText(recipe, finiteScenario, description, { applied: true }), /Applied to the active lattice\./);
    assert.match(recipeText(recipe, finiteScenario, description, { applied: false }), /Draft only — the active lattice is unchanged\./);
    assert.match(recipeText(recipe, finiteScenario, description, { applied: JSON.parse(JSON.stringify(recipe)) }), /Applied to the active lattice\./);
    const altered = JSON.parse(JSON.stringify(recipe)); altered.randomSeed = 99;
    assert.match(recipeText(recipe, finiteScenario, description, { applied: altered }), /Draft only — differs from the last applied preparation\./);
});

const nativeScenario = { id: 'flux-pulse', title: 'Transverse Packet — Finite-Box Boundary Test', category: '1. Validated Native Dynamics', backend: 'native', intent: 'Probe the finite-box boundary law.', epistemicStatus: '[EMERGENT] under [IMPOSED] computational boundary laws' };
function nativeRecipe() {
    return { version: 2, scenarioId: 'flux-pulse', size: 17, blank: false, randomSeed: 0, components: [], overrides: { 'protocol.boundary': 2, 'source.x': 5, 'packet.amplitude': 2.5, 'random.streamSeed': 7 } };
}
function nativeDescription() {
    return [
        { path: ['overrides', 'protocol.boundary'], label: 'Boundary', group: 'protocol', value: 2, default: 2, options: [[1, 'Reflective'], [2, 'Periodic']] },
        { path: ['overrides', 'source.x'], label: 'Packet start x', group: 'source', value: 5, default: 8, units: 'lattice units' },
        { path: ['overrides', 'packet.amplitude'], label: 'Packet amplitude', group: 'packet', value: 2.5, default: 1, units: 'lattice units' },
        { path: ['overrides', 'random.streamSeed'], label: 'Stream seed', group: 'random', value: 7, default: 0, units: '' },
    ];
}

test('a named native preparation exposes all resolved ingredients', () => {
    const text = recipeText(nativeRecipe(), nativeScenario, nativeDescription());
    const section = text.slice(text.indexOf('## Ordered ingredients'), text.indexOf('## Randomness'));
    assert.match(section, /Packet start x: 5 lattice units/);
    assert.match(section, /Packet amplitude: 2.5 lattice units/);
});

test('native preparation protocol groups resolved properties in constructor (list) order, excluding randomness', () => {
    const text = recipeText(nativeRecipe(), nativeScenario, nativeDescription());
    const section = text.slice(text.indexOf('## Preparation protocol'), text.indexOf('## Resolved results'));
    assert.ok(section.includes('- protocol'));
    assert.doesNotMatch(section, /- source|- packet/);
    assert.doesNotMatch(section, /- random/);
    assert.match(section, /Boundary: 2 — Periodic/);
});

test('native randomness section surfaces only the random-group property, by its own label', () => {
    const text = recipeText(nativeRecipe(), nativeScenario, nativeDescription());
    const section = text.slice(text.indexOf('## Randomness'), text.indexOf('## Preparation protocol'));
    assert.match(section, /Stream seed: 7/);
});

test('native provenance lists every explicit override by name and flags the preparation as custom', () => {
    const text = recipeText(nativeRecipe(), nativeScenario, nativeDescription());
    assert.match(text, /4 explicit override\(s\) applied over the frozen constructor default: protocol\.boundary, source\.x, packet\.amplitude, random\.streamSeed\./);
    assert.match(text, /Edits here are custom preparations and do not inherit transport\/recovery qualification from the parent preset\./);
});

test('a native preparation with no overrides reports none and is not flagged custom', () => {
    const recipe = { ...nativeRecipe(), overrides: {} };
    const description = nativeDescription().map(p => ({ ...p, default: p.value }));
    const text = recipeText(recipe, nativeScenario, description);
    assert.match(text, /Explicit overrides: None — the frozen constructor defaults are used unmodified\./);
    assert.doesNotMatch(text, /do not inherit transport\/recovery qualification/);
});

test('a blank multi-preparation native build lists each ingredient with its own named preparation and overrides', () => {
    const recipe = {
        version: 2, scenarioId: 'flux-pulse', size: 17, blank: true, randomSeed: 0,
        components: [
            { id: 'ingredient-1', kind: 'native', enabled: true, scenarioId: 'empty', overrides: { 'protocol.boundary': 1 } },
            { id: 'ingredient-2', kind: 'native', enabled: false, scenarioId: 'flux-dipole', overrides: {} },
        ],
        overrides: {},
    };
    const description = [
        { path: ['components', 0, 'enabled'], label: 'Enabled', value: true, default: true, options: [[false, 'Off'], [true, 'On']] },
        { path: ['components', 0, 'overrides', 'protocol.boundary'], label: 'Boundary', group: 'protocol', value: 1, default: 2, options: [[1, 'Reflective'], [2, 'Periodic']] },
        { path: ['components', 1, 'enabled'], label: 'Enabled', value: false, default: true, options: [[false, 'Off'], [true, 'On']] },
    ];
    const text = recipeText(recipe, nativeScenario, description);
    assert.match(text, /Multi-preparation native build/);
    assert.match(text, /1\. empty/);
    assert.match(text, /Named preparation: empty/);
    assert.match(text, /Boundary: 1 — Reflective/);
    assert.match(text, /2\. flux-dipole \(disabled\)/);
});
