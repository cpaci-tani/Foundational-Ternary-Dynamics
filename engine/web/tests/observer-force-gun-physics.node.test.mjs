import test from 'node:test';
import assert from 'node:assert/strict';
import {ObserverSession} from '../js/observer/session.js';
import {ObserverWorkerClient} from '../js/observer/worker-client.js';
import {computeGunForce,pointInverseMass,forceGunSettings} from '../js/observer/force-gun-physics.js';
import {reflectPolar,reflectAxial,reflectEulerXYZ} from '../js/observer/mirror-frame.js';

const close=(a,b,tolerance=2e-5)=>assert.ok(Math.abs(a-b)<=tolerance,`${a} != ${b}`);
const vectorClose=(a,b,tolerance)=>a.forEach((value,i)=>close(value,b[i],tolerance));
const advance=(session,steps=1,input={})=>{for(let i=0;i<steps;i++)session.advance(1/120,input);};
let token=0;
async function world(properties={},settings={}) {
    const session=new ObserverSession({profile:'playground',playing:false});
    session.state.entities=[];session.state.segments=[];
    Object.assign(session.state,{gravityStrength:0,planeCollision:false,objectCollisions:true},settings);
    session.addEntity({position:[0,2,0],damping:0,friction:0,restitution:0,gravity:false,...properties});
    await session.initialize();return session;
}
function begin(session,patch={}) {
    const e=session.state.entities[0];
    return session.command({type:'gun-begin',id:e.id,token:`test-${++token}`,expectedEpoch:session.state.epoch,expectedTargetRevision:e.revision,
        historical:false,mode:'pull',localAnchor:[0,0,0],target:[2,2,0],direction:[1,0,0],sensitivity:'normal',multiplier:1,sequence:1,...patch});
}
const update=(session,patch={})=>({token:session.forceGunGrab.token,epoch:session.state.epoch,sequence:session.forceGunGrab.sequence+1,
    target:[3,2,0],direction:[1,0,0],sensitivity:'normal',multiplier:1,...patch});

test('point inertia includes angular response and the implicit spring has bounded force',()=>{
    assert.deepEqual(pointInverseMass([1,1,1],[[2,0,0],[0,3,0],[0,0,4]],[1,0,0]),[[1,0,0],[0,5,0],[0,0,4]]);
    for(const sensitivity of ['delicate','normal','strong'])for(const multiplier of [.1,1,10]){
        const force=computeGunForce({anchor:[0,0,0],target:[1e9,-1e9,1e9],velocity:[0,0,0],direction:[1,0,0],mode:'pull',sensitivity,multiplier,dt:1/120,
            effectiveInverseMass:[[1,0,0],[0,1,0],[0,0,1]]});
        close(force.magnitude,forceGunSettings(sensitivity,multiplier).maxForce,1e-9);
    }
    assert.throws(()=>forceGunSettings('unknown',1));assert.throws(()=>forceGunSettings('normal',NaN));
});

test('requested force reports zero, below-cap, exact-cap and over-cap demand without changing the clamp',()=>{
    const dt=1/120;
    for(const sensitivity of ['delicate','normal','strong'])for(const ratio of [0,.5,1,1.5]){
        const {stiffness,damping,maxForce}=forceGunSettings(sensitivity,1);
        const requested=ratio*maxForce;
        // For a unit-mass central pull at rest, A = 1 + damping*dt + stiffness*dt².
        const distance=requested*(1+damping*dt+stiffness*dt*dt)/stiffness;
        const result=computeGunForce({anchor:[0,0,0],target:[distance,0,0],velocity:[0,0,0],direction:[1,0,0],mode:'pull',sensitivity,multiplier:1,dt,
            effectiveInverseMass:[[1,0,0],[0,1,0],[0,0,1]]});
        close(result.requestedMagnitude,requested,1e-9);
        close(result.magnitude,Math.min(requested,maxForce),1e-9);
        vectorClose(result.force,[Math.min(requested,maxForce),0,0],1e-9);
        assert.equal(result.maxForce,maxForce);
        assert.equal(result.requestedMagnitude>maxForce*(1+1e-6),ratio>1);
    }
});

