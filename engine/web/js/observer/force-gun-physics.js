// @ts-check
/** Stable, mass-sensitive game-physics tether. No DOM, worker or Rapier imports. */
export const FORCE_GUN_PRESETS = Object.freeze({
    delicate: Object.freeze({ stiffness: 6, damping: 3, maxForce: 20 }),
    normal: Object.freeze({ stiffness: 24, damping: 8, maxForce: 120 }),
    strong: Object.freeze({ stiffness: 80, damping: 16, maxForce: 600 }),
});
/** @param {string} sensitivity @param {number} multiplier */
export function forceGunSettings(sensitivity, multiplier) {
    if (!Object.hasOwn(FORCE_GUN_PRESETS, sensitivity)) throw new Error('Unknown force-gun sensitivity.');
    if (!Number.isFinite(multiplier) || multiplier < 0.1 || multiplier > 10) throw new Error('Force-gun multiplier must be in [0.1, 10].');
    const preset=FORCE_GUN_PRESETS[/** @type {keyof typeof FORCE_GUN_PRESETS} */(sensitivity)];
    return {stiffness:preset.stiffness*multiplier,damping:preset.damping*Math.sqrt(multiplier),maxForce:preset.maxForce*multiplier};
}
/** @param {number[]} a @param {number[]} b */
const cross=(a,b)=>[a[1]*b[2]-a[2]*b[1],a[2]*b[0]-a[0]*b[2],a[0]*b[1]-a[1]*b[0]];
/** @param {number[][]} m @param {number[]} v */
const multiply=(m,v)=>m.map(row=>row.reduce((sum,x,i)=>sum+x*v[i],0));
/** Point response to an impulse: K = invMass - [r]x invInertia [r]x.
 * @param {number[]} inverseMass @param {number[][]} inverseInertia @param {number[]} lever
 */
export function pointInverseMass(inverseMass,inverseInertia,lever) {
    const columns=[0,1,2].map(axis=>{
        const unit=[0,0,0];unit[axis]=1;
        const rotational=cross(multiply(inverseInertia,cross(lever,unit)),lever);
        return rotational.map((v,i)=>v+(i===axis?inverseMass[i]:0));
    });
    return [0,1,2].map(i=>columns.map(column=>column[i]));
}
/** Pivoted 3x3 solve; implicit spring regularization keeps the matrix nonsingular.
 * @param {number[][]} matrix @param {number[]} rhs
 */
function solve(matrix,rhs) {
    const a=matrix.map((row,i)=>[...row,rhs[i]]);
    for(let col=0;col<3;col++){
        let pivot=col;for(let row=col+1;row<3;row++)if(Math.abs(a[row][col])>Math.abs(a[pivot][col]))pivot=row;
        [a[col],a[pivot]]=[a[pivot],a[col]];
        const diagonal=a[col][col];if(!Number.isFinite(diagonal)||Math.abs(diagonal)<1e-30)throw new Error('Invalid tether inertia.');
        for(let j=col;j<4;j++)a[col][j]/=diagonal;
        for(let row=0;row<3;row++)if(row!==col){const factor=a[row][col];for(let j=col;j<4;j++)a[row][j]-=factor*a[col][j];}
    }
    return a.map(row=>row[3]);
}
/** Pull follows the full target. Push steers laterally toward the aim ray and
 * drives outward along it, with damping limiting drift. Force is never scaled
 * by body mass to disguise inertia. Implicit point inertia stabilizes tiny bodies.
 * @param {{anchor:number[],target:number[],velocity:number[],direction:number[],mode:'pull'|'push',sensitivity:string,multiplier:number,dt:number,effectiveInverseMass:number[][]}} input
 */
export function computeGunForce(input) {
    const {anchor,target,velocity,mode,dt,effectiveInverseMass}=input;
    if(!(dt>0&&dt<=0.1))throw new Error('Invalid force-gun integration step.');
    const {stiffness:k,damping:d,maxForce}=forceGunSettings(input.sensitivity,input.multiplier);
    const norm=Math.hypot(...input.direction);
    if(!(norm>1e-12)&&mode==='push')throw new Error('Push direction must be nonzero.');
    const direction=input.direction.map(v=>norm>0?v/norm:0);
    let error=target.map((v,i)=>v-anchor[i]);
    let predicted=velocity.map(v=>v*dt);
    if(mode==='push'){
        const along=error.reduce((sum,v,i)=>sum+v*direction[i],0),motion=predicted.reduce((sum,v,i)=>sum+v*direction[i],0);
        error=error.map((v,i)=>v-along*direction[i]);predicted=predicted.map((v,i)=>v-motion*direction[i]);
    }
    const rhs=error.map((v,i)=>k*(v-predicted[i])-d*velocity[i]+(mode==='push'?maxForce*0.3*direction[i]:0));
    const scale=d*dt+k*dt*dt;
    const matrix=effectiveInverseMass.map((row,i)=>row.map((v,j)=>(i===j?1:0)+scale*v));
    let force=solve(matrix,rhs);
    if(!force.every(Number.isFinite))throw new Error('Tether force is nonfinite.');
    const raw=Math.hypot(...force);
    if(raw>maxForce)force=force.map(v=>v*maxForce/raw);
    return {force,magnitude:Math.hypot(...force),requestedMagnitude:raw,maxForce};
}
