// @ts-check
/** Adopted Schwarzschild exterior in isotropic coordinates, c=1. No native FTD claim.
 * Local velocities are measured by static orthonormal observers. The source is
 * a static floating sphere; no stellar interior, equation of state or backreaction.
 */
import { add, sub, scale, dot, length, normalize, gamma, cameraBasis, observerRayToWorld, integrateFourVelocity, MAX_BETA } from './math.js';
export const LIGHT_SPEED = 299792458;
export const NOMINAL_SOLAR_GM = 1.3271244e20;
export const COMPACT_STAR_METHOD = 'gr-schwarzschild-exterior-rk4-v1';
export const BLACK_HOLE_METHOD = 'gr-schwarzschild-black-hole-rk4-v1';
/** The black-hole radiusKm/radius are horizon coordinates, never emitting surfaces.
 * @typedef {{kind:'schwarzschild-exterior'|'schwarzschild-black-hole',massSolar:number,radiusKm:number,lengthUnitMeters:number,rs:number,radius:number,center:number[],sourceId:string,observerMode:'guided'|'freefall',escapeRadius:number,observerBoundary?:number}} CompactStar */
/** @param {number} [massSolar] @param {number} [radiusKm] @returns {CompactStar} */
export function compactStar(massSolar=1.4, radiusKm=12) {
    if (!Number.isFinite(massSolar)||massSolar<0||massSolar>10||!Number.isFinite(radiusKm)||radiusKm<1e-8||radiusKm>100) throw new Error('Mass must be 0–10 solar masses; radius must be positive and at most 100 km.');
    const lengthUnitMeters=radiusKm*1000/.25, rs=2*NOMINAL_SOLAR_GM*massSolar/LIGHT_SPEED**2/lengthUnitMeters;
    if (.25<=1.5*rs) throw new Error('This exterior experiment requires the surface outside the photon sphere (R > 1.5 Schwarzschild radii).');
    const radius=(.25-rs/2+Math.sqrt(.25*(.25-rs)))/2;
    return {kind:'schwarzschild-exterior',massSolar,radiusKm,lengthUnitMeters,rs,radius,center:[0,1.6,0],sourceId:'',observerMode:'guided',escapeRadius:40};
}
/** Adopted nonrotating, uncharged black hole. The exterior isotropic chart ends
 * at rho=rs/4; observerBoundary is a declared numerical guard outside it.
 * @param {number} [massSolar] @returns {CompactStar} */
export function blackHole(massSolar=10) {
    if(!Number.isFinite(massSolar)||massSolar<=0||massSolar>1e6)throw new Error('Black-hole mass must be positive and at most one million nominal solar masses.');
    const rs=.25,lengthUnitMeters=2*NOMINAL_SOLAR_GM*massSolar/LIGHT_SPEED**2/rs,radius=rs/4;
    return {kind:'schwarzschild-black-hole',massSolar,radiusKm:rs*lengthUnitMeters/1000,lengthUnitMeters,rs,radius,center:[0,1.6,0],sourceId:'',observerMode:'guided',escapeRadius:40,observerBoundary:radius*1.01};
}
/** @param {CompactStar} star @param {number[]} position */
export function starMetric(star, position) {
    const relative=sub(position,star.center),rho=length(relative),q=star.rs/(4*rho),radial=normalize(relative);
    const lapse=(1-q)/(1+q),spatial=(1+q)**2,index=spatial/lapse;
    const gradA=scale(radial,2*q/(rho*(1+q)**2));
    const gradLogB=scale(radial,-2*q/(rho*(1+q)));
    const gradLogN=scale(radial,-q/rho*(3/(1+q)+1/(1-q)));
    return {rho,lapse,spatial,index,areal:rho*spatial,gradA,gradLogB,gradLogN};
}
/** @param {CompactStar} star @param {number[]} position */
export function assertExterior(star,position) {
    const rho=length(sub(position,star.center));
    const inner=star.observerBoundary??star.radius+1e-7;
    if(!Number.isFinite(rho)||rho<=inner||rho>=star.escapeRadius) throw new Error(star.kind==='schwarzschild-black-hole'?'Observer must remain outside the black-hole exterior numerical guard and inside the 40-unit optical boundary. This chart does not cross the horizon.':'Observer must remain outside the stellar surface and inside the 40-unit optical boundary.');
}
/** @param {CompactStar} star */
export function starApparatus(star) {
    return [
        {name:'Compact star · static emitting surface',shape:'sphere',position:[...star.center],size:[2*star.radius,2*star.radius,2*star.radius],color:[1,.8,.5],emission:2},
    ];
}
/** RK4 used for null and timelike paths. @param {number[]} y @param {number} h @param {(y:number[])=>number[]} derivative */
function rk4(y,h,derivative) {
    const k1=derivative(y),k2=derivative(add(y,scale(k1,h/2))),k3=derivative(add(y,scale(k2,h/2))),k4=derivative(add(y,scale(k3,h)));
    return y.map((v,i)=>v+h*(k1[i]+2*k2[i]+2*k3[i]+k4[i])/6);
}
/** @param {CompactStar} star @param {number[]} position @param {number[]} velocity */
export function starEnergy(star,position,velocity) {return starMetric(star,position).lapse*gamma(velocity);}
/** Coordinate-time integration, local momentum controls or a timelike geodesic.
 * Stops at the exterior boundary; never projects a freefall trajectory onto a speed cap.
 * @param {CompactStar} star @param {import('./types.js').ObserverState} observer @param {number} dt @param {number[]} [force]
 */
