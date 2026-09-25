import test from 'node:test';
import assert from 'node:assert/strict';
import {AssistantService} from '../js/assistant/service.js';
import {validatePlan,validateValue,assertObservation} from '../js/assistant/contracts.js';
import {JevClient} from '../js/assistant/jev-client.js';

test('JEV transport retains the global receiver for default and injected fetch',async t=>{
    let calls=0;
    async function browserFetch(){
        if(this!==globalThis)throw new TypeError('Illegal invocation');
        calls++;
        return new globalThis.Response(JSON.stringify({decision:'execute',confidence:1,model:'transport-fixture'}));
    }
    t.mock.method(globalThis,'fetch',browserFetch);
    for(const options of [{localConfigured:true},{localConfigured:true,fetchImpl:browserFetch}]){
        const client=new JevClient(options);
        const decision=await client.evaluate({intent:'Pause'},new AbortController().signal);
        assert.equal(decision.decision,'execute');client.dispose();
    }
    assert.equal(calls,2);
});

function fixture(overrides={}) {
    let version=0,tick=0;const events=[],writes=[];
    const observe=()=>({workspace:'lattice',ownerId:'one',preparationVersion:String(version),tick,capabilities:['lattice.step'],facts:{running:false},actions:[{type:'lattice.step',args:{type:'object',properties:{count:{type:'integer',minimum:1,maximum:4096}},required:['count'],additionalProperties:false}}]});
    const plan={kind:'actions',message:'Step',actions:[{type:'lattice.step',args:{count:100}}]};
    const control={observe,async execute(action,{expected,assertActive}){assertActive();assertObservation(observe(),expected);const before=observe();writes.push(action);tick+=action.args.count;return{status:'applied',action,before,after:observe()};}};
    const deps={getControl:()=>control,model:{plan:async()=>plan,explain:async()=> 'Measured facts only',cancel(){}},jev:{evaluate:async()=>({decision:'execute',confidence:.99})},knowledge:{search:async()=>[]},onEvent:e=>events.push(e),...overrides};
    deps.model.experimentStep=async(...args)=>({...await deps.model.plan(...args),experiment:{phase:'act',waitMs:0,reason:'Continue the requested bounded experiment.'}});
    return{service:new AssistantService(deps),events,writes,control,observe,plan,deps,edit:()=>++version,advance:()=>++tick};
}

test('an explicit workspace continuation acquires new capabilities before interpreting the next command',async()=>{
    let workspace='lattice',count=0;const plans=[],writes=[];
    const observe=()=>({workspace,ownerId:workspace,preparationVersion:'1',facts:{running:false},capabilities:workspace==='lattice'?['workspace.switch']:['observer.step'],
        actions:workspace==='lattice'?[{type:'workspace.switch',args:{type:'object',properties:{workspace:{type:'string',enum:['observer']}},required:['workspace']}}]:[{type:'observer.step',args:{type:'object',properties:{count:{type:'integer',minimum:1,maximum:120}},required:['count']}}]});
    const control={observe,async execute(action,options){options.assertActive();assertObservation(observe(),options.expected);writes.push(action.type);if(action.type==='workspace.switch')workspace='observer';else count+=action.args.count;return{status:'applied',action,after:observe()};}};
    const service=new AssistantService({getControl:()=>control,model:{cancel(){},plan:async(text,observation)=>{plans.push([text,observation.workspace]);return observation.workspace==='lattice'
        ?{kind:'actions',message:'',actions:[{type:'workspace.switch',args:{workspace:'observer'}}],continuation:'advance 12 ticks'}
        :{kind:'actions',message:'',actions:[{type:'observer.step',args:{count:12}}]};}},jev:{evaluate:async()=>({decision:'execute',confidence:1})},knowledge:{search:async()=>[]}});
    await service.submit('Switch to Mind’s Eye then advance 12 ticks');
    assert.deepEqual(plans,[['Switch to Mind’s Eye then advance 12 ticks','lattice'],['advance 12 ticks','observer']]);
    assert.deepEqual(writes,['workspace.switch','observer.step']);assert.equal(count,12);service.dispose();
});

