#!/usr/bin/env node

import assert from 'node:assert/strict';
import { spawn } from 'node:child_process';
import net from 'node:net';

const [, , serverPath, smokePath] = process.argv;
assert(serverPath, 'ws_server executable path is required');
assert(smokePath, 'native-ws-smoke.mjs path is required');

async function reservePort() {
    const probe = net.createServer();
    await new Promise((resolve, reject) => {
        probe.once('error', reject);
        probe.listen(0, '127.0.0.1', resolve);
    });
    const address = probe.address();
    assert(address && typeof address === 'object');
    const { port } = address;
    await new Promise((resolve, reject) => probe.close((error) => {
        if (error) reject(error);
        else resolve();
    }));
    return port;
}

function waitForListening(child, timeoutMs = 60000) {
    return new Promise((resolve, reject) => {
        let output = '';
        const timer = setTimeout(() => {
            reject(new Error(`ws_server startup timed out\n${output}`));
        }, timeoutMs);
        const consume = (chunk) => {
            const text = chunk.toString();
            output += text;
            process.stderr.write(text);
            if (output.includes('[ws_server] Listening on ')) {
                clearTimeout(timer);
                resolve();
            }
        };
        child.stdout.on('data', consume);
        child.stderr.on('data', consume);
        child.once('error', (error) => {
            clearTimeout(timer);
            reject(error);
        });
        child.once('exit', (code, signal) => {
            clearTimeout(timer);
            reject(new Error(
                `ws_server exited before listening (code=${code}, signal=${signal})\n${output}`));
        });
    });
}

function waitForExit(child, timeoutMs) {
    if (child.exitCode !== null || child.signalCode !== null)
        return Promise.resolve(true);
    return new Promise((resolve) => {
        const timer = setTimeout(() => resolve(false), timeoutMs);
        child.once('exit', () => {
            clearTimeout(timer);
            resolve(true);
        });
    });
}

async function terminate(child) {
    if (await waitForExit(child, 1000)) return;
    child.kill('SIGTERM');
    if (await waitForExit(child, 5000)) return;
    child.kill('SIGKILL');
    await waitForExit(child, 5000);
}

const port = await reservePort();
const server = spawn(serverPath, ['16', String(port), '--bind', '127.0.0.1', '--once'], {
    windowsHide: true,
    stdio: ['ignore', 'pipe', 'pipe'],
});

try {
    await waitForListening(server);
    const smoke = spawn(process.execPath, [smokePath, `ws://127.0.0.1:${port}`], {
        windowsHide: true,
        stdio: 'inherit',
    });
    const smokeResult = await new Promise((resolve, reject) => {
        smoke.once('error', reject);
        smoke.once('exit', (code, signal) => resolve({ code, signal }));
    });
    assert.equal(smokeResult.signal, null, 'native WebSocket smoke was terminated by a signal');
    assert.equal(smokeResult.code, 0, 'native WebSocket smoke failed');
    assert.equal(await waitForExit(server, 10000), true,
        'single-client ws_server did not exit after the smoke client disconnected');
    assert.equal(server.exitCode, 0, 'ws_server returned a failure status');
} finally {
    await terminate(server);
}
