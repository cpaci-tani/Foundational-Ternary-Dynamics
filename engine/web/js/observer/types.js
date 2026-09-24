// @ts-check
/** @typedef {number[]} Vec3 */
/** @typedef {'sr'|'playground'} PhysicsProfile */
/** @typedef {{position:Vec3,velocity:Vec3,properTime:number,yaw:number,pitch:number,roll:number,worldline:number}} ObserverState */
/** @typedef {{id:string,name:string,shape:string,position:Vec3,size:Vec3,rotation:Vec3,velocity:Vec3,color:Vec3,emission:number,mass:number,overlay:boolean,alive:boolean,revision:number,editRevision?:number,clockOffset:number,originTime:number,createdAt:number,deletedAt:number|null,bodyType:string,restitution:number,friction:number,damping:number,gravity:boolean,collisions:boolean,angularVelocity:Vec3,properAcceleration:Vec3,spectral:string}} WorldEntity */
/** @typedef {{entityId:string,revision:number,start:number,end:number|null,originTime:number,position:Vec3,velocity:Vec3,size:Vec3,rotation:Vec3,shape:string,color:Vec3,emission:number,clockOffset:number,name:string,mass:number,spectral:string}} WorldSegment */
/** @typedef {{preset:string,seed:number,radius:number,density:number,spacing:number,orientation:number,opacity:number,color:Vec3,animationRate:number,anchor:'world'|'camera'}} EnvironmentState */
/** @typedef {{start:number,end:number|null,revision:number,environment:EnvironmentState}} EnvironmentRevision */
/** @typedef {{token:string,epoch:number,sequence:number,target:Vec3,direction:Vec3,sensitivity:'delicate'|'normal'|'strong',multiplier:number}} ForceGunInput */
/** @typedef {{token:string,id:string,epoch:number,targetRevision:number,sequence:number,mode:'pull'|'push',localAnchor:Vec3,target:Vec3,direction:Vec3,sensitivity:'delicate'|'normal'|'strong',multiplier:number}} ForceGunGrab */
/** @typedef {{token:string,id:string,mode:'pull'|'push',anchorWorld:Vec3,target:Vec3,force:Vec3,forceMagnitude:number,requestedForceMagnitude:number,maxForce:number,mass:number,sensitivity:string,multiplier:number}} ForceGunTelemetry */
/** @typedef {{move?:Vec3,yaw?:number,pitch?:number,roll?:number,speed?:number,acceleration?:number,grounded?:boolean,axisLocks?:boolean[],worldUp?:boolean,forceGun?:ForceGunInput}} ObserverInput */
/** @typedef {{id:string,origin:Vec3,start:number,color:Vec3}} LightPulse */
/** @typedef {{a:string,b:string,restLength:number,stiffness:number,damping:number}} SpringJoint */
/** @typedef {{gravityMode:'plane'|'uniform',gravityStrength:number,objectCollisions:boolean,planeCollision:boolean}} WorldPhysicsSettings */
/** @typedef {{schemaVersion:1,sessionId:string,epoch:number,revision:number,preparationVersion?:number,tick:number,time:number,profile:PhysicsProfile,playing:boolean,observer:ObserverState,entities:WorldEntity[],segments:WorldSegment[],historyStart:number,historyWindow:number,environment:EnvironmentState,environmentHistory:EnvironmentRevision[],playbackSpeed:number,units:string,warnings:string[],backlogSeconds:number,experiment:string,preparedEvents:boolean,scrubTime:number|null,pulses:LightPulse[],joints:SpringJoint[],gravity:Vec3,gravityMode:'plane'|'uniform',gravityStrength:number,objectCollisions:boolean,planeCollision:boolean,physicsEngine:string,collisionOccurred:boolean}} WorldSnapshot */
/** @typedef {{type:string,id?:string,entity?:Partial<WorldEntity>,patch?:Record<string,unknown>,payload?:Record<string,unknown>,expectedEpoch?:number,expectedRevision?:number,expectedTargetRevision?:number,expectedPreparationVersion?:number,expectedTargetEditRevision?:number,sessionId?:string,[key:string]:unknown}} WorldCommand */
/** @typedef {{ok:boolean,error?:string,snapshot:WorldSnapshot,forceGun?:ForceGunTelemetry|null}} CommandResult */
export const OBSERVER_SCHEMA_VERSION = 1;
export const ENTITY_LIMIT = 256;
export const HISTORY_WINDOW = 60;
export const SEGMENT_LIMIT = 65536;
/** Covers the norm of every legacy gravity vector with component bounds 1e12. */
export const MAX_GRAVITY_STRENGTH = 2e12;
