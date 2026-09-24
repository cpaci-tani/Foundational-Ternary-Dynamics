import test from 'node:test';
import assert from 'node:assert/strict';
import {scenarioCandidates,scenarioSettingCandidates,chooseScenario,designScenario,explicitScenarioSettings} from '../js/assistant/scenario-designer.js';

const template={scenarioId:'wave',label:'Wave',backend:'effective-lattice',size:9,
    recipe:{version:2,scenarioId:'wave',size:9,blank:false,randomSeed:0,components:[],overrides:{}},
    properties:[{key:'packet.amplitude',path:['overrides','packet.amplitude'],type:'real',min:0,max:5,value:1,default:1,units:'sim flux',description:'Packet amplitude'}],allowedMeasurements:['manifested','totalFlux']};
const signal=()=>new AbortController().signal;

test('automatic scenario selection searches the complete catalog and excludes unsupported backend/size',()=>{
    const catalog=Array.from({length:150},(_,i)=>({id:`other-${i}`,label:'Other',category:'general',backend:'effective-lattice'}));
    catalog.push({id:'last',label:'Gaussian wave packet',category:'wave',backend:'effective-lattice',sizes:[9]});
    catalog.push({id:'wrong-size',label:'Gaussian wave packet',backend:'effective-lattice',sizes:[17]});
    catalog.push({id:'wrong-backend',label:'Gaussian wave packet',backend:'finite-records',sizes:[9]});
    const result=scenarioCandidates(catalog,'Gaussian wave packet',{facts:{backend:'Native',latticeSize:9}});
    assert.equal(result[0].id,'last');assert.equal(result.length,8);assert.ok(!result.some(row=>row.id.startsWith('wrong')));
});

test('model scenario IDs are constrained to actual retrieved templates',async()=>{
    const model={generate:async()=>JSON.stringify({scenarioId:'invented'})};
    await assert.rejects(chooseScenario(model,[{id:'wave',label:'Wave'}],'wave',signal()),/Unsupported/);
});

test('an explicit registry ID cannot be silently replaced with a similar template',()=>{
    const catalog=[{id:'wave-pulse',label:'Wave pulse',sizes:[9]},{id:'another-wave-pulse',label:'Wave pulse',sizes:[9]},{id:'small-wave',sizes:[3]}];
    const observation={facts:{backend:'Worker',latticeSize:9}};
    assert.deepEqual(scenarioCandidates(catalog,'Choose (wave-pulse) and measure it.',observation),[catalog[0]]);
    assert.deepEqual(scenarioCandidates(catalog,'Choose another-wave-pulse.',observation),[catalog[1]]);
    assert.throws(()=>scenarioCandidates(catalog,'Choose small-wave.',observation),/unavailable/);
    assert.throws(()=>scenarioCandidates(catalog,'Compare wave-pulse with another-wave-pulse.',observation),/ambiguous/);
    assert.throws(()=>scenarioCandidates(catalog,'Do not use wave-pulse; choose a similar template.',observation),/negated/);
});

test('LLM receives observed empty lattice and authoritative settings with a fixed user budget',async()=>{
    let context;
    const model={generate:async messages=>{context=JSON.parse(messages.at(-1).content);return JSON.stringify({name:'Weak wave',settings:[{key:'packet.amplitude',value:.5}],measurements:['totalFlux']});}};
    const empty={workspace:'lattice',ownerId:'empty-owner',tick:'0',facts:{totalFlux:0,manifested:'0'}};
    const draft=await designScenario(model,template,{goal:'Weak wave',ticks:100,sampleEvery:25},empty,signal());
    assert.deepEqual(context.emptyObservation,empty);assert.equal(context.settings[0].units,'sim flux');
    assert.deepEqual(context.settingsCoverage,{provided:1,total:1});assert.equal(draft.ticks,100);assert.equal(draft.sampleEvery,25);
    assert.deepEqual(draft.settings,[{key:'packet.amplitude',value:.5}]);assert.deepEqual(template.recipe.overrides,{});
});

test('invented settings and late cancelled designs cannot become drafts',async()=>{
    await assert.rejects(designScenario({generate:async()=>JSON.stringify({name:'Bad',settings:[{key:'law',value:1}],measurements:['totalFlux']})},template,{goal:'Wave',ticks:10,sampleEvery:10},{},signal()));
    const controller=new AbortController();
    await assert.rejects(designScenario({generate:async()=>{controller.abort();return '{}';}},template,{goal:'Wave',ticks:10,sampleEvery:10},{},controller.signal),{name:'AbortError'});
});

test('settings retrieval reports a bounded subset rather than exposing arbitrary writable keys',()=>{
    const many={...template,properties:Array.from({length:100},(_,i)=>({...template.properties[0],key:`parameter.${i}`,description:i===99?'special target':''}))};
    const found=scenarioSettingCandidates(many,'special target');assert.equal(found.length,24);assert.equal(found[0].key,'parameter.99');
    assert.equal(scenarioSettingCandidates(many,'Explore the scenario').length,24);
});

test('explicit scenario quantities constrain model values and reject ambiguity, unsupported units and omitted edits',async()=>{
    const property={key:'packet.amplitude',label:'Packet amplitude',type:'real',min:0,max:2,units:'lattice units',path:['overrides','packet.amplitude'],value:0.25};
    assert.deepEqual([...explicitScenarioSettings([property],'Set the pulse amplitude to 0.5.')],[['packet.amplitude',0.5]]);
    for(const goal of ['amplitude 0.5 or 1','amplitude 0.5 and amplitude 1','amplitude 3','amplitude 1 joule','amplitude 50%',
        'Do not set amplitude to 0.5.','amplitude 0.5 m.','amplitude 0.5 J.','amplitude 0.5kg','amplitude 0.5 widgets'])
        assert.throws(()=>explicitScenarioSettings([property],goal),undefined,goal);
    const phases=[{...property,key:'a.phase',label:'Phase'},{...property,key:'b.phase',label:'Phase'}];
    assert.throws(()=>explicitScenarioSettings(phases,'Set phase to 1.'),/ambiguous/);
    assert.deepEqual([...explicitScenarioSettings(phases,'Set a.phase to 1.')],[['a.phase',1]]);
    const t={...template,properties:[property]};
    const request={goal:'Set packet.amplitude to 0.5',ticks:4,sampleEvery:2};
    let schema;
    const model={generate:async(_messages,_signal,format)=>{schema=JSON.parse(format.schema);return JSON.stringify({name:'Pulse',settings:[{key:property.key,value:0.5}],measurements:['manifested']});}};
    const draft=await designScenario(model,t,request,{facts:{manifested:0}},signal());
    assert.equal(draft.settings[0].value,0.5);assert.deepEqual(schema.properties.settings.items.anyOf[0].properties.value.enum,[0.5]);
    model.generate=async()=>JSON.stringify({name:'Wrong pulse',settings:[{key:property.key,value:0.25}],measurements:['manifested']});
    await assert.rejects(designScenario(model,t,request,{facts:{}},signal()),/Invalid/);
});
