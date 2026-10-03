/* global structuredClone */
import {test} from 'node:test';
import assert from 'node:assert/strict';
import {compactStar,starMetric,starEnergy,advanceStarObserver,traceStarPath,starApparatus,LIGHT_SPEED,NOMINAL_SOLAR_GM,COMPACT_STAR_METHOD} from '../js/observer/compact-star.js';
import {ObserverSession} from '../js/observer/session.js';
import {traceObserverRay} from '../js/observer/optics.js';
import {observationDiagnostics} from '../js/observer/diagnostics.js';
import {ObserverStorage} from '../js/observer/storage.js';
import {DEFAULT_SETTINGS} from '../js/observer/catalog.js';
import {createObserverControl} from '../js/assistant/observer-control.js';
import {dot,cross,sub,scale,length,gamma,normalize} from '../js/observer/math.js';
const close=(a,b,tolerance=1e-9)=>assert.ok(Math.abs(a-b)<=tolerance,`${a} != ${b}, tolerance ${tolerance}`);
const observer=(position=[0,1.6,3],velocity=[0,0,0])=>({position,velocity,properTime:0,yaw:0,pitch:0,roll:0,worldline:1});
// Independent areal-coordinate Simpson quadrature, never using production ODEs.
function quadrature(fn,a,b,n=20000){let sum=fn(a)+fn(b);for(let i=1;i<n;i++)sum+=(i%2?4:2)*fn(a+(b-a)*i/n);return sum*(b-a)/(3*n);}

test('physical scaling, surface metric and horizon restrictions are explicit',()=>{
    const s=compactStar(),m=starMetric(s,[s.radius,1.6,0]);
    close(s.lengthUnitMeters,48000);close(s.rs*s.lengthUnitMeters,2*NOMINAL_SOLAR_GM*1.4/LIGHT_SPEED**2);
    close(m.areal,.25);close(m.lapse,Math.sqrt(1-s.rs/.25));
    close(1/m.lapse-1,1/Math.sqrt(1-2*NOMINAL_SOLAR_GM*1.4/(12000*LIGHT_SPEED**2))-1);
    assert.throws(()=>compactStar(10,1),/photon sphere/);assert.throws(()=>compactStar(-1,12));assert.throws(()=>compactStar(1.4,NaN));
    // A physically hand-sized object at the same mean density has tiny compactness.
    const small=compactStar(1.4*(.1/12000)**3,.0001),sm=starMetric(small,[small.radius,1.6,0]);
    assert.ok(1-sm.lapse<2e-11);
});

test('radial null travel time agrees with the exact Schwarzschild Shapiro delay',()=>{
    const s=compactStar(),path=traceStarPath(s,[0,1.6,3],[0,0,-1],[starApparatus(s)[0]]),ro=starMetric(s,[0,1.6,3]).areal;
    const expected=ro-.25+s.rs*Math.log((ro-s.rs)/(.25-s.rs));
    assert.equal(path.status,'hit');close(path.delay,expected,2e-6);close(path.distance,3-s.radius,2e-7);
    assert.ok(path.delay>path.distance);
});

test('oblique null bending and travel time agree with independent areal quadrature',()=>{
    const s=compactStar(),origin=[0,1.6,3],d=normalize([.06,0,-1]),m=starMetric(s,origin),b=m.index*m.rho*length(cross([0,0,1],d));
    const path=traceStarPath(s,origin,d,[starApparatus(s)[0]]);
    assert.equal(path.status,'hit');
    const root=r=>Math.sqrt(1-b*b*(1-s.rs/r)/(r*r));
    const delay=quadrature(r=>1/((1-s.rs/r)*root(r)),.25,m.areal);
    const angle=quadrature(r=>b/(r*r*root(r)),.25,m.areal);
    close(path.delay,delay,2e-6);close(Math.atan2(path.position[0],path.position[2]),angle,2e-6);
    const end=starMetric(s,path.position),invariant=end.index*end.rho*length(cross(normalize(sub(path.position,s.center)),path.direction));
    close(invariant,b,2e-7);
});