test('closed schemas reject unsupported fields and nonfinite numbers',()=>{
    const f=fixture();assert.throws(()=>validatePlan({...f.plan,actions:[{type:'eval',args:{}}]},f.observe()),/Unavailable/);
    for(const count of [NaN,Infinity,0,1.5,5000])assert.throws(()=>validatePlan({...f.plan,actions:[{type:'lattice.step',args:{count}}]},f.observe()));
    assert.throws(()=>validateValue({a:1,b:2},{type:'object',properties:{a:{type:'number'}},additionalProperties:false}),/Unsupported/);
});
test('ordinary advancing ticks do not stale a proposal, receipts describe completed work',async()=>{
    const f=fixture();f.deps.model.plan=async()=>{f.advance();return f.plan;};await f.service.submit('advance 100 ticks');
    assert.equal(f.writes.length,1);assert.equal(f.observe().tick,101);assert.equal(f.events.find(e=>e.type==='receipt').receipt.status,'applied');f.service.dispose();
});
test('manual edit during inference blocks every proposed write',async()=>{
    const f=fixture();f.deps.model.plan=async()=>{f.edit();return f.plan;};await f.service.submit('advance 100 ticks');
    assert.equal(f.writes.length,0);assert.match(f.events.find(e=>e.type==='error').text,/world changed/);f.service.dispose();
});
test('stop fences late model and JEV continuations',async()=>{
    const f=fixture();let release;f.deps.jev.evaluate=()=>new Promise(resolve=>{release=resolve;});
    const pending=f.service.submit('advance 100 ticks');await new Promise(resolve=>setTimeout(resolve,0));f.service.stop();release({decision:'execute',confidence:1});await pending;
    assert.equal(f.writes.length,0);f.service.dispose();
});
test('rejected decision and unavailable capability never write',async()=>{
    const f=fixture();f.deps.jev.evaluate=async()=>({decision:'reject',confidence:.9});await f.service.submit('step');assert.equal(f.writes.length,0);f.service.dispose();
});
test('answers do not require JEV and cannot include action payloads',async()=>{
    const f=fixture();f.deps.model.plan=async()=>({kind:'answer',message:'',actions:[]});f.deps.jev.evaluate=()=>{throw new Error('must not call');};await f.service.submit('explain');
    assert.equal(f.writes.length,0);assert.equal(f.events.find(e=>e.type==='answer').text,'Measured facts only');
    assert.throws(()=>validatePlan({...f.plan,kind:'answer'},f.observe()),/Only command/);f.service.dispose();
});

test('greetings and capability help work before model loading or JEV connection',async()=>{
    const f=fixture();f.deps.model.ready=false;f.deps.jev.connected=false;
    f.deps.model.plan=async()=>{throw new Error('Help must not require inference');};
    await f.service.submit('Hello');await f.service.submit('What can you do?');
    assert.equal(f.writes.length,0);assert.equal(f.events.filter(e=>e.type==='error').length,0);
    const answer=f.events.filter(e=>e.type==='answer').at(-1).text;
    assert.match(answer,/Advance 10 ticks/);assert.match(answer,/Download \/ load model/);assert.match(answer,/Connect a JEV key/);f.service.dispose();
});

test('Help leaves an independent model download running, while explicit Stop cancels it',async()=>{
    const f=fixture();let cancellations=0;
    f.deps.model.cancel=()=>{cancellations++;};
    await f.service.submit('What can you do?');
    assert.equal(cancellations,0);
    await f.service.submit('Stop AI');
    assert.equal(cancellations,1);
    f.service.dispose();
});

test('Help still cancels an active generation before answering',async()=>{
    const f=fixture(),started=deferred(),finish=deferred();let cancellations=0;
    f.deps.model.cancel=()=>{cancellations++;};
    f.deps.model.plan=async()=>{started.resolve();await finish.promise;return f.plan;};
    const pending=f.service.submit('Advance 100 ticks');await started.promise;
    await f.service.submit('What can you do?');
    assert.equal(cancellations,1);
    finish.resolve();await pending;
    assert.equal(f.writes.length,0);
    f.service.dispose();
});

test('an explicit experiment request enters bounded experiment mode without a checkbox',async()=>{
    const f=fixture();let called=false;
    f.deps.model.experimentStep=async goal=>{called=true;assert.match(goal,/Run an experiment/);return{kind:'clarify',message:'Name the quantity to measure.',actions:[],experiment:{phase:'clarify',waitMs:0,reason:'A quantity is needed.'}};};
    f.deps.model.plan=async()=>{throw new Error('Goal entered the literal command parser');};
    await f.service.submit('Run an experiment to measure the current lattice');
    assert.equal(called,true);assert.equal(f.writes.length,0);assert.equal(f.events.filter(e=>e.type==='error').length,0);f.service.dispose();
});