export function advanceStarObserver(star,observer,dt,force=[0,0,0]) {
    assertExterior(star,observer.position);
    const m0=starMetric(star,observer.position),u0=scale(observer.velocity,gamma(observer.velocity));
    const guided=star.observerMode==='guided',kick=integrateFourVelocity(observer.velocity,force,dt);
    const effectiveForce=dt>0?scale(sub(scale(kick.velocity,gamma(kick.velocity)),u0),1/dt):[0,0,0];
    const y0=[...observer.position,...scale(u0,guided?1:m0.spatial),0];
    /** @param {number[]} y */
    const derivative=y=>{
        const m=starMetric(star,y.slice(0,3)),u=scale(y.slice(3,6),guided?1:1/m.spatial),g=Math.sqrt(1+dot(u,u));
        const drift=scale(u,m.lapse/(m.spatial*g));
        const momentum=guided?effectiveForce:add(scale(m.gradA,-g),scale(m.gradLogB,m.lapse*dot(u,u)/g));
        return [...drift,...momentum,m.lapse/g];
    };
    // Substeps resolve close approaches; a surface event is bracketed and refined.
    let y=y0,elapsed=0,stopped=false;
    /** @type {'inner-boundary'|'outer-boundary'|'speed-limit'|null} */ let stoppedReason=null;
    const innerBoundary=star.observerBoundary===undefined?star.radius+2e-7:star.observerBoundary+1e-8;
    while(elapsed<dt-1e-14){
        const m=starMetric(star,y.slice(0,3)),h=Math.min(dt-elapsed,.002,m.rho*.02);
        let next=rk4(y,h,derivative),used=h;
        const nextRho=length(sub(next.slice(0,3),star.center)),boundary=nextRho<=innerBoundary||nextRho>=star.escapeRadius-1e-6;
        const v=scale(next.slice(3,6),guided?1:1/starMetric(star,next.slice(0,3)).spatial);
        if(boundary||length(v)/Math.sqrt(1+dot(v,v))>MAX_BETA){
            stoppedReason=nextRho<=innerBoundary?'inner-boundary':nextRho>=star.escapeRadius-1e-6?'outer-boundary':'speed-limit';
            let lo=0,hi=h;
            for(let i=0;i<40;i++){const mid=(lo+hi)/2,p=rk4(y,mid,derivative),m=starMetric(star,p.slice(0,3)),u=scale(p.slice(3,6),guided?1:1/m.spatial);if(m.rho<=innerBoundary||m.rho>=star.escapeRadius-1e-6||length(u)/Math.sqrt(1+dot(u,u))>MAX_BETA)hi=mid;else lo=mid;}
            const eventRho=starMetric(star,rk4(y,hi,derivative).slice(0,3)).rho;
            stoppedReason=eventRho<=innerBoundary?'inner-boundary':eventRho>=star.escapeRadius-1e-6?'outer-boundary':'speed-limit';
            used=lo;next=rk4(y,lo,derivative);stopped=true;
        }
        y=next;elapsed+=used;if(stopped)break;
    }
    const m=starMetric(star,y.slice(0,3)),u=scale(y.slice(3,6),guided?1:1/m.spatial);
    return {position:y.slice(0,3),velocity:scale(u,1/Math.sqrt(1+dot(u,u))),properElapsed:y[6],elapsed,stopped,stoppedReason,capApplied:guided&&kick.capApplied};
}
/** @param {number[]} p @param {{shape:string,position:number[],size:number[]}} entity */
export function starSurfaceDistance(p,entity) {
    const local=sub(p,entity.position);
    if(entity.shape==='sphere')return length(local)-entity.size[0]/2;
    const q=local.map((v,i)=>Math.abs(v)-entity.size[i]/2);
    return Math.hypot(...q.map(v=>Math.max(v,0)))+Math.min(Math.max(...q),0);
}
/** Static scene null rays: dx/dl=d, dd/dl=grad(log n)-d(d·grad(log n)), dt/dl=n.
 * Distance-based stepping prevents skipping any of the opaque surfaces.
 * @param {CompactStar} star @param {number[]} origin @param {number[]} direction
 * @param {{shape:string,position:number[],size:number[]}[]} entities @param {number} [epsilon]
 */
