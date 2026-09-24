// @ts-check
import { ObserverSession } from './session.js';
/** @type {ObserverSession|null} */ let session=null;
/** @type {Promise<void>} */ let chain=Promise.resolve();
/** @type {Set<number>} */ const pendingIds = new Set();
/** @type {Set<number>} */ const cancelledIds = new Set();
/** Serialize initialization, commands, and advances into one state owner. */
globalThis.addEventListener('message', event => {
    const message=/** @type {MessageEvent<{id:number,type:string,commandId?:number,options?:object,command?:import('./types.js').WorldCommand,dt?:number,input?:import('./types.js').ObserverInput,epoch?:number,sessionId?:string}>} */(event).data;
    if(message.type==='cancel') {
        if(message.commandId!==undefined&&pendingIds.has(message.commandId))cancelledIds.add(message.commandId);
        return;
    }
    pendingIds.add(message.id);
    chain=chain.then(async()=>{
        if(message.type==='init') {
            session?.dispose();session=new ObserverSession(message.options);await session.initialize();
            globalThis.postMessage({id:message.id,ok:true,snapshot:session.snapshot(),forceGun:null});return;
        }
        if(!session) throw new Error('Observer worker is not initialized.');
        if(message.type==='dispose') {session.dispose();session=null;globalThis.postMessage({id:message.id,ok:true});return;}
        if(message.type==='command') {
            if(!message.command) throw new Error('Missing world command.');
            // Give a queued cancellation a turn before starting a guarded
            // assistant edit. Simulation advances keep their original cadence.
            if(message.command.expectedPreparationVersion!==undefined)await new Promise(resolve=>setTimeout(resolve,0));
            globalThis.postMessage({id:message.id,...await session.command(message.command,{isCancelled:()=>cancelledIds.has(message.id)})});return;
        }
        if(message.type==='advance') {
            if((message.sessionId!==undefined&&message.sessionId!==session.state.sessionId)||(message.epoch!==undefined&&message.epoch!==session.state.epoch)) {
                globalThis.postMessage({id:message.id,ok:false,error:'Stale advance epoch.',snapshot:session.snapshot(),forceGun:session.getForceGunTelemetry()});return;
            }
            session.advance(message.dt??0,message.input??{});
            globalThis.postMessage({id:message.id,ok:true,snapshot:session.snapshot(),forceGun:session.getForceGunTelemetry()});return;
        }
        throw new Error('Unsupported worker message.');
    }).catch(error=>{session?.releaseForceGun();globalThis.postMessage({id:message.id,ok:false,error:error instanceof Error?error.message:String(error),snapshot:session?.snapshot(),forceGun:null});})
        .finally(()=>{pendingIds.delete(message.id);cancelledIds.delete(message.id);});
});
