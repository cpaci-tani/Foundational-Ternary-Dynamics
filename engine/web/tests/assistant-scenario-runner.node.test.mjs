import test from 'node:test';
import assert from 'node:assert/strict';
import {AssistantService} from '../js/assistant/service.js';
import {assertObservation,validateValue} from '../js/assistant/contracts.js';
import {validateRecipe} from '../js/seeding/recipe.js';
import {LIMITS} from '../js/assistant/contracts.js';
import {SCENARIO_LIMITS} from '../js/assistant/scenario-limits.js';

const clone=value=>structuredClone(value);
const allow={decision:'execute',confidence:1,model:'test-jev'};
const schema=properties=>({type:'object',properties,required:Object.keys(properties),additionalProperties:false});
function deferred(){let resolve;const promise=new Promise(done=>{resolve=done;});return {promise,resolve};}
function fixture(t){
    const f={events:[],trace:[],operations:[],approvals:[],generations:[],installed:[],hooks:{},preparedTick:'0',clock:0};
    f.template={scenarioId:'test-wave',label:'Test wave',backend:'effective-lattice',size:33,
        recipe:{version:2,scenarioId:'test-wave',size:33,blank:false,randomSeed:0,components:[],overrides:{}},
        properties:[{key:'amplitude',path:['overrides','amplitude'],type:'real',min:0,max:10,units:'simulation units',value:1,default:1},
            {key:'protocol.boundary',path:['overrides','protocol.boundary'],type:'choice',min:0,max:1,options:[[0,'periodic'],[1,'reflective']],units:'',value:0,default:0}],
        allowedMeasurements:['manifested','positive','negative','dynamicEnergy']};
    const descriptors=[
        {type:'lattice.seed.describe',args:schema({scenarioId:{type:'string',enum:['test-wave']}})},
        {type:'lattice.seed.preview',args:schema({recipeJson:{type:'string',maxLength:65536}})},
        {type:'lattice.seed.apply',args:schema({previewId:{type:'string',maxLength:200}})},
        {type:'lattice.pause',args:schema({})},
        {type:'lattice.step',args:schema({count:{type:'integer',minimum:1,maximum:4096}})},
    ];
    f.state={workspace:'lattice',ownerId:'original',preparationVersion:'initial',tick:'500',
        capabilities:descriptors.map(row=>row.type),actions:descriptors,selected:null,
        facts:{mode:'lattice',backend:'Worker',latticeSize:33,scenarioId:'old-scenario',available:true,stale:false,running:true,
            sampleTick:'500',manifested:'4',positive:'2',negative:'2',totalFlux:1,dynamicEnergy:1}};
    const previews=new Map();let previewId=0,owner=0;
    f.control={observe:()=>clone(f.state),listScenarioTemplates:()=>[{id:'test-wave',label:'Test wave',category:'Waves',description:'Wave amplitude',backend:'effective-lattice',sizes:[33]}],
        async execute(action,options){
            options.signal?.throwIfAborted();options.assertActive?.();assertObservation(f.control.observe(),options.expected);
            const spec=descriptors.find(row=>row.type===action.type);assert.ok(spec,`Unexpected action ${action.type}`);validateValue(action.args,spec.args);
            const before=f.control.observe();f.trace.push(`control:${action.type}`);
            if(action.type==='lattice.seed.describe')return {status:'applied',action,before,after:f.control.observe(),result:{template:clone(f.template)}};
            f.operations.push(clone(action));
            if(f.hooks.beforeOperation)await f.hooks.beforeOperation(action,options);
            options.signal?.throwIfAborted();options.assertActive?.();assertObservation(f.control.observe(),options.expected);
            let result;
            if(action.type==='lattice.seed.preview'){
                const recipe=JSON.parse(action.args.recipeJson);validateRecipe(recipe,[{id:'test-wave',backend:'effective-lattice',sizes:[33]}]);
                const id=`preview-${++previewId}`;previews.set(id,recipe);result={previewId:id};
            }else if(action.type==='lattice.seed.apply'){
                const recipe=previews.get(action.args.previewId);assert.ok(recipe,'An apply must consume a prepared preview');previews.delete(action.args.previewId);
                f.installed.push(clone(recipe));f.state.ownerId=`owner-${++owner}`;f.state.preparationVersion=`seed-${owner}`;
                const blank=recipe.blank && recipe.components.every(component=>!component.enabled);
                f.state.tick=blank?'0':f.preparedTick;Object.assign(f.state.facts,{scenarioId:recipe.scenarioId,running:false,sampleTick:f.state.tick,
                    manifested:blank?'0':'6',positive:blank?'0':'3',negative:blank?'0':'3',totalFlux:blank?0:1,dynamicEnergy:blank?0:recipe.overrides.amplitude??1});
                if(f.hooks.installed)f.hooks.installed(recipe,blank);
            }else if(action.type==='lattice.pause')f.state.facts.running=false;
            else if(action.type==='lattice.step'){
                assert.equal(f.state.facts.running,false,'Tick requests must use a paused baseline');
                const next=BigInt(f.state.tick)+BigInt(action.args.count);
                f.state.tick=typeof f.state.tick==='number'?Number(next):String(next);f.state.facts.sampleTick=f.state.tick;
            }
            let receipt={status:'applied',action:clone(action),before,after:f.control.observe(),...(result?{result}:{})};
            if(f.hooks.receipt)receipt=await f.hooks.receipt(receipt);
            return receipt;
        }};
    f.model={ready:true,cancel(){},async generate(messages,signal){
        signal.throwIfAborted();const context=JSON.parse(messages[1].content);f.generations.push(context);f.trace.push(context.candidates?'model:choose':'model:design');
        if(f.hooks.generate)return f.hooks.generate(context,signal);
        return JSON.stringify(context.candidates?{scenarioId:'test-wave'}:{name:'Amplitude experiment',settings:[{key:'amplitude',value:2.5},{key:'protocol.boundary',value:1}],measurements:['manifested','dynamicEnergy']});
    }};
    f.jev={connected:true,async evaluate(request,signal){
        signal.throwIfAborted();f.approvals.push(clone(request));
        const phase=request.observation.facts.scenarioExperiment.phase;
        f.trace.push(`jev:${phase}:${request.plan.actions[0]?.type??'complete'}`);
        return f.hooks.evaluate?f.hooks.evaluate(request,signal):allow;
    }};
    f.service=new AssistantService({getControl:()=>f.control,model:f.model,jev:f.jev,knowledge:{},now:()=>f.clock,
        onEvent:event=>{f.events.push(event);f.hooks.event?.(event);}});
    f.request={scenarioId:'test-wave',goal:'Measure the amplitude response with a chosen boundary.',ticks:10,sampleEvery:4,useLLM:true};
    t.after(()=>f.service.dispose());return f;
}

