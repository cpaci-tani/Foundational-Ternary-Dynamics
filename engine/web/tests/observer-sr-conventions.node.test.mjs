import test from 'node:test';
import assert from 'node:assert/strict';
import { ObserverSession } from '../js/observer/session.js';
import { gamma, dot, scale, add, integrateFourVelocity } from '../js/observer/math.js';
import { boost, projectPoint } from '../js/observer/optics.js';
import { boostEvent, inverseBoost, intervalSquared, retardedTime, normalize, sub, dopplerFactor, observerRayToWorld, fourMomentum, elasticCollision1D } from '../js/observer/math.js';

const close = (actual, expected, tolerance = 1e-11) => assert.ok(Math.abs(actual - expected) <= tolerance, `${actual} != ${expected} (tol ${tolerance})`);
const vectorClose = (a, b, tolerance) => a.forEach((x, i) => close(x, b[i], tolerance));

// Independent closed-form integral of a constant coordinate force, including a
// rationalized gamma difference to avoid cancellation in the position oracle.
function oracle(v, f, h) {
    const F = Math.hypot(...f), u = scale(v, gamma(v));
    if (!F) return { displacement: scale(v, h), properElapsed: h / gamma(v), velocity: [...v] };
    const n = scale(f, 1 / F), z0 = dot(n, u), perpendicular = add(u, scale(n, -z0));
    const a = Math.sqrt(1 + dot(perpendicular, perpendicular)), z1 = z0 + F * h;
    const g0 = Math.hypot(a, z0), g1 = Math.hypot(a, z1);
    const tau = (Math.asinh(z1 / a) - Math.asinh(z0 / a)) / F;
    const dx = h * (z1 + z0) / (g1 + g0);
    const endU = add(u, scale(f, h));
    return { displacement: add(scale(n, dx), scale(perpendicular, tau)), properElapsed: tau, velocity: scale(endU, 1 / Math.hypot(1, ...endU)) };
}

async function fixture(v = [0, 0, 0], f = [.5, 0, 0]) {
    const session = new ObserverSession({ playing: false });
    await session.initialize();
    const marker = session.addEntity({ shape: 'beacon', position: [0, 0, 0], velocity: v, properAcceleration: f });
    session.state.observer.position = [0, 0, 0]; session.state.observer.velocity = [...v];
    return { session, marker };
}

test('real session observer and marker share a second-order worldline and proper clock', async () => {
    const errors = [];
    for (const count of [60, 120, 240, 480]) {
        const { session, marker } = await fixture();
        const dt = 1 / count;
        // Steering is a constant +x force until its .99 target is reached.
        for (let i = 0; i < count; i++) session.step(dt, { move: [1, 0, 0], speed: .99, acceleration: .5 });
        const o = session.state.observer, expected = oracle([0, 0, 0], [.5, 0, 0], 1);
        vectorClose(o.position, marker.position, 1e-12); close(o.properTime, marker.clockOffset, 1e-12);
        vectorClose(marker.velocity, expected.velocity, 1e-12);
        const error = [Math.abs(marker.position[0] - expected.displacement[0]), Math.abs(marker.clockOffset - expected.properElapsed)];
        if (count === 120) error.forEach(e => assert.ok(e < 2e-6));
        errors.push(error);
        session.dispose();
    }
    for (let i = 1; i < errors.length; i++) errors[i].forEach((e, j) => assert.ok(Math.log2(errors[i - 1][j] / e) >= 1.8));
    console.log('SR convergence errors [position, proper time]:', JSON.stringify(errors));
});

