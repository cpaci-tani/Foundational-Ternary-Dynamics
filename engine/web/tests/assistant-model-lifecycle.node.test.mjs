import test from 'node:test';
import assert from 'node:assert/strict';
import {BrowserModel} from '../js/assistant/model.js';

function deferred(){let resolve,reject;const promise=new Promise((yes,no)=>{resolve=yes;reject=no;});return {promise,resolve,reject};}
const stream=text=>(async function*(){yield {choices:[{delta:{content:text}}]};})();
function runtime(model,pending){
    const worker={terminated:0,terminate(){this.terminated++;}};
    const engine={interruptGenerate(){},unload(){throw new Error('Unload must not wait for a worker RPC');},chat:{completions:{create:()=>pending.promise}}};
    Object.assign(model,{worker,engine,ready:true});return {worker,engine};
}
function replaceGlobal(t,key,value){const descriptor=Object.getOwnPropertyDescriptor(globalThis,key);Object.defineProperty(globalThis,key,{configurable:true,writable:true,value});t.after(()=>{if(descriptor)Object.defineProperty(globalThis,key,descriptor);else delete globalThis[key];});}

test('unload retires stalled generation immediately and stale settlement cannot release a new generation',async()=>{
    const activity=[];const model=new BrowserModel({modelBase:'',onActivity:value=>activity.push(value)});
    const oldPending=deferred();const old=runtime(model,oldPending);
    const first=model.generate([],new AbortController().signal);
    const stopped=assert.rejects(first,/Model unloaded/);
    await model.unload();
    assert.equal(old.worker.terminated,1);assert.equal(model.busy,false);
    const newPending=deferred();const replacement=runtime(model,newPending);
    const second=model.generate([],new AbortController().signal);
    await stopped;oldPending.resolve(stream('stale'));
    await Promise.resolve();await Promise.resolve();
    assert.equal(model.engine,replacement.engine);assert.equal(model.worker,replacement.worker);
    assert.equal(model.busy,true);assert.equal(replacement.worker.terminated,0);
    assert.deepEqual(activity,[true,false,true]);
    newPending.resolve(stream('current'));assert.equal(await second,'current');
    assert.deepEqual(activity,[true,false,true,false]);
});

test('a retired generation timeout cannot terminate a replacement worker',async t=>{
    const timers=[];replaceGlobal(t,'setTimeout',callback=>{timers.push(callback);return timers.length;});replaceGlobal(t,'clearTimeout',()=>{});
    const statuses=[];const model=new BrowserModel({modelBase:'',onStatus:value=>statuses.push(value.text)});
    runtime(model,deferred());const first=model.generate([],new AbortController().signal);
    const stopped=assert.rejects(first,/Model unloaded/);await model.unload();await stopped;
    const replacement=runtime(model,deferred());const count=statuses.length;
    timers[0]();
    assert.equal(replacement.worker.terminated,0);assert.equal(model.engine,replacement.engine);
    assert.equal(model.ready,true);assert.equal(statuses.length,count);
});

test('generation deadline terminates a stalled worker and settles the active request',async t=>{
    const timers=[];replaceGlobal(t,'setTimeout',callback=>{timers.push(callback);return timers.length;});replaceGlobal(t,'clearTimeout',()=>{});
    const activity=[];const model=new BrowserModel({modelBase:'',onActivity:value=>activity.push(value)});
    const old=runtime(model,deferred());const pending=model.generate([],new AbortController().signal);
    const failed=assert.rejects(pending,/exceeded 60 seconds/);timers[0]();await failed;
    assert.equal(old.worker.terminated,1);assert.equal(model.ready,false);assert.equal(model.busy,false);
    assert.equal(model.engine,null);assert.deepEqual(activity,[true,false]);
});

test('load deadline also bounds WebGPU adapter discovery and ignores its late completion',async t=>{
    const timers=[];replaceGlobal(t,'setTimeout',callback=>{timers.push(callback);return timers.length;});replaceGlobal(t,'clearTimeout',()=>{});
    const adapter=deferred();replaceGlobal(t,'isSecureContext',true);replaceGlobal(t,'navigator',{gpu:{requestAdapter:()=>adapter.promise}});
    let requests=0;replaceGlobal(t,'fetch',()=>{requests++;throw new Error('Late load must not fetch');});
    const model=new BrowserModel({modelBase:''});const loading=model.load();const failed=assert.rejects(loading,/exceeded 90 seconds/);
    timers[0]();await failed;assert.equal(model.loading,null);assert.equal(model.ready,false);
    adapter.resolve({features:new Set()});await Promise.resolve();await Promise.resolve();assert.equal(requests,0);
});

test('unloading a pending load permits reload without stale cleanup touching its controller or status',async t=>{
    const adapters=[deferred(),deferred()];let next=0;
    replaceGlobal(t,'isSecureContext',true);replaceGlobal(t,'navigator',{gpu:{requestAdapter:()=>adapters[next++].promise}});
    const statuses=[];const model=new BrowserModel({modelBase:'',onStatus:value=>statuses.push(value.text)});
    const first=model.load();const stopped=assert.rejects(first,/Model unloaded/);await model.unload();
    const second=model.load();const controller=model.loading;const count=statuses.length;
    await stopped;adapters[0].resolve(null);await Promise.resolve();await Promise.resolve();
    assert.equal(model.loading,controller);assert.equal(statuses.length,count);
    const cancelled=assert.rejects(second,/Model loading cancelled/);model.cancel();await cancelled;
});

test('cache removal excludes load and duplicate removals until deletion settles',async t=>{
    const deletion=deferred();const started=deferred();
    replaceGlobal(t,'document',{baseURI:'https://example.test/'});
    replaceGlobal(t,'fetch',async()=>({ok:true,json:async()=>({variants:[{id:'one',modelPath:'one/',modelLibPath:'one.wasm'}]})}));
    const model=new BrowserModel({modelBase:'/models/'});
    model.runtime={deleteModelAllInfoInCache:async()=>{started.resolve();await deletion.promise;}};
    const clearing=model.clearCache();await started.promise;
    await assert.rejects(model.load(),/cache removal is still running/);
    await assert.rejects(model.clearCache(),/cache removal is already running/);
    deletion.resolve();await clearing;assert.equal(model.clearing,false);
    model.ready=true;await model.load();
});

test('dispose settles stalled generation and forbids later loading',async()=>{
    const model=new BrowserModel({modelBase:''});const old=runtime(model,deferred());
    const pending=model.generate([],new AbortController().signal);const stopped=assert.rejects(pending,/Model disposed/);
    model.dispose();await stopped;assert.equal(old.worker.terminated,1);assert.equal(model.busy,false);
    await assert.rejects(model.load(),/disposed/);await assert.rejects(model.clearCache(),/disposed/);
});