test('real service runs approved blank preparation, LLM settings, compiled baseline, exact intervals and conclusion',async t=>{
    const f=fixture(t),original=clone(f.template);const result=await f.service.runScenarioExperiment(f.request);
    assert.equal(result.status,'complete',result.summary);assert.deepEqual(f.template,original);
    assert.deepEqual(f.operations.map(action=>action.type),['lattice.seed.preview','lattice.seed.apply','lattice.seed.preview','lattice.seed.apply','lattice.step','lattice.step','lattice.step']);
    assert.equal(f.installed[0].blank,true);assert.ok(f.installed[0].components.every(component=>component.enabled===false));
    assert.deepEqual(f.installed[0].overrides,{});assert.equal(f.generations.length,1);
    assert.equal(f.generations[0].emptyObservation.facts.manifested,'0');assert.equal(f.generations[0].emptyObservation.facts.running,false);
    assert.ok(f.trace.indexOf('model:design')>f.trace.indexOf('control:lattice.seed.apply'));
    assert.equal(f.installed[1].overrides.amplitude,2.5);assert.equal(f.installed[1].overrides['protocol.boundary'],1);
    assert.equal(result.emptyBaseline.ownerId,'owner-1');assert.equal(result.baseline.ownerId,'owner-2');
    assert.deepEqual(result.samples.map(sample=>sample.tick),['0','4','8','10']);assert.equal(f.state.facts.running,false);
    assert.deepEqual(f.operations.filter(action=>action.type==='lattice.step').map(action=>action.args.count),[4,4,2]);
    assert.deepEqual(f.approvals.map(row=>row.observation.facts.scenarioExperiment.phase),['empty','protocol']);
    assert.deepEqual(f.approvals.map(row=>row.observation.facts.scenarioExperiment.authorization.scope),['empty-preparation','fixed-protocol']);
    assert.equal(f.approvals.length,2);
    assert.deepEqual(result.approvalReceipts.map(row=>[row.phase,row.scope,row.decision]),
        [['empty','empty-preparation','execute'],['protocol','fixed-protocol','execute']]);
    assert.ok(f.trace.indexOf('jev:empty:lattice.seed.preview')<f.trace.indexOf('control:lattice.seed.preview'));
    for(const approved of f.approvals.filter(row=>row.observation.facts.scenarioExperiment.phase==='protocol')){
        const shared=approved.observation.facts.scenarioExperiment.template;assert.deepEqual(shared,result.template);
        assert.deepEqual(shared.settings,[{key:'amplitude',value:2.5,units:'simulation units'},{key:'protocol.boundary',value:1,units:''}]);
    }
    assert.ok(f.events.some(event=>event.type==='scenario-result'&&event.result.status==='complete'));
});

