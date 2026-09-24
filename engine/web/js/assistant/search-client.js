// @ts-check
/** @typedef {{loadedShards:number,totalShards:number,complete:boolean,corpus?:string,sourceRevision?:string,sourceFiles?:number,extractionGaps?:number}} Coverage */
export class KnowledgeSearch {
    /** @param {{manifestUrl:string,onCoverage?:((coverage:Coverage)=>void)|null,workerFactory?:((url:URL)=>Worker)|null}} options */
    constructor({ manifestUrl, onCoverage = null, workerFactory = null }) {
        /** @type {Coverage} */ this.coverage = { loadedShards: 0, totalShards: 0, complete: false };
        this.onCoverage = onCoverage;
        /** @type {Map<number,{resolve:(value:any)=>void,reject:(error:Error)=>void,cleanup:()=>void}>} */ this.pending = new Map();
        this.sequence = 0; this.disposed = false;this.manifestUrl=new URL(manifestUrl, document.baseURI).href;
        const url = new URL('./search.worker.js', import.meta.url);
        this.worker = workerFactory ? workerFactory(url) : new Worker(url, { type: 'module', name: 'FTDDocumentationSearch' });
        this.worker.addEventListener('message', ({ data }) => {
            if (data.coverage) { this.coverage = data.coverage; this.onCoverage?.(data.coverage); }
            const item = this.pending.get(data.id);
            if (!item) return;
            this.pending.delete(data.id);
            item.cleanup();
            if (data.error) item.reject(new Error(data.error)); else item.resolve(data.result);
        });
        this.worker.addEventListener('error', () => this.fail(new Error('Documentation worker failed')));
        this.ready = this.request({ type: 'init', manifestUrl: this.manifestUrl });
        // A panel may be created before the index is built; do not report an
        // unhandled rejection until its caller actually requests a search.
        this.ready.catch(() => {});
    }
    /** @param {Record<string,any>} message @param {AbortSignal} [signal] @returns {Promise<any>} */
    request(message, signal) {
        if (this.disposed) return Promise.reject(new Error('Documentation search disposed'));
        if (signal?.aborted) return Promise.reject(new DOMException('Search cancelled', 'AbortError'));
        const id = ++this.sequence;
        return new Promise((resolve, reject) => {
            const cancel = () => {
                this.worker.postMessage({ type: 'cancel', id });
                this.pending.delete(id); cleanup();
                reject(new DOMException('Search cancelled', 'AbortError'));
            };
            const timer = setTimeout(cancel, 120000);
            const cleanup = () => { clearTimeout(timer); signal?.removeEventListener('abort', cancel); };
            signal?.addEventListener('abort', cancel, { once: true });
            this.pending.set(id, { resolve, reject, cleanup });
            this.worker.postMessage({ ...message, id });
        });
    }
    /** @param {string} query @param {{limit?:number,includeHistorical?:boolean,signal?:AbortSignal}} [options] @returns {Promise<any[]>} */
    async search(query, { limit = 4, includeHistorical = true, signal } = {}) {
        if (signal?.aborted) throw new DOMException('Search cancelled', 'AbortError');
        await new Promise((resolve, reject) => {
            const abort = () => { cleanup(); reject(new DOMException('Search cancelled', 'AbortError')); };
            const cleanup = () => signal?.removeEventListener('abort', abort);
            signal?.addEventListener('abort', abort, { once: true });
            this.ready.then(value => { cleanup(); resolve(value); }, error => { cleanup(); reject(error); });
        });
        return this.request({ type: 'search', query, limit, includeHistorical }, signal);
    }
    /** @param {Error} error */
    fail(error) { for (const item of this.pending.values()) { item.cleanup(); item.reject(error); } this.pending.clear(); }
    dispose() { if (this.disposed) return; this.disposed = true; this.worker.terminate(); this.fail(new Error('Documentation search disposed')); }
}
