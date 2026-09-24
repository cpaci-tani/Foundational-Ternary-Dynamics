import test from 'node:test';
import assert from 'node:assert/strict';
import { ObserverSession } from '../js/observer/session.js';
import { reflectPolar } from '../js/observer/mirror-frame.js';

const close=(a,b,tolerance=2e-5)=>assert.ok(Math.abs(a-b)<tolerance,`${a} != ${b}`);
const vectorClose=(a,b,tolerance)=>a.forEach((value,i)=>close(value,b[i],tolerance));
const advance=(session,time)=>{for(let t=0;t<time-1e-10;t+=1/120)session.advance(1/120);};
async function emptyWorld(settings={}) {
    const session=new ObserverSession({profile:'playground'});
    session.state.entities=[];session.state.segments=[];
    await session.initialize();
    const result=await session.command({type:'world-physics',patch:settings});assert.ok(result.ok,result.error);
    return session;
}
async function create(session,properties) {
    const result=await session.command({type:'create',entity:{shape:'sphere',size:[1,1,1],damping:0,friction:0,restitution:1,...properties}});
    assert.ok(result.ok,result.error);return result.snapshot.entities.at(-1).id;
}
const entity=(session,id)=>session.state.entities.find(item=>item.id===id);

test('plane gravity is the new default, and lower-source fall and bounce mirror upper-source dynamics',async()=>{
    const session=await emptyWorld({});
    try {
        assert.equal(session.state.gravityMode,'plane');assert.equal(session.state.gravityStrength,9.81);
        assert.equal(session.state.objectCollisions,true);assert.equal(session.state.planeCollision,true);
        const above=await create(session,{position:[-2,3,0]}),below=await create(session,{position:[2,-3,0]});
        assert.equal(session.playground.bodies.size,2,'reflections never allocate another independently integrated body');
        advance(session,.25);
        const a=entity(session,above),b=entity(session,below);
        close(a.position[1],-b.position[1]);close(a.velocity[1],-b.velocity[1]);
        close(a.velocity[1],-9.81*.25,2e-4);
        assert.ok(a.position[1]<3&&b.position[1]>-3);
        let bounced=false;
        for(let i=0;i<120;i++){
            advance(session,1/120);
            close(a.position[1],-b.position[1]);close(a.velocity[1],-b.velocity[1]);
            assert.ok(a.position[1]>.45&&b.position[1]<-.45);
            if(a.velocity[1]>1&&b.velocity[1]<-1)bounced=true;
        }
        assert.ok(bounced,'both faces bounce away from the shared plane');
    } finally {session.dispose();}
});

for(const collisions of [true,false])test(`opposite-side authored bodies ${collisions?'collide through their linked images':'pass when object collisions are disabled'}`,async()=>{
    const session=await emptyWorld({gravityStrength:0,objectCollisions:collisions});
    try{
        const a=await create(session,{position:[-2,2,0],velocity:[2,0,0]});
        const b=await create(session,{position:[2,-2,0],velocity:[-2,0,0]});
        advance(session,1.2);
        const left=entity(session,a),right=entity(session,b);
        close(left.velocity[0]+right.velocity[0],0,1e-4);
        if(collisions){assert.ok(left.velocity[0]<-1.9);assert.ok(right.velocity[0]>1.9);assert.ok(left.position[0]<right.position[0]);}
        else {close(left.velocity[0],2);close(right.velocity[0],-2);assert.ok(left.position[0]>right.position[0]);}
        assert.equal(session.playground.bodies.size,2);
    }finally{session.dispose();}
});

test('per-object ghosting disables both body and plane contacts without disabling gravity',async()=>{
    const session=await emptyWorld({gravityStrength:0});
    try{
        const ghost=await create(session,{position:[-2,2,0],velocity:[2,0,0],collisions:false});
        await create(session,{position:[2,-2,0],velocity:[-2,0,0]});
        advance(session,1.2);close(entity(session,ghost).velocity[0],2);
        const crossing=await create(session,{position:[10,.2,0],velocity:[0,-2,0],collisions:false});
        advance(session,.25);assert.ok(entity(session,crossing).position[1]<-.2);close(entity(session,crossing).velocity[1],-2);
        await session.command({type:'world-physics',patch:{gravityStrength:2}});
        const before=entity(session,crossing).velocity[1];advance(session,.1);
        assert.ok(entity(session,crossing).velocity[1]>before,'gravity still pulls a below-plane ghost upward');
    }finally{session.dispose();}
});