test('canned templates use unchanged constructor defaults without invoking model generation',async t=>{
    const f=fixture(t);f.model.ready=false;f.hooks.generate=()=>{throw new Error('Canned execution must not call the model');};
    const result=await f.service.runScenarioExperiment({...f.request,useLLM:false});
    assert.equal(result.status,'complete',result.summary);assert.equal(f.generations.length,0);
    assert.deepEqual(f.installed[1],f.template.recipe);assert.deepEqual(result.template.settings,[]);
    assert.equal(f.approvals.length,1);assert.equal(result.progress.approvalCount,1);
    const context=f.approvals[0].observation.facts.scenarioExperiment;
    assert.equal(context.phase,'protocol');assert.equal(context.currentPreparation.ownerId,'original');
    assert.equal(context.authorization.includesEmptyPreparation,true);assert.equal(context.emptyPreparation.blank,true);
    assert.equal(context.protocol.ticks,10);assert.equal(context.protocol.sampleEvery,4);assert.equal(context.protocol.chunkTicks,128);
    assert.deepEqual(context.preparation,f.template.recipe);
    assert.deepEqual(context.operations.map(row=>row.type),['lattice.seed.preview','lattice.seed.apply','lattice.pause','lattice.seed.preview','lattice.seed.apply','lattice.pause','lattice.step']);
});

test('external scenario drafts use the same empty-lattice preparation and JEV settings context without a model',async t=>{
    const f=fixture(t);f.model.ready=false;f.hooks.generate=()=>assert.fail('External planner must not call embedded model');
    const draft={name:'External amplitude experiment',goal:f.request.goal,ticks:10,sampleEvery:4,settings:[{key:'amplitude',value:3}],measurements:['dynamicEnergy']};
    const result=await f.service.runScenarioExperiment({...f.request,useLLM:false,draft,expected:f.control.observe()});
    assert.equal(result.status,'complete',result.summary);assert.equal(f.generations.length,0);assert.equal(f.installed[0].blank,true);
    assert.equal(f.installed[1].overrides.amplitude,3);assert.equal(result.baseline.facts.dynamicEnergy,3);
    assert.ok(f.approvals.every(row=>row.observation.facts.scenarioExperiment.template.settings[0].value===3));
});

test('invalid external drafts and stale expected preparations fail before any empty-lattice write',async t=>{
    for(const change of [{settings:[{key:'amplitude',value:999}]},{ticks:11},{goal:'A different protocol'}]){
        const f=fixture(t),draft={name:'External',goal:f.request.goal,ticks:10,sampleEvery:4,settings:[],measurements:['dynamicEnergy'],...change};
        const result=await f.service.runScenarioExperiment({...f.request,useLLM:false,draft});
        assert.equal(result.status,'failed');assert.equal(f.operations.length,0);
    }
    const f=fixture(t),expected=f.control.observe();expected.preparationVersion='old';
    assert.equal((await f.service.runScenarioExperiment({...f.request,useLLM:false,expected})).status,'failed');assert.equal(f.operations.length,0);
});

test('automatic template selection is bounded to registry candidates before describing and preparing',async t=>{
    const f=fixture(t);const result=await f.service.runScenarioExperiment({...f.request,scenarioId:'__design__'});
    assert.equal(result.status,'complete',result.summary);assert.equal(f.generations.length,2);
    assert.deepEqual(f.generations[0].candidates.map(row=>row.id),['test-wave']);
    assert.equal(result.template.scenarioId,'test-wave');
});

