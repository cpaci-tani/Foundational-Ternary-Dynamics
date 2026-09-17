import assert from 'node:assert/strict';

// Run against an isolated ws_server --once instance, never a user's session.
const address = process.argv[2];
assert(address, 'An isolated native WebSocket URL is required');
const ws = new WebSocket(address);
await new Promise((resolve, reject) => {ws.onopen = resolve; ws.onerror = reject;});
let sequence=0;
const pending=new Map();
ws.onmessage = event => {
    if (typeof event.data !== 'string') return;
    const value=JSON.parse(event.data), entry=pending.get(value._requestId);
    if (!entry || value.type === 'operation_progress') return;
    pending.delete(value._requestId); clearTimeout(entry.timer); entry.resolve(value);
};
function rpc(cmd, args={}) {
    return new Promise((resolve,reject) => {
        const id=++sequence;
        const timer=setTimeout(() => {pending.delete(id);reject(new Error(`Timeout: ${cmd}`));},60000);
        pending.set(id,{resolve,timer}); ws.send(JSON.stringify({cmd,...args,_requestId:id}));
    });
}
const digest = async () => {
    const d=await rpc('get_dynamical_state_digest');
    assert(!d.error, d.error); return [d.hashLo,d.hashHi,d.tick,d.sourceEpoch];
};
try {
    const info=await rpc('info'); assert.equal(info.seedRecipeVersion,2);
    const before=await digest();
    const preparation={name:'flux-pulse',size:17,overrides:{'packet.amplitude':0.6}};
    const preview=await rpc('seed_prepare',preparation);
    assert.equal(preview.ok,true,preview.error);
    assert.equal(preview.description.properties.find(p=>p.key==='packet.amplitude').value,0.6);
    assert.deepEqual(await digest(),before,'preview mutated the active state');
    const invalid=await rpc('seed_prepare',{...preparation,overrides:{'packet.amplitude':-1}});
    assert(invalid.error); assert.deepEqual(await digest(),before,'invalid preview mutated state');
    const commit=await rpc('seed_commit',{...preparation,expectedSourceEpoch:preview.sourceEpoch});
    assert.equal(commit.ok,true,commit.error); assert.equal(commit.latticeSize,17);
    const applied=await digest(); assert.equal(applied[2],0); assert.notDeepEqual(applied,before);
    const stale=await rpc('seed_commit',{...preparation,expectedSourceEpoch:preview.sourceEpoch});
    assert.match(stale.error,/superseded/); assert.deepEqual(await digest(),applied,'stale commit replaced live state');
    const bad=await rpc('seed_commit',{...preparation,expectedSourceEpoch:applied[3],overrides:{unknown:1}});
    assert(bad.error); assert.deepEqual(await digest(),applied,'invalid commit replaced live state');
    const repeat=await rpc('seed_commit',{...preparation,expectedSourceEpoch:applied[3]});
    assert.equal(repeat.ok,true,repeat.error); const reset=await digest();
    assert.deepEqual(reset.slice(0,3),applied.slice(0,3),'deterministic recipe replay differs');
    console.log('Native seed v2: preview isolation, exact property echo, invalid/stale rejection, paused commit and replay passed.');
} finally {ws.close();}