test('the plane remains collidable when object collisions are disabled',async()=>{
    const session=await emptyWorld({objectCollisions:false});
    try{
        const id=await create(session,{position:[0,2,0]});
        let bounce=false;
        for(let i=0;i<180;i++){advance(session,1/120);const e=entity(session,id);assert.ok(e.position[1]>.45);if(e.velocity[1]>1)bounce=true;}
        assert.ok(bounce);
    }finally{session.dispose();}
});

for(const shape of ['cone','wedge'])test(`open-plane crossing preserves the source frame for a rotating ${shape}`,async()=>{
    const folded=await emptyWorld({gravityStrength:0,planeCollision:false,objectCollisions:false});
    const uniform=await emptyWorld({gravityMode:'uniform',gravity:[0,0,0],planeCollision:false,objectCollisions:false});
    try{
        const properties={shape,position:[0,.1,0],size:[1,2,1],rotation:[.21,-.33,.17],velocity:[.2,-1,.1],angularVelocity:[.6,.4,-.2],gravity:false};
        const id=await create(folded,properties),reference=await create(uniform,properties);
        advance(folded,.3);advance(uniform,.3);
        const a=entity(folded,id),b=entity(uniform,reference);
        assert.ok(a.position[1]<0);assert.equal(folded.playground.sides.get(id),-1);
        vectorClose(a.position,b.position);vectorClose(a.velocity,b.velocity);
        // Rapier computes convex inertia and gyroscopic updates in Float32;
        // rebuilding a reflected hull can perturb angular rates by < 1e-4.
        vectorClose(a.angularVelocity,b.angularVelocity,1e-4);vectorClose(a.rotation,b.rotation);
        assert.equal(folded.playground.bodies.size,1);
        close(folded.playground.bodies.get(id).mass(),1);
        const sourceCollider=uniform.playground.bodies.get(reference).collider(0),imageCollider=folded.playground.bodies.get(id).collider(0),R=folded.playground.api;
        for(const direction of [[1,0,0],[0,1,0],[0,0,1]]){
            const origin=b.position.map((n,i)=>n-direction[i]*5),vec=a=>({x:a[0],y:a[1],z:a[2]});
            const source=sourceCollider.castRayAndGetNormal(new R.Ray(vec(origin),vec(direction)),10,true);
            const image=imageCollider.castRayAndGetNormal(new R.Ray(vec(reflectPolar(origin)),vec(reflectPolar(direction))),10,true);
            assert.ok(source&&image);close(source.timeOfImpact,image.timeOfImpact);
        }
        const impulse=await folded.command({type:'impulse',id,impulse:[0,2,0]});assert.ok(impulse.ok,impulse.error);
        close(entity(folded,id).velocity[1],1);
        advance(folded,.4);assert.ok(entity(folded,id).position[1]>0);assert.equal(folded.playground.sides.get(id),1);
    }finally{folded.dispose();uniform.dispose();}
});

test('prescribed velocity moves a kinematic body through the folded plane without gravity acceleration',async()=>{
    const session=await emptyWorld({gravityStrength:20,planeCollision:false});
    try{
        const id=await create(session,{bodyType:'kinematic',position:[0,-.1,0],velocity:[.3,1,.2]});
        advance(session,.3);
        vectorClose(entity(session,id).position,[.09,.2,.06]);vectorClose(entity(session,id).velocity,[.3,1,.2]);
        assert.equal(session.playground.sides.get(id),1);
        assert.equal((await session.command({type:'impulse',id,impulse:[0,1,0]})).ok,false);
    }finally{session.dispose();}
});

for(const shape of ['cone','pyramid','wedge'])test(`folded ${shape} collider reflects local geometry and orientation exactly`,async()=>{
    const folded=await emptyWorld({gravityStrength:0,planeCollision:false});
    const uniform=await emptyWorld({gravityMode:'uniform',gravity:[0,0,0],planeCollision:false});
    try{
        const properties={shape,position:[0,-3,0],size:[1,2,1.5],rotation:[.3,.2,.4],gravity:false};
        const id=await create(folded,properties),reference=await create(uniform,properties);
        const a=folded.playground.bodies.get(id).collider(0),b=uniform.playground.bodies.get(reference).collider(0);
        const R=folded.playground.api;
        for(const outward of [[1,0,0],[-1,0,0],[0,1,0],[0,-1,0],[0,0,1],[0,0,-1]]){
            const origin=properties.position.map((v,i)=>v+outward[i]*5),direction=outward.map(v=>-v);
            const v=a=>({x:a[0],y:a[1],z:a[2]});
            const source=b.castRayAndGetNormal(new R.Ray(v(origin),v(direction)),10,true);
            const image=a.castRayAndGetNormal(new R.Ray(v(reflectPolar(origin)),v(reflectPolar(direction))),10,true);
            assert.ok(source&&image,`both colliders intersect along ${outward}`);
            close(source.timeOfImpact,image.timeOfImpact);
            vectorClose([image.normal.x,image.normal.y,image.normal.z],reflectPolar([source.normal.x,source.normal.y,source.normal.z]));
        }
    }finally{folded.dispose();uniform.dispose();}
});

