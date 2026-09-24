import test from 'node:test';
import assert from 'node:assert/strict';
import {parseExperimentObjective,experimentObjectiveProgress,validateExperimentObjectiveProposal} from '../js/assistant/experiment-objective.js';

const goal='Run an experiment to advance exactly 10 ticks in total from the baseline, then compare the before and after measurements. Keep playback paused.';
const objective=parseExperimentObjective(goal);
const sample=(tick='1',overrides={})=>({workspace:'lattice',ownerId:'one',preparationVersion:'1:0:1',tick,
    facts:{sampleTick:tick,running:false,available:true,stale:false},...overrides});
const step=count=>({kind:'actions',actions:[{type:'lattice.step',args:{count}}],experiment:{phase:'act'}});

test('complete recognised interval goals preserve the user quantity and requested comparison',()=>{
    assert.deepEqual(objective,{kind:'tick-interval',requestedTicks:'10',compare:true,keepPaused:true});
    assert.equal(parseExperimentObjective('Advance exactly ten ticks.').requestedTicks,'10');
    assert.equal(parseExperimentObjective('Step by 12 ticks total from baseline, then compare before and after observations.').requestedTicks,'12');
    assert.equal(parseExperimentObjective('Conduct a bounded experiment to advance 3 ticks.').requestedTicks,'3');
    assert.equal(parseExperimentObjective('Advance 9007199254740993 ticks.').requestedTicks,'9007199254740993');
});

test('polite prefixes preserve the exact objective and explicit workspace constraints',()=>{
    assert.deepEqual(parseExperimentObjective(`JEV, could you please ${goal}`),objective);
    for(const text of ['Advance observer 10 ticks.', 'Advance 10 observer ticks.', 'Advance the observer 10 observer ticks.']) {
        const named=parseExperimentObjective(text);
        assert.equal(named.workspace,'observer');assert.equal(named.requestedTicks,'10');
        assert.equal(experimentObjectiveProgress(named,sample(),sample('11')).valid,false);
        assert.equal(experimentObjectiveProgress(named,sample('1',{workspace:'observer'}),sample('11',{workspace:'observer'})).valid,true);
    }
    const lattice=parseExperimentObjective('Please advance the lattice by exactly ten lattice ticks.');
    assert.equal(lattice.workspace,'lattice');assert.equal(experimentObjectiveProgress(lattice,sample(),sample('11')).valid,true);
    assert.equal(parseExperimentObjective('Advance observer 10 lattice ticks.'),null);
    assert.equal(parseExperimentObjective('Advance lattice 10 observer ticks.'),null);
    assert.equal(parseExperimentObjective('Could you please not advance 10 ticks?'),null);
});

test('ambiguous, compound, physical-unit and scientific goals stay outside the exact objective parser',()=>{
    for(const text of ['Advance 4 ticks then advance 6 ticks.', 'Advance 10 or 20 ticks.', 'Advance -10 ticks.',
        'Advance 1.5 ticks.', 'Advance 0 ticks.', 'Advance ten seconds.', 'Advance 10 ticks in 2 seconds.',
        'Advance 10 ticks then set gravity to 2.', 'Advance 10 ticks and test the energy law.',
        'How do I advance 10 ticks?', 'Do not advance 10 ticks.', 'Investigate whether matter emerges.',
        'Run an experiment to advance 10 ticks and delete the selected object.', 'Advance approximately 10 ticks.']) {
        assert.equal(parseExperimentObjective(text),null,text);
    }
});

test('progress uses exact serializable arithmetic above the safe integer range',()=>{
    const baseline=sample('9007199254740993'),current=sample('9007199254740997');
    const progress=experimentObjectiveProgress(objective,baseline,current);
    assert.equal(progress.valid,true);assert.equal(progress.targetTick,'9007199254741003');
    assert.equal(progress.elapsedTicks,'4');assert.equal(progress.remainingTicks,'6');
    assert.equal(progress.reached,false);assert.equal(progress.overshot,false);
    assert.doesNotThrow(()=>JSON.stringify(progress));
    assert.equal(experimentObjectiveProgress(objective,sample(1),sample(11)).reached,true);
    const overshot=experimentObjectiveProgress(objective,sample(),sample('12'));
    assert.equal(overshot.overshot,true);assert.equal(overshot.reached,false);assert.equal(overshot.remainingTicks,'0');
});

test('owner, workspace, preparation, unsafe numbers and backwards counters invalidate progress',()=>{
    for(const patch of [{ownerId:'two'},{workspace:'observer'},{preparationVersion:'2:0:1'},{ownerId:null},
        {tick:Number.MAX_SAFE_INTEGER+1},{tick:Infinity},{tick:1.5},{tick:'1e2'},{tick:'-1'},{tick:'0'}]) {
        const progress=experimentObjectiveProgress(objective,sample(),sample('11',patch));
        assert.equal(progress.valid,false,JSON.stringify(patch));assert.equal(progress.remainingTicks,null);
    }
    assert.equal(experimentObjectiveProgress(objective,sample(Number.MAX_SAFE_INTEGER+1),sample('11')).baselineTick,null);
});

test('steps cannot overshoot or repeat a fulfilled exact interval',()=>{
    assert.equal(validateExperimentObjectiveProposal(objective,sample(),sample('5'),step(6)).valid,true);
    for(const [current,count] of [['5',7],['11',1],['12',1],['5',1.5],['5',Number.MAX_SAFE_INTEGER+1]]) {
        assert.equal(validateExperimentObjectiveProposal(objective,sample(),sample(current),step(count)).valid,false);
    }
    assert.equal(validateExperimentObjectiveProposal(objective,sample(),sample('5'),{...step(1),actions:[{type:'lattice.reset',args:{}}]}).valid,false);
    const running=sample('5',{facts:{sampleTick:'5',running:true}});
    assert.equal(validateExperimentObjectiveProposal(objective,sample(),running,step(6)).valid,false);
    assert.equal(validateExperimentObjectiveProposal(objective,sample(),running,{...step(1),actions:[{type:'lattice.pause',args:{}}]}).valid,true);
});

test('completion needs the exact interval, coherent samples and paused playback',()=>{
    const complete={phase:'complete'};
    assert.equal(validateExperimentObjectiveProposal(objective,sample(),sample('11'),complete).valid,true);
    for(const current of [sample('10'),sample('12'),sample('11',{facts:{sampleTick:'10',running:false}}),
        sample('11',{facts:{sampleTick:11,running:true}}),sample('11',{facts:{sampleTick:11,stale:true,running:false}}),
        sample('11',{facts:{sampleTick:Number.MAX_SAFE_INTEGER+1,running:false}})]) {
        assert.equal(validateExperimentObjectiveProposal(objective,sample(),current,complete).valid,false);
    }
    assert.equal(validateExperimentObjectiveProposal(objective,sample('1',{facts:{sampleTick:'0',running:false}}),sample('11'),complete).valid,false);
    assert.equal(validateExperimentObjectiveProposal(objective,sample(),sample('11'),{phase:'clarify'}).valid,true);
});