test('a question about running experiments cannot enable autonomy',async()=>{
    const f=fixture();f.deps.model.plan=async()=>({kind:'answer',message:'',actions:[]});
    f.deps.model.experimentStep=async()=>{throw new Error('Question enabled autonomy');};
    await f.service.submit('How do I run an experiment?');
    await f.service.submit('How do I run an experiment?',{autonomous:true});
    assert.equal(f.writes.length,0);assert.equal(f.events.filter(e=>e.type==='error').length,0);f.service.dispose();
});

test('negative requests cannot become experiment goals through a retained checkbox',async()=>{
    const f=fixture();
    f.deps.model.plan=async()=>({kind:'clarify',message:'No action was taken.',actions:[]});
    f.deps.model.experimentStep=async()=>{throw new Error('Negative request enabled autonomy');};
    await f.service.submit('Please do not pause.',{autonomous:true});
    await f.service.submit('Never run an experiment.',{autonomous:true});
    assert.equal(f.writes.length,0);assert.equal(f.events.filter(e=>e.type==='error').length,0);f.service.dispose();
});
test('a bounded goal cannot exceed 50 actions',async()=>{
    const f=fixture();let time=0;f.service.now=()=>time;f.deps.model.plan=async()=>{time+=301000;return f.plan;};await f.service.submit('keep stepping',{autonomous:true});
    assert.equal(f.writes.length,0);assert.match(f.events.find(e=>e.type==='error').text,/limit/);f.service.dispose();
});
test('JEV key appears only in request authorization, not body or error',async()=>{
    let sent;const client=new JevClient({fetchImpl:async(url,init)=>{sent=init;return new globalThis.Response(JSON.stringify({decision:'execute',confidence:.9,model:'jev'}));}});
    client.setKey('private-test-key');await client.evaluate({intent:'pause',plan:{},observation:{}},new AbortController().signal);
    assert.equal(sent.headers.Authorization,'Bearer private-test-key');assert.ok(!sent.body.includes('private-test-key'));client.dispose();assert.equal(client.connected,false);
});
test('JEV fails closed on malformed decisions and error statuses',async()=>{
    const client=new JevClient({localConfigured:true,fetchImpl:async()=>new globalThis.Response(JSON.stringify({decision:'execute',confidence:Infinity}))});
    await assert.rejects(client.evaluate({},new AbortController().signal),/Invalid JEV/);
});

function deferred() {
    let resolve;
    const promise=new Promise(done=>{resolve=done;});
    return {promise,resolve};
}

function playbackFixture() {
    const worlds={lattice:{ownerId:'lattice-one',version:0,tick:0,running:true},observer:{ownerId:'observer-one',version:0,tick:0,running:false}};
    let workspace='lattice';
    const f=fixture();
    const empty={type:'object',properties:{},required:[],additionalProperties:false};
    const observe=()=>{
        const world=worlds[workspace];
        const actions=['pause','resume','seed.apply'].map(name=>({type:`${workspace}.${name}`,args:empty}));
        actions.push({type:`${workspace}.step`,args:{type:'object',properties:{count:{type:'integer',minimum:1,maximum:4096}},required:['count'],additionalProperties:false}});
        actions.push({type:'workspace.switch',args:{type:'object',properties:{workspace:{type:'string',enum:['lattice','observer']}},required:['workspace'],additionalProperties:false}});
        return {workspace,ownerId:world.ownerId,preparationVersion:String(world.version),tick:world.tick,
            facts:{running:world.running},capabilities:actions.map(action=>action.type),actions};
    };
    f.control.observe=observe;
    f.control.execute=async(action,{expected,assertActive})=>{
        assertActive();assertObservation(observe(),expected);
        const before=observe(),world=worlds[workspace];
        f.writes.push(action);
        if(action.type.endsWith('.pause'))world.running=false;
        if(action.type.endsWith('.resume'))world.running=true;
        if(action.type.endsWith('.step')){world.running=false;world.tick+=action.args.count;}
        if(action.type.endsWith('.seed.apply')){world.running=false;world.version++;world.tick=0;}
        if(action.type==='workspace.switch')workspace=action.args.workspace;
        return {status:'applied',action,before,after:observe()};
    };
    return {...f,observe,worlds};
}

