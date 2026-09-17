import test from 'node:test';
import assert from 'node:assert/strict';
import {readFileSync} from 'node:fs';
import {SCALE0_SCENARIOS} from '../js/scales/scale0/scenario-registry.js';
import {
    RECIPE_VERSION, COMPONENT_KINDS, RECORD_SEED_SIZES, cloneRecipe, recipesEqual, getAtPath, setAtPath,
    createRecipe, createBaseRecipe, createNativeBaseRecipe, isCustomNativeRecipe, createComponent, validateRecipe,
    parseRecipe, parseRecipeEnvelope, buildRecipeEnvelope, ENVELOPE_FORMAT, migrateRecipe, seedCoverage, markFiniteOverride, hasFiniteOverride,
} from '../js/seeding/recipe.js';
import {findFiniteSchema, findFiniteBaseSchema, findComponentTemplates, collectionRowBounds, resetFiniteCatalogForTests} from '../js/seeding/catalog.js';
import {isEditedRecordRecipe} from '../js/seeding/runtime.js';

const finiteCatalog = JSON.parse(readFileSync(new URL('../js/seeding/generated/finite-seeds.json', import.meta.url)));
const record = {id: 'record-relation', backend: 'finite-records', category: 'Finite records', sizes: [3, 4, 7, 9]};
const scenarios = [...SCALE0_SCENARIOS, record];

test('every native preset has a typed constructor disposition and a valid default v2 recipe', () => {
    assert.equal(SCALE0_SCENARIOS.length, 143);
    for (const s of SCALE0_SCENARIOS) {
        const recipe = createRecipe(s, 33);
        assert.equal(recipe.version, RECIPE_VERSION);
        assert.deepEqual(recipe.overrides, {});
        assert.equal(validateRecipe(recipe, scenarios), s);
        assert.deepEqual(parseRecipe(JSON.stringify(recipe), scenarios), recipe);
    }
    const coverage = seedCoverage(scenarios);
    assert.equal(new Set(coverage.map(r => r.scenarioId)).size, scenarios.length);
    assert.equal(coverage.filter(r => r.parameterAudit === 'typed-constructor-bindings').length, 143);
    const recorded = JSON.parse(readFileSync(new URL('../../config/web_seeding_coverage.json', import.meta.url)));
    assert.deepEqual(recorded.scenarios.filter(r => r.capability === 'native-preparation').map(({scenarioId,category,capability,parameterAudit})=>({scenarioId,category,capability,parameterAudit})), coverage.slice(0, 143));
});

test('the generated catalog exposes a default v2 recipe for a registered finite preparation', () => {
    const schema = findFiniteSchema(finiteCatalog, 'record-relation', 3);
    assert.ok(schema);
    assert.equal(schema.recipe.version, RECIPE_VERSION);
    assert.equal(validateRecipe(schema.recipe, scenarios), record);
    assert.equal(schema.recipe.components[0].kind, 'relation');
});

test('the category base covers every finite kind, inactive, and rescales to an ungenerated size', () => {
    const base9 = findFiniteBaseSchema(finiteCatalog, 9);
    assert.deepEqual(base9.recipe.components.map(c => c.kind), Object.keys(COMPONENT_KINDS));
    assert.ok(base9.recipe.blank && base9.recipe.components.every(c => !c.enabled));

    const rescaled = findFiniteBaseSchema(finiteCatalog, 3);
    assert.equal(rescaled.recipe.size, 3);
    for (const c of rescaled.recipe.components) if (c.region.shape === 'all') {
        assert.equal(c.region.dx, 3); assert.equal(c.region.dy, 3); assert.equal(c.region.dz, 3);
    }
    const nTokens = rescaled.properties.find(p => p.path.at(-1) === 'nTokens');
    assert.equal(nTokens.max, 3 ** 3 * 9);

    const r = createBaseRecipe(record, rescaled);
    assert.equal(r.scenarioId, record.id);
    assert.equal(validateRecipe(r, scenarios), record);
    assert.throws(() => createBaseRecipe(SCALE0_SCENARIOS[0], rescaled), /parameterized native constructors/);
});

