import test from 'node:test';
import assert from 'node:assert/strict';
import {buildDecisionRequest,DECISION_REQUEST_MAX_BYTES} from '../js/assistant/decision-context.js';
import {decisionObservation} from '../js/assistant/contracts.js';
import {McpControl} from '../js/assistant/mcp-control.js';
import {AssistantService} from '../js/assistant/service.js';
import {getScale0SeedingScenarios} from '../js/scales/scale0/scenario-registry.js';

const object=properties=>({type:'object',properties,required:Object.keys(properties),additionalProperties:false});
const descriptors=[
    {type:'lattice.scenario',description:'Load a registered preparation.',args:object({scenarioId:{type:'string',maxLength:120}})},
    {type:'lattice.step',description:'Pause and advance exact ticks.',args:object({count:{type:'integer',minimum:1,maximum:4096}})},
    {type:'lattice.seed.preview',description:'Stage a validated recipe.',args:object({recipeJson:{type:'string',maxLength:65536}})},
    {type:'lattice.seed.apply',description:'Apply a previously staged recipe.',args:object({previewId:{type:'string',maxLength:120}})},
    {type:'lattice.pause',description:'Pause the owner.',args:object({})},
];
const catalog=getScale0SeedingScenarios();
function observation(){return{workspace:'lattice',ownerId:'owner-1',preparationVersion:'preparation-1',tick:'32',
    actions:structuredClone(descriptors),capabilities:descriptors.map(row=>row.type),selected:null,
    facts:{scenarioId:'flux-pulse',scenarioCount:catalog.length,scenarios:catalog.slice(0,12).map(row=>({id:row.id,title:row.title})),
        sampleTick:'32',available:true,stale:false,running:false,manifested:'0'}};}
const plan=(type,args)=>({kind:'actions',message:'Run the requested supported action.',actions:[{type,args}]});
const bytes=value=>new TextEncoder().encode(JSON.stringify(value)).length;

test('JEV receives authoritative targets beyond the sampled catalog',()=>{
    for(const id of ['flux-dipole','flux-cascade']){
        const source=observation();assert.ok(catalog.findIndex(row=>row.id===id)>=12);
        const request=buildDecisionRequest(`Load ${id}.`,source,plan('lattice.scenario',{scenarioId:id}),{scenarioCatalog:catalog});
        const evidence=request.observation.facts.scenarioEvidence.targets.find(row=>row.scenarioId===id);
        assert.equal(evidence.registered,true);
        assert.equal(evidence.descriptor.title,catalog.find(row=>row.id===id).title);
        assert.equal(evidence.descriptor.qualification,catalog.find(row=>row.id===id).qualification);
        assert.deepEqual(request.observation.actions,[descriptors[0]]);
        assert.equal(request.observation.facts.scenarios,undefined,'Unrelated catalog samples must not imply a target allowlist');
        assert.ok(bytes(request)<=DECISION_REQUEST_MAX_BYTES);
        assert.equal(source.facts.scenarios.length,12,'Building context must not mutate the source observation');
    }
});

test('a 32-tick proposal carries the actual 1..4096 argument schema',()=>{
    const request=buildDecisionRequest('Advance exactly 32 ticks.',observation(),plan('lattice.step',{count:32}),{scenarioCatalog:catalog});
    assert.equal(request.plan.actions[0].args.count,32);
    assert.deepEqual(request.observation.actions[0].args.properties.count,{type:'integer',minimum:1,maximum:4096});
    assert.equal(request.observation.facts.decisionScope.proposedActions,1);
    assert.throws(()=>buildDecisionRequest('Advance too far.',observation(),plan('lattice.step',{count:4097})),/Invalid args.count/);
});

