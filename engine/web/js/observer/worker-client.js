// @ts-check
/** @typedef {import('./types.js').WorldSnapshot} WorldSnapshot */
/** @typedef {import('./types.js').WorldCommand} WorldCommand */
/** @typedef {import('./types.js').CommandResult} CommandResult */
/** @typedef {import('./types.js').ObserverInput} ObserverInput */
/** Nonblocking render client. Exactly one advance is in flight; elapsed time is retained. */
export class ObserverWorkerClient {
    /** @type {WorldSnapshot|null} */ latest=null;
    /** @type {import('./types.js').ForceGunTelemetry|null} */ latestForceGun=null;
    latestResponseId=0;
    /** @type {Promise<WorldSnapshot>} */ ready;
    /** @type {Map<number,{resolve:(value:CommandResult)=>void,reject:(reason:Error)=>void,cleanup?:()=>void}>} */ pending=new Map();
    sequence=1;
    disposed=false;
    /** @type {Error|null} */ failure=null;
    backlogReported=false;
    advancePending=false;
    queuedSeconds=0;
    /** @type {ObserverInput} */ queuedInput={};
    /** @type {((snapshot:WorldSnapshot)=>void)|undefined} */ onSnapshot;
    /** @type {((error:Error)=>void)|undefined} */ onError;
    /** @type {Worker} */ worker;
    /** @param {{onSnapshot?:(snapshot:WorldSnapshot)=>void,onError?:(error:Error)=>void,workerFactory?:(url:URL)=>Worker,profile?:'sr'|'playground',preset?:string,playing?:boolean}} [options] */
    constructor(options={}) {
        this.onSnapshot=options.onSnapshot;this.onError=options.onError;
        const url=new URL('./session-worker.js',import.meta.url);
        this.worker=options.workerFactory?options.workerFactory(url):new Worker(url,{type:'module',name:'MindsEyeSession'});
        this.worker.addEventListener('message',event=>this.receive(event.data));
        this.worker.addEventListener('error',event=>this.fail(new Error(event.message||'Observer worker failed.')));
        this.worker.addEventListener('messageerror',()=>this.fail(new Error('Observer worker message could not be decoded.')));
        this.ready=this.send({type:'init',options:{profile:options.profile,preset:options.preset,playing:options.playing}}).then(result=>{
            if(!result.ok) throw new Error(result.error??'Observer initialization failed.');
            return result.snapshot;
        });
    }
    get snapshot() {return this.latest;}
    /** @param {Record<string,unknown>} message @param {{signal?:AbortSignal}} [options] @returns {Promise<CommandResult>} */
    send(message, options = {}) {
        if(this.disposed) return Promise.reject(new Error('Observer client is disposed.'));
        if(this.failure) return Promise.reject(this.failure);
        if(options.signal?.aborted) return Promise.reject(new DOMException('Assistant command cancelled.', 'AbortError'));
        const id=this.sequence++;
        return new Promise((resolve,reject)=>{
            // Keep waiting for the authoritative receipt: abort may race an
            // already applied transaction, and must never claim to undo it.
            const cancel = () => { if(this.pending.has(id)) this.worker.postMessage({type:'cancel',commandId:id}); };
            options.signal?.addEventListener('abort',cancel,{once:true});
            this.pending.set(id,{resolve,reject,cleanup:()=>options.signal?.removeEventListener('abort',cancel)});
            this.worker.postMessage({...message,id});
        });
    }
    /** @param {CommandResult&{id:number}} message */
    receive(message) {
        if(this.disposed) return;
        const pending=this.pending.get(message.id);
        if(!pending) return;
        this.pending.delete(message.id);
        pending.cleanup?.();
        if(message.snapshot) {
            const old=this.latest,s=message.snapshot;
            // An epoch change is only accepted in the response sequence; old frames cannot overwrite it.
            if(message.id>this.latestResponseId&&(!old||s.sessionId!==old.sessionId||s.epoch>old.epoch||(s.epoch===old.epoch&&(s.tick>old.tick||s.tick===old.tick&&s.revision>=old.revision)))) {
                this.latestResponseId=message.id;this.latest=s;this.latestForceGun=message.forceGun??null;this.onSnapshot?.(s);
            }
        }
        pending.resolve(message);
    }
    /** @param {WorldCommand} command @param {{signal?:AbortSignal,assertActive?:()=>unknown}} [options] */
    async request(command, options = {}) {
        await this.ready;
        if(options.signal?.aborted) throw new DOMException('Assistant command cancelled.', 'AbortError');
        if(options.assertActive?.() === false) throw new Error('Observer is no longer active.');
        // Commands and advances share worker FIFO. Discard only queued presentation time on branch-changing commands.
        if(['reset','preset','profile','undo','load','scrub'].includes(command.type)) this.queuedSeconds=0;
        const result=await this.send({type:'command',command:{...command,sessionId:command.sessionId??this.latest?.sessionId,expectedEpoch:command.expectedEpoch??this.latest?.epoch}},options);
        return result;
    }
    /** @param {number} dt @param {ObserverInput} [input] */
    advance(dt,input={}) {
        if(this.disposed||!this.latest) return;
        if(!Number.isFinite(dt)||dt<0||dt>10) {this.onError?.(new Error('Invalid elapsed frame time.'));return;}
        this.queuedSeconds+=dt;this.queuedInput=input;
        if(this.queuedSeconds>5&&!this.backlogReported) {
            this.backlogReported=true;
            this.onError?.(new Error('Observer client backlog exceeds 5 seconds; playback is being paused.'));
            void this.request({type:'pause'}).catch(error=>this.onError?.(error));
        }
        this.flushAdvance();
    }
    flushAdvance() {
        if(this.advancePending||this.disposed||!this.latest||this.queuedSeconds<=0) return;
        const dt=Math.min(this.queuedSeconds,.25);this.queuedSeconds-=dt;this.advancePending=true;
        if(this.queuedSeconds<1)this.backlogReported=false;
        void this.send({type:'advance',dt,input:this.queuedInput,sessionId:this.latest.sessionId,epoch:this.latest.epoch}).then(result=>{
            if(!result.ok&&result.error!=='Stale advance epoch.') this.onError?.(new Error(result.error??'Observer advance failed.'));
        }).catch(error=>this.onError?.(error)).finally(()=>{this.advancePending=false;this.flushAdvance();});
    }
    /** @param {Error} error */
    fail(error) {this.failure=error;for(const pending of this.pending.values()) {pending.cleanup?.();pending.reject(error);}this.pending.clear();this.onError?.(error);}
    dispose() {
        if(this.disposed)return;this.disposed=true;this.worker.terminate();
        for(const pending of this.pending.values()){pending.cleanup?.();pending.reject(new Error('Observer client disposed.'));}
        this.pending.clear();this.latest=null;this.latestForceGun=null;this.queuedSeconds=0;
    }
}
