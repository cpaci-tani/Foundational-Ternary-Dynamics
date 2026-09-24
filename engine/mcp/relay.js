import { randomUUID } from 'node:crypto';

const MAX_REQUEST_BYTES = 128 * 1024;
const MAX_RESPONSE_BYTES = 2 * 1024 * 1024;
export const CALL_TIMEOUT_MS = 60000;

/** Credentials are process-memory values; never include them in errors. */
export function readConfig(env = process.env) {
    const source = env.FTD_MCP_URL || 'http://127.0.0.1:8080';
    if (!/^http:\/\/(?:127\.0\.0\.1|localhost|\[::1\])(?::[0-9]{1,5})?\/?$/.test(source))
        throw new Error('FTD_MCP_URL must be an HTTP loopback base URL without credentials, path, query, or fragment.');
    let url;
    try { url = new URL(source); } catch { throw new Error('FTD_MCP_URL contains an invalid port.'); }
    if (url.port === '0') throw new Error('FTD_MCP_URL must use a nonzero port.');
    const sessionId = env.FTD_MCP_SESSION, token = env.FTD_MCP_TOKEN;
    if (typeof sessionId !== 'string' || !/^[a-zA-Z0-9_.:-]{1,240}$/.test(sessionId)) throw new Error('FTD_MCP_SESSION is required and must be a valid session identifier.');
    if (typeof token !== 'string' || !/^[\x21-\x7e]{1,2048}$/.test(token)) throw new Error('FTD_MCP_TOKEN is required and must contain visible ASCII without spaces.');
    return Object.freeze({ baseUrl: url.origin, sessionId, token });
}

/** Do not forward credentials even if an upstream error accidentally echoes them. */
function redact(value, config) {
    if (typeof value === 'string') return [config.token, config.sessionId]
        .reduce((text, secret) => text.split(secret).join('[redacted]'), value);
    if (Array.isArray(value)) return value.map(item => redact(item, config));
    if (value && typeof value === 'object') return Object.fromEntries(Object.entries(value)
        .map(([key, item]) => [redact(key, config), redact(item, config)]));
    return value;
}

async function readBounded(response) {
    const length = response.headers.get('content-length');
    if (length !== null && Number(length) > MAX_RESPONSE_BYTES) { await response.body?.cancel(); throw new Error('Relay response exceeds the size limit.'); }
    if (!response.body) throw new Error('Relay response is empty.');
    const reader = response.body.getReader();
    const chunks = []; let bytes = 0;
    try {
        while (true) {
            const { done, value } = await reader.read(); if (done) break;
            bytes += value.byteLength;
            if (bytes > MAX_RESPONSE_BYTES) throw new Error('Relay response exceeds the size limit.');
            chunks.push(value);
        }
    } catch (error) { await reader.cancel().catch(() => {}); throw error; }
    return JSON.parse(Buffer.concat(chunks).toString('utf8'));
}

export class RelayClient {
    constructor(config, { fetchImpl = fetch, timeoutMs = CALL_TIMEOUT_MS } = {}) {
        this.config = config; this.fetch = fetchImpl; this.timeoutMs = timeoutMs;
    }
    async cancel(id) {
        const response = await this.fetch(`${this.config.baseUrl}/api/mcp/cancel`, {
            method: 'POST', redirect: 'error', signal: AbortSignal.timeout(3000),
            headers: { 'content-type': 'application/json', authorization: `Bearer ${this.config.token}` },
            body: JSON.stringify({ sessionId: this.config.sessionId, id }),
        });
        await response.body?.cancel();
    }
    async call(method, args, signal) {
        const id = randomUUID();
        const body = JSON.stringify({ sessionId: this.config.sessionId, id, method, args });
        if (Buffer.byteLength(body) > MAX_REQUEST_BYTES) return { error: { code: 'REQUEST_TOO_LARGE', message: 'Tool request exceeds 128 KiB.' } };
        if (signal?.aborted) return { error: { code: 'CANCELLED', message: 'Request cancelled before dispatch.' } };
        const timeout = AbortSignal.timeout(this.timeoutMs);
        const combined = signal ? AbortSignal.any([signal, timeout]) : timeout;
        let cancellation;
        const abort = () => { cancellation = this.cancel(id).catch(() => {}); };
        combined.addEventListener('abort', abort, { once: true });
        try {
            const response = await this.fetch(`${this.config.baseUrl}/api/mcp/call`, {
                method: 'POST', redirect: 'error', signal: combined,
                headers: { 'content-type': 'application/json', authorization: `Bearer ${this.config.token}` }, body,
            });
            const data = await readBounded(response);
            if (!data || typeof data !== 'object' || Array.isArray(data)
                || !Object.hasOwn(data, 'result') && !Object.hasOwn(data, 'error')) throw new Error('Invalid relay response.');
            if (!response.ok && !data.error) throw new Error('Relay rejected the request.');
            return redact(data, this.config);
        } catch {
            return { error: { code: timeout.aborted ? 'TIMEOUT' : signal?.aborted ? 'CANCELLED' : 'RELAY_UNAVAILABLE',
                message: combined.aborted ? 'Request interrupted after dispatch. Inspect run status or observe before retrying.'
                    : 'Local relay did not return a valid response. Inspect run status or observe before retrying.', outcome: 'unknown' } };
        } finally {
            combined.removeEventListener('abort', abort);
            await cancellation;
        }
    }
}
