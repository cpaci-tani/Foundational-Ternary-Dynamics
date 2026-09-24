import { createReadStream } from 'node:fs';
import { readFile, mkdir, writeFile } from 'node:fs/promises';
import { createInterface } from 'node:readline';
import { createHash } from 'node:crypto';
import { resolve } from 'node:path';
import MiniSearch from '../../engine/web/js/vendor/minisearch/7.2.0/index.js';

const [input, metadataPath, destination] = process.argv.slice(2);
if (!destination) throw new Error('Expected input, metadata, destination');
await mkdir(destination, { recursive: true });
const files = [], shards = [];
const claims = {};
const options = { fields: ['title', 'heading', 'text', 'claimIds'], storeFields: ['sourcePath', 'historical', 'authority'] };
const save = async (prefix, value) => {
    const body = JSON.stringify(value);
    const sha256 = createHash('sha256').update(body).digest('hex');
    const path = `${prefix}-${sha256.slice(0, 20)}.json`;
    await writeFile(resolve(destination, path), body);
    files.push({ path, sha256, size: Buffer.byteLength(body) });
    return { path, sha256 };
};
let rows = [];
async function flush() {
    if (!rows.length) return;
    const search = new MiniSearch(options);
    search.addAll(rows.map(row => ({ ...row, claimIds: row.claimIds.join(' ') })));
    const index = await save('index', search.toJSON());
    const chunks = await save('chunks', Object.fromEntries(rows.map(row => [row.id, row])));
    shards.push({ indexPath: index.path, indexSha256: index.sha256, chunksPath: chunks.path, chunksSha256: chunks.sha256, count: rows.length });
    for (const row of rows) {
        if (row.historical || row.authority < 2) continue;
        for (const claimId of row.claimIds) {
            const heading = row.heading.startsWith(claimId + ':') || row.heading.startsWith(claimId + ' ');
            const table = row.text.split('\n').find(line => new RegExp('^\\|\\s*' + claimId + '\\s*\\|').test(line));
            const noticeText = row.authority === 3 ? row.text.split(/\n\s*\n/).find(paragraph =>
                /(?:supersession|correction|successor|retirement|retraction)/i.test(paragraph)
                && /(?:notice|supersession)/i.test(paragraph) && paragraph.includes(claimId)) : null;
            const notice = !!noticeText;
            // Ledger cross-references do not establish another claim's status.
            if (row.authority === 3 && !heading && !table && !notice) continue;
            const priority = row.authority * 100 + (notice ? 50 : heading ? 25 : table ? 15 : 0) + (row.statusStatements?.length ? 10 : 0);
            if (claims[claimId] && claims[claimId].priority >= priority) continue;
            claims[claimId] = { priority, chunkId: row.id, chunksPath: chunks.path, chunksSha256: chunks.sha256,
                match: notice ? 'controlling-notice' : heading ? 'heading' : table ? 'table' : 'constitution',
                ...(notice ? { excerptStart: row.text.indexOf(noticeText), excerptEnd: row.text.indexOf(noticeText) + noticeText.length } : {}) };
        }
    }
    rows = [];
}
for await (const line of createInterface({ input: createReadStream(input), crlfDelay: Infinity })) {
    if (!line) continue;
    rows.push(JSON.parse(line));
    if (rows.length >= 256) await flush();
}
await flush();
const claimIndex = await save('claims', claims);
const { sourceDocuments = [], ...metadata } = JSON.parse(await readFile(metadataPath, 'utf8'));
if (metadata.corpus !== 'local' && sourceDocuments.length) throw new Error('Local source pages cannot enter the public corpus');
files.push(...sourceDocuments);
const manifest = { schemaVersion: 1, searchLibrary: 'minisearch', searchVersion: '7.2.0', ...metadata, options, files, shards,
    claimsPath: claimIndex.path, claimsSha256: claimIndex.sha256 };
await writeFile(resolve(destination, 'manifest.json'), JSON.stringify(manifest, null, 2) + '\n');
console.log(JSON.stringify({ corpus: metadata.corpus, sourceFiles: metadata.sourceFiles, chunks: metadata.chunks, shards: shards.length, bytes: files.reduce((sum, file) => sum + file.size, 0) }));
