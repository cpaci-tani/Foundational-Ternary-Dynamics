// @ts-check
import MiniSearch from '../vendor/minisearch/7.2.0/index.js';

/** @typedef {{indexPath:string,indexSha256:string,chunksPath:string,chunksSha256:string,count:number}} Shard */
/** @typedef {{schemaVersion:number,searchVersion:string,shards:Shard[],options:any,corpus:string,sourceRevision:string,sourceFiles?:number,coverageIssues?:any[],claimsPath?:string,claimsSha256?:string}} Manifest */
/** @type {Manifest|null} */ let manifest = null;
/** @type {URL|null} */ let base = null;
let examined = 0;
/** @type {Map<string,any>} */ const indexes = new Map();
/** @type {Map<string,Record<string,any>>} */ const texts = new Map();
const workerScope = /** @type {any} */ (self);
/** @type {Record<string,any>|null} */ let claims = null;
/** @type {Map<number,AbortController>} */ const requests = new Map();
function coverage() { return { loadedShards: examined, totalShards: manifest?.shards.length || 0, complete: !!manifest && examined === manifest.shards.length, corpus: manifest?.corpus, sourceRevision: manifest?.sourceRevision,sourceFiles:manifest?.sourceFiles,extractionGaps:manifest?.coverageIssues?.length||0 }; }
/** @param {URL} url @param {string|undefined} hash @param {AbortSignal} signal @returns {Promise<any>} */
async function json(url, hash, signal) {
    signal.throwIfAborted();
    const response = await fetch(url, { signal });
    if (!response.ok) throw new Error(`Documentation asset unavailable (${response.status})`);
    const bytes = await response.arrayBuffer();
    signal.throwIfAborted();
    if (hash) {
        const digest = Array.from(new Uint8Array(await crypto.subtle.digest('SHA-256', bytes)), x => x.toString(16).padStart(2, '0')).join('');
        if (digest !== hash) throw new Error('Documentation index integrity mismatch');
    }
    return JSON.parse(new TextDecoder().decode(bytes));
}
/** @param {string} url @param {AbortSignal} signal */
async function initialize(url, signal) {
    indexes.clear(); texts.clear(); examined = 0; claims = null;
    base = new URL(url);
    const value = await json(base, undefined, signal);
    if (value.schemaVersion !== 1 || value.searchVersion !== '7.2.0' || !Array.isArray(value.shards)) throw new Error('Unsupported documentation index');
    for (const shard of value.shards) {
        for (const key of ['indexPath', 'chunksPath']) if (!/^(index|chunks)-[a-f0-9]{20}\.json$/.test(shard[key])) throw new Error('Invalid documentation shard');
    }
    if (value.claimsPath && !/^claims-[a-f0-9]{20}\.json$/.test(value.claimsPath)) throw new Error('Invalid claim index');
    manifest = value;
}
/** @param {string} path @param {string} hash @param {AbortSignal} signal @returns {Promise<Record<string,any>>} */
async function textShard(path, hash, signal) {
    if (!base || !/^chunks-[a-f0-9]{20}\.json$/.test(path)) throw new Error('Invalid source shard');
    signal.throwIfAborted();
    if (!texts.has(path)) {
        texts.set(path, await json(new URL(path, base), hash, signal));
        if (texts.size > 4) texts.delete(/** @type {string} */ (texts.keys().next().value));
    }
    return /** @type {Record<string,any>} */ (texts.get(path));
}
/** @param {string} query @param {number} limit @param {boolean} includeHistorical @param {AbortSignal} signal */
async function search(query, limit, includeHistorical, signal) {
    if (!manifest || !base) throw new Error('Search is not initialized');
    if (typeof query !== 'string' || !query.trim() || query.length > 1024) return [];
    /** @type {any[]} */ const candidates = [];
    examined = 0;
    // All shards participate. Only eight parsed indexes and four text shards
    // survive at once; later queries reuse immutable HTTP cache bytes.
    for (const shard of manifest.shards) {
        signal.throwIfAborted();
        let index = indexes.get(shard.indexPath);
        if (!index) {
            index = MiniSearch.loadJS(await json(new URL(shard.indexPath, base), shard.indexSha256, signal), manifest.options);
            indexes.set(shard.indexPath, index);
            if (indexes.size > 8) indexes.delete(/** @type {string} */ (indexes.keys().next().value));
        }
        for (const hit of index.search(query, { prefix: true, boost: { title: 3, heading: 2, claimIds: 5 } })
            .filter((/** @type {any} */ hit) => includeHistorical || !hit.historical).slice(0, 16)) {
            candidates.push({ ...hit, shard, rank: hit.score * (hit.historical ? .5 : 1) * (hit.authority === 3 ? 1.6 : hit.authority === 2 ? 1.3 : 1) });
        }
        examined++;
        workerScope.postMessage({ type: 'coverage', coverage: coverage() });
    }
    candidates.sort((a, b) => b.rank - a.rank || a.id.localeCompare(b.id));
    /** @type {any[]} */ const results = [];
    for (const hit of candidates.slice(0, Math.max(1, Math.min(12, limit)))) {
        const chunk = (await textShard(hit.shard.chunksPath, hit.shard.chunksSha256, signal))[hit.id];
        if (chunk) results.push({ ...chunk, score: hit.rank });
    }
    // Keep retrieved text/tags unchanged and attach authoritative companions.
    if (claims === null && manifest.claimsPath) claims = await json(new URL(manifest.claimsPath, base), manifest.claimsSha256, signal);
    const queryClaims = [...query.matchAll(/FTD-\d{4,}/g)].map(match => match[0]);
    const allClaims = [...new Set([...queryClaims, ...results.flatMap(row => row.claimIds || [])])];
    /** @type {Map<string,any>} */ const companions = new Map();
    for (const claimId of allClaims.slice(0, 6)) {
        const pointer = claims?.[claimId];
        if (!pointer) { companions.set(claimId, { claimId, available: false, reason: 'No current authoritative record in this corpus' }); continue; }
        const source = (await textShard(pointer.chunksPath, pointer.chunksSha256, signal))[pointer.chunkId];
        if (!source) throw new Error('Missing authoritative source chunk');
        let text = source.text;
        if (pointer.match === 'table') text = text.split('\n').find((/** @type {string} */ line) => new RegExp('^\\|\\s*' + claimId + '\\s*\\|').test(line)) || text;
        if (pointer.match === 'controlling-notice') text = text.slice(pointer.excerptStart, pointer.excerptEnd);
        const scoped = pointer.match === 'table' || pointer.match === 'controlling-notice';
        companions.set(claimId, { ...source, text, claimId, available: true, basis: pointer.match,
            ...(scoped ? { statusTags: [...text.matchAll(/\[([A-Z][A-Z0-9 _–—/-]{1,90})\]/g)].map((/** @type {RegExpMatchArray} */ match) => match[1]),
                statusStatements: text.split('\n').filter((/** @type {string} */ line) => /\b(?:tag|status)[_* .:]*[:.]/i.test(line)).slice(0, 6) } : {}) });
    }
    for (const row of results) {
        const relevant = [...new Set([...queryClaims, ...(row.claimIds || [])])];
        row.authoritativeCompanions = relevant.slice(0, 6).map(claimId => companions.get(claimId)
            || { claimId, available: false, reason: 'Authoritative lookup bound reached; status not checked' });
        row.authoritativeLookup = { checked: relevant.filter(id => companions.has(id)).length, total: relevant.length, truncated: relevant.some(id => !companions.has(id)) };
    }
    return results;
}
// Serialize requests so coverage and corpus identity cannot cross requests.
let queue = Promise.resolve();
workerScope.addEventListener('message', (/** @type {MessageEvent<any>} */ { data }) => {
    if (data.type === 'cancel') { requests.get(data.id)?.abort(); return; }
    const controller = new AbortController(); requests.set(data.id, controller);
    queue = queue.then(async () => {
        try {
            controller.signal.throwIfAborted();
            if (data.type === 'init') { await initialize(data.manifestUrl, controller.signal); workerScope.postMessage({ id: data.id, result: true, coverage: coverage() }); }
            else if (data.type === 'search') workerScope.postMessage({ id: data.id, result: await search(data.query, data.limit ?? 4, data.includeHistorical ?? true, controller.signal), coverage: coverage() });
        } catch (error) { workerScope.postMessage({ id: data.id, error: controller.signal.aborted ? 'Search cancelled' : error instanceof Error ? error.message : 'Documentation search failed', coverage: coverage() }); }
        finally { requests.delete(data.id); }
    });
});