export function traceStarPath(star,origin,direction,entities,epsilon=1e-7) {
    let y=[...origin,...normalize(direction),0],arc=0;
    /** @param {number[]} state */
    const derivative=state=>{const m=starMetric(star,state.slice(0,3)),d=state.slice(3,6);return [...d,...sub(m.gradLogN,scale(d,dot(d,m.gradLogN))),m.index];};
    for(let step=0;step<768;step++){
        const p=y.slice(0,3),m=starMetric(star,p),distances=entities.map(e=>starSurfaceDistance(p,e));
        const nearest=Math.min(...distances),index=distances.indexOf(nearest);
        if(nearest<=epsilon)return {status:'hit',index,position:p,direction:y.slice(3,6),delay:y[6],distance:arc,steps:step};
        if(m.rho>=star.escapeRadius)return {status:'escaped',index:-1,position:p,direction:y.slice(3,6),delay:y[6],distance:arc,steps:step};
        const h=Math.min(.04*m.rho,.5,Math.max(epsilon*.1,.8*nearest));
        let next=rk4(y,h,derivative);
        if(length(sub(next.slice(0,3),star.center))>=star.escapeRadius){
            let lo=0,hi=h;
            for(let i=0;i<40;i++){const mid=(lo+hi)/2,state=rk4(y,mid,derivative);if(length(sub(state.slice(0,3),star.center))>=star.escapeRadius)hi=mid;else lo=mid;}
            const used=(lo+hi)/2;next=rk4(y,used,derivative);next.splice(3,3,...normalize(next.slice(3,6)));arc+=used;
            return {status:'escaped',index:-1,position:next.slice(0,3),direction:next.slice(3,6),delay:next[6],distance:arc,steps:step+1};
        }
        y=next;y.splice(3,3,...normalize(y.slice(3,6)));arc+=h;
    }
    return {status:'budget-exhausted',index:-1,position:y.slice(0,3),direction:y.slice(3,6),delay:y[6],distance:arc,steps:768};
}
/** @param {import('./optics.js').OpticalSnapshot} snapshot @param {import('./optics.js').OpticalSettings} settings @param {number} ndcX @param {number} ndcY @returns {import('./optics.js').OpticalHit|null} */
export function traceCompactStar(snapshot,settings,ndcX,ndcY) {
    const star=/** @type {CompactStar} */(snapshot.spacetime),observer={...snapshot.observer,...settings.cameraOverride},basis=cameraBasis(observer.yaw||0,observer.pitch||0,observer.roll||0);
    const tangent=Math.tan((settings.fov||60)*Math.PI/360),look=normalize(add(basis.forward,add(scale(basis.right,ndcX*tangent*(settings.aspect||1)),scale(basis.up,ndcY*tangent))));
    const direction=observerRayToWorld(look,observer.velocity).direction,path=traceStarPath(star,observer.position,direction,snapshot.segments);
    if(path.status!=='hit')return null;
    const s=snapshot.segments[path.index],emissionTime=snapshot.time-path.delay;
    if(emissionTime<Math.max(s.start??-Infinity,snapshot.historyStart)||emissionTime>=(s.end??Infinity))return null;
    const id=s.entityId||s.id||'',A=starMetric(star,path.index===0?add(star.center,[star.radius,0,0]):path.position).lapse;
    const local=sub(path.position,s.position),normal=normalize(local);
    if(s.shape==='box'){const q=local.map((v,i)=>Math.abs(v)-s.size[i]/2),axis=q.indexOf(Math.max(...q));normal.fill(0);normal[axis]=Math.sign(local[axis]);}
    return {entityId:id,id,revision:s.revision,distance:path.distance,emissionTime,properTime:A*emissionTime,position:path.position,sourcePosition:path.position,restPosition:local,normal,historical:false,mirrored:false,segmentIndex:path.index,doppler:A/starMetric(star,observer.position).lapse*gamma(observer.velocity)*(1+dot(observer.velocity,direction))};
}
/** Validate imported metric and locked preparation rather than trusting cached parameters.
 * @param {import('./types.js').WorldSnapshot} snapshot */
