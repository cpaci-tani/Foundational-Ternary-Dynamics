import test from 'node:test';
import assert from 'node:assert/strict';
import {validateScenarioDraft,buildScenarioRecipe,buildEmptyScenarioRecipe} from '../js/assistant/scenario-template.js';
import {SCENARIO_LIMITS,SCENARIO_DEFAULTS} from '../js/assistant/scenario-limits.js';

const copy=value=>JSON.parse(JSON.stringify(value));
function native(){return {scenarioId:'test-wave',backend:'effective-lattice',size:33,
    recipe:{version:2,scenarioId:'test-wave',size:33,blank:false,randomSeed:0,components:[],overrides:{}},
    properties:[
        {key:'amplitude',path:['overrides','amplitude'],type:'real',min:0,max:10,units:'simulation units',value:1,default:1},
        {key:'count',path:['overrides','count'],type:'integer',min:1,max:64,value:2,default:2},
        {key:'protocol.boundary',path:['overrides','protocol.boundary'],type:'choice',min:0,max:1,options:[[0,'periodic'],[1,'reflective']],value:0,default:0},
    ],allowedMeasurements:['manifested','sampleTick','dynamicEnergy']};}
function finite(){return {scenarioId:'record-test',backend:'finite-records',size:3,
    recipe:{version:2,scenarioId:'record-test',size:3,blank:false,randomSeed:0,components:[
        {id:'field-0',kind:'field',enabled:true,region:{shape:'point',x:1,y:1,z:1,dx:1,dy:1,dz:1,radius:0},parameters:{amount:2,density:1}},
    ]},properties:[
        {key:'randomSeed',path:['randomSeed'],type:'integer',min:0,max:4294967295},
        {key:'amount',path:['components',0,'parameters','amount'],type:'integer',min:0,max:10},
        {key:'enabled',path:['components',0,'enabled'],type:'choice',min:0,max:1,options:[[false,'off'],[true,'on']]},
        {key:'shape',path:['components',0,'region','shape'],type:'choice',min:0,max:1,options:[['point','point'],['box','box']]},
        {key:'dx',path:['components',0,'region','dx'],type:'integer',min:1,max:3},
    ],allowedMeasurements:['manifested','fieldTokens','incidence']};}
function draft(extra={}){return {name:'Small change',goal:'Record the current constructor response.',settings:[],ticks:100,sampleEvery:10,measurements:['manifested'],...extra};}

test('shared native preparation is immutable, serializable and does not mutate compiler defaults',()=>{
    const source=native(),before=copy(source),input=draft({settings:[{key:'amplitude',value:2.5},{key:'protocol.boundary',value:1}]});
    const result=buildScenarioRecipe(source,input);
    assert.deepEqual(source,before);assert.equal(result.recipe.overrides.amplitude,2.5);
    assert.deepEqual(result.settings,[{key:'amplitude',value:2.5,units:'simulation units'},{key:'protocol.boundary',value:1,units:''}]);
    assert.deepEqual(result.protocol,{ticks:100,sampleEvery:10});assert.equal(result.start,'empty');assert.equal(result.schemaVersion,1);
    assert.equal(result.backend,'effective-lattice');assert.equal(result.size,33);assert.equal(result.scenarioId,'test-wave');
    assert.deepEqual(JSON.parse(JSON.stringify(result)),result);
    assert.ok(Object.isFrozen(result) && Object.isFrozen(result.recipe) && Object.isFrozen(result.settings[0]) && Object.isFrozen(result.protocol));
    input.settings[0].value=9;assert.equal(result.settings[0].value,2.5);
    assert.throws(()=>{result.recipe.overrides.amplitude=7;},TypeError);
});

test('finite scalar edits preserve component identity and explicit-override provenance',()=>{
    const source=finite(),before=copy(source),result=buildScenarioRecipe(source,draft({settings:[{key:'amount',value:7},{key:'enabled',value:false},{key:'randomSeed',value:4294967295}]}));
    assert.deepEqual(source,before);assert.equal(result.recipe.components[0].parameters.amount,7);
    assert.equal(result.recipe.components[0].enabled,false);assert.equal(result.recipe.randomSeed,4294967295);
    assert.ok(result.recipe.explicitOverrides.includes('["component","field-0","parameters","amount"]'));
    assert.ok(result.recipe.explicitOverrides.includes('["randomSeed"]'));
});

