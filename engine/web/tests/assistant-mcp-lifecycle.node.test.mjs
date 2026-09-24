import test from 'node:test';
import assert from 'node:assert/strict';
import {McpBridge} from '../js/assistant/mcp-bridge.js';

function deferred(){
    let resolve;
    const promise=new Promise(done=>{resolve=done;});
    return {promise,resolve};
}
const turn=()=>new Promise(resolve=>setImmediate(resolve));
const response=value=>({ok:true,json:async()=>value});

function fixture(t){
    const original=Object.getOwnPropertyDescriptor(globalThis,'location');
    Object.defineProperty(globalThis,'location',{configurable:true,value:{hostname:'localhost',origin:'http://localhost:8080'}});
    const f={registrations:[],polls:[],revoked:[],replies:[],active:new Set(),statuses:[]};
    const service={run:null,stop(){},deps:{model:{cancel(){}}}};
    const fetchImpl=async(path,options)=>{
        const body=JSON.parse(options.body);
        if(path.endsWith('/register')){
            // A relay can accept registration before cancellation reaches it.
            // Delay its response independently of the caller's aborted signal.
            const pending=deferred(),id=`session-${f.registrations.length+1}`;
            const session={sessionId:id,browserToken:`browser-${id}`,clientToken:`client-${id}`,serverPath:'server.mjs'};
            f.active.add(id);f.registrations.push({session,signal:options.signal,release:()=>pending.resolve(response(session))});
            return pending.promise;
        }
        options.signal?.throwIfAborted();
        if(path.endsWith('/poll')){
            f.polls.push(body.sessionId);
            return new Promise((resolve,reject)=>options.signal.addEventListener('abort',()=>reject(options.signal.reason),{once:true}));
        }
        if(path.endsWith('/disconnect')){f.revoked.push(body.sessionId);f.active.delete(body.sessionId);return response({});}
        if(path.endsWith('/reply')){f.replies.push(body);return response({});}
        throw new Error(`Unexpected relay path ${path}`);
    };
    f.bridge=new McpBridge({service,fetchImpl,onStatus:status=>f.statuses.push(status)});
    t.after(async()=>{
        await f.bridge.disconnect();
        if(original)Object.defineProperty(globalThis,'location',original);else delete globalThis.location;
    });
    return f;
}

test('overlapping Enable calls produce one paired session and Disconnect revokes it',async t=>{
    const f=fixture(t);
    const first=f.bridge.connect(),second=f.bridge.connect();
    await turn();
    assert.equal(f.registrations.length,1,'A superseded Enable must not register another live session');
    f.registrations[0].release();await Promise.all([first,second]);
    assert.deepEqual(f.polls,['session-1']);
    assert.equal(f.statuses.filter(status=>status.connected).length,1);
    await f.bridge.disconnect();
    assert.equal(f.active.size,0,'Disconnect must revoke every session created by these Enable calls');
});

test('a late superseded registration is revoked without replacing the current pairing',async t=>{
    const f=fixture(t);
    const first=f.bridge.connect();await turn();
    const second=f.bridge.connect();await turn();
    assert.equal(f.registrations.length,2);
    assert.equal(f.registrations[0].signal.aborted,true);
    f.registrations[1].release();await second;
    f.registrations[0].release();await first;
    assert.equal(f.bridge.session.sessionId,'session-2');
    assert.deepEqual(f.polls,['session-2']);
    assert.deepEqual(f.revoked,['session-1']);
    assert.deepEqual([...f.active],['session-2']);
    await f.bridge.disconnect();assert.equal(f.active.size,0);
});

test('disposing while Enable yields prevents a new registration',async t=>{
    const f=fixture(t);
    const connecting=f.bridge.connect();
    f.bridge.dispose();await connecting;
    assert.equal(f.registrations.length,0);
    assert.equal(f.bridge.connection,null);
    assert.equal(f.bridge.session,null);
    assert.equal(f.statuses.some(status=>status.connected),false);
});

test('disposing during registration revokes its late response and never starts polling',async t=>{
    const f=fixture(t);
    const connecting=f.bridge.connect();await turn();
    assert.equal(f.registrations.length,1);
    f.bridge.dispose();f.registrations[0].release();await connecting;
    assert.equal(f.active.size,0);
    assert.deepEqual(f.revoked,['session-1']);
    assert.deepEqual(f.polls,[]);
    assert.equal(f.bridge.session,null);
    assert.equal(f.statuses.some(status=>status.connected),false);
});

test('old request cleanup preserves cancellation of the same id in a new connection',async t=>{
    const f=fixture(t),oldResult=deferred(),newResult=deferred(),signals=[];
    f.bridge.control.handle=async(method,args,signal)=>{
        signals.push(signal);
        return signals.length===1?oldResult.promise:newResult.promise;
    };
    const request={id:'reused-request',method:'observe',args:{},deadline:Date.now()+60000};
    const oldConnection=new AbortController(),oldSession={sessionId:'old',browserToken:'old'};
    f.bridge.connection=oldConnection;f.bridge.session=oldSession;
    const oldDispatch=f.bridge.dispatch(request,oldConnection,oldSession);
    await f.bridge.disconnect();assert.equal(signals[0].aborted,true);
    const newConnection=new AbortController(),newSession={sessionId:'new',browserToken:'new'};
    f.bridge.connection=newConnection;f.bridge.session=newSession;f.bridge.control.rearm();
    const newDispatch=f.bridge.dispatch(request,newConnection,newSession);
    oldResult.resolve({status:'old'});await oldDispatch;
    const current=f.bridge.pending.get(request.id);
    assert.ok(current,'Old completion must leave the new request reachable for relay cancellation');
    current.abort(new Error('MCP client cancelled the request.'));
    assert.equal(signals[1].aborted,true);
    newResult.resolve({status:'new'});await newDispatch;
    assert.equal(f.bridge.pending.has(request.id),false);
    assert.equal(f.replies.some(reply=>reply.sessionId==='old'),false,'Disconnected requests cannot publish into the new session');
});

test('requests captured from a superseded connection are never dispatched',async t=>{
    const f=fixture(t);let handled=0;
    f.bridge.control.handle=async()=>{++handled;return {};};
    f.bridge.connection=new AbortController();
    await f.bridge.dispatch({id:'old',method:'observe',args:{},deadline:Date.now()+60000},new AbortController(),{sessionId:'old',browserToken:'old'});
    assert.equal(handled,0);
    assert.equal(f.bridge.pending.size,0);
    assert.deepEqual(f.replies,[]);
});