test('stop retains an in-flight completion receipt but suppresses its late answer and observations',async()=>{
    const f=fixture(),started=deferred(),finish=deferred(),execute=f.control.execute;
    f.control.execute=async(...args)=>{const receipt=await execute(...args);started.resolve();await finish.promise;return receipt;};
    const pending=f.service.submit('advance 100 ticks');
    await started.promise;f.service.stop();const boundary=f.events.length;finish.resolve();await pending;
    assert.equal(f.writes.length,1);
    assert.equal(f.events.slice(boundary).filter(event=>event.type==='receipt').length,1);
    assert.deepEqual(f.events.slice(boundary).filter(event=>['answer','observation'].includes(event.type)),[]);
    f.service.dispose();
});

test('a superseded in-flight action cannot append a completion answer to the replacement request',async()=>{
    const f=fixture(),started=deferred(),finish=deferred(),execute=f.control.execute;
    f.control.execute=async(...args)=>{const receipt=await execute(...args);started.resolve();await finish.promise;return receipt;};
    const pending=f.service.submit('advance 100 ticks');await started.promise;
    f.deps.model.plan=async()=>({kind:'answer',message:'',actions:[]});
    await f.service.submit('Explain the current state');
    finish.resolve();await pending;
    assert.deepEqual(f.events.filter(event=>event.type==='answer').map(event=>event.text),['Measured facts only']);
    f.service.dispose();
});

test('pause while thinking restores only an unchanged preparation after an answer',async()=>{
    for(const manualEdit of [false,true]) {
        const f=playbackFixture();
        f.deps.model.plan=async()=>({kind:'answer',message:'',actions:[]});
        f.deps.model.explain=async()=>{if(manualEdit)f.worlds.lattice.version++;return 'Captured observation';};
        await f.service.submit('Explain the lattice',{pauseWhileThinking:true});
        assert.deepEqual(f.writes.map(action=>action.type),manualEdit?['lattice.pause']:['lattice.pause','lattice.resume']);
        assert.equal(f.worlds.lattice.running,!manualEdit);
        f.service.dispose();
    }
});

test('stop during pause-while-thinking inference leaves playback paused',async()=>{
    const f=playbackFixture(),started=deferred(),finish=deferred();
    f.deps.model.plan=async()=>{started.resolve();await finish.promise;return{kind:'answer',message:'',actions:[]};};
    const pending=f.service.submit('Explain the lattice',{pauseWhileThinking:true});
    await started.promise;f.service.stop();finish.resolve();await pending;
    assert.deepEqual(f.writes.map(action=>action.type),['lattice.pause']);
    assert.equal(f.worlds.lattice.running,false);f.service.dispose();
});

test('applying a seed on the same native owner does not restore the retired preparation playback',async()=>{
    const f=playbackFixture();
    f.deps.model.plan=async()=>({kind:'actions',message:'Install the prepared seed',actions:[{type:'lattice.seed.apply',args:{}}]});
    await f.service.submit('Apply the prepared seed',{pauseWhileThinking:true});
    assert.equal(f.observe().ownerId,'lattice-one');
    assert.equal(f.observe().preparationVersion,'1');
    assert.deepEqual(f.writes.map(action=>action.type),['lattice.pause','lattice.seed.apply']);
    assert.equal(f.worlds.lattice.running,false);f.service.dispose();
});

test('workspace switch restores source playback before taking the destination receipt fence',async()=>{
    const f=playbackFixture();
    f.deps.model.plan=async()=>({kind:'actions',message:'Open observer',actions:[{type:'workspace.switch',args:{workspace:'observer'}}]});
    await f.service.submit('Open Mind’s Eye',{pauseWhileThinking:true});
    assert.deepEqual(f.writes.map(action=>action.type),['lattice.pause','lattice.resume','workspace.switch']);
    const receipt=f.events.filter(event=>event.type==='receipt').at(-1).receipt;
    assert.equal(receipt.before.workspace,'lattice');assert.equal(receipt.before.facts.running,true);
    assert.equal(receipt.after.workspace,'observer');assert.equal(receipt.after.ownerId,'observer-one');
    assert.equal(f.worlds.observer.running,false);assert.equal(f.worlds.lattice.running,true);
    assert.equal(f.events.filter(event=>event.type==='error').length,0);f.service.dispose();
});

