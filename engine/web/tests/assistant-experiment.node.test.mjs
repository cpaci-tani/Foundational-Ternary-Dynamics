import {test} from 'node:test';
import assert from 'node:assert/strict';
import {AssistantService} from '../js/assistant/service.js';
import {assertObservation,validatePlan} from '../js/assistant/contracts.js';
import {ExperimentJournal,hasExperimentEvidence} from '../js/assistant/experiment-state.js';

const phase=(phase,actions=[])=>({kind:phase==='act'?'actions':'answer',message:'',actions,experiment:{phase,waitMs:phase==='observe'?250:0,reason:'Compare recorded observations.'}});
function fixture(){
    const state={tick:1,sampleTick:0,version:0,running:true},events=[],writes=[],decisions=[];
    const observe=()=>({workspace:'lattice',ownerId:'owner',preparationVersion:String(state.version),tick:state.tick,capabilities:['lattice.step'],
        actions:[{type:'lattice.step',args:{type:'object',properties:{count:{type:'integer',minimum:1,maximum:4096}},required:['count']}}],
        facts:{running:state.running,sampleTick:state.sampleTick,manifested:String(state.sampleTick+2),available:true}});
    const control={observe,async execute(action,options){options.assertActive();assertObservation(observe(),options.expected);writes.push(action);state.tick+=action.args.count;state.sampleTick=state.tick;return{status:'applied',action,after:observe()};}};
    const deps={getControl:()=>control,model:{cancel(){},plan(){throw new Error('Command translation cannot plan a goal');},experimentStep:async()=>phase('complete'),explain:async()=> 'Interpretation of measured observations'},
        jev:{connected:true,evaluate:async request=>{decisions.push(request);return{decision:'execute',confidence:.9};}},knowledge:{search:async()=>[]},onEvent:event=>events.push(event)};
    return{state,events,writes,decisions,observe,deps,service:new AssistantService(deps)};
}
test('live experiment observes, evaluates, executes and replans from fresh completed records',async t=>{
    const f=fixture(),timers=setTimeout,seen=[];let round=0;
    t.mock.method(globalThis,'setTimeout',(callback,ms,...args)=>timers(()=>{if(ms===250){f.state.tick=5;f.state.sampleTick=5;}callback(...args);},ms===250||ms===1000?0:ms));
    f.deps.model.experimentStep=async(goal,observation,context)=>{
        seen.push({goal,tick:observation.tick,sampleTick:observation.facts.sampleTick,context});
        if(++round===1){f.state.tick=2;return phase('observe');}
        if(round===2)return phase('act',[{type:'lattice.step',args:{count:3}}]);
        return phase('complete');
    };
    f.deps.model.explain=async()=>{throw new Error('Measured results must not come from generated prose');};
    await f.service.submit('Measure, advance three ticks, compare the result',{autonomous:true});
    assert.deepEqual(seen.map(row=>[row.tick,row.sampleTick]),[[1,0],[5,5],[8,8]]);
    assert.ok(seen.every(row=>row.goal==='Measure, advance three ticks, compare the result'));
    assert.equal(f.decisions[0].observation.tick,2);
    assert.equal(f.decisions[0].observation.facts.sampleTick,0,'completion cannot relabel an older sample');
    assert.deepEqual(f.decisions.map(row=>row.plan.experiment.phase),['observe','act','complete']);
    assert.equal(f.decisions[2].observation.facts.experimentRun.actionsCompleted,2);
    assert.equal(f.writes.length,1);assert.equal(f.state.running,true);
    assert.equal(f.decisions.at(-1).observation.facts.experimentRun.baseline.facts.sampleTick,0);
    assert.ok(f.decisions.at(-1).observation.facts.experimentRun.completedActions.some(row=>row.action?.args.count===3));
    assert.match(f.events.find(e=>e.type==='answer').text,/Published diagnostic sample tick: 0 → 8/);
    assert.equal(f.events.filter(e=>e.type==='experiment').at(-1).status,'complete');
    f.service.dispose();
});
test('JEV rejection blocks proposed observations as well as writes and completion',async()=>{
    const f=fixture();f.deps.model.experimentStep=async()=>phase('act',[{type:'lattice.step',args:{count:1}}]);
    f.deps.jev.evaluate=async()=>({decision:'reject',confidence:1});
    await f.service.submit('Conduct a live comparison',{autonomous:true});
    assert.equal(f.writes.length,0);assert.equal(f.events.filter(e=>e.type==='experiment').at(-1).status,'reject');f.service.dispose();
});
test('Stop interrupts a passive observation interval and preserves playback',async()=>{
    const f=fixture();let observing;
    const ready=new Promise(resolve=>{observing=resolve;});
    f.deps.onEvent=event=>{f.events.push(event);if(event.type==='experiment'&&event.phase==='observing')observing();};
    f.deps.model.experimentStep=async()=>({...phase('observe'),experiment:{phase:'observe',waitMs:5000,reason:'Wait for a sample.'}});
    const pending=f.service.submit('Watch the live lattice',{autonomous:true});await ready;f.service.stop();await pending;
    assert.equal(f.writes.length,0);assert.equal(f.state.running,true);assert.equal(f.events.filter(e=>e.type==='experiment').at(-1).status,'stopped');f.service.dispose();
});
test('author edits during experiment inference reject before JEV or writes',async()=>{
    const f=fixture();f.deps.model.experimentStep=async()=>{f.state.version++;return phase('act',[{type:'lattice.step',args:{count:1}}]);};
    await f.service.submit('Conduct an experiment',{autonomous:true});
    assert.equal(f.decisions.length,0);assert.equal(f.writes.length,0);assert.match(f.events.find(e=>e.type==='error').text,/world changed/);f.service.dispose();
});

