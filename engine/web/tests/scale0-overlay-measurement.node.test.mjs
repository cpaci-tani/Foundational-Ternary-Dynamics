import test from 'node:test';
import assert from 'node:assert/strict';
import { readFileSync } from 'node:fs';
import vm from 'node:vm';

// Evaluate unchanged card method bodies with browser/import dependencies stubbed.
// These are numerical/availability tests, not browser lifecycle or FPS evidence.
function card(name, exported, extra = {}) {
    const source = readFileSync(new URL(`../js/scales/scale0/ui/overlays/p1-observables/${name}.js`, import.meta.url), 'utf8')
        .replace(/import[\s\S]*?from\s+['"][^'"]+['"];\s*/g, '')
        .replace(/export\s+/g, '');
    const context = {
        BaseComponent: class {}, cardStyle: () => '', titleStyle: () => '', heroStyle: () => '',
        tagBadge: () => '', formatExp: String, ALPHA: 1/137, SCHWINGER_C2: -.328,
        A_E_CODATA: .001, A_MU_CODATA: .001, getParticleCharge: p => p.state,
        getPhysicsHarness: () => ({ sampleEFieldAlongRay: () => null }),
        performance: { now: () => 0 }, ...extra,
    };
    return vm.runInNewContext(`${source}\n${exported}`, context);
}

test('latency proxy rejects corrupt records and leaves undefined zero/zero ratio unavailable', () => {
    const Gravity = card('gravity', 'GravityComponent');
    const instance = Object.create(Gravity.prototype);
    const bridge = sample => ({ latticeSize: 7, getLatencySampled: () => sample });
    for (const sample of [
        {count: 2, values: [0], positions: [0,0,0]},
        {count: 1, values: [NaN], positions: [0,0,0]},
        {count: 1, values: [0], positions: [Infinity,0,0]},
        {count: 1, values: [-.1], positions: [0,0,0]},
    ]) assert.equal(instance._probeTimeDilation(bridge(sample)), null);
    const sample = instance._probeTimeDilation(bridge({ count: 1, values: [1], positions: [3,3,3] }));
    assert.equal(sample.ratio, null);
    const target = {};
    instance._renderGravitySection(target, sample, 0);
    assert.match(target.innerHTML, /7³ reference-engine/);
    assert.doesNotMatch(target.innerHTML, /Infinity|NaN|0\.004%/);
});

test('zero anisotropy denominator is unavailable and a missing frame clears the cached profile', () => {
    const Aniso = card('anisotropy', 'AnisotropyComponent');
    const instance = Object.create(Aniso.prototype);
    const sampler = { data: new Float32Array(27), latticeSize: 3, axisCount: 3, stride: 1 };
    assert.equal(instance._computeDecayPoints(sampler, 1,1,1), null);
    sampler.data.fill(1);
    assert.ok(instance._computeDecayPoints(sampler,1,1,1).every(point => point.aniso === 0));
    Object.assign(instance, {_lastBridge: null, _lastSourceKey: '', _lastSampleAt: -Infinity,
        refs: {plot: {}, desc: {}}, _renderAnisotropyDecay: () => {}});
    let available = true;
    const bridge = {latticeSize: 3, isNativeGPU: true, getFluxVolume: () => available ? sampler.data : null};
    instance.update(bridge,0,[]);
    assert.ok(instance._decayPoints);
    available = false;
    instance.update(bridge,1000,[]);
    assert.equal(instance._decayPoints,null);
});

test('Coulomb rejects singleton/empty/nonfinite samples and reports an unsigned maximum', () => {
    const particles = [{id: 1, state: 1, x: 0,y: 0,z: 0}, {id: 2,state: -1,x: 10,y: 0,z: 0}];
    const Coulomb = card('coulomb','CoulombComponent', {
        findOppositeChargePairFromList: () => ({pPos: particles[0],pNeg: particles[1]}),
    });
    const instance = Object.create(Coulomb.prototype);
    for (const direct of [{count: 1,V:[1]}, {count: 0,V:[]}, {count: 2,V:[NaN,0]}]) {
        assert.equal(instance._probeCoulombEngineE({sampleVAtRay: () => direct},particles),null);
    }
    const sample = instance._probeCoulombEngineE({sampleVAtRay: () => ({count: 2,V:[0,0]})},particles);
    assert.ok(sample.meta.maxAbsResidual > 0);
    assert.ok(sample.samples.every(p => p.residual < 0));
});

test('G2 scalar-spin illustration creates neither fake measured rates nor growing histories', () => {
    const G2 = card('g2','G2Component');
    const instance = Object.create(G2.prototype);
    Object.assign(instance, {trackingState: null, _getSpinArrowManager: () => null,
        _renderG2PrecessionSubsection: () => {}, refs: {precession: {}}});
    const particle = {id: 1,state: 1,x: 1,y: 2,z: 3,spin: 1};
    const bridge = {getScale0ParticleList: () => [particle]};
    instance._trackParticle(bridge);
    for (let tick=0;tick<10000;tick++) instance.update(bridge,[particle],tick);
    assert.equal(instance.trackingState.omegaMeasured,undefined);
    assert.equal(instance.trackingState.omegaHistory,undefined);
    const target = {};
    G2.prototype._renderG2PrecessionSubsection.call(instance,target,instance.trackingState);
    assert.match(target.innerHTML,/unavailable/);
    assert.match(target.innerHTML,/imposed/);
});