test('stored accelerated intervals reproduce endpoints, interior clocks and half-open joins', async () => {
    const { session, marker } = await fixture([.2, .1, -.15], [.3, -.2, .25]);
    for (let i = 0; i < 40; i++) session.step(1 / 120, {});
    const segments = session.state.segments.filter(s => s.entityId === marker.id);
    for (let i = 0; i < segments.length - 1; i++) {
        const a = segments[i], b = segments[i + 1], h = a.end - a.originTime;
        vectorClose(add(a.position, scale(a.velocity, h)), b.position, 2e-12);
        close(a.clockOffset + h / gamma(a.velocity), b.clockOffset, 2e-12);
        const middle = (Math.max(0, a.start) + a.end) / 2;
        session.state.scrubTime = middle;
        const e = session.snapshot().entities.find(e => e.id === marker.id);
        vectorClose(e.position, add(a.position, scale(a.velocity, middle - a.originTime)), 1e-12);
        close(e.clockOffset, a.clockOffset + (middle - a.originTime) / gamma(a.velocity));
        const dx = scale(a.velocity, h), tau = h / gamma(a.velocity);
        close(tau * tau, h * h - dot(dx, dx));
        assert.equal(segments.find(s => s.start <= a.end && (s.end === null || a.end < s.end)), b);
    }
    session.state.scrubTime = null;
    const last = segments.at(-1);
    vectorClose(last.position, marker.position); close(last.clockOffset, marker.clockOffset);
    session.dispose();
});

test('oblique, decelerating, rotated and inertial force steps converge to independent 3D integrals', () => {
    for (const [v, f] of [ [[.3, -.2, .1], [.2, .4, -.1]], [[.3, 0, 0], [-.8, 0, 0]], [[0, .3, 0], [0, -.8, 0]], [[.2, .1, .1], [0, 0, 0]] ]) {
        let current = v, x = [0, 0, 0], tau = 0;
        for (let i = 0; i < 480; i++) { const next = integrateFourVelocity(current, f, 1 / 480); current = next.velocity; x = add(x, next.displacement); tau += next.properElapsed; }
        const expected = oracle(v, f, 1);
        vectorClose(x, expected.displacement, 5e-7); close(tau, expected.properElapsed, 5e-7); vectorClose(current, expected.velocity, 1e-12);
    }
});

test('each shared drift obeys its own Minkowski interval including the application cap', () => {
    for (const [v, f] of [[[0, 0, 0], [.5, 0, 0]], [[.98, 0, 0], [1000, 500, 0]], [[.8, 0, 0], [0, .5, 0]]]) {
        const h = .1, next = integrateFourVelocity(v, f, h);
        close(next.properElapsed ** 2, h ** 2 - dot(next.displacement, next.displacement), 1e-13);
        assert.ok(Math.hypot(...next.velocity) <= .99 + 1e-12);
    }
});

test('force semantics agree with the independent four-acceleration invariant in 3D', () => {
    for (const [v, f] of [ [[.8,0,0],[0,.5,0]], [[.8,0,0],[.5,0,0]], [[.3,-.4,.2],[.1,.2,.3]], [[.8,0,0],[0,0,0]], [[0,.8,0],[-.5,0,0]] ]) {
        const g=gamma(v), h=1e-7, end=integrateFourVelocity(v,f,h).velocity;
        const before=[g,...scale(v,g)], g1=gamma(end), after=[g1,...scale(end,g1)];
        const fourAcceleration=after.map((x,i)=>(x-before[i])*g/h);
        const alpha=Math.sqrt(Math.max(0,-intervalSquared(fourAcceleration)));
        const expected=g*Math.sqrt(dot(f,f)-dot(v,f)**2);
        close(alpha,expected,2e-7);
        if(v[0]===.8&&f[1]===.5)close(alpha,5/6,2e-7);
    }
});