test('native blank start has an inactive same-scenario anchor and no inherited overrides',()=>{
    const source=native();source.recipe.overrides={amplitude:9};
    const empty=buildEmptyScenarioRecipe(source);
    assert.equal(empty.blank,true);assert.equal(empty.size,33);assert.equal(empty.scenarioId,source.scenarioId);
    assert.deepEqual(empty.overrides,{});assert.equal(empty.components.length,1);
    assert.deepEqual(empty.components[0],{id:'assistant-empty-anchor',kind:'native',enabled:false,scenarioId:'test-wave',overrides:{}});
    assert.equal(source.recipe.blank,false);assert.equal(source.recipe.overrides.amplitude,9);assert.ok(Object.isFrozen(empty.components[0]));
});

test('blank starts disable every finite or native ingredient without changing size',()=>{
    const records=finite();records.recipe.explicitOverrides=['["randomSeed"]'];
    const empty=buildEmptyScenarioRecipe(records);
    assert.equal(empty.blank,true);assert.equal(empty.components[0].enabled,false);assert.equal(empty.size,3);
    assert.equal(empty.components[0].id,'field-0');assert.equal(empty.components[0].parameters.amount,2);assert.equal(empty.explicitOverrides,undefined);
    const source=native();source.recipe.blank=true;source.recipe.components=[{id:'anchor',kind:'native',enabled:true,scenarioId:source.scenarioId,overrides:{amplitude:7}}];
    const nativeEmpty=buildEmptyScenarioRecipe(source);assert.equal(nativeEmpty.components.length,1);
    assert.deepEqual(nativeEmpty.components[0],{id:'anchor',kind:'native',enabled:false,scenarioId:source.scenarioId,overrides:{}});
});

test('draft and setting keys are closed; model cannot supply a recipe, backend or executable field',()=>{
    for(const key of ['recipe','backend','size','scenarioId','code','script','imports'])assert.throws(()=>validateScenarioDraft({...draft(),[key]:'anything'},native()),/unexpected or missing/);
    const missing=draft();delete missing.goal;assert.throws(()=>validateScenarioDraft(missing,native()),/unexpected or missing/);
    assert.throws(()=>validateScenarioDraft(draft({settings:[{key:'amplitude',value:1,path:['size']}]}),native()),/unexpected or missing/);
    assert.throws(()=>validateScenarioDraft(draft({settings:[{key:'unknown',value:1}]}),native()),/Unknown setting/);
});

test('prototype keys, inherited objects, getters, cycles and non-JSON values are rejected without executing them',()=>{
    for(const key of ['__proto__','constructor','prototype']){
        const input=JSON.parse(JSON.stringify(draft()).slice(0,-1)+`,"${key}":{}}`);
        assert.throws(()=>validateScenarioDraft(input,native()),/forbidden property/);
    }
    let invoked=false;const getter=draft();Object.defineProperty(getter,'goal',{get(){invoked=true;return 'bad';},enumerable:true});
    assert.throws(()=>validateScenarioDraft(getter,native()),/accessors/);assert.equal(invoked,false);
    assert.throws(()=>validateScenarioDraft(Object.assign(Object.create({inherited:true}),draft()),native()),/plain objects/);
    const cyclic=draft();cyclic.settings.push(cyclic);assert.throws(()=>validateScenarioDraft(cyclic,native()),/circular/);
    for(const value of [()=>{},undefined,1n,Symbol('bad'),new Date(),new Map(),NaN,Infinity]){
        assert.throws(()=>validateScenarioDraft(draft({settings:[{key:'amplitude',value}]}),native()));
    }
});

test('property paths cannot rewrite domain identity, replace structures or reach prototypes',()=>{
    for(const path of [['size'],['scenarioId'],['backend'],['version'],['blank'],['components'],['overrides','__proto__'],['overrides','constructor.x'],['overrides','recipe.blank'],['overrides','ingredient.0.scenario']]){
        const source=native();source.properties[0].path=path;
        assert.throws(()=>buildScenarioRecipe(source,draft({settings:[{key:'amplitude',value:1}]})),/path|identity or structure/);
    }
    const source=finite();source.properties[1].path=['components',0,'id'];
    assert.throws(()=>validateScenarioDraft(draft({settings:[{key:'amount',value:1}]}),source),/identity or structure/);
    source.properties[1].path=['components',0,'parameters','missing'];
    assert.throws(()=>validateScenarioDraft(draft({settings:[{key:'amount',value:1}]}),source),/existing scalar/);
});

test('real, integer and choice validation use the actual descriptor domains',()=>{
    for(const value of [0,10,1.25])assert.doesNotThrow(()=>validateScenarioDraft(draft({settings:[{key:'amplitude',value}]}),native()));
    for(const value of [-1,10.1,'2.5','(()=>1)()'])assert.throws(()=>validateScenarioDraft(draft({settings:[{key:'amplitude',value}]}),native()));
    for(const value of [1.5,0,65,Number.MAX_SAFE_INTEGER+1])assert.throws(()=>validateScenarioDraft(draft({settings:[{key:'count',value}]}),native()));
    for(const value of [2,true,'1'])assert.throws(()=>validateScenarioDraft(draft({settings:[{key:'protocol.boundary',value}]}),native()),/admitted states/);
    assert.throws(()=>validateScenarioDraft(draft({settings:[{key:'amplitude',value:{code:'run()'}}]}),native()),/finite scalar/);
    assert.throws(()=>validateScenarioDraft(draft({settings:[{key:'enabled',value:1}]}),finite()),/boolean/);
});

