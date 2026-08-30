/**
 * Multi-owner sampler-want union for the WASM worker proxy.
 *
 * Overlay sweeps, instrument panels, and direct getters each own a set of
 * `kind@stride` keys. The worker is told to want/unwant only when the UNION
 * of those owners changes — so hiding overlays can drop O(N³) sampler work
 * without refcounting every getScale0FieldSamples call (which would leak).
 *
 * If multiple owners request the same key, realtime wins: one 60 Hz sample can
 * satisfy both consumers, while throttling it would visibly stutter the
 * realtime overlay.
 *
 * @param {(op: 'want'|'unwant', kind: string, stride: number, cadenceClass?: string) => void} apply
 * @param {(changes: Array<{op:'want'|'unwant',kind:string,stride:number,cadenceClass?:string}>) => void} [applyBatch]
 */
export function createSamplerWantSet(apply, applyBatch = null) {
    const owners = new Map();
    let union = new Set();
    let unionCadence = new Map();

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
        const nextCadence = new Map();
        const changes = [];
        for (const { keys, cadenceClass } of owners.values()) {
            for (const k of keys) {
                next.add(k);
                const current = nextCadence.get(k);
                if (!current || cadenceClass === 'realtime') {
                    nextCadence.set(k, cadenceClass);
                }
            }
        }
        for (const k of union) {
            if (!next.has(k)) {
                const { kind, stride } = parse(k);
                changes.push({ op: 'unwant', kind, stride });
            }
        }
        for (const k of next) {
            const cadenceClass = nextCadence.get(k) || 'realtime';
            if (!union.has(k) || unionCadence.get(k) !== cadenceClass) {
                const { kind, stride } = parse(k);
                changes.push({ op: 'want', kind, stride, cadenceClass });
            }
        }
        if (changes.length) {
            if (typeof applyBatch === 'function') applyBatch(changes);
            else for (const { op, kind, stride, cadenceClass } of changes) {
                apply(op, kind, stride, cadenceClass);
            }
        }
        union = next;
        unionCadence = nextCadence;
    }

    function replace(owner, nextKeys, { cadenceClass = 'realtime' } = {}) {
        const list = Array.isArray(nextKeys) ? nextKeys.filter(Boolean).map(String) : [];
        if (list.length === 0) owners.delete(owner);
        else owners.set(owner, {
            keys: new Set(list),
            cadenceClass: cadenceClass === 'bounded-instrument'
                ? 'bounded-instrument' : 'realtime',
        });
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
