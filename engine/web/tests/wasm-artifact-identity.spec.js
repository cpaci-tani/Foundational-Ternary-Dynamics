// @ts-check
import { test, expect } from '@playwright/test';
import crypto from 'node:crypto';
import fs from 'node:fs';
import path from 'node:path';
import { fileURLToPath } from 'node:url';
import { gotoAndReady, selectScale0Scenario } from './_helpers.js';

const webRoot = path.resolve(path.dirname(fileURLToPath(import.meta.url)), '..');
const wasmRoot = path.join(webRoot, 'wasm');

function sha256(buffer) {
    return crypto.createHash('sha256').update(buffer).digest('hex');
}

function verifyIdentityShape(identity, variantId) {
    expect(identity).not.toBeNull();
    expect(identity.schemaVersion).toBe(1);
    expect(identity.bundleSha256).toMatch(/^[0-9a-f]{64}$/);
    expect(identity.source.commit).toMatch(/^[0-9a-f]{40}$/);
    expect(identity.source.dirty).toBe(false);
    expect(identity.variant.id).toBe(variantId);
    expect(identity.variant.artifacts).toHaveLength(2);
    for (const artifact of identity.variant.artifacts) {
        expect(artifact.sha256).toMatch(/^[0-9a-f]{64}$/);
        expect(Number.isSafeInteger(artifact.sizeBytes)).toBe(true);
        expect(artifact.sizeBytes).toBeGreaterThan(1024);
    }
}

test('deterministic manifest exactly identifies all six checked-in artifacts', () => {
    const manifestPath = path.join(wasmRoot, 'build_info.json');
    const manifest = JSON.parse(fs.readFileSync(manifestPath, 'utf8'));
    expect(manifest.schemaVersion).toBe(1);
    expect(manifest.bundleSha256).toMatch(/^[0-9a-f]{64}$/);
    expect(manifest.source.commit).toMatch(/^[0-9a-f]{40}$/);
    expect(manifest.source.dirty).toBe(false);
    expect(manifest.variants.map((variant) => variant.id))
        .toEqual(['wasm32', 'wasm64', 'wasm32-threads']);

    const expectedVariants = {
        wasm32: { factory: 'createFTDModule', pointerBits: 32, memory64: false, threads: false, sharedMemory: false },
        wasm64: { factory: 'createFTDModule64', pointerBits: 64, memory64: true, threads: false, sharedMemory: false },
        'wasm32-threads': { factory: 'createFTDModuleMT', pointerBits: 32, memory64: false, threads: true, sharedMemory: true },
    };
    const seen = new Set();
    let canonical = 'ftd-wasm-bundle-v1\n';
    for (const variant of manifest.variants) {
        expect({ factory: variant.factory, ...variant.abi }).toEqual(expectedVariants[variant.id]);
        expect(variant.artifacts).toHaveLength(2);
        for (const artifact of variant.artifacts) {
            expect(seen.has(artifact.file), `${artifact.file} is duplicated`).toBe(false);
            seen.add(artifact.file);
            const bytes = fs.readFileSync(path.join(wasmRoot, artifact.file));
            expect(bytes.byteLength).toBe(artifact.sizeBytes);
            expect(sha256(bytes)).toBe(artifact.sha256);
            canonical += `${artifact.file}\0${artifact.sizeBytes}\0${artifact.sha256}\n`;
        }
    }
    expect(seen.size).toBe(6);
    expect(sha256(Buffer.from(canonical, 'utf8'))).toBe(manifest.bundleSha256);
});

test('pre-instantiation verification rejects a one-byte mixed generation', async ({ page }) => {
    await page.goto('/');
    const result = await page.evaluate(async () => {
        const { loadVerifiedWasmVariant } = await import('/js/bridge/wasm-artifact-identity.js');
        const realFetch = globalThis.fetch.bind(globalThis);
        const corruptingFetch = async (input, init) => {
            const response = await realFetch(input, init);
            const url = String(input?.url || input);
            if (!url.includes('ftd_core_mt.wasm?')) return response;
            const bytes = new Uint8Array(await response.arrayBuffer());
            bytes[Math.floor(bytes.length / 2)] ^= 0x01;
            return new Response(bytes, {
                status: response.status,
                headers: { 'Content-Type': 'application/wasm' },
            });
        };
        try {
            await loadVerifiedWasmVariant('wasm32-threads', corruptingFetch);
            return { accepted: true, error: null };
        } catch (error) {
            return { accepted: false, error: String(error?.message || error) };
        }
    });
    expect(result.accepted).toBe(false);
    expect(result.error).toMatch(/artifact hash mismatch: ftd_core_mt\.wasm/);
});

test('main-thread WASM exposes and verifies the actually selected artifact pair', async ({ page }) => {
    test.setTimeout(120_000);
    await page.addInitScript(() => { window.__ftdWasmWorker = false; });
    await gotoAndReady(page, { path: '/?engine=wasm', timeout: 90_000 });
    await selectScale0Scenario(page, 'empty', { settleMs: 0 });
    const result = await page.evaluate(async () => {
        const bridge = window.__ftdCtx?.bridge;
        const identity = await bridge?.artifactIdentityReady;
        const hashes = {};
        for (const artifact of identity?.variant?.artifacts || []) {
            const bytes = await (await fetch(`/wasm/${artifact.file}`, { cache: 'no-store' })).arrayBuffer();
            const digest = await crypto.subtle.digest('SHA-256', bytes);
            hashes[artifact.file] = [...new Uint8Array(digest)]
                .map((value) => value.toString(16).padStart(2, '0')).join('');
        }
        return {
            state: bridge?.artifactIdentityState,
            selectedVariant: bridge?.isWasm64 ? 'wasm64' : 'wasm32',
            identity,
            hashes,
        };
    });
    expect(result.state).toBe('ready');
    verifyIdentityShape(result.identity, result.selectedVariant);
    for (const artifact of result.identity.variant.artifacts) {
        expect(result.hashes[artifact.file]).toBe(artifact.sha256);
    }
});

test('worker WASM exposes and verifies the actually loaded threaded artifact pair', async ({ page }) => {
    test.setTimeout(120_000);
    await gotoAndReady(page, { path: '/?engine=wasm', timeout: 90_000 });
    await selectScale0Scenario(page, 'empty', { settleMs: 0 });
    const result = await page.evaluate(async () => {
        const { getScale0State } = await import('/js/scales/scale0/state/store.js');
        const bridge = getScale0State().fluxMock;
        const identity = await bridge?.artifactIdentityReady;
        const hashes = {};
        for (const artifact of identity?.variant?.artifacts || []) {
            const bytes = await (await fetch(`/wasm/${artifact.file}`, { cache: 'no-store' })).arrayBuffer();
            const digest = await crypto.subtle.digest('SHA-256', bytes);
            hashes[artifact.file] = [...new Uint8Array(digest)]
                .map((value) => value.toString(16).padStart(2, '0')).join('');
        }
        return {
            isWorker: bridge?.isWorker === true,
            state: bridge?.artifactIdentityState,
            identity,
            hashes,
        };
    });
    expect(result.isWorker).toBe(true);
    expect(result.state).toBe('ready');
    verifyIdentityShape(result.identity, 'wasm32-threads');
    for (const artifact of result.identity.variant.artifacts) {
        expect(result.hashes[artifact.file]).toBe(artifact.sha256);
    }
});
