import test from 'node:test';
import assert from 'node:assert/strict';
import {McpControl} from '../js/assistant/mcp-control.js';
import {AssistantService} from '../js/assistant/service.js';
import {assertObservation} from '../js/assistant/contracts.js';

const signal=()=>new AbortController().signal;
function fixture(){
    let tick=0,version=0;const writes=[],events=[];
    const observe=()=>({workspace:'lattice',ownerId:'single-owner',preparationVersion:String(version),tick,facts:{running:false,sampleTick:tick},capabilities:['lattice.step'],actions:[{type:'lattice.step',args:{type:'object',properties:{count:{type:'integer',minimum:1,maximum:4096}},required:['count'],additionalProperties:false}}]});
    const control={observe,async execute(action,options){options.assertActive();assertObservation(observe(),options.expected);writes.push(action);tick+=action.args.count;return{status:'applied',action,after:observe()};},listScenarioTemplates:()=>Array.from({length:77},(_,i)=>({id:`scenario-${i}`,label:`Geometry ${i}`}))};
    const service=new AssistantService({getControl:()=>control,model:{cancel(){},load(){assert.fail('MCP must not load a model');}},jev:{connected:true,evaluate:async()=>({decision:'execute',confidence:1,model:'test'})},knowledge:{search:async()=>[{sourcePath:'source.md',statusTags:['[OPEN]']}],coverage:{complete:true}},onEvent:event=>events.push(event)});
    const mcp=new McpControl(service);
    const request=()=>({intent:'Advance two ticks',expected:{workspace:'lattice',ownerId:'single-owner',preparationVersion:String(version)},actions:[{type:'lattice.step',args:{count:2}}]});
    return{mcp,service,control,observe,writes,events,request,edit:()=>++version,tick:()=>++tick};
}

test('MCP exposes dynamic capabilities, full paginated catalog and attributed sources without a model',async()=>{
    const f=fixture();const state=await f.mcp.handle('observe',{},signal());assert.equal(state.observation.actions[0].type,'lattice.step');
    const catalog=await f.mcp.handle('list_scenarios',{limit:50},signal());assert.equal(catalog.total,77);assert.equal(catalog.items.length,50);assert.equal(catalog.nextOffset,50);
    const second=await f.mcp.handle('list_scenarios',{offset:50,limit:50},signal());assert.equal(second.items.length,27);assert.equal(second.nextOffset,null);
    const docs=await f.mcp.handle('search_docs',{query:'lattice'},signal());assert.equal(docs.sources[0].statusTags[0],'[OPEN]');assert.equal(f.writes.length,0);
});

test('MCP flux sectors is a fenced read available without JEV and after writes stop',async()=>{
    const f=fixture();let measured=0;
    f.service.deps.jev.connected=false;f.mcp.stop();
    f.control.measureFluxSectors=async(args,readSignal)=>{readSignal.throwIfAborted();measured++;return{status:'measured',tick:args.expectedTick};};
    const args={expected:f.request().expected,expectedTick:'0'};
    assert.equal((await f.mcp.handle('measure_flux_sectors',args,signal())).status,'measured');
    assert.equal(measured,1);assert.equal(f.writes.length,0);
    f.edit();await assert.rejects(f.mcp.handle('measure_flux_sectors',args,signal()),/world changed/);
    assert.equal(measured,1);
});

test('MCP validates schemas and JEV connectivity before mutation',async()=>{
    const f=fixture();
    await assert.rejects(f.mcp.handle('execute_plan',{...f.request(),actions:[{type:'eval',args:{code:'x'}}]},signal()),/Unavailable/);
    await assert.rejects(f.mcp.handle('execute_plan',{...f.request(),actions:[{type:'lattice.step',args:{count:NaN}}]},signal()),/Invalid/);
    await assert.rejects(f.mcp.handle('execute_plan',{...f.request(),unexpected:true},signal()),/Unsupported/);
    f.service.deps.jev.connected=false;await assert.rejects(f.mcp.handle('execute_plan',f.request(),signal()),/Connect JEV/);
    assert.equal(f.writes.length,0);
});

test('ordinary tick progression stays valid, author edits during JEV invalidate the plan',async()=>{
    const f=fixture();f.service.deps.jev.evaluate=async()=>{f.tick();return{decision:'execute'};};
    assert.equal((await f.mcp.handle('execute_plan',f.request(),signal())).status,'applied');assert.equal(f.observe().tick,3);
    f.service.deps.jev.evaluate=async()=>{f.edit();return{decision:'execute'};};
    const rejected=await f.mcp.handle('execute_plan',f.request(),signal());assert.equal(rejected.status,'failed');assert.match(rejected.error,/world changed/);assert.equal(f.writes.length,1);
});

