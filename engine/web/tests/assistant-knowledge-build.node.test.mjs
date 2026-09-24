import { test } from 'node:test';
import assert from 'node:assert/strict';
import { mkdtemp, writeFile, readFile, rm } from 'node:fs/promises';
import { join, resolve, sep } from 'node:path';
import { tmpdir } from 'node:os';
import { spawnSync } from 'node:child_process';

test('claim pointers select a defined current Ledger entry over constitution, history and cross-references', async () => {
    const temporary = await mkdtemp(join(tmpdir(), 'ftd-knowledge-test-'));
    try {
        const common = { title: 'Source', heading: 'FTD-0001: Claim', text: '[OPEN] FTD-0001', claimIds: ['FTD-0001'], historical: false, statusStatements: ['**tag:** OPEN'] };
        const rows = [
            { ...common, id: 'constitution', sourcePath: 'constitution.md', authority: 2 },
            { ...common, id: 'archived-ledger', sourcePath: 'old-ledger.md', authority: 3, historical: true },
            { ...common, id: 'cross-reference', sourcePath: 'ledger.md', heading: 'Other section', authority: 3 },
            { ...common, id: 'current-ledger', sourcePath: 'ledger.md', authority: 3 },
        ];
        await writeFile(join(temporary, 'rows.jsonl'), rows.map(row => JSON.stringify(row)).join('\n'));
        await writeFile(join(temporary, 'metadata.json'), JSON.stringify({ corpus: 'public', sourceRevision: 'test', sourceFiles: 4, chunks: 4 }));
        const result = spawnSync(process.execPath, ['scripts/assistant/build_knowledge.mjs', join(temporary, 'rows.jsonl'), join(temporary, 'metadata.json'), join(temporary, 'output')], { cwd: new URL('../../../', import.meta.url), encoding: 'utf8' });
        assert.equal(result.status, 0, result.stderr);
        const manifest = JSON.parse(await readFile(join(temporary, 'output/manifest.json'), 'utf8'));
        const claims = JSON.parse(await readFile(join(temporary, 'output', manifest.claimsPath), 'utf8'));
        assert.equal(claims['FTD-0001'].chunkId, 'current-ledger');
        assert.equal(claims['FTD-0001'].match, 'heading');
    } finally {
        assert.ok(resolve(temporary).startsWith(resolve(tmpdir()) + sep));
        await rm(temporary, { recursive: true, force: true });
    }
});
