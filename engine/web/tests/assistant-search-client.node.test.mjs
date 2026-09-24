import { test } from 'node:test';
import assert from 'node:assert/strict';
import { KnowledgeSearch } from '../js/assistant/search-client.js';

globalThis.document = { baseURI: 'https://site.example/repository/' };
class FakeWorker {
    handlers = new Map(); sent = []; terminated = false;
    addEventListener(type, handler) { this.handlers.set(type, handler); }
    postMessage(message) {
        this.sent.push(message);
        if (message.type === 'init') queueMicrotask(() => this.reply(message.id, true));
    }
    reply(id, result) { this.handlers.get('message')({ data: { id, result } }); }
    terminate() { this.terminated = true; }
}
test('abort clears pending work and permits the next search without disposing its worker', async () => {
    const worker = new FakeWorker();
    const client = new KnowledgeSearch({ manifestUrl: './manifest.json', workerFactory: () => worker });
    await client.ready;
    const controller = new AbortController();
    const running = client.search('first', { signal: controller.signal });
    await new Promise(resolve => setImmediate(resolve));
    const id = worker.sent.at(-1).id;
    controller.abort();
    await assert.rejects(running, error => error.name === 'AbortError');
    assert.deepEqual(worker.sent.at(-1), { type: 'cancel', id });
    assert.equal(client.pending.size, 0);
    assert.equal(worker.terminated, false);
    const second = client.search('second');
    await new Promise(resolve => setImmediate(resolve));
    worker.reply(worker.sent.at(-1).id, [{ text: 'second result' }]);
    assert.equal((await second)[0].text, 'second result');
    assert.equal(client.pending.size, 0);
    client.dispose();
});