test('Rapier telemetry includes unclamped demand while applied impulse still obeys the existing force cap',async()=>{
    const session=await world({mass:2});
    try{
        const result=await begin(session,{target:[1000,2,0]});assert.ok(result.ok,result.error);
        assert.ok(result.forceGun.requestedForceMagnitude>result.forceGun.maxForce);
        close(result.forceGun.forceMagnitude,120);vectorClose(result.forceGun.force,[120,0,0]);
        advance(session);
        const telemetry=session.getForceGunTelemetry();
        assert.ok(telemetry.requestedForceMagnitude>telemetry.maxForce);
        vectorClose(session.state.entities[0].velocity,[120/2/120,0,0]);
        assert.equal('requestedForceMagnitude' in session.snapshot(),false,'live effort is not persistent world data');
    }finally{session.dispose();}
});

test('the same pull force accelerates a light body faster without teleportation or history records',async()=>{
    const light=await world({mass:1}),heavy=await world({mass:10});
    try{
        const history=light.undoStack.length,revision=light.state.entities[0].revision;
        assert.ok((await begin(light)).ok);assert.ok((await begin(heavy)).ok);
        assert.equal(light.state.playing,true,'holding explicitly resumes a paused world');
        assert.equal(light.state.entities[0].position[0],0,'begin does not teleport');
        advance(light,12);advance(heavy,12);
        const a=light.state.entities[0],b=heavy.state.entities[0];
        assert.ok(a.velocity[0]>b.velocity[0]*5,`${a.velocity[0]} vs ${b.velocity[0]}`);
        assert.ok(a.position[0]>b.position[0]*5);
        assert.equal(light.undoStack.length,history);assert.equal(a.revision,revision);
        assert.ok(light.forceGunTelemetry.forceMagnitude<=120);close(light.forceGunTelemetry.mass,1);
        assert.equal('forceGun' in light.snapshot(),false);assert.equal('forceGunGrab' in light.snapshot(),false);
    }finally{light.dispose();heavy.dispose();}
});

test('release leaves only acquired momentum and an old token cannot release a newer tether',async()=>{
    const session=await world();
    try{
        await begin(session);advance(session,10);const first=session.forceGunGrab.token;
        await session.command({type:'gun-end',token:first,expectedEpoch:session.state.epoch});
        const velocity=[...session.state.entities[0].velocity];advance(session,20);
        vectorClose(session.state.entities[0].velocity,velocity);assert.equal(session.forceGunTelemetry,null);
        await begin(session);const second=session.forceGunGrab.token;
        await session.command({type:'gun-end',token:first,expectedEpoch:session.state.epoch});
        assert.equal(session.forceGunGrab.token,second);
        await session.command({type:'gun-end',token:second,expectedEpoch:session.state.epoch});
        const retired=await begin(session,{token:first});assert.equal(retired.ok,false);assert.match(retired.error,/ended/);
    }finally{session.dispose();}
});

test('push drives outward while following lateral aim, independently of target depth',async()=>{
    const session=await world();
    try{
        const result=await begin(session,{mode:'push',direction:[1,0,0],target:[-50,3,0]});assert.ok(result.ok,result.error);
        assert.ok(result.forceGun.force[0]>0,'push cannot pull back toward a shallower target');
        assert.ok(result.forceGun.force[1]>0,'sideways spring follows aim');
        advance(session,30);assert.ok(session.state.entities[0].position[0]>.1);assert.ok(session.state.entities[0].position[1]>2.1);
    }finally{session.dispose();}
});

