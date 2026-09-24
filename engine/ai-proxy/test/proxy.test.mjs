import { test } from 'node:test';
import assert from 'node:assert/strict';
import { createHandler, validateInput, normalizeAnswer } from '../src/index.js';

const input = { intent: 'Pause lattice', observation: { workspace: 'lattice' }, plan: { action: 'pause' } };
const answer = { model: 'jev-test', answers: { decision: { type: 'choice', choice: 'execute', confidence: .91 } }, usage: { input_tokens: 12 } };
let serial = 0;
function request(body = input, extra = {}) {
    return new Request('https://proxy.example/api/ai/jev', { method: 'POST', headers: {
        Origin: 'https://cpaci-tani.github.io', 'Content-Type': 'application/json', Authorization: 'Bearer TEST_ONLY_KEY',
        'CF-Connecting-IP': 'test-' + serial++, ...extra,
    }, body: JSON.stringify(body) });
}
test('fixed upstream/questions and reduced response; BYOK is forwarded only', async () => {
    let calls = 0;
    const handler = createHandler(async (url, options) => {
        calls++;
        assert.equal(url, 'https://api.typesafe.ai/v1/systemone');
        assert.equal(options.headers.Authorization, 'Bearer TEST_ONLY_KEY');
        const sent = JSON.parse(options.body);
        assert.deepEqual(JSON.parse(sent.state), input);
        assert.equal(sent.questions.decision.type, 'choice');
        assert.equal(sent.model, 'jev-latest');
        return Response.json(answer);
    });
    const response = await handler(request());
    assert.equal(response.status, 200);
    assert.equal(response.headers.get('Access-Control-Allow-Origin'), 'https://cpaci-tani.github.io');
    assert.deepEqual(await response.json(), { decision: 'execute', confidence: .91, model: 'jev-test', usage: { input_tokens: 12 } });
    assert.equal(calls, 1);
});
test('rejects origin, key, injected question schema and oversized body before upstream', async () => {
    const handler = createHandler(() => { throw new Error('must not call'); });
    assert.equal((await handler(request(input, { Origin: 'https://other.example' }))).status, 403);
    assert.equal((await handler(request(input, { Authorization: '' }))).status, 401);
    assert.equal((await handler(request({ ...input, questions: {} }))).status, 400);
    assert.equal((await handler(request({ ...input, intent: 'x'.repeat(70000) }))).status, 400);
});
test('fails closed on invalid answers and strips upstream credential-bearing errors', async () => {
    assert.throws(() => normalizeAnswer({ ...answer, answers: { decision: { type: 'choice', choice: 'execute', confidence: 2 } } }));
    assert.throws(() => validateInput({ ...input, plan: [] }));
    const failed = await createHandler(async () => new Response('TEST_ONLY_KEY provider body', { status: 401 }))(request());
    assert.equal(failed.status, 401);
    assert.ok(!(await failed.text()).includes('TEST_ONLY_KEY'));
    const huge = await createHandler(async () => new Response('x'.repeat(70000)))(request());
    assert.equal(huge.status, 502);
});
test('same client has bounded rate and concurrent calls without retaining its key', async () => {
    const handler = createHandler(async () => Response.json(answer));
    for (let i = 0; i < 20; i++) assert.equal((await handler(request(input, { 'CF-Connecting-IP': 'rate-test' }))).status, 200);
    assert.equal((await handler(request(input, { 'CF-Connecting-IP': 'rate-test' }))).status, 429);
    const releases = [];
    const blocked = createHandler(() => new Promise(resolve => releases.push(() => resolve(Response.json(answer)))));
    const first = blocked(request(input, { 'CF-Connecting-IP': 'concurrency-test' }));
    const second = blocked(request(input, { 'CF-Connecting-IP': 'concurrency-test' }));
    await new Promise(resolve => setImmediate(resolve));
    assert.equal((await blocked(request(input, { 'CF-Connecting-IP': 'concurrency-test' }))).status, 429);
    releases.forEach(release => release());
    assert.equal((await first).status, 200); assert.equal((await second).status, 200);
});
