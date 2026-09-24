// @ts-check
/** Credentials are scoped to this in-memory instance. */
export class JevClient {
    /** @param {{apiBase?:string,localConfigured?:boolean,fetchImpl?:typeof fetch}} [options] */
    constructor({apiBase='',localConfigured=false,fetchImpl=fetch} = {}) {
        this.apiBase=apiBase; this.localConfigured=localConfigured;
        // Window.fetch checks its receiver. Calling an unbound copy as this.fetch
        // would pass the JevClient instance and throw before sending any request.
        this.fetch=fetchImpl.bind(globalThis);
        this.key=''; this.calls=0;this.disabled=false;
    }
    /** @param {string} value */
    setKey(value) { this.key=value.trim();this.disabled=false; }
    disconnect() {this.key='';this.disabled=true;}
    get connected() { return !this.disabled && (!!this.key || this.localConfigured); }
    /** @param {any} request @param {AbortSignal} signal */
    async evaluate(request, signal) {
        if (!this.connected) throw new Error('Connect a JEV key before asking AI to change the simulation. Local explanations and search remain available.');
        const headers = /** @type {Record<string,string>} */ ({'Content-Type':'application/json'});
        if (this.key) headers.Authorization=`Bearer ${this.key}`;
        const controller=new AbortController();
        const abort=()=>controller.abort(signal.reason);
        signal.addEventListener('abort',abort,{once:true});
        const timeout=setTimeout(()=>controller.abort(new Error('JEV request timed out')),20000);
        try {
            signal.throwIfAborted(); ++this.calls;
            const response=await this.fetch(`${this.apiBase}/api/ai/jev`,{method:'POST',headers,body:JSON.stringify(request),signal:controller.signal,cache:'no-store',credentials:'omit'});
            if (!response.ok) throw new Error(response.status===401 ? 'JEV key was rejected.' : response.status===429 ? 'JEV request limit reached. Try again later.' : `JEV service unavailable (${response.status}).`);
            const answer=await response.json();
            if (!['execute','clarify','reject'].includes(answer.decision) || !Number.isFinite(answer.confidence) || answer.confidence<0 || answer.confidence>1) throw new Error('Invalid JEV decision; no action was executed.');
            return answer;
        } finally { clearTimeout(timeout); signal.removeEventListener('abort',abort); }
    }
    dispose() { this.key=''; }
}