test('component templates and collection editors use constructor-authored defaults', () => {
    const base=findFiniteBaseSchema(finiteCatalog,3), templates=findComponentTemplates(finiteCatalog,3,base);
    assert.equal(templates.size,Object.keys(COMPONENT_KINDS).length);
    for(const c of base.collections) {
        assert.deepEqual(collectionRowBounds(c,c.rows),{canAdd:c.rows<c.maxRows,canRemove:c.rows>c.minRows,defaultRow:c.defaultRow});
        const rows=base.properties.filter(p=>p.path.length===c.path.length+1 && p.path.slice(0,-1).join('.')===c.path.join('.'));
        assert.equal(rows.length,c.rows);
        assert.ok(rows.every(p=>['integer','choice'].includes(p.type)&&p.binding));
    }
});

test('explicit finite overrides survive ingredient reordering and reset restores following defaults',()=>{
    const recipe=createBaseRecipe(record,findFiniteBaseSchema(finiteCatalog,3));
    const path=['components',0,'region','x'];
    markFiniteOverride(recipe,path);
    assert.ok(hasFiniteOverride(recipe,path));
    const moved=recipe.components.shift();recipe.components.push(moved);
    const movedPath=['components',recipe.components.length-1,'region','x'];
    assert.ok(hasFiniteOverride(recipe,movedPath));
    validateRecipe(recipe,scenarios);
    markFiniteOverride(recipe,movedPath,false);
    assert.ok(!Object.hasOwn(recipe,'explicitOverrides'));
});

test('getAtPath/setAtPath round-trip and recipesEqual is order-insensitive', () => {
    const a = {version: 2, x: {y: [1, 2]}};
    assert.equal(getAtPath(a, ['x', 'y', 1]), 2);
    setAtPath(a, ['x', 'y', 1], 9);
    assert.equal(a.x.y[1], 9);
    assert.ok(recipesEqual({a: 1, b: 2}, {b: 2, a: 1}));
    assert.ok(!recipesEqual({a: 1}, {a: 2}));
});

test('legacy v1 recipes migrate to v2 by expanding the registered default, or pass a blank base through', () => {
    const schema = findFiniteSchema(finiteCatalog, 'record-relation', 3);
    const legacy = {version: 1, scenarioId: 'record-relation', size: 3, blank: false, randomSeed: 5,
        components: [{id: 'extra', kind: 'field', enabled: true,
            region: {shape: 'point', x: 0, y: 0, z: 0, dx: 1, dy: 1, dz: 1, radius: 0},
            parameters: {channel: 0, occupied: true, density: 1}}]};
    const migrated = migrateRecipe(legacy, record, schema.recipe);
    assert.equal(migrated.version, RECIPE_VERSION);
    assert.equal(migrated.randomSeed, 5);
    assert.deepEqual(migrated.components.map(c => c.id), [...schema.recipe.components.map(c => c.id), 'extra']);
    assert.equal(validateRecipe(migrated, scenarios), record);

    const blankBase = findFiniteBaseSchema(finiteCatalog, 3);
    const legacyBlank = {...cloneRecipe(blankBase.recipe), version: 1};
    const migratedBlank = migrateRecipe(legacyBlank, record, schema.recipe);
    assert.deepEqual(migratedBlank.components, blankBase.recipe.components);

    const native = createRecipe(SCALE0_SCENARIOS[0], 33);
    const legacyNative = {version: 1, scenarioId: native.scenarioId, size: 33, blank: false, randomSeed: 0, components: []};
    const migratedNative = migrateRecipe(legacyNative, SCALE0_SCENARIOS[0], null);
    assert.deepEqual(migratedNative.overrides, {});
    assert.equal(validateRecipe(migratedNative, scenarios), SCALE0_SCENARIOS[0]);
});

test('a recipe is "edited" only when it differs from its registered v2 default, not merely because it has enabled components', async () => {
    resetFiniteCatalogForTests(finiteCatalog);
    const schema = findFiniteSchema(finiteCatalog, 'record-relation', 3);
    assert.equal(await isEditedRecordRecipe(schema.recipe), false);
    const edited = cloneRecipe(schema.recipe); edited.components[0].parameters.polarity = -1;
    assert.equal(await isEditedRecordRecipe(edited), true);
    resetFiniteCatalogForTests();
});

