import assert from 'node:assert/strict';
import { readFile } from 'node:fs/promises';
import test from 'node:test';
import { createNativeParticleEngine } from '../js/bridge/native-particle-engine.js';
import { installScale1ScenarioManifest, getScale1Scenario } from '../js/scales/scale1/scenario-registry.js';
import { M_U_PHYS, M_D_PHYS, M_S_PHYS, M_E_PHYS } from '../js/constants.js';

function nativeStub() {
    const calls = [];
    let constructions = 0;
    const adapter = createNativeParticleEngine({ _module: {
        ParticleEngine: class { constructor() { constructions++; } },
        peAddParticleEx(...args) { calls.push(args); return calls.length - 1; },
    } });
    adapter.peSetMode('effective_lab');
    return { adapter, calls, get constructions() { return constructions; } };
}

const fields = () => ({ spin: 1, colorId: 2, spinAxis: [0, 0, 1] });
const inject = (adapter, record = fields(), catalogId = null, charge = 0, mass = 2) =>
    adapter.peAddParticle(catalogId, charge, 1, 2, 3, 0, 0, 0, mass, 0.3, record);

test('strong-term scenario creates three distinct, unnamed neutral native color records', () => {
    const stub = nativeStub();
    installScale1ScenarioManifest({ scenarios: [{
        id: 's1-quantum-color-triplet', setupId: 'quantum_color_triplet',
    }] });
    const scenario = getScale1Scenario('s1-quantum-color-triplet');
    assert.deepEqual(scenario.physics, { coulomb: false, strong: true, dt: 0.05, softening: 0.05 });
    scenario.setup({ bridge: stub.adapter });
    assert.equal(stub.constructions, 1);
    assert.deepEqual(stub.calls.map(args => args.slice(1)), [
        [0, -4, -2, 0, 0, 0, 0, M_U_PHYS, 0.3, 1, 1, 0, 0, 1],
        [0, 4, -2, 0, 0, 0, 0, M_D_PHYS, 0.3, 1, 2, 0, 0, 1],
        [0, 0, 4, 0, 0, 0, 0, M_S_PHYS, 0.3, 1, 3, 0, 0, 1],
    ]);
    assert.deepEqual([...stub.adapter.peGetParticleTypes()], [[0, null], [1, null], [2, null]]);
});

test('invalid anonymous overrides are rejected before constructing or mutating native state', () => {
    const invalid = [
        null, true, [], {}, { ...fields(), extra: 1 },
        { colorId: 1, spinAxis: [0, 0, 1] },
        ...[-2, 2, 0.5, NaN, Infinity, '1'].map(spin => ({ ...fields(), spin })),
        ...[-1, 4, 1.5, NaN, Infinity, '2'].map(colorId => ({ ...fields(), colorId })),
        ...[null, [0, 1], [0, 0, 1, 0], [0, 0, NaN], [0, Infinity, 1],
            [0, 0, '1'], new Array(3), { 0: 0, 1: 0, 2: 1, length: 3 }]
            .map(spinAxis => ({ ...fields(), spinAxis })),
    ];
    const stub = nativeStub();
    for (const record of invalid) assert.equal(inject(stub.adapter, record), -1);
    for (const catalogId of ['electron', 'up', '', undefined, false]) {
        const charge = catalogId === 'electron' ? -1 : 0;
        assert.equal(stub.adapter.peAddParticle(catalogId, charge, 0, 0, 0, 0, 0, 0,
            M_E_PHYS, 0.3, fields()), -1);
    }
    assert.equal(stub.constructions, 0);
    assert.equal(stub.calls.length, 0);
    assert.equal(stub.adapter.peGetParticleTypes().size, 0);
});

test('anonymous overrides preserve catalog, mass, int8-charge and owner admission guards', () => {
    const stub = nativeStub();
    for (const charge of [-129, 128, 2 / 3, NaN]) {
        assert.equal(inject(stub.adapter, fields(), null, charge), -1);
    }
    for (const mass of [0, -1, null, NaN, Infinity]) {
        assert.equal(inject(stub.adapter, fields(), null, 0, mass), -1);
    }
    for (const [id, charge, mass] of [['up', 0, M_U_PHYS], ['up', 2 / 3, M_U_PHYS],
        ['electron', 1, M_E_PHYS], ['nu_e', 0, 1], ['unknown', 0, 1]]) {
        assert.equal(stub.adapter.peAddParticle(id, charge, 0, 0, 0, 0, 0, 0, mass, 0.3), -1);
    }
    stub.adapter.peSetMode('native_matter');
    assert.equal(inject(stub.adapter), -1);
    assert.equal(stub.constructions, 0);
    stub.adapter.peSetMode('effective_lab');
    assert.equal(stub.adapter.peAddParticle('electron', -1, 0, 0, 0, 0, 0, 0, M_E_PHYS, 0.3), 0);
    assert.equal(stub.adapter.peGetParticleTypes().get(0), 'electron');
    assert.equal(stub.calls[0][10], 1);
    assert.equal(stub.calls[0][11], 0);
    assert.equal(inject(stub.adapter, { spin: -1, colorId: 0, spinAxis: [0, 0, -2] }), 1);
    assert.deepEqual(stub.calls[1].slice(10), [-1, 0, 0, 0, -2]);
    const count = stub.calls.length;
    assert.equal(inject(stub.adapter, { ...fields(), colorId: 4 }), -1);
    assert.equal(stub.calls.length, count);
    assert.equal(stub.adapter.peGetParticleTypes().size, count);
});

test('WASM, WebSocket fallback and harness methods forward anonymous fields unchanged', async () => {
    // Evaluate only the actual forwarding method, avoiding browser-only module
    // startup. The native adapter itself is imported and exercised above.
    const cases = [
        ['bridge/wasm-bridge.js', adapter => ({ _peEngine: adapter })],
        ['bridge/ws-scale-fallback-facade.js', adapter => ({ _ensureFallback: () => adapter })],
        ['physics/physics-harness.js', adapter => ({ bridge: adapter })],
    ];
    for (const [path, receiver] of cases) {
        const source = await readFile(new URL(`../js/${path}`, import.meta.url), 'utf8');
        const method = source.match(/\bpeAddParticle\([^)]*\)\s*\{[^}]*\}/)?.[0];
        assert.ok(method, path);
        const forward = Function(`return ({ ${method} });`)().peAddParticle;
        const stub = nativeStub();
        const record = fields();
        assert.equal(forward.call(receiver(stub.adapter), null, 0, 1, 2, 3, 0, 0, 0, 2, 0.3, record), 0);
        assert.deepEqual(stub.calls[0].slice(10), [1, 2, 0, 0, 1], path);
        assert.deepEqual(record, fields(), path);
    }
});
