// Static validity of the shipped loop recipes (engine/web/recipes/).
//
// The recipes are hand-authored SeedRecipeV2 envelopes composed from existing
// native constructors. A typo in an override key is only reported by the
// engine at staging time, so this test checks every key and value against the
// constructor-generated schema before a browser is involved. The physics the
// recipes are meant to show is asserted on the real WASM engine in
// loop-recipes.spec.js.
import test from 'node:test';
import assert from 'node:assert/strict';
import {readFileSync} from 'node:fs';
import {SCALE0_SCENARIOS} from '../js/scales/scale0/scenario-registry.js';
import {ENVELOPE_FORMAT, RECIPE_VERSION, parseRecipeEnvelope, isCustomNativeRecipe} from '../js/seeding/recipe.js';
import {NATIVE_SEED_IDS} from '../js/seeding/generated/native-seed-ids.js';
import {MAX_WASM_INTERACTIVE_LATTICE} from '../js/scales/scale0/ui/toolbar/limits.js';

const RECIPES = ['loop-repair-seed.json', 'loop-pair-seed.json'];
const DASHBOARD_DEFAULT_LATTICE = 33;
const nativeSchemas = JSON.parse(readFileSync(new URL('../js/seeding/generated/native-seeds.json', import.meta.url))).presets;
const read = name => readFileSync(new URL(`../recipes/${name}`, import.meta.url), 'utf8');

// Constructor defaults follow the lattice size, but keys and bounds for these
// constructors are registered at 33; the largest registered size is the
// schema the dashboard itself falls back to (native-runtime.js).
const schemaFor = id => nativeSchemas[`${id}@33`];

function assertWithinSchema(schema, key, value, where) {
    const property = schema.properties.find(p => p.key === key);
    assert.ok(property, `${where}: '${key}' is not a seed property of ${schema.scenarioId}`);
    assert.equal(typeof value, 'number', `${where}: '${key}' must be numeric`);
    // Position and radius bounds scale with N; only fixed bounds are checked.
    if (!/^(source|geometry)\./.test(key) || property.type === 'choice') {
        assert.ok(value >= property.min && value <= property.max,
            `${where}: '${key}' = ${value} is outside [${property.min}, ${property.max}]`);
    }
    if (property.type === 'integer' || property.type === 'choice' || property.type === 'boolean')
        assert.ok(Number.isInteger(value), `${where}: '${key}' must be an integer`);
}

for (const name of RECIPES) {
    test(`${name} is a valid, stageable composite recipe`, () => {
        const text = read(name);
        const envelope = JSON.parse(text);
        assert.equal(envelope.format, ENVELOPE_FORMAT);
        assert.equal(envelope.version, RECIPE_VERSION);
        assert.ok(envelope.title && envelope.description, 'recipes carry a title and a description');

        // The dashboard's own import path.
        const recipe = parseRecipeEnvelope(text, SCALE0_SCENARIOS);
        assert.deepEqual(recipe, envelope.recipe);
        assert.equal(recipe.blank, true);
        assert.ok(isCustomNativeRecipe(recipe));
        assert.deepEqual(envelope.domain, {kind: 'native', size: recipe.size});
        assert.equal(envelope.provenance.parentPreset, recipe.scenarioId);

        // prepareNativeRecipe refuses even sizes and sizes above the worker limit.
        assert.equal(recipe.size % 2, 1);
        assert.ok(recipe.size >= 3 && recipe.size <= MAX_WASM_INTERACTIVE_LATTICE);
        // The seeding panel stages a native recipe at the toolbar's lattice
        // size, not at the size the file carries (panel.js: recipe.size =
        // bridge.latticeSize). A recipe with absolute positions must therefore
        // be authored for the default lattice or it is refused on import.
        assert.equal(recipe.size, DASHBOARD_DEFAULT_LATTICE);
        for (const component of recipe.components) {
            for (const axis of ['x', 'y', 'z']) {
                const value = component.overrides[`source.${axis}`];
                if (value !== undefined) assert.ok(value >= 0 && value <= recipe.size - 1,
                    `${component.id}: source.${axis} = ${value} is outside the ${recipe.size} lattice`);
            }
        }

        assert.ok(recipe.components.length >= 2);
        for (const component of recipe.components) {
            assert.equal(component.enabled, true, `${component.id} must be enabled`);
            assert.ok(NATIVE_SEED_IDS.includes(component.scenarioId), `${component.scenarioId} is not a native constructor`);
            const schema = schemaFor(component.scenarioId);
            assert.ok(schema, `no generated schema for ${component.scenarioId}`);
            for (const [key, value] of Object.entries(component.overrides))
                assertWithinSchema(schema, key, value, `${name}:${component.id}`);
        }
        const anchor = schemaFor(recipe.scenarioId);
        for (const [key, value] of Object.entries(recipe.overrides)) {
            assert.match(key, /^protocol\./, 'recipe-level overrides are protocol settings only');
            assertWithinSchema(anchor, key, value, `${name}:recipe`);
        }
    });
}