test('uniform legacy gravity keeps its vector and a below-plane body is not ejected by the opposite halfspace',async()=>{
    const session=await emptyWorld({});
    try{
        const id=await create(session,{position:[0,-3,0]});
        assert.ok((await session.command({type:'gravity',value:[1,-2,3]})).ok);
        assert.equal(session.state.gravityMode,'uniform');assert.deepEqual(session.state.gravity,[1,-2,3]);
        advance(session,.1);
        const e=entity(session,id);assert.ok(e.position[1]<-3);vectorClose(e.velocity,[.1,-.2,.3],1e-5);
        const noGravity=await create(session,{position:[5,-3,0],gravity:false});advance(session,.1);
        vectorClose(entity(session,noGravity).velocity,[0,0,0]);
    }finally{session.dispose();}
});

test('world controls and entity collisions survive undo, snapshots, reset and profile transitions',async()=>{
    const session=await emptyWorld({gravityStrength:4,objectCollisions:false,planeCollision:false});
    const restored=new ObserverSession();
    try{
        const id=await create(session,{position:[0,-3,0],collisions:false});
        const before=session.snapshot();
        assert.ok((await session.command({type:'world-physics',patch:{gravityMode:'uniform',gravity:[0,2,0],gravityStrength:2,objectCollisions:true,planeCollision:true}})).ok);
        assert.ok((await session.command({type:'undo'})).ok);
        for(const key of ['gravityMode','gravityStrength','objectCollisions','planeCollision','gravity'])assert.deepEqual(session.state[key],before[key]);
        assert.ok((await restored.command({type:'load',snapshot:JSON.parse(JSON.stringify(session.snapshot()))})).ok);
        assert.equal(entity(restored,id).collisions,false);assert.equal(restored.state.gravityMode,'plane');
        await session.command({type:'profile',profile:'sr'});assert.equal(session.state.gravityStrength,4);
        await session.command({type:'profile',profile:'playground'});assert.equal(entity(session,id).collisions,false);
        await session.command({type:'reset'});assert.equal(session.state.gravityStrength,4);assert.equal(session.state.planeCollision,false);
    }finally{session.dispose();restored.dispose();}
});

test('old schema-1 worlds explicitly migrate to uniform gravity and collisions enabled',async()=>{
    const session=await emptyWorld({gravityMode:'uniform',gravity:[1,2,3]});
    const restored=new ObserverSession();
    try{
        await create(session,{position:[0,2,0]});const legacy=session.snapshot();
        for(const key of ['gravityMode','gravityStrength','objectCollisions','planeCollision'])delete legacy[key];
        for(const e of legacy.entities)delete e.collisions;
        const result=await restored.command({type:'load',snapshot:legacy});assert.ok(result.ok,result.error);
        assert.equal(restored.state.gravityMode,'uniform');close(restored.state.gravityStrength,Math.sqrt(14));
        assert.deepEqual(restored.state.gravity,[1,2,3]);assert.equal(restored.state.objectCollisions,true);assert.equal(restored.state.planeCollision,true);
        assert.equal(restored.state.entities[0].collisions,true);
    }finally{session.dispose();restored.dispose();}
});

test('invalid world physics commands and imports reject atomically; SR exposes no classical gravity',async()=>{
    const session=await emptyWorld({}),sr=new ObserverSession();
    try{
        for(const patch of [{gravityMode:'other'},{gravityStrength:-1},{gravityStrength:NaN},{objectCollisions:0},{planeCollision:null},{gravity:[0,1]},{unknown:true}]){
            const before=session.snapshot(),result=await session.command({type:'world-physics',patch});
            assert.equal(result.ok,false);assert.deepEqual(session.snapshot(),before);
        }
        for(const patch of [{gravityMode:null},{gravityStrength:-1},{objectCollisions:'false'},{planeCollision:null}]){
            const before=session.snapshot(),result=await session.command({type:'load',snapshot:{...before,...patch}});
            assert.equal(result.ok,false);assert.deepEqual(session.snapshot(),before);
        }
        const before=sr.snapshot();assert.equal((await sr.command({type:'world-physics',patch:{gravityStrength:1}})).ok,false);assert.deepEqual(sr.snapshot(),before);
    }finally{session.dispose();sr.dispose();}
});
