import test from 'node:test';
import assert from 'node:assert/strict';
import { createServer } from 'node:http';
import { once } from 'node:events';
import { fileURLToPath } from 'node:url';
import { Client } from '@modelcontextprotocol/sdk/client/index.js';
import { StdioClientTransport } from '@modelcontextprotocol/sdk/client/stdio.js';
import { readConfig, RelayClient } from '../relay.js';

const serverPath = fileURLToPath(new URL('../server.mjs', import.meta.url));
const token = 'test-token-only-abcdef0123456789';
const sessionId = 'test-session-abcdef0123456789';
const expected = { workspace: 'lattice', ownerId: 'owner-1', preparationVersion: 'preparation-1' };
const json = (response, body, status = 200) => { response.writeHead(status, { 'content-type': 'application/json' }); response.end(JSON.stringify(body)); };

async function fixture(t, handler = ({ body }, response) => json(response, { result: { method: body.method, args: body.args } })) {
    const requests = [];
    const http = createServer(async (request, response) => {
        let text = ''; for await (const chunk of request) text += chunk;
        const item = { path: request.url, authorization: request.headers.authorization, body: text ? JSON.parse(text) : null };
        requests.push(item); handler(item, response);
    });
    http.listen(0, '127.0.0.1'); await once(http, 'listening');
    const baseUrl = `http://127.0.0.1:${http.address().port}`;
    const transport = new StdioClientTransport({ command: process.execPath, args: [serverPath], stderr: 'pipe',
        env: { ...process.env, FTD_MCP_URL: baseUrl, FTD_MCP_SESSION: sessionId, FTD_MCP_TOKEN: token } });
    let stderr = ''; transport.stderr.on('data', chunk => { stderr += chunk.toString(); });
    const client = new Client({ name: 'ftd-integration-test', version: '1.0.0' });
    t.after(async () => { await client.close(); http.closeAllConnections(); await new Promise(resolve => http.close(resolve)); });
    await client.connect(transport);
    return { client, requests, baseUrl, stderr: () => stderr };
}

test('real SDK stdio discovers typed tools, scientific workflow resource and prompt', async t => {
    const f = await fixture(t);
    const { tools } = await f.client.listTools();
    assert.equal(tools.length, 9);
    const execute = tools.find(row => row.name === 'ftd_execute_plan');
    assert.equal(execute.inputSchema.properties.actions.maxItems, 8);
    assert.equal(execute.annotations.readOnlyHint, false); assert.equal(execute.annotations.destructiveHint, true);
    assert.equal(tools.find(row => row.name === 'ftd_observe').annotations.readOnlyHint, true);
    assert.equal(tools.find(row => row.name === 'ftd_measure_flux_sectors').annotations.readOnlyHint, true);
    assert.equal(tools.find(row => row.name === 'ftd_stop').annotations.destructiveHint, false);
    assert.equal(tools.find(row => row.name === 'ftd_run_scenario').inputSchema.properties.draft.properties.settings.maxItems, 12);
    assert.equal(tools.find(row => row.name === 'ftd_run_scenario').inputSchema.properties.ticks.maximum, 100000);
    assert.equal(tools.find(row => row.name === 'ftd_run_scenario').inputSchema.properties.draft.properties.ticks.maximum, 100000);
    const resources = await f.client.listResources(); assert.equal(resources.resources[0].uri, 'ftd://workflow');
    const resource = await f.client.readResource({ uri: 'ftd://workflow' }); assert.match(resource.contents[0].text, /JEV approval/);
    assert.match(resource.contents[0].text, /one JEV approval for the complete fixed protocol/);
    assert.match(resource.contents[0].text, /100000 ticks, 200 measured intervals/);
    const prompts = await f.client.listPrompts(); assert.equal(prompts.prompts[0].name, 'ftd_live_experiment');
    const prompt = await f.client.getPrompt({ name: 'ftd_live_experiment', arguments: { goal: 'Compare ten completed ticks.' } });
    assert.match(prompt.messages[0].content.text, /Compare ten completed ticks/);
    assert.equal(f.requests.length, 0); assert.equal(f.stderr(), '');
});

