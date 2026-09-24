// @ts-check
import { add, sub, scale, length, normalize, gamma, clamp, FIXED_DT, MAX_BETA, cameraBasis, integrateFourVelocity, elasticCollision1D } from './math.js';
import { ENTITY_LIMIT, HISTORY_WINDOW, SEGMENT_LIMIT, MAX_GRAVITY_STRENGTH } from './types.js';
import { PlaygroundPhysics } from './playground.js';
import { forceGunSettings } from './force-gun-physics.js';
/** @typedef {import('./types.js').WorldEntity} WorldEntity */
/** @typedef {import('./types.js').WorldSegment} WorldSegment */
/** @typedef {import('./types.js').WorldSnapshot} WorldSnapshot */
/** @typedef {import('./types.js').WorldCommand} WorldCommand */
/** @typedef {import('./types.js').ObserverInput} ObserverInput */
/** @typedef {import('./types.js').CommandResult} CommandResult */
const SHAPES = new Set(['sphere','box','plane','disk','capsule','cylinder','cone','torus','tetrahedron','octahedron','icosahedron','dodecahedron','ellipsoid','pyramid','prism','wedge','clock','ruler','beacon','pulse','light-pulse']);
/** @template T @param {T} value @returns {T} */
const clone = value => structuredClone(value);
/** @param {unknown} value @param {string} name @param {number} [min] @param {number} [max] */
function number(value, name, min = -1e12, max = 1e12) {
    if (typeof value !== 'number' || !Number.isFinite(value) || value < min || value > max) throw new Error(`${name} must be finite in [${min}, ${max}].`);
    return value;
}
/** @param {unknown} value @param {string} name @param {number} [limit] @returns {number[]} */
function vec(value, name, limit = 1e12) {
    if (!Array.isArray(value) || value.length !== 3) throw new Error(`${name} must contain three numbers.`);
    return value.map(x => number(x, name, -limit, limit));
}
/** @param {unknown} value @returns {Record<string,unknown>} */
function record(value) {
    if (!value || typeof value !== 'object' || Array.isArray(value)) throw new Error('Expected a data object.');
    return /** @type {Record<string,unknown>} */ (value);
}
/** Reject executable, cyclic, nonfinite, or excessively nested imported data. @param {unknown} value @param {number} [depth] @param {Set<object>} [seen] */
function assertPortable(value, depth=0, seen=new Set()) {
    if(depth>32) throw new Error('Snapshot nesting exceeds the data limit.');
    if(value===null||typeof value==='string'||typeof value==='boolean') return;
    if(typeof value==='number') {if(!Number.isFinite(value))throw new Error('Snapshot contains a nonfinite number.');return;}
    if(typeof value!=='object') throw new Error('Snapshot must contain portable data only.');
    if(seen.has(value)) throw new Error('Snapshot contains circular references.');
    seen.add(value);for(const item of Object.values(value))assertPortable(item,depth+1,seen);seen.delete(value);
}
/** @param {WorldEntity} entity @param {Record<string,unknown>} patch @param {'sr'|'playground'} profile */
function applyEntityPatch(entity, patch, profile) {
    for (const key of ['position','size','rotation','velocity','color','angularVelocity','properAcceleration']) {
        if (!(key in patch)) continue;
        const value = vec(patch[key], key);
        if (key === 'size' && value.some(x => x <= 0 || x > 1e5)) throw new Error('Dimensions must be positive and no larger than 100000.');
        if (key === 'color' && value.some(x => x < 0 || x > 1)) throw new Error('Color channels must be in [0,1].');
        Object.assign(entity, { [key]: value });
    }
    for (const key of ['emission','mass','restitution','friction','damping']) if (key in patch) Object.assign(entity, { [key]: number(patch[key], key, key === 'mass' ? 1e-6 : 0, key === 'restitution' ? 1 : 1e5) });
    for (const key of ['name','shape','bodyType','spectral']) if (key in patch) {
        if (typeof patch[key] !== 'string' || String(patch[key]).length > 120) throw new Error(`${key} must be a short string.`);
        Object.assign(entity, { [key]: patch[key] });
    }
    if ('overlay' in patch) {
        if (typeof patch.overlay !== 'boolean') throw new Error('Overlay must be boolean.');
        entity.overlay = patch.overlay;
    }
    for (const key of ['gravity','collisions']) if (key in patch) {
        if (typeof patch[key] !== 'boolean') throw new Error(`${key} must be boolean.`);
        Object.assign(entity,{[key]:patch[key]});
    }
    if (!SHAPES.has(entity.shape)) throw new Error('Unknown geometric primitive.');
    if (!['fixed','dynamic','kinematic'].includes(entity.bodyType)) throw new Error('Unknown body type.');
    if (profile === 'sr') {
        if (length(entity.velocity) > MAX_BETA + 1e-12) throw new Error('SR velocity must not exceed 0.99c.');
        if (length(entity.angularVelocity) > 0) throw new Error('Continuous rigid rotation is available only in Playground.');
        if (length(entity.properAcceleration) > 0 && !['clock','beacon'].includes(entity.shape)) throw new Error('SR acceleration is restricted to point-clock or beacon markers.');
    }
}
/** Authoritative, finite reference world. It never imports or mutates the lattice owner. */
export class ObserverSession {
    /** @type {WorldSnapshot} */ state;
    /** @type {WorldSnapshot[]} */ undoStack = [];
    /** @type {PlaygroundPhysics|null} */ playground = null;
    /** @type {number} */ accumulator = 0;
    nextId = 1;
    disposed = false;
    initialized = false;
    relocated = false;
    backlogPaused = false;
    /** Transient controls never enter world snapshots or undo checkpoints. @type {import('./types.js').ForceGunGrab|null} */ forceGunGrab = null;
    /** @type {import('./types.js').ForceGunTelemetry|null} */ forceGunTelemetry = null;
    /** @type {Set<string>} */ retiredGunTokens = new Set();
    /** @param {{profile?:'sr'|'playground',preset?:string,playing?:boolean}} [options] */
    constructor(options = {}) {
        this.state = {
            schemaVersion: 1, sessionId: globalThis.crypto?.randomUUID?.() ?? `observer-${Date.now()}-${Math.random().toString(36).slice(2)}`,
            epoch: 1, revision: 0, preparationVersion: 0, tick: 0, time: 0, profile: options.profile ?? 'sr', playing: options.playing ?? true,
            observer: { position: [0,1.6,8], velocity: [0,0,0], properTime: 0, yaw: 0, pitch: 0, roll: 0, worldline: 1 },
            entities: [], segments: [], historyStart: -HISTORY_WINDOW, historyWindow: HISTORY_WINDOW,
            environment: { preset: 'void', seed: 1, radius: 40, density: 1, spacing: 4, orientation: 0, opacity: 1, color: [.2,.7,1], animationRate: 0, anchor: 'world' },environmentHistory:[],
            playbackSpeed: 1, units: 'normalized (c = 1)', warnings: [], backlogSeconds: 0, experiment: options.preset ?? 'baseline',preparedEvents:true,scrubTime: null, pulses: [], joints: [], gravity:[0,-9.81,0], gravityMode:'plane',gravityStrength:9.81,objectCollisions:true,planeCollision:true, physicsEngine: options.profile === 'playground' ? 'Rapier 0.20.0' : 'Minkowski reference c=1', collisionOccurred: false,
        };
        this.prepare(this.state.experiment);
    }
    async initialize() {
        if (this.disposed) throw new Error('Session is disposed.');
        if (this.state.profile === 'playground') await this.initializePlayground();
        this.initialized = true;
        return this.snapshot();
    }
    async initializePlayground() {
        this.releaseForceGun();
        if (!this.playground) { this.playground = new PlaygroundPhysics(); await this.playground.initialize(); }
        this.playground.rebuild(this.state.entities, this.state.joints, this.state.gravity, this.state);
    }
    /** @returns {WorldSnapshot} */
    snapshot() {
        const result = clone(this.state);
        result.backlogSeconds = this.accumulator;
        if (result.scrubTime !== null) {
            const time = result.scrubTime;
            result.time = time;
            const environment=result.environmentHistory.find(e=>e.start<=time&&(e.end===null||time<e.end));
            if(environment)result.environment=clone(environment.environment);
            result.entities = result.entities.map(e => {
                const s = result.segments.find(s => s.entityId === e.id && s.start <= time && (s.end === null || time < s.end));
                return s ? { ...e, ...clone(s), id: e.id, position: add(s.position, scale(s.velocity, time - s.originTime)), originTime: time, clockOffset: s.clockOffset + (time - s.originTime) / gamma(s.velocity), alive: true } : { ...e, alive: false };
            });
        }
        return result;
    }
    /** @param {Partial<WorldEntity>} properties @param {boolean} [prehistory] */
    addEntity(properties = {}, prehistory = false) {
        if (this.state.entities.length >= ENTITY_LIMIT) throw new Error(`Entity limit ${ENTITY_LIMIT} reached; no authored entity was evicted.`);
        const id = `object-${this.nextId++}`;
        /** @type {WorldEntity} */
        const entity = { id, name: 'Geometric object', shape: 'sphere', position: [0,1,-2], size: [1,1,1], rotation: [0,0,0], velocity: [0,0,0], color: [.15,.7,1], emission: 1, mass: 1, overlay: false, alive: true, revision: 1, clockOffset: 0, originTime: this.state.time, createdAt: prehistory ? -HISTORY_WINDOW : this.state.time, deletedAt: null, bodyType: 'dynamic', restitution: .5, friction: .5, damping: .05, gravity:true,collisions:true, angularVelocity: [0,0,0], properAcceleration: [0,0,0], spectral: 'white' };
        applyEntityPatch(entity, /** @type {Record<string,unknown>} */ (properties), this.state.profile);
        entity.editRevision = 1;
        this.state.entities.push(entity);
        this.openSegment(entity, prehistory ? -HISTORY_WINDOW : this.state.time);
        return entity;
    }
    /** @param {WorldEntity} e @param {number} [start] @param {number} [originTime] */
    openSegment(e, start = this.state.time, originTime = this.state.time) {
        if (this.state.segments.length >= SEGMENT_LIMIT) throw new Error(`Optical history limit ${SEGMENT_LIMIT} reached. Pause or reduce recorded trajectories.`);
        this.state.segments.push({ entityId:e.id, revision:e.revision, start, end:null, originTime, position:[...e.position], velocity:[...e.velocity], size:[...e.size], rotation:[...e.rotation], shape:e.shape, color:[...e.color], emission:e.emission, clockOffset:e.clockOffset, name:e.name, mass:e.mass, spectral:e.spectral });
    }
    /** @param {WorldEntity} e @param {number} [time] */
    closeSegment(e, time = this.state.time) {
        for (let i = this.state.segments.length - 1; i >= 0; i--) {
            const s = this.state.segments[i];
            if (s.entityId !== e.id || s.end !== null) continue;
            if (s.start === time) this.state.segments.splice(i, 1);
            else s.end = time;
            break;
        }
    }
    /** @param {string} preset */
    prepare(preset) {
        const aliases = /** @type {Record<string,string>} */ ({ 'approaching-receding':'clocks', 'approaching-receding-clocks':'clocks', 'moving-cube-sphere':'moving-shapes', 'synchronized-clocks':'clock-avenue', 'light-clock-journey':'light-clock', 'point-collision':'collision', 'delayed-intervention':'intervention', 'delayed-edit':'intervention' });
        preset = aliases[preset] ?? preset;
        if (!['baseline','clocks','moving-shapes','clock-avenue','light-clock','twin-journey','collision','intervention','performance'].includes(preset)) throw new Error('Unknown prepared experiment.');
        this.state.experiment = preset;this.state.preparedEvents=true; this.state.entities = []; this.state.segments = []; this.state.pulses = []; this.state.joints = []; this.state.collisionOccurred = false;
        this.state.time = 0; this.state.tick = 0; this.state.historyStart = -HISTORY_WINDOW; this.state.scrubTime = null; this.accumulator = 0;
        this.state.environmentHistory=[{start:-HISTORY_WINDOW,end:null,revision:1,environment:clone(this.state.environment)}];
        this.state.observer = { position:[0,1.6,8],velocity:[0,0,0],properTime:0,yaw:0,pitch:0,roll:0,worldline:this.state.observer.worldline+1 };
        if (preset === 'baseline' || preset === 'intervention') {
            this.addEntity({name:'Luminous sphere',shape:'sphere',position:[0,1,-2],color:[.15,.8,1],overlay:true},true);
            this.addEntity({name:'Amber cube',shape:'box',position:[-3,.8,-4],size:[1.6,1.6,1.6],color:[1,.45,.12],rotation:[0,.4,0]},true);
            this.addEntity({name:'Violet octahedron',shape:'octahedron',position:[3,1.2,-5],size:[2,2,2],color:[.65,.3,1]},true);
            this.addEntity({name:'Reference clock',shape:'clock',position:[0,3,-7],size:[.4,.4,.4],overlay:true},true);
        } else if (preset === 'clocks') {
            this.addEntity({name:'Approaching clock beta=0.6',shape:'clock',position:[-2,1.6,-6],velocity:[0,0,.6],size:[.7,.7,.7],overlay:true},true);
            this.addEntity({name:'Receding clock beta=0.8',shape:'clock',position:[2,1.6,-6],velocity:[0,0,-.8],size:[.7,.7,.7],color:[1,.3,.2],overlay:true},true);
        } else if (preset === 'moving-shapes') {
            this.addEntity({name:'Moving cube',shape:'box',position:[-3,1,-4],velocity:[.8,0,0],size:[2,2,2],overlay:true},true);
            this.addEntity({name:'Moving sphere',shape:'sphere',position:[3,1,-8],velocity:[-.8,0,0],size:[2,2,2],color:[1,.5,.15],overlay:true},true);
        } else if (preset === 'clock-avenue') {
            for(let i=0;i<12;i++) for(const side of [-1,1]) this.addEntity({name:`Synchronized clock ${i}-${side}`,shape:'clock',position:[side*3,1.6,4-i*4],size:[.35,.35,.35],overlay:true},true);
        } else if (preset === 'light-clock') {
            this.addEntity({name:'Lower mirror',shape:'box',position:[0,1,-4],size:[2,.1,1],velocity:[.6,0,0],overlay:true},true);
            this.addEntity({name:'Upper mirror',shape:'box',position:[0,3,-4],size:[2,.1,1],velocity:[.6,0,0],overlay:true},true);
            this.addEntity({name:'Light-clock center',shape:'clock',position:[0,2,-4],size:[.15,.15,.15],velocity:[.6,0,0],overlay:true},true);
            this.state.pulses.push({id:'light-clock-0',origin:[0,1,-4],start:0,color:[.3,.9,1]});
        } else if (preset === 'twin-journey') {
            this.addEntity({name:'Stay-at-home clock',shape:'clock',position:[-1,1.6,-4],size:[.3,.3,.3],overlay:true},true);
            this.addEntity({name:'Traveling point clock',shape:'clock',position:[1,1.6,-4],size:[.3,.3,.3],velocity:[.6,0,0],overlay:true,color:[1,.5,.2]},true);
        } else if (preset === 'collision') {
            this.addEntity({name:'Point A (m=1)',shape:'beacon',position:[-2,1.6,-3],velocity:[.6,0,0],size:[.16,.16,.16],overlay:true},true);
            this.addEntity({name:'Point B (m=2)',shape:'beacon',position:[2,1.6,-3],velocity:[-.3,0,0],mass:2,size:[.16,.16,.16],color:[1,.4,.2],overlay:true},true);
        } else {
            for(let i=0;i<64;i++) this.addEntity({name:`Benchmark body ${i}`,shape:i%2?'sphere':'box',position:[(i%8-3.5)*2,1+Math.floor(i/32)*2,-3-Math.floor((i%32)/8)*3],size:[.7,.7,.7],velocity:[i%2?.08:-.08,0,0],overlay:i<32},true);
        }
        if (this.state.profile === 'playground') this.state.physicsEngine = 'Rapier 0.20.0';
    }
    /** @param {WorldSnapshot} snapshot */
    checkpoint(snapshot) {
        this.undoStack.push(snapshot);
        while (this.undoStack.length > 8 || (this.undoStack.length > 1 && JSON.stringify(this.undoStack).length > 16_000_000)) this.undoStack.shift();
    }
    releaseForceGun() {
        if(this.forceGunGrab)this.retiredGunTokens.add(this.forceGunGrab.token);
        // End-before-begin cancellation is bounded independently of authored state.
        while(this.retiredGunTokens.size>256)this.retiredGunTokens.delete(/** @type {string} */(this.retiredGunTokens.values().next().value));
        this.forceGunGrab=null;this.forceGunTelemetry=null;
    }
    getForceGunTelemetry() {return clone(this.forceGunTelemetry);}
    /** @param {Record<string,unknown>} data */
    gunTarget(data) {
        const target=vec(data.target,'force-gun target'),direction=vec(data.direction,'force-gun direction');
        if(length(direction)<1e-12)throw new Error('Force-gun direction must be nonzero.');
        const sensitivity=String(data.sensitivity??'normal'),multiplier=number(data.multiplier??1,'force-gun multiplier',.1,10);
        forceGunSettings(sensitivity,multiplier);
        return {target,direction:normalize(direction),sensitivity:/** @type {'delicate'|'normal'|'strong'} */(sensitivity),multiplier};
    }
    /** @param {Record<string,unknown>} data @returns {boolean} */
    updateForceGun(data) {
        const grab=this.forceGunGrab;
        if(!grab||data.token!==grab.token||data.epoch!==this.state.epoch||data.epoch!==grab.epoch)return false;
        if(!Number.isSafeInteger(data.sequence)||/** @type {number} */(data.sequence)<=grab.sequence)return false;
        const target=this.gunTarget(data);
        Object.assign(grab,target,{sequence:data.sequence});return true;
    }
    /** Transient commands do not checkpoint, rebuild the solver, or revise objects.
     * @param {WorldCommand} command @returns {CommandResult}
     */
    gunCommand(command) {
        try {
            if(this.disposed)throw new Error('Session is disposed.');
            if(command.sessionId!==undefined&&command.sessionId!==this.state.sessionId)throw new Error('Stale session identity.');
            if(command.expectedEpoch!==this.state.epoch)throw new Error('Stale or missing force-gun epoch.');
            const data=command.payload??command,token=data.token;
            if(typeof token!=='string'||!token.length||token.length>128)throw new Error('Force-gun token must be a short nonempty string.');
            if(command.type==='gun-end') {
                this.retiredGunTokens.add(token);
                if(this.forceGunGrab?.token===token)this.releaseForceGun();
                while(this.retiredGunTokens.size>256)this.retiredGunTokens.delete(/** @type {string} */(this.retiredGunTokens.values().next().value));
            } else if(command.type==='gun-update') {
                if(!this.updateForceGun(data))throw new Error('Stale force-gun update.');
            } else {
                if(this.retiredGunTokens.has(token))throw new Error('Force-gun interaction has already ended.');
                if(this.forceGunGrab)throw new Error('Release the current force-gun interaction first.');
                if(this.state.profile!=='playground'||!this.playground)throw new Error('The force gun is available only in Playground.');
                if(this.state.scrubTime!==null||data.historical!==false)throw new Error('The force gun requires a current live surface.');
                const entity=this.state.entities.find(e=>e.id===data.id);
                if(!entity?.alive||entity.bodyType!=='dynamic')throw new Error('The force gun requires a live dynamic body.');
                if(command.expectedTargetRevision!==entity.revision)throw new Error('Stale or missing object revision.');
                if(data.mode!=='pull'&&data.mode!=='push')throw new Error('Force-gun mode must be pull or push.');
                const localAnchor=vec(data.localAnchor,'local surface anchor');
                if(localAnchor.some((v,i)=>Math.abs(v)>entity.size[i]/2+1e-4))throw new Error('Force-gun anchor is outside the object bounds.');
                const sequence=data.sequence??0;
                if(!Number.isSafeInteger(sequence)||/** @type {number} */(sequence)<0)throw new Error('Force-gun sequence must be a nonnegative safe integer.');
                /** @type {import('./types.js').ForceGunGrab} */
                const grab={token,id:entity.id,epoch:this.state.epoch,targetRevision:entity.revision,sequence:/** @type {number} */(sequence),mode:data.mode,localAnchor,...this.gunTarget(data)};
                this.forceGunTelemetry=this.playground.forceGun(grab,FIXED_DT,false);
                this.forceGunGrab=grab;
                if(this.relocated){this.state.observer.properTime=0;this.state.observer.worldline++;this.relocated=false;}
                this.state.playing=true;this.state.preparedEvents=false;this.state.revision++;
            }
            return {ok:true,snapshot:this.snapshot(),forceGun:this.getForceGunTelemetry()};
        } catch(error) {return {ok:false,error:error instanceof Error?error.message:String(error),snapshot:this.snapshot(),forceGun:this.getForceGunTelemetry()};}
    }
    /** @param {WorldCommand} command @param {{isCancelled?:()=>boolean}} [options] @returns {Promise<CommandResult>} */
    async command(command, options = {}) {
        if(['gun-begin','gun-update','gun-end'].includes(command.type))return this.gunCommand(command);
        const before = clone(this.state), previousId = this.nextId, previousAccumulator=this.accumulator, previousRelocated=this.relocated;
        const previousUndo = this.undoStack.slice();
        try {
            if (options.isCancelled?.()) throw new Error('Assistant command cancelled before application.');
            if (this.disposed) throw new Error('Session is disposed.');
            if (command.sessionId !== undefined && command.sessionId !== this.state.sessionId) throw new Error('Stale session identity.');
            if (command.expectedEpoch !== undefined && command.expectedEpoch !== this.state.epoch) throw new Error('Stale session epoch.');
            if (command.expectedRevision !== undefined && command.expectedRevision !== this.state.revision) throw new Error('Stale session revision.');
            if (command.expectedPreparationVersion !== undefined && command.expectedPreparationVersion !== (this.state.preparationVersion ?? 0)) throw new Error('Stale preparation version.');
            const payload = command.payload ?? command;
            const id = String(command.id ?? payload.id ?? '');
            let entity = this.state.entities.find(e => e.id === id);
            if (command.expectedTargetRevision !== undefined && entity?.revision !== command.expectedTargetRevision) throw new Error('Stale object revision.');
            if (command.expectedTargetEditRevision !== undefined && (entity?.editRevision ?? 0) !== command.expectedTargetEditRevision) throw new Error('Stale object edit revision.');
            if(!['play','observer','dolly','settings'].includes(command.type))this.releaseForceGun();
            const edits = ['create','update','delete','duplicate','restore','environment','impulse','joint'].includes(command.type);
            if (edits && this.state.scrubTime !== null) { this.branchAtScrub(); entity=this.state.entities.find(e=>e.id===id); }
            const changesParticipants=['create','delete','duplicate','restore','impulse','joint'].includes(command.type);
            const physicalPatch=command.type==='update'&&Object.keys(record(command.patch??payload.patch??{})).some(key=>['position','size','rotation','velocity','mass','shape','properAcceleration','angularVelocity','bodyType'].includes(key));
            // Prepared trajectories certify the untouched preparation only. Authoring may not
            // schedule a later reversal/contact for a deleted, moved, or replaced participant.
            if(changesParticipants||physicalPatch)this.state.preparedEvents=false;
            if (command.type === 'create') this.addEntity(/** @type {Partial<WorldEntity>} */ (record(command.entity ?? payload.entity ?? payload.patch ?? {})));
            else if (['update','delete','duplicate','restore'].includes(command.type)) {
                if (!entity) throw new Error('Unknown object identity.');
                if (command.type === 'duplicate') this.addEntity({...entity,name:`${entity.name} copy`,position:add(entity.position,[1,0,0])});
                else if (command.type === 'restore') {
                    if (entity.alive) throw new Error('Object is already alive.');
                    entity.alive=true; entity.deletedAt=null; entity.revision++; this.openSegment(entity);
                } else {
                    if (!entity.alive) throw new Error('Historical object is deleted; restore or duplicate it first.');
                    this.closeSegment(entity);
                    if (command.type === 'delete') { entity.alive=false; entity.deletedAt=this.state.time; this.state.joints=this.state.joints.filter(j=>j.a!==entity.id&&j.b!==entity.id); }
                    else applyEntityPatch(entity, record(command.patch ?? payload.patch ?? {}), this.state.profile);
                    entity.revision++;
                    if (entity.alive) this.openSegment(entity);
                }
            } else if (command.type === 'step') {
                if (this.state.playing || this.state.scrubTime !== null) throw new Error('Pause at the present before stepping.');
                const count = number(command.count ?? payload.count ?? 1, 'step count', 1, 120);
                if (!Number.isInteger(count)) throw new Error('Step count must be an integer.');
                for (let i = 0; i < count; i++) this.step(FIXED_DT, {});
            } else if (command.type === 'pause') this.state.playing=false;
            else if (command.type === 'play') {
                if (this.state.scrubTime !== null) throw new Error('Return to the present or edit to branch before playing.');
                if (this.relocated) { this.state.observer.properTime=0; this.state.observer.worldline++; this.relocated=false; }
                this.state.playing=true;
            } else if (command.type === 'dolly') {
                // Wheel travel is a camera relocation, not a physical velocity or a
                // projection change. Coordinates have no scene-distance limit.
                const displacement=vec(command.displacement??payload.displacement,'displacement',Infinity);
                this.relocateObserver(vec(add(this.state.observer.position,displacement),'observer position',Infinity));
            } else if (command.type === 'observer') {
                const patch=record(command.patch??payload.patch??{}), observer=this.state.observer;
                if ('position' in patch) this.relocateObserver(vec(patch.position,'position',Infinity));
                if ('velocity' in patch) { observer.velocity=vec(patch.velocity,'velocity'); if (length(observer.velocity)>MAX_BETA) throw new Error('Observer velocity exceeds 0.99c.'); }
                for(const k of ['yaw','pitch','roll']) if(k in patch) Object.assign(observer,{[k]:number(patch[k],k)});
                if ('playbackSpeed' in patch) this.state.playbackSpeed=number(patch.playbackSpeed,'playback speed',.01,20);
                if ('units' in patch) this.state.units=String(patch.units).slice(0,80);
            } else if(command.type==='settings') {
                const patch=record(command.patch??payload.patch??{});
                if('playbackSpeed' in patch) this.state.playbackSpeed=number(patch.playbackSpeed,'playback speed',.01,20);
                if('units' in patch) this.state.units=String(patch.units).slice(0,80);
            } else if(command.type==='gravity') {
                if(this.state.profile!=='playground'||!this.playground?.world) throw new Error('Gravity is available only in Playground.');
                this.state.gravity=vec(command.value??payload.value,'gravity');
                this.state.gravityMode='uniform';
                this.state.gravityStrength=Math.hypot(...this.state.gravity);
                await this.initializePlayground();
            } else if(command.type==='world-physics') {
                if(this.state.profile!=='playground') throw new Error('World gravity and collisions are available only in Playground.');
                const patch=record(command.patch??payload.patch??{});
                for(const key of Object.keys(patch))if(!['gravityMode','gravityStrength','gravity','objectCollisions','planeCollision'].includes(key))throw new Error(`Unknown world physics setting: ${key}.`);
                if('gravityMode' in patch) {
                    if(patch.gravityMode!=='plane'&&patch.gravityMode!=='uniform')throw new Error('Gravity mode must be plane or uniform.');
                    this.state.gravityMode=patch.gravityMode;
                }
                if('gravityStrength' in patch)this.state.gravityStrength=number(patch.gravityStrength,'gravity strength',0,MAX_GRAVITY_STRENGTH);
                if('gravity' in patch)this.state.gravity=vec(patch.gravity,'gravity');
                for(const key of ['objectCollisions','planeCollision'])if(key in patch){
                    if(typeof patch[key]!=='boolean')throw new Error(`${key} must be boolean.`);
                    Object.assign(this.state,{[key]:patch[key]});
                }
            } else if (command.type === 'environment') {
                const patch=record(command.patch??payload.patch??{}), environment=this.state.environment;
                for(const key of ['radius','density','spacing','opacity','seed','orientation','animationRate']) if(key in patch) Object.assign(environment,{[key]:number(patch[key],key, ['orientation','animationRate'].includes(key)?-1e6:0,1e6)});
                if(environment.spacing<=0||environment.radius<=0||environment.opacity>1) throw new Error('Environment radius/spacing must be positive; opacity must be [0,1].');
                if('color' in patch) {environment.color=vec(patch.color,'color');if(environment.color.some(c=>c<0||c>1))throw new Error('Environment color channels must be in [0,1].');}
                if('preset' in patch) environment.preset=String(patch.preset).slice(0,80);
                if('anchor' in patch) { if(!['world','camera'].includes(String(patch.anchor))) throw new Error('Invalid shell anchoring.'); environment.anchor=/** @type {'world'|'camera'} */(patch.anchor); }
                const last=this.state.environmentHistory.at(-1),revision=(last?.revision??0)+1;
                if(last?.start===this.state.time)this.state.environmentHistory.pop();
                else if(last)last.end=this.state.time;
                if(this.state.environmentHistory.length>=16)throw new Error('Environment history limit 16 reached; wait for old optical history to expire or reset the preparation.');
                this.state.environmentHistory.push({start:this.state.time,end:null,revision,environment:clone(environment)});
            } else if (command.type === 'reset' || command.type === 'preset') {
                this.state.epoch++; this.prepare(String(command.preset??payload.preset??this.state.experiment));
            } else if (command.type === 'profile') {
                const profile=command.profile??payload.profile;
                if(profile!=='sr'&&profile!=='playground') throw new Error('Unknown physics profile.');
                this.state.profile=profile; this.state.epoch++; this.state.physicsEngine=profile==='sr'?'Minkowski reference c=1':'Rapier 0.20.0';
                this.prepareProfile();
            } else if (command.type === 'undo') {
                const previous=this.undoStack.pop();
                if(!previous) throw new Error('No author transaction to undo.');
                this.state=clone(previous); this.state.epoch=before.epoch+1; this.state.revision=before.revision;
                this.accumulator=0; this.state.playing=false; this.state.scrubTime=null;
            } else if(command.type==='scrub') {
                if(this.state.profile==='playground') throw new Error('Historical optical scrubbing is available in SR Reference.');
                const time=command.time??payload.time;
                this.state.scrubTime=time===null?null:number(time,'scrub time',this.state.historyStart,this.state.time);
                this.state.playing=false;
            } else if(command.type==='impulse') {
                if(this.state.profile!=='playground'||!this.playground) throw new Error('Impulses are available only in initialized Playground.');
                if(!entity?.alive) throw new Error('Select a live object before applying an impulse.');
                if(entity.bodyType!=='dynamic') throw new Error('Impulses require a dynamic body. Change Body behavior and apply the edit first.');
                const impulse=vec(command.impulse??payload.impulse,'impulse');
                if(!impulse.some(value=>value!==0)) throw new Error('Set a nonzero impulse to change momentum.');
                // A central impulse changes only linear momentum. Read its
                // velocity directly; never run a zero-duration solver step or
                // round-trip an unchanged Float64 author pose through Rapier.
                entity.velocity=this.playground.impulse(id,impulse);
                entity.revision++;
            } else if(command.type==='joint') {
                if(this.state.profile!=='playground'||!this.playground) throw new Error('Spring joints are available only in Playground.');
                const joint={a:String(payload.a),b:String(payload.b),restLength:number(payload.restLength??1,'rest length',0),stiffness:number(payload.stiffness??10,'stiffness',0),damping:number(payload.damping??1,'damping',0)};
                if(joint.a===joint.b) throw new Error('Spring endpoints must differ.');
                if(this.state.joints.length>=256) throw new Error('Joint limit reached.');
                this.playground.addJoint(joint); this.state.joints.push(joint);
            } else if(command.type==='pulse') {
                if(!entity||!entity.alive) throw new Error('Select a live pulse emitter.');
                this.state.pulses.push({id:`pulse-${this.state.revision}`,origin:[...entity.position],start:this.state.time,color:[...entity.color]});
                if(this.state.pulses.length>256) throw new Error('Pulse history limit reached.');
            } else if(command.type==='load') {
                this.load(record(command.snapshot??payload.snapshot)); this.state.epoch=before.epoch+1; this.state.sessionId=before.sessionId; this.state.playing=false;
            } else throw new Error(`Unsupported command ${command.type}.`);
            if(this.state.profile==='playground'&&!['play','pause','step','observer','dolly','settings','gravity','environment','impulse','joint'].includes(command.type)) await this.initializePlayground();
            if (options.isCancelled?.()) throw new Error('Assistant command cancelled before application.');
            if(this.state.profile==='sr'&&this.playground) { this.playground.dispose(); this.playground=null; }
            if(!['play','pause','step','observer','dolly','scrub','undo'].includes(command.type)) this.checkpoint(before);
            if (entity && ['update','delete','restore','impulse'].includes(command.type)) entity.editRevision = (entity.editRevision ?? 0) + 1;
            this.state.preparationVersion = (before.preparationVersion ?? 0) + 1;
            this.state.revision++; this.prune();
            return {ok:true,snapshot:this.snapshot(),forceGun:this.getForceGunTelemetry()};
        } catch(error) {
            this.releaseForceGun();
            this.state=before; this.nextId=previousId; this.accumulator=previousAccumulator; this.relocated=previousRelocated;
            this.undoStack = previousUndo;
            if(this.playground&&this.state.profile==='playground'&&command.type!=='dolly') this.playground.rebuild(this.state.entities,this.state.joints,this.state.gravity,this.state);
            return {ok:false,error:error instanceof Error?error.message:String(error),snapshot:this.snapshot(),forceGun:null};
        }
    }
    /** Begin a new clock origin after privileged camera relocation. @param {number[]} position */
    relocateObserver(position) {
        const observer=this.state.observer;
        observer.position=position; observer.properTime=0; observer.worldline++; observer.velocity=[0,0,0]; this.relocated=true;
    }
    branchAtScrub() {
        const time=this.state.scrubTime;
        if(time===null) return;
        const observed=this.snapshot();
        this.state.entities=observed.entities; this.state.time=time; this.state.tick=Math.max(0,Math.floor(time/FIXED_DT));
        this.state.segments=this.state.segments.filter(s=>s.start<=time).map(s=>({...s,end:s.end===null||s.end>time?null:s.end}));
        this.state.environmentHistory=this.state.environmentHistory.filter(e=>e.start<=time).map(e=>({...e,end:e.end===null||e.end>time?null:e.end}));
        const environment=this.state.environmentHistory.at(-1);if(environment)this.state.environment=clone(environment.environment);
        this.state.pulses=this.state.pulses.filter(p=>p.start<=time); this.state.scrubTime=null; this.state.epoch++; this.accumulator=0;
        this.state.preparedEvents=false;this.state.collisionOccurred=false;
        this.state.observer.properTime=0; this.state.observer.velocity=[0,0,0]; this.state.observer.worldline++;
    }
    /** @param {Record<string,unknown>} data */
    load(data) {
        this.releaseForceGun();
        assertPortable(data);
        if('forceGun' in data||'forceGunGrab' in data||'forceGunTelemetry' in data)throw new Error('Snapshots cannot contain live force-gun controls.');
        if(data.schemaVersion!==1||!Array.isArray(data.entities)||!Array.isArray(data.segments)) throw new Error('Unsupported snapshot schema.');
        if(data.entities.length>ENTITY_LIMIT||data.segments.length>SEGMENT_LIMIT) throw new Error('Snapshot exceeds world limits.');
        if(data.profile!=='sr'&&data.profile!=='playground') throw new Error('Invalid snapshot physics profile.');
        const candidate=/** @type {WorldSnapshot} */(clone(data));
        candidate.preparationVersion ??= 0;
        if (!Number.isSafeInteger(candidate.preparationVersion) || candidate.preparationVersion < 0) throw new Error('Invalid preparation version.');
        number(candidate.time,'snapshot time'); number(candidate.historyStart,'history start');
        if(candidate.historyStart>candidate.time) throw new Error('Invalid history coverage.');
        number(candidate.historyWindow,'history window',1,HISTORY_WINDOW);
        if(candidate.time-candidate.historyStart>candidate.historyWindow+1e-8) throw new Error('Snapshot history coverage exceeds its declared window.');
        for(const key of ['tick','epoch','revision']) if(!Number.isSafeInteger(candidate[/** @type {'tick'|'epoch'|'revision'} */(key)])||candidate[/** @type {'tick'|'epoch'|'revision'} */(key)]<0) throw new Error(`Invalid snapshot ${key}.`);
        number(candidate.playbackSpeed,'playback speed',.01,20);
        if(typeof candidate.units!=='string'||candidate.units.length>80||typeof candidate.experiment!=='string'||typeof candidate.playing!=='boolean') throw new Error('Invalid session settings.');
        const ids=new Set();
        for(const e of candidate.entities) {
            e.editRevision ??= 0;
            if (!Number.isSafeInteger(e.editRevision) || e.editRevision < 0) throw new Error('Invalid object edit revision.');
            if(typeof e.id!=='string'||e.id.length>120||ids.has(e.id)) throw new Error('Snapshot object identities must be unique short strings.');
            if(typeof e.alive!=='boolean'||typeof e.overlay!=='boolean') throw new Error('Invalid object lifecycle flags.');
            e.gravity??=true;
            if(e.collisions===undefined)e.collisions=true;
            ids.add(e.id); applyEntityPatch(e,record(e),candidate.profile); number(e.clockOffset,'clock'); number(e.originTime,'origin time'); number(e.revision,'revision',0);
            number(e.createdAt,'object creation time');if(e.deletedAt!==null)number(e.deletedAt,'object deletion time');
        }
        /** @type {Map<string,WorldSegment[]>} */ const segmentsByEntity=new Map();
        for(const s of candidate.segments) {
            if(!ids.has(s.entityId)) throw new Error('Segment refers to an absent object.');
            number(s.start,'segment start'); number(s.originTime,'segment origin'); number(s.clockOffset,'clock');
            if(s.end!==null&&number(s.end,'segment end')<=s.start) throw new Error('Invalid segment interval.');
            number(s.revision,'segment revision',0);number(s.emission,'segment emission',0);number(s.mass,'segment mass',1e-6);
            if(!SHAPES.has(s.shape)||typeof s.name!=='string'||typeof s.spectral!=='string')throw new Error('Invalid segment shape or material.');
            for(const k of ['position','velocity','size','rotation','color']) vec(s[/** @type {'position'|'velocity'|'size'|'rotation'|'color'} */(k)],k);
            if(s.size.some(x=>x<=0)||s.color.some(x=>x<0||x>1))throw new Error('Invalid historical dimensions or color.');
            if(length(s.velocity)>MAX_BETA+1e-12&&candidate.profile==='sr') throw new Error('Imported SR segment velocity exceeds 0.99c.');
            const list=segmentsByEntity.get(s.entityId)??[];list.push(s);segmentsByEntity.set(s.entityId,list);
        }
        for(const segments of segmentsByEntity.values()) {
            segments.sort((a,b)=>a.start-b.start);
            for(let i=1;i<segments.length;i++)if(segments[i-1].end===null||/** @type {number} */(segments[i-1].end)>segments[i].start)throw new Error('Historical worldtube intervals overlap.');
        }
        vec(candidate.observer.position,'observer position',Infinity); vec(candidate.observer.velocity,'observer velocity');
        if(length(candidate.observer.velocity)>MAX_BETA) throw new Error('Observer speed exceeds limit.');
        number(candidate.observer.properTime,'observer clock',0);
        for(const key of ['yaw','pitch','roll','worldline'])number(candidate.observer[/** @type {'yaw'|'pitch'|'roll'|'worldline'} */(key)],key);
        const env=candidate.environment;
        for(const key of ['seed','radius','density','spacing','orientation','opacity','animationRate'])number(env[/** @type {'seed'|'radius'|'density'|'spacing'|'orientation'|'opacity'|'animationRate'} */(key)],key);
        vec(env.color,'environment color');if(env.radius<=0||env.spacing<=0||env.opacity<0||env.opacity>1||!['world','camera'].includes(env.anchor)||typeof env.preset!=='string')throw new Error('Invalid environment settings.');
        candidate.environmentHistory??=[{start:candidate.historyStart,end:null,revision:1,environment:clone(env)}];
        if(candidate.environmentHistory.length>16)throw new Error('Environment history exceeds 16 retained configurations.');
        for(const entry of candidate.environmentHistory){number(entry.start,'environment start');if(entry.end!==null&&number(entry.end,'environment end')<=entry.start)throw new Error('Invalid environment interval.');number(entry.revision,'environment revision',0);record(entry.environment);}
        candidate.pulses??=[]; candidate.joints??=[]; candidate.warnings=[]; candidate.backlogSeconds=0; candidate.scrubTime=null;
        candidate.gravity??=[0,-9.81,0];vec(candidate.gravity,'gravity');
        // Schema 1 worlds predating plane gravity retain their uniform vector.
        if(candidate.gravityMode===undefined)candidate.gravityMode='uniform';
        if(candidate.gravityStrength===undefined)candidate.gravityStrength=Math.hypot(...candidate.gravity);
        if(candidate.objectCollisions===undefined)candidate.objectCollisions=true;
        if(candidate.planeCollision===undefined)candidate.planeCollision=true;
        if(!['plane','uniform'].includes(candidate.gravityMode))throw new Error('Invalid imported gravity mode.');
        number(candidate.gravityStrength,'gravity strength',0,MAX_GRAVITY_STRENGTH);
        if(typeof candidate.objectCollisions!=='boolean'||typeof candidate.planeCollision!=='boolean')throw new Error('Invalid imported collision settings.');
        candidate.preparedEvents??=true;
        candidate.physicsEngine=candidate.profile==='sr'?'Minkowski reference c=1':'Rapier 0.20.0';
        if(candidate.pulses.length>256||candidate.joints.length>256)throw new Error('Snapshot exceeds instrument or joint budget.');
        for(const p of candidate.pulses){number(p.start,'pulse start');vec(p.origin,'pulse origin');vec(p.color,'pulse color');}
        for(const j of candidate.joints){if(!ids.has(j.a)||!ids.has(j.b)||j.a===j.b)throw new Error('Invalid joint endpoints.');number(j.restLength,'rest length',0);number(j.stiffness,'stiffness',0);number(j.damping,'damping',0);}
        this.state=candidate; this.accumulator=0;
        this.nextId=Math.max(this.nextId,1,...candidate.entities.map(e=>Number(e.id.replace('object-',''))+1).filter(Number.isFinite));
    }
    /** @param {number} elapsedSeconds @param {ObserverInput} [input] */
    advance(elapsedSeconds,input={}) {
        if(this.disposed) return;
        if(!Number.isFinite(elapsedSeconds)||elapsedSeconds<0||elapsedSeconds>10) throw new Error('Elapsed advance must be finite within 0–10 seconds.');
        if(input.forceGun)try{this.updateForceGun(record(input.forceGun));}catch(error){this.releaseForceGun();this.state.warnings=[error instanceof Error?error.message:String(error)];}
        const o=this.state.observer;
        if(input.yaw!==undefined) o.yaw=number(input.yaw,'yaw');
        if(input.pitch!==undefined) o.pitch=clamp(number(input.pitch,'pitch'),-Math.PI/2+.001,Math.PI/2-.001);
        if(input.roll!==undefined) o.roll=number(input.roll,'roll');
        if(!this.state.playing||this.state.scrubTime!==null) {
            if(this.state.scrubTime===null&&length(input.move??[0,0,0])>0) { o.position=add(o.position,scale(this.movement(input),elapsedSeconds*(input.speed??.5)*6)); this.relocated=true; }
            return;
        }
        this.accumulator+=elapsedSeconds*this.state.playbackSpeed;
        if(this.accumulator>5&&!this.backlogPaused) { this.releaseForceGun();this.backlogPaused=true;this.state.playing=false; this.state.warnings=['Simulation backlog exceeded 5 s; paused with all unprocessed time retained. Resume to drain retained time.']; return; }
        if(this.accumulator<1) this.backlogPaused=false;
        let steps=0;
        while(this.accumulator+1e-12>=FIXED_DT&&steps<30) {
            try {this.step(FIXED_DT,input);} catch(error) {this.releaseForceGun();this.state.playing=false;this.state.warnings=[error instanceof Error?error.message:String(error)];break;}
            this.accumulator=Math.max(0,this.accumulator-FIXED_DT);steps++;
        }
        if(this.state.playing) this.state.warnings=this.accumulator>FIXED_DT*2?['Simulation is running slower than requested playback; unprocessed time retained.']:[];
        this.prune();
    }
    /** @param {ObserverInput} input */
    movement(input) {
        const move=vec(input.move??[0,0,0],'movement'), o=this.state.observer;
        const basis=cameraBasis(o.yaw,input.grounded?0:o.pitch,!input.grounded&&input.worldUp===false?o.roll:0);
        let direction=add(add(scale(basis.right,move[0]),scale(input.worldUp===false?basis.up:[0,1,0],input.grounded?0:move[1])),scale(basis.forward,move[2]));
        direction=direction.map((v,i)=>input.axisLocks?.[i]?0:v);
        return length(direction)>1?normalize(direction):direction;
    }
    /** @param {number} dt @param {ObserverInput} input */
    step(dt,input) {
        const s=this.state,o=s.observer;
        const accelerated=s.profile==='sr'?s.entities.filter(e=>e.alive&&length(e.properAcceleration)>0):[];
        if(s.segments.length+accelerated.length>SEGMENT_LIMIT) throw new Error('Optical history budget exhausted; simulation paused before losing history.');
        const desired=scale(this.movement(input),clamp(input.speed??.5,0,MAX_BETA));
        if(s.profile==='playground') {
            if(!this.playground) throw new Error('Playground is still loading.');
            // Ordinary game kinematics: acceleration changes coordinate velocity and
            // the observer clock shares the world's coordinate time, without gamma.
            const delta=sub(desired,o.velocity),change=scale(normalize(delta),Math.min(length(delta),Math.max(0,input.acceleration??1)*dt));
            const velocity=add(o.velocity,change);
            o.position=add(o.position,scale(add(o.velocity,velocity),dt/2));o.velocity=velocity;o.properTime+=dt;
            const grab=this.forceGunGrab;
            if(grab){
                const entity=s.entities.find(e=>e.id===grab.id);
                if(grab.epoch!==s.epoch||!entity?.alive||entity.revision!==grab.targetRevision||entity.bodyType!=='dynamic')this.releaseForceGun();
                else this.forceGunTelemetry=this.playground.forceGun(grab,dt);
            }
            this.playground.step(dt,s.entities);
            for(const e of s.entities.filter(e=>e.alive)) {e.clockOffset+=dt;e.originTime=s.time+dt;}
        } else {
            const targetU=scale(desired,gamma(desired)),currentU=scale(o.velocity,gamma(o.velocity));
            const delta=sub(targetU,currentU),force=scale(normalize(delta),Math.min(length(delta)/dt,Math.max(0,input.acceleration??1)));
            const next=integrateFourVelocity(o.velocity,force,dt);
            o.position=add(o.position,next.displacement);o.velocity=next.velocity;o.properTime+=next.properElapsed;
            for(const e of s.entities.filter(e=>e.alive)) {
                e.position=add(e.position,scale(e.velocity,dt));e.clockOffset+=dt/gamma(e.velocity);e.originTime=s.time+dt;
            }
            s.time+=dt;
            for(const e of accelerated) {
                this.closeSegment(e);e.velocity=integrateFourVelocity(e.velocity,e.properAcceleration,dt).velocity;e.revision++;this.openSegment(e);
            }
            this.demoEvents(s.time-dt,s.time);
            s.time-=dt;
        }
        s.time+=dt;s.tick++;
    }
    /** @param {number} from @param {number} to */
    demoEvents(from,to) {
        const s=this.state;
        if(!s.preparedEvents)return;
        if(s.experiment==='twin-journey') {
            const traveler=s.entities[1];
            if(!traveler?.alive){s.preparedEvents=false;return;}
            for(const event of [{time:10,velocity:-.6},{time:20,velocity:0}]) if(from<event.time&&to>=event.time) {
                this.velocityEvent(traveler,event.time,to,[event.velocity,0,0]);
            }
        }
        if(s.experiment==='collision'&&!s.collisionOccurred) {
            const [a,b]=s.entities;
            if(!a?.alive||!b?.alive){s.preparedEvents=false;return;}
            const collisionTime=4/.9;
            if(from<collisionTime&&to>=collisionTime) {
                const result=elasticCollision1D(a.mass,a.velocity[0],b.mass,b.velocity[0]);
                this.velocityEvent(a,collisionTime,to,[result.velocityA,0,0]);this.velocityEvent(b,collisionTime,to,[result.velocityB,0,0]);s.collisionOccurred=true;
            }
        }
        if(s.experiment==='light-clock'&&Math.floor(from/2.5)<Math.floor(to/2.5)) {
            const cycle=Math.floor(to/2.5), emitter=s.entities[cycle%2];
            if(!emitter?.alive){s.preparedEvents=false;return;}
            const eventTime=cycle*2.5;
            s.pulses.push({id:`light-clock-${cycle}`,origin:add(emitter.position,scale(emitter.velocity,eventTime-to)),start:eventTime,color:[.3,.9,1]});
        }
    }
    /** A prepared point event is placed at its exact event time within the integration step.
     * @param {WorldEntity} entity @param {number} eventTime @param {number} now @param {number[]} velocity
     */
    velocityEvent(entity,eventTime,now,velocity) {
        const remainder=now-eventTime;
        entity.position=sub(entity.position,scale(entity.velocity,remainder));
        entity.clockOffset-=remainder/gamma(entity.velocity);
        this.closeSegment(entity,eventTime);entity.velocity=velocity;entity.revision++;
        this.openSegment(entity,eventTime,eventTime);
        entity.position=add(entity.position,scale(velocity,remainder));entity.clockOffset+=remainder/gamma(velocity);entity.originTime=now;
    }
    /** A profile change preserves the authored scene but declares a fresh preparation. */
    prepareProfile() {
        const s=this.state;
        s.time=0;s.tick=0;s.scrubTime=null;s.segments=[];s.pulses=[];s.preparedEvents=false;s.collisionOccurred=false;
        s.historyStart=-s.historyWindow;s.entities=s.entities.filter(e=>e.alive);this.accumulator=0;this.backlogPaused=false;
        s.observer.properTime=0;s.observer.worldline++;
        const changes=[];
        if(s.profile==='sr') {
            for(const e of s.entities) {
                if(length(e.velocity)>MAX_BETA){e.velocity=[0,0,0];changes.push(`${e.name}: speed above 0.99c reset to rest`);}
                if(length(e.angularVelocity)>0){e.angularVelocity=[0,0,0];changes.push(`${e.name}: continuous rotation cleared`);}
                if(length(e.properAcceleration)>0&&!['clock','beacon'].includes(e.shape)){e.properAcceleration=[0,0,0];changes.push(`${e.name}: extended-body acceleration cleared`);}
            }
            s.joints=[];
        }
        for(const e of s.entities){e.clockOffset=0;e.originTime=0;e.createdAt=-s.historyWindow;e.deletedAt=null;e.revision++;this.openSegment(e,-s.historyWindow,0);}
        s.environmentHistory=[{start:-s.historyWindow,end:null,revision:1,environment:clone(s.environment)}];
        s.warnings=['Profile changed at the current poses. Clocks and optical history restart with declared inertial prehistory; prescribed demonstration events are disabled.',...changes];
    }
    prune() {
        const s=this.state,cut=s.time-s.historyWindow;
        s.historyStart=Math.max(s.historyStart,cut);
        s.segments=s.segments.filter(segment=>segment.end===null||segment.end>cut);
        s.environmentHistory=s.environmentHistory.filter(entry=>entry.end===null||entry.end>cut);
        s.pulses=s.pulses.filter(p=>p.start>=cut);
        // Deleted identity records remain while photons from their retained segments can be inspected.
        const referenced=new Set(s.segments.map(segment=>segment.entityId));
        s.entities=s.entities.filter(e=>e.alive||referenced.has(e.id)||(e.deletedAt!==null&&e.deletedAt>=cut));
    }
    dispose() { this.releaseForceGun();this.disposed=true;this.state.playing=false;this.playground?.dispose();this.playground=null;this.undoStack=[]; }
}
