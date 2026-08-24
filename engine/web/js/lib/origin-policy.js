/**
 * Loopback-only policy for the dashboard's native WebSocket probe and for
 * the C++ handshake's Origin allowlist (kept in sync with ws_origin_allowed).
 *
 * Empty / "null" / file: Origin is allowed only from a loopback peer.
 * Present http(s) Origin must be localhost / 127.0.0.1 / ::1.
 */

export function wsOriginAllowed(origin, peerIsLoopback = true) {
    if (origin == null || origin === '') return !!peerIsLoopback;
    const raw = String(origin);
    if (raw === 'null' || raw === 'NULL') return !!peerIsLoopback;
    const lower = raw.toLowerCase();
    if (lower.startsWith('file:')) return !!peerIsLoopback;
    const scheme = lower.indexOf('://');
    if (scheme < 0) return false;
    const hostStart = scheme + 3;
    let host;
    if (lower[hostStart] === '[') {
        const close = lower.indexOf(']', hostStart);
        if (close < 0) return false;
        host = lower.slice(hostStart + 1, close);
    } else {
        let hostEnd = lower.length;
        for (let i = hostStart; i < lower.length; i++) {
            const c = lower[i];
            if (c === '/' || c === ':') {
                hostEnd = i;
                break;
            }
        }
        host = lower.slice(hostStart, hostEnd);
    }
    return host === 'localhost' || host === '127.0.0.1' || host === '::1';
}

export function isLoopbackPageOrigin(hrefOrOrigin) {
    if (hrefOrOrigin == null || hrefOrOrigin === '') return false;
    try {
        const url = new URL(hrefOrOrigin, 'http://127.0.0.1');
        if (url.protocol === 'file:') return true;
        const host = (url.hostname || '').replace(/^\[|\]$/g, '').toLowerCase();
        return host === 'localhost' || host === '127.0.0.1' || host === '::1';
    } catch {
        return false;
    }
}

/** Honor ?wsPort= only on loopback pages; otherwise keep the default 9100. */
export function parseNativeWsPort(search, pageHref, fallback = 9100) {
    const params = search instanceof URLSearchParams
        ? search
        : new URLSearchParams(search || '');
    const queryPort = params.get('wsPort');
    if (queryPort === null) return fallback;
    if (!isLoopbackPageOrigin(pageHref)) return fallback;
    const parsed = Number(queryPort);
    return Number.isInteger(parsed) && parsed >= 1 && parsed <= 65535
        ? parsed
        : fallback;
}

export function escapeHtml(value) {
    return String(value ?? '')
        .replace(/&/g, '&amp;')
        .replace(/</g, '&lt;')
        .replace(/>/g, '&gt;')
        .replace(/"/g, '&quot;')
        .replace(/'/g, '&#39;');
}