test('all nine tools forward authenticated fixed-route data and preserve typed replies', async t => {
    const f = await fixture(t);
    const cases = [
        ['ftd_observe', 'observe', {}], ['ftd_list_scenarios', 'list_scenarios', { query: 'pulse', offset: 1, limit: 50 }],
        ['ftd_describe_scenario', 'describe_scenario', { scenarioId: 'flux-pulse' }],
        ['ftd_measure_flux_sectors', 'measure_flux_sectors', { expected, expectedTick: '20000' }],
        ['ftd_execute_plan', 'execute_plan', { intent: 'Pause.', expected, actions: [{ type: 'lattice.pause', args: {} }] }],
        ['ftd_run_scenario', 'run_scenario', { goal: 'Measure count.', scenarioId: 'flux-pulse', ticks: 10, sampleEvery: 5, expected,
            draft: { name: 'Count experiment', goal: 'Measure count.', settings: [{ key: 'amplitude', value: .2 }], ticks: 10, sampleEvery: 5, measurements: ['manifested'] } }],
        ['ftd_run_status', 'run_status', { runId: 'run-1' }], ['ftd_search_docs', 'search_docs', { query: 'epistemic labels', limit: 6 }],
        ['ftd_stop', 'stop', {}],
    ];
    for (const [name, method, args] of cases) {
        const result = await f.client.callTool({ name, arguments: args });
        assert.deepEqual(result.structuredContent, { result: { method, args } });
        assert.deepEqual(JSON.parse(result.content[0].text), result.structuredContent);
    }
    assert.equal(f.requests.length, 9);
    for (const request of f.requests) {
        assert.equal(request.path, '/api/mcp/call'); assert.equal(request.authorization, `Bearer ${token}`);
        assert.equal(request.body.sessionId, sessionId); assert.match(request.body.id, /^[0-9a-f-]{36}$/);
        assert.ok(!JSON.stringify(request.body).includes(token));
    }
    assert.equal(new Set(f.requests.map(row => row.body.id)).size, 9); assert.equal(f.stderr(), '');
});

test('invalid actions, unknown properties and conflicting drafts are rejected before relay', async t => {
    const f = await fixture(t);
    const requests = [
        ['ftd_observe', { code: 'eval' }], ['ftd_list_scenarios', { limit: 51 }], ['ftd_describe_scenario', { scenarioId: '' }],
        ['ftd_measure_flux_sectors', { expected, expectedTick: '02' }],
        ['ftd_measure_flux_sectors', { expected, expectedTick: '2e4' }],
        ['ftd_execute_plan', { intent: 'Run.', expected, actions: Array.from({ length: 9 }, () => ({ type: 'lattice.step', args: { count: 1 } })) }],
        ['ftd_execute_plan', { intent: 'Run.', expected: { ...expected, epoch: 0 }, actions: [{ type: 'lattice.step', args: {} }] }],
        ['ftd_run_scenario', { goal: 'Measure.', scenarioId: 'empty', ticks: 10, sampleEvery: 11, expected }],
        ['ftd_run_scenario', { goal: 'Measure.', scenarioId: 'empty', ticks: 10, sampleEvery: 5, expected,
            draft: { name: 'Conflict', goal: 'Other goal.', settings: [], ticks: 10, sampleEvery: 5, measurements: ['manifested'] } }],
    ];
    for (const [name, args] of requests) {
        const result = await f.client.callTool({ name, arguments: args });
        assert.equal(result.isError, true); assert.equal(result.structuredContent.error.code, 'INVALID_ARGUMENTS');
    }
    assert.equal(f.requests.length, 0);
});

test('long fixed protocols accept ten thousand through one hundred thousand ticks without expensive physics', async t => {
    const f = await fixture(t);
    for (const [ticks, sampleEvery] of [[10000, 250], [50000, 250], [100000, 500]]) {
        const draft = { name: 'Long count run', goal: 'Measure counts.', settings: [], ticks, sampleEvery, measurements: ['manifested'] };
        const result = await f.client.callTool({ name: 'ftd_run_scenario', arguments: { goal: draft.goal, scenarioId: 'empty', ticks, sampleEvery, expected, draft } });
        assert.equal(result.isError, undefined); assert.equal(result.structuredContent.result.args.ticks, ticks);
    }
    for (const [ticks, sampleEvery] of [[100001, 501], [100000, 499], [50000, 249], [10000, 49]]) {
        const result = await f.client.callTool({ name: 'ftd_run_scenario', arguments: { goal: 'Measure counts.', scenarioId: 'empty', ticks, sampleEvery, expected } });
        assert.equal(result.isError, true); assert.equal(result.structuredContent.error.code, 'INVALID_ARGUMENTS');
    }
    assert.equal(f.requests.length, 3);
});

test('relay errors preserve uncertain outcomes but redact credential echoes', async t => {
    const f = await fixture(t, (_, response) => json(response, { error: { code: 'TIMEOUT', outcome: 'unknown',
        message: `Do not retry ${token} ${sessionId}` } }, 504));
    const result = await f.client.callTool({ name: 'ftd_observe', arguments: {} });
    assert.equal(result.isError, true); assert.equal(result.structuredContent.error.outcome, 'unknown');
    assert.match(result.content[0].text, /redacted/); assert.ok(!JSON.stringify(result).includes(token));
    assert.ok(!JSON.stringify(result).includes(sessionId)); assert.equal(f.stderr(), '');
});

