import test from 'node:test';
import assert from 'node:assert/strict';
import {readFileSync} from 'node:fs';
import {assertPropertyDescriptor} from '../js/seeding/property-schema.js';
import {SCALE0_SCENARIOS} from '../js/scales/scale0/scenario-registry.js';
import {NATIVE_SEED_IDS} from '../js/seeding/generated/native-seed-ids.js';

const json = path=>JSON.parse(readFileSync(new URL(path,import.meta.url),'utf8'));
const native=json('../js/seeding/generated/native-seeds.json');
const finite=json('../js/seeding/generated/finite-seeds.json');
const coverage=json('../../config/web_seeding_coverage.json');

test('every emitted property has a supported numeric editor, usable recommendation, concrete binding and effect test',()=>{
    const bindings=new Set(coverage.propertyBindings.map(p=>`${p.schema}:${p.key}`));
    let checked=0;
    for(const [key,schema] of [...Object.entries(native.presets),...Object.entries(finite.schemas)]) {
        const unique=new Set();
        for(const p of schema.properties) {
            assertPropertyDescriptor(p);
            assert.ok(!unique.has(p.key),`Duplicate descriptor ${key}:${p.key}`); unique.add(p.key);
            if(!key.startsWith('record-base@')) assert.ok(bindings.has(`${key}:${p.key}`),`Missing effect binding ${key}:${p.key}`);
            checked++;
        }
    }
    assert.ok(checked>40000);
});

test('registry additions cannot bypass native schema/effect or category-base coverage',()=>{
    assert.deepEqual([...NATIVE_SEED_IDS].sort(),SCALE0_SCENARIOS.map(s=>s.id).sort());
    const registered=new Set(coverage.scenarios.map(s=>s.scenarioId));
    for(const s of SCALE0_SCENARIOS) {
        assert.ok(registered.has(s.id));
        for(const n of [17,32,33]) assert.ok(native.presets[`${s.id}@${n}`]);
        assert.ok(coverage.categories.find(c=>c.name===s.category)?.scenarios.includes(s.id));
    }
    const effects=json('../../config/native_seed_effects.json');
    assert.ok(effects.effects.every(e=>e.effectVerified));
    assert.equal(effects.effects.length,coverage.counts.nativeEffectBindings);
    assert.equal(coverage.counts.scenarios,coverage.scenarios.length);
});