test('missing JEV credentials and unloaded design model perform no preparations',async t=>{
    const noKey=fixture(t);noKey.jev.connected=false;
    const first=await noKey.service.runScenarioExperiment(noKey.request);
    assert.equal(first.status,'failed');assert.match(first.summary,/Connect a JEV key/);assert.equal(noKey.operations.length,0);assert.equal(noKey.trace.length,0);
    const noModel=fixture(t);noModel.model.ready=false;
    const second=await noModel.service.runScenarioExperiment(noModel.request);
    assert.equal(second.status,'failed');assert.match(second.summary,/Load the local model/);assert.equal(noModel.operations.length,0);
});

test('Stop AI while JEV is outstanding prevents the approved write after a late response',async t=>{
    const f=fixture(t),entered=deferred(),decision=deferred();
    f.hooks.evaluate=async()=>{entered.resolve();return decision.promise;};
    const pending=f.service.runScenarioExperiment(f.request);await entered.promise;f.service.stop('User stop');decision.resolve(allow);
    const result=await pending;assert.equal(result.status,'stopped');assert.equal(f.operations.length,0);assert.equal(f.installed.length,0);
});

test('Stop AI at the observed empty baseline suppresses design and later preparation',async t=>{
    const f=fixture(t);f.hooks.event=event=>{if(event.type==='scenario-sample'&&event.phase==='empty')f.service.stop('Stopped between operations');};
    const result=await f.service.runScenarioExperiment(f.request);
    assert.equal(result.status,'stopped');assert.equal(f.installed.length,1);assert.equal(f.generations.length,0);
    assert.deepEqual(f.operations.map(action=>action.type),['lattice.seed.preview','lattice.seed.apply']);
});

test('manual preparation changes during JEV or model work invalidate pending continuations',async t=>{
    const f=fixture(t);f.hooks.evaluate=()=>{f.state.preparationVersion='manual-change';return allow;};
    const first=await f.service.runScenarioExperiment(f.request);assert.equal(first.status,'failed');assert.match(first.summary,/world changed/);assert.equal(f.operations.length,0);
    const g=fixture(t);g.hooks.generate=()=>{g.state.preparationVersion='manual-change';return JSON.stringify({name:'Edited',settings:[],measurements:['manifested']});};
    const second=await g.service.runScenarioExperiment(g.request);assert.equal(second.status,'failed');assert.match(second.summary,/world changed/);assert.equal(g.installed.length,1);
});

test('JEV rejection or clarification prevents every subsequent operation',async t=>{
    for(const decision of ['reject','clarify']){
        const f=fixture(t);f.hooks.evaluate=request=>request.observation.facts.scenarioExperiment.phase==='protocol'?{...allow,decision}:allow;
        const result=await f.service.runScenarioExperiment(f.request);
        assert.equal(result.status,decision);assert.equal(f.installed.length,1);assert.equal(f.operations.length,2);
        assert.equal(f.operations.some(action=>action.type==='lattice.step'),false);
    }
});

test('malicious model identity/backend fields and unsupported settings cannot reach a second seed preview',async t=>{
    for(const extra of [{backend:'finite-records'},{scenarioId:'other'},{recipe:{}},{size:65},{ticks:4096},{settings:[{key:'size',value:65}]}]){
        const f=fixture(t);f.hooks.generate=()=>JSON.stringify({name:'Invalid design',settings:[],measurements:['manifested'],...extra});
        const result=await f.service.runScenarioExperiment(f.request);
        assert.equal(result.status,'failed');assert.equal(f.operations.length,2);assert.equal(f.installed.length,1);
        assert.equal(f.installed[0].size,33);assert.equal(f.installed[0].scenarioId,'test-wave');
    }
});

test('two hundred measured intervals plus baseline are allowed; larger protocols fail before writes',async t=>{
    const f=fixture(t);const result=await f.service.runScenarioExperiment({...f.request,ticks:200,sampleEvery:1,useLLM:false});
    assert.equal(result.status,'complete',result.summary);assert.equal(result.samples.length,201);assert.equal(f.approvals.length,1);
    for(const request of [{ticks:201,sampleEvery:1},{ticks:100001,sampleEvery:1000}]){
        const g=fixture(t);const tooMany=await g.service.runScenarioExperiment({...g.request,...request,useLLM:false});
        assert.equal(tooMany.status,'failed');assert.match(tooMany.summary,/200 measured intervals/);assert.equal(g.operations.length,0);
    }
});