test('SDK request cancellation forwards the exact dispatched UUID to the authenticated cancel route', async t => {
    let started, cancelled;
    const startedPromise = new Promise(resolve => { started = resolve; });
    const cancelledPromise = new Promise(resolve => { cancelled = resolve; });
    const f = await fixture(t, (item, response) => {
        if (item.path === '/api/mcp/cancel') { cancelled(item); json(response, { result: { cancelled: true } }); }
        else started(item);
    });
    const controller = new AbortController();
    const call = f.client.callTool({ name: 'ftd_execute_plan', arguments: { intent: 'Pause.', expected,
        actions: [{ type: 'lattice.pause', args: {} }] } }, undefined, { signal: controller.signal });
    const rejected = assert.rejects(call);
    const dispatched = await startedPromise; controller.abort();
    const cancellation = await Promise.race([cancelledPromise, new Promise((_, reject) => {
        const timer = setTimeout(() => reject(new Error('Cancellation relay was not called.')), 4000); timer.unref();
    })]);
    await rejected;
    assert.equal(cancellation.body.id, dispatched.body.id); assert.equal(cancellation.body.sessionId, sessionId);
    assert.equal(cancellation.authorization, `Bearer ${token}`); assert.equal(f.requests.length, 2);
});

test('redirects fail closed without another HTTP request or retry', async t => {
    const f = await fixture(t, (_, response) => { response.writeHead(307, { location: '/should-not-be-followed' }); response.end(); });
    const result = await f.client.callTool({ name: 'ftd_observe', arguments: {} });
    assert.equal(result.structuredContent.error.code, 'RELAY_UNAVAILABLE');
    assert.equal(result.structuredContent.error.outcome, 'unknown'); assert.equal(f.requests.length, 1);
});

test('malformed and oversized relay replies are bounded and never exposed as successful results', async t => {
    let count = 0;
    const f = await fixture(t, (_, response) => {
        count++;
        if (count === 1) { response.writeHead(200, { 'content-type': 'application/json' }); response.end('{broken'); }
        else json(response, { result: 'x'.repeat(2 * 1024 * 1024) });
    });
    for (let i = 0; i < 2; i++) {
        const result = await f.client.callTool({ name: 'ftd_observe', arguments: {} });
        assert.equal(result.isError, true); assert.equal(result.structuredContent.error.code, 'RELAY_UNAVAILABLE');
        assert.equal(result.structuredContent.error.outcome, 'unknown');
        assert.ok(result.content[0].text.length < 1000);
    }
    assert.equal(f.requests.length, 2);
});

test('oversized action arguments stop before any HTTP dispatch', async t => {
    const f = await fixture(t);
    const result = await f.client.callTool({ name: 'ftd_execute_plan', arguments: { intent: 'Oversized data.', expected,
        actions: [{ type: 'lattice.seed.preview', args: { recipeJson: 'x'.repeat(130 * 1024) } }] } });
    assert.equal(result.isError, true); assert.equal(result.structuredContent.error.code, 'REQUEST_TOO_LARGE');
    assert.equal(f.requests.length, 0);
});

test('configuration permits only credential-free loopback HTTP bases', () => {
    const env = { FTD_MCP_SESSION: sessionId, FTD_MCP_TOKEN: token };
    assert.equal(readConfig(env).baseUrl, 'http://127.0.0.1:8080');
    for (const url of ['http://localhost:8080', 'http://[::1]:8080/']) assert.equal(typeof readConfig({ ...env, FTD_MCP_URL: url }).baseUrl, 'string');
    for (const url of ['https://localhost:8080', 'http://example.com', 'http://127.1:8080', 'http://127.0.0.1.evil:8080',
        'http://user:secret@localhost', 'http://localhost/api', 'http://localhost/?token=x', 'http://localhost#x',
        'http://localhost:0', 'http://localhost:65536', 'http://localhost/../']) {
        assert.throws(() => readConfig({ ...env, FTD_MCP_URL: url }), /FTD_MCP_URL/);
    }
    assert.throws(() => readConfig({ FTD_MCP_SESSION: sessionId }), /FTD_MCP_TOKEN/);
    assert.throws(() => readConfig({ FTD_MCP_TOKEN: token }), /FTD_MCP_SESSION/);
});

test('bounded relay deadline cancels dispatched work and reports unknown instead of retrying', async t => {
    let cancellations = 0;
    const f = await fixture(t, (item, response) => {
        if (item.path === '/api/mcp/cancel') { cancellations++; json(response, { result: true }); }
    });
    const relay = new RelayClient({ baseUrl: f.baseUrl, sessionId, token }, { timeoutMs: 30 });
    const result = await relay.call('observe', {}, new AbortController().signal);
    assert.equal(result.error.code, 'TIMEOUT'); assert.equal(result.error.outcome, 'unknown');
    assert.equal(cancellations, 1); assert.equal(f.requests.length, 2);
});