export function validateStarSnapshot(snapshot) {
    const s=snapshot.spacetime;if(!s)return;
    if(s.kind==='schwarzschild-black-hole'){
        const canonical=blackHole(s.massSolar);
        if(snapshot.profile!=='sr'||snapshot.experiment!=='black-hole'||snapshot.integratorVersion!==BLACK_HOLE_METHOD||!['guided','freefall'].includes(s.observerMode))throw new Error('Invalid black-hole exterior reference provider.');
        for(const key of ['radiusKm','lengthUnitMeters','rs','radius','escapeRadius','center','observerBoundary','sourceId'])if(JSON.stringify(s[/** @type {keyof CompactStar} */(key)])!==JSON.stringify(canonical[/** @type {keyof CompactStar} */(key)]))throw new Error('Imported black-hole metric differs from its physical parameters.');
        if(snapshot.entities.length||snapshot.segments.length||snapshot.pulses?.length||snapshot.joints?.length||snapshot.scrubTime!==null)throw new Error('The black-hole exterior has no emitting horizon, authored objects or Minkowski histories.');
        assertExterior(s,snapshot.observer.position);return;
    }
    const canonical=compactStar(s.massSolar,s.radiusKm);
    if(snapshot.profile!=='sr'||snapshot.experiment!=='compact-star'||s.kind!==canonical.kind||!['guided','freefall'].includes(s.observerMode))throw new Error('Invalid compact-star reference provider.');
    if(snapshot.integratorVersion!==COMPACT_STAR_METHOD)throw new Error('Unknown compact-star evolution method.');
    if(snapshot.scrubTime!==null||snapshot.pulses?.length||snapshot.joints?.length)throw new Error('Compact-star preparation cannot contain Minkowski scrubs, pulses or joints.');
    for(const key of ['lengthUnitMeters','rs','radius','escapeRadius','center'])if(JSON.stringify(s[/** @type {keyof CompactStar} */(key)])!==JSON.stringify(canonical[/** @type {keyof CompactStar} */(key)]))throw new Error('Imported compact-star metric differs from its physical parameters.');
    assertExterior(s,snapshot.observer.position);
    const apparatus=starApparatus(s);
    if(snapshot.entities.length===4&&snapshot.segments.length===4)throw new Error('This compact-star save contains the obsolete pedestal. Re-enter the compact-star preparation to create the floating sphere.');
    if(snapshot.entities.length!==1||snapshot.segments.length!==1||snapshot.entities[0].id!==s.sourceId)throw new Error('The compact-star preparation requires one static floating sphere.');
    snapshot.entities.forEach((e,i)=>{
        for(const key of ['shape','position','size','color','emission'])if(JSON.stringify(e[/** @type {keyof typeof e} */(key)])!==JSON.stringify(apparatus[i][/** @type {keyof typeof apparatus[0]} */(key)]))throw new Error('Compact-star apparatus geometry is locked.');
        const seg=snapshot.segments[i];
        if(!e.alive||e.bodyType!=='fixed'||e.gravity||e.collisions||length(e.velocity)||length(e.rotation)||length(e.coordinateForcePerMass)||length(e.angularVelocity)||seg.entityId!==e.id||seg.revision!==e.revision||seg.end!==null||seg.originTime!==0||seg.clockOffset!==0||seg.start>snapshot.historyStart||e.spectral!=='white'||seg.integratorVersion!==COMPACT_STAR_METHOD)throw new Error('Invalid compact-star static history.');
        for(const key of ['shape','position','size','color','emission','velocity','rotation','spectral'])if(JSON.stringify(seg[/** @type {keyof typeof seg} */(key)])!==JSON.stringify(e[/** @type {keyof typeof e} */(key)]))throw new Error('Compact-star history differs from the static preparation.');
        const clockPosition=i===0?add(s.center,[s.radius,0,0]):e.position;
        if(e.originTime!==snapshot.time||Math.abs(e.clockOffset-starMetric(s,clockPosition).lapse*snapshot.time)>1e-10)throw new Error('Invalid compact-star source clock.');
    });
}
