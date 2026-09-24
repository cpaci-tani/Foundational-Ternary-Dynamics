import contract from '../contract.json' with { type: 'json' };

const DECISIONS = new Set(['execute', 'clarify', 'reject']);
const buckets = new Map();
const MAX_CLIENTS = 1024;

export function validateInput(value) {
    if (!value || typeof value !== 'object' || Array.isArray(value)
        || Object.keys(value).sort().join(',') !== 'intent,observation,plan'
        || typeof value.intent !== 'string' || !value.intent.trim() || value.intent.length > 8192
        || !value.observation || typeof value.observation !== 'object' || Array.isArray(value.observation)
        || !value.plan || typeof value.plan !== 'object' || Array.isArray(value.plan)) throw new Error('Invalid decision request');
    return value;
}

export function normalizeAnswer(value) {
    const answer = value?.answers?.decision;
    if (answer?.type !== 'choice' || !DECISIONS.has(answer.choice)
        || !Number.isFinite(answer.confidence) || answer.confidence < 0 || answer.confidence > 1
        || typeof value.model !== 'string' || !value.model.length || value.model.length > 128) throw new Error('Invalid decision response');
    const usage = {};
    for (const key of ['input_tokens', 'output_tokens']) {
        if (Number.isSafeInteger(value.usage?.[key]) && value.usage[key] >= 0) usage[key] = value.usage[key];
    }
    return { decision: answer.choice, confidence: answer.confidence, model: value.model, usage };
}

async function boundedBody(stream, maxBytes, signal) {
    if (!stream) throw new Error('Empty body');
    const reader = stream.getReader();
    const chunks = [];
    let length = 0;
    const abort = () => { void reader.cancel().catch(() => {}); };
    signal.addEventListener('abort', abort, { once: true });
    try {
        while (true) {
            if (signal.aborted) throw new Error('Timed out');
            const { value, done } = await reader.read();
            if (signal.aborted) throw new Error('Timed out');
            if (done) break;
            length += value.byteLength;
            if (length > maxBytes) { await reader.cancel(); throw new Error('Body too large'); }
            chunks.push(value);
        }
        const all = new Uint8Array(length);
        let offset = 0;
        for (const chunk of chunks) { all.set(chunk, offset); offset += chunk.length; }
        return new TextDecoder('utf-8', { fatal: true }).decode(all);
    } finally {
        signal.removeEventListener('abort', abort);
        reader.releaseLock();
    }
}

function acquire(client, now) {
    for (const [key, state] of buckets) if (!state.active && state.started < now - 60000) buckets.delete(key);
    let state = buckets.get(client);
    if (!state) {
        if (buckets.size >= MAX_CLIENTS) return null;
        state = { started: now, count: 0, active: 0 };
        buckets.set(client, state);
    }
    if (state.started < now - 60000) { state.started = now; state.count = 0; }
    if (state.count >= 20 || state.active >= 2) return null;
    state.count++; state.active++;
    return () => { state.active--; };
}

/** Inject fetch only for offline transport tests. Keys are never stored in state. */
export function createHandler(fetchUpstream = fetch) {
    return async function handle(request, env = {}) {
        const origin = request.headers.get('Origin');
        const allowed = new Set(String(env.ALLOWED_ORIGINS || 'https://cpaci-tani.github.io').split(',').map(s => s.trim()).filter(Boolean));
        const cors = allowed.has(origin) ? { 'Access-Control-Allow-Origin': origin, 'Vary': 'Origin' } : {};
        const reply = (value, status = 200) => new Response(JSON.stringify(value), {
            status, headers: { ...cors, 'Content-Type': 'application/json', 'Cache-Control': 'no-store', 'X-Content-Type-Options': 'nosniff' },
        });
        if (!origin || !allowed.has(origin)) return reply({ error: 'Origin not allowed' }, 403);
        const route = new URL(request.url).pathname;
        if (!['/api/ai/status', '/api/ai/jev'].includes(route)) return reply({ error: 'Unknown AI route' }, 404);
        if (request.method === 'OPTIONS') return new Response(null, { status: 204, headers: {
            ...cors, 'Access-Control-Allow-Methods': 'GET, POST, OPTIONS',
            'Access-Control-Allow-Headers': 'Authorization, Content-Type', 'Access-Control-Max-Age': '600',
        } });
        if (request.method === 'GET' && route === '/api/ai/status') return reply({ available: true, byok: true, jevConfigured: false });
        if (request.method !== 'POST' || route !== '/api/ai/jev') return reply({ error: 'Method not allowed' }, 405);
        if (request.headers.get('Content-Type')?.split(';')[0].trim() !== 'application/json') return reply({ error: 'JSON required' }, 415);
        const auth = request.headers.get('Authorization') || '';
        if (!/^Bearer [\x21-\x7e]{1,4096}$/.test(auth)) return reply({ error: 'Supply your TypeSafe API key' }, 401);
        const declared = Number(request.headers.get('Content-Length'));
        if (declared > contract.maxBodyBytes) return reply({ error: 'Request exceeds 64 KiB' }, 413);
        const release = acquire(request.headers.get('CF-Connecting-IP') || 'unknown', Date.now());
        if (!release) return reply({ error: 'Decision request limit reached' }, 429);
        const controller = new AbortController();
        const timer = setTimeout(() => controller.abort(), contract.timeoutSeconds * 1000);
        try {
            let input;
            try { input = validateInput(JSON.parse(await boundedBody(request.body, contract.maxBodyBytes, controller.signal))); }
            catch { return reply({ error: 'Invalid or oversized decision request' }, controller.signal.aborted ? 408 : 400); }
            let response;
            try {
                response = await fetchUpstream(contract.upstream, {
                    method: 'POST', redirect: 'error', signal: controller.signal,
                    headers: { 'Content-Type': 'application/json', 'Authorization': auth },
                    body: JSON.stringify({ state: JSON.stringify(input), model: contract.model, questions: contract.questions }),
                });
                if (!response.ok) {
                    await response.body?.cancel();
                    return reply({ error: 'TypeSafe rejected the request' }, [401, 403].includes(response.status) ? 401 : 502);
                }
                return reply(normalizeAnswer(JSON.parse(await boundedBody(response.body, contract.maxResponseBytes, controller.signal))));
            } catch { return reply({ error: 'Decision service unavailable or returned an invalid answer' }, 502); }
        } finally {
            clearTimeout(timer);
            release();
        }
    };
}

export default { fetch: createHandler() };
