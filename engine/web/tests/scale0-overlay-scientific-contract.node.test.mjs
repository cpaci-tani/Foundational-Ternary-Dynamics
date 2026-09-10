import test from 'node:test';
import assert from 'node:assert/strict';
import { readFileSync } from 'node:fs';
import vm from 'node:vm';
import { getScale0Scenario } from '../js/scales/scale0/scenario-registry.js';
import {
    SCALE0_MASS_GRAVITY_SCENARIOS, SCALE0_SCENARIO_OVERRIDES, SCALE0_TOGGLES,
} from '../js/config/toggles.js';
import {
    getScale0StandardModelContext, isScale0StandardModelScenario,
} from '../js/scales/scale0/ui/overlays/standard-model.js';

// Run the production classifier without mounting its browser-only panel shell.
const source = readFileSync(new URL('../js/scales/scale0/ui/overlays/applicability.js', import.meta.url), 'utf8')
    .replace(/^import[\s\S]*?from\s+['"][^'"]+['"];\s*/gm, '')
    .replace(/export\s+/g, '');
const classify = vm.runInNewContext(`${source}\ngetScale0OverlayApplicability`, {
    SCALE0_MASS_GRAVITY_SCENARIOS, SCALE0_SCENARIO_OVERRIDES, SCALE0_TOGGLES,
    getScale0Scenario, isScale0StandardModelScenario, FIELD_TOGGLE_BINDINGS: [],
});
const waveOnly = Object.fromEntries(SCALE0_TOGGLES.map(([key]) => [key, key === 'wave_propagation']));

test('inert native particle-template markers remain observable without forces or genesis', () => {
    const leptonsAndW = [
        'electron', 'positron', 'muon', 'antimuon', 'tau', 'antitau',
        'w-boson', 'w-minus-boson',
    ].map((suffix) => `s0-vacuum-${suffix}`);
    const quarks = ['up', 'down', 'strange', 'charm', 'bottom', 'top']
        .flatMap((flavor) => [`s0-seed-${flavor}-quark`, `s0-seed-anti-${flavor}-quark`]);
    for (const id of [...leptonsAndW, ...quarks]) {
        const profile = classify(id, waveOnly);
        assert.equal(profile.domains.state, true, id);
        assert.ok(profile.applicable.has('toggle-state-field'), id);
        assert.equal(profile.applicable.has('toggle-color-charge'), quarks.includes(id), id);
        assert.equal(profile.domains.emForce, false, id);
        assert.equal(profile.domains.strong, false, id);
    }
});

test('catalog particle identity alone does not create a native state or color observation', () => {
    for (const id of [
        's0-vacuum-photon', 's0-vacuum-z-boson', 's0-vacuum-higgs', 's0-seed-gluon',
        's0-vacuum-electron-neutrino', 's0-vacuum-electron-antineutrino',
    ]) {
        const profile = classify(id, waveOnly);
        assert.equal(profile.domains.standardModel, true, id);
        assert.equal(profile.domains.state, false, id);
        assert.equal(profile.applicable.has('toggle-color-charge'), false, id);
    }
});

test('SM badges retain identity boundaries and describe the conjugate weak field', () => {
    const context = (id) => getScale0StandardModelContext(id, getScale0Scenario(id));
    assert.equal(context('s0-vacuum-electron-neutrino').status, 'CONJECTURE');
    assert.equal(context('s0-vacuum-electron-antineutrino').status, 'CONJECTURE');
    assert.equal(context('s0-vacuum-electron').status, 'CLOSED NEGATIVE');
    assert.equal(context('s0-vacuum-photon').status, 'OPEN');
    assert.equal(context('s0-vacuum-electron-neutrino').chirality, 'L weak field');
    assert.equal(context('s0-vacuum-electron-antineutrino').chirality, 'Conjugate of L weak field');
    assert.equal(context('s0-vacuum-electron-antineutrino').charge, '0');
    assert.match(context('s0-vacuum-electron-neutrino').statusDetail, /identity is not claimed/);
});
