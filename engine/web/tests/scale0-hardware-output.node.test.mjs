import assert from 'node:assert/strict';
import { randomBytes } from 'node:crypto';
import { existsSync, readFileSync, realpathSync, rmSync } from 'node:fs';
import { join, relative, isAbsolute } from 'node:path';
import test from 'node:test';
import { HARDWARE_SCRATCH_ROOT, hardwareRunId, reserveHardwareReport } from './scale0-hardware-output.js';

test('run IDs are safe path components', () => {
    for (const runId of ['', '.', '..', '../evidence', 'a/b', 'a\\b', 'C:escape', 'a b', 'NUL', 'x'.repeat(65)]) {
        assert.throws(() => hardwareRunId(runId));
    }
    assert.equal(hardwareRunId('20260924T120000Z-a1_b2'), '20260924T120000Z-a1_b2');
    assert.match(hardwareRunId(), /^[A-Za-z0-9][A-Za-z0-9_-]{0,63}$/);
});

test('reports remain in ignored scratch and an existing destination is never overwritten', (t) => {
    const runId = `test-${randomBytes(8).toString('hex')}`;
    const directory = join(HARDWARE_SCRATCH_ROOT, runId);
    t.after(() => {
        if (!existsSync(directory)) return;
        const relativeDirectory = relative(realpathSync(HARDWARE_SCRATCH_ROOT), realpathSync(directory));
        assert.equal(relativeDirectory, runId);
        assert.equal(isAbsolute(relativeDirectory), false);
        rmSync(directory, { recursive: true });
    });
    const filename = 'scale0-hardware-overlays-L33-2026-09-07.json';
    const report = { rows: [] };
    const { output, save } = reserveHardwareReport({ runId, filename, report });
    assert.equal(output, join(directory, filename));
    assert.deepEqual(JSON.parse(readFileSync(output, 'utf8')).provenance, {
        runId,
        scratchPath: `engine/build/scale0-hardware/${runId}/${filename}`,
        evidenceStatus: 'scratch-unpromoted',
        startedAt: report.provenance.startedAt,
    });
    report.rows.push({ id: 'sample' });
    save();
    const before = readFileSync(output, 'utf8');
    assert.match(before, /sample/);
    assert.throws(() => reserveHardwareReport({ runId, filename, report: {} }), /Refusing to overwrite/);
    assert.equal(readFileSync(output, 'utf8'), before);
    assert.throws(() => reserveHardwareReport({ runId, filename: '../scale0-hardware-escape.json', report: {} }), /Unsafe/);
    assert.throws(() => reserveHardwareReport({ runId: '../evidence', filename, report: {} }), /FTD_AUDIT_RUN_ID/);
});
