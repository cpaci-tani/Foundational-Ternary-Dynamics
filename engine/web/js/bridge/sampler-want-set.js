/**
 * Multi-owner sampler-want union for the WASM worker proxy.
 *
 * Overlay sweeps, instrument panels, and direct getters each own a set of
 * `kind@stride` keys. The worker is told to want/unwant only when the UNION
 * of those owners changes — so hiding overlays can drop O(N³) sampler work
 * without refcounting every getScale0FieldSamples call (which would leak).
 *
 * @param {(op: 'want'|'unwant', kind: string, stride: number) => void} apply
 */
export function createSamplerWantSet(apply) {
    const owners = new Map();
    let union = new Set();

    function parse(key) {
        const raw = String(key);
        const at = raw.lastIndexOf('@');
        if (at < 0) return { kind: raw, stride: 2 };
        const stride = Number(raw.slice(at + 1));
        return {
            kind: raw.slice(0, at),
            stride: Number.isFinite(stride) ? stride : 2,
        };
    }

    function recompute() {
        const next = new Set();
        for (const keys of owners.values()) {
            for (const k of keys) next.add(k);
        }
        for (const k of union) {
            if (!next.has(k)) {
                const { kind, stride } = parse(k);
                apply('unwant', kind, stride);
            }
        }
        for (const k of next) {
            if (!union.has(k)) {
                const { kind, stride } = parse(k);
                apply('want', kind, stride);
            }
        }
        union = next;
    }

    function replace(owner, nextKeys) {
        const list = Array.isArray(nextKeys) ? nextKeys.filter(Boolean).map(String) : [];
        if (list.length === 0) owners.delete(owner);
        else owners.set(owner, new Set(list));
        recompute();
    }

    function clear() {
        if (owners.size === 0 && union.size === 0) return;
        owners.clear();
        recompute();
    }

    return {
        replace,
        clear,
        wanted() { return new Set(union); },
    };
}