test('large exact string ticks keep single-tick sampling exact',async t=>{
    const f=fixture(t);f.preparedTick='900719925474099312345';
    const result=await f.service.runScenarioExperiment({...f.request,ticks:3,sampleEvery:1});
    assert.equal(result.status,'complete',result.summary);
    assert.deepEqual(result.samples.map(sample=>sample.tick),['900719925474099312345','900719925474099312346','900719925474099312347','900719925474099312348']);
});

test('an uncertain committed receipt remains auditable and is never retried or followed by more writes',async t=>{
    const f=fixture(t);let seen=0;f.hooks.receipt=receipt=>receipt.action.type==='lattice.seed.apply'&&++seen===2?{...receipt,status:'unknown',error:'Commit acknowledgement was lost'}:receipt;
    const result=await f.service.runScenarioExperiment(f.request);
    assert.equal(result.status,'failed');assert.match(result.summary,/acknowledgement was lost/);assert.equal(f.installed.length,2);
    assert.equal(f.operations.length,4);assert.equal(f.events.filter(event=>event.type==='receipt'&&event.receipt.status==='unknown').length,1);
});

test('an incorrect acknowledged step count stops before further measurements or conclusion',async t=>{
    const f=fixture(t);f.hooks.receipt=receipt=>{
        if(receipt.action.type==='lattice.step'){f.state.tick=String(BigInt(f.state.tick)+1n);f.state.facts.sampleTick=f.state.tick;return {...receipt,after:f.control.observe()};}
        return receipt;
    };
    const result=await f.service.runScenarioExperiment(f.request);
    assert.equal(result.status,'failed');assert.match(result.summary,/tick interval/);assert.equal(f.operations.filter(action=>action.type==='lattice.step').length,1);
    assert.equal(f.approvals.some(row=>row.observation.facts.scenarioExperiment.phase==='complete'),false);
});

test('an allegedly blank preparation with nonzero published counts stops before LLM design',async t=>{
    const f=fixture(t);f.hooks.installed=(recipe,blank)=>{if(blank)f.state.facts.manifested='1';};
    const result=await f.service.runScenarioExperiment(f.request);
    assert.equal(result.status,'failed');assert.match(result.summary,/nonzero manifested/);assert.equal(f.installed.length,1);assert.equal(f.generations.length,0);
});

test('unsafe numeric completed ticks cannot certify an exact experimental interval',async t=>{
    const f=fixture(t);f.preparedTick=Number.MAX_SAFE_INTEGER+1;
    const result=await f.service.runScenarioExperiment({...f.request,ticks:2,sampleEvery:2});
    assert.equal(result.status,'failed','An unsafe JS counter must not be promoted to exact BigInt evidence');
    assert.equal(f.approvals.some(row=>row.observation.facts.scenarioExperiment.phase==='complete'),false);
});

test('absent empty-state counters cannot establish a blank preparation',async t=>{
    const f=fixture(t);f.hooks.installed=(recipe,blank)=>{
        if(blank){delete f.state.facts.manifested;delete f.state.facts.positive;delete f.state.facts.negative;}
    };
    const result=await f.service.runScenarioExperiment(f.request);
    assert.equal(result.status,'failed','Blankness requires published count evidence');assert.equal(f.installed.length,1);assert.equal(f.generations.length,0);
});

test('missing selected measurements cannot produce a completed experiment',async t=>{
    const f=fixture(t);f.hooks.installed=(recipe,blank)=>{if(!blank)delete f.state.facts.dynamicEnergy;};
    const result=await f.service.runScenarioExperiment(f.request);
    assert.equal(result.status,'failed','Requested dynamicEnergy was never published');
    assert.equal(f.approvals.some(row=>row.observation.facts.scenarioExperiment.phase==='complete'),false);
});