test('sampling is bounded by exact tick limits and includes baseline plus a final partial interval',()=>{
    for(const [ticks,sampleEvery] of [[10000,250],[50000,250],[100000,500],[100000,503]]){
        const valid=validateScenarioDraft(draft({ticks,sampleEvery}),native());
        assert.equal(valid.ticks,ticks);assert.equal(valid.sampleEvery,sampleEvery);
    }
    assert.deepEqual(SCENARIO_DEFAULTS,{ticks:10000,sampleEvery:250});
    assert.equal(SCENARIO_LIMITS.durationMs,1800000);assert.equal(SCENARIO_LIMITS.chunkTicks,128);
    assert.doesNotThrow(()=>validateScenarioDraft(draft({ticks:10,sampleEvery:3}),native()));
    assert.doesNotThrow(()=>validateScenarioDraft(draft({ticks:1,sampleEvery:1}),native()));
    for(const [ticks,sampleEvery] of [[100000,499],[50000,249],[10000,49]])
        assert.throws(()=>validateScenarioDraft(draft({ticks,sampleEvery}),native()),/200 measured intervals plus the baseline/);
    for(const [ticks,sampleEvery] of [[0,1],[100001,501],[10,0],[10,11],[1.5,1],[10,1.5],[10,'1'],[Number.MAX_SAFE_INTEGER+1,1]])
        assert.throws(()=>validateScenarioDraft(draft({ticks,sampleEvery}),native()),/exact integer/);
});

test('measurement names must be unique admitted fields with no code or unknown channels',()=>{
    for(const measurements of [[],['unknown'],['manifested','manifested'],['manifested','constructor'],['(()=>run())()'],[{}]])
        assert.throws(()=>validateScenarioDraft(draft({measurements}),native()),/template allowlist/);
    const valid=validateScenarioDraft(draft({measurements:['sampleTick','dynamicEnergy']}),native());
    assert.deepEqual(valid.measurements,['sampleTick','dynamicEnergy']);assert.ok(Object.isFrozen(valid.measurements));
});

test('at most twelve distinct descriptor settings can be selected',()=>{
    const source=native();source.properties=Array.from({length:13},(_,i)=>({key:`value${i}`,path:['overrides',`value${i}`],type:'real',min:0,max:10}));
    const settings=source.properties.map(p=>({key:p.key,value:1}));
    assert.doesNotThrow(()=>validateScenarioDraft(draft({settings:settings.slice(0,12)}),source));
    assert.throws(()=>validateScenarioDraft(draft({settings}),source),/12 changed settings/);
    assert.throws(()=>validateScenarioDraft(draft({settings:[settings[0],settings[0]]}),source),/unique strings/);
    source.properties[1].path=source.properties[0].path;
    assert.throws(()=>validateScenarioDraft(draft({settings:settings.slice(0,2)}),source),/same property twice/);
});

test('malformed template identity, bounds, measurement allowlists and hidden content fail closed',()=>{
    const mismatch=native();mismatch.size=65;assert.throws(()=>buildEmptyScenarioRecipe(mismatch),/identity or size/);
    const backend=native();backend.backend='custom';assert.throws(()=>validateScenarioDraft(draft(),backend),/backend/);
    const duplicate=native();duplicate.properties.push(copy(duplicate.properties[0]));assert.throws(()=>validateScenarioDraft(draft(),duplicate),/unique/);
    const dangerous=native();dangerous.properties[0].key='constructor';assert.throws(()=>validateScenarioDraft(draft(),dangerous),/safe/);
    const unbounded=native();delete unbounded.properties[0].max;assert.throws(()=>validateScenarioDraft(draft({settings:[{key:'amplitude',value:1}]}),unbounded),/bounds or type/);
    const allowlist=native();allowlist.allowedMeasurements=['constructor'];assert.throws(()=>validateScenarioDraft(draft(),allowlist),/allowlist/);
    const hidden=native();hidden.run=()=>{};assert.throws(()=>buildScenarioRecipe(hidden,draft()),/plain JSON data/);
});

test('final recipe validation catches coupled structural bounds before returning prepared JSON',()=>{
    assert.throws(()=>buildScenarioRecipe(finite(),draft({settings:[{key:'shape',value:'box'},{key:'dx',value:3}]})),/Box extends/);
});