test('an autonomous run stops at its actual 50-action limit across single-step plans',async t=>{
    const setTimeoutActual=globalThis.setTimeout;
    t.mock.method(globalThis,'setTimeout',(callback,delay,...args)=>setTimeoutActual(callback,delay===1000?0:delay,...args));
    const f=fixture();
    f.deps.model.plan=async()=>({...f.plan,actions:[{type:'lattice.step',args:{count:1}}]});
    await f.service.submit('Keep advancing until the bounded run ends',{autonomous:true});
    assert.equal(f.writes.length,50);
    assert.match(f.events.find(event=>event.type==='error').text,/limit/);
    f.service.dispose();
});

test('deadline exhaustion blocks a decision that arrives late and any queued playback restoration',async()=>{
    const f=playbackFixture();let now=0;f.service.now=()=>now;
    f.deps.jev.evaluate=async()=>{now=300001;return{decision:'execute',confidence:1};};
    await f.service.submit('Step once',{pauseWhileThinking:true,autonomous:true});
    assert.deepEqual(f.writes.map(action=>action.type),['lattice.pause']);
    assert.equal(f.worlds.lattice.running,false);
    assert.match(f.events.find(event=>event.type==='error').text,/limit/);f.service.dispose();
});

test('optional playback writes share the 50-action budget and cannot resume after exhaustion',async t=>{
    const setTimeoutActual=globalThis.setTimeout;
    t.mock.method(globalThis,'setTimeout',(callback,delay,...args)=>setTimeoutActual(callback,delay===1000?0:delay,...args));
    const f=playbackFixture();
    f.deps.model.plan=async()=>({...f.plan,actions:[{type:'lattice.step',args:{count:1}}]});
    await f.service.submit('Keep advancing until the bounded run ends',{autonomous:true,pauseWhileThinking:true});
    assert.equal(f.writes.length,50);
    assert.equal(f.writes[0].type,'lattice.pause');
    assert.equal(f.writes.filter(action=>action.type==='lattice.step').length,49);
    assert.equal(f.worlds.lattice.running,false);
    assert.equal(f.writes.filter(action=>action.type==='lattice.resume').length,0);
    f.service.dispose();
});

test('reaching the explicit action limit still permits the requested read-only completion explanation',async()=>{
    const f=fixture();let explanations=0;
    f.deps.model.plan=async()=>({...f.plan,actions:Array.from({length:8},()=>({type:'lattice.step',args:{count:1}}))});
    f.deps.model.explain=async()=>{explanations++;return 'Eight completed ticks are recorded.';};
    await f.service.submit('Advance eight times, then explain what changed');
    assert.equal(f.writes.length,8);
    assert.equal(explanations,1);
    assert.equal(f.events.filter(event=>event.type==='error').length,0);
    assert.equal(f.events.filter(event=>event.type==='answer').at(-1).text,'Eight completed ticks are recorded.');
    f.service.dispose();
});

test('an uncertain step acknowledgement suppresses automatic playback restoration',async()=>{
    const f=playbackFixture(),execute=f.control.execute;
    f.control.execute=async(...args)=>{
        const receipt=await execute(...args);
        return args[0].type==='lattice.step'?{...receipt,status:'unknown',error:'Tick acknowledgement timed out'}:receipt;
    };
    await f.service.submit('Advance 100 ticks',{pauseWhileThinking:true});
    assert.deepEqual(f.writes.map(action=>action.type),['lattice.pause','lattice.step']);
    assert.equal(f.worlds.lattice.running,false);
    assert.match(f.events.find(event=>event.type==='error').text,/acknowledgement timed out/);
    f.service.dispose();
});

test('an unknown transport exception cannot enqueue a resume after a possibly committed step',async()=>{
    const f=playbackFixture(),execute=f.control.execute;
    f.control.execute=async(...args)=>{
        const receipt=await execute(...args);
        if(args[0].type==='lattice.step')throw Object.assign(new Error('Connection lost after tick submission'),{status:'unknown'});
        return receipt;
    };
    await f.service.submit('Advance 100 ticks',{pauseWhileThinking:true});
    assert.deepEqual(f.writes.map(action=>action.type),['lattice.pause','lattice.step']);
    assert.equal(f.worlds.lattice.running,false);
    assert.match(f.events.find(event=>event.type==='error').text,/Connection lost/);
    f.service.dispose();
});