test('a mistyped or out-of-range override is caught statically', () => {
    const schema = schemaFor('s0-cell-torus');
    assert.throws(() => assertWithinSchema(schema, 'geometry.majorRadious', 6, 'probe'), /not a seed property/);
    assert.throws(() => assertWithinSchema(schema, 'ring.circulation', 3, 'probe'), /outside/);
    assert.throws(() => assertWithinSchema(schema, 'ring.signSectors', 1.5, 'probe'), /integer/);
});

test('the canned loop scenarios are registered, admitted and presented as specified', async () => {
    const {SCALE0_SCENARIO_VISUAL_PROFILES} = await import('../js/scales/scale0/runtime/scenario-visual-profile.js');
    const {SCALE0_SCENARIO_OVERRIDES, SCALE0_SCENARIO_BOUNDARY} = await import('../js/config/toggles.js');
    for (const id of ['s0-cell-loop-repair', 's0-cell-loop-pair']) {
        const scenario = SCALE0_SCENARIOS.find(row => row.id === id);
        assert.ok(scenario, `${id} is in the picker registry`);
        assert.ok(NATIVE_SEED_IDS.includes(id), `${id} is a native constructor`);
        assert.equal(scenario.category, 'Energy · Storage & Boundaries');
        assert.doesNotMatch(scenario.qualification, /RESEARCH SETUP/);
        assert.deepEqual(SCALE0_SCENARIO_BOUNDARY[id], {mode: 0});
        assert.ok(schemaFor(id), `${id} has a generated seed schema`);
    }
    const on = id => SCALE0_SCENARIO_OVERRIDES[id].filter(([, enabled]) => enabled === true).map(([key]) => key);
    assert.deepEqual(on('s0-cell-loop-repair'), ['gauss_projection']);
    assert.deepEqual(on('s0-cell-loop-pair'), ['wave_propagation']);
    // One projection pass removes most of the gradient blob, so loop repair is
    // shown at its seeded state; no other profile opts out of the prime tick.
    const holding = Object.entries(SCALE0_SCENARIO_VISUAL_PROFILES).filter(([, p]) => p.holdSeededState).map(([id]) => id);
    assert.deepEqual(holding, ['s0-cell-loop-repair']);
});

test('the two recipes keep separate drafts and the protocols the spec states', () => {
    const [repair, pair] = RECIPES.map(name => parseRecipeEnvelope(read(name), SCALE0_SCENARIOS));
    // The seeding panel stores one imported draft per anchor scenario.
    assert.notEqual(repair.scenarioId, pair.scenarioId);
    // Loop repair: projection is the only active term.
    assert.equal(repair.overrides['protocol.gauss_projection'], 1);
    assert.equal(repair.overrides['protocol.wave_propagation'], 0);
    // Both run on the periodic box.
    assert.equal(repair.overrides['protocol.boundary'], 0);
    assert.equal(pair.overrides['protocol.boundary'], 0);
    // Loop pair: the coherent ring is seeded last so the regional ledger follows it.
    assert.deepEqual(pair.components.map(c => c.scenarioId), ['s0-cell-torus-scrambled', 's0-cell-torus']);
});
