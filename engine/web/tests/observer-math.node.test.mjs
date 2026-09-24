import test from 'node:test';
import assert from 'node:assert/strict';
import {gamma,boostEvent,inverseBoost,intervalSquared,velocityAddition,aberration,dopplerFactor,observerRayToWorld,constantProperAcceleration,integrateFourVelocity,retardedTime,fourMomentum,elasticCollision1D,cameraBasis,dot,length} from '../js/observer/math.js';
const close=(a,b,tol=1e-10)=>assert.ok(Math.abs(a-b)<tol,`${a} != ${b}`);
test('Lorentz boosts preserve intervals and invert for general 3D velocity',()=>{
    const event=[12,2,-3,4],v=[.3,-.4,.2],transformed=boostEvent(event,v);
    close(intervalSquared(transformed),intervalSquared(event));
    inverseBoost(transformed,v).forEach((x,i)=>close(x,event[i]));
});
test('null rays remain null under boosts',()=>{
    const ray=[1,0,.6,.8];
    close(intervalSquared(boostEvent(ray,[.6,.1,.2])),0);
});
test('proper clock rates are 0.8 and 0.6 for requested betas',()=>{
    close(1/gamma(.6),.8); close(1/gamma(.8),.6);
    assert.throws(()=>gamma(1),RangeError); assert.throws(()=>gamma(NaN),RangeError);
});
test('velocity addition remains timelike and agrees with 1D formula',()=>{
    close(velocityAddition([.6,0,0],[.8,0,0])[0],1.4/1.48);
    assert.ok(length(velocityAddition([.6,.2,0],[.3,.5,0]))<1);
});
test('transverse aberration and longitudinal Doppler agree with analytic results',()=>{
    const ray=aberration([0,1,0],[.6,0,0]);close(ray[0],-.6);close(ray[1],.8);
    close(dopplerFactor([1,0,0],[0,0,0],[.6,0,0]),.5);
    close(dopplerFactor([-1,0,0],[0,0,0],[.6,0,0]),2);
});
test('observer backward rays are null and oriented toward source',()=>{
    const ray=observerRayToWorld([0,0,-1],[.6,0,0]);
    close(length(ray.direction),1);close(ray.direction[0],-.6);close(ray.direction[2],-.8);
});
test('inertial retarded center solves null condition including approaching branch',()=>{
    for(const v of [[.8,0,0],[-.8,0,0],[0,.3,.4]]){
        const t=retardedTime([0,0,0],10,[3,2,0],v,0);
        const p=[3,2,0].map((x,i)=>x+v[i]*t);
        close(length(p),10-t);
    }
});
test('proper-force integration agrees with analytic constant proper acceleration',()=>{
    let v=[0,0,0],x=0,tau=0; const dt=1/120;
    for(let i=0;i<120;i++){const step=integrateFourVelocity(v,[.5,0,0],dt);v=step.velocity;x+=step.displacement[0];tau+=step.properElapsed;}
    const analytic=constantProperAcceleration(Math.asinh(.5)/.5,.5);
    close(v[0],analytic.velocity,1e-12);close(x,analytic.position,2e-6);close(tau,Math.asinh(.5)/.5,2e-6);
});
test('acceleration respects speed ceiling',()=>{
    const result=integrateFourVelocity([.98,0,0],[1000,0,0],1);
    close(length(result.velocity),.99);
});
test('elastic point collision conserves full four-momentum',()=>{
    const c=elasticCollision1D(1,.6,2,-.3);
    const a=fourMomentum(1,[c.velocityA,0,0]),b=fourMomentum(2,[c.velocityB,0,0]);
    a.map((x,i)=>x+b[i]).forEach((x,i)=>close(x,c.momentumBefore[i]));
});
test('camera basis is orthonormal and initial look is negative Z',()=>{
    assert.deepEqual(cameraBasis(0,0).forward,[-0,0,-1]);
    const basis=cameraBasis(.4,.2,-.7);
    Object.values(basis).forEach(v=>close(length(v),1));close(dot(basis.forward,basis.right),0);close(dot(basis.forward,basis.up),0);
});
