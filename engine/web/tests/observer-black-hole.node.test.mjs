/* global structuredClone */
import {test} from 'node:test';
import assert from 'node:assert/strict';
import {blackHole,blackHoleShadowAngle,traceBlackHolePath,traceBlackHole} from '../js/observer/black-hole.js';
import {compactStar,starMetric,starApparatus,traceStarPath,starEnergy,advanceStarObserver,LIGHT_SPEED,NOMINAL_SOLAR_GM,BLACK_HOLE_METHOD} from '../js/observer/compact-star.js';
import {ObserverSession} from '../js/observer/session.js';
import {ObserverStorage} from '../js/observer/storage.js';
import {traceObserverRay} from '../js/observer/optics.js';
import {DEFAULT_SETTINGS} from '../js/observer/catalog.js';
import {length,normalize,sub,cross,gamma,scale} from '../js/observer/math.js';
const close=(a,b,tolerance=1e-9)=>assert.ok(Math.abs(a-b)<=tolerance,`${a} != ${b}, tolerance ${tolerance}`);
const observer=(position=[0,1.6,3],velocity=[0,0,0])=>({position,velocity,properTime:0,yaw:0,pitch:0,roll:0,worldline:1});
function quadrature(fn,a,b,n=20000){let sum=fn(a)+fn(b);for(let i=1;i<n;i++)sum+=(i%2?4:2)*fn(a+(b-a)*i/n);return sum*(b-a)/(3*n);}

test('black-hole horizon scaling is explicit and no emitting surface is prepared',()=>{
    const hole=blackHole(),session=new ObserverSession({preset:'black-hole',playing:false});
    close(hole.rs*hole.lengthUnitMeters,2*NOMINAL_SOLAR_GM*10/LIGHT_SPEED**2);
    close(hole.radius,hole.rs/4);close(hole.radiusKm,2*NOMINAL_SOLAR_GM*10/LIGHT_SPEED**2/1000);
    assert.ok(hole.observerBoundary>hole.radius);assert.equal(hole.sourceId,'');
    assert.equal(session.state.entities.length,0);assert.equal(session.state.segments.length,0);assert.equal(session.state.integratorVersion,BLACK_HOLE_METHOD);
    assert.equal(traceObserverRay(session.snapshot()),null);
    assert.throws(()=>blackHole(0));assert.throws(()=>blackHole(NaN));
});

test('null capture and escape bracket the exact Schwarzschild shadow on both photon-sphere sides',()=>{
    const hole=blackHole(),critical=1.5*Math.sqrt(3)*hole.rs;
    for(const rho of [3,1,.4,.15,.11]){
        const origin=[0,1.6,rho],m=starMetric(hole,origin),angle=blackHoleShadowAngle(hole,origin);
        close(Math.sin(angle),critical*m.lapse/m.areal);
        assert.equal(angle>Math.PI/2,m.areal<1.5*hole.rs);
        for(const fraction of [.99,1.01]){
            const a=angle*fraction,path=traceBlackHolePath(hole,origin,[Math.sin(a),0,-Math.cos(a)]);
            assert.equal(path.status,fraction<1?'captured':'escaped',`rho=${rho}, fraction=${fraction}`);
        }
    }
});

test('radial sky delays terminate at the exact finite shell for both Schwarzschild preparations',()=>{
    for(const metric of [blackHole(),compactStar()]){
        const origin=[0,1.6,3],path=metric.kind==='schwarzschild-black-hole'?traceBlackHolePath(metric,origin,[0,0,1]):traceStarPath(metric,origin,[0,0,1],starApparatus(metric));
        const ro=starMetric(metric,origin).areal,re=starMetric(metric,[0,1.6,metric.escapeRadius]).areal;
        assert.equal(path.status,'escaped');close(length(sub(path.position,metric.center)),metric.escapeRadius,2e-12);
        close(path.delay,re-ro+metric.rs*Math.log((re-metric.rs)/(ro-metric.rs)),2e-6);
        close(path.distance,metric.escapeRadius-3,2e-12);
    }
});

