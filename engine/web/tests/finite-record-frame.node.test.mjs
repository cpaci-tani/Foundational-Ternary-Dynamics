import {test} from 'node:test';
import assert from 'node:assert/strict';
import {recordAt, recordFrame} from '../js/bridge/finite-record-frame.js';
import {registerScale0RecordScenarios, getScale0Scenario} from '../js/scales/scale0/scenario-registry.js';
const view = () => ({lattice_size: '3', width: '1', microtick: '7',
    field_tokens: Array.from({length: 27}, (_, i) => String(i)), relation_tokens: Array(27).fill('2'),
    incidence: Array.from({length: 27}, (_, i) => String(i - 13)),
    manifestation_counts: Array.from({length: 27}, (_, i) => i === 5 ? ['1','0','0'] : i === 21 ? ['0','0','1'] : ['0','1','0'])});

test('every x-major site is transposed to the normal z-major lattice volume without changing counts', () => {
    const input = view(), original = JSON.stringify(input), frame = recordFrame(input);
    for (let x = 0; x < 3; x++) for (let y = 0; y < 3; y++) for (let z = 0; z < 3; z++)
        assert.equal(frame.volume[(z*3+y)*3+x], (x*3+y)*3+z+2);
    assert.equal(frame.volume.reduce((a,b) => a+b), 405);
    assert.equal(JSON.stringify(input), original);
});
test('normal particle markers use lagged manifestation, independent of incidence', () => {
    const input = view(), {particles} = recordFrame(input);
    assert.equal(particles.count, 2);
    assert.deepEqual([...particles.states], [-1, 1]);
    assert.deepEqual([...particles.positions], [.5,1.5,2.5, 2.5,1.5,.5]);
    assert.equal(recordAt(input, 0, 1, 2).state, -1);
    assert.equal(recordAt(input, 0, 1, 2).incidence, -8);
    assert.equal(recordAt(input, 0, 1, 2).tick, '7');
});
test('quantity selection changes only passive frame values and retains signed inspector Q', () => {
    const input = view();
    assert.equal(recordFrame(input, 'field_tokens').volume[18], 2);
    assert.equal(recordFrame(input, 'incidence').volume[18], 11);
    assert.equal(recordFrame(input, 'relation_tokens').volume[18], 2);
    assert.equal(recordAt(input, 0, 0, 2).incidence, -11);
    assert.equal(recordAt(input, -1, 0, 0), null);
    assert.equal(recordAt(input, 3, 0, 0), null);
});
test('coarse and incomplete observations cannot impersonate complete lattice arrays', () => {
    assert.throws(() => recordFrame({...view(), width: '3'}));
    assert.throws(() => recordFrame({...view(), incidence: []}));
    assert.throws(() => recordFrame(view(), 'energy'));
});
test('record scenarios use the existing registry without replacing an effective scenario or claiming behavioral qualification', () => {
    const law = 'phi-v2-staged-candidate-1';
    registerScale0RecordScenarios({law_id: law, canonical_adoption: false,
        scenarios: [{id:'empty',title:'Empty record',sizes:[3],law_id:law,family:'Controls',observation:'field_tokens',tests:[],source:'state.py'}]});
    assert.equal(getScale0Scenario('record-empty').backend, 'finite-records');
    assert.equal(getScale0Scenario('record-empty').observation, 'field_tokens');
    assert.equal(getScale0Scenario('record-empty').validation, undefined);
    assert.equal(getScale0Scenario('empty').backend, undefined);
    assert.throws(() => registerScale0RecordScenarios({law_id:'another-law',canonical_adoption:false,scenarios:[]}));
});
import {copyScalarActivation} from '../js/viewport/scalar-activation.js';
test('count activation preserves exact support and never adds neighbour or manifestation energy',()=>{
    const density=new Float32Array(27);density[13]=7;const activation=new Float32Array(27).fill(99);
    assert.equal(copyScalarActivation(density,activation).instantMax,7);assert.deepEqual(activation,density);
    density.fill(0);copyScalarActivation(density,activation);assert.equal(activation.every(x=>x===0),true);
});
import {DEFAULT_FLUX_THRESHOLD,FLUX_THRESHOLD_SLIDER_STEP,sliderPositionToFluxThreshold,fluxThresholdToSliderPosition,formatFluxThreshold} from '../js/viewport/flux-threshold.js';
test('default is the first positive UI step, displayed distinctly from all-voxel zero',()=>{
    assert.equal(DEFAULT_FLUX_THRESHOLD,sliderPositionToFluxThreshold(FLUX_THRESHOLD_SLIDER_STEP));
    assert.equal(fluxThresholdToSliderPosition(DEFAULT_FLUX_THRESHOLD),.0001);
    assert.ok(DEFAULT_FLUX_THRESHOLD>0);assert.equal(formatFluxThreshold(DEFAULT_FLUX_THRESHOLD),'2.00e-8');assert.notEqual(formatFluxThreshold(0),formatFluxThreshold(DEFAULT_FLUX_THRESHOLD));
});
