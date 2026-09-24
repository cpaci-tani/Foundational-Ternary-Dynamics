/** Shared, read-only index of canonical telemetry histories.
 * Small block summaries make window statistics and pixel envelopes independent
 * of run length after the initial indexing pass. Histories remain unmodified.
 * Cache ownership follows the source buffer; a reset generation invalidates it.
 */
const BLOCK = 32;
const indexes = new WeakMap();
const empty = () => ({ count: 0, sum: 0, min: Infinity, max: -Infinity,
    minIndex: -1, maxIndex: -1, gapIndex: -1 });

function merge(a, b) {
    if (!a) return b;
    if (!b) return a;
    const low = a.min <= b.min ? a : b;
    const high = a.max >= b.max ? a : b;
    return { count: a.count + b.count, sum: a.sum + b.sum,
        min: low.min, max: high.max, minIndex: low.minIndex, maxIndex: high.maxIndex,
        gapIndex: a.gapIndex >= 0 ? a.gapIndex : b.gapIndex };
}

class HistoryIndex {
    constructor(buffer) {
        this.buffer = buffer;
        this.capacity = 1;
        this.tree = [];
        this.count = 0;
        this.total = -1;
    }

    scan(start, end) {
        const result = empty();
        for (let i = start; i < end; i++) {
            const value = this.buffer.get(i);
            if (!Number.isFinite(value)) {
                if (result.gapIndex < 0) result.gapIndex = i;
                continue;
            }
            result.count++;
            result.sum += value;
            if (value < result.min) { result.min = value; result.minIndex = i; }
            if (value > result.max) { result.max = value; result.maxIndex = i; }
        }
        return result;
    }

    sync() {
        const b = this.buffer;
        const count = b.count || 0;
        const total = b.total ?? count;
        const generation = b.generation ?? b.parent?.generation ?? 0;
        const firstTick = count ? b.getTick?.(0) : undefined;
        const last = count ? b.get(count - 1) : NaN;
        if (this.generation === generation && this.count === count && this.total === total
            && Object.is(this.last, last) && Object.is(this.firstTick, firstTick)) return;
        if (this.generation !== generation || count < this.count || total < this.total
            || !Object.is(this.firstTick, firstTick)
            || total - this.total !== count - this.count) {
            this.count = 0;
            this.tree = [];
        }
        const blocks = Math.ceil(count / BLOCK);
        if (blocks > this.capacity) {
            const oldTree = this.tree, oldCapacity = this.capacity;
            while (this.capacity < blocks) this.capacity *= 2;
            this.tree = [];
            for (let i = 0; i < Math.ceil(this.count / BLOCK); i++) {
                this.tree[this.capacity + i] = oldTree[oldCapacity + i];
            }
            for (let i = this.capacity - 1; i > 0; i--) {
                this.tree[i] = merge(this.tree[i * 2], this.tree[i * 2 + 1]);
            }
        }
        // Refresh the previous tail as well: setLast() may refine it before
        // an appended sample arrives in the same publication transaction.
        for (let block = Math.floor(Math.max(0, this.count - 1) / BLOCK); block < blocks; block++) {
            let node = this.capacity + block;
            this.tree[node] = this.scan(block * BLOCK, Math.min(count, (block + 1) * BLOCK));
            while (node > 1) {
                node = Math.floor(node / 2);
                this.tree[node] = merge(this.tree[node * 2], this.tree[node * 2 + 1]);
            }
        }
        Object.assign(this, { count, total, generation, firstTick, last });
    }

    range(start, end) {
        if (end <= start) return empty();
        const firstBlock = Math.ceil(start / BLOCK);
        const lastBlock = Math.floor(end / BLOCK);
        if (firstBlock >= lastBlock) return this.scan(start, end);
        let result = this.scan(start, firstBlock * BLOCK);
        let left = firstBlock + this.capacity, right = lastBlock + this.capacity;
        while (left < right) {
            if (left % 2) result = merge(result, this.tree[left++]);
            if (right % 2) result = merge(result, this.tree[--right]);
            left = Math.floor(left / 2); right = Math.floor(right / 2);
        }
        return merge(result, this.scan(lastBlock * BLOCK, end));
    }
}

function indexFor(buffer) {
    let index = indexes.get(buffer);
    if (!index) { index = new HistoryIndex(buffer); indexes.set(buffer, index); }
    index.sync();
    return index;
}

export function getHistoryStats(buffer, start = 0, count = buffer?.count || 0) {
    if (!buffer || typeof buffer.get !== 'function') return { ...empty(), avg: null };
    start = Math.max(0, start);
    const end = Math.min(buffer.count, start + count);
    // Opening a short rolling window late in a long run must not index the
    // whole run. Small windows are cheaper to scan directly and remain bounded.
    const result = end - start <= 512
        ? new HistoryIndex(buffer).scan(start, end)
        : indexFor(buffer).range(start, end);
    return { ...result, avg: result.count ? result.sum / result.count : null };
}

/** Actual sample indices, including extrema and missing-data breaks per pixel
 * bucket. This is a drawing projection, never a replacement telemetry source.
 * Aligned channels may share the projection to preserve each channel's peaks.
 */
export function projectHistoryIndices(buffers, count, width = 320) {
    const first = buffers[0];
    const start = Math.max(0, first.count - count);
    const budget = Math.max(32, Math.min(4096, Math.round(width || 320) * 2));
    if (count <= budget) return null;
    const sources = buffers.filter(Boolean).map(indexFor);
    const buckets = Math.max(1, Math.floor((budget - 2) / (3 * sources.length)));
    const selected = new Set([start, first.count - 1]);
    for (let bucket = 0; bucket < buckets; bucket++) {
        const lo = start + Math.floor(count * bucket / buckets);
        const hi = start + Math.floor(count * (bucket + 1) / buckets);
        for (const source of sources) {
            const range = source.range(lo, hi);
            for (const index of [range.minIndex, range.maxIndex, range.gapIndex]) {
                if (index >= lo && index < hi) selected.add(index);
            }
        }
    }
    return Array.from(selected).sort((a, b) => a - b);
}