test('missing or unavailable target evidence is explicit and cannot be invented',()=>{
    const proposed=plan('lattice.scenario',{scenarioId:'not-a-registered-scenario'});
    const request=buildDecisionRequest('Load the named scenario.',observation(),proposed,{scenarioCatalog:catalog});
    const missing=request.observation.facts.scenarioEvidence.targets.find(row=>row.scenarioId===proposed.actions[0].args.scenarioId);
    assert.deepEqual(missing,{scenarioId:'not-a-registered-scenario',registered:false});
    const withoutCatalog=buildDecisionRequest('Load the named scenario.',observation(),proposed);
    assert.equal(withoutCatalog.observation.facts.scenarioEvidence.catalogAvailable,false);
    assert.equal(withoutCatalog.observation.facts.scenarioEvidence.targets.at(-1).registered,null);
});

test('recent evidence includes only confirmed receipts for the current owner and preparation',()=>{
    const current=observation();
    const receipt=(status,tick,ownerId=current.ownerId,preparationVersion=current.preparationVersion)=>({status,
        action:{type:'lattice.step',args:{count:16}},before:{...current,tick:String(tick-16)},after:{...current,ownerId,preparationVersion,tick:String(tick)},completedTicks:16});
    const receipts=[receipt('applied',16),receipt('applied',32),receipt('unknown',48),receipt('applied',64,'other-owner'),receipt('applied',80,current.ownerId,'old-preparation')];
    const request=buildDecisionRequest('Advance another 32 ticks.',current,plan('lattice.step',{count:32}),{receipts});
    const confirmed=request.observation.facts.confirmedReceipts;
    assert.deepEqual(confirmed.map(row=>row.after.tick),['16','32']);
    assert.deepEqual(confirmed.map(row=>row.action.args.count),[16,16]);
    assert.equal(confirmed[0].after.facts,undefined);
    assert.equal(confirmed[0].completedTicks,16);
});

test('large unrelated catalogs are excluded and credential fields never enter evidence',()=>{
    const source=observation();
    Object.assign(source.facts,{apiKey:'HIDDEN_API',headers:{Authorization:'HIDDEN_AUTH'},connection:{browserToken:'HIDDEN_BROWSER',clientToken:'HIDDEN_CLIENT'}});
    const rows=[...catalog,...Array.from({length:3000},(_,i)=>({id:`unrelated-${i}`,description:'x'.repeat(1000),password:'HIDDEN_PASSWORD'}))];
    const request=buildDecisionRequest('Advance 32 ticks.',source,plan('lattice.step',{count:32}),{scenarioCatalog:rows});
    const serialized=JSON.stringify(request);
    assert.ok(bytes(request)<=DECISION_REQUEST_MAX_BYTES);
    assert.equal(serialized.includes('HIDDEN_'),false);
    assert.equal(serialized.includes('unrelated-'),false);
    assert.equal(source.facts.apiKey,'HIDDEN_API');
});

test('seed application carries the matching controller-validated recipe without promoting approval',()=>{
    const current=observation(),recipe={version:2,scenarioId:'flux-dipole',size:97,blank:false,components:[],overrides:{amplitude:0.5}};
    const receipt={status:'applied',action:{type:'lattice.seed.preview',args:{recipeJson:JSON.stringify(recipe)}},
        before:current,after:current,result:{previewId:'preview-97',scenarioId:recipe.scenarioId,size:97}};
    const request=buildDecisionRequest('Apply the larger preparation.',current,plan('lattice.seed.apply',{previewId:'preview-97'}),{receipts:[receipt],scenarioCatalog:catalog});
    const evidence=request.observation.facts.preparedSeeds[0];
    assert.equal(evidence.available,true);assert.deepEqual(evidence.recipe,recipe);
    assert.equal(evidence.preparedAgainst.ownerId,current.ownerId);
    assert.equal(evidence.validation,'Recipe accepted for staging; not yet applied.');
    assert.equal(request.observation.facts.scenarioEvidence.targets.find(row=>row.scenarioId==='flux-dipole').registered,true);
    assert.equal(request.plan.actions[0].type,'lattice.seed.apply');
});