test('a reflected surface grab preserves polar force and axial torque in the source frame',async()=>{
    const properties={shape:'box',size:[1,1.4,.8],rotation:[.2,.3,-.1]};
    const upper=await world(properties),lower=await world({...properties,position:[0,-2,0],rotation:reflectEulerXYZ(properties.rotation)});
    try{
        const anchor=[.4,.5,.2],target=[2,3,-1];
        assert.ok((await begin(upper,{localAnchor:anchor,target})).ok);
        assert.ok((await begin(lower,{localAnchor:reflectPolar(anchor),target:reflectPolar(target),direction:reflectPolar([1,0,0])})).ok);
        vectorClose(lower.forceGunTelemetry.anchorWorld,reflectPolar(upper.forceGunTelemetry.anchorWorld));
        vectorClose(lower.forceGunTelemetry.force,reflectPolar(upper.forceGunTelemetry.force));
        advance(upper,15);advance(lower,15);
        const a=upper.state.entities[0],b=lower.state.entities[0];
        vectorClose(b.position,reflectPolar(a.position));vectorClose(b.velocity,reflectPolar(a.velocity));
        vectorClose(b.angularVelocity,reflectAxial(a.angularVelocity));
        assert.ok(Math.hypot(...a.angularVelocity)>.1,'off-center attachment produces torque');
        assert.equal(lower.playground.bodies.size,1);
    }finally{upper.dispose();lower.dispose();}
});

test('a lower-authored wedge has the same source-frame force response in folded and unfolded solvers',async()=>{
    const properties={shape:'wedge',position:[0,-2,0],size:[1,1.4,.8],rotation:[.2,.3,-.1]};
    const folded=await world(properties),unfolded=await world(properties,{gravityMode:'uniform',gravity:[0,0,0]});
    try{
        const gun={localAnchor:[.3,.3,.2],target:[2,-3,-1]};
        assert.ok((await begin(folded,gun)).ok);assert.ok((await begin(unfolded,gun)).ok);
        vectorClose(folded.forceGunTelemetry.anchorWorld,unfolded.forceGunTelemetry.anchorWorld);
        vectorClose(folded.forceGunTelemetry.force,unfolded.forceGunTelemetry.force);
        advance(folded,30);advance(unfolded,30);
        const a=folded.state.entities[0],b=unfolded.state.entities[0];
        vectorClose(a.position,b.position);vectorClose(a.velocity,b.velocity);vectorClose(a.angularVelocity,b.angularVelocity,1e-4);
    }finally{folded.dispose();unfolded.dispose();}
});

test('minimum-mass tiny off-center bodies remain finite under the strongest gun',async()=>{
    const session=await world({shape:'box',mass:1e-6,size:[.001,.001,.001]});
    try{
        const result=await begin(session,{localAnchor:[.0005,.0005,0],sensitivity:'strong',multiplier:10,target:[1,2,0]});assert.ok(result.ok,result.error);
        for(let i=0;i<600;i++){
            advance(session);const e=session.state.entities[0];
            assert.ok([...e.position,...e.velocity,...e.angularVelocity].every(Number.isFinite));
            assert.ok(Math.hypot(...e.velocity)<1000);assert.ok(Math.hypot(...e.angularVelocity)<1e6);
            assert.ok(session.forceGunTelemetry.forceMagnitude<=6000+1e-8);
        }
        assert.ok(Math.abs(session.state.entities[0].position[0]-1)<.1,'tiny body reaches the target without an unstable orbit');
    }finally{session.dispose();}
});

test('an attached asymmetric body crosses an open mirrored plane with continuous source force and torque',async()=>{
    const properties={shape:'wedge',position:[0,.15,0],velocity:[.1,-1,.2],rotation:[.2,.3,-.1],angularVelocity:[.2,-.1,.3],size:[1,1,1]};
    const folded=await world(properties),unfolded=await world(properties,{gravityMode:'uniform',gravity:[0,0,0]});
    try{
        const gun={localAnchor:[.3,.3,.2],target:[.2,-1,.4],sensitivity:'delicate'};
        assert.ok((await begin(folded,gun)).ok);assert.ok((await begin(unfolded,gun)).ok);
        advance(folded,50);advance(unfolded,50);
        const a=folded.state.entities[0],b=unfolded.state.entities[0];
        assert.ok(a.position[1]<0);assert.equal(folded.playground.sides.get(a.id),-1);
        vectorClose(a.position,b.position,1e-4);vectorClose(a.velocity,b.velocity,1e-4);
        vectorClose(a.angularVelocity,b.angularVelocity,2e-4);
        vectorClose(folded.forceGunTelemetry.anchorWorld,unfolded.forceGunTelemetry.anchorWorld,1e-4);
        vectorClose(folded.forceGunTelemetry.force,unfolded.forceGunTelemetry.force,1e-4);
    }finally{folded.dispose();unfolded.dispose();}
});