test('lensed stellar silhouette follows the exact impact-parameter limit',()=>{
    const s=compactStar(),origin=[0,1.6,3],m=starMetric(s,origin),As=Math.sqrt(1-s.rs/.25);
    const sine=(.25/As)/(m.areal/m.lapse),angle=Math.asin(sine),star=[starApparatus(s)[0]];
    for(const fraction of [.999,1.001]){
        const a=angle*fraction,path=traceStarPath(s,origin,[Math.sin(a),0,-Math.cos(a)],star);
        assert.equal(path.status,fraction<1?'hit':'escaped');
    }
    assert.ok(angle>Math.asin(s.radius/3));
});

test('floating preparation contains only the emitting sphere and no support-ray hits',()=>{
    const session=new ObserverSession({preset:'compact-star',playing:false}),snapshot=session.snapshot();
    assert.equal(snapshot.entities.length,1);assert.equal(snapshot.segments.length,1);
    assert.equal(snapshot.entities[0].shape,'sphere');assert.equal(snapshot.entities[0].id,snapshot.spacetime.sourceId);
    const hit=traceObserverRay(snapshot,{fov:60,aspect:1},0,0);
    assert.equal(hit.entityId,snapshot.spacetime.sourceId);
    // These downward rays previously hit the pedestal stem; the floating source
    // leaves the same rays unobstructed without changing the metric or clocks.
    for(const y of [-.22,-.4,-.7])assert.equal(traceObserverRay(snapshot,{fov:60,aspect:1},0,y),null);
});

test('zero mass gives straight null rays and local SR motion and clock rates',()=>{
    const s=compactStar(0),o=observer([0,1.6,3],[.3,0,0]);
    const next=advanceStarObserver(s,o,.1,[0,0,0]);
    close(next.position[0],.03);close(next.position[2],3);close(next.properElapsed,.1/gamma(o.velocity));
    const path=traceStarPath(s,[0,1.6,3],[0,0,-1],[starApparatus(s)[0]]);
    close(path.delay,2.75,2e-7);close(path.distance,path.delay);
    const forced=advanceStarObserver(s,observer(),.3,[.4,0,0]),u=.4*.3;
    close(forced.position[0],(Math.sqrt(1+u*u)-1)/.4,2e-12);
    close(forced.properElapsed,Math.asinh(u)/.4,2e-12);
});

test('supported clocks and received frequencies combine gravitational and local SR factors',()=>{
    const session=new ObserverSession({preset:'compact-star',playing:false}),s=session.state.spacetime,m=starMetric(s,session.state.observer.position);
    for(let i=0;i<120;i++)session.step(1/120,{});
    close(session.state.observer.properTime,m.lapse,1e-12);
    const snapshot=session.snapshot(),hit=traceObserverRay(snapshot),As=Math.sqrt(1-s.rs/.25);
    close(hit.doppler,As/m.lapse);close(hit.properTime,As*hit.emissionTime);
    snapshot.observer.velocity=[0,0,-.6];const moving=traceObserverRay(snapshot);
    close(moving.doppler,As/m.lapse*gamma(.6)*1.6);
    const diagnostics=observationDiagnostics(snapshot,{},moving);
    assert.equal(diagnostics.spacetime.kind,'schwarzschild-exterior');assert.match(diagnostics.velocityConvention,/local/);
});

test('free fall conserves Killing energy and angular momentum',()=>{
    const s=compactStar();s.observerMode='freefall';let o=observer([0,1.6,1],[.1,0,0]);
    const E=starEnergy(s,o.position,o.velocity),L=cross(sub(o.position,s.center),scale(o.velocity,starMetric(s,o.position).spatial*gamma(o.velocity)));
    for(let i=0;i<500;i++){const next=advanceStarObserver(s,o,1/120);o={...o,position:next.position,velocity:next.velocity,properTime:o.properTime+next.properElapsed};assert.equal(next.stopped,false);}
    close(starEnergy(s,o.position,o.velocity),E,2e-11);
    const Ln=cross(sub(o.position,s.center),scale(o.velocity,starMetric(s,o.position).spatial*gamma(o.velocity)));
    L.forEach((v,i)=>close(v,Ln[i],2e-11));assert.ok(o.properTime<500/120);
});

test('radial free fall matches its exact energy-speed relation and stops at the surface',()=>{
    const s=compactStar();s.observerMode='freefall';let o=observer([0,1.6,.6]);const A0=starMetric(s,o.position).lapse;let stopped=false;
    for(let i=0;i<2000;i++){
        const next=advanceStarObserver(s,o,1/120);o={...o,position:next.position,velocity:next.velocity,properTime:o.properTime+next.properElapsed};
        close(dot(o.velocity,o.velocity),1-(starMetric(s,o.position).lapse/A0)**2,2e-10);
        if(next.stopped){stopped=true;break;}
    }
    assert.ok(stopped);assert.ok(length(sub(o.position,s.center))>s.radius);close(length(sub(o.position,s.center)),s.radius,3e-7);
});

