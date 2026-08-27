import assert from 'node:assert/strict';
import { spawnSync } from 'node:child_process';
import test from 'node:test';
import { fileURLToPath } from 'node:url';

const verifier = fileURLToPath(
    new URL('./verify_web_consistency.js', import.meta.url),
);

test('web consistency verifier loads current ES modules and succeeds', () => {
    const result = spawnSync(process.execPath, [verifier], {
        cwd: fileURLToPath(new URL('.', import.meta.url)),
        encoding: 'utf8',
    });

    assert.equal(
        result.status,
        0,
        `verifier failed:\n${result.stdout}${result.stderr}`,
    );
    assert.match(result.stdout, /Consistency verification succeeded!/);
});