test('tethers preserve rigid-body and plane contacts instead of dragging through obstacles',async()=>{
    const session=await world({position:[0,2,0],size:[.5,.5,.5]});
    try{
        const created=await session.command({type:'create',entity:{shape:'box',bodyType:'fixed',position:[2,2,0],size:[.5,5,5]}});assert.ok(created.ok,created.error);
        const started=await begin(session,{localAnchor:[.25,0,0],target:[4,2,0],sensitivity:'strong'});assert.ok(started.ok,started.error);
        advance(session,240);assert.ok(session.state.entities[0].position[0]<1.55,'wall remains solid');
        await session.command({type:'world-physics',patch:{planeCollision:true}});
        await begin(session,{target:[0,-4,0],direction:[0,-1,0],sensitivity:'strong'});advance(session,240);
        assert.ok(session.state.entities[0].position[1]>.20,'plane remains solid under downward force');
    }finally{session.dispose();}
});

test('force-gun sequencing rejects delayed updates and stale epochs or target revisions',async()=>{
    const session=await world();
    try{
        const id=session.state.entities[0].id;
        for(const patch of [{expectedEpoch:99},{expectedTargetRevision:99},{historical:true},{localAnchor:[2,0,0]},{sequence:-1},{sequence:.5}]){
            const result=await begin(session,patch);assert.equal(result.ok,false);assert.equal(session.forceGunGrab,null);
        }
        assert.ok((await begin(session,{sequence:10})).ok);
        const original=[...session.forceGunGrab.target];
        session.advance(0,{forceGun:update(session,{sequence:10,target:[99,2,0]})});assert.deepEqual(session.forceGunGrab.target,original);
        session.advance(0,{forceGun:update(session,{epoch:99,target:[99,2,0]})});assert.deepEqual(session.forceGunGrab.target,original);
        session.advance(0,{forceGun:update(session,{token:'wrong-token',target:[99,2,0]})});assert.deepEqual(session.forceGunGrab.target,original);
        session.advance(0,{forceGun:update(session,{sequence:11,target:[3,2,0]})});assert.deepEqual(session.forceGunGrab.target,[3,2,0]);
        const stale=await session.command({type:'gun-update',expectedEpoch:session.state.epoch,...update(session,{sequence:9})});assert.equal(stale.ok,false);
        assert.equal(session.forceGunGrab.sequence,11);
        const edited=await session.command({type:'update',id,patch:{mass:2}});assert.ok(edited.ok);assert.equal(session.forceGunGrab,null);
        const oldInput={token:'never-begin',epoch:session.state.epoch,sequence:99,target:[10,2,0],direction:[1,0,0],sensitivity:'normal',multiplier:1};
        session.advance(1/120,{forceGun:oldInput});assert.equal(session.forceGunGrab,null,'updates never begin a grab');
    }finally{session.dispose();}
});

test('SR, fixed, kinematic, and deleted targets reject force-gun begin',async()=>{
    const sr=new ObserverSession();await sr.initialize();
    try{const result=await begin(sr);assert.equal(result.ok,false);assert.match(result.error,/Playground/);}finally{sr.dispose();}
    for(const bodyType of ['fixed','kinematic']){
        const session=await world({bodyType});try{assert.equal((await begin(session)).ok,false);}finally{session.dispose();}
    }
    const session=await world();try{await session.command({type:'delete',id:session.state.entities[0].id});assert.equal((await begin(session)).ok,false);}finally{session.dispose();}
});