test('old, uncertain, replaced or mismatched preview evidence cannot authorize an apply target',()=>{
    const current=observation(),recipe={version:2,scenarioId:'flux-dipole',size:97};
    const preview=id=>({status:'applied',action:{type:'lattice.seed.preview',args:{recipeJson:JSON.stringify(recipe)}},
        before:current,after:current,result:{previewId:id,scenarioId:recipe.scenarioId,size:97}});
    const proposed=plan('lattice.seed.apply',{previewId:'wanted'});
    for(const receipts of [[],[{...preview('wanted'),status:'unknown'}],
        [{...preview('wanted'),after:{...current,ownerId:'other'}}],
        [{...preview('wanted'),after:{...current,preparationVersion:'old'}}],
        [preview('wanted'),preview('replacement')]]){
        const request=buildDecisionRequest('Apply staged recipe.',current,proposed,{receipts});
        assert.equal(request.observation.facts.preparedSeeds[0].available,false);
        assert.equal(request.observation.facts.preparedSeeds[0].recipe,undefined);
    }
    const bad=preview('wanted');bad.result.size=33;
    assert.throws(()=>buildDecisionRequest('Apply staged recipe.',current,proposed,{receipts:[bad]}),/metadata disagrees/);
});

test('required prepared recipe evidence is not silently redacted or truncated',()=>{
    const current=observation();
    const request=recipe=>buildDecisionRequest('Apply staged recipe.',current,plan('lattice.seed.apply',{previewId:'p'}),{receipts:[{
        status:'applied',action:{type:'lattice.seed.preview',args:{recipeJson:JSON.stringify(recipe)}},before:current,after:current,
        result:{previewId:'p',scenarioId:recipe.scenarioId,size:recipe.size}}]});
    assert.throws(()=>request({scenarioId:'flux-pulse',size:97,apiKey:'secret'}),/Required seed preview evidence/);
    assert.throws(()=>request({scenarioId:'flux-pulse',size:97,description:'x'.repeat(62000)}),/60 KiB decision budget/);
});

test('complete protocol recipes and authorization survive context construction unchanged',()=>{
    const recipe={version:2,scenarioId:'flux-dipole',size:33,blank:false,components:[],overrides:{amplitude:0.5}};
    const context={phase:'protocol',template:{scenarioId:'flux-dipole',settings:[{key:'amplitude',value:0.5,units:'simulation units'}]},
        preparation:recipe,emptyPreparation:{...recipe,blank:true},protocol:{ticks:512,sampleEvery:128,chunkTicks:128},
        authorization:{scope:'fixed-protocol',includesEmptyPreparation:true,finishPaused:true},
        operations:[{type:'lattice.seed.preview',recipeRef:'preparation'},{type:'lattice.step',totalTicks:512,maxChunkTicks:128}]};
    const request=buildDecisionRequest('Run the fixed protocol.',observation(),plan('lattice.seed.preview',{recipeJson:JSON.stringify(recipe)}),{scenarioCatalog:catalog,experimentContext:context});
    assert.deepEqual(request.observation.facts.scenarioExperiment,context);
    assert.deepEqual(request.observation.actions.map(row=>row.type),['lattice.step','lattice.seed.preview']);
    assert.equal(request.observation.facts.decisionScope.kind,'fixed-scenario-protocol');
    assert.notEqual(request.observation.facts.scenarioExperiment,context);
});

test('required evidence over the UTF-8 byte budget fails instead of truncating authorization',()=>{
    const source=observation(),proposed=plan('lattice.step',{count:32});
    const context={protocol:{ticks:32,sampleEvery:16},preparation:{scenarioId:'flux-pulse',description:'界'.repeat(22000)}};
    assert.throws(()=>buildDecisionRequest('Run the full preparation.',source,proposed,{experimentContext:context}),/60 KiB decision budget/);
    assert.equal(context.preparation.description.length,22000);
});