test('a native category base composes every constructor registered in the scenario\'s own category, all inactive', () => {
    const anchor = SCALE0_SCENARIOS[0];
    const base = createNativeBaseRecipe(anchor, SCALE0_SCENARIOS);
    const expected = SCALE0_SCENARIOS.filter(s => s.category === anchor.category);
    assert.equal(base.scenarioId, anchor.id); // parent id preserved — real registry lookup still works
    assert.equal(base.blank, true);
    assert.equal(base.components.length, expected.length);
    assert.ok(base.components.every(c => c.kind === 'native' && c.enabled === false && Object.keys(c.overrides).length === 0));
    assert.deepEqual(new Set(base.components.map(c => c.scenarioId)), new Set(expected.map(s => s.id)));
    assert.equal(new Set(base.components.map(c => c.id)).size, base.components.length); // unique ids
    assert.equal(validateRecipe(base, scenarios), anchor);
    assert.equal(isCustomNativeRecipe(base), true); // blank alone is already custom

    base.components[0].enabled = true;
    assert.equal(validateRecipe(base, scenarios), anchor);
    const preset = createRecipe(anchor, 33);
    assert.equal(isCustomNativeRecipe(preset), false);
    preset.overrides.foo = 1;
    assert.equal(isCustomNativeRecipe(preset), true);
});

test('native ingredient overrides accept dotted 160-character keys and full-uint32 seeds; malformed ingredients are rejected', () => {
    const anchor = SCALE0_SCENARIOS[0];
    const base = createNativeBaseRecipe(anchor, SCALE0_SCENARIOS);
    base.components[0].enabled = true;
    base.components[0].overrides['protocol.boundary'] = 2;
    base.components[0].overrides.seed = 4294967295;
    assert.equal(validateRecipe(base, scenarios), anchor);
    assert.throws(() => { const r = cloneRecipe(base); r.components[0].overrides.seed = 4294967296; validateRecipe(r, scenarios); }, /seed must be an integer/);
    assert.throws(() => { const r = cloneRecipe(base); r.components[0].overrides.seed = -1; validateRecipe(r, scenarios); }, /seed must be an integer/);
    assert.throws(() => { const r = cloneRecipe(base); r.components[0].kind = 'field'; validateRecipe(r, scenarios); }, /kind "native"/);
    assert.throws(() => { const r = cloneRecipe(base); delete r.components[0].scenarioId; r.components[0].region = {}; validateRecipe(r, scenarios); }, /unexpected or missing/);
    assert.throws(() => { const r = cloneRecipe(base); r.components = []; validateRecipe(r, scenarios); }, /at least one constructor/);
    const longKey = 'a'.repeat(161);
    assert.throws(() => { const r = cloneRecipe(base); r.components[0].overrides[longKey] = 1; validateRecipe(r, scenarios); }, /Invalid override name/);
    const okKey = 'a'.repeat(160);
    const r = cloneRecipe(base); r.components[0].overrides[okKey] = 1;
    assert.equal(validateRecipe(r, scenarios), anchor);
});

test('the export envelope round-trips through parseRecipeEnvelope and rejects a mismatched domain', () => {
    const schema = findFiniteSchema(finiteCatalog, 'record-relation', 3);
    const description = schema;
    const envelope = buildRecipeEnvelope(schema.recipe, record, description, {custom: false});
    assert.equal(envelope.format, ENVELOPE_FORMAT);
    assert.deepEqual(envelope.domain, {kind: 'finite-records', size: 3});
    assert.equal(envelope.provenance.parentPreset, 'record-relation');
    const text = JSON.stringify(envelope);
    const parsed = parseRecipeEnvelope(text, scenarios);
    assert.deepEqual(parsed, schema.recipe);
    // A bare (non-envelope) v2 recipe still parses via the legacy path.
    assert.deepEqual(parseRecipeEnvelope(JSON.stringify(schema.recipe), scenarios), schema.recipe);
    // Domain/recipe disagreement is rejected before validation runs.
    const tampered = JSON.parse(text); tampered.domain.size = 4;
    assert.throws(() => parseRecipeEnvelope(JSON.stringify(tampered), scenarios), /domain does not match/);
    assert.throws(() => parseRecipeEnvelope(' '.repeat(262145), scenarios), /256 KiB/);
});