for(const command of ['pause','profile','reset','update','delete','undo','load','world-physics'])test(`${command} clears the transient gun before changing state`,async()=>{
    const session=await world();
    try{
        const id=session.state.entities[0].id;
        await session.command({type:'update',id,patch:{name:'Undo checkpoint'}});
        const saved=session.snapshot();await begin(session);advance(session,2);
        const before=session.forceGunGrab,commands={pause:{},profile:{profile:'sr'},reset:{},update:{id,patch:{mass:2}},delete:{id},undo:{},load:{snapshot:saved},'world-physics':{patch:{gravityStrength:1}}};
        const result=await session.command({type:command,...commands[command]});assert.ok(result.ok,result.error);
        assert.equal(session.forceGunGrab,null);assert.equal(result.forceGun,null);
        await session.command({type:'play'});session.advance(1/120,{forceGun:{...before,sequence:100}});
        assert.equal(session.forceGunGrab,null,'old input cannot resurrect after resume');
    }finally{session.dispose();}
});

test('snapshot roundtrip carries body momentum but no tether, and rejects injected transient controls',async()=>{
    const session=await world();
    try{
        await begin(session);advance(session,20);const saved=session.snapshot();
        assert.equal(JSON.stringify(saved).includes('gun-'),false);
        const loaded=await session.command({type:'load',snapshot:JSON.parse(JSON.stringify(saved))});assert.ok(loaded.ok,loaded.error);
        assert.equal(session.forceGunGrab,null);assert.equal(loaded.forceGun,null);
        vectorClose(loaded.snapshot.entities[0].velocity,saved.entities[0].velocity);
        const rejected=await session.command({type:'load',snapshot:{...saved,forceGun:{token:'injected'}}});assert.equal(rejected.ok,false);assert.match(rejected.error,/live force-gun/);
    }finally{session.dispose();}
});

test('end-before-begin, malformed live updates, backlog pause and disposal cannot leave a force running',async()=>{
    const session=await world();
    try{
        const cancelled='cancel-before-ack';
        await session.command({type:'gun-end',token:cancelled,expectedEpoch:session.state.epoch});
        assert.equal((await begin(session,{token:cancelled})).ok,false);
        await begin(session);session.advance(0,{forceGun:update(session,{target:[NaN,0,0]})});
        assert.equal(session.forceGunGrab,null);assert.equal(session.forceGunTelemetry,null);
        await begin(session);session.advance(6);
        assert.equal(session.state.playing,false);assert.equal(session.forceGunGrab,null);assert.equal(session.forceGunTelemetry,null);
        await begin(session);session.dispose();assert.equal(session.forceGunGrab,null);assert.equal(session.forceGunTelemetry,null);
    }finally{session.dispose();}
});

test('worker client accepts telemetry only with the newest accepted snapshot response',async()=>{
    const messages=[],listeners={};
    const worker={addEventListener:(name,callback)=>{listeners[name]=callback;},postMessage:message=>messages.push(message),terminate(){}};
    const client=new ObserverWorkerClient({workerFactory:()=>worker});
    const session=new ObserverSession();const snapshot=session.snapshot();
    const respond=(id,forceGun,state=snapshot)=>listeners.message({data:{id,ok:true,snapshot:state,forceGun}});
    respond(messages[0].id,null);await client.ready;
    const a=client.send({type:'advance'}),first=messages.at(-1).id;
    const b=client.send({type:'command'}),second=messages.at(-1).id;
    respond(second,null);respond(first,{token:'obsolete'});await Promise.all([a,b]);assert.equal(client.latestForceGun,null);
    const c=client.send({type:'advance'});respond(messages.at(-1).id,{token:'active'});await c;assert.equal(client.latestForceGun.token,'active');
    client.dispose();session.dispose();assert.equal(client.latestForceGun,null);
});