test('both boost conventions are inverse even at tiny speeds and reject invalid velocities', () => {
    for(const v of [[0,0,0],[1e-12,-1e-13,0],[1e-8,0,0],[.2,-.3,.4],[.99,0,0]]) {
        const e=[12,2,-3,4], b=boost(e[0],e.slice(1),v), expected=inverseBoost(e,v);
        vectorClose([b.time,...b.space],expected,1e-12);
    }
    // A tiny boost with a large time must still retain the first-order displacement.
    const b=boost(1e12,[0,0,0],[1e-12,0,0]);close(b.space[0],1,1e-12);
    const observer={position:[0,0,0],velocity:[1e-12,0,0],yaw:0,pitch:0,roll:0};
    const projected=projectPoint({profile:'sr',observer},{},[0,0,-1e12]);
    close(projected.x,.5+1/(2*Math.tan(Math.PI/6)*1e12),1e-15);
    for(const v of [[1,0,0],[NaN,0,0],[Infinity,0,0]]) {assert.throws(()=>boost(1,[0,0,0],v));assert.throws(()=>boostEvent([1,0,0,0],v));}
});

test('seeded boosts, rays, roots and fixed-emitter clock derivatives preserve SR invariants', () => {
    let seed=0x5eed2026;const random=()=>{seed=(Math.imul(seed,1664525)+1013904223)>>>0;return seed/4294967296;};
    const vec=()=>[random()-.5,random()-.5,random()-.5], vel=()=>scale(normalize(vec()),.99*random());
    const maxima={interval:0,inverse:0,nullRay:0,retarded:0,clockDoppler:0};
    for(let i=0;i<2000;i++) {
        const e=scale([random(),...vec()],20),v=vel(),b=boostEvent(e,v),inverse=inverseBoost(b,v);
        const interval=Math.abs(intervalSquared(b)-intervalSquared(e))/Math.max(1,e.reduce((sum,x)=>sum+x*x,0));
        const inverseError=Math.max(...e.map((x,j)=>Math.abs(x-inverse[j])))/Math.max(1,...e.map(Math.abs));
        const ray=observerRayToWorld(vec(),v),nullError=Math.abs(dot(ray.direction,ray.direction)-1);
        const origin=scale(vec(),10), source=scale(vec(),10),vo=vel(),ve=vel(),t=20;
        const emit=at=>retardedTime(add(origin,scale(vo,at)),at,source,ve);
        const te=emit(t),xe=add(source,scale(ve,te)),xo=add(origin,scale(vo,t)),r=sub(xo,xe);
        const residual=Math.abs(t-te-Math.hypot(...r))/Math.max(1,Math.hypot(...r));
        const D=dopplerFactor(r,ve,vo),h=1e-4, measured=(emit(t+h)-emit(t-h))/(2*h)*gamma(vo)/gamma(ve);
        const dError=Math.abs(D-measured)/Math.max(1,D);
        for(const [key,value]of Object.entries({interval,inverse:inverseError,nullRay:nullError,retarded:residual,clockDoppler:dError}))maxima[key]=Math.max(maxima[key],value);
        assert.ok(interval<1e-11&&inverseError<1e-11&&nullError<1e-11&&residual<1e-10&&dError<2e-7);
    }
    console.log('Seeded Float64 SR maximum residuals:',JSON.stringify(maxima));
});

test('D equals the arrival derivative of one fixed clock in real retained accelerated history',async()=>{
    const {session,marker}=await fixture([.1,0,0],[.4,0,0]);
    marker.position=[0,0,-.1];session.state.segments.find(s=>s.entityId===marker.id).position=[...marker.position];
    session.state.observer.velocity=[.1,0,0];
    for(let i=0;i<120;i++)session.step(1/120,{move:[1,0,0],speed:.1,acceleration:0});
    const intervals=session.state.segments.filter(s=>s.entityId===marker.id);
    const o=session.state.observer,t=session.state.time,delta=1e-6;
    // Bisection of arrival=emission+distance is independent of retardedTime.
    const emit=arrival=>{
        const observer=add(o.position,scale(o.velocity,arrival-t));let low=0,high=t;
        for(let i=0;i<60;i++){
            const at=(low+high)/2,s=intervals.find(s=>s.start<=at&&(s.end===null||at<s.end));
            const x=add(s.position,scale(s.velocity,at-s.originTime));
            if(at+Math.hypot(...sub(x,observer))<arrival)low=at;else high=at;
        }
        const at=(low+high)/2,s=intervals.find(s=>s.start<=at&&(s.end===null||at<s.end));
        return {time:at,segment:s,clock:s.clockOffset+(at-s.originTime)/gamma(s.velocity),position:add(s.position,scale(s.velocity,at-s.originTime))};
    };
    const center=emit(t),before=emit(t-delta),after=emit(t+delta);
    assert.equal(before.segment,after.segment);
    close(retardedTime(o.position,t,center.segment.position,center.segment.velocity,center.segment.originTime),center.time,1e-12);
    const physical=dopplerFactor(sub(o.position,center.position),center.segment.velocity,o.velocity);
    const measured=(after.clock-before.clock)/(2*delta)*gamma(o.velocity);
    close(physical,measured,2e-8);session.dispose();
});