test('JEV rejection and wrong owners never write',async()=>{
    const f=fixture();f.service.deps.jev.evaluate=async()=>({decision:'reject'});
    assert.equal((await f.mcp.handle('execute_plan',f.request(),signal())).status,'reject');
    const request=f.request();request.expected.ownerId='another-tab';await assert.rejects(f.mcp.handle('execute_plan',request,signal()),/world changed/);assert.equal(f.writes.length,0);
});

test('MCP serializes mutations across clients and cancellation fences late JEV approval',async()=>{
    const f=fixture();let approve;f.service.deps.jev.evaluate=()=>new Promise(resolve=>{approve=resolve;});
    const controller=new AbortController(),pending=f.mcp.handle('execute_plan',f.request(),controller.signal);
    await assert.rejects(f.mcp.handle('execute_plan',f.request(),signal()),/active/);
    controller.abort();approve({decision:'execute'});assert.equal((await pending).status,'stopped');assert.equal(f.writes.length,0);
});

test('Stop preserves an acknowledged first action and blocks the rest and future MCP writes',async()=>{
    const f=fixture(),execute=f.control.execute;
    f.control.execute=async(...args)=>{const receipt=await execute(...args);f.mcp.stop();return receipt;};
    const request=f.request();request.actions.push(...request.actions);
    const result=await f.mcp.handle('execute_plan',request,signal());assert.equal(result.status,'stopped');assert.equal(result.receipts.length,1);assert.equal(f.writes.length,1);
    await assert.rejects(f.mcp.handle('execute_plan',f.request(),signal()),/stopped/);
    assert.equal((await f.mcp.handle('observe',{},signal())).writesStopped,true);
});

test('missing command acknowledgment is an unknown outcome, not an automatic retry',async()=>{
    const f=fixture();f.control.execute=async()=>{throw new Error('Worker disconnected after dispatch');};
    const result=await f.mcp.handle('execute_plan',f.request(),signal());assert.equal(result.status,'unknown');assert.equal(result.receipts.length,0);
});

test('scenario requests pass external data and authority into the bounded runner without an embedded model',async()=>{
    const f=fixture();let received,finish;
    f.service.runScenarioExperiment=request=>{received=request;return new Promise(resolve=>{finish=resolve;});};
    const draft={name:'Double pulse',goal:'Measure pulse',ticks:10,sampleEvery:5,settings:[{key:'amplitude',value:.5}],measurements:['totalFlux']};
    const result=await f.mcp.handle('run_scenario',{scenarioId:'flux-pulse',goal:draft.goal,ticks:10,sampleEvery:5,draft,expected:f.request().expected},signal());
    assert.equal(result.status,'running');assert.equal(received.useLLM,false);assert.deepEqual(received.draft,draft);
    finish({status:'complete',samples:[{tick:0},{tick:5},{tick:10}]});await new Promise(resolve=>setTimeout(resolve,0));
    const completed=await f.mcp.handle('run_status',{runId:result.runId},signal());assert.equal(completed.result.samples.length,3);
});

test('MCP fixed-protocol bounds accept long runs and reject oversized or oversampled requests before execution',async()=>{
    for(const [ticks,sampleEvery] of [[10000,250],[50000,250],[100000,500]]){
        const f=fixture();let received;
        f.service.runScenarioExperiment=async request=>{received=request;return{status:'complete',samples:[]};};
        const result=await f.mcp.handle('run_scenario',{scenarioId:'flux-pulse',goal:'Measure a long run.',ticks,sampleEvery,expected:f.request().expected},signal());
        assert.ok(result.runId);assert.equal(received.ticks,ticks);assert.equal(received.sampleEvery,sampleEvery);
    }
    for(const [ticks,sampleEvery] of [[100001,501],[100000,499],[50000,249],[10000,49]]){
        const f=fixture();let starts=0;f.service.runScenarioExperiment=async()=>{starts++;};
        await assert.rejects(f.mcp.handle('run_scenario',{scenarioId:'flux-pulse',goal:'Measure a long run.',ticks,sampleEvery,expected:f.request().expected},signal()),/Invalid request.ticks|200 measured intervals/);
        assert.equal(starts,0);assert.equal(f.writes.length,0);
    }
});
