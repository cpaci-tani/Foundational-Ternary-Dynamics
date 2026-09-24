import { test } from 'node:test';
import assert from 'node:assert/strict';
import { createHash } from 'node:crypto';
import MiniSearch from '../js/vendor/minisearch/7.2.0/index.js';

const options = { fields: ['title', 'heading', 'text', 'claimIds'], storeFields: ['sourcePath', 'historical', 'authority'] };
const assets = new Map();
const shards = [];
function asset(prefix, value) {
    const body = JSON.stringify(value), hash = createHash('sha256').update(body).digest('hex');
    const path = `${prefix}-${hash.slice(0, 20)}.json`;
    assets.set('https://local.example/knowledge/' + path, body);
    return { path, hash };
}
for (let i = 0; i < 12; i++) {
    const row = { id: String(i), title: 'Source ' + i, heading: 'Evidence', text: i === 11 ? 'finalshardneedle FTD-0001 FTD-9999' : 'sharedterm',
        claimIds: i === 11 || i === 0 ? ['FTD-0001', 'FTD-9999'] : [], sourcePath: `docs/source-${i}.md`, sourceHash: 'a'.repeat(64), startLine: 2, endLine: 3,
        historical: i % 2 === 0, authority: 1, statusTags: [i % 2 === 0 ? 'RETRACTED' : 'OPEN'] };
    const search = new MiniSearch(options); search.add({ ...row, claimIds: row.claimIds.join(' ') });
    const index = asset('index', search.toJSON()), chunks = asset('chunks', { [row.id]: row });
    shards.push({ indexPath: index.path, indexSha256: index.hash, chunksPath: chunks.path, chunksSha256: chunks.hash, count: 1 });
}
const authoritative = asset('chunks', { ledger: { id: 'ledger', sourcePath: 'docs/theory/07_assessment/core_ledgers/LEDGER.md', sourceHash: 'b'.repeat(64),
    text: 'FTD-0001 is [OPEN] in the current record.', statusTags: ['OPEN'], startLine: 8, endLine: 9, authority: 3 } });
const claimIndex = asset('claims', { 'FTD-0001': { chunkId: 'ledger', chunksPath: authoritative.path, chunksSha256: authoritative.hash, match: 'heading' } });
assets.set('https://local.example/knowledge/manifest.json', JSON.stringify({ schemaVersion: 1, searchVersion: '7.2.0', options, shards, corpus: 'public', sourceRevision: 'abc', claimsPath: claimIndex.path, claimsSha256: claimIndex.hash }));
let listener, sequence = 0;
const pending = new Map(), progress = [];
globalThis.self = {
    addEventListener(type, fn) { listener = fn; },
    postMessage(message) {
        if (message.type === 'coverage') progress.push(message.coverage);
        const done = pending.get(message.id);
        if (done) { pending.delete(message.id); done(message); }
    },
};
globalThis.fetch = async url => assets.has(String(url)) ? new Response(assets.get(String(url))) : new Response('', { status: 404 });
await import('../js/assistant/search.worker.js');
function send(data) {
    return new Promise(resolve => { const id = ++sequence; pending.set(id, resolve); listener({ data: { ...data, id } }); });
}
test('complete corpus search reaches late shards and returns provenance', async () => {
    assert.equal((await send({ type: 'init', manifestUrl: 'https://local.example/knowledge/manifest.json' })).result, true);
    const result = await send({ type: 'search', query: 'finalshardneedle', limit: 4 });
    assert.equal(result.result[0].sourcePath, 'docs/source-11.md');
    assert.equal(result.result[0].sourceHash.length, 64);
    assert.equal(result.coverage.complete, true);
    assert.equal(result.coverage.loadedShards, 12);
    assert.equal(progress.at(-1).complete, true);
    assert.equal(result.result[0].authoritativeCompanions[0].sourcePath, 'docs/theory/07_assessment/core_ledgers/LEDGER.md');
    assert.equal(result.result[0].authoritativeCompanions[1].available, false);
});
test('history stays visible by default and may be filtered', async () => {
    const all = await send({ type: 'search', query: 'sharedterm', limit: 12 });
    assert.equal(all.result.length, 11);
    assert.ok(all.result.some(row => row.historical));
    const old = all.result.find(row => row.id === '0');
    assert.deepEqual(old.statusTags, ['RETRACTED']);
    assert.deepEqual(old.authoritativeCompanions[0].statusTags, ['OPEN']);
    const active = await send({ type: 'search', query: 'sharedterm', limit: 12, includeHistorical: false });
    assert.ok(active.result.length && active.result.every(row => !row.historical));
});
test('cancellation aborts an in-flight scan and the next query still succeeds', async () => {
    await send({ type: 'init', manifestUrl: 'https://local.example/knowledge/manifest.json' });
    const originalFetch = globalThis.fetch;
    let begun;
    const started = new Promise(resolve => { begun = resolve; });
    globalThis.fetch = (url, options) => {
        begun();
        return new Promise((resolve, reject) => options.signal.addEventListener('abort', () => reject(new DOMException('cancelled', 'AbortError')), { once: true }));
    };
    const running = send({ type: 'search', query: 'sharedterm' });
    const id = sequence;
    await started;
    listener({ data: { type: 'cancel', id } });
    assert.match((await running).error, /cancelled/);
    globalThis.fetch = originalFetch;
    assert.equal((await send({ type: 'search', query: 'finalshardneedle' })).result[0].id, '11');
});
test('tampered shard fails closed without claiming complete coverage', async () => {
    assets.set('https://local.example/knowledge/' + shards[0].indexPath, '{}');
    await send({ type: 'init', manifestUrl: 'https://local.example/knowledge/manifest.json' });
    const failed = await send({ type: 'search', query: 'sharedterm' });
    assert.match(failed.error, /integrity/);
    assert.equal(failed.coverage.complete, false);
});