test('an exact experiment interval rejects an excessive model step before JEV',async()=>{
    const f=fixture();f.state.running=false;f.state.sampleTick=f.state.tick;
    f.deps.model.experimentStep=async()=>phase('act',[{type:'lattice.step',args:{count:11}}]);
    await f.service.submit('Run an experiment to advance exactly 10 ticks in total from the baseline, then compare the before and after measurements. Keep playback paused.');
    assert.equal(f.writes.length,0);assert.equal(f.decisions.length,0);
    assert.ok(f.events.some(event=>event.type==='error'));f.service.dispose();
});

test('exact interval authority is rechecked after asynchronous JEV evaluation',async()=>{
    const f=fixture();f.state.running=false;f.state.sampleTick=f.state.tick;
    f.deps.model.experimentStep=async()=>phase('act',[{type:'lattice.step',args:{count:10}}]);
    f.deps.jev.evaluate=async()=>{f.state.tick+=6;f.state.sampleTick=f.state.tick;return{decision:'execute',confidence:1};};
    await f.service.submit('Run an experiment to advance exactly 10 ticks in total from the baseline, then compare the before and after measurements. Keep playback paused.');
    assert.equal(f.writes.length,0);assert.ok(f.events.some(event=>event.type==='error'));f.service.dispose();
});
test('disconnected experiments cannot pause or mutate the current owner',async()=>{
    const f=fixture();f.deps.jev.connected=false;
    await f.service.submit('Run an experiment',{autonomous:true,pauseWhileThinking:true});
    assert.equal(f.writes.length,0);assert.match(f.events.find(e=>e.type==='error').text,/Connect a JEV key/);f.service.dispose();
});
test('experiment journal retains a fixed baseline and bounded recent evidence',()=>{
    const f=fixture(),journal=new ExperimentJournal(f.observe(),0);
    for(let i=0;i<20;i++){f.state.tick++;journal.record(f.observe(),i+1);}
    assert.equal(journal.baseline.tick,1);assert.equal(journal.recent.length,4);assert.equal(journal.recent.at(-1).sample.tick,21);
    const observation=f.observe();assert.throws(()=>validatePlan({...phase('observe'),actions:[{type:'lattice.step',args:{count:1}}]},observation));
    assert.throws(()=>validatePlan({...phase('act',[{type:'lattice.step',args:{count:1}},{type:'lattice.step',args:{count:1}}])},observation));
    f.service.dispose();
});
test('a model cannot conclude from an initial snapshot or relabel a pending sample as fresh evidence',async()=>{
    const f=fixture(),context={baseline:f.observe(),completedActions:[]};
    assert.equal(hasExperimentEvidence(f.observe(),context),false);
    f.state.tick=100;
    assert.equal(hasExperimentEvidence(f.observe(),context),false);
    f.state.sampleTick=100;
    assert.equal(hasExperimentEvidence(f.observe(),context),true);
    await f.service.submit('Run an experiment to compare two measurements',{autonomous:true});
    assert.equal(f.decisions.length,0);assert.match(f.events.find(event=>event.type==='error').text,/no new completed measurement/);f.service.dispose();
});
