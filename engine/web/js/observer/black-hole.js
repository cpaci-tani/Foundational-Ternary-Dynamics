// @ts-check
/** Adopted Schwarzschild exterior. Captures use the null radial potential;
 * no emitting horizon, accretion model or native FTD identification is present. */
import {starMetric,assertExterior} from './compact-star.js';
import {add,sub,scale,dot,cross,length,normalize,gamma,cameraBasis,observerRayToWorld} from './math.js';
export {blackHole} from './compact-star.js';
/** @typedef {import('./compact-star.js').CompactStar} BlackHole */
/** @param {number[]} y @param {number} h @param {(y:number[])=>number[]} derivative */
function rk4(y,h,derivative){
    const k1=derivative(y),k2=derivative(add(y,scale(k1,h/2))),k3=derivative(add(y,scale(k2,h/2))),k4=derivative(add(y,scale(k3,h)));
    return y.map((v,i)=>v+h*(k1[i]+2*k2[i]+2*k3[i]+k4[i])/6);
}
/** Half-angle of the shadow measured by a local static observer. Below the
 * photon sphere the shadow covers more than one hemisphere.
 * @param {BlackHole} hole @param {number[]} position */
export function blackHoleShadowAngle(hole,position){
    const m=starMetric(hole,position),sine=Math.min(1,1.5*Math.sqrt(3)*hole.rs*m.lapse/m.areal),angle=Math.asin(sine);
    return m.areal<1.5*hole.rs?Math.PI-angle:angle;
}
/** Backward null ray integrated with Euclidean isotropic arclength. An inward
 * ray below the photon sphere has no inner radial turning point and is captured.
 * Rays numerically indistinguishable from the critical orbit remain unresolved.
 * @param {BlackHole} hole @param {number[]} origin @param {number[]} direction
 * @param {{maxSteps?:number}} [options] */
export function traceBlackHolePath(hole,origin,direction,options={}){
    if(hole.kind!=='schwarzschild-black-hole')throw new Error('A black-hole null ray requires its Schwarzschild black-hole provider.');
    if(direction.length!==3||direction.some(v=>!Number.isFinite(v))||length(direction)===0)throw new Error('A null-ray direction must be a finite nonzero three-vector.');
    assertExterior(hole,origin);
    const initial=normalize(direction),start=starMetric(hole,origin),radial=normalize(sub(origin,hole.center));
    const impactParameter=start.index*start.rho*length(cross(radial,initial));
    const critical=1.5*Math.sqrt(3)*hole.rs,photonRadius=hole.rs*(1+Math.sqrt(.75))/2;
    let y=[...origin,...initial,0],arc=0;
    const maximum=options.maxSteps??1536;
    if(!Number.isSafeInteger(maximum)||maximum<0||maximum>4096)throw new Error('Null-ray step budget must be an integer in [0,4096].');
    /** @param {number[]} state */
    const derivative=state=>{const m=starMetric(hole,state.slice(0,3)),d=state.slice(3,6);return [...d,...sub(m.gradLogN,scale(d,dot(d,m.gradLogN))),m.index];};
    /** @param {'captured'|'escaped'|'budget-exhausted'} status @param {number} steps */
    const result=(status,steps)=>({status,position:y.slice(0,3),direction:y.slice(3,6),delay:y[6],distance:arc,steps,impactParameter});
    const criticalApproach=start.rho>=photonRadius?dot(radial,initial)<0:dot(radial,initial)>0;
    const criticalUncertain=criticalApproach&&Math.abs(impactParameter/critical-1)<1e-10;
    for(let step=0;step<maximum;step++){
        const m=starMetric(hole,y.slice(0,3)),inward=dot(sub(y.slice(0,3),hole.center),y.slice(3,6))<0;
        if(m.rho<photonRadius*(1-1e-6)&&inward){
            if(criticalUncertain)return result('budget-exhausted',step);
            if(start.rho<photonRadius||impactParameter<critical*(1-1e-10))return result('captured',step);
            return result('budget-exhausted',step);
        }
        const h=Math.min(.015*m.rho,.25,.5*(m.rho-hole.radius));
        let next=rk4(y,h,derivative),used=h;
        if(length(sub(next.slice(0,3),hole.center))>=hole.escapeRadius){
            let lo=0,hi=h;
            for(let i=0;i<40;i++){const mid=(lo+hi)/2,p=rk4(y,mid,derivative);if(length(sub(p.slice(0,3),hole.center))>=hole.escapeRadius)hi=mid;else lo=mid;}
            used=(lo+hi)/2;next=rk4(y,used,derivative);next.splice(3,3,...normalize(next.slice(3,6)));y=next;arc+=used;
            return result(criticalUncertain?'budget-exhausted':'escaped',step+1);
        }
        next.splice(3,3,...normalize(next.slice(3,6)));y=next;arc+=used;
    }
    return result('budget-exhausted',maximum);
}
/** Current observer-rest camera ray and its static finite-shell frequency.
 * Partial capture delays describe integration termination, never horizon light.
 * @param {import('./optics.js').OpticalSnapshot} snapshot @param {import('./optics.js').OpticalSettings} [settings]
 * @param {number} [ndcX] @param {number} [ndcY] */
export function traceBlackHole(snapshot,settings={},ndcX=0,ndcY=0){
    const hole=/** @type {BlackHole} */(snapshot.spacetime),observer={...snapshot.observer,...settings.cameraOverride};
    const basis=cameraBasis(observer.yaw||0,observer.pitch||0,observer.roll||0),tangent=Math.tan((settings.fov||60)*Math.PI/360);
    const look=normalize(add(basis.forward,add(scale(basis.right,ndcX*tangent*(settings.aspect||1)),scale(basis.up,ndcY*tangent))));
    const initialDirection=observerRayToWorld(look,observer.velocity).direction,path=traceBlackHolePath(hole,observer.position,initialDirection);
    const observerLapse=starMetric(hole,observer.position).lapse,endLapse=starMetric(hole,path.position).lapse;
    const doppler=path.status==='escaped'?endLapse/observerLapse*gamma(observer.velocity)*(1+dot(observer.velocity,initialDirection)):null;
    return {...path,source:'Float64 CPU geodesic',observationTime:snapshot.time,observerPosition:[...observer.position],observerVelocity:[...observer.velocity],observerYaw:observer.yaw||0,observerPitch:observer.pitch||0,observerRoll:observer.roll||0,cameraFov:settings.fov||60,cameraAspect:settings.aspect||1,ndcX,ndcY,initialDirection,observerLapse,endLapse,receiverLapse:observerLapse,emissionLapse:path.status==='escaped'?endLapse:null,doppler,
        emissionTime:path.status==='escaped'?snapshot.time-path.delay:null,
        // The stationary sky is prescribed radiance, with no source entity or
        // declared clock origin. Its lapse does not supply a proper-clock reading.
        properTime:null,
        termination:path.status==='captured'?'Inward null ray below the photon sphere; no inner radial turning point':path.status==='escaped'?'Static finite sky boundary':'Numerically unresolved or integration budget exhausted'};
}
