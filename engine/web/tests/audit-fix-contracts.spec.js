// @ts-check
/**
 * Contract locks for the 2026-08 web/native boundary fixes.
 * These import the helper modules directly (no Chromium / WASM required).
 */
import { test, expect } from '@playwright/test';
import fs from 'node:fs';
import path from 'node:path';
import { fileURLToPath } from 'node:url';

import { visualSampleGrid } from '../js/lib/visual-sample-grid.js';
import { parseFtv2Frame, FTV2_MAGIC } from '../js/lib/ftv2.js';
import { wsOriginAllowed, parseNativeWsPort } from '../js/lib/origin-policy.js';

const webRoot = path.resolve(path.dirname(fileURLToPath(import.meta.url)), '..');

test('worker gravity sampler names the embind export, not the JS alias', () => {
    const src = fs.readFileSync(
        path.join(webRoot, 'js', 'bridge', 'wasm-bridge.worker.js'),
        'utf8',
    );
    expect(src).toMatch(/['"]gravity['"]\s*:\s*\[['"]getGravityFieldSampled['"]/);
    expect(src).not.toMatch(/['"]gravity['"]\s*:\s*\[['"]getGravityForceField['"]/);
});

test('worker command dispatch is allowlisted', () => {
    const src = fs.readFileSync(
        path.join(webRoot, 'js', 'bridge', 'wasm-bridge.worker.js'),
        'utf8',
    );
    expect(src).toMatch(/WORKER_COMMAND_ALLOWLIST/);
    expect(src).toMatch(/if\s*\(\s*!WORKER_COMMAND_ALLOWLIST\.has\(method\)/);
});

test('centre-anchored visual grid matches C++ for stride-3 L=33', () => {
    expect(visualSampleGrid(33, 3, false)).toEqual({ stride: 3, origin: 1, count: 11 });
});

test('FTV2 parser reads origin from 20-byte headers and infers it on 16-byte frames', () => {
    const axis = 3;
    const count = axis ** 3;
    const v20 = new ArrayBuffer(20 + count * 4);
    const h20 = new DataView(v20);
    h20.setUint32(0, FTV2_MAGIC, true);
    h20.setUint32(4, 33, true);
    h20.setUint32(8, 3, true);
    h20.setUint32(12, 1, true);
    h20.setUint32(16, axis, true);
    new Float32Array(v20, 20, count)[0] = 1.5;

    const v16 = new ArrayBuffer(16 + count * 4);
    const h16 = new DataView(v16);
    h16.setUint32(0, FTV2_MAGIC, true);
    h16.setUint32(4, 33, true);
    h16.setUint32(8, 3, true);
    h16.setUint32(12, axis, true);
    new Float32Array(v16, 16, count)[2] = 2.25;

    const modern = parseFtv2Frame(v20);
    const legacy = parseFtv2Frame(v16);
    expect(modern.origin).toBe(1);
    expect(modern.axisCount).toBe(3);
    expect(modern.data[0]).toBeCloseTo(1.5);
    expect(legacy.origin).toBe(1);
    expect(legacy.data[2]).toBeCloseTo(2.25);
});

test('Origin allowlist accepts loopback and rejects foreign sites', () => {
    expect(wsOriginAllowed('')).toBe(true);
    expect(wsOriginAllowed('null')).toBe(true);
    expect(wsOriginAllowed('http://localhost:8080')).toBe(true);
    expect(wsOriginAllowed('http://127.0.0.1:9100')).toBe(true);
    expect(wsOriginAllowed('http://[::1]:8080')).toBe(true);
    expect(wsOriginAllowed('https://evil.example')).toBe(false);
    expect(parseNativeWsPort('?wsPort=5555', 'http://127.0.0.1:8080')).toBe(5555);
    expect(parseNativeWsPort('?wsPort=5555', 'https://evil.example')).toBe(9100);
});

test('toolbar Reset/Step and keyboard handlers cover planetary/cosmic/meta', () => {
    const app = fs.readFileSync(path.join(webRoot, 'js', 'app.js'), 'utf8');
    const resetStart = app.indexOf("getElementById('btn-reset')");
    const resetEnd = app.indexOf("getElementById('ticks-per-frame')", resetStart);
    const reset = app.slice(resetStart, resetEnd);
    expect(reset).toMatch(/engineMode === 'planetary'/);
    expect(reset).toMatch(/engineMode === 'cosmic'/);
    expect(reset).toMatch(/engineMode === 'meta'/);

    const stepKb = app.slice(app.indexOf('stepScenario:'), app.indexOf('reloadScenario:'));
    expect(stepKb).toMatch(/planetary/);
    expect(stepKb).toMatch(/cosmic/);
    expect(stepKb).toMatch(/meta/);

    const reloadKb = app.slice(app.indexOf('reloadScenario:'), app.indexOf('Scale0Controller,'));
    expect(reloadKb).toMatch(/planetary/);
    expect(reloadKb).toMatch(/cosmic/);
    expect(reloadKb).toMatch(/meta/);
});