test('explicit quantity ambiguity, negation and unsupported units fail before clearing the lattice',async t=>{
    for(const goal of ['Set amplitude to 0.5 m.','Do not set amplitude to 0.5.','Set amplitude to 2 or 3.']){
        const f=fixture(t),result=await f.service.runScenarioExperiment({...f.request,goal});
        assert.equal(result.status,'failed',goal);assert.equal(f.operations.length,0);assert.equal(f.installed.length,0);
        assert.equal(f.state.ownerId,'original');assert.equal(f.generations.length,0);
    }
});

test('10,000 ticks use one complete pre-write approval, exact chunks and only requested samples',async t=>{
    const f=fixture(t);f.preparedTick='900719925474099312345';
    f.hooks.evaluate=()=>{assert.equal(f.operations.length,0,'Approval must precede every preparation write');return allow;};
    f.hooks.receipt=receipt=>{
        if(receipt.action.type==='lattice.step'&&f.clock===0)f.clock=6*60*1000;
        return receipt;
    };
    const result=await f.service.runScenarioExperiment({...f.request,ticks:10000,sampleEvery:250,useLLM:false});
    assert.equal(result.status,'complete',result.summary);assert.equal(f.approvals.length,1);
    assert.deepEqual(result.progress,{completedTicks:10000,totalTicks:10000,sampleCount:41,approvalCount:1});
    const counts=f.operations.filter(action=>action.type==='lattice.step').map(action=>action.args.count);
    assert.equal(counts.length,80);assert.equal(counts.reduce((sum,count)=>sum+count,0),10000);
    assert.deepEqual(counts.slice(0,4),[128,122,128,122]);assert.ok(counts.every(count=>count<=SCENARIO_LIMITS.chunkTicks));
    assert.deepEqual(result.samples.map(row=>row.tick),Array.from({length:41},(_,i)=>String(BigInt(f.preparedTick)+BigInt(i*250))));
    assert.equal(f.state.facts.running,false);assert.equal(result.outcomeUnknown,false);
    assert.equal(f.events.filter(event=>event.type==='scenario-progress'&&event.completedTicks===10000).at(-1).sampleCount,41);
    assert.equal(LIMITS.goalActions,50);assert.equal(LIMITS.goalMs,300000,'General autonomous budgets must remain unchanged');
    assert.match(result.summary,/1 JEV approval/);assert.match(result.summary,/84 acknowledged preparation\/chunk operations/);
    assert.deepEqual(result.approvalReceipts,[{phase:'protocol',scope:'fixed-protocol',decision:'execute',confidence:1,
        model:'test-jev',at:'1970-01-01T00:00:00.000Z',ticks:10000,sampleEvery:250}]);
});

test('canned rejection and clarification make no empty preparation writes',async t=>{
    for(const decision of ['reject','clarify']){
        const f=fixture(t);f.hooks.evaluate=()=>({...allow,decision});
        const result=await f.service.runScenarioExperiment({...f.request,ticks:10000,sampleEvery:250,useLLM:false});
        assert.equal(result.status,decision);assert.equal(f.operations.length,0);assert.equal(f.installed.length,0);
        assert.equal(result.progress.completedTicks,0);assert.equal(result.progress.approvalCount,0);
        assert.equal(result.approvalReceipts.length,1);assert.equal(result.approvalReceipts[0].decision,decision);
    }
});

test('Stop between chunks retains confirmed partial ticks and prevents later writes or new samples',async t=>{
    const f=fixture(t);f.hooks.event=event=>{
        if(event.type==='scenario-progress'&&event.completedTicks===256)f.service.stop('Stopped midway through the fixed protocol');
    };
    const result=await f.service.runScenarioExperiment({...f.request,ticks:10000,sampleEvery:1000,useLLM:false});
    assert.equal(result.status,'stopped');assert.equal(result.progress.completedTicks,256);
    assert.equal(result.samples.length,1,'An uncompleted measurement interval cannot become a sampled observation');
    assert.equal(f.operations.filter(action=>action.type==='lattice.step').length,2);
    assert.equal(f.approvals.length,1);assert.equal(f.state.facts.running,false);assert.equal(result.outcomeUnknown,false);
    assert.match(result.summary,/256 \/ 10000 confirmed ticks/);
});

