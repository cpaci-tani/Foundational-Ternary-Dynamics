import test from 'node:test';
import assert from 'node:assert/strict';
import {ObserverSession} from '../js/observer/session.js';
import {gamma,fourMomentum,retardedTime} from '../js/observer/math.js';
import {ObserverWorkerClient} from '../js/observer/worker-client.js';
import {Worker as NodeWorker} from 'node:worker_threads';
const close=(a,b,tol=1e-9)=>assert.ok(Math.abs(a-b)<tol,`${a} != ${b}`);
const advance=(session,seconds,input={})=>{for(let t=0;t<seconds-1e-10;t+=.1)session.advance(Math.min(.1,seconds-t),input);};
test('baseline has explicit inertial prehistory and portable snapshots',async()=>{
    const s=new ObserverSession();await s.initialize();const snap=s.snapshot();
    assert.equal(snap.entities.length,4);assert.equal(snap.historyStart,-60);
    assert.ok(snap.segments.every(x=>x.start===-60&&x.end===null));
    assert.deepEqual(JSON.parse(JSON.stringify(snap)),snap);
    snap.entities[0].position[0]=99;assert.notEqual(s.snapshot().entities[0].position[0],99);
});
test('fixed stepping is independent of rendering rate',()=>{
    const a=new ObserverSession(),b=new ObserverSession();
    for(let i=0;i<60;i++)a.advance(1/60,{move:[0,0,1]});
    for(let i=0;i<144;i++)b.advance(1/144,{move:[0,0,1]});
    assert.equal(a.state.tick,120);assert.equal(b.state.tick,120);
    a.state.observer.position.forEach((x,i)=>close(x,b.state.observer.position[i]));
});
test('same-time edits own disjoint worldtube intervals and preserve prior appearance',async()=>{
    const s=new ObserverSession();advance(s,2);const id=s.state.entities[0].id;
    const first=await s.command({type:'update',id,patch:{color:[1,0,0]}});assert.ok(first.ok);
    const second=await s.command({type:'update',id,patch:{color:[0,1,0]}});assert.ok(second.ok);
    const segments=s.state.segments.filter(x=>x.entityId===id);
    assert.equal(segments.length,2);assert.equal(segments[0].end,s.state.time);assert.equal(segments[1].start,s.state.time);
    assert.deepEqual(segments[1].color,[0,1,0]);assert.deepEqual(segments[0].color,[.15,.8,1]);
});
test('deletion remains optically inspectable until retained history expires',async()=>{
    const s=new ObserverSession();const id=s.state.entities[0].id;advance(s,1);
    await s.command({type:'delete',id});assert.equal(s.state.entities[0].alive,false);
    const segment=s.state.segments.find(x=>x.entityId===id),emit=retardedTime(s.state.observer.position,s.state.time,segment.position,segment.velocity,segment.originTime);
    assert.ok(emit<segment.end);assert.ok(emit>=segment.start);
    advance(s,61);assert.ok(!s.state.entities.some(x=>x.id===id));
});
test('create has no fabricated prehistory and restore creates new revision',async()=>{
    const s=new ObserverSession();advance(s,1);
    await s.command({type:'create',entity:{name:'New clock',shape:'clock'}});
    const e=s.state.entities.at(-1),start=s.state.time;assert.equal(e.createdAt,start);
    assert.equal(s.state.segments.at(-1).start,start);
    await s.command({type:'delete',id:e.id});advance(s,.1);await s.command({type:'restore',id:e.id});
    assert.equal(s.state.entities.at(-1).alive,true);assert.equal(s.state.segments.at(-1).revision,3);
});
test('stale epoch, revision, and identity are rejected atomically',async()=>{
    const s=new ObserverSession();const baseline=s.snapshot();
    for(const guard of [{expectedEpoch:0},{expectedRevision:9},{sessionId:'another'},{expectedTargetRevision:0}]) {
        const result=await s.command({type:'update',id:baseline.entities[0].id,patch:{name:'Rejected'},...guard});
        assert.equal(result.ok,false);assert.deepEqual(s.snapshot(),baseline);
    }
});
test('invalid updates preserve the prior segment and body state',async()=>{
    const s=new ObserverSession();advance(s,1);const before=s.snapshot();
    const r=await s.command({type:'update',id:before.entities[0].id,patch:{position:[2,3,4],velocity:[1,0,0]}});
    assert.equal(r.ok,false);assert.deepEqual(s.snapshot(),before);
});
test('SR restricts extended-body acceleration and continuous rotation',async()=>{
    const s=new ObserverSession(),id=s.state.entities[0].id;
    assert.equal((await s.command({type:'update',id,patch:{properAcceleration:[1,0,0]}})).ok,false);
    assert.equal((await s.command({type:'update',id,patch:{angularVelocity:[0,1,0]}})).ok,false);
    assert.equal((await s.command({type:'impulse',id,impulse:[1,0,0]})).ok,false);
});
test('a paused impulse changes momentum by J without stepping time, positions, or other bodies',async()=>{
    const s=new ObserverSession({profile:'playground',playing:false});await s.initialize();
    const id=s.state.entities[0].id;
    assert.ok((await s.command({type:'update',id,patch:{mass:2,gravity:false,damping:0,velocity:[1,0,0],position:[0.123456789,1.123456789,-2.123456789]}})).ok);
    const before=s.snapshot();
    // Applying an instantaneous command must not call the solver, even with dt=0.
    const world=s.playground.world,realStep=world.step;
    world.step=()=>{throw new Error('Unexpected physics step during an impulse');};
    const result=await s.command({type:'impulse',id,impulse:[2,6,-4],expectedTargetRevision:before.entities[0].revision});
    world.step=realStep;
    assert.ok(result.ok,result.error);
    assert.deepEqual(result.snapshot.entities[0].velocity,[2,3,-2]);
    assert.equal(result.snapshot.entities[0].revision,before.entities[0].revision+1);
    assert.equal(result.snapshot.time,before.time);assert.equal(result.snapshot.tick,before.tick);
    assert.equal(result.snapshot.playing,false);
    result.snapshot.entities.forEach((e,i)=>assert.deepEqual(e.position,before.entities[i].position));
    assert.deepEqual(result.snapshot.entities.slice(1),before.entities.slice(1));
    assert.deepEqual(result.snapshot.observer,before.observer);
    assert.ok((await s.command({type:'play'})).ok);s.advance(.05);
    assert.ok(s.state.entities[0].position[1]>before.entities[0].position[1]);
    s.dispose();
});
for(const bodyType of ['fixed','kinematic'])test(`${bodyType} bodies reject impulses instead of acknowledging no motion`,async()=>{
    const s=new ObserverSession({profile:'playground',playing:false});await s.initialize();
    const id=s.state.entities[0].id;await s.command({type:'update',id,patch:{bodyType}});
    const before=s.snapshot(),result=await s.command({type:'impulse',id,impulse:[0,3,0]});
    assert.equal(result.ok,false);assert.match(result.error,/dynamic body/);
    assert.deepEqual(s.snapshot(),before);s.dispose();
});
test('zero, deleted, and stale-target impulses are rejected atomically',async()=>{
    const s=new ObserverSession({profile:'playground',playing:false});await s.initialize();
    const id=s.state.entities[0].id;
    for(const command of [{type:'impulse',id,impulse:[0,0,0]},{type:'impulse',id,impulse:[0,3,0],expectedTargetRevision:0}]) {
        const before=s.snapshot(),result=await s.command(command);
        assert.equal(result.ok,false);assert.deepEqual(s.snapshot(),before);
    }
    await s.command({type:'delete',id});const before=s.snapshot();
    const result=await s.command({type:'impulse',id,impulse:[0,3,0]});
    assert.equal(result.ok,false);assert.match(result.error,/live object/);assert.deepEqual(s.snapshot(),before);s.dispose();
});
test('impulse undo restores momentum and paused import keeps the accepted velocity',async()=>{
    const s=new ObserverSession({profile:'playground',playing:false});await s.initialize();
    const id=s.state.entities[0].id,before=s.snapshot();
    assert.ok((await s.command({type:'impulse',id,impulse:[1,3,0]})).ok);
    const saved=s.snapshot(),other=new ObserverSession();
    assert.ok((await other.command({type:'load',snapshot:saved})).ok);
    assert.deepEqual(other.state.entities[0].velocity,[1,3,0]);
    assert.ok((await s.command({type:'undo'})).ok);assert.deepEqual(s.state.entities[0].velocity,before.entities[0].velocity);
    const velocity=s.playground.bodies.get(id).linvel();assert.deepEqual([velocity.x,velocity.y,velocity.z],[0,0,0]);
    s.dispose();other.dispose();
});
test('point-clock acceleration produces timelike historical segments',async()=>{
    const s=new ObserverSession();const clock=s.state.entities.find(e=>e.shape==='clock');
    await s.command({type:'update',id:clock.id,patch:{properAcceleration:[.5,0,0]}});advance(s,1);
    assert.ok(s.state.segments.filter(x=>x.entityId===clock.id).length>100);
    close(clock.clockOffset,Math.asinh(.5)/.5,3e-3);
    assert.ok(s.state.segments.every(x=>Number.isFinite(gamma(x.velocity))));
});
test('moving clocks maintain expected proper time',()=>{
    const s=new ObserverSession({preset:'clocks'});advance(s,3);
    close(s.state.entities[0].clockOffset,2.4);close(s.state.entities[1].clockOffset,1.8);
});
test('paused movement relocates and resume resets observer clock origin',async()=>{
    const s=new ObserverSession();advance(s,1);await s.command({type:'pause'});const pos=[...s.state.observer.position];
    s.advance(.1,{move:[1,0,0]});assert.notDeepEqual(s.state.observer.position,pos);close(s.state.time,1);
    const worldline=s.state.observer.worldline;await s.command({type:'play'});
    assert.equal(s.state.observer.properTime,0);assert.equal(s.state.observer.worldline,worldline+1);
});
test('dolly travel has no scene-distance cap and reverses without changing preparation',async()=>{
    const s=new ObserverSession({playing:false}),before=s.snapshot(),distance=2**35;
    for(let i=0;i<128;i++)assert.equal((await s.command({type:'dolly',displacement:[0,0,-distance]})).ok,true);
    assert.deepEqual(s.state.observer.position,[0,1.6,8-128*distance]);
    assert.ok(Math.abs(s.state.observer.position[2])>1e12);
    const imported=new ObserverSession();
    const restored=await imported.command({type:'load',snapshot:JSON.parse(JSON.stringify(s.snapshot()))});
    assert.ok(restored.ok,restored.error);assert.deepEqual(imported.state.observer.position,s.state.observer.position);
    for(let i=0;i<128;i++)assert.equal((await s.command({type:'dolly',payload:{displacement:[0,0,distance]}})).ok,true);
    assert.deepEqual(s.state.observer.position,before.observer.position);
    const {observer:originalObserver,revision:originalRevision,preparationVersion:originalPreparation,...originalWorld}=before;
    const {observer,revision,preparationVersion,...world}=s.snapshot();
    assert.deepEqual(world,originalWorld);assert.equal(revision,originalRevision+256);
    assert.equal(preparationVersion,originalPreparation+256,'explicit camera edits invalidate assistant intentions, not physical world state');
    assert.equal(observer.worldline,originalObserver.worldline+256);assert.equal(s.undoStack.length,0);
    imported.dispose();s.dispose();
});
for(const profile of ['sr','playground'])test(`dolly relocates the ${profile} camera while preserving live physical state`,async()=>{
    const s=new ObserverSession({profile});await s.initialize();
    await s.command({type:'observer',patch:{velocity:[.6,0,0],yaw:.7,pitch:.2,roll:.1}});advance(s,.2);
    const before=s.snapshot(),worldOwner=s.playground?.world,bodies=s.playground&&[...s.playground.bodies.values()];
    const result=await s.command({type:'dolly',displacement:[3,-2,-4]});assert.ok(result.ok,result.error);
    assert.deepEqual(s.state.observer.position,before.observer.position.map((value,i)=>value+[3,-2,-4][i]));
    assert.equal(s.state.observer.properTime,0);assert.deepEqual(s.state.observer.velocity,[0,0,0]);
    assert.equal(s.state.observer.worldline,before.observer.worldline+1);
    for(const key of ['yaw','pitch','roll'])assert.equal(s.state.observer[key],before.observer[key]);
    const {observer:_oldObserver,revision:_oldRevision,preparationVersion:oldPreparation,...previousState}=before;
    const {observer:_observer,revision:_revision,preparationVersion,...currentState}=s.snapshot();
    assert.deepEqual(currentState,previousState);assert.equal(s.undoStack.length,0);
    assert.equal(preparationVersion,oldPreparation+1);
    assert.equal(s.playground?.world,worldOwner);if(bodies)assert.deepEqual([...s.playground.bodies.values()],bodies);
    s.dispose();
});
for(const profile of ['sr','playground'])test(`invalid dolly displacements reject atomically in ${profile}`,async()=>{
    const s=new ObserverSession({profile});await s.initialize();
    const worldOwner=s.playground?.world;
    for(const displacement of [[Infinity,0,0],[0,NaN,0],[0,0,-Infinity],[1,2],['1',0,0],null]){
        const before=s.snapshot(),result=await s.command({type:'dolly',displacement});
        assert.equal(result.ok,false);assert.deepEqual(s.snapshot(),before);assert.equal(s.playground?.world,worldOwner);
    }
    assert.equal((await s.command({type:'dolly',displacement:[Number.MAX_VALUE,0,0]})).ok,true);
    const before=s.snapshot(),relocated=s.relocated;
    assert.equal((await s.command({type:'dolly',displacement:[Number.MAX_VALUE,0,0]})).ok,false);
    assert.deepEqual(s.snapshot(),before);assert.equal(s.relocated,relocated);assert.equal(s.playground?.world,worldOwner);
    s.dispose();
});
test('grounded movement and axis locks constrain movement',()=>{
    const s=new ObserverSession();advance(s,1,{move:[1,1,1],grounded:true,axisLocks:[true,false,false]});
    close(s.state.observer.position[0],0);close(s.state.observer.position[1],1.6);assert.ok(s.state.observer.position[2]<8);
});
test('free flight follows the pitched view and crosses the reference plane',()=>{
    const s=new ObserverSession();
    advance(s,2,{move:[0,0,1],pitch:Math.PI/3,speed:.8,acceleration:4,grounded:false});
    assert.ok(s.state.observer.position[1]>2);assert.ok(s.state.observer.position[2]<8);
    advance(s,10,{move:[0,0,1],pitch:-Math.PI/3,speed:.8,acceleration:4,grounded:false});
    assert.ok(s.state.observer.position[1]<0);assert.ok(Math.hypot(...s.state.observer.velocity)<1);
});
test('world-up unlock applies camera roll while grounded movement remains level',()=>{
    const s=new ObserverSession();s.state.observer.roll=Math.PI/2;s.state.observer.pitch=.4;
    const rolled=s.movement({move:[1,0,0],worldUp:false,grounded:false});
    assert.ok(Math.abs(rolled[1])>.9);
    close(s.movement({move:[1,0,0],worldUp:true,grounded:false})[1],0);
    close(s.movement({move:[1,1,1],worldUp:false,grounded:true})[1],0);
});
test('undo restores checkpoint in a new epoch; reset prepares a fresh epoch',async()=>{
    const s=new ObserverSession();const id=s.state.entities[0].id;
    await s.command({type:'update',id,patch:{name:'Edited'}});const epoch=s.state.epoch;
    await s.command({type:'undo'});assert.equal(s.state.entities[0].name,'Luminous sphere');assert.equal(s.state.epoch,epoch+1);assert.equal(s.state.playing,false);
    await s.command({type:'reset'});assert.equal(s.state.epoch,epoch+2);assert.equal(s.state.time,0);
});
test('scrubbing is read-only until an edit creates a branch at historical time',async()=>{
    const s=new ObserverSession({preset:'moving-shapes'});advance(s,3);const id=s.state.entities[0].id;
    await s.command({type:'scrub',time:1});assert.equal(s.snapshot().time,1);close(s.state.time,3);
    const epoch=s.state.epoch;assert.equal((await s.command({type:'play'})).ok,false);
    await s.command({type:'update',id,patch:{name:'Branched'}});
    assert.equal(s.state.time,1);assert.equal(s.state.epoch,epoch+1);assert.equal(s.state.entities[0].name,'Branched');close(s.state.entities[0].position[0],-2.2);
});
test('bounded catchup retains unprocessed time',()=>{
    const s=new ObserverSession();s.advance(1);close(s.state.time,.25);close(s.snapshot().backlogSeconds,.75);
    s.advance(0);s.advance(0);s.advance(0);close(s.state.time,1);close(s.snapshot().backlogSeconds,0);
});
test('entity allocation limit rejects without eviction',async()=>{
    const s=new ObserverSession();for(let i=4;i<256;i++)s.addEntity();const ids=s.state.entities.map(e=>e.id);
    assert.equal((await s.command({type:'create',entity:{}})).ok,false);assert.deepEqual(s.state.entities.map(e=>e.id),ids);
});
test('all prepared experiment IDs instantiate and preserve portable histories',async()=>{
    const s=new ObserverSession();
    for(const preset of ['baseline','clocks','moving-shapes','clock-avenue','light-clock','twin-journey','point-collision','delayed-edit']) {
        const r=await s.command({type:'preset',preset});assert.ok(r.ok,r.error);advance(s,.2);assert.deepEqual(JSON.parse(JSON.stringify(s.snapshot())),s.snapshot());
    }
});
test('twin journey returns and has lower proper time',()=>{
    const s=new ObserverSession({preset:'twin-journey'});advance(s,20.1);const [home,twin]=s.state.entities;
    close(home.clockOffset,20.1,1e-8);close(twin.clockOffset,16.1,1e-8);close(twin.position[0],1,1e-8);
});
test('prepared relativistic collision conserves momentum',()=>{
    const s=new ObserverSession({preset:'point-collision'});
    const total=()=>s.state.entities.map(e=>fourMomentum(e.mass,e.velocity)).reduce((a,b)=>a.map((x,i)=>x+b[i]));
    const before=total();advance(s,5);assert.equal(s.state.collisionOccurred,true);total().forEach((x,i)=>close(x,before[i]));
    const collisionSegments=s.state.segments.filter(x=>x.start>0);assert.equal(collisionSegments.length,2);
    close(collisionSegments[0].start,4/.9);collisionSegments[0].position.forEach((x,i)=>close(x,collisionSegments[1].position[i]));
});
test('deleting a prepared twin prevents later optical resurrection at turnaround',async()=>{
    const s=new ObserverSession({preset:'twin-journey'}),id=s.state.entities[1].id;
    const result=await s.command({type:'delete',id});assert.ok(result.ok,result.error);
    assert.equal(s.state.preparedEvents,false);advance(s,10.25);
    assert.equal(s.state.entities.find(e=>e.id===id).alive,false);
    assert.ok(s.state.segments.filter(segment=>segment.entityId===id).every(segment=>segment.end===0));
    assert.ok(!s.state.segments.some(segment=>segment.entityId===id&&segment.start>0));
});
test('editing prepared collision participants prevents forced scripted contact',async()=>{
    const s=new ObserverSession({preset:'point-collision'}),id=s.state.entities[1].id;
    await s.command({type:'update',id,patch:{position:[20,1.6,-3],velocity:[0,0,0]}});
    assert.equal(s.state.preparedEvents,false);advance(s,5);
    assert.equal(s.state.collisionOccurred,false);assert.deepEqual(s.state.entities[1].velocity,[0,0,0]);
    assert.ok(!s.state.segments.some(segment=>segment.start>0));
});
test('branching before a recorded collision drops future demo bookkeeping',async()=>{
    const s=new ObserverSession({preset:'point-collision'});advance(s,5);assert.equal(s.state.collisionOccurred,true);
    await s.command({type:'scrub',time:2});
    await s.command({type:'environment',patch:{preset:'stars'}});
    assert.equal(s.state.time,2);assert.equal(s.state.collisionOccurred,false);assert.equal(s.state.preparedEvents,false);
    assert.ok(!s.state.segments.some(segment=>segment.start>2));
    await s.command({type:'play'});advance(s,3);assert.equal(s.state.collisionOccurred,false);
});
test('presentation-only overlay edits preserve prepared trajectories',async()=>{
    const s=new ObserverSession({preset:'twin-journey'}),id=s.state.entities[1].id;
    await s.command({type:'update',id,patch:{overlay:false}});assert.equal(s.state.preparedEvents,true);
    advance(s,10.25);assert.deepEqual(s.state.entities[1].velocity,[-.6,0,0]);
});
test('snapshot load restarts epoch and malformed loads are atomic',async()=>{
    const s=new ObserverSession(),saved=s.snapshot();await s.command({type:'update',id:s.state.entities[0].id,patch:{name:'Changed'}});
    assert.equal((await s.command({type:'load',snapshot:saved})).ok,true);assert.equal(s.state.entities[0].name,'Luminous sphere');assert.equal(s.state.epoch,2);
    const bad=s.snapshot();bad.entities[0].velocity=[Infinity,0,0];const before=s.snapshot();
    assert.equal((await s.command({type:'load',snapshot:bad})).ok,false);assert.deepEqual(s.snapshot(),before);
});
test('Rapier profile falls, bounces, accepts impulses/springs, and releases world',async()=>{
    const s=new ObserverSession({profile:'playground'});await s.initialize();const id=s.state.entities[0].id;
    advance(s,.2);assert.ok(s.state.entities[0].position[1]<1);
    assert.equal((await s.command({type:'impulse',id,impulse:[0,3,0]})).ok,true);advance(s,.1);assert.ok(s.state.entities[0].velocity[1]>0);
    assert.equal((await s.command({type:'joint',a:id,b:s.state.entities[1].id,restLength:3,stiffness:2,damping:1})).ok,true);
    assert.equal(s.state.joints.length,1);assert.equal((await s.command({type:'profile',profile:'sr'})).ok,true);assert.equal(s.playground,null);
    s.dispose();
});
test('Playground colliders cover every library shape and per-body/world gravity',async()=>{
    const s=new ObserverSession({profile:'playground'});await s.initialize();
    for(const shape of ['sphere','box','plane','disk','capsule','cylinder','cone','torus','tetrahedron','octahedron','icosahedron','dodecahedron','ellipsoid','pyramid','prism','wedge']){
        const r=await s.command({type:'create',entity:{shape,position:[20+s.state.entities.length*4,5,-10],size:[1,2,1.5],gravity:false}});assert.ok(r.ok,`${shape}: ${r.error}`);
    }
    assert.equal((await s.command({type:'gravity',value:[0,0,0]})).ok,true);advance(s,.1);
    assert.deepEqual(s.snapshot().gravity,[0,0,0]);
    const isolated=s.state.entities.at(-1);close(isolated.velocity[1],0,1e-7);
    assert.equal((await s.command({type:'settings',patch:{playbackSpeed:2,units:'light seconds'}})).ok,true);
    assert.equal(s.snapshot().playbackSpeed,2);assert.equal(s.snapshot().units,'light seconds');s.dispose();
});
test('profile switching retains authored geometry and compatible velocity, reports incompatible motion',async()=>{
    const s=new ObserverSession();await s.initialize();const id=s.state.entities[0].id;
    await s.command({type:'update',id,patch:{position:[7,8,9],velocity:[.6,0,0],name:'Authored'}});
    let r=await s.command({type:'profile',profile:'playground'});assert.ok(r.ok,r.error);
    assert.deepEqual(s.state.entities[0].position,[7,8,9]);assert.deepEqual(s.state.entities[0].velocity,[.6,0,0]);assert.equal(s.state.entities[0].name,'Authored');
    await s.command({type:'update',id,patch:{velocity:[10,0,0],angularVelocity:[0,2,0]}});
    r=await s.command({type:'profile',profile:'sr'});assert.ok(r.ok,r.error);assert.deepEqual(s.state.entities[0].position,[7,8,9]);
    assert.deepEqual(s.state.entities[0].velocity,[0,0,0]);assert.deepEqual(s.state.entities[0].angularVelocity,[0,0,0]);assert.ok(r.snapshot.warnings.some(s=>s.includes('0.99c')));
    assert.equal(s.state.preparedEvents,false);assert.equal(s.state.time,0);assert.equal(s.state.segments[0].start,-60);
    await s.command({type:'undo'});assert.equal(s.state.profile,'playground');assert.deepEqual(s.state.entities[0].velocity,[10,0,0]);s.dispose();
});
test('Playground observer uses ordinary acceleration and coordinate clock time',async()=>{
    const s=new ObserverSession({profile:'playground'});await s.initialize();
    const initialPosition=[...s.state.observer.position];
    advance(s,.5,{move:[1,0,0],speed:.8,acceleration:1});
    close(s.state.observer.velocity[0],.5);close(s.state.observer.position[0]-initialPosition[0],.125);
    close(s.state.observer.properTime,s.state.time);close(s.state.observer.properTime,.5);
    advance(s,1,{move:[1,0,0],speed:.8,acceleration:1});
    close(s.state.observer.velocity[0],.8);close(s.state.observer.properTime,s.state.time);
    s.dispose();
});
test('switching observer clock profiles resets the origin and isolates dilation',async()=>{
    const s=new ObserverSession();await s.initialize();
    await s.command({type:'observer',patch:{velocity:[.8,0,0]}});
    advance(s,1,{move:[1,0,0],speed:.8,acceleration:1});
    close(s.state.observer.properTime,.6);close(s.state.time,1);
    assert.equal((await s.command({type:'profile',profile:'playground'})).ok,true);
    close(s.state.time,0);close(s.state.observer.properTime,0);
    advance(s,1,{move:[1,0,0],speed:.8,acceleration:1});
    close(s.state.observer.properTime,1);close(s.state.time,1);
    assert.equal((await s.command({type:'profile',profile:'sr'})).ok,true);
    close(s.state.time,0);close(s.state.observer.properTime,0);
    advance(s,1,{move:[1,0,0],speed:.8,acceleration:1});
    close(s.state.observer.properTime,.6);close(s.state.time,1);
    await s.command({type:'reset'});close(s.state.time,0);close(s.state.observer.properTime,0);
    s.dispose();
});
test('load rejects invalid clocks, overlapping histories, and nonfinite settings',async()=>{
    const s=new ObserverSession();
    for(const corrupt of [snapshot=>{snapshot.observer.properTime=NaN;},snapshot=>{snapshot.segments.push({...snapshot.segments[0]});},snapshot=>{snapshot.environment.radius=Infinity;},snapshot=>{snapshot.entities[0].alive='yes';}]){
        const before=s.snapshot(),bad=s.snapshot();corrupt(bad);assert.equal((await s.command({type:'load',snapshot:bad})).ok,false);assert.deepEqual(s.snapshot(),before);
    }
});
test('imported SR history uses the same 0.99c velocity limit as authored objects',async()=>{
    const s=new ObserverSession();
    for(const beta of [.9901,.9999999]){
        const before=s.snapshot(),bad=s.snapshot();bad.segments[0].velocity=[beta,0,0];
        const result=await s.command({type:'load',snapshot:bad});assert.equal(result.ok,false);assert.match(result.error,/0\.99c/);assert.deepEqual(s.snapshot(),before);
    }
    const valid=s.snapshot();valid.segments[0].velocity=[.99,0,0];
    assert.equal((await s.command({type:'load',snapshot:valid})).ok,true);
});
test('backlog pause can resume and drain retained simulation time',async()=>{
    const s=new ObserverSession();s.advance(6);assert.equal(s.state.playing,false);close(s.snapshot().backlogSeconds,6);
    await s.command({type:'play'});for(let i=0;i<24;i++)s.advance(0);
    close(s.state.time,6);close(s.snapshot().backlogSeconds,0);
});
test('physical shell edits preserve historical configurations and same-time ownership',async()=>{
    const s=new ObserverSession();advance(s,1);
    await s.command({type:'environment',patch:{preset:'nested-cubes'}});
    await s.command({type:'environment',patch:{radius:20}});
    assert.equal(s.state.environmentHistory.length,2);close(s.state.environmentHistory[0].end,1);
    assert.equal(s.state.environmentHistory[0].environment.preset,'void');assert.equal(s.state.environmentHistory[1].environment.radius,20);
    await s.command({type:'scrub',time:.5});assert.equal(s.snapshot().environment.preset,'void');
    await s.command({type:'environment',patch:{preset:'stars'}});assert.equal(s.state.time,.5);assert.equal(s.state.environment.preset,'stars');
    assert.equal(s.state.environmentHistory.length,2);
});
test('physical environment history budget rejects new allocation atomically',async()=>{
    const s=new ObserverSession();
    for(let i=0;i<15;i++){advance(s,.1);assert.equal((await s.command({type:'environment',patch:{radius:10+i}})).ok,true);}
    advance(s,.1);const before=s.snapshot();
    assert.equal((await s.command({type:'environment',patch:{radius:99}})).ok,false);assert.deepEqual(s.snapshot(),before);
});
/** The real browser worker module runs with a small transport-only Node adapter. */
function workerFactory(url){
    const source=`import {parentPort} from 'node:worker_threads';
      globalThis.postMessage=message=>parentPort.postMessage(message);
      globalThis.addEventListener=(name,fn)=>{if(name==='message')parentPort.on('message',data=>fn({data}));};
      await import(${JSON.stringify(url.href)});`;
    const worker=new NodeWorker(new URL(`data:text/javascript,${encodeURIComponent(source)}`));
    return {postMessage:data=>worker.postMessage(data),terminate:()=>worker.terminate(),addEventListener:(type,listener)=>{
        if(type==='message')worker.on('message',data=>listener({data}));
        if(type==='error')worker.on('error',error=>listener({message:error.message}));
        if(type==='messageerror')worker.on('messageerror',listener);
    }};
}
test('real worker serializes commands and coalesced advances; disposal rejects requests',async()=>{
    const errors=[],client=new ObserverWorkerClient({workerFactory,onError:error=>errors.push(error)});
    await client.ready;
    const id=client.snapshot.entities[0].id;
    assert.equal((await client.request({type:'update',id,patch:{name:'Worker-owned'}})).ok,true);
    for(let i=0;i<20;i++)client.advance(.01,{move:[0,0,1]});
    while(client.advancePending||client.queuedSeconds>0)await new Promise(resolve=>setTimeout(resolve,5));
    close(client.snapshot.time,.2);assert.equal(client.snapshot.entities[0].name,'Worker-owned');
    assert.equal((await client.request({type:'preset',preset:'clocks'})).ok,true);
    assert.equal(client.snapshot.epoch,2);assert.equal(client.snapshot.time,0);
    client.dispose();await assert.rejects(client.request({type:'play'}),/disposed/);assert.equal(errors.length,0);
});
