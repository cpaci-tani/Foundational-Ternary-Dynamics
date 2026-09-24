import test from 'node:test';
import assert from 'node:assert/strict';
import {commandClauses,proposalDescriptors} from '../js/assistant/proposal.js';
import {BrowserModel,relevantActions} from '../js/assistant/model.js';
import {OBSERVER_ACTION_DESCRIPTORS} from '../js/assistant/observer-control.js';

test('command clauses preserve vectors, quoted names, conditions and catalog limits',()=>{
    assert.deepEqual(commandClauses('Pause, advance 37 ticks, then resume.'),['Pause','advance 37 ticks','resume.']);
    assert.deepEqual(commandClauses('While Observer is paused, advance 12 ticks.'),['advance 12 ticks.']);
    assert.deepEqual(commandClauses('Search catalog for "wave" and return at most 5 matches.'),['Search catalog for "wave" and return at most 5 matches.']);
    assert.deepEqual(commandClauses('Create a box named "pause, set gravity" at [1, 2, 3], then resume.'),['Create a box named "pause, set gravity" at [1, 2, 3]','resume.']);
});

test('questions and unsupported commands cannot inherit a default pause capability',async()=>{
    const model=new BrowserModel({modelBase:''});model.generate=async()=>{throw new Error('Should not generate an action');};
    const o={workspace:'observer',ownerId:'test',preparationVersion:'1',capabilities:OBSERVER_ACTION_DESCRIPTORS.map(x=>x.type),actions:OBSERVER_ACTION_DESCRIPTORS,facts:{}};
    assert.equal((await model.plan('Why is the force tether overloaded?',o,[],new AbortController().signal)).kind,'answer');
    assert.deepEqual(relevantActions('Upload this world to a remote server.',o.actions),[]);
    assert.equal((await model.plan('Upload this world to a remote server.',o,[],new AbortController().signal)).kind,'clarify');
    assert.equal((await model.plan('Do not pause.',o,[],new AbortController().signal)).kind,'clarify');
});

test('polite questions and help never reach action generation or split off a command',async()=>{
    const model=new BrowserModel({modelBase:''});model.generate=async()=>{throw new Error('Question reached action generation');};
    const o={workspace:'observer',ownerId:'test',preparationVersion:'1',capabilities:OBSERVER_ACTION_DESCRIPTORS.map(x=>x.type),actions:OBSERVER_ACTION_DESCRIPTORS,facts:{}};
    for(const text of ['Hello','Help me pause.','Please explain the current observation.','Could you explain pause?',
        'Is the simulation paused?','Is it safe to resume?','How do I pause and resume?','Can you tell me how to change gravity?',
        'Show me how to pause.','Can you show me how to pause and resume?','Give me an explanation of pause.',
        'Walk me through running an experiment.']){
        const plan=await model.plan(text,o,[],new AbortController().signal);
        assert.equal(plan.kind,'answer',text);assert.deepEqual(plan.actions,[],text);
    }
    assert.equal((await model.plan('Could you not pause?',o,[],new AbortController().signal)).kind,'clarify');
});

test('look-at and new-shape shorthand reach their supported action schemas',async()=>{
    const model=new BrowserModel({modelBase:''});
    const o={workspace:'observer',ownerId:'test',preparationVersion:'1',capabilities:OBSERVER_ACTION_DESCRIPTORS.map(x=>x.type),actions:OBSERVER_ACTION_DESCRIPTORS,facts:{},
        selected:{id:'selected-cube',current:{alive:true,shape:'box',mass:2}}};
    for(const [text,action] of [
        ['Look at the selected cube.',{type:'observer.camera',args:{mode:'lookAt',id:'selected-cube'}}],
        ['New sphere.',{type:'observer.create',args:{shape:'sphere'}}],
        ['New cube.',{type:'observer.create',args:{shape:'box'}}],
    ]){
        let generated=false;
        model.generate=async(_messages,_signal,format)=>{
            generated=true;
            const schema=JSON.parse(format.schema);
            assert.ok(schema.properties.actions.items.anyOf.some(row=>row.properties.type.enum.includes(action.type)),text);
            return JSON.stringify({kind:'actions',message:'',actions:[action]});
        };
        const plan=await model.plan(text,o,[],new AbortController().signal);
        assert.equal(generated,true,text);assert.equal(plan.kind,'actions',text);assert.deepEqual(plan.actions,[action],text);
    }
});

test('ordinary research goals cannot inherit mutations from incidental keywords',async()=>{
    const model=new BrowserModel({modelBase:''});model.generate=async()=>{throw new Error('Goal reached command generation');};
    const o={workspace:'observer',ownerId:'test',preparationVersion:'1',capabilities:OBSERVER_ACTION_DESCRIPTORS.map(x=>x.type),actions:OBSERVER_ACTION_DESCRIPTORS,facts:{}};
    for(const text of ['Monitor the pause control.','Watch how the cube rotates.','Measure the gravity.',
        'Keep advancing until something changes.','Pause then monitor the reset control.']){
        const plan=await model.plan(text,o,[],new AbortController().signal);
        assert.equal(plan.kind,'clarify',text);assert.deepEqual(plan.actions,[],text);assert.match(plan.message,/Live experiment/);
    }
});

test('an explicit command for a different workspace cannot mutate the current workspace',async()=>{
    const model=new BrowserModel({modelBase:''});model.generate=async()=>{throw new Error('Wrong workspace reached generation');};
    const o={workspace:'observer',ownerId:'test',preparationVersion:'1',capabilities:OBSERVER_ACTION_DESCRIPTORS.map(x=>x.type),actions:OBSERVER_ACTION_DESCRIPTORS,facts:{}};
    for(const text of ['Pause the lattice simulation.','lattice.pause']){
        const plan=await model.plan(text,o,[],new AbortController().signal);assert.equal(plan.kind,'clarify');assert.match(plan.message,/Switch to Lattice Sim/);
    }
});

test('a property edit cannot advertise an unrelated profile or world change',()=>{
    const actions=proposalDescriptors('Set world gravity strength to 4.',{},relevantActions('Set world gravity strength to 4.',OBSERVER_ACTION_DESCRIPTORS));
    assert.deepEqual(Object.keys(actions[0].args.properties),['gravityStrength']);
    const resume=relevantActions('Resume the Playground.',OBSERVER_ACTION_DESCRIPTORS);
    assert.deepEqual(resume.map(x=>x.type),['observer.resume']);
});

test('generation retains its single-operation lease until interrupted work acknowledges completion',async()=>{
    const model=new BrowserModel({modelBase:''});let finish,interrupts=0;
    model.ready=true;model.engine={interruptGenerate:()=>interrupts++,chat:{completions:{create:()=>new Promise(resolve=>{finish=resolve;})}}};
    const controller=new AbortController();const first=model.generate([],controller.signal);controller.abort(new Error('Stop'));
    await assert.rejects(model.generate([],new AbortController().signal),/still stopping/);
    assert.equal(model.busy,true);assert.equal(interrupts,1);
    finish((async function*(){yield {choices:[{delta:{content:'late'}}]};})());await assert.rejects(first,/Stop/);assert.equal(model.busy,false);
    model.engine.chat.completions.create=async()=>((async function*(){yield {choices:[{delta:{content:'recovered'}}]};})());
    assert.equal(await model.generate([],new AbortController().signal),'recovered');
});
