import test from 'node:test';
import assert from 'node:assert/strict';
import {WebSocketBridge} from '../js/ws-bridge.js';

function fixture() {
    const calls=[], accepted=[];
    const description={properties:[{key:'packet.amplitude',value:.6}]};
    const bridge={seedRecipeVersion:2,_nativeInstanceId:'server-1',
        async _sendJSON(command) {calls.push(command);return {ok:true,sourceEpoch:'17',nativeInstanceId:'server-1',description};},
        async _sendOperationWithRetry(command) {calls.push(command);return {ok:true,latticeSize:17,fluxBoundaryMode:0,fluxPeriodicAxis:2};},
        _requireSuccessfulResponse(response) {if(response.error)throw new Error(response.error);return response;},
        _acceptScenarioResponse(...args) {accepted.push(args);},
    };
    const recipe={scenarioId:'flux-pulse',size:17,overrides:{'packet.amplitude':.6}};
    return {bridge,calls,accepted,recipe,description,
        prepare:signal=>WebSocketBridge.prototype.prepareScenarioSeed.call(bridge,recipe,signal)};
}

test('WebSocket preview is detached and successful commit installs once with its original epoch and property snapshot',async()=>{
    const f=fixture(), preview=await f.prepare();
    assert.equal(f.accepted.length,0);
    f.recipe.overrides['packet.amplitude']=.9;
    assert.equal(await preview.commit(),f.bridge);
    assert.equal(f.calls[1].expectedSourceEpoch,'17');
    assert.deepEqual(f.calls[1].overrides,{'packet.amplitude':.6});
    assert.equal(f.accepted.length,1);
    assert.equal(f.bridge.seedDescription,f.description);
    await assert.rejects(()=>preview.commit(),/no longer current/);
});

test('cancelled, disposed or replaced-server previews never dispatch a commit',async()=>{
    for(const mode of ['abort','dispose','restart']) {
        const f=fixture(), controller=new AbortController(), preview=await f.prepare(controller.signal);
        if(mode==='abort')controller.abort();
        if(mode==='dispose')preview.dispose();
        if(mode==='restart')f.bridge._nativeInstanceId='server-2';
        await assert.rejects(()=>preview.commit());
        assert.equal(f.calls.length,1);assert.equal(f.accepted.length,0);
    }
});

test('native rejection preserves the accepted owner and its prior recipe description',async()=>{
    const f=fixture(), previous={properties:[]};f.bridge.seedDescription=previous;
    const preview=await f.prepare();
    f.bridge._sendOperationWithRetry=async()=>({error:'source epoch changed'});
    await assert.rejects(()=>preview.commit(),/source epoch changed/);
    assert.equal(f.accepted.length,0);assert.equal(f.bridge.seedDescription,previous);
    f.bridge.seedRecipeVersion=1;
    await assert.rejects(()=>f.prepare(),/does not advertise/);
});
