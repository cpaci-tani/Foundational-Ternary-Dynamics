// @ts-check
/** Locally vendored, explicitly Newtonian Rapier 0.20.0 adapter. */
import { restMesh } from './geometry.js';
import { computeGunForce, pointInverseMass } from './force-gun-physics.js';
import { reflectPolar, reflectAxial, reflectEulerXYZ, reflectQuaternion } from './mirror-frame.js';
/** @typedef {import('./types.js').WorldEntity} WorldEntity */
/** @typedef {import('./types.js').SpringJoint} SpringJoint */
/** @typedef {import('./types.js').WorldPhysicsSettings} WorldPhysicsSettings */
/** @typedef {import('../vendor/rapier/runtime/rapier.js').RigidBody} RigidBody */
/** @typedef {typeof import('../vendor/rapier/runtime/rapier.js')} RapierModule */
/** @typedef {{mass:number,center:{x:number,y:number,z:number},inertia:{x:number,y:number,z:number},frame:{x:number,y:number,z:number,w:number}}} MassProperties */
/** @param {number[]} a */
const vector = a => ({ x: a[0], y: a[1], z: a[2] });
/** @param {number[]} e */
function quaternion(e) {
    const [x, y, z] = e.map(v => v / 2), c1 = Math.cos(x), c2 = Math.cos(y), c3 = Math.cos(z), s1 = Math.sin(x), s2 = Math.sin(y), s3 = Math.sin(z);
    return { x: s1 * c2 * c3 + c1 * s2 * s3, y: c1 * s2 * c3 - s1 * c2 * s3, z: c1 * c2 * s3 + s1 * s2 * c3, w: c1 * c2 * c3 - s1 * s2 * s3 };
}
/** @param {{x:number,y:number,z:number,w:number}} q */
function euler(q) {
    const m13 = 2 * (q.x * q.z + q.w * q.y);
    return [Math.atan2(2 * (q.w * q.x - q.y * q.z), 1 - 2 * (q.x * q.x + q.y * q.y)), Math.asin(Math.max(-1, Math.min(1, m13))), Math.atan2(2 * (q.w * q.z - q.x * q.y), 1 - 2 * (q.y * q.y + q.z * q.z))];
}
export class PlaygroundPhysics {
    /** @type {RapierModule|null} */ api = null;
    /** @type {import('../vendor/rapier/runtime/rapier.js').World|null} */ world = null;
    /** @type {Map<string,import('../vendor/rapier/runtime/rapier.js').RigidBody>} */ bodies = new Map();
    /** @type {SpringJoint[]} */ joints = [];
    /** Authored side of each shared body in the folded plane-gravity world. @type {Map<string,number>} */ sides = new Map();
    /** @type {WorldPhysicsSettings} */ settings = {gravityMode:'uniform',gravityStrength:9.81,objectCollisions:true,planeCollision:true};
    async initialize() {
        // Module bytes and embedded WASM are shipped locally; no CDN or bundler required.
        const moduleUrl = new URL('../vendor/rapier/runtime/rapier.mjs', import.meta.url).href;
        this.api = /** @type {RapierModule} */ (await import(moduleUrl));
        await this.api.init();
    }
    /** Plane mode solves one shared body per reflected pair in y >= 0.
     * Uniform mode keeps authored coordinates and a two-sided, filtered plane.
     * @param {WorldEntity[]} entities @param {SpringJoint[]} [joints] @param {number[]} [gravity] @param {WorldPhysicsSettings} [settings]
     */
    rebuild(entities, joints = [], gravity = [0,-9.81,0], settings = {gravityMode:'uniform',gravityStrength:9.81,objectCollisions:true,planeCollision:true}) {
        const R = this.api;
        if (!R) throw new Error('Playground physics is not initialized.');
        this.world?.free();
        this.settings = {gravityMode:settings.gravityMode,gravityStrength:settings.gravityStrength,
            objectCollisions:settings.objectCollisions,planeCollision:settings.planeCollision};
        this.world = new R.World(vector(settings.gravityMode==='plane'?[0,-settings.gravityStrength,0]:gravity));
        if(settings.planeCollision) {
            this.world.createCollider(new R.ColliderDesc(new R.HalfSpace({ x: 0, y: 1, z: 0 })).setCollisionGroups((2<<16)|1));
            if(settings.gravityMode==='uniform')this.world.createCollider(new R.ColliderDesc(new R.HalfSpace({ x: 0, y: -1, z: 0 })).setCollisionGroups((4<<16)|1));
        }
        this.bodies.clear();this.sides.clear();
        for (const e of entities.filter(e => e.alive)) this.add(e);
        this.joints = [...joints];
        for (const j of joints) this.addJoint(j);
    }
    /** @param {WorldEntity} e */
    add(e) {
        const R = this.api, world = this.world;
        if (!R || !world) throw new Error('Playground physics is not initialized.');
        const side=this.settings.gravityMode==='plane'&&e.position[1]<0?-1:1;
        this.sides.set(e.id,side);
        const position=side<0?reflectPolar(e.position):e.position;
        const rotation=quaternion(side<0?reflectEulerXYZ(e.rotation):e.rotation);
        const velocity=side<0?reflectPolar(e.velocity):e.velocity;
        const angularVelocity=side<0?reflectAxial(e.angularVelocity):e.angularVelocity;
        const desc = e.bodyType === 'fixed' ? R.RigidBodyDesc.fixed() : e.bodyType === 'kinematic' ? R.RigidBodyDesc.kinematicVelocityBased() : R.RigidBodyDesc.dynamic();
        desc.setTranslation(position[0], position[1], position[2]).setRotation(rotation).setLinvel(.../** @type {[number,number,number]} */ (velocity)).setAngvel(vector(angularVelocity)).setLinearDamping(e.damping).setAngularDamping(e.damping).setGravityScale(e.gravity===false?0:1).setCcdEnabled(true);
        const body = world.createRigidBody(desc);
        this.attachCollider(e,body,1);
        if(side<0){
            body.recomputeMassPropertiesFromColliders();
            const mass=this.reflectedMassProperties(body);
            while(body.numColliders())world.removeCollider(body.collider(0),false);
            this.attachCollider(e,body,side,mass);body.recomputeMassPropertiesFromColliders();
        }
        this.bodies.set(e.id, body);
    }
    /** @param {WorldEntity} e @param {number} side */
    collisionGroups(e,side) {
        const planeGroup=this.settings.gravityMode==='plane'||side>=0?2:4;
        const filter=e.collisions===false?0:(this.settings.objectCollisions?1:0)|(this.settings.planeCollision?planeGroup:0);
        return (1<<16)|filter;
    }
    /** Reflect local mesh vertices as well as the rotation: S R = (S R S) S.
     * This matters for asymmetric cones, pyramids and wedges.
     * @param {WorldEntity} e @param {RigidBody} body @param {number} side @param {MassProperties} [massProperties]
     */
    attachCollider(e,body,side,massProperties) {
        const R=this.api,world=this.world;
        if(!R||!world)throw new Error('Playground physics is not initialized.');
        const [x, y, z] = e.size.map(x => x / 2);
        let collider;
        if (['sphere','ellipsoid','beacon','clock','pulse','light-pulse'].includes(e.shape)) {
            if (Math.abs(x-y)<1e-10&&Math.abs(x-z)<1e-10) collider=R.ColliderDesc.ball(x);
            else {
                const points=[];
                for(let a=0;a<=16;a++)for(let b=0;b<32;b++) {const u=Math.PI*a/16,v=2*Math.PI*b/32;points.push(x*Math.sin(u)*Math.cos(v),y*Math.cos(u),z*Math.sin(u)*Math.sin(v));}
                collider=R.ColliderDesc.convexHull(new Float32Array(points));
            }
        } else if(['box','plane','ruler'].includes(e.shape)) collider=R.ColliderDesc.cuboid(x,y,z);
        else if(e.shape==='disk'&&Math.abs(x-z)<1e-10) collider=R.ColliderDesc.cylinder(y,x);
        else {
            const mesh=restMesh(e.shape==='disk'?'cylinder':e.shape);
            const vertices=new Float32Array(mesh.vertices.flatMap(v=>v.map((n,i)=>n*e.size[i]*(i===1?side:1))));
            if(e.shape==='torus') {
                // One convex prism per adjacent torus section preserves its hole and shared mesh surface.
                const parts=[],positions=[],rotations=[];
                for(let section=0;section<32;section++) {
                    const points=[];
                    for(const ring of [section,(section+1)%32])for(let k=0;k<16;k++)for(let axis=0;axis<3;axis++)points.push(vertices[(ring*16+k)*3+axis]);
                    const part=R.ColliderDesc.convexHull(new Float32Array(points));
                    if(part){parts.push(part.shape);positions.push({x:0,y:0,z:0});rotations.push({x:0,y:0,z:0,w:1});}
                }
                collider=R.ColliderDesc.compound(parts,positions,rotations);
            } else collider=R.ColliderDesc.convexHull(vertices);
        }
        if(!collider) throw new Error(`Unable to construct collision geometry for ${e.shape}.`);
        const planeSide=this.settings.gravityMode==='plane'?1:(e.position[1]<0?-1:1);
        collider.setRestitution(e.restitution).setFriction(e.friction).setMass(e.mass).setCollisionGroups(this.collisionGroups(e,planeSide));
        if(massProperties)collider.setMassProperties(massProperties.mass,massProperties.center,massProperties.inertia,massProperties.frame);
        world.createCollider(collider, body);
    }
    /** Reflect an existing inertia tensor exactly instead of asking a mirrored
     * convex hull to choose a new (potentially degenerate) principal-axis frame.
     * @param {RigidBody} body @returns {MassProperties}
     */
    reflectedMassProperties(body) {
        const center=body.localCom(),inertia=body.principalInertia(),frame=body.principalInertiaLocalFrame();
        return {mass:body.mass(),center:{x:center.x,y:-center.y,z:center.z},inertia:{x:inertia.x,y:inertia.y,z:inertia.z},frame:reflectQuaternion(frame)};
    }
    /** @param {string} id @param {number[]} impulse */
    impulse(id, impulse) {
        const body = this.bodies.get(id);
        if (!body) throw new Error('Unknown Playground body.');
        if (!body.isDynamic()) throw new Error('Impulses require a dynamic body. Change Body behavior and apply the edit first.');
        const side=this.sides.get(id)??1;
        body.applyImpulse(vector(side<0?reflectPolar(impulse):impulse), true);
        const velocity = body.linvel();
        return side<0?reflectPolar([velocity.x,velocity.y,velocity.z]):[velocity.x,velocity.y,velocity.z];
    }
    /** Apply one bounded F*dt impulse at the clicked surface anchor. Nothing is
     * left in Rapier's persistent force accumulators after release.
     * @param {import('./types.js').ForceGunGrab} grab @param {number} dt @param {boolean} [apply]
     * @returns {import('./types.js').ForceGunTelemetry}
     */
    forceGun(grab,dt,apply=true) {
        const body=this.bodies.get(grab.id);
        if(!body?.isDynamic())throw new Error('Force gun requires a live dynamic body.');
        const side=this.sides.get(grab.id)??1;
        const local=side<0?reflectPolar(grab.localAnchor):grab.localAnchor;
        const q=body.rotation(),u=[q.x,q.y,q.z];
        const cross=(/** @type {number[]} */ a,/** @type {number[]} */ b)=>[a[1]*b[2]-a[2]*b[1],a[2]*b[0]-a[0]*b[2],a[0]*b[1]-a[1]*b[0]];
        const uv=cross(u,local),uuv=cross(u,uv),p=body.translation();
        const anchor=local.map((v,i)=>v+2*(q.w*uv[i]+uuv[i])+[p.x,p.y,p.z][i]);
        const at=vector(anchor),velocity=body.velocityAtPoint(at),center=body.worldCom(),im=body.effectiveInvMass(),ii=body.effectiveWorldInvInertia();
        const inverseMass=pointInverseMass([im.x,im.y,im.z],[[ii.m11,ii.m12,ii.m13],[ii.m12,ii.m22,ii.m23],[ii.m13,ii.m23,ii.m33]],anchor.map((v,i)=>v-[center.x,center.y,center.z][i]));
        const result=computeGunForce({anchor,target:side<0?reflectPolar(grab.target):grab.target,velocity:[velocity.x,velocity.y,velocity.z],
            direction:side<0?reflectPolar(grab.direction):grab.direction,mode:grab.mode,sensitivity:grab.sensitivity,multiplier:grab.multiplier,dt,effectiveInverseMass:inverseMass});
        if(apply)body.applyImpulseAtPoint(vector(result.force.map(v=>v*dt)),at,true);
        return {token:grab.token,id:grab.id,mode:grab.mode,anchorWorld:side<0?reflectPolar(anchor):anchor,target:[...grab.target],
            force:side<0?reflectPolar(result.force):result.force,forceMagnitude:result.magnitude,requestedForceMagnitude:result.requestedMagnitude,maxForce:result.maxForce,mass:body.mass(),sensitivity:grab.sensitivity,multiplier:grab.multiplier};
    }
    /** @param {SpringJoint} joint */
    addJoint(joint) {
        const a = this.bodies.get(joint.a), b = this.bodies.get(joint.b);
        if (!this.api || !this.world || !a || !b) throw new Error('Spring endpoints must be live Playground bodies.');
        const zero = { x: 0, y: 0, z: 0 };
        this.world.createImpulseJoint(this.api.JointData.spring(joint.restLength, joint.stiffness, joint.damping, zero, zero), a, b, true);
    }
    /** @param {number} dt @param {WorldEntity[]} entities */
    step(dt, entities) {
        if (!this.world) throw new Error('Playground physics is not initialized.');
        this.world.timestep = dt;
        this.world.step();
        for(const e of entities) {
            const body=this.bodies.get(e.id);if(!body||!e.alive)continue;
            if(this.settings.gravityMode==='plane')this.foldCrossing(e,body);
            else {
                const side=body.translation().y<0?-1:1;
                for(let i=0;i<body.numColliders();i++)body.collider(i).setCollisionGroups(this.collisionGroups(e,side));
            }
        }
        this.synchronize(entities);
    }
    /** Crossing an open plane changes source side, never identity or source momentum.
     * The collider is reattached with opposite handedness; no second body is advanced.
     * @param {WorldEntity} e @param {RigidBody} body
     */
    foldCrossing(e,body) {
        const position=body.translation();if(position.y>=0||!this.world)return;
        const velocity=body.linvel(),angular=body.angvel(),rotation=body.rotation();
        const mass=this.reflectedMassProperties(body);
        const side=-(this.sides.get(e.id)??1);this.sides.set(e.id,side);
        while(body.numColliders())this.world.removeCollider(body.collider(0),false);
        body.setTranslation(vector(reflectPolar([position.x,position.y,position.z])),false);
        body.setRotation(reflectQuaternion(rotation),false);
        this.attachCollider(e,body,side,mass);
        body.recomputeMassPropertiesFromColliders();
        body.setLinvel(vector(reflectPolar([velocity.x,velocity.y,velocity.z])),true);
        body.setAngvel(vector(reflectAxial([angular.x,angular.y,angular.z])),true);
    }
    /** Read back a completed physics step.
     * @param {WorldEntity[]} entities
     */
    synchronize(entities) {
        for (const e of entities) {
            const body = this.bodies.get(e.id);
            if (!body || !e.alive) continue;
            const p = body.translation(), v = body.linvel(), av = body.angvel();
            const side=this.sides.get(e.id)??1;
            e.position = side<0?reflectPolar([p.x,p.y,p.z]):[p.x,p.y,p.z];
            e.velocity = side<0?reflectPolar([v.x,v.y,v.z]):[v.x,v.y,v.z];
            e.rotation = side<0?reflectEulerXYZ(euler(body.rotation())):euler(body.rotation());
            e.angularVelocity = side<0?reflectAxial([av.x,av.y,av.z]):[av.x,av.y,av.z];
        }
    }
    dispose() { this.world?.free(); this.world = null; this.bodies.clear(); this.sides.clear(); }
}