test('escaped oblique null rays agree with independent areal-coordinate quadrature and impact invariant',()=>{
    const hole=blackHole(),origin=[0,1.6,3],direction=normalize([.2,0,1]),path=traceBlackHolePath(hole,origin,direction),start=starMetric(hole,origin),end=starMetric(hole,path.position);
    assert.equal(path.status,'escaped');
    const b=start.index*start.rho*length(cross([0,0,1],direction)),root=r=>Math.sqrt(1-b*b*(1-hole.rs/r)/(r*r));
    const delay=quadrature(r=>1/((1-hole.rs/r)*root(r)),start.areal,end.areal),angle=quadrature(r=>b/(r*r*root(r)),start.areal,end.areal);
    close(path.delay,delay,2e-6);close(Math.atan2(path.position[0],path.position[2]),angle,2e-8);
    close(path.impactParameter,b,1e-12);close(end.index*end.rho*length(cross(normalize(sub(path.position,hole.center)),path.direction)),b,1e-8);
});

test('critical and budget-limited rays are unresolved and do not invent horizon emission',()=>{
    const hole=blackHole(),origin=[0,1.6,3],angle=blackHoleShadowAngle(hole,origin);
    assert.equal(traceBlackHolePath(hole,origin,[Math.sin(angle),0,-Math.cos(angle)]).status,'budget-exhausted');
    assert.equal(traceBlackHolePath(hole,origin,[0,0,1],{maxSteps:0}).status,'budget-exhausted');
    assert.throws(()=>traceBlackHolePath(hole,origin,[0,0,1],{maxSteps:1.5}));
    assert.throws(()=>traceBlackHolePath(hole,origin,[0,0,0]),/finite nonzero/);
    assert.throws(()=>traceBlackHolePath(compactStar(),origin,[0,0,1]),/black-hole provider/);
    const session=new ObserverSession({preset:'black-hole',playing:false}),captured=traceBlackHole(session.snapshot());
    assert.equal(captured.status,'captured');assert.equal(captured.doppler,null);assert.equal(captured.emissionTime,null);assert.equal(captured.properTime,null);assert.equal(captured.emissionLapse,null);
});

test('finite-shell received frequency combines gravitational lapse with local SR without inventing a source clock',()=>{
    const session=new ObserverSession({preset:'black-hole',playing:false}),snapshot=session.snapshot(),hole=snapshot.spacetime;
    snapshot.time=50;snapshot.observer.yaw=Math.PI;snapshot.observer.velocity=[0,0,.6];
    const ray=traceBlackHole(snapshot),Ao=starMetric(hole,snapshot.observer.position).lapse,Ae=starMetric(hole,[0,1.6,hole.escapeRadius]).lapse;
    assert.equal(ray.status,'escaped');close(ray.doppler,Ae/Ao*gamma(.6)*1.6,2e-12);
    close(ray.emissionTime,snapshot.time-ray.delay,2e-12);assert.equal(ray.properTime,null);assert.equal(ray.source,'Float64 CPU geodesic');
    assert.deepEqual(ray.observerPosition,snapshot.observer.position);close(ray.observerYaw,Math.PI);
});

test('black-hole free fall conserves energy and angular momentum while ignoring guided keyboard forces',()=>{
    const hole=blackHole();hole.observerMode='freefall';let o=observer([0,1.6,1],[.1,0,0]);
    const E=starEnergy(hole,o.position,o.velocity),L=cross(sub(o.position,hole.center),scale(o.velocity,starMetric(hole,o.position).spatial*gamma(o.velocity)));
    for(let i=0;i<150;i++){const next=advanceStarObserver(hole,o,1/120,[1,1,1]);assert.equal(next.stopped,false);o={...o,position:next.position,velocity:next.velocity,properTime:o.properTime+next.properElapsed};}
    close(starEnergy(hole,o.position,o.velocity),E,2e-10);
    const Ln=cross(sub(o.position,hole.center),scale(o.velocity,starMetric(hole,o.position).spatial*gamma(o.velocity)));L.forEach((v,i)=>close(v,Ln[i],2e-10));
    const session=new ObserverSession({preset:'black-hole',playing:false});session.state.spacetime.observerMode='freefall';
    const before=structuredClone(session.state.observer),expected=advanceStarObserver(session.state.spacetime,before,.1);
    session.step(.1,{move:[1,0,0],speed:.9,acceleration:100});expected.position.forEach((v,i)=>close(session.state.observer.position[i],v));
    assert.ok(session.state.observer.velocity[2]<0);
});