test('required protocol fields are rejected rather than silently redacted',()=>{
    const context={protocol:{ticks:32,sampleEvery:16},preparation:{scenarioId:'flux-pulse',apiKey:'private'}};
    assert.throws(()=>buildDecisionRequest('Run the complete protocol.',observation(),plan('lattice.step',{count:32}),{experimentContext:context}),/Required JEV plan or protocol/);
});

test('general compact observations mark their short scenario sample as incomplete',()=>{
    const received=decisionObservation(observation());
    assert.equal(received.facts.scenarios.length,8);
    assert.equal(received.facts.scenariosTruncated,true);
    assert.equal(received.actions,undefined);
});

function serviceFixture(){
    let current=observation();const requests=[];const actions=[];
    const control={observe:()=>structuredClone(current),listScenarioTemplates:()=>catalog,
        async execute(action,options){options.assertActive();options.signal.throwIfAborted();const before=structuredClone(current);actions.push(action);
            if(action.type==='lattice.step'){current.tick=String(BigInt(current.tick)+BigInt(action.args.count));current.facts.sampleTick=current.tick;}
            return{status:'applied',action,before,after:structuredClone(current)};}};
    const service=new AssistantService({getControl:()=>control,model:{cancel(){},async plan(){return plan('lattice.step',{count:32});}},knowledge:{},
        jev:{connected:true,async evaluate(request){requests.push(request);return{decision:'execute',confidence:1,model:'test'};}}});
    return{service,control,requests,actions};
}

test('MCP execute_plan uses target evidence and preserves a rejecting JEV decision',async()=>{
    const f=serviceFixture(),mcp=new McpControl(f.service);
    f.service.deps.jev.evaluate=async request=>{f.requests.push(request);return{decision:'reject',confidence:0.9,model:'test'};};
    const source=f.control.observe();
    const result=await mcp.handle('execute_plan',{intent:'Load flux-cascade.',expected:{workspace:source.workspace,ownerId:source.ownerId,preparationVersion:source.preparationVersion},
        actions:[{type:'lattice.scenario',args:{scenarioId:'flux-cascade'}}]},new AbortController().signal);
    assert.equal(result.status,'reject');assert.equal(f.requests.length,1);assert.equal(f.actions.length,0);
    assert.equal(f.requests[0].observation.facts.scenarioEvidence.targets.find(row=>row.scenarioId==='flux-cascade').registered,true);
});

test('ordinary assistant plans send the same authoritative step schema to JEV',async()=>{
    const f=serviceFixture();await f.service.submit('Advance exactly 32 ticks.');
    assert.equal(f.requests.length,1);assert.equal(f.actions.length,1);
    assert.equal(f.requests[0].observation.actions[0].args.properties.count.maximum,4096);
    assert.equal(f.requests[0].plan.actions[0].args.count,32);
});

test('complete preview evidence never overrides a rejecting JEV application decision',async()=>{
    const f=serviceFixture(),mcp=new McpControl(f.service),source=f.control.observe();
    const recipe={version:2,scenarioId:'flux-dipole',size:97,blank:false,components:[],overrides:{}};
    f.service.transcript.push({type:'receipt',receipt:{status:'applied',
        action:{type:'lattice.seed.preview',args:{recipeJson:JSON.stringify(recipe)}},
        before:source,after:source,result:{previewId:'p97',scenarioId:recipe.scenarioId,size:97}}});
    f.service.deps.jev.evaluate=async request=>{f.requests.push(request);return{decision:'reject',confidence:0.7,model:'test'};};
    const result=await mcp.handle('execute_plan',{intent:'Apply the larger lattice preview.',
        expected:{workspace:source.workspace,ownerId:source.ownerId,preparationVersion:source.preparationVersion},
        actions:[{type:'lattice.seed.apply',args:{previewId:'p97'}}]},new AbortController().signal);
    assert.equal(result.status,'reject');assert.deepEqual(result.receipts,[]);assert.equal(f.actions.length,0);
    assert.deepEqual(f.requests[0].observation.facts.preparedSeeds[0].recipe,recipe);
});