test('every complete native category export round-trips with its resolved property and protocol metadata', () => {
    const catalog=JSON.parse(readFileSync(new URL('../js/seeding/generated/native-seeds.json',import.meta.url)));
    let largest=0;
    for (const group of Object.values(Object.groupBy(SCALE0_SCENARIOS,s=>s.category))) {
        const recipe=createNativeBaseRecipe(group[0],SCALE0_SCENARIOS);
        const properties=group.flatMap((s,i)=>catalog.presets[`${s.id}@33`].properties.map(p=>({
            ...p,key:`ingredient.${i}.${p.key}`,path:['components',i,'overrides',p.key],
        })));
        const envelope=buildRecipeEnvelope(recipe,group[0],{schemaIdentity:'native-seed-2',properties});
        const text=JSON.stringify(envelope,null,2);
        largest=Math.max(largest,Buffer.byteLength(text));
        assert.deepEqual(parseRecipeEnvelope(text,scenarios),recipe);
    }
    assert.ok(largest>262144,'The largest category exercises metadata larger than the compiled-recipe budget');
    assert.throws(()=>parseRecipeEnvelope(' '.repeat(2097153),scenarios),/2 MiB/);
});

test('legacy v1 migration renames a colliding default component instead of renaming the legacy overlay', () => {
    const schema = findFiniteSchema(finiteCatalog, 'record-relation', 3);
    const collidingId = schema.recipe.components[0].id;
    const legacy = {version: 1, scenarioId: 'record-relation', size: 3, blank: false, randomSeed: 7,
        components: [{id: collidingId, kind: 'field', enabled: true,
            region: {shape: 'point', x: 0, y: 0, z: 0, dx: 1, dy: 1, dz: 1, radius: 0},
            parameters: {channel: 0, occupied: true, density: 1}}]};
    const migrated = migrateRecipe(legacy, record, schema.recipe);
    assert.equal(validateRecipe(migrated, scenarios), record);
    // The overlay's own id is preserved exactly (its RNG stream depends on it)...
    const overlay = migrated.components.at(-1);
    assert.equal(overlay.id, collidingId); assert.equal(overlay.kind, 'field');
    // ...and the expanded default's same-id component was renamed instead, so ids stay unique.
    const ids = migrated.components.map(c => c.id);
    assert.equal(new Set(ids).size, ids.length);
    assert.equal(ids.filter(id => id === collidingId).length, 1);
});

test('unknown, incompatible, nonfinite and ambiguous recipe inputs are rejected', () => {
    const base = findFiniteBaseSchema(finiteCatalog, 9);
    const original = createBaseRecipe(record, base);
    const changes = [r => {r.version = 1;}, r => {r.extra = true;}, r => {r.scenarioId = 'missing';},
        r => {r.components[0].parameters.channel = Number.POSITIVE_INFINITY;}, r => {r.randomSeed = -1;},
        r => {r.components[0].parameters.density = NaN;}, r => {r.components[0].kind = 'not-a-kind';},
        r => {r.components[0].region.x = 9;},
        r => {r.components[0].region.shape = 'box'; r.components[0].region.x = 1;},
        r => {r.components.push(cloneRecipe(r.components[0]));}, r => {r.components = Array(65).fill(r.components[0]);},
        r => {r.components[0].parameters['bad key'] = 1;}];
    for (const change of changes) {
        const r = cloneRecipe(original); change(r);
        assert.throws(() => validateRecipe(r, scenarios));
    }
    const native = createRecipe(SCALE0_SCENARIOS[0], 33);
    native.components.push(createComponent(base.recipe.components[0], 'field', 33));
    assert.throws(() => validateRecipe(native, scenarios), /does not yet accept/);
    assert.throws(() => parseRecipe(' '.repeat(262145), scenarios), /256 KiB/);
    assert.throws(() => parseRecipe('{"version":999}', scenarios), /unexpected or missing/);
    assert.equal(validateRecipe(original, scenarios), record);
});