test('local SR navigation limit agrees with flat-space dynamics and curved clock rate is labeled by the lapse',()=>{
    const flat=compactStar(0),o=observer([0,1.6,3],[.3,0,0]),next=advanceStarObserver(flat,o,.1);
    close(next.position[0],.03);close(next.properElapsed,.1/gamma(o.velocity));
    const hole=blackHole(),staticObserver=observer(),held=advanceStarObserver(hole,staticObserver,.1);
    close(held.properElapsed,.1*starMetric(hole,staticObserver.position).lapse,1e-12);
    assert.ok(held.properElapsed<.1);close(length(held.velocity),0);
});

test('guided approach stops outside the numerical horizon guard and the paused endpoint remains portable',async()=>{
    const session=new ObserverSession({preset:'black-hole',playing:false}),hole=session.state.spacetime;
    assert.equal((await session.command({type:'observer',patch:{position:[0,1.6,hole.observerBoundary+1e-5],velocity:[0,0,-.2]}})).ok,true);
    session.state.playing=true;session.step(.1,{move:[0,0,1],speed:.2});
    assert.equal(session.state.playing,false);assert.match(session.state.warnings[0],/exterior numerical guard/);
    assert.ok(starMetric(hole,session.state.observer.position).rho>hole.observerBoundary);
    const storage=new ObserverStorage(),portable=storage.exportWorld(session.snapshot(),DEFAULT_SETTINGS),restored=storage.importWorld(portable);
    assert.equal((await session.command({type:'load',snapshot:restored.snapshot})).ok,true);
});

test('black-hole exterior import, locked controls, numerical boundaries and preparation switching preserve model separation',async()=>{
    const session=new ObserverSession({preset:'black-hole',playing:false}),storage=new ObserverStorage();
    session.step(.1,{});const portable=storage.exportWorld(session.snapshot(),DEFAULT_SETTINGS),restored=storage.importWorld(portable);
    assert.equal((await session.command({type:'load',snapshot:restored.snapshot})).ok,true);
    const corrupt=session.snapshot();corrupt.spacetime.observerBoundary*=2;assert.equal((await session.command({type:'load',snapshot:corrupt})).ok,false);
    const emitter=session.snapshot();emitter.spacetime.sourceId='fake-horizon';assert.equal((await session.command({type:'load',snapshot:emitter})).ok,false);
    assert.equal((await session.command({type:'create',entity:{shape:'sphere'}})).ok,false);
    assert.equal((await session.command({type:'observer',patch:{position:[0,1.6,.0625]}})).ok,false);
    assert.equal((await session.command({type:'scrub',time:0})).ok,false);
    const hole=session.state.spacetime;hole.observerMode='freefall';session.state.observer=observer([0,1.6,.3]);session.state.playing=true;
    for(let i=0;i<4000&&session.state.playing;i++)session.step(1/120,{});
    assert.equal(session.state.playing,false);assert.match(session.state.warnings[0],/0\.99c local speed limit/);assert.ok(length(session.state.observer.velocity)<=.99+1e-12);
    assert.ok(starMetric(hole,session.state.observer.position).rho>hole.observerBoundary);
    await session.command({type:'preset',preset:'compact-star'});assert.equal(session.state.entities.length,1);assert.equal(session.state.spacetime.kind,'schwarzschild-exterior');assert.ok(traceObserverRay(session.snapshot()));
    await session.command({type:'preset',preset:'baseline'});assert.equal(session.state.spacetime,undefined);assert.equal(session.state.physicsEngine,'Minkowski reference c=1');
});