test('session preparation, commands, history cutoff, persistence and model separation',async()=>{
    const session=new ObserverSession({preset:'compact-star',playing:false});
    assert.equal(session.state.entities.length,1);assert.equal(session.state.integratorVersion,COMPACT_STAR_METHOD);
    assert.equal(session.state.segments[0].integratorVersion,COMPACT_STAR_METHOD);
    assert.equal((await session.command({type:'update',id:session.state.entities[0].id,patch:{mass:2}})).ok,false);
    assert.equal((await session.command({type:'observer',patch:{position:[0,1.6,0]}})).ok,false);
    assert.equal((await session.command({type:'scrub',time:-1})).ok,false);
    session.step(.1,{});assert.equal((await session.command({type:'load',snapshot:session.snapshot()})).ok,true);
    const storage=new ObserverStorage(),portable=storage.exportWorld(session.snapshot(),DEFAULT_SETTINGS),restored=storage.importWorld(portable);
    assert.deepEqual(restored.snapshot.spacetime,session.state.spacetime);
    const bad=JSON.parse(portable);bad.snapshot.entities[0].size[0]*=2;assert.throws(()=>storage.importWorld(JSON.stringify(bad)),/locked/);
    const corrupt=session.snapshot();corrupt.spacetime.rs*=2;assert.equal((await session.command({type:'load',snapshot:corrupt})).ok,false);
    const badClock=session.snapshot();badClock.segments[0].clockOffset=3;assert.equal((await session.command({type:'load',snapshot:badClock})).ok,false);
    const oldPedestal=session.snapshot();oldPedestal.entities.push(...Array.from({length:3},(_,i)=>({...structuredClone(oldPedestal.entities[0]),id:`old-pedestal-${i}`})));oldPedestal.segments.push(...Array.from({length:3},(_,i)=>({...structuredClone(oldPedestal.segments[0]),entityId:`old-pedestal-${i}`})));
    const rejected=await session.command({type:'load',snapshot:oldPedestal});assert.equal(rejected.ok,false);assert.match(rejected.error,/obsolete pedestal/);
    const oldPortable=JSON.parse(portable);oldPortable.snapshot=oldPedestal;assert.throws(()=>storage.importWorld(JSON.stringify(oldPortable)),/obsolete pedestal/);
    assert.equal(session.state.entities.length,1);assert.equal(session.state.segments.length,1);
    assert.equal((await session.command({type:'compact-star-motion',mode:'freefall'})).ok,true);
    session.step(.1,{});assert.ok(session.state.observer.velocity[2]<0);
    const tau=session.state.observer.properTime;await session.command({type:'compact-star-motion',mode:'guided'});close(session.state.observer.properTime,tau);close(length(session.state.observer.velocity),0);
    const snapshot=session.snapshot();snapshot.historyStart=snapshot.time-1;assert.equal(traceObserverRay(snapshot),null);
    assert.equal(observationDiagnostics(snapshot,{},null).historyStatus,'before-history-start');
    await session.command({type:'preset',preset:'baseline'});assert.equal(session.state.spacetime,undefined);assert.equal(session.state.physicsEngine,'Minkowski reference c=1');
    session.state.observer.position[1]=1;assert.ok(traceObserverRay(session.snapshot()));
});

test('assistant observations identify the GR provider and omit locked authoring actions',()=>{
    const session=new ObserverSession({preset:'compact-star',playing:false}),workspace={snapshot:session.snapshot(),settings:DEFAULT_SETTINGS,selectedId:null,selectedHit:null,hit:null,authorMirrored:false,disposed:false};
    const control=createObserverControl({getWorkspace:()=>workspace,isActive:()=>true}),observed=control.observe();
    assert.equal(observed.facts.spacetime.massSolar,1.4);assert.match(observed.facts.referenceBoundary,/Schwarzschild/);
    assert.ok(!observed.capabilities.includes('observer.create'));assert.ok(observed.capabilities.includes('observer.preset'));
    control.dispose();
});