test('real observer and point marker agree under oblique forces, reversals, zero force and clipping',async()=>{
    const {session,marker}=await fixture([.3,-.2,.1],[0,0,0]);
    const sequence=Array(30).fill([.2,.4,-.1]).concat(Array(30).fill([-.8,0,0]),Array(20).fill([0,0,0]),[[10000,1000,0]]);
    for(const force of sequence){
        const result=await session.command({type:'update',id:marker.id,patch:{coordinateForcePerMass:force}});assert.equal(result.ok,true);
        session.step(1/120,{coordinateForcePerMass:force});
        const e=session.state.entities.find(e=>e.id===marker.id),o=session.state.observer;
        vectorClose(e.position,o.position,2e-12);vectorClose(e.velocity,o.velocity,2e-12);close(e.clockOffset,o.properTime,2e-12);
        assert.equal(e.capApplied===true,o.capApplied===true);
    }
    session.dispose();
});

test('seeded point collisions preserve all four momentum components',()=>{
    let seed=0x5eed;const random=()=>{seed=(Math.imul(seed,1664525)+1013904223)>>>0;return seed/4294967296;};
    let maxResidual=0;
    for(let i=0;i<2000;i++){
        const m1=1+2*random(),m2=1+2*random(),v1=1.6*random()-.8,v2=1.6*random()-.8;
        const collision=elasticCollision1D(m1,v1,m2,v2),a=fourMomentum(m1,[collision.velocityA,0,0]),b=fourMomentum(m2,[collision.velocityB,0,0]);
        for(let j=0;j<4;j++){const error=Math.abs(a[j]+b[j]-collision.momentumBefore[j])/Math.max(1,Math.abs(collision.momentumBefore[j]));maxResidual=Math.max(maxResidual,error);assert.ok(error<1e-12);}
    }
    console.log('Seeded four-momentum maximum relative residual:',maxResidual);
});

test('prepared impulses inside a tick split histories at the exact event before the remainder',()=>{
    const s=new ObserverSession({preset:'collision',playing:false});
    for(let i=0;i<533;i++)s.step(1/120,{});
    s.step(1/120,{coordinateForcePerMass:[.1,.2,0]});
    assert.equal(s.state.collisionOccurred,true);
    const event=4/.9;
    for(const entity of s.state.entities){
        const before=s.state.segments.find(x=>x.entityId===entity.id&&x.end===event),after=s.state.segments.find(x=>x.entityId===entity.id&&x.start===event);
        assert.ok(before&&after);
        vectorClose(add(before.position,scale(before.velocity,event-before.originTime)),after.position,1e-12);
        close(before.clockOffset+(event-before.originTime)/gamma(before.velocity),after.clockOffset,1e-12);
    }
    const snapshot=s.snapshot();s.step(0,{});assert.deepEqual(s.snapshot(),snapshot);s.dispose();
});

test('all prepared light-clock events inside an explicit long step are retained',()=>{
    const s=new ObserverSession({preset:'light-clock',playing:false});
    s.step(7.6,{});
    assert.deepEqual(s.state.pulses.map(p=>p.start),[0,2.5,5,7.5]);
    assert.equal(s.state.time,7.6);assert.equal(s.state.tick,1);s.dispose();
});