test('owner replacement or manual ticks between chunks invalidate the remaining authorized protocol',async t=>{
    for(const change of ['owner','ticks','playback']){
        const f=fixture(t);f.hooks.event=event=>{
            if(event.type==='scenario-progress'&&event.completedTicks===128){
                if(change==='owner')f.state.ownerId='manual-owner';
                if(change==='ticks'){f.state.tick='129';f.state.facts.sampleTick='129';}
                if(change==='playback')f.state.facts.running=true;
            }
        };
        const result=await f.service.runScenarioExperiment({...f.request,ticks:10000,sampleEvery:1000,useLLM:false});
        assert.equal(result.status,'failed',change);assert.equal(result.progress.completedTicks,128);
        assert.equal(f.operations.filter(action=>action.type==='lattice.step').length,1);assert.equal(f.approvals.length,1);
        assert.match(result.summary,/world changed|outside the approved fixed protocol/);
        if(change==='playback')assert.equal(f.state.facts.running,true,'The runner must preserve manual playback after losing authority');
    }
});

test('a partially acknowledged failing chunk records confirmed ticks without retry or fabricated measurements',async t=>{
    const f=fixture(t);let chunks=0;
    f.hooks.receipt=receipt=>{
        if(receipt.action.type==='lattice.step'&&++chunks===3){
            f.state.tick=String(BigInt(receipt.before.tick)+5n);f.state.facts.sampleTick=f.state.tick;
            return {...receipt,after:f.control.observe(),status:'unknown',completedTicks:5,error:'Worker failed after five confirmed ticks'};
        }
        return receipt;
    };
    const result=await f.service.runScenarioExperiment({...f.request,ticks:10000,sampleEvery:1000,useLLM:false});
    assert.equal(result.status,'failed');assert.equal(result.progress.completedTicks,261);assert.equal(chunks,3);
    assert.equal(result.outcomeUnknown,true);assert.equal(result.samples.length,1);
    assert.equal(result.receipts.at(-1).completedTicks,5);assert.equal(result.receipts.at(-1).status,'unknown');
    assert.match(result.summary,/261 \/ 10000 confirmed ticks/);assert.match(result.summary,/uncertain/);
});

test('a manual tick arriving in a progress notification is fenced before the next dispatch',async t=>{
    const f=fixture(t);f.hooks.event=event=>{
        if(event.type==='scenario-stage'&&event.phase==='measuring'){
            f.state.tick=String(BigInt(f.state.tick)+1n);f.state.facts.sampleTick=f.state.tick;
        }
    };
    const result=await f.service.runScenarioExperiment({...f.request,ticks:10000,sampleEvery:1000,useLLM:false});
    assert.equal(result.status,'failed');assert.equal(result.progress.completedTicks,0);
    assert.equal(f.operations.filter(action=>action.type==='lattice.step').length,0);
    assert.equal(result.outcomeUnknown,false);assert.match(result.summary,/outside the approved fixed protocol/);
});

test('the deterministic deadline stops further chunks at 30 minutes with partial progress intact',async t=>{
    const f=fixture(t);f.hooks.event=event=>{
        if(event.type==='scenario-progress'&&event.completedTicks===128)f.clock=SCENARIO_LIMITS.durationMs;
    };
    const result=await f.service.runScenarioExperiment({...f.request,ticks:10000,sampleEvery:1000,useLLM:false});
    assert.equal(result.status,'failed');assert.equal(result.progress.completedTicks,128);
    assert.equal(f.operations.filter(action=>action.type==='lattice.step').length,1);assert.match(result.summary,/time or action limit/);
});

test('long-run receipt storage preserves preparation provenance and declares omitted chunk receipts',async t=>{
    const f=fixture(t);const result=await f.service.runScenarioExperiment({...f.request,ticks:40000,sampleEvery:1000,useLLM:false});
    assert.equal(result.status,'complete',result.summary);assert.equal(result.progress.completedTicks,40000);
    assert.equal(result.receiptCount,324);assert.equal(result.receipts.length,256);assert.equal(result.omittedReceipts,68);
    assert.deepEqual(result.receipts.slice(0,4).map(row=>row.action.type),['lattice.seed.preview','lattice.seed.apply','lattice.seed.preview','lattice.seed.apply']);
    assert.equal(result.receipts.at(-1).after.tick,'40000');assert.equal(result.samples.length,41);
    assert.equal(f.approvals.length,1);
    assert.equal(result.approvalReceipts.length,1);assert.equal(result.approvalReceipts[0].ticks,40000);
    assert.equal(result.approvalReceipts[0].decision,'execute');
});
