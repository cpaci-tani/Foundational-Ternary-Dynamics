import { test, expect } from '@playwright/test';
import { writeFile } from 'node:fs/promises';

test('real documentation worker scans the complete corpus and recovers after cancellation', async ({ page, request }, testInfo) => {
    test.setTimeout(150000);
    const statusResponse = await request.get('/api/ai/status');
    test.skip(statusResponse.status() === 404, 'The local AI asset service is unavailable on this host.');
    expect(statusResponse.ok()).toBe(true);
    const status = await statusResponse.json();
    test.skip(status.knowledgeReady !== true, 'Local documentation index is not built; run scripts/assistant/build_knowledge.py --mode local.');
    const manifestResponse = await request.get(status.knowledgeManifest);
    expect(manifestResponse.ok(), 'An advertised local index must be fetchable').toBe(true);
    const manifest = await manifestResponse.json();
    expect(manifest.shards.length).toBeGreaterThan(8);

    // A same-origin document is enough for the real module worker. No renderer,
    // model inference, replacement index, or mocked fetch participates here.
    const fetchedChunks = new Set();
    page.on('response', response => {
        if (/\/chunks-[a-f0-9]{20}\.json$/.test(response.url()) && response.ok()) fetchedChunks.add(response.url());
    });
    await page.goto('/js/assistant/config.json');
    const result = await page.evaluate(async manifestUrl => {
        const { KnowledgeSearch } = await import('/js/assistant/search-client.js');
        const controller = new AbortController();
        let cancelling = true, observedProgressBeforeCancel = 0;
        const search = new KnowledgeSearch({ manifestUrl, onCoverage: coverage => {
            if (cancelling && coverage.loadedShards > 0) {
                observedProgressBeforeCancel = coverage.loadedShards;
                controller.abort();
            }
        } });
        try {
            await search.ready;
            let cancellation = null;
            try { await search.search('FTD-1023', { limit: 6, signal: controller.signal }); }
            catch (error) { cancellation = { name: error.name, message: error.message }; }
            cancelling = false;
            const started = performance.now();
            const sources = await search.search('FTD-1023', { limit: 6 });
            return { cancellation, observedProgressBeforeCancel, sources,
                coverage: search.coverage, searchMs: performance.now() - started };
        } finally { search.dispose(); }
    }, status.knowledgeManifest);

    expect(result.cancellation?.name).toBe('AbortError');
    expect(result.observedProgressBeforeCancel).toBeGreaterThan(0);
    expect(result.searchMs).toBeLessThanOrEqual(120000);
    expect(result.coverage).toMatchObject({ complete: true, loadedShards: manifest.shards.length,
        totalShards: manifest.shards.length, corpus: manifest.corpus, sourceRevision: manifest.sourceRevision });
    expect(result.sources.length).toBeGreaterThan(0);
    for (const source of result.sources) {
        expect(source.sourcePath).toMatch(/\.(md|qmd|tex|txt|html|ipynb)$/);
        expect(source.sourceHash).toMatch(/^[a-f0-9]{64}$/);
        expect(Number.isSafeInteger(source.startLine)).toBe(true);
        expect(Number.isSafeInteger(source.endLine)).toBe(true);
        expect(source.startLine).toBeGreaterThan(0);
        expect(source.endLine).toBeGreaterThanOrEqual(source.startLine);
        expect(source.sourceRevision).toBe(manifest.sourceRevision);
    }

    // Fetch the actual immutable assets independently of the worker's caches.
    // This resolves every displayed source and the claim's ledger companion,
    // and verifies asset bytes before checking their source-hash/line metadata.
    const evidence = await page.evaluate(async ({ manifestUrl, manifest, urls, sources }) => {
        const base = new URL(manifestUrl, location.href);
        async function verifiedJson(url, expectedHash) {
            const response = await fetch(url);
            if (!response.ok) throw new Error(`Source asset fetch failed (${response.status}): ${url}`);
            const bytes = await response.arrayBuffer();
            const hash = [...new Uint8Array(await crypto.subtle.digest('SHA-256', bytes))].map(value => value.toString(16).padStart(2, '0')).join('');
            if (hash !== expectedHash) throw new Error(`Source asset hash mismatch: ${url}`);
            return JSON.parse(new TextDecoder().decode(bytes));
        }
        const claims = await verifiedJson(new URL(manifest.claimsPath, base).href, manifest.claimsSha256);
        const pointer = claims['FTD-1023'];
        if (!pointer) throw new Error('FTD-1023 authoritative claim pointer is missing');
        const companionUrl = new URL(pointer.chunksPath, base).href;
        const allUrls = [...new Set([...urls, companionUrl])];
        const chunks = new Map();
        for (const url of allUrls) {
            const file = manifest.files.find(entry => new URL(entry.path, base).href === url);
            if (!file) throw new Error('Fetched source asset is absent from the manifest');
            const chunk = await verifiedJson(url, file.sha256);
            for (const row of Object.values(chunk)) chunks.set(row.id, row);
        }
        const resolved = sources.map(source => {
            const stored = chunks.get(source.id);
            if (!stored) throw new Error(`Returned source ${source.id} did not resolve in fetched assets`);
            return { sourcePath: stored.sourcePath, sourceHash: stored.sourceHash, startLine: stored.startLine,
                endLine: stored.endLine, textMatches: source.text === stored.text };
        });
        const ledger = chunks.get(pointer.chunkId);
        return { resolved, ledger, fetchedSourceAssets: allUrls.length };
    }, { manifestUrl: status.knowledgeManifest, manifest, urls: [...fetchedChunks], sources: result.sources });
    expect(evidence.resolved).toEqual(result.sources.map(source => ({ sourcePath: source.sourcePath,
        sourceHash: source.sourceHash, startLine: source.startLine, endLine: source.endLine, textMatches: true })));
    const companion = result.sources.flatMap(source => source.authoritativeCompanions)
        .find(source => source.claimId === 'FTD-1023' && source.available);
    expect(companion).toBeTruthy();
    expect(companion.sourcePath).toBe('docs/theory/07_assessment/core_ledgers/LEDGER.md');
    expect(companion.authority).toBe(3);
    expect(companion.sourceHash).toBe(evidence.ledger.sourceHash);
    expect(companion.startLine).toBe(evidence.ledger.startLine);
    expect(companion.endLine).toBe(evidence.ledger.endLine);
    expect(evidence.ledger.text).toContain(companion.text);
    const citations = await page.evaluate(async ({ sources, companion, manifest, manifestUrl }) => {
        const base = new URL(manifestUrl, location.href), pages = new Map();
        const resolved = [];
        for (const source of [...sources, companion]) {
            if (!source.sourceUrl) throw new Error(`No resolvable citation URL for ${source.sourcePath}`);
            const url = new URL(source.sourceUrl, location.href);
            if (url.hash !== `#L${source.startLine}`) throw new Error('Citation does not identify the returned start line');
            url.hash = '';
            if (!pages.has(url.href)) {
                const entry = manifest.files.find(file => new URL(file.path, base).href === url.href);
                if (!entry) throw new Error('Citation page is absent from the local asset manifest');
                const response = await fetch(url);
                if (!response.ok) throw new Error(`Citation fetch failed (${response.status})`);
                const bytes = await response.arrayBuffer();
                const hash = [...new Uint8Array(await crypto.subtle.digest('SHA-256', bytes))].map(value => value.toString(16).padStart(2, '0')).join('');
                if (hash !== entry.sha256) throw new Error('Citation page integrity mismatch');
                pages.set(url.href, new DOMParser().parseFromString(new TextDecoder().decode(bytes), 'text/html'));
            }
            const document = pages.get(url.href);
            resolved.push({ sourcePath: source.sourcePath,
                startAnchor: !!document.getElementById(`L${source.startLine}`),
                endAnchor: !!document.getElementById(`L${source.endLine}`),
                pathShown: document.body.textContent.includes(source.sourcePath),
                hashShown: document.body.textContent.includes(source.sourceHash) });
        }
        return resolved;
    }, { sources: result.sources, companion, manifest, manifestUrl: status.knowledgeManifest });
    for (const citation of citations) expect(citation).toMatchObject({ startAnchor: true, endAnchor: true, pathShown: true, hashShown: true });
    const evidencePath = testInfo.outputPath('complete-corpus-search.json');
    await writeFile(evidencePath, JSON.stringify({
        query: 'FTD-1023', searchMs: result.searchMs, coverage: result.coverage,
        cancelledAfterShards: result.observedProgressBeforeCancel, fetchedSourceAssets: evidence.fetchedSourceAssets,
        sources: evidence.resolved, citations, authoritativeCompanion: { sourcePath: companion.sourcePath,
            sourceHash: companion.sourceHash, startLine: companion.startLine, endLine: companion.endLine },
    }, null, 2));
    await testInfo.attach('complete-corpus-search.json', { path: evidencePath, contentType: 'application/json' });
});
